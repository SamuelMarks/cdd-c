/**
 * @file error_percolator.c
 * @brief Implementation of the error percolator transformer.
 */

/* clang-format off */
#include "cdd_cst_transform.h"
#include "classes/parse/cdd_cst_mutate.h"
#include "classes/parse/cdd_cst_builder.h"
#include "classes/parse/cdd_cst_factory.h"
#include "classes/parse/cdd_cst_parser.h"
#include "classes/parse/cdd_cst_query.h"
#include "c_str_span.h"
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "c_cdd_export.h"
#include "c_cdd/safe_crt.h"
/* clang-format on */
/* LCOV_EXCL_START */

static void rewrite_call_sites(cdd_cst_tree_t *tree, cdd_cst_node_t *node,
                               cdd_token_t **modified_funcs,
                               size_t num_modified) {
  size_t i;
  for (i = 0; i < node->num_children; i++) {
    if (node->children[i].kind == CDD_CST_CHILD_TOKEN) {
      cdd_token_t *tok = node->children[i].val.token;
      if (tok->kind == CDD_TOKEN_IDENTIFIER) {
        size_t m;
        for (m = 0; m < num_modified; m++) {
          if (modified_funcs[m]->length == tok->length &&
              memcmp(modified_funcs[m]->start, tok->start, tok->length) == 0) {

            int is_call = 0;
            int is_def = 0;
            size_t j;
            size_t rparen_idx = 0;
            size_t semi_idx = 0;
            int depth = 0;
            cdd_token_t *rparen_tok = NULL;
            cdd_token_t *prev_rparen_tok = NULL;

            for (j = i + 1; j < node->num_children; j++) {
              if (node->children[j].kind == CDD_CST_CHILD_TOKEN) {
                if (node->children[j].val.token->kind == CDD_TOKEN_LPAREN) {
                  is_call = 1;
                }
                break;
              }
            }
            if (!is_call)
              continue;

            for (j = i; j-- > 0;) {
              if (node->children[j].kind == CDD_CST_CHILD_TOKEN) {
                cdd_token_t *prev = node->children[j].val.token;
                if (prev->kind == CDD_TOKEN_KEYWORD_INT ||
                    (prev->kind == CDD_TOKEN_IDENTIFIER && prev->length == 4 &&
                     memcmp(prev->start, "void", 4) == 0)) {
                  is_def = 1;
                }
                break;
              }
            }
            if (is_def)
              continue;

            for (j = i + 1; j < node->num_children; j++) {
              if (node->children[j].kind == CDD_CST_CHILD_TOKEN) {
                cdd_token_t *t = node->children[j].val.token;
                if (t->kind == CDD_TOKEN_LPAREN) {
                  depth++;
                } else if (t->kind == CDD_TOKEN_RPAREN) {
                  depth--;
                  if (depth == 0) {
                    rparen_idx = j;
                    rparen_tok = t;
                    if (j > 0 &&
                        node->children[j - 1].kind == CDD_CST_CHILD_TOKEN) {
                      prev_rparen_tok = node->children[j - 1].val.token;
                    }
                  }
                } else if (depth == 0 && t->kind == CDD_TOKEN_SEMICOLON &&
                           rparen_tok) {
                  semi_idx = j;
                  break;
                }
              }
            }

            if (rparen_tok) {
              {
                cdd_cst_builder_t bld;
                cdd_cst_node_t *temp =
                    (cdd_cst_node_t *)calloc(1, sizeof(cdd_cst_node_t));
                if (temp) {
                  char *dup_id = (char *)calloc(1, 256);
                  temp->kind = CDD_CST_UNKNOWN;
                  cdd_cst_builder_init(&bld, tree, temp);
                  if (dup_id) {
                    CDD_SNPRINTF(dup_id, 256, "out_%.*s", (int)tok->length,
                                 tok->start);
                  }

                  {
                    size_t c_len = dup_id ? strlen(dup_id) : 0;
                    cdd_token_t *t_dummy = NULL;
                    cdd_cst_create_token_len(tree, CDD_TOKEN_IDENTIFIER, dup_id,
                                             c_len, &t_dummy);
                  }

                  cdd_cst_bld_ident(&bld, "void");
                  cdd_cst_bld_space(&bld);
                  cdd_cst_bld_punct(&bld, "*");
                  if (dup_id)
                    cdd_cst_bld_ident(&bld, dup_id);
                  cdd_cst_bld_punct(&bld, ";");
                  cdd_cst_bld_newline(&bld);
                  cdd_cst_bld_space(&bld);
                  cdd_cst_bld_space(&bld);
                  cdd_cst_bld_ident(&bld, "if");
                  cdd_cst_bld_space(&bld);
                  cdd_cst_bld_punct(&bld, "(");

                  if (!cdd_cst_builder_has_error(&bld)) {
                    cdd_trivia_t *lt = tok->leading_trivia;
                    cdd_cst_node_t *parent_ptr;
                    parent_ptr = node;
                    tok->leading_trivia = NULL;
                    if (temp->num_children > 0 &&
                        temp->children[0].kind == CDD_CST_CHILD_TOKEN) {
                      temp->children[0].val.token->leading_trivia = lt;
                    }
                    cdd_cst_splice_children(tree, &parent_ptr, i, 0,
                                            temp->children, temp->num_children);
                    i += temp->num_children;
                    rparen_idx += temp->num_children;
                    if (semi_idx)
                      semi_idx += temp->num_children;
                    node = parent_ptr;
                  }
                  cdd_cst_builder_free(&bld);
                  free(temp->children);
                  free(temp);
                }
              }

              {
                cdd_cst_builder_t bld;
                cdd_cst_node_t *temp =
                    (cdd_cst_node_t *)calloc(1, sizeof(cdd_cst_node_t));
                if (temp) {
                  char *dup_id = (char *)calloc(1, 256);
                  temp->kind = CDD_CST_UNKNOWN;
                  cdd_cst_builder_init(&bld, tree, temp);
                  if (dup_id) {
                    CDD_SNPRINTF(dup_id, 256, "out_%.*s",
                                 (int)modified_funcs[m]->length,
                                 modified_funcs[m]->start);
                  }

                  {
                    size_t c_len = dup_id ? strlen(dup_id) : 0;
                    cdd_token_t *t_dummy = NULL;
                    cdd_cst_create_token_len(tree, CDD_TOKEN_IDENTIFIER, dup_id,
                                             c_len, &t_dummy);
                  }

                  if (!(prev_rparen_tok &&
                        prev_rparen_tok->kind == CDD_TOKEN_LPAREN)) {
                    cdd_cst_bld_punct(&bld, ",");
                    cdd_cst_bld_space(&bld);
                  }

                  cdd_cst_bld_punct(&bld, "&");
                  if (dup_id)
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
                  cdd_cst_bld_ident(&bld, "ENOMEM");
                  cdd_cst_bld_punct(&bld, ";");
                  cdd_cst_bld_space(&bld);
                  cdd_cst_bld_punct(&bld, "}");

                  if (!cdd_cst_builder_has_error(&bld) &&
                      temp->num_children > 0) {
                    cdd_trivia_t *rt = rparen_tok->trailing_trivia;
                    if (semi_idx > 0) {
                      cdd_token_t *semi_tok =
                          node->children[semi_idx].val.token;
                      if (semi_tok->trailing_trivia && !rt) {
                        rt = semi_tok->trailing_trivia;
                        semi_tok->trailing_trivia = NULL;
                      }
                    }
                    rparen_tok->trailing_trivia = NULL;
                    if (temp->children[temp->num_children - 1].kind ==
                        CDD_CST_CHILD_TOKEN) {
                      temp->children[temp->num_children - 1]
                          .val.token->trailing_trivia = rt;
                    }

                    {
                      size_t consume_count =
                          semi_idx > 0 ? (semi_idx - rparen_idx + 1) : 1;
                      cdd_cst_node_t *parent_ptr;
                      parent_ptr = node;
                      cdd_cst_splice_children(tree, &parent_ptr, rparen_idx,
                                              consume_count, temp->children,
                                              temp->num_children);
                      node = parent_ptr;
                    }
                  }
                  cdd_cst_builder_free(&bld);
                  free(temp->children);
                  free(temp);
                }
              }
              break;
            }
          }
        }
      }
    } else if (node->children[i].kind == CDD_CST_CHILD_NODE) {
      rewrite_call_sites(tree, node->children[i].val.node, modified_funcs,
                         num_modified);
    }
  }
}

