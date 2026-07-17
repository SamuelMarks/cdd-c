/**
 * @file declarator.c
 * @brief Implementation of the Spiral Rule declaration parser.
 *
 * Supports parsing of complex C declarations, including:
 * - Nested function pointers and arrays.
 * - Abstract declarators.
 * - Type qualifiers (`const`, `volatile`, etc.) on pointers.
 * - C11/C23 constructs like `_Atomic(type)` and `typeof`.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include "c_cdd_export.h"
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "functions/parse/declarator.h"
#include "functions/parse/str.h"
#include "c_cdd/log.h"
/* clang-format on */

#ifdef CDD_BUILD_TESTS
extern int g_fail_io_after;
extern int g_io_calls;
#undef malloc
#define malloc(sz)                                                             \
  ((g_fail_io_after >= 0 && ++g_io_calls > g_fail_io_after) ? NULL             \
                                                            : (malloc)(sz))
#undef calloc
#define calloc(n, sz)                                                          \
  ((g_fail_io_after >= 0 && ++g_io_calls > g_fail_io_after) ? NULL             \
                                                            : (calloc)(n, sz))
#endif

#ifndef SIZE_MAX
/** @brief SIZE_MAX definition */
#define SIZE_MAX ((size_t) - 1)
#endif

/* --- Helpers --- */

