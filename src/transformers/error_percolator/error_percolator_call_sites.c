/**
 * @file error_percolator_call_sites.c
 * @brief Call site rewriting routines for error percolator transformer.
 *
 * @author Samuel Marks
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
extern C_CDD_EXPORT int g_err_perc_fail;
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

#ifdef CDD_BUILD_TESTS
  if (g_err_perc_fail == 22)
    return CDD_C_ERROR_INVALID_ARGUMENT;
#endif

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

#ifdef CDD_BUILD_TESTS
  if (g_err_perc_fail == 23)
    return CDD_C_ERROR_INVALID_ARGUMENT;
#endif

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

              rc = cdd_check_is_call(node, i, &is_call);
              if (rc != CDD_C_SUCCESS)
                return rc;
              if (!is_call)
                continue;

              rc = cdd_check_is_function_def(node, i, &is_def);
              if (rc != CDD_C_SUCCESS)
                return rc;
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

#ifdef CDD_BUILD_TESTS
                if (g_err_perc_fail == 13)
                  rc = CDD_C_ERROR_MEMORY;
                else
#endif
                  rc = cdd_cst_splice_children(tree, &parent_ptr, i, 0,
                                               temp->children,
                                               temp->num_children);
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
#ifdef CDD_BUILD_TESTS
                  if (g_err_perc_fail == 14)
                    rc = CDD_C_ERROR_MEMORY;
                  else
#endif
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
