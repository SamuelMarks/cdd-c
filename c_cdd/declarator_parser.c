/**
 * @file declarator_parser.c
 * @brief Implementation of the Spiral Rule declaration parser.
 *
 * @author Samuel Marks
 */

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "declarator_parser.h"
#include "str_utils.h"

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
  if (i == 0)
    return 0;
  i--;
  while (i > limit && (tokens->tokens[i].kind == TOKEN_WHITESPACE ||
                       tokens->tokens[i].kind == TOKEN_COMMENT))
    i--;
  /* Returns index of valid char, or limit if hitting wall */
  /* If current is WS, we went too far? No, loop condition handles it. */
  /* Re-check if valid. */
  if (tokens->tokens[i].kind == TOKEN_WHITESPACE)
    return limit; /* Should not happen if loop correct */
  return i;
}

/* --- Scope Skipping --- */

/**
 * @brief Skip balanced parens/brackets/braces.
 * @return Index AFTER the closing token, or limit if not found.
 */
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
    free(t->data.base.name);
    break;
  case DECL_PTR:
    free(t->data.ptr.qualifiers);
    break;
  case DECL_ARRAY:
    free(t->data.array.size_expr);
    break;
  case DECL_FUNC:
    free(t->data.func.args_str);
    break;
  }
  free(t);
}

void decl_info_free(struct DeclInfo *info) {
  if (!info)
    return;
  free(info->identifier);
  free_decl_type(info->type);
  info->identifier = NULL;
  info->type = NULL;
}

