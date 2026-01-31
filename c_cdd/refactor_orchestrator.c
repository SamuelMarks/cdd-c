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
#define strcasecmp _stricmp
#endif
#else
#include <strings.h>
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
  int returns_void;           /**< 1 if returns void */
  int returns_ptr;            /**< 1 if returns pointer/complex type */
  char *original_return_type; /**< String rep of return type (needed for temp
                                 vars) */
  int is_main;                /**< 1 if function is 'main' */

  /* Analysis state */
  int contains_allocs;     /**< 1 if body contains allocations */
  int marked_for_refactor; /**< 1 if this function must change signature/logic
                            */

  /* Parsing ranges */
  size_t token_start; /**< Token index of start of definition */
  size_t body_start;  /**< Token index of '{' */
  size_t token_end;   /**< Token index of end of definition */

  /* Dependencies (Adjacency List: Reverse Call Graph) */
  /* This list contains indices of functions that CALL this function (Callers)
   */
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
  dst->capacity = 0; /* Borrowed reference, do not free tokens */
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

static char *join_tokens_str(const struct TokenList *tokens, size_t start,
                             size_t end) {
  size_t len = 0;
  size_t i;
  char *buf, *p;
  if (start >= end)
    return strdup("");
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

/* Analyze simple return traits */
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
  for (i = 0; i < callee->num_callers; i++) {
    if (callee->callers[i] == caller_idx)
      return 0; /* Exists */
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

/* Recursive Propagation */
static void propagate_refactor_mark(struct DependencyGraph *g, size_t idx) {
  struct FuncNode *node = &g->nodes[idx];
  size_t i;

  if (node->marked_for_refactor)
    return;

  /* Main stops propagation up, but gets marked itself for body update */
  node->marked_for_refactor = 1;

  if (node->is_main)
    return;

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
  /* Count functions first */
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

  /* Populate Nodes */
  {
    size_t f_idx = 0;
    size_t current_tok_idx = 0;

    for (i = 0; i < cst.size; i++) {
      /* Identify range of current node in token list */
      const uint8_t *node_end_ptr = cst.nodes[i].start + cst.nodes[i].length;
      size_t start_idx = current_tok_idx;
      size_t end_idx = start_idx;

      while (end_idx < tokens->size) {
        if ((tokens->tokens[end_idx].start + tokens->tokens[end_idx].length) >
            node_end_ptr)
          break;
        end_idx++;
      }
      current_tok_idx = end_idx; /* Advance checker */

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
                                 &fn->returns_ptr, &fn->returns_void,
                                 &fn->original_return_type);

        /* Map Allocations to Function */
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

    /* Populate Edges (Calls) */
    for (f_idx = 0; f_idx < graph.count; f_idx++) {
      struct FuncNode *caller = &graph.nodes[f_idx];
      size_t t;
      /* Scan body for calls */
      for (t = caller->body_start; t < caller->token_end; t++) {
        if (tokens->tokens[t].kind == TOKEN_IDENTIFIER) {
          /* Check if looks like call: ID ( */
          size_t next = t + 1;
          while (next < tokens->size &&
                 tokens->tokens[next].kind == TOKEN_WHITESPACE)
            next++;
          if (next < tokens->size &&
              tokens->tokens[next].kind == TOKEN_LPAREN) {
            /* Check if ID maps to known function */
            size_t target_idx;
            for (target_idx = 0; target_idx < graph.count; target_idx++) {
              if (f_idx == target_idx)
                continue; /* Self recursion ignored for graph prop */
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

  /* Phase 4: Identify Seeds and Phase 5: Propagate */
  for (i = 0; i < graph.count; i++) {
    struct FuncNode *n = &graph.nodes[i];
    int seed = 0;

    /* Policy: If contains allocations, we enforce integer return code safety
       if it returns PTR/VOID. If already INT, we assume it's compliant or
       check later. */
    if (n->contains_allocs) {
      if (n->returns_void || n->returns_ptr) {
        seed = 1;
      }
    }

    if (seed) {
      propagate_refactor_mark(&graph, i);
    }
  }

  /* Phase 6: Prepare RefactorContext */
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
            ref_funcs[r].original_return_type =
                graph.nodes[i].original_return_type;
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

  /* Phase 7: Rewrite Source */
  {
    size_t f_idx = 0;
    size_t current_tok_offset = 0;

    output = strdup("");
    if (!output) {
      rc = ENOMEM;
      goto cleanup;
    }

    /* Iterate CST nodes to preserve file order and non-function content */
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

        if (node->marked_for_refactor) {
          struct TokenList sig_slice, body_slice;
          char *new_sig = NULL, *new_body = NULL;
          struct SignatureTransform trans = {0};

          if (get_token_slice(tokens, start_idx, node->body_start,
                              &sig_slice) == 0 &&
              get_token_slice(tokens, node->body_start, end_idx, &body_slice) ==
                  0) {

            /* Generate Sig */
            if (!node->is_main) {
              if (rewrite_signature(&sig_slice, &new_sig) != 0) {
                /* Fallback if parse fail */
                new_sig = join_tokens_str(tokens, start_idx, node->body_start);
              }
              trans.type = node->returns_ptr ? TRANSFORM_RET_PTR_TO_ARG
                                             : TRANSFORM_VOID_TO_INT;
              trans.arg_name = "out";
              trans.success_code = "0";
              trans.error_code = "ENOMEM";
              /* Pass the original return type for safe injection */
              trans.return_type = node->original_return_type;
            } else {
              /* Keep main signature */
              new_sig = join_tokens_str(tokens, start_idx, node->body_start);
              trans.type = TRANSFORM_NONE;
            }

            /* Generate Body */
            {
              /* Filter allocs for this range and relativize */
              struct AllocationSiteList local_allocs;
              size_t k;

              allocation_site_list_init(&local_allocs);
              for (k = 0; k < allocs.size; k++) {
                if (allocs.sites[k].token_index >= node->body_start &&
                    allocs.sites[k].token_index < end_idx) {
                  struct AllocationSite site = allocs.sites[k];
                  site.token_index -= node->body_start; /* Relativize */
                  /* Deep copy relevant strings */
                  if (site.var_name)
                    site.var_name = strdup(site.var_name);
                  /* Note: spec is static const, no copy needed */

                  /* Append to local list */
                  /* NOTE: manual append logic reused from earlier or assume API
                   * exists */
                  /* We manually realloc here for simplicity of integration */
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
                asprintf(&segment, "%s %s", new_sig, new_body);
                free(new_sig);
                free(new_body);
              }

              allocation_site_list_free(&local_allocs);
            }
          }
        }

        if (segment) {
          char *joined;
          asprintf(&joined, "%s%s", output, segment);
          free(output);
          output = joined;
          free(segment);
        } else {
          /* Not refactored, copy original range */
          char *orig = join_tokens_str(tokens, start_idx, end_idx);
          char *joined;
          asprintf(&joined, "%s%s", output, orig);
          free(output);
          output = joined;
          free(orig);
        }
        f_idx++;
        continue;
      }

      /* Non-Function Nodes */
      {
        char *orig = join_tokens_str(tokens, start_idx, end_idx);
        char *joined;
        asprintf(&joined, "%s%s", output, orig);
        free(output);
        output = joined;
        free(orig);
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

/* --- Directory Walking & Main Logic --- */

/**
 * @brief Context for directory walker.
 */
struct FixWalkContext {
  int in_place;
  const char *single_output_file;
  int error_count;
};

/**
 * @brief Check if filename ends with .c extension.
 */
static int is_c_source(const char *path) {
  const char *dot = strrchr(path, '.');
  if (!dot)
    return 0;
  return (strcasecmp(dot, ".c") == 0);
}

/**
 * @brief Callback for directory/file walker.
 */
static int fix_file_callback(const char *path, void *user_data) {
  struct FixWalkContext *ctx = (struct FixWalkContext *)user_data;
  char *content = NULL;
  size_t sz = 0;
  char *result = NULL;
  int rc;
  const char *out_path = path;

  /* Filter .c only if directory walking, but walk_directory might call us for a
   * single file passed by user which might not end in .c but user wants to fix
   * it. However, standard policy is .c filtration. */
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

  /* Determine output path */
  if (ctx->in_place) {
    out_path = path;
  } else if (ctx->single_output_file) {
    out_path = ctx->single_output_file;
  } else {
    /* Should not happen per validation */
    free(result);
    return 0;
  }

  {
    FILE *f;
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
    errno_t err = fopen_s(&f, out_path, "w, ccs=UTF-8");
    if (err != 0 || !f) {
      ctx->error_count++;
    }
#else
    f = fopen(out_path, "w");
    if (!f) {
      ctx->error_count++;
    }
#endif
    if (f) {
      fputs(result, f);
      fclose(f);
      printf("Fixed: %s\n", out_path);
    } else {
      fprintf(stderr, "Failed to write %s\n", out_path);
    }
  }

  free(result);
  return 0;
}

int fix_code_main(int argc, char **argv) {
  const char *in_path;
  const char *arg2;
  int is_dir;
  int in_place = 0;
  struct FixWalkContext ctx;

  if (argc < 1 || argc > 2) {
    fprintf(stderr,
            "Usage: fix <path> [--in-place] OR fix <input.c> <output.c>\n");
    return EXIT_FAILURE;
  }

  in_path = argv[0];
  arg2 = (argc == 2) ? argv[1] : NULL;

  if (arg2 && strcmp(arg2, "--in-place") == 0) {
    in_place = 1;
  }

  is_dir = fs_is_directory(in_path);

  if (is_dir && !in_place) {
    fprintf(stderr, "Error: directory input requires --in-place\n");
    return EXIT_FAILURE;
  }

  if (!is_dir && !in_place && !arg2) {
    fprintf(stderr,
            "Error: output file or --in-place required for file input\n");
    return EXIT_FAILURE;
  }

  /* Setup context */
  ctx.in_place = in_place;
  ctx.error_count = 0;
  ctx.single_output_file = (in_place) ? NULL : arg2;

  if (walk_directory(in_path, fix_file_callback, &ctx) != 0) {
    return EXIT_FAILURE;
  }

  return (ctx.error_count == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
