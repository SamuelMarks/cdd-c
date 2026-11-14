/**
 * @file c_lowering_modern.c
 * @brief C99/C11/C23 lowering passes required on both Legacy and Modern MSVC.
 */

/* clang-format off */
#include "c_lowering_internal.h"
/* clang-format on */

/**
 * @brief Appends a string fragment into `buf` within `cap`.
 *
 * @param[in,out] buf Destination buffer.
 * @param[in] cap Capacity of `buf`.
 * @param[in,out] io_len Current string length in `buf`.
 * @param[in] str Fragment to append.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t c_low_buf_cat(char *buf, size_t cap, size_t *io_len,
                                   const char *str) {
  size_t sl = strlen(str);
  if (*io_len + sl + 1 < cap) {
    memcpy(buf + *io_len, str, sl);
    *io_len += sl;
    buf[*io_len] = '\0';
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks whether token `t`'s bounded slice contains `needle`.
 *
 * @param[in] t Token to inspect.
 * @param[in] needle Substring to match.
 * @param[out] out_has Receives 1 if found within `t->length`, 0 otherwise.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t c_low_tok_contains(const cdd_token_t *t,
                                        const char *needle, int *out_has) {
  size_t nlen = strlen(needle);
  size_t k;
  *out_has = 0;
  for (k = 0; k + nlen <= t->length; k++) {
    if (memcmp(t->start + k, needle, nlen) == 0) {
      *out_has = 1;
      return CDD_C_SUCCESS;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Strips whitespace and newline trivia nodes while preserving comments.
 *
 * @param[in,out] head_ptr Address of trivia list head pointer.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t c_low_strip_ws_trivia(cdd_trivia_t **head_ptr) {
  cdd_trivia_t *cur = *head_ptr;
  cdd_trivia_t *prev = NULL;
  while (cur) {
    cdd_trivia_t *nxt = cur->next;
    if (cur->kind == TRIVIA_WHITESPACE || cur->kind == TRIVIA_NEWLINE) {
      if (prev)
        prev->next = nxt;
      else
        *head_ptr = nxt;
      C_CDD_FREE(cur);
    } else {
      prev = cur;
    }
    cur = nxt;
  }
  return CDD_C_SUCCESS;
}

/*
 * @brief Prepends a raw C snippet node at index 0 of `tree->root`.
 */
cdd_c_error_t c_low_prepend_root(cdd_cst_tree_t *tree, const char *snippet) {
  const char *pooled = NULL;
  cdd_cst_node_t *node = NULL;
  cdd_cst_builder_t bld;
  cdd_c_error_t rc;
  if (!tree || !tree->root || !snippet)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = cdd_pool_string_safe(tree, snippet, &pooled);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = cdd_cst_alloc_node(CDD_CST_UNKNOWN, &node);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = (cdd_c_error_t)((int)rc | (int)cdd_cst_builder_init(&bld, tree, node));
  rc = (cdd_c_error_t)((int)rc | (int)cdd_cst_bld_snippet(&bld, pooled));
  if (bld.error_state != CDD_C_SUCCESS) {
    rc = (cdd_c_error_t)((int)CDD_C_ERROR_MEMORY |
                         (int)cdd_cst_builder_free(&bld) |
                         (int)cdd_cst_free_node_only(node));
    return rc;
  }
  rc = cdd_cst_splice_children(tree, &tree->root, 0, 0, node->children,
                               node->num_children);
  rc = (cdd_c_error_t)((int)rc | (int)cdd_cst_builder_free(&bld) |
                       (int)cdd_cst_free_node_only(node));
  return rc;
}

/*
 * @brief Extracts space-separated token text from `toks[s..e_excl)`.
 */
