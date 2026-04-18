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

int cdd_transform_extern_c(cdd_cst_tree_t *tree,
                           const cdd_transform_config_t *config) {
  int rc;
  cdd_cst_query_result_t res;
  size_t i;
  int rc;
  int found_cpp = 0;
  cdd_cst_node_t *insert_after_node = NULL;
  (void)config;

  if (!tree || !tree->root) {
    rc = EINVAL;
    return rc;
  }

  /* 1. Check if __cplusplus is already checked */
  rc = cdd_cst_find_nodes_by_type(tree->root, CDD_CST_PREPROC_DIRECTIVE, &res);
  if (rc == 0) {
    for (i = 0; i < res.size; i++) {
      cdd_cst_node_t *dir = res.nodes[i];
      if (dir->num_children > 0 &&
          dir->children[0].kind == CDD_CST_CHILD_TOKEN) {
        cdd_token_t *tok = dir->children[0].val.token;
        if (tok->kind == CDD_TOKEN_PREPROC_IFDEF) {
          if (dir->num_children > 1 &&
              dir->children[1].kind == CDD_CST_CHILD_TOKEN) {
            cdd_token_t *ident = dir->children[1].val.token;
            if (ident->length == 11 &&
                memcmp(ident->start, "__cplusplus", 11) == 0) {
              found_cpp = 1;
              break;
            }
          }
        }
      }
    }
    free(res.nodes);
  }

  if (found_cpp)
    return 0;

  /* 2. Find insertion boundary: After #includes and comments, before C
   * declarations */
  for (i = 0; i < tree->root->num_children; i++) {
    cdd_cst_node_t *child = tree->root->children[i].val.node;
    if (child->kind == CDD_CST_PREPROC_DIRECTIVE) {
      if (child->num_children > 0 &&
          child->children[0].kind == CDD_CST_CHILD_TOKEN) {
        cdd_token_t *tok = child->children[0].val.token;
        if (tok->kind == CDD_TOKEN_PREPROC_INCLUDE) {
          insert_after_node = child;
        }
      }
    } else if (child->kind == CDD_CST_DECLARATION ||
               child->kind == CDD_CST_FUNCTION_DEFINITION) {
      break;
    }
  }

  /* 3. Synthesize Top Nodes */
  {
    cdd_cst_tree_t *top_tree = NULL;
    const char *top_str =
        "\n#ifdef __cplusplus\nextern \"C\" {\n#endif /* __cplusplus */\n";
    if (cdd_cst_parse(az_span_create_from_str((char *)top_str), &top_tree) ==
        0) {
      if (top_tree->root->num_children > 0) {
        cdd_cst_node_t *cloned = NULL;
        /* Insert all children of the parsed fragment */
        if (cdd_cst_node_clone(tree, top_tree->root->children[0].val.node,
                               &cloned) == 0) {
          if (insert_after_node) {
            cdd_cst_node_insert_after(insert_after_node, cloned);
          } else if (tree->root->num_children > 0) {
            cdd_cst_node_insert_before(tree->root->children[0].val.node,
                                       cloned);
          }
        }
      }
      cdd_cst_tree_free(top_tree);
    }
  }

  /* 4. Synthesize Bottom Nodes */
  {
    cdd_cst_tree_t *bot_tree = NULL;
    const char *bot_str = "\n#ifdef __cplusplus\n}\n#endif /* __cplusplus */\n";
    if (cdd_cst_parse(az_span_create_from_str((char *)bot_str), &bot_tree) ==
        0) {
      if (bot_tree->root->num_children > 0) {
        cdd_cst_node_t *cloned = NULL;
        if (cdd_cst_node_clone(tree, bot_tree->root->children[0].val.node,
                               &cloned) == 0) {
          if (tree->root->num_children > 0) {
            cdd_cst_node_t *last_node =
                tree->root->children[tree->root->num_children - 1].val.node;
            cdd_cst_node_insert_after(last_node, cloned);
          }
        }
      }
      cdd_cst_tree_free(bot_tree);
    }
  }

  return 0;
}