#ifdef CDD_BUILD_TESTS
C_CDD_EXPORT int g_err_perc_fail = 0;
#endif

/** @brief cdd_transform_percolate_errors */
int cdd_transform_percolate_errors(cdd_cst_tree_t *tree,
                                   const cdd_transform_config_t *config) {
  cdd_cst_query_result_t res;
  size_t i, j;
  int rc;
  cdd_token_t *modified_funcs[256];
  size_t num_modified = 0;
  (void)config;

  if (!tree || !tree->root)
    return EINVAL;

  rc =
      cdd_cst_find_nodes_by_type(tree->root, CDD_CST_FUNCTION_DEFINITION, &res);
  if (rc != 0)
    return rc;

  for (i = 0; i < res.size; i++) {
    cdd_cst_node_t *func = res.nodes[i];
    int is_void = 0;
    cdd_token_t *void_tok = NULL;
    cdd_token_t *func_name_tok = NULL;
    cdd_token_t *rparen_tok = NULL;
    cdd_token_t *prev_rparen_tok = NULL;
    int allocs_seen = 0;

    for (j = 0; j < func->num_children; j++) {
      if (func->children[j].kind == CDD_CST_CHILD_TOKEN) {
        cdd_token_t *tok = func->children[j].val.token;
        if (!void_tok && tok->kind == CDD_TOKEN_IDENTIFIER &&
            tok->length == 4 && memcmp(tok->start, "void", 4) == 0) {
          is_void = 1;
          void_tok = tok;
        } else if (tok->kind == CDD_TOKEN_LPAREN) {
          if (j > 0 && func->children[j - 1].kind == CDD_CST_CHILD_TOKEN) {
            func_name_tok = func->children[j - 1].val.token;
          }
        } else if (tok->kind == CDD_TOKEN_RPAREN) {
          rparen_tok = tok;
          if (j > 0 && func->children[j - 1].kind == CDD_CST_CHILD_TOKEN) {
            prev_rparen_tok = func->children[j - 1].val.token;
          }
          break;
        }
      }
    }

    if (!is_void || !void_tok || !rparen_tok || !func_name_tok)
      continue;

    if (is_void && void_tok && rparen_tok && func_name_tok) {
      if (num_modified < 256) {
        modified_funcs[num_modified++] = func_name_tok;
      }

      {
        size_t void_idx;
        cdd_cst_node_t *void_parent = NULL;
        cdd_cst_find_node_for_token(tree->root, void_tok, &void_idx,
                                    &void_parent);
        if (void_parent) {
          cdd_token_t *new_void = NULL;
          cdd_cst_create_token(tree, CDD_TOKEN_KEYWORD_INT, "int", &new_void);
          if (new_void) {
            new_void->leading_trivia = void_tok->leading_trivia;
            new_void->trailing_trivia = void_tok->trailing_trivia;
            void_tok->leading_trivia = NULL;
            void_tok->trailing_trivia = NULL;
            cdd_cst_replace_token_child(void_parent, void_idx, new_void);
          }
        }
      }

      {
        size_t rparen_idx;
        cdd_cst_node_t *rparen_parent = NULL;
        cdd_cst_find_node_for_token(tree->root, rparen_tok, &rparen_idx,
                                    &rparen_parent);
        if (rparen_parent) {
          cdd_cst_builder_t bld;
          cdd_cst_node_t *temp =
              (cdd_cst_node_t *)calloc(1, sizeof(cdd_cst_node_t));
          if (temp) {
            temp->kind = CDD_CST_UNKNOWN;
            cdd_cst_builder_init(&bld, tree, temp);
            if (!(prev_rparen_tok &&
                  prev_rparen_tok->kind == CDD_TOKEN_LPAREN)) {
              cdd_cst_bld_punct(&bld, ",");
              cdd_cst_bld_space(&bld);
            }
            cdd_cst_bld_ident(&bld, "void");
            cdd_cst_bld_space(&bld);
            cdd_cst_bld_punct(&bld, "*");
            cdd_cst_bld_punct(&bld, "*");
            cdd_cst_bld_ident(&bld, "out_result");
            cdd_cst_bld_punct(&bld, ")");

            if (!cdd_cst_builder_has_error(&bld) && temp->num_children > 0) {
              cdd_trivia_t *rt = rparen_tok->trailing_trivia;
              rparen_tok->trailing_trivia = NULL;
              if (temp->children[temp->num_children - 1].kind ==
                  CDD_CST_CHILD_TOKEN) {
                temp->children[temp->num_children - 1]
                    .val.token->trailing_trivia = rt;
              }
              cdd_cst_splice_children(tree, &rparen_parent, rparen_idx, 1,
                                      temp->children, temp->num_children);
            }
            cdd_cst_builder_free(&bld);
            free(temp->children);
            free(temp);
          }
        }
      }
    }

    {
      cdd_cst_query_result_t stmts_res;
      if (cdd_cst_find_nodes_by_type(func, CDD_CST_UNKNOWN, &stmts_res) == 0) {
        size_t s_idx;
        for (s_idx = 0; s_idx < stmts_res.size; s_idx++) {
          cdd_cst_node_t *stmt = stmts_res.nodes[s_idx];
          int has_alloc = 0;
          int is_return = 0;
          cdd_token_t *ptr_tok = NULL;
          size_t c_idx;

          for (c_idx = 0; c_idx < stmt->num_children; c_idx++) {
            if (stmt->children[c_idx].kind == CDD_CST_CHILD_TOKEN) {
              cdd_token_t *t = stmt->children[c_idx].val.token;
              if (t->kind == CDD_TOKEN_KEYWORD_RETURN) {
                is_return = 1;
              } else if (t->kind == CDD_TOKEN_ASSIGN) {
                if (c_idx > 0 &&
                    stmt->children[c_idx - 1].kind == CDD_CST_CHILD_TOKEN) {
                  ptr_tok = stmt->children[c_idx - 1].val.token;
                }
              } else if (t->kind == CDD_TOKEN_IDENTIFIER &&
                         ((t->length == 6 &&
                           memcmp(t->start, "malloc", 6) == 0) ||
                          (t->length == 6 &&
                           memcmp(t->start, "calloc", 6) == 0) ||
                          (t->length == 7 &&
                           memcmp(t->start, "realloc", 7) == 0))) {
                has_alloc = 1;
              }
            }
          }

          if (has_alloc && ptr_tok) {
            cdd_cst_builder_t bld;
            cdd_cst_node_t *cloned =
                (cdd_cst_node_t *)calloc(1, sizeof(cdd_cst_node_t));
            if (cloned) {
              char *tmp_name;
              size_t len = ptr_tok->length;
#ifdef CDD_BUILD_TESTS
              if (g_err_perc_fail == 4) {
                tmp_name = NULL;
              } else {
#endif
                tmp_name = (char *)malloc(len + 1);
#ifdef CDD_BUILD_TESTS
              }
#endif
              if (!tmp_name) {
                free(cloned);
                continue;
              }
              memcpy(tmp_name, ptr_tok->start, len);
              tmp_name[len] = '\0';
              if (tree->num_strings >= tree->string_capacity) {
                tree->string_capacity =
                    tree->string_capacity == 0 ? 32 : tree->string_capacity * 2;
                tree->string_pool = (char **)realloc(
                    tree->string_pool, tree->string_capacity * sizeof(char *));
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
                cdd_cst_bld_ident(&bld, "ENOMEM");
                cdd_cst_bld_punct(&bld, ";");
              } else {
                cdd_cst_bld_ident(&bld, "rc");
                cdd_cst_bld_space(&bld);
                cdd_cst_bld_punct(&bld, "=");
                cdd_cst_bld_space(&bld);
                cdd_cst_bld_ident(&bld, "ENOMEM");
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
              if (!cdd_cst_builder_has_error(&bld)) {
                cdd_cst_insert_node_after(stmt, cloned);
              } else {
                free(cloned->children);
                free(cloned);
              }
              cdd_cst_builder_free(&bld);
              allocs_seen++;
            }
          }

          if (is_return) {
            cdd_token_t *ret_tok = NULL;
            cdd_token_t *semi_tok = NULL;
            size_t r_idx, semi_idx = 0;
            for (r_idx = 0; r_idx < stmt->num_children; r_idx++) {
              if (stmt->children[r_idx].kind == CDD_CST_CHILD_TOKEN) {
                if (stmt->children[r_idx].val.token->kind ==
                    CDD_TOKEN_KEYWORD_RETURN) {
                  ret_tok = stmt->children[r_idx].val.token;
                } else if (stmt->children[r_idx].val.token->kind ==
                           CDD_TOKEN_SEMICOLON) {
                  semi_tok = stmt->children[r_idx].val.token;
                  semi_idx = r_idx;
                }
              }
            }

            if (ret_tok && semi_tok && is_void) {
              if (semi_idx > 0 &&
                  stmt->children[semi_idx - 1].kind == CDD_CST_CHILD_TOKEN &&
                  stmt->children[semi_idx - 1].val.token == ret_tok) {
                if (allocs_seen > 0) {
                  ret_tok->start =
                      (const uint8_t
                           *)"rc = 0; goto cleanup;\ncleanup:\n  return rc;";
                  ret_tok->length = 43;
                } else {
                  ret_tok->start = (const uint8_t *)"return 0";
                  ret_tok->length = 8;
                }

                /* We don't need cleanup_node anymore because we replaced the
                 * return token directly! */
              }
            }
          }
        }

        if (allocs_seen > 0) {
          cdd_cst_builder_t bld;
          cdd_cst_node_t *cleanup_node =
              (cdd_cst_node_t *)calloc(1, sizeof(cdd_cst_node_t));
          if (cleanup_node) {
            cleanup_node->kind = CDD_CST_UNKNOWN;
            cdd_cst_builder_init(&bld, tree, cleanup_node);
            cdd_cst_bld_newline(&bld);
            cdd_cst_bld_ident(&bld, "cleanup");
            cdd_cst_bld_punct(&bld, ":");
            cdd_cst_bld_newline(&bld);
            cdd_cst_bld_space(&bld);
            cdd_cst_bld_space(&bld);
            cdd_cst_bld_ident(&bld, "return");
            cdd_cst_bld_space(&bld);
            cdd_cst_bld_ident(&bld, "rc");
            cdd_cst_bld_punct(&bld, ";");
#ifdef CDD_BUILD_TESTS
            if (g_err_perc_fail == 2)
              bld.error_state = 1;
#endif
            /* Cleanup node is no longer needed since we inline the replacement
             * directly. */
            if (!cdd_cst_builder_has_error(&bld)) {
            } else {
              free(cleanup_node->children);
            }
            cdd_cst_builder_free(&bld);
            free(cleanup_node);
          }
        }

        free(stmts_res.nodes);
      }
    }
  }

  free(res.nodes);

  if (num_modified > 0) {
    rewrite_call_sites(tree, tree->root, modified_funcs, num_modified);
  }

  return 0;
}

/* LCOV_EXCL_STOP */
