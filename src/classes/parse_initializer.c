/**
 * @file initializer_parser.c
 * @brief Implementation of the recursive descent initializer parser.
 *
 * @author Samuel Marks
 */

#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "classes/parse_initializer.h"
#include "functions/parse_str.h"

/* --- Helper: Token Joiner --- */

/**
 * @brief Join tokens into a string, skipping whitespace and comments.
 * Matches test expectations for normalization.
 */
static char *join_tokens_skipping_ws(const struct TokenList *tokens,
                                     size_t start, size_t end) {
  size_t len = 0;
  size_t i;
  char *buf, *p;

  /* Calculate required length */
  for (i = start; i < end; ++i) {
    enum TokenKind k = tokens->tokens[i].kind;
    if (k != TOKEN_WHITESPACE && k != TOKEN_COMMENT) {
      len += tokens->tokens[i].length;
    }
  }

  buf = (char *)malloc(len + 1);
  if (!buf)
    return NULL;

  p = buf;
  for (i = start; i < end; ++i) {
    enum TokenKind k = tokens->tokens[i].kind;
    if (k != TOKEN_WHITESPACE && k != TOKEN_COMMENT) {
      const struct Token *t = &tokens->tokens[i];
      memcpy(p, t->start, t->length);
      p += t->length;
    }
  }
  *p = '\0';
  return buf;
}

/* --- List Management --- */

int init_list_init(struct InitList *const list) {
  if (!list)
    return EINVAL;
  list->items = NULL;
  list->count = 0;
  list->capacity = 0;
  return 0;
}

static void init_value_free(struct InitValue *val);

void init_list_free(struct InitList *const list) {
  size_t i;
  if (!list)
    return;
  if (list->items) {
    for (i = 0; i < list->count; ++i) {
      if (list->items[i].designator)
        free(list->items[i].designator);
      if (list->items[i].value) {
        init_value_free(list->items[i].value);
        free(list->items[i].value);
      }
    }
    free(list->items);
    list->items = NULL;
  }
  list->count = 0;
  list->capacity = 0;
}

static void init_value_free(struct InitValue *const val) {
  if (!val)
    return;
  if (val->kind == INIT_KIND_SCALAR && val->data.scalar) {
    free(val->data.scalar);
  } else if (val->kind == INIT_KIND_COMPOUND && val->data.compound) {
    init_list_free(val->data.compound);
    free(val->data.compound);
  }
}

static int init_list_add(struct InitList *const list, char *desig,
                         struct InitValue *val) {
  if (list->count >= list->capacity) {
    size_t new_cap = (list->capacity == 0) ? 4 : list->capacity * 2;
    struct InitItem *new_arr = (struct InitItem *)realloc(
        list->items, new_cap * sizeof(struct InitItem));
    if (!new_arr)
      return ENOMEM;
    list->items = new_arr;
    list->capacity = new_cap;
  }
  list->items[list->count].designator = desig;
  list->items[list->count].value = val;
  list->count++;
  return 0;
}

/* --- Parsing Logic --- */

static size_t skip_ws(const struct TokenList *tokens, size_t idx,
                      size_t limit) {
  while (idx < limit && tokens->tokens[idx].kind == TOKEN_WHITESPACE)
    idx++;
  return idx;
}

static int is_designator_start(enum TokenKind k) {
  return (k == TOKEN_DOT || k == TOKEN_LBRACKET);
}

/**
 * @brief Parse the designator part: `.x`, `[0]`, `.x[1].y`.
 * Ends at `=` token.
 */
static int parse_designator(const struct TokenList *tokens, size_t start,
                            size_t limit, char **out_str, size_t *out_next) {
  size_t i = start;
  size_t end_desig;

  while (i < limit) {
    if (tokens->tokens[i].kind == TOKEN_ASSIGN)
      break;
    /* Boundary check for list delimiters just in case syntax is invalid */
    if (tokens->tokens[i].kind == TOKEN_COMMA ||
        tokens->tokens[i].kind == TOKEN_RBRACE ||
        tokens->tokens[i].kind == TOKEN_SEMICOLON) {
      return EINVAL; /* Designator must end with = */
    }
    i++;
  }

  if (i >= limit || tokens->tokens[i].kind != TOKEN_ASSIGN)
    return EINVAL;

  end_desig = i;     /* Exclusive of = */
  *out_next = i + 1; /* Skip = */

  *out_str = join_tokens_skipping_ws(tokens, start, end_desig);
  if (!*out_str)
    return ENOMEM;

  return 0;
}

/**
 * @brief Parse a single expression (scalar value) until comma or brace.
 * Respects nested parens/blocks.
 */
