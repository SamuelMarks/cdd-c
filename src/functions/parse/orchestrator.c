/* clang-format off */
#include "c_cdd/memory.h"
/**
 * @file orchestrator.c
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

#include "c_cdd/safe_crt_msvc.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <c_str_span.h>

#include "c_cdd/log.h"
#include "functions/emit/rewriter_body.h"
#include "functions/emit/rewriter_sig.h"
#include "functions/parse/analysis.h"
#include "functions/parse/cst.h"
#include "functions/parse/fs.h"
#include "functions/parse/orchestrator.h"
#include "functions/parse/str.h" /* For c_cdd_strdup */
#include "functions/parse/tokenizer.h"

#if defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__)
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#ifndef strdup
#define strdup _strdup
#endif
#endif
#else
#include "c_cdd/log.h"
#include <errno.h>
#endif
/* clang-format on */

#ifdef CDD_BUILD_TESTS
extern C_CDD_EXPORT int g_force_find_allocations_fail;
extern C_CDD_EXPORT int g_force_parse_tokens_fail;
extern C_CDD_EXPORT int g_force_tokenize_fail;
extern C_CDD_EXPORT int g_cdd_fail_token_eq_str;
extern C_CDD_EXPORT int g_cdd_fail_propagate;
#endif

/* --- Helpers --- */

/**
 * @brief Extract a slice of tokens into a temporary view.
 * Does not copy token data, just pointers.
 */
