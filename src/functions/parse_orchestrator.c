/**
 * @file refactor_orchestrator.c
 * @brief Implementation of the refactoring orchestrator.
 *
 * Coordinates the full "fix" pipeline:
 * 1. Tokenize & Parse Source.
 * 2. Analyze Allocations (Identify sources of potential failure).
 * 3. Build Dependency Graph (Identify who calls unsafe functions).
 * 4. Propagate (Refactor signatures up the call stack).
 * 5. Rewrite (Apply Patching to signatures and bodies).
 *
 * Integrates `cst_parser`, `analysis`, `rewriter_sig`, and `rewriter_body`.
 *
 * @author Samuel Marks
 */

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <c_str_span.h>

#include "functions/emit_rewriter_body.h"
#include "functions/emit_rewriter_sig.h"
#include "functions/parse_analysis.h"
#include "functions/parse_cst.h"
#include "functions/parse_fs.h"
#include "functions/parse_orchestrator.h"
#include "functions/parse_str.h" /* For c_cdd_strdup */
#include "functions/parse_tokenizer.h"

#if defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__)
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#define strdup _strdup
#define strcasecmp _stricmp
#endif
#else
#include <strings.h>
#include <sys/errno.h>
#endif

/* --- Graph Data Structures --- */

/**
 * @brief Represents a function in the call graph.
 */
struct FuncNode {
  size_t node_idx; /**< Index in the CST node list */
  char *name;      /**< Name of the function */

  /* Analyzed Signature Properties */
  int returns_void; /**< True if function currently returns void */
  int returns_ptr;  /**< True if function currently returns a pointer/struct */
  char *original_return_type; /**< String literal of return type (needed for
                                 temp vars) */
  int is_main;                /**< Special handling for main() entry point */

  /* Flags */
  int contains_allocs;     /**< True if body performs allocations */
  int marked_for_refactor; /**< True if signature/body needs rewriting */

  /* Range Metadata */
  size_t token_start; /**< Func start index in token list */
  size_t body_start;  /**< Index of opening brace '{' */
  size_t token_end;   /**< Func end index (exclusive) */

  /* Directed Graph Edges (Reverse Call Graph) */
  size_t *callers;      /**< Indices of functions that call this function */
  size_t num_callers;   /**< Count of callers */
  size_t alloc_callers; /**< Capacity of array */
};

/**
 * @brief Container for the dependency graph.
 */
struct DependencyGraph {
  struct FuncNode *nodes; /**< Array of function nodes */
  size_t count;           /**< Total number of functions */
};

/* --- Helpers --- */

/**
 * @brief Extract a slice of tokens into a temporary view.
 * Does not copy token data, just pointers.
 */
static int get_token_slice(const struct TokenList *src, size_t start,
                           size_t end, struct TokenList *dst) {
  if (start >= src->size || end > src->size || start > end)
    return EINVAL;
  dst->tokens = src->tokens + start;
  dst->size = end - start;
  dst->capacity = 0; /* Marker: do not free */
  return 0;
}

/* Helper to find specific token type within range */
static size_t find_token_in_range(const struct TokenList *tokens, size_t start,
                                  size_t end, enum TokenKind kind) {
  size_t i;
  for (i = start; i < end; ++i) {
    if (tokens->tokens[i].kind == kind)
      return i;
  }
  return end;
}

/* Helper for token string comparison */
static int token_eq_str(const struct Token *tok, const char *s) {
  size_t len = strlen(s);
  return (tok->length == len && strncmp((const char *)tok->start, s, len) == 0);
}

/**
 * @brief Extract function name from tokens.
 * Finds the identifier immediately preceding the argument list LPAREN.
 */
