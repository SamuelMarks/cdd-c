/**
 * @file error_percolator.c
 * @brief Implementation of the error percolator transformer.
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "c_cdd/format_specifiers.h"
#include "c_cdd/memory.h"
#include "c_cdd/safe_crt.h"
#include "c_cdd_export.h"
#include "c_str_span.h"
#include "cdd_cst_transform.h"
#include "classes/parse/cdd_cst_builder.h"
#include "classes/parse/cdd_cst_factory.h"
#include "classes/parse/cdd_cst_mutate.h"
#include "classes/parse/cdd_cst_parser.h"
#include "classes/parse/cdd_cst_query.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

#ifdef CDD_BUILD_TESTS
C_CDD_EXPORT int g_err_perc_fail = 0;
#endif

/**
 * @brief Checks if a token in a CST node represents a function call site.
 *
 * @param[in] node The CST node.
 * @param[in] ident_idx Index of the identifier token in node->children.
 * @param[out] out_is_call Pointer to int storing 1 if call, 0 otherwise.
 * @return CDD_C_SUCCESS on success or error code.
 */
C_CDD_EXPORT cdd_c_error_t cdd_check_is_call(const cdd_cst_node_t *node,
                                             size_t ident_idx,
                                             int *out_is_call) {
  size_t j;
  if (!node || !out_is_call)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  *out_is_call = 0;
  for (j = ident_idx + 1; j < node->num_children; j++) {
    if (node->children[j].kind == CDD_CST_CHILD_TOKEN) {
      if (node->children[j].val.token->kind == CDD_TOKEN_LPAREN) {
        *out_is_call = 1;
      }
      return CDD_C_SUCCESS;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks if a token in a CST node represents a function definition.
 *
 * @param[in] node The CST node.
 * @param[in] ident_idx Index of the identifier token in node->children.
 * @param[out] out_is_def Pointer to int storing 1 if definition, 0 otherwise.
 * @return CDD_C_SUCCESS on success or error code.
 */
C_CDD_EXPORT cdd_c_error_t cdd_check_is_function_def(const cdd_cst_node_t *node,
                                                     size_t ident_idx,
                                                     int *out_is_def) {
  size_t j;
  if (!node || !out_is_def)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  *out_is_def = 0;
  if (node->kind == CDD_CST_FUNCTION_DEFINITION) {
    *out_is_def = 1;
    return CDD_C_SUCCESS;
  }

  for (j = ident_idx; j-- > 0;) {
    if (node->children[j].kind == CDD_CST_CHILD_TOKEN) {
      cdd_token_t *prev = node->children[j].val.token;
      switch (prev->kind) {
      case CDD_TOKEN_KEYWORD_INT:
        *out_is_def = 1;
        break;
      case CDD_TOKEN_IDENTIFIER:
        if (prev->length == 4) {
          if (memcmp(prev->start, "void", 4) == 0) {
            *out_is_def = 1;
          }
        }
        break;
      default:
        break;
      }
      return CDD_C_SUCCESS;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Rewrites call sites in a CST node for functions whose signatures have
 * been modified.
 *
 * @param[in,out] tree The CST tree.
 * @param[in,out] node The current node to inspect and rewrite.
 * @param[in] modified_funcs Array of tokens representing modified function
 * names.
 * @param[in] num_modified Number of modified functions.
 * @return CDD_C_SUCCESS on success or error code.
 */
C_CDD_EXPORT cdd_c_error_t cdd_rewrite_call_sites(cdd_cst_tree_t *tree,
                                                  cdd_cst_node_t *node,
                                                  cdd_token_t **modified_funcs,
                                                  size_t num_modified) {
  size_t i;
  cdd_c_error_t rc;

  if (!tree)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (!node)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (!modified_funcs || num_modified == 0)
    return CDD_C_SUCCESS;

  for (i = 0; i < node->num_children; i++) {
    if (node->children[i].kind == CDD_CST_CHILD_TOKEN) {
      cdd_token_t *tok = node->children[i].val.token;
      if (tok->kind == CDD_TOKEN_IDENTIFIER) {
        size_t m;
        for (m = 0; m < num_modified; m++) {
          if (modified_funcs[m]->length == tok->length) {
            if (memcmp(modified_funcs[m]->start, tok->start, tok->length) ==
                0) {
              int is_call = 0;
              int is_def = 0;
              size_t j;
              size_t rparen_idx = 0;
              size_t semi_idx = 0;
              int depth = 0;
              cdd_token_t *rparen_tok = NULL;
              cdd_token_t *prev_rparen_tok = NULL;

              (void)cdd_check_is_call(node, i, &is_call);
              if (!is_call)
                continue;

              (void)cdd_check_is_function_def(node, i, &is_def);
              if (is_def)
                continue;

              for (j = i + 1; j < node->num_children; j++) {
                if (node->children[j].kind == CDD_CST_CHILD_TOKEN) {
                  cdd_token_t *t = node->children[j].val.token;
                  switch (t->kind) {
                  case CDD_TOKEN_LPAREN:
                    depth++;
                    break;
                  case CDD_TOKEN_RPAREN:
                    depth--;
                    if (depth == 0) {
                      rparen_idx = j;
                      rparen_tok = t;
                      if (node->children[j - 1].kind == CDD_CST_CHILD_TOKEN) {
                        prev_rparen_tok = node->children[j - 1].val.token;
                      }
                    }
                    break;
                  case CDD_TOKEN_SEMICOLON:
                    if (rparen_tok) {
                      semi_idx = j;
                      j = node->num_children;
                    }
                    break;
                  default:
                    break;
                  }
                }
              }

              if (rparen_tok) {
                cdd_cst_builder_t bld;
                cdd_cst_node_t *temp;
                char *dup_id;
                size_t c_len;
                cdd_token_t *t_dummy = NULL;
                cdd_trivia_t *lt;
                cdd_cst_node_t *parent_ptr;
                int needs_comma;

                dup_id = (char *)(size_t)C_CDD_CALLOC(1, 256);
                if (!dup_id)
                  return CDD_C_ERROR_MEMORY;

                CDD_SNPRINTF(dup_id, 256, "out_%.*s", (int)tok->length,
                             tok->start);

                if (tree->num_strings >= tree->string_capacity) {
                  size_t new_cap = tree->string_capacity * 2;
                  char **new_pool;
                  if (new_cap < 32)
                    new_cap = 32;
                  new_pool = (char **)C_CDD_REALLOC(tree->string_pool,
                                                    new_cap * sizeof(char *));
                  if (!new_pool) {
                    C_CDD_FREE(dup_id);
                    return CDD_C_ERROR_MEMORY;
                  }
                  tree->string_pool = new_pool;
                  tree->string_capacity = new_cap;
                }
                tree->string_pool[tree->num_strings++] = dup_id;

                c_len = strlen(dup_id);
                cdd_cst_create_token_len(tree, CDD_TOKEN_IDENTIFIER, dup_id,
                                         c_len, &t_dummy);

                /* Prefix builder */
                temp =
                    (cdd_cst_node_t *)C_CDD_CALLOC(1, sizeof(cdd_cst_node_t));
                if (!temp)
                  return CDD_C_ERROR_MEMORY;

                temp->kind = CDD_CST_UNKNOWN;
                cdd_cst_builder_init(&bld, tree, temp);
                cdd_cst_bld_punct(&bld, "{");
                cdd_cst_bld_space(&bld);
                cdd_cst_bld_ident(&bld, "void");
                cdd_cst_bld_space(&bld);
                cdd_cst_bld_punct(&bld, "*");
                cdd_cst_bld_ident(&bld, dup_id);
                cdd_cst_bld_punct(&bld, ";");
                cdd_cst_bld_newline(&bld);
                cdd_cst_bld_space(&bld);
                cdd_cst_bld_space(&bld);
                cdd_cst_bld_ident(&bld, "if");
                cdd_cst_bld_space(&bld);
                cdd_cst_bld_punct(&bld, "(");

#ifdef CDD_BUILD_TESTS
                if (g_err_perc_fail == 5)
                  bld.error_state = CDD_C_ERROR_MEMORY;
#endif

                if (bld.error_state != 0) {
                  cdd_cst_builder_free(&bld);
                  C_CDD_FREE(temp->children);
                  C_CDD_FREE(temp);
                  return (cdd_c_error_t)bld.error_state;
                }

                lt = tok->leading_trivia;
                parent_ptr = node;
                tok->leading_trivia = NULL;
                temp->children[0].val.token->leading_trivia = lt;

                rc =
                    cdd_cst_splice_children(tree, &parent_ptr, i, 0,
                                            temp->children, temp->num_children);
                if (rc != CDD_C_SUCCESS) {
                  tok->leading_trivia = lt;
                  temp->children[0].val.token->leading_trivia = NULL;
                  cdd_cst_builder_free(&bld);
                  C_CDD_FREE(temp->children);
                  C_CDD_FREE(temp);
                  return rc;
                }
                i += temp->num_children;
                rparen_idx += temp->num_children;
                if (semi_idx > 0)
                  semi_idx += temp->num_children;
                node = parent_ptr;

                cdd_cst_builder_free(&bld);
                C_CDD_FREE(temp->children);
                C_CDD_FREE(temp);

                /* Suffix builder */
                temp =
                    (cdd_cst_node_t *)C_CDD_CALLOC(1, sizeof(cdd_cst_node_t));
                if (!temp)
                  return CDD_C_ERROR_MEMORY;

                temp->kind = CDD_CST_UNKNOWN;
                cdd_cst_builder_init(&bld, tree, temp);

                needs_comma = 1;
                if (prev_rparen_tok) {
                  if (prev_rparen_tok->kind == CDD_TOKEN_LPAREN) {
                    needs_comma = 0;
                  }
                }
                if (needs_comma) {
                  cdd_cst_bld_punct(&bld, ",");
                  cdd_cst_bld_space(&bld);
                }

                cdd_cst_bld_punct(&bld, "&");
                cdd_cst_bld_ident(&bld, dup_id);
                cdd_cst_bld_punct(&bld, ")");
                cdd_cst_bld_space(&bld);
                cdd_cst_bld_punct(&bld, "!=");
                cdd_cst_bld_space(&bld);
                cdd_cst_bld_int(&bld, 0);
                cdd_cst_bld_punct(&bld, ")");
                cdd_cst_bld_space(&bld);
                cdd_cst_bld_punct(&bld, "{");
                cdd_cst_bld_space(&bld);
                cdd_cst_bld_ident(&bld, "return");
                cdd_cst_bld_space(&bld);
                cdd_cst_bld_ident(&bld, "CDD_C_ERROR_MEMORY");
                cdd_cst_bld_punct(&bld, ";");
                cdd_cst_bld_space(&bld);
                cdd_cst_bld_punct(&bld, "}");
                cdd_cst_bld_space(&bld);
                cdd_cst_bld_punct(&bld, "}");

#ifdef CDD_BUILD_TESTS
                if (g_err_perc_fail == 6)
                  bld.error_state = CDD_C_ERROR_MEMORY;
#endif

                if (bld.error_state != 0) {
                  cdd_cst_builder_free(&bld);
                  C_CDD_FREE(temp->children);
                  C_CDD_FREE(temp);
                  return (cdd_c_error_t)bld.error_state;
                }

                {
                  cdd_trivia_t *rt = rparen_tok->trailing_trivia;
                  size_t consume_count;
                  if (semi_idx > 0) {
                    cdd_token_t *semi_tok = node->children[semi_idx].val.token;
                    if (semi_tok->trailing_trivia) {
                      if (!rt) {
                        rt = semi_tok->trailing_trivia;
                        semi_tok->trailing_trivia = NULL;
                      }
                    }
                  }
                  rparen_tok->trailing_trivia = NULL;
                  temp->children[temp->num_children - 1]
                      .val.token->trailing_trivia = rt;

                  consume_count =
                      (semi_idx > rparen_idx) ? (semi_idx - rparen_idx + 1) : 1;
                  parent_ptr = node;
                  rc = cdd_cst_splice_children(tree, &parent_ptr, rparen_idx,
                                               consume_count, temp->children,
                                               temp->num_children);
                  if (rc != CDD_C_SUCCESS) {
                    rparen_tok->trailing_trivia = rt;
                    temp->children[temp->num_children - 1]
                        .val.token->trailing_trivia = NULL;
                    cdd_cst_builder_free(&bld);
                    C_CDD_FREE(temp->children);
                    C_CDD_FREE(temp);
                    return rc;
                  }
                  node = parent_ptr;
                }

                cdd_cst_builder_free(&bld);
                C_CDD_FREE(temp->children);
                C_CDD_FREE(temp);
                break;
              }
            }
          }
        }
      }
    } else {
      rc = cdd_rewrite_call_sites(tree, node->children[i].val.node,
                                  modified_funcs, num_modified);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Percolates errors by rewriting function signatures and returning int.
 *
 * @param[in,out] tree The CST tree.
 * @param[in] config Optional configuration for trivia generation.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
C_CDD_EXPORT cdd_c_error_t cdd_transform_percolate_errors(
    cdd_cst_tree_t *tree, const cdd_transform_config_t *config) {
  cdd_cst_query_result_t res;
  size_t i, j;
  cdd_c_error_t rc;
  cdd_token_t **modified_funcs = NULL;
  size_t modified_cap = 32;
  size_t num_modified = 0;
  (void)config;

  if (!tree || !tree->root)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  memset(&res, 0, sizeof(res));
  rc =
      cdd_cst_find_nodes_by_type(tree->root, CDD_CST_FUNCTION_DEFINITION, &res);
  if (rc != CDD_C_SUCCESS)
    return rc;

  modified_funcs =
      (cdd_token_t **)C_CDD_CALLOC(modified_cap, sizeof(cdd_token_t *));
  if (!modified_funcs) {
    C_CDD_FREE(res.nodes);
    return CDD_C_ERROR_MEMORY;
  }

  for (i = 0; i < res.size; i++) {
    cdd_cst_node_t *func = res.nodes[i];
    cdd_token_t *func_name_tok = NULL;
    cdd_token_t *rparen_tok = NULL;
    cdd_token_t *prev_rparen_tok = NULL;
    size_t func_name_idx = 0;
    size_t rparen_idx = 0;
    int is_strict_void = 0;
    int allocs_seen = 0;
    size_t ret_type_end = 0;
    int has_cdd_c_error = 0;
    size_t k;

    for (j = 0; j < func->num_children; j++) {
      if (func->children[j].kind == CDD_CST_CHILD_TOKEN) {
        cdd_token_t *tok = func->children[j].val.token;
        if (tok->kind == CDD_TOKEN_LPAREN) {
          if (j > 0) {
            if (func->children[j - 1].kind == CDD_CST_CHILD_TOKEN) {
              func_name_tok = func->children[j - 1].val.token;
              func_name_idx = j - 1;
              ret_type_end = j - 1;
            }
          }
        } else if (tok->kind == CDD_TOKEN_RPAREN) {
          rparen_tok = tok;
          rparen_idx = j;
          if (j > 0) {
            prev_rparen_tok = func->children[j - 1].val.token;
          }
          break;
        }
      }
    }

    if (!rparen_tok)
      continue;
    if (!func_name_tok)
      continue;
    if (func_name_idx == 0)
      continue;

    /* Semantic return type resolution */
    {
      int num_tokens = 0;
      cdd_token_t *last_ident = NULL;
      for (k = 0; k < ret_type_end; k++) {
        cdd_token_t *t = func->children[k].val.token;
        num_tokens++;
        if (t->kind == CDD_TOKEN_IDENTIFIER ||
            t->kind == CDD_TOKEN_KEYWORD_INT) {
          last_ident = t;
        }
      }

      if (num_tokens == 1 && last_ident) {
        if (last_ident->kind == CDD_TOKEN_IDENTIFIER) {
          if (last_ident->length == 4) {
            if (memcmp(last_ident->start, "void", 4) == 0) {
              is_strict_void = 1;
            }
          } else if (last_ident->length == 8) {
            if (memcmp(last_ident->start, "CDD_VOID", 8) == 0) {
              is_strict_void = 1;
            }
          }
        }
      }
    }

    /* Check if already returning error enum */
    for (k = 0; k < ret_type_end; k++) {
      cdd_token_t *t = func->children[k].val.token;
      if (t->kind == CDD_TOKEN_IDENTIFIER) {
        if (t->length == 13) {
          if (memcmp(t->start, "cdd_c_error_t", 13) == 0) {
            has_cdd_c_error = 1;
            break;
          }
        } else if (t->length == 11) {
          if (memcmp(t->start, "cdd_c_error", 11) == 0) {
            has_cdd_c_error = 1;
            break;
          }
        }
      }
    }
    if (has_cdd_c_error)
      continue;

#ifdef CDD_BUILD_TESTS
    if (g_err_perc_fail == 7) {
      C_CDD_FREE(modified_funcs);
      C_CDD_FREE(res.nodes);
      return CDD_C_ERROR_MEMORY;
    }
#endif

    if (num_modified >= modified_cap) {
      size_t new_cap = modified_cap * 2;
      cdd_token_t **new_funcs;
#ifdef CDD_BUILD_TESTS
      if (g_err_perc_fail == 8) {
        new_funcs = NULL;
      } else {
#endif
        new_funcs = (cdd_token_t **)C_CDD_REALLOC(
            modified_funcs, new_cap * sizeof(cdd_token_t *));
#ifdef CDD_BUILD_TESTS
      }
#endif
      if (!new_funcs) {
        C_CDD_FREE(modified_funcs);
        C_CDD_FREE(res.nodes);
        return CDD_C_ERROR_MEMORY;
      }
      modified_funcs = new_funcs;
      modified_cap = new_cap;
    }
    modified_funcs[num_modified++] = func_name_tok;

    if (!is_strict_void) {
      cdd_cst_builder_t bld;
      cdd_cst_node_t *temp =
          (cdd_cst_node_t *)C_CDD_CALLOC(1, sizeof(cdd_cst_node_t));
      int needs_comma;

      if (!temp) {
        C_CDD_FREE(modified_funcs);
        C_CDD_FREE(res.nodes);
        return CDD_C_ERROR_MEMORY;
      }

      temp->kind = CDD_CST_UNKNOWN;
      cdd_cst_builder_init(&bld, tree, temp);

      needs_comma = 1;
      if (prev_rparen_tok->kind == CDD_TOKEN_LPAREN) {
        needs_comma = 0;
      }
      if (needs_comma) {
        cdd_cst_bld_punct(&bld, ",");
        cdd_cst_bld_space(&bld);
      }

      /* Append original return type + *out_result */
      for (k = 0; k < ret_type_end; k++) {
        cdd_token_t *t = func->children[k].val.token;
        switch (t->kind) {
        case CDD_TOKEN_IDENTIFIER: {
          char *dup_id;
#ifdef CDD_BUILD_TESTS
          if (g_err_perc_fail == 9) {
            dup_id = NULL;
          } else {
#endif
            dup_id = (char *)(size_t)C_CDD_MALLOC(t->length + 1);
#ifdef CDD_BUILD_TESTS
          }
#endif
          if (dup_id) {
            memcpy(dup_id, t->start, t->length);
            dup_id[t->length] = '\0';
            cdd_cst_bld_ident(&bld, dup_id);
            C_CDD_FREE(dup_id);
          }
          break;
        }
        case CDD_TOKEN_KEYWORD_INT:
          cdd_cst_bld_ident(&bld, "int");
          break;
        case CDD_TOKEN_STAR: {
          char *dup_p;
#ifdef CDD_BUILD_TESTS
          if (g_err_perc_fail == 10) {
            dup_p = NULL;
          } else {
#endif
            dup_p = (char *)(size_t)C_CDD_MALLOC(t->length + 1);
#ifdef CDD_BUILD_TESTS
          }
#endif
          if (dup_p) {
            memcpy(dup_p, t->start, t->length);
            dup_p[t->length] = '\0';
            cdd_cst_bld_punct(&bld, dup_p);
            C_CDD_FREE(dup_p);
          }
          break;
        }
        default:
          break;
        }
        if (t->trailing_trivia) {
          cdd_cst_bld_space(&bld);
        }
      }

      cdd_cst_bld_space(&bld);
      cdd_cst_bld_punct(&bld, "*");
      cdd_cst_bld_ident(&bld, "out_result");
      cdd_cst_bld_punct(&bld, ")");

#ifdef CDD_BUILD_TESTS
      if (g_err_perc_fail == 11)
        bld.error_state = 1;
#endif

      if (bld.error_state == 0) {
        cdd_trivia_t *rt = rparen_tok->trailing_trivia;
        rparen_tok->trailing_trivia = NULL;
        temp->children[temp->num_children - 1].val.token->trailing_trivia = rt;
        rc = cdd_cst_splice_children(tree, &func, rparen_idx, 1, temp->children,
                                     temp->num_children);
        if (rc != CDD_C_SUCCESS) {
          rparen_tok->trailing_trivia = rt;
          temp->children[temp->num_children - 1].val.token->trailing_trivia =
              NULL;
          cdd_cst_builder_free(&bld);
          C_CDD_FREE(temp->children);
          C_CDD_FREE(temp);
          C_CDD_FREE(modified_funcs);
          C_CDD_FREE(res.nodes);
          return rc;
        }
      }
      cdd_cst_builder_free(&bld);
      C_CDD_FREE(temp->children);
      C_CDD_FREE(temp);
    }

    {
      /* Replace entire return type with cdd_c_error_t */
      cdd_cst_node_t *parent_ptr = func;
      cdd_cst_node_t *temp =
          (cdd_cst_node_t *)C_CDD_CALLOC(1, sizeof(cdd_cst_node_t));
      cdd_cst_builder_t bld;

      if (!temp) {
        C_CDD_FREE(modified_funcs);
        C_CDD_FREE(res.nodes);
        return CDD_C_ERROR_MEMORY;
      }

      temp->kind = CDD_CST_UNKNOWN;
      cdd_cst_builder_init(&bld, tree, temp);
      cdd_cst_bld_ident(&bld, "cdd_c_error_t");
      cdd_cst_bld_space(&bld);

#ifdef CDD_BUILD_TESTS
      if (g_err_perc_fail == 12)
        bld.error_state = 1;
#endif

      if (bld.error_state == 0) {
        size_t start_idx = 0;
        cdd_trivia_t *lt = NULL;
        lt = func->children[0].val.token->leading_trivia;
        func->children[0].val.token->leading_trivia = NULL;
        temp->children[0].val.token->leading_trivia = lt;

        rc = cdd_cst_splice_children(tree, &parent_ptr, start_idx, ret_type_end,
                                     temp->children, temp->num_children);
        if (rc != CDD_C_SUCCESS) {
          func->children[0].val.token->leading_trivia = lt;
          temp->children[0].val.token->leading_trivia = NULL;
          cdd_cst_builder_free(&bld);
          C_CDD_FREE(temp->children);
          C_CDD_FREE(temp);
          C_CDD_FREE(modified_funcs);
          C_CDD_FREE(res.nodes);
          return rc;
        }
        func = parent_ptr;
        func_name_idx = func_name_idx - ret_type_end + temp->num_children;
        rparen_idx = rparen_idx - ret_type_end + temp->num_children;
      }
      cdd_cst_builder_free(&bld);
      C_CDD_FREE(temp->children);
      C_CDD_FREE(temp);
    }

    {
      cdd_cst_query_result_t stmts_res;
      cdd_c_error_t q_rc =
          cdd_cst_find_nodes_by_type(func, CDD_CST_UNKNOWN, &stmts_res);
      if (q_rc != CDD_C_SUCCESS) {
        C_CDD_FREE(modified_funcs);
        C_CDD_FREE(res.nodes);
        return q_rc;
      }

      {
        size_t s_idx;
        for (s_idx = 0; s_idx < stmts_res.size; s_idx++) {
          cdd_cst_node_t *stmt = stmts_res.nodes[s_idx];
          int has_alloc = 0;
          int is_return = 0;
          size_t c_idx;
          size_t assign_idx = (size_t)-1;

          for (c_idx = 0; c_idx < stmt->num_children; c_idx++) {
            cdd_token_t *t = stmt->children[c_idx].val.token;
            switch (t->kind) {
            case CDD_TOKEN_KEYWORD_RETURN:
              is_return = 1;
              break;
            case CDD_TOKEN_ASSIGN:
              assign_idx = c_idx;
              break;
            case CDD_TOKEN_IDENTIFIER:
              if (t->length == 6) {
                if (memcmp(t->start, "malloc", 6) == 0) {
                  has_alloc = 1;
                } else if (memcmp(t->start, "calloc", 6) == 0) {
                  has_alloc = 1;
                } else if (memcmp(t->start, "strdup", 6) == 0) {
                  has_alloc = 1;
                }
              } else if (t->length == 7) {
                if (memcmp(t->start, "realloc", 7) == 0) {
                  has_alloc = 1;
                }
              }
              break;
            default:
              break;
            }
          }

          if (has_alloc) {
            cdd_cst_builder_t bld;
            cdd_cst_node_t *cloned;
            char *tmp_name = NULL;
            size_t len = 0;
            size_t start_idx = 0;
            size_t off = 0;

            if (assign_idx == (size_t)-1) {
              C_CDD_FREE(stmts_res.nodes);
              C_CDD_FREE(modified_funcs);
              C_CDD_FREE(res.nodes);
              return CDD_C_ERROR_PARSE;
            }

            cloned = (cdd_cst_node_t *)C_CDD_CALLOC(1, sizeof(cdd_cst_node_t));
            if (!cloned) {
              C_CDD_FREE(stmts_res.nodes);
              C_CDD_FREE(modified_funcs);
              C_CDD_FREE(res.nodes);
              return CDD_C_ERROR_MEMORY;
            }

            /* Extract L-value */
            for (k = assign_idx; k-- > 0;) {
              cdd_token_t *t = stmt->children[k].val.token;
              switch (t->kind) {
              case CDD_TOKEN_IDENTIFIER:
              case CDD_TOKEN_DOT:
              case CDD_TOKEN_ARROW:
              case CDD_TOKEN_LBRACKET:
              case CDD_TOKEN_RBRACKET:
              case CDD_TOKEN_NUMBER:
              case CDD_TOKEN_STAR:
                start_idx = k;
                break;
              default:
                k = 0;
                break;
              }
            }

            for (k = start_idx; k < assign_idx; k++) {
              cdd_token_t *t = stmt->children[k].val.token;
              len += t->length;
            }

            tmp_name = (char *)(size_t)C_CDD_MALLOC(len + 1);
            if (!tmp_name) {
              C_CDD_FREE(cloned);
              C_CDD_FREE(stmts_res.nodes);
              C_CDD_FREE(modified_funcs);
              C_CDD_FREE(res.nodes);
              return CDD_C_ERROR_MEMORY;
            }

            for (k = start_idx; k < assign_idx; k++) {
              cdd_token_t *t = stmt->children[k].val.token;
              memcpy(tmp_name + off, t->start, t->length);
              off += t->length;
            }
            tmp_name[len] = '\0';

            if (tree->num_strings >= tree->string_capacity) {
              size_t new_cap = tree->string_capacity * 2;
              char **new_pool;
              if (new_cap < 32)
                new_cap = 32;
              new_pool = (char **)C_CDD_REALLOC(tree->string_pool,
                                                new_cap * sizeof(char *));
              if (!new_pool) {
                C_CDD_FREE(tmp_name);
                C_CDD_FREE(cloned);
                C_CDD_FREE(stmts_res.nodes);
                C_CDD_FREE(modified_funcs);
                C_CDD_FREE(res.nodes);
                return CDD_C_ERROR_MEMORY;
              }
              tree->string_pool = new_pool;
              tree->string_capacity = new_cap;
            }
            tree->string_pool[tree->num_strings++] = tmp_name;

            cloned->kind = CDD_CST_UNKNOWN;
            cdd_cst_builder_init(&bld, tree, cloned);
            cdd_cst_bld_newline(&bld);
            cdd_cst_bld_space(&bld);
            cdd_cst_bld_space(&bld);
            cdd_cst_bld_ident(&bld, "if");
            cdd_cst_bld_space(&bld);
            cdd_cst_bld_punct(&bld, "(");
            cdd_cst_bld_punct(&bld, "!");
            cdd_cst_bld_ident(&bld, tmp_name);
            cdd_cst_bld_punct(&bld, ")");
            cdd_cst_bld_space(&bld);
            cdd_cst_bld_punct(&bld, "{");
            cdd_cst_bld_space(&bld);

            if (allocs_seen == 0) {
              cdd_cst_bld_ident(&bld, "return");
              cdd_cst_bld_space(&bld);
              cdd_cst_bld_ident(&bld, "CDD_C_ERROR_MEMORY");
              cdd_cst_bld_punct(&bld, ";");
            } else {
              cdd_cst_bld_ident(&bld, "rc");
              cdd_cst_bld_space(&bld);
              cdd_cst_bld_punct(&bld, "=");
              cdd_cst_bld_space(&bld);
              cdd_cst_bld_ident(&bld, "CDD_C_ERROR_MEMORY");
              cdd_cst_bld_punct(&bld, ";");
              cdd_cst_bld_space(&bld);
              cdd_cst_bld_ident(&bld, "goto");
              cdd_cst_bld_space(&bld);
              cdd_cst_bld_ident(&bld, "cleanup");
              cdd_cst_bld_punct(&bld, ";");
            }
            cdd_cst_bld_space(&bld);
            cdd_cst_bld_punct(&bld, "}");

#ifdef CDD_BUILD_TESTS
            if (g_err_perc_fail == 1)
              bld.error_state = 1;
#endif
            if (bld.error_state == 0) {
              cdd_cst_insert_node_after(stmt, cloned);
            } else {
              C_CDD_FREE(cloned->children);
              C_CDD_FREE(cloned);
            }
            cdd_cst_builder_free(&bld);
            allocs_seen++;
          }

          if (is_return) {
            cdd_token_t *ret_tok = NULL;
            cdd_token_t *semi_tok = NULL;
            size_t r_idx;
            size_t ret_semi_idx = 0;
            for (r_idx = 0; r_idx < stmt->num_children; r_idx++) {
              cdd_token_t *t = stmt->children[r_idx].val.token;
              if (t->kind == CDD_TOKEN_KEYWORD_RETURN) {
                ret_tok = t;
              } else if (t->kind == CDD_TOKEN_SEMICOLON) {
                semi_tok = t;
                ret_semi_idx = r_idx;
              }
            }

            if (semi_tok) {
              cdd_token_t *prev_t = stmt->children[ret_semi_idx - 1].val.token;
              if (prev_t == ret_tok) {
                if (allocs_seen > 0) {
                  ret_tok->start =
                      (const uint8_t *)"rc = CDD_C_SUCCESS; goto "
                                       "cleanup;\ncleanup:\n  return rc;";
                  ret_tok->length = 43;
                } else {
                  ret_tok->start = (const uint8_t *)"return 0";
                  ret_tok->length = 8;
                }
              }
            }
          }
        }

        if (allocs_seen > 0) {
          size_t c_i;
          cdd_cst_node_t *decl_node =
              (cdd_cst_node_t *)C_CDD_CALLOC(1, sizeof(cdd_cst_node_t));
          cdd_cst_node_t *cleanup_node =
              (cdd_cst_node_t *)C_CDD_CALLOC(1, sizeof(cdd_cst_node_t));

          if (!decl_node || !cleanup_node) {
            if (decl_node)
              C_CDD_FREE(decl_node);
            if (cleanup_node)
              C_CDD_FREE(cleanup_node);
            C_CDD_FREE(stmts_res.nodes);
            C_CDD_FREE(modified_funcs);
            C_CDD_FREE(res.nodes);
            return CDD_C_ERROR_MEMORY;
          }

          {
            cdd_cst_builder_t bld_decl;
            cdd_cst_builder_t bld_cleanup;

            decl_node->kind = CDD_CST_UNKNOWN;
            cdd_cst_builder_init(&bld_decl, tree, decl_node);
            cdd_cst_bld_newline(&bld_decl);
            cdd_cst_bld_space(&bld_decl);
            cdd_cst_bld_space(&bld_decl);
            cdd_cst_bld_ident(&bld_decl, "enum");
            cdd_cst_bld_space(&bld_decl);
            cdd_cst_bld_ident(&bld_decl, "cdd_c_error");
            cdd_cst_bld_space(&bld_decl);
            cdd_cst_bld_ident(&bld_decl, "rc");
            cdd_cst_bld_space(&bld_decl);
            cdd_cst_bld_punct(&bld_decl, "=");
            cdd_cst_bld_space(&bld_decl);
            cdd_cst_bld_ident(&bld_decl, "CDD_C_SUCCESS");
            cdd_cst_bld_punct(&bld_decl, ";");

#ifdef CDD_BUILD_TESTS
            if (g_err_perc_fail == 3) {
              bld_decl.error_state = 1;
            }
#endif

            cleanup_node->kind = CDD_CST_UNKNOWN;
            cdd_cst_builder_init(&bld_cleanup, tree, cleanup_node);
            cdd_cst_bld_newline(&bld_cleanup);
            cdd_cst_bld_ident(&bld_cleanup, "cleanup");
            cdd_cst_bld_punct(&bld_cleanup, ":");
            cdd_cst_bld_newline(&bld_cleanup);
            cdd_cst_bld_space(&bld_cleanup);
            cdd_cst_bld_space(&bld_cleanup);
            cdd_cst_bld_ident(&bld_cleanup, "return");
            cdd_cst_bld_space(&bld_cleanup);
            cdd_cst_bld_ident(&bld_cleanup, "rc");
            cdd_cst_bld_punct(&bld_cleanup, ";");
            cdd_cst_bld_newline(&bld_cleanup);

#ifdef CDD_BUILD_TESTS
            if (g_err_perc_fail == 2) {
              bld_cleanup.error_state = 1;
            }
#endif

            if (bld_decl.error_state == 0 && bld_cleanup.error_state == 0) {
              int found_lbrace = 0;
              int found_rbrace = 0;
              size_t brace_idx = 0;
              cdd_cst_node_t *body_parent =
                  func->children[func->num_children - 1].val.node;

              for (c_i = 0; c_i < body_parent->num_children; c_i++) {
                if (body_parent->children[c_i].kind == CDD_CST_CHILD_TOKEN) {
                  if (body_parent->children[c_i].val.token->kind ==
                      CDD_TOKEN_LBRACE) {
                    found_lbrace = 1;
                    brace_idx = c_i;
                    break;
                  }
                }
              }
              if (found_lbrace) {
                cdd_cst_splice_children(tree, &body_parent, brace_idx + 1, 0,
                                        decl_node->children,
                                        decl_node->num_children);
                func = body_parent;
              }

              for (c_i = body_parent->num_children; c_i-- > 0;) {
                if (body_parent->children[c_i].kind == CDD_CST_CHILD_TOKEN) {
                  if (body_parent->children[c_i].val.token->kind ==
                      CDD_TOKEN_RBRACE) {
                    found_rbrace = 1;
                    brace_idx = c_i;
                    break;
                  }
                }
              }
              if (found_rbrace) {
                cdd_cst_splice_children(tree, &body_parent, brace_idx, 0,
                                        cleanup_node->children,
                                        cleanup_node->num_children);
                func = body_parent;
              }
            }

            cdd_cst_builder_free(&bld_decl);
            cdd_cst_builder_free(&bld_cleanup);
          }
          C_CDD_FREE(decl_node->children);
          C_CDD_FREE(decl_node);
          C_CDD_FREE(cleanup_node->children);
          C_CDD_FREE(cleanup_node);
        }

        C_CDD_FREE(stmts_res.nodes);
      }
    }
  }

  C_CDD_FREE(res.nodes);

  if (num_modified > 0) {
    rc = cdd_rewrite_call_sites(tree, tree->root, modified_funcs, num_modified);
    C_CDD_FREE(modified_funcs);
    if (rc != CDD_C_SUCCESS)
      return rc;
  } else {
    C_CDD_FREE(modified_funcs);
  }

  return CDD_C_SUCCESS;
}
