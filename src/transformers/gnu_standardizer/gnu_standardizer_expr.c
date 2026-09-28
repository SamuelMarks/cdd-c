/**
 * @file gnu_standardizer_expr.c
 * @brief Expression and compound literal standardization pass for GNU
 * standardizer.
 */

/* clang-format off */
#include "gnu_standardizer_internal.h"
/* clang-format on */

cdd_c_error_t gnu_standardize_expr(cdd_cst_tree_t *tree) {
  size_t i;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  for (i = 0; i < tree->base_tokens->size; i++) {
    cdd_token_t *tok = &tree->base_tokens->tokens[i];
    if (tok->kind == CDD_TOKEN_IDENTIFIER) {
      if (tok->length == 13 && memcmp(tok->start, "__extension__", 13) == 0) {
        rc = replace_token_with_text(tree, tok, CDD_TOKEN_IDENTIFIER, "", 0);
        if (rc != CDD_C_SUCCESS)
          return rc;
      } else if (tok->length == 17 &&
                 memcmp(tok->start, "__builtin_shuffle", 17) == 0) {
        rc = replace_token_with_text(tree, tok, CDD_TOKEN_IDENTIFIER,
                                     "cdd_builtin_shuffle", 19);
        if (rc != CDD_C_SUCCESS)
          return rc;
      } else if (tok->length == 23 &&
                 memcmp(tok->start, "__builtin_shufflevector", 23) == 0) {
        rc = replace_token_with_text(tree, tok, CDD_TOKEN_IDENTIFIER,
                                     "cdd_builtin_shufflevector", 25);
        if (rc != CDD_C_SUCCESS)
          return rc;
      } else if ((tok->length == 11 &&
                  memcmp(tok->start, "__alignof__", 11) == 0) ||
                 (tok->length == 9 &&
                  memcmp(tok->start, "__alignof", 9) == 0)) {
        /* If it's applied to an expression, rewrite to _Alignof(typeof(expr))?
           For now we just map to _Alignof.
           In a full AST, we would check if the child is an expression or type.
         */
        rc = replace_token_with_text(tree, tok, CDD_TOKEN_IDENTIFIER,
                                     "_Alignof", 8);
        if (rc != CDD_C_SUCCESS)
          return rc;
      } else if (tok->length == 5 && memcmp(tok->start, "union", 5) == 0) {
        if (i + 1 < tree->base_tokens->size &&
            tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_LBRACE) {
          /* Anonymous union: GNU extension. Inject dummy name. */
          static int anon_counter = 0;
          size_t child_idx;
          cdd_cst_node_t *owning_node = NULL;
          rc = gnu_find_node(tree->root, tok, &child_idx, &owning_node);
          if (rc != CDD_C_SUCCESS)
            return rc;
          {
            cdd_cst_node_t *temp = NULL;
            cdd_cst_builder_t bld;
            rc = gnu_alloc_node(CDD_CST_UNKNOWN, &temp);
            if (rc != CDD_C_SUCCESS)
              return rc;
            cdd_cst_builder_init(&bld, tree, temp);
            cdd_cst_bld_ident(&bld, "union");
            cdd_cst_bld_space(&bld);
            {
              char tb2[128];
              CDD_SNPRINTF(tb2, 128, "_cdd_anon_%d", anon_counter++);
              cdd_cst_bld_ident(&bld, pool_string_safe(tree, tb2));
            }
            if (bld.error_state != 0) {
              cdd_cst_builder_free(&bld);
              cdd_cst_free_node_only(temp);
              return CDD_C_ERROR_MEMORY;
            }
            cdd_cst_splice_children(tree, &owning_node, child_idx, 1,
                                    temp->children, temp->num_children);
            cdd_cst_builder_free(&bld);
            cdd_cst_free_node_only(temp);
          }
        }
      }
    } else if (tok->kind == CDD_TOKEN_KEYWORD_STRUCT) {
      if (i + 1 < tree->base_tokens->size &&
          tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_LBRACE) {
        /* Anonymous struct: GNU extension. Inject dummy name to make C89 happy.
         */
        static int anon_counter = 0;
        size_t child_idx;
        cdd_cst_node_t *owning_node = NULL;
        rc = gnu_find_node(tree->root, tok, &child_idx, &owning_node);
        if (rc != CDD_C_SUCCESS)
          return rc;
        {
          cdd_cst_node_t *temp = NULL;
          cdd_cst_builder_t bld;
          rc = gnu_alloc_node(CDD_CST_UNKNOWN, &temp);
          if (rc != CDD_C_SUCCESS)
            return rc;
          cdd_cst_builder_init(&bld, tree, temp);
          cdd_cst_bld_ident(&bld, "struct");
          cdd_cst_bld_space(&bld);
          {
            char tb2[128];
            CDD_SNPRINTF(tb2, 128, "_cdd_anon_%d", anon_counter++);
            cdd_cst_bld_ident(&bld, pool_string_safe(tree, tb2));
          }
          if (bld.error_state != 0) {
            cdd_cst_builder_free(&bld);
            cdd_cst_free_node_only(temp);
            return CDD_C_ERROR_MEMORY;
          }
          cdd_cst_splice_children(tree, &owning_node, child_idx, 1,
                                  temp->children, temp->num_children);
          cdd_cst_builder_free(&bld);
          cdd_cst_free_node_only(temp);
        }
      }
    } else if (tok->kind == CDD_TOKEN_LPAREN && tok->length > 0) {
      size_t k;
      int is_cast_lvalue = 0;
      size_t rparen_idx = 0;
      for (k = i + 1; k < tree->base_tokens->size; k++) {
        if (tree->base_tokens->tokens[k].kind == CDD_TOKEN_RPAREN) {
          rparen_idx = k;
          break;
        } else if (tree->base_tokens->tokens[k].kind == CDD_TOKEN_SEMICOLON ||
                   tree->base_tokens->tokens[k].kind == CDD_TOKEN_LBRACE ||
                   tree->base_tokens->tokens[k].kind == CDD_TOKEN_RBRACE) {
          break;
        }
      }
      if (rparen_idx > i + 1 && rparen_idx + 2 < tree->base_tokens->size) {
        if (tree->base_tokens->tokens[rparen_idx + 1].kind ==
                CDD_TOKEN_IDENTIFIER &&
            tree->base_tokens->tokens[rparen_idx + 2].kind ==
                CDD_TOKEN_ASSIGN) {
          is_cast_lvalue = 1;
          if (i > 0 && tree->base_tokens->tokens[i - 1].kind ==
                           CDD_TOKEN_KEYWORD_TYPEOF) {
            is_cast_lvalue = 0;
          }
          /* Check if it's the `(union U)val = ` pattern */
          if (is_cast_lvalue &&
              tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_IDENTIFIER &&
              tree->base_tokens->tokens[i + 1].length == 5 &&
              memcmp(tree->base_tokens->tokens[i + 1].start, "union", 5) == 0) {
            is_cast_lvalue = 2;
          }
        }
      }

      if (is_cast_lvalue == 2) {
        rc = replace_token_with_text(tree,
                                     &tree->base_tokens->tokens[rparen_idx],
                                     CDD_TOKEN_RPAREN, "){", 2);
        if (rc != CDD_C_SUCCESS)
          return rc;

        rc = replace_token_with_text(
            tree, &tree->base_tokens->tokens[rparen_idx + 1],
            tree->base_tokens->tokens[rparen_idx + 1].kind, "}", 1);
        if (rc != CDD_C_SUCCESS)
          return rc;
      } else if (is_cast_lvalue == 1) {
        size_t val_len = tree->base_tokens->tokens[rparen_idx + 1].length;
        char *buf;
        const char *pooled;

        rc = replace_token_with_text(tree, tok, CDD_TOKEN_LPAREN, "*(", 2);
        if (rc != CDD_C_SUCCESS)
          return rc;

        rc = replace_token_with_text(tree,
                                     &tree->base_tokens->tokens[rparen_idx],
                                     CDD_TOKEN_RPAREN, "*)", 2);
        if (rc != CDD_C_SUCCESS)
          return rc;

        rc = gnu_malloc(val_len + 2, (void **)&buf);
        if (rc != CDD_C_SUCCESS)
          return rc;
        buf[0] = '&';
        memcpy(buf + 1, tree->base_tokens->tokens[rparen_idx + 1].start,
               val_len);
        buf[val_len + 1] = '\0';
        pooled = pool_string_safe(tree, buf);
        free(buf);
        if (!pooled)
          return CDD_C_ERROR_MEMORY;
        rc = replace_token_with_text(
            tree, &tree->base_tokens->tokens[rparen_idx + 1],
            tree->base_tokens->tokens[rparen_idx + 1].kind, pooled,
            val_len + 1);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
    } else if (tok->kind == CDD_TOKEN_OTHER && tok->start[0] == '?') {
      if (i + 1 < tree->base_tokens->size &&
          tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_COLON) {
        /* `x ? : y` -> `x ? x : y` */
        size_t k;
        size_t lhs_start = i;
        int depth = 0;
        char *buf;
        size_t len = 0;
        char *p;
        for (k = i; k-- > 0;) {
          int k_kind = (int)tree->base_tokens->tokens[k].kind;
          if (k_kind == CDD_TOKEN_RPAREN || k_kind == CDD_TOKEN_RBRACKET ||
              k_kind == CDD_TOKEN_RBRACE) {
            depth++;
          } else if (k_kind == CDD_TOKEN_LPAREN ||
                     k_kind == CDD_TOKEN_LBRACKET ||
                     k_kind == CDD_TOKEN_LBRACE) {
            depth--;
          }
          if (depth == 0) {
            if (k_kind == CDD_TOKEN_ASSIGN || k_kind == CDD_TOKEN_COMMA ||
                k_kind == CDD_TOKEN_SEMICOLON ||
                k_kind == CDD_TOKEN_KEYWORD_RETURN ||
                k_kind == CDD_TOKEN_COLON) {
              lhs_start = k + 1;
              break;
            } else if (k_kind == CDD_TOKEN_OTHER) {
              if (tree->base_tokens->tokens[k].start[0] == '?') {
                lhs_start = k + 1;
                break;
              }
            } else if (k_kind == CDD_TOKEN_IDENTIFIER) {
              if ((tree->base_tokens->tokens[k].length == 4 &&
                   memcmp(tree->base_tokens->tokens[k].start, "case", 4) ==
                       0) ||
                  (tree->base_tokens->tokens[k].length == 5 &&
                   memcmp(tree->base_tokens->tokens[k].start, "while", 5) ==
                       0) ||
                  (tree->base_tokens->tokens[k].length == 3 &&
                   memcmp(tree->base_tokens->tokens[k].start, "for", 3) == 0) ||
                  (tree->base_tokens->tokens[k].length == 2 &&
                   memcmp(tree->base_tokens->tokens[k].start, "do", 2) == 0) ||
                  (tree->base_tokens->tokens[k].length == 6 &&
                   memcmp(tree->base_tokens->tokens[k].start, "switch", 6) ==
                       0)) {
                lhs_start = k + 1;
                break;
              }
            }
          } else if (depth < 0) {
            lhs_start = k + 1;
            break;
          }
        }
        if (
#ifdef CDD_BUILD_TESTS
            g_gnu_standardizer_fail == 19 ||
#endif
            k == (size_t)-1) {
          lhs_start = 0;
        }

        if (lhs_start < i) {
          const uint8_t *expr_start =
              tree->base_tokens->tokens[lhs_start].start;
          const uint8_t *expr_end = tree->base_tokens->tokens[i - 1].start +
                                    tree->base_tokens->tokens[i - 1].length;
          const char *pooled;
          len = (size_t)(expr_end - expr_start);
          rc = gnu_malloc(len + 4, (void **)&buf);
          if (rc != CDD_C_SUCCESS)
            return rc;
          p = buf;
          *p++ = '?';
          *p++ = ' ';
          memcpy(p, expr_start, len);
          p += len;
          *p++ = ' ';
          *p = '\0';
          pooled = pool_string_safe(tree, buf);
          free(buf);
          if (!pooled)
            return CDD_C_ERROR_MEMORY;
          rc = replace_token_with_text(tree, tok, tok->kind, pooled,
                                       strlen(pooled));
          if (rc != CDD_C_SUCCESS)
            return rc;
        }
      }

    } else if (tok->kind == CDD_TOKEN_KEYWORD_RETURN) {
      if (i + 4 < tree->base_tokens->size &&
          tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_LPAREN &&
          tree->base_tokens->tokens[i + 2].length == 4 &&
          memcmp(tree->base_tokens->tokens[i + 2].start, "void", 4) == 0 &&
          tree->base_tokens->tokens[i + 3].kind == CDD_TOKEN_RPAREN) {
        /* Rewrite `return (void)expr;` to `(void)expr; return;` */
        size_t k;
        for (k = i + 4; k < tree->base_tokens->size; k++) {
          if (tree->base_tokens->tokens[k].kind == CDD_TOKEN_SEMICOLON) {
            rc = replace_token_with_text(tree, tok, CDD_TOKEN_KEYWORD_RETURN,
                                         "", 0);
            if (rc != CDD_C_SUCCESS)
              return rc;
#ifdef CDD_BUILD_TESTS
            if (g_gnu_standardizer_fail != 6)
#endif
            {
              rc = replace_token_with_text(tree, &tree->base_tokens->tokens[k],
                                           CDD_TOKEN_SEMICOLON, "; return;", 9);
              if (rc != CDD_C_SUCCESS)
                return rc;
            }
            break;
          }
        }
      }
    }
  }
  return CDD_C_SUCCESS;
}
