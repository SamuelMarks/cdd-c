/**
 * @file refactor_orchestrator.c
 * @brief Implementation of the refactoring orchestrator and propagation engine.
 * @author Samuel Marks
 */

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <c89stringutils_string_extras.h>
#include <c_str_span.h>

#include "analysis.h"
#include "cst_parser.h"
#include "fs.h"
#include "refactor_orchestrator.h"
#include "rewriter_body.h"
#include "rewriter_sig.h"
#include "tokenizer.h"

#if defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__)
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#define strdup _strdup
#endif
#else
#include <sys/errno.h>
#endif

/* --- Graph Types --- */

/**
 * @brief Node in the function call dependency graph.
 */
struct FuncNode {
  size_t node_idx; /**< Index in the CST node list */
  char *name;      /**< Function name */

  /* Signature properties */
  int returns_void; /**< 1 if returns void */
  int returns_ptr;  /**< 1 if returns pointer/complex type */
  int is_main;      /**< 1 if function is 'main' */

  /* Analysis state */
  int contains_allocs;     /**< 1 if body contains allocations */
  int marked_for_refactor; /**< 1 if this function must change signature/logic
                            */

  /* Parsing ranges */
  size_t token_start; /**< Token index of start of definition */
  size_t body_start;  /**< Token index of '{' */
  size_t token_end;   /**< Token index of end of definition */

  /* Dependencies (Adjacency List) */
  size_t *callers;      /**< Array of indices of functions calling this node */
  size_t num_callers;   /**< Count of callers */
  size_t alloc_callers; /**< Capacity of callers array */
};

/**
 * @brief The Dependency Graph container.
 */
struct DependencyGraph {
  struct FuncNode *nodes; /**< Array of nodes */
  size_t count;           /**< Number of function nodes */
};

/* --- Helpers --- */

static int get_token_slice(const struct TokenList *src, size_t start,
                           size_t end, struct TokenList *dst) {
  if (start >= src->size || end > src->size || start > end)
    return EINVAL;
  dst->tokens = src->tokens + start;
  dst->size = end - start;
  dst->capacity = 0;
  return 0;
}

static size_t find_token_in_range(const struct TokenList *tokens, size_t start,
                                  size_t end, enum TokenKind kind) {
  size_t i;
  for (i = start; i < end; ++i) {
    if (tokens->tokens[i].kind == kind)
      return i;
  }
  return end;
}

static int token_eq_str(const struct Token *tok, const char *s) {
  size_t len = strlen(s);
  return (tok->length == len && strncmp((const char *)tok->start, s, len) == 0);
}

/* Extract just the function name before the parens */
static char *extract_func_name(const struct TokenList *tokens, size_t start,
                               size_t body_start) {
  size_t lparen = find_token_in_range(tokens, start, body_start, TOKEN_LPAREN);
  size_t i;
  if (lparen == body_start)
    return NULL;

  i = lparen;
  while (i > start) {
    i--;
    if (tokens->tokens[i].kind == TOKEN_WHITESPACE)
      continue;
    if (tokens->tokens[i].kind == TOKEN_IDENTIFIER) {
      size_t len = tokens->tokens[i].length;
      char *name = malloc(len + 1);
      if (!name)
        return NULL;
      memcpy(name, tokens->tokens[i].start, len);
      name[len] = '\0';
      return name;
    }
  }
  return NULL;
}

/* Analyze simple return traits */
static void analyze_signature_tokens(const struct TokenList *tokens,
                                     size_t start, size_t body_start,
                                     int *is_ptr, int *is_void) {
  size_t i;
  *is_ptr = 0;
  *is_void = 0;

  /* Iterate tokens before body */
  for (i = start; i < body_start; ++i) {
    const struct Token *tok = &tokens->tokens[i];
    if (tok->kind == TOKEN_OTHER && tok->length == 1 && *tok->start == '*') {
      *is_ptr = 1;
    } else if (tok->kind == TOKEN_IDENTIFIER) {
      if (token_eq_str(tok, "void")) {
        *is_void = 1;
      }
    }
  }
  /* Pointer takes precedence (e.g. void*) */
  if (*is_ptr)
    *is_void = 0;
}

/* Graph Management */

static int graph_add_node(struct DependencyGraph *g, size_t idx,
                          const char *name) {
  g->nodes[idx].node_idx = idx;
  g->nodes[idx].name = strdup(name);
  if (!g->nodes[idx].name)
    return ENOMEM;
  g->nodes[idx].callers = NULL;
  g->nodes[idx].num_callers = 0;
  g->nodes[idx].alloc_callers = 0;

  g->nodes[idx].marked_for_refactor = 0;
  g->nodes[idx].contains_allocs = 0;
  g->nodes[idx].is_main = (strcmp(name, "main") == 0);

  return 0;
}