cdd_c_error_t c_low_slice_text(const cdd_token_t *toks, size_t s, size_t e_excl,
                               char *buf, size_t cap) {
  size_t pos = 0;
  size_t k;
  size_t prev_end_off = 0;
  int have_prev = 0;
  if (!buf || cap == 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  buf[0] = '\0';
  if (!toks || s >= e_excl)
    return CDD_C_SUCCESS;
  for (k = s; k < e_excl; k++) {
    if (toks[k].length == 0)
      continue;
    if (have_prev && pos + 1 < cap) {
      int prev_alnum = (pos > 0 && (isalnum((unsigned char)buf[pos - 1]) ||
                                    buf[pos - 1] == '_'));
      int cur_alnum =
          (isalnum((unsigned char)toks[k].start[0]) || toks[k].start[0] == '_');
      if (toks[k].offset > prev_end_off || (prev_alnum && cur_alnum)) {
        buf[pos++] = ' ';
      }
    }
    if (pos + toks[k].length < cap) {
      memcpy(buf + pos, toks[k].start, toks[k].length);
      pos += toks[k].length;
    }
    prev_end_off = toks[k].offset + toks[k].length;
    have_prev = 1;
  }
  buf[pos] = '\0';
  return CDD_C_SUCCESS;
}

/*
 * @brief Replaces token span `toks[s..e_incl]` with `repl` on `toks[s]` and
 * wipes `toks[s+1..e_incl]`.
 */
cdd_c_error_t c_low_replace_span(cdd_cst_tree_t *tree, size_t s, size_t e_incl,
                                 const char *repl) {
  const char *pooled = NULL;
  cdd_c_error_t rc;
  size_t w;
  rc = cdd_pool_string_safe(tree, repl, &pooled);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = replace_token_with_text(tree, &tree->base_tokens->tokens[s],
                               CDD_TOKEN_IDENTIFIER, pooled, strlen(pooled));
  if (rc != CDD_C_SUCCESS)
    return rc;
  for (w = s + 1; w <= e_incl; w++) {
    size_t c_idx = 0;
    cdd_cst_node_t *p_node = NULL;
    if (tree->base_tokens->tokens[w].length == 0 &&
        cdd_cst_find_node_for_token(tree->root, &tree->base_tokens->tokens[w],
                                    &c_idx, &p_node) != CDD_C_SUCCESS) {
      continue;
    }
    rc = replace_token_with_text(tree, &tree->base_tokens->tokens[w],
                                 CDD_TOKEN_OTHER, "", 0);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc =
        (cdd_c_error_t)((int)rc | (int)cdd_cst_find_node_for_token(
                                      tree->root, &tree->base_tokens->tokens[w],
                                      &c_idx, &p_node));
    rc = (cdd_c_error_t)((int)rc | (int)c_low_strip_ws_trivia(
                                       &p_node->children[c_idx]
                                            .val.token->leading_trivia));
    if (w < e_incl) {
      rc = (cdd_c_error_t)((int)rc | (int)c_low_strip_ws_trivia(
                                         &p_node->children[c_idx]
                                              .val.token->trailing_trivia));
    }
  }
  return CDD_C_SUCCESS;
}

/*
 * @brief Prepends `prefix` onto `tok` in `tree`.
 */
cdd_c_error_t c_low_prepend_tok(cdd_cst_tree_t *tree, cdd_token_t *tok,
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
  C_CDD_FREE(buf);
  if (rc != CDD_C_SUCCESS)
    return rc;
  return replace_token_with_text(tree, tok, tok->kind, pooled,
                                 plen + tok->length);
}

/*
 * @brief Appends `suffix` onto `tok` in `tree`.
 */
cdd_c_error_t c_low_append_tok(cdd_cst_tree_t *tree, cdd_token_t *tok,
                               const char *suffix) {
  size_t slen = strlen(suffix);
  char *buf = NULL;
  const char *pooled = NULL;
  cdd_c_error_t rc;
  rc = gnu_malloc(tok->length + slen + 1, (void **)&buf);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (tok->length > 0)
    memcpy(buf, tok->start, tok->length);
  memcpy(buf + tok->length, suffix, slen);
  buf[tok->length + slen] = '\0';
  rc = cdd_pool_string_safe(tree, buf, &pooled);
  C_CDD_FREE(buf);
  if (rc != CDD_C_SUCCESS)
    return rc;
  return replace_token_with_text(tree, tok, tok->kind, pooled,
                                 tok->length + slen);
}

/**
 * @brief Checks if `toks[s..e_excl)` contains a runtime variable reference
 * (making an array bound a C99 VLA rather than an integer constant expression).
 *
 * @param[in] ctx Lowering context.
 * @param[in] toks Token array.
 * @param[in] s Start index.
 * @param[in] e_excl End index (exclusive).
 * @param[out] out_is_vla Set to 1 if runtime VLA bound.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t c_low_is_vla_bound(struct c_lowering_ctx_t *ctx,
                                        const cdd_token_t *toks, size_t s,
                                        size_t e_excl, int *out_is_vla) {
  size_t k;
  *out_is_vla = 0;
  for (k = s; k < e_excl; k++) {
    const cdd_token_t *tk = &toks[k];
    if (tk->length == 0)
      continue;
    if (tk->kind == CDD_TOKEN_IDENTIFIER && tk->length == 6 &&
        memcmp(tk->start, "sizeof", 6) == 0) {
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
    if (tk->kind == CDD_TOKEN_IDENTIFIER && tk->length < 120) {
      char nm[128];
      cdd_cst_symbol_t *esym = NULL;
      memcpy(nm, tk->start, tk->length);
      nm[tk->length] = '\0';
      if (cdd_cst_scope_lookup_in_scope(ctx->sem_env->global_scope, nm,
                                        CDD_CST_SYMBOL_ENUM_CONSTANT,
                                        &esym) == CDD_C_SUCCESS &&
          esym->kind == CDD_CST_SYMBOL_ENUM_CONSTANT) {
        continue;
      }
      *out_is_vla = 1;
      return CDD_C_SUCCESS;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Lowers C23 additions (`[[...]]`, `nullptr`, `constexpr`, `1'000`,
 * `<stdckdint.h>`) and `restrict` / `_Thread_local` / `__thread` keywords.
 *
 * @param[in,out] ctx Lowering context.
 * @return CDD_C_SUCCESS or error code.
 */
static cdd_c_error_t c_lower_keywords_and_c23(struct c_lowering_ctx_t *ctx) {
  cdd_cst_tree_t *tree = ctx->tree;
  size_t i;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  for (i = 0; i < tree->base_tokens->size; i++) {
    cdd_token_t *t = &tree->base_tokens->tokens[i];
    if (t->length == 0)
      continue;

    /* Strip C23 digit separators (`1'000` -> `1000`) in numeric literals, or if
       lexer split `1` `'000'` across adjacent tokens */
    if (t->kind == CDD_TOKEN_NUMBER && memchr(t->start, '\'', t->length)) {
      char nbuf[128];
      size_t k = 0;
      size_t w = 0;
      const char *pn = NULL;
      while (k < t->length && w + 1 < sizeof(nbuf)) {
        if (t->start[k] != '\'')
          nbuf[w++] = (char)t->start[k];
        k++;
      }
      nbuf[w] = '\0';
      rc = cdd_pool_string_safe(tree, nbuf, &pn);
      if (rc != CDD_C_SUCCESS)
        return rc;
      rc = replace_token_with_text(tree, t, CDD_TOKEN_NUMBER, pn, w);
      if (rc != CDD_C_SUCCESS)
        return rc;
      continue;
    }

    /* `#include <stdckdint.h>` */
    if (t->kind == CDD_TOKEN_PREPROC_INCLUDE) {
      int has_ckd = 0;
      rc = c_low_tok_contains(t, "stdckdint.h", &has_ckd);
      if (has_ckd) {
        ctx->used_ckd = 1;
        rc = replace_token_with_text(tree, t, t->kind,
                                     "/* <stdckdint.h> lowered */\n", 28);
        if (rc != CDD_C_SUCCESS)
          return rc;
        continue;
      }
    }

    /* C23 `[[ ... ]]` attributes */
    if (t->kind == CDD_TOKEN_LBRACKET && i + 3 < tree->base_tokens->size &&
        tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_LBRACKET &&
        tree->base_tokens->tokens[i + 1].offset == t->offset + 1) {
      size_t k = i + 2;
      int bd = 2;
      size_t end_rb = 0;
      char attr_nm[64];
      attr_nm[0] = '\0';
      for (; k < tree->base_tokens->size; k++) {
        if (tree->base_tokens->tokens[k].kind == CDD_TOKEN_LBRACKET)
          bd++;
        else if (tree->base_tokens->tokens[k].kind == CDD_TOKEN_RBRACKET) {
          bd--;
          if (bd == 0) {
            end_rb = k;
            break;
          }
        } else if (attr_nm[0] == '\0' &&
                   tree->base_tokens->tokens[k].kind == CDD_TOKEN_IDENTIFIER) {
          size_t al = tree->base_tokens->tokens[k].length < 63
                          ? tree->base_tokens->tokens[k].length
                          : 63;
          /* If `gnu::nodiscard`, take the identifier after `::` */
          if (k + 3 < tree->base_tokens->size &&
              tree->base_tokens->tokens[k + 1].kind == CDD_TOKEN_COLON &&
              tree->base_tokens->tokens[k + 2].kind == CDD_TOKEN_COLON &&
              tree->base_tokens->tokens[k + 3].kind == CDD_TOKEN_IDENTIFIER) {
            k += 3;
            al = tree->base_tokens->tokens[k].length < 63
                     ? tree->base_tokens->tokens[k].length
                     : 63;
          }
          memcpy(attr_nm, tree->base_tokens->tokens[k].start, al);
          attr_nm[al] = '\0';
          if (al > 4 && attr_nm[0] == '_' && attr_nm[1] == '_' &&
              attr_nm[al - 2] == '_' && attr_nm[al - 1] == '_') {
            memmove(attr_nm, attr_nm + 2, al - 4);
            attr_nm[al - 4] = '\0';
          }
        }
      }
      if (end_rb > i + 1) {
        const char *repl = "";
        if (ctx->is_msvc) {
          if (strcmp(attr_nm, "nodiscard") == 0 ||
              strcmp(attr_nm, "warn_unused_result") == 0)
            repl = "_Check_return_";
          else if (strcmp(attr_nm, "noreturn") == 0)
            repl = "__declspec(noreturn)";
          else if (strcmp(attr_nm, "deprecated") == 0)
            repl = "__declspec(deprecated)";
        } else if (!ctx->is_legacy_c89 && strcmp(attr_nm, "noreturn") == 0) {
          repl = "_Noreturn";
        }
        rc = c_low_replace_span(tree, i, end_rb, repl);
        if (rc != CDD_C_SUCCESS)
          return rc;
        i = end_rb;
        continue;
      }
    }

    if (t->kind == CDD_TOKEN_KEYWORD_CONSTEXPR) {
      rc = replace_token_with_text(tree, t, CDD_TOKEN_IDENTIFIER, "const", 5);
      if (rc != CDD_C_SUCCESS)
        return rc;
      continue;
    }

    if (t->kind == CDD_TOKEN_IDENTIFIER) {
      if (t->length == 7 && memcmp(t->start, "nullptr", 7) == 0) {
        rc = replace_token_with_text(tree, t, CDD_TOKEN_IDENTIFIER,
                                     "((void *)0)", 11);
        if (rc != CDD_C_SUCCESS)
          return rc;
      } else if (t->length == 8 && memcmp(t->start, "restrict", 8) == 0) {
        if (ctx->is_msvc) {
          rc = replace_token_with_text(tree, t, CDD_TOKEN_IDENTIFIER,
                                       "__restrict", 10);
          if (rc != CDD_C_SUCCESS)
            return rc;
        } else if (ctx->is_iso_c89) {
          rc = replace_token_with_text(tree, t, CDD_TOKEN_IDENTIFIER, "", 0);
          if (rc != CDD_C_SUCCESS)
            return rc;
        }
      } else if ((t->length == 13 &&
                  memcmp(t->start, "_Thread_local", 13) == 0) ||
                 (t->length == 12 &&
                  memcmp(t->start, "thread_local", 12) == 0) ||
                 (t->length == 8 && memcmp(t->start, "__thread", 8) == 0)) {
        if (ctx->is_msvc) {
          rc = replace_token_with_text(tree, t, CDD_TOKEN_IDENTIFIER,
                                       "__declspec(thread)", 18);
          if (rc != CDD_C_SUCCESS)
            return rc;
        } else if (ctx->is_iso_c89) {
          rc = replace_token_with_text(tree, t, CDD_TOKEN_IDENTIFIER,
                                       "/* thread_local */", 18);
          if (rc != CDD_C_SUCCESS)
            return rc;
        }
      } else if ((t->length == 7 && memcmp(t->start, "ckd_add", 7) == 0) ||
                 (t->length == 7 && memcmp(t->start, "ckd_sub", 7) == 0) ||
                 (t->length == 7 && memcmp(t->start, "ckd_mul", 7) == 0)) {
        char ckd_nm[32];
        const char *pc = NULL;
        ctx->used_ckd = 1;
        CDD_SNPRINTF(ckd_nm, sizeof(ckd_nm), "__cdd_%.*s", (int)t->length,
                     (const char *)t->start);
        rc = cdd_pool_string_safe(tree, ckd_nm, &pc);
        if (rc != CDD_C_SUCCESS)
          return rc;
        rc = replace_token_with_text(tree, t, CDD_TOKEN_IDENTIFIER, pc,
                                     strlen(pc));
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Lowers C99 array parameter qualifiers (`f(T a[static 16])`,
 * `f(T a[restrict 16])`, `f(T a[const N])`) and VLA parameters (`f(int n, T
 * a[n])`, `f(T a[*])`) to pointer parameters (`T *a` / `T * __restrict a`).
 *
 * @param[in,out] ctx Lowering context.
 * @return CDD_C_SUCCESS or error code.
 */
static cdd_c_error_t c_lower_array_params(struct c_lowering_ctx_t *ctx) {
  cdd_cst_tree_t *tree = ctx->tree;
  size_t i;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  for (i = 0; i + 2 < tree->base_tokens->size; i++) {
    /* Identify function parameter lists: `<fn_ident> ( ... )` followed by `{`
       or `;` */
    if (tree->base_tokens->tokens[i].kind == CDD_TOKEN_IDENTIFIER &&
        tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_LPAREN) {
      const cdd_token_t *fn_t = &tree->base_tokens->tokens[i];
      size_t lp = i + 1;
      size_t rp = lp + 1;
      int pd = 1;
      if ((fn_t->length == 3 && memcmp(fn_t->start, "for", 3) == 0) ||
          (fn_t->length == 5 && memcmp(fn_t->start, "while", 5) == 0) ||
          (fn_t->length == 6 && memcmp(fn_t->start, "switch", 6) == 0) ||
          (fn_t->length == 6 && memcmp(fn_t->start, "sizeof", 6) == 0)) {
        continue;
      }
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
      if (rp >= tree->base_tokens->size || rp + 1 >= tree->base_tokens->size)
        continue;
      if (tree->base_tokens->tokens[rp + 1].kind != CDD_TOKEN_LBRACE &&
          tree->base_tokens->tokens[rp + 1].kind != CDD_TOKEN_SEMICOLON) {
        continue;
      }

      /* Walk top-level parameters inside `( ... )` */
      {
        size_t ps = lp + 1;
        while (ps < rp) {
          size_t pe = ps;
          int ppd = 0;
          while (pe < rp) {
            if (tree->base_tokens->tokens[pe].kind == CDD_TOKEN_LPAREN ||
                tree->base_tokens->tokens[pe].kind == CDD_TOKEN_LBRACKET)
              ppd++;
            else if (tree->base_tokens->tokens[pe].kind == CDD_TOKEN_RPAREN ||
                     tree->base_tokens->tokens[pe].kind == CDD_TOKEN_RBRACKET)
              ppd--;
            else if (ppd == 0 &&
                     tree->base_tokens->tokens[pe].kind == CDD_TOKEN_COMMA)
              break;
            pe++;
          }
          /* Check if parameter `[ps..pe)` has `ident [ ... ]` */
          {
            size_t lb = ps;
            while (lb < pe &&
                   tree->base_tokens->tokens[lb].kind != CDD_TOKEN_LBRACKET)
              lb++;
            if (lb > ps && lb + 1 < pe &&
                tree->base_tokens->tokens[lb - 1].kind ==
                    CDD_TOKEN_IDENTIFIER) {
              size_t rb0 = lb + 1;
              int bd = 1;
              int has_qual = 0;
              int has_restrict = 0;
              int has_const = 0;
              int has_star = 0;
              int is_vla = 0;
              while (rb0 < pe) {
                cdd_token_t *bt = &tree->base_tokens->tokens[rb0];
                if (bt->kind == CDD_TOKEN_LBRACKET)
                  bd++;
                else if (bt->kind == CDD_TOKEN_RBRACKET) {
                  bd--;
                  if (bd == 0)
                    break;
                } else if (bd == 1) {
                  if (bt->kind == CDD_TOKEN_STAR)
                    has_star = 1;
                  else if (bt->kind == CDD_TOKEN_IDENTIFIER) {
                    if (bt->length == 6 && memcmp(bt->start, "static", 6) == 0)
                      has_qual = 1;
                    else if ((bt->length == 8 &&
                              memcmp(bt->start, "restrict", 8) == 0) ||
                             (bt->length == 10 &&
                              memcmp(bt->start, "__restrict", 10) == 0)) {
                      has_qual = 1;
                      has_restrict = 1;
                    } else if (bt->length == 5 &&
                               memcmp(bt->start, "const", 5) == 0) {
                      has_qual = 1;
                      has_const = 1;
                    } else if (bt->length == 8 &&
                               memcmp(bt->start, "volatile", 8) == 0) {
                      has_qual = 1;
                    }
                  }
                }
                rb0++;
              }
              if (rb0 < pe && !has_qual && !has_star) {
                rc = (cdd_c_error_t)((int)rc |
                                     (int)c_low_is_vla_bound(
                                         ctx, tree->base_tokens->tokens, lb + 1,
                                         rb0, &is_vla));
              }
              if (rb0 < pe && (has_qual || has_star || is_vla)) {
                cdd_token_t *id_t = &tree->base_tokens->tokens[lb - 1];
                char pdecl[192];
                const char *qstr = "";
                if (has_restrict)
                  qstr = ctx->is_msvc ? " __restrict "
                                      : (ctx->is_iso_c89 ? "" : " restrict ");
                else if (has_const)
                  qstr = " const ";
                CDD_SNPRINTF(pdecl, sizeof(pdecl), "*%s%.*s", qstr,
                             (int)id_t->length, (const char *)id_t->start);
                /* Consume trailing `[...]` dimensions if multi-dim VLA param */
                while (rb0 + 1 < pe &&
                       tree->base_tokens->tokens[rb0 + 1].kind ==
                           CDD_TOKEN_LBRACKET) {
                  rb0 += 2;
                  while (rb0 < pe && tree->base_tokens->tokens[rb0].kind !=
                                         CDD_TOKEN_RBRACKET)
                    rb0++;
                }
                rc = c_low_replace_span(tree, lb - 1, rb0, pdecl);
                if (rc != CDD_C_SUCCESS)
                  return rc;
              }
            }
          }
          ps = (pe < rp) ? (pe + 1) : pe;
        }
      }
      i = rp;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Lowers local C99 Variable Length Arrays (arbitrary size expressions,
 * multi-dimensional VLAs, scoped `sizeof(vla)`, and `alloca`/`malloc`/`hybrid`
 * modes with scope/return/break/continue cleanup).
 *
 * @param[in,out] ctx Lowering context.
 * @return CDD_C_SUCCESS or error code.
 */
static cdd_c_error_t c_lower_vlas(struct c_lowering_ctx_t *ctx) {
  cdd_cst_tree_t *tree = ctx->tree;
  struct {
    char name[96];
    char elems_expr[256];
    char dim_exprs[4][128];
    int dims;
    int depth;
    int mode; /* 0 alloca, 1 malloc, 2 hybrid */
  } active_vlas[32];
  size_t blk_lb_stack[32];
  size_t num_vlas = 0;
  struct {
    int depth;
    int is_switch;
  } ctrl_stack[32];
  size_t num_ctrl = 0;
  int pending_ctrl = -1;
  int pending_pd = 0;
  int cur_depth = 0;
  int blk_is_cond[32];
  int just_returned = 0;
  char fn_ret[128];
  size_t i;
  cdd_c_error_t rc = CDD_C_SUCCESS;
  int eff_mode = 0;

  memset(blk_is_cond, 0, sizeof(blk_is_cond));
  fn_ret[0] = '\0';
  if (ctx->config) {
    if (ctx->config->vla_mode == CDD_VLA_MODE_HYBRID)
      eff_mode = 2;
    else if (ctx->config->vla_mode == CDD_VLA_MODE_MALLOC ||
             ctx->config->fallback_vla_to_malloc)
      eff_mode = 1;
  }

  for (i = 0; i < tree->base_tokens->size; i++) {
    cdd_token_t *t = &tree->base_tokens->tokens[i];
    if (t->length == 0)
      continue;
    if (t->kind != CDD_TOKEN_RBRACE)
      just_returned = 0;

    if (pending_ctrl >= 0) {
      if (t->kind == CDD_TOKEN_LPAREN)
        pending_pd++;
      else if (t->kind == CDD_TOKEN_RPAREN && pending_pd > 0)
        pending_pd--;
      else if (pending_pd == 0 && t->kind == CDD_TOKEN_SEMICOLON)
        pending_ctrl = -1;
    }

    if (t->kind == CDD_TOKEN_LBRACE) {
      if (cur_depth == 0 && i >= 2 &&
          tree->base_tokens->tokens[i - 1].kind == CDD_TOKEN_RPAREN) {
        int pd = 0;
        size_t back = i;
        fn_ret[0] = '\0';
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
                char fn_nm[128];
                cdd_cst_symbol_t *fsym = NULL;
                memcpy(fn_nm, tree->base_tokens->tokens[back - 1].start,
                       tree->base_tokens->tokens[back - 1].length);
                fn_nm[tree->base_tokens->tokens[back - 1].length] = '\0';
                if (cdd_cst_scope_lookup_in_scope(
                        ctx->sem_env->global_scope, fn_nm,
                        CDD_CST_SYMBOL_FUNCTION, &fsym) == CDD_C_SUCCESS) {
                  CDD_SNPRINTF(fn_ret, sizeof(fn_ret), "%s", fsym->type_str);
                }
              }
              break;
            }
          }
        }
      }
      blk_is_cond[(size_t)cur_depth & 31U] =
          (cur_depth > 0 &&
           (tree->base_tokens->tokens[i - 1].kind == CDD_TOKEN_RPAREN ||
            tree->base_tokens->tokens[i - 1].kind == CDD_TOKEN_KEYWORD_ELSE))
              ? 1
              : 0;
      blk_lb_stack[(size_t)cur_depth & 31U] = i;
      cur_depth++;
      if (pending_ctrl >= 0 && pending_pd == 0 && num_ctrl < 32) {
        ctrl_stack[num_ctrl].depth = cur_depth;
        ctrl_stack[num_ctrl].is_switch = pending_ctrl;
        num_ctrl++;
        pending_ctrl = -1;
      }
      continue;
    }

    if (t->kind == CDD_TOKEN_IDENTIFIER &&
        ((t->length == 3 && memcmp(t->start, "for", 3) == 0) ||
         (t->length == 5 && memcmp(t->start, "while", 5) == 0) ||
         (t->length == 2 && memcmp(t->start, "do", 2) == 0))) {
      pending_ctrl = 0;
      pending_pd = 0;
      continue;
    }
    if (t->kind == CDD_TOKEN_IDENTIFIER && t->length == 6 &&
        memcmp(t->start, "switch", 6) == 0) {
      pending_ctrl = 1;
      pending_pd = 0;
      continue;
    }

    /* Scoped `sizeof(vla)` or `sizeof vla` rewriting */
    if (num_vlas > 0 && t->kind == CDD_TOKEN_IDENTIFIER && t->length == 6 &&
        memcmp(t->start, "sizeof", 6) == 0 && i + 1 < tree->base_tokens->size) {
      size_t id_i = i + 1;
      size_t end_i = i + 1;
      if (tree->base_tokens->tokens[id_i].kind == CDD_TOKEN_LPAREN &&
          i + 3 < tree->base_tokens->size &&
          tree->base_tokens->tokens[i + 3].kind == CDD_TOKEN_RPAREN) {
        id_i = i + 2;
        end_i = i + 3;
      }
      if (tree->base_tokens->tokens[id_i].kind == CDD_TOKEN_IDENTIFIER) {
        cdd_token_t *vt = &tree->base_tokens->tokens[id_i];
        size_t vi;
        for (vi = num_vlas; vi-- > 0;) {
          if (strlen(active_vlas[vi].name) == vt->length &&
              memcmp(active_vlas[vi].name, vt->start, vt->length) == 0) {
            char sz_repl[384];
            CDD_SNPRINTF(sz_repl, sizeof(sz_repl), "((%s) * sizeof(*%s))",
                         active_vlas[vi].elems_expr, active_vlas[vi].name);
            rc = c_low_replace_span(tree, i, end_i, sz_repl);
            if (rc != CDD_C_SUCCESS)
              return rc;
            i = end_i;
            break;
          }
        }
        continue;
      }
    }

    /* Flatten multi-dimensional VLA subscripts `vla[i0][i1]` -> `vla[...]` */
    if (num_vlas > 0 && t->kind == CDD_TOKEN_IDENTIFIER &&
        i + 4 < tree->base_tokens->size &&
        tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_LBRACKET) {
      size_t vi;
      for (vi = num_vlas; vi-- > 0;) {
        if (active_vlas[vi].dims >= 2 &&
            strlen(active_vlas[vi].name) == t->length &&
            memcmp(active_vlas[vi].name, t->start, t->length) == 0) {
          size_t sk = i + 1;
          int d = 0;
          char flat_idx[384];
          size_t end_sub = i;
          flat_idx[0] = '\0';
          while (d < active_vlas[vi].dims && sk < tree->base_tokens->size &&
                 tree->base_tokens->tokens[sk].kind == CDD_TOKEN_LBRACKET) {
            size_t rbk = sk + 1;
            int bdk = 1;
            char idx_s[128];
            while (rbk < tree->base_tokens->size) {
              if (tree->base_tokens->tokens[rbk].kind == CDD_TOKEN_LBRACKET)
                bdk++;
              else if (tree->base_tokens->tokens[rbk].kind ==
                       CDD_TOKEN_RBRACKET) {
                bdk--;
                if (bdk == 0)
                  break;
              }
              rbk++;
            }
            rc = c_low_slice_text(tree->base_tokens->tokens, sk + 1, rbk, idx_s,
                                  sizeof(idx_s));
            if (d == 0) {
              size_t fp = 0;
              flat_idx[0] = '\0';
              rc = (cdd_c_error_t)((int)rc |
                                   (int)c_low_buf_cat(
                                       flat_idx, sizeof(flat_idx), &fp, "("));
              rc = (cdd_c_error_t)((int)rc |
                                   (int)c_low_buf_cat(
                                       flat_idx, sizeof(flat_idx), &fp, idx_s));
              rc = (cdd_c_error_t)((int)rc |
                                   (int)c_low_buf_cat(
                                       flat_idx, sizeof(flat_idx), &fp, ")"));
            } else {
              char prev_idx[384];
              size_t fp = 0;
              memcpy(prev_idx, flat_idx, sizeof(flat_idx));
              flat_idx[0] = '\0';
              rc = (cdd_c_error_t)((int)rc |
                                   (int)c_low_buf_cat(
                                       flat_idx, sizeof(flat_idx), &fp, "(("));
              rc = (cdd_c_error_t)((int)rc | (int)c_low_buf_cat(
                                                 flat_idx, sizeof(flat_idx),
                                                 &fp, prev_idx));
              rc = (cdd_c_error_t)((int)rc | (int)c_low_buf_cat(
                                                 flat_idx, sizeof(flat_idx),
                                                 &fp, ") * ("));
              rc = (cdd_c_error_t)((int)rc |
                                   (int)c_low_buf_cat(
                                       flat_idx, sizeof(flat_idx), &fp,
                                       active_vlas[vi].dim_exprs[d & 3]));
              rc = (cdd_c_error_t)((int)rc | (int)c_low_buf_cat(
                                                 flat_idx, sizeof(flat_idx),
                                                 &fp, ") + ("));
              rc = (cdd_c_error_t)((int)rc |
                                   (int)c_low_buf_cat(
                                       flat_idx, sizeof(flat_idx), &fp, idx_s));
              rc = (cdd_c_error_t)((int)rc |
                                   (int)c_low_buf_cat(
                                       flat_idx, sizeof(flat_idx), &fp, "))"));
            }
            end_sub = rbk;
            sk = rbk + 1;
            d++;
          }
          if (d == active_vlas[vi].dims) {
            char sub_repl[512];
            size_t sp = 0;
            sub_repl[0] = '\0';
            rc = (cdd_c_error_t)((int)rc |
                                 (int)c_low_buf_cat(sub_repl, sizeof(sub_repl),
                                                    &sp, active_vlas[vi].name));
            rc = (cdd_c_error_t)((int)rc |
                                 (int)c_low_buf_cat(sub_repl, sizeof(sub_repl),
                                                    &sp, "["));
            rc = (cdd_c_error_t)((int)rc |
                                 (int)c_low_buf_cat(sub_repl, sizeof(sub_repl),
                                                    &sp, flat_idx));
            rc = (cdd_c_error_t)((int)rc |
                                 (int)c_low_buf_cat(sub_repl, sizeof(sub_repl),
                                                    &sp, "]"));
            rc = (cdd_c_error_t)((int)rc | (int)c_low_replace_span(
                                               tree, i, end_sub, sub_repl));
            if (rc != CDD_C_SUCCESS)
              return rc;
            i = end_sub;
          }
          break;
        }
      }
    }

    /* Unwind heap/hybrid VLAs at `}` scope exit */
    if (t->kind == CDD_TOKEN_RBRACE) {
      char free_buf[1024];
      size_t fpos = 0;
      free_buf[0] = '\0';
      while (num_ctrl > 0 && ctrl_stack[num_ctrl - 1].depth == cur_depth)
        num_ctrl--;
      while (num_vlas > 0 && active_vlas[num_vlas - 1].depth == cur_depth) {
        num_vlas--;
        if (active_vlas[num_vlas].mode == 1) {
          fpos +=
              (size_t)CDD_SNPRINTF(free_buf + fpos, sizeof(free_buf) - fpos,
                                   "free(%s); ", active_vlas[num_vlas].name);
        } else if (active_vlas[num_vlas].mode == 2) {
          fpos += (size_t)CDD_SNPRINTF(free_buf + fpos, sizeof(free_buf) - fpos,
                                       "if (__cdd_vla_hp_%s) free(%s); ",
                                       active_vlas[num_vlas].name,
                                       active_vlas[num_vlas].name);
        }
      }
      if (!just_returned && fpos > 0) {
        rc = c_low_prepend_tok(tree, t, free_buf);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
      if (cur_depth > 0) {
        cur_depth--;
        if (blk_is_cond[(size_t)cur_depth & 31U])
          just_returned = 0;
      }
      continue;
    }

    /* Unwind heap/hybrid VLAs on `return` */
    if (t->kind == CDD_TOKEN_KEYWORD_RETURN && num_vlas > 0) {
      char free_buf[1024];
      size_t fpos = 0;
      size_t vi;
      free_buf[0] = '\0';
      for (vi = num_vlas; vi-- > 0;) {
        if (active_vlas[vi].mode == 1) {
          fpos += (size_t)CDD_SNPRINTF(free_buf + fpos, sizeof(free_buf) - fpos,
                                       "free(%s); ", active_vlas[vi].name);
        } else if (active_vlas[vi].mode == 2) {
          fpos +=
              (size_t)CDD_SNPRINTF(free_buf + fpos, sizeof(free_buf) - fpos,
                                   "if (__cdd_vla_hp_%s) free(%s); ",
                                   active_vlas[vi].name, active_vlas[vi].name);
        }
      }
      if (fpos > 0) {
        size_t k = i + 1;
        while (k < tree->base_tokens->size &&
               tree->base_tokens->tokens[k].kind != CDD_TOKEN_SEMICOLON)
          k++;
        if (k < tree->base_tokens->size) {
          int is_void_r = (k == i + 1 || strcmp(fn_ret, "void") == 0);
          if (is_void_r) {
            char rb[1152];
            CDD_SNPRINTF(rb, sizeof(rb), "{ %sreturn", free_buf);
            rc = replace_token_with_text(tree, t, CDD_TOKEN_IDENTIFIER, "", 0);
            if (rc != CDD_C_SUCCESS)
              return rc;
            rc = c_low_prepend_tok(tree, t, rb);
            if (rc != CDD_C_SUCCESS)
              return rc;
            rc = c_low_append_tok(tree, &tree->base_tokens->tokens[k], " }");
            if (rc != CDD_C_SUCCESS)
              return rc;
          } else {
            char rb[256];
            char sb[1152];
            const char *rt = fn_ret[0] != '\0' ? fn_ret : "int";
            CDD_SNPRINTF(rb, sizeof(rb), "{ %s __cdd_vret = (", rt);
            CDD_SNPRINTF(sb, sizeof(sb), "); %sreturn __cdd_vret; }", free_buf);
            rc = replace_token_with_text(tree, t, CDD_TOKEN_IDENTIFIER, "", 0);
            if (rc != CDD_C_SUCCESS)
              return rc;
            rc = c_low_prepend_tok(tree, t, rb);
            if (rc != CDD_C_SUCCESS)
              return rc;
            rc = replace_token_with_text(tree, &tree->base_tokens->tokens[k],
                                         CDD_TOKEN_SEMICOLON, "", 0);
            if (rc != CDD_C_SUCCESS)
              return rc;
            rc = c_low_append_tok(tree, &tree->base_tokens->tokens[k], sb);
            if (rc != CDD_C_SUCCESS)
              return rc;
          }
          just_returned = 1;
          i = k;
          continue;
        }
      }
    }

    /* Unwind heap/hybrid VLAs on `break` / `continue` across loop/switch */
    if (t->kind == CDD_TOKEN_IDENTIFIER &&
        ((t->length == 5 && memcmp(t->start, "break", 5) == 0) ||
         (t->length == 8 && memcmp(t->start, "continue", 8) == 0)) &&
        num_vlas > 0 && num_ctrl > 0 && i + 1 < tree->base_tokens->size &&
        tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_SEMICOLON) {
      int is_cont = (t->length == 8);
      int target_d = 0;
      size_t ci;
      for (ci = num_ctrl; ci-- > 0;) {
        if (!is_cont || !ctrl_stack[ci].is_switch) {
          target_d = ctrl_stack[ci].depth;
          break;
        }
      }
      if (target_d > 0) {
        char free_buf[1024];
        size_t fpos = 0;
        size_t vi;
        free_buf[0] = '\0';
        for (vi = num_vlas; vi-- > 0;) {
          if (active_vlas[vi].depth < target_d)
            break;
          if (active_vlas[vi].mode == 1) {
            fpos +=
                (size_t)CDD_SNPRINTF(free_buf + fpos, sizeof(free_buf) - fpos,
                                     "free(%s); ", active_vlas[vi].name);
          } else if (active_vlas[vi].mode == 2) {
            fpos += (size_t)CDD_SNPRINTF(
                free_buf + fpos, sizeof(free_buf) - fpos,
                "if (__cdd_vla_hp_%s) free(%s); ", active_vlas[vi].name,
                active_vlas[vi].name);
          }
        }
        if (fpos > 0) {
          char cb[1152];
          CDD_SNPRINTF(cb, sizeof(cb), "{ %s%s; }", free_buf,
                       is_cont ? "continue" : "break");
          rc = c_low_replace_span(tree, i, i + 1, cb);
          if (rc != CDD_C_SUCCESS)
            return rc;
          i++;
          continue;
        }
      }
    }

    /* Local VLA declaration: `<Type> <ident>[<dim0>][<dim1>]...;` */
    if (cur_depth > 0 && t->kind == CDD_TOKEN_LBRACKET && i >= 2 &&
        tree->base_tokens->tokens[i - 1].kind == CDD_TOKEN_IDENTIFIER) {
      /* Verify `<ident>` is preceded by a type specifier (declaration, not
         array subscript `arr[idx];`!) */
      cdd_token_t *id_tok = &tree->base_tokens->tokens[i - 1];
      cdd_token_t *prev_spec = &tree->base_tokens->tokens[i - 2];
      int is_decl_ctx = 0;
      if (prev_spec->kind == CDD_TOKEN_KEYWORD_INT ||
          prev_spec->kind == CDD_TOKEN_STAR ||
          prev_spec->kind == CDD_TOKEN_IDENTIFIER) {
        if (!(prev_spec->length == 6 &&
              memcmp(prev_spec->start, "sizeof", 6) == 0)) {
          /* Scan back to statement start (`;`, `{`, `}`) to verify no `=` or
             `(` before `id_tok` */
          size_t ds = i - 1;
          is_decl_ctx = 1;
          for (;;) {
            enum cdd_token_kind_t dk = tree->base_tokens->tokens[ds - 1].kind;
            if (dk == CDD_TOKEN_SEMICOLON || dk == CDD_TOKEN_LBRACE ||
                dk == CDD_TOKEN_RBRACE)
              break;
            if (dk == CDD_TOKEN_ASSIGN || dk == CDD_TOKEN_LPAREN ||
                dk == CDD_TOKEN_PLUS || dk == CDD_TOKEN_MINUS) {
              is_decl_ctx = 0;
              break;
            }
            ds--;
          }
          if (is_decl_ctx) {
            size_t k = i;
            int any_vla_dim = 0;
            int dims = 0;
            char cur_dims[4][128];
            char total_expr[256];
            size_t tpos = 0;
            memset(cur_dims, 0, sizeof(cur_dims));
            total_expr[0] = '\0';

            while (k < tree->base_tokens->size &&
                   tree->base_tokens->tokens[k].kind == CDD_TOKEN_LBRACKET) {
              size_t rb = k + 1;
              int bd = 1;
              int dim_vla = 0;
              char dim_s[128];
              while (rb < tree->base_tokens->size) {
                if (tree->base_tokens->tokens[rb].kind == CDD_TOKEN_LBRACKET)
                  bd++;
                else if (tree->base_tokens->tokens[rb].kind ==
                         CDD_TOKEN_RBRACKET) {
                  bd--;
                  if (bd == 0)
                    break;
                }
                rb++;
              }
              if (rb >= tree->base_tokens->size || rb == k + 1) {
                dims = 0;
                break;
              }
              rc = (cdd_c_error_t)((int)rc | (int)c_low_is_vla_bound(
                                                 ctx, tree->base_tokens->tokens,
                                                 k + 1, rb, &dim_vla));
              if (dim_vla)
                any_vla_dim = 1;
              rc = (cdd_c_error_t)((int)rc | (int)c_low_slice_text(
                                                 tree->base_tokens->tokens,
                                                 k + 1, rb, dim_s,
                                                 sizeof(dim_s)));
              memcpy(cur_dims[dims & 3], dim_s, sizeof(dim_s));
              if (dims > 0) {
                rc = (cdd_c_error_t)((int)rc |
                                     (int)c_low_buf_cat(total_expr,
                                                        sizeof(total_expr),
                                                        &tpos, " * "));
              }
              rc = (cdd_c_error_t)((int)rc | (int)c_low_buf_cat(
                                                 total_expr, sizeof(total_expr),
                                                 &tpos, "("));
              rc = (cdd_c_error_t)((int)rc | (int)c_low_buf_cat(
                                                 total_expr, sizeof(total_expr),
                                                 &tpos, dim_s));
              rc = (cdd_c_error_t)((int)rc | (int)c_low_buf_cat(
                                                 total_expr, sizeof(total_expr),
                                                 &tpos, ")"));
              dims++;
              k = rb + 1;
            }

            if (dims > 0 && any_vla_dim && k < tree->base_tokens->size &&
                tree->base_tokens->tokens[k].kind == CDD_TOKEN_SEMICOLON &&
                id_tok->length < 95) {
              char vname[96];
              char ty_s[128];
              char repl[2048];
              const char *alloca_fn = ctx->is_msvc ? "_alloca" : "alloca";
              memcpy(vname, id_tok->start, id_tok->length);
              vname[id_tok->length] = '\0';
              rc = c_low_slice_text(tree->base_tokens->tokens, ds, i - 1, ty_s,
                                    sizeof(ty_s));

              if (num_vlas < 32) {
                CDD_SNPRINTF(active_vlas[num_vlas].name,
                             sizeof(active_vlas[0].name), "%s", vname);
                CDD_SNPRINTF(active_vlas[num_vlas].elems_expr,
                             sizeof(active_vlas[0].elems_expr), "%s",
                             total_expr);
                memcpy(active_vlas[num_vlas].dim_exprs, cur_dims,
                       sizeof(cur_dims));
                active_vlas[num_vlas].dims = dims;
                active_vlas[num_vlas].depth = cur_depth;
                active_vlas[num_vlas].mode = eff_mode;
                num_vlas++;
              }

              if (ctx->is_legacy_c89) {
                char top_v[512];
                size_t lb_idx = blk_lb_stack[(size_t)(cur_depth - 1) & 31U];
                if (eff_mode == 2) {
                  CDD_SNPRINTF(top_v, sizeof(top_v),
                               " size_t __cdd_vla_sz_%s; int __cdd_vla_hp_%s; "
                               "%s *%s;",
                               vname, vname, ty_s, vname);
                  CDD_SNPRINTF(
                      repl, sizeof(repl),
                      "__cdd_vla_sz_%s = (%s) * sizeof(%s); __cdd_vla_hp_%s = "
                      "(__cdd_vla_sz_%s > 4096); %s = (%s *)(__cdd_vla_hp_%s ? "
                      "malloc(__cdd_vla_sz_%s) : %s(__cdd_vla_sz_%s));",
                      vname, total_expr, ty_s, vname, vname, vname, ty_s, vname,
                      vname, alloca_fn, vname);
                } else {
                  CDD_SNPRINTF(top_v, sizeof(top_v), " %s *%s;", ty_s, vname);
                  CDD_SNPRINTF(repl, sizeof(repl),
                               "%s = (%s *)%s((%s) * sizeof(*%s));", vname,
                               ty_s, eff_mode == 1 ? "malloc" : alloca_fn,
                               total_expr, vname);
                }
                rc = c_low_insert_after_lbrace(
                    tree, &tree->base_tokens->tokens[lb_idx], top_v);
                if (rc != CDD_C_SUCCESS)
                  return rc;
                rc = c_low_replace_span(tree, ds, k, repl);
              } else if (eff_mode == 2) {
                CDD_SNPRINTF(
                    repl, sizeof(repl),
                    "size_t __cdd_vla_sz_%s = (%s) * sizeof(%s); int "
                    "__cdd_vla_hp_%s = (__cdd_vla_sz_%s > 4096); %s *%s = "
                    "(%s *)(__cdd_vla_hp_%s ? malloc(__cdd_vla_sz_%s) : "
                    "%s(__cdd_vla_sz_%s));",
                    vname, total_expr, ty_s, vname, vname, ty_s, vname, ty_s,
                    vname, vname, alloca_fn, vname);
                rc = c_low_replace_span(tree, ds, k, repl);
              } else if (eff_mode == 1) {
                CDD_SNPRINTF(repl, sizeof(repl),
                             "%s *%s = (%s *)malloc((%s) * sizeof(*%s));", ty_s,
                             vname, ty_s, total_expr, vname);
                rc = c_low_replace_span(tree, ds, k, repl);
              } else {
                CDD_SNPRINTF(repl, sizeof(repl),
                             "%s *%s = (%s *)%s((%s) * sizeof(*%s));", ty_s,
                             vname, ty_s, alloca_fn, total_expr, vname);
                rc = c_low_replace_span(tree, ds, k, repl);
              }
              if (rc != CDD_C_SUCCESS)
                return rc;
              i = k;
              continue;
            }
          }
        }
      }
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks if `name` is recorded as a lowered `_Complex` variable.
 *
 * @param[in] ctx Lowering context.
 * @param[in] s Identifier bytes.
 * @param[in] len Byte length.
 * @param[out] out_is_flt Receives 1 if `_Fcomplex`, 0 if `_Dcomplex`.
 * @param[out] out_found Receives 1 if matched.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t c_low_find_complex_var(const struct c_lowering_ctx_t *ctx,
                                            const uint8_t *s, size_t len,
                                            int *out_is_flt, int *out_found) {
  size_t k;
  *out_is_flt = 0;
  *out_found = 0;
  for (k = 0; k < ctx->num_complex_vars; k++) {
    if (strlen(ctx->complex_vars[k]) == len &&
        memcmp(ctx->complex_vars[k], s, len) == 0) {
      *out_is_flt = ctx->complex_is_float[k];
      *out_found = 1;
      return CDD_C_SUCCESS;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Lowers C99 `_Complex` / `<complex.h>` types, imaginary literals, and
 * `+ - * / == !=` arithmetic expressions into `_Fcomplex`/`_Dcomplex` structs
 * and `awm_c*` helper calls.
 *
 * @param[in,out] ctx Lowering context.
 * @return CDD_C_SUCCESS or error code.
 */
static cdd_c_error_t c_lower_complex(struct c_lowering_ctx_t *ctx) {
  cdd_cst_tree_t *tree = ctx->tree;
  size_t i;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  /* 1. Lower `<complex.h>`, `[float|double|long double] _Complex|complex`,
     imaginary literals (`2.0i`, `3.0fi`, `_Complex_I`), and `creal`/`cimag` */
  for (i = 0; i < tree->base_tokens->size; i++) {
    cdd_token_t *t = &tree->base_tokens->tokens[i];
    if (t->length == 0)
      continue;

    if (t->kind == CDD_TOKEN_PREPROC_INCLUDE &&
        (!ctx->config || !ctx->config->with_auto_win_msvc)) {
      int has_cplx = 0;
      rc = c_low_tok_contains(t, "<complex.h>", &has_cplx);
      if (has_cplx) {
        ctx->used_complex |= 3;
        rc = replace_token_with_text(tree, t, t->kind,
                                     "/* <complex.h> lowered */\n", 26);
        if (rc != CDD_C_SUCCESS)
          return rc;
        continue;
      }
    }

    if (t->kind == CDD_TOKEN_NUMBER && t->length >= 2 &&
        (t->start[t->length - 1] == 'i' || t->start[t->length - 1] == 'I' ||
         t->start[t->length - 1] == 'j' || t->start[t->length - 1] == 'J') &&
        !(t->start[0] == '0' && (t->start[1] == 'x' || t->start[1] == 'X'))) {
      int is_f = (t->length >= 3 && (t->start[t->length - 2] == 'f' ||
                                     t->start[t->length - 2] == 'F'));
      size_t num_len = t->length - (is_f ? 2 : 1);
      char cbuf[128];
      const char *pc = NULL;
      ctx->used_complex |= is_f ? 1 : 2;
      if (is_f) {
        CDD_SNPRINTF(cbuf, sizeof(cbuf), "awm_cmakef(0.0f, %.*sf)",
                     (int)num_len, (const char *)t->start);
      } else {
        CDD_SNPRINTF(cbuf, sizeof(cbuf), "awm_cmake(0.0, (double)(%.*s))",
                     (int)num_len, (const char *)t->start);
      }
      rc = cdd_pool_string_safe(tree, cbuf, &pc);
      if (rc != CDD_C_SUCCESS)
        return rc;
      rc = replace_token_with_text(tree, t, CDD_TOKEN_IDENTIFIER, pc,
                                   strlen(pc));
      if (rc != CDD_C_SUCCESS)
        return rc;
      continue;
    }

    if (t->kind == CDD_TOKEN_IDENTIFIER) {
      if (t->length == 10 && memcmp(t->start, "_Complex_I", 10) == 0) {
        ctx->used_complex |= 2;
        rc = replace_token_with_text(tree, t, CDD_TOKEN_IDENTIFIER,
                                     "awm_cmake(0.0, 1.0)", 19);
        if (rc != CDD_C_SUCCESS)
          return rc;
        continue;
      }
      if ((t->length == 5 && memcmp(t->start, "creal", 5) == 0) ||
          (t->length == 5 && memcmp(t->start, "cimag", 5) == 0) ||
          (t->length == 6 && memcmp(t->start, "crealf", 6) == 0) ||
          (t->length == 6 && memcmp(t->start, "cimagf", 6) == 0) ||
          (t->length == 4 && memcmp(t->start, "conj", 4) == 0) ||
          (t->length == 5 && memcmp(t->start, "conjf", 5) == 0)) {
        if (ctx->used_complex != 0 && i + 1 < tree->base_tokens->size &&
            tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_LPAREN) {
          char fnm[32];
          const char *pf = NULL;
          CDD_SNPRINTF(fnm, sizeof(fnm), "awm_%.*s", (int)t->length,
                       (const char *)t->start);
          rc = cdd_pool_string_safe(tree, fnm, &pf);
          if (rc != CDD_C_SUCCESS)
            return rc;
          rc = replace_token_with_text(tree, t, CDD_TOKEN_IDENTIFIER, pf,
                                       strlen(pf));
          if (rc != CDD_C_SUCCESS)
            return rc;
          continue;
        }
      }

      if ((t->length == 8 && memcmp(t->start, "_Complex", 8) == 0) ||
          (ctx->used_complex != 0 && t->length == 7 &&
           memcmp(t->start, "complex", 7) == 0)) {
        int is_f = 0;
        size_t s_idx = i;
        if (i > 0 &&
            tree->base_tokens->tokens[i - 1].kind == CDD_TOKEN_IDENTIFIER) {
          cdd_token_t *pt = &tree->base_tokens->tokens[i - 1];
          if (pt->length == 5 && memcmp(pt->start, "float", 5) == 0) {
            is_f = 1;
            s_idx = i - 1;
          } else if (pt->length == 6 && memcmp(pt->start, "double", 6) == 0) {
            s_idx = i - 1;
            if (i >= 2 && tree->base_tokens->tokens[i - 2].length == 4 &&
                memcmp(tree->base_tokens->tokens[i - 2].start, "long", 4) ==
                    0) {
              s_idx = i - 2;
            }
          }
        }
        ctx->used_complex |= is_f ? 1 : 2;
        rc = c_low_replace_span(tree, s_idx, i,
                                is_f ? "_Fcomplex" : "_Dcomplex");
        if (rc != CDD_C_SUCCESS)
          return rc;
        /* Record comma-separated complex declarator identifiers */
        {
          size_t k = i + 1;
          int pd = 0;
          while (k < tree->base_tokens->size) {
            cdd_token_t *dk = &tree->base_tokens->tokens[k];
            if (dk->kind == CDD_TOKEN_LPAREN || dk->kind == CDD_TOKEN_LBRACE)
              pd++;
            else if (dk->kind == CDD_TOKEN_RPAREN ||
                     dk->kind == CDD_TOKEN_RBRACE) {
              if (pd == 0)
                break;
              pd--;
            } else if (pd == 0 && dk->kind == CDD_TOKEN_SEMICOLON) {
              break;
            } else if (pd == 0 && dk->kind == CDD_TOKEN_IDENTIFIER &&
                       dk->length < 95 &&
                       !(k + 1 < tree->base_tokens->size &&
                         tree->base_tokens->tokens[k + 1].kind ==
                             CDD_TOKEN_LPAREN)) {
              /* Only record LHS declarator before `=` */
              size_t back_eq = k;
              int after_eq = 0;
              while (back_eq > i) {
                if (tree->base_tokens->tokens[back_eq].kind == CDD_TOKEN_COMMA)
                  break;
                if (tree->base_tokens->tokens[back_eq].kind ==
                    CDD_TOKEN_ASSIGN) {
                  after_eq = 1;
                  break;
                }
                back_eq--;
              }
              if (!after_eq && ctx->num_complex_vars < 64) {
                memcpy(ctx->complex_vars[ctx->num_complex_vars], dk->start,
                       dk->length);
                ctx->complex_vars[ctx->num_complex_vars][dk->length] = '\0';
                ctx->complex_is_float[ctx->num_complex_vars] = is_f;
                ctx->num_complex_vars++;
              }
            }
            k++;
          }
        }
      }
    }
  }

  /* 2. Rewrite `+`, `-`, `*`, `/`, `==`, `!=` on lowered complex expressions */
  if (ctx->used_complex != 0 && ctx->num_complex_vars > 0) {
    int changed = 1;
    while (changed) {
      changed = 0;
      for (i = 1; i + 1 < tree->base_tokens->size; i++) {
        cdd_token_t *tk = &tree->base_tokens->tokens[i];
        const char *op_fn = NULL;
        int neg_eq = 0;
        if (tk->length == 0)
          continue;
        if (tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_ASSIGN &&
            tree->base_tokens->tokens[i + 1].offset == tk->offset + 1) {
          continue;
        }
        if (tk->kind == CDD_TOKEN_PLUS)
          op_fn = "cadd";
        else if (tk->kind == CDD_TOKEN_MINUS)
          op_fn = "csub";
        else if (tk->kind == CDD_TOKEN_STAR)
          op_fn = "cmul";
        else if (tk->kind == CDD_TOKEN_SLASH)
          op_fn = "cdiv";
        else if (tk->kind == CDD_TOKEN_EQ)
          op_fn = "ceq";
        else if (tk->kind == CDD_TOKEN_NEQ) {
          op_fn = "ceq";
          neg_eq = 1;
        }
        if (!op_fn)
          continue;

        /* Ensure binary operator context */
        {
          size_t prev_nz = i;
          while (prev_nz > 0 &&
                 tree->base_tokens->tokens[prev_nz - 1].length == 0)
            prev_nz--;
          if (prev_nz == 0)
            continue;
          {
            enum cdd_token_kind_t pk =
                tree->base_tokens->tokens[prev_nz - 1].kind;
            if (pk != CDD_TOKEN_IDENTIFIER && pk != CDD_TOKEN_NUMBER &&
                pk != CDD_TOKEN_RPAREN)
              continue;
          }
        }

        /* Extract primary/call LHS `[ls..i)` and RHS `[i+1..re)` */
        {
          size_t ls = i;
          size_t re = i + 1;
          int pd = 0;
          int l_cplx = 0, r_cplx = 0;
          int l_flt = 0, r_flt = 0;
          size_t k;

          while (ls > 0) {
            cdd_token_t *lt = &tree->base_tokens->tokens[ls - 1];
            if (lt->length == 0) {
              ls--;
              continue;
            }
            if (lt->kind == CDD_TOKEN_RPAREN)
              pd++;
            else if (lt->kind == CDD_TOKEN_LPAREN) {
              if (pd == 0)
                break;
              pd--;
            } else if (pd == 0 &&
                       (lt->kind == CDD_TOKEN_SEMICOLON ||
                        lt->kind == CDD_TOKEN_COMMA ||
                        lt->kind == CDD_TOKEN_ASSIGN ||
                        lt->kind == CDD_TOKEN_LBRACE ||
                        lt->kind == CDD_TOKEN_RBRACE ||
                        lt->kind == CDD_TOKEN_KEYWORD_RETURN ||
                        lt->kind == CDD_TOKEN_COLON ||
                        (lt->kind == CDD_TOKEN_OTHER &&
                         (lt->start[0] == '?' || lt->start[0] == '&' ||
                          lt->start[0] == '|')) ||
                        lt->kind == CDD_TOKEN_EQ || lt->kind == CDD_TOKEN_NEQ ||
                        ((strcmp(op_fn, "cmul") == 0 ||
                          strcmp(op_fn, "cdiv") == 0) &&
                         (lt->kind == CDD_TOKEN_PLUS ||
                          lt->kind == CDD_TOKEN_MINUS)))) {
              break;
            }
            ls--;
          }
          while (tree->base_tokens->tokens[ls].length == 0)
            ls++;
          if (tree->base_tokens->tokens[ls].length == 2 &&
              memcmp(tree->base_tokens->tokens[ls].start, "if", 2) == 0) {
            ls++;
          }

          pd = 0;
          while (re < tree->base_tokens->size) {
            cdd_token_t *rt = &tree->base_tokens->tokens[re];
            if (rt->length == 0) {
              re++;
              continue;
            }
            if (rt->kind == CDD_TOKEN_LPAREN)
              pd++;
            else if (rt->kind == CDD_TOKEN_RPAREN) {
              if (pd == 0)
                break;
              pd--;
            } else if (pd == 0 &&
                       (rt->kind == CDD_TOKEN_SEMICOLON ||
                        rt->kind == CDD_TOKEN_COMMA ||
                        rt->kind == CDD_TOKEN_RBRACE ||
                        rt->kind == CDD_TOKEN_COLON ||
                        (rt->kind == CDD_TOKEN_OTHER &&
                         (rt->start[0] == '?' || rt->start[0] == '&' ||
                          rt->start[0] == '|')) ||
                        rt->kind == CDD_TOKEN_EQ || rt->kind == CDD_TOKEN_NEQ ||
                        (re > i + 1 && (rt->kind == CDD_TOKEN_PLUS ||
                                        rt->kind == CDD_TOKEN_MINUS ||
                                        rt->kind == CDD_TOKEN_STAR ||
                                        rt->kind == CDD_TOKEN_SLASH)))) {
              break;
            }
            re++;
          }

          for (k = ls; k < i; k++) {
            cdd_token_t *ct = &tree->base_tokens->tokens[k];
            if (ct->kind == CDD_TOKEN_IDENTIFIER) {
              int fnd = 0;
              rc = c_low_find_complex_var(ctx, ct->start, ct->length, &l_flt,
                                          &fnd);
              if (fnd)
                l_cplx = 1;
              else if (ct->length >= 9 &&
                       memcmp(ct->start, "awm_cmake", 9) == 0) {
                l_cplx = 1;
                l_flt = (ct->length >= 10 && ct->start[9] == 'f');
              } else if (ct->length >= 8 &&
                         (memcmp(ct->start, "awm_cadd", 8) == 0 ||
                          memcmp(ct->start, "awm_csub", 8) == 0 ||
                          memcmp(ct->start, "awm_cmul", 8) == 0 ||
                          memcmp(ct->start, "awm_cdiv", 8) == 0)) {
                l_cplx = 1;
                l_flt = (ct->length >= 9 && ct->start[8] == 'f');
              }
            }
          }
          for (k = i + 1; k < re; k++) {
            cdd_token_t *ct = &tree->base_tokens->tokens[k];
            if (ct->kind == CDD_TOKEN_IDENTIFIER) {
              int fnd = 0;
              rc = c_low_find_complex_var(ctx, ct->start, ct->length, &r_flt,
                                          &fnd);
              if (fnd)
                r_cplx = 1;
              else if (ct->length >= 9 &&
                       memcmp(ct->start, "awm_cmake", 9) == 0) {
                r_cplx = 1;
                r_flt = (ct->length >= 10 && ct->start[9] == 'f');
              } else if (ct->length >= 8 &&
                         (memcmp(ct->start, "awm_cadd", 8) == 0 ||
                          memcmp(ct->start, "awm_csub", 8) == 0 ||
                          memcmp(ct->start, "awm_cmul", 8) == 0 ||
                          memcmp(ct->start, "awm_cdiv", 8) == 0)) {
                r_cplx = 1;
                r_flt = (ct->length >= 9 && ct->start[8] == 'f');
              }
            }
          }

          if (re > i + 1 && (l_cplx || r_cplx)) {
            int use_f = (l_cplx && r_cplx) ? (l_flt && r_flt)
                                           : (l_cplx ? l_flt : r_flt);
            char lhs_s[192], rhs_s[192], lhs_w[256], rhs_w[256], repl[640];
            rc = (cdd_c_error_t)((int)rc | (int)c_low_slice_text(
                                               tree->base_tokens->tokens, ls, i,
                                               lhs_s, sizeof(lhs_s)));
            rc = (cdd_c_error_t)((int)rc | (int)c_low_slice_text(
                                               tree->base_tokens->tokens, i + 1,
                                               re, rhs_s, sizeof(rhs_s)));
            if (l_cplx) {
              CDD_SNPRINTF(lhs_w, sizeof(lhs_w), "%s", lhs_s);
            } else {
              CDD_SNPRINTF(lhs_w, sizeof(lhs_w),
                           use_f ? "awm_cmakef((float)(%s), 0.0f)"
                                 : "awm_cmake((double)(%s), 0.0)",
                           lhs_s);
            }
            if (r_cplx) {
              CDD_SNPRINTF(rhs_w, sizeof(rhs_w), "%s", rhs_s);
            } else {
              CDD_SNPRINTF(rhs_w, sizeof(rhs_w),
                           use_f ? "awm_cmakef((float)(%s), 0.0f)"
                                 : "awm_cmake((double)(%s), 0.0)",
                           rhs_s);
            }
            CDD_SNPRINTF(repl, sizeof(repl), "%sawm_%s%s(%s, %s)%s",
                         neg_eq ? "(!" : "", op_fn, use_f ? "f" : "", lhs_w,
                         rhs_w, neg_eq ? ")" : "");
            rc = c_low_replace_span(tree, ls, re - 1, repl);
            if (rc != CDD_C_SUCCESS)
              return rc;
            changed = 1;
            break;
          }
        }
      }
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks if `s[0..len)` is recorded in `ctx->atomic_vars`.
 *
 * @param[in] ctx Lowering context.
 * @param[in] s Identifier bytes.
 * @param[in] len Byte length.
 * @param[out] out_is_atomic Set to 1 if matched.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t c_low_is_atomic_var(const struct c_lowering_ctx_t *ctx,
                                         const uint8_t *s, size_t len,
                                         int *out_is_atomic) {
  size_t k;
  *out_is_atomic = 0;
  for (k = 0; k < ctx->num_atomic_vars; k++) {
    if (strlen(ctx->atomic_vars[k]) == len &&
        memcmp(ctx->atomic_vars[k], s, len) == 0) {
      *out_is_atomic = 1;
      return CDD_C_SUCCESS;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Lowers C11 `_Atomic(T)` / `_Atomic T` declarations and direct load,
 * store, `++`, `--`, `+=`, `-=`, `|=`, `&=`, `^=` operations into explicit
 * `atomic_*` helper calls.
 *
 * @param[in,out] ctx Lowering context.
 * @return CDD_C_SUCCESS or error code.
 */
static cdd_c_error_t c_lower_atomics(struct c_lowering_ctx_t *ctx) {
  cdd_cst_tree_t *tree = ctx->tree;
  size_t i;
  int brace_d = 0;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  /* 1. Lower `_Atomic(T)` and `_Atomic` qualifier + record atomic variables */
  for (i = 0; i < tree->base_tokens->size; i++) {
    cdd_token_t *t = &tree->base_tokens->tokens[i];
    if (t->kind == CDD_TOKEN_IDENTIFIER && t->length == 7 &&
        memcmp(t->start, "_Atomic", 7) == 0) {
      size_t after_spec = i + 1;
      ctx->used_atomic = 1;
      if (i + 3 < tree->base_tokens->size &&
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
          char inner_t[128];
          char vspec[160];
          rc = c_low_slice_text(tree->base_tokens->tokens, i + 2, rp, inner_t,
                                sizeof(inner_t));
          CDD_SNPRINTF(vspec, sizeof(vspec), "volatile %s", inner_t);
          rc = c_low_replace_span(tree, i, rp, vspec);
          if (rc != CDD_C_SUCCESS)
            return rc;
          after_spec = rp + 1;
        }
      } else {
        rc = replace_token_with_text(tree, t, CDD_TOKEN_IDENTIFIER, "volatile",
                                     8);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }

      /* Record variable declarator name(s) up to `;`, `,`, `)`, `=` */
      {
        size_t k = after_spec;
        int saw_eq = 0;
        cdd_token_t *last_id = NULL;
        while (k < tree->base_tokens->size) {
          cdd_token_t *dk = &tree->base_tokens->tokens[k];
          if (dk->kind == CDD_TOKEN_SEMICOLON || dk->kind == CDD_TOKEN_RPAREN ||
              dk->kind == CDD_TOKEN_LBRACE) {
            if (last_id && last_id->length < 95 && ctx->num_atomic_vars < 64) {
              memcpy(ctx->atomic_vars[ctx->num_atomic_vars], last_id->start,
                     last_id->length);
              ctx->atomic_vars[ctx->num_atomic_vars][last_id->length] = '\0';
              ctx->num_atomic_vars++;
            }
            break;
          }
          if (dk->kind == CDD_TOKEN_COMMA) {
            if (last_id && last_id->length < 95 && ctx->num_atomic_vars < 64) {
              memcpy(ctx->atomic_vars[ctx->num_atomic_vars], last_id->start,
                     last_id->length);
              ctx->atomic_vars[ctx->num_atomic_vars][last_id->length] = '\0';
              ctx->num_atomic_vars++;
            }
            last_id = NULL;
            saw_eq = 0;
          } else if (dk->kind == CDD_TOKEN_ASSIGN) {
            saw_eq = 1;
          } else if (!saw_eq && dk->kind == CDD_TOKEN_IDENTIFIER) {
            last_id = dk;
          }
          k++;
        }
      }
    }
  }

  if (ctx->num_atomic_vars == 0)
    return CDD_C_SUCCESS;

  /* 2. Rewrite atomic loads, stores, increments, and compound assignments */
  for (i = 0; i < tree->base_tokens->size; i++) {
    cdd_token_t *t = &tree->base_tokens->tokens[i];
    int is_at = 0;
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
    if (brace_d == 0)
      continue;

    /* Pre-inc/dec: `++avar` or `--avar` */
    if (i + 2 < tree->base_tokens->size &&
        ((t->kind == CDD_TOKEN_PLUS &&
          tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_PLUS) ||
         (t->kind == CDD_TOKEN_MINUS &&
          tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_MINUS)) &&
        tree->base_tokens->tokens[i + 1].offset == t->offset + 1 &&
        tree->base_tokens->tokens[i + 2].kind == CDD_TOKEN_IDENTIFIER) {
      cdd_token_t *vt = &tree->base_tokens->tokens[i + 2];
      rc = c_low_is_atomic_var(ctx, vt->start, vt->length, &is_at);
      if (is_at) {
        char repl[256];
        int is_add = (t->kind == CDD_TOKEN_PLUS);
        CDD_SNPRINTF(repl, sizeof(repl), "(atomic_fetch_%s(&(%.*s), 1) %c 1)",
                     is_add ? "add" : "sub", (int)vt->length,
                     (const char *)vt->start, is_add ? '+' : '-');
        rc = c_low_replace_span(tree, i, i + 2, repl);
        if (rc != CDD_C_SUCCESS)
          return rc;
        i += 2;
        continue;
      }
    }

    if (t->kind != CDD_TOKEN_IDENTIFIER)
      continue;
    rc = c_low_is_atomic_var(ctx, t->start, t->length, &is_at);
    if (!is_at)
      continue;

    /* Skip declaration site (preceded by type specifier or `*`) or `&avar` /
       `.avar` / `->avar` */
    {
      cdd_token_t *pt = &tree->base_tokens->tokens[i - 1];
      if (pt->kind == CDD_TOKEN_DOT || pt->kind == CDD_TOKEN_ARROW ||
          (pt->length == 1 && pt->start[0] == '&') ||
          pt->kind == CDD_TOKEN_KEYWORD_INT ||
          pt->kind == CDD_TOKEN_IDENTIFIER) {
        continue;
      }
      if (pt->kind == CDD_TOKEN_STAR &&
          tree->base_tokens->tokens[i - 2].kind == CDD_TOKEN_IDENTIFIER) {
        continue;
      }
    }

    /* Post-inc/dec: `avar++` / `avar--` */
    if (i + 2 < tree->base_tokens->size &&
        ((tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_PLUS &&
          tree->base_tokens->tokens[i + 2].kind == CDD_TOKEN_PLUS) ||
         (tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_MINUS &&
          tree->base_tokens->tokens[i + 2].kind == CDD_TOKEN_MINUS)) &&
        tree->base_tokens->tokens[i + 2].offset ==
            tree->base_tokens->tokens[i + 1].offset + 1) {
      char repl[256];
      int is_add = (tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_PLUS);
      CDD_SNPRINTF(repl, sizeof(repl), "atomic_fetch_%s(&(%.*s), 1)",
                   is_add ? "add" : "sub", (int)t->length,
                   (const char *)t->start);
      rc = c_low_replace_span(tree, i, i + 2, repl);
      if (rc != CDD_C_SUCCESS)
        return rc;
      i += 2;
      continue;
    }

    /* Compound assignment: `avar += expr`, `-=`, `|=`, `&=`, `^=` */
    if (i + 2 < tree->base_tokens->size &&
        tree->base_tokens->tokens[i + 2].kind == CDD_TOKEN_ASSIGN &&
        tree->base_tokens->tokens[i + 2].offset ==
            tree->base_tokens->tokens[i + 1].offset + 1) {
      cdd_token_t *ot = &tree->base_tokens->tokens[i + 1];
      const char *fop = NULL;
      if (ot->kind == CDD_TOKEN_PLUS)
        fop = "add";
      else if (ot->kind == CDD_TOKEN_MINUS)
        fop = "sub";
      else if (ot->start[0] == '|')
        fop = "or";
      else if (ot->start[0] == '&')
        fop = "and";
      else if (ot->start[0] == '^')
        fop = "xor";
      if (fop) {
        size_t re = i + 3;
        int pd = 0;
        char rhs_s[256];
        char repl[384];
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
        rc = c_low_slice_text(tree->base_tokens->tokens, i + 3, re, rhs_s,
                              sizeof(rhs_s));
        CDD_SNPRINTF(repl, sizeof(repl), "atomic_fetch_%s(&(%.*s), (%s))", fop,
                     (int)t->length, (const char *)t->start, rhs_s);
        rc = c_low_replace_span(tree, i, re - 1, repl);
        if (rc != CDD_C_SUCCESS)
          return rc;
        i = re - 1;
        continue;
      }
    }

    /* Direct store: `avar = expr` (not `==`) */
    if (i + 1 < tree->base_tokens->size &&
        tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_ASSIGN) {
      size_t re = i + 2;
      int pd = 0;
      char rhs_s[256];
      char repl[384];
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
      rc = c_low_slice_text(tree->base_tokens->tokens, i + 2, re, rhs_s,
                            sizeof(rhs_s));
      CDD_SNPRINTF(repl, sizeof(repl), "atomic_store(&(%.*s), (%s))",
                   (int)t->length, (const char *)t->start, rhs_s);
      rc = c_low_replace_span(tree, i, re - 1, repl);
      if (rc != CDD_C_SUCCESS)
        return rc;
      i = re - 1;
      continue;
    }

    /* Direct rvalue load: `avar` */
    {
      char lrepl[192];
      const char *pl = NULL;
      CDD_SNPRINTF(lrepl, sizeof(lrepl), "atomic_load(&(%.*s))", (int)t->length,
                   (const char *)t->start);
      rc = cdd_pool_string_safe(tree, lrepl, &pl);
      if (rc != CDD_C_SUCCESS)
        return rc;
      rc = replace_token_with_text(tree, t, CDD_TOKEN_IDENTIFIER, pl,
                                   strlen(pl));
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Injects portable C89/MSVC companion helper definitions at the top of
 * the TU when `_Complex`, `<stdckdint.h>`, or `_Atomic` lowering was active.
 *
 * @param[in,out] ctx Lowering context.
 * @return CDD_C_SUCCESS or error code.
 */
static cdd_c_error_t
c_lower_inject_runtime_shims(struct c_lowering_ctx_t *ctx) {
  char *hdr = NULL;
  size_t cap = 8192;
  size_t len = 0;
  cdd_c_error_t rc;

  if (!ctx->used_complex && !ctx->used_ckd && !ctx->used_atomic)
    return CDD_C_SUCCESS;

  rc = gnu_malloc(cap, (void **)&hdr);
  if (rc != CDD_C_SUCCESS)
    return rc;
  hdr[0] = '\0';

  if (ctx->used_ckd) {
    rc = (cdd_c_error_t)((int)rc |
                         (int)c_low_buf_cat(
                             hdr, cap, &len,
                             "#ifndef CDD_STDCKDINT_SHIM_H\n"
                             "#define CDD_STDCKDINT_SHIM_H\n"
                             "#define __cdd_ckd_add(r, a, b) ((*(r) = (a) + "
                             "(b)), ((a) > 0 && (b) > 0 && *(r) < (a)))\n"
                             "#define __cdd_ckd_sub(r, a, b) ((*(r) = (a) - "
                             "(b)), ((b) > 0 && *(r) > (a)))\n"
                             "#define __cdd_ckd_mul(r, a, b) ((*(r) = (a) * "
                             "(b)), ((a) != 0 && (*(r) / (a)) != (b)))\n"
                             "#endif\n"));
  }

  if (ctx->used_atomic) {
    rc = (cdd_c_error_t)((int)rc |
                         (int)c_low_buf_cat(
                             hdr, cap, &len,
                             "#ifndef atomic_load\n"
                             "#if defined(_MSC_VER) || defined(__WATCOMC__)\n"
                             "#define AWM_ATOM_INLINE __inline\n"
                             "#elif defined(__GNUC__) || defined(__clang__)\n"
                             "#define AWM_ATOM_INLINE __inline__ "
                             "__attribute__((__unused__))\n"
                             "#else\n#define AWM_ATOM_INLINE\n#endif\n"
                             "static AWM_ATOM_INLINE long __awm_atom_val("
                             "long v) { return v; }\n"));
    rc = (cdd_c_error_t)((int)rc |
                         (int)c_low_buf_cat(
                             hdr, cap, &len,
                             "#define atomic_load(p) (*(p))\n"
                             "#define atomic_store(p, v) (*(p) = (v))\n"
                             "#define atomic_fetch_add(p, v) "
                             "__awm_atom_val((long)((*(p) += (v)) - (v)))\n"
                             "#define atomic_fetch_sub(p, v) "
                             "__awm_atom_val((long)((*(p) -= (v)) + (v)))\n"
                             "#define atomic_fetch_or(p, v) (*(p) |= (v))\n"
                             "#define atomic_fetch_and(p, v) (*(p) &= (v))\n"
                             "#define atomic_fetch_xor(p, v) (*(p) ^= (v))\n"
                             "#endif\n"));
  }

  if (ctx->used_complex) {
    rc = (cdd_c_error_t)((int)rc |
                         (int)c_low_buf_cat(
                             hdr, cap, &len,
                             "#ifndef AWM_C99_COMPLEX_SHIM_H\n"
                             "#define AWM_C99_COMPLEX_SHIM_H\n"
                             "#if defined(_MSC_VER) || defined(__WATCOMC__)\n"
                             "#define AWM_CPLX_INLINE __inline\n"
                             "#elif defined(__GNUC__) || defined(__clang__)\n"
                             "#define AWM_CPLX_INLINE __inline__ "
                             "__attribute__((__unused__))\n"
                             "#else\n#define AWM_CPLX_INLINE\n#endif\n"
                             "#ifndef _C_COMPLEX_T\n#define _C_COMPLEX_T\n"
                             "typedef struct { double _Val[2]; } _Dcomplex;\n"
                             "typedef struct { float _Val[2]; } _Fcomplex;\n"
                             "typedef struct { long double _Val[2]; } "
                             "_Lcomplex;\n#endif\n"));
    rc = (cdd_c_error_t)((int)rc |
                         (int)c_low_buf_cat(
                             hdr, cap, &len,
                             "static AWM_CPLX_INLINE _Dcomplex awm_cmake("
                             "double r, double i) { _Dcomplex c; c._Val[0] = "
                             "r; c._Val[1] = i; return c; }\n"
                             "static AWM_CPLX_INLINE _Dcomplex awm_cadd("
                             "_Dcomplex a, _Dcomplex b) { return awm_cmake("
                             "a._Val[0] + b._Val[0], a._Val[1] + b._Val[1]); }"
                             "\n"
                             "static AWM_CPLX_INLINE _Dcomplex awm_csub("
                             "_Dcomplex a, _Dcomplex b) { return awm_cmake("
                             "a._Val[0] - b._Val[0], a._Val[1] - b._Val[1]); }"
                             "\n"));
    rc = (cdd_c_error_t)((int)rc |
                         (int)c_low_buf_cat(
                             hdr, cap, &len,
                             "static AWM_CPLX_INLINE _Dcomplex awm_cmul("
                             "_Dcomplex a, _Dcomplex b) { return awm_cmake("
                             "a._Val[0] * b._Val[0] - a._Val[1] * b._Val[1], "
                             "a._Val[0] * b._Val[1] + a._Val[1] * b._Val[0]); "
                             "}\n"
                             "static AWM_CPLX_INLINE _Dcomplex awm_cdiv("
                             "_Dcomplex a, _Dcomplex b) { double d = b._Val[0] "
                             "* b._Val[0] + b._Val[1] * b._Val[1]; return "
                             "awm_cmake((a._Val[0] * b._Val[0] + a._Val[1] * "
                             "b._Val[1]) / d, (a._Val[1] * b._Val[0] - "
                             "a._Val[0] * b._Val[1]) / d); }\n"));
    rc = (cdd_c_error_t)((int)rc |
                         (int)c_low_buf_cat(
                             hdr, cap, &len,
                             "static AWM_CPLX_INLINE double awm_creal("
                             "_Dcomplex a) { return a._Val[0]; }\n"
                             "static AWM_CPLX_INLINE double awm_cimag("
                             "_Dcomplex a) { return a._Val[1]; }\n"
                             "static AWM_CPLX_INLINE _Dcomplex awm_conj("
                             "_Dcomplex a) { return awm_cmake(a._Val[0], "
                             "-a._Val[1]); }\n"
                             "static AWM_CPLX_INLINE int awm_ceq(_Dcomplex a, "
                             "_Dcomplex b) { return a._Val[0] == b._Val[0] && "
                             "a._Val[1] == b._Val[1]; }\n"));
    rc = (cdd_c_error_t)((int)rc |
                         (int)c_low_buf_cat(
                             hdr, cap, &len,
                             "static AWM_CPLX_INLINE _Fcomplex awm_cmakef("
                             "float r, float i) { _Fcomplex c; c._Val[0] = r; "
                             "c._Val[1] = i; return c; }\n"
                             "static AWM_CPLX_INLINE _Fcomplex awm_caddf("
                             "_Fcomplex a, _Fcomplex b) { return awm_cmakef("
                             "a._Val[0] + b._Val[0], a._Val[1] + b._Val[1]); }"
                             "\n"
                             "static AWM_CPLX_INLINE _Fcomplex awm_csubf("
                             "_Fcomplex a, _Fcomplex b) { return awm_cmakef("
                             "a._Val[0] - b._Val[0], a._Val[1] - b._Val[1]); }"
                             "\n"));
    rc = (cdd_c_error_t)((int)rc |
                         (int)c_low_buf_cat(
                             hdr, cap, &len,
                             "static AWM_CPLX_INLINE _Fcomplex awm_cmulf("
                             "_Fcomplex a, _Fcomplex b) { return awm_cmakef("
                             "a._Val[0] * b._Val[0] - a._Val[1] * b._Val[1], "
                             "a._Val[0] * b._Val[1] + a._Val[1] * b._Val[0]); "
                             "}\n"
                             "static AWM_CPLX_INLINE _Fcomplex awm_cdivf("
                             "_Fcomplex a, _Fcomplex b) { float d = b._Val[0] "
                             "* b._Val[0] + b._Val[1] * b._Val[1]; return "
                             "awm_cmakef((a._Val[0] * b._Val[0] + a._Val[1] * "
                             "b._Val[1]) / d, (a._Val[1] * b._Val[0] - "
                             "a._Val[0] * b._Val[1]) / d); }\n"));
    rc = (cdd_c_error_t)((int)rc |
                         (int)c_low_buf_cat(
                             hdr, cap, &len,
                             "static AWM_CPLX_INLINE float awm_crealf("
                             "_Fcomplex a) { return a._Val[0]; }\n"
                             "static AWM_CPLX_INLINE float awm_cimagf("
                             "_Fcomplex a) { return a._Val[1]; }\n"
                             "static AWM_CPLX_INLINE _Fcomplex awm_conjf("
                             "_Fcomplex a) { return awm_cmakef(a._Val[0], "
                             "-a._Val[1]); }\n"
                             "static AWM_CPLX_INLINE int awm_ceqf(_Fcomplex a, "
                             "_Fcomplex b) { return a._Val[0] == b._Val[0] && "
                             "a._Val[1] == b._Val[1]; }\n#endif\n"));
  }

  rc = (cdd_c_error_t)((int)rc | (int)c_low_prepend_root(ctx->tree, hdr));
  C_CDD_FREE(hdr);
  return rc;
}

/*
 * @brief Executes Phase 3.1 C99/C11/C23 lowering passes required on both Legacy
 * and Modern MSVC.
 */
cdd_c_error_t c_lower_modern_passes(struct c_lowering_ctx_t *ctx) {
  cdd_c_error_t rc;
  rc = c_lower_keywords_and_c23(ctx);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = c_lower_array_params(ctx);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = c_lower_vlas(ctx);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = c_lower_complex(ctx);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = c_lower_atomics(ctx);
  if (rc != CDD_C_SUCCESS)
    return rc;
  return c_lower_inject_runtime_shims(ctx);
}
