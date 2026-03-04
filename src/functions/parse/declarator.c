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

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "functions/parse/declarator.h"
#include "functions/parse/str.h"

#ifndef SIZE_MAX
/** @brief SIZE_MAX definition */
#define SIZE_MAX ((size_t)-1)
#endif

/* --- Helpers --- */

static int join_tokens_range(const struct TokenList *tokens, size_t start,
                             size_t end, char **_out_val) {
  char *_ast_strdup_0 = NULL;
  size_t len = 0;
  size_t i;
  char *buf, *p;

  if (start >= end) {
    *_out_val = (c_cdd_strdup("", &_ast_strdup_0), _ast_strdup_0);
    return 0;
  }

  for (i = start; i < end; ++i) {
    len += tokens->tokens[i].length;
  }

  buf = (char *)malloc(len + 1);
  if (!buf) {
    *_out_val = NULL;
    return 0;
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
    return 0;
  }
}

static int skip_ws(const struct TokenList *tokens, size_t i, size_t limit,
                   size_t *_out_val) {
  while (i < limit && (tokens->tokens[i].kind == TOKEN_WHITESPACE ||
                       tokens->tokens[i].kind == TOKEN_COMMENT))
    i++;
  {
    *_out_val = i;
    return 0;
  }
}

static int skip_ws_back(const struct TokenList *tokens, size_t i, size_t limit,
                        size_t *_out_val) {
  if (i <= limit) {
    *_out_val = SIZE_MAX;
    return 0;
  }

  i--;
  while (i > limit && (tokens->tokens[i].kind == TOKEN_WHITESPACE ||
                       tokens->tokens[i].kind == TOKEN_COMMENT))
    i--;

  if (i == limit && (tokens->tokens[i].kind == TOKEN_WHITESPACE ||
                     tokens->tokens[i].kind == TOKEN_COMMENT)) {
    {
      *_out_val = SIZE_MAX;
      return 0;
    }
  }

  {
    *_out_val = i;
    return 0;
  }
}

/* --- Scope Skipping --- */

static int skip_group(const struct TokenList *tokens, size_t start,
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
    return 0;
  }
  {
    *_out_val = limit;
    return 0;
  }
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

static int create_node(enum DeclTypeKind kind, struct DeclType **_out_val) {
  struct DeclType *t = (struct DeclType *)calloc(1, sizeof(struct DeclType));
  if (t)
    t->kind = kind;
  {
    *_out_val = t;
    return 0;
  }
}

/* --- Parse Logic --- */