static int graph_add_edge(struct DependencyGraph *g, size_t caller_idx,
                          size_t callee_idx) {
  struct FuncNode *callee = &g->nodes[callee_idx];
  size_t i;
  for (i = 0; i < callee->num_callers; i++) {
    if (callee->callers[i] == caller_idx)
      return 0;
  }

  if (callee->num_callers >= callee->alloc_callers) {
    size_t new_cap = callee->alloc_callers == 0 ? 4 : callee->alloc_callers * 2;
    size_t *new_arr = realloc(callee->callers, new_cap * sizeof(size_t));
    if (!new_arr)
      return ENOMEM;
    callee->callers = new_arr;
    callee->alloc_callers = new_cap;
  }
  callee->callers[callee->num_callers++] = caller_idx;
  return 0;
}

static void graph_free_contents(struct DependencyGraph *g) {
  size_t i;
  if (!g || !g->nodes)
    return;
  for (i = 0; i < g->count; i++) {
    free(g->nodes[i].name);
    free(g->nodes[i].callers);
  }
  free(g->nodes);
  g->nodes = NULL;
  g->count = 0;
}

/* Recursive Propagation */
static void propagate_refactor_mark(struct DependencyGraph *g, size_t idx) {
  struct FuncNode *node = &g->nodes[idx];
  size_t i;

  if (node->marked_for_refactor)
    return;

  node->marked_for_refactor = 1;

  for (i = 0; i < node->num_callers; i++) {
    size_t caller_idx = node->callers[i];
    propagate_refactor_mark(g, caller_idx);
  }
}

/* --- Orchestration Logic --- */