static int parse_expression_str(const struct TokenList *tokens, size_t start,
                                size_t limit, char **out_str,
                                size_t *out_next) {
  size_t i = start;
  int depth_paren = 0;
  int depth_brace = 0; /* Should be 0 for scalar, but compound literals
                          (type){...} might appear */
  int depth_bracket = 0;

  while (i < limit) {
    enum TokenKind k = tokens->tokens[i].kind;

    if (depth_paren == 0 && depth_brace == 0 && depth_bracket == 0) {
      if (k == TOKEN_COMMA || k == TOKEN_RBRACE)
        break;
    }

    if (k == TOKEN_LPAREN)
      depth_paren++;
    else if (k == TOKEN_RPAREN)
      depth_paren--;
    else if (k == TOKEN_LBRACE)
      depth_brace++;
    else if (k == TOKEN_RBRACE)
      depth_brace--;
    else if (k == TOKEN_LBRACKET)
      depth_bracket++;
    else if (k == TOKEN_RBRACKET)
      depth_bracket--;

    i++;
  }

  if (i == start) {
    /* Empty expression? Valid in some error cases, or just missing val */
    return EINVAL;
  }

  *out_str = join_tokens_skipping_ws(tokens, start, i);
  if (!*out_str)
    return ENOMEM;

  *out_next = i;
  return 0;
}

int parse_initializer(const struct TokenList *tokens, size_t start_idx,
                      size_t end_idx, struct InitList *out, size_t *consumed) {
  size_t i;
  int rc = 0;

  if (!tokens || !out)
    return EINVAL;

  i = skip_ws(tokens, start_idx, end_idx);

  /* Expect opening brace */
  if (i >= end_idx || tokens->tokens[i].kind != TOKEN_LBRACE) {
    return EINVAL;
  }
  i++; /* Consume { */

  while (i < end_idx) {
    char *desig_str = NULL;
    struct InitValue *val_obj = NULL;

    i = skip_ws(tokens, i, end_idx);

    if (i >= end_idx)
      break;

    /* Check end of list */
    if (tokens->tokens[i].kind == TOKEN_RBRACE) {
      i++; /* Consume } */
      if (consumed)
        *consumed = (i - start_idx);
      return 0;
    }

    /* Check designator: starts with . or [ */
    if (is_designator_start(tokens->tokens[i].kind)) {
      size_t next_after_desig;
      rc = parse_designator(tokens, i, end_idx, &desig_str, &next_after_desig);
      if (rc != 0)
        goto error;
      i = next_after_desig;
    }

    i = skip_ws(tokens, i, end_idx);

    /* Allocate Value Object */
    val_obj = (struct InitValue *)calloc(1, sizeof(struct InitValue));
    if (!val_obj) {
      rc = ENOMEM;
      if (desig_str)
        free(desig_str);
      goto error;
    }

    /* Check if Nested Initializer */
    if (tokens->tokens[i].kind == TOKEN_LBRACE) {
      /* Recursive Parse */
      struct InitList *nested_list =
          (struct InitList *)calloc(1, sizeof(struct InitList));
      size_t sub_consumed = 0;

      if (!nested_list) {
        rc = ENOMEM;
        free(val_obj);
        if (desig_str)
          free(desig_str);
        goto error;
      }

      val_obj->kind = INIT_KIND_COMPOUND;
      val_obj->data.compound = nested_list;

      rc = parse_initializer(tokens, i, end_idx, nested_list, &sub_consumed);
      if (rc != 0) {
        free(val_obj);
        init_list_free(nested_list);
        free(nested_list);
        if (desig_str)
          free(desig_str);
        goto error;
      }
      i += sub_consumed;

    } else {
      /* Scalar Expression */
      char *expr_str = NULL;
      size_t next_after_expr;

      rc =
          parse_expression_str(tokens, i, end_idx, &expr_str, &next_after_expr);
      if (rc != 0) {
        free(val_obj);
        if (desig_str)
          free(desig_str);
        goto error;
      }

      val_obj->kind = INIT_KIND_SCALAR;
      val_obj->data.scalar = expr_str;
      i = next_after_expr;
    }

    /* Add to list */
    if ((rc = init_list_add(out, desig_str, val_obj)) != 0) {
      if (desig_str)
        free(desig_str);
      init_value_free(val_obj);
      free(val_obj);
      goto error;
    }

    i = skip_ws(tokens, i, end_idx);

    /* Consume comma if present */
    if (i < end_idx && tokens->tokens[i].kind == TOKEN_COMMA) {
      i++;
    }
  }

  /* Missing closing brace if we loop out */
  rc = EINVAL;

error:
  init_list_free(out);
  return rc;
}
