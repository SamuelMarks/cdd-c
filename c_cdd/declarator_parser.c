/**
 * @file declarator_parser.c
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

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "declarator_parser.h"
#include "str_utils.h"

#ifndef SIZE_MAX
#define SIZE_MAX ((size_t)-1)
#endif

/* --- Helpers --- */

static char *join_tokens_range(const struct TokenList *tokens, size_t start,
                               size_t end) {
  size_t len = 0;
  size_t i;
  char *buf, *p;

  if (start >= end)
    return c_cdd_strdup("");

  for (i = start; i < end; ++i) {
    len += tokens->tokens[i].length;
  }

  buf = (char *)malloc(len + 1);
  if (!buf)
    return NULL;

  p = buf;
  for (i = start; i < end; ++i) {
    const struct Token *t = &tokens->tokens[i];
    memcpy(p, t->start, t->length);
    p += t->length;
  }
  *p = '\0';
  return buf;
}

static size_t skip_ws(const struct TokenList *tokens, size_t i, size_t limit) {
  while (i < limit && (tokens->tokens[i].kind == TOKEN_WHITESPACE ||
                       tokens->tokens[i].kind == TOKEN_COMMENT))
    i++;
  return i;
}

static size_t skip_ws_back(const struct TokenList *tokens, size_t i,
                           size_t limit) {
  if (i <= limit)
    return SIZE_MAX;

  i--;
  while (i > limit && (tokens->tokens[i].kind == TOKEN_WHITESPACE ||
                       tokens->tokens[i].kind == TOKEN_COMMENT))
    i--;

  if (i == limit && (tokens->tokens[i].kind == TOKEN_WHITESPACE ||
                     tokens->tokens[i].kind == TOKEN_COMMENT)) {
    return SIZE_MAX;
  }

  return i;
}

/* --- Scope Skipping --- */

static size_t skip_group(const struct TokenList *tokens, size_t start,
                         size_t limit, enum TokenKind open_k,
                         enum TokenKind close_k) {
  size_t i = start + 1;
  int depth = 1;

  while (i < limit && depth > 0) {
    if (tokens->tokens[i].kind == open_k)
      depth++;
    else if (tokens->tokens[i].kind == close_k)
      depth--;
    i++;
  }

  if (depth == 0)
    return i;
  return limit;
}

/* --- Type Chain Management --- */

void decl_info_init(struct DeclInfo *info) {
  if (info) {
    info->identifier = NULL;
    info->type = NULL;
  }
}

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

void decl_info_free(struct DeclInfo *info) {
  if (!info)
    return;
  if (info->identifier)
    free(info->identifier);
  free_decl_type(info->type);
  info->identifier = NULL;
  info->type = NULL;
}

static int add_type_node(struct DeclInfo *info, struct DeclType **current_tail,
                         struct DeclType *node) {
  if (!node)
    return EINVAL;
  if (!info->type) {
    info->type = node;
  } else {
    (*current_tail)->inner = node;
  }
  *current_tail = node;
  return 0;
}

static struct DeclType *create_node(enum DeclTypeKind kind) {
  struct DeclType *t = (struct DeclType *)calloc(1, sizeof(struct DeclType));
  if (t)
    t->kind = kind;
  return t;
}

/* --- Parse Logic --- */

static int is_grouping_paren(const struct TokenList *tokens, size_t paren_idx,
                             size_t limit) {
  size_t i = skip_ws(tokens, paren_idx + 1, limit);
  if (i >= limit)
    return 0;
  if (tokens->tokens[i].kind == TOKEN_STAR ||
      tokens->tokens[i].kind == TOKEN_CARET ||
      tokens->tokens[i].kind == TOKEN_LBRACKET) {
    return 1;
  }
  if (tokens->tokens[i].kind == TOKEN_LPAREN) {
    return 1;
  }
  return 0;
}