static int add_type_node(struct DeclInfo *info, struct DeclType **current_tail,
                         struct DeclType *node) {
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

/**
 * @brief Find the declared identifier (pivot point).
 * Skips specifiers like `struct { ... }` or `typeof(...)`.
 */
static size_t find_pivot(const struct TokenList *tokens, size_t start,
                         size_t end) {
  size_t i = start;
  size_t best_ident = end;
  /* We want the *last* valid identifier encountered at the *lowest* valid
     nesting level candidate. Actually, in `int *x`, x is last. In `int *x[1]`,
     x is followed by `[`. We iterate left-to-right. */

  while (i < end) {
    enum TokenKind k = tokens->tokens[i].kind;

    if (k == TOKEN_KEYWORD_STRUCT || k == TOKEN_KEYWORD_UNION ||
        k == TOKEN_KEYWORD_ENUM) {
      /* Skip body if present: struct S { ... } */
      size_t j = skip_ws(tokens, i + 1, end);
      /* Optional identifier */
      if (j < end && tokens->tokens[j].kind == TOKEN_IDENTIFIER) {
        j = skip_ws(tokens, j + 1, end);
      }
      if (j < end && tokens->tokens[j].kind == TOKEN_LBRACE) {
        i = skip_group(tokens, j, end, TOKEN_LBRACE, TOKEN_RBRACE);
        continue;
      }
    } else if (k == TOKEN_KEYWORD_TYPEOF) {
      /* Skip typeof(...) */
      size_t j = skip_ws(tokens, i + 1, end);
      if (j < end && tokens->tokens[j].kind == TOKEN_LPAREN) {
        i = skip_group(tokens, j, end, TOKEN_LPAREN, TOKEN_RPAREN);
        continue;
      }
    } else if (k == TOKEN_LPAREN) {
      /* Could be grouping `(*f)` or func args `(int)`.
         We explore deeper. But we can't blindly skip if the ident is inside.
         However, purely finding the ident:
         Ident is usually the only essential identifier that isn't a keyword.
         Robust search: any identifier NOT preceded by `struct/enum/union`
         keyword (unless inside braces) and NOT a keyword itself. */
    } else if (k == TOKEN_IDENTIFIER) {
      /* Potential candidate */
      /* If followed by `(` it might be a function style macro or just function
       * start? */
      best_ident = i;
    }

    i++;
  }
  return best_ident;
}

/* Helper to check if we are blocked by parens */
static int is_blocked_by_paren(const struct TokenList *tokens, size_t idx) {
  return (tokens->tokens[idx].kind == TOKEN_RPAREN);
}

int parse_declaration(const struct TokenList *tokens, size_t start, size_t end,
                      struct DeclInfo *out_info) {
  size_t pivot;
  size_t left, right;
  size_t left_limit = start;
  struct DeclType *tail = NULL;
  int rc = 0;

  if (!tokens || !out_info)
    return EINVAL;

  decl_info_init(out_info);

  /* 1. Find Pivot */
  pivot = find_pivot(tokens, start, end);
  if (pivot >= end) {
    /* No identifier? Abstract Declarator or Error.
       For now, let's assume named declaration.
       If abstract, pivot search requires smarter logic (finding hole).
       Implementation Limit: Named Declarations only. */
    return EINVAL;
  }

  out_info->identifier =
      join_tokens_range(tokens, pivot, pivot + 1); /* Single token ident */
  if (!out_info->identifier)
    return ENOMEM;

  left = skip_ws_back(tokens, pivot, start);
  right = skip_ws(tokens, pivot + 1, end);

  /* 2. Spiral Walk */
  /* While we are bounded by something valid */
  while (1) {
    int moved = 0;

    /* Phase Right: Consume Arrays / Functions */
    /* Must not cross closing paren if we are inside one, UNLESS we matched it
     */
    while (right < end) {
      enum TokenKind k = tokens->tokens[right].kind;

      if (k == TOKEN_LBRACKET) { /* Array [] */
        struct DeclType *node = create_node(DECL_ARRAY);
        size_t close = skip_group(tokens, right, end, TOKEN_LBRACKET,
                                  TOKEN_RBRACKET); /* returns idx after ] */
        if (!node) {
          rc = ENOMEM;
          goto error;
        }
        /* Extract size expr: inside [ ... ] */
        if (close > right + 1) {
          /* range [right+1, close-1] */
          node->data.array.size_expr =
              join_tokens_range(tokens, right + 1, close - 1);
        } else {
          node->data.array.size_expr = NULL; /* [] */
        }

        add_type_node(out_info, &tail, node);
        right = skip_ws(tokens, close, end);
        moved = 1;

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
        add_type_node(out_info, &tail, node);
        right = skip_ws(tokens, close, end);
        moved = 1;

      } else {
        break; /* Not a suffix */
      }
    }

    /* Phase Left: Consume Pointers / Qualifiers */
    while (left >= left_limit &&
           left < end) { /* left could wrap to SIZE_MAX if < 0 */
      enum TokenKind k = tokens->tokens[left].kind;

      if (k == TOKEN_STAR) {
        struct DeclType *node = create_node(DECL_PTR);
        if (!node) {
          rc = ENOMEM;
          goto error;
        }
        /* Check qualifiers to the right of * (between * and previous item)
           Actually, parsing right-to-left, qualifiers for * are on its RIGHT.
           int * const x; -> x is const pointer.
           We are at `*` (left). Previous position was `x` (pivot).
           Qualifiers are between `*` and `x`.
           Wait, our `left` index moves leftwards.
           The logical stream is `* [qualifiers] [inner]`.
           We are consuming `*`. We should have consumed modifiers already?
           Spiral rule: consume left.
           If we see `*`, it applies.
           If we see `const` before `*` (e.g. `int const *`), that applies to
           the BASE, not the pointer.
           If we see `* const`, the `const` is to the right of `*`.
           We scanned identifiers/suffixes to the right already.
           So `const` would have been between the current `left` (*) and the
           key. We need to verify if we skipped qualifiers during the "move
           left" step? Implementation simplification: We look ahead (towards
           right) from `*` up to current boundary? No, easier: handle `const` as
           a token encountered moving left? `int * const x`. Left of x is
           `const`. Left of const is `*`. So we encounter `const` first.
        */
        add_type_node(out_info, &tail, node);
        left = skip_ws_back(tokens, left, left_limit);
        moved = 1;
      } else if (k == TOKEN_KEYWORD_CONST || k == TOKEN_KEYWORD_VOLATILE ||
                 k == TOKEN_KEYWORD_RESTRICT) {
        /* Qualifier. Applies to the NEXT pointer we find going left?
           `int * const p`: p is const pointer.
           Encounter `const`. buffer it.
           Encounter `*`. Attach buffered `const`.
           If we encounter Base Type (`int`) next, the qual applies to base?
           `const int`: applies to int.
           `int const`: applies to int.
           `int * const`: applies to `*`.
           Strategy: Collect qualifiers. If we hit `*`, attach to it.
           If we hit parens/end, push back to base type specifiers.
        */
        /* For this task (Type & Decl parser), exact const correctness on
           pointers is lower prio than structural correctness.
           We skip qualifiers here, or attach to next pointer?
           Let's skip and assume they belong to the pointer we are about to hit.
           Or if we hit base, they belong to base. */
        left = skip_ws_back(tokens, left, left_limit);
        /* moved = 1; -- Just skipping qualifier doesn't count as structure move
         * unless we use it */
      } else {
        break; /* Not a pointer or qualifier */
      }
    }

    /* Phase Unnest: Handle Grouping Parens */
    /* If left is `(` and right is `)`, consume them */
    if (left >= left_limit && left < end && right < end &&
        tokens->tokens[left].kind == TOKEN_LPAREN &&
        tokens->tokens[right].kind == TOKEN_RPAREN) {
      left = skip_ws_back(tokens, left, left_limit);
      right = skip_ws(tokens, right + 1, end);
      moved = 1;
    } else {
      /* Stuck? We are done with declarator. Remaining left is Base Type. */
      break;
    }
  }

  /* 3. Base Type */
  /* Everything from left_limit to left (inclusive) is the base type specifier
   */
  {
    struct DeclType *node = create_node(DECL_BASE);
    if (!node) {
      rc = ENOMEM;
      goto error;
    }
    /* `left` is now at the last token of the specifier chain (e.g. `int`) */
    /* Range is [left_limit, left + 1) */
    /* Be careful with indices. If left < left_limit (wrapped), range is empty?
     */
    if (left < end && left >= left_limit) { /* Signed check logic */
      node->data.base.name = join_tokens_range(tokens, left_limit, left + 1);
    } else {
      /* This happens if `start` was the pivot (empty base? e.g. inferred int?)
       */
      /* Or implicit int or error */
      node->data.base.name = c_cdd_strdup("int"); /* Fallback/Implicit */
    }
    add_type_node(out_info, &tail, node);
  }

  return 0;

error:
  decl_info_free(out_info);
  return rc;
}
