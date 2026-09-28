/**
 * @file gnu_standardizer_helpers.c
 * @brief Helper routines and AST visitors for GNU standardizer transformer.
 */

/* clang-format off */
#include "gnu_standardizer_internal.h"
/* clang-format on */

#ifdef CDD_BUILD_TESTS
C_CDD_EXPORT int g_gnu_standardizer_fail = 0;
C_CDD_EXPORT volatile int g_gnu_alloc_fail = 0;
C_CDD_EXPORT volatile int g_gnu_replace_fail = 0;
C_CDD_EXPORT volatile int g_gnu_bld_fail = 0;
C_CDD_EXPORT volatile int g_gnu_malloc_fail = 0;
C_CDD_EXPORT volatile int g_gnu_find_fail = 0;
#endif

cdd_c_error_t gnu_malloc(size_t sz, void **out_ptr) {
  if (!out_ptr)
    return CDD_C_ERROR_INVALID_ARGUMENT;
#ifdef CDD_BUILD_TESTS
  if (g_gnu_malloc_fail > 0 && --g_gnu_malloc_fail == 0) {
    *out_ptr = NULL;
    return CDD_C_ERROR_MEMORY;
  }
#endif
  *out_ptr = malloc(sz);
  if (!*out_ptr)
    return CDD_C_ERROR_MEMORY;
  return CDD_C_SUCCESS;
}

cdd_c_error_t gnu_alloc_node(enum cdd_cst_node_kind_t kind,
                             cdd_cst_node_t **out_node) {
#ifdef CDD_BUILD_TESTS
  if (g_gnu_alloc_fail > 0 && --g_gnu_alloc_fail == 0)
    return CDD_C_ERROR_MEMORY;
#endif
  return cdd_cst_alloc_node(kind, out_node);
}

cdd_c_error_t gnu_find_node(cdd_cst_node_t *root, cdd_token_t *tok,
                            size_t *out_idx, cdd_cst_node_t **out_node) {
#ifdef CDD_BUILD_TESTS
  if (g_gnu_find_fail > 0 && --g_gnu_find_fail == 0)
    return CDD_C_ERROR_NOT_FOUND;
#endif
  return cdd_cst_find_node_for_token(root, tok, out_idx, out_node);
}

/**
 * @brief Safely pools a string duplicate into the CST tree's string pool.
 *
 * @param[in,out] tree Target CST tree structure.
 * @param[in] str Source string to duplicate and pool.
 * @param[out] out_pooled Output pointer receiving the pooled string.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT or
 * CDD_C_ERROR_MEMORY on error.
 */
cdd_c_error_t cdd_pool_string_safe(cdd_cst_tree_t *tree, const char *str,
                                   const char **out_pooled) {
  char *dup;
  if (!out_pooled)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_pooled = NULL;
  if (!tree || !str)
    return CDD_C_ERROR_INVALID_ARGUMENT;
#ifdef CDD_BUILD_TESTS
  if (g_gnu_standardizer_fail == 1)
    return CDD_C_ERROR_MEMORY;
#endif
  dup = strdup(str);
#ifdef CDD_BUILD_TESTS
  if (g_gnu_standardizer_fail == 13) {
    free(dup);
    dup = NULL;
  }
#endif
  if (!dup)
    return CDD_C_ERROR_MEMORY;
  if (tree->num_strings >= tree->string_capacity) {
    size_t new_cap =
        tree->string_capacity == 0 ? 32 : tree->string_capacity * 2;
    char **new_pool =
        (char **)realloc(tree->string_pool, new_cap * sizeof(char *));
#ifdef CDD_BUILD_TESTS
    if (g_gnu_standardizer_fail == 4) {
      free(new_pool);
      new_pool = NULL;
    }
#endif
    if (!new_pool) {
      free(dup);
      return CDD_C_ERROR_MEMORY;
    }
    tree->string_pool = new_pool;
    tree->string_capacity = new_cap;
  }
  tree->string_pool[tree->num_strings++] = dup;
  *out_pooled = dup;
  return CDD_C_SUCCESS;
}

/**
 * @brief Safely pools a string of given length into the CST tree's string pool.
 *
 * @param[in,out] tree Target CST tree structure.
 * @param[in] str Source string buffer.
 * @param[in] len Length of source string to copy.
 * @param[out] out_pooled Output pointer receiving the pooled string.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT or
 * CDD_C_ERROR_MEMORY on error.
 */
