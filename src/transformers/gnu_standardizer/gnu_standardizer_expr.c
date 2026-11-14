/**
 * @file gnu_standardizer_expr.c
 * @brief Expression and compound literal standardization pass for GNU
 * standardizer.
 */

/* clang-format off */
#include "gnu_standardizer_internal.h"
/* clang-format on */

/**
 * @brief Recursively scans `sc` and its child scopes for `void *`,
 * function-pointer, or 128-bit integer symbols.
 *
 * @param[in] sc Scope subtree root.
 * @param[in,out] io_has_vptr Set to 1 if a `void *` or fn-ptr symbol exists.
 * @param[in,out] io_has_i128 Set to 1 if an `int128` symbol exists.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t gnu_scan_scope_types(const cdd_cst_scope_t *sc,
                                          int *io_has_vptr, int *io_has_i128) {
  const cdd_cst_symbol_t *sym;
  size_t i;
  cdd_c_error_t rc = CDD_C_SUCCESS;
  for (sym = sc->symbols; sym; sym = sym->next) {
    if (sym->type_str) {
      if (strstr(sym->type_str, "void *") != NULL ||
          strstr(sym->type_str, "(*)") != NULL) {
        *io_has_vptr = 1;
      }
      if (strstr(sym->type_str, "int128") != NULL) {
        *io_has_i128 = 1;
      }
    }
  }
  for (i = 0; i < sc->num_children; i++) {
    rc = (cdd_c_error_t)((int)rc |
                         (int)gnu_scan_scope_types(sc->children[i], io_has_vptr,
                                                   io_has_i128));
  }
  return rc;
}

/**
 * @brief Checks if an unnamed `struct { ... }` or `union { ... }` has no
 * declarator after its closing brace (i.e. `}` is immediately followed by `;`).
 *
 * @param[in] tokens Token list.
 * @param[in] lbrace_idx Index of `{` starting the aggregate body.
 * @param[out] out_is_unnamed_member Receives 1 if `}` is followed by `;`.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t expr_check_unnamed_member(const cdd_token_list_t *tokens,
                                               size_t lbrace_idx,
                                               int *out_is_unnamed_member) {
  size_t k;
  int depth = 0;
  *out_is_unnamed_member = 0;
  for (k = lbrace_idx; k < tokens->size; k++) {
    if (tokens->tokens[k].kind == CDD_TOKEN_LBRACE)
      depth++;
    else if (tokens->tokens[k].kind == CDD_TOKEN_RBRACE) {
      depth--;
      if (depth == 0) {
        if (k + 1 < tokens->size &&
            tokens->tokens[k + 1].kind == CDD_TOKEN_SEMICOLON) {
          *out_is_unnamed_member = 1;
        }
        return CDD_C_SUCCESS;
      }
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Prepends `#pragma warning(disable: 4201)` once at the top of a CST
 * translation unit for MSVC unnamed struct/union compatibility.
 *
 * @param[in,out] tree Target CST tree.
 * @param[in,out] gctx GNU standardizer context.
 * @return CDD_C_SUCCESS on success, or error code.
 */