static char *extract_func_name(const struct TokenList *tokens, size_t start,
                               size_t body_start) {
  size_t lparen = find_token_in_range(tokens, start, body_start, TOKEN_LPAREN);
  size_t i;
  if (lparen == body_start)
    return NULL;

  /* Backtrack from LPAREN to find Identifier */
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

/**
 * @brief Join tokens into a single string.
 */
static char *join_tokens_str(const struct TokenList *tokens, size_t start,
                             size_t end) {
  size_t len = 0;
  size_t i;
  char *buf, *p;
  if (start >= end)
    return c_cdd_strdup("");
  for (i = start; i < end; ++i)
    len += tokens->tokens[i].length;
  buf = malloc(len + 1);
  if (!buf)
    return NULL;
  p = buf;
  for (i = start; i < end; ++i) {
    memcpy(p, tokens->tokens[i].start, tokens->tokens[i].length);
    p += tokens->tokens[i].length;
  }
  *p = '\0';
  return buf;
}

/**
 * @brief Analyze return type tokens to determine void/int/pointer status.
 */
static void analyze_signature_tokens(const struct TokenList *tokens,
                                     size_t start, size_t body_start,
                                     int *is_ptr, int *is_void,
                                     char **type_str) {
  size_t i;
  size_t lparen = find_token_in_range(tokens, start, body_start, TOKEN_LPAREN);
  size_t name_end_idx = 0;

  *is_ptr = 0;
  *is_void = 0;
  *type_str = NULL;

  if (lparen == body_start)
    return;

  /* Find name start to delimit type end */
  i = lparen;
  while (i > start) {
    i--;
    if (tokens->tokens[i].kind == TOKEN_WHITESPACE)
      continue;
    if (tokens->tokens[i].kind == TOKEN_IDENTIFIER) {
      name_end_idx = i;
      break;
    }
  }

  /* Capture return type string [start, name_end_idx) */
  *type_str = join_tokens_str(tokens, start, name_end_idx);

  /* Check properties */
  for (i = start; i < name_end_idx; ++i) {
    const struct Token *tok = &tokens->tokens[i];
    if (tok->kind == TOKEN_STAR) {
      *is_ptr = 1;
    } else if (tok->kind == TOKEN_KEYWORD_VOID) {
      *is_void = 1;
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

/* --- Graph Management --- */

static int graph_add_node(struct DependencyGraph *g, size_t idx,
                          const char *name) {
  g->nodes[idx].node_idx = idx;
  g->nodes[idx].name = c_cdd_strdup(name);
  if (!g->nodes[idx].name)
    return ENOMEM;
  g->nodes[idx].callers = NULL;
  g->nodes[idx].num_callers = 0;
  g->nodes[idx].alloc_callers = 0;
  g->nodes[idx].original_return_type = NULL;

  g->nodes[idx].marked_for_refactor = 0;
  g->nodes[idx].contains_allocs = 0;
  g->nodes[idx].is_main = (strcmp(name, "main") == 0);

  return 0;
}

static int graph_add_edge(struct DependencyGraph *g, size_t caller_idx,
                          size_t callee_idx) {
  struct FuncNode *callee = &g->nodes[callee_idx];
  size_t i;
  /* Prevent duplicate edges */
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
    free(g->nodes[i].original_return_type);
  }
  free(g->nodes);
  g->nodes = NULL;
  g->count = 0;
}

/* --- Propagation Logic --- */

static void propagate_refactor_mark(struct DependencyGraph *g, size_t idx) {
  struct FuncNode *node = &g->nodes[idx];
  size_t i;

  if (node->marked_for_refactor)
    return;

  /* Mark current node */
  node->marked_for_refactor = 1;

  /* If main function, mark for modification but stop propagating upwards */
  if (node->is_main)
    return;

  /* Recurse to all callers */
  for (i = 0; i < node->num_callers; i++) {
    size_t caller_idx = node->callers[i];
    propagate_refactor_mark(g, caller_idx);
  }
}

/* --- Main Orchestrator --- */

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

  if (!source_code || !out_code)
    return EINVAL;

  /* 1. Parse */
  if ((rc = tokenize(az_span_create_from_str((char *)source_code), &tokens)) !=
      0)
    return rc;

  if ((rc = parse_tokens(tokens, &cst)) != 0) {
    free_token_list(tokens);
    return rc;
  }

  /* 2. Analyze Allocations */
  if ((rc = find_allocations(tokens, &allocs)) != 0) {
    free_cst_node_list(&cst);
    free_token_list(tokens);
    return rc;
  }

  /* 3. Build Graph */
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

  /* Populate Functions */
  {
    size_t f_idx = 0;
    size_t current_tok_idx = 0;

    for (i = 0; i < cst.size; i++) {
      /* Calculate bounds of node */
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
          name = c_cdd_strdup("");

        rc = graph_add_node(&graph, f_idx, name);
        free(name);
        if (rc != 0)
          goto cleanup;

        analyze_signature_tokens(tokens, start_idx, fn->body_start,
                                 &fn->returns_ptr, &fn->returns_void,
                                 &fn->original_return_type);

        /* Check for allocs in this function's scope */
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

    /* Populate Edges */
    for (f_idx = 0; f_idx < graph.count; f_idx++) {
      struct FuncNode *caller = &graph.nodes[f_idx];
      size_t t;
      for (t = caller->body_start; t < caller->token_end; t++) {
        if (tokens->tokens[t].kind == TOKEN_IDENTIFIER) {
          /* Heuristic: Call is Identifier + LPAREN */
          size_t next = t + 1;
          while (next < tokens->size &&
                 tokens->tokens[next].kind == TOKEN_WHITESPACE)
            next++;
          if (next < tokens->size &&
              tokens->tokens[next].kind == TOKEN_LPAREN) {
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
  }

  /* 4. Propagate Safety Requirements */
  for (i = 0; i < graph.count; i++) {
    struct FuncNode *n = &graph.nodes[i];
    /* Seed: Function contains allocations and returns unsafe type */
    if (n->contains_allocs && (n->returns_void || n->returns_ptr)) {
      propagate_refactor_mark(&graph, i);
    }
  }

  /* 5. Rewrite */
  if (graph.count > 0) {
    /* Build Refactor Context List for rewriter */
    for (i = 0; i < graph.count; i++)
      if (graph.nodes[i].marked_for_refactor)
        marked_count++;

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
            ref_funcs[r].original_return_type =
                graph.nodes[i].original_return_type;
            ref_funcs[r].type = graph.nodes[i].returns_ptr ? REF_PTR_TO_INT_OUT
                                                           : REF_VOID_TO_INT;
            r++;
          }
        }
      }
    }
  }

  {
    /* Reconstruct file node by node */
    size_t f_idx = 0;
    size_t current_tok_offset = 0;
    output = c_cdd_strdup("");
    if (!output) {
      rc = ENOMEM;
      goto cleanup;
    }

    for (i = 0; i < cst.size; ++i) {
      /* Identify Node range */
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

        if (node->marked_for_refactor) {
          struct TokenList sig_slice, body_slice;
          char *new_sig = NULL, *new_body = NULL;
          struct SignatureTransform trans = {0};

          if (get_token_slice(tokens, start_idx, node->body_start,
                              &sig_slice) == 0 &&
              get_token_slice(tokens, node->body_start, end_idx, &body_slice) ==
                  0) {

            /* Generate Signature */
            if (!node->is_main) {
              if (rewrite_signature(&sig_slice, &new_sig) != 0)
                new_sig = join_tokens_str(tokens, start_idx, node->body_start);

              trans.type = node->returns_ptr ? TRANSFORM_RET_PTR_TO_ARG
                                             : TRANSFORM_VOID_TO_INT;
              trans.arg_name = "out";
              trans.success_code = "0";
              trans.error_code = "ENOMEM";
              trans.return_type = node->original_return_type;
            } else {
              new_sig = join_tokens_str(tokens, start_idx, node->body_start);
              trans.type = TRANSFORM_NONE;
            }

            /* Generate Body */
            {
              struct AllocationSiteList local_allocs;
              size_t k;
              allocation_site_list_init(&local_allocs);
              for (k = 0; k < allocs.size; k++) {
                if (allocs.sites[k].token_index >= node->body_start &&
                    allocs.sites[k].token_index < end_idx) {
                  struct AllocationSite site = allocs.sites[k];
                  site.token_index -= node->body_start; /* Relativize */
                  if (site.var_name)
                    site.var_name = c_cdd_strdup(site.var_name);
                  if (local_allocs.size >= local_allocs.capacity) {
                    size_t nc = local_allocs.capacity == 0
                                    ? 4
                                    : local_allocs.capacity * 2;
                    local_allocs.sites = realloc(
                        local_allocs.sites, nc * sizeof(struct AllocationSite));
                    local_allocs.capacity = nc;
                  }
                  local_allocs.sites[local_allocs.size++] = site;
                }
              }

              if (rewrite_body(&body_slice, &local_allocs, ref_funcs,
                               marked_count, &trans, &new_body) == 0) {
#ifdef HAVE_ASPRINTF
                asprintf(&segment, "%s %s", new_sig, new_body);
#else
                char *buf = malloc(strlen(new_sig) + strlen(new_body) + 2);
                if (buf) {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
                  sprintf_s(buf, strlen(new_sig) + strlen(new_body) + 2,
                            "%s %s", new_sig, new_body);
#else
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
                  sprintf_s(buf, sizeof(buf), "%s %s", new_sig, new_body);
#else
                  sprintf(buf, "%s %s", new_sig, new_body);
#endif
#endif
                }
                segment = buf;
#endif
                free(new_sig);
                free(new_body);
              }
              allocation_site_list_free(&local_allocs);
            }
          }
        }

        if (!segment) {
          segment = join_tokens_str(tokens, start_idx, end_idx);
        }

        {
          char *joined;
#ifdef HAVE_ASPRINTF
          asprintf(&joined, "%s%s", output, segment);
#else
          joined = malloc(strlen(output) + strlen(segment) + 1);
          if (joined) {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
            sprintf_s(joined, strlen(output) + strlen(segment) + 1, "%s%s",
                      output, segment);
#else
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
            sprintf_s(joined, sizeof(joined), "%s%s", output, segment);
#else
            sprintf(joined, "%s%s", output, segment);
#endif
#endif
          }
#endif
          free(output);
          output = joined;
          free(segment);
        }
        f_idx++;
        continue;
      }

      /* Copy Non-Function Nodes verbatim */
      {
        char *content = join_tokens_str(tokens, start_idx, end_idx);
        char *joined;
#ifdef HAVE_ASPRINTF
        asprintf(&joined, "%s%s", output, content);
#else
        joined = malloc(strlen(output) + strlen(content) + 1);
        if (joined) {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
          sprintf_s(joined, strlen(output) + strlen(content) + 1, "%s%s",
                    output, content);
#else
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
          sprintf_s(joined, sizeof(joined), "%s%s", output, content);
#else
          sprintf(joined, "%s%s", output, content);
#endif
#endif
        }
#endif
        free(output);
        output = joined;
        free(content);
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

/* --- CLI Integration --- */

struct FixWalkContext {
  int in_place;
  const char *single_output_file;
  int error_count;
};

static int is_c_source(const char *path) {
  const char *dot = strrchr(path, '.');
  return (dot && strcasecmp(dot, ".c") == 0);
}

static int fix_file_callback(const char *path, void *user_data) {
  struct FixWalkContext *ctx = (struct FixWalkContext *)user_data;
  char *content = NULL;
  char *result = NULL;
  size_t sz = 0;
  int rc;
  const char *out_path =
      ctx->single_output_file ? ctx->single_output_file : path;

  if (!is_c_source(path))
    return 0;

  if (read_to_file(path, "r", &content, &sz) != 0) {
    fprintf(stderr, "Failed to read %s\n", path);
    ctx->error_count++;
    return 0;
  }

  rc = orchestrate_fix(content, &result);
  free(content);

  if (rc != 0) {
    fprintf(stderr, "Refactoring failed for %s (code %d)\n", path, rc);
    ctx->error_count++;
    return 0;
  }

  /* Write result */
  {
    FILE *f;
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
    if (fopen_s(&f, out_path, "w") != 0)
      f = NULL;
#else
    f = fopen(out_path, "w");
#endif
    if (f) {
      fputs(result, f);
      fclose(f);
      printf("Fixed: %s\n", out_path);
    } else {
      fprintf(stderr, "Failed to write %s\n", out_path);
      ctx->error_count++;
    }
  }

  free(result);
  return 0;
}

int fix_code_main(int argc, char **argv) {
  struct FixWalkContext ctx = {0};
  const char *target;

  if (argc < 1 || argc > 2) {
    fprintf(stderr, "Usage: fix <path> [--in-place] OR fix <in.c> <out.c>\n");
    return EXIT_FAILURE;
  }

  target = argv[0];
  if (argc == 2) {
    if (strcmp(argv[1], "--in-place") == 0)
      ctx.in_place = 1;
    else
      ctx.single_output_file = argv[1];
  } else {
    /* Implicit single file or error? Assume directory implicit checking */
    if (fs_is_directory(target)) {
      fprintf(stderr, "Directory requires --in-place\n");
      return EXIT_FAILURE;
    }
    /* Single file default output? No, usage requires explicit spec. */
    fprintf(stderr, "Output argument required for single file\n");
    return EXIT_FAILURE;
  }

  if (walk_directory(target, fix_file_callback, &ctx) != 0)
    return EXIT_FAILURE;
  return (ctx.error_count == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
