/**
 * @file gnu_standardizer_unroll.c
 * @brief Statement unrolling and VLA standardization pass for GNU standardizer.
 */

/* clang-format off */
#include "gnu_standardizer_internal.h"
/* clang-format on */

/** @brief Local `__label__` alpha-renaming entry */
typedef struct {
  const uint8_t *name; /**< Original label identifier bytes */
  size_t length;       /**< Byte length of `name` */
  char rename[64];     /**< Generated unique label identifier */
  int depth;           /**< Brace depth where declared */
} local_label_t;

/** @brief Active heap-lowered VLA entry */
typedef struct {
  const uint8_t *name; /**< VLA variable identifier bytes */
  size_t length;       /**< Byte length of `name` */
  int depth;           /**< Brace depth where declared */
} vla_t;

/** @brief Active `__attribute__((cleanup(fn)))` handler entry */
typedef struct {
  const uint8_t *var_name;  /**< Variable identifier bytes */
  size_t var_length;        /**< Byte length of `var_name` */
  const uint8_t *func_name; /**< Cleanup function identifier bytes */
  size_t func_length;       /**< Byte length of `func_name` */
  int depth;                /**< Brace depth where declared */
} cleanup_t;

/** @brief Loop/switch boundary entry for `break`/`continue` cleanup unwinding
 */
typedef struct {
  int depth;     /**< Brace depth of loop or switch body */
  int is_switch; /**< 1 if `switch`, 0 if `for`/`while`/`do` */
} ctrl_scope_t;

/**
 * @brief Checks if a token matches a keyword string.
 *
 * @param[in] tok Token to inspect.
 * @param[in] kw Null-terminated keyword.
 * @param[out] out_eq Receives 1 if equal, 0 otherwise.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t unroll_tok_eq(const cdd_token_t *tok, const char *kw,
                                   int *out_eq) {
  size_t l = strlen(kw);
  *out_eq = 0;
  if (tok->length == l && memcmp(tok->start, kw, l) == 0)
    *out_eq = 1;
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks if `tokens[start..end)` inside a statement expression starts
 * with a control statement or declaration rather than a value expression.
 *
 * @param[in] gctx Optional GNU standardizer context.
 * @param[in] scope_node CST block node of the statement expression.
 * @param[in] tokens Base token array.
 * @param[in] start Inclusive start token index of final statement.
 * @param[in] end Exclusive end token index (before `;`).
 * @param[out] out_is_void Receives 1 if void/non-value statement, 0 otherwise.
 * @param[out] out_type Buffer receiving evaluated value type when non-void.
 * @param[in] type_cap Capacity of `out_type`.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t
unroll_classify_final_stmt(struct gnu_standardizer_ctx_t *gctx,
                           cdd_cst_node_t *scope_node,
                           const cdd_token_t *tokens, size_t start, size_t end,
                           int *out_is_void, char *out_type, size_t type_cap) {
  static const char *const k_non_expr[] = {
      "if",     "for",      "while",    "do",     "switch",  "return", "goto",
      "break",  "continue", "typedef",  "static", "extern",  "struct", "union",
      "enum",   "void",     "char",     "short",  "int",     "long",   "float",
      "double", "signed",   "unsigned", "const",  "volatile"};
  size_t w;
  int is_tn = 0;

  *out_is_void = 1;
  out_type[0] = '\0';
  if (start >= end)
    return CDD_C_SUCCESS;

  if (tokens[start].kind == CDD_TOKEN_KEYWORD_RETURN ||
      tokens[start].kind == CDD_TOKEN_KEYWORD_GOTO ||
      tokens[start].kind == CDD_TOKEN_KEYWORD_INT ||
      tokens[start].kind == CDD_TOKEN_KEYWORD_STRUCT ||
      tokens[start].kind == CDD_TOKEN_KEYWORD___INT128) {
    return CDD_C_SUCCESS;
  }
  for (w = 0; w < sizeof(k_non_expr) / sizeof(k_non_expr[0]); w++) {
    int m = 0;
    cdd_c_error_t rc_eq = unroll_tok_eq(&tokens[start], k_non_expr[w], &m);
    if (m) {
      return rc_eq;
    }
  }
  /* Label `lbl:` with nothing after `:` */
  if (end == start + 2 && tokens[start].kind == CDD_TOKEN_IDENTIFIER &&
      tokens[start + 1].kind == CDD_TOKEN_COLON) {
    return CDD_C_SUCCESS;
  }
  /* Typedef-named variable declaration `my_t x = ...` */
  if (end > start + 1 && tokens[start].kind == CDD_TOKEN_IDENTIFIER &&
      (tokens[start + 1].kind == CDD_TOKEN_IDENTIFIER ||
       tokens[start + 1].kind == CDD_TOKEN_STAR) &&
      gctx->sem_env && tokens[start].length < 120) {
    char tname[128];
    cdd_cst_scope_t *sc = NULL;
    cdd_cst_symbol_t *tsym = NULL;
    memcpy(tname, tokens[start].start, tokens[start].length);
    tname[tokens[start].length] = '\0';
    {
      cdd_c_error_t rc_sc =
          cdd_cst_scope_find_for_node(gctx->sem_env, scope_node, &sc);
      if (cdd_cst_scope_lookup_in_scope(sc, tname, CDD_CST_SYMBOL_TYPEDEF,
                                        &tsym) == CDD_C_SUCCESS &&
          tsym->kind == CDD_CST_SYMBOL_TYPEDEF) {
        return rc_sc;
      }
    }
  }

  {
    cdd_c_error_t rc_ev = cdd_eval_expr_type_scoped(
        gctx, scope_node, &tokens[start], end - start, CDD_CST_TYPE_MODE_RVALUE,
        out_type, type_cap, &is_tn);
    if (strcmp(out_type, "void") == 0) {
      *out_is_void = 1;
      return rc_ev;
    }
    *out_is_void = 0;
    return rc_ev;
  }
}

/**
 * @brief Hoists a GNU statement expression `({ s_1; ... s_k; })` starting at
 * `tokens[lp_idx]` (`(`) and `tokens[lp_idx + 1]` (`{`) before its enclosing
 * statement in CST.
 *
 * @param[in,out] tree Target CST tree.
 * @param[in,out] gctx GNU standardizer context.
 * @param[in] lp_idx Token index of `(` in `tree->base_tokens`.
 * @param[out] out_rbrace_idx Receives token index of matching `}` if hoisted.
 * @return CDD_C_SUCCESS on success, or error code on OOM.
 */