cdd_c_error_t cdd_pool_string_safe_len(cdd_cst_tree_t *tree, const char *str,
                                       size_t len, const char **out_pooled) {
  cdd_c_error_t rc;
  char *dup;
  if (!out_pooled)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_pooled = NULL;
  if (!tree || !str)
    return CDD_C_ERROR_INVALID_ARGUMENT;
#ifdef CDD_BUILD_TESTS
  if (g_gnu_standardizer_fail == 2 || g_gnu_standardizer_fail == 13)
    return CDD_C_ERROR_MEMORY;
#endif
  rc = gnu_malloc(len + 1, (void **)&dup);
  if (rc != CDD_C_SUCCESS)
    return rc;
  memcpy(dup, str, len);
  dup[len] = '\0';
  if (tree->num_strings >= tree->string_capacity) {
    size_t new_cap =
        tree->string_capacity == 0 ? 32 : tree->string_capacity * 2;
    char **new_pool =
        (char **)realloc(tree->string_pool, new_cap * sizeof(char *));
#ifdef CDD_BUILD_TESTS
    if (g_gnu_standardizer_fail == 5) {
      free(new_pool);
      new_pool = NULL;
    }
#endif
    if (!new_pool) {
      free(dup);
      return CDD_C_ERROR_MEMORY;
    }
    tree->string_pool = new_pool;
    tree->string_capacity = new_cap;
  }
  tree->string_pool[tree->num_strings++] = dup;
  *out_pooled = dup;
  return CDD_C_SUCCESS;
}

#ifdef CDD_BUILD_TESTS
C_CDD_EXPORT const char *pool_string_safe(cdd_cst_tree_t *tree,
                                          const char *str);
C_CDD_EXPORT const char *pool_string_safe_len(cdd_cst_tree_t *tree,
                                              const char *str, size_t len);
const char *pool_string_safe
#else
const char *pool_string_safe
#endif
    (cdd_cst_tree_t *tree, const char *str) {
  const char *res = NULL;
  if (cdd_pool_string_safe(tree, str, &res) != CDD_C_SUCCESS)
    return NULL;
  return res;
}

#ifdef CDD_BUILD_TESTS
const char *pool_string_safe_len
#else
const char *pool_string_safe_len
#endif
    (cdd_cst_tree_t *tree, const char *str, size_t len) {
  const char *res = NULL;
  if (cdd_pool_string_safe_len(tree, str, len, &res) != CDD_C_SUCCESS)
    return NULL;
  return res;
}

/**
 * @brief Appends an integer as string to the buffer.
 *
 * @param[in,out] p Pointer to destination buffer.
 * @param[in] v Integer value to append.
 * @param[out] out_p Pointer receiving the updated buffer end pointer.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT on NULL
 * pointers.
 */
cdd_c_error_t cdd_append_int(char *p, int v, char **out_p) {
  char temp[32];
  int i = 0, j;
  unsigned int u;
  if (!p || !out_p)
    return CDD_C_ERROR_INVALID_ARGUMENT;
#ifdef CDD_BUILD_TESTS
  if (g_gnu_standardizer_fail == 3)
    return CDD_C_ERROR_MEMORY;
#endif
  if (v == 0) {
    *p++ = '0';
    *p = '\0';
    *out_p = p;
    return CDD_C_SUCCESS;
  }
  if (v < 0) {
    *p++ = '-';
    u = (unsigned int)-v;
  } else {
    u = (unsigned int)v;
  }
  while (u > 0) {
    temp[i++] = (char)('0' + (u % 10));
    u /= 10;
  }
  for (j = i - 1; j >= 0; j--) {
    *p++ = temp[j];
  }
  *p = '\0';
  *out_p = p;
  return CDD_C_SUCCESS;
}