static cdd_c_error_t
expr_emit_msvc_4201_once(cdd_cst_tree_t *tree,
                         struct gnu_standardizer_ctx_t *gctx) {
  cdd_cst_node_t *pragma_node = NULL;
  cdd_cst_builder_t bld;
  cdd_c_error_t rc;

  if (gctx->emitted_msvc_4201)
    return CDD_C_SUCCESS;

  rc = gnu_alloc_node(CDD_CST_PREPROC_DIRECTIVE, &pragma_node);
  if (rc != CDD_C_SUCCESS)
    return rc;

  bld.error_state = cdd_cst_builder_init(&bld, tree, pragma_node);
  bld.error_state =
      (cdd_c_error_t)((int)bld.error_state |
                      (int)cdd_cst_bld_token(&bld, CDD_TOKEN_PREPROC_PRAGMA,
                                             "#pragma warning(disable: 4201)"));
  bld.error_state =
      (cdd_c_error_t)((int)bld.error_state | (int)cdd_cst_bld_newline(&bld));
  if (bld.error_state != CDD_C_SUCCESS) {
    rc = (cdd_c_error_t)((int)CDD_C_ERROR_MEMORY |
                         (int)cdd_cst_builder_free(&bld) |
                         (int)cdd_cst_free_node_only(pragma_node));
    return rc;
  }

  rc = cdd_cst_splice_children(tree, &tree->root, 0, 0, pragma_node->children,
                               pragma_node->num_children);
  rc = (cdd_c_error_t)((int)rc | (int)cdd_cst_builder_free(&bld) |
                       (int)cdd_cst_free_node_only(pragma_node));
  if (rc != CDD_C_SUCCESS)
    return rc;

  gctx->emitted_msvc_4201 = 1;
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks if a condition slice `tokens[start..end)` contains side effects
 * (calls, `++`/`--`, or assignments) requiring temporary hoisting in `x ?: y`.
 *
 * @param[in] tokens Base token array.
 * @param[in] start Inclusive start index.
 * @param[in] end Exclusive end index.
 * @param[out] out_has_side_effect Receives 1 if non-trivial/side-effecting.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t expr_cond_has_side_effects(const cdd_token_t *tokens,
                                                size_t start, size_t end,
                                                int *out_has_side_effect) {
  size_t k;
  *out_has_side_effect = 0;
  for (k = start; k < end; k++) {
    if (tokens[k].kind == CDD_TOKEN_IDENTIFIER && k + 1 < end &&
        tokens[k + 1].kind == CDD_TOKEN_LPAREN) {
      *out_has_side_effect = 1;
      return CDD_C_SUCCESS;
    }
    if (tokens[k].kind == CDD_TOKEN_ASSIGN ||
        tokens[k].kind == CDD_TOKEN_LBRACE) {
      *out_has_side_effect = 1;
      return CDD_C_SUCCESS;
    }
    if (k + 1 < end && ((tokens[k].kind == CDD_TOKEN_PLUS &&
                         tokens[k + 1].kind == CDD_TOKEN_PLUS) ||
                        (tokens[k].kind == CDD_TOKEN_MINUS &&
                         tokens[k + 1].kind == CDD_TOKEN_MINUS))) {
      *out_has_side_effect = 1;
      return CDD_C_SUCCESS;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Locates the enclosing statement node and its index inside the nearest
 * `CDD_CST_BLOCK`.
 *
 * @param[in] node Starting CST node inside a statement.
 * @param[out] out_block Parent `CDD_CST_BLOCK` node.
 * @param[out] out_stmt_idx Child index of the statement inside `out_block`.
 * @return CDD_C_SUCCESS if inside a block, or CDD_C_ERROR_NOT_FOUND.
 */
static cdd_c_error_t expr_find_enclosing_block_stmt(cdd_cst_node_t *node,
                                                    cdd_cst_node_t **out_block,
                                                    size_t *out_stmt_idx) {
  cdd_cst_node_t *cur = node;
  while (cur->parent) {
    if (cur->parent->kind == CDD_CST_BLOCK) {
      cdd_cst_node_t *blk = cur->parent;
      size_t j = 0;
      while (blk->children[j].kind != CDD_CST_CHILD_NODE ||
             blk->children[j].val.node != cur) {
        j++;
      }
      *out_block = blk;
      *out_stmt_idx = j;
      return CDD_C_SUCCESS;
    }
    cur = cur->parent;
  }
  return CDD_C_ERROR_NOT_FOUND;
}

/*
 * @brief Standardizes GNU expression extensions (`__extension__`,
 * `__builtin_shuffle`, `__alignof__`, anonymous structs/unions, cast-as-lvalue,
 * omitted ternary middle operands, and `return (void)expr;`) using semantic
 * context.
 *
 * @param[in,out] tree CST tree to transform.
 * @param[in,out] gctx Optional GNU standardizer context.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
cdd_c_error_t gnu_standardize_expr_ctx(cdd_cst_tree_t *tree,
                                       struct gnu_standardizer_ctx_t *gctx) {
  size_t i;
  cdd_c_error_t rc = CDD_C_SUCCESS;
  struct gnu_standardizer_ctx_t local_ctx;
  struct gnu_standardizer_ctx_t *ctx_p = gctx;
  int is_msvc = 0;

  if (!ctx_p) {
    memset(&local_ctx, 0, sizeof(local_ctx));
    ctx_p = &local_ctx;
  }
  rc = gnu_is_msvc_target(ctx_p->config, &is_msvc);

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
        const char *align_kw = is_msvc ? "__alignof" : "_Alignof";
        rc = replace_token_with_text(tree, tok, CDD_TOKEN_IDENTIFIER, align_kw,
                                     strlen(align_kw));
        if (rc != CDD_C_SUCCESS)
          return rc;
      } else if (tok->length == 5 && memcmp(tok->start, "union", 5) == 0) {
        if (i + 1 < tree->base_tokens->size &&
            tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_LBRACE) {
          int is_unnamed = 0;
          rc = (cdd_c_error_t)((int)rc |
                               (int)expr_check_unnamed_member(
                                   tree->base_tokens, i + 1, &is_unnamed));
          if (!is_unnamed)
            continue;
          if (is_msvc) {
            rc = expr_emit_msvc_4201_once(tree, ctx_p);
            if (rc != CDD_C_SUCCESS)
              return rc;
            continue;
          }
          {
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
              bld.error_state = cdd_cst_builder_init(&bld, tree, temp);
              bld.error_state = cdd_cst_bld_ident(&bld, "union");
              bld.error_state = cdd_cst_bld_space(&bld);
              {
                char tb2[128];
                const char *pooled = NULL;
                CDD_SNPRINTF(tb2, 128, "_cdd_anon_%d", ctx_p->anon_idx++);
                bld.error_state = (cdd_c_error_t)((int)bld.error_state |
                                                  (int)cdd_pool_string_safe(
                                                      tree, tb2, &pooled));
                bld.error_state = cdd_cst_bld_ident(&bld, pooled);
              }
              if (bld.error_state != 0) {
                bld.error_state =
                    (cdd_c_error_t)((int)bld.error_state |
                                    (int)cdd_cst_builder_free(&bld));
                bld.error_state =
                    (cdd_c_error_t)((int)bld.error_state |
                                    (int)cdd_cst_free_node_only(temp));
                return CDD_C_ERROR_MEMORY;
              }
              bld.error_state =
                  cdd_cst_splice_children(tree, &owning_node, child_idx, 1,
                                          temp->children, temp->num_children);
              bld.error_state =
                  (cdd_c_error_t)((int)bld.error_state |
                                  (int)cdd_cst_builder_free(&bld));
              bld.error_state =
                  (cdd_c_error_t)((int)bld.error_state |
                                  (int)cdd_cst_free_node_only(temp));
            }
          }
        }
      }
    } else if (tok->kind == CDD_TOKEN_KEYWORD_STRUCT) {
      if (i + 1 < tree->base_tokens->size &&
          tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_LBRACE) {
        int is_unnamed = 0;
        rc = (cdd_c_error_t)((int)rc |
                             (int)expr_check_unnamed_member(
                                 tree->base_tokens, i + 1, &is_unnamed));
        if (!is_unnamed)
          continue;
        if (is_msvc) {
          rc = expr_emit_msvc_4201_once(tree, ctx_p);
          if (rc != CDD_C_SUCCESS)
            return rc;
          continue;
        }
        {
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
            bld.error_state = cdd_cst_builder_init(&bld, tree, temp);
            bld.error_state = cdd_cst_bld_ident(&bld, "struct");
            bld.error_state = cdd_cst_bld_space(&bld);
            {
              char tb2[128];
              const char *pooled = NULL;
              CDD_SNPRINTF(tb2, 128, "_cdd_anon_%d", ctx_p->anon_idx++);
              bld.error_state = (cdd_c_error_t)((int)bld.error_state |
                                                (int)cdd_pool_string_safe(
                                                    tree, tb2, &pooled));
              bld.error_state = cdd_cst_bld_ident(&bld, pooled);
            }
            if (bld.error_state != 0) {
              bld.error_state =
                  (cdd_c_error_t)((int)bld.error_state |
                                  (int)cdd_cst_builder_free(&bld));
              bld.error_state =
                  (cdd_c_error_t)((int)bld.error_state |
                                  (int)cdd_cst_free_node_only(temp));
              return CDD_C_ERROR_MEMORY;
            }
            bld.error_state =
                cdd_cst_splice_children(tree, &owning_node, child_idx, 1,
                                        temp->children, temp->num_children);
            bld.error_state = (cdd_c_error_t)((int)bld.error_state |
                                              (int)cdd_cst_builder_free(&bld));
            bld.error_state =
                (cdd_c_error_t)((int)bld.error_state |
                                (int)cdd_cst_free_node_only(temp));
          }
        }
      }
    } else if (tok->kind == CDD_TOKEN_LPAREN) {
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
          if (i > 0 &&
              (tree->base_tokens->tokens[i - 1].kind ==
                   CDD_TOKEN_KEYWORD_TYPEOF ||
               tree->base_tokens->tokens[i - 1].kind == CDD_TOKEN_IDENTIFIER)) {
            is_cast_lvalue = 0;
          }
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
        rc = cdd_pool_string_safe(tree, buf, &pooled);
        C_CDD_FREE(buf);
        if (rc != CDD_C_SUCCESS)
          return rc;
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
        /* Elvis `x ?: y`: hoist typed temporary if `x` has side effects */
        size_t k;
        size_t lhs_start = i;
        int depth = 0;
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
        if (k == (size_t)-1) {
          lhs_start = 0;
        }

        if (lhs_start < i) {
          const uint8_t *expr_start =
              tree->base_tokens->tokens[lhs_start].start;
          const uint8_t *expr_end = tree->base_tokens->tokens[i - 1].start +
                                    tree->base_tokens->tokens[i - 1].length;
          size_t len = (size_t)(expr_end - expr_start);
          int has_se = 0;
          cdd_cst_node_t *owning_node = NULL;
          cdd_cst_node_t *blk_node = NULL;
          size_t tok_cidx = 0;
          size_t stmt_idx = 0;

          rc = (cdd_c_error_t)((int)rc | (int)expr_cond_has_side_effects(
                                             tree->base_tokens->tokens,
                                             lhs_start, i, &has_se));
          if (has_se &&
              gnu_find_node(tree->root, tok, &tok_cidx, &owning_node) ==
                  CDD_C_SUCCESS &&
              expr_find_enclosing_block_stmt(owning_node, &blk_node,
                                             &stmt_idx) == CDD_C_SUCCESS) {
            char cond_type[160];
            char tmp_name[64];
            char q_repl[96];
            char *decl_snip = NULL;
            const char *pooled_tmp = NULL;
            const char *pooled_q = NULL;
            cdd_cst_node_t *hoist_node = NULL;
            cdd_cst_builder_t bld;
            size_t wipe;
            int cidx = ctx_p->cond_idx++;

            rc = (cdd_c_error_t)((int)rc |
                                 (int)cdd_eval_expr_type_scoped(
                                     ctx_p, owning_node,
                                     &tree->base_tokens->tokens[lhs_start],
                                     i - lhs_start, CDD_CST_TYPE_MODE_RVALUE,
                                     cond_type, sizeof(cond_type), NULL));
            CDD_SNPRINTF(tmp_name, sizeof(tmp_name), "__cdd_cond_%d", cidx);
            CDD_SNPRINTF(q_repl, sizeof(q_repl),
                         tok->trailing_trivia ? "? %s" : "? %s ", tmp_name);

            rc = gnu_malloc(strlen(cond_type) + strlen(tmp_name) + len + 16,
                            (void **)&decl_snip);
            if (rc != CDD_C_SUCCESS)
              return rc;
            CDD_SNPRINTF(decl_snip,
                         strlen(cond_type) + strlen(tmp_name) + len + 16,
                         "%s %s = (%.*s); ", cond_type, tmp_name, (int)len,
                         (const char *)expr_start);

            rc = gnu_alloc_node(CDD_CST_UNKNOWN, &hoist_node);
            if (rc != CDD_C_SUCCESS) {
              C_CDD_FREE(decl_snip);
              return rc;
            }
            bld.error_state = cdd_cst_builder_init(&bld, tree, hoist_node);
            bld.error_state =
                (cdd_c_error_t)((int)bld.error_state |
                                (int)cdd_cst_bld_snippet(&bld, decl_snip));
            C_CDD_FREE(decl_snip);
            if (bld.error_state != CDD_C_SUCCESS) {
              rc = (cdd_c_error_t)((int)CDD_C_ERROR_MEMORY |
                                   (int)cdd_cst_builder_free(&bld) |
                                   (int)cdd_cst_free_node_only(hoist_node));
              return rc;
            }
            rc = cdd_cst_splice_children(tree, &blk_node, stmt_idx, 0,
                                         hoist_node->children,
                                         hoist_node->num_children);
            rc = (cdd_c_error_t)((int)rc | (int)cdd_cst_builder_free(&bld) |
                                 (int)cdd_cst_free_node_only(hoist_node));
            if (rc != CDD_C_SUCCESS)
              return rc;

            rc = cdd_pool_string_safe(tree, tmp_name, &pooled_tmp);
            if (rc != CDD_C_SUCCESS)
              return rc;
            rc = cdd_pool_string_safe(tree, q_repl, &pooled_q);
            if (rc != CDD_C_SUCCESS)
              return rc;

            rc = replace_token_with_text(
                tree, &tree->base_tokens->tokens[lhs_start],
                CDD_TOKEN_IDENTIFIER, pooled_tmp, strlen(pooled_tmp));
            if (rc != CDD_C_SUCCESS)
              return rc;
            for (wipe = lhs_start + 1; wipe < i; wipe++) {
              rc = replace_token_with_text(
                  tree, &tree->base_tokens->tokens[wipe],
                  tree->base_tokens->tokens[wipe].kind, "", 0);
              if (rc != CDD_C_SUCCESS)
                return rc;
            }
            rc = replace_token_with_text(tree, tok, tok->kind, pooled_q,
                                         strlen(pooled_q));
            if (rc != CDD_C_SUCCESS)
              return rc;
          } else {
            char *buf;
            char *p;
            const char *pooled;
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
            rc = cdd_pool_string_safe(tree, buf, &pooled);
            C_CDD_FREE(buf);
            if (rc != CDD_C_SUCCESS)
              return rc;
            rc = replace_token_with_text(tree, tok, tok->kind, pooled,
                                         strlen(pooled));
            if (rc != CDD_C_SUCCESS)
              return rc;
          }
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
            rc = replace_token_with_text(tree, &tree->base_tokens->tokens[k],
                                         CDD_TOKEN_SEMICOLON, "; return;", 9);
            if (rc != CDD_C_SUCCESS)
              return rc;
            break;
          }
        }
      }
    }
  }

  /* Lower `void *` / function-pointer arithmetic and `__int128` operators */
  if (ctx_p->sem_env || ctx_p->used_int128) {
    int has_vptr = 0;
    int has_i128 = ctx_p->used_int128;

    if (ctx_p->sem_env) {
      rc = gnu_scan_scope_types(ctx_p->sem_env->global_scope, &has_vptr,
                                &has_i128);
    }

    if (has_vptr || has_i128) {
      for (i = 0; i < tree->base_tokens->size; i++) {
        cdd_token_t *tk = &tree->base_tokens->tokens[i];
        size_t cidx = 0;
        cdd_cst_node_t *owner = NULL;
        cdd_cst_node_t *blk_node = NULL;
        size_t stmt_idx = 0;
        int in_exec = 0;

        if (tk->length == 0)
          continue;

        /* Pre/post `++` / `--` on `void *` or fn-ptr */
        if (has_vptr && i + 1 < tree->base_tokens->size &&
            ((tk->kind == CDD_TOKEN_PLUS &&
              tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_PLUS) ||
             (tk->kind == CDD_TOKEN_MINUS &&
              tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_MINUS)) &&
            tree->base_tokens->tokens[i + 1].offset == tk->offset + 1) {
          char op_ch = (tk->kind == CDD_TOKEN_PLUS) ? '+' : '-';
          /* Post-inc/dec: `v++` */
          if (i > 0 &&
              tree->base_tokens->tokens[i - 1].kind == CDD_TOKEN_IDENTIFIER &&
              tree->base_tokens->tokens[i - 1].length < 96) {
            cdd_token_t *vt = &tree->base_tokens->tokens[i - 1];
            char vty[128];
            int is_tn = 0;
            if (gnu_find_node(tree->root, vt, &cidx, &owner) == CDD_C_SUCCESS) {
              rc = (cdd_c_error_t)((int)rc | (int)cdd_eval_expr_type_scoped(
                                                 ctx_p, owner, vt, 1,
                                                 CDD_CST_TYPE_MODE_RVALUE, vty,
                                                 sizeof(vty), &is_tn));
              if (!is_tn && (strstr(vty, "void *") != NULL ||
                             strstr(vty, "(*)") != NULL)) {
                const char *vp =
                    (strstr(vty, "const ") != NULL) ? "const void *" : "void *";
                const char *cp =
                    (strstr(vty, "const ") != NULL) ? "const char *" : "char *";
                char repl[320];
                const char *pooled_r = NULL;
                CDD_SNPRINTF(repl, sizeof(repl),
                             "((%s)((%s)(%.*s = (%s)((%s)(%.*s) %c 1)) %c 1))",
                             vp, cp, (int)vt->length, (const char *)vt->start,
                             vp, cp, (int)vt->length, (const char *)vt->start,
                             op_ch, op_ch == '+' ? '-' : '+');
                rc = cdd_pool_string_safe(tree, repl, &pooled_r);
                if (rc != CDD_C_SUCCESS)
                  return rc;
                rc = replace_token_with_text(tree, vt, CDD_TOKEN_IDENTIFIER,
                                             pooled_r, strlen(pooled_r));
                if (rc != CDD_C_SUCCESS)
                  return rc;
                rc = replace_token_with_text(tree, tk, tk->kind, "", 0);
                if (rc != CDD_C_SUCCESS)
                  return rc;
                rc = replace_token_with_text(
                    tree, &tree->base_tokens->tokens[i + 1],
                    tree->base_tokens->tokens[i + 1].kind, "", 0);
                if (rc != CDD_C_SUCCESS)
                  return rc;
                i++;
                continue;
              }
            }
          }
          /* Pre-inc/dec: `++v` */
          if (i + 2 < tree->base_tokens->size &&
              tree->base_tokens->tokens[i + 2].kind == CDD_TOKEN_IDENTIFIER &&
              tree->base_tokens->tokens[i + 2].length < 96) {
            cdd_token_t *vt = &tree->base_tokens->tokens[i + 2];
            char vty[128];
            int is_tn = 0;
            if (gnu_find_node(tree->root, vt, &cidx, &owner) == CDD_C_SUCCESS) {
              rc = (cdd_c_error_t)((int)rc | (int)cdd_eval_expr_type_scoped(
                                                 ctx_p, owner, vt, 1,
                                                 CDD_CST_TYPE_MODE_RVALUE, vty,
                                                 sizeof(vty), &is_tn));
              if (!is_tn && (strstr(vty, "void *") != NULL ||
                             strstr(vty, "(*)") != NULL)) {
                const char *vp =
                    (strstr(vty, "const ") != NULL) ? "const void *" : "void *";
                const char *cp =
                    (strstr(vty, "const ") != NULL) ? "const char *" : "char *";
                char repl[256];
                const char *pooled_r = NULL;
                CDD_SNPRINTF(repl, sizeof(repl),
                             "(%.*s = (%s)((%s)(%.*s) %c 1))", (int)vt->length,
                             (const char *)vt->start, vp, cp, (int)vt->length,
                             (const char *)vt->start, op_ch);
                rc = cdd_pool_string_safe(tree, repl, &pooled_r);
                if (rc != CDD_C_SUCCESS)
                  return rc;
                rc = replace_token_with_text(tree, tk, CDD_TOKEN_IDENTIFIER,
                                             pooled_r, strlen(pooled_r));
                if (rc != CDD_C_SUCCESS)
                  return rc;
                rc = replace_token_with_text(
                    tree, &tree->base_tokens->tokens[i + 1],
                    tree->base_tokens->tokens[i + 1].kind, "", 0);
                if (rc != CDD_C_SUCCESS)
                  return rc;
                rc = replace_token_with_text(tree, vt, vt->kind, "", 0);
                if (rc != CDD_C_SUCCESS)
                  return rc;
                i += 2;
                continue;
              }
            }
          }
        }

        /* Compound assignments: `+=`, `-=`, `*=`, `/=`, `%=`, `&=`, `|=`,
           `^=`, `<<=`, `>>=` */
        {
          size_t op_len = 0;
          const char *i128_op = NULL;
          int is_shift_assign = 0;
          char vop_ch = 0;

          if (i + 1 < tree->base_tokens->size &&
              tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_ASSIGN &&
              tree->base_tokens->tokens[i + 1].offset == tk->offset + 1) {
            if (tk->kind == CDD_TOKEN_PLUS) {
              op_len = 2;
              i128_op = "add";
              vop_ch = '+';
            } else if (tk->kind == CDD_TOKEN_MINUS) {
              op_len = 2;
              i128_op = "sub";
              vop_ch = '-';
            } else if (tk->kind == CDD_TOKEN_STAR) {
              op_len = 2;
              i128_op = "mul";
            } else if (tk->kind == CDD_TOKEN_SLASH) {
              op_len = 2;
              i128_op = "div";
            } else if (tk->start[0] == '%') {
              op_len = 2;
              i128_op = "mod";
            } else if (tk->start[0] == '&') {
              op_len = 2;
              i128_op = "and";
            } else if (tk->start[0] == '|') {
              op_len = 2;
              i128_op = "or";
            } else if (tk->start[0] == '^') {
              op_len = 2;
              i128_op = "xor";
            }
          } else if (i + 2 < tree->base_tokens->size &&
                     ((tk->kind == CDD_TOKEN_LT &&
                       tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_LT) ||
                      (tk->kind == CDD_TOKEN_GT &&
                       tree->base_tokens->tokens[i + 1].kind ==
                           CDD_TOKEN_GT)) &&
                     tree->base_tokens->tokens[i + 2].kind ==
                         CDD_TOKEN_ASSIGN &&
                     tree->base_tokens->tokens[i + 1].offset ==
                         tk->offset + 1 &&
                     tree->base_tokens->tokens[i + 2].offset ==
                         tk->offset + 2) {
            op_len = 3;
            is_shift_assign = 1;
            i128_op = (tk->kind == CDD_TOKEN_LT) ? "shl" : "shr";
          }

          if (op_len > 0 && i > 0 && i + op_len < tree->base_tokens->size) {
            size_t ls = i;
            size_t re = i + op_len;
            int pd = 0;
            while (ls > 0) {
              enum cdd_token_kind_t lk = tree->base_tokens->tokens[ls - 1].kind;
              if (lk == CDD_TOKEN_RPAREN || lk == CDD_TOKEN_RBRACKET)
                pd++;
              else if (lk == CDD_TOKEN_LPAREN || lk == CDD_TOKEN_LBRACKET) {
                if (pd == 0)
                  break;
                pd--;
              } else if (pd == 0 &&
                         (lk == CDD_TOKEN_SEMICOLON || lk == CDD_TOKEN_COMMA ||
                          lk == CDD_TOKEN_LBRACE || lk == CDD_TOKEN_RBRACE ||
                          lk == CDD_TOKEN_ASSIGN)) {
                break;
              }
              ls--;
            }
            pd = 0;
            while (re < tree->base_tokens->size) {
              enum cdd_token_kind_t rk = tree->base_tokens->tokens[re].kind;
              if (rk == CDD_TOKEN_LPAREN || rk == CDD_TOKEN_LBRACKET)
                pd++;
              else if (rk == CDD_TOKEN_RPAREN || rk == CDD_TOKEN_RBRACKET) {
                if (pd == 0)
                  break;
                pd--;
              } else if (pd == 0 &&
                         (rk == CDD_TOKEN_SEMICOLON || rk == CDD_TOKEN_COMMA ||
                          rk == CDD_TOKEN_RBRACE)) {
                break;
              }
              re++;
            }
            if (ls < i && re > i + op_len &&
                gnu_find_node(tree->root, tk, &cidx, &owner) == CDD_C_SUCCESS) {
              char lty[128];
              int l_tn = 0;
              if (cdd_eval_expr_type_scoped(
                      ctx_p, owner, &tree->base_tokens->tokens[ls], i - ls,
                      CDD_CST_TYPE_MODE_RVALUE, lty, sizeof(lty),
                      &l_tn) == CDD_C_SUCCESS &&
                  !l_tn) {
                char lhs_s[160];
                char rhs_s[256];
                size_t k_t;
                size_t lp = 0;
                size_t rp_l = 0;
                lhs_s[0] = '\0';
                rhs_s[0] = '\0';
                for (k_t = ls; k_t < i; k_t++) {
                  if (tree->base_tokens->tokens[k_t].length == 0)
                    continue;
                  if (lp > 0 && lp + 1 < sizeof(lhs_s))
                    lhs_s[lp++] = ' ';
                  if (lp + tree->base_tokens->tokens[k_t].length <
                      sizeof(lhs_s)) {
                    memcpy(lhs_s + lp, tree->base_tokens->tokens[k_t].start,
                           tree->base_tokens->tokens[k_t].length);
                    lp += tree->base_tokens->tokens[k_t].length;
                  }
                }
                lhs_s[lp] = '\0';
                for (k_t = i + op_len; k_t < re; k_t++) {
                  if (tree->base_tokens->tokens[k_t].length == 0)
                    continue;
                  if (rp_l > 0 && rp_l + 1 < sizeof(rhs_s))
                    rhs_s[rp_l++] = ' ';
                  if (rp_l + tree->base_tokens->tokens[k_t].length <
                      sizeof(rhs_s)) {
                    memcpy(rhs_s + rp_l, tree->base_tokens->tokens[k_t].start,
                           tree->base_tokens->tokens[k_t].length);
                    rp_l += tree->base_tokens->tokens[k_t].length;
                  }
                }
                rhs_s[rp_l] = '\0';

                if (has_vptr && vop_ch != 0 &&
                    (strstr(lty, "void *") != NULL ||
                     strstr(lty, "(*)") != NULL)) {
                  const char *vp = (strstr(lty, "const ") != NULL)
                                       ? "const void *"
                                       : "void *";
                  const char *cp = (strstr(lty, "const ") != NULL)
                                       ? "const char *"
                                       : "char *";
                  char repl[640];
                  const char *pooled_r = NULL;
                  size_t w;
                  CDD_SNPRINTF(repl, sizeof(repl),
                               "%s = (%s)((%s)(%s) %c (%s))", lhs_s, vp, cp,
                               lhs_s, vop_ch, rhs_s);
                  rc = cdd_pool_string_safe(tree, repl, &pooled_r);
                  if (rc != CDD_C_SUCCESS)
                    return rc;
                  rc = replace_token_with_text(
                      tree, &tree->base_tokens->tokens[ls],
                      CDD_TOKEN_IDENTIFIER, pooled_r, strlen(pooled_r));
                  if (rc != CDD_C_SUCCESS)
                    return rc;
                  for (w = ls + 1; w < re; w++) {
                    rc = replace_token_with_text(
                        tree, &tree->base_tokens->tokens[w],
                        tree->base_tokens->tokens[w].kind, "", 0);
                    if (rc != CDD_C_SUCCESS)
                      return rc;
                  }
                  i = re - 1;
                  continue;
                }

                if (has_i128 && strstr(lty, "int128") != NULL) {
                  int is_u = (strstr(lty, "uint128") != NULL ||
                              strstr(lty, "unsigned") != NULL);
                  const char *pfx = is_u ? "cdd_uint128" : "cdd_int128";
                  char repl[640];
                  const char *pooled_r = NULL;
                  size_t w;
                  ctx_p->used_int128 = 1;
                  if (is_shift_assign) {
                    CDD_SNPRINTF(repl, sizeof(repl),
                                 "%s_%s(%s, (unsigned int)(%s), &(%s))", pfx,
                                 i128_op, lhs_s, rhs_s, lhs_s);
                  } else {
                    CDD_SNPRINTF(repl, sizeof(repl), "%s_%s(%s, %s, &(%s))",
                                 pfx, i128_op, lhs_s, rhs_s, lhs_s);
                  }
                  rc = cdd_pool_string_safe(tree, repl, &pooled_r);
                  if (rc != CDD_C_SUCCESS)
                    return rc;
                  rc = replace_token_with_text(
                      tree, &tree->base_tokens->tokens[ls],
                      CDD_TOKEN_IDENTIFIER, pooled_r, strlen(pooled_r));
                  if (rc != CDD_C_SUCCESS)
                    return rc;
                  for (w = ls + 1; w < re; w++) {
                    rc = replace_token_with_text(
                        tree, &tree->base_tokens->tokens[w],
                        tree->base_tokens->tokens[w].kind, "", 0);
                    if (rc != CDD_C_SUCCESS)
                      return rc;
                  }
                  i = re - 1;
                  continue;
                }
              }
            }
          }
        }

        /* Unary `-x` or `~x` on 128-bit integer inside a statement block */
        if (has_i128 &&
            (tk->kind == CDD_TOKEN_TILDE || tk->kind == CDD_TOKEN_MINUS) &&
            i + 1 < tree->base_tokens->size &&
            tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_IDENTIFIER) {
          int is_unary = (tk->kind == CDD_TOKEN_TILDE);
          if (!is_unary && i > 0) {
            enum cdd_token_kind_t pk = tree->base_tokens->tokens[i - 1].kind;
            if (pk == CDD_TOKEN_ASSIGN || pk == CDD_TOKEN_LPAREN ||
                pk == CDD_TOKEN_COMMA || pk == CDD_TOKEN_KEYWORD_RETURN ||
                pk == CDD_TOKEN_COLON || pk == CDD_TOKEN_SEMICOLON ||
                pk == CDD_TOKEN_LBRACE) {
              is_unary = 1;
            }
          }
          if (is_unary &&
              gnu_find_node(tree->root, tk, &cidx, &owner) == CDD_C_SUCCESS) {
            cdd_cst_node_t *cur = owner;
            while (cur->parent) {
              if (cur->parent->kind == CDD_CST_BLOCK) {
                size_t si2 = 0;
                blk_node = cur->parent;
                while (blk_node->children[si2].kind != CDD_CST_CHILD_NODE ||
                       blk_node->children[si2].val.node != cur) {
                  si2++;
                }
                stmt_idx = si2;
                in_exec = 1;
                break;
              }
              cur = cur->parent;
            }
            if (in_exec) {
              cdd_token_t *ut = &tree->base_tokens->tokens[i + 1];
              char uty[128];
              int u_tn = 0;
              rc = (cdd_c_error_t)((int)rc | (int)cdd_eval_expr_type_scoped(
                                                 ctx_p, owner, ut, 1,
                                                 CDD_CST_TYPE_MODE_RVALUE, uty,
                                                 sizeof(uty), &u_tn));
              if (!u_tn && strstr(uty, "int128") != NULL) {
                int is_u = (strstr(uty, "uint128") != NULL ||
                            strstr(uty, "unsigned") != NULL);
                int tid = ++ctx_p->i128_idx;
                char tmp_nm[64];
                char h_snip[320];
                const char *pooled_tmp = NULL;
                cdd_cst_node_t *hnode = NULL;
                cdd_cst_builder_t bld;
                ctx_p->used_int128 = 1;
                CDD_SNPRINTF(tmp_nm, sizeof(tmp_nm), "__cdd_i128_%d", tid);
                CDD_SNPRINTF(h_snip, sizeof(h_snip),
                             "%s %s; { cdd_c_error_t __cdd_rc_%d = "
                             "%s_%s(%.*s, &%s); if (__cdd_rc_%d != "
                             "CDD_C_SUCCESS) {} } ",
                             is_u ? "cdd_uint128_t" : "cdd_int128_t", tmp_nm,
                             tid, is_u ? "cdd_uint128" : "cdd_int128",
                             tk->kind == CDD_TOKEN_TILDE ? "not" : "neg",
                             (int)ut->length, (const char *)ut->start, tmp_nm,
                             tid);
                rc = gnu_alloc_node(CDD_CST_UNKNOWN, &hnode);
                if (rc != CDD_C_SUCCESS)
                  return rc;
                bld.error_state = cdd_cst_builder_init(&bld, tree, hnode);
                bld.error_state =
                    (cdd_c_error_t)((int)bld.error_state |
                                    (int)cdd_cst_bld_snippet(&bld, h_snip));
                if (bld.error_state != CDD_C_SUCCESS) {
                  rc = (cdd_c_error_t)((int)cdd_cst_builder_free(&bld) |
                                       (int)cdd_cst_free_node_only(hnode));
                  return CDD_C_ERROR_MEMORY;
                }
                rc = cdd_cst_splice_children(tree, &blk_node, stmt_idx, 0,
                                             hnode->children,
                                             hnode->num_children);
                rc = (cdd_c_error_t)((int)rc | (int)cdd_cst_builder_free(&bld) |
                                     (int)cdd_cst_free_node_only(hnode));
                if (rc != CDD_C_SUCCESS)
                  return rc;
                rc = cdd_pool_string_safe(tree, tmp_nm, &pooled_tmp);
                if (rc != CDD_C_SUCCESS)
                  return rc;
                rc = replace_token_with_text(tree, tk, CDD_TOKEN_IDENTIFIER,
                                             pooled_tmp, strlen(pooled_tmp));
                if (rc != CDD_C_SUCCESS)
                  return rc;
                rc = replace_token_with_text(tree, ut, ut->kind, "", 0);
                if (rc != CDD_C_SUCCESS)
                  return rc;
                i++;
                continue;
              }
            }
          }
        }

        /* Binary `+`, `-`, `*`, `/`, `%`, `&`, `|`, `^`, `<<`, `>>`, `==`,
           `!=`, `<`, `<=`, `>`, `>=` */
        {
          size_t bop_len = 0;
          const char *i128_fn = NULL;
          const char *cmp_sym = NULL;
          int is_shift = 0;
          char v_addsub = 0;

          if (tk->kind == CDD_TOKEN_PLUS) {
            bop_len = 1;
            i128_fn = "add";
            v_addsub = '+';
          } else if (tk->kind == CDD_TOKEN_MINUS) {
            bop_len = 1;
            i128_fn = "sub";
            v_addsub = '-';
          } else if (tk->kind == CDD_TOKEN_STAR) {
            bop_len = 1;
            i128_fn = "mul";
          } else if (tk->kind == CDD_TOKEN_SLASH) {
            bop_len = 1;
            i128_fn = "div";
          } else if (tk->length == 1 && tk->start[0] == '%') {
            bop_len = 1;
            i128_fn = "mod";
          } else if (tk->length == 1 && tk->start[0] == '&' &&
                     !(i + 1 < tree->base_tokens->size &&
                       tree->base_tokens->tokens[i + 1].length == 1 &&
                       tree->base_tokens->tokens[i + 1].start[0] == '&' &&
                       tree->base_tokens->tokens[i + 1].offset ==
                           tk->offset + 1)) {
            bop_len = 1;
            i128_fn = "and";
          } else if (tk->length == 1 && tk->start[0] == '|' &&
                     !(i + 1 < tree->base_tokens->size &&
                       tree->base_tokens->tokens[i + 1].length == 1 &&
                       tree->base_tokens->tokens[i + 1].start[0] == '|' &&
                       tree->base_tokens->tokens[i + 1].offset ==
                           tk->offset + 1)) {
            bop_len = 1;
            i128_fn = "or";
          } else if (tk->length == 1 && tk->start[0] == '^') {
            bop_len = 1;
            i128_fn = "xor";
          } else if (i + 1 < tree->base_tokens->size &&
                     ((tk->kind == CDD_TOKEN_LT &&
                       tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_LT) ||
                      (tk->kind == CDD_TOKEN_GT &&
                       tree->base_tokens->tokens[i + 1].kind ==
                           CDD_TOKEN_GT)) &&
                     tree->base_tokens->tokens[i + 1].offset ==
                         tk->offset + 1) {
            bop_len = 2;
            is_shift = 1;
            i128_fn = (tk->kind == CDD_TOKEN_LT) ? "shl" : "shr";
          } else if (tk->kind == CDD_TOKEN_EQ) {
            bop_len = 1;
            cmp_sym = "==";
          } else if (tk->kind == CDD_TOKEN_NEQ) {
            bop_len = 1;
            cmp_sym = "!=";
          } else if (tk->kind == CDD_TOKEN_LT) {
            if (i + 1 < tree->base_tokens->size &&
                tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_ASSIGN &&
                tree->base_tokens->tokens[i + 1].offset == tk->offset + 1) {
              bop_len = 2;
              cmp_sym = "<=";
            } else {
              bop_len = 1;
              cmp_sym = "<";
            }
          } else if (tk->kind == CDD_TOKEN_GT) {
            if (i + 1 < tree->base_tokens->size &&
                tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_ASSIGN &&
                tree->base_tokens->tokens[i + 1].offset == tk->offset + 1) {
              bop_len = 2;
              cmp_sym = ">=";
            } else {
              bop_len = 1;
              cmp_sym = ">";
            }
          }

          /* Must follow an rvalue token (not unary `+`/`-`/`*`/`&`) */
          if (bop_len > 0 && i > 0 && i + bop_len < tree->base_tokens->size) {
            enum cdd_token_kind_t pk = tree->base_tokens->tokens[i - 1].kind;
            if (pk == CDD_TOKEN_IDENTIFIER || pk == CDD_TOKEN_NUMBER ||
                pk == CDD_TOKEN_CHAR || pk == CDD_TOKEN_RPAREN ||
                pk == CDD_TOKEN_RBRACKET) {
              size_t ls = i;
              size_t re = i + bop_len;
              int pd = 0;

              while (ls > 0) {
                cdd_token_t *ltok = &tree->base_tokens->tokens[ls - 1];
                enum cdd_token_kind_t lk = ltok->kind;
                if (lk == CDD_TOKEN_RPAREN || lk == CDD_TOKEN_RBRACKET)
                  pd++;
                else if (lk == CDD_TOKEN_LPAREN || lk == CDD_TOKEN_LBRACKET) {
                  if (pd == 0)
                    break;
                  pd--;
                } else if (pd == 0) {
                  if (lk == CDD_TOKEN_SEMICOLON || lk == CDD_TOKEN_COMMA ||
                      lk == CDD_TOKEN_LBRACE || lk == CDD_TOKEN_RBRACE ||
                      lk == CDD_TOKEN_ASSIGN || lk == CDD_TOKEN_COLON ||
                      lk == CDD_TOKEN_KEYWORD_RETURN ||
                      (ltok->length == 1 && ltok->start[0] == '?')) {
                    break;
                  }
                  if (cmp_sym == NULL &&
                      (lk == CDD_TOKEN_EQ || lk == CDD_TOKEN_NEQ ||
                       lk == CDD_TOKEN_LT || lk == CDD_TOKEN_GT)) {
                    break;
                  }
                }
                ls--;
              }

              pd = 0;
              while (re < tree->base_tokens->size) {
                cdd_token_t *rtok = &tree->base_tokens->tokens[re];
                enum cdd_token_kind_t rk = rtok->kind;
                if (rk == CDD_TOKEN_LPAREN || rk == CDD_TOKEN_LBRACKET)
                  pd++;
                else if (rk == CDD_TOKEN_RPAREN || rk == CDD_TOKEN_RBRACKET) {
                  if (pd == 0)
                    break;
                  pd--;
                } else if (pd == 0) {
                  if (rk == CDD_TOKEN_SEMICOLON || rk == CDD_TOKEN_COMMA ||
                      rk == CDD_TOKEN_RBRACE || rk == CDD_TOKEN_COLON ||
                      (rtok->length == 1 && rtok->start[0] == '?')) {
                    break;
                  }
                  if (cmp_sym == NULL &&
                      (rk == CDD_TOKEN_EQ || rk == CDD_TOKEN_NEQ ||
                       rk == CDD_TOKEN_LT || rk == CDD_TOKEN_GT)) {
                    break;
                  }
                  if (v_addsub != 0 && re > i + bop_len &&
                      (rk == CDD_TOKEN_PLUS || rk == CDD_TOKEN_MINUS)) {
                    break;
                  }
                }
                re++;
              }

              if (re > i + bop_len && gnu_find_node(tree->root, tk, &cidx,
                                                    &owner) == CDD_C_SUCCESS) {
                char lty[128];
                char rty[128];
                int l_tn = 0;
                int r_tn = 0;
                if (cdd_eval_expr_type_scoped(
                        ctx_p, owner, &tree->base_tokens->tokens[ls], i - ls,
                        CDD_CST_TYPE_MODE_RVALUE, lty, sizeof(lty),
                        &l_tn) == CDD_C_SUCCESS &&
                    cdd_eval_expr_type_scoped(
                        ctx_p, owner, &tree->base_tokens->tokens[i + bop_len],
                        re - (i + bop_len), CDD_CST_TYPE_MODE_RVALUE, rty,
                        sizeof(rty), &r_tn) == CDD_C_SUCCESS &&
                    !l_tn && !r_tn) {
                  char lhs_s[192];
                  char rhs_s[192];
                  size_t kt;
                  size_t ll = 0;
                  size_t rl = 0;
                  int l_vptr = (strstr(lty, "void *") != NULL ||
                                strstr(lty, "(*)") != NULL);
                  int r_vptr = (strstr(rty, "void *") != NULL ||
                                strstr(rty, "(*)") != NULL);
                  int l_i128 = (strstr(lty, "int128") != NULL);
                  int r_i128 = (strstr(rty, "int128") != NULL);

                  lhs_s[0] = '\0';
                  rhs_s[0] = '\0';
                  if ((has_vptr && v_addsub != 0 && (l_vptr || r_vptr)) ||
                      (has_i128 && (l_i128 || r_i128))) {
                    for (kt = ls; kt < i; kt++) {
                      if (tree->base_tokens->tokens[kt].length == 0)
                        continue;
                      if (ll > 0 && ll + 1 < sizeof(lhs_s))
                        lhs_s[ll++] = ' ';
                      if (ll + tree->base_tokens->tokens[kt].length <
                          sizeof(lhs_s)) {
                        memcpy(lhs_s + ll, tree->base_tokens->tokens[kt].start,
                               tree->base_tokens->tokens[kt].length);
                        ll += tree->base_tokens->tokens[kt].length;
                      }
                    }
                    lhs_s[ll] = '\0';
                    for (kt = i + bop_len; kt < re; kt++) {
                      if (tree->base_tokens->tokens[kt].length == 0)
                        continue;
                      if (rl > 0 && rl + 1 < sizeof(rhs_s))
                        rhs_s[rl++] = ' ';
                      if (rl + tree->base_tokens->tokens[kt].length <
                          sizeof(rhs_s)) {
                        memcpy(rhs_s + rl, tree->base_tokens->tokens[kt].start,
                               tree->base_tokens->tokens[kt].length);
                        rl += tree->base_tokens->tokens[kt].length;
                      }
                    }
                    rhs_s[rl] = '\0';
                  }

                  /* 1. `void *` / function-pointer binary `+` / `-` */
                  if (has_vptr && v_addsub != 0 && (l_vptr || r_vptr)) {
                    char repl[512];
                    const char *pooled_r = NULL;
                    size_t w;
                    repl[0] = '\0';
                    if (l_vptr && r_vptr && v_addsub == '-') {
                      CDD_SNPRINTF(repl, sizeof(repl),
                                   "((const char *)(%s) - (const char *)(%s))",
                                   lhs_s, rhs_s);
                    } else if (l_vptr && !r_vptr && strchr(rty, '*') == NULL) {
                      const char *vp = (strstr(lty, "const ") != NULL)
                                           ? "const void *"
                                           : "void *";
                      const char *cp = (strstr(lty, "const ") != NULL)
                                           ? "const char *"
                                           : "char *";
                      CDD_SNPRINTF(repl, sizeof(repl),
                                   "((%s)((%s)(%s) %c (%s)))", vp, cp, lhs_s,
                                   v_addsub, rhs_s);
                    } else if (r_vptr && !l_vptr && v_addsub == '+' &&
                               strchr(lty, '*') == NULL) {
                      const char *vp = (strstr(rty, "const ") != NULL)
                                           ? "const void *"
                                           : "void *";
                      const char *cp = (strstr(rty, "const ") != NULL)
                                           ? "const char *"
                                           : "char *";
                      CDD_SNPRINTF(repl, sizeof(repl),
                                   "((%s)((%s) + (%s)(%s)))", vp, lhs_s, cp,
                                   rhs_s);
                    }
                    if (repl[0] != '\0') {
                      rc = cdd_pool_string_safe(tree, repl, &pooled_r);
                      if (rc != CDD_C_SUCCESS)
                        return rc;
                      rc = replace_token_with_text(
                          tree, &tree->base_tokens->tokens[ls],
                          CDD_TOKEN_IDENTIFIER, pooled_r, strlen(pooled_r));
                      if (rc != CDD_C_SUCCESS)
                        return rc;
                      for (w = ls + 1; w < re; w++) {
                        rc = replace_token_with_text(
                            tree, &tree->base_tokens->tokens[w],
                            tree->base_tokens->tokens[w].kind, "", 0);
                        if (rc != CDD_C_SUCCESS)
                          return rc;
                      }
                      i = re - 1;
                      continue;
                    }
                  }

                  /* 2. `__int128` binary & comparison operators */
                  if (has_i128 && (l_i128 || r_i128)) {
                    cdd_cst_node_t *cur = owner;
                    while (cur->parent) {
                      if (cur->parent->kind == CDD_CST_BLOCK) {
                        size_t si2 = 0;
                        blk_node = cur->parent;
                        while (blk_node->children[si2].kind !=
                                   CDD_CST_CHILD_NODE ||
                               blk_node->children[si2].val.node != cur) {
                          si2++;
                        }
                        stmt_idx = si2;
                        in_exec = 1;
                        break;
                      }
                      cur = cur->parent;
                    }
                    if (in_exec) {
                      int is_u = (strstr(lty, "uint128") != NULL ||
                                  strstr(lty, "unsigned") != NULL ||
                                  strstr(rty, "uint128") != NULL ||
                                  strstr(rty, "unsigned") != NULL);
                      const char *pfx = is_u ? "cdd_uint128" : "cdd_int128";
                      int tid = ++ctx_p->i128_idx;
                      char h_snip[640];
                      char expr_repl[128];
                      const char *pooled_er = NULL;
                      cdd_cst_node_t *hnode = NULL;
                      cdd_cst_builder_t bld;
                      size_t w;
                      ctx_p->used_int128 = 1;

                      if (cmp_sym != NULL) {
                        CDD_SNPRINTF(
                            h_snip, sizeof(h_snip),
                            "int __cdd_i128_cmp_%d = 0; { cdd_c_error_t "
                            "__cdd_rc_%d = %s_cmp(%s, %s, &__cdd_i128_cmp_%d); "
                            "if (__cdd_rc_%d != CDD_C_SUCCESS) {} } ",
                            tid, tid, pfx, lhs_s, rhs_s, tid, tid);
                        CDD_SNPRINTF(expr_repl, sizeof(expr_repl),
                                     "(__cdd_i128_cmp_%d %s 0)", tid, cmp_sym);
                      } else if (is_shift) {
                        CDD_SNPRINTF(
                            h_snip, sizeof(h_snip),
                            "%s_t __cdd_i128_%d; { cdd_c_error_t __cdd_rc_%d = "
                            "%s_%s(%s, (unsigned int)(%s), &__cdd_i128_%d); "
                            "if (__cdd_rc_%d != CDD_C_SUCCESS) {} } ",
                            pfx, tid, tid, pfx, i128_fn, lhs_s, rhs_s, tid,
                            tid);
                        CDD_SNPRINTF(expr_repl, sizeof(expr_repl),
                                     "__cdd_i128_%d", tid);
                      } else {
                        CDD_SNPRINTF(
                            h_snip, sizeof(h_snip),
                            "%s_t __cdd_i128_%d; { cdd_c_error_t __cdd_rc_%d = "
                            "%s_%s(%s, %s, &__cdd_i128_%d); if (__cdd_rc_%d != "
                            "CDD_C_SUCCESS) {} } ",
                            pfx, tid, tid, pfx, i128_fn, lhs_s, rhs_s, tid,
                            tid);
                        CDD_SNPRINTF(expr_repl, sizeof(expr_repl),
                                     "__cdd_i128_%d", tid);
                      }

                      rc = gnu_alloc_node(CDD_CST_UNKNOWN, &hnode);
                      if (rc != CDD_C_SUCCESS)
                        return rc;
                      bld.error_state = cdd_cst_builder_init(&bld, tree, hnode);
                      bld.error_state =
                          (cdd_c_error_t)((int)bld.error_state |
                                          (int)cdd_cst_bld_snippet(&bld,
                                                                   h_snip));
                      if (bld.error_state != CDD_C_SUCCESS) {
                        rc =
                            (cdd_c_error_t)((int)cdd_cst_builder_free(&bld) |
                                            (int)cdd_cst_free_node_only(hnode));
                        return CDD_C_ERROR_MEMORY;
                      }
                      rc = cdd_cst_splice_children(tree, &blk_node, stmt_idx, 0,
                                                   hnode->children,
                                                   hnode->num_children);
                      rc = (cdd_c_error_t)((int)rc |
                                           (int)cdd_cst_builder_free(&bld) |
                                           (int)cdd_cst_free_node_only(hnode));
                      if (rc != CDD_C_SUCCESS)
                        return rc;

                      rc = cdd_pool_string_safe(tree, expr_repl, &pooled_er);
                      if (rc != CDD_C_SUCCESS)
                        return rc;
                      rc = replace_token_with_text(
                          tree, &tree->base_tokens->tokens[ls],
                          CDD_TOKEN_IDENTIFIER, pooled_er, strlen(pooled_er));
                      if (rc != CDD_C_SUCCESS)
                        return rc;
                      for (w = ls + 1; w < re; w++) {
                        rc = replace_token_with_text(
                            tree, &tree->base_tokens->tokens[w],
                            tree->base_tokens->tokens[w].kind, "", 0);
                        if (rc != CDD_C_SUCCESS)
                          return rc;
                      }
                      i = re - 1;
                      continue;
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

  return CDD_C_SUCCESS;
}

/*
 * @brief Standardizes GNU expression extensions without external context.
 *
 * @param[in,out] tree CST tree to transform.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
cdd_c_error_t gnu_standardize_expr(cdd_cst_tree_t *tree) {
  return gnu_standardize_expr_ctx(tree, NULL);
}