static cdd_c_error_t unroll_hoist_stmt_expr(cdd_cst_tree_t *tree,
                                            struct gnu_standardizer_ctx_t *gctx,
                                            size_t lp_idx,
                                            size_t *out_rbrace_idx) {
  cdd_token_t *tokens = tree->base_tokens->tokens;
  size_t total = tree->base_tokens->size;
  size_t rbrace_idx = 0;
  size_t rparen_idx;
  int br_depth = 0;
  size_t k;
  size_t lb_cidx = 0;
  size_t lp_cidx = 0;
  cdd_cst_node_t *lbrace_owner = NULL;
  cdd_cst_node_t *lp_parent = NULL;
  cdd_cst_node_t *inner_block = NULL;
  size_t blk_pos_in_lp = 0;
  cdd_cst_node_t *enc_container = NULL;
  size_t enc_stmt_idx = 0;
  size_t last_semi = 0;
  size_t last_stmt_start;
  int has_last_semi = 0;
  int is_void_final = 1;
  char val_type[160];
  cdd_c_error_t rc;

  *out_rbrace_idx = 0;

  for (k = lp_idx + 1; k < total; k++) {
    if (tokens[k].kind == CDD_TOKEN_LBRACE)
      br_depth++;
    else if (tokens[k].kind == CDD_TOKEN_RBRACE) {
      br_depth--;
      if (br_depth == 0) {
        rbrace_idx = k;
        break;
      }
    }
  }
  if (rbrace_idx == 0 || rbrace_idx + 1 >= total ||
      tokens[rbrace_idx + 1].kind != CDD_TOKEN_RPAREN) {
    return CDD_C_SUCCESS;
  }
  rparen_idx = rbrace_idx + 1;

  rc = gnu_find_node(tree->root, &tokens[lp_idx + 1], &lb_cidx, &lbrace_owner);
  if (rc != CDD_C_SUCCESS)
    return rc;
  inner_block = lbrace_owner;

  rc = gnu_find_node(tree->root, &tokens[lp_idx], &lp_cidx, &lp_parent);
  if (rc != CDD_C_SUCCESS)
    return rc;

  while (lp_parent->children[blk_pos_in_lp].kind != CDD_CST_CHILD_NODE ||
         lp_parent->children[blk_pos_in_lp].val.node != inner_block) {
    blk_pos_in_lp++;
  }

  /* Locate enclosing block or translation unit statement slot */
  {
    cdd_cst_node_t *cur = lp_parent;
    while (cur->parent->kind != CDD_CST_BLOCK &&
           cur->parent->kind != CDD_CST_TRANSLATION_UNIT) {
      cur = cur->parent;
    }
    enc_container = cur->parent;
    while (enc_container->children[enc_stmt_idx].kind != CDD_CST_CHILD_NODE ||
           enc_container->children[enc_stmt_idx].val.node != cur) {
      enc_stmt_idx++;
    }
  }

  /* Scan top-level statements inside `{ ... }` */
  last_stmt_start = lp_idx + 2;
  {
    int p_d = 0;
    int b_d = 0;
    size_t prev_boundary = lp_idx + 2;
    for (k = lp_idx + 2; k < rbrace_idx; k++) {
      enum cdd_token_kind_t tk = tokens[k].kind;
      if (tk == CDD_TOKEN_LPAREN || tk == CDD_TOKEN_LBRACKET)
        p_d++;
      else if ((tk == CDD_TOKEN_RPAREN || tk == CDD_TOKEN_RBRACKET) && p_d > 0)
        p_d--;
      else if (tk == CDD_TOKEN_LBRACE)
        b_d++;
      else if (tk == CDD_TOKEN_RBRACE) {
        b_d--;
        if (b_d == 0 && p_d == 0 && k + 1 < rbrace_idx) {
          prev_boundary = k + 1;
        }
      } else if (p_d == 0 && b_d == 0 && tk == CDD_TOKEN_SEMICOLON) {
        last_stmt_start = prev_boundary;
        last_semi = k;
        has_last_semi = 1;
        prev_boundary = k + 1;
      }
    }
  }

  if (has_last_semi) {
    rc = (cdd_c_error_t)((int)rc | (int)unroll_classify_final_stmt(
                                       gctx, inner_block, tokens,
                                       last_stmt_start, last_semi,
                                       &is_void_final, val_type,
                                       sizeof(val_type)));
  }

  if (enc_container->kind == CDD_CST_BLOCK) {
    char tmp_name[64];
    cdd_cst_node_t *hoist_wrap = NULL;
    cdd_cst_builder_t bld;
    int se_idx = gctx->stmt_expr_idx++;

    CDD_SNPRINTF(tmp_name, sizeof(tmp_name), "__cdd_stmt_expr_%d", se_idx);

    if (!is_void_final) {
      size_t expr_cidx = 0;
      cdd_cst_node_t *expr_owner = NULL;
      rc = gnu_find_node(inner_block, &tokens[last_stmt_start], &expr_cidx,
                         &expr_owner);
      if (rc == CDD_C_SUCCESS) {
        cdd_cst_node_t *assign_pre = NULL;
        cdd_cst_builder_t abld;
        char assign_snip[96];
        CDD_SNPRINTF(assign_snip, sizeof(assign_snip), "%s = (", tmp_name);
        rc = gnu_alloc_node(CDD_CST_UNKNOWN, &assign_pre);
        if (rc != CDD_C_SUCCESS)
          return rc;
        abld.error_state = cdd_cst_builder_init(&abld, tree, assign_pre);
        abld.error_state =
            (cdd_c_error_t)((int)abld.error_state |
                            (int)cdd_cst_bld_snippet(&abld, assign_snip));
        if (abld.error_state != CDD_C_SUCCESS) {
          rc = (cdd_c_error_t)((int)CDD_C_ERROR_MEMORY |
                               (int)cdd_cst_builder_free(&abld) |
                               (int)cdd_cst_free_node_only(assign_pre));
          return rc;
        }
        rc = cdd_cst_splice_children(tree, &expr_owner, expr_cidx, 0,
                                     assign_pre->children,
                                     assign_pre->num_children);
        rc = (cdd_c_error_t)((int)rc | (int)cdd_cst_builder_free(&abld) |
                             (int)cdd_cst_free_node_only(assign_pre));
        if (rc != CDD_C_SUCCESS)
          return rc;
        rc = replace_token_with_text(tree, &tokens[last_semi],
                                     CDD_TOKEN_SEMICOLON, ");", 2);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
    }

    rc = (cdd_c_error_t)((int)rc |
                         (int)cdd_cst_remove_child(lp_parent, blk_pos_in_lp));

    rc = (cdd_c_error_t)((int)rc |
                         (int)gnu_alloc_node(CDD_CST_UNKNOWN, &hoist_wrap));
    if (rc != CDD_C_SUCCESS) {
      rc = (cdd_c_error_t)((int)rc | (int)cdd_cst_free_node(inner_block));
      return rc;
    }
    bld.error_state = cdd_cst_builder_init(&bld, tree, hoist_wrap);
    if (!is_void_final) {
      char decl_hdr[256];
      CDD_SNPRINTF(decl_hdr, sizeof(decl_hdr), "%s %s; ", val_type, tmp_name);
      bld.error_state =
          (cdd_c_error_t)((int)bld.error_state |
                          (int)cdd_cst_bld_snippet(&bld, decl_hdr));
    }
    if (bld.error_state == CDD_C_SUCCESS) {
      bld.error_state = cdd_cst_append_child_node(hoist_wrap, inner_block);
      if (bld.error_state != CDD_C_SUCCESS) {
        bld.error_state = (cdd_c_error_t)((int)bld.error_state |
                                          (int)cdd_cst_free_node(inner_block));
      }
    } else {
      bld.error_state = (cdd_c_error_t)((int)bld.error_state |
                                        (int)cdd_cst_free_node(inner_block));
    }
    bld.error_state =
        (cdd_c_error_t)((int)bld.error_state | (int)cdd_cst_bld_space(&bld));
    if (bld.error_state != CDD_C_SUCCESS) {
      rc = (cdd_c_error_t)((int)CDD_C_ERROR_MEMORY |
                           (int)cdd_cst_builder_free(&bld) |
                           (int)cdd_cst_free_node(hoist_wrap));
      return rc;
    }

    rc =
        cdd_cst_splice_children(tree, &enc_container, enc_stmt_idx, 0,
                                hoist_wrap->children, hoist_wrap->num_children);
    rc = (cdd_c_error_t)((int)rc | (int)cdd_cst_builder_free(&bld) |
                         (int)cdd_cst_free_node_only(hoist_wrap));
    if (rc != CDD_C_SUCCESS)
      return rc;

    if (!is_void_final) {
      const char *pooled_tmp = NULL;
      rc = cdd_pool_string_safe(tree, tmp_name, &pooled_tmp);
      if (rc != CDD_C_SUCCESS)
        return rc;
      rc = replace_token_with_text(tree, &tokens[lp_idx], CDD_TOKEN_IDENTIFIER,
                                   pooled_tmp, strlen(pooled_tmp));
      if (rc != CDD_C_SUCCESS)
        return rc;
      rc = replace_token_with_text(tree, &tokens[rparen_idx], CDD_TOKEN_RPAREN,
                                   "", 0);
      if (rc != CDD_C_SUCCESS)
        return rc;
    } else {
      rc = replace_token_with_text(tree, &tokens[lp_idx], CDD_TOKEN_LPAREN,
                                   "((void)0", 8);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  } else {
    /* Global / translation-unit scope: synthesize static helper function */
    char fn_name[64];
    char call_open[72];
    char fn_hdr[256];
    const char *pooled_call = NULL;
    cdd_cst_node_t *hoist_fn = NULL;
    cdd_cst_builder_t bld;
    int se_idx = gctx->stmt_expr_idx++;

    CDD_SNPRINTF(fn_name, sizeof(fn_name), "__cdd_stmt_fn_%d", se_idx);
    CDD_SNPRINTF(call_open, sizeof(call_open), "%s(", fn_name);
    CDD_SNPRINTF(fn_hdr, sizeof(fn_hdr), "static %s %s(void) ",
                 !is_void_final ? val_type : "void", fn_name);

    if (!is_void_final) {
      size_t expr_cidx = 0;
      cdd_cst_node_t *expr_owner = NULL;
      if (gnu_find_node(inner_block, &tokens[last_stmt_start], &expr_cidx,
                        &expr_owner) == CDD_C_SUCCESS) {
        cdd_cst_node_t *ret_pre = NULL;
        cdd_cst_builder_t rbld;
        rc = gnu_alloc_node(CDD_CST_UNKNOWN, &ret_pre);
        if (rc != CDD_C_SUCCESS)
          return rc;
        rbld.error_state = cdd_cst_builder_init(&rbld, tree, ret_pre);
        rbld.error_state =
            (cdd_c_error_t)((int)rbld.error_state |
                            (int)cdd_cst_bld_snippet(&rbld, "return ("));
        if (rbld.error_state != CDD_C_SUCCESS) {
          rc = (cdd_c_error_t)((int)CDD_C_ERROR_MEMORY |
                               (int)cdd_cst_builder_free(&rbld) |
                               (int)cdd_cst_free_node_only(ret_pre));
          return rc;
        }
        rc = cdd_cst_splice_children(tree, &expr_owner, expr_cidx, 0,
                                     ret_pre->children, ret_pre->num_children);
        rc = (cdd_c_error_t)((int)rc | (int)cdd_cst_builder_free(&rbld) |
                             (int)cdd_cst_free_node_only(ret_pre));
        if (rc != CDD_C_SUCCESS)
          return rc;
        rc = replace_token_with_text(tree, &tokens[last_semi],
                                     CDD_TOKEN_SEMICOLON, ");", 2);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
    }

    rc = (cdd_c_error_t)((int)rc |
                         (int)cdd_cst_remove_child(lp_parent, blk_pos_in_lp));

    rc = (cdd_c_error_t)((int)rc | (int)gnu_alloc_node(
                                       CDD_CST_FUNCTION_DEFINITION, &hoist_fn));
    if (rc != CDD_C_SUCCESS) {
      rc = (cdd_c_error_t)((int)rc | (int)cdd_cst_free_node(inner_block));
      return rc;
    }
    bld.error_state = cdd_cst_builder_init(&bld, tree, hoist_fn);
    bld.error_state = (cdd_c_error_t)((int)bld.error_state |
                                      (int)cdd_cst_bld_snippet(&bld, fn_hdr));
    if (bld.error_state == CDD_C_SUCCESS) {
      bld.error_state = cdd_cst_append_child_node(hoist_fn, inner_block);
      if (bld.error_state != CDD_C_SUCCESS) {
        bld.error_state = (cdd_c_error_t)((int)bld.error_state |
                                          (int)cdd_cst_free_node(inner_block));
      }
    } else {
      bld.error_state = (cdd_c_error_t)((int)bld.error_state |
                                        (int)cdd_cst_free_node(inner_block));
    }
    bld.error_state =
        (cdd_c_error_t)((int)bld.error_state | (int)cdd_cst_bld_newline(&bld));
    if (bld.error_state != CDD_C_SUCCESS) {
      rc = (cdd_c_error_t)((int)CDD_C_ERROR_MEMORY |
                           (int)cdd_cst_builder_free(&bld) |
                           (int)cdd_cst_free_node(hoist_fn));
      return rc;
    }
    rc = cdd_cst_splice_children(tree, &enc_container, enc_stmt_idx, 0,
                                 hoist_fn->children, hoist_fn->num_children);
    rc = (cdd_c_error_t)((int)rc | (int)cdd_cst_builder_free(&bld) |
                         (int)cdd_cst_free_node_only(hoist_fn));
    if (rc != CDD_C_SUCCESS)
      return rc;

    rc = cdd_pool_string_safe(tree, call_open, &pooled_call);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = replace_token_with_text(tree, &tokens[lp_idx], CDD_TOKEN_IDENTIFIER,
                                 pooled_call, strlen(pooled_call));
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  *out_rbrace_idx = rbrace_idx;
  return CDD_C_SUCCESS;
}

/*
 * @brief Standardizes GNU scope/statement extensions
 * (`__attribute__((cleanup))`, `__label__` local labels, statement expressions
 * `({...})`, VLAs, and zero-length arrays) using semantic context.
 *
 * @param[in,out] tree CST tree to transform.
 * @param[in,out] gctx Optional GNU standardizer context.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
cdd_c_error_t gnu_standardize_unroll_ctx(cdd_cst_tree_t *tree,
                                         struct gnu_standardizer_ctx_t *gctx) {
  local_label_t local_labels[MAX_LOCAL_LABELS];
  vla_t vlas[MAX_VLAS];
  cleanup_t cleanups[MAX_CLEANUPS];
  ctrl_scope_t ctrl_stack[32];
  size_t num_local_labels = 0;
  size_t num_vlas = 0;
  size_t num_cleanups = 0;
  size_t num_ctrl = 0;
  int pending_ctrl = -1; /* -1: none, 0: loop, 1: switch */
  int pending_paren_depth = 0;
  int current_depth = 0;
  int label_counter = 0;
  char enclosing_fn_ret[128];
  struct gnu_standardizer_ctx_t local_ctx;
  struct gnu_standardizer_ctx_t *ctx_p = gctx;
  const cdd_transform_config_t *config;
  size_t i;
  size_t last_uncond_ret_rbrace = (size_t)-1;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  if (!ctx_p) {
    memset(&local_ctx, 0, sizeof(local_ctx));
    ctx_p = &local_ctx;
  }
  config = ctx_p->config;
  enclosing_fn_ret[0] = '\0';

  for (i = 0; i < tree->base_tokens->size; i++) {
    cdd_token_t *t = &tree->base_tokens->tokens[i];
    if (pending_ctrl >= 0) {
      if (t->kind == CDD_TOKEN_LPAREN)
        pending_paren_depth++;
      else if (t->kind == CDD_TOKEN_RPAREN && pending_paren_depth > 0)
        pending_paren_depth--;
      else if (pending_paren_depth == 0 && t->kind == CDD_TOKEN_SEMICOLON)
        pending_ctrl = -1;
    }

    /* Hoist statement expressions `({ ... })` before processing `{` */
    if (t->kind == CDD_TOKEN_LPAREN && i + 1 < tree->base_tokens->size &&
        tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_LBRACE) {
      size_t hoisted_rb = 0;
      rc = unroll_hoist_stmt_expr(tree, ctx_p, i, &hoisted_rb);
      if (rc != CDD_C_SUCCESS)
        return rc;
      if (hoisted_rb == 0) {
        /* Fallback for malformed fragments */
        rc = replace_token_with_text(tree, t, t->kind, "", 0);
        if (rc != CDD_C_SUCCESS)
          return rc;
        rc = replace_token_with_text(tree, &tree->base_tokens->tokens[i + 1],
                                     tree->base_tokens->tokens[i + 1].kind, "",
                                     0);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
      continue;
    }

    if (t->kind == CDD_TOKEN_LBRACE) {
      if (current_depth == 0 && ctx_p->sem_env && i >= 2 &&
          tree->base_tokens->tokens[i - 1].kind == CDD_TOKEN_RPAREN) {
        /* Identify function name before parameter list */
        int pd = 0;
        size_t back = i;
        enclosing_fn_ret[0] = '\0';
        while (back > 0) {
          back--;
          if (tree->base_tokens->tokens[back].kind == CDD_TOKEN_RPAREN)
            pd++;
          else if (tree->base_tokens->tokens[back].kind == CDD_TOKEN_LPAREN) {
            pd--;
            if (pd == 0) {
              if (back > 0 &&
                  tree->base_tokens->tokens[back - 1].kind ==
                      CDD_TOKEN_IDENTIFIER &&
                  tree->base_tokens->tokens[back - 1].length < 120) {
                char fn_id[128];
                cdd_cst_symbol_t *fsym = NULL;
                memcpy(fn_id, tree->base_tokens->tokens[back - 1].start,
                       tree->base_tokens->tokens[back - 1].length);
                fn_id[tree->base_tokens->tokens[back - 1].length] = '\0';
                if (cdd_cst_scope_lookup_in_scope(
                        ctx_p->sem_env->global_scope, fn_id,
                        CDD_CST_SYMBOL_FUNCTION, &fsym) == CDD_C_SUCCESS &&
                    fsym->type_str) {
                  CDD_SNPRINTF(enclosing_fn_ret, sizeof(enclosing_fn_ret), "%s",
                               fsym->type_str);
                }
              }
              break;
            }
          }
        }
      }
      current_depth++;
      if (pending_ctrl >= 0 && pending_paren_depth == 0 && num_ctrl < 32) {
        ctrl_stack[num_ctrl].depth = current_depth;
        ctrl_stack[num_ctrl].is_switch = pending_ctrl;
        num_ctrl++;
        pending_ctrl = -1;
      }
    } else if (t->kind == CDD_TOKEN_IDENTIFIER &&
               ((t->length == 3 && memcmp(t->start, "for", 3) == 0) ||
                (t->length == 5 && memcmp(t->start, "while", 5) == 0) ||
                (t->length == 2 && memcmp(t->start, "do", 2) == 0))) {
      pending_ctrl = 0;
      pending_paren_depth = 0;
    } else if (t->kind == CDD_TOKEN_IDENTIFIER && t->length == 6 &&
               memcmp(t->start, "switch", 6) == 0) {
      pending_ctrl = 1;
      pending_paren_depth = 0;
    } else if (t->kind == CDD_TOKEN_IDENTIFIER &&
               ((t->length == 13 &&
                 memcmp(t->start, "__attribute__", 13) == 0) ||
                (t->length == 11 &&
                 memcmp(t->start, "__attribute", 11) == 0))) {
      if (i + 8 < tree->base_tokens->size &&
          tree->base_tokens->tokens[i + 4].kind == CDD_TOKEN_LPAREN &&
          tree->base_tokens->tokens[i + 8].kind == CDD_TOKEN_RPAREN) {
        cdd_token_t *attr = &tree->base_tokens->tokens[i + 3];
        if ((attr->length == 7 && memcmp(attr->start, "cleanup", 7) == 0) ||
            (attr->length == 11 &&
             memcmp(attr->start, "__cleanup__", 11) == 0)) {
          cdd_token_t *func_val = &tree->base_tokens->tokens[i + 5];
          cdd_token_t *var_tok = NULL;
          size_t k;
          for (k = i + 9; k < tree->base_tokens->size; k++) {
            if (tree->base_tokens->tokens[k].kind == CDD_TOKEN_SEMICOLON ||
                tree->base_tokens->tokens[k].kind == CDD_TOKEN_ASSIGN ||
                tree->base_tokens->tokens[k].kind == CDD_TOKEN_COMMA) {
              size_t back;
              for (back = k; back > i + 8; back--) {
                if (tree->base_tokens->tokens[back].kind ==
                    CDD_TOKEN_IDENTIFIER) {
                  var_tok = &tree->base_tokens->tokens[back];
                  break;
                }
              }
              break;
            }
          }
          if (var_tok && num_cleanups < MAX_CLEANUPS) {
            cleanups[num_cleanups].var_name = var_tok->start;
            cleanups[num_cleanups].var_length = var_tok->length;
            cleanups[num_cleanups].func_name = func_val->start;
            cleanups[num_cleanups].func_length = func_val->length;
            cleanups[num_cleanups].depth = current_depth;
            num_cleanups++;
          }

          {
            size_t tk_idx;
            for (tk_idx = 0; tk_idx <= 8; tk_idx++) {
              rc = replace_token_with_text(
                  tree, &tree->base_tokens->tokens[i + tk_idx],
                  tree->base_tokens->tokens[i + tk_idx].kind, "", 0);
              if (rc != CDD_C_SUCCESS)
                return rc;
            }
          }
          i += 8;
        }
      }
    } else if (t->kind == CDD_TOKEN_RBRACE) {
      while (num_local_labels > 0 &&
             local_labels[num_local_labels - 1].depth == current_depth) {
        num_local_labels--;
      }
      while (num_ctrl > 0 && ctrl_stack[num_ctrl - 1].depth == current_depth) {
        num_ctrl--;
      }
      if (i == last_uncond_ret_rbrace) {
        while (num_cleanups > 0 &&
               cleanups[num_cleanups - 1].depth == current_depth) {
          num_cleanups--;
        }
        if (config && config->fallback_vla_to_malloc) {
          while (num_vlas > 0 && vlas[num_vlas - 1].depth == current_depth) {
            num_vlas--;
          }
        }
      }
      {
        char buf[4096] = {0};
        char *p = buf;
        int appended = 0;
        size_t child_idx;
        cdd_cst_node_t *owning_node = NULL;
        rc = gnu_find_node(tree->root, t, &child_idx, &owning_node);
        if (rc != CDD_C_SUCCESS)
          return rc;

        /* Run cleanups in reverse order */
        while (num_cleanups > 0 &&
               cleanups[num_cleanups - 1].depth == current_depth) {
          cdd_cst_node_t *temp = NULL;
          cdd_cst_builder_t bld;
          const char *pooled_fn = NULL;
          const char *pooled_var = NULL;
          rc = gnu_alloc_node(CDD_CST_UNKNOWN, &temp);
          if (rc != CDD_C_SUCCESS)
            return rc;
          bld.error_state = cdd_cst_builder_init(&bld, tree, temp);
          bld.error_state =
              (cdd_c_error_t)((int)bld.error_state |
                              (int)cdd_pool_string_safe_len(
                                  tree,
                                  (const char *)cleanups[num_cleanups - 1]
                                      .func_name,
                                  cleanups[num_cleanups - 1].func_length,
                                  &pooled_fn));
          bld.error_state = cdd_cst_bld_ident(&bld, pooled_fn);
          bld.error_state = cdd_cst_bld_punct(&bld, "(");
          bld.error_state = cdd_cst_bld_punct(&bld, "&");
          bld.error_state =
              (cdd_c_error_t)((int)bld.error_state |
                              (int)cdd_pool_string_safe_len(
                                  tree,
                                  (const char *)cleanups[num_cleanups - 1]
                                      .var_name,
                                  cleanups[num_cleanups - 1].var_length,
                                  &pooled_var));
          bld.error_state = cdd_cst_bld_ident(&bld, pooled_var);
          bld.error_state = cdd_cst_bld_punct(&bld, ")");
          bld.error_state = cdd_cst_bld_punct(&bld, ";");
          if (bld.error_state != 0) {
            bld.error_state = (cdd_c_error_t)((int)bld.error_state |
                                              (int)cdd_cst_builder_free(&bld));
            bld.error_state =
                (cdd_c_error_t)((int)bld.error_state |
                                (int)cdd_cst_free_node_only(temp));
            return CDD_C_ERROR_MEMORY;
          }
          bld.error_state =
              cdd_cst_splice_children(tree, &owning_node, child_idx, 0,
                                      temp->children, temp->num_children);
          bld.error_state = (cdd_c_error_t)((int)bld.error_state |
                                            (int)cdd_cst_builder_free(&bld));
          bld.error_state = (cdd_c_error_t)((int)bld.error_state |
                                            (int)cdd_cst_free_node_only(temp));
          num_cleanups--;
          appended = 1;
        }

        if (config && config->fallback_vla_to_malloc) {
          while (num_vlas > 0 && vlas[num_vlas - 1].depth == current_depth) {
            cdd_cst_node_t *temp = NULL;
            cdd_cst_builder_t bld;
            const char *pooled_vla = NULL;
            rc = gnu_alloc_node(CDD_CST_UNKNOWN, &temp);
            if (rc != CDD_C_SUCCESS)
              return rc;
            bld.error_state = cdd_cst_builder_init(&bld, tree, temp);
            bld.error_state = cdd_cst_bld_ident(&bld, "free");
            bld.error_state = cdd_cst_bld_punct(&bld, "(");
            bld.error_state =
                (cdd_c_error_t)((int)bld.error_state |
                                (int)cdd_pool_string_safe_len(
                                    tree, (const char *)vlas[num_vlas - 1].name,
                                    vlas[num_vlas - 1].length, &pooled_vla));
            bld.error_state = cdd_cst_bld_ident(&bld, pooled_vla);
            bld.error_state = cdd_cst_bld_punct(&bld, ")");
            bld.error_state = cdd_cst_bld_punct(&bld, ";");
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
                cdd_cst_splice_children(tree, &owning_node, child_idx, 0,
                                        temp->children, temp->num_children);
            bld.error_state = (cdd_c_error_t)((int)bld.error_state |
                                              (int)cdd_cst_builder_free(&bld));
            bld.error_state =
                (cdd_c_error_t)((int)bld.error_state |
                                (int)cdd_cst_free_node_only(temp));
            num_vlas--;
            appended = 1;
          }
        }

        if (appended) {
          char *heap_buf;
          const char *pooled = NULL;
#if defined(_MSC_VER)
          strcat_s(p, sizeof(buf) - (size_t)(p - buf), " }");
#else
          strcat(p, " }");
#endif
          rc = gnu_malloc(strlen(buf) + 1, (void **)&heap_buf);
          if (rc != CDD_C_SUCCESS)
            return rc;
#if defined(_MSC_VER)
          strcpy_s(heap_buf, strlen(buf) + 1, buf);
#else
          strcpy(heap_buf, buf);
#endif
          rc = cdd_pool_string_safe(tree, heap_buf, &pooled);
          if (rc != CDD_C_SUCCESS) {
            C_CDD_FREE(heap_buf);
            return rc;
          }
          rc = replace_token_with_text(tree, t, t->kind, pooled,
                                       strlen(heap_buf));
          C_CDD_FREE(heap_buf);
          if (rc != CDD_C_SUCCESS) {
            return rc;
          }
        }
      }
      current_depth--;
      if (current_depth == 0) {
        enclosing_fn_ret[0] = '\0';
      }
    } else if (t->kind == CDD_TOKEN_IDENTIFIER &&
               ((t->length == 5 && memcmp(t->start, "break", 5) == 0) ||
                (t->length == 8 && memcmp(t->start, "continue", 8) == 0)) &&
               i + 1 < tree->base_tokens->size &&
               tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_SEMICOLON &&
               num_ctrl > 0 &&
               (num_cleanups > 0 ||
                (config && config->fallback_vla_to_malloc && num_vlas > 0))) {
      int is_cont = (t->length == 8);
      int target_depth = -1;
      size_t m;
      for (m = num_ctrl; m > 0; m--) {
        if (is_cont && ctrl_stack[m - 1].is_switch)
          continue;
        target_depth = ctrl_stack[m - 1].depth;
        break;
      }
      if (target_depth > 0) {
        char ubuf[4096];
        char *up = ubuf;
        int any_unwind = 0;
        size_t c_idx;
        size_t v_idx;
        ubuf[0] = '\0';
        up += CDD_SNPRINTF(up, sizeof(ubuf), "{ ");
        for (c_idx = num_cleanups; c_idx > 0; c_idx--) {
          if (cleanups[c_idx - 1].depth >= target_depth) {
            up += CDD_SNPRINTF(up, sizeof(ubuf) - (size_t)(up - ubuf),
                               "%.*s(&%.*s); ",
                               (int)cleanups[c_idx - 1].func_length,
                               cleanups[c_idx - 1].func_name,
                               (int)cleanups[c_idx - 1].var_length,
                               cleanups[c_idx - 1].var_name);
            any_unwind = 1;
          }
        }
        if (config && config->fallback_vla_to_malloc) {
          for (v_idx = num_vlas; v_idx > 0; v_idx--) {
            if (vlas[v_idx - 1].depth >= target_depth) {
              up += CDD_SNPRINTF(up, sizeof(ubuf) - (size_t)(up - ubuf),
                                 "free(%.*s); ", (int)vlas[v_idx - 1].length,
                                 vlas[v_idx - 1].name);
              any_unwind = 1;
            }
          }
        }
        if (any_unwind) {
          const char *pooled_kw = NULL;
          up += CDD_SNPRINTF(up, sizeof(ubuf) - (size_t)(up - ubuf), "%s",
                             is_cont ? "continue" : "break");
          rc = cdd_pool_string_safe(tree, ubuf, &pooled_kw);
          if (rc != CDD_C_SUCCESS)
            return rc;
          rc = replace_token_with_text(tree, t, t->kind, pooled_kw,
                                       strlen(pooled_kw));
          if (rc != CDD_C_SUCCESS)
            return rc;
          rc = replace_token_with_text(tree, &tree->base_tokens->tokens[i + 1],
                                       CDD_TOKEN_SEMICOLON, "; }", 3);
          if (rc != CDD_C_SUCCESS)
            return rc;
        }
      }
    } else if (t->kind == CDD_TOKEN_KEYWORD_RETURN) {
      if (num_cleanups > 0 ||
          (config && config->fallback_vla_to_malloc && num_vlas > 0)) {
        size_t k;
        for (k = i + 1; k < tree->base_tokens->size; k++) {
          if (tree->base_tokens->tokens[k].kind == CDD_TOKEN_SEMICOLON) {
            int has_expr = (k > i + 1);
            char buf[4096] = {0};
            char *p = buf;
            size_t c_idx, v_idx;

            if (has_expr) {
              char rtype[160];
              char open_ret[224];
              const char *pooled_open = NULL;
              rtype[0] = '\0';
              if (enclosing_fn_ret[0] != '\0' &&
                  strcmp(enclosing_fn_ret, "void") != 0) {
                CDD_SNPRINTF(rtype, sizeof(rtype), "%s", enclosing_fn_ret);
              } else {
                size_t rc_cidx = 0;
                cdd_cst_node_t *ret_owner = NULL;
                if (gnu_find_node(tree->root, t, &rc_cidx, &ret_owner) !=
                    CDD_C_SUCCESS) {
                  ret_owner = NULL;
                }
                if (cdd_eval_expr_type_scoped(
                        ctx_p, ret_owner, &tree->base_tokens->tokens[i + 1],
                        k - (i + 1), CDD_CST_TYPE_MODE_RVALUE, rtype,
                        sizeof(rtype), NULL) != CDD_C_SUCCESS ||
                    rtype[0] == '\0' || strcmp(rtype, "void") == 0) {
                  CDD_SNPRINTF(rtype, sizeof(rtype), "int");
                }
              }
              CDD_SNPRINTF(open_ret, sizeof(open_ret), "{ %s __cdd_ret = (",
                           rtype);
              rc = cdd_pool_string_safe(tree, open_ret, &pooled_open);
              if (rc != CDD_C_SUCCESS)
                return rc;
              rc = replace_token_with_text(tree, t, t->kind, pooled_open,
                                           strlen(pooled_open));
              if (rc != CDD_C_SUCCESS)
                return rc;
#if defined(_MSC_VER)
              strcpy_s(p, sizeof(buf) - (size_t)(p - buf), "); ");
#else
              strcpy(p, "); ");
#endif
              p += 3;
            } else {
              rc = replace_token_with_text(tree, t, t->kind, "{ ", 2);
              if (rc != CDD_C_SUCCESS)
                return rc;
            }

            for (c_idx = num_cleanups; c_idx > 0; c_idx--) {
              p += CDD_SNPRINTF(p, sizeof(buf) - (size_t)(p - buf),
                                "%.*s(&%.*s); ",
                                (int)cleanups[c_idx - 1].func_length,
                                cleanups[c_idx - 1].func_name,
                                (int)cleanups[c_idx - 1].var_length,
                                cleanups[c_idx - 1].var_name);
            }
            if (config && config->fallback_vla_to_malloc) {
              for (v_idx = num_vlas; v_idx > 0; v_idx--) {
                p += CDD_SNPRINTF(p, sizeof(buf) - (size_t)(p - buf),
                                  "free(%.*s); ", (int)vlas[v_idx - 1].length,
                                  vlas[v_idx - 1].name);
              }
            }

            if (has_expr) {
#if defined(_MSC_VER)
              strcpy_s(p, sizeof(buf) - (size_t)(p - buf),
                       "return __cdd_ret; }");
#else
              strcpy(p, "return __cdd_ret; }");
#endif
              p += 19;
            } else {
#if defined(_MSC_VER)
              strcpy_s(p, sizeof(buf) - (size_t)(p - buf), "return; }");
#else
              strcpy(p, "return; }");
#endif
              p += 9;
            }
            {
              char *dup;
              const char *pooled = NULL;
              rc = gnu_malloc(strlen(buf) + 1, (void **)&dup);
              if (rc != CDD_C_SUCCESS)
                return rc;
#if defined(_MSC_VER)
              strcpy_s(dup, strlen(buf) + 1, buf);
#else
              strcpy(dup, buf);
#endif
              rc = cdd_pool_string_safe(tree, dup, &pooled);
              C_CDD_FREE(dup);
              if (rc != CDD_C_SUCCESS)
                return rc;
              rc = replace_token_with_text(tree, &tree->base_tokens->tokens[k],
                                           tree->base_tokens->tokens[k].kind,
                                           pooled, strlen(buf));
              if (rc != CDD_C_SUCCESS)
                return rc;
              if (k + 1 < tree->base_tokens->size &&
                  tree->base_tokens->tokens[k + 1].kind == CDD_TOKEN_RBRACE &&
                  (tree->base_tokens->tokens[i - 1].kind == CDD_TOKEN_LBRACE ||
                   tree->base_tokens->tokens[i - 1].kind ==
                       CDD_TOKEN_SEMICOLON ||
                   tree->base_tokens->tokens[i - 1].kind == CDD_TOKEN_RBRACE)) {
                last_uncond_ret_rbrace = k + 1;
              }
            }
            break;
          }
        }
      }
    } else if (t->kind == CDD_TOKEN_KEYWORD_GOTO) {
      if (num_cleanups > 0) {
        rc = replace_token_with_text(
            tree, t, t->kind,
            "/* warning: goto cross-scope cleanups unsupported */ goto", 57);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }

      if (num_vlas > 0) {
        rc = replace_token_with_text(
            tree, t, t->kind,
            "/* warning: goto crossing VLA scopes unsupported */ goto", 56);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
    } else if (t->kind == CDD_TOKEN_IDENTIFIER && t->length == 7 &&
               memcmp(t->start, "longjmp", 7) == 0) {
      if (num_cleanups > 0) {
        rc = replace_token_with_text(
            tree, t, t->kind,
            "/* warning: longjmp bypasses cleanups */ longjmp", 48);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
    } else if (t->kind == CDD_TOKEN_KEYWORD___LABEL__) {
      rc = replace_token_with_text(tree, t, t->kind, "", 0);
      if (rc != CDD_C_SUCCESS)
        return rc;
      i++;
      while (i < tree->base_tokens->size) {
        cdd_token_t *nt = &tree->base_tokens->tokens[i];
        if (nt->kind == CDD_TOKEN_IDENTIFIER) {
          if (num_local_labels < MAX_LOCAL_LABELS) {
            local_labels[num_local_labels].name = nt->start;
            local_labels[num_local_labels].length = nt->length;
            local_labels[num_local_labels].depth = current_depth;
            {
              char *p = local_labels[num_local_labels].rename;
#if defined(_MSC_VER)
              strcpy_s(p, sizeof(local_labels[num_local_labels].rename),
                       "__cdd_ll_");
#else
              strcpy(p, "__cdd_ll_");
#endif
              p += 9;
              memcpy(p, nt->start, nt->length);
              p += nt->length;
              *p++ = '_';
              rc = (cdd_c_error_t)((int)rc |
                                   (int)cdd_append_int(p, ++label_counter, &p));
            }
            num_local_labels++;
          }
          rc = replace_token_with_text(tree, nt, nt->kind, "", 0);
          if (rc != CDD_C_SUCCESS)
            return rc;
        } else if (nt->kind == CDD_TOKEN_COMMA) {
          rc = replace_token_with_text(tree, nt, nt->kind, "", 0);
          if (rc != CDD_C_SUCCESS)
            return rc;
        } else if (nt->kind == CDD_TOKEN_SEMICOLON) {
          rc = replace_token_with_text(tree, nt, nt->kind, "", 0);
          if (rc != CDD_C_SUCCESS)
            return rc;
          break;
        } else {
          break;
        }
        i++;
      }
    } else if (t->kind == CDD_TOKEN_IDENTIFIER && num_local_labels > 0) {
      int is_label_ref = 0;
      if (tree->base_tokens->tokens[i - 1].kind == CDD_TOKEN_KEYWORD_GOTO) {
        is_label_ref = 1;
      } else if (tree->base_tokens->tokens[i - 1].length == 1 &&
                 tree->base_tokens->tokens[i - 1].start[0] == '&' &&
                 tree->base_tokens->tokens[i - 2].length == 1 &&
                 tree->base_tokens->tokens[i - 2].start[0] == '&') {
        is_label_ref = 1;
      } else if (i + 1 < tree->base_tokens->size &&
                 tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_COLON) {
        is_label_ref = 1;
      }

      if (is_label_ref) {
        size_t j;
        for (j = num_local_labels; j-- > 0;) {
          if (local_labels[j].length == t->length &&
              memcmp(local_labels[j].name, t->start, t->length) == 0) {
            char *dup;
            const char *pooled = NULL;
            rc = gnu_malloc(strlen(local_labels[j].rename) + 1, (void **)&dup);
            if (rc != CDD_C_SUCCESS)
              return rc;
#if defined(_MSC_VER)
            strcpy_s(dup, strlen(local_labels[j].rename) + 1,
                     local_labels[j].rename);
#else
            strcpy(dup, local_labels[j].rename);
#endif
            rc = cdd_pool_string_safe(tree, dup, &pooled);
            C_CDD_FREE(dup);
            if (rc != CDD_C_SUCCESS)
              return rc;
            rc = replace_token_with_text(tree, t, t->kind, pooled,
                                         strlen(local_labels[j].rename));
            if (rc != CDD_C_SUCCESS)
              return rc;
            break;
          }
        }
      }
    }

    if (t->kind == CDD_TOKEN_LBRACKET && i > 0 &&
        i + 2 < tree->base_tokens->size) {
      cdd_token_t *prev = &tree->base_tokens->tokens[i - 1];
      cdd_token_t *next = &tree->base_tokens->tokens[i + 1];
      cdd_token_t *semi = &tree->base_tokens->tokens[i + 2];
      if (prev->kind == CDD_TOKEN_IDENTIFIER &&
          next->kind == CDD_TOKEN_IDENTIFIER) {
        int dims = 0;
        size_t k = i;
        char size_expr[256] = {0};
        char *p_sz = size_expr;
        while (k + 2 < tree->base_tokens->size &&
               tree->base_tokens->tokens[k].kind == CDD_TOKEN_LBRACKET &&
               tree->base_tokens->tokens[k + 1].kind == CDD_TOKEN_IDENTIFIER &&
               tree->base_tokens->tokens[k + 2].kind == CDD_TOKEN_RBRACKET) {
          if (dims > 0) {
            *p_sz++ = ' ';
            *p_sz++ = '*';
            *p_sz++ = ' ';
          }
          memcpy(p_sz, tree->base_tokens->tokens[k + 1].start,
                 tree->base_tokens->tokens[k + 1].length);
          p_sz += tree->base_tokens->tokens[k + 1].length;
          dims++;
          k += 3;
        }
        *p_sz = '\0';
        if (dims > 0 && k < tree->base_tokens->size) {
          cdd_token_t *end_tok = &tree->base_tokens->tokens[k];
          if (end_tok->kind == CDD_TOKEN_SEMICOLON) {
            if (config && config->fallback_vla_to_malloc) {
              {
                size_t child_idx;
                cdd_cst_node_t *owning_node = NULL;
                rc = gnu_find_node(tree->root, prev, &child_idx, &owning_node);
                if (rc != CDD_C_SUCCESS)
                  return rc;
                {
                  cdd_cst_node_t *temp = NULL;
                  cdd_cst_builder_t bld;
                  const char *pooled_prev1 = NULL;
                  const char *pooled_prev2 = NULL;
                  rc = gnu_alloc_node(CDD_CST_UNKNOWN, &temp);
                  if (rc != CDD_C_SUCCESS)
                    return rc;
                  bld.error_state = cdd_cst_builder_init(&bld, tree, temp);
                  bld.error_state = cdd_cst_bld_punct(&bld, "*");
                  bld.error_state =
                      (cdd_c_error_t)((int)bld.error_state |
                                      (int)cdd_pool_string_safe_len(
                                          tree, (const char *)prev->start,
                                          prev->length, &pooled_prev1));
                  bld.error_state = cdd_cst_bld_ident(&bld, pooled_prev1);
                  bld.error_state = cdd_cst_bld_space(&bld);
                  bld.error_state = cdd_cst_bld_punct(&bld, "=");
                  bld.error_state = cdd_cst_bld_space(&bld);
                  bld.error_state = cdd_cst_bld_ident(&bld, "malloc");
                  bld.error_state = cdd_cst_bld_punct(&bld, "(");
                  bld.error_state = cdd_cst_bld_punct(&bld, "(");
                  bld.error_state = cdd_cst_bld_snippet(&bld, size_expr);
                  bld.error_state = cdd_cst_bld_punct(&bld, ")");
                  bld.error_state = cdd_cst_bld_space(&bld);
                  bld.error_state = cdd_cst_bld_punct(&bld, "*");
                  bld.error_state = cdd_cst_bld_space(&bld);
                  bld.error_state = cdd_cst_bld_ident(&bld, "sizeof");
                  bld.error_state = cdd_cst_bld_punct(&bld, "(");
                  bld.error_state = cdd_cst_bld_punct(&bld, "*");
                  bld.error_state =
                      (cdd_c_error_t)((int)bld.error_state |
                                      (int)cdd_pool_string_safe_len(
                                          tree, (const char *)prev->start,
                                          prev->length, &pooled_prev2));
                  bld.error_state = cdd_cst_bld_ident(&bld, pooled_prev2);
                  bld.error_state = cdd_cst_bld_punct(&bld, ")");
                  bld.error_state = cdd_cst_bld_punct(&bld, ")");
                  bld.error_state = cdd_cst_bld_punct(&bld, ";");
                  if (bld.error_state != 0) {
                    bld.error_state =
                        (cdd_c_error_t)((int)bld.error_state |
                                        (int)cdd_cst_builder_free(&bld));
                    bld.error_state =
                        (cdd_c_error_t)((int)bld.error_state |
                                        (int)cdd_cst_free_node_only(temp));
                    return CDD_C_ERROR_MEMORY;
                  }
                  bld.error_state = cdd_cst_splice_children(
                      tree, &owning_node, child_idx, 0, temp->children,
                      temp->num_children);
                  bld.error_state =
                      (cdd_c_error_t)((int)bld.error_state |
                                      (int)cdd_cst_builder_free(&bld));
                  bld.error_state =
                      (cdd_c_error_t)((int)bld.error_state |
                                      (int)cdd_cst_free_node_only(temp));
                }
              }
              if (num_vlas < MAX_VLAS) {
                vlas[num_vlas].name = prev->start;
                vlas[num_vlas].length = prev->length;
                vlas[num_vlas].depth = current_depth;
                num_vlas++;
              }
              rc = replace_token_with_text(tree, prev, prev->kind, "", 0);
              if (rc != CDD_C_SUCCESS)
                return rc;
              rc = replace_token_with_text(tree, end_tok, end_tok->kind, "", 0);
              if (rc != CDD_C_SUCCESS)
                return rc;
            } else {
              {
                size_t child_idx;
                cdd_cst_node_t *owning_node = NULL;
                rc = gnu_find_node(tree->root, prev, &child_idx, &owning_node);
                if (rc != CDD_C_SUCCESS)
                  return rc;
                {
                  cdd_cst_node_t *temp = NULL;
                  cdd_cst_builder_t bld;
                  const char *pooled_prev1 = NULL;
                  const char *pooled_prev2 = NULL;
                  rc = gnu_alloc_node(CDD_CST_UNKNOWN, &temp);
                  if (rc != CDD_C_SUCCESS)
                    return rc;
                  bld.error_state = cdd_cst_builder_init(&bld, tree, temp);
                  bld.error_state = cdd_cst_bld_punct(&bld, "*");
                  bld.error_state =
                      (cdd_c_error_t)((int)bld.error_state |
                                      (int)cdd_pool_string_safe_len(
                                          tree, (const char *)prev->start,
                                          prev->length, &pooled_prev1));
                  bld.error_state = cdd_cst_bld_ident(&bld, pooled_prev1);
                  bld.error_state = cdd_cst_bld_space(&bld);
                  bld.error_state = cdd_cst_bld_punct(&bld, "=");
                  bld.error_state = cdd_cst_bld_space(&bld);
                  bld.error_state = cdd_cst_bld_ident(&bld, "alloca");
                  bld.error_state = cdd_cst_bld_punct(&bld, "(");
                  bld.error_state = cdd_cst_bld_punct(&bld, "(");
                  bld.error_state = cdd_cst_bld_snippet(&bld, size_expr);
                  bld.error_state = cdd_cst_bld_punct(&bld, ")");
                  bld.error_state = cdd_cst_bld_space(&bld);
                  bld.error_state = cdd_cst_bld_punct(&bld, "*");
                  bld.error_state = cdd_cst_bld_space(&bld);
                  bld.error_state = cdd_cst_bld_ident(&bld, "sizeof");
                  bld.error_state = cdd_cst_bld_punct(&bld, "(");
                  bld.error_state = cdd_cst_bld_punct(&bld, "*");
                  bld.error_state =
                      (cdd_c_error_t)((int)bld.error_state |
                                      (int)cdd_pool_string_safe_len(
                                          tree, (const char *)prev->start,
                                          prev->length, &pooled_prev2));
                  bld.error_state = cdd_cst_bld_ident(&bld, pooled_prev2);
                  bld.error_state = cdd_cst_bld_punct(&bld, ")");
                  bld.error_state = cdd_cst_bld_punct(&bld, ")");
                  bld.error_state = cdd_cst_bld_punct(&bld, ";");
                  if (bld.error_state != 0) {
                    bld.error_state =
                        (cdd_c_error_t)((int)bld.error_state |
                                        (int)cdd_cst_builder_free(&bld));
                    bld.error_state =
                        (cdd_c_error_t)((int)bld.error_state |
                                        (int)cdd_cst_free_node_only(temp));
                    return CDD_C_ERROR_MEMORY;
                  }
                  bld.error_state = cdd_cst_splice_children(
                      tree, &owning_node, child_idx, 0, temp->children,
                      temp->num_children);
                  bld.error_state =
                      (cdd_c_error_t)((int)bld.error_state |
                                      (int)cdd_cst_builder_free(&bld));
                  bld.error_state =
                      (cdd_c_error_t)((int)bld.error_state |
                                      (int)cdd_cst_free_node_only(temp));
                }
                rc = replace_token_with_text(tree, prev, prev->kind, "", 0);
                if (rc != CDD_C_SUCCESS)
                  return rc;
                rc = replace_token_with_text(tree, end_tok, end_tok->kind, "",
                                             0);
                if (rc != CDD_C_SUCCESS)
                  return rc;
              }
            }
            {
              size_t wipe;
              for (wipe = i; wipe < k; wipe++) {
                tree->base_tokens->tokens[wipe].length = 0;
              }
            }
          } else if (end_tok->kind == CDD_TOKEN_COMMA ||
                     end_tok->kind == CDD_TOKEN_RPAREN) {
            /* VLA parameter */
            size_t wipe;
            for (wipe = i; wipe < k; wipe++) {
              if (tree->base_tokens->tokens[wipe].kind ==
                  CDD_TOKEN_IDENTIFIER) {
                tree->base_tokens->tokens[wipe].length = 0;
              }
            }
          }
        }
      } else if (i >= 2 && prev->kind == CDD_TOKEN_IDENTIFIER &&
                 tree->base_tokens->tokens[i - 2].kind != CDD_TOKEN_SEMICOLON &&
                 tree->base_tokens->tokens[i - 2].kind != CDD_TOKEN_LBRACE &&
                 tree->base_tokens->tokens[i - 2].kind != CDD_TOKEN_RBRACE &&
                 tree->base_tokens->tokens[i - 2].kind != CDD_TOKEN_LPAREN &&
                 tree->base_tokens->tokens[i - 2].kind != CDD_TOKEN_RPAREN &&
                 tree->base_tokens->tokens[i - 2].kind != CDD_TOKEN_ASSIGN &&
                 next->kind == CDD_TOKEN_NUMBER && next->length == 1 &&
                 next->start[0] == '0' && semi->kind == CDD_TOKEN_RBRACKET &&
                 i + 3 < tree->base_tokens->size &&
                 tree->base_tokens->tokens[i + 3].kind == CDD_TOKEN_SEMICOLON) {
        int found_rbrace =
            (i + 4 < tree->base_tokens->size &&
             tree->base_tokens->tokens[i + 4].kind == CDD_TOKEN_RBRACE);
        if (found_rbrace) {
          rc = replace_token_with_text(tree, next, next->kind, "1", 1);
          if (rc != CDD_C_SUCCESS)
            return rc;
        } else {
          rc = replace_token_with_text(
              tree, next, next->kind,
              "-1 /* zero-length array in middle of struct */", 46);
          if (rc != CDD_C_SUCCESS)
            return rc;
        }
      }
    }
  }

  /* Lower GNU `&&label` and `goto *expr` per function body */
  {
    size_t fi = 0;
    while (fi < tree->base_tokens->size) {
      if (tree->base_tokens->tokens[fi].kind == CDD_TOKEN_LBRACE && fi > 0 &&
          tree->base_tokens->tokens[fi - 1].kind == CDD_TOKEN_RPAREN) {
        size_t fn_lb = fi;
        size_t fn_rb = 0;
        int bd = 0;
        size_t k;
        for (k = fn_lb; k < tree->base_tokens->size; k++) {
          if (tree->base_tokens->tokens[k].kind == CDD_TOKEN_LBRACE)
            bd++;
          else if (tree->base_tokens->tokens[k].kind == CDD_TOKEN_RBRACE) {
            bd--;
            if (bd == 0) {
              fn_rb = k;
              break;
            }
          }
        }
        if (fn_rb > fn_lb) {
          char lbl_names[64][128];
          size_t num_lbls = 0;
          int has_cgoto = 0;
          int has_addr_lbl = 0;

          /* Collect defined labels & detect `goto *` / `&&lbl` */
          for (k = fn_lb + 1; k < fn_rb; k++) {
            cdd_token_t *tk = &tree->base_tokens->tokens[k];
            if (tk->kind == CDD_TOKEN_KEYWORD_GOTO && k + 1 < fn_rb &&
                tree->base_tokens->tokens[k + 1].kind == CDD_TOKEN_STAR) {
              has_cgoto = 1;
            }
            if (tk->kind == CDD_TOKEN_IDENTIFIER && k + 1 < fn_rb &&
                tree->base_tokens->tokens[k + 1].kind == CDD_TOKEN_COLON &&
                !(tk->length == 7 && memcmp(tk->start, "default", 7) == 0) &&
                num_lbls < 64 && tk->length < 127) {
              size_t li;
              int exists = 0;
              for (li = 0; li < num_lbls; li++) {
                if (strlen(lbl_names[li]) == tk->length &&
                    memcmp(lbl_names[li], tk->start, tk->length) == 0) {
                  exists = 1;
                  break;
                }
              }
              if (!exists) {
                memcpy(lbl_names[num_lbls], tk->start, tk->length);
                lbl_names[num_lbls][tk->length] = '\0';
                num_lbls++;
              }
            }
          }

          for (k = fn_lb + 1; k + 2 < fn_rb; k++) {
            cdd_token_t *t0 = &tree->base_tokens->tokens[k];
            cdd_token_t *t1 = &tree->base_tokens->tokens[k + 1];
            cdd_token_t *t2 = &tree->base_tokens->tokens[k + 2];
            if (t0->length == 1 && t0->start[0] == '&' && t1->length == 1 &&
                t1->start[0] == '&' && t1->offset == t0->offset + 1 &&
                t2->kind == CDD_TOKEN_IDENTIFIER && t2->length < 127) {
              size_t li;
              int is_lbl = 0;
              for (li = 0; li < num_lbls; li++) {
                if (strlen(lbl_names[li]) == t2->length &&
                    memcmp(lbl_names[li], t2->start, t2->length) == 0) {
                  is_lbl = 1;
                  break;
                }
              }
              if (!is_lbl) {
                enum cdd_token_kind_t pk =
                    tree->base_tokens->tokens[k - 1].kind;
                if (pk == CDD_TOKEN_ASSIGN || pk == CDD_TOKEN_COMMA ||
                    pk == CDD_TOKEN_LBRACE || pk == CDD_TOKEN_LBRACKET ||
                    pk == CDD_TOKEN_COLON || pk == CDD_TOKEN_KEYWORD_RETURN) {
                  if (num_lbls < 64) {
                    memcpy(lbl_names[num_lbls], t2->start, t2->length);
                    lbl_names[num_lbls][t2->length] = '\0';
                    num_lbls++;
                    is_lbl = 1;
                  }
                }
              }
              if (is_lbl)
                has_addr_lbl = 1;
            }
          }

          if (has_addr_lbl) {
            for (k = fn_lb + 1; k + 2 < fn_rb; k++) {
              cdd_token_t *t0 = &tree->base_tokens->tokens[k];
              cdd_token_t *t1 = &tree->base_tokens->tokens[k + 1];
              cdd_token_t *t2 = &tree->base_tokens->tokens[k + 2];
              if (t0->length == 1 && t0->start[0] == '&' && t1->length == 1 &&
                  t1->start[0] == '&' && t1->offset == t0->offset + 1 &&
                  t2->kind == CDD_TOKEN_IDENTIFIER) {
                size_t li;
                for (li = 0; li < num_lbls; li++) {
                  if (strlen(lbl_names[li]) == t2->length &&
                      memcmp(lbl_names[li], t2->start, t2->length) == 0) {
                    char id_buf[64];
                    const char *pooled_id = NULL;
                    CDD_SNPRINTF(id_buf, sizeof(id_buf), "((void *)(size_t)%u)",
                                 (unsigned int)(li + 1));
                    rc = cdd_pool_string_safe(tree, id_buf, &pooled_id);
                    if (rc != CDD_C_SUCCESS)
                      return rc;
                    rc = replace_token_with_text(tree, t0, CDD_TOKEN_IDENTIFIER,
                                                 pooled_id, strlen(pooled_id));
                    if (rc != CDD_C_SUCCESS)
                      return rc;
                    rc = replace_token_with_text(tree, t1, t1->kind, "", 0);
                    if (rc != CDD_C_SUCCESS)
                      return rc;
                    rc = replace_token_with_text(tree, t2, t2->kind, "", 0);
                    if (rc != CDD_C_SUCCESS)
                      return rc;
                    k += 2;
                    break;
                  }
                }
              }
            }
          }

          if (has_cgoto) {
            char *sw_buf = NULL;
            size_t sw_cap = num_lbls * 160 + 256;
            size_t sw_len = 0;
            size_t li;
            const char *pooled_lb = NULL;
            const char *pooled_rb = NULL;
            char lb_buf[128];

            for (k = fn_lb + 1; k + 1 < fn_rb; k++) {
              cdd_token_t *gk = &tree->base_tokens->tokens[k];
              if (gk->kind == CDD_TOKEN_KEYWORD_GOTO &&
                  tree->base_tokens->tokens[k + 1].kind == CDD_TOKEN_STAR) {
                size_t semi_k = k + 2;
                while (semi_k < fn_rb &&
                       tree->base_tokens->tokens[semi_k].kind !=
                           CDD_TOKEN_SEMICOLON) {
                  semi_k++;
                }
                if (semi_k < fn_rb) {
                  rc = replace_token_with_text(
                      tree, gk, CDD_TOKEN_IDENTIFIER,
                      "{ __cdd_cgoto_target = (int)(size_t)(", 37);
                  if (rc != CDD_C_SUCCESS)
                    return rc;
                  rc = replace_token_with_text(
                      tree, &tree->base_tokens->tokens[k + 1], CDD_TOKEN_STAR,
                      "", 0);
                  if (rc != CDD_C_SUCCESS)
                    return rc;
                  rc = replace_token_with_text(
                      tree, &tree->base_tokens->tokens[semi_k],
                      CDD_TOKEN_SEMICOLON, "); goto __cdd_cgoto_dispatch; }",
                      31);
                  if (rc != CDD_C_SUCCESS)
                    return rc;
                  k = semi_k;
                }
              }
            }

            CDD_SNPRINTF(lb_buf, sizeof(lb_buf),
                         "%.*s int __cdd_cgoto_target = 0;",
                         (int)tree->base_tokens->tokens[fn_lb].length,
                         (const char *)tree->base_tokens->tokens[fn_lb].start);
            rc = cdd_pool_string_safe(tree, lb_buf, &pooled_lb);
            if (rc != CDD_C_SUCCESS)
              return rc;
            rc = replace_token_with_text(
                tree, &tree->base_tokens->tokens[fn_lb], CDD_TOKEN_LBRACE,
                pooled_lb, strlen(pooled_lb));
            if (rc != CDD_C_SUCCESS)
              return rc;

            rc = gnu_malloc(sw_cap, (void **)&sw_buf);
            if (rc != CDD_C_SUCCESS)
              return rc;
            sw_len += (size_t)CDD_SNPRINTF(
                sw_buf, sw_cap,
                "if (0) { __cdd_cgoto_dispatch: switch (__cdd_cgoto_target) "
                "{ ");
            for (li = 0; li < num_lbls; li++) {
              sw_len += (size_t)CDD_SNPRINTF(
                  sw_buf + sw_len, sw_cap - sw_len, "case %u: goto %s; ",
                  (unsigned int)(li + 1), lbl_names[li]);
            }
            sw_len += (size_t)CDD_SNPRINTF(
                sw_buf + sw_len, sw_cap - sw_len, "default: break; } } %.*s",
                (int)tree->base_tokens->tokens[fn_rb].length,
                (const char *)tree->base_tokens->tokens[fn_rb].start);
            rc = cdd_pool_string_safe(tree, sw_buf, &pooled_rb);
            C_CDD_FREE(sw_buf);
            if (rc != CDD_C_SUCCESS)
              return rc;
            rc = replace_token_with_text(
                tree, &tree->base_tokens->tokens[fn_rb], CDD_TOKEN_RBRACE,
                pooled_rb, strlen(pooled_rb));
            if (rc != CDD_C_SUCCESS)
              return rc;
          }
          fi = fn_rb + 1;
          continue;
        }
      }
      fi++;
    }
  }

  return CDD_C_SUCCESS;
}

/*
 * @brief Standardizes GNU scope/statement extensions without pre-built context.
 *
 * @param[in,out] tree CST tree to transform.
 * @param[in] config Optional transformation configuration.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
cdd_c_error_t gnu_standardize_unroll(cdd_cst_tree_t *tree,
                                     const cdd_transform_config_t *config) {
  struct gnu_standardizer_ctx_t local_ctx;
  memset(&local_ctx, 0, sizeof(local_ctx));
  local_ctx.config = config;
  return gnu_standardize_unroll_ctx(tree, &local_ctx);
}
