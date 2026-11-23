/**
 * @file c_lowering_legacy.c
 * @brief Legacy MSVC (2005-2017) and strict ISO C89 CST lowering passes.
 */

/* clang-format off */
#include "c_lowering_internal.h"
/* clang-format on */

/*
 * @brief Inserts `ins` immediately after the opening `{` character of `tok`.
 */
cdd_c_error_t c_low_insert_after_lbrace(cdd_cst_tree_t *tree, cdd_token_t *tok,
                                        const char *ins) {
  size_t ilen = strlen(ins);
  size_t tail = tok->length - 1;
  char *buf = NULL;
  const char *pooled = NULL;
  cdd_c_error_t rc;
  rc = gnu_malloc(1 + ilen + tail + 1, (void **)&buf);
  if (rc != CDD_C_SUCCESS)
    return rc;
  buf[0] = '{';
  memcpy(buf + 1, ins, ilen);
  if (tail > 0)
    memcpy(buf + 1 + ilen, tok->start + 1, tail);
  buf[1 + ilen + tail] = '\0';
  rc = cdd_pool_string_safe(tree, buf, &pooled);
  if (rc != CDD_C_SUCCESS) {
    C_CDD_FREE(buf);
    return rc;
  }
  C_CDD_FREE(buf);
  return replace_token_with_text(tree, tok, tok->kind, pooled, 1 + ilen + tail);
}

/**
 * @brief Checks if `tok` is `struct`, `union`, or `enum`.
 *
 * @param[in] tok Token to inspect.
 * @param[out] out_is_agg Set to 1 if aggregate/enum tag keyword.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t c_low_tok_is_agg_kw(const cdd_token_t *tok,
                                         int *out_is_agg) {
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  *out_is_agg = (tok->kind == CDD_TOKEN_KEYWORD_STRUCT) ||
                (tok->length == 5 && memcmp(tok->start, "union", 5) == 0) ||
                (tok->length == 4 && memcmp(tok->start, "enum", 4) == 0);
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks if `tok` begins a C type specifier (primitive keyword,
 * qualifier, `struct`/`union`/`enum`, or scoped typedef in `ctx->sem_env`).
 *
 * @param[in] ctx Lowering context.
 * @param[in] tok Token to inspect.
 * @param[out] out_is_type Set to 1 if `tok` is a declaration type starter.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t
c_low_tok_is_type_starter(const struct c_lowering_ctx_t *ctx,
                          const cdd_token_t *tok, int *out_is_type) {
  static const char *const k_starters[] = {
      "void",      "char",         "short",         "int",       "long",
      "float",     "double",       "signed",        "unsigned",  "_Bool",
      "bool",      "const",        "volatile",      "register",  "auto",
      "size_t",    "ssize_t",      "ptrdiff_t",     "intptr_t",  "uintptr_t",
      "int8_t",    "int16_t",      "int32_t",       "int64_t",   "uint8_t",
      "uint16_t",  "uint32_t",     "uint64_t",      "_Fcomplex", "_Dcomplex",
      "_Lcomplex", "cdd_int128_t", "cdd_uint128_t", "union",     "enum",
      "_Alignas"};
  size_t k;
  cdd_c_error_t rc;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  *out_is_type = 0;
  if (tok->length == 0)
    return CDD_C_SUCCESS;
  if (tok->kind == CDD_TOKEN_KEYWORD_INT ||
      tok->kind == CDD_TOKEN_KEYWORD_STRUCT ||
      tok->kind == CDD_TOKEN_KEYWORD___INT128) {
    *out_is_type = 1;
    return CDD_C_SUCCESS;
  }
  if (tok->kind != CDD_TOKEN_IDENTIFIER)
    return CDD_C_SUCCESS;
  if (tok->length >= 10 && memcmp(tok->start, "__declspec", 10) == 0) {
    *out_is_type = 1;
    return CDD_C_SUCCESS;
  }
  for (k = 0; k < sizeof(k_starters) / sizeof(k_starters[0]); k++) {
    size_t kl = strlen(k_starters[k]);
    if (tok->length == kl && memcmp(tok->start, k_starters[k], kl) == 0) {
      *out_is_type = 1;
      return CDD_C_SUCCESS;
    }
  }
  if (tok->length < 120) {
    char nm[128];
    cdd_cst_symbol_t *tsym = NULL;
    memcpy(nm, tok->start, tok->length);
    nm[tok->length] = '\0';
    rc = c_low_opt_scope_lookup(ctx->sem_env->global_scope, nm,
                                CDD_CST_SYMBOL_TYPEDEF, &tsym);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (tsym && tsym->kind == CDD_CST_SYMBOL_TYPEDEF) {
      *out_is_type = 1;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks if `toks[s..e_excl)` contains only constant literals/operators
 * (valid inside a strict C89 static/aggregate initializer).
 *
 * @param[in] ctx Lowering context.
 * @param[in] toks Token array.
 * @param[in] s Start index.
 * @param[in] e_excl End index (exclusive).
 * @param[out] out_is_const Set to 1 if constant expression.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t
c_low_is_const_init_expr(const struct c_lowering_ctx_t *ctx,
                         const cdd_token_t *toks, size_t s, size_t e_excl,
                         int *out_is_const) {
  size_t k;
  cdd_c_error_t rc;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  *out_is_const = 1;
  for (k = s; k < e_excl; k++) {
    const cdd_token_t *tk = &toks[k];
    if (tk->length == 0)
      continue;
    if (tk->kind == CDD_TOKEN_IDENTIFIER) {
      if (k > s && toks[k - 1].kind == CDD_TOKEN_DOT)
        continue;
      if (tk->length == 6 && memcmp(tk->start, "sizeof", 6) == 0) {
        int pd = 0;
        k++;
        while (k < e_excl) {
          if (toks[k].kind == CDD_TOKEN_LPAREN)
            pd++;
          else if (toks[k].kind == CDD_TOKEN_RPAREN) {
            pd--;
            if (pd <= 0)
              break;
          }
          k++;
        }
        continue;
      }
      if (tk->length == 4 && memcmp(tk->start, "NULL", 4) == 0)
        continue;
      if (tk->length < 120) {
        char nm[128];
        cdd_cst_symbol_t *es = NULL;
        memcpy(nm, tk->start, tk->length);
        nm[tk->length] = '\0';
        rc = c_low_opt_scope_lookup(ctx->sem_env->global_scope, nm,
                                    CDD_CST_SYMBOL_ENUM_CONSTANT, &es);
        if (rc != CDD_C_SUCCESS)
          return rc;
        if (es && (es->kind == CDD_CST_SYMBOL_ENUM_CONSTANT ||
                   es->kind == CDD_CST_SYMBOL_TYPEDEF)) {
          continue;
        }
      }
      *out_is_const = 0;
      return CDD_C_SUCCESS;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Lowers C99 compound literals `(Type){ init... }` by hoisting a
 * synthetic temporary variable (`static` at file scope, local at block scope)
 * and replacing the compound literal expression with `__cdd_cl_N`.
 *
 * @param[in,out] ctx Lowering context.
 * @return CDD_C_SUCCESS or error code.
 */