static int is_grouping_paren(const struct TokenList *tokens, size_t paren_idx,
                             size_t limit) {
  size_t _ast_skip_ws_0;
  size_t i =
      (skip_ws(tokens, paren_idx + 1, limit, &_ast_skip_ws_0), _ast_skip_ws_0);
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

static int find_abstract_pivot(const struct TokenList *tokens, size_t start,
                               size_t end, size_t *_out_val) {
  size_t _ast_skip_ws_1;
  size_t _ast_skip_ws_2;
  size_t _ast_skip_group_3;
  size_t _ast_skip_ws_4;
  size_t _ast_skip_group_5;
  size_t _ast_skip_group_6;
  size_t _ast_skip_group_7;
  size_t i = start;
  size_t best_pivot = end;
  int current_depth = 0;
  int best_depth = -1;

  while (i < end) {
    enum TokenKind k = tokens->tokens[i].kind;

    /* Skip aggregrate definitions */
    if (k == TOKEN_KEYWORD_STRUCT || k == TOKEN_KEYWORD_UNION ||
        k == TOKEN_KEYWORD_ENUM) {
      size_t j = (skip_ws(tokens, i + 1, end, &_ast_skip_ws_1), _ast_skip_ws_1);
      if (j < end && tokens->tokens[j].kind == TOKEN_IDENTIFIER) {
        j = (skip_ws(tokens, j + 1, end, &_ast_skip_ws_2), _ast_skip_ws_2);
      }
      if (j < end && tokens->tokens[j].kind == TOKEN_LBRACE) {
        i = (skip_group(tokens, j, end, TOKEN_LBRACE, TOKEN_RBRACE,
                        &_ast_skip_group_3),
             _ast_skip_group_3);
        continue;
      }
    }
    /* Skip parameterized specifiers like typeof() or _Atomic() */
    else if (k == TOKEN_KEYWORD_TYPEOF || k == TOKEN_KEYWORD_ATOMIC) {
      size_t j = (skip_ws(tokens, i + 1, end, &_ast_skip_ws_4), _ast_skip_ws_4);
      if (j < end && tokens->tokens[j].kind == TOKEN_LPAREN) {
        /* _Atomic(int) ... */
        i = (skip_group(tokens, j, end, TOKEN_LPAREN, TOKEN_RPAREN,
                        &_ast_skip_group_5),
             _ast_skip_group_5);
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
        i = (skip_group(tokens, i, end, TOKEN_LPAREN, TOKEN_RPAREN,
                        &_ast_skip_group_6),
             _ast_skip_group_6);
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
      i = (skip_group(tokens, i, end, TOKEN_LBRACKET, TOKEN_RBRACKET,
                      &_ast_skip_group_7),
           _ast_skip_group_7);
      continue;
    }
    i++;
  }

  if (best_pivot == end && best_depth == -1) {
    {
      *_out_val = end;
      return 0;
    }
  }
  {
    *_out_val = best_pivot;
    return 0;
  }
}

/**
 * @brief Find the declared identifier (pivot point).
 */
static int find_pivot(const struct TokenList *tokens, size_t start, size_t end,
                      int *is_abstract, size_t *_out_val) {
  size_t _ast_skip_ws_8;
  size_t _ast_skip_ws_9;
  size_t _ast_skip_group_10;
  size_t _ast_skip_ws_11;
  size_t _ast_skip_group_12;
  size_t _ast_find_abstract_pivot_13;
  size_t i = start;
  size_t best_ident = end;

  /* 1. Try to find an explicit identifier */
  while (i < end) {
    enum TokenKind k = tokens->tokens[i].kind;

    if (k == TOKEN_KEYWORD_STRUCT || k == TOKEN_KEYWORD_UNION ||
        k == TOKEN_KEYWORD_ENUM) {
      size_t j = (skip_ws(tokens, i + 1, end, &_ast_skip_ws_8), _ast_skip_ws_8);
      if (j < end && tokens->tokens[j].kind == TOKEN_IDENTIFIER) {
        j = (skip_ws(tokens, j + 1, end, &_ast_skip_ws_9), _ast_skip_ws_9);
      }
      if (j < end && tokens->tokens[j].kind == TOKEN_LBRACE) {
        i = (skip_group(tokens, j, end, TOKEN_LBRACE, TOKEN_RBRACE,
                        &_ast_skip_group_10),
             _ast_skip_group_10);
        continue;
      }
    } else if (k == TOKEN_KEYWORD_TYPEOF || k == TOKEN_KEYWORD_ATOMIC) {
      size_t j =
          (skip_ws(tokens, i + 1, end, &_ast_skip_ws_11), _ast_skip_ws_11);
      if (j < end && tokens->tokens[j].kind == TOKEN_LPAREN) {
        i = (skip_group(tokens, j, end, TOKEN_LPAREN, TOKEN_RPAREN,
                        &_ast_skip_group_12),
             _ast_skip_group_12);
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
      return 0;
    }
  }

  /* 2. Abstract Declarator search */
  *is_abstract = 1;
  {
    *_out_val =
        (find_abstract_pivot(tokens, start, end, &_ast_find_abstract_pivot_13),
         _ast_find_abstract_pivot_13);
    return 0;
  }
}

int parse_declaration(const struct TokenList *tokens, size_t start, size_t end,
                      struct DeclInfo *out_info) {
  size_t _ast_find_pivot_14;
  char *_ast_join_tokens_range_15;
  size_t _ast_skip_ws_back_16;
  size_t _ast_skip_ws_17;
  size_t _ast_skip_ws_back_18;
  size_t _ast_skip_ws_19;
  struct DeclType *_ast_create_node_20;
  size_t _ast_skip_group_21;
  char *_ast_join_tokens_range_22;
  size_t _ast_skip_ws_23;
  struct DeclType *_ast_create_node_24;
  size_t _ast_skip_group_25;
  char *_ast_join_tokens_range_26;
  size_t _ast_skip_ws_27;
  struct DeclType *_ast_create_node_28;
  char *_ast_join_tokens_range_29;
  size_t _ast_skip_ws_back_30;
  size_t _ast_skip_ws_back_31;
  size_t _ast_skip_ws_back_32;
  size_t _ast_skip_ws_33;
  struct DeclType *_ast_create_node_34;
  char *_ast_join_tokens_range_35;
  char *_ast_strdup_1 = NULL;
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
  pivot = (find_pivot(tokens, start, end, &is_abstract, &_ast_find_pivot_14),
           _ast_find_pivot_14);

  if (!is_abstract) {
    if (pivot >= end)
      return EINVAL;
    out_info->identifier = (join_tokens_range(tokens, pivot, pivot + 1,
                                              &_ast_join_tokens_range_15),
                            _ast_join_tokens_range_15);
    if (!out_info->identifier)
      return ENOMEM;
    left = (skip_ws_back(tokens, pivot, start, &_ast_skip_ws_back_16),
            _ast_skip_ws_back_16);
    right =
        (skip_ws(tokens, pivot + 1, end, &_ast_skip_ws_17), _ast_skip_ws_17);
  } else {
    out_info->identifier = NULL; /* Abstract */
    if (pivot > start) {
      left = (skip_ws_back(tokens, pivot, start, &_ast_skip_ws_back_18),
              _ast_skip_ws_back_18);
    } else {
      left = SIZE_MAX;
    }
    right = (skip_ws(tokens, pivot, end, &_ast_skip_ws_19), _ast_skip_ws_19);
  }

  /* 2. Spiral Walk */
  while (1) {
    /* Phase Right: Consume Arrays / Functions */
    while (right < end) {
      enum TokenKind k = tokens->tokens[right].kind;

      if (k == TOKEN_LBRACKET) { /* Array [] */
        struct DeclType *node = (create_node(DECL_ARRAY, &_ast_create_node_20),
                                 _ast_create_node_20);
        size_t close = (skip_group(tokens, right, end, TOKEN_LBRACKET,
                                   TOKEN_RBRACKET, &_ast_skip_group_21),
                        _ast_skip_group_21);
        if (!node) {
          rc = ENOMEM;
          goto error;
        }

        if (close > right + 1) {
          node->data.array.size_expr =
              (join_tokens_range(tokens, right + 1, close - 1,
                                 &_ast_join_tokens_range_22),
               _ast_join_tokens_range_22);
        } else {
          node->data.array.size_expr = NULL; /* [] */
        }

        if (add_type_node(out_info, &tail, node) != 0) {
          free_decl_type(node);
          rc = ENOMEM;
          goto error;
        }
        right =
            (skip_ws(tokens, close, end, &_ast_skip_ws_23), _ast_skip_ws_23);

      } else if (k == TOKEN_LPAREN) { /* Function () */
        struct DeclType *node =
            (create_node(DECL_FUNC, &_ast_create_node_24), _ast_create_node_24);
        size_t close = (skip_group(tokens, right, end, TOKEN_LPAREN,
                                   TOKEN_RPAREN, &_ast_skip_group_25),
                        _ast_skip_group_25);
        if (!node) {
          rc = ENOMEM;
          goto error;
        }

        if (close > right + 1) {
          node->data.func.args_str =
              (join_tokens_range(tokens, right + 1, close - 1,
                                 &_ast_join_tokens_range_26),
               _ast_join_tokens_range_26);
        }

        if (add_type_node(out_info, &tail, node) != 0) {
          free_decl_type(node);
          rc = ENOMEM;
          goto error;
        }
        right =
            (skip_ws(tokens, close, end, &_ast_skip_ws_27), _ast_skip_ws_27);
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
          struct DeclType *node = (create_node(DECL_PTR, &_ast_create_node_28),
                                   _ast_create_node_28);
          if (!node) {
            rc = ENOMEM;
            goto error;
          }

          if (qual_start < qual_end) {
            node->data.ptr.qualifiers =
                (join_tokens_range(tokens, qual_start, qual_end,
                                   &_ast_join_tokens_range_29),
                 _ast_join_tokens_range_29);
          }

          if (add_type_node(out_info, &tail, node) != 0) {
            free_decl_type(node);
            rc = ENOMEM;
            goto error;
          }

          left = (skip_ws_back(tokens, left, left_limit, &_ast_skip_ws_back_30),
                  _ast_skip_ws_back_30);
          qual_end = (left != SIZE_MAX) ? left + 1 : 0;
          qual_start = qual_end;

        } else if (k == TOKEN_KEYWORD_CONST || k == TOKEN_KEYWORD_VOLATILE ||
                   k == TOKEN_KEYWORD_RESTRICT || k == TOKEN_KEYWORD_ATOMIC) {
          qual_start = left;
          left = (skip_ws_back(tokens, left, left_limit, &_ast_skip_ws_back_31),
                  _ast_skip_ws_back_31);
        } else {
          break; /* Not a pointer or qualifier */
        }
      }
    }

    /* Phase Unnest: Handle Grouping Parens */
    if (left != SIZE_MAX && left >= left_limit && left < end && right < end &&
        tokens->tokens[left].kind == TOKEN_LPAREN &&
        tokens->tokens[right].kind == TOKEN_RPAREN) {
      left = (skip_ws_back(tokens, left, left_limit, &_ast_skip_ws_back_32),
              _ast_skip_ws_back_32);
      right =
          (skip_ws(tokens, right + 1, end, &_ast_skip_ws_33), _ast_skip_ws_33);
    } else {
      break; /* Done or stuck */
    }
  }

  /* 3. Base Type */
  {
    struct DeclType *node =
        (create_node(DECL_BASE, &_ast_create_node_34), _ast_create_node_34);
    if (!node) {
      rc = ENOMEM;
      goto error;
    }

    if (left != SIZE_MAX && left < end && left >= left_limit) {
      node->data.base.name = (join_tokens_range(tokens, left_limit, left + 1,
                                                &_ast_join_tokens_range_35),
                              _ast_join_tokens_range_35);
    } else {
      node->data.base.name = (c_cdd_strdup("int", &_ast_strdup_1),
                              _ast_strdup_1); /* Implicit or error */
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