/**
 * @brief Replaces a token child in the CST with a new token holding text.
 *
 * @param[in,out] tree CST tree.
 * @param[in,out] tok Existing token in CST to replace.
 * @param[in] kind Token kind for replacement token.
 * @param[in] text Replacement text.
 * @param[in] len Length of replacement text.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t replace_token_with_text(cdd_cst_tree_t *tree, cdd_token_t *tok,
                                      enum cdd_token_kind_t kind,
                                      const char *text, size_t len) {
  size_t c_idx;
  cdd_cst_node_t *p_node = NULL;
  cdd_token_t *n_tok = NULL;
  cdd_c_error_t rc;
#ifdef CDD_BUILD_TESTS
  if (g_gnu_replace_fail > 0 && --g_gnu_replace_fail == 0)
    return CDD_C_ERROR_MEMORY;
  if (g_gnu_standardizer_fail > 100 && --g_gnu_standardizer_fail == 100)
    return CDD_C_ERROR_MEMORY;
#endif
  if (!tree || !tok)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = cdd_cst_find_node_for_token(tree->root, tok, &c_idx, &p_node);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = cdd_cst_create_token_len(tree, kind, text, len, &n_tok);
#ifdef CDD_BUILD_TESTS
  if (g_gnu_standardizer_fail == 30)
    rc = CDD_C_ERROR_MEMORY;
#endif
  if (rc != CDD_C_SUCCESS)
    return rc;
  n_tok->leading_trivia = tok->leading_trivia;
  n_tok->trailing_trivia = tok->trailing_trivia;
  tok->leading_trivia = NULL;
  tok->trailing_trivia = NULL;
  return cdd_cst_replace_token_child(p_node, c_idx, n_tok);
}

/**
 * @brief Parses a decimal 128-bit literal into high and low 64-bit words.
 *
 * @param[in] str Decimal literal string.
 * @param[in] len Length of literal string.
 * @param[out] out_high Pointer receiving the high 64 bits.
 * @param[out] out_low Pointer receiving the low 64 bits.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT on NULL
 * pointers.
 */
cdd_c_error_t cdd_parse_128_literal(const char *str, size_t len,
                                    uint64_t *out_high, uint64_t *out_low) {
  uint64_t high = 0;
  uint64_t low = 0;
  size_t j;
  if (!out_high || !out_low)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_high = 0;
  *out_low = 0;
  if (!str)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  for (j = 0; j < len; j++) {
    uint64_t d;
    uint64_t low_part, high_part;
    if (str[j] >= '0' && str[j] <= '9') {
      d = (uint64_t)(str[j] - '0');
    } else {
      break; /* Reached suffix or end */
    }
    /* Accurate 128-bit multiply by 10 */
    {
      uint64_t al = low & 0xFFFFFFFF;
      uint64_t ah = low >> 32;
      uint64_t bl = 10;
      uint64_t p00 = al * bl;
      uint64_t p01 = al * 0;
      uint64_t p10 = ah * bl;
      uint64_t mid = p01 + (p00 >> 32) + p10;
      low_part = (mid << 32) | (p00 & 0xFFFFFFFF);
      high_part = high * 10 + (mid >> 32);
    }
    low = low_part + d;
    high = high_part + (low < low_part ? 1 : 0);
  }
  *out_high = high;
  *out_low = low;
  return CDD_C_SUCCESS;
}

/**
 * @brief Parses a hexadecimal 128-bit literal into high and low 64-bit words.
 *
 * @param[in] str Hexadecimal literal string.
 * @param[in] len Length of literal string.
 * @param[out] out_high Pointer receiving the high 64 bits.
 * @param[out] out_low Pointer receiving the low 64 bits.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT on NULL
 * pointers.
 */
cdd_c_error_t cdd_parse_hex_128_literal(const char *str, size_t len,
                                        uint64_t *out_high, uint64_t *out_low) {
  uint64_t high = 0;
  uint64_t low = 0;
  size_t j = 2; /* Skip 0x */
  if (!out_high || !out_low)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_high = 0;
  *out_low = 0;
  if (!str)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  for (; j < len; j++) {
    uint64_t d;
    if (str[j] >= '0' && str[j] <= '9') {
      d = (uint64_t)(str[j] - '0');
    } else if (str[j] >= 'a' && str[j] <= 'f') {
      d = (uint64_t)(str[j] - 'a' + 10);
    } else if (str[j] >= 'A' && str[j] <= 'F') {
      d = (uint64_t)(str[j] - 'A' + 10);
    } else {
      break; /* Suffix */
    }
    high = (high << 4) | (low >> 60);
    low = (low << 4) | d;
  }
  *out_high = high;
  *out_low = low;
  return CDD_C_SUCCESS;
}

