/**
 * @file gnu_standardizer_builtins.c
 * @brief GNU builtin intrinsics and GCC inline assembly lowering pass.
 */

/* clang-format off */
#include "gnu_standardizer_internal.h"
/* clang-format on */

/** @brief Bitmask flags for portable builtin/intrinsic shims injected per TU.
 */
enum gnu_builtin_shim_flags_t {
  GNU_SHIM_CLZ = 1u << 0,
  GNU_SHIM_CTZ = 1u << 1,
  GNU_SHIM_FFS = 1u << 2,
  GNU_SHIM_POPCOUNT = 1u << 3,
  GNU_SHIM_PARITY = 1u << 4,
  GNU_SHIM_BSWAP16 = 1u << 5,
  GNU_SHIM_BSWAP32 = 1u << 6,
  GNU_SHIM_BSWAP64 = 1u << 7,
  GNU_SHIM_OVF_ADD = 1u << 8,
  GNU_SHIM_OVF_SUB = 1u << 9,
  GNU_SHIM_OVF_MUL = 1u << 10,
  GNU_SHIM_ASM_BARRIER = 1u << 11,
  GNU_SHIM_ASM_PAUSE = 1u << 12,
  GNU_SHIM_ASM_RDTSC = 1u << 13,
  GNU_SHIM_ASM_CPUID = 1u << 14,
  GNU_SHIM_TRAP_MSVC = 1u << 15
};

/**
 * @brief Appends a string fragment into `hdr` within `cap`.
 *
 * @param[in,out] hdr Destination buffer.
 * @param[in] cap Buffer capacity.
 * @param[in,out] io_len Current length.
 * @param[in] str Fragment to append.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t gnu_shim_append(char *hdr, size_t cap, size_t *io_len,
                                     const char *str) {
  size_t sl = strlen(str) & (cap - 1);
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  memcpy(hdr + *io_len, str, sl);
  *io_len += sl;
  hdr[*io_len] = '\0';
  return CDD_C_SUCCESS;
}

/**
 * @brief Finds the matching `)` and top-level `,` argument boundaries for a
 * call starting at `lparen_idx`.
 *
 * @param[in] toks Base token array.
 * @param[in] n Total token count.
 * @param[in] lparen_idx Index of opening `(`.
 * @param[out] out_rparen Receives index of matching `)`.
 * @param[out] out_commas Array of up to 8 top-level comma token indices.
 * @param[out] out_num_commas Number of commas recorded.
 * @param[out] out_found Receives 1 if matching `)` was found.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t gnu_parse_call_args(const cdd_token_t *toks, size_t n,
                                         size_t lparen_idx, size_t *out_rparen,
                                         size_t out_commas[8],
                                         size_t *out_num_commas,
                                         int *out_found) {
  size_t k;
  int depth = 0;
  size_t nc = 0;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  *out_rparen = 0;
  *out_num_commas = 0;
  *out_found = 0;
  for (k = lparen_idx; k < n; k++) {
    if (toks[k].kind == CDD_TOKEN_LPAREN ||
        toks[k].kind == CDD_TOKEN_LBRACKET ||
        toks[k].kind == CDD_TOKEN_LBRACE) {
      depth++;
    } else if (toks[k].kind == CDD_TOKEN_RPAREN ||
               toks[k].kind == CDD_TOKEN_RBRACKET ||
               toks[k].kind == CDD_TOKEN_RBRACE) {
      depth--;
      if (depth == 0) {
        if (toks[k].kind == CDD_TOKEN_RPAREN) {
          *out_rparen = k;
          *out_num_commas = nc;
          *out_found = 1;
        }
        return CDD_C_SUCCESS;
      }
    } else if (depth == 1 && toks[k].kind == CDD_TOKEN_COMMA && nc < 8) {
      out_commas[nc++] = k;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Extracts the token text covered by `toks[s..e_excl)`.
 *
 * @param[in] toks Token array.
 * @param[in] s Start token index (inclusive).
 * @param[in] e_excl End token index (exclusive).
 * @param[out] buf Output buffer.
 * @param[in] cap Output buffer capacity.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t gnu_slice_text(const cdd_token_t *toks, size_t s,
                                    size_t e_excl, char *buf, size_t cap) {
  size_t pos = 0;
  size_t k;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  buf[0] = '\0';
  if (s >= e_excl)
    return CDD_C_SUCCESS;
  for (k = s; k < e_excl; k++) {
    if (toks[k].length == 0)
      continue;
    if (pos > 0 && pos + 1 < cap) {
      buf[pos++] = ' ';
    }
    if (pos + toks[k].length < cap) {
      memcpy(buf + pos, toks[k].start, toks[k].length);
      pos += toks[k].length;
    }
  }
  buf[pos] = '\0';
  return CDD_C_SUCCESS;
}

/**
 * @brief Strips pure whitespace and newline trivia nodes from `*head_ptr` while
 * preserving comment trivia.
 *
 * @param[in,out] head_ptr Address of trivia list head pointer.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t gnu_strip_ws_trivia(cdd_trivia_t **head_ptr) {
  cdd_trivia_t *cur = *head_ptr;
  cdd_trivia_t *prev = NULL;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
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

/**
 * @brief Replaces token span `toks[start_idx..end_idx]` inclusive with a single
 * pooled replacement string on `toks[start_idx]` and wipes the rest.
 *
 * @param[in,out] tree CST tree.
 * @param[in] start_idx First token index.
 * @param[in] end_idx Last token index (inclusive).
 * @param[in] repl Null-terminated replacement string.
 * @return CDD_C_SUCCESS or error code.
 */