int cdd_transform_msvc(cdd_cst_tree_t *tree,
                       const cdd_transform_config_t *config) {
  cdd_cst_query_result_t res;
  size_t i;
  int rc;
  (void)config;

  if (!tree || !tree->root) {
    rc = EINVAL;
    return rc;
  }

  /* 1. Wrap unistd.h and sys/time.h */
  rc = cdd_cst_find_nodes_by_type(tree->root, CDD_CST_PREPROC_DIRECTIVE, &res);
  if (rc == 0) {
    for (i = 0; i < res.size; i++) {
      cdd_cst_node_t *dir = res.nodes[i];
      if (dir->num_children > 0 &&
          dir->children[0].kind == CDD_CST_CHILD_TOKEN) {
        cdd_token_t *tok = dir->children[0].val.token;
        if (tok->kind == CDD_TOKEN_PREPROC_INCLUDE && dir->num_children > 1 &&
            dir->children[1].kind == CDD_CST_CHILD_TOKEN) {
          cdd_token_t *inc_tok = dir->children[1].val.token;
          if (inc_tok->length == 10 &&
              memcmp(inc_tok->start, "<unistd.h>", 10) == 0) {
            cdd_cst_tree_t *wrap_tree = NULL;
            if (cdd_cst_parse(
                    az_span_create_from_str("#ifndef _MSC_VER\n#include "
                                            "<unistd.h>\n#else\n#include "
                                            "\"win_compat_sym.h\"\n#endif\n"),
                    &wrap_tree) == 0) {
              if (wrap_tree->root->num_children > 0) {
                cdd_cst_node_t *cloned = NULL;
                rc = cdd_cst_node_clone(tree,
                                       wrap_tree->root->children[0].val.node,
                                       &cloned);
                if (rc == 0) {
                  rc = cdd_cst_node_replace(tree, dir, cloned);
                  if (rc != 0) {
                    /* Handle error */
                    fprintf(stderr, "Error replacing node: %d\n", rc);
                  }
                } else {
                    fprintf(stderr, "Error cloning node: %d\n", rc);
                }
              }
              cdd_cst_tree_free(wrap_tree);
            }
          } else if (inc_tok->length == 12 &&
                     memcmp(inc_tok->start, "<sys/time.h>", 12) == 0) {
            cdd_cst_tree_t *wrap_tree = NULL;
            if (cdd_cst_parse(
                    az_span_create_from_str("#ifndef _MSC_VER\n#include "
                                            "<sys/time.h>\n#else\n#include "
                                            "\"win_compat_sym.h\"\n#endif\n"),
                    &wrap_tree) == 0) {
              if (wrap_tree->root->num_children > 0) {
                cdd_cst_node_t *cloned = NULL;
                rc = cdd_cst_node_clone(tree,
                                       wrap_tree->root->children[0].val.node,
                                       &cloned);
                if (rc == 0) {
                  rc = cdd_cst_node_replace(tree, dir, cloned);
                  if (rc != 0) {
                    /* Handle error */
                    fprintf(stderr, "Error replacing node: %d\n", rc);
                  }
                } else {
                    fprintf(stderr, "Error cloning node: %d\n", rc);
                }
              }
              cdd_cst_tree_free(wrap_tree);
            }
          }
        }
      }
    }
    free(res.nodes);
  }

  /* 2. Traverse tokens and replace POSIX identifiers directly */
  for (i = 0; i < tree->base_tokens->size; i++) {
    cdd_token_t *t = &tree->base_tokens->tokens[i];
    if (t->kind == CDD_TOKEN_IDENTIFIER) {
      if (t->length == 10 && memcmp(t->start, "strcasecmp", 10) == 0) {
        t->start = (const uint8_t *)"_stricmp";
        t->length = 8;
      } else if (t->length == 11 && memcmp(t->start, "strncasecmp", 11) == 0) {
        t->start = (const uint8_t *)"_strnicmp";
        t->length = 9;
      } else if (t->length == 6 && memcmp(t->start, "strdup", 6) == 0) {
        t->start = (const uint8_t *)"_strdup";
        t->length = 7;
      } else if (t->length == 7 && memcmp(t->start, "ssize_t", 7) == 0) {
        t->start = (const uint8_t *)"SSIZE_T";
        t->length = 7;
      } else if (t->length == 16 &&
                 memcmp(t->start, "__builtin_expect", 16) == 0) {
        t->start = (const uint8_t *)"cdd_builtin_expect";
        t->length = 18;
      }
    }
  }

  return 0;
}

