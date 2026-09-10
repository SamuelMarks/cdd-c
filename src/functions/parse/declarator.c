/**
 * @file declarator.c
 * @brief Implementation of the Spiral Rule declaration parser.
 *
 * Supports parsing of complex C declarations, including:
 * - Nested function pointers and arrays.
 * - Abstract declarators.
 * - Type qualifiers ('const', 'volatile', etc.) on pointers.
 * - C11/C23 constructs like '_Atomic(type)' and 'typeof'.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "c_cdd/log.h"
#include "c_cdd/memory.h"
#include "functions/parse/declarator.h"
#include "functions/parse/str.h"
/* clang-format on */

#ifdef CDD_BUILD_TESTS
extern C_CDD_EXPORT int g_cdd_fail_skip_ws;
extern C_CDD_EXPORT int g_cdd_fail_skip_ws_back;
extern C_CDD_EXPORT int g_cdd_fail_skip_group;
extern C_CDD_EXPORT int g_cdd_fail_add_type_node;
extern C_CDD_EXPORT int g_cdd_fail_create_node;
extern C_CDD_EXPORT int g_cdd_fail_find_pivot;
extern C_CDD_EXPORT int g_cdd_fail_is_grouping_paren;
#endif

#ifndef SIZE_MAX
/** @brief SIZE_MAX definition */
#define SIZE_MAX ((size_t) - 1)
#endif

C_CDD_EXPORT cdd_c_error_t add_type_node(struct DeclInfo *info,
                                         struct DeclType **tail,
                                         struct DeclType *node);
C_CDD_EXPORT cdd_c_error_t is_grouping_paren(const struct TokenList *tokens,
                                             size_t start, size_t end,
                                             int *out_res);

/* --- Helpers --- */

/**
 * @brief Joins a range of tokens into a single heap-allocated string.
 *
 * @param[in] tokens Token list.
 * @param[in] start Start index (inclusive).
 * @param[in] end End index (exclusive).
 * @param[out] _out_val Pointer to receive allocated string.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_MEMORY on allocation failure.
 */
