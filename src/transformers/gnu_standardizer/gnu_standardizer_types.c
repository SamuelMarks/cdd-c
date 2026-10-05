/**
 * @file gnu_standardizer_types.c
 * @brief Type, attribute, and macro standardization pass for GNU standardizer.
 */

/* clang-format off */
#include "gnu_standardizer_internal.h"
/* clang-format on */

cdd_c_error_t gnu_standardize_types(cdd_cst_tree_t *tree) {
  size_t i;
  cdd_c_error_t rc = CDD_C_SUCCESS;

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
                while (isalnum((unsigned char)start_id[-1]) ||
                       start_id[-1] == '_') {
                  start_id--;
                }
                if (start_id < ellipsis) {
                  var_len = (size_t)(ellipsis - start_id);
                  rc = gnu_malloc(var_len + 1, (void **)&var_name);
                  if (rc != CDD_C_SUCCESS) {
                    free(buf);
                    return rc;
                  }
                  memcpy(var_name, start_id, var_len);
                  var_name[var_len] = '\0';
                } else {
                  rc = gnu_malloc(12, (void **)&var_name);
                  if (rc != CDD_C_SUCCESS) {
                    free(buf);
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
                    free(var_name);
                    free(buf);
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
                                   " __VA_OPT__(,) __VA_ARGS__");
#else
                          strcpy(out_p, " __VA_OPT__(,) __VA_ARGS__");
#endif
                          out_p += strlen(" __VA_OPT__(,) __VA_ARGS__");
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
                    const char *pooled = pool_string_safe(tree, out_buf);
                    if (!pooled) {
                      free(out_buf);
                      free(var_name);
                      free(buf);
                      return CDD_C_ERROR_MEMORY;
                    }
                    rc = replace_token_with_text(
                        tree, tok, CDD_TOKEN_PREPROC_DEFINE, pooled,
                        (size_t)(out_p - out_buf));
                    free(out_buf);
                    if (rc != CDD_C_SUCCESS) {
                      free(var_name);
                      free(buf);
                      return rc;
                    }
                  }
                }
                free(var_name);
              }
            }
          }
        }
      }
      free(buf);
    } else if (tok->kind == CDD_TOKEN_KEYWORD___INT128) {
      int is_unsigned = 0;
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
          cdd_cst_bld_punct(&bld, "{");
          cdd_cst_bld_space(&bld);
          cdd_cst_bld_ident(
              &bld, pool_string_safe_len(tree, (const char *)next_tok->start,
                                         next_tok->length));
          cdd_cst_bld_space(&bld);
          cdd_cst_bld_ident(&bld, "real");
          cdd_cst_bld_punct(&bld, ",");
          cdd_cst_bld_space(&bld);
          cdd_cst_bld_ident(&bld, "imag");
          cdd_cst_bld_punct(&bld, ";");
          cdd_cst_bld_space(&bld);
          cdd_cst_bld_punct(&bld, "}");
          cdd_cst_bld_space(&bld);
          if (bld.error_state != 0) {
            cdd_cst_builder_free(&bld);
            cdd_cst_free_node_only(temp);
            return CDD_C_ERROR_MEMORY;
          }
          cdd_cst_splice_children(tree, &owning_node, child_idx, 2,
                                  temp->children, temp->num_children);
          cdd_cst_builder_free(&bld);
          cdd_cst_free_node_only(temp);
        }
      }
    } else if (tok->kind == CDD_TOKEN_KEYWORD___REAL__) {
      if (i + 1 < tree->base_tokens->size &&
          tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_IDENTIFIER) {
        cdd_token_t *next_tok = &tree->base_tokens->tokens[i + 1];
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
          cdd_cst_bld_ident(
              &bld, pool_string_safe_len(tree, (const char *)next_tok->start,
                                         next_tok->length));
          cdd_cst_bld_punct(&bld, ".");
          cdd_cst_bld_ident(&bld, "real");
          cdd_cst_bld_space(&bld);
          if (bld.error_state != 0) {
            cdd_cst_builder_free(&bld);
            cdd_cst_free_node_only(temp);
            return CDD_C_ERROR_MEMORY;
          }
          cdd_cst_splice_children(tree, &owning_node, child_idx, 2,
                                  temp->children, temp->num_children);
          cdd_cst_builder_free(&bld);
          cdd_cst_free_node_only(temp);
        }
      }
    } else if (tok->kind == CDD_TOKEN_KEYWORD___IMAG__) {
      if (i + 1 < tree->base_tokens->size &&
          tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_IDENTIFIER) {
        cdd_token_t *next_tok = &tree->base_tokens->tokens[i + 1];
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
          cdd_cst_bld_ident(
              &bld, pool_string_safe_len(tree, (const char *)next_tok->start,
                                         next_tok->length));
          cdd_cst_bld_punct(&bld, ".");
          cdd_cst_bld_ident(&bld, "imag");
          cdd_cst_bld_space(&bld);
          if (bld.error_state != 0) {
            cdd_cst_builder_free(&bld);
            cdd_cst_free_node_only(temp);
            return CDD_C_ERROR_MEMORY;
          }
          cdd_cst_splice_children(tree, &owning_node, child_idx, 2,
                                  temp->children, temp->num_children);
          cdd_cst_builder_free(&bld);
          cdd_cst_free_node_only(temp);
        }
      }
    } else if (tok->kind == CDD_TOKEN_KEYWORD_TYPEOF) {
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
          const char *inferred = NULL;
          size_t child_idx;
          cdd_cst_node_t *owning_node = NULL;

          rc = cdd_infer_type(inner, num_inner, &inferred);
#ifdef CDD_BUILD_TESTS
          if (g_gnu_standardizer_fail == 35)
            rc = CDD_C_ERROR_MEMORY;
#endif
          if (rc != CDD_C_SUCCESS)
            return rc;
          rc = gnu_find_node(tree->root, tok, &child_idx, &owning_node);
          if (rc != CDD_C_SUCCESS)
            return rc;
          {
            cdd_cst_node_t *temp = NULL;
            cdd_cst_builder_t bld;
            if (inferred) {
              rc = gnu_alloc_node(CDD_CST_UNKNOWN, &temp);
              if (rc != CDD_C_SUCCESS)
                return rc;
              cdd_cst_builder_init(&bld, tree, temp);
              cdd_cst_bld_ident(&bld, pool_string_safe(tree, inferred));
              cdd_cst_bld_space(&bld);
              if (bld.error_state != 0) {
                cdd_cst_builder_free(&bld);
                cdd_cst_free_node_only(temp);
                return CDD_C_ERROR_MEMORY;
              }
              cdd_cst_splice_children(tree, &owning_node, child_idx,
                                      (rparen_idx - i) + 1, temp->children,
                                      temp->num_children);
              cdd_cst_builder_free(&bld);
              cdd_cst_free_node_only(temp);
            } else {
              /* It's already a type (transparent resolution). Just extract the
               * tokens inside and drop typeof() */

              char buf[1024];
              char *p = buf;
              size_t k;
              int is_unqual = (tok->length == 13 || tok->length == 17);
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
                if (p != buf) {
                  *p++ = ' ';
                }
                memcpy(p, inner[k].start, inner[k].length);
                p += inner[k].length;
              }

              *p++ = ' ';
              *p = '\0';
              if (strchr(buf, '[')) {
                static int typeof_arr_idx = 0;
                char typedef_buf[1024];
                /* Extract the base type and the array part. Very hacky for the
                 * test. */
                char base[256] = {0};
                char arr[256] = {0};
                char arr_clean[256] = {0};
                char *lb = strchr(buf, '[');
                char *rb = strchr(buf, ']');
#ifdef CDD_BUILD_TESTS
                if (g_gnu_standardizer_fail == 15)
                  rb = NULL;
#endif
                if (rb) {
                  int m, ac = 0;
#if defined(_MSC_VER)
                  strncpy_s(base, sizeof(base), buf, (size_t)(lb - buf));
                  strncpy_s(arr, sizeof(arr), lb, (size_t)(rb - lb + 1));
#else
                  strncpy(base, buf, (size_t)(lb - buf));
                  strncpy(arr, lb, (size_t)(rb - lb + 1));
#endif
                  for (m = 0; arr[m]; m++) {
                    if (arr[m] != ' ') {
                      arr_clean[ac++] = arr[m];
                    }
                  }
                  CDD_SNPRINTF(
                      typedef_buf, sizeof(typedef_buf),
                      "typedef %s__cdd_typeof_arr_%d%s; __cdd_typeof_arr_%d ",
                      base, typeof_arr_idx, arr_clean, typeof_arr_idx);
                  typeof_arr_idx++;
                  rc = gnu_alloc_node(CDD_CST_UNKNOWN, &temp);
                  if (rc != CDD_C_SUCCESS)
                    return rc;
                  cdd_cst_builder_init(&bld, tree, temp);
                  cdd_cst_bld_ident(&bld, "typedef");
                  cdd_cst_bld_space(&bld);
                  cdd_cst_bld_ident(&bld, pool_string_safe(tree, base));
                  {
                    char tb2[128];
                    CDD_SNPRINTF(tb2, 128, "__cdd_typeof_arr_%d",
                                 typeof_arr_idx);
                    cdd_cst_bld_ident(&bld, pool_string_safe(tree, tb2));
                  }
                  cdd_cst_bld_snippet(&bld, arr_clean);
                  cdd_cst_bld_punct(&bld, ";");
                  cdd_cst_bld_space(&bld);
                  {
                    char tb2[128];
                    CDD_SNPRINTF(tb2, 128, "__cdd_typeof_arr_%d",
                                 typeof_arr_idx);
                    cdd_cst_bld_ident(&bld, pool_string_safe(tree, tb2));
                  }
                  cdd_cst_bld_space(&bld);
                  typeof_arr_idx++;
                  if (bld.error_state != 0) {
                    cdd_cst_builder_free(&bld);
                    cdd_cst_free_node_only(temp);
                    return CDD_C_ERROR_MEMORY;
                  }
                  cdd_cst_splice_children(tree, &owning_node, child_idx,
                                          (rparen_idx - i) + 1, temp->children,
                                          temp->num_children);
                  cdd_cst_builder_free(&bld);
                  cdd_cst_free_node_only(temp);
                } else {
                  rc = gnu_alloc_node(CDD_CST_UNKNOWN, &temp);
                  if (rc != CDD_C_SUCCESS)
                    return rc;
                  cdd_cst_builder_init(&bld, tree, temp);
                  cdd_cst_bld_ident(&bld, pool_string_safe(tree, buf));
                  if (bld.error_state != 0) {
                    cdd_cst_builder_free(&bld);
                    cdd_cst_free_node_only(temp);
                    return CDD_C_ERROR_MEMORY;
                  }
                  cdd_cst_splice_children(tree, &owning_node, child_idx,
                                          (rparen_idx - i) + 1, temp->children,
                                          temp->num_children);
                  cdd_cst_builder_free(&bld);
                  cdd_cst_free_node_only(temp);
                }
              } else {
                rc = gnu_alloc_node(CDD_CST_UNKNOWN, &temp);
                if (rc != CDD_C_SUCCESS)
                  return rc;
                cdd_cst_builder_init(&bld, tree, temp);
                cdd_cst_bld_ident(&bld, pool_string_safe(tree, buf));
                if (bld.error_state != 0) {
                  cdd_cst_builder_free(&bld);
                  cdd_cst_free_node_only(temp);
                  return CDD_C_ERROR_MEMORY;
                }
                cdd_cst_splice_children(tree, &owning_node, child_idx,
                                        (rparen_idx - i) + 1, temp->children,
                                        temp->num_children);
                cdd_cst_builder_free(&bld);
                cdd_cst_free_node_only(temp);
              }
            }
          }
        }
      }
    } else if (tok->kind == CDD_TOKEN_KEYWORD___AUTO_TYPE) {
      size_t child_idx;
      cdd_cst_node_t *temp = NULL;
      cdd_cst_node_t *owning_node = NULL;
      rc = gnu_find_node(tree->root, tok, &child_idx, &owning_node);
      if (rc != CDD_C_SUCCESS)
        return rc;
      {
        /* __auto_type inference: search forward for = and infer from RHS */
        size_t j;
        const char *inferred = "int";
        cdd_cst_builder_t bld;
        for (j = i + 1;
             j < tree->base_tokens->size &&
             tree->base_tokens->tokens[j].kind != CDD_TOKEN_SEMICOLON;
             j++) {
          if (tree->base_tokens->tokens[j].kind == CDD_TOKEN_ASSIGN) {
            if (j + 1 < tree->base_tokens->size) {
              const char *type_guess = NULL;
              cdd_c_error_t rc_inf = cdd_infer_type(
                  &tree->base_tokens->tokens[j + 1], 1, &type_guess);
#ifdef CDD_BUILD_TESTS
              if (g_gnu_standardizer_fail == 39)
                rc_inf = CDD_C_ERROR_MEMORY;
#endif
              if (rc_inf != CDD_C_SUCCESS || !type_guess) {
                inferred = "int"; /* fallback if inferred as type */
              } else {
                inferred = type_guess;
              }
            }
            break;
          }
        }
        rc = gnu_alloc_node(CDD_CST_UNKNOWN, &temp);
        if (rc != CDD_C_SUCCESS)
          return rc;
        cdd_cst_builder_init(&bld, tree, temp);
        cdd_cst_bld_ident(&bld, pool_string_safe(tree, inferred));
        cdd_cst_bld_space(&bld);
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
    } else if (tok->kind == CDD_TOKEN_NUMBER) {
      char buf[256];
      size_t copy_len = tok->length < 255 ? tok->length : 255;
      struct NumericValue nv;
      memcpy(buf, tok->start, copy_len);
      buf[copy_len] = '\0';
      if (parse_numeric_literal(buf, &nv) == CDD_C_ERROR_PARSE) {
        /* Exceeds 64-bit */
        uint64_t high = 0, low = 0;
        if (buf[0] == '0' && (buf[1] == 'x' || buf[1] == 'X')) {
          rc = cdd_parse_hex_128_literal(buf, copy_len, &high, &low);
#ifdef CDD_BUILD_TESTS
          if (g_gnu_standardizer_fail == 36)
            rc = CDD_C_ERROR_MEMORY;
#endif
        } else {
          rc = cdd_parse_128_literal(buf, copy_len, &high, &low);
#ifdef CDD_BUILD_TESTS
          if (g_gnu_standardizer_fail == 37)
            rc = CDD_C_ERROR_MEMORY;
#endif
        }
        if (rc != CDD_C_SUCCESS)
          return rc;
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
            cdd_cst_builder_init(&bld, tree, temp);
            cdd_cst_bld_ident(&bld, "cdd_make_uint128");
            cdd_cst_bld_punct(&bld, "(");
            {
              char tb2[128];
              CDD_SNPRINTF(tb2, 128, "0x%08lx%08lxULL",
                           (unsigned long)(high >> 32),
                           (unsigned long)(high & 0xFFFFFFFF));
              cdd_cst_bld_ident(&bld, pool_string_safe(tree, tb2));
            }
            cdd_cst_bld_punct(&bld, ",");
            cdd_cst_bld_space(&bld);
            {
              char tb2[128];
              CDD_SNPRINTF(tb2, 128, "0x%08lx%08lxULL",
                           (unsigned long)(low >> 32),
                           (unsigned long)(low & 0xFFFFFFFF));
              cdd_cst_bld_ident(&bld, pool_string_safe(tree, tb2));
            }
            cdd_cst_bld_punct(&bld, ")");
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
    } else if (tok->kind == CDD_TOKEN_IDENTIFIER && tok->length == 13 &&
               memcmp(tok->start, "__attribute__", 13) == 0) {
      if (i + 2 < tree->base_tokens->size &&
          tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_LPAREN &&
          tree->base_tokens->tokens[i + 2].kind == CDD_TOKEN_LPAREN) {
        if (i + 8 < tree->base_tokens->size &&
            tree->base_tokens->tokens[i + 4].kind == CDD_TOKEN_LPAREN &&
            tree->base_tokens->tokens[i + 6].kind == CDD_TOKEN_RPAREN &&
            tree->base_tokens->tokens[i + 7].kind == CDD_TOKEN_RPAREN &&
            tree->base_tokens->tokens[i + 8].kind == CDD_TOKEN_RPAREN) {
          cdd_token_t *attr = &tree->base_tokens->tokens[i + 3];
          if (attr->length == 11 &&
              memcmp(attr->start, "vector_size", 11) == 0) {
            rc = replace_token_with_text(tree, tok, tok->kind, "", 0);
            if (rc != CDD_C_SUCCESS)
              return rc;
            {
              size_t w;
              for (w = i + 1; w <= i + 5; w++) {
                rc = replace_token_with_text(
                    tree, &tree->base_tokens->tokens[w],
                    tree->base_tokens->tokens[w].kind, "", 0);
                if (rc != CDD_C_SUCCESS)
                  return rc;
              }
            }
            tree->base_tokens->tokens[i + 6].length = 0;
            tree->base_tokens->tokens[i + 7].length = 0;
            tree->base_tokens->tokens[i + 8].length = 0;
          } else if (attr->length == 7 &&
                     memcmp(attr->start, "aligned", 7) == 0) {
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
              cdd_cst_bld_ident(&bld, "_Alignas");
              cdd_cst_bld_punct(&bld, "(");
              cdd_cst_bld_ident(
                  &bld,
                  pool_string_safe_len(
                      tree,
                      (const char *)tree->base_tokens->tokens[i + 5].start,
                      tree->base_tokens->tokens[i + 5].length));
              cdd_cst_bld_punct(&bld, ")");
              if (bld.error_state != 0) {
                cdd_cst_builder_free(&bld);
                cdd_cst_free_node_only(temp);
                return CDD_C_ERROR_MEMORY;
              }
              cdd_cst_splice_children(tree, &owning_node, child_idx, 9,
                                      temp->children, temp->num_children);
              cdd_cst_builder_free(&bld);
              cdd_cst_free_node_only(temp);
            }
            tree->base_tokens->tokens[i + 3].length = 0;
            tree->base_tokens->tokens[i + 4].length = 0;
            tree->base_tokens->tokens[i + 5].length = 0;
            tree->base_tokens->tokens[i + 6].length = 0;
            tree->base_tokens->tokens[i + 7].length = 0;
            tree->base_tokens->tokens[i + 8].length = 0;
          } else if (attr->length == 4 && memcmp(attr->start, "mode", 4) == 0) {
            cdd_token_t *mode_val = &tree->base_tokens->tokens[i + 5];
            const char *map = "";
            if (mode_val->length == 2) {
              if (memcmp(mode_val->start, "QI", 2) == 0)
                map = "/* mode(QI) -> int8_t */";
              else if (memcmp(mode_val->start, "HI", 2) == 0)
                map = "/* mode(HI) -> int16_t */";
              else if (memcmp(mode_val->start, "SI", 2) == 0)
                map = "/* mode(SI) -> int32_t */";
              else if (memcmp(mode_val->start, "DI", 2) == 0)
                map = "/* mode(DI) -> int64_t */";
              else if (memcmp(mode_val->start, "TI", 2) == 0)
                map = "/* mode(TI) -> int128_t */";
            }
            rc =
                replace_token_with_text(tree, tok, tok->kind, map, strlen(map));
            if (rc != CDD_C_SUCCESS)
              return rc;
            tree->base_tokens->tokens[i + 1].length = 0;
            tree->base_tokens->tokens[i + 2].length = 0;
            tree->base_tokens->tokens[i + 3].length = 0;
            tree->base_tokens->tokens[i + 4].length = 0;
            tree->base_tokens->tokens[i + 5].length = 0;
            tree->base_tokens->tokens[i + 6].length = 0;
            tree->base_tokens->tokens[i + 7].length = 0;
            tree->base_tokens->tokens[i + 8].length = 0;
          }
        } else if (i + 5 < tree->base_tokens->size &&
                   tree->base_tokens->tokens[i + 4].kind == CDD_TOKEN_RPAREN &&
                   tree->base_tokens->tokens[i + 5].kind == CDD_TOKEN_RPAREN) {
          cdd_token_t *attr = &tree->base_tokens->tokens[i + 3];
          if (attr->length == 8 && memcmp(attr->start, "noreturn", 8) == 0) {
            rc = replace_token_with_text(tree, tok, tok->kind, "_Noreturn", 9);
            if (rc != CDD_C_SUCCESS)
              return rc;
            {
              size_t w;
              for (w = i + 1; w <= i + 5; w++) {
                rc = replace_token_with_text(
                    tree, &tree->base_tokens->tokens[w],
                    tree->base_tokens->tokens[w].kind, "", 0);
                if (rc != CDD_C_SUCCESS)
                  return rc;
              }
            }
          } else if (attr->length == 6 &&
                     memcmp(attr->start, "unused", 6) == 0) {
            rc = replace_token_with_text(tree, tok, tok->kind, "/* unused */",
                                         12);
            if (rc != CDD_C_SUCCESS)
              return rc;
            {
              size_t w;
              for (w = i + 1; w <= i + 5; w++) {
                rc = replace_token_with_text(
                    tree, &tree->base_tokens->tokens[w],
                    tree->base_tokens->tokens[w].kind, "", 0);
                if (rc != CDD_C_SUCCESS)
                  return rc;
              }
            }
          } else if (attr->length == 17 &&
                     memcmp(attr->start, "transparent_union", 17) == 0) {
            rc = replace_token_with_text(tree, tok, tok->kind,
                                         "/* transparent_union */", 23);
            if (rc != CDD_C_SUCCESS)
              return rc;
            {
              size_t w;
              for (w = i + 1; w <= i + 5; w++) {
                rc = replace_token_with_text(
                    tree, &tree->base_tokens->tokens[w],
                    tree->base_tokens->tokens[w].kind, "", 0);
                if (rc != CDD_C_SUCCESS)
                  return rc;
              }
            }
          } else if (attr->length == 6 &&
                     memcmp(attr->start, "packed", 6) == 0) {
#ifdef CDD_BUILD_TESTS
            if (g_gnu_standardizer_fail != 16)
#endif
            {
              rc = replace_token_with_text(tree, tok, tok->kind,
                                           "\n#pragma pack(push, 1)\n", 23);
              if (rc != CDD_C_SUCCESS)
                return rc;
            }
            {
              size_t w;
              for (w = i + 1; w <= i + 5; w++) {
                rc = replace_token_with_text(
                    tree, &tree->base_tokens->tokens[w],
                    tree->base_tokens->tokens[w].kind, "", 0);
                if (rc != CDD_C_SUCCESS)
                  return rc;
              }
            }

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
          } else {
            char *heap_buf;
            rc = gnu_malloc(attr->length + 7, (void **)&heap_buf);
            if (rc != CDD_C_SUCCESS)
              return rc;
            memcpy(heap_buf, "/* ", 3);
            memcpy(heap_buf + 3, attr->start, attr->length);
            memcpy(heap_buf + 3 + attr->length, " */", 3);
            heap_buf[6 + attr->length] = '\0';
#ifdef CDD_BUILD_TESTS
            if (g_gnu_standardizer_fail != 7)
#endif
            {
              const char *pooled = pool_string_safe(tree, heap_buf);
              if (!pooled) {
                free(heap_buf);
                return CDD_C_ERROR_MEMORY;
              }
              rc = replace_token_with_text(tree, tok, tok->kind, pooled,
                                           6 + attr->length);
              if (rc != CDD_C_SUCCESS) {
                free(heap_buf);
                return rc;
              }
            }
            free(heap_buf);
            tree->base_tokens->tokens[i + 1].length = 0;
            tree->base_tokens->tokens[i + 2].length = 0;
            tree->base_tokens->tokens[i + 3].length = 0;
            tree->base_tokens->tokens[i + 4].length = 0;
            tree->base_tokens->tokens[i + 5].length = 0;
          }
        } else {
          /* Generic multi-arg attribute parser: find the matching )) */
          int lparen_count = 2;
          size_t k;
          for (k = i + 3; k < tree->base_tokens->size; k++) {
            if (tree->base_tokens->tokens[k].kind == CDD_TOKEN_LPAREN)
              lparen_count++;
            else if (tree->base_tokens->tokens[k].kind == CDD_TOKEN_RPAREN)
              lparen_count--;
            if (lparen_count == 0) {
              /* Strip it */
              size_t wipe;
              rc = replace_token_with_text(tree, tok, tok->kind,
                                           "/* attribute */", 15);
              if (rc != CDD_C_SUCCESS)
                return rc;
              for (wipe = i + 1; wipe <= k; wipe++) {
                rc = replace_token_with_text(
                    tree, &tree->base_tokens->tokens[wipe],
                    tree->base_tokens->tokens[wipe].kind, "", 0);
                if (rc != CDD_C_SUCCESS)
                  return rc;
              }
              break;
            }
          }
        }
      }
    }
  }
  return CDD_C_SUCCESS;
}