int cdd_transform_gnu(cdd_cst_tree_t *tree,
                      const cdd_transform_config_t *config) {
  size_t i;
  (void)config;

  if (!tree || !tree->root) {
    rc = EINVAL;
    return rc;
  }

  for (i = 0; i < tree->base_tokens->size; i++) {
    cdd_token_t *tok = &tree->base_tokens->tokens[i];
    if (tok->kind == CDD_TOKEN_IDENTIFIER && tok->length == 13 &&
        memcmp(tok->start, "__attribute__", 13) == 0) {
      if (i + 5 < tree->base_tokens->size &&
          tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_LPAREN &&
          tree->base_tokens->tokens[i + 2].kind == CDD_TOKEN_LPAREN &&
          tree->base_tokens->tokens[i + 4].kind == CDD_TOKEN_RPAREN &&
          tree->base_tokens->tokens[i + 5].kind == CDD_TOKEN_RPAREN) {

        cdd_token_t *attr = &tree->base_tokens->tokens[i + 3];
        if (attr->length == 8 && memcmp(attr->start, "noreturn", 8) == 0) {
          tok->start = (const uint8_t *)"_Noreturn";
          tok->length = 9;
          tree->base_tokens->tokens[i + 1].length = 0;
          tree->base_tokens->tokens[i + 2].length = 0;
          tree->base_tokens->tokens[i + 3].length = 0;
          tree->base_tokens->tokens[i + 4].length = 0;
          tree->base_tokens->tokens[i + 5].length = 0;
        } else if (attr->length == 6 && memcmp(attr->start, "unused", 6) == 0) {
          tok->start = (const uint8_t *)"/* unused */";
          tok->length = 12;
          tree->base_tokens->tokens[i + 1].length = 0;
          tree->base_tokens->tokens[i + 2].length = 0;
          tree->base_tokens->tokens[i + 3].length = 0;
          tree->base_tokens->tokens[i + 4].length = 0;
          tree->base_tokens->tokens[i + 5].length = 0;
        } else if (attr->length == 6 && memcmp(attr->start, "packed", 6) == 0) {
          tok->start = (const uint8_t *)"\n#pragma pack(push, 1)\n";
          tok->length = 23;
          tree->base_tokens->tokens[i + 1].length = 0;
          tree->base_tokens->tokens[i + 2].length = 0;
          tree->base_tokens->tokens[i + 3].length = 0;
          tree->base_tokens->tokens[i + 4].length = 0;
          tree->base_tokens->tokens[i + 5].length = 0;

          /* scan forward for closing brace and semicolon */
          {
            size_t k;
            int depth = 0;
            for (k = i; k < tree->base_tokens->size; k++) {
              if (tree->base_tokens->tokens[k].kind == CDD_TOKEN_LBRACE)
                depth++;
              else if (tree->base_tokens->tokens[k].kind == CDD_TOKEN_RBRACE)
                depth--;
              else if (depth == 0 && tree->base_tokens->tokens[k].kind ==
                                         CDD_TOKEN_SEMICOLON) {
                tree->base_tokens->tokens[k].start =
                    (const uint8_t *)";\n#pragma pack(pop)\n";
                tree->base_tokens->tokens[k].length = 19;
                break;
              }
            }
          }
        }
      }
    } else if (tok->kind == CDD_TOKEN_IDENTIFIER) {
      if (tok->length == 13 && memcmp(tok->start, "__extension__", 13) == 0) {
        tok->start = (const uint8_t *)"";
        tok->length = 0;
      } else if (tok->length == 11 &&
                 memcmp(tok->start, "__alignof__", 11) == 0) {
        tok->start = (const uint8_t *)"_Alignof";
        tok->length = 8;
      }
    }
  }

  /* Unroll statement expressions and VLAs */
  for (i = 0; i < tree->base_tokens->size; i++) {
    cdd_token_t *t = &tree->base_tokens->tokens[i];
    if (t->kind == CDD_TOKEN_LPAREN && i + 1 < tree->base_tokens->size &&
        tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_LBRACE) {
      /* Remove ({ */
      t->start = (const uint8_t *)"";
      t->length = 0;
      tree->base_tokens->tokens[i + 1].start = (const uint8_t *)"";
      tree->base_tokens->tokens[i + 1].length = 0;
    } else if (t->kind == CDD_TOKEN_RBRACE && i + 1 < tree->base_tokens->size &&
               tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_RPAREN) {
      /* Remove }) */
      t->start = (const uint8_t *)"";
      t->length = 0;
      tree->base_tokens->tokens[i + 1].start = (const uint8_t *)"";
      tree->base_tokens->tokens[i + 1].length = 0;
    } else if (t->kind == CDD_TOKEN_LBRACKET && i > 0) {
      cdd_token_t *prev = &tree->base_tokens->tokens[i - 1];
      cdd_token_t *next = &tree->base_tokens->tokens[i + 1];
      cdd_token_t *semi = &tree->base_tokens->tokens[i + 2];
      if (prev->kind == CDD_TOKEN_IDENTIFIER &&
          next->kind == CDD_TOKEN_IDENTIFIER &&
          semi->kind == CDD_TOKEN_RBRACKET) {
        /* This is an array declaration, possibly VLA. Let's see if it ends with
         * semicolon */
        size_t k;
        cdd_token_t *semi_tok = NULL;
        for (k = i + 2; k < tree->base_tokens->size; k++) {
          if (tree->base_tokens->tokens[k].kind == CDD_TOKEN_SEMICOLON) {
            semi_tok = &tree->base_tokens->tokens[k];
            break;
          }
        }
        if (semi_tok) {
          /* Found VLA pattern `int arr[n];` */
          /* Find type specifier before identifier */
          if (i >= 2) {
            char buf[256];
            char *heap_buf;
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
            sprintf_s(buf, sizeof(buf), "*%.*s = alloca(%.*s * sizeof(*%.*s));",
                      (int)prev->length, prev->start, (int)next->length,
                      next->start, (int)prev->length, prev->start);
#else
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
            sprintf_s(buf, sizeof(buf),
                      "*%.*s = alloca(%.*s * sizeof(*%.*s));
#else
            sprintf(buf,
                    "*%.*s = alloca(%.*s * sizeof(*%.*s));
#endif ",
                      (int)prev->length,
                      prev->start, (int)next->length, next->start,
                      (int)prev->length, prev->start);
#endif
            heap_buf = strdup(buf);
            prev->start = (const uint8_t *)heap_buf;
            prev->length = strlen(heap_buf);
            t->length = 0;
            next->length = 0;
            semi->length = 0;
          }
        }
      }
    } else if (t->kind == CDD_TOKEN_LBRACE && i + 1 < tree->base_tokens->size &&
               tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_RBRACE) {
      tree->base_tokens->tokens[i + 1].start = (const uint8_t *)" char _pad; }";
      tree->base_tokens->tokens[i + 1].length = 13;
    }
  }

  return 0;
}

int cdd_transform_percolate_errors(cdd_cst_tree_t *tree,
                                   const cdd_transform_config_t *config) {
  cdd_cst_query_result_t res;
  size_t i, j;
  int rc;
  char *modified_funcs[256];
  size_t num_modified = 0;
  (void)config;

  if (!tree || !tree->root) {
    rc = EINVAL;
    return rc;
  }

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
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
        sprintf_s(mbuf, sizeof(mbuf), "%.*s", (int)func_name_tok->length,
                  func_name_tok->start);
#else
        sprintf(mbuf, "%.*s", (int)func_name_tok->length, func_name_tok->start);
#endif
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