int orchestrate_fix(const char *const source_code, char **const out_code) {
  struct TokenList *tokens = NULL;
  struct CstNodeList cst = {0};
  struct AllocationSiteList allocs = {0};
  struct DependencyGraph graph = {0};
  struct RefactoredFunction *ref_funcs = NULL;
  char *output = NULL;
  size_t i;
  size_t marked_count = 0;
  int rc = 0;

  /* Phase 1: Parse */
  if (!source_code || !out_code)
    return EINVAL;

  if ((rc = tokenize(az_span_create_from_str((char *)source_code), &tokens)) !=
      0)
    return rc;

  if ((rc = parse_tokens(tokens, &cst)) != 0) {
    free_token_list(tokens);
    return rc;
  }

  /* Phase 2: Allocation Analysis */
  if ((rc = find_allocations(tokens, &allocs)) != 0) {
    free_cst_node_list(&cst);
    free_token_list(tokens);
    return rc;
  }

  /* Phase 3: Graph Construction */
  for (i = 0; i < cst.size; ++i) {
    if (cst.nodes[i].kind == CST_NODE_FUNCTION)
      graph.count++;
  }

  if (graph.count > 0) {
    graph.nodes = calloc(graph.count, sizeof(struct FuncNode));
    if (!graph.nodes) {
      rc = ENOMEM;
      goto cleanup;
    }
  }

  {
    size_t f_idx = 0;
    size_t current_tok_idx = 0;

    for (i = 0; i < cst.size; i++) {
      const uint8_t *node_end_ptr = cst.nodes[i].start + cst.nodes[i].length;
      size_t start_idx = current_tok_idx;
      size_t end_idx = start_idx;

      while (end_idx < tokens->size) {
        if ((tokens->tokens[end_idx].start + tokens->tokens[end_idx].length) >
            node_end_ptr)
          break;
        end_idx++;
      }
      current_tok_idx = end_idx;

      if (cst.nodes[i].kind == CST_NODE_FUNCTION) {
        struct FuncNode *fn = &graph.nodes[f_idx];
        char *name;

        fn->token_start = start_idx;
        fn->token_end = end_idx;
        fn->body_start =
            find_token_in_range(tokens, start_idx, end_idx, TOKEN_LBRACE);

        name = extract_func_name(tokens, start_idx, fn->body_start);
        if (!name)
          name = strdup("");

        rc = graph_add_node(&graph, f_idx, name);
        free(name);
        if (rc != 0)
          goto cleanup;

        analyze_signature_tokens(tokens, start_idx, fn->body_start,
                                 &fn->returns_ptr, &fn->returns_void);

        {
          size_t k;
          for (k = 0; k < allocs.size; k++) {
            if (allocs.sites[k].token_index >= fn->body_start &&
                allocs.sites[k].token_index < fn->token_end) {
              fn->contains_allocs = 1;
              break;
            }
          }
        }
        f_idx++;
      }
    }

    for (f_idx = 0; f_idx < graph.count; f_idx++) {
      struct FuncNode *caller = &graph.nodes[f_idx];
      size_t t;
      for (t = caller->body_start; t < caller->token_end; t++) {
        if (tokens->tokens[t].kind == TOKEN_IDENTIFIER) {
          size_t target_idx;
          for (target_idx = 0; target_idx < graph.count; target_idx++) {
            if (f_idx == target_idx)
              continue;
            if (token_eq_str(&tokens->tokens[t],
                             graph.nodes[target_idx].name)) {
              rc = graph_add_edge(&graph, f_idx, target_idx);
              if (rc != 0)
                goto cleanup;
            }
          }
        }
      }
    }
  }

  /* Phase 4 & 5: Seeding and Propagation */
  for (i = 0; i < graph.count; i++) {
    struct FuncNode *n = &graph.nodes[i];
    int seed = 0;

    if (n->contains_allocs) {
      if (n->returns_void || n->returns_ptr) {
        seed = 1;
      }
    }

    if (seed) {
      propagate_refactor_mark(&graph, i);
    }
  }

  if (graph.count > 0) {
    for (i = 0; i < graph.count; i++) {
      if (graph.nodes[i].marked_for_refactor)
        marked_count++;
    }

    if (marked_count > 0) {
      ref_funcs = calloc(marked_count, sizeof(struct RefactoredFunction));
      if (!ref_funcs) {
        rc = ENOMEM;
        goto cleanup;
      }
      {
        size_t r = 0;
        for (i = 0; i < graph.count; i++) {
          if (graph.nodes[i].marked_for_refactor) {
            ref_funcs[r].name = graph.nodes[i].name;
            if (graph.nodes[i].returns_ptr)
              ref_funcs[r].type = REF_PTR_TO_INT_OUT;
            else
              ref_funcs[r].type = REF_VOID_TO_INT;
            r++;
          }
        }
      }
    }
  }

  /* Phase 6: Rewrite */
  {
    size_t f_idx = 0;
    size_t current_tok_offset = 0;

    output = strdup("");
    if (!output) {
      rc = ENOMEM;
      goto cleanup;
    }

    for (i = 0; i < cst.size; ++i) {
      const uint8_t *node_end_ptr = cst.nodes[i].start + cst.nodes[i].length;
      size_t start_idx = current_tok_offset;
      size_t end_idx = start_idx;
      while (end_idx < tokens->size) {
        if ((tokens->tokens[end_idx].start + tokens->tokens[end_idx].length) >
            node_end_ptr)
          break;
        end_idx++;
      }
      current_tok_offset = end_idx;

      if (cst.nodes[i].kind == CST_NODE_FUNCTION) {
        struct FuncNode *node = &graph.nodes[f_idx];
        char *segment = NULL;

        if (node->marked_for_refactor && !node->is_main) {
          struct TokenList sig_slice, body_slice;
          char *new_sig = NULL, *new_body = NULL;
          struct SignatureTransform trans;

          if (get_token_slice(tokens, start_idx, node->body_start,
                              &sig_slice) == 0 &&
              get_token_slice(tokens, node->body_start, end_idx, &body_slice) ==
                  0) {

            if (rewrite_signature(&sig_slice, &new_sig) == 0) {
              trans.type = node->returns_ptr ? TRANSFORM_RET_PTR_TO_ARG
                                             : TRANSFORM_VOID_TO_INT;
              trans.arg_name = "out";
              trans.success_code = "0";
              trans.error_code = "ENOMEM";

              {
                struct AllocationSiteList local_allocs;
                size_t k;

                allocation_site_list_init(&local_allocs); /* allocs a block */
                for (k = 0; k < allocs.size; k++) {
                  if (allocs.sites[k].token_index >= node->body_start &&
                      allocs.sites[k].token_index < end_idx) {
                    struct AllocationSite site = allocs.sites[k];
                    site.token_index -= node->body_start; /* Relativize */
                    if (local_allocs.size >= local_allocs.capacity) {
                      size_t nc = local_allocs.capacity * 2;
                      local_allocs.sites =
                          realloc(local_allocs.sites,
                                  nc * sizeof(struct AllocationSite));
                      local_allocs.capacity = nc;
                    }
                    if (site.var_name)
                      site.var_name = strdup(site.var_name);
                    local_allocs.sites[local_allocs.size++] = site;
                  }
                }

                if (rewrite_body(&body_slice, &local_allocs, ref_funcs,
                                 marked_count, &trans, &new_body) == 0) {
                  asprintf(&segment, "%s %s", new_sig, new_body);
                  free(new_sig);
                  free(new_body);
                }

                allocation_site_list_free(&local_allocs);
              }
            }
          }
        } else if (node->marked_for_refactor && node->is_main) {
          struct TokenList body_slice;
          char *new_body = NULL;
          struct SignatureTransform trans = {TRANSFORM_NONE, NULL, NULL, NULL};

          if (get_token_slice(tokens, node->body_start, end_idx, &body_slice) ==
              0) {
            struct AllocationSiteList local_allocs;
            size_t k;
            allocation_site_list_init(&local_allocs);
            for (k = 0; k < allocs.size; k++) {
              if (allocs.sites[k].token_index >= node->body_start &&
                  allocs.sites[k].token_index < end_idx) {
                struct AllocationSite site = allocs.sites[k];
                site.token_index -= node->body_start;
                if (site.var_name)
                  site.var_name = strdup(site.var_name);
                if (local_allocs.size >= local_allocs.capacity) {
                  local_allocs.sites = realloc(
                      local_allocs.sites, (local_allocs.capacity *= 2) *
                                              sizeof(struct AllocationSite));
                }
                local_allocs.sites[local_allocs.size++] = site;
              }
            }

            if (rewrite_body(&body_slice, &local_allocs, ref_funcs,
                             marked_count, &trans, &new_body) == 0) {
              /* Reconstruct main sig (original tokens) + new body */
              struct TokenList sig_slice;
              char *sig_str;
              size_t t;
              size_t off = 0;

              get_token_slice(tokens, start_idx, node->body_start, &sig_slice);
              sig_str = malloc(1024); /* dangerous fixed size */
              sig_str[0] = 0;
              for (t = 0; t < sig_slice.size; t++) {
                memcpy(sig_str + off, sig_slice.tokens[t].start,
                       sig_slice.tokens[t].length);
                off += sig_slice.tokens[t].length;
              }
              sig_str[off] = 0;
              asprintf(&segment, "%s %s", sig_str, new_body);
              free(sig_str);
              free(new_body);
            }
            allocation_site_list_free(&local_allocs);
          }
        }

        if (segment) {
          char *joined;
          asprintf(&joined, "%s%s", output, segment);
          free(output);
          output = joined;
          free(segment);
        } else {
          size_t k;
          for (k = start_idx; k < end_idx; ++k) {
            size_t old_len = strlen(output);
            size_t add_len = tokens->tokens[k].length;
            output = realloc(output, old_len + add_len + 1);
            memcpy(output + old_len, tokens->tokens[k].start, add_len);
            output[old_len + add_len] = '\0';
          }
        }
        f_idx++;
        continue;
      }

      {
        size_t k;
        for (k = start_idx; k < end_idx; ++k) {
          size_t old_len = strlen(output);
          size_t add_len = tokens->tokens[k].length;
          output = realloc(output, old_len + add_len + 1);
          memcpy(output + old_len, tokens->tokens[k].start, add_len);
          output[old_len + add_len] = '\0';
        }
      }
    }
  }

  *out_code = output;
  output = NULL;

cleanup:
  if (ref_funcs)
    free(ref_funcs);
  graph_free_contents(&graph);
  if (output)
    free(output);
  free_cst_node_list(&cst);
  allocation_site_list_free(&allocs);
  free_token_list(tokens);

  return rc;
}

int fix_code_main(int argc, char **argv) {
  const char *in_file = NULL;
  const char *out_file = NULL;
  char *content = NULL;
  size_t sz = 0;
  char *result = NULL;
  int rc;

  if (argc != 2) {
    fprintf(stderr, "Usage: fix_code <input.c> <output.c>\n");
    return EXIT_FAILURE;
  }

  in_file = argv[0];
  out_file = argv[1];

  if ((rc = read_to_file(in_file, "r", &content, &sz)) != 0) {
    fprintf(stderr, "Failed to read input file %s\n", in_file);
    return EXIT_FAILURE;
  }

  rc = orchestrate_fix(content, &result);
  free(content);

  if (rc != 0) {
    fprintf(stderr, "Refactoring failed with code %d\n", rc);
    return EXIT_FAILURE;
  }

  {
    FILE *f;
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
    errno_t err = fopen_s(&f, out_file, "w, ccs=UTF-8");
    if (err != 0 || !f)
      rc = EXIT_FAILURE;
#else
    f = fopen(out_file, "w");
    if (!f)
      rc = EXIT_FAILURE;
#endif
    if (f) {
      fputs(result, f);
      fclose(f);
    }
  }

  free(result);
  return rc == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