static cdd_c_error_t c_lower_compound_literals(struct c_lowering_ctx_t *ctx) {
  cdd_cst_tree_t *tree = ctx->tree;
  size_t i;
  int brace_d = 0;
  size_t blk_lb_stack[64];
  cdd_c_error_t rc = CDD_C_SUCCESS;

  for (i = 0; i + 3 < tree->base_tokens->size; i++) {
    cdd_token_t *t = &tree->base_tokens->tokens[i];
    if (t->length == 0)
      continue;
    if (t->kind == CDD_TOKEN_LBRACE) {
      if (brace_d < 64)
        blk_lb_stack[brace_d] = i;
      brace_d++;
      continue;
    }
    if (t->kind == CDD_TOKEN_RBRACE) {
      if (brace_d > 0)
        brace_d--;
      continue;
    }
    if (t->kind != CDD_TOKEN_LPAREN)
      continue;

    /* Avoid matching `fn(args) { body }` or `if (cond) { body }` */
    if (i > 0 &&
        tree->base_tokens->tokens[i - 1].kind == CDD_TOKEN_IDENTIFIER) {
      const cdd_token_t *pt = &tree->base_tokens->tokens[i - 1];
      if (!(pt->length == 6 && memcmp(pt->start, "sizeof", 6) == 0)) {
        continue;
      }
    }

    {
      size_t rp = i + 1;
      int pd = 1;
      while (rp < tree->base_tokens->size) {
        if (tree->base_tokens->tokens[rp].kind == CDD_TOKEN_LPAREN)
          pd++;
        else if (tree->base_tokens->tokens[rp].kind == CDD_TOKEN_RPAREN) {
          pd--;
          if (pd == 0)
            break;
        }
        rp++;
      }
      if (rp > i + 1 && rp + 2 < tree->base_tokens->size &&
          tree->base_tokens->tokens[rp + 1].kind == CDD_TOKEN_LBRACE) {
        char eval_ty[128];
        int is_tn = 0;
        rc = cdd_cst_eval_tokens_type(ctx->sem_env, ctx->sem_env->global_scope,
                                      &tree->base_tokens->tokens[i + 1],
                                      rp - (i + 1), CDD_CST_TYPE_MODE_EXACT,
                                      eval_ty, sizeof(eval_ty), &is_tn);
        if (rc != CDD_C_SUCCESS)
          return rc;
        if (is_tn) {
          size_t lb = rp + 1;
          size_t rb = lb + 1;
          int bd = 1;
          while (rb < tree->base_tokens->size) {
            if (tree->base_tokens->tokens[rb].kind == CDD_TOKEN_LBRACE)
              bd++;
            else if (tree->base_tokens->tokens[rb].kind == CDD_TOKEN_RBRACE) {
              bd--;
              if (bd == 0)
                break;
            }
            rb++;
          }
          if (rb < tree->base_tokens->size) {
            int cid = ++ctx->cl_idx;
            char ty_raw[128];
            char init_s[512];
            char cl_name[48];
            char decl_s[768];
            size_t br_idx = 0;
            int has_arr = 0;

            rc = c_low_slice_text(tree->base_tokens->tokens, i + 1, rp, ty_raw,
                                  sizeof(ty_raw));
            if (rc != CDD_C_SUCCESS)
              return rc;
            rc = c_low_slice_text(tree->base_tokens->tokens, lb, rb + 1, init_s,
                                  sizeof(init_s));
            if (rc != CDD_C_SUCCESS)
              return rc;
            CDD_SNPRINTF(cl_name, sizeof(cl_name), "__cdd_cl_%d", cid);

            /* Split array type `(int[2]){1, 2}` -> `int __cdd_cl_1[2]` */
            {
              char *lbr = strchr(ty_raw, '[');
              if (lbr) {
                has_arr = 1;
                br_idx = (size_t)(lbr - ty_raw);
              }
            }

            if (brace_d == 0) {
              /* File-scope compound literal -> `static` top-level temp */
              size_t stmt_s = i;
              while (stmt_s > 0 &&
                     tree->base_tokens->tokens[stmt_s - 1].kind !=
                         CDD_TOKEN_SEMICOLON &&
                     tree->base_tokens->tokens[stmt_s - 1].kind !=
                         CDD_TOKEN_RBRACE) {
                stmt_s--;
              }
              if (has_arr) {
                CDD_SNPRINTF(decl_s, sizeof(decl_s), "static %.*s %s%s = %s;\n",
                             (int)br_idx, ty_raw, cl_name, ty_raw + br_idx,
                             init_s);
              } else {
                CDD_SNPRINTF(decl_s, sizeof(decl_s), "static %s %s = %s;\n",
                             ty_raw, cl_name, init_s);
              }
              rc = c_low_prepend_tok(tree, &tree->base_tokens->tokens[stmt_s],
                                     decl_s);
              if (rc != CDD_C_SUCCESS)
                return rc;
              rc = c_low_replace_span(tree, i, rb, cl_name);
              if (rc != CDD_C_SUCCESS)
                return rc;
            } else {
              /* Block-scope compound literal: declare at top of block + assign
                 element-by-element or struct init before statement */
              size_t blk_lb = blk_lb_stack[brace_d - 1];
              size_t stmt_s = i;
              int is_const_cl = 0;
              int pd2 = 0;
              rc = c_low_is_const_init_expr(ctx, tree->base_tokens->tokens,
                                            lb + 1, rb, &is_const_cl);
              if (rc != CDD_C_SUCCESS)
                return rc;
              while (stmt_s > blk_lb + 1) {
                enum cdd_token_kind_t sk =
                    tree->base_tokens->tokens[stmt_s - 1].kind;
                if (sk == CDD_TOKEN_RPAREN)
                  pd2++;
                else if (sk == CDD_TOKEN_LPAREN) {
                  if (pd2 > 0)
                    pd2--;
                } else if (pd2 == 0 && (sk == CDD_TOKEN_SEMICOLON ||
                                        sk == CDD_TOKEN_RBRACE)) {
                  break;
                }
                stmt_s--;
              }
              if (has_arr) {
                /* Hoist `T __cdd_cl_N[M];` to top of block and assign elements
                   before statement */
                char top_d[256];
                char asgn_s[768];
                size_t apos = 0;
                size_t ek = lb + 1;
                int eidx = 0;
                const char *arr_tail = ty_raw + br_idx;
                if (strcmp(arr_tail, "[]") == 0 ||
                    strcmp(arr_tail, "[ ]") == 0) {
                  /* Count comma-separated elements */
                  int cnt = 0;
                  int epd = 0;
                  size_t ck;
                  for (ck = lb + 1; ck < rb; ck++) {
                    if (tree->base_tokens->tokens[ck].kind ==
                            CDD_TOKEN_LPAREN ||
                        tree->base_tokens->tokens[ck].kind == CDD_TOKEN_LBRACE)
                      epd++;
                    else if (tree->base_tokens->tokens[ck].kind ==
                                 CDD_TOKEN_RPAREN ||
                             tree->base_tokens->tokens[ck].kind ==
                                 CDD_TOKEN_RBRACE)
                      epd--;
                    else if (epd == 0 && tree->base_tokens->tokens[ck].kind ==
                                             CDD_TOKEN_COMMA)
                      cnt++;
                  }
                  CDD_SNPRINTF(top_d, sizeof(top_d), " %.*s %s[%d];",
                               (int)br_idx, ty_raw, cl_name, cnt + 1);
                } else {
                  CDD_SNPRINTF(top_d, sizeof(top_d), " %.*s %s%s;", (int)br_idx,
                               ty_raw, cl_name, arr_tail);
                }
                rc = c_low_insert_after_lbrace(
                    tree, &tree->base_tokens->tokens[blk_lb], top_d);
                if (rc != CDD_C_SUCCESS)
                  return rc;
                asgn_s[0] = '\0';
                while (ek < rb) {
                  size_t ee = ek;
                  int epd = 0;
                  char item_b[128];
                  while (ee < rb) {
                    if (tree->base_tokens->tokens[ee].kind ==
                            CDD_TOKEN_LPAREN ||
                        tree->base_tokens->tokens[ee].kind == CDD_TOKEN_LBRACE)
                      epd++;
                    else if (tree->base_tokens->tokens[ee].kind ==
                                 CDD_TOKEN_RPAREN ||
                             tree->base_tokens->tokens[ee].kind ==
                                 CDD_TOKEN_RBRACE)
                      epd--;
                    else if (epd == 0 && tree->base_tokens->tokens[ee].kind ==
                                             CDD_TOKEN_COMMA)
                      break;
                    ee++;
                  }
                  rc = c_low_slice_text(tree->base_tokens->tokens, ek, ee,
                                        item_b, sizeof(item_b));
                  if (rc != CDD_C_SUCCESS)
                    return rc;
                  if (item_b[0] != '\0') {
                    apos += (size_t)CDD_SNPRINTF(
                        asgn_s + apos, sizeof(asgn_s) - apos, " %s[%d] = %s;",
                        cl_name, eidx++, item_b);
                  }
                  ek = (ee < rb) ? (ee + 1) : ee;
                }
                if (apos > 0) {
                  rc = c_low_append_tok(
                      tree, &tree->base_tokens->tokens[stmt_s - 1], asgn_s);
                  if (rc != CDD_C_SUCCESS)
                    return rc;
                }
              } else {
                /* Hoist `T __cdd_cl_N;` to top of block and emit C89
                   block-scoped initialization before statement */
                char top_d[256];
                int has_dot_cl = 0;
                size_t dk;
                const char *eff_init = (lb + 1 >= rb) ? "{ 0 }" : init_s;
                for (dk = lb + 1; dk < rb; dk++) {
                  if (tree->base_tokens->tokens[dk].kind == CDD_TOKEN_DOT) {
                    has_dot_cl = 1;
                    break;
                  }
                }
                CDD_SNPRINTF(top_d, sizeof(top_d), " %s %s;", ty_raw, cl_name);
                rc = c_low_insert_after_lbrace(
                    tree, &tree->base_tokens->tokens[blk_lb], top_d);
                if (rc != CDD_C_SUCCESS)
                  return rc;
                if (!is_const_cl && strncmp(ty_raw, "struct ", 7) != 0 &&
                    strncmp(ty_raw, "union ", 6) != 0) {
                  char inner_b[384];
                  rc = c_low_slice_text(tree->base_tokens->tokens, lb + 1, rb,
                                        inner_b, sizeof(inner_b));
                  if (rc != CDD_C_SUCCESS)
                    return rc;
                  CDD_SNPRINTF(decl_s, sizeof(decl_s), " %s = (%s);", cl_name,
                               inner_b);
                } else if (has_dot_cl) {
                  size_t dpos = 0;
                  size_t ek = lb + 1;
                  dpos += (size_t)CDD_SNPRINTF(decl_s, sizeof(decl_s),
                                               " { %s __cdd_ci_%d = { 0 };",
                                               ty_raw, cid);
                  while (ek < rb) {
                    size_t ee = ek;
                    int epd = 0;
                    while (ee < rb) {
                      if (tree->base_tokens->tokens[ee].kind ==
                              CDD_TOKEN_LPAREN ||
                          tree->base_tokens->tokens[ee].kind ==
                              CDD_TOKEN_LBRACE)
                        epd++;
                      else if (tree->base_tokens->tokens[ee].kind ==
                                   CDD_TOKEN_RPAREN ||
                               tree->base_tokens->tokens[ee].kind ==
                                   CDD_TOKEN_RBRACE)
                        epd--;
                      else if (epd == 0 && tree->base_tokens->tokens[ee].kind ==
                                               CDD_TOKEN_COMMA)
                        break;
                      ee++;
                    }
                    if (ek + 2 < ee &&
                        tree->base_tokens->tokens[ek].kind == CDD_TOKEN_DOT) {
                      size_t eq_k = ek + 1;
                      while (eq_k < ee &&
                             tree->base_tokens->tokens[eq_k].kind !=
                                 CDD_TOKEN_ASSIGN)
                        eq_k++;
                      if (eq_k < ee) {
                        char path_s[128], val_s[256];
                        rc = c_low_slice_text(tree->base_tokens->tokens, ek,
                                              eq_k, path_s, sizeof(path_s));
                        if (rc != CDD_C_SUCCESS)
                          return rc;
                        rc = c_low_slice_text(tree->base_tokens->tokens,
                                              eq_k + 1, ee, val_s,
                                              sizeof(val_s));
                        if (rc != CDD_C_SUCCESS)
                          return rc;
                        dpos += (size_t)CDD_SNPRINTF(
                            decl_s + dpos, sizeof(decl_s) - dpos,
                            " __cdd_ci_%d%s = %s;", cid, path_s, val_s);
                      }
                    }
                    ek = (ee < rb) ? (ee + 1) : ee;
                  }
                  CDD_SNPRINTF(decl_s + dpos, sizeof(decl_s) - dpos,
                               " %s = __cdd_ci_%d; }", cl_name, cid);
                } else {
                  CDD_SNPRINTF(decl_s, sizeof(decl_s),
                               " { %s __cdd_ci_%d = %s; %s = __cdd_ci_%d; }",
                               ty_raw, cid, eff_init, cl_name, cid);
                }
                rc = c_low_append_tok(
                    tree, &tree->base_tokens->tokens[stmt_s - 1], decl_s);
                if (rc != CDD_C_SUCCESS)
                  return rc;
              }
              /* If compound literal was followed by `&__cdd_cl_N` or `.field`,
                 parenthesize `(__cdd_cl_N)` safely (`&__cdd_cl_N` needs bare
                 lvalue identifier!) */
              rc = c_low_replace_span(tree, i, rb, cl_name);
              if (rc != CDD_C_SUCCESS)
                return rc;
            }
            i = rb;
          }
        }
      }
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Looks up the ordered `cdd_cst_field_info_t` member layout for a
 * `struct` or `union` type string (resolving typedefs if needed).
 *
 * @param[in] ctx Lowering context.
 * @param[in] raw_type Type specifier text (e.g. `"struct Pt"` or `"pt_t"`).
 * @param[out] out_fields Linked list of struct/union fields.
 * @param[out] out_is_union Set to 1 if `union`.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t c_low_lookup_agg_fields(
    const struct c_lowering_ctx_t *ctx, const char *raw_type,
    const cdd_cst_field_info_t **out_fields, int *out_is_union) {
  char resolved[128];
  const char *tag = NULL;
  cdd_cst_symbol_t *sym = NULL;
  cdd_c_error_t rc;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  *out_fields = NULL;
  *out_is_union = 0;

  CDD_SNPRINTF(resolved, sizeof(resolved), "%s", raw_type);
  /* Strip leading `static` / `const` qualifiers */
  while (strncmp(resolved, "static ", 7) == 0 ||
         strncmp(resolved, "const ", 6) == 0) {
    size_t sk = (resolved[0] == 's') ? 7 : 6;
    memmove(resolved, resolved + sk, strlen(resolved + sk) + 1);
  }

  if (strncmp(resolved, "struct ", 7) != 0 &&
      strncmp(resolved, "union ", 6) != 0) {
    cdd_cst_symbol_t *td = NULL;
    rc = c_low_opt_scope_lookup(ctx->sem_env->global_scope, resolved,
                                CDD_CST_SYMBOL_TYPEDEF, &td);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (td && td->kind == CDD_CST_SYMBOL_TYPEDEF) {
      CDD_SNPRINTF(resolved, sizeof(resolved), "%s", td->type_str);
    }
  }

  if (strncmp(resolved, "struct ", 7) == 0) {
    tag = resolved + 7;
  } else if (strncmp(resolved, "union ", 6) == 0) {
    tag = resolved + 6;
    *out_is_union = 1;
  }
  if (!tag)
    return CDD_C_SUCCESS;

  rc = c_low_opt_scope_lookup(ctx->sem_env->global_scope, tag,
                              *out_is_union ? CDD_CST_SYMBOL_UNION_TAG
                                            : CDD_CST_SYMBOL_STRUCT_TAG,
                              &sym);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (sym && sym->fields) {
    *out_fields = sym->fields;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Lowers C99 designated initializers (`{ .y = 2, .x = 1 }`,
 * `{ [2] = 5 }`) and non-constant local aggregate initializers into C89
 * positional initializers and/or post-declaration assignments.
 *
 * @param[in,out] ctx Lowering context.
 * @return CDD_C_SUCCESS or error code.
 */
static cdd_c_error_t c_lower_designated_inits(struct c_lowering_ctx_t *ctx) {
  cdd_cst_tree_t *tree = ctx->tree;
  size_t i;
  int brace_d = 0;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  for (i = 0; i + 3 < tree->base_tokens->size; i++) {
    cdd_token_t *t = &tree->base_tokens->tokens[i];
    if (t->length == 0)
      continue;
    if (t->kind == CDD_TOKEN_LBRACE) {
      brace_d++;
      continue;
    }
    if (t->kind == CDD_TOKEN_RBRACE) {
      if (brace_d > 0)
        brace_d--;
      continue;
    }

    /* Match `= { ... } ;` declaration initializer */
    if (t->kind == CDD_TOKEN_ASSIGN &&
        tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_LBRACE) {
      size_t lb = i + 1;
      size_t rb = lb + 1;
      int bd = 1;
      int has_dot_desig = 0;
      int has_arr_desig = 0;
      int all_const = 1;

      while (rb < tree->base_tokens->size) {
        cdd_token_t *rk = &tree->base_tokens->tokens[rb];
        if (rk->kind == CDD_TOKEN_LBRACE)
          bd++;
        else if (rk->kind == CDD_TOKEN_RBRACE) {
          bd--;
          if (bd == 0)
            break;
        } else if (bd == 1) {
          if (rk->kind == CDD_TOKEN_DOT && rb + 2 < tree->base_tokens->size &&
              tree->base_tokens->tokens[rb + 1].kind == CDD_TOKEN_IDENTIFIER) {
            has_dot_desig = 1;
          } else if (rk->kind == CDD_TOKEN_LBRACKET &&
                     (tree->base_tokens->tokens[rb - 1].kind ==
                          CDD_TOKEN_LBRACE ||
                      tree->base_tokens->tokens[rb - 1].kind ==
                          CDD_TOKEN_COMMA)) {
            has_arr_desig = 1;
          }
        }
        rb++;
      }
      if (rb + 1 >= tree->base_tokens->size ||
          tree->base_tokens->tokens[rb + 1].kind != CDD_TOKEN_SEMICOLON) {
        continue;
      }

      rc = c_low_is_const_init_expr(ctx, tree->base_tokens->tokens, lb + 1, rb,
                                    &all_const);
      if (rc != CDD_C_SUCCESS)
        return rc;

      /* Also check if a local aggregate initializer inside a block has
         non-constant expressions even without designators! */
      if (!has_dot_desig && !has_arr_desig && (brace_d == 0 || all_const)) {
        i = rb + 1;
        continue;
      }

      /* Locate declaration start, variable name, and type string */
      {
        size_t ds = i;
        size_t id_idx = 0;
        int found_id = 0;
        int is_static_decl = 0;
        char var_nm[96];
        char ty_s[128];

        while (ds > 0) {
          enum cdd_token_kind_t dk = tree->base_tokens->tokens[ds - 1].kind;
          if (dk == CDD_TOKEN_SEMICOLON || dk == CDD_TOKEN_LBRACE ||
              dk == CDD_TOKEN_RBRACE)
            break;
          ds--;
        }
        {
          size_t k;
          for (k = ds; k < i; k++) {
            cdd_token_t *kt = &tree->base_tokens->tokens[k];
            if (kt->length == 6 && memcmp(kt->start, "static", 6) == 0)
              is_static_decl = 1;
            if (kt->kind == CDD_TOKEN_LBRACKET)
              break;
            if (kt->kind == CDD_TOKEN_IDENTIFIER && kt->length > 0) {
              id_idx = k;
              found_id = 1;
            }
          }
        }
        if (!found_id || tree->base_tokens->tokens[id_idx].length >= 95) {
          i = rb + 1;
          continue;
        }
        memcpy(var_nm, tree->base_tokens->tokens[id_idx].start,
               tree->base_tokens->tokens[id_idx].length);
        var_nm[tree->base_tokens->tokens[id_idx].length] = '\0';
        rc = c_low_slice_text(tree->base_tokens->tokens, ds, id_idx, ty_s,
                              sizeof(ty_s));
        if (rc != CDD_C_SUCCESS)
          return rc;

        /* Case A: Array designated or non-constant local array initializer */
        if (has_arr_desig ||
            (id_idx + 1 < i && tree->base_tokens->tokens[id_idx + 1].kind ==
                                   CDD_TOKEN_LBRACKET)) {
          if (brace_d == 0 || is_static_decl) {
            /* File/static scope constant array designators -> positional C89 */
            char vals[128][96];
            int max_idx = -1;
            int cur_idx = 0;
            int valid_pos = 1;
            size_t ek = lb + 1;
            memset(vals, 0, sizeof(vals));
            while (ek < rb) {
              size_t ee = ek;
              int epd = 0;
              while (ee < rb) {
                if (tree->base_tokens->tokens[ee].kind == CDD_TOKEN_LPAREN ||
                    tree->base_tokens->tokens[ee].kind == CDD_TOKEN_LBRACE ||
                    tree->base_tokens->tokens[ee].kind == CDD_TOKEN_LBRACKET)
                  epd++;
                else if (tree->base_tokens->tokens[ee].kind ==
                             CDD_TOKEN_RPAREN ||
                         tree->base_tokens->tokens[ee].kind ==
                             CDD_TOKEN_RBRACE ||
                         tree->base_tokens->tokens[ee].kind ==
                             CDD_TOKEN_RBRACKET)
                  epd--;
                else if (epd == 0 &&
                         tree->base_tokens->tokens[ee].kind == CDD_TOKEN_COMMA)
                  break;
                ee++;
              }
              if (ek < ee) {
                if (tree->base_tokens->tokens[ek].kind == CDD_TOKEN_LBRACKET &&
                    ek + 3 < ee &&
                    tree->base_tokens->tokens[ek + 1].kind ==
                        CDD_TOKEN_NUMBER &&
                    tree->base_tokens->tokens[ek + 2].kind ==
                        CDD_TOKEN_RBRACKET &&
                    tree->base_tokens->tokens[ek + 3].kind ==
                        CDD_TOKEN_ASSIGN) {
                  char ib[32];
                  size_t il = tree->base_tokens->tokens[ek + 1].length < 31
                                  ? tree->base_tokens->tokens[ek + 1].length
                                  : 31;
                  memcpy(ib, tree->base_tokens->tokens[ek + 1].start, il);
                  ib[il] = '\0';
                  cur_idx = atoi(ib);
                  if (cur_idx < 128) {
                    rc = c_low_slice_text(tree->base_tokens->tokens, ek + 4, ee,
                                          vals[cur_idx], sizeof(vals[0]));
                    if (rc != CDD_C_SUCCESS)
                      return rc;
                    if (cur_idx > max_idx)
                      max_idx = cur_idx;
                    cur_idx++;
                  } else {
                    valid_pos = 0;
                  }
                } else {
                  if (cur_idx < 128) {
                    rc = c_low_slice_text(tree->base_tokens->tokens, ek, ee,
                                          vals[cur_idx], sizeof(vals[0]));
                    if (rc != CDD_C_SUCCESS)
                      return rc;
                    if (cur_idx > max_idx)
                      max_idx = cur_idx;
                    cur_idx++;
                  }
                }
              }
              ek = (ee < rb) ? (ee + 1) : ee;
            }
            if (valid_pos) {
              char pos_init[2048];
              size_t ppos = 0;
              int ai;
              pos_init[0] = '\0';
              rc = c_low_replace_span(tree, lb, rb, "");
              if (rc != CDD_C_SUCCESS)
                return rc;
              ppos += (size_t)CDD_SNPRINTF(pos_init, sizeof(pos_init), "{ ");
              for (ai = 0; ai <= max_idx; ai++) {
                ppos += (size_t)CDD_SNPRINTF(
                    pos_init + ppos, sizeof(pos_init) - ppos, "%s%s",
                    vals[ai][0] != '\0' ? vals[ai] : "0",
                    ai < max_idx ? ", " : " }");
              }
              rc = c_low_prepend_tok(tree, &tree->base_tokens->tokens[lb],
                                     pos_init);
              if (rc != CDD_C_SUCCESS)
                return rc;
            }
          } else {
            /* Block-scope array designators / dynamic items -> `{ 0 };` +
               `arr[idx] = val;` statements! */
            char post_s[2048];
            size_t ppos = 0;
            int cur_idx = 0;
            size_t ek = lb + 1;
            post_s[0] = '\0';
            while (ek < rb) {
              size_t ee = ek;
              int epd = 0;
              while (ee < rb) {
                if (tree->base_tokens->tokens[ee].kind == CDD_TOKEN_LPAREN ||
                    tree->base_tokens->tokens[ee].kind == CDD_TOKEN_LBRACE ||
                    tree->base_tokens->tokens[ee].kind == CDD_TOKEN_LBRACKET)
                  epd++;
                else if (tree->base_tokens->tokens[ee].kind ==
                             CDD_TOKEN_RPAREN ||
                         tree->base_tokens->tokens[ee].kind ==
                             CDD_TOKEN_RBRACE ||
                         tree->base_tokens->tokens[ee].kind ==
                             CDD_TOKEN_RBRACKET)
                  epd--;
                else if (epd == 0 &&
                         tree->base_tokens->tokens[ee].kind == CDD_TOKEN_COMMA)
                  break;
                ee++;
              }
              if (ek < ee) {
                if (tree->base_tokens->tokens[ek].kind == CDD_TOKEN_LBRACKET) {
                  size_t rbk = ek + 1;
                  int bdk = 1;
                  while (rbk < ee) {
                    if (tree->base_tokens->tokens[rbk].kind ==
                        CDD_TOKEN_LBRACKET)
                      bdk++;
                    else if (tree->base_tokens->tokens[rbk].kind ==
                             CDD_TOKEN_RBRACKET) {
                      bdk--;
                      if (bdk == 0)
                        break;
                    }
                    rbk++;
                  }
                  if (rbk + 1 < ee && tree->base_tokens->tokens[rbk + 1].kind ==
                                          CDD_TOKEN_ASSIGN) {
                    char idx_s[128], val_s[192];
                    rc = c_low_slice_text(tree->base_tokens->tokens, ek + 1,
                                          rbk, idx_s, sizeof(idx_s));
                    if (rc != CDD_C_SUCCESS)
                      return rc;
                    rc = c_low_slice_text(tree->base_tokens->tokens, rbk + 2,
                                          ee, val_s, sizeof(val_s));
                    if (rc != CDD_C_SUCCESS)
                      return rc;
                    ppos += (size_t)CDD_SNPRINTF(
                        post_s + ppos, sizeof(post_s) - ppos, " %s[%s] = %s;",
                        var_nm, idx_s, val_s);
                  }
                } else {
                  char val_s[192];
                  rc = c_low_slice_text(tree->base_tokens->tokens, ek, ee,
                                        val_s, sizeof(val_s));
                  if (rc != CDD_C_SUCCESS)
                    return rc;
                  if (val_s[0] != '\0') {
                    ppos += (size_t)CDD_SNPRINTF(
                        post_s + ppos, sizeof(post_s) - ppos, " %s[%d] = %s;",
                        var_nm, cur_idx++, val_s);
                  }
                }
              }
              ek = (ee < rb) ? (ee + 1) : ee;
            }
            rc = c_low_replace_span(tree, lb, rb, "{ 0 }");
            if (rc != CDD_C_SUCCESS)
              return rc;
            if (ppos > 0) {
              rc = c_low_append_tok(tree, &tree->base_tokens->tokens[rb + 1],
                                    post_s);
              if (rc != CDD_C_SUCCESS)
                return rc;
            }
          }
          i = rb + 1;
          continue;
        }

        /* Case B: Struct / Union designated or non-constant local initializer
         */
        {
          const cdd_cst_field_info_t *fields = NULL;
          int is_uni = 0;
          rc = c_low_lookup_agg_fields(ctx, ty_s, &fields, &is_uni);
          if (rc != CDD_C_SUCCESS)
            return rc;
          if (brace_d > 0 && !is_static_decl &&
              (!all_const || is_uni || !fields)) {
            /* Emit `= { 0 };` + post-declaration `<var>.<field> = <val>;`! */
            char post_s[2048];
            size_t ppos = 0;
            const cdd_cst_field_info_t *cur_f = fields;
            size_t ek = lb + 1;
            post_s[0] = '\0';
            while (ek < rb) {
              size_t ee = ek;
              int epd = 0;
              while (ee < rb) {
                if (tree->base_tokens->tokens[ee].kind == CDD_TOKEN_LPAREN ||
                    tree->base_tokens->tokens[ee].kind == CDD_TOKEN_LBRACE ||
                    tree->base_tokens->tokens[ee].kind == CDD_TOKEN_LBRACKET)
                  epd++;
                else if (tree->base_tokens->tokens[ee].kind ==
                             CDD_TOKEN_RPAREN ||
                         tree->base_tokens->tokens[ee].kind ==
                             CDD_TOKEN_RBRACE ||
                         tree->base_tokens->tokens[ee].kind ==
                             CDD_TOKEN_RBRACKET)
                  epd--;
                else if (epd == 0 &&
                         tree->base_tokens->tokens[ee].kind == CDD_TOKEN_COMMA)
                  break;
                ee++;
              }
              if (ek < ee) {
                if (tree->base_tokens->tokens[ek].kind == CDD_TOKEN_DOT) {
                  /* Handle `.a = val` or chained `.a.b = val` */
                  size_t eq_k = ek + 1;
                  while (eq_k < ee && tree->base_tokens->tokens[eq_k].kind !=
                                          CDD_TOKEN_ASSIGN)
                    eq_k++;
                  if (eq_k < ee) {
                    char path_s[128], val_s[256];
                    size_t pk, p_l = 0;
                    for (pk = ek; pk < eq_k && p_l + 1 < sizeof(path_s); pk++) {
                      size_t tl = tree->base_tokens->tokens[pk].length;
                      if (p_l + tl < sizeof(path_s)) {
                        memcpy(path_s + p_l,
                               tree->base_tokens->tokens[pk].start, tl);
                        p_l += tl;
                      }
                    }
                    path_s[p_l] = '\0';
                    rc = c_low_slice_text(tree->base_tokens->tokens, eq_k + 1,
                                          ee, val_s, sizeof(val_s));
                    if (rc != CDD_C_SUCCESS)
                      return rc;
                    ppos += (size_t)CDD_SNPRINTF(
                        post_s + ppos, sizeof(post_s) - ppos, " %s%s = %s;",
                        var_nm, path_s, val_s);
                  }
                } else if (cur_f) {
                  char val_s[256];
                  rc = c_low_slice_text(tree->base_tokens->tokens, ek, ee,
                                        val_s, sizeof(val_s));
                  if (rc != CDD_C_SUCCESS)
                    return rc;
                  if (val_s[0] != '\0') {
                    ppos += (size_t)CDD_SNPRINTF(
                        post_s + ppos, sizeof(post_s) - ppos, " %s.%s = %s;",
                        var_nm, cur_f->name, val_s);
                  }
                  cur_f = cur_f->next;
                }
              }
              ek = (ee < rb) ? (ee + 1) : ee;
            }
            if (ppos > 0) {
              rc = c_low_replace_span(tree, lb, rb, "{ 0 }");
              if (rc != CDD_C_SUCCESS)
                return rc;
              rc = c_low_append_tok(tree, &tree->base_tokens->tokens[rb + 1],
                                    post_s);
              if (rc != CDD_C_SUCCESS)
                return rc;
            }
          } else if (fields && !is_uni) {
            /* Strictly ordered C89 positional struct initializer `{ v0, v1 }`
             */
            char fnames[64][64];
            char fvals[64][160];
            size_t nf = 0;
            size_t cur_pos_f = 0;
            const cdd_cst_field_info_t *fp;
            size_t ek = lb + 1;
            memset(fvals, 0, sizeof(fvals));
            for (fp = fields; fp && nf < 64; fp = fp->next) {
              CDD_SNPRINTF(fnames[nf++], sizeof(fnames[0]), "%s", fp->name);
            }
            while (ek < rb) {
              size_t ee = ek;
              int epd = 0;
              while (ee < rb) {
                if (tree->base_tokens->tokens[ee].kind == CDD_TOKEN_LPAREN ||
                    tree->base_tokens->tokens[ee].kind == CDD_TOKEN_LBRACE ||
                    tree->base_tokens->tokens[ee].kind == CDD_TOKEN_LBRACKET)
                  epd++;
                else if (tree->base_tokens->tokens[ee].kind ==
                             CDD_TOKEN_RPAREN ||
                         tree->base_tokens->tokens[ee].kind ==
                             CDD_TOKEN_RBRACE ||
                         tree->base_tokens->tokens[ee].kind ==
                             CDD_TOKEN_RBRACKET)
                  epd--;
                else if (epd == 0 &&
                         tree->base_tokens->tokens[ee].kind == CDD_TOKEN_COMMA)
                  break;
                ee++;
              }
              if (ek < ee) {
                if (tree->base_tokens->tokens[ek].kind == CDD_TOKEN_DOT &&
                    ek + 2 < ee &&
                    tree->base_tokens->tokens[ek + 1].kind ==
                        CDD_TOKEN_IDENTIFIER &&
                    tree->base_tokens->tokens[ek + 2].kind ==
                        CDD_TOKEN_ASSIGN) {
                  cdd_token_t *fn_t = &tree->base_tokens->tokens[ek + 1];
                  size_t fi;
                  for (fi = 0; fi < nf; fi++) {
                    if (strlen(fnames[fi]) == fn_t->length &&
                        memcmp(fnames[fi], fn_t->start, fn_t->length) == 0) {
                      rc = c_low_slice_text(tree->base_tokens->tokens, ek + 3,
                                            ee, fvals[fi], sizeof(fvals[0]));
                      if (rc != CDD_C_SUCCESS)
                        return rc;
                      cur_pos_f = fi + 1;
                      break;
                    }
                  }
                } else if (cur_pos_f < nf) {
                  rc = c_low_slice_text(tree->base_tokens->tokens, ek, ee,
                                        fvals[cur_pos_f], sizeof(fvals[0]));
                  if (rc != CDD_C_SUCCESS)
                    return rc;
                  cur_pos_f++;
                }
              }
              ek = (ee < rb) ? (ee + 1) : ee;
            }
            {
              char pos_init[2048];
              size_t ppos = 0;
              size_t fi;
              ppos += (size_t)CDD_SNPRINTF(pos_init, sizeof(pos_init), "{ ");
              for (fi = 0; fi < nf; fi++) {
                ppos += (size_t)CDD_SNPRINTF(
                    pos_init + ppos, sizeof(pos_init) - ppos, "%s%s",
                    fvals[fi][0] != '\0' ? fvals[fi] : "0",
                    fi + 1 < nf ? ", " : " }");
              }
              rc = c_low_replace_span(tree, lb, rb, pos_init);
              if (rc != CDD_C_SUCCESS)
                return rc;
            }
          }
        }
      }
      i = rb + 1;
    }
  }
  return CDD_C_SUCCESS;
}

#if defined(__DOS__) || defined(__WATCOMC__)
/** @brief Maximum nested block depth tracked during DOS for-init hoisting. */
#define CDD_FOR_BLK_DEPTH 16
/** @brief Maximum hoisted variables per block on DOS targets. */
#define CDD_FOR_BLK_VARS 8
/** @brief Maximum hoisted variable name buffer size on DOS targets. */
#define CDD_FOR_VAR_NAME_CAP 32
#else
/** @brief Maximum nested block depth tracked during for-init hoisting. */
#define CDD_FOR_BLK_DEPTH 64
/** @brief Maximum hoisted variables per block. */
#define CDD_FOR_BLK_VARS 32
/** @brief Maximum hoisted variable name buffer size. */
#define CDD_FOR_VAR_NAME_CAP 64
#endif

/**
 * @brief Hoists C99 `for (T i = 0; ...)` loop declarations and mid-block mixed
 * declarations & code to the top of their enclosing `{ ... }` block.
 *
 * @param[in,out] ctx Lowering context.
 * @return CDD_C_SUCCESS or error code.
 */
static cdd_c_error_t c_lower_mixed_decls_and_for(struct c_lowering_ctx_t *ctx) {
  cdd_cst_tree_t *tree = ctx->tree;
  size_t i;
  size_t blk_stack[CDD_FOR_BLK_DEPTH];
  char blk_vars[CDD_FOR_BLK_DEPTH][CDD_FOR_BLK_VARS][CDD_FOR_VAR_NAME_CAP];
  size_t blk_var_cnt[CDD_FOR_BLK_DEPTH];
  int depth = 0;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  memset(blk_var_cnt, 0, sizeof(blk_var_cnt));

  /* Pass 1: Hoist `for (T i = init, j = init2; ...)` loop declarations */
  for (i = 0; i < tree->base_tokens->size; i++) {
    cdd_token_t *t = &tree->base_tokens->tokens[i];
    if (t->length == 0)
      continue;
    if (t->kind == CDD_TOKEN_LBRACE) {
      if (depth < CDD_FOR_BLK_DEPTH) {
        blk_stack[depth] = i;
        blk_var_cnt[depth] = 0;
      }
      depth++;
      continue;
    }
    if (t->kind == CDD_TOKEN_RBRACE) {
      if (depth > 0)
        depth--;
      continue;
    }
    if (depth == 0 || depth > CDD_FOR_BLK_DEPTH)
      continue;

    if (t->kind == CDD_TOKEN_IDENTIFIER && t->length == 3 &&
        memcmp(t->start, "for", 3) == 0 && i + 3 < tree->base_tokens->size &&
        tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_LPAREN) {
      size_t init_s = i + 2;
      int is_ty = 0;
      rc = c_low_tok_is_type_starter(ctx, &tree->base_tokens->tokens[init_s],
                                     &is_ty);
      if (rc != CDD_C_SUCCESS)
        return rc;
      if (is_ty) {
        size_t semi = init_s;
        size_t rp_for = init_s;
        int pd = 1;
        size_t spec_e = init_s;
        char base_ty[128];
        char hoist_decls[512];
        char init_asgns[512];
        size_t hpos = 0;
        size_t apos = 0;
        int first_a = 1;

        while (semi < tree->base_tokens->size &&
               tree->base_tokens->tokens[semi].kind != CDD_TOKEN_SEMICOLON)
          semi++;
        while (rp_for < tree->base_tokens->size) {
          if (tree->base_tokens->tokens[rp_for].kind == CDD_TOKEN_LPAREN)
            pd++;
          else if (tree->base_tokens->tokens[rp_for].kind == CDD_TOKEN_RPAREN) {
            pd--;
            if (pd == 0)
              break;
          }
          rp_for++;
        }
        if (semi >= rp_for || rp_for >= tree->base_tokens->size)
          continue;

        /* Advance `spec_e` across all leading type specifiers */
        while (spec_e < semi) {
          int st_m = 0;
          if (spec_e > init_s && (tree->base_tokens->tokens[spec_e - 1].kind ==
                                  CDD_TOKEN_KEYWORD_STRUCT)) {
            spec_e++;
            continue;
          }
          rc = c_low_tok_is_type_starter(
              ctx, &tree->base_tokens->tokens[spec_e], &st_m);
          if (rc != CDD_C_SUCCESS)
            return rc;
          if (!st_m)
            break;
          /* Stop if next token is `=` or `;` or `,` (meaning current token is
             actually the declarator name!) */
          if (spec_e + 1 < semi &&
              (tree->base_tokens->tokens[spec_e + 1].kind == CDD_TOKEN_ASSIGN ||
               tree->base_tokens->tokens[spec_e + 1].kind == CDD_TOKEN_COMMA)) {
            break;
          }
          spec_e++;
        }
        if (spec_e >= semi)
          continue;

        rc = c_low_slice_text(tree->base_tokens->tokens, init_s, spec_e,
                              base_ty, sizeof(base_ty));
        if (rc != CDD_C_SUCCESS)
          return rc;
        hoist_decls[0] = '\0';
        init_asgns[0] = '\0';

        /* Parse comma-separated declarators `[*]* name [= expr]` */
        {
          size_t ds = spec_e;
          while (ds < semi) {
            size_t de = ds;
            int dpd = 0;
            size_t eq_k = 0;
            size_t nm_k = 0;
            int found_nm = 0;
            char stars[16];
            size_t st_l = 0;
            size_t k;

            while (de < semi) {
              if (tree->base_tokens->tokens[de].kind == CDD_TOKEN_LPAREN ||
                  tree->base_tokens->tokens[de].kind == CDD_TOKEN_LBRACKET)
                dpd++;
              else if (tree->base_tokens->tokens[de].kind == CDD_TOKEN_RPAREN ||
                       tree->base_tokens->tokens[de].kind == CDD_TOKEN_RBRACKET)
                dpd--;
              else if (dpd == 0 &&
                       tree->base_tokens->tokens[de].kind == CDD_TOKEN_COMMA)
                break;
              else if (dpd == 0 && eq_k == 0 &&
                       tree->base_tokens->tokens[de].kind == CDD_TOKEN_ASSIGN)
                eq_k = de;
              de++;
            }
            for (k = ds; k < (eq_k ? eq_k : de); k++) {
              if (tree->base_tokens->tokens[k].kind == CDD_TOKEN_STAR &&
                  st_l + 1 < sizeof(stars))
                stars[st_l++] = '*';
              else if (tree->base_tokens->tokens[k].kind ==
                       CDD_TOKEN_IDENTIFIER) {
                nm_k = k;
                found_nm = 1;
              }
            }
            stars[st_l] = '\0';
            if (found_nm && tree->base_tokens->tokens[nm_k].length < 56) {
              char orig_nm[64];
              char eff_nm[96];
              size_t vi;
              int collide = 0;
              memcpy(orig_nm, tree->base_tokens->tokens[nm_k].start,
                     tree->base_tokens->tokens[nm_k].length);
              orig_nm[tree->base_tokens->tokens[nm_k].length] = '\0';
              for (vi = 0; vi < blk_var_cnt[depth - 1]; vi++) {
                if (strcmp(blk_vars[depth - 1][vi], orig_nm) == 0) {
                  collide = 1;
                  break;
                }
              }
              if (collide) {
                CDD_SNPRINTF(eff_nm, sizeof(eff_nm), "__cdd_for_%s_%d", orig_nm,
                             ++ctx->for_idx);
                /* Rename `orig_nm` across `for (...)` header and body */
                {
                  size_t body_end = rp_for + 1;
                  size_t rk;
                  if (body_end < tree->base_tokens->size &&
                      tree->base_tokens->tokens[body_end].kind ==
                          CDD_TOKEN_LBRACE) {
                    int bbd = 1;
                    body_end++;
                    while (body_end < tree->base_tokens->size && bbd > 0) {
                      if (tree->base_tokens->tokens[body_end].kind ==
                          CDD_TOKEN_LBRACE)
                        bbd++;
                      else if (tree->base_tokens->tokens[body_end].kind ==
                               CDD_TOKEN_RBRACE)
                        bbd--;
                      body_end++;
                    }
                  } else {
                    while (body_end < tree->base_tokens->size &&
                           tree->base_tokens->tokens[body_end].kind !=
                               CDD_TOKEN_SEMICOLON)
                      body_end++;
                  }
                  for (rk = semi + 1; rk < body_end; rk++) {
                    cdd_token_t *rtk = &tree->base_tokens->tokens[rk];
                    if (rtk->kind == CDD_TOKEN_IDENTIFIER &&
                        rtk->length == strlen(orig_nm) &&
                        memcmp(rtk->start, orig_nm, rtk->length) == 0 &&
                        !(tree->base_tokens->tokens[rk - 1].kind ==
                              CDD_TOKEN_DOT ||
                          tree->base_tokens->tokens[rk - 1].kind ==
                              CDD_TOKEN_ARROW)) {
                      const char *pr = NULL;
                      rc = cdd_pool_string_safe(tree, eff_nm, &pr);
                      if (rc != CDD_C_SUCCESS)
                        return rc;
                      rc = replace_token_with_text(
                          tree, rtk, CDD_TOKEN_IDENTIFIER, pr, strlen(pr));
                      if (rc != CDD_C_SUCCESS)
                        return rc;
                    }
                  }
                }
              } else {
                CDD_SNPRINTF(eff_nm, sizeof(eff_nm), "%s", orig_nm);
                if (blk_var_cnt[depth - 1] < CDD_FOR_BLK_VARS) {
                  CDD_SNPRINTF(blk_vars[depth - 1][blk_var_cnt[depth - 1]++],
                               CDD_FOR_VAR_NAME_CAP, "%s", orig_nm);
                }
              }

              hpos += (size_t)CDD_SNPRINTF(hoist_decls + hpos,
                                           sizeof(hoist_decls) - hpos,
                                           " %s %s%s;", base_ty, stars, eff_nm);
              if (eq_k > 0 && eq_k + 1 < de) {
                char rhs_b[192];
                rc = c_low_slice_text(tree->base_tokens->tokens, eq_k + 1, de,
                                      rhs_b, sizeof(rhs_b));
                if (rc != CDD_C_SUCCESS)
                  return rc;
                apos += (size_t)CDD_SNPRINTF(
                    init_asgns + apos, sizeof(init_asgns) - apos, "%s%s = %s",
                    first_a ? "" : ", ", eff_nm, rhs_b);
                first_a = 0;
              }
            }
            ds = (de < semi) ? (de + 1) : de;
          }
        }

        if (hpos > 0) {
          rc = c_low_insert_after_lbrace(
              tree, &tree->base_tokens->tokens[blk_stack[depth - 1]],
              hoist_decls);
          if (rc != CDD_C_SUCCESS)
            return rc;
          rc = c_low_replace_span(tree, init_s, semi - 1, init_asgns);
          if (rc != CDD_C_SUCCESS)
            return rc;
        }
      }
    }
  }

  /* Pass 2: Hoist mid-block variable declarations inside `{ ... }` blocks */
  {
    size_t bi;
    for (bi = 0; bi < tree->base_tokens->size; bi++) {
      /* Only inspect compound statement `{ ... }` blocks inside functions (not
         `struct`/`union`/`enum` or initializer `{ ... }` blocks!) */
      if (tree->base_tokens->tokens[bi].length > 0 &&
          tree->base_tokens->tokens[bi].kind == CDD_TOKEN_LBRACE && bi > 0) {
        enum cdd_token_kind_t prev_k = tree->base_tokens->tokens[bi - 1].kind;
        size_t br_end = 0;
        int bd = 0;
        size_t cur;
        int seen_exec = (memchr(tree->base_tokens->tokens[bi].start, '=',
                                tree->base_tokens->tokens[bi].length) != NULL)
                            ? 1
                            : 0;
        char hoisted_types[1024];
        size_t hty_pos = 0;
        char hoisted_all[4096];
        size_t hall_pos = 0;

        if (prev_k == CDD_TOKEN_ASSIGN || prev_k == CDD_TOKEN_COMMA)
          continue;
        if (prev_k == CDD_TOKEN_IDENTIFIER ||
            prev_k == CDD_TOKEN_KEYWORD_STRUCT) {
          /* Skip `struct S {`, `union U {`, `enum E {` */
          int is_agg = 0;
          rc = c_low_tok_is_agg_kw(&tree->base_tokens->tokens[bi - 1], &is_agg);
          if (rc != CDD_C_SUCCESS)
            return rc;
          if (!is_agg && bi >= 2) {
            rc = c_low_tok_is_agg_kw(&tree->base_tokens->tokens[bi - 2],
                                     &is_agg);
            if (rc != CDD_C_SUCCESS)
              return rc;
          }
          if (is_agg)
            continue;
        }

        for (cur = bi; cur < tree->base_tokens->size; cur++) {
          if (tree->base_tokens->tokens[cur].length == 0)
            continue;
          if (tree->base_tokens->tokens[cur].kind == CDD_TOKEN_LBRACE)
            bd++;
          else if (tree->base_tokens->tokens[cur].kind == CDD_TOKEN_RBRACE) {
            bd--;
            if (bd == 0) {
              br_end = cur;
              break;
            }
          }
        }
        if (br_end <= bi + 1)
          continue;

        hoisted_types[0] = '\0';
        hoisted_all[0] = '\0';
        cur = bi + 1;
        while (cur < br_end) {
          /* Skip empty wiped tokens */
          while (cur < br_end && tree->base_tokens->tokens[cur].length == 0)
            cur++;
          if (cur >= br_end)
            break;

          /* Skip nested `{ ... }` block statement as an executable statement */
          if (tree->base_tokens->tokens[cur].kind == CDD_TOKEN_LBRACE) {
            int nbd = 1;
            seen_exec = 1;
            cur++;
            while (cur < br_end && nbd > 0) {
              if (tree->base_tokens->tokens[cur].kind == CDD_TOKEN_LBRACE)
                nbd++;
              else if (tree->base_tokens->tokens[cur].kind == CDD_TOKEN_RBRACE)
                nbd--;
              cur++;
            }
            continue;
          }

          /* Find end of direct statement (`;` or `{ ... }`) */
          {
            size_t st_s = cur;
            size_t st_e = cur;
            int spd = 0;
            int has_blk_stmt = 0;
            while (st_e < br_end) {
              enum cdd_token_kind_t sk = tree->base_tokens->tokens[st_e].kind;
              if (sk == CDD_TOKEN_LPAREN || sk == CDD_TOKEN_LBRACKET)
                spd++;
              else if (sk == CDD_TOKEN_RPAREN || sk == CDD_TOKEN_RBRACKET) {
                if (spd > 0)
                  spd--;
              } else if (spd == 0 && sk == CDD_TOKEN_LBRACE &&
                         tree->base_tokens->tokens[st_e - 1].kind !=
                             CDD_TOKEN_ASSIGN) {
                const cdd_token_t *s0 = &tree->base_tokens->tokens[st_s];
                int is_agg0 = 0;
                int cbd = 1;
                rc = c_low_tok_is_agg_kw(s0, &is_agg0);
                if (rc != CDD_C_SUCCESS)
                  return rc;
                st_e++;
                while (st_e < br_end && cbd > 0) {
                  if (tree->base_tokens->tokens[st_e].kind == CDD_TOKEN_LBRACE)
                    cbd++;
                  else if (tree->base_tokens->tokens[st_e].kind ==
                           CDD_TOKEN_RBRACE)
                    cbd--;
                  st_e++;
                }
                if (!(is_agg0 || (s0->length == 7 &&
                                  memcmp(s0->start, "typedef", 7) == 0))) {
                  has_blk_stmt = 1;
                  break;
                }
                continue;
              } else if (spd == 0 && sk == CDD_TOKEN_SEMICOLON) {
                break;
              }
              st_e++;
            }

            if (has_blk_stmt || st_e >= br_end) {
              seen_exec = 1;
              cur = has_blk_stmt ? st_e : (st_e + 1);
              continue;
            }

            /* Inspect simple `;`-terminated statement `[st_s .. st_e]` */
            {
              const cdd_token_t *ft = &tree->base_tokens->tokens[st_s];
              int is_decl = 0;
              int is_static_or_td = 0;
              int is_type_def = 0;
              int ft_is_agg = 0;
              rc = c_low_tok_is_agg_kw(ft, &ft_is_agg);
              if (rc != CDD_C_SUCCESS)
                return rc;

              if (ft->kind == CDD_TOKEN_IDENTIFIER &&
                  ((ft->length == 7 && memcmp(ft->start, "typedef", 7) == 0) ||
                   (ft->length == 6 && memcmp(ft->start, "static", 6) == 0) ||
                   (ft->length == 6 && memcmp(ft->start, "extern", 6) == 0))) {
                is_decl = 1;
                is_static_or_td = 1;
                if (ft->length == 7)
                  is_type_def = 1;
              } else if (ft_is_agg &&
                         tree->base_tokens->tokens[st_e - 1].kind ==
                             CDD_TOKEN_RBRACE &&
                         (tree->base_tokens->tokens[st_s + 1].kind ==
                              CDD_TOKEN_LBRACE ||
                          tree->base_tokens->tokens[st_s + 2].kind ==
                              CDD_TOKEN_LBRACE)) {
                is_decl = 1;
                is_static_or_td = 1;
                is_type_def = 1;
              } else {
                rc = c_low_tok_is_type_starter(ctx, ft, &is_decl);
                if (rc != CDD_C_SUCCESS)
                  return rc;
              }

              /* Disambiguate `my_type_t * x;` (decl) vs `var * y;` (expr): if
                 `is_decl` came from a single identifier followed by `=`/`+=`/
                 `(`/`++`/`--`/`;`, it's an expression, NOT a declaration
                 (unless `__declspec(...)` or `_Alignas(...)`)! */
              if (is_decl && !is_static_or_td &&
                  !(ft->length >= 10 &&
                    memcmp(ft->start, "__declspec", 10) == 0) &&
                  !(ft->length == 8 && memcmp(ft->start, "_Alignas", 8) == 0)) {
                enum cdd_token_kind_t nk =
                    tree->base_tokens->tokens[st_s + 1].kind;
                if (nk == CDD_TOKEN_ASSIGN || nk == CDD_TOKEN_LPAREN ||
                    nk == CDD_TOKEN_SEMICOLON || nk == CDD_TOKEN_PLUS ||
                    nk == CDD_TOKEN_MINUS || nk == CDD_TOKEN_DOT ||
                    nk == CDD_TOKEN_ARROW || nk == CDD_TOKEN_COLON) {
                  is_decl = 0;
                }
              }

              if (!is_decl) {
                seen_exec = 1;
              } else if (seen_exec) {
                if (is_static_or_td) {
                  /* Hoist `typedef`/tag defs or `static` verbatim to top */
                  char full_d[512];
                  rc = c_low_slice_text(tree->base_tokens->tokens, st_s,
                                        st_e + 1, full_d, sizeof(full_d));
                  if (rc != CDD_C_SUCCESS)
                    return rc;
                  if (is_type_def) {
                    hty_pos += (size_t)CDD_SNPRINTF(
                        hoisted_types + hty_pos,
                        sizeof(hoisted_types) - hty_pos, " %s", full_d);
                  } else {
                    hall_pos += (size_t)CDD_SNPRINTF(
                        hoisted_all + hall_pos, sizeof(hoisted_all) - hall_pos,
                        " %s", full_d);
                  }
                  rc = c_low_replace_span(tree, st_s, st_e, "");
                  if (rc != CDD_C_SUCCESS)
                    return rc;
                } else {
                  /* Split `BaseType [*]* v1 [= e1], [*]* v2[N] [= e2];` */
                  size_t sp_e = st_s;
                  char bty[128];
                  char asgns[1024];
                  size_t apos = 0;
                  while (sp_e < st_e) {
                    int stm = 0;
                    if (sp_e > st_s &&
                        (tree->base_tokens->tokens[sp_e - 1].kind ==
                             CDD_TOKEN_KEYWORD_STRUCT ||
                         (tree->base_tokens->tokens[sp_e - 1].length == 5 &&
                          memcmp(tree->base_tokens->tokens[sp_e - 1].start,
                                 "union", 5) == 0) ||
                         (tree->base_tokens->tokens[sp_e - 1].length == 4 &&
                          memcmp(tree->base_tokens->tokens[sp_e - 1].start,
                                 "enum", 4) == 0))) {
                      sp_e++;
                      continue;
                    }
                    rc = c_low_tok_is_type_starter(
                        ctx, &tree->base_tokens->tokens[sp_e], &stm);
                    if (rc != CDD_C_SUCCESS)
                      return rc;
                    if (!stm)
                      break;
                    if (sp_e + 1 < st_e &&
                        tree->base_tokens->tokens[sp_e + 1].kind ==
                            CDD_TOKEN_LPAREN &&
                        tree->base_tokens->tokens[sp_e].length == 10 &&
                        memcmp(tree->base_tokens->tokens[sp_e].start,
                               "__declspec", 10) == 0) {
                      int apd = 1;
                      sp_e += 2;
                      while (apd > 0) {
                        if (tree->base_tokens->tokens[sp_e].kind ==
                            CDD_TOKEN_LPAREN)
                          apd++;
                        else if (tree->base_tokens->tokens[sp_e].kind ==
                                 CDD_TOKEN_RPAREN)
                          apd--;
                        sp_e++;
                      }
                      continue;
                    }
                    if (tree->base_tokens->tokens[sp_e + 1].kind ==
                            CDD_TOKEN_ASSIGN ||
                        tree->base_tokens->tokens[sp_e + 1].kind ==
                            CDD_TOKEN_COMMA ||
                        tree->base_tokens->tokens[sp_e + 1].kind ==
                            CDD_TOKEN_SEMICOLON ||
                        tree->base_tokens->tokens[sp_e + 1].kind ==
                            CDD_TOKEN_LBRACKET) {
                      break;
                    }
                    sp_e++;
                  }
                  if (sp_e > st_s && sp_e < st_e) {
                    size_t ds2 = sp_e;
                    rc = c_low_slice_text(tree->base_tokens->tokens, st_s, sp_e,
                                          bty, sizeof(bty));
                    if (rc != CDD_C_SUCCESS)
                      return rc;
                    asgns[0] = '\0';
                    while (ds2 < st_e) {
                      size_t de2 = ds2;
                      int dpd = 0;
                      size_t eq2 = 0;
                      size_t nm2 = 0;
                      int fnm2 = 0;
                      char decl_part[192];
                      size_t k2;

                      while (de2 < st_e) {
                        if (tree->base_tokens->tokens[de2].kind ==
                                CDD_TOKEN_LPAREN ||
                            tree->base_tokens->tokens[de2].kind ==
                                CDD_TOKEN_LBRACE ||
                            tree->base_tokens->tokens[de2].kind ==
                                CDD_TOKEN_LBRACKET)
                          dpd++;
                        else if (tree->base_tokens->tokens[de2].kind ==
                                     CDD_TOKEN_RPAREN ||
                                 tree->base_tokens->tokens[de2].kind ==
                                     CDD_TOKEN_RBRACE ||
                                 tree->base_tokens->tokens[de2].kind ==
                                     CDD_TOKEN_RBRACKET)
                          dpd--;
                        else if (dpd == 0 &&
                                 tree->base_tokens->tokens[de2].kind ==
                                     CDD_TOKEN_COMMA)
                          break;
                        else if (dpd == 0 && eq2 == 0 &&
                                 tree->base_tokens->tokens[de2].kind ==
                                     CDD_TOKEN_ASSIGN)
                          eq2 = de2;
                        de2++;
                      }
                      rc = c_low_slice_text(tree->base_tokens->tokens, ds2,
                                            eq2 ? eq2 : de2, decl_part,
                                            sizeof(decl_part));
                      if (rc != CDD_C_SUCCESS)
                        return rc;
                      for (k2 = ds2; k2 < (eq2 ? eq2 : de2); k2++) {
                        if (tree->base_tokens->tokens[k2].kind ==
                            CDD_TOKEN_LBRACKET)
                          break;
                        if (tree->base_tokens->tokens[k2].kind ==
                            CDD_TOKEN_IDENTIFIER) {
                          nm2 = k2;
                          fnm2 = 1;
                        }
                      }
                      if (decl_part[0] != '\0') {
                        hall_pos +=
                            (size_t)CDD_SNPRINTF(hoisted_all + hall_pos,
                                                 sizeof(hoisted_all) - hall_pos,
                                                 " %s %s;", bty, decl_part);
                      }
                      if (eq2 > 0 && fnm2 && eq2 + 1 < de2) {
                        /* Hoisted aggregate `{ ... }` or scalar assignment */
                        if (tree->base_tokens->tokens[eq2 + 1].kind ==
                                CDD_TOKEN_LBRACE ||
                            (tree->base_tokens->tokens[eq2 + 1].length > 0 &&
                             tree->base_tokens->tokens[eq2 + 1].start[0] ==
                                 '{')) {
                          char vnm[96];
                          char rhs2[384];
                          const char *arr_d = strchr(decl_part, '[');
                          int ag_id = ++ctx->cl_idx;
                          memcpy(vnm, tree->base_tokens->tokens[nm2].start,
                                 tree->base_tokens->tokens[nm2].length);
                          vnm[tree->base_tokens->tokens[nm2].length] = '\0';
                          rc = c_low_slice_text(tree->base_tokens->tokens,
                                                eq2 + 1, de2, rhs2,
                                                sizeof(rhs2));
                          if (rc != CDD_C_SUCCESS)
                            return rc;
                          if (arr_d) {
                            apos += (size_t)CDD_SNPRINTF(
                                asgns + apos, sizeof(asgns) - apos,
                                "%s{ %s __cdd_ag_%d%s = %s; unsigned int "
                                "__cdd_k_%d; for (__cdd_k_%d = 0; __cdd_k_%d < "
                                "(unsigned int)(sizeof(%s) / sizeof(%s[0])); "
                                "__cdd_k_%d++) %s[__cdd_k_%d] = "
                                "__cdd_ag_%d[__cdd_k_%d]; }",
                                apos > 0 ? " " : "", bty, ag_id, arr_d, rhs2,
                                ag_id, ag_id, ag_id, vnm, vnm, ag_id, vnm,
                                ag_id, ag_id, ag_id);
                          } else {
                            apos += (size_t)CDD_SNPRINTF(
                                asgns + apos, sizeof(asgns) - apos,
                                "%s{ %s __cdd_ag_%d = %s; %s = __cdd_ag_%d; }",
                                apos > 0 ? " " : "", bty, ag_id, rhs2, vnm,
                                ag_id);
                          }
                        } else {
                          char rhs2[256];
                          rc = c_low_slice_text(tree->base_tokens->tokens,
                                                eq2 + 1, de2, rhs2,
                                                sizeof(rhs2));
                          if (rc != CDD_C_SUCCESS)
                            return rc;
                          apos += (size_t)CDD_SNPRINTF(
                              asgns + apos, sizeof(asgns) - apos,
                              "%s%.*s = %s;", apos > 0 ? " " : "",
                              (int)tree->base_tokens->tokens[nm2].length,
                              (const char *)tree->base_tokens->tokens[nm2]
                                  .start,
                              rhs2);
                        }
                      }
                      ds2 = (de2 < st_e) ? (de2 + 1) : de2;
                    }
                    if (tree->base_tokens->tokens[st_e].length > 1) {
                      apos += (size_t)CDD_SNPRINTF(
                          asgns + apos, sizeof(asgns) - apos, "%.*s",
                          (int)(tree->base_tokens->tokens[st_e].length - 1),
                          (const char *)tree->base_tokens->tokens[st_e].start +
                              1);
                    }
                    rc = c_low_replace_span(tree, st_s, st_e, asgns);
                    if (rc != CDD_C_SUCCESS)
                      return rc;
                  }
                }
              }
            }
            cur = st_e + 1;
          }
        }

        if (hall_pos > 0) {
          rc = c_low_insert_after_lbrace(tree, &tree->base_tokens->tokens[bi],
                                         hoisted_all);
          if (rc != CDD_C_SUCCESS)
            return rc;
        }
        if (hty_pos > 0) {
          rc = c_low_insert_after_lbrace(tree, &tree->base_tokens->tokens[bi],
                                         hoisted_types);
          if (rc != CDD_C_SUCCESS)
            return rc;
        }
      }
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Converts `// ...` `TRIVIA_LINE_COMMENT` nodes attached to `tok` into
 * C89 `TRIVIA_BLOCK_COMMENT` nodes for strict `-Za` / ISO C89 mode.
 *
 * @param[in,out] tree CST tree.
 * @param[in,out] tr_head Head of trivia list.
 * @return CDD_C_SUCCESS or error code.
 */
static cdd_c_error_t c_low_convert_line_comments_list(cdd_cst_tree_t *tree,
                                                      cdd_trivia_t *tr_head) {
  cdd_trivia_t *tr = tr_head;
  while (tr) {
    if (tr->kind == TRIVIA_LINE_COMMENT) {
      size_t body_len = tr->length - 2;
      char *cbuf = NULL;
      size_t k, w = 0;
      const char *pooled = NULL;
      cdd_c_error_t rc;
      while (body_len > 0 && tr->start[2 + body_len - 1] == '\r') {
        body_len--;
      }
      rc = gnu_malloc(body_len * 2 + 8, (void **)&cbuf);
      if (rc != CDD_C_SUCCESS)
        return rc;
      cbuf[w++] = '/';
      cbuf[w++] = '*';
      for (k = 0; k < body_len; k++) {
        char ch = (char)tr->start[2 + k];
        cbuf[w++] = ch;
        if (ch == '*' && k + 1 < body_len && tr->start[2 + k + 1] == '/')
          cbuf[w++] = ' ';
      }
      cbuf[w++] = ' ';
      cbuf[w++] = '*';
      cbuf[w++] = '/';
      cbuf[w] = '\0';
      rc = cdd_pool_string_safe(tree, cbuf, &pooled);
      if (rc != CDD_C_SUCCESS) {
        C_CDD_FREE(cbuf);
        return rc;
      }
      C_CDD_FREE(cbuf);
      tr->kind = TRIVIA_BLOCK_COMMENT;
      tr->start = (const uint8_t *)pooled;
      tr->length = w;
    }
    tr = tr->next;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Walks CST nodes converting `//` line comments in token trivia to C89
 * block comments.
 *
 * @param[in,out] tree CST tree.
 * @param[in,out] node Current CST node.
 * @return CDD_C_SUCCESS or error code.
 */
static cdd_c_error_t c_low_convert_node_comments(cdd_cst_tree_t *tree,
                                                 cdd_cst_node_t *node) {
  size_t i;
  cdd_c_error_t rc;
  for (i = 0; i < node->num_children; i++) {
    if (node->children[i].kind == CDD_CST_CHILD_TOKEN) {
      rc = c_low_convert_line_comments_list(
          tree, node->children[i].val.token->leading_trivia);
      if (rc != CDD_C_SUCCESS)
        return rc;
      rc = c_low_convert_line_comments_list(
          tree, node->children[i].val.token->trailing_trivia);
      if (rc != CDD_C_SUCCESS)
        return rc;
    } else {
      rc = c_low_convert_node_comments(tree, node->children[i].val.node);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Rewrites C99 `%z*`, `%t*`, `%j*`, and `%ll*` printf/scanf format
 * specifiers inside string literal `tok` to Legacy MSVC `%I*` / `%I64*`.
 *
 * @param[in,out] tree CST tree.
 * @param[in,out] tok String literal token.
 * @return CDD_C_SUCCESS or error code.
 */
static cdd_c_error_t c_low_rewrite_msvc_format_str(cdd_cst_tree_t *tree,
                                                   cdd_token_t *tok) {
  const char *s = (const char *)tok->start;
  size_t len = tok->length;
  size_t j = 0;
  char *out = NULL;
  size_t w = 0;
  int changed = 0;
  const char *pooled = NULL;
  cdd_c_error_t rc;

  if (len < 4 || memchr(s, '%', len) == NULL)
    return CDD_C_SUCCESS;

  rc = gnu_malloc(len * 2 + 16, (void **)&out);
  if (rc != CDD_C_SUCCESS)
    return rc;

  while (j < len) {
    if (s[j] == '\\' && j + 1 < len) {
      out[w++] = s[j++];
      out[w++] = s[j++];
      continue;
    }
    if (s[j] != '%') {
      out[w++] = s[j++];
      continue;
    }
    if (j + 1 < len && s[j + 1] == '%') {
      out[w++] = '%';
      out[w++] = '%';
      j += 2;
      continue;
    }
    /* Copy `%` + flags/width/precision */
    out[w++] = s[j++];
    while (j < len && (s[j] == '-' || s[j] == '+' || s[j] == ' ' ||
                       s[j] == '#' || s[j] == '0' || s[j] == '*' ||
                       s[j] == '.' || (s[j] >= '1' && s[j] <= '9'))) {
      out[w++] = s[j++];
    }
    if (j + 1 < len && (s[j] == 'z' || s[j] == 't') &&
        (s[j + 1] == 'u' || s[j + 1] == 'd' || s[j + 1] == 'i' ||
         s[j + 1] == 'x' || s[j + 1] == 'X' || s[j + 1] == 'o')) {
      out[w++] = 'I';
      out[w++] = (s[j + 1] == 'i') ? 'd' : s[j + 1];
      j += 2;
      changed = 1;
    } else if (j + 2 < len && s[j] == 'l' && s[j + 1] == 'l' &&
               (s[j + 2] == 'd' || s[j + 2] == 'i' || s[j + 2] == 'u' ||
                s[j + 2] == 'x' || s[j + 2] == 'X' || s[j + 2] == 'o')) {
      out[w++] = 'I';
      out[w++] = '6';
      out[w++] = '4';
      out[w++] = (s[j + 2] == 'i') ? 'd' : s[j + 2];
      j += 3;
      changed = 1;
    } else if (j + 1 < len && s[j] == 'j' &&
               (s[j + 1] == 'd' || s[j + 1] == 'i' || s[j + 1] == 'u' ||
                s[j + 1] == 'x' || s[j + 1] == 'X')) {
      out[w++] = 'I';
      out[w++] = '6';
      out[w++] = '4';
      out[w++] = (s[j + 1] == 'i') ? 'd' : s[j + 1];
      j += 2;
      changed = 1;
    }
  }
  out[w] = '\0';

  if (!changed) {
    C_CDD_FREE(out);
    return CDD_C_SUCCESS;
  }
  rc = cdd_pool_string_safe(tree, out, &pooled);
  if (rc != CDD_C_SUCCESS) {
    C_CDD_FREE(out);
    return rc;
  }
  C_CDD_FREE(out);
  return replace_token_with_text(tree, tok, CDD_TOKEN_STRING, pooled, w);
}

/**
 * @brief Lowers Legacy MSVC / C89 keywords (`inline`, `__func__`, `_Alignas`,
 * `_Alignof`, `_Noreturn`, `_Static_assert`), `< 1400` `UI64`/`I64` suffixes,
 * `//` comments, and `%zu`/`%lld` format specifiers.
 *
 * @param[in,out] ctx Lowering context.
 * @return CDD_C_SUCCESS or error code.
 */
static cdd_c_error_t c_lower_legacy_dialect(struct c_lowering_ctx_t *ctx) {
  cdd_cst_tree_t *tree = ctx->tree;
  size_t i;
  int brace_d = 0;
  char cur_fn[128];
  int legacy_fmt =
      (ctx->config->target_profile == CDD_TARGET_PROFILE_MSVC_LEGACY &&
       (ctx->config->msvc_ver == 0 || ctx->config->msvc_ver < 1900));
  int legacy_i64_sfx =
      (ctx->config->msvc_ver > 0 && ctx->config->msvc_ver < 1400);
  cdd_c_error_t rc;

  cur_fn[0] = '\0';

  rc = c_low_convert_node_comments(tree, tree->root);
  if (rc != CDD_C_SUCCESS)
    return rc;

  for (i = 0; i < tree->base_tokens->size; i++) {
    cdd_token_t *t = &tree->base_tokens->tokens[i];
    if (t->length == 0)
      continue;

    if (t->kind == CDD_TOKEN_LBRACE) {
      if (brace_d == 0 && i >= 2 &&
          tree->base_tokens->tokens[i - 1].kind == CDD_TOKEN_RPAREN) {
        int pd = 0;
        size_t back = i;
        while (back > 0) {
          back--;
          if (tree->base_tokens->tokens[back].kind == CDD_TOKEN_RPAREN)
            pd++;
          else if (tree->base_tokens->tokens[back].kind == CDD_TOKEN_LPAREN) {
            pd--;
            if (pd == 0) {
              if (back > 0 && tree->base_tokens->tokens[back - 1].kind ==
                                  CDD_TOKEN_IDENTIFIER) {
                size_t fl = tree->base_tokens->tokens[back - 1].length < 127
                                ? tree->base_tokens->tokens[back - 1].length
                                : 127;
                memcpy(cur_fn, tree->base_tokens->tokens[back - 1].start, fl);
                cur_fn[fl] = '\0';
              }
              break;
            }
          }
        }
      }
      brace_d++;
      continue;
    }
    if (t->kind == CDD_TOKEN_RBRACE) {
      if (brace_d > 0)
        brace_d--;
      if (brace_d == 0)
        cur_fn[0] = '\0';
      continue;
    }

    if (legacy_fmt && t->kind == CDD_TOKEN_STRING) {
      rc = c_low_rewrite_msvc_format_str(tree, t);
      if (rc != CDD_C_SUCCESS)
        return rc;
      continue;
    }

    if (legacy_i64_sfx && t->kind == CDD_TOKEN_NUMBER && t->length >= 3) {
      const char *ns = (const char *)t->start;
      size_t nl = t->length;
      char nbuf[128];
      const char *pn = NULL;
      nbuf[0] = '\0';
      if (nl >= 4 && (memcmp(ns + nl - 3, "ULL", 3) == 0 ||
                      memcmp(ns + nl - 3, "ull", 3) == 0 ||
                      memcmp(ns + nl - 3, "uLL", 3) == 0 ||
                      memcmp(ns + nl - 3, "Ull", 3) == 0)) {
        CDD_SNPRINTF(nbuf, sizeof(nbuf), "%.*sUI64", (int)(nl - 3), ns);
      } else if (memcmp(ns + nl - 2, "LL", 2) == 0 ||
                 memcmp(ns + nl - 2, "ll", 2) == 0) {
        CDD_SNPRINTF(nbuf, sizeof(nbuf), "%.*sI64", (int)(nl - 2), ns);
      }
      if (nbuf[0] != '\0') {
        rc = cdd_pool_string_safe(tree, nbuf, &pn);
        if (rc != CDD_C_SUCCESS)
          return rc;
        rc = replace_token_with_text(tree, t, CDD_TOKEN_NUMBER, pn, strlen(pn));
        if (rc != CDD_C_SUCCESS)
          return rc;
        continue;
      }
    }

    if (t->kind == CDD_TOKEN_IDENTIFIER) {
      if (t->length == 6 && memcmp(t->start, "inline", 6) == 0) {
        const char *inl = (ctx->is_msvc || ctx->config->target_profile ==
                                               CDD_TARGET_PROFILE_WATCOM)
                              ? "__inline"
                              : "";
        rc = replace_token_with_text(tree, t, CDD_TOKEN_IDENTIFIER, inl,
                                     strlen(inl));
        if (rc != CDD_C_SUCCESS)
          return rc;
      } else if (t->length == 8 && memcmp(t->start, "__func__", 8) == 0) {
        char fstr[160];
        const char *pf = NULL;
        CDD_SNPRINTF(fstr, sizeof(fstr), "\"%s\"",
                     cur_fn[0] != '\0' ? cur_fn : "");
        rc = cdd_pool_string_safe(tree, fstr, &pf);
        if (rc != CDD_C_SUCCESS)
          return rc;
        rc = replace_token_with_text(tree, t, CDD_TOKEN_STRING, pf, strlen(pf));
        if (rc != CDD_C_SUCCESS)
          return rc;
      } else if (t->length == 9 && memcmp(t->start, "_Noreturn", 9) == 0) {
        const char *nr = ctx->is_msvc ? "__declspec(noreturn)" : "";
        rc = replace_token_with_text(tree, t, CDD_TOKEN_IDENTIFIER, nr,
                                     strlen(nr));
        if (rc != CDD_C_SUCCESS)
          return rc;
      } else if (t->length == 8 && memcmp(t->start, "_Alignas", 8) == 0 &&
                 i + 3 < tree->base_tokens->size &&
                 tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_LPAREN) {
        size_t rp = i + 2;
        int pd = 1;
        while (rp < tree->base_tokens->size) {
          if (tree->base_tokens->tokens[rp].kind == CDD_TOKEN_LPAREN)
            pd++;
          else if (tree->base_tokens->tokens[rp].kind == CDD_TOKEN_RPAREN) {
            pd--;
            if (pd == 0)
              break;
          }
          rp++;
        }
        if (rp < tree->base_tokens->size) {
          if (ctx->is_msvc) {
            char al_s[64], dspec[128];
            rc = c_low_slice_text(tree->base_tokens->tokens, i + 2, rp, al_s,
                                  sizeof(al_s));
            if (rc != CDD_C_SUCCESS)
              return rc;
            CDD_SNPRINTF(dspec, sizeof(dspec), "__declspec(align(%s))", al_s);
            rc = c_low_replace_span(tree, i, rp, dspec);
            if (rc != CDD_C_SUCCESS)
              return rc;
          } else {
            rc = c_low_replace_span(tree, i, rp, "");
            if (rc != CDD_C_SUCCESS)
              return rc;
          }
          i = rp;
        }
      } else if (t->length == 8 && memcmp(t->start, "_Alignof", 8) == 0 &&
                 i + 3 < tree->base_tokens->size &&
                 tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_LPAREN) {
        size_t rp = i + 2;
        int pd = 1;
        while (rp < tree->base_tokens->size) {
          if (tree->base_tokens->tokens[rp].kind == CDD_TOKEN_LPAREN)
            pd++;
          else if (tree->base_tokens->tokens[rp].kind == CDD_TOKEN_RPAREN) {
            pd--;
            if (pd == 0)
              break;
          }
          rp++;
        }
        if (rp < tree->base_tokens->size) {
          char ty_a[128], arepl[256];
          rc = c_low_slice_text(tree->base_tokens->tokens, i + 2, rp, ty_a,
                                sizeof(ty_a));
          if (rc != CDD_C_SUCCESS)
            return rc;
          if (ctx->is_msvc) {
            CDD_SNPRINTF(arepl, sizeof(arepl), "__alignof(%s)", ty_a);
          } else {
            CDD_SNPRINTF(arepl, sizeof(arepl),
                         "((size_t)&(((struct { char _c; %s _x; } *)0)->_x))",
                         ty_a);
          }
          rc = c_low_replace_span(tree, i, rp, arepl);
          if (rc != CDD_C_SUCCESS)
            return rc;
          i = rp;
        }
      } else if (((t->length == 14 &&
                   memcmp(t->start, "_Static_assert", 14) == 0) ||
                  (t->length == 13 &&
                   memcmp(t->start, "static_assert", 13) == 0)) &&
                 i + 3 < tree->base_tokens->size &&
                 tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_LPAREN) {
        size_t rp = i + 2;
        size_t comma0 = 0;
        int pd = 1;
        while (rp < tree->base_tokens->size) {
          if (tree->base_tokens->tokens[rp].kind == CDD_TOKEN_LPAREN)
            pd++;
          else if (tree->base_tokens->tokens[rp].kind == CDD_TOKEN_RPAREN) {
            pd--;
            if (pd == 0)
              break;
          } else if (pd == 1 && comma0 == 0 &&
                     tree->base_tokens->tokens[rp].kind == CDD_TOKEN_COMMA) {
            comma0 = rp;
          }
          rp++;
        }
        if (rp < tree->base_tokens->size) {
          char cond_s[256], sa_repl[384];
          size_t c_end = comma0 ? comma0 : rp;
          rc = c_low_slice_text(tree->base_tokens->tokens, i + 2, c_end, cond_s,
                                sizeof(cond_s));
          if (rc != CDD_C_SUCCESS)
            return rc;
          CDD_SNPRINTF(sa_repl, sizeof(sa_repl),
                       "typedef char __cdd_sa_%d[(%s) ? 1 : -1]", ++ctx->sa_idx,
                       cond_s);
          rc = c_low_replace_span(tree, i, rp, sa_repl);
          if (rc != CDD_C_SUCCESS)
            return rc;
          i = rp;
        }
      }
    }
  }
  return CDD_C_SUCCESS;
}

/*
 * @brief Executes Phase 3.2 Legacy MSVC / C89 lowering passes.
 */
cdd_c_error_t c_lower_legacy_passes(struct c_lowering_ctx_t *ctx) {
  cdd_c_error_t rc;
  rc = c_lower_compound_literals(ctx);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = c_lower_designated_inits(ctx);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = c_lower_legacy_dialect(ctx);
  if (rc != CDD_C_SUCCESS)
    return rc;
  return c_lower_mixed_decls_and_for(ctx);
}

/*
 * @brief Lowers C99/C11/C23 language features according to target profile.
 *
 * @param[in,out] tree The CST tree to lower.
 * @param[in] config Optional configuration specifying dialect/MSVC profile.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t cdd_transform_c_lowering(cdd_cst_tree_t *tree,
                                       const cdd_transform_config_t *config) {
  struct c_lowering_ctx_t ctx;
  cdd_c_error_t rc;

  if (!tree || !tree->root)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (config && config->msvc_ver < 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (!tree->base_tokens || tree->base_tokens->size == 0)
    return CDD_C_SUCCESS;

  memset(&ctx, 0, sizeof(ctx));
  ctx.tree = tree;
  ctx.config = config;

  if (config) {
    if (config->target_profile == CDD_TARGET_PROFILE_MSVC_LEGACY ||
        config->target_profile == CDD_TARGET_PROFILE_MSVC_MODERN ||
        config->msvc_ver > 0) {
      ctx.is_msvc = 1;
    }
    if (config->target_profile == CDD_TARGET_PROFILE_MSVC_LEGACY ||
        config->target_profile == CDD_TARGET_PROFILE_WATCOM ||
        config->target_profile == CDD_TARGET_PROFILE_ISO_C89 ||
        config->target_c89) {
      ctx.is_legacy_c89 = 1;
    }
    if (!ctx.is_msvc && ctx.is_legacy_c89) {
      ctx.is_iso_c89 = 1;
    }
  }

  rc = cdd_cst_build_semantic_info_ex(
      tree, config ? config->include_dirs : NULL,
      config ? config->num_include_dirs : 0, &ctx.sem_env);
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc = c_lower_modern_passes(&ctx);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  if (ctx.is_legacy_c89) {
    rc = c_lower_legacy_passes(&ctx);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
  }

cleanup:;
  return cdd_c_keep_err(rc, cdd_cst_scope_env_free(ctx.sem_env));
}
