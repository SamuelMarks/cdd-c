/* clang-format off */
#include "cdd_cst_transform.h"
#include "classes/parse/cdd_cst_mutate.h"
#include "classes/parse/cdd_cst_parser.h"
#include "classes/parse/cdd_cst_query.h"
#include "c_str_span.h"
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
/* clang-format on */

int cdd_transform_percolate_errors(cdd_cst_tree_t *tree,
                                   const cdd_transform_config_t *config) {
  cdd_cst_query_result_t res;
  size_t i, j;
  int rc;
  char *modified_funcs[256];
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
        char mbuf[256];
        sprintf(mbuf, "%.*s", (int)func_name_tok->length, func_name_tok->start);
        modified_funcs[num_modified++] = strdup(mbuf);
      }

      void_tok->start = (const uint8_t *)"int";
      void_tok->length = 3;

      if (prev_rparen_tok && prev_rparen_tok->kind == CDD_TOKEN_LPAREN) {
        rparen_tok->start = (const uint8_t *)"void **out_result)";
        rparen_tok->length = 18;
      } else {
        rparen_tok->start = (const uint8_t *)", void **out_result)";
        rparen_tok->length = 20;
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
            cdd_cst_tree_t *wrap_tree = NULL;
            char buf[256];
            char *heap_buf;
            cdd_cst_node_t *cloned = NULL;
            if (allocs_seen == 0) {
              sprintf(buf, "\n  if (!%.*s) { return ENOMEM; }",
                      (int)ptr_tok->length, ptr_tok->start);
            } else {
              sprintf(buf, "\n  if (!%.*s) { rc = ENOMEM; goto cleanup; }",
                      (int)ptr_tok->length, ptr_tok->start);
            }
            allocs_seen++;
            heap_buf = strdup(buf);
            if (cdd_cst_parse(az_span_create_from_str(heap_buf), &wrap_tree) ==
                0) {
              if (wrap_tree->root->num_children > 0) {
                if (cdd_cst_node_clone(tree,
                                       wrap_tree->root->children[0].val.node,
                                       &cloned) == 0) {
                  cdd_cst_node_insert_after(stmt, cloned);
                }
              }
              cdd_cst_tree_free(wrap_tree);
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
                  ret_tok->start = (const uint8_t *)"rc = 0; goto cleanup";
                  ret_tok->length = 20;
                } else {
                  ret_tok->start = (const uint8_t *)"return 0";
                  ret_tok->length = 8;
                }
              }
            }
          }
        }

        if (allocs_seen > 0) {
          cdd_cst_tree_t *wrap_tree = NULL;
          char *heap_buf = strdup("\ncleanup:\n  return rc;");
          cdd_cst_node_t *cloned = NULL;
          if (cdd_cst_parse(az_span_create_from_str(heap_buf), &wrap_tree) ==
              0) {
            if (wrap_tree->root->num_children > 0) {
              if (cdd_cst_node_clone(tree,
                                     wrap_tree->root->children[0].val.node,
                                     &cloned) == 0) {
                cdd_cst_node_t *last_stmt = stmts_res.nodes[stmts_res.size - 1];
                cdd_cst_node_insert_after(last_stmt, cloned);
              }
            }
            cdd_cst_tree_free(wrap_tree);
          }
        }

        free(stmts_res.nodes);
      }
    }
  }

  free(res.nodes);

  /* Call Site Updates */
  if (num_modified > 0) {
    for (i = 0; i < tree->base_tokens->size; i++) {
      cdd_token_t *tok = &tree->base_tokens->tokens[i];
      if (tok->kind == CDD_TOKEN_IDENTIFIER) {
        size_t m;
        for (m = 0; m < num_modified; m++) {
          if (strlen(modified_funcs[m]) == tok->length &&
              memcmp(modified_funcs[m], tok->start, tok->length) == 0) {
            /* Check if it's a function call */
            if (i + 1 < tree->base_tokens->size &&
                tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_LPAREN) {
              size_t k;
              int depth = 0;
              cdd_token_t *rparen_tok = NULL;
              cdd_token_t *prev_rparen_tok = NULL;
              int is_def = 0;
              for (k = i + 1; k-- > 0;) {
                if (tree->base_tokens->tokens[k].kind == CDD_TOKEN_IDENTIFIER &&
                    ((tree->base_tokens->tokens[k].length == 4 &&
                      memcmp(tree->base_tokens->tokens[k].start, "void", 4) ==
                          0) ||
                     (tree->base_tokens->tokens[k].length == 3 &&
                      memcmp(tree->base_tokens->tokens[k].start, "int", 3) ==
                          0))) {
                  is_def = 1;
                  break;
                } else if (tree->base_tokens->tokens[k].kind ==
                               CDD_TOKEN_SEMICOLON ||
                           tree->base_tokens->tokens[k].kind ==
                               CDD_TOKEN_RBRACE ||
                           tree->base_tokens->tokens[k].kind ==
                               CDD_TOKEN_LBRACE) {
                  break;
                }
              }

              if (!is_def) {
                for (k = i + 1; k < tree->base_tokens->size; k++) {
                  if (tree->base_tokens->tokens[k].kind == CDD_TOKEN_LPAREN) {
                    depth++;
                  } else if (tree->base_tokens->tokens[k].kind ==
                             CDD_TOKEN_RPAREN) {
                    depth--;
                    if (depth == 0) {
                      rparen_tok = &tree->base_tokens->tokens[k];
                      prev_rparen_tok = &tree->base_tokens->tokens[k - 1];
                      break;
                    }
                  }
                }

                if (rparen_tok) {
                  char buf[256];
                  char *heap_buf;
                  sprintf(buf, "void *out_%.*s;\n  if (%.*s", (int)tok->length,
                          tok->start, (int)tok->length, tok->start);
                  heap_buf = strdup(buf);
                  tok->start = (const uint8_t *)heap_buf;
                  tok->length = strlen(heap_buf);

                  if (prev_rparen_tok &&
                      prev_rparen_tok->kind == CDD_TOKEN_LPAREN) {
                    sprintf(buf, "&out_%.*s) != 0) { return ENOMEM; }",
                            (int)tok->length, modified_funcs[m]);
                  } else {
                    sprintf(buf, ", &out_%.*s) != 0) { return ENOMEM; }",
                            (int)tok->length, modified_funcs[m]);
                  }
                  heap_buf = strdup(buf);
                  rparen_tok->start = (const uint8_t *)heap_buf;
                  rparen_tok->length = strlen(heap_buf);

                  for (k = i + 1; k < tree->base_tokens->size; k++) {
                    if (tree->base_tokens->tokens[k].kind ==
                        CDD_TOKEN_SEMICOLON) {
                      tree->base_tokens->tokens[k].length = 0;
                      tree->base_tokens->tokens[k].start = (const uint8_t *)"";
                      break;
                    } else if (tree->base_tokens->tokens[k].kind ==
                               CDD_TOKEN_RBRACE) {
                      break;
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }

  for (i = 0; i < num_modified; i++) {
    free(modified_funcs[i]);
  }

  return 0;
}