static cdd_c_error_t join_tokens_range(const struct TokenList *tokens,
                                       size_t start, size_t end,
                                       char **_out_val) {
  size_t len = 0;
  size_t i;
  char *buf;
  char *p;

  if (!tokens || !_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (start >= end) {
    buf = (char *)C_CDD_MALLOC(1);
    if (!buf) {
      *_out_val = NULL;
      return CDD_C_ERROR_MEMORY;
    }
    buf[0] = '\0';
    *_out_val = buf;
    return CDD_C_SUCCESS;
  }

  for (i = start; i < end; ++i) {
    len += tokens->tokens[i].length;
  }

  buf = (char *)C_CDD_MALLOC(len + 1);
  if (!buf) {
    *_out_val = NULL;
    return CDD_C_ERROR_MEMORY;
  }

  p = buf;
  for (i = start; i < end; ++i) {
    const struct Token *t = &tokens->tokens[i];
    memcpy(p, t->start, t->length);
    p += t->length;
  }
  *p = '\0';
  *_out_val = buf;
  return CDD_C_SUCCESS;
}

/**
 * @brief Advances index past whitespace and comments.
 *
 * @param[in] tokens Token list.
 * @param[in] i Current index.
 * @param[in] limit Index limit.
 * @param[out] _out_val Pointer to receive new index.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
static cdd_c_error_t skip_ws(const struct TokenList *tokens, size_t i,
                             size_t limit, size_t *_out_val) {
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_skip_ws && --g_cdd_fail_skip_ws == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  if (!tokens || !_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  while (i < limit && (tokens->tokens[i].kind == TOKEN_WHITESPACE ||
                       tokens->tokens[i].kind == TOKEN_COMMENT))
    i++;
  *_out_val = i;
  return CDD_C_SUCCESS;
}

/**
 * @brief Retreats index backwards past whitespace and comments.
 *
 * @param[in] tokens Token list.
 * @param[in] i Current index.
 * @param[in] limit Lower index limit.
 * @param[out] _out_val Pointer to receive new index or SIZE_MAX.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
static cdd_c_error_t skip_ws_back(const struct TokenList *tokens, size_t i,
                                  size_t limit, size_t *_out_val) {
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_skip_ws_back && --g_cdd_fail_skip_ws_back == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  if (!tokens || !_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (i <= limit) {
    *_out_val = SIZE_MAX;
    return CDD_C_SUCCESS;
  }

  i--;
  while (i > limit && (tokens->tokens[i].kind == TOKEN_WHITESPACE ||
                       tokens->tokens[i].kind == TOKEN_COMMENT))
    i--;

  if (i == limit && (tokens->tokens[i].kind == TOKEN_WHITESPACE ||
                     tokens->tokens[i].kind == TOKEN_COMMENT)) {
    *_out_val = SIZE_MAX;
    return CDD_C_SUCCESS;
  }

  *_out_val = i;
  return CDD_C_SUCCESS;
}

/* --- Scope Skipping --- */

/**
 * @brief Skips a balanced pair of opening and closing tokens.
 *
 * @param[in] tokens Token list.
 * @param[in] start Index of open token.
 * @param[in] limit Search limit.
 * @param[in] open_k Kind of opening token.
 * @param[in] close_k Kind of closing token.
 * @param[out] _out_val Pointer to receive index after closing token or limit.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
static cdd_c_error_t skip_group(const struct TokenList *tokens, size_t start,
                                size_t limit, enum TokenKind open_k,
                                enum TokenKind close_k, size_t *_out_val) {
  size_t i = start + 1;
  int depth = 1;

#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_skip_group && --g_cdd_fail_skip_group == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif

  if (!tokens || !_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  while (i < limit && depth > 0) {
    if (tokens->tokens[i].kind == open_k)
      depth++;
    else if (tokens->tokens[i].kind == close_k)
      depth--;
    i++;
  }

  if (depth == 0) {
    *_out_val = i;
    return CDD_C_SUCCESS;
  }
  *_out_val = limit;
  return CDD_C_SUCCESS;
}

/* --- Type Chain Management --- */

/**
 * @brief Initializes a DeclInfo structure.
 *
 * @param[out] info Pointer to structure to initialize.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT on NULL info.
 */
cdd_c_error_t decl_info_init(struct DeclInfo *info) {
  if (!info)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  info->identifier = NULL;
  info->type = NULL;
  return CDD_C_SUCCESS;
}

/**
 * @brief Frees the memory associated with a DeclType node and its inner chain.
 *
 * @param[in] t Node to free.
 */
static void free_decl_type(struct DeclType *t) {
  if (!t)
    return;
  free_decl_type(t->inner);
  switch (t->kind) {
  case DECL_BASE:
    if (t->data.base.name)
      C_CDD_FREE(t->data.base.name);
    break;
  case DECL_PTR:
    if (t->data.ptr.qualifiers)
      C_CDD_FREE(t->data.ptr.qualifiers);
    break;
  case DECL_ARRAY:
    if (t->data.array.size_expr)
      C_CDD_FREE(t->data.array.size_expr);
    break;
  case DECL_FUNC:
    if (t->data.func.args_str)
      C_CDD_FREE(t->data.func.args_str);
    break;
  }
  C_CDD_FREE(t);
}

/**
 * @brief Frees resources in a DeclInfo structure.
 *
 * @param[in,out] info Structure to free.
 */
void decl_info_free(struct DeclInfo *info) {
  if (!info)
    return;
  if (info->identifier) {
    C_CDD_FREE(info->identifier);
    info->identifier = NULL;
  }
  if (info->type) {
    free_decl_type(info->type);
    info->type = NULL;
  }
}

/**
 * @brief Appends a type node to the end of a type chain.
 *
 * @param[in,out] info Parent DeclInfo structure.
 * @param[in,out] current_tail Pointer to tail pointer.
 * @param[in] node Node to append.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t add_type_node(struct DeclInfo *info,
                                         struct DeclType **current_tail,
                                         struct DeclType *node) {
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_add_type_node && --g_cdd_fail_add_type_node == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  if (!info || !current_tail || !node)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (!info->type) {
    info->type = node;
  } else {
    (*current_tail)->inner = node;
  }
  *current_tail = node;
  return CDD_C_SUCCESS;
}

/**
 * @brief Allocates and initializes a new DeclType node.
 *
 * @param[in] kind Node kind.
 * @param[out] _out_val Pointer to receive allocated node.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_MEMORY on allocation failure.
 */
static cdd_c_error_t create_node(enum DeclTypeKind kind,
                                 struct DeclType **_out_val) {
  struct DeclType *t;

#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_create_node && --g_cdd_fail_create_node == 0) {
    if (_out_val)
      *_out_val = NULL;
    return CDD_C_ERROR_MEMORY;
  }
#endif

  if (!_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  t = (struct DeclType *)C_CDD_CALLOC(1, sizeof(struct DeclType));
  if (!t) {
    *_out_val = NULL;
    return CDD_C_ERROR_MEMORY;
  }
  t->kind = kind;
  *_out_val = t;
  return CDD_C_SUCCESS;
}

/* --- Parse Logic --- */

/**
 * @brief Checks if a parenthesis indicates grouping in a declarator.
 *
 * @param[in] tokens Token list.
 * @param[in] paren_idx Index of opening parenthesis.
 * @param[in] limit Index limit in token list.
 * @param[out] out_is_grouping Pointer to receive 1 if grouping, 0 otherwise.
 * @return CDD_C_SUCCESS on success or error code.
 */
C_CDD_EXPORT cdd_c_error_t is_grouping_paren(const struct TokenList *tokens,
                                             size_t paren_idx, size_t limit,
                                             int *out_is_grouping) {
  size_t i;
  cdd_c_error_t rc;

#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_is_grouping_paren && --g_cdd_fail_is_grouping_paren == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif

  if (!tokens || !out_is_grouping)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_is_grouping = 0;

  rc = skip_ws(tokens, paren_idx + 1, limit, &i);
  if (rc != CDD_C_SUCCESS)
    return rc;

  if (i >= limit)
    return CDD_C_SUCCESS;

  if (tokens->tokens[i].kind == TOKEN_STAR ||
      tokens->tokens[i].kind == TOKEN_CARET ||
      tokens->tokens[i].kind == TOKEN_LBRACKET ||
      tokens->tokens[i].kind == TOKEN_LPAREN) {
    *out_is_grouping = 1;
    return CDD_C_SUCCESS;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Finds the pivot token in an abstract declarator.
 *
 * @param[in] tokens Token list.
 * @param[in] start Start index.
 * @param[in] end End index.
 * @param[out] _out_val Pointer to receive pivot index.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
static cdd_c_error_t find_abstract_pivot(const struct TokenList *tokens,
                                         size_t start, size_t end,
                                         size_t *_out_val) {
  size_t i = start;
  size_t best_pivot = end;
  int current_depth = 0;
  int best_depth = -1;
  cdd_c_error_t rc;

  if (!tokens || !_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  while (i < end) {
    enum TokenKind k = tokens->tokens[i].kind;

    /* Skip aggregate definitions */
    if (k == TOKEN_KEYWORD_STRUCT || k == TOKEN_KEYWORD_UNION ||
        k == TOKEN_KEYWORD_ENUM) {
      size_t j;
      rc = skip_ws(tokens, i + 1, end, &j);
      if (rc != CDD_C_SUCCESS)
        return rc;

      if (j < end && tokens->tokens[j].kind == TOKEN_IDENTIFIER) {
        rc = skip_ws(tokens, j + 1, end, &j);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
      if (j < end && tokens->tokens[j].kind == TOKEN_LBRACE) {
        rc = skip_group(tokens, j, end, TOKEN_LBRACE, TOKEN_RBRACE, &i);
        if (rc != CDD_C_SUCCESS)
          return rc;
        continue;
      }
    } else if (k == TOKEN_KEYWORD_TYPEOF || k == TOKEN_KEYWORD_ATOMIC) {
      size_t j;
      rc = skip_ws(tokens, i + 1, end, &j);
      if (rc != CDD_C_SUCCESS)
        return rc;

      if (j < end && tokens->tokens[j].kind == TOKEN_LPAREN) {
        rc = skip_group(tokens, j, end, TOKEN_LPAREN, TOKEN_RPAREN, &i);
        if (rc != CDD_C_SUCCESS)
          return rc;
        continue;
      }
    }

    /* Abstract Pivot Logic */
    if (k == TOKEN_LPAREN) {
      int is_grouping = 0;
      rc = is_grouping_paren(tokens, i, end, &is_grouping);
      if (rc != CDD_C_SUCCESS)
        return rc;

      if (is_grouping) {
        current_depth++;
      } else {
        if (current_depth > best_depth) {
          best_depth = current_depth;
          best_pivot = i;
        }
        rc = skip_group(tokens, i, end, TOKEN_LPAREN, TOKEN_RPAREN, &i);
        if (rc != CDD_C_SUCCESS)
          return rc;
        continue;
      }
    } else if (k == TOKEN_RPAREN) {
      best_depth = current_depth;
      best_pivot = i;
      current_depth--;
    } else if (k == TOKEN_LBRACKET) {
      best_depth = current_depth;
      best_pivot = i;
      rc = skip_group(tokens, i, end, TOKEN_LBRACKET, TOKEN_RBRACKET, &i);
      if (rc != CDD_C_SUCCESS)
        return rc;
      continue;
    }
    i++;
  }

  if (best_pivot == end) {
    *_out_val = end;
    return CDD_C_SUCCESS;
  }
  *_out_val = best_pivot;
  return CDD_C_SUCCESS;
}

/**
 * @brief Finds the declared identifier (pivot point) in a declaration.
 *
 * @param[in] tokens Token list.
 * @param[in] start Start index.
 * @param[in] end End index.
 * @param[out] is_abstract Pointer to receive 1 if abstract, 0 if concrete.
 * @param[out] _out_val Pointer to receive pivot index.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
static cdd_c_error_t find_pivot(const struct TokenList *tokens, size_t start,
                                size_t end, int *is_abstract,
                                size_t *_out_val) {
  size_t i = start;
  size_t best_ident = end;
  cdd_c_error_t rc;

#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_find_pivot && --g_cdd_fail_find_pivot == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif

  if (!tokens || !is_abstract || !_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  /* 1. Try to find an explicit identifier */
  while (i < end) {
    enum TokenKind k = tokens->tokens[i].kind;

    if (k == TOKEN_KEYWORD_STRUCT || k == TOKEN_KEYWORD_UNION ||
        k == TOKEN_KEYWORD_ENUM) {
      size_t j;
      rc = skip_ws(tokens, i + 1, end, &j);
      if (rc != CDD_C_SUCCESS)
        return rc;

      if (j < end && tokens->tokens[j].kind == TOKEN_IDENTIFIER) {
        rc = skip_ws(tokens, j + 1, end, &j);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
      if (j < end && tokens->tokens[j].kind == TOKEN_LBRACE) {
        rc = skip_group(tokens, j, end, TOKEN_LBRACE, TOKEN_RBRACE, &i);
        if (rc != CDD_C_SUCCESS)
          return rc;
        continue;
      }
    } else if (k == TOKEN_KEYWORD_TYPEOF || k == TOKEN_KEYWORD_ATOMIC) {
      size_t j;
      rc = skip_ws(tokens, i + 1, end, &j);
      if (rc != CDD_C_SUCCESS)
        return rc;

      if (j < end && tokens->tokens[j].kind == TOKEN_LPAREN) {
        rc = skip_group(tokens, j, end, TOKEN_LPAREN, TOKEN_RPAREN, &i);
        if (rc != CDD_C_SUCCESS)
          return rc;
        continue;
      }
    } else if (k == TOKEN_IDENTIFIER) {
      best_ident = i;
    }
    i++;
  }

  if (best_ident < end) {
    *is_abstract = 0;
    *_out_val = best_ident;
    return CDD_C_SUCCESS;
  }

  /* 2. Abstract Declarator search */
  *is_abstract = 1;
  return find_abstract_pivot(tokens, start, end, _out_val);
}

/**
 * @brief Parses a C declaration token range into a DeclInfo structure.
 *
 * @param[in] tokens The full token stream.
 * @param[in] start Start index of the declaration statement.
 * @param[in] end End index (exclusive), typically at semicolon or comma.
 * @param[out] out_info Pointer to destination structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t parse_declaration(const struct TokenList *tokens, size_t start,
                                size_t end, struct DeclInfo *out_info) {
  size_t pivot;
  size_t left;
  size_t right;
  size_t left_limit = start;
  struct DeclType *tail = NULL;
  struct DeclType *node = NULL;
  int is_abstract = 0;
  cdd_c_error_t rc;

  rc = decl_info_init(out_info);
  if (rc != CDD_C_SUCCESS)
    return rc;

  if (!tokens)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  /* 1. Find Pivot */
  rc = find_pivot(tokens, start, end, &is_abstract, &pivot);
  if (rc != CDD_C_SUCCESS)
    goto error;

  if (!is_abstract) {
    rc = join_tokens_range(tokens, pivot, pivot + 1, &out_info->identifier);
    if (rc != CDD_C_SUCCESS)
      goto error;

    rc = skip_ws_back(tokens, pivot, start, &left);
    if (rc != CDD_C_SUCCESS)
      goto error;

    rc = skip_ws(tokens, pivot + 1, end, &right);
    if (rc != CDD_C_SUCCESS)
      goto error;
  } else {
    out_info->identifier = NULL; /* Abstract */
    if (pivot > start) {
      rc = skip_ws_back(tokens, pivot, start, &left);
      if (rc != CDD_C_SUCCESS)
        goto error;
    } else {
      left = SIZE_MAX;
    }
    rc = skip_ws(tokens, pivot, end, &right);
    if (rc != CDD_C_SUCCESS)
      goto error;
  }

  /* 2. Spiral Walk */
  for (;;) {
    /* Phase Right: Consume Arrays / Functions */
    while (right < end) {
      enum TokenKind k = tokens->tokens[right].kind;

      if (k == TOKEN_LBRACKET) { /* Array [] */
        size_t close = 0;

        node = NULL;
        rc = create_node(DECL_ARRAY, &node);
        if (rc != CDD_C_SUCCESS)
          goto error;

        rc = skip_group(tokens, right, end, TOKEN_LBRACKET, TOKEN_RBRACKET,
                        &close);
        if (rc != CDD_C_SUCCESS) {
          goto error;
        }

        if (close > right + 2) {
          rc = join_tokens_range(tokens, right + 1, close - 1,
                                 &node->data.array.size_expr);
          if (rc != CDD_C_SUCCESS) {
            goto error;
          }
        } else {
          node->data.array.size_expr = NULL; /* [] */
        }

        rc = add_type_node(out_info, &tail, node);
        if (rc != CDD_C_SUCCESS) {
          goto error;
        }
        node = NULL;

        rc = skip_ws(tokens, close, end, &right);
        if (rc != CDD_C_SUCCESS)
          goto error;

      } else if (k == TOKEN_LPAREN) { /* Function () */
        size_t close = 0;

        node = NULL;
        rc = create_node(DECL_FUNC, &node);
        if (rc != CDD_C_SUCCESS)
          goto error;

        rc = skip_group(tokens, right, end, TOKEN_LPAREN, TOKEN_RPAREN, &close);
        if (rc != CDD_C_SUCCESS) {
          goto error;
        }

        if (close > right + 2) {
          rc = join_tokens_range(tokens, right + 1, close - 1,
                                 &node->data.func.args_str);
          if (rc != CDD_C_SUCCESS) {
            goto error;
          }
        } else {
          node->data.func.args_str = NULL;
        }

        rc = add_type_node(out_info, &tail, node);
        if (rc != CDD_C_SUCCESS) {
          goto error;
        }
        node = NULL;

        rc = skip_ws(tokens, close, end, &right);
        if (rc != CDD_C_SUCCESS)
          goto error;
      } else {
        break; /* Not a suffix */
      }
    }

    /* Phase Left: Consume Pointers / Qualifiers */
    {
      size_t qual_end = (left != SIZE_MAX) ? left + 1 : 0;
      size_t qual_start = qual_end;

      while (left != SIZE_MAX && left >= left_limit) {
        enum TokenKind k = tokens->tokens[left].kind;

        if (k == TOKEN_STAR) {
          node = NULL;
          rc = create_node(DECL_PTR, &node);
          if (rc != CDD_C_SUCCESS)
            goto error;

          if (qual_start < qual_end) {
            rc = join_tokens_range(tokens, qual_start, qual_end,
                                   &node->data.ptr.qualifiers);
            if (rc != CDD_C_SUCCESS) {
              goto error;
            }
          }

          rc = add_type_node(out_info, &tail, node);
          if (rc != CDD_C_SUCCESS) {
            goto error;
          }
          node = NULL;

          rc = skip_ws_back(tokens, left, left_limit, &left);
          if (rc != CDD_C_SUCCESS)
            goto error;

          qual_end = (left != SIZE_MAX) ? left + 1 : 0;
          qual_start = qual_end;

        } else if (k == TOKEN_KEYWORD_CONST || k == TOKEN_KEYWORD_VOLATILE ||
                   k == TOKEN_KEYWORD_RESTRICT || k == TOKEN_KEYWORD_ATOMIC) {
          qual_start = left;
          rc = skip_ws_back(tokens, left, left_limit, &left);
          if (rc != CDD_C_SUCCESS)
            goto error;
        } else {
          break; /* Not a pointer or qualifier */
        }
      }
    }

    /* Phase Unnest: Handle Grouping Parens */
    if (left != SIZE_MAX && tokens->tokens[left].kind == TOKEN_LPAREN) {
      rc = skip_ws_back(tokens, left, left_limit, &left);
      if (rc != CDD_C_SUCCESS)
        goto error;

      rc = skip_ws(tokens, right + 1, end, &right);
      if (rc != CDD_C_SUCCESS)
        goto error;
    } else {
      break; /* Done or stuck */
    }
  }

  /* 3. Base Type */
  {
    node = NULL;
    rc = create_node(DECL_BASE, &node);
    if (rc != CDD_C_SUCCESS)
      goto error;

    if (left != SIZE_MAX) {
      rc = join_tokens_range(tokens, left_limit, left + 1,
                             &node->data.base.name);
      if (rc != CDD_C_SUCCESS) {
        goto error;
      }
    } else {
      rc = c_cdd_strdup("int", &node->data.base.name);
      if (rc != CDD_C_SUCCESS) {
        goto error;
      }
    }

    rc = add_type_node(out_info, &tail, node);
    if (rc != CDD_C_SUCCESS) {
      goto error;
    }
    node = NULL;
  }

  return CDD_C_SUCCESS;

error:
  if (node)
    free_decl_type(node);
  decl_info_free(out_info);
  return rc;
}

#ifdef CDD_BUILD_TESTS
/**
 * @brief Joins tokens in range for unit testing.
 * @param[in] tokens Token list.
 * @param[in] start Start index.
 * @param[in] end End index.
 * @param[out] out_val Result buffer.
 * @return CDD_C_SUCCESS or error code.
 */
cdd_c_error_t cdd_test_join_tokens_range(const struct TokenList *tokens,
                                         size_t start, size_t end,
                                         char **out_val) {
  return join_tokens_range(tokens, start, end, out_val);
}

/**
 * @brief Skips whitespace for unit testing.
 * @param[in] tokens Token list.
 * @param[in] i Current index.
 * @param[in] limit Limit index.
 * @param[out] out_val Result index.
 * @return CDD_C_SUCCESS or error code.
 */
cdd_c_error_t cdd_test_skip_ws(const struct TokenList *tokens, size_t i,
                               size_t limit, size_t *out_val) {
  return skip_ws(tokens, i, limit, out_val);
}

/**
 * @brief Skips whitespace backwards for unit testing.
 * @param[in] tokens Token list.
 * @param[in] i Current index.
 * @param[in] limit Limit index.
 * @param[out] out_val Result index.
 * @return CDD_C_SUCCESS or error code.
 */
cdd_c_error_t cdd_test_skip_ws_back(const struct TokenList *tokens, size_t i,
                                    size_t limit, size_t *out_val) {
  return skip_ws_back(tokens, i, limit, out_val);
}

/**
 * @brief Skips group for unit testing.
 * @param[in] tokens Token list.
 * @param[in] start Start index.
 * @param[in] limit Limit index.
 * @param[in] open_k Open kind.
 * @param[in] close_k Close kind.
 * @param[out] out_val Result index.
 * @return CDD_C_SUCCESS or error code.
 */
cdd_c_error_t cdd_test_skip_group(const struct TokenList *tokens, size_t start,
                                  size_t limit, enum TokenKind open_k,
                                  enum TokenKind close_k, size_t *out_val) {
  return skip_group(tokens, start, limit, open_k, close_k, out_val);
}

/**
 * @brief Creates a node for unit testing.
 * @param[in] kind Node kind.
 * @param[out] out_val Result node.
 * @return CDD_C_SUCCESS or error code.
 */
cdd_c_error_t cdd_test_create_node(enum DeclTypeKind kind,
                                   struct DeclType **out_val) {
  return create_node(kind, out_val);
}

/**
 * @brief Finds abstract pivot for unit testing.
 * @param[in] tokens Token list.
 * @param[in] start Start index.
 * @param[in] end End index.
 * @param[out] out_val Result index.
 * @return CDD_C_SUCCESS or error code.
 */
cdd_c_error_t cdd_test_find_abstract_pivot(const struct TokenList *tokens,
                                           size_t start, size_t end,
                                           size_t *out_val) {
  return find_abstract_pivot(tokens, start, end, out_val);
}

/**
 * @brief Finds pivot for unit testing.
 * @param[in] tokens Token list.
 * @param[in] start Start index.
 * @param[in] end End index.
 * @param[out] is_abstract Abstract flag.
 * @param[out] out_val Result index.
 * @return CDD_C_SUCCESS or error code.
 */
cdd_c_error_t cdd_test_find_pivot(const struct TokenList *tokens, size_t start,
                                  size_t end, int *is_abstract,
                                  size_t *out_val) {
  return find_pivot(tokens, start, end, is_abstract, out_val);
}

/**
 * @brief Frees a DeclType node for unit testing.
 * @param[in] t Node to free.
 */
void cdd_test_free_decl_type(struct DeclType *t) { free_decl_type(t); }
#endif