static cdd_c_error_t gnu_replace_token_range(cdd_cst_tree_t *tree,
                                             size_t start_idx, size_t end_idx,
                                             const char *repl) {
  const char *pooled = NULL;
  cdd_c_error_t rc;
  size_t w;
  rc = cdd_pool_string_safe(tree, repl, &pooled);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = replace_token_with_text(tree, &tree->base_tokens->tokens[start_idx],
                               CDD_TOKEN_IDENTIFIER, pooled, strlen(pooled));
  if (rc != CDD_C_SUCCESS)
    return rc;
  for (w = start_idx + 1; w <= end_idx; w++) {
    size_t c_idx = 0;
    cdd_cst_node_t *p_node = NULL;
    rc = replace_token_with_text(tree, &tree->base_tokens->tokens[w],
                                 tree->base_tokens->tokens[w].kind, "", 0);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = gnu_opt_find_node(tree->root, &tree->base_tokens->tokens[w], &c_idx,
                           &p_node);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (p_node) {
      rc = gnu_strip_ws_trivia(
          &p_node->children[c_idx].val.token->leading_trivia);
      if (rc != CDD_C_SUCCESS)
        return rc;
      if (w < end_idx) {
        rc = gnu_strip_ws_trivia(
            &p_node->children[c_idx].val.token->trailing_trivia);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks if `toks[s..e_excl)` is a compile-time constant expression and
 * optionally evaluates simple integer constants.
 *
 * @param[in] tree CST tree.
 * @param[in] gctx Optional GNU standardizer context.
 * @param[in] toks Token array.
 * @param[in] s Start index.
 * @param[in] e_excl End index (exclusive).
 * @param[out] out_is_const Receives 1 if compile-time constant, 0 otherwise.
 * @param[out] out_val Receives integer constant value when evaluable.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t gnu_eval_const_arg(cdd_cst_tree_t *tree,
                                        struct gnu_standardizer_ctx_t *gctx,
                                        cdd_token_t *toks, size_t s,
                                        size_t e_excl, int *out_is_const,
                                        long *out_val) {
  size_t k;
  int any = 0;
  long val = 0;
  int sign = 1;
  cdd_c_error_t rc;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  *out_is_const = 0;
  *out_val = 0;
  if (s >= e_excl)
    return CDD_C_SUCCESS;

  for (k = s; k < e_excl; k++) {
    cdd_token_t *t = &toks[k];
    if (t->length == 0)
      continue;
    any = 1;
    if (t->kind == CDD_TOKEN_NUMBER) {
      char nb[64];
      size_t nl = t->length < 63 ? t->length : 63;
      memcpy(nb, t->start, nl);
      nb[nl] = '\0';
      if (nl >= 3 && nb[0] == '0' && (nb[1] == 'x' || nb[1] == 'X'))
        val = sign * strtol(nb + 2, NULL, 16);
      else if (nl >= 3 && nb[0] == '0' && (nb[1] == 'b' || nb[1] == 'B'))
        val = sign * strtol(nb + 2, NULL, 2);
      else
        val = sign * strtol(nb, NULL, 10);
      sign = 1;
      continue;
    }
    if (t->kind == CDD_TOKEN_CHAR || t->kind == CDD_TOKEN_STRING) {
      val = 1;
      continue;
    }
    if (t->kind == CDD_TOKEN_MINUS) {
      sign = -sign;
      continue;
    }
    if (t->kind == CDD_TOKEN_LPAREN || t->kind == CDD_TOKEN_RPAREN ||
        t->kind == CDD_TOKEN_PLUS || t->kind == CDD_TOKEN_STAR ||
        t->kind == CDD_TOKEN_SLASH || t->kind == CDD_TOKEN_TILDE ||
        (t->kind == CDD_TOKEN_OTHER && t->length == 1 && t->start[0] == '!')) {
      if (t->kind == CDD_TOKEN_OTHER)
        val = !val;
      continue;
    }
    if (t->kind == CDD_TOKEN_IDENTIFIER && t->length == 6 &&
        memcmp(t->start, "sizeof", 6) == 0) {
      int d = 0;
      val = 4;
      k++;
      while (k < e_excl) {
        if (toks[k].kind == CDD_TOKEN_LPAREN)
          d++;
        else if (toks[k].kind == CDD_TOKEN_RPAREN) {
          d--;
          if (d <= 0)
            break;
        }
        k++;
      }
      continue;
    }
    if (t->kind == CDD_TOKEN_IDENTIFIER && t->length < 128 && gctx &&
        gctx->sem_env) {
      char nm[128];
      cdd_cst_scope_t *sc = gctx->sem_env->global_scope;
      cdd_cst_symbol_t *sym = NULL;
      size_t cidx = 0;
      cdd_cst_node_t *owner = NULL;
      memcpy(nm, t->start, t->length);
      nm[t->length] = '\0';
      rc = gnu_opt_find_node(tree->root, t, &cidx, &owner);
      if (rc != CDD_C_SUCCESS)
        return rc;
      if (owner) {
        rc = gnu_opt_scope_for_node(gctx->sem_env, owner, &sc);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
      rc = gnu_opt_scope_lookup(sc, nm, CDD_CST_SYMBOL_ENUM_CONSTANT, &sym);
      if (rc != CDD_C_SUCCESS)
        return rc;
      if (sym && sym->has_enum_value) {
        val = sign * sym->enum_value;
        sign = 1;
        continue;
      }
      *out_is_const = 0;
      return CDD_C_SUCCESS;
    }
    *out_is_const = 0;
    return CDD_C_SUCCESS;
  }

  *out_is_const = any;
  *out_val = val;
  return CDD_C_SUCCESS;
}

/**
 * @brief Prepends required portable builtin/intrinsic C89/MSVC helper
 * definitions to the root of `tree` based on `shim_mask`.
 *
 * @param[in,out] tree CST tree.
 * @param[in] shim_mask Bitmask of `enum gnu_builtin_shim_flags_t`.
 * @return CDD_C_SUCCESS or error code.
 */
static cdd_c_error_t gnu_inject_builtin_shims(cdd_cst_tree_t *tree,
                                              unsigned int shim_mask) {
  char *hdr = NULL;
  size_t cap = 16384;
  size_t len = 0;
  cdd_c_error_t rc;

  rc = gnu_malloc(cap, (void **)&hdr);
  if (rc != CDD_C_SUCCESS)
    return rc;
  hdr[0] = '\0';

  rc = gnu_shim_append(hdr, cap, &len,
                       "#ifndef CDD_GNU_BUILTIN_SHIMS_DEFINED\n"
                       "#define CDD_GNU_BUILTIN_SHIMS_DEFINED\n"
                       "#if defined(_MSC_VER) || defined(__WATCOMC__)\n"
                       "#define CDD_BI_INLINE __inline\n"
                       "typedef __int64 cdd_bi_i64_t;\n"
                       "typedef unsigned __int64 cdd_bi_u64_t;\n"
                       "#elif defined(__GNUC__) || defined(__clang__)\n"
                       "#define CDD_BI_INLINE __inline__ "
                       "__attribute__((__unused__))\n"
                       "__extension__ typedef long long cdd_bi_i64_t;\n"
                       "__extension__ typedef unsigned long long "
                       "cdd_bi_u64_t;\n"
                       "#else\n");
  if (rc != CDD_C_SUCCESS)
    goto cleanup;
  rc = gnu_shim_append(hdr, cap, &len,
                       "#define CDD_BI_INLINE\n"
                       "#if defined(__STDC_VERSION__) && __STDC_VERSION__ "
                       ">= 199901L\n"
                       "typedef long long cdd_bi_i64_t;\n"
                       "typedef unsigned long long cdd_bi_u64_t;\n"
                       "#else\n"
                       "typedef long cdd_bi_i64_t;\n"
                       "typedef unsigned long cdd_bi_u64_t;\n"
                       "#endif\n#endif\n");
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  if (shim_mask & GNU_SHIM_ASM_BARRIER) {
    rc = gnu_shim_append(hdr, cap, &len,
                         "#ifdef _MSC_VER\nvoid _ReadWriteBarrier(void);\n"
                         "#pragma intrinsic(_ReadWriteBarrier)\n#endif\n");
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
  }
  if (shim_mask & GNU_SHIM_ASM_PAUSE) {
    rc = gnu_shim_append(hdr, cap, &len,
                         "#ifdef _MSC_VER\nvoid _mm_pause(void);\n#endif\n");
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
  }
  if (shim_mask & GNU_SHIM_ASM_RDTSC) {
    rc = gnu_shim_append(
        hdr, cap, &len,
        "#ifdef _MSC_VER\nunsigned __int64 __rdtsc(void);\n#pragma "
        "intrinsic(__rdtsc)\n#endif\n");
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
  }
  if (shim_mask & GNU_SHIM_ASM_CPUID) {
    rc = gnu_shim_append(hdr, cap, &len,
                         "#ifdef _MSC_VER\nvoid __cpuid(int[4], int);\n"
                         "#pragma intrinsic(__cpuid)\n#endif\n");
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
  }
  if (shim_mask & GNU_SHIM_TRAP_MSVC) {
    rc = gnu_shim_append(
        hdr, cap, &len,
        "#ifdef _MSC_VER\nvoid __cdecl __debugbreak(void);\n#endif\n");
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
  }
  if (shim_mask & GNU_SHIM_CLZ) {
    rc = gnu_shim_append(
        hdr, cap, &len,
        "static CDD_BI_INLINE int __cdd_builtin_clzll(cdd_bi_u64_t x) {\n"
        "  int n = 0;\n  if (x == 0) return 64;\n"
        "  while ((x & (((cdd_bi_u64_t)1) << 63)) == 0) { n++; x <<= 1; }\n"
        "  return n;\n}\n");
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    rc = gnu_shim_append(
        hdr, cap, &len,
        "static CDD_BI_INLINE int __cdd_builtin_clz(unsigned int x) {\n"
        "  int n = 0;\n  if (x == 0) return 32;\n"
        "  while ((x & 0x80000000U) == 0) { n++; x <<= 1; }\n"
        "  return n;\n}\n"
        "static CDD_BI_INLINE int __cdd_builtin_clzl(unsigned long x) {\n"
        "#ifdef _MSC_VER\n"
        "  return __cdd_builtin_clz((unsigned int)x);\n"
        "#else\n"
        "  return (sizeof(unsigned long) == 8)\n"
        "    ? __cdd_builtin_clzll((cdd_bi_u64_t)x)\n"
        "    : __cdd_builtin_clz((unsigned int)x);\n"
        "#endif\n}\n");
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
  }
  if (shim_mask & (GNU_SHIM_CTZ | GNU_SHIM_FFS)) {
    rc = gnu_shim_append(
        hdr, cap, &len,
        "static CDD_BI_INLINE int __cdd_builtin_ctzll(cdd_bi_u64_t x) {\n"
        "  int n = 0;\n  if (x == 0) return 64;\n"
        "  while ((x & 1U) == 0) { n++; x >>= 1; }\n"
        "  return n;\n}\n");
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    rc = gnu_shim_append(
        hdr, cap, &len,
        "static CDD_BI_INLINE int __cdd_builtin_ctz(unsigned int x) {\n"
        "  int n = 0;\n  if (x == 0) return 32;\n"
        "  while ((x & 1U) == 0) { n++; x >>= 1; }\n"
        "  return n;\n}\n"
        "static CDD_BI_INLINE int __cdd_builtin_ctzl(unsigned long x) {\n"
        "  return __cdd_builtin_ctzll((cdd_bi_u64_t)x);\n}\n");
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
  }
  if (shim_mask & GNU_SHIM_FFS) {
    rc = gnu_shim_append(
        hdr, cap, &len,
        "static CDD_BI_INLINE int __cdd_builtin_ffsll(cdd_bi_i64_t x) {\n"
        "  return (x == 0) ? 0 : (__cdd_builtin_ctzll((cdd_bi_u64_t)x) + 1);"
        "\n}\n"
        "static CDD_BI_INLINE int __cdd_builtin_ffs(int x) {\n"
        "  return (x == 0) ? 0 : (__cdd_builtin_ctz((unsigned int)x) + 1);\n}\n"
        "static CDD_BI_INLINE int __cdd_builtin_ffsl(long x) {\n"
        "  return __cdd_builtin_ffsll((cdd_bi_i64_t)x);\n}\n");
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
  }
  if (shim_mask & (GNU_SHIM_POPCOUNT | GNU_SHIM_PARITY)) {
    rc = gnu_shim_append(
        hdr, cap, &len,
        "static CDD_BI_INLINE int __cdd_builtin_popcountll(cdd_bi_u64_t x) {\n"
        "  int c = 0;\n  while (x) { x &= (x - 1U); c++; }\n  return c;\n}\n"
        "static CDD_BI_INLINE int __cdd_builtin_popcount(unsigned int x) {\n"
        "  return __cdd_builtin_popcountll((cdd_bi_u64_t)x);\n}\n"
        "static CDD_BI_INLINE int __cdd_builtin_popcountl(unsigned long x) {\n"
        "  return __cdd_builtin_popcountll((cdd_bi_u64_t)x);\n}\n");
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
  }
  if (shim_mask & GNU_SHIM_PARITY) {
    rc = gnu_shim_append(
        hdr, cap, &len,
        "static CDD_BI_INLINE int __cdd_builtin_parityll(cdd_bi_u64_t x) {\n"
        "  return __cdd_builtin_popcountll(x) & 1;\n}\n"
        "static CDD_BI_INLINE int __cdd_builtin_parity(unsigned int x) {\n"
        "  return __cdd_builtin_popcount(x) & 1;\n}\n"
        "static CDD_BI_INLINE int __cdd_builtin_parityl(unsigned long x) {\n"
        "  return __cdd_builtin_popcountl(x) & 1;\n}\n");
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
  }
  if (shim_mask & GNU_SHIM_BSWAP16) {
    rc = gnu_shim_append(
        hdr, cap, &len,
        "static CDD_BI_INLINE unsigned short __cdd_builtin_bswap16("
        "unsigned short x) {\n"
        "  return (unsigned short)((x << 8) | (x >> 8));\n}\n");
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
  }
  if (shim_mask & GNU_SHIM_BSWAP32) {
    rc = gnu_shim_append(
        hdr, cap, &len,
        "static CDD_BI_INLINE unsigned int __cdd_builtin_bswap32("
        "unsigned int x) {\n"
        "  return ((x & 0x000000FFU) << 24) | ((x & 0x0000FF00U) << 8) |\n"
        "         ((x & 0x00FF0000U) >> 8) | ((x & 0xFF000000U) >> 24);\n}\n");
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
  }
  if (shim_mask & GNU_SHIM_BSWAP64) {
    rc = gnu_shim_append(
        hdr, cap, &len,
        "static CDD_BI_INLINE cdd_bi_u64_t __cdd_builtin_bswap64("
        "cdd_bi_u64_t x) {\n"
        "  return ((x & 0xFFU) << 56) | (((x >> 8) & 0xFFU) << 48) |\n"
        "         (((x >> 16) & 0xFFU) << 40) | (((x >> 24) & 0xFFU) << 32) |\n"
        "         (((x >> 32) & 0xFFU) << 24) | (((x >> 40) & 0xFFU) << 16) |\n"
        "         (((x >> 48) & 0xFFU) << 8) | ((x >> 56) & 0xFFU);\n}\n");
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
  }
  if (shim_mask & GNU_SHIM_OVF_ADD) {
    rc = gnu_shim_append(
        hdr, cap, &len,
        "static CDD_BI_INLINE int __cdd_builtin_sadd_overflow("
        "int a, int b, int *r) {\n"
        "  unsigned int ua = (unsigned int)a, ub = (unsigned int)b;\n"
        "  unsigned int ur = ua + ub;\n  *r = (int)ur;\n"
        "  return (((ua ^ ur) & (ub ^ ur)) >> 31) != 0;\n}\n"
        "static CDD_BI_INLINE int __cdd_builtin_uadd_overflow("
        "unsigned int a, unsigned int b, unsigned int *r) {\n"
        "  *r = a + b;\n  return *r < a;\n}\n");
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    rc = gnu_shim_append(
        hdr, cap, &len,
        "static CDD_BI_INLINE int __cdd_builtin_saddl_overflow("
        "long a, long b, long *r) {\n"
        "  unsigned long ua = (unsigned long)a, ub = (unsigned long)b;\n"
        "  unsigned long ur = ua + ub;\n  *r = (long)ur;\n"
        "  return (((ua ^ ur) & (ub ^ ur)) >> (sizeof(long) * 8 - 1)) != 0;\n"
        "}\n"
        "static CDD_BI_INLINE int __cdd_builtin_uaddl_overflow("
        "unsigned long a, unsigned long b, unsigned long *r) {\n"
        "  *r = a + b;\n  return *r < a;\n}\n");
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    rc = gnu_shim_append(
        hdr, cap, &len,
        "static CDD_BI_INLINE int __cdd_builtin_saddll_overflow("
        "cdd_bi_i64_t a, cdd_bi_i64_t b, cdd_bi_i64_t *r) {\n"
        "  cdd_bi_u64_t ua = (cdd_bi_u64_t)a, ub = (cdd_bi_u64_t)b;\n"
        "  cdd_bi_u64_t ur = ua + ub;\n  *r = (cdd_bi_i64_t)ur;\n"
        "  return (((ua ^ ur) & (ub ^ ur)) >> 63) != 0;\n}\n"
        "static CDD_BI_INLINE int __cdd_builtin_uaddll_overflow("
        "cdd_bi_u64_t a, cdd_bi_u64_t b, cdd_bi_u64_t *r) {\n"
        "  *r = a + b;\n  return *r < a;\n}\n");
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
  }
  if (shim_mask & GNU_SHIM_OVF_SUB) {
    rc = gnu_shim_append(
        hdr, cap, &len,
        "static CDD_BI_INLINE int __cdd_builtin_ssub_overflow("
        "int a, int b, int *r) {\n"
        "  unsigned int ua = (unsigned int)a, ub = (unsigned int)b;\n"
        "  unsigned int ur = ua - ub;\n  *r = (int)ur;\n"
        "  return (((ua ^ ub) & (ua ^ ur)) >> 31) != 0;\n}\n"
        "static CDD_BI_INLINE int __cdd_builtin_usub_overflow("
        "unsigned int a, unsigned int b, unsigned int *r) {\n"
        "  *r = a - b;\n  return a < b;\n}\n");
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    rc = gnu_shim_append(
        hdr, cap, &len,
        "static CDD_BI_INLINE int __cdd_builtin_ssubl_overflow("
        "long a, long b, long *r) {\n"
        "  unsigned long ua = (unsigned long)a, ub = (unsigned long)b;\n"
        "  unsigned long ur = ua - ub;\n  *r = (long)ur;\n"
        "  return (((ua ^ ub) & (ua ^ ur)) >> (sizeof(long) * 8 - 1)) != 0;\n"
        "}\n"
        "static CDD_BI_INLINE int __cdd_builtin_usubl_overflow("
        "unsigned long a, unsigned long b, unsigned long *r) {\n"
        "  *r = a - b;\n  return a < b;\n}\n");
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    rc = gnu_shim_append(
        hdr, cap, &len,
        "static CDD_BI_INLINE int __cdd_builtin_ssubll_overflow("
        "cdd_bi_i64_t a, cdd_bi_i64_t b, cdd_bi_i64_t *r) {\n"
        "  cdd_bi_u64_t ua = (cdd_bi_u64_t)a, ub = (cdd_bi_u64_t)b;\n"
        "  cdd_bi_u64_t ur = ua - ub;\n  *r = (cdd_bi_i64_t)ur;\n"
        "  return (((ua ^ ub) & (ua ^ ur)) >> 63) != 0;\n}\n"
        "static CDD_BI_INLINE int __cdd_builtin_usubll_overflow("
        "cdd_bi_u64_t a, cdd_bi_u64_t b, cdd_bi_u64_t *r) {\n"
        "  *r = a - b;\n  return a < b;\n}\n");
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
  }
  if (shim_mask & GNU_SHIM_OVF_MUL) {
    rc = gnu_shim_append(
        hdr, cap, &len,
        "static CDD_BI_INLINE int __cdd_builtin_smul_overflow("
        "int a, int b, int *r) {\n"
        "  cdd_bi_i64_t p = (cdd_bi_i64_t)a * (cdd_bi_i64_t)b;\n"
        "  *r = (int)p;\n  return p != (cdd_bi_i64_t)*r;\n}\n"
        "static CDD_BI_INLINE int __cdd_builtin_umul_overflow("
        "unsigned int a, unsigned int b, unsigned int *r) {\n"
        "  cdd_bi_u64_t p = (cdd_bi_u64_t)a * (cdd_bi_u64_t)b;\n"
        "  *r = (unsigned int)p;\n  return (p >> 32) != 0;\n}\n");
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    rc = gnu_shim_append(
        hdr, cap, &len,
        "static CDD_BI_INLINE int __cdd_builtin_smull_overflow("
        "long a, long b, long *r) {\n"
        "  *r = (long)((unsigned long)a * (unsigned long)b);\n"
        "  return (a != 0 && *r / a != b);\n}\n"
        "static CDD_BI_INLINE int __cdd_builtin_umull_overflow("
        "unsigned long a, unsigned long b, unsigned long *r) {\n"
        "  *r = a * b;\n  return (a != 0 && *r / a != b);\n}\n");
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    rc = gnu_shim_append(
        hdr, cap, &len,
        "static CDD_BI_INLINE int __cdd_builtin_smulll_overflow("
        "cdd_bi_i64_t a, cdd_bi_i64_t b, cdd_bi_i64_t *r) {\n"
        "  *r = (cdd_bi_i64_t)((cdd_bi_u64_t)a * (cdd_bi_u64_t)b);\n"
        "  return (a != 0 && *r / a != b);\n}\n"
        "static CDD_BI_INLINE int __cdd_builtin_umulll_overflow("
        "cdd_bi_u64_t a, cdd_bi_u64_t b, cdd_bi_u64_t *r) {\n"
        "  *r = a * b;\n  return (a != 0 && *r / a != b);\n}\n");
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
  }

  rc = gnu_shim_append(hdr, cap, &len,
                       "#endif /* CDD_GNU_BUILTIN_SHIMS_DEFINED */\n");
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  rc = gnu_splice_snippet_at(tree, &tree->root, 0, 0, hdr);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

cleanup:
  C_CDD_FREE(hdr);
  return rc;
}

/*
 * @brief Lowers GCC inline assembly (`__asm__`, `__asm`, `asm`) idioms in a CST
 * tree (`memory` barrier, `pause`, `cpuid`, `rdtsc`, `bswap`) and guards
 * unrecognized blocks for MSVC/ISO C89.
 */
cdd_c_error_t gnu_standardize_asm_ctx(cdd_cst_tree_t *tree,
                                      struct gnu_standardizer_ctx_t *gctx) {
  size_t i;
  int is_msvc = 0;
  int is_iso_c89 = 0;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  if (!tree || !tree->base_tokens)
    return CDD_C_SUCCESS;
  if (gctx && gctx->config) {
    rc = gnu_is_msvc_target(gctx->config, &is_msvc);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = gnu_is_strict_iso_c89(gctx->config, &is_iso_c89);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  for (i = 0; i < tree->base_tokens->size; i++) {
    cdd_token_t *tok = &tree->base_tokens->tokens[i];
    if (tok->kind == CDD_TOKEN_IDENTIFIER &&
        ((tok->length == 3 && memcmp(tok->start, "asm", 3) == 0) ||
         (tok->length == 5 && memcmp(tok->start, "__asm", 5) == 0) ||
         (tok->length == 7 && memcmp(tok->start, "__asm__", 7) == 0))) {
      size_t lp = i + 1;
      size_t rp = 0;
      size_t commas[8];
      size_t nc = 0;
      int found = 0;
      while (lp < tree->base_tokens->size &&
             (tree->base_tokens->tokens[lp].kind == CDD_TOKEN_KEYWORD_GOTO ||
              (tree->base_tokens->tokens[lp].kind == CDD_TOKEN_IDENTIFIER &&
               ((tree->base_tokens->tokens[lp].length == 8 &&
                 memcmp(tree->base_tokens->tokens[lp].start, "volatile", 8) ==
                     0) ||
                (tree->base_tokens->tokens[lp].length == 12 &&
                 memcmp(tree->base_tokens->tokens[lp].start, "__volatile__",
                        12) == 0))))) {
        lp++;
      }
      if (lp >= tree->base_tokens->size ||
          tree->base_tokens->tokens[lp].kind != CDD_TOKEN_LPAREN) {
        continue;
      }
      rc = gnu_parse_call_args(tree->base_tokens->tokens,
                               tree->base_tokens->size, lp, &rp, commas, &nc,
                               &found);
      if (rc != CDD_C_SUCCESS)
        return rc;
      if (!found || rp <= lp + 1)
        continue;

      {
        char tmpl[256];
        int has_mem_clobber = 0;
        size_t colons[4];
        size_t num_colons = 0;
        size_t k;
        int d = 0;
        size_t end_tok = rp;
        tmpl[0] = '\0';

        if (tree->base_tokens->tokens[lp + 1].kind == CDD_TOKEN_STRING &&
            tree->base_tokens->tokens[lp + 1].length >= 2) {
          size_t sl = tree->base_tokens->tokens[lp + 1].length - 2;
          if (sl >= sizeof(tmpl))
            sl = sizeof(tmpl) - 1;
          memcpy(tmpl, tree->base_tokens->tokens[lp + 1].start + 1, sl);
          tmpl[sl] = '\0';
        }

        for (k = lp + 1; k < rp; k++) {
          cdd_token_t *tk = &tree->base_tokens->tokens[k];
          if (tk->kind == CDD_TOKEN_LPAREN)
            d++;
          else if (tk->kind == CDD_TOKEN_RPAREN && d > 0)
            d--;
          else if (d == 0 && tk->kind == CDD_TOKEN_COLON && num_colons < 4)
            colons[num_colons++] = k;
          if (tk->kind == CDD_TOKEN_STRING && tk->length == 8 &&
              memcmp(tk->start, "\"memory\"", 8) == 0) {
            has_mem_clobber = 1;
          }
        }

        if (tmpl[0] == '\0' && has_mem_clobber) {
          if (is_msvc)
            gctx->builtin_shim_mask |= GNU_SHIM_ASM_BARRIER;
          rc = gnu_replace_token_range(
              tree, i, rp, is_msvc ? "_ReadWriteBarrier()" : "((void)0)");
          if (rc != CDD_C_SUCCESS)
            return rc;
          i = rp;
          continue;
        }
        if (strcmp(tmpl, "pause") == 0 || strcmp(tmpl, "rep; nop") == 0 ||
            strcmp(tmpl, "rep;nop") == 0 || strcmp(tmpl, "yield") == 0) {
          if (is_msvc)
            gctx->builtin_shim_mask |= GNU_SHIM_ASM_PAUSE;
          rc = gnu_replace_token_range(tree, i, rp,
                                       is_msvc ? "_mm_pause()" : "((void)0)");
          if (rc != CDD_C_SUCCESS)
            return rc;
          i = rp;
          continue;
        }
        if (strncmp(tmpl, "rdtsc", 5) == 0 && is_msvc) {
          char out0[64];
          char out1[64];
          char repl[320];
          out0[0] = '\0';
          out1[0] = '\0';
          if (num_colons >= 1) {
            size_t o_end = (num_colons >= 2) ? colons[1] : rp;
            size_t p_idx = 0;
            for (k = colons[0] + 1; k + 2 < o_end; k++) {
              if (tree->base_tokens->tokens[k].kind == CDD_TOKEN_LPAREN) {
                size_t close_p = k + 1;
                while (close_p < o_end &&
                       tree->base_tokens->tokens[close_p].kind !=
                           CDD_TOKEN_RPAREN)
                  close_p++;
                if (close_p > k + 1) {
                  if (p_idx == 0) {
                    rc = gnu_slice_text(tree->base_tokens->tokens, k + 1,
                                        close_p, out0, sizeof(out0));
                    if (rc != CDD_C_SUCCESS)
                      return rc;
                  } else if (p_idx == 1) {
                    rc = gnu_slice_text(tree->base_tokens->tokens, k + 1,
                                        close_p, out1, sizeof(out1));
                    if (rc != CDD_C_SUCCESS)
                      return rc;
                  }
                  p_idx++;
                  k = close_p;
                }
              }
            }
          }
          gctx->builtin_shim_mask |= GNU_SHIM_ASM_RDTSC;
          if (out0[0] != '\0' && out1[0] != '\0') {
            if (rp + 1 < tree->base_tokens->size &&
                tree->base_tokens->tokens[rp + 1].kind == CDD_TOKEN_SEMICOLON)
              end_tok = rp + 1;
            CDD_SNPRINTF(
                repl, sizeof(repl),
                "{ unsigned __int64 __cdd_tsc = __rdtsc(); (%s) = "
                "(unsigned int)__cdd_tsc; (%s) = (unsigned int)(__cdd_tsc >> "
                "32); }",
                out0, out1);
            rc = gnu_replace_token_range(tree, i, end_tok, repl);
            if (rc != CDD_C_SUCCESS)
              return rc;
          } else {
            rc = gnu_replace_token_range(tree, i, rp, "__rdtsc()");
            if (rc != CDD_C_SUCCESS)
              return rc;
          }
          i = end_tok;
          continue;
        }
        if (strstr(tmpl, "cpuid") != NULL && is_msvc) {
          char outs[4][64];
          char leaf[64];
          char repl[512];
          size_t oi = 0;
          memset(outs, 0, sizeof(outs));
          CDD_SNPRINTF(leaf, sizeof(leaf), "0");
          if (num_colons >= 1) {
            size_t o_end = (num_colons >= 2) ? colons[1] : rp;
            for (k = colons[0] + 1; k + 2 < o_end && oi < 4; k++) {
              if (tree->base_tokens->tokens[k].kind == CDD_TOKEN_LPAREN) {
                size_t cp = k + 1;
                while (cp < o_end &&
                       tree->base_tokens->tokens[cp].kind != CDD_TOKEN_RPAREN)
                  cp++;
                if (cp > k + 1) {
                  rc = gnu_slice_text(tree->base_tokens->tokens, k + 1, cp,
                                      outs[oi++], sizeof(outs[0]));
                  if (rc != CDD_C_SUCCESS)
                    return rc;
                  k = cp;
                }
              }
            }
          }
          if (num_colons >= 2) {
            size_t in_end = (num_colons >= 3) ? colons[2] : rp;
            for (k = colons[1] + 1; k + 2 < in_end; k++) {
              if (tree->base_tokens->tokens[k].kind == CDD_TOKEN_LPAREN) {
                size_t cp = k + 1;
                while (cp < in_end &&
                       tree->base_tokens->tokens[cp].kind != CDD_TOKEN_RPAREN)
                  cp++;
                if (cp > k + 1) {
                  rc = gnu_slice_text(tree->base_tokens->tokens, k + 1, cp,
                                      leaf, sizeof(leaf));
                  if (rc != CDD_C_SUCCESS)
                    return rc;
                  break;
                }
              }
            }
          }
          gctx->builtin_shim_mask |= GNU_SHIM_ASM_CPUID;
          if (rp + 1 < tree->base_tokens->size &&
              tree->base_tokens->tokens[rp + 1].kind == CDD_TOKEN_SEMICOLON)
            end_tok = rp + 1;
          if (oi == 4) {
            CDD_SNPRINTF(
                repl, sizeof(repl),
                "{ int __cdd_cpui[4]; __cpuid(__cdd_cpui, (int)(%s)); (%s) = "
                "__cdd_cpui[0]; (%s) = __cdd_cpui[1]; (%s) = __cdd_cpui[2]; "
                "(%s) = __cdd_cpui[3]; }",
                leaf, outs[0], outs[1], outs[2], outs[3]);
          } else {
            CDD_SNPRINTF(
                repl, sizeof(repl),
                "{ int __cdd_cpui[4]; __cpuid(__cdd_cpui, (int)(%s)); }", leaf);
          }
          rc = gnu_replace_token_range(tree, i, end_tok, repl);
          if (rc != CDD_C_SUCCESS)
            return rc;
          i = end_tok;
          continue;
        }
        if (strncmp(tmpl, "bswap", 5) == 0 && num_colons >= 1) {
          char op_var[64];
          int is_64 = (tmpl[5] == 'q');
          op_var[0] = '\0';
          for (k = colons[0] + 1; k + 2 < rp; k++) {
            if (tree->base_tokens->tokens[k].kind == CDD_TOKEN_LPAREN) {
              size_t cp = k + 1;
              while (cp < rp &&
                     tree->base_tokens->tokens[cp].kind != CDD_TOKEN_RPAREN)
                cp++;
              if (cp > k + 1) {
                rc = gnu_slice_text(tree->base_tokens->tokens, k + 1, cp,
                                    op_var, sizeof(op_var));
                if (rc != CDD_C_SUCCESS)
                  return rc;
                break;
              }
            }
          }
          if (op_var[0] != '\0') {
            char repl[192];
            if (gctx) {
              gctx->builtin_shim_mask |=
                  is_64 ? GNU_SHIM_BSWAP64 : GNU_SHIM_BSWAP32;
            }
            CDD_SNPRINTF(repl, sizeof(repl), "(%s) = %s(%s)", op_var,
                         is_64 ? "__cdd_builtin_bswap64"
                               : "__cdd_builtin_bswap32",
                         op_var);
            rc = gnu_replace_token_range(tree, i, rp, repl);
            if (rc != CDD_C_SUCCESS)
              return rc;
            i = rp;
            continue;
          }
        }

        /* Unrecognized inline assembly: guard on MSVC / strict ISO C89 */
        if (is_msvc || is_iso_c89) {
          char pre_buf[64];
          const char *p_pre = NULL;
          if (rp + 1 < tree->base_tokens->size &&
              tree->base_tokens->tokens[rp + 1].kind == CDD_TOKEN_SEMICOLON)
            end_tok = rp + 1;
          CDD_SNPRINTF(pre_buf, sizeof(pre_buf), "\n%s\n%.*s",
                       is_msvc ? "#if !defined(_MSC_VER)"
                               : "#if defined(__GNUC__)",
                       (int)tok->length, (const char *)tok->start);
          rc = cdd_pool_string_safe(tree, pre_buf, &p_pre);
          if (rc != CDD_C_SUCCESS)
            return rc;
          rc = replace_token_with_text(tree, tok, tok->kind, p_pre,
                                       strlen(p_pre));
          if (rc != CDD_C_SUCCESS)
            return rc;
          if (end_tok == rp + 1) {
            rc = replace_token_with_text(tree,
                                         &tree->base_tokens->tokens[end_tok],
                                         CDD_TOKEN_SEMICOLON, ";\n#endif\n", 9);
            if (rc != CDD_C_SUCCESS)
              return rc;
          } else {
            rc = replace_token_with_text(tree, &tree->base_tokens->tokens[rp],
                                         CDD_TOKEN_RPAREN, ")\n#endif\n", 9);
            if (rc != CDD_C_SUCCESS)
              return rc;
          }
          i = end_tok;
        }
      }
    }
  }
  return CDD_C_SUCCESS;
}

/*
 * @brief Lowers GNU `__builtin_*` intrinsics in a CST tree with semantic
 * context and injects portable C89/MSVC inline helper definitions on demand.
 */
cdd_c_error_t
gnu_standardize_builtins_ctx(cdd_cst_tree_t *tree,
                             struct gnu_standardizer_ctx_t *gctx) {
  static const struct {
    const char *src;
    const char *dst;
    unsigned int flag;
  } k_simple_map[] = {
      {"__builtin_clz", "__cdd_builtin_clz", GNU_SHIM_CLZ},
      {"__builtin_clzl", "__cdd_builtin_clzl", GNU_SHIM_CLZ},
      {"__builtin_clzll", "__cdd_builtin_clzll", GNU_SHIM_CLZ},
      {"__builtin_ctz", "__cdd_builtin_ctz", GNU_SHIM_CTZ},
      {"__builtin_ctzl", "__cdd_builtin_ctzl", GNU_SHIM_CTZ},
      {"__builtin_ctzll", "__cdd_builtin_ctzll", GNU_SHIM_CTZ},
      {"__builtin_ffs", "__cdd_builtin_ffs", GNU_SHIM_FFS},
      {"__builtin_ffsl", "__cdd_builtin_ffsl", GNU_SHIM_FFS},
      {"__builtin_ffsll", "__cdd_builtin_ffsll", GNU_SHIM_FFS},
      {"__builtin_popcount", "__cdd_builtin_popcount", GNU_SHIM_POPCOUNT},
      {"__builtin_popcountl", "__cdd_builtin_popcountl", GNU_SHIM_POPCOUNT},
      {"__builtin_popcountll", "__cdd_builtin_popcountll", GNU_SHIM_POPCOUNT},
      {"__builtin_parity", "__cdd_builtin_parity", GNU_SHIM_PARITY},
      {"__builtin_parityl", "__cdd_builtin_parityl", GNU_SHIM_PARITY},
      {"__builtin_parityll", "__cdd_builtin_parityll", GNU_SHIM_PARITY},
      {"__builtin_bswap16", "__cdd_builtin_bswap16", GNU_SHIM_BSWAP16},
      {"__builtin_bswap32", "__cdd_builtin_bswap32", GNU_SHIM_BSWAP32},
      {"__builtin_bswap64", "__cdd_builtin_bswap64", GNU_SHIM_BSWAP64},
      {"__builtin_sadd_overflow", "__cdd_builtin_sadd_overflow",
       GNU_SHIM_OVF_ADD},
      {"__builtin_saddl_overflow", "__cdd_builtin_saddl_overflow",
       GNU_SHIM_OVF_ADD},
      {"__builtin_saddll_overflow", "__cdd_builtin_saddll_overflow",
       GNU_SHIM_OVF_ADD},
      {"__builtin_uadd_overflow", "__cdd_builtin_uadd_overflow",
       GNU_SHIM_OVF_ADD},
      {"__builtin_uaddl_overflow", "__cdd_builtin_uaddl_overflow",
       GNU_SHIM_OVF_ADD},
      {"__builtin_uaddll_overflow", "__cdd_builtin_uaddll_overflow",
       GNU_SHIM_OVF_ADD},
      {"__builtin_ssub_overflow", "__cdd_builtin_ssub_overflow",
       GNU_SHIM_OVF_SUB},
      {"__builtin_ssubl_overflow", "__cdd_builtin_ssubl_overflow",
       GNU_SHIM_OVF_SUB},
      {"__builtin_ssubll_overflow", "__cdd_builtin_ssubll_overflow",
       GNU_SHIM_OVF_SUB},
      {"__builtin_usub_overflow", "__cdd_builtin_usub_overflow",
       GNU_SHIM_OVF_SUB},
      {"__builtin_usubl_overflow", "__cdd_builtin_usubl_overflow",
       GNU_SHIM_OVF_SUB},
      {"__builtin_usubll_overflow", "__cdd_builtin_usubll_overflow",
       GNU_SHIM_OVF_SUB},
      {"__builtin_smul_overflow", "__cdd_builtin_smul_overflow",
       GNU_SHIM_OVF_MUL},
      {"__builtin_smull_overflow", "__cdd_builtin_smull_overflow",
       GNU_SHIM_OVF_MUL},
      {"__builtin_smulll_overflow", "__cdd_builtin_smulll_overflow",
       GNU_SHIM_OVF_MUL},
      {"__builtin_umul_overflow", "__cdd_builtin_umul_overflow",
       GNU_SHIM_OVF_MUL},
      {"__builtin_umull_overflow", "__cdd_builtin_umull_overflow",
       GNU_SHIM_OVF_MUL},
      {"__builtin_umulll_overflow", "__cdd_builtin_umulll_overflow",
       GNU_SHIM_OVF_MUL},
      {"__builtin_memcpy", "memcpy", 0},
      {"__builtin_memset", "memset", 0},
      {"__builtin_memmove", "memmove", 0},
      {"__builtin_memcmp", "memcmp", 0},
      {"__builtin_strlen", "strlen", 0},
      {"__builtin_strcmp", "strcmp", 0},
      {"__builtin_strcpy", "strcpy", 0},
      {"__builtin_strncpy", "strncpy", 0}};
  size_t i;
  int is_msvc = 0;
  unsigned int local_mask = gctx ? gctx->builtin_shim_mask : 0;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  if (!tree || !tree->base_tokens)
    return CDD_C_SUCCESS;
  if (gctx && gctx->config) {
    rc = gnu_is_msvc_target(gctx->config, &is_msvc);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  for (i = 0; i < tree->base_tokens->size; i++) {
    cdd_token_t *tok = &tree->base_tokens->tokens[i];
    size_t m;
    int matched_simple = 0;

    if (tok->kind != CDD_TOKEN_IDENTIFIER || tok->length <= 10 ||
        memcmp(tok->start, "__builtin_", 10) != 0) {
      continue;
    }

    for (m = 0; m < sizeof(k_simple_map) / sizeof(k_simple_map[0]); m++) {
      size_t sl = strlen(k_simple_map[m].src);
      if (tok->length == sl &&
          memcmp(tok->start, k_simple_map[m].src, sl) == 0) {
        local_mask |= k_simple_map[m].flag;
        rc = replace_token_with_text(tree, tok, CDD_TOKEN_IDENTIFIER,
                                     k_simple_map[m].dst,
                                     strlen(k_simple_map[m].dst));
        if (rc != CDD_C_SUCCESS)
          return rc;
        matched_simple = 1;
        break;
      }
    }
    if (matched_simple)
      continue;

    if (tok->length == 16 && memcmp(tok->start, "__builtin_alloca", 16) == 0) {
      const char *afn = is_msvc ? "_alloca" : "alloca";
      rc = replace_token_with_text(tree, tok, CDD_TOKEN_IDENTIFIER, afn,
                                   strlen(afn));
      if (rc != CDD_C_SUCCESS)
        return rc;
      continue;
    }

    if (i + 1 >= tree->base_tokens->size ||
        tree->base_tokens->tokens[i + 1].kind != CDD_TOKEN_LPAREN) {
      continue;
    }

    {
      size_t lp = i + 1;
      size_t rp = 0;
      size_t commas[8];
      size_t nc = 0;
      int found = 0;

      rc = gnu_parse_call_args(tree->base_tokens->tokens,
                               tree->base_tokens->size, lp, &rp, commas, &nc,
                               &found);
      if (rc != CDD_C_SUCCESS)
        return rc;
      if (!found)
        continue;

      /* Generic __builtin_{add,sub,mul}_overflow(a, b, res_ptr) */
      if ((tok->length == 22 &&
           memcmp(tok->start, "__builtin_add_overflow", 22) == 0) ||
          (tok->length == 22 &&
           memcmp(tok->start, "__builtin_sub_overflow", 22) == 0) ||
          (tok->length == 22 &&
           memcmp(tok->start, "__builtin_mul_overflow", 22) == 0)) {
        const char *op = "add";
        unsigned int flag = GNU_SHIM_OVF_ADD;
        int is_uns = 0;
        int is_ll = 0;
        int is_l = 0;
        char target_fn[64];
        const char *pooled_fn = NULL;

        if (memcmp(tok->start + 10, "sub", 3) == 0) {
          op = "sub";
          flag = GNU_SHIM_OVF_SUB;
        } else if (memcmp(tok->start + 10, "mul", 3) == 0) {
          op = "mul";
          flag = GNU_SHIM_OVF_MUL;
        }
        if (nc == 2 && rp > commas[1] + 1) {
          size_t cidx = 0;
          cdd_cst_node_t *owner = NULL;
          char pt[128];
          int is_tn = 0;
          int ev_ok = 0;
          rc = gnu_opt_find_node(tree->root, tok, &cidx, &owner);
          if (rc != CDD_C_SUCCESS)
            return rc;
          if (owner) {
            rc = gnu_opt_eval_type_scoped(
                gctx, owner, &tree->base_tokens->tokens[commas[1] + 1],
                rp - (commas[1] + 1), CDD_CST_TYPE_MODE_RVALUE, pt, sizeof(pt),
                &is_tn, &ev_ok);
            if (rc != CDD_C_SUCCESS)
              return rc;
          }
          if (ev_ok) {
            if (strstr(pt, "unsigned") != NULL || strstr(pt, "uint") != NULL ||
                strstr(pt, "size_t") != NULL) {
              is_uns = 1;
            }
            if (strstr(pt, "long long") != NULL || strstr(pt, "64") != NULL)
              is_ll = 1;
            else if (strstr(pt, "long") != NULL || strstr(pt, "size_t") != NULL)
              is_l = 1;
          }
        }
        local_mask |= flag;
        CDD_SNPRINTF(target_fn, sizeof(target_fn),
                     "__cdd_builtin_%c%s%s_overflow", is_uns ? 'u' : 's', op,
                     is_ll ? "ll" : (is_l ? "l" : ""));
        rc = cdd_pool_string_safe(tree, target_fn, &pooled_fn);
        if (rc != CDD_C_SUCCESS)
          return rc;
        rc = replace_token_with_text(tree, tok, CDD_TOKEN_IDENTIFIER, pooled_fn,
                                     strlen(pooled_fn));
        if (rc != CDD_C_SUCCESS)
          return rc;
        continue;
      }

      if (tok->length == 21 &&
          memcmp(tok->start, "__builtin_unreachable", 21) == 0) {
        rc = gnu_replace_token_range(tree, i, rp,
                                     is_msvc ? "__assume(0)" : "((void)0)");
        if (rc != CDD_C_SUCCESS)
          return rc;
        i = rp;
        continue;
      }

      if (tok->length == 14 && memcmp(tok->start, "__builtin_trap", 14) == 0) {
        if (is_msvc)
          local_mask |= GNU_SHIM_TRAP_MSVC;
        rc = gnu_replace_token_range(tree, i, rp,
                                     is_msvc ? "__debugbreak()" : "abort()");
        if (rc != CDD_C_SUCCESS)
          return rc;
        i = rp;
        continue;
      }

      if (tok->length == 18 &&
          memcmp(tok->start, "__builtin_prefetch", 18) == 0) {
        char addr_buf[256];
        char repl[300];
        size_t a0_end = (nc >= 1) ? commas[0] : rp;
        rc = gnu_slice_text(tree->base_tokens->tokens, lp + 1, a0_end, addr_buf,
                            sizeof(addr_buf));
        if (rc != CDD_C_SUCCESS)
          return rc;
        CDD_SNPRINTF(repl, sizeof(repl), "((void)(%s))",
                     addr_buf[0] != '\0' ? addr_buf : "0");
        rc = gnu_replace_token_range(tree, i, rp, repl);
        if (rc != CDD_C_SUCCESS)
          return rc;
        i = rp;
        continue;
      }

      if (tok->length == 20 &&
          memcmp(tok->start, "__builtin_constant_p", 20) == 0) {
        int is_c = 0;
        long cv = 0;
        rc = gnu_eval_const_arg(tree, gctx, tree->base_tokens->tokens, lp + 1,
                                rp, &is_c, &cv);
        if (rc != CDD_C_SUCCESS)
          return rc;
        rc = gnu_replace_token_range(tree, i, rp, is_c ? "1" : "0");
        if (rc != CDD_C_SUCCESS)
          return rc;
        i = rp;
        continue;
      }

      if (tok->length == 21 &&
          memcmp(tok->start, "__builtin_choose_expr", 21) == 0 && nc == 2) {
        int is_c = 0;
        long cv = 0;
        char e1[256];
        char e2[256];
        char cexpr[128];
        char repl[768];
        rc = gnu_eval_const_arg(tree, gctx, tree->base_tokens->tokens, lp + 1,
                                commas[0], &is_c, &cv);
        if (rc != CDD_C_SUCCESS)
          return rc;
        rc = gnu_slice_text(tree->base_tokens->tokens, commas[0] + 1, commas[1],
                            e1, sizeof(e1));
        if (rc != CDD_C_SUCCESS)
          return rc;
        rc = gnu_slice_text(tree->base_tokens->tokens, commas[1] + 1, rp, e2,
                            sizeof(e2));
        if (rc != CDD_C_SUCCESS)
          return rc;
        if (is_c) {
          CDD_SNPRINTF(repl, sizeof(repl), "(%s)", cv ? e1 : e2);
        } else {
          rc = gnu_slice_text(tree->base_tokens->tokens, lp + 1, commas[0],
                              cexpr, sizeof(cexpr));
          if (rc != CDD_C_SUCCESS)
            return rc;
          CDD_SNPRINTF(repl, sizeof(repl), "((%s) ? (%s) : (%s))", cexpr, e1,
                       e2);
        }
        rc = gnu_replace_token_range(tree, i, rp, repl);
        if (rc != CDD_C_SUCCESS)
          return rc;
        i = rp;
        continue;
      }

      if (tok->length == 28 &&
          memcmp(tok->start, "__builtin_types_compatible_p", 28) == 0 &&
          nc == 1) {
        size_t cidx = 0;
        cdd_cst_node_t *owner = NULL;
        char t1[128];
        char t2[128];
        int tn1 = 0;
        int tn2 = 0;
        int ok1 = 0;
        int ok2 = 0;
        int compat = 0;
        t1[0] = '\0';
        t2[0] = '\0';
        rc = gnu_opt_find_node(tree->root, tok, &cidx, &owner);
        if (rc != CDD_C_SUCCESS)
          return rc;
        if (owner) {
          rc = gnu_opt_eval_type_scoped(
              gctx, owner, &tree->base_tokens->tokens[lp + 1],
              commas[0] - (lp + 1), CDD_CST_TYPE_MODE_UNQUAL, t1, sizeof(t1),
              &tn1, &ok1);
          if (rc != CDD_C_SUCCESS)
            return rc;
          if (ok1) {
            rc = gnu_opt_eval_type_scoped(
                gctx, owner, &tree->base_tokens->tokens[commas[0] + 1],
                rp - (commas[0] + 1), CDD_CST_TYPE_MODE_UNQUAL, t2, sizeof(t2),
                &tn2, &ok2);
            if (rc != CDD_C_SUCCESS)
              return rc;
          }
          if (ok1 && ok2) {
            if (gctx && gctx->sem_env) {
              cdd_cst_symbol_t *s1 = NULL;
              cdd_cst_symbol_t *s2 = NULL;
              rc = gnu_opt_scope_lookup(gctx->sem_env->global_scope, t1,
                                        CDD_CST_SYMBOL_TYPEDEF, &s1);
              if (rc != CDD_C_SUCCESS)
                return rc;
              if (s1 && s1->kind == CDD_CST_SYMBOL_TYPEDEF) {
                CDD_SNPRINTF(t1, sizeof(t1), "%s", s1->type_str);
              }
              rc = gnu_opt_scope_lookup(gctx->sem_env->global_scope, t2,
                                        CDD_CST_SYMBOL_TYPEDEF, &s2);
              if (rc != CDD_C_SUCCESS)
                return rc;
              if (s2 && s2->kind == CDD_CST_SYMBOL_TYPEDEF) {
                CDD_SNPRINTF(t2, sizeof(t2), "%s", s2->type_str);
              }
            }
            if (t1[0] != '\0' && strcmp(t1, t2) == 0)
              compat = 1;
          }
        }
        rc = gnu_replace_token_range(tree, i, rp, compat ? "1" : "0");
        if (rc != CDD_C_SUCCESS)
          return rc;
        i = rp;
        continue;
      }

      if (tok->length == 18 &&
          memcmp(tok->start, "__builtin_offsetof", 18) == 0 && nc == 1) {
        char ty_s[128];
        char mem_s[128];
        char repl[320];
        rc = gnu_slice_text(tree->base_tokens->tokens, lp + 1, commas[0], ty_s,
                            sizeof(ty_s));
        if (rc != CDD_C_SUCCESS)
          return rc;
        rc = gnu_slice_text(tree->base_tokens->tokens, commas[0] + 1, rp, mem_s,
                            sizeof(mem_s));
        if (rc != CDD_C_SUCCESS)
          return rc;
        CDD_SNPRINTF(repl, sizeof(repl), "((size_t)&(((%s *)0)->%s))", ty_s,
                     mem_s);
        rc = gnu_replace_token_range(tree, i, rp, repl);
        if (rc != CDD_C_SUCCESS)
          return rc;
        i = rp;
        continue;
      }
    }
  }

  if (gctx)
    gctx->builtin_shim_mask = local_mask;
  if (local_mask != 0) {
    rc = gnu_inject_builtin_shims(tree, local_mask);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  return CDD_C_SUCCESS;
}

/*
 * @brief Lowers GNU `__builtin_*` intrinsics without external context.
 */
cdd_c_error_t gnu_standardize_builtins(cdd_cst_tree_t *tree) {
  return gnu_standardize_builtins_ctx(tree, NULL);
}
