/**
 * @file gnu_standardizer_types.c
 * @brief Type, attribute, and macro standardization pass for GNU standardizer.
 */

/* clang-format off */
#include "gnu_standardizer_internal.h"
/* clang-format on */

/**
 * @brief Strips C23 digit separators (`'`) from `raw` and checks whether the
 * token is strictly an integer literal (decimal or hex, with optional `u`/`l`
 * suffixes and no floating/imaginary characters).
 *
 * @param[in] raw Raw number token string.
 * @param[in] len Byte length of `raw`.
 * @param[out] clean_out Destination buffer with `'` removed.
 * @param[out] out_len Receives byte length of `clean_out`.
 * @param[out] out_is_int Set to 1 if `clean_out` is a plain integer literal.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t
gnu_strip_and_check_int_literal(const char *raw, size_t len, char *clean_out,
                                size_t *out_len, int *out_is_int) {
  size_t i, w = 0;
  int is_hex = 0;
  size_t k = 0;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif

  for (i = 0; i < len; i++) {
    if (raw[i] != '\'')
      clean_out[w++] = raw[i];
  }
  clean_out[w] = '\0';
  *out_len = w;
  *out_is_int = 0;

  if (w >= 3 && clean_out[0] == '0' &&
      tolower((unsigned char)clean_out[1]) == 'x') {
    is_hex = 1;
    k = 2;
  }
  while (k < w) {
    unsigned char uc = (unsigned char)clean_out[k];
    if (!(isxdigit(uc) && (is_hex || isdigit(uc))))
      break;
    k++;
  }
  if (k == (is_hex ? 2 : 0))
    return CDD_C_SUCCESS;
  while (k < w && (tolower((unsigned char)clean_out[k]) == 'u' ||
                   tolower((unsigned char)clean_out[k]) == 'l')) {
    k++;
  }
  if (k == w)
    *out_is_int = 1;
  return CDD_C_SUCCESS;
}

/**
 * @brief Strips optional leading and trailing `__` from a GNU attribute name.
 *
 * @param[in] s Raw attribute identifier bytes.
 * @param[in] len Byte length.
 * @param[out] out Normalized name buffer.
 * @param[in] cap Buffer capacity.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t gnu_norm_attr_name(const uint8_t *s, size_t len, char *out,
                                        size_t cap) {
  size_t off = 0;
  size_t n = len;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  if (n > 4 && s[0] == '_' && s[1] == '_' && s[n - 2] == '_' &&
      s[n - 1] == '_') {
    off = 2;
    n -= 4;
  }
  if (n >= cap)
    n = cap - 1;
  memcpy(out, s + off, n);
  out[n] = '\0';
  return CDD_C_SUCCESS;
}

/**
 * @brief Finds the first token of the declaration containing `attr_idx`
 * (stepping backward across balanced `{ ... }` struct bodies and `(...)`
 * parameter lists).
 *
 * @param[in] toks Base token array.
 * @param[in] attr_idx Index of `__attribute__` token.
 * @param[out] out_decl_idx Receives index of declaration start token.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t gnu_find_decl_start(const cdd_token_t *toks,
                                         size_t attr_idx,
                                         size_t *out_decl_idx) {
  size_t k = attr_idx;
  int brace_d = 0;
  int paren_d = 0;
  size_t best = attr_idx;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  while (k > 0) {
    const cdd_token_t *tk;
    k--;
    tk = &toks[k];
    if (tk->length == 0)
      continue;
    if (tk->kind == CDD_TOKEN_RBRACE) {
      brace_d++;
      continue;
    }
    if (tk->kind == CDD_TOKEN_LBRACE) {
      if (brace_d > 0) {
        brace_d--;
        if (brace_d == 0 && k > 0 && toks[k - 1].kind == CDD_TOKEN_RPAREN) {
          /* Preceding `{ ... }` was a function definition body, not a struct */
          break;
        }
        continue;
      }
      break;
    }
    if (brace_d > 0)
      continue;
    if (tk->kind == CDD_TOKEN_RPAREN) {
      paren_d++;
      continue;
    }
    if (tk->kind == CDD_TOKEN_LPAREN) {
      if (paren_d > 0) {
        paren_d--;
        continue;
      }
      break;
    }
    if (paren_d > 0)
      continue;
    if (tk->kind == CDD_TOKEN_SEMICOLON ||
        (tk->start[0] == '#' && tk->kind != CDD_TOKEN_OTHER)) {
      break;
    }
    best = k;
  }
  *out_decl_idx = best;
  return CDD_C_SUCCESS;
}

/**
 * @brief Prepends `prefix` onto `tok`'s text in `tree`.
 *
 * @param[in,out] tree CST tree.
 * @param[in,out] tok Target token.
 * @param[in] prefix Null-terminated prefix string.
 * @return CDD_C_SUCCESS or error code.
 */
static cdd_c_error_t gnu_prepend_tok(cdd_cst_tree_t *tree, cdd_token_t *tok,
                                     const char *prefix) {
  size_t plen = strlen(prefix);
  char *buf = NULL;
  const char *pooled = NULL;
  cdd_c_error_t rc;
  rc = gnu_malloc(plen + tok->length + 1, (void **)&buf);
  if (rc != CDD_C_SUCCESS)
    return rc;
  memcpy(buf, prefix, plen);
  if (tok->length > 0)
    memcpy(buf + plen, tok->start, tok->length);
  buf[plen + tok->length] = '\0';
  rc = cdd_pool_string_safe(tree, buf, &pooled);
  if (rc != CDD_C_SUCCESS) {
    C_CDD_FREE(buf);
    return rc;
  }
  C_CDD_FREE(buf);
  return replace_token_with_text(tree, tok, tok->kind, pooled,
                                 plen + tok->length);
}

/**
 * @brief Appends `suffix` onto `tok`'s text in `tree`.
 *
 * @param[in,out] tree CST tree.
 * @param[in,out] tok Target token.
 * @param[in] suffix Null-terminated suffix string.
 * @return CDD_C_SUCCESS or error code.
 */
static cdd_c_error_t gnu_append_tok(cdd_cst_tree_t *tree, cdd_token_t *tok,
                                    const char *suffix) {
  size_t slen = strlen(suffix);
  char *buf = NULL;
  const char *pooled = NULL;
  cdd_c_error_t rc;
  rc = gnu_malloc(tok->length + slen + 1, (void **)&buf);
  if (rc != CDD_C_SUCCESS)
    return rc;
  memcpy(buf, tok->start, tok->length);
  memcpy(buf + tok->length, suffix, slen);
  buf[tok->length + slen] = '\0';
  rc = cdd_pool_string_safe(tree, buf, &pooled);
  if (rc != CDD_C_SUCCESS) {
    C_CDD_FREE(buf);
    return rc;
  }
  C_CDD_FREE(buf);
  return replace_token_with_text(tree, tok, tok->kind, pooled,
                                 tok->length + slen);
}

/**
 * @brief Injects SAL `_Printf_format_string_ ` before 1-based parameter
 * `fmt_idx` of the function declaration starting at `decl_idx`.
 *
 * @param[in,out] tree CST tree.
 * @param[in] decl_idx Start token index of function declaration.
 * @param[in] attr_end End token index of attribute.
 * @param[in] fmt_idx 1-based parameter index of format string.
 * @return CDD_C_SUCCESS or error code.
 */
