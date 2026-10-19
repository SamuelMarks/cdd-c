/**
 * @file initializer.c
 * @brief Implementation of the recursive descent initializer parser.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "c_cdd/log.h"
#include "c_cdd/memory.h"
#include "classes/parse/initializer.h"
#include "functions/parse/str.h"
/* clang-format on */

#ifdef CDD_BUILD_TESTS
/** @brief Mock failure counter for skip_ws in initializer parser tests */
C_CDD_EXPORT int g_initializer_fail_skip_ws = 0;
#endif

/* --- Helper: Token Joiner --- */

/**
 * @brief Join tokens into a string, skipping whitespace and comments.
 * Matches test expectations for normalization.
 *
 * @param[in] tokens Token list containing the span.
 * @param[in] start Inclusive start token index.
 * @param[in] end Exclusive end token index.
 * @param[out] _out_val Pointer to receive the allocated joined string.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_MEMORY on OOM.
 */
static cdd_c_error_t join_tokens_skipping_ws(const struct TokenList *tokens,
                                             size_t start, size_t end,
                                             char **_out_val) {
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

  buf = (char *)C_CDD_MALLOC(len + 1);
  if (!buf) {
    *_out_val = NULL;
    return CDD_C_ERROR_MEMORY;
  }

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
  {
    *_out_val = buf;
    return CDD_C_SUCCESS;
  }
}

/* --- List Management --- */

/**
 * @brief Initializes a C initializer list container.
 *
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT if NULL.
 */
cdd_c_error_t init_list_init(struct InitList *list) {
  if (!list)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  list->items = NULL;
  list->count = 0;
  list->capacity = 0;
  return CDD_C_SUCCESS;
}

/**
 * @brief Releases memory owned by an initializer value node.
 *
 * @param[in,out] val Initializer value node to free.
 */
static void init_value_free(struct InitValue *val);

/**
 * @brief Releases all elements in an initializer list container.
 */
void init_list_free(struct InitList *list) {
  size_t i;
  if (!list)
    return;
  if (list->items) {
    for (i = 0; i < list->count; ++i) {
      if (list->items[i].designator)
        C_CDD_FREE(list->items[i].designator);
      if (list->items[i].value) {
        init_value_free(list->items[i].value);
        C_CDD_FREE(list->items[i].value);
      }
    }
    C_CDD_FREE(list->items);
    list->items = NULL;
  }
  list->count = 0;
  list->capacity = 0;
}

/**
 * @brief Releases memory owned by an initializer value node.
 *
 * @param[in,out] val Initializer value node to free.
 */
static void init_value_free(struct InitValue *val) {
  if (val->kind == INIT_KIND_SCALAR) {
    C_CDD_FREE(val->data.scalar);
  } else {
    init_list_free(val->data.compound);
    C_CDD_FREE(val->data.compound);
  }
}

/**
 * @brief Appends an initializer entry to an initializer list.
 *
 * @param[in,out] list Target initializer list.
 * @param[in] desig Optional allocated designator string (ownership
 * transferred).
 * @param[in] val Allocated initializer value node (ownership transferred).
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_MEMORY on OOM.
 */
static cdd_c_error_t init_list_add(struct InitList *list, char *desig,
                                   struct InitValue *val) {
  if (list->count >= list->capacity) {
    size_t new_cap = (list->capacity == 0) ? 4 : list->capacity * 2;
    struct InitItem *new_arr = (struct InitItem *)C_CDD_REALLOC(
        list->items, new_cap * sizeof(struct InitItem));
    if (!new_arr) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      return CDD_C_ERROR_MEMORY;
    }
    list->items = new_arr;
    list->capacity = new_cap;
  }
  list->items[list->count].designator = desig;
  list->items[list->count].value = val;
  list->count++;
  return CDD_C_SUCCESS;
}

/* --- Parsing Logic --- */

/**
 * @brief Advances token index `idx` past any `TOKEN_WHITESPACE` tokens up to
 * `limit`.
 *
 * @param[in] tokens Token list.
 * @param[in] idx Starting token index.
 * @param[in] limit Exclusive upper bound token index.
 * @param[out] _out_val Pointer to receive the advanced token index.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
static cdd_c_error_t skip_ws(const struct TokenList *tokens, size_t idx,
                             size_t limit, size_t *_out_val) {
#ifdef CDD_BUILD_TESTS
  if (g_initializer_fail_skip_ws && --g_initializer_fail_skip_ws == 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;
#endif
  while (idx < limit && tokens->tokens[idx].kind == TOKEN_WHITESPACE)
    idx++;
  {
    *_out_val = idx;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Checks if a token kind starts a C99 designator (`.` or `[`).
 *
 * @param[in] k Token kind to test.
 * @return Non-zero if designator start, 0 otherwise.
 */
static cdd_c_error_t is_designator_start(enum TokenKind k) {
  return (k == TOKEN_DOT || k == TOKEN_LBRACKET);
}