C_CDD_EXPORT cdd_c_error_t get_token_slice(const struct TokenList *src,
                                           size_t start, size_t end,
                                           struct TokenList *dst) {
#ifdef CDD_BUILD_TESTS
  extern C_CDD_EXPORT int g_cdd_fail_get_token_slice;
  if (g_cdd_fail_get_token_slice && --g_cdd_fail_get_token_slice == 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;
#endif
  if (!src || !dst)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (start >= src->size || end > src->size || start > end)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  dst->tokens = src->tokens + start;
  dst->size = end - start;
  dst->capacity = 0; /* Marker: do not free */
  return CDD_C_SUCCESS;
}

/* Helper to find specific token type within range */
/**
 * @brief Retrieves the token in range.
 */
C_CDD_EXPORT cdd_c_error_t find_token_in_range(const struct TokenList *tokens,
                                               size_t start, size_t end,
                                               enum TokenKind kind,
                                               size_t *_out_val) {
  size_t i;
  if (!_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (!tokens) {
    *_out_val = end;
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
  for (i = start; i < end; ++i) {
    if (tokens->tokens[i].kind == kind) {
      *_out_val = i;
      return CDD_C_SUCCESS;
    }
  }
  *_out_val = end;
  return CDD_C_SUCCESS;
}

/* Helper for token string comparison */
/**
 * @brief Executes the token eq str operation.
 */
C_CDD_EXPORT cdd_c_error_t token_eq_str(const struct Token *tok, const char *s,
                                        int *out_eq) {
  size_t len;
#ifdef CDD_BUILD_TESTS
  extern C_CDD_EXPORT int g_cdd_fail_token_eq_str;
  if (g_cdd_fail_token_eq_str && --g_cdd_fail_token_eq_str == 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;
#endif
  if (!out_eq)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_eq = 0;
  if (!tok || !s)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  len = strlen(s);
  *out_eq =
      (tok->length == len && strncmp((const char *)tok->start, s, len) == 0);
  return CDD_C_SUCCESS;
}

/**
 * @brief Extract function name from tokens.
 * Finds the identifier immediately preceding the argument list LPAREN.
 */
C_CDD_EXPORT cdd_c_error_t extract_func_name(const struct TokenList *tokens,
                                             size_t start, size_t body_start,
                                             char **_out_val) {
  size_t _ast_find_token_in_range_0 = 0;
  size_t lparen;
  size_t i;

#ifdef CDD_BUILD_TESTS
  extern C_CDD_EXPORT int g_cdd_fail_extract_func_name;
  if (g_cdd_fail_extract_func_name && --g_cdd_fail_extract_func_name == 0)
    return CDD_C_ERROR_MEMORY;
#endif

  if (!_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *_out_val = NULL;

  if (!tokens)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  find_token_in_range(tokens, start, body_start, TOKEN_LPAREN,
                      &_ast_find_token_in_range_0);
  lparen = _ast_find_token_in_range_0;
  if (lparen == body_start)
    return CDD_C_SUCCESS;

  /* Backtrack from LPAREN to find Identifier */
  i = lparen;
  while (i > start) {
    i--;
    if (tokens->tokens[i].kind == TOKEN_WHITESPACE)
      continue;
    if (tokens->tokens[i].kind == TOKEN_IDENTIFIER) {
      size_t len = tokens->tokens[i].length;
      char *name = (char *)C_CDD_MALLOC(len + 1);
      if (!name)
        return CDD_C_ERROR_MEMORY;
      memcpy(name, tokens->tokens[i].start, len);
      name[len] = '\0';
      *_out_val = name;
      return CDD_C_SUCCESS;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Join tokens into a single string.
 */
C_CDD_EXPORT cdd_c_error_t join_tokens_str(const struct TokenList *tokens,
                                           size_t start, size_t end,
                                           char **_out_val) {
  size_t len = 0;
  size_t i;
  char *buf, *p;

  if (!_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *_out_val = NULL;

  if (!tokens)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (start >= end) {
    return c_cdd_strdup("", _out_val);
  }
  for (i = start; i < end; ++i)
    len += tokens->tokens[i].length;
  buf = (char *)C_CDD_MALLOC(len + 1);
  if (!buf)
    return CDD_C_ERROR_MEMORY;
  p = buf;
  for (i = start; i < end; ++i) {
    memcpy(p, tokens->tokens[i].start, tokens->tokens[i].length);
    p += tokens->tokens[i].length;
  }
  *p = '\0';
  *_out_val = buf;
  return CDD_C_SUCCESS;
}

/**
 * @brief Concatenates two strings with an optional delimiter.
 *
 * @param[in] s1 First string (can be NULL or empty).
 * @param[in] delim Delimiter string (can be NULL).
 * @param[in] s2 Second string (can be NULL or empty).
 * @param[out] out_str Pointer to store newly allocated concatenated string.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_MEMORY on allocation failure,
 *         or CDD_C_ERROR_INVALID_ARGUMENT.
 */
C_CDD_EXPORT cdd_c_error_t concat_strings(const char *s1, const char *delim,
                                          const char *s2, char **out_str) {
  size_t l1;
  size_t ld;
  size_t l2;
  size_t total;
  char *buf;

#ifdef CDD_BUILD_TESTS
  extern C_CDD_EXPORT int g_cdd_fail_concat_strings;
  if (g_cdd_fail_concat_strings && --g_cdd_fail_concat_strings == 0)
    return CDD_C_ERROR_MEMORY;
#endif

  if (!out_str)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_str = NULL;

  l1 = s1 ? strlen(s1) : 0;
  ld = delim ? strlen(delim) : 0;
  l2 = s2 ? strlen(s2) : 0;
  total = l1 + ld + l2;

  buf = (char *)C_CDD_MALLOC(total + 1);
  if (!buf) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM");
    return CDD_C_ERROR_MEMORY;
  }

#if (defined(_MSC_VER) && !defined(__INTEL_COMPILER)) ||                       \
    (defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__)
  buf[0] = '\0';
  if (s1)
    strcat_s(buf, total + 1, s1);
  if (delim)
    strcat_s(buf, total + 1, delim);
  if (s2)
    strcat_s(buf, total + 1, s2);
#else
  buf[0] = '\0';
  if (s1)
    strcat(buf, s1);
  if (delim)
    strcat(buf, delim);
  if (s2)
    strcat(buf, s2);
#endif

  *out_str = buf;
  return CDD_C_SUCCESS;
}

/**
 * @brief Analyze return type tokens to determine void/int/pointer status.
 */
C_CDD_EXPORT cdd_c_error_t analyze_signature_tokens(
    const struct TokenList *tokens, size_t start, size_t body_start,
    int *is_ptr, int *is_void, char **type_str) {
  size_t _ast_find_token_in_range_1 = 0;
  size_t i;
  size_t lparen;
  size_t name_end_idx = 0;
  cdd_c_error_t rc;

  if (!tokens || !is_ptr || !is_void || !type_str)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  *is_ptr = 0;
  *is_void = 0;
  *type_str = NULL;

  find_token_in_range(tokens, start, body_start, TOKEN_LPAREN,
                      &_ast_find_token_in_range_1);
  lparen = _ast_find_token_in_range_1;
  if (lparen == body_start)
    return CDD_C_SUCCESS;

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
  rc = join_tokens_str(tokens, start, name_end_idx, type_str);
  if (rc != CDD_C_SUCCESS)
    return rc;

  /* Check properties */
  for (i = start; i < name_end_idx; ++i) {
    const struct Token *tok = &tokens->tokens[i];
    if (tok->kind == TOKEN_STAR) {
      *is_ptr = 1;
    } else if (tok->kind == TOKEN_KEYWORD_VOID) {
      *is_void = 1;
    } else if (tok->kind == TOKEN_IDENTIFIER) {
      int eq = 0;
      token_eq_str(tok, "void", &eq);
      if (eq) {
        *is_void = 1;
      }
    }
  }
  /* Pointer takes precedence (e.g. void*) */
  if (*is_ptr)
    *is_void = 0;
  return CDD_C_SUCCESS;
}

/* --- Graph Management --- */

/**
 * @brief Executes the graph add node operation.
 */
C_CDD_EXPORT cdd_c_error_t graph_add_node(struct DependencyGraph *g, size_t idx,
                                          const char *name) {
  cdd_c_error_t rc;
  if (!g || !g->nodes || !name || idx >= g->count)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  g->nodes[idx].node_idx = idx;
  rc = c_cdd_strdup(name, &g->nodes[idx].name);
  if (rc != CDD_C_SUCCESS)
    return rc;
  g->nodes[idx].callers = NULL;
  g->nodes[idx].num_callers = 0;
  g->nodes[idx].alloc_callers = 0;
  g->nodes[idx].original_return_type = NULL;

  g->nodes[idx].marked_for_refactor = 0;
  g->nodes[idx].contains_allocs = 0;
  g->nodes[idx].is_main = (strcmp(name, "main") == 0);

  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the graph add edge operation.
 */
C_CDD_EXPORT cdd_c_error_t graph_add_edge(struct DependencyGraph *g,
                                          size_t caller_idx,
                                          size_t callee_idx) {
  struct FuncNode *callee;
  size_t i;

  if (!g || !g->nodes || callee_idx >= g->count || caller_idx >= g->count)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  callee = &g->nodes[callee_idx];
  /* Prevent duplicate edges */
  for (i = 0; i < callee->num_callers; i++) {
    if (callee->callers[i] == caller_idx)
      return CDD_C_SUCCESS;
  }

  if (callee->num_callers >= callee->alloc_callers) {
    size_t new_cap = callee->alloc_callers == 0 ? 4 : callee->alloc_callers * 2;
    size_t *new_arr =
        (size_t *)C_CDD_REALLOC(callee->callers, new_cap * sizeof(size_t));
    if (!new_arr) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM");
      return CDD_C_ERROR_MEMORY;
    }
    callee->callers = new_arr;
    callee->alloc_callers = new_cap;
  }
  callee->callers[callee->num_callers++] = caller_idx;
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the graph free contents operation.
 */
C_CDD_EXPORT cdd_c_error_t graph_free_contents(struct DependencyGraph *g) {
  size_t i;
  if (!g || !g->nodes)
    return CDD_C_SUCCESS;
  for (i = 0; i < g->count; i++) {
    C_CDD_FREE(g->nodes[i].name);
    C_CDD_FREE(g->nodes[i].callers);
    C_CDD_FREE(g->nodes[i].original_return_type);
  }
  C_CDD_FREE(g->nodes);
  g->nodes = NULL;
  g->count = 0;
  return CDD_C_SUCCESS;
}

/* --- Propagation Logic --- */

/**
 * @brief Executes the propagate refactor mark operation.
 */
C_CDD_EXPORT cdd_c_error_t propagate_refactor_mark(struct DependencyGraph *g,
                                                   size_t idx) {
  struct FuncNode *node;
  size_t i;

#ifdef CDD_BUILD_TESTS
  extern C_CDD_EXPORT int g_cdd_fail_propagate;
  if (g_cdd_fail_propagate && --g_cdd_fail_propagate == 0)
    return CDD_C_ERROR_MEMORY;
#endif

  if (!g || !g->nodes || idx >= g->count)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  node = &g->nodes[idx];
  if (node->marked_for_refactor)
    return CDD_C_SUCCESS;

  /* Mark current node */
  node->marked_for_refactor = 1;

  /* If main function, mark for modification but stop propagating upwards */
  if (node->is_main)
    return CDD_C_SUCCESS;

  /* Recurse to all callers */
  for (i = 0; i < node->num_callers; i++) {
    size_t caller_idx = node->callers[i];
    {
      cdd_c_error_t rc_or = propagate_refactor_mark(g, caller_idx);
      if (rc_or != CDD_C_SUCCESS)
        return rc_or;
    }
  }
  return CDD_C_SUCCESS;
}

/* --- Main Orchestrator --- */

/**
 * @brief Executes the orchestrate fix operation.
 */
cdd_c_error_t orchestrate_fix(const char *source_code, char **out_code) {
  size_t _ast_find_token_in_range_3 = 0;
  char *_ast_join_tokens_str_5 = NULL;
  char *_ast_join_tokens_str_6 = NULL;
  char *_ast_join_tokens_str_7 = NULL;
  struct TokenList *tokens = NULL;
  struct CstNodeList cst = {0};
  struct AllocationSiteList allocs = {0};
  struct DependencyGraph graph = {0};
  struct RefactoredFunction *ref_funcs = NULL;
  char *output = NULL;
  size_t i;
  size_t marked_count = 0;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  (void)_ast_join_tokens_str_5;
  (void)_ast_join_tokens_str_6;
  (void)_ast_join_tokens_str_7;

  if (!source_code || !out_code)
    return CDD_C_ERROR_INVALID_ARGUMENT;

    /* 1. Parse */
#ifdef CDD_BUILD_TESTS
  if (g_force_tokenize_fail)
    return CDD_C_ERROR_MEMORY;
#endif
  if ((rc = tokenize(az_span_create_from_str((char *)(size_t)source_code),
                     &tokens)) != CDD_C_SUCCESS)
    return rc;

#ifdef CDD_BUILD_TESTS
  if (g_force_parse_tokens_fail) {
    rc = CDD_C_ERROR_MEMORY;
    free_token_list(tokens);
    return rc;
  }
#endif
  if ((rc = parse_tokens(tokens, &cst)) != CDD_C_SUCCESS) {
    free_token_list(tokens);
    return rc;
  }

  /* 2. Analyze Allocations */
#ifdef CDD_BUILD_TESTS
  if (g_force_find_allocations_fail) {
    rc = CDD_C_ERROR_MEMORY;
    free_cst_node_list(&cst);
    free_token_list(tokens);
    return rc;
  }
#endif
  if ((rc = find_allocations(tokens, &allocs)) != CDD_C_SUCCESS) {
    allocation_site_list_free(&allocs);
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
    graph.nodes =
        (struct FuncNode *)C_CDD_CALLOC(graph.count, sizeof(struct FuncNode));
    if (!graph.nodes) {
      rc = CDD_C_ERROR_MEMORY;
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
        char *name = NULL;

        fn->token_start = start_idx;
        fn->token_end = end_idx;
        find_token_in_range(tokens, start_idx, end_idx, TOKEN_LBRACE,
                            &_ast_find_token_in_range_3);
        fn->body_start = _ast_find_token_in_range_3;

        rc = extract_func_name(tokens, start_idx, fn->body_start, &name);
        if (rc != CDD_C_SUCCESS)
          goto cleanup;

        if (!name) {
          rc = c_cdd_strdup("", &name);
          if (rc != CDD_C_SUCCESS)
            goto cleanup;
        }

        rc = graph_add_node(&graph, f_idx, name);
        C_CDD_FREE(name);
        if (rc != CDD_C_SUCCESS)
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
          while (tokens->tokens[next].kind == TOKEN_WHITESPACE)
            next++;
          if (tokens->tokens[next].kind == TOKEN_LPAREN) {
            size_t target_idx;
            for (target_idx = 0; target_idx < graph.count; target_idx++) {
              int eq = 0;
              if (f_idx == target_idx)
                continue;
              rc = token_eq_str(&tokens->tokens[t],
                                graph.nodes[target_idx].name, &eq);
              if (rc != CDD_C_SUCCESS)
                goto cleanup;
              if (eq) {
                rc = graph_add_edge(&graph, f_idx, target_idx);
                if (rc != CDD_C_SUCCESS)
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
      {
        cdd_c_error_t rc_or = propagate_refactor_mark(&graph, i);
        if (rc_or != CDD_C_SUCCESS) {
          rc = rc_or;
          goto cleanup;
        }
      }
    }
  }

  /* 5. Rewrite */
  if (graph.count > 0) {
    /* Build Refactor Context List for rewriter */
    for (i = 0; i < graph.count; i++)
      if (graph.nodes[i].marked_for_refactor)
        marked_count++;

    if (marked_count > 0) {
      ref_funcs = (struct RefactoredFunction *)C_CDD_CALLOC(
          marked_count, sizeof(struct RefactoredFunction));
      if (!ref_funcs) {
        rc = CDD_C_ERROR_MEMORY;
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
    rc = c_cdd_strdup("", &output);
    if (rc != CDD_C_SUCCESS) {
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
                              &sig_slice) == CDD_C_SUCCESS &&
              get_token_slice(tokens, node->body_start, end_idx, &body_slice) ==
                  CDD_C_SUCCESS) {

            /* Generate Signature */
            if (!node->is_main) {
              if (rewrite_signature(&sig_slice, &new_sig) != 0) {
                join_tokens_str(tokens, start_idx, node->body_start,
                                &_ast_join_tokens_str_5);
                new_sig = _ast_join_tokens_str_5;
              }

              trans.type = node->returns_ptr ? TRANSFORM_RET_PTR_TO_ARG
                                             : TRANSFORM_VOID_TO_INT;
              trans.arg_name = "out";
              trans.success_code = "0";
              trans.error_code = "CDD_C_ERROR_MEMORY";
              trans.return_type = node->original_return_type;
            } else {
              join_tokens_str(tokens, start_idx, node->body_start,
                              &_ast_join_tokens_str_6);
              new_sig = _ast_join_tokens_str_6;
              trans.type = TRANSFORM_NONE;
            }

            /* Generate Body */
            {
              struct AllocationSiteList local_allocs = {0};
              size_t k;
              {
                cdd_c_error_t rc_or;
#ifdef CDD_BUILD_TESTS
                extern C_CDD_EXPORT int g_cdd_fail_local_alloc_init;
                if (g_cdd_fail_local_alloc_init &&
                    --g_cdd_fail_local_alloc_init == 0)
                  rc_or = CDD_C_ERROR_MEMORY;
                else
#endif
                  rc_or = allocation_site_list_init(&local_allocs);
                if (rc_or != CDD_C_SUCCESS) {
                  rc = rc_or;
                  C_CDD_FREE(new_sig);
                  goto cleanup;
                }
              }
              for (k = 0; k < allocs.size; k++) {
                if (allocs.sites[k].token_index >= node->body_start &&
                    allocs.sites[k].token_index < end_idx) {
                  struct AllocationSite site = allocs.sites[k];
                  site.token_index -= node->body_start; /* Relativize */
                  if (site.var_name) {
                    rc = c_cdd_strdup(site.var_name, &site.var_name);
                    if (rc != CDD_C_SUCCESS) {
                      allocation_site_list_free(&local_allocs);
                      C_CDD_FREE(new_sig);
                      goto cleanup;
                    }
                  }
                  if (local_allocs.size >= local_allocs.capacity) {
                    struct AllocationSite *new_sites;
                    size_t nc = local_allocs.capacity * 2;
                    new_sites = (struct AllocationSite *)C_CDD_REALLOC(
                        local_allocs.sites, nc * sizeof(struct AllocationSite));
                    if (!new_sites) {
                      allocation_site_list_free(&local_allocs);
                      C_CDD_FREE(new_sig);
                      rc = CDD_C_ERROR_MEMORY;
                      goto cleanup;
                    }
                    local_allocs.sites = new_sites;
                    local_allocs.capacity = nc;
                  }
                  local_allocs.sites[local_allocs.size++] = site;
                }
              }

              if (rewrite_body(&body_slice, &local_allocs, ref_funcs,
                               marked_count, &trans, &new_body) == 0) {
                rc = concat_strings(new_sig, " ", new_body, &segment);
                C_CDD_FREE(new_body);
                if (rc != CDD_C_SUCCESS) {
                  C_CDD_FREE(new_sig);
                  allocation_site_list_free(&local_allocs);
                  goto cleanup;
                }
              }
              C_CDD_FREE(new_sig);
              allocation_site_list_free(&local_allocs);
            }
          }
        }

        if (!segment) {
          join_tokens_str(tokens, start_idx, end_idx, &_ast_join_tokens_str_7);
          segment = _ast_join_tokens_str_7;
        }

        {
          char *joined = NULL;
          rc = concat_strings(output, NULL, segment, &joined);
          C_CDD_FREE(output);
          output = joined;
          if (rc != CDD_C_SUCCESS) {
            C_CDD_FREE(segment);
            goto cleanup;
          }
          C_CDD_FREE(segment);
        }
        f_idx++;
        continue;
      }

      /* Copy Non-Function Nodes verbatim */
      {
        char *content = NULL;
        char *joined = NULL;
        rc = join_tokens_str(tokens, start_idx, end_idx, &content);
        if (rc != CDD_C_SUCCESS)
          goto cleanup;

        rc = concat_strings(output, NULL, content, &joined);
        C_CDD_FREE(output);
        output = joined;
        if (rc != CDD_C_SUCCESS) {
          C_CDD_FREE(content);
          goto cleanup;
        }
        C_CDD_FREE(content);
      }
    }
  }

  *out_code = output;
  output = NULL;

cleanup:
  if (ref_funcs)
    C_CDD_FREE(ref_funcs);
  graph_free_contents(&graph);
  if (output)
    C_CDD_FREE(output);
  free_cst_node_list(&cst);
  allocation_site_list_free(&allocs);
  free_token_list(tokens);
  return rc;
}

/* --- CLI Integration --- */

/**
 * @brief Checks if c source.
 */
C_CDD_EXPORT cdd_c_error_t is_c_source(const char *path, int *out_is_src) {
  const char *dot;
  int diff = 0;
  if (!path || !out_is_src)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_is_src = 0;
  dot = strrchr(path, '.');
  if (!dot)
    return CDD_C_SUCCESS;
  {
    cdd_c_error_t rc_or = c_cdd_stricmp(dot, ".c", &diff);
    if (rc_or != CDD_C_SUCCESS)
      return rc_or;
  }
  *out_is_src = (diff == 0);
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the fix file callback operation.
 */
C_CDD_EXPORT cdd_c_error_t fix_file_callback(const char *path,
                                             void *user_data) {
  struct FixWalkContext *ctx;
  char *content = NULL;
  char *result = NULL;
  size_t sz = 0;
  cdd_c_error_t rc;
  const char *out_path;

  if (!path || !user_data)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  ctx = (struct FixWalkContext *)user_data;
  out_path = ctx->single_output_file ? ctx->single_output_file : path;

  {
    int is_src = 0;
    {
      cdd_c_error_t rc_or = is_c_source(path, &is_src);
      if (rc_or != CDD_C_SUCCESS)
        return rc_or;
    }
    if (!is_src)
      return CDD_C_SUCCESS;
  }

  if (read_to_file(path, "r", &content, &sz) != 0) {
    fprintf(stderr, "Failed to read %s", path);
    ctx->error_count++;
    return CDD_C_SUCCESS;
  }

  rc = orchestrate_fix(content, &result);
  C_CDD_FREE(content);

  if (rc != CDD_C_SUCCESS) {
    fprintf(stderr, "Refactoring failed for %s (code %d)", path, rc);
    ctx->error_count++;
    return CDD_C_SUCCESS;
  }

  /* Write result */
  {
    FILE *f;
#if defined(_MSC_VER)
    if (fopen_s(&f, out_path, "w") != 0)
      f = NULL;
#else
    f = fopen(out_path, "w");
#endif
    if (f) {
      fputs(result, f);
      fclose(f);
      printf("Fixed: %s", out_path);
    } else {
      fprintf(stderr, "Failed to write %s", out_path);
      ctx->error_count++;
    }
  }

  C_CDD_FREE(result);
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the fix code main operation.
 */
cdd_c_error_t fix_code_main(int argc, char **argv) {
  struct FixWalkContext ctx = {0};
  const char *target;

  if (argc < 1 || argc > 2 || !argv) {
    fprintf(stderr, "Usage: fix <path> [--in-place] OR fix <in.c> <out.c>");
    return CDD_C_ERROR_UNKNOWN;
  }

  target = argv[0];
  if (argc == 2) {
    if (argv[1] && strcmp(argv[1], "--in-place") == 0)
      ctx.in_place = 1;
    else
      ctx.single_output_file = argv[1];
  } else {
    /* Implicit single file or error? Assume directory implicit checking */
    int is_dir = 0;
    {
      cdd_c_error_t rc_or = fs_is_directory(target, &is_dir);
      if (rc_or != CDD_C_SUCCESS)
        return rc_or;
    }
    if (is_dir) {
      fprintf(stderr, "Directory requires --in-place");
      return CDD_C_ERROR_UNKNOWN;
    }
    /* Single file default output? No, usage requires explicit spec. */
    fprintf(stderr, "Output argument required for single file");
    return CDD_C_ERROR_UNKNOWN;
  }

  if (walk_directory(target, fix_file_callback, &ctx) != 0)
    return CDD_C_ERROR_UNKNOWN;
  return (ctx.error_count == 0) ? CDD_C_SUCCESS : CDD_C_ERROR_MEMORY;
}