static enum cdd_c_error join_tokens_range(const struct TokenList *tokens,
                                          size_t start, size_t end,
                                          char **_out_val) {
  size_t len = 0;
  size_t i;
  char *buf, *p;

  for (i = start; i < end; ++i) {
    len += tokens->tokens[i].length;
  }

  buf = (char *)malloc(len + 1);
  if (!buf) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }

  p = buf;
  for (i = start; i < end; ++i) {
    const struct Token *t = &tokens->tokens[i];
    memcpy(p, t->start, t->length);
    p += t->length;
  }
  *p = '\0';
  {
    *_out_val = buf;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Executes the skip ws operation.
 */
static enum cdd_c_error skip_ws(const struct TokenList *tokens, size_t i,
                                size_t limit, size_t *_out_val) {
  while (i < limit && (tokens->tokens[i].kind == TOKEN_WHITESPACE ||
                       tokens->tokens[i].kind == TOKEN_COMMENT))
    i++;
  {
    *_out_val = i;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Executes the skip ws back operation.
 */
static enum cdd_c_error skip_ws_back(const struct TokenList *tokens, size_t i,
                                     size_t limit, size_t *_out_val) {
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
    {
      *_out_val = SIZE_MAX;
      return CDD_C_SUCCESS;
    }
  }

  {
    *_out_val = i;
    return CDD_C_SUCCESS;
  }
}

/* --- Scope Skipping --- */

/**
 * @brief Executes the skip group operation.
 */
static enum cdd_c_error skip_group(const struct TokenList *tokens, size_t start,
                                   size_t limit, enum TokenKind open_k,
                                   enum TokenKind close_k, size_t *_out_val) {
  size_t i = start + 1;
  int depth = 1;

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
  {
    *_out_val = limit;
    return CDD_C_SUCCESS;
  }
}

/* --- Type Chain Management --- */

/**
 * @brief Executes the decl info init operation.
 */
enum cdd_c_error decl_info_init(struct DeclInfo *info) {
  if (!info)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  info->identifier = NULL;
  info->type = NULL;
  return CDD_C_SUCCESS;
}

/**
 * @brief Frees the memory associated with decl type.
 */
static void free_decl_type(struct DeclType *t) {
  if (!t)
    return;
  free_decl_type(t->inner);
  switch (t->kind) {
  case DECL_BASE:
    if (t->data.base.name)
      free(t->data.base.name);
    break;
  case DECL_PTR:
    if (t->data.ptr.qualifiers)
      free(t->data.ptr.qualifiers);
    break;
  case DECL_ARRAY:
    if (t->data.array.size_expr)
      free(t->data.array.size_expr);
    break;
  case DECL_FUNC:
    if (t->data.func.args_str)
      free(t->data.func.args_str);
    break;
  }
  free(t);
}

/**
 * @brief Executes the decl info free operation.
 */
void decl_info_free(struct DeclInfo *info) {
  if (!info)
    return;
  if (info->identifier)
    free(info->identifier);
  free_decl_type(info->type);
  info->identifier = NULL;
  info->type = NULL;
}

/**
 * @brief Adds or sets type node.
 */
C_CDD_EXPORT enum cdd_c_error add_type_node(struct DeclInfo *info,
                                            struct DeclType **current_tail,
                                            struct DeclType *node) {
  if (!node)
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
 * @brief Executes the create node operation.
 */
static enum cdd_c_error create_node(enum DeclTypeKind kind,
                                    struct DeclType **_out_val) {
  struct DeclType *t = NULL;
  t = (struct DeclType *)calloc(1, sizeof(struct DeclType));
  if (t)
    t->kind = kind;
  {
    *_out_val = t;
    return CDD_C_SUCCESS;
  }
}

/* --- Parse Logic --- */

C_CDD_EXPORT enum cdd_c_error is_grouping_paren(const struct TokenList *tokens,
                                                size_t paren_idx, size_t limit,
                                                int *out_is_grouping) {
  size_t i;
  if (!out_is_grouping)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_is_grouping = 0;
  skip_ws(tokens, paren_idx + 1, limit, &i);
  if (i >= limit)
    return CDD_C_SUCCESS;
  if (tokens->tokens[i].kind == TOKEN_STAR ||
      tokens->tokens[i].kind == TOKEN_CARET ||
      tokens->tokens[i].kind == TOKEN_LBRACKET) {
    *out_is_grouping = 1;
    return CDD_C_SUCCESS;
  }
  if (tokens->tokens[i].kind == TOKEN_LPAREN) {
    *out_is_grouping = 1;
    return CDD_C_SUCCESS;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Retrieves the abstract pivot.
 */
static enum cdd_c_error find_abstract_pivot(const struct TokenList *tokens,
                                            size_t start, size_t end,
                                            size_t *_out_val) {
  size_t i = start;
  size_t best_pivot = end;
  int current_depth = 0;
  int best_depth = -1;

  while (i < end) {
    enum TokenKind k = tokens->tokens[i].kind;

    /* Skip aggregrate definitions */
    if (k == TOKEN_KEYWORD_STRUCT || k == TOKEN_KEYWORD_UNION ||
        k == TOKEN_KEYWORD_ENUM) {
      size_t j;
      skip_ws(tokens, i + 1, end, &j);
      if (j < end && tokens->tokens[j].kind == TOKEN_IDENTIFIER) {
        skip_ws(tokens, j + 1, end, &j);
      }
      if (j < end && tokens->tokens[j].kind == TOKEN_LBRACE) {
        skip_group(tokens, j, end, TOKEN_LBRACE, TOKEN_RBRACE, &i);
        continue;
      }
    }
    /* Skip parameterized specifiers like typeof() or _Atomic() */
    else if (k == TOKEN_KEYWORD_TYPEOF || k == TOKEN_KEYWORD_ATOMIC) {
      size_t j;
      skip_ws(tokens, i + 1, end, &j);
      if (j < end && tokens->tokens[j].kind == TOKEN_LPAREN) {
        /* _Atomic(int) ... */
        skip_group(tokens, j, end, TOKEN_LPAREN, TOKEN_RPAREN, &i);
        continue;
      }
    }

    /* Abstract Pivot Logic */
    if (k == TOKEN_LPAREN) {
      int is_grouping = 0;
      is_grouping_paren(tokens, i, end, &is_grouping);
      if (is_grouping) {
        current_depth++;
      } else {
        if (current_depth > best_depth) {
          best_depth = current_depth;
          best_pivot = i;
        }
        skip_group(tokens, i, end, TOKEN_LPAREN, TOKEN_RPAREN, &i);
        continue;
      }
    } else if (k == TOKEN_RPAREN) {
      if (current_depth > best_depth) {
        best_depth = current_depth;
        best_pivot = i;
      }
      current_depth--;
    } else if (k == TOKEN_LBRACKET) {
      if (current_depth > best_depth) {
        best_depth = current_depth;
        best_pivot = i;
      }
      skip_group(tokens, i, end, TOKEN_LBRACKET, TOKEN_RBRACKET, &i);
      continue;
    }
    i++;
  }

  if (best_pivot == end && best_depth == -1) {
    {
      *_out_val = end;
      return CDD_C_SUCCESS;
    }
  }
  {
    *_out_val = best_pivot;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Find the declared identifier (pivot point).
 */
static enum cdd_c_error find_pivot(const struct TokenList *tokens, size_t start,
                                   size_t end, int *is_abstract,
                                   size_t *_out_val) {
  size_t i = start;
  size_t best_ident = end;

  /* 1. Try to find an explicit identifier */
  while (i < end) {
    enum TokenKind k = tokens->tokens[i].kind;

    if (k == TOKEN_KEYWORD_STRUCT || k == TOKEN_KEYWORD_UNION ||
        k == TOKEN_KEYWORD_ENUM) {
      size_t j;
      skip_ws(tokens, i + 1, end, &j);
      if (j < end && tokens->tokens[j].kind == TOKEN_IDENTIFIER) {
        skip_ws(tokens, j + 1, end, &j);
      }
      if (j < end && tokens->tokens[j].kind == TOKEN_LBRACE) {
        skip_group(tokens, j, end, TOKEN_LBRACE, TOKEN_RBRACE, &i);
        continue;
      }
    } else if (k == TOKEN_KEYWORD_TYPEOF || k == TOKEN_KEYWORD_ATOMIC) {
      size_t j;
      skip_ws(tokens, i + 1, end, &j);
      if (j < end && tokens->tokens[j].kind == TOKEN_LPAREN) {
        skip_group(tokens, j, end, TOKEN_LPAREN, TOKEN_RPAREN, &i);
        continue;
      }
    } else if (k == TOKEN_IDENTIFIER) {
      best_ident = i;
    }
    i++;
  }

  if (best_ident < end) {
    *is_abstract = 0;
    {
      *_out_val = best_ident;
      return CDD_C_SUCCESS;
    }
  }

  /* 2. Abstract Declarator search */
  *is_abstract = 1;
  {
    find_abstract_pivot(tokens, start, end, _out_val);
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Parses declaration from the given input.
 */
enum cdd_c_error parse_declaration(const struct TokenList *tokens, size_t start,
                                   size_t end, struct DeclInfo *out_info) {
  size_t pivot;
  size_t left, right;
  size_t left_limit = start;
  struct DeclType *tail = NULL;
  int is_abstract = 0;
  int rc = 0;

  if (!tokens || !out_info)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (decl_info_init(out_info) != CDD_C_SUCCESS)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  /* 1. Find Pivot */
  find_pivot(tokens, start, end, &is_abstract, &pivot);

  if (!is_abstract) {
    if (pivot >= end)
      return CDD_C_ERROR_INVALID_ARGUMENT;
    join_tokens_range(tokens, pivot, pivot + 1, &out_info->identifier);
    if (!out_info->identifier) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      return CDD_C_ERROR_MEMORY;
    }
    skip_ws_back(tokens, pivot, start, &left);
    skip_ws(tokens, pivot + 1, end, &right);
  } else {
    out_info->identifier = NULL; /* Abstract */
    if (pivot > start) {
      skip_ws_back(tokens, pivot, start, &left);
    } else {
      left = SIZE_MAX;
    }
    skip_ws(tokens, pivot, end, &right);
  }

  /* 2. Spiral Walk */
  while (1) {
    /* Phase Right: Consume Arrays / Functions */
    while (right < end) {
      enum TokenKind k = tokens->tokens[right].kind;

      if (k == TOKEN_LBRACKET) { /* Array [] */
        struct DeclType *node = NULL;
        size_t close = 0;
        create_node(DECL_ARRAY, &node);
        skip_group(tokens, right, end, TOKEN_LBRACKET, TOKEN_RBRACKET, &close);
        if (!node) {
          rc = CDD_C_ERROR_MEMORY;
          goto error;
        }

        if (close > right + 1) {
          join_tokens_range(tokens, right + 1, close - 1,
                            &node->data.array.size_expr);
        } else {
          node->data.array.size_expr = NULL; /* [] */
        }

        (void)add_type_node(out_info, &tail, node);
        skip_ws(tokens, close, end, &right);

      } else if (k == TOKEN_LPAREN) { /* Function () */
        struct DeclType *node = NULL;
        size_t close = 0;
        create_node(DECL_FUNC, &node);
        skip_group(tokens, right, end, TOKEN_LPAREN, TOKEN_RPAREN, &close);
        if (!node) {
          rc = CDD_C_ERROR_MEMORY;
          goto error;
        }

        if (close > right + 1) {
          join_tokens_range(tokens, right + 1, close - 1,
                            &node->data.func.args_str);
        }

        (void)add_type_node(out_info, &tail, node);
        skip_ws(tokens, close, end, &right);
      } else {
        break; /* Not a suffix */
      }
    }

    /* Phase Left: Consume Pointers / Qualifiers */
    {
      size_t qual_end = (left != SIZE_MAX) ? left + 1 : 0;
      size_t qual_start = qual_end;

      while (left != SIZE_MAX && left >= left_limit && left < end) {
        enum TokenKind k = tokens->tokens[left].kind;

        if (k == TOKEN_STAR) {
          struct DeclType *node = NULL;
          create_node(DECL_PTR, &node);
          if (!node) {
            rc = CDD_C_ERROR_MEMORY;
            goto error;
          }

          if (qual_start < qual_end) {
            join_tokens_range(tokens, qual_start, qual_end,
                              &node->data.ptr.qualifiers);
          }

          (void)add_type_node(out_info, &tail, node);

          skip_ws_back(tokens, left, left_limit, &left);
          qual_end = (left != SIZE_MAX) ? left + 1 : 0;
          qual_start = qual_end;

        } else if (k == TOKEN_KEYWORD_CONST || k == TOKEN_KEYWORD_VOLATILE ||
                   k == TOKEN_KEYWORD_RESTRICT || k == TOKEN_KEYWORD_ATOMIC) {
          qual_start = left;
          skip_ws_back(tokens, left, left_limit, &left);
        } else {
          break; /* Not a pointer or qualifier */
        }
      }
    }

    /* Phase Unnest: Handle Grouping Parens */
    if (left != SIZE_MAX && left >= left_limit && left < end && right < end &&
        tokens->tokens[left].kind == TOKEN_LPAREN) {
      skip_ws_back(tokens, left, left_limit, &left);
      skip_ws(tokens, right + 1, end, &right);
    } else {
      break; /* Done or stuck */
    }
  }

  /* 3. Base Type */
  {
    struct DeclType *node = NULL;
    create_node(DECL_BASE, &node);
    if (!node) {
      rc = CDD_C_ERROR_MEMORY;
      goto error;
    }

    if (left != SIZE_MAX && left < end && left >= left_limit) {
      join_tokens_range(tokens, left_limit, left + 1, &node->data.base.name);
    } else {
      c_cdd_strdup("int", &node->data.base.name); /* Implicit or error */
    }

    (void)add_type_node(out_info, &tail, node);
  }

  return CDD_C_SUCCESS;

error:
  decl_info_free(out_info);
  return rc;
}