/**
 * @brief Parse the designator part: `.x`, `[0]`, `.x[1].y`.
 * Ends at `=` token.
 *
 * @param[in] tokens Token list.
 * @param[in] start Start token index of the designator.
 * @param[in] limit Exclusive upper bound token index.
 * @param[out] out_str Pointer to receive the normalized designator string.
 * @param[out] out_next Pointer to receive the token index immediately after
 * `=`.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
static cdd_c_error_t parse_designator(const struct TokenList *tokens,
                                      size_t start, size_t limit,
                                      char **out_str, size_t *out_next) {
  size_t i = start;
  size_t end_desig;

  while (i < limit) {
    if (tokens->tokens[i].kind == TOKEN_ASSIGN)
      break;
    /* Boundary check for list delimiters just in case syntax is invalid */
    if (tokens->tokens[i].kind == TOKEN_COMMA ||
        tokens->tokens[i].kind == TOKEN_RBRACE ||
        tokens->tokens[i].kind == TOKEN_SEMICOLON) {
      return CDD_C_ERROR_INVALID_ARGUMENT; /* Designator must end with = */
    }
    i++;
  }

  if (i >= limit)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  end_desig = i;     /* Exclusive of = */
  *out_next = i + 1; /* Skip = */

  *out_str = NULL;
  {
    cdd_c_error_t rc_str =
        join_tokens_skipping_ws(tokens, start, end_desig, out_str);
    if (rc_str != CDD_C_SUCCESS)
      return rc_str;
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Parse a single expression (scalar value) until comma or brace.
 * Respects nested parens/blocks.
 *
 * @param[in] tokens Token list.
 * @param[in] start Start token index of the expression.
 * @param[in] limit Exclusive upper bound token index.
 * @param[out] out_str Pointer to receive the normalized expression string.
 * @param[out] out_next Pointer to receive the token index after the expression.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
static cdd_c_error_t parse_expression_str(const struct TokenList *tokens,
                                          size_t start, size_t limit,
                                          char **out_str, size_t *out_next) {
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
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  *out_str = NULL;
  {
    cdd_c_error_t rc_str = join_tokens_skipping_ws(tokens, start, i, out_str);
    if (rc_str != CDD_C_SUCCESS)
      return rc_str;
  }

  *out_next = i;
  return CDD_C_SUCCESS;
}

/**
 * @brief Parses initializer from the given input.
 *
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t parse_initializer(const struct TokenList *tokens,
                                size_t start_idx, size_t end_idx,
                                struct InitList *out, size_t *consumed) {
  size_t i;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  if (!tokens || !out)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  rc = skip_ws(tokens, start_idx, end_idx, &i);
  if (rc != CDD_C_SUCCESS)
    return rc;

  /* Expect opening brace */
  if (i >= end_idx || tokens->tokens[i].kind != TOKEN_LBRACE) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
  i++; /* Consume { */

  while (i < end_idx) {
    char *desig_str = NULL;
    struct InitValue *val_obj = NULL;

    rc = skip_ws(tokens, i, end_idx, &i);
    if (rc != CDD_C_SUCCESS)
      goto error;

    if (i >= end_idx)
      break;

    /* Check end of list */
    if (tokens->tokens[i].kind == TOKEN_RBRACE) {
      i++; /* Consume } */
      if (consumed)
        *consumed = (i - start_idx);
      return CDD_C_SUCCESS;
    }

    /* Check designator: starts with . or [ */
    if (is_designator_start(tokens->tokens[i].kind)) {
      size_t next_after_desig;
      rc = parse_designator(tokens, i, end_idx, &desig_str, &next_after_desig);
      if (rc != CDD_C_SUCCESS)
        goto error;
      i = next_after_desig;
    }

    rc = skip_ws(tokens, i, end_idx, &i);
    if (rc != CDD_C_SUCCESS) {
      if (desig_str)
        C_CDD_FREE(desig_str);
      goto error;
    }

    /* Allocate Value Object */
    val_obj = (struct InitValue *)C_CDD_CALLOC(1, sizeof(struct InitValue));
    if (!val_obj) {
      rc = CDD_C_ERROR_MEMORY;
      if (desig_str)
        C_CDD_FREE(desig_str);
      goto error;
    }

    /* Check if Nested Initializer */
    if (tokens->tokens[i].kind == TOKEN_LBRACE) {
      /* Recursive Parse */
      struct InitList *nested_list =
          (struct InitList *)C_CDD_CALLOC(1, sizeof(struct InitList));
      size_t sub_consumed = 0;

      if (!nested_list) {
        rc = CDD_C_ERROR_MEMORY;
        C_CDD_FREE(val_obj);
        if (desig_str)
          C_CDD_FREE(desig_str);
        goto error;
      }

      val_obj->kind = INIT_KIND_COMPOUND;
      val_obj->data.compound = nested_list;

      rc = parse_initializer(tokens, i, end_idx, nested_list, &sub_consumed);
      if (rc != CDD_C_SUCCESS) {
        C_CDD_FREE(val_obj);
        init_list_free(nested_list);
        C_CDD_FREE(nested_list);
        if (desig_str)
          C_CDD_FREE(desig_str);
        goto error;
      }
      i += sub_consumed;

    } else {
      /* Scalar Expression */
      char *expr_str = NULL;
      size_t next_after_expr;

      rc =
          parse_expression_str(tokens, i, end_idx, &expr_str, &next_after_expr);
      if (rc != CDD_C_SUCCESS) {
        C_CDD_FREE(val_obj);
        if (desig_str)
          C_CDD_FREE(desig_str);
        goto error;
      }

      val_obj->kind = INIT_KIND_SCALAR;
      val_obj->data.scalar = expr_str;
      i = next_after_expr;
    }

    /* Add to list */
    if ((rc = init_list_add(out, desig_str, val_obj)) != 0) {
      if (desig_str)
        C_CDD_FREE(desig_str);
      init_value_free(val_obj);
      C_CDD_FREE(val_obj);
      goto error;
    }

    rc = skip_ws(tokens, i, end_idx, &i);
    if (rc != CDD_C_SUCCESS)
      goto error;

    /* Consume comma if present */
    if (i < end_idx && tokens->tokens[i].kind == TOKEN_COMMA) {
      i++;
    }
  }

  /* Missing closing brace if we loop out */
  rc = CDD_C_ERROR_INVALID_ARGUMENT;

error:
  init_list_free(out);
  return rc;
}