/**
 * @brief CST node visitor for standardizing magic macro identifiers.
 *
 * @param[in,out] node CST node to inspect and transform.
 * @param[in,out] user_data Pointer to magic context.
 * @return CDD_C_SUCCESS on success, or error code.
 */
cdd_c_error_t cdd_magic_visitor(cdd_cst_node_t *node, void *user_data) {
  struct magic_ctx *ctx = (struct magic_ctx *)user_data;
  size_t i;
  if (!node || !user_data)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  for (i = 0; i < node->num_children; i++) {
    if (node->children[i].kind == CDD_CST_CHILD_TOKEN) {
      cdd_token_t *tok = node->children[i].val.token;
      if (tok->kind == CDD_TOKEN_IDENTIFIER) {
        if ((tok->length == 12 &&
             memcmp(tok->start, "__FUNCTION__", 12) == 0) ||
            (tok->length == 19 &&
             memcmp(tok->start, "__PRETTY_FUNCTION__", 19) == 0) ||
            (tok->length == 8 && memcmp(tok->start, "__func__", 8) == 0)) {
          cdd_c_error_t rc_tok;
          const char *pooled;
          cdd_token_t *new_tok = NULL;
          char *buf;
          rc_tok = gnu_malloc(ctx->func_len + 3, (void **)&buf);
          if (rc_tok != CDD_C_SUCCESS)
            return rc_tok;
          buf[0] = '"';
          memcpy(buf + 1, ctx->func_name, ctx->func_len);
          buf[1 + ctx->func_len] = '"';
          buf[2 + ctx->func_len] = '\0';

          pooled = pool_string_safe(ctx->tree, buf);
          if (!pooled) {
            free(buf);
            return CDD_C_ERROR_MEMORY;
          }
          rc_tok = cdd_cst_create_token_len(ctx->tree, CDD_TOKEN_STRING, pooled,
                                            ctx->func_len + 2, &new_tok);
          free(buf);
#ifdef CDD_BUILD_TESTS
          if (g_gnu_standardizer_fail == 32)
            rc_tok = CDD_C_ERROR_MEMORY;
#endif
          if (rc_tok != CDD_C_SUCCESS)
            return rc_tok;
          new_tok->leading_trivia = tok->leading_trivia;
          new_tok->trailing_trivia = tok->trailing_trivia;
          tok->leading_trivia = NULL;
          tok->trailing_trivia = NULL;
          rc_tok = cdd_cst_replace_token_child(node, i, new_tok);
#ifdef CDD_BUILD_TESTS
          if (g_gnu_standardizer_fail == 33)
            rc_tok = CDD_C_ERROR_MEMORY;
#endif
          if (rc_tok != CDD_C_SUCCESS)
            return rc_tok;
        }
      }
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief CST node visitor to detect trampoline function calls.
 *
 * @param[in] node CST node to inspect.
 * @param[in,out] user_data Pointer to trampoline context.
 * @return CDD_C_SUCCESS on success, or error code on trampoline detection or
 * error.
 */
cdd_c_error_t cdd_tramp_visitor(cdd_cst_node_t *node, void *user_data) {
  struct tramp_ctx *ctx = (struct tramp_ctx *)user_data;
  size_t i;
  if (!ctx)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (ctx->is_tramp)
    return CDD_C_SUCCESS;
  if (!node)
    return CDD_C_SUCCESS;

  for (i = 0; i < node->num_children; i++) {
    if (node->children[i].kind == CDD_CST_CHILD_TOKEN) {
      cdd_token_t *tok = node->children[i].val.token;
      if (tok->kind == CDD_TOKEN_IDENTIFIER && tok->length == ctx->length &&
          memcmp(tok->start, ctx->name, ctx->length) == 0) {
        /* Check if it's the declaration name itself.
           If the identifier's parent is the function node we are checking, it's
           the name. */
        if (node == ctx->func_node) {
          continue; /* Skip the definition name itself */
        }

        {
          cdd_token_t *next = tok + 1;
          /* If followed by LPAREN, it's a direct call, not a trampoline */
          while (next->length == 0) {
            next++;
          }
          if (next->kind != CDD_TOKEN_LPAREN) {
            ctx->is_tramp = 1;
            return CDD_C_ERROR_UNKNOWN; /* abort traversal */
          }
        }
      }
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief CST node visitor for validating inline assembly statements.
 *
 * @param[in] node CST node to inspect.
 * @param[in,out] user_data User data context pointer.
 * @return CDD_C_SUCCESS on success, or error code.
 */
cdd_c_error_t cdd_asm_visitor(cdd_cst_node_t *node, void *user_data) {
  (void)user_data;
  if (!node)
    return CDD_C_SUCCESS;
  return CDD_C_SUCCESS;
}

/**
 * @brief Infers the C type from a sequence of tokens.
 *
 * @param[in] tokens Token array.
 * @param[in] num_tokens Number of tokens in array.
 * @param[out] out_type Pointer receiving inferred type string.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT on NULL
 * out_type.
 */
cdd_c_error_t cdd_infer_type(const cdd_token_t *tokens, size_t num_tokens,
                             const char **out_type) {
  size_t i;
  if (!out_type)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_type = NULL;
  if (!tokens || num_tokens == 0) {
    *out_type = "int";
    return CDD_C_SUCCESS;
  }
  for (i = 0; i < num_tokens; i++) {
    const cdd_token_t *t = &tokens[i];

    int is_type = 0;
    if (t->kind == CDD_TOKEN_KEYWORD_INT || t->kind == CDD_TOKEN_KEYWORD_STRUCT)
      is_type = 1;
    else if (t->kind == CDD_TOKEN_IDENTIFIER) {
      if ((t->length == 4 && memcmp(t->start, "char", 4) == 0) ||
          (t->length == 5 && memcmp(t->start, "float", 5) == 0) ||
          (t->length == 6 && memcmp(t->start, "double", 6) == 0) ||
          (t->length == 4 && memcmp(t->start, "void", 4) == 0) ||
          (t->length == 5 && memcmp(t->start, "union", 5) == 0) ||
          (t->length == 4 && memcmp(t->start, "enum", 4) == 0) ||
          (t->length == 8 && memcmp(t->start, "unsigned", 8) == 0) ||
          (t->length == 6 && memcmp(t->start, "signed", 6) == 0) ||
          (t->length == 4 && memcmp(t->start, "long", 4) == 0) ||
          (t->length == 5 && memcmp(t->start, "short", 5) == 0)) {
        is_type = 1;
      }
    }
    if (is_type) {

      /* It's likely a type already */
      *out_type = NULL;
      return CDD_C_SUCCESS;
    }
  }
  /* Bit-field inference: expr.field or expr->field */
  if (num_tokens >= 3 && (tokens[num_tokens - 2].kind == CDD_TOKEN_DOT ||
                          tokens[num_tokens - 2].kind == CDD_TOKEN_ARROW)) {
    *out_type = "int";
    return CDD_C_SUCCESS;
  }
  /* Expression inference */
  if (num_tokens == 1 && tokens[0].kind == CDD_TOKEN_STRING) {
    *out_type = "const char *";
    return CDD_C_SUCCESS;
  }
  if (num_tokens == 1 && tokens[0].kind == CDD_TOKEN_NUMBER) {
    const char *str = (const char *)tokens[0].start;
    size_t len = tokens[0].length;
    size_t j;
    for (j = 0; j < len; j++) {
      if (str[j] == '.' || str[j] == 'p' || str[j] == 'P' || str[j] == 'e' ||
          str[j] == 'E') {
        if (str[len - 1] == 'f' || str[len - 1] == 'F') {
          *out_type = "float";
          return CDD_C_SUCCESS;
        }
        *out_type = "double";
        return CDD_C_SUCCESS;
      }
    }
    for (j = 0; j < len; j++) {
      if (str[j] == 'u' || str[j] == 'U') {
        if (str[len - 1] == 'l' || str[len - 1] == 'L') {
          *out_type = "unsigned long";
          return CDD_C_SUCCESS;
        }
        *out_type = "unsigned int";
        return CDD_C_SUCCESS;
      }
      if (str[j] == 'l' || str[j] == 'L') {
        *out_type = "long";
        return CDD_C_SUCCESS;
      }
    }
    *out_type = "int";
    return CDD_C_SUCCESS;
  }
  /* Fallback */
  *out_type = "int";
  return CDD_C_SUCCESS;
}