static cdd_c_error_t gnu_inject_printf_sal(cdd_cst_tree_t *tree,
                                           size_t decl_idx, size_t attr_end,
                                           int fmt_idx) {
  size_t k;
  size_t param_lp = 0;
  int found_lp = 0;
  if (fmt_idx <= 0)
    return CDD_C_SUCCESS;
  for (k = decl_idx; k < tree->base_tokens->size; k++) {
    cdd_token_t *tk = &tree->base_tokens->tokens[k];
    if (k >= attr_end &&
        (tk->kind == CDD_TOKEN_SEMICOLON || tk->kind == CDD_TOKEN_LBRACE)) {
      break;
    }
    if (tk->kind == CDD_TOKEN_IDENTIFIER && k + 1 < tree->base_tokens->size &&
        tree->base_tokens->tokens[k + 1].kind == CDD_TOKEN_LPAREN &&
        !(tk->length == 13 && memcmp(tk->start, "__attribute__", 13) == 0) &&
        !(tk->length == 10 && memcmp(tk->start, "__declspec", 10) == 0)) {
      param_lp = k + 1;
      found_lp = 1;
      break;
    }
  }
  if (!found_lp)
    return CDD_C_SUCCESS;
  {
    int cur_p = 1;
    int pd = 0;
    for (k = param_lp + 1; k < tree->base_tokens->size; k++) {
      cdd_token_t *tk = &tree->base_tokens->tokens[k];
      if (pd == 0 && tk->kind == CDD_TOKEN_RPAREN)
        break;
      if (pd == 0 && cur_p == fmt_idx && tk->length > 0) {
        return gnu_prepend_tok(tree, tk, "_Printf_format_string_ ");
      }
      if (tk->kind == CDD_TOKEN_LPAREN)
        pd++;
      else if (tk->kind == CDD_TOKEN_RPAREN)
        pd--;
      else if (pd == 0 && tk->kind == CDD_TOKEN_COMMA)
        cur_p++;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Appends MSVC `.CRT$XCU` initializer registration after a `constructor`
 * or `destructor` function declaration/definition.
 *
 * @param[in,out] tree CST tree.
 * @param[in] decl_idx Start token index of function.
 * @param[in] attr_end End token index of `__attribute__((...))`.
 * @param[in] is_dtor Non-zero for `destructor`, 0 for `constructor`.
 * @return CDD_C_SUCCESS or error code.
 */
static cdd_c_error_t gnu_emit_msvc_crt_xcu(cdd_cst_tree_t *tree,
                                           size_t decl_idx, size_t attr_end,
                                           int is_dtor) {
  char fn_name[128];
  size_t k;
  size_t end_tok = 0;
  int bd = 0;
  int saw_body = 0;
  char reg_buf[1024];
  fn_name[0] = '\0';

  for (k = decl_idx; k < tree->base_tokens->size; k++) {
    cdd_token_t *tk = &tree->base_tokens->tokens[k];
    if (fn_name[0] == '\0' && tk->kind == CDD_TOKEN_IDENTIFIER &&
        k + 1 < tree->base_tokens->size &&
        tree->base_tokens->tokens[k + 1].kind == CDD_TOKEN_LPAREN &&
        !(tk->length == 13 && memcmp(tk->start, "__attribute__", 13) == 0) &&
        !(tk->length == 10 && memcmp(tk->start, "__declspec", 10) == 0)) {
      size_t nl =
          tk->length < sizeof(fn_name) - 1 ? tk->length : sizeof(fn_name) - 1;
      memcpy(fn_name, tk->start, nl);
      fn_name[nl] = '\0';
    }
    if (tk->kind == CDD_TOKEN_LBRACE) {
      bd++;
      saw_body = 1;
    } else if (tk->kind == CDD_TOKEN_RBRACE) {
      bd--;
      if (saw_body && bd == 0) {
        end_tok = k;
        break;
      }
    } else if (!saw_body && k >= attr_end && tk->kind == CDD_TOKEN_SEMICOLON) {
      end_tok = k;
      break;
    }
  }
  if (fn_name[0] == '\0' || end_tok == 0)
    return CDD_C_SUCCESS;

  if (is_dtor) {
    CDD_SNPRINTF(
        reg_buf, sizeof(reg_buf),
        "\nstatic void __cdecl __cdd_dtor_atexit_%s(void) { atexit(%s); }\n"
        "#pragma section(\".CRT$XCU\", read)\n"
        "__declspec(allocate(\".CRT$XCU\")) static void (__cdecl "
        "*__cdd_dtor_%s)(void) = __cdd_dtor_atexit_%s;\n",
        fn_name, fn_name, fn_name, fn_name);
  } else {
    CDD_SNPRINTF(reg_buf, sizeof(reg_buf),
                 "\n#pragma section(\".CRT$XCU\", read)\n"
                 "__declspec(allocate(\".CRT$XCU\")) static void (__cdecl "
                 "*__cdd_ctor_%s)(void) = %s;\n",
                 fn_name, fn_name);
  }
  return gnu_append_tok(tree, &tree->base_tokens->tokens[end_tok], reg_buf);
}

/**
 * @brief Rewrites unescaped GNU `\e` escape sequences to octal `\033` inside a
 * string or character literal token.
 *
 * @param[in,out] tree CST tree.
 * @param[in,out] tok String or char token.
 * @return CDD_C_SUCCESS or error code.
 */
static cdd_c_error_t gnu_lower_esc_e_token(cdd_cst_tree_t *tree,
                                           cdd_token_t *tok) {
  const char *s = (const char *)tok->start;
  size_t len = tok->length;
  size_t j;
  size_t count = 0;
  int esc = 0;

  for (j = 0; j < len; j++) {
    if (esc) {
      if (s[j] == 'e')
        count++;
      esc = 0;
    } else if (s[j] == '\\') {
      esc = 1;
    }
  }
  if (count == 0)
    return CDD_C_SUCCESS;

  {
    size_t cap = len + count * 3 + 1;
    char *buf = NULL;
    char *w;
    const char *pooled = NULL;
    cdd_c_error_t rc;
    rc = gnu_malloc(cap, (void **)&buf);
    if (rc != CDD_C_SUCCESS)
      return rc;
    w = buf;
    esc = 0;
    for (j = 0; j < len; j++) {
      if (esc) {
        if (s[j] == 'e') {
          *w++ = '0';
          *w++ = '3';
          *w++ = '3';
        } else {
          *w++ = s[j];
        }
        esc = 0;
      } else if (s[j] == '\\') {
        *w++ = '\\';
        esc = 1;
      } else {
        *w++ = s[j];
      }
    }
    *w = '\0';
    rc = cdd_pool_string_safe(tree, buf, &pooled);
    if (rc != CDD_C_SUCCESS) {
      C_CDD_FREE(buf);
      return rc;
    }
    C_CDD_FREE(buf);
    return replace_token_with_text(tree, tok, tok->kind, pooled,
                                   (size_t)(w - buf));
  }
}

/**
 * @brief Classifies whether an integer literal string fits in 64 bits or
 * overflows 64 bits (`CDD_C_ERROR_PARSE`) while percolating fault injection.
 *
 * @param[in] str Cleaned integer literal string.
 * @param[out] out_fits_64 Set to 1 if `parse_numeric_literal` succeeded.
 * @param[out] out_exceeds_64 Set to 1 if `parse_numeric_literal` overflowed.
 * @return CDD_C_SUCCESS on classification, or error code on fault injection.
 */
static cdd_c_error_t gnu_classify_num_literal(const char *str, int *out_fits_64,
                                              int *out_exceeds_64) {
  struct NumericValue nv;
  cdd_c_error_t rc;
  *out_fits_64 = 0;
  *out_exceeds_64 = 0;
  rc = parse_numeric_literal(str, &nv);
  if (rc == CDD_C_SUCCESS) {
    *out_fits_64 = 1;
    return rc;
  }
  if (rc == CDD_C_ERROR_PARSE) {
    *out_exceeds_64 = 1;
    return CDD_C_SUCCESS;
  }
  return rc;
}

/*
 * @brief Standardizes GNU type and macro extensions (named variadic macros,
 * `__int128`, decimal/half floats, `__complex__`, `typeof`, `__auto_type`, and
 * `__attribute__`) using semantic context.
 *
 * @param[in,out] tree CST tree to transform.
 * @param[in,out] gctx Optional GNU standardizer context.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
cdd_c_error_t gnu_standardize_types_ctx(cdd_cst_tree_t *tree,
                                        struct gnu_standardizer_ctx_t *gctx) {
  size_t i;
  cdd_c_error_t rc = CDD_C_SUCCESS;
  struct gnu_standardizer_ctx_t local_ctx;
  struct gnu_standardizer_ctx_t *ctx_p = gctx;
  int saw_variadic_macro = 0;

  if (!ctx_p) {
    memset(&local_ctx, 0, sizeof(local_ctx));
    ctx_p = &local_ctx;
  }

  for (i = 0; i < tree->base_tokens->size; i++) {
    cdd_token_t *tok = &tree->base_tokens->tokens[i];
    if (tok->kind == CDD_TOKEN_PREPROC_DEFINE) {
      const char *p = (const char *)tok->start;
      size_t len = tok->length;
      char *buf;
      rc = gnu_malloc(len + 1, (void **)&buf);
      if (rc != CDD_C_SUCCESS)
        return rc;
      memcpy(buf, p, len);
      buf[len] = '\0';

      {
        char *id_start = buf;
        while (*id_start && *id_start != ' ' && *id_start != '\t')
          id_start++;
        while (*id_start == ' ' || *id_start == '\t')
          id_start++;

        if (*id_start) {
          char *id_end = id_start;
          while (isalnum((unsigned char)*id_end) || *id_end == '_')
            id_end++;

          if (*id_end == '(') {
            char *lparen = id_end;
            char *rparen = strchr(lparen, ')');
            if (rparen) {
              char *ellipsis = strstr(lparen, "...");
              if (ellipsis && ellipsis < rparen) {
                char *var_name = NULL;
                size_t var_len = 0;
                char *start_id = ellipsis;
                saw_variadic_macro = 1;
                while (isalnum((unsigned char)start_id[-1]) ||
                       start_id[-1] == '_') {
                  start_id--;
                }
                if (start_id < ellipsis) {
                  var_len = (size_t)(ellipsis - start_id);
                  rc = gnu_malloc(var_len + 1, (void **)&var_name);
                  if (rc != CDD_C_SUCCESS) {
                    C_CDD_FREE(buf);
                    return rc;
                  }
                  memcpy(var_name, start_id, var_len);
                  var_name[var_len] = '\0';
                } else {
                  rc = gnu_malloc(12, (void **)&var_name);
                  if (rc != CDD_C_SUCCESS) {
                    C_CDD_FREE(buf);
                    return rc;
                  }
                  memcpy(var_name, "__VA_ARGS__", 11);
                  var_name[11] = '\0';
                  var_len = 11;
                }

                {
                  size_t out_cap = len * 2 + 128;
                  char *out_buf;
                  char *out_p;
                  char *in_p;
                  rc = gnu_malloc(out_cap, (void **)&out_buf);
                  if (rc != CDD_C_SUCCESS) {
                    C_CDD_FREE(var_name);
                    C_CDD_FREE(buf);
                    return rc;
                  }
                  out_p = out_buf;
                  in_p = buf;

                  memcpy(out_p, in_p, (size_t)(start_id - in_p));
                  out_p += (start_id - in_p);
                  in_p = start_id;

                  memcpy(out_p, "...", 3);
                  out_p += 3;
                  in_p = ellipsis + 3;

                  memcpy(out_p, in_p, (size_t)((rparen + 1) - in_p));
                  out_p += (rparen + 1) - in_p;
                  in_p = rparen + 1;

                  while (*in_p) {
                    int matched = 0;
                    if (*in_p == ',') {
                      char *t = in_p + 1;
                      while (*t == ' ' || *t == '\t')
                        t++;
                      if (t[0] == '#' && t[1] == '#') {
                        t += 2;
                        while (*t == ' ' || *t == '\t')
                          t++;
                        if (strncmp(t, var_name, var_len) == 0 &&
                            !isalnum((unsigned char)t[var_len]) &&
                            t[var_len] != '_') {
#if defined(_MSC_VER)
                          strcpy_s(out_p, out_cap - (size_t)(out_p - out_buf),
                                   ", __VA_ARGS__");
#else
                          strcpy(out_p, ", __VA_ARGS__");
#endif
                          out_p += strlen(", __VA_ARGS__");
                          in_p = t + var_len;
                          matched = 1;
                        }
                      }
                    }
                    if (!matched && strncmp(in_p, var_name, var_len) == 0 &&
                        !isalnum((unsigned char)in_p[var_len]) &&
                        in_p[var_len] != '_' &&
                        !isalnum((unsigned char)in_p[-1]) && in_p[-1] != '_') {
                      CDD_STRCPY(out_p, out_cap - (size_t)(out_p - out_buf),
                                 "__VA_ARGS__");
                      out_p += strlen("__VA_ARGS__");
                      in_p += var_len;
                      matched = 1;
                    }

                    if (!matched) {
                      *out_p++ = *in_p++;
                    }
                  }
                  *out_p = '\0';
                  {
                    const char *pooled = NULL;
                    rc = cdd_pool_string_safe(tree, out_buf, &pooled);
                    if (rc != CDD_C_SUCCESS) {
                      C_CDD_FREE(out_buf);
                      C_CDD_FREE(var_name);
                      C_CDD_FREE(buf);
                      return rc;
                    }
                    rc = replace_token_with_text(
                        tree, tok, CDD_TOKEN_PREPROC_DEFINE, pooled,
                        (size_t)(out_p - out_buf));
                    if (rc != CDD_C_SUCCESS) {
                      C_CDD_FREE(out_buf);
                      C_CDD_FREE(var_name);
                      C_CDD_FREE(buf);
                      return rc;
                    }
                    C_CDD_FREE(out_buf);
                  }
                }
                C_CDD_FREE(var_name);
              }
            }
          }
        }
      }
      C_CDD_FREE(buf);
    } else if (tok->kind == CDD_TOKEN_KEYWORD___INT128) {
      int is_unsigned = 0;
      ctx_p->used_int128 = 1;
      if (i > 0) {
        cdd_token_t *prev = &tree->base_tokens->tokens[i - 1];
        if (prev->length == 8 && memcmp(prev->start, "unsigned", 8) == 0)
          is_unsigned = 1;
      }
      if (is_unsigned) {
        rc = replace_token_with_text(tree, &tree->base_tokens->tokens[i - 1],
                                     CDD_TOKEN_IDENTIFIER, "", 0);
        if (rc != CDD_C_SUCCESS)
          return rc;
        rc = replace_token_with_text(tree, tok, CDD_TOKEN_IDENTIFIER,
                                     "cdd_uint128_t", 13);
        if (rc != CDD_C_SUCCESS)
          return rc;
      } else {
        rc = replace_token_with_text(tree, tok, CDD_TOKEN_IDENTIFIER,
                                     "cdd_int128_t", 12);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
      /* Rewrite scalar literal initializers `= 10` / `= -5` to `{ lo, hi }` */
      {
        size_t ik = i + 1;
        while (ik < tree->base_tokens->size &&
               tree->base_tokens->tokens[ik].kind != CDD_TOKEN_SEMICOLON &&
               tree->base_tokens->tokens[ik].kind != CDD_TOKEN_LBRACE &&
               tree->base_tokens->tokens[ik].kind != CDD_TOKEN_RPAREN) {
          if (tree->base_tokens->tokens[ik].kind == CDD_TOKEN_ASSIGN) {
            if (ik + 2 < tree->base_tokens->size &&
                tree->base_tokens->tokens[ik + 1].kind == CDD_TOKEN_NUMBER &&
                tree->base_tokens->tokens[ik + 1].length < 32 &&
                (tree->base_tokens->tokens[ik + 2].kind == CDD_TOKEN_COMMA ||
                 tree->base_tokens->tokens[ik + 2].kind ==
                     CDD_TOKEN_SEMICOLON)) {
              char nchk[36];
              size_t clen = 0;
              int is_pint = 0;
              int fits_64 = 0;
              int exc_64 = 0;
              rc = gnu_strip_and_check_int_literal(
                  (const char *)tree->base_tokens->tokens[ik + 1].start,
                  tree->base_tokens->tokens[ik + 1].length, nchk, &clen,
                  &is_pint);
              if (rc != CDD_C_SUCCESS)
                return rc;
              if (is_pint) {
                rc = gnu_classify_num_literal(nchk, &fits_64, &exc_64);
                if (rc != CDD_C_SUCCESS)
                  return rc;
              }
              if (fits_64) {
                char ibuf[128];
                const char *ipooled = NULL;
                CDD_SNPRINTF(ibuf, sizeof(ibuf), "{ (uint64_t)(%s), 0 }", nchk);
                rc = cdd_pool_string_safe(tree, ibuf, &ipooled);
                if (rc != CDD_C_SUCCESS)
                  return rc;
                rc = replace_token_with_text(
                    tree, &tree->base_tokens->tokens[ik + 1], CDD_TOKEN_OTHER,
                    ipooled, strlen(ipooled));
                if (rc != CDD_C_SUCCESS)
                  return rc;
              }
            } else if (ik + 3 < tree->base_tokens->size &&
                       tree->base_tokens->tokens[ik + 1].kind ==
                           CDD_TOKEN_MINUS &&
                       tree->base_tokens->tokens[ik + 2].kind ==
                           CDD_TOKEN_NUMBER &&
                       tree->base_tokens->tokens[ik + 2].length < 24 &&
                       (tree->base_tokens->tokens[ik + 3].kind ==
                            CDD_TOKEN_COMMA ||
                        tree->base_tokens->tokens[ik + 3].kind ==
                            CDD_TOKEN_SEMICOLON)) {
              char ibuf[128];
              const char *ipooled = NULL;
              CDD_SNPRINTF(
                  ibuf, sizeof(ibuf), "{ (uint64_t)(-(int64_t)(%.*s)), %s }",
                  (int)tree->base_tokens->tokens[ik + 2].length,
                  (const char *)tree->base_tokens->tokens[ik + 2].start,
                  is_unsigned ? "(uint64_t)(-(int64_t)1)" : "-1");
              rc = cdd_pool_string_safe(tree, ibuf, &ipooled);
              if (rc != CDD_C_SUCCESS)
                return rc;
              rc = replace_token_with_text(
                  tree, &tree->base_tokens->tokens[ik + 1], CDD_TOKEN_OTHER,
                  ipooled, strlen(ipooled));
              if (rc != CDD_C_SUCCESS)
                return rc;
              rc = replace_token_with_text(tree,
                                           &tree->base_tokens->tokens[ik + 2],
                                           CDD_TOKEN_OTHER, "", 0);
              if (rc != CDD_C_SUCCESS)
                return rc;
            }
          }
          ik++;
        }
      }
    } else if (tok->kind == CDD_TOKEN_KEYWORD__DECIMAL32 ||
               tok->kind == CDD_TOKEN_KEYWORD__FRACT) {
      rc = replace_token_with_text(tree, tok, CDD_TOKEN_IDENTIFIER, "float", 5);
      if (rc != CDD_C_SUCCESS)
        return rc;
    } else if (tok->kind == CDD_TOKEN_KEYWORD__DECIMAL64 ||
               tok->kind == CDD_TOKEN_KEYWORD__DECIMAL128 ||
               tok->kind == CDD_TOKEN_KEYWORD__ACCUM) {
      rc =
          replace_token_with_text(tree, tok, CDD_TOKEN_IDENTIFIER, "double", 6);
      if (rc != CDD_C_SUCCESS)
        return rc;
    } else if (tok->kind == CDD_TOKEN_KEYWORD___FP16 ||
               tok->kind == CDD_TOKEN_KEYWORD__FLOAT16 ||
               tok->kind == CDD_TOKEN_KEYWORD___BF16) {
      rc = replace_token_with_text(tree, tok, CDD_TOKEN_IDENTIFIER, "uint16_t",
                                   8);
      if (rc != CDD_C_SUCCESS)
        return rc;
    } else if (tok->kind == CDD_TOKEN_KEYWORD___COMPLEX__) {
      /* `__complex__ int` -> `struct { int real, imag; }` */
      if (i + 1 < tree->base_tokens->size) {
        cdd_token_t *next_tok = &tree->base_tokens->tokens[i + 1];
        size_t child_idx;
        cdd_cst_node_t *owning_node = NULL;
        char snip[256];
        rc = gnu_find_node(tree->root, tok, &child_idx, &owning_node);
        if (rc != CDD_C_SUCCESS)
          return rc;
        CDD_SNPRINTF(snip, sizeof(snip), "struct { %.*s real, imag; } ",
                     (int)next_tok->length, (const char *)next_tok->start);
        rc = gnu_splice_snippet_at(tree, &owning_node, child_idx, 2, snip);
        if (rc != CDD_C_SUCCESS)
          return rc;
        tok->kind = CDD_TOKEN_OTHER;
        next_tok->kind = CDD_TOKEN_OTHER;
        i++;
      }
    } else if (tok->kind == CDD_TOKEN_KEYWORD___REAL__) {
      if (i + 1 < tree->base_tokens->size &&
          tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_IDENTIFIER) {
        cdd_token_t *next_tok = &tree->base_tokens->tokens[i + 1];
        size_t child_idx;
        cdd_cst_node_t *owning_node = NULL;
        char snip[256];
        rc = gnu_find_node(tree->root, tok, &child_idx, &owning_node);
        if (rc != CDD_C_SUCCESS)
          return rc;
        CDD_SNPRINTF(snip, sizeof(snip), "%.*s.real ", (int)next_tok->length,
                     (const char *)next_tok->start);
        rc = gnu_splice_snippet_at(tree, &owning_node, child_idx, 2, snip);
        if (rc != CDD_C_SUCCESS)
          return rc;
        tok->kind = CDD_TOKEN_OTHER;
        next_tok->kind = CDD_TOKEN_OTHER;
        i++;
      }
    } else if (tok->kind == CDD_TOKEN_KEYWORD___IMAG__) {
      if (i + 1 < tree->base_tokens->size &&
          tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_IDENTIFIER) {
        cdd_token_t *next_tok = &tree->base_tokens->tokens[i + 1];
        size_t child_idx;
        cdd_cst_node_t *owning_node = NULL;
        char snip[256];
        rc = gnu_find_node(tree->root, tok, &child_idx, &owning_node);
        if (rc != CDD_C_SUCCESS)
          return rc;
        CDD_SNPRINTF(snip, sizeof(snip), "%.*s.imag ", (int)next_tok->length,
                     (const char *)next_tok->start);
        rc = gnu_splice_snippet_at(tree, &owning_node, child_idx, 2, snip);
        if (rc != CDD_C_SUCCESS)
          return rc;
        tok->kind = CDD_TOKEN_OTHER;
        next_tok->kind = CDD_TOKEN_OTHER;
        i++;
      }
    } else if (tok->kind == CDD_TOKEN_KEYWORD_TYPEOF) {
      int is_unqual = (tok->length == 13 || tok->length == 17);
      if (ctx_p->config &&
          ctx_p->config->target_profile == CDD_TARGET_PROFILE_MSVC_MODERN &&
          ctx_p->config->msvc_ver >= 1939 && !ctx_p->config->target_c89 &&
          !ctx_p->config->target_c99) {
        const char *kw = is_unqual ? "typeof_unqual" : "typeof";
        rc = replace_token_with_text(tree, tok, CDD_TOKEN_KEYWORD_TYPEOF, kw,
                                     strlen(kw));
        if (rc != CDD_C_SUCCESS)
          return rc;
        continue;
      }
      if (i + 2 < tree->base_tokens->size &&
          tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_LPAREN) {
        size_t rparen_idx = 0;
        int paren_depth = 0;
        size_t j;
        for (j = i + 1; j < tree->base_tokens->size; j++) {
          if (tree->base_tokens->tokens[j].kind == CDD_TOKEN_LPAREN)
            paren_depth++;
          else if (tree->base_tokens->tokens[j].kind == CDD_TOKEN_RPAREN) {
            paren_depth--;
            if (paren_depth == 0) {
              rparen_idx = j;
              break;
            }
          }
        }
        if (rparen_idx > 0) {
          size_t num_inner = rparen_idx - (i + 1) - 1;
          cdd_token_t *inner = &tree->base_tokens->tokens[i + 2];
          size_t child_idx;
          cdd_cst_node_t *owning_node = NULL;
          char eval_buf[512];
          int is_type_expr = 0;
          int has_unmatched_bracket = 0;
          size_t k;
          int b_depth = 0;

          for (k = 0; k < num_inner; k++) {
            if (inner[k].kind == CDD_TOKEN_LBRACKET)
              b_depth++;
            else if (inner[k].kind == CDD_TOKEN_RBRACKET)
              b_depth--;
          }
          if (b_depth != 0)
            has_unmatched_bracket = 1;

          rc = gnu_find_node(tree->root, tok, &child_idx, &owning_node);
          if (rc != CDD_C_SUCCESS)
            return rc;

          eval_buf[0] = '\0';
          if (!has_unmatched_bracket && num_inner > 0) {
            int ev_ok = 0;
            rc = gnu_opt_eval_type_scoped(
                ctx_p, owning_node, inner, num_inner,
                is_unqual ? CDD_CST_TYPE_MODE_UNQUAL : CDD_CST_TYPE_MODE_EXACT,
                eval_buf, sizeof(eval_buf), &is_type_expr, &ev_ok);
            if (rc != CDD_C_SUCCESS)
              return rc;
            if (!ev_ok)
              eval_buf[0] = '\0';
          }

          {
            char *lb_in_eval = strchr(eval_buf, '[');

            if (!has_unmatched_bracket && eval_buf[0] != '\0' && !lb_in_eval &&
                !is_type_expr) {
              char snip[540];
              CDD_SNPRINTF(snip, sizeof(snip), "%s ", eval_buf);
              rc = gnu_splice_snippet_at(tree, &owning_node, child_idx,
                                         (rparen_idx - i) + 1, snip);
              if (rc != CDD_C_SUCCESS)
                return rc;
            } else {
              /* Parse CST declarator or evaluated array type into base + dims
               */
              char base[512];
              char arr_dims[512];
              char *bp = base;
              char *ap = arr_dims;
              size_t first_lbracket = num_inner;
              int has_matching_rbracket = 0;

              base[0] = '\0';
              arr_dims[0] = '\0';

              if (!has_unmatched_bracket && lb_in_eval && !is_type_expr) {
                size_t b_len = (size_t)(lb_in_eval - eval_buf);
                memcpy(base, eval_buf, b_len);
                base[b_len] = ' ';
                base[b_len + 1] = '\0';
                CDD_SNPRINTF(arr_dims, sizeof(arr_dims), "%s", lb_in_eval);
                has_matching_rbracket = 1;
              } else {
                for (k = 0; k < num_inner; k++) {
                  if (inner[k].kind == CDD_TOKEN_LBRACKET &&
                      first_lbracket == num_inner) {
                    first_lbracket = k;
                  } else if (inner[k].kind == CDD_TOKEN_RBRACKET) {
                    has_matching_rbracket = 1;
                  }
                }
                if (has_unmatched_bracket)
                  has_matching_rbracket = 0;

                for (k = 0; k < num_inner; k++) {
                  int is_qual = 0;
                  if (inner[k].kind == CDD_TOKEN_IDENTIFIER) {
                    if ((inner[k].length == 5 &&
                         memcmp(inner[k].start, "const", 5) == 0) ||
                        (inner[k].length == 8 &&
                         memcmp(inner[k].start, "volatile", 8) == 0) ||
                        (inner[k].length == 8 &&
                         memcmp(inner[k].start, "restrict", 8) == 0)) {
                      is_qual = 1;
                    }
                  }
                  if (is_unqual && is_qual) {
                    continue;
                  }
                  if (has_matching_rbracket && k >= first_lbracket) {
                    memcpy(ap, inner[k].start, inner[k].length);
                    ap += inner[k].length;
                  } else {
                    if (bp != base && inner[k].kind != CDD_TOKEN_STAR) {
                      *bp++ = ' ';
                    }
                    memcpy(bp, inner[k].start, inner[k].length);
                    bp += inner[k].length;
                  }
                }
                *bp++ = ' ';
                *bp = '\0';
                *ap = '\0';
              }

              if (has_matching_rbracket) {
                char snip[1280];
                int cur_arr_idx = ctx_p->typeof_arr_idx++;
                CDD_SNPRINTF(snip, sizeof(snip),
                             "typedef %s__cdd_typeof_arr_%d%s; "
                             "__cdd_typeof_arr_%d ",
                             base, cur_arr_idx, arr_dims, cur_arr_idx);
                rc = gnu_splice_snippet_at(tree, &owning_node, child_idx,
                                           (rparen_idx - i) + 1, snip);
                if (rc != CDD_C_SUCCESS)
                  return rc;
              } else {
                rc = gnu_splice_snippet_at(tree, &owning_node, child_idx,
                                           (rparen_idx - i) + 1, base);
                if (rc != CDD_C_SUCCESS)
                  return rc;
              }
            }
            for (k = i; k <= rparen_idx; k++) {
              tree->base_tokens->tokens[k].kind = CDD_TOKEN_OTHER;
            }
            i = rparen_idx;
          }
        }
      }
    } else if (tok->kind == CDD_TOKEN_KEYWORD___AUTO_TYPE) {
      size_t child_idx;
      cdd_cst_node_t *owning_node = NULL;
      rc = gnu_find_node(tree->root, tok, &child_idx, &owning_node);
      if (rc != CDD_C_SUCCESS)
        return rc;
      {
        /* __auto_type inference: search forward for = and infer full RHS */
        size_t j;
        char auto_buf[256];
        char snip[280];
        const char *inferred = "int";
        auto_buf[0] = '\0';
        for (j = i + 1;
             j < tree->base_tokens->size &&
             tree->base_tokens->tokens[j].kind != CDD_TOKEN_SEMICOLON;
             j++) {
          if (tree->base_tokens->tokens[j].kind == CDD_TOKEN_ASSIGN) {
            size_t rhs_end = j + 1;
            int d = 0;
            while (rhs_end < tree->base_tokens->size) {
              enum cdd_token_kind_t rk =
                  tree->base_tokens->tokens[rhs_end].kind;
              if (rk == CDD_TOKEN_LPAREN || rk == CDD_TOKEN_LBRACKET ||
                  rk == CDD_TOKEN_LBRACE)
                d++;
              else if (rk == CDD_TOKEN_RPAREN || rk == CDD_TOKEN_RBRACKET ||
                       rk == CDD_TOKEN_RBRACE) {
                if (d == 0)
                  break;
                d--;
              } else if (d == 0 &&
                         (rk == CDD_TOKEN_SEMICOLON || rk == CDD_TOKEN_COMMA)) {
                break;
              }
              rhs_end++;
            }
            if (rhs_end > j + 1) {
              int is_tn = 0;
              int ev_ok = 0;
              rc = gnu_opt_eval_type_scoped(
                  ctx_p, owning_node, &tree->base_tokens->tokens[j + 1],
                  rhs_end - (j + 1), CDD_CST_TYPE_MODE_RVALUE, auto_buf,
                  sizeof(auto_buf), &is_tn, &ev_ok);
              if (rc != CDD_C_SUCCESS)
                return rc;
              if (ev_ok && !is_tn) {
                inferred = auto_buf;
              }
            }
            break;
          }
        }
        CDD_SNPRINTF(snip, sizeof(snip), "%s ", inferred);
        rc = gnu_splice_snippet_at(tree, &owning_node, child_idx, 1, snip);
        if (rc != CDD_C_SUCCESS)
          return rc;
        tok->kind = CDD_TOKEN_OTHER;
      }
    } else if (tok->kind == CDD_TOKEN_STRING || tok->kind == CDD_TOKEN_CHAR) {
      rc = gnu_lower_esc_e_token(tree, tok);
      if (rc != CDD_C_SUCCESS)
        return rc;
    } else if (tok->kind == CDD_TOKEN_OTHER && tok->length >= 8 &&
               tok->start[0] == '#') {
      const char *wp = (const char *)tok->start + 1;
      size_t rem = tok->length - 1;
      while (rem > 0 && (*wp == ' ' || *wp == '\t')) {
        wp++;
        rem--;
      }
      if (rem >= 7 && memcmp(wp, "warning", 7) == 0 &&
          (rem == 7 || !isalnum((unsigned char)wp[7]))) {
        int is_msvc = 0;
        int is_iso_c89 = 0;
        if (ctx_p->config) {
          rc = gnu_is_msvc_target(ctx_p->config, &is_msvc);
          if (rc != CDD_C_SUCCESS)
            return rc;
          rc = gnu_is_strict_iso_c89(ctx_p->config, &is_iso_c89);
          if (rc != CDD_C_SUCCESS)
            return rc;
        }
        if (is_msvc || is_iso_c89) {
          const char *msg = wp + 7;
          size_t mlen = rem - 7;
          char wbuf[512];
          const char *pooled_w = NULL;
          while (mlen > 0 && (*msg == ' ' || *msg == '\t')) {
            msg++;
            mlen--;
          }
          while (mlen > 0 && (msg[mlen - 1] == '\n' || msg[mlen - 1] == '\r' ||
                              msg[mlen - 1] == ' ' || msg[mlen - 1] == '\t')) {
            mlen--;
          }
          if (mlen >= 2 && msg[0] == '"' && msg[mlen - 1] == '"') {
            msg++;
            mlen -= 2;
          }
          if (is_msvc) {
            CDD_SNPRINTF(wbuf, sizeof(wbuf),
                         "#pragma message(\"warning: %.*s\")", (int)mlen, msg);
          } else {
            CDD_SNPRINTF(wbuf, sizeof(wbuf), "/* #warning %.*s */", (int)mlen,
                         msg);
          }
          rc = cdd_pool_string_safe(tree, wbuf, &pooled_w);
          if (rc != CDD_C_SUCCESS)
            return rc;
          rc = replace_token_with_text(tree, tok, tok->kind, pooled_w,
                                       strlen(pooled_w));
          if (rc != CDD_C_SUCCESS)
            return rc;
        }
      }
    } else if (tok->kind == CDD_TOKEN_NUMBER) {
      char buf[256];
      size_t copy_len = tok->length < 255 ? tok->length : 255;
      const char *u64_sfx = (ctx_p->config && ctx_p->config->msvc_ver > 0 &&
                             ctx_p->config->msvc_ver < 1400)
                                ? "UI64"
                                : "ULL";
      memcpy(buf, tok->start, copy_len);
      buf[copy_len] = '\0';
      if (copy_len >= 3 && buf[0] == '0' && (buf[1] == 'b' || buf[1] == 'B')) {
        uint64_t high = 0;
        uint64_t low = 0;
        size_t sfx_idx = 2;
        rc = cdd_parse_bin_128_literal(buf, copy_len, &high, &low);
        if (rc != CDD_C_SUCCESS)
          return rc;
        while (sfx_idx < copy_len &&
               (buf[sfx_idx] == '0' || buf[sfx_idx] == '1' ||
                buf[sfx_idx] == '\'')) {
          sfx_idx++;
        }
        if (high == 0) {
          char hex_buf[128];
          char sfx_buf[16];
          const char *pooled_hex = NULL;
          size_t sl = copy_len - sfx_idx;
          if (sl >= sizeof(sfx_buf))
            sl = sizeof(sfx_buf) - 1;
          memcpy(sfx_buf, buf + sfx_idx, sl);
          sfx_buf[sl] = '\0';
          if (ctx_p->config && ctx_p->config->msvc_ver > 0 &&
              ctx_p->config->msvc_ver < 1400) {
            if (strcmp(sfx_buf, "ULL") == 0 || strcmp(sfx_buf, "ull") == 0)
              CDD_SNPRINTF(sfx_buf, sizeof(sfx_buf), "UI64");
            else if (strcmp(sfx_buf, "LL") == 0 || strcmp(sfx_buf, "ll") == 0)
              CDD_SNPRINTF(sfx_buf, sizeof(sfx_buf), "I64");
          }
          if ((low >> 32) != 0) {
            CDD_SNPRINTF(hex_buf, sizeof(hex_buf), "0x%lx%08lx%s",
                         (unsigned long)(low >> 32),
                         (unsigned long)(low & 0xFFFFFFFFUL), sfx_buf);
          } else {
            CDD_SNPRINTF(hex_buf, sizeof(hex_buf), "0x%lx%s",
                         (unsigned long)low, sfx_buf);
          }
          rc = cdd_pool_string_safe(tree, hex_buf, &pooled_hex);
          if (rc != CDD_C_SUCCESS)
            return rc;
          rc = replace_token_with_text(tree, tok, CDD_TOKEN_NUMBER, pooled_hex,
                                       strlen(pooled_hex));
          if (rc != CDD_C_SUCCESS)
            return rc;
          continue;
        }
        /* >64-bit binary literal falls through to cdd_make_uint128 */
        ctx_p->used_int128 = 1;
        {
          size_t child_idx;
          cdd_cst_node_t *owning_node = NULL;
          char snip[280];
          rc = gnu_find_node(tree->root, tok, &child_idx, &owning_node);
          if (rc != CDD_C_SUCCESS)
            return rc;
          CDD_SNPRINTF(snip, sizeof(snip),
                       "cdd_make_uint128(0x%08lx%08lx%s, 0x%08lx%08lx%s)",
                       (unsigned long)(high >> 32),
                       (unsigned long)(high & 0xFFFFFFFFUL), u64_sfx,
                       (unsigned long)(low >> 32),
                       (unsigned long)(low & 0xFFFFFFFFUL), u64_sfx);
          rc = gnu_splice_snippet_at(tree, &owning_node, child_idx, 1, snip);
          if (rc != CDD_C_SUCCESS)
            return rc;
          tok->kind = CDD_TOKEN_OTHER;
        }
      } else {
        char clean_num[256];
        size_t clean_len = 0;
        int is_plain_int = 0;
        int fits_64 = 0;
        int exc_64 = 0;
        rc = gnu_strip_and_check_int_literal(buf, copy_len, clean_num,
                                             &clean_len, &is_plain_int);
        if (rc != CDD_C_SUCCESS)
          return rc;
        if (is_plain_int) {
          rc = gnu_classify_num_literal(clean_num, &fits_64, &exc_64);
          if (rc != CDD_C_SUCCESS)
            return rc;
        }
        if (exc_64) {
          /* Exceeds 64-bit */
          uint64_t high = 0, low = 0;
          size_t child_idx;
          cdd_cst_node_t *owning_node = NULL;
          char snip[280];
          ctx_p->used_int128 = 1;
          if (clean_num[0] == '0' &&
              (clean_num[1] == 'x' || clean_num[1] == 'X')) {
            rc = cdd_parse_hex_128_literal(clean_num, clean_len, &high, &low);
            if (rc != CDD_C_SUCCESS)
              return rc;
          } else {
            rc = cdd_parse_128_literal(clean_num, clean_len, &high, &low);
            if (rc != CDD_C_SUCCESS)
              return rc;
          }
          rc = gnu_find_node(tree->root, tok, &child_idx, &owning_node);
          if (rc != CDD_C_SUCCESS)
            return rc;
          CDD_SNPRINTF(snip, sizeof(snip),
                       "cdd_make_uint128(0x%08lx%08lx%s, 0x%08lx%08lx%s)",
                       (unsigned long)(high >> 32),
                       (unsigned long)(high & 0xFFFFFFFF), u64_sfx,
                       (unsigned long)(low >> 32),
                       (unsigned long)(low & 0xFFFFFFFF), u64_sfx);
          rc = gnu_splice_snippet_at(tree, &owning_node, child_idx, 1, snip);
          if (rc != CDD_C_SUCCESS)
            return rc;
          tok->kind = CDD_TOKEN_OTHER;
        }
      }
    } else if (tok->kind == CDD_TOKEN_IDENTIFIER &&
               ((tok->length == 13 &&
                 memcmp(tok->start, "__attribute__", 13) == 0) ||
                (tok->length == 11 &&
                 memcmp(tok->start, "__attribute", 11) == 0))) {
      if (i + 2 < tree->base_tokens->size &&
          tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_LPAREN &&
          tree->base_tokens->tokens[i + 2].kind == CDD_TOKEN_LPAREN) {
        size_t attr_end = 0;
        int lp_cnt = 2;
        size_t scan_k;
        for (scan_k = i + 3; scan_k < tree->base_tokens->size; scan_k++) {
          if (tree->base_tokens->tokens[scan_k].kind == CDD_TOKEN_LPAREN)
            lp_cnt++;
          else if (tree->base_tokens->tokens[scan_k].kind == CDD_TOKEN_RPAREN) {
            lp_cnt--;
            if (lp_cnt == 0) {
              attr_end = scan_k;
              break;
            }
          }
        }
        if (attr_end > i + 3) {
          int is_msvc = 0;
          int is_iso_c89 = 0;
          size_t decl_idx = i;
          size_t item_s = i + 3;
          int has_cleanup_only = 0;

          if (ctx_p->config) {
            rc = gnu_is_msvc_target(ctx_p->config, &is_msvc);
            if (rc != CDD_C_SUCCESS)
              return rc;
            rc = gnu_is_strict_iso_c89(ctx_p->config, &is_iso_c89);
            if (rc != CDD_C_SUCCESS)
              return rc;
          }
          rc = gnu_find_decl_start(tree->base_tokens->tokens, i, &decl_idx);
          if (rc != CDD_C_SUCCESS)
            return rc;
          if (decl_idx == i && attr_end + 1 < tree->base_tokens->size) {
            decl_idx = attr_end + 1;
          }

          /* Check if single `cleanup(fn)` / `__cleanup__(fn)`: leave for
             `gnu_standardize_unroll_ctx`. */
          {
            char anm0[64];
            rc = gnu_norm_attr_name(tree->base_tokens->tokens[i + 3].start,
                                    tree->base_tokens->tokens[i + 3].length,
                                    anm0, sizeof(anm0));
            if (rc != CDD_C_SUCCESS)
              return rc;
            if (strcmp(anm0, "cleanup") == 0)
              has_cleanup_only = 1;
          }
          if (has_cleanup_only)
            continue;

          /* Preserve exact legacy single-arg `aligned(N)` CST splice on
             default non-MSVC, non-C89 targets so existing node-OOM tests hold
           */
          if (!is_msvc && !is_iso_c89 && attr_end == i + 8 &&
              tree->base_tokens->tokens[i + 4].kind == CDD_TOKEN_LPAREN &&
              tree->base_tokens->tokens[i + 6].kind == CDD_TOKEN_RPAREN) {
            char anm_al[64];
            rc = gnu_norm_attr_name(tree->base_tokens->tokens[i + 3].start,
                                    tree->base_tokens->tokens[i + 3].length,
                                    anm_al, sizeof(anm_al));
            if (rc != CDD_C_SUCCESS)
              return rc;
            if (strcmp(anm_al, "aligned") == 0) {
              size_t child_idx;
              cdd_cst_node_t *owning_node = NULL;
              char snip[160];
              rc = gnu_find_node(tree->root, tok, &child_idx, &owning_node);
              if (rc != CDD_C_SUCCESS)
                return rc;
              CDD_SNPRINTF(
                  snip, sizeof(snip), "_Alignas(%.*s)",
                  (int)tree->base_tokens->tokens[i + 5].length,
                  (const char *)tree->base_tokens->tokens[i + 5].start);
              rc =
                  gnu_splice_snippet_at(tree, &owning_node, child_idx, 9, snip);
              if (rc != CDD_C_SUCCESS)
                return rc;
              tok->kind = CDD_TOKEN_OTHER;
              i = attr_end;
              continue;
            }
          }

          /* Walk comma-separated items inside `__attribute__(( ... ))` */
          {
            const char *inline_repl = NULL;
            char inline_dyn[256];
            int did_action = 0;
            size_t item_Count = 0;
            inline_dyn[0] = '\0';

            while (item_s < attr_end - 1) {
              size_t item_e = item_s;
              int pd = 0;
              char aname[64];
              while (item_e < attr_end - 1) {
                if (tree->base_tokens->tokens[item_e].kind == CDD_TOKEN_LPAREN)
                  pd++;
                else if (tree->base_tokens->tokens[item_e].kind ==
                         CDD_TOKEN_RPAREN) {
                  pd--;
                } else if (pd == 0 && tree->base_tokens->tokens[item_e].kind ==
                                          CDD_TOKEN_COMMA) {
                  break;
                }
                item_e++;
              }
              if (item_e > item_s) {
                cdd_token_t *at = &tree->base_tokens->tokens[item_s];
                item_Count++;
                rc = gnu_norm_attr_name(at->start, at->length, aname,
                                        sizeof(aname));
                if (rc != CDD_C_SUCCESS)
                  return rc;

                if (strcmp(aname, "packed") == 0) {
                  size_t push_idx = decl_idx;
                  size_t k_semi;
                  int bd = 0;
                  rc = gnu_prepend_tok(tree,
                                       &tree->base_tokens->tokens[push_idx],
                                       "\n#pragma pack(push, 1)\n");
                  if (rc != CDD_C_SUCCESS)
                    return rc;
                  for (k_semi = i; k_semi < tree->base_tokens->size; k_semi++) {
                    if (tree->base_tokens->tokens[k_semi].kind ==
                        CDD_TOKEN_LBRACE)
                      bd++;
                    else if (tree->base_tokens->tokens[k_semi].kind ==
                             CDD_TOKEN_RBRACE)
                      bd--;
                    else if (bd == 0 &&
                             tree->base_tokens->tokens[k_semi].kind ==
                                 CDD_TOKEN_SEMICOLON) {
                      rc = gnu_append_tok(tree,
                                          &tree->base_tokens->tokens[k_semi],
                                          "\n#pragma pack(pop)\n");
                      if (rc != CDD_C_SUCCESS)
                        return rc;
                      break;
                    }
                  }
                  inline_repl = "";
                  did_action = 1;
                } else if (strcmp(aname, "aligned") == 0) {
                  char al_val[64];
                  al_val[0] = '\0';
                  if (item_s + 2 < item_e &&
                      tree->base_tokens->tokens[item_s + 1].kind ==
                          CDD_TOKEN_LPAREN) {
                    size_t av = item_s + 2;
                    size_t avl = tree->base_tokens->tokens[av].length < 63
                                     ? tree->base_tokens->tokens[av].length
                                     : 63;
                    memcpy(al_val, tree->base_tokens->tokens[av].start, avl);
                    al_val[avl] = '\0';
                  } else {
                    CDD_SNPRINTF(al_val, sizeof(al_val), "16");
                  }
                  if (is_msvc) {
                    char dspec[96];
                    CDD_SNPRINTF(dspec, sizeof(dspec), "__declspec(align(%s)) ",
                                 al_val);
                    rc = gnu_prepend_tok(
                        tree, &tree->base_tokens->tokens[decl_idx], dspec);
                    if (rc != CDD_C_SUCCESS)
                      return rc;
                    inline_repl = "";
                  } else if (is_iso_c89) {
                    CDD_SNPRINTF(inline_dyn, sizeof(inline_dyn),
                                 "/* aligned(%s) */", al_val);
                    inline_repl = inline_dyn;
                  } else {
                    CDD_SNPRINTF(inline_dyn, sizeof(inline_dyn), "_Alignas(%s)",
                                 al_val);
                    inline_repl = inline_dyn;
                  }
                  did_action = 1;
                } else if (strcmp(aname, "noreturn") == 0) {
                  if (is_msvc) {
                    rc = gnu_prepend_tok(tree,
                                         &tree->base_tokens->tokens[decl_idx],
                                         "__declspec(noreturn) ");
                    if (rc != CDD_C_SUCCESS)
                      return rc;
                    inline_repl = "";
                  } else if (is_iso_c89) {
                    inline_repl = "/* noreturn */";
                  } else {
                    inline_repl = "_Noreturn";
                  }
                  did_action = 1;
                } else if (strcmp(aname, "noinline") == 0) {
                  if (is_msvc) {
                    rc = gnu_prepend_tok(tree,
                                         &tree->base_tokens->tokens[decl_idx],
                                         "__declspec(noinline) ");
                    if (rc != CDD_C_SUCCESS)
                      return rc;
                    inline_repl = "";
                    did_action = 1;
                  } else {
                    inline_repl = "/* noinline */";
                  }
                } else if (strcmp(aname, "always_inline") == 0) {
                  if (is_msvc) {
                    size_t ki;
                    for (ki = decl_idx; ki < attr_end; ki++) {
                      cdd_token_t *itk = &tree->base_tokens->tokens[ki];
                      if (itk->kind == CDD_TOKEN_IDENTIFIER &&
                          ((itk->length == 6 &&
                            memcmp(itk->start, "inline", 6) == 0) ||
                           (itk->length == 8 &&
                            memcmp(itk->start, "__inline", 8) == 0) ||
                           (itk->length == 10 &&
                            memcmp(itk->start, "__inline__", 10) == 0))) {
                        rc = replace_token_with_text(tree, itk, itk->kind, "",
                                                     0);
                        if (rc != CDD_C_SUCCESS)
                          return rc;
                      }
                    }
                    rc = gnu_prepend_tok(tree,
                                         &tree->base_tokens->tokens[decl_idx],
                                         "__forceinline ");
                    if (rc != CDD_C_SUCCESS)
                      return rc;
                    inline_repl = "";
                    did_action = 1;
                  } else {
                    inline_repl = "/* always_inline */";
                  }
                } else if (strcmp(aname, "visibility") == 0) {
                  int is_default = 0;
                  if (item_s + 2 < item_e &&
                      tree->base_tokens->tokens[item_s + 2].length >= 9 &&
                      memcmp(tree->base_tokens->tokens[item_s + 2].start,
                             "\"default\"", 9) == 0) {
                    is_default = 1;
                  }
                  if (is_msvc && is_default) {
                    rc = gnu_prepend_tok(tree,
                                         &tree->base_tokens->tokens[decl_idx],
                                         "__declspec(dllexport) ");
                    if (rc != CDD_C_SUCCESS)
                      return rc;
                    inline_repl = "";
                    did_action = 1;
                  } else {
                    inline_repl = "/* attribute */";
                  }
                } else if (strcmp(aname, "deprecated") == 0) {
                  if (is_msvc) {
                    rc = gnu_prepend_tok(tree,
                                         &tree->base_tokens->tokens[decl_idx],
                                         "__declspec(deprecated) ");
                    if (rc != CDD_C_SUCCESS)
                      return rc;
                    inline_repl = "";
                    did_action = 1;
                  }
                } else if (strcmp(aname, "section") == 0 && is_msvc &&
                           item_s + 2 < item_e &&
                           tree->base_tokens->tokens[item_s + 2].kind ==
                               CDD_TOKEN_STRING) {
                  char s_spec[128];
                  cdd_token_t *st = &tree->base_tokens->tokens[item_s + 2];
                  CDD_SNPRINTF(s_spec, sizeof(s_spec),
                               "__declspec(allocate(%.*s)) ", (int)st->length,
                               (const char *)st->start);
                  rc = gnu_prepend_tok(
                      tree, &tree->base_tokens->tokens[decl_idx], s_spec);
                  if (rc != CDD_C_SUCCESS)
                    return rc;
                  inline_repl = "";
                  did_action = 1;
                } else if (strcmp(aname, "warn_unused_result") == 0 &&
                           is_msvc) {
                  rc = gnu_prepend_tok(tree,
                                       &tree->base_tokens->tokens[decl_idx],
                                       "_Check_return_ ");
                  if (rc != CDD_C_SUCCESS)
                    return rc;
                  inline_repl = "";
                  did_action = 1;
                } else if (strcmp(aname, "format") == 0 &&
                           item_s + 4 < item_e) {
                  char archetype[32];
                  cdd_token_t *arch_t = &tree->base_tokens->tokens[item_s + 2];
                  rc = gnu_norm_attr_name(arch_t->start, arch_t->length,
                                          archetype, sizeof(archetype));
                  if (rc != CDD_C_SUCCESS)
                    return rc;
                  if (is_msvc && strcmp(archetype, "printf") == 0 &&
                      tree->base_tokens->tokens[item_s + 4].kind ==
                          CDD_TOKEN_NUMBER) {
                    char fnum[16];
                    size_t fl = tree->base_tokens->tokens[item_s + 4].length;
                    if (fl >= sizeof(fnum))
                      fl = sizeof(fnum) - 1;
                    memcpy(fnum, tree->base_tokens->tokens[item_s + 4].start,
                           fl);
                    fnum[fl] = '\0';
                    rc = gnu_inject_printf_sal(tree, decl_idx, attr_end,
                                               atoi(fnum));
                    if (rc != CDD_C_SUCCESS)
                      return rc;
                    inline_repl = "";
                  } else {
                    inline_repl = "/* attribute */";
                  }
                  did_action = 1;
                } else if ((strcmp(aname, "constructor") == 0 ||
                            strcmp(aname, "destructor") == 0) &&
                           is_msvc) {
                  rc = gnu_emit_msvc_crt_xcu(
                      tree, decl_idx, attr_end,
                      strcmp(aname, "destructor") == 0 ? 1 : 0);
                  if (rc != CDD_C_SUCCESS)
                    return rc;
                  inline_repl = "";
                  did_action = 1;
                } else if (strcmp(aname, "vector_size") == 0) {
                  inline_repl = "";
                  did_action = 1;
                } else if (strcmp(aname, "mode") == 0 && item_s + 2 < item_e) {
                  char mnorm[16];
                  cdd_token_t *mv = &tree->base_tokens->tokens[item_s + 2];
                  rc = gnu_norm_attr_name(mv->start, mv->length, mnorm,
                                          sizeof(mnorm));
                  if (rc != CDD_C_SUCCESS)
                    return rc;
                  if (strcmp(mnorm, "QI") == 0)
                    inline_repl = "/* mode(QI) -> int8_t */";
                  else if (strcmp(mnorm, "HI") == 0)
                    inline_repl = "/* mode(HI) -> int16_t */";
                  else if (strcmp(mnorm, "SI") == 0)
                    inline_repl = "/* mode(SI) -> int32_t */";
                  else if (strcmp(mnorm, "DI") == 0)
                    inline_repl = "/* mode(DI) -> int64_t */";
                  else if (strcmp(mnorm, "TI") == 0)
                    inline_repl = "/* mode(TI) -> int128_t */";
                  else
                    inline_repl = "";
                  did_action = 1;
                } else if (strcmp(aname, "unused") == 0) {
                  inline_repl = "/* unused */";
                  did_action = 1;
                } else if (strcmp(aname, "transparent_union") == 0) {
                  inline_repl = "/* transparent_union */";
                  did_action = 1;
                }
              }
              item_s = (item_e < attr_end - 1) ? (item_e + 1) : item_e;
            }

            if (!did_action) {
              if (item_Count == 1 && attr_end == i + 5) {
                cdd_token_t *attr = &tree->base_tokens->tokens[i + 3];
                char *heap_buf = NULL;
                const char *pooled = NULL;
                rc = gnu_malloc(attr->length + 7, (void **)&heap_buf);
                if (rc != CDD_C_SUCCESS)
                  return rc;
                memcpy(heap_buf, "/* ", 3);
                memcpy(heap_buf + 3, attr->start, attr->length);
                memcpy(heap_buf + 3 + attr->length, " */", 3);
                heap_buf[6 + attr->length] = '\0';
                rc = cdd_pool_string_safe(tree, heap_buf, &pooled);
                if (rc != CDD_C_SUCCESS) {
                  C_CDD_FREE(heap_buf);
                  return rc;
                }
                C_CDD_FREE(heap_buf);
                inline_repl = pooled;
              } else {
                inline_repl = "/* attribute */";
              }
            } else if (inline_repl == inline_dyn) {
              const char *pooled_d = NULL;
              rc = cdd_pool_string_safe(tree, inline_dyn, &pooled_d);
              if (rc != CDD_C_SUCCESS)
                return rc;
              inline_repl = pooled_d;
            }

            rc = replace_token_with_text(tree, tok, tok->kind, inline_repl,
                                         strlen(inline_repl));
            if (rc != CDD_C_SUCCESS)
              return rc;
            {
              size_t wipe;
              for (wipe = i + 1; wipe <= attr_end; wipe++) {
                rc = replace_token_with_text(
                    tree, &tree->base_tokens->tokens[wipe],
                    tree->base_tokens->tokens[wipe].kind, "", 0);
                if (rc != CDD_C_SUCCESS)
                  return rc;
              }
            }
          }
        }
      }
    }
  }

  if (saw_variadic_macro && ctx_p->config) {
    int is_iso_c89 = 0;
    rc = gnu_is_strict_iso_c89(ctx_p->config, &is_iso_c89);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (is_iso_c89) {
      rc = cdd_transform_macros(tree, ctx_p->config);
      if (rc != CDD_C_SUCCESS)
        return rc;
      /* Expand variadic macro call sites for strict ISO C89 */
      for (i = 0; i < tree->base_tokens->size; i++) {
        cdd_token_t *dt = &tree->base_tokens->tokens[i];
        if (dt->kind == CDD_TOKEN_PREPROC_DEFINE) {
          const char *dp = (const char *)dt->start;
          size_t dlen = dt->length;
          char mname[64];
          char fparam[64];
          char mbody[256];
          size_t p_i = 0;
          mname[0] = '\0';
          fparam[0] = '\0';
          mbody[0] = '\0';
          while (p_i < dlen && dp[p_i] != ' ' && dp[p_i] != '\t')
            p_i++;
          while (p_i < dlen && (dp[p_i] == ' ' || dp[p_i] == '\t'))
            p_i++;
          {
            size_t ms = p_i;
            while (p_i < dlen &&
                   (isalnum((unsigned char)dp[p_i]) || dp[p_i] == '_'))
              p_i++;
            if (p_i > ms && p_i - ms < sizeof(mname) && p_i < dlen &&
                dp[p_i] == '(') {
              size_t ps;
              memcpy(mname, dp + ms, p_i - ms);
              mname[p_i - ms] = '\0';
              p_i++;
              while (p_i < dlen && (dp[p_i] == ' ' || dp[p_i] == '\t'))
                p_i++;
              ps = p_i;
              while (p_i < dlen &&
                     (isalnum((unsigned char)dp[p_i]) || dp[p_i] == '_'))
                p_i++;
              if (p_i < dlen && dp[p_i] == ',') {
                size_t fl = p_i - ps < 63 ? p_i - ps : 63;
                memcpy(fparam, dp + ps, fl);
                fparam[fl] = '\0';
              }
              while (p_i < dlen && dp[p_i] != ')')
                p_i++;
              if (p_i < dlen) {
                size_t bl;
                p_i++;
                while (p_i < dlen && (dp[p_i] == ' ' || dp[p_i] == '\t'))
                  p_i++;
                bl = dlen - p_i;
                while (bl > 0 &&
                       (dp[p_i + bl - 1] == '\n' || dp[p_i + bl - 1] == '\r'))
                  bl--;
                if (bl < sizeof(mbody)) {
                  memcpy(mbody, dp + p_i, bl);
                  mbody[bl] = '\0';
                }
              }
            }
          }

          if (mbody[0] != '\0') {
            size_t ci;
            size_t mnl = strlen(mname);
            for (ci = i + 1; ci + 2 < tree->base_tokens->size; ci++) {
              cdd_token_t *ct = &tree->base_tokens->tokens[ci];
              if (ct->kind == CDD_TOKEN_IDENTIFIER && ct->length == mnl &&
                  memcmp(ct->start, mname, mnl) == 0 &&
                  tree->base_tokens->tokens[ci + 1].kind == CDD_TOKEN_LPAREN) {
                size_t rp = 0;
                size_t first_comma = 0;
                int pd = 0;
                size_t ak;
                for (ak = ci + 1; ak < tree->base_tokens->size; ak++) {
                  if (tree->base_tokens->tokens[ak].kind == CDD_TOKEN_LPAREN)
                    pd++;
                  else if (tree->base_tokens->tokens[ak].kind ==
                           CDD_TOKEN_RPAREN) {
                    pd--;
                    if (pd == 0) {
                      rp = ak;
                      break;
                    }
                  } else if (pd == 1 && first_comma == 0 &&
                             tree->base_tokens->tokens[ak].kind ==
                                 CDD_TOKEN_COMMA) {
                    first_comma = ak;
                  }
                }
                if (rp > ci + 1) {
                  char arg0[128];
                  char vargs[256];
                  char exp_buf[512];
                  const char *pooled_exp = NULL;
                  size_t wipe_c;
                  arg0[0] = '\0';
                  vargs[0] = '\0';
                  if (fparam[0] != '\0') {
                    size_t a0_end = first_comma ? first_comma : rp;
                    if (a0_end > ci + 2) {
                      const uint8_t *as =
                          tree->base_tokens->tokens[ci + 2].start;
                      const uint8_t *ae =
                          tree->base_tokens->tokens[a0_end - 1].start +
                          tree->base_tokens->tokens[a0_end - 1].length;
                      size_t al = (size_t)(ae - as);
                      if (al < sizeof(arg0)) {
                        memcpy(arg0, as, al);
                        arg0[al] = '\0';
                      }
                    }
                    if (first_comma && first_comma + 1 < rp) {
                      const uint8_t *vs =
                          tree->base_tokens->tokens[first_comma + 1].start;
                      const uint8_t *ve =
                          tree->base_tokens->tokens[rp - 1].start +
                          tree->base_tokens->tokens[rp - 1].length;
                      size_t vl = (size_t)(ve - vs);
                      if (vl < sizeof(vargs)) {
                        memcpy(vargs, vs, vl);
                        vargs[vl] = '\0';
                      }
                    }
                  } else if (rp > ci + 2) {
                    const uint8_t *vs = tree->base_tokens->tokens[ci + 2].start;
                    const uint8_t *ve =
                        tree->base_tokens->tokens[rp - 1].start +
                        tree->base_tokens->tokens[rp - 1].length;
                    size_t vl = (size_t)(ve - vs);
                    if (vl < sizeof(vargs)) {
                      memcpy(vargs, vs, vl);
                      vargs[vl] = '\0';
                    }
                  }
                  /* Substitute fixed param and variadic args into mbody */
                  {
                    char *ep = exp_buf;
                    const char *bp_in = mbody;
                    size_t fpl = strlen(fparam);
                    while (*bp_in &&
                           (size_t)(ep - exp_buf) + 64 < sizeof(exp_buf)) {
                      if (fpl > 0 && strncmp(bp_in, fparam, fpl) == 0 &&
                          !isalnum((unsigned char)bp_in[fpl]) &&
                          bp_in[fpl] != '_') {
                        size_t a0l = strlen(arg0);
                        memcpy(ep, arg0, a0l);
                        ep += a0l;
                        bp_in += fpl;
                      } else if (bp_in[0] == ',') {
                        const char *look = bp_in + 1;
                        while (*look == ' ' || *look == '\t')
                          look++;
                        if (look[0] == '#' && look[1] == '#') {
                          look += 2;
                          while (*look == ' ' || *look == '\t')
                            look++;
                          if (vargs[0] != '\0') {
                            size_t vl = strlen(vargs);
                            *ep++ = ',';
                            *ep++ = ' ';
                            memcpy(ep, vargs, vl);
                            ep += vl;
                          }
                          while (isalnum((unsigned char)*look) || *look == '_')
                            look++;
                          bp_in = look;
                        } else if (strncmp(look, "__VA_ARGS__", 11) == 0 &&
                                   !isalnum((unsigned char)look[11]) &&
                                   look[11] != '_') {
                          if (vargs[0] != '\0') {
                            size_t vl = strlen(vargs);
                            *ep++ = ',';
                            *ep++ = ' ';
                            memcpy(ep, vargs, vl);
                            ep += vl;
                          }
                          bp_in = look + 11;
                        } else {
                          *ep++ = *bp_in++;
                        }
                      } else if (strncmp(bp_in, "__VA_ARGS__", 11) == 0 &&
                                 !isalnum((unsigned char)bp_in[11]) &&
                                 bp_in[11] != '_') {
                        size_t vl = strlen(vargs);
                        if (vl > 0) {
                          memcpy(ep, vargs, vl);
                          ep += vl;
                        }
                        bp_in += 11;
                      } else {
                        *ep++ = *bp_in++;
                      }
                    }
                    *ep = '\0';
                  }
                  rc = cdd_pool_string_safe(tree, exp_buf, &pooled_exp);
                  if (rc != CDD_C_SUCCESS)
                    return rc;
                  rc = replace_token_with_text(tree, ct, CDD_TOKEN_IDENTIFIER,
                                               pooled_exp, strlen(pooled_exp));
                  if (rc != CDD_C_SUCCESS)
                    return rc;
                  for (wipe_c = ci + 1; wipe_c <= rp; wipe_c++) {
                    rc = replace_token_with_text(
                        tree, &tree->base_tokens->tokens[wipe_c],
                        tree->base_tokens->tokens[wipe_c].kind, "", 0);
                    if (rc != CDD_C_SUCCESS)
                      return rc;
                  }
                  ci = rp;
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
 * @brief Standardizes GNU type and macro extensions without external context.
 *
 * @param[in,out] tree CST tree to transform.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
cdd_c_error_t gnu_standardize_types(cdd_cst_tree_t *tree) {
  return gnu_standardize_types_ctx(tree, NULL);
}