static size_t find_abstract_pivot(const struct TokenList *tokens, size_t start,
                                  size_t end) {
  size_t i = start;
  size_t best_pivot = end;
  int current_depth = 0;
  int best_depth = -1;

  while (i < end) {
    enum TokenKind k = tokens->tokens[i].kind;

    /* Skip aggregrate definitions */
    if (k == TOKEN_KEYWORD_STRUCT || k == TOKEN_KEYWORD_UNION ||
        k == TOKEN_KEYWORD_ENUM) {
      size_t j = skip_ws(tokens, i + 1, end);
      if (j < end && tokens->tokens[j].kind == TOKEN_IDENTIFIER) {
        j = skip_ws(tokens, j + 1, end);
      }
      if (j < end && tokens->tokens[j].kind == TOKEN_LBRACE) {
        i = skip_group(tokens, j, end, TOKEN_LBRACE, TOKEN_RBRACE);
        continue;
      }
    }
    /* Skip parameterized specifiers like typeof() or _Atomic() */
    else if (k == TOKEN_KEYWORD_TYPEOF || k == TOKEN_KEYWORD_ATOMIC) {
      size_t j = skip_ws(tokens, i + 1, end);
      if (j < end && tokens->tokens[j].kind == TOKEN_LPAREN) {
        /* _Atomic(int) ... */
        i = skip_group(tokens, j, end, TOKEN_LPAREN, TOKEN_RPAREN);
        continue;
      }
    }

    /* Abstract Pivot Logic */
    if (k == TOKEN_LPAREN) {
      if (is_grouping_paren(tokens, i, end)) {
        current_depth++;
      } else {
        if (current_depth > best_depth) {
          best_depth = current_depth;
          best_pivot = i;
        }
        i = skip_group(tokens, i, end, TOKEN_LPAREN, TOKEN_RPAREN);
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
      i = skip_group(tokens, i, end, TOKEN_LBRACKET, TOKEN_RBRACKET);
      continue;
    }
    i++;
  }

  if (best_pivot == end && best_depth == -1) {
    return end;
  }
  return best_pivot;
}

/**
 * @brief Find the declared identifier (pivot point).
 */
static size_t find_pivot(const struct TokenList *tokens, size_t start,
                         size_t end, int *is_abstract) {
  size_t i = start;
  size_t best_ident = end;

  /* 1. Try to find an explicit identifier */
  while (i < end) {
    enum TokenKind k = tokens->tokens[i].kind;

    if (k == TOKEN_KEYWORD_STRUCT || k == TOKEN_KEYWORD_UNION ||
        k == TOKEN_KEYWORD_ENUM) {
      size_t j = skip_ws(tokens, i + 1, end);
      if (j < end && tokens->tokens[j].kind == TOKEN_IDENTIFIER) {
        j = skip_ws(tokens, j + 1, end);
      }
      if (j < end && tokens->tokens[j].kind == TOKEN_LBRACE) {
        i = skip_group(tokens, j, end, TOKEN_LBRACE, TOKEN_RBRACE);
        continue;
      }
    } else if (k == TOKEN_KEYWORD_TYPEOF || k == TOKEN_KEYWORD_ATOMIC) {
      size_t j = skip_ws(tokens, i + 1, end);
      if (j < end && tokens->tokens[j].kind == TOKEN_LPAREN) {
        i = skip_group(tokens, j, end, TOKEN_LPAREN, TOKEN_RPAREN);
        continue;
      }
    } else if (k == TOKEN_IDENTIFIER) {
      best_ident = i;
    }
    i++;
  }

  if (best_ident < end) {
    *is_abstract = 0;
    return best_ident;
  }

  /* 2. Abstract Declarator search */
  *is_abstract = 1;
  return find_abstract_pivot(tokens, start, end);
}

int parse_declaration(const struct TokenList *tokens, size_t start, size_t end,
                      struct DeclInfo *out_info) {
  size_t pivot;
  size_t left, right;
  size_t left_limit = start;
  struct DeclType *tail = NULL;
  int is_abstract = 0;
  int rc = 0;

  if (!tokens || !out_info)
    return EINVAL;

  decl_info_init(out_info);

  /* 1. Find Pivot */
  pivot = find_pivot(tokens, start, end, &is_abstract);

  if (!is_abstract) {
    if (pivot >= end)
      return EINVAL;
    out_info->identifier = join_tokens_range(tokens, pivot, pivot + 1);
    if (!out_info->identifier)
      return ENOMEM;
    left = skip_ws_back(tokens, pivot, start);
    right = skip_ws(tokens, pivot + 1, end);
  } else {
    out_info->identifier = NULL; /* Abstract */
    if (pivot > start) {
      left = skip_ws_back(tokens, pivot, start);
    } else {
      left = SIZE_MAX;
    }
    right = skip_ws(tokens, pivot, end);
  }

  /* 2. Spiral Walk */
  while (1) {
    /* Phase Right: Consume Arrays / Functions */
    while (right < end) {
      enum TokenKind k = tokens->tokens[right].kind;

      if (k == TOKEN_LBRACKET) { /* Array [] */
        struct DeclType *node = create_node(DECL_ARRAY);
        size_t close =
            skip_group(tokens, right, end, TOKEN_LBRACKET, TOKEN_RBRACKET);
        if (!node) {
          rc = ENOMEM;
          goto error;
        }

        if (close > right + 1) {
          node->data.array.size_expr =
              join_tokens_range(tokens, right + 1, close - 1);
        } else {
          node->data.array.size_expr = NULL; /* [] */
        }

        if (add_type_node(out_info, &tail, node) != 0) {
          free_decl_type(node);
          rc = ENOMEM;
          goto error;
        }
        right = skip_ws(tokens, close, end);

      } else if (k == TOKEN_LPAREN) { /* Function () */
        struct DeclType *node = create_node(DECL_FUNC);
        size_t close =
            skip_group(tokens, right, end, TOKEN_LPAREN, TOKEN_RPAREN);
        if (!node) {
          rc = ENOMEM;
          goto error;
        }

        if (close > right + 1) {
          node->data.func.args_str =
              join_tokens_range(tokens, right + 1, close - 1);
        }

        if (add_type_node(out_info, &tail, node) != 0) {
          free_decl_type(node);
          rc = ENOMEM;
          goto error;
        }
        right = skip_ws(tokens, close, end);
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
          struct DeclType *node = create_node(DECL_PTR);
          if (!node) {
            rc = ENOMEM;
            goto error;
          }

          if (qual_start < qual_end) {
            node->data.ptr.qualifiers =
                join_tokens_range(tokens, qual_start, qual_end);
          }

          if (add_type_node(out_info, &tail, node) != 0) {
            free_decl_type(node);
            rc = ENOMEM;
            goto error;
          }

          left = skip_ws_back(tokens, left, left_limit);
          qual_end = (left != SIZE_MAX) ? left + 1 : 0;
          qual_start = qual_end;

        } else if (k == TOKEN_KEYWORD_CONST || k == TOKEN_KEYWORD_VOLATILE ||
                   k == TOKEN_KEYWORD_RESTRICT || k == TOKEN_KEYWORD_ATOMIC) {
          qual_start = left;
          left = skip_ws_back(tokens, left, left_limit);
        } else {
          break; /* Not a pointer or qualifier */
        }
      }
    }

    /* Phase Unnest: Handle Grouping Parens */
    if (left != SIZE_MAX && left >= left_limit && left < end && right < end &&
        tokens->tokens[left].kind == TOKEN_LPAREN &&
        tokens->tokens[right].kind == TOKEN_RPAREN) {
      left = skip_ws_back(tokens, left, left_limit);
      right = skip_ws(tokens, right + 1, end);
    } else {
      break; /* Done or stuck */
    }
  }

  /* 3. Base Type */
  {
    struct DeclType *node = create_node(DECL_BASE);
    if (!node) {
      rc = ENOMEM;
      goto error;
    }

    if (left != SIZE_MAX && left < end && left >= left_limit) {
      node->data.base.name = join_tokens_range(tokens, left_limit, left + 1);
    } else {
      node->data.base.name = c_cdd_strdup("int"); /* Implicit or error */
    }

    if (add_type_node(out_info, &tail, node) != 0) {
      free_decl_type(node);
      rc = ENOMEM;
      goto error;
    }
  }

  return 0;

error:
  decl_info_free(out_info);
  return rc;
}
