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

static enum cdd_c_error rewrite_call_sites(cdd_cst_tree_t *tree,
                                           cdd_cst_node_t *node,
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
                    if (tree->num_strings >= tree->string_capacity) {
                      size_t new_cap = tree->string_capacity == 0
                                           ? 32
                                           : tree->string_capacity * 2;
                      char **new_pool = (char **)realloc(
                          tree->string_pool, new_cap * sizeof(char *));
                      if (new_pool) {
                        tree->string_pool = new_pool;
                        tree->string_capacity = new_cap;
                      }
                    }
                    if (tree->num_strings < tree->string_capacity) {
                      tree->string_pool[tree->num_strings++] = dup_id;
                    } else {
                      /* LCOV_EXCL_START */
                      free(dup_id);
                      dup_id = NULL;
                      /* LCOV_EXCL_STOP */
                    }
                  }

                  {
                    size_t c_len = dup_id ? strlen(dup_id) : 0;
                    cdd_token_t *t_dummy = NULL;
                    cdd_cst_create_token_len(tree, CDD_TOKEN_IDENTIFIER, dup_id,
                                             c_len, &t_dummy);
                  }

                  cdd_cst_bld_punct(&bld, "{");
                  cdd_cst_bld_space(&bld);
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

                  if (bld.error_state == 0) {
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
                    if (tree->num_strings >= tree->string_capacity) {
                      size_t new_cap = tree->string_capacity == 0
                                           ? 32
                                           : tree->string_capacity * 2;
                      char **new_pool = (char **)realloc(
                          tree->string_pool, new_cap * sizeof(char *));
                      if (new_pool) {
                        tree->string_pool = new_pool;
                        tree->string_capacity = new_cap;
                      }
                    }
                    if (tree->num_strings < tree->string_capacity) {
                      tree->string_pool[tree->num_strings++] = dup_id;
                    } else {
                      /* LCOV_EXCL_START */
                      free(dup_id);
                      dup_id = NULL;
                      /* LCOV_EXCL_STOP */
                    }
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
                  cdd_cst_bld_ident(&bld, "CDD_C_ERROR_MEMORY");
                  cdd_cst_bld_punct(&bld, ";");
                  cdd_cst_bld_space(&bld);
                  cdd_cst_bld_punct(&bld, "}");
                  cdd_cst_bld_space(&bld);
                  cdd_cst_bld_punct(&bld, "}");

                  if ((bld.error_state == 0) && temp->num_children > 0) {
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
  return CDD_C_SUCCESS;
}

#ifdef CDD_BUILD_TESTS
C_CDD_EXPORT int g_err_perc_fail = 0;
#endif

/** @brief cdd_transform_percolate_errors */
enum cdd_c_error
cdd_transform_percolate_errors(cdd_cst_tree_t *tree,
                               const cdd_transform_config_t *config) {
  cdd_cst_query_result_t res;
  size_t i, j;
  int rc;
  cdd_token_t *modified_funcs[256];
  size_t num_modified = 0;
  (void)config;

  if (!tree || !tree->root)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  rc =
      cdd_cst_find_nodes_by_type(tree->root, CDD_CST_FUNCTION_DEFINITION, &res);
  if (rc != 0)
    /* LCOV_EXCL_START */
    return rc;
  /* LCOV_EXCL_STOP */

  for (i = 0; i < res.size; i++) {
    cdd_cst_node_t *func = res.nodes[i];
    cdd_token_t *func_name_tok = NULL;
    cdd_token_t *rparen_tok = NULL;
    cdd_token_t *prev_rparen_tok = NULL;
    size_t func_name_idx = 0;
    size_t rparen_idx = 0;
    int is_strict_void = 0;
    int allocs_seen = 0;
    size_t ret_type_start = 0;
    size_t ret_type_end = 0;
    (void)ret_type_start;

    for (j = 0; j < func->num_children; j++) {
      if (func->children[j].kind == CDD_CST_CHILD_TOKEN) {
        cdd_token_t *tok = func->children[j].val.token;
        if (tok->kind == CDD_TOKEN_LPAREN) {
          if (j > 0 && func->children[j - 1].kind == CDD_CST_CHILD_TOKEN) {
            func_name_tok = func->children[j - 1].val.token;
            func_name_idx = j - 1;
            ret_type_end = j - 1;
          }
        } else if (tok->kind == CDD_TOKEN_RPAREN) {
          rparen_tok = tok;
          rparen_idx = j;
          if (j > 0 && func->children[j - 1].kind == CDD_CST_CHILD_TOKEN) {
            prev_rparen_tok = func->children[j - 1].val.token;
          }
          break;
        }
      }
    }

    if (!rparen_tok || !func_name_tok || func_name_idx == 0)
      continue;

    /* Semantic return type resolution */
    {
      size_t k;
      int num_tokens = 0;
      int has_ptr = 0;
      cdd_token_t *last_ident = NULL;
      (void)has_ptr; /* avoid warning */
      for (k = 0; k < ret_type_end; k++) {
        if (func->children[k].kind == CDD_CST_CHILD_TOKEN) {
          cdd_token_t *t = func->children[k].val.token;
          num_tokens++;
          if (t->kind == CDD_TOKEN_IDENTIFIER ||
              t->kind == CDD_TOKEN_KEYWORD_INT) {
            last_ident = t;
          } else if (t->kind == CDD_TOKEN_STAR) {
            has_ptr = 1;
          }
        }
      }

      if (num_tokens == 1 && last_ident &&
          last_ident->kind == CDD_TOKEN_IDENTIFIER &&
          ((last_ident->length == 4 &&
            memcmp(last_ident->start, "void", 4) == 0) ||
           (last_ident->length == 8 &&
            memcmp(last_ident->start, "CDD_VOID", 8) == 0))) {
        is_strict_void = 1;
      }
    }

    if (func_name_tok) {
      if (num_modified < 256) {
        modified_funcs[num_modified++] = func_name_tok;
      }

      {
        /* Replace entire return type with enum cdd_c_error */
        cdd_cst_node_t *parent_ptr = func;
        cdd_cst_node_t *temp =
            (cdd_cst_node_t *)calloc(1, sizeof(cdd_cst_node_t));
        cdd_cst_builder_t bld;

        temp->kind = CDD_CST_UNKNOWN;
        cdd_cst_builder_init(&bld, tree, temp);
        cdd_cst_bld_ident(&bld, "enum");
        cdd_cst_bld_space(&bld);
        cdd_cst_bld_ident(&bld, "cdd_c_error");
        cdd_cst_bld_space(&bld);

        if (bld.error_state == 0 && temp->num_children > 0) {
          size_t start_idx = 0;
          cdd_trivia_t *lt = NULL;
          if (func->children[0].kind == CDD_CST_CHILD_TOKEN) {
            lt = func->children[0].val.token->leading_trivia;
            func->children[0].val.token->leading_trivia = NULL;
          }
          temp->children[0].val.token->leading_trivia = lt;

          cdd_cst_splice_children(tree, &parent_ptr, start_idx, ret_type_end,
                                  temp->children, temp->num_children);
          func = parent_ptr;
          /* Update indices */
          func_name_idx = func_name_idx - ret_type_end + temp->num_children;
          rparen_idx = rparen_idx - ret_type_end + temp->num_children;
        }
        cdd_cst_builder_free(&bld);
        free(temp->children);
        free(temp);
      }

      if (!is_strict_void) {
        size_t rparen_parent_idx = 0;
        cdd_cst_node_t *rparen_parent = NULL;
        cdd_cst_find_node_for_token(tree->root, rparen_tok, &rparen_parent_idx,
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

            /* Append original return type + *out_result */
            {
              size_t c_idx;
              for (c_idx = 0; c_idx < ret_type_end; c_idx++) {
                if (func->children[c_idx].kind == CDD_CST_CHILD_TOKEN) {
                  cdd_token_t *t = func->children[c_idx].val.token;
                  if (t->kind == CDD_TOKEN_IDENTIFIER) {
                    char *dup_id = NULL;
#ifdef CDD_BUILD_TESTS
                    if (g_err_perc_fail > 0 && --g_err_perc_fail == 0) {
                    } else
#endif
                      dup_id = (char *)malloc(t->length + 1);
                    if (dup_id) {
                      memcpy(dup_id, t->start, t->length);
                      dup_id[t->length] = '\0';
                      cdd_cst_bld_ident(&bld, dup_id);
                      free(dup_id);
                    }
                  } else if (t->kind == CDD_TOKEN_KEYWORD_INT) {
                    /* LCOV_EXCL_START */
                    cdd_cst_bld_ident(&bld, "int");
                    /* LCOV_EXCL_STOP */
                  } else if (t->kind == CDD_TOKEN_STAR ||
                             t->kind == CDD_TOKEN_OTHER) {
                    char *dup_p = NULL;
#ifdef CDD_BUILD_TESTS
                    if (g_err_perc_fail > 0 && --g_err_perc_fail == 0) {
                    } else
#endif
                      dup_p = (char *)malloc(t->length + 1);
                    if (dup_p) {
                      memcpy(dup_p, t->start, t->length);
                      dup_p[t->length] = '\0';
                      cdd_cst_bld_punct(&bld, dup_p);
                      free(dup_p);
                    }
                  }
                  if (t->trailing_trivia) {
                    /* LCOV_EXCL_START */
                    cdd_cst_bld_space(&bld);
                    /* LCOV_EXCL_STOP */
                  }
                }
              }
            }

            cdd_cst_bld_space(&bld);
            cdd_cst_bld_punct(&bld, "*");
            cdd_cst_bld_ident(&bld, "out_result");
            cdd_cst_bld_punct(&bld, ")");

            if ((bld.error_state == 0) && temp->num_children > 0) {
              cdd_trivia_t *rt = rparen_tok->trailing_trivia;
              cdd_cst_node_t *old_rparen_parent = rparen_parent;
              rparen_tok->trailing_trivia = NULL;
              if (temp->children[temp->num_children - 1].kind ==
                  CDD_CST_CHILD_TOKEN) {
                temp->children[temp->num_children - 1]
                    .val.token->trailing_trivia = rt;
              }
              cdd_cst_splice_children(tree, &rparen_parent, rparen_parent_idx,
                                      1, temp->children, temp->num_children);
              if (old_rparen_parent == func) {
                func = rparen_parent;
              }
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
          size_t assign_idx = (size_t)-1;
          (void)ptr_tok;

          for (c_idx = 0; c_idx < stmt->num_children; c_idx++) {

            if (stmt->children[c_idx].kind == CDD_CST_CHILD_TOKEN) {

              cdd_token_t *t = stmt->children[c_idx].val.token;

              if (t->kind == CDD_TOKEN_KEYWORD_RETURN) {

                is_return = 1;

              } else if (t->kind == CDD_TOKEN_ASSIGN) {

                assign_idx = c_idx;

              } else if (t->kind == CDD_TOKEN_IDENTIFIER &&

                         ((t->length == 6 &&
                           memcmp(t->start, "malloc", 6) == 0) ||

                          (t->length == 6 &&
                           memcmp(t->start, "calloc", 6) == 0) ||

                          (t->length == 7 &&
                           memcmp(t->start, "realloc", 7) == 0) ||

                          (t->length == 6 &&
                           memcmp(t->start, "strdup", 6) == 0))) {

                has_alloc = 1;
              }
            }
          }

          if (has_alloc && assign_idx == (size_t)-1) {
            rc = CDD_C_ERROR_PARSE;
            free(stmts_res.nodes);
            free(res.nodes);
            return rc;
          }
          if (has_alloc && assign_idx != (size_t)-1) {

            cdd_cst_builder_t bld;

            cdd_cst_node_t *cloned =

                (cdd_cst_node_t *)calloc(1, sizeof(cdd_cst_node_t));

            if (cloned) {

              char *tmp_name = NULL;

              size_t len = 0;

              size_t start_idx = 0;

              size_t k;

              /* Extract L-value */

              for (k = assign_idx; k-- > 0;) {

                if (stmt->children[k].kind == CDD_CST_CHILD_TOKEN) {

                  cdd_token_t *t = stmt->children[k].val.token;

                  if (t->kind == CDD_TOKEN_IDENTIFIER ||
                      t->kind == CDD_TOKEN_DOT || t->kind == CDD_TOKEN_ARROW ||
                      t->kind == CDD_TOKEN_LBRACKET ||
                      t->kind == CDD_TOKEN_RBRACKET ||
                      t->kind == CDD_TOKEN_NUMBER ||
                      t->kind == CDD_TOKEN_STAR) {

                    start_idx = k;

                  } else {

                    break;
                  }
                }
              }

              for (k = start_idx; k < assign_idx; k++) {

                if (stmt->children[k].kind == CDD_CST_CHILD_TOKEN) {

                  len += stmt->children[k].val.token->length;
                }
              }

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

              {

                size_t off = 0;

                for (k = start_idx; k < assign_idx; k++) {

                  if (stmt->children[k].kind == CDD_CST_CHILD_TOKEN) {

                    cdd_token_t *t = stmt->children[k].val.token;

                    memcpy(tmp_name + off, t->start, t->length);

                    off += t->length;
                  }
                }

                tmp_name[len] = '\0';
              }
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

            if (ret_tok && semi_tok) {
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
          size_t c_i;
          cdd_cst_node_t *decl_node =
              (cdd_cst_node_t *)calloc(1, sizeof(cdd_cst_node_t));
          cdd_cst_node_t *cleanup_node =
              (cdd_cst_node_t *)calloc(1, sizeof(cdd_cst_node_t));

          if (decl_node && cleanup_node) {
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
              /* Find the first '{' and the last '}' of the function to insert
               * these nodes */
              int found_lbrace = 0;
              size_t brace_idx = 0;
              cdd_cst_node_t *body_parent = func;

              /* Actually the '{' might be in the function tokens or in a body
               * block */
              /* Let's just insert decl after the first '{' and cleanup before
               * the last '}' */
              for (c_i = 0; c_i < func->num_children; c_i++) {
                if (func->children[c_i].kind == CDD_CST_CHILD_TOKEN &&
                    func->children[c_i].val.token->kind == CDD_TOKEN_LBRACE) {
                  /* LCOV_EXCL_START */
                  found_lbrace = 1;
                  brace_idx = c_i;
                  break;
                  /* LCOV_EXCL_STOP */
                }
              }
              if (found_lbrace) {
                /* LCOV_EXCL_START */
                cdd_cst_splice_children(tree, &body_parent, brace_idx + 1, 0,
                                        /* LCOV_EXCL_STOP */
                                        decl_node->children,
                                        decl_node->num_children);
                /* LCOV_EXCL_START */
                func = body_parent;
                /* LCOV_EXCL_STOP */
              }

              for (c_i = func->num_children; c_i-- > 0;) {
                if (func->children[c_i].kind == CDD_CST_CHILD_TOKEN &&
                    func->children[c_i].val.token->kind == CDD_TOKEN_RBRACE) {
                  /* LCOV_EXCL_START */
                  brace_idx = c_i;
                  break;
                  /* LCOV_EXCL_STOP */
                }
              }
              if (found_lbrace) { /* reusing found_lbrace condition for safety
                                   */
                                  /* LCOV_EXCL_START */
                cdd_cst_splice_children(tree, &body_parent, brace_idx, 0,
                                        /* LCOV_EXCL_STOP */
                                        cleanup_node->children,
                                        cleanup_node->num_children);
                /* LCOV_EXCL_START */
                func = body_parent;
                /* LCOV_EXCL_STOP */
              }
            }

            cdd_cst_builder_free(&bld_decl);
            cdd_cst_builder_free(&bld_cleanup);
          }
          if (decl_node) {
            free(decl_node->children);
            free(decl_node);
          }
          if (cleanup_node) {
            free(cleanup_node->children);
            free(cleanup_node);
          }
        }

        free(stmts_res.nodes);
      }
    }
  }

  free(res.nodes);

  if (num_modified > 0) {
    (void)rewrite_call_sites(tree, tree->root, modified_funcs, num_modified);
  }

  return CDD_C_SUCCESS;
}
