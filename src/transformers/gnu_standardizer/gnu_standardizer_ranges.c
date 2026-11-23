/**
 * @file gnu_standardizer_ranges.c
 * @brief Case ranges, initializers, and builtin expect standardization pass.
 */

/* clang-format off */
#include "gnu_standardizer_internal.h"
/* clang-format on */

/**
 * @brief Decodes a C character literal token (including standard and GNU `\e`
 * escape sequences) into its integer byte value.
 *
 * @param[in] raw Pointer to character literal bytes (`'...'` or `L'...'`).
 * @param[in] len Length of token in bytes.
 * @param[out] out_val Decoded integer value.
 * @param[out] out_ok Receives 1 on valid single-char literal, 0 otherwise.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t gnu_decode_char_tok(const char *raw, size_t len,
                                         int *out_val, int *out_ok) {
  size_t p = 1;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  *out_val = 0;
  *out_ok = 0;
  if (len < 3 || raw[0] != '\'' || raw[len - 1] != '\'')
    return CDD_C_SUCCESS;
  if (raw[p] != '\\') {
    *out_val = (int)(unsigned char)raw[p];
    *out_ok = 1;
    return CDD_C_SUCCESS;
  }
  p++;
  if (p >= len - 1)
    return CDD_C_SUCCESS;
  switch (raw[p]) {
  case 'n':
    *out_val = '\n';
    break;
  case 'r':
    *out_val = '\r';
    break;
  case 't':
    *out_val = '\t';
    break;
  case 'a':
    *out_val = '\a';
    break;
  case 'b':
    *out_val = '\b';
    break;
  case 'f':
    *out_val = '\f';
    break;
  case 'v':
    *out_val = '\v';
    break;
  case 'e':
  case 'E':
    *out_val = 27;
    break;
  case '\\':
  case '\'':
  case '"':
  case '?':
    *out_val = (int)(unsigned char)raw[p];
    break;
  case 'x':
  case 'X': {
    unsigned int hv = 0;
    int digits = 0;
    p++;
    while (p < len - 1 && digits < 4) {
      char ch = raw[p];
      if (ch >= '0' && ch <= '9')
        hv = (hv << 4) | (unsigned int)(ch - '0');
      else if (ch >= 'a' && ch <= 'f')
        hv = (hv << 4) | (unsigned int)(ch - 'a' + 10);
      else if (ch >= 'A' && ch <= 'F')
        hv = (hv << 4) | (unsigned int)(ch - 'A' + 10);
      else
        break;
      p++;
      digits++;
    }
    if (digits == 0)
      return CDD_C_SUCCESS;
    *out_val = (int)(hv & 0xFFU);
    break;
  }
  default:
    if (raw[p] >= '0' && raw[p] <= '7') {
      unsigned int ov = 0;
      int digits = 0;
      while (p < len - 1 && digits < 3 && raw[p] >= '0' && raw[p] <= '7') {
        ov = (ov << 3) | (unsigned int)(raw[p] - '0');
        p++;
        digits++;
      }
      *out_val = (int)(ov & 0xFFU);
    } else {
      *out_val = (int)(unsigned char)raw[p];
    }
    break;
  }
  *out_ok = 1;
  return CDD_C_SUCCESS;
}

/**
 * @brief Evaluates a range bound token (decimal/hex/octal/binary integer,
 * character literal, or scoped enum constant).
 *
 * @param[in] tree CST tree.
 * @param[in] gctx Optional GNU standardizer context holding `sem_env`.
 * @param[in] tok Bound token.
 * @param[in] is_neg Non-zero if preceded by unary `-`.
 * @param[out] out_val Evaluated integer bound.
 * @param[out] out_is_char Receives 1 if bound was a character literal.
 * @param[out] out_ok Receives 1 if successfully evaluated.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t gnu_eval_range_bound(
    cdd_cst_tree_t *tree, const struct gnu_standardizer_ctx_t *gctx,
    cdd_token_t *tok, int is_neg, int *out_val, int *out_is_char, int *out_ok) {
  *out_val = 0;
  *out_is_char = 0;
  *out_ok = 0;
  if (tok->length == 0)
    return CDD_C_SUCCESS;

  if (tok->kind == CDD_TOKEN_CHAR) {
    int cv = 0;
    int ok = 0;
    cdd_c_error_t rc_ch =
        gnu_decode_char_tok((const char *)tok->start, tok->length, &cv, &ok);
    if (rc_ch != CDD_C_SUCCESS)
      return rc_ch;
    if (ok) {
      *out_val = is_neg ? -cv : cv;
      *out_is_char = !is_neg;
      *out_ok = 1;
    }
    return CDD_C_SUCCESS;
  }
  if (tok->kind == CDD_TOKEN_NUMBER) {
    char buf[128];
    size_t clen = tok->length < sizeof(buf) - 1 ? tok->length : sizeof(buf) - 1;
    size_t k = 0;
    size_t w = 0;
    long v = 0;
    while (k < clen) {
      if (tok->start[k] != '\'')
        buf[w++] = (char)tok->start[k];
      k++;
    }
    buf[w] = '\0';
    if (w >= 3 && buf[0] == '0' && (buf[1] == 'b' || buf[1] == 'B')) {
      uint64_t hi = 0;
      uint64_t lo = 0;
      cdd_c_error_t rc_b = cdd_parse_bin_128_literal(buf, w, &hi, &lo);
      if (rc_b != CDD_C_SUCCESS)
        return rc_b;
      if (hi == 0) {
        v = (long)lo;
        *out_val = is_neg ? (int)(-v) : (int)v;
        *out_ok = 1;
      }
      return CDD_C_SUCCESS;
    }
    if (w >= 3 && buf[0] == '0' && (buf[1] == 'x' || buf[1] == 'X')) {
      v = strtol(buf + 2, NULL, 16);
    } else if (w >= 2 && buf[0] == '0' && buf[1] >= '0' && buf[1] <= '7') {
      v = strtol(buf + 1, NULL, 8);
    } else {
      v = strtol(buf, NULL, 10);
    }
    *out_val = is_neg ? (int)(-v) : (int)v;
    *out_ok = 1;
    return CDD_C_SUCCESS;
  }

  if (tok->kind == CDD_TOKEN_IDENTIFIER && tok->length < 128 && gctx &&
      gctx->sem_env) {
    char name[128];
    cdd_cst_scope_t *sc = gctx->sem_env->global_scope;
    cdd_cst_symbol_t *sym = NULL;
    size_t cidx = 0;
    cdd_cst_node_t *owner = NULL;
    cdd_c_error_t rc_fn;
    cdd_c_error_t rc_lu;
    memcpy(name, tok->start, tok->length);
    name[tok->length] = '\0';
    rc_fn = gnu_find_node(tree->root, tok, &cidx, &owner);
    if (rc_fn == CDD_C_SUCCESS) {
      cdd_c_error_t rc_sc =
          cdd_cst_scope_find_for_node(gctx->sem_env, owner, &sc);
      if (rc_sc != CDD_C_SUCCESS) {
        return cdd_c_keep_err(rc_sc, rc_fn);
      }
    }
    rc_lu = cdd_cst_scope_lookup_in_scope(sc, name,
                                          CDD_CST_SYMBOL_ENUM_CONSTANT, &sym);
    if (rc_lu == CDD_C_SUCCESS && sym->has_enum_value) {
      int ev = (int)sym->enum_value;
      *out_val = is_neg ? -ev : ev;
      *out_ok = 1;
      return rc_lu;
    }
    return CDD_C_SUCCESS;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Appends a formatted case label value (either printable char `'c'` or
 * signed integer) to `p`.
 *
 * @param[in,out] p Destination cursor.
 * @param[in] v Value to format.
 * @param[in] use_char Non-zero to emit printable ASCII character literal.
 * @param[out] out_p Updated destination cursor.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t gnu_append_case_val(char *p, int v, int use_char,
                                         char **out_p) {
  if (use_char && v >= 32 && v <= 126 && v != '\'' && v != '\\') {
    *p++ = '\'';
    *p++ = (char)v;
    *p++ = '\'';
    *p = '\0';
    *out_p = p;
    return CDD_C_SUCCESS;
  }
  return cdd_append_int(p, v, out_p);
}

/*
 * @brief Standardizes GNU case ranges (`case A ... B:`), array range
 * designators (`[A ... B] = V`), empty braces (`{}`), and `__builtin_expect`.
 *
 * @param[in,out] tree CST tree to transform.
 * @param[in,out] gctx Optional GNU standardizer context.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
cdd_c_error_t gnu_standardize_ranges_ctx(cdd_cst_tree_t *tree,
                                         struct gnu_standardizer_ctx_t *gctx) {
  size_t i;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  for (i = 0; i < tree->base_tokens->size; i++) {
    cdd_token_t *t = &tree->base_tokens->tokens[i];
    if (t->kind == CDD_TOKEN_COMMA) {
      /* Strip trailing commas before RBRACE */
      size_t next_idx = i + 1;
      while (next_idx < tree->base_tokens->size &&
             tree->base_tokens->tokens[next_idx].length == 0) {
        next_idx++;
      }
      if (next_idx < tree->base_tokens->size &&
          tree->base_tokens->tokens[next_idx].kind == CDD_TOKEN_RBRACE) {
        rc = replace_token_with_text(tree, t, t->kind, "", 0);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
    } else if (t->kind == CDD_TOKEN_COLON) {
      size_t next_idx = i + 1;
      while (next_idx < tree->base_tokens->size &&
             tree->base_tokens->tokens[next_idx].length == 0) {
        next_idx++;
      }
      if (next_idx < tree->base_tokens->size &&
          tree->base_tokens->tokens[next_idx].kind == CDD_TOKEN_RBRACE) {
        rc = replace_token_with_text(tree, t, t->kind, ": ;", 3);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
    } else if (t->kind == CDD_TOKEN_LBRACE && i + 1 < tree->base_tokens->size &&
               tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_RBRACE) {
      /* Determine if it's an empty struct or empty initializer */
      int is_initializer = 1;
      size_t k;
      int depth = 0;
      for (k = i; k-- > 0;) {
        cdd_token_t *tk = &tree->base_tokens->tokens[k];
        if (tk->kind == CDD_TOKEN_RBRACE)
          depth++;
        else if (tk->kind == CDD_TOKEN_LBRACE)
          depth--;

        if (depth == 0) {
          if (tk->kind == CDD_TOKEN_KEYWORD_STRUCT ||
              (tk->kind == CDD_TOKEN_IDENTIFIER && tk->length == 5 &&
               memcmp(tk->start, "union", 5) == 0) ||
              (tk->kind == CDD_TOKEN_IDENTIFIER && tk->length == 4 &&
               memcmp(tk->start, "enum", 4) == 0)) {
            is_initializer = 0;
            break;
          } else if (tk->kind == CDD_TOKEN_SEMICOLON) {
            break;
          } else if (tk->kind == CDD_TOKEN_ASSIGN) {
            is_initializer = 1;
            break;
          }
        }
      }

      if (i > 0 && tree->base_tokens->tokens[i - 1].kind == CDD_TOKEN_COLON) {
        /* Empty block following a case or default label */
        tree->base_tokens->tokens[i].start = (const uint8_t *)";";
        tree->base_tokens->tokens[i].length = 1;
        tree->base_tokens->tokens[i + 1].start = (const uint8_t *)"";
        tree->base_tokens->tokens[i + 1].length = 0;
      } else if (is_initializer) {
        tree->base_tokens->tokens[i + 1].start = (const uint8_t *)" 0 }";
        tree->base_tokens->tokens[i + 1].length = 4;
      } else {
        tree->base_tokens->tokens[i + 1].start =
            (const uint8_t *)" char _pad; }";
        tree->base_tokens->tokens[i + 1].length = 13;
      }
    } else if (t->kind == CDD_TOKEN_DOT && i > 1 &&
               i + 3 < tree->base_tokens->size) {
      /* Look for ... */
      if (tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_DOT &&
          tree->base_tokens->tokens[i + 2].kind == CDD_TOKEN_DOT) {

        cdd_token_t *prev = &tree->base_tokens->tokens[i - 1];
        size_t next_idx = i + 3;
        cdd_token_t *next = &tree->base_tokens->tokens[next_idx];
        int is_case = 0;
        int is_range_init = 0;
        size_t is_case_idx = 0;
        size_t is_range_idx = 0;
        int is_neg_start = 0;
        int is_neg_end = 0;
        int start_val = 0;
        int end_val = 0;
        int start_char = 0;
        int end_char = 0;
        int ok_s = 0;
        int ok_e = 0;

        if (tree->base_tokens->tokens[i - 2].kind == CDD_TOKEN_MINUS) {
          is_neg_start = 1;
        }
        if (next->kind == CDD_TOKEN_MINUS && i + 4 < tree->base_tokens->size) {
          is_neg_end = 1;
          next_idx = i + 4;
          next = &tree->base_tokens->tokens[next_idx];
        }

        rc = gnu_eval_range_bound(tree, gctx, prev, is_neg_start, &start_val,
                                  &start_char, &ok_s);
        if (rc != CDD_C_SUCCESS)
          return rc;
        rc = gnu_eval_range_bound(tree, gctx, next, is_neg_end, &end_val,
                                  &end_char, &ok_e);
        if (rc != CDD_C_SUCCESS)
          return rc;

        if (ok_s && ok_e &&
            (end_val < start_val || end_val - start_val <= 4096)) {
          /* Determine context: case or range init */
          {
            size_t k;
            for (k = i - 1; k-- > 0;) {
              if (tree->base_tokens->tokens[k].length == 4 &&
                  memcmp(tree->base_tokens->tokens[k].start, "case", 4) == 0) {
                is_case = 1;
                is_case_idx = k;
                break;
              } else if (tree->base_tokens->tokens[k].kind ==
                         CDD_TOKEN_LBRACKET) {
                is_range_init = 1;
                is_range_idx = k;
                break;
              } else if (tree->base_tokens->tokens[k].kind ==
                             CDD_TOKEN_SEMICOLON ||
                         tree->base_tokens->tokens[k].kind ==
                             CDD_TOKEN_LBRACE ||
                         tree->base_tokens->tokens[k].kind ==
                             CDD_TOKEN_RBRACE) {
                break;
              }
            }
          }

          if (is_case) {
            char *heap_buf;
            size_t alloc_sz;
            char *p;
            int v;
            int use_ch = (start_char && end_char);
            const char *pooled;
            if (start_val > end_val) {
              alloc_sz = 128;
            } else {
              alloc_sz = ((size_t)(end_val - start_val + 2)) * 32 + 128;
            }
            rc = gnu_malloc(alloc_sz, (void **)&heap_buf);
            if (rc != CDD_C_SUCCESS)
              return rc;
            p = heap_buf;
            if (start_val > end_val) {
#if defined(_MSC_VER)
              strcpy_s(p, alloc_sz - (size_t)(p - heap_buf),
                       "/* WARNING: Invalid case range */ ");
#else
              strcpy(p, "/* WARNING: Invalid case range */ ");
#endif
              p += 34;
            }
            for (v = start_val; v <= end_val; v++) {
#if defined(_MSC_VER)
              strcpy_s(p, alloc_sz - (size_t)(p - heap_buf), "case ");
#else
              strcpy(p, "case ");
#endif
              p += 5;
              rc = gnu_append_case_val(p, v, use_ch, &p);
              if (rc != CDD_C_SUCCESS) {
                C_CDD_FREE(heap_buf);
                return rc;
              }
              if (v != end_val) {
#if defined(_MSC_VER)
                strcpy_s(p, alloc_sz - (size_t)(p - heap_buf), ": ");
#else
                strcpy(p, ": ");
#endif
                p += 2;
              }
            }
            *p = '\0';
            if (is_neg_start) {
              tree->base_tokens->tokens[i - 2].length = 0;
            }
            rc = cdd_pool_string_safe(tree, heap_buf, &pooled);
            if (rc != CDD_C_SUCCESS) {
              C_CDD_FREE(heap_buf);
              return rc;
            }
            tree->base_tokens->tokens[is_case_idx].start =
                (const uint8_t *)pooled;
            tree->base_tokens->tokens[is_case_idx].length = strlen(heap_buf);
            C_CDD_FREE(heap_buf);
            rc = replace_token_with_text(tree, prev, prev->kind, "", 0);
            if (rc != CDD_C_SUCCESS)
              return rc;
            rc = replace_token_with_text(tree, t, t->kind, "", 0);
            if (rc != CDD_C_SUCCESS)
              return rc;
            tree->base_tokens->tokens[i + 1].length = 0;
            tree->base_tokens->tokens[i + 2].length = 0;
            if (is_neg_end) {
              tree->base_tokens->tokens[i + 3].length = 0;
            }
            rc = replace_token_with_text(tree, next, next->kind, "", 0);
            if (rc != CDD_C_SUCCESS)
              return rc;
          } else if (is_range_init && start_val >= 0 && start_val <= end_val) {
            if (next_idx + 3 < tree->base_tokens->size &&
                tree->base_tokens->tokens[next_idx + 1].kind ==
                    CDD_TOKEN_RBRACKET &&
                tree->base_tokens->tokens[next_idx + 2].kind ==
                    CDD_TOKEN_ASSIGN) {
              size_t val_start = next_idx + 3;
              size_t val_end = val_start;
              int vd = 0;
              while (val_end < tree->base_tokens->size) {
                enum cdd_token_kind_t vk =
                    tree->base_tokens->tokens[val_end].kind;
                if (vk == CDD_TOKEN_LPAREN || vk == CDD_TOKEN_LBRACKET ||
                    vk == CDD_TOKEN_LBRACE)
                  vd++;
                else if (vk == CDD_TOKEN_RPAREN || vk == CDD_TOKEN_RBRACKET ||
                         vk == CDD_TOKEN_RBRACE) {
                  if (vd == 0)
                    break;
                  vd--;
                } else if (vd == 0 && vk == CDD_TOKEN_COMMA) {
                  break;
                }
                val_end++;
              }
              if (val_end > val_start) {
                const uint8_t *vs = tree->base_tokens->tokens[val_start].start;
                const uint8_t *ve =
                    tree->base_tokens->tokens[val_end - 1].start +
                    tree->base_tokens->tokens[val_end - 1].length;
                size_t vlen = (size_t)(ve - vs);
                char *heap_buf;
                size_t alloc_sz =
                    ((size_t)(end_val - start_val + 2)) * (32 + vlen) + 1;
                char *p;
                int v;
                const char *pooled;
                size_t w_idx;

                rc = gnu_malloc(alloc_sz, (void **)&heap_buf);
                if (rc != CDD_C_SUCCESS)
                  return rc;
                p = heap_buf;
                for (v = start_val; v <= end_val; v++) {
                  *p++ = '[';
                  rc = cdd_append_int(p, v, &p);
                  if (rc != CDD_C_SUCCESS) {
                    C_CDD_FREE(heap_buf);
                    return rc;
                  }
#if defined(_MSC_VER)
                  strcpy_s(p, alloc_sz - (size_t)(p - heap_buf), "] = ");
#else
                  strcpy(p, "] = ");
#endif
                  p += 4;
                  memcpy(p, vs, vlen);
                  p += vlen;
                  if (v != end_val) {
#if defined(_MSC_VER)
                    strcpy_s(p, alloc_sz - (size_t)(p - heap_buf), ", ");
#else
                    strcpy(p, ", ");
#endif
                    p += 2;
                  }
                }
                *p = '\0';

                rc = cdd_pool_string_safe(tree, heap_buf, &pooled);
                if (rc != CDD_C_SUCCESS) {
                  C_CDD_FREE(heap_buf);
                  return rc;
                }
                tree->base_tokens->tokens[is_range_idx].start =
                    (const uint8_t *)pooled;
                tree->base_tokens->tokens[is_range_idx].length =
                    strlen(heap_buf);
                C_CDD_FREE(heap_buf);

                for (w_idx = is_range_idx + 1; w_idx < val_end; w_idx++) {
                  rc = replace_token_with_text(
                      tree, &tree->base_tokens->tokens[w_idx],
                      tree->base_tokens->tokens[w_idx].kind, "", 0);
                  if (rc != CDD_C_SUCCESS)
                    return rc;
                }
              }
            }
          }
        }
      }
    }

    /* builtin expect */
    if (t->kind == CDD_TOKEN_IDENTIFIER && t->length == 16 &&
        memcmp(t->start, "__builtin_expect", 16) == 0) {
      size_t j = i + 1;
      if (j < tree->base_tokens->size) {
        if (tree->base_tokens->tokens[j].kind == CDD_TOKEN_LPAREN) {
          int depth = 1;
          size_t comma_idx = 0;
          size_t k;
          for (k = j + 1; k < tree->base_tokens->size; k++) {
            if (tree->base_tokens->tokens[k].kind == CDD_TOKEN_LPAREN) {
              depth++;
            } else if (tree->base_tokens->tokens[k].kind == CDD_TOKEN_RPAREN) {
              depth--;
              if (depth == 0) {
                if (comma_idx > 0) {
                  t->length = 0;
                  {
                    size_t wipe;
                    for (wipe = comma_idx; wipe < k; wipe++) {
                      tree->base_tokens->tokens[wipe].length = 0;
                    }
                  }
                }
                break;
              }
            } else if (tree->base_tokens->tokens[k].kind == CDD_TOKEN_COMMA &&
                       depth == 1) {
              comma_idx = k;
            }
          }
        }
      }
    }
  }
  return CDD_C_SUCCESS;
}

/*
 * @brief Expands GNU case range statements (case A ... B:) without external
 * context.
 *
 * @param[in,out] tree CST tree to transform.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
cdd_c_error_t gnu_standardize_ranges(cdd_cst_tree_t *tree) {
  return gnu_standardize_ranges_ctx(tree, NULL);
}
