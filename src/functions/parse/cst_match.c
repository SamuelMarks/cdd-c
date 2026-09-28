/**
 * @file cst_match.c
 * @brief Token matching and scanning routines for CST parsing.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "c_cdd/log.h"
#include "c_cdd/memory.h"
#include "c_cdd_export.h"
#include "functions/parse/cst.h"
#include "functions/parse/cst_match.h"
/* clang-format on */

#ifdef CDD_BUILD_TESTS
extern C_CDD_EXPORT int g_cdd_fail_alloc;
extern C_CDD_EXPORT int g_cdd_fail_skip_ws;
extern C_CDD_EXPORT int g_cdd_fail_skip_ws_back;
extern C_CDD_EXPORT int g_cdd_fail_is_type_start;
extern C_CDD_EXPORT int g_cdd_fail_consume_balanced_parens;
extern C_CDD_EXPORT int g_cdd_fail_consume_attributes;
extern C_CDD_EXPORT int g_cdd_fail_consume_static_assert;
extern C_CDD_EXPORT int g_cdd_fail_consume_generic_selection;
extern C_CDD_EXPORT int g_cdd_fail_is_expression_brace;
extern C_CDD_EXPORT int g_cdd_fail_consume_balanced_braces;
extern C_CDD_EXPORT int g_cdd_fail_match_function_definition;
extern C_CDD_EXPORT int g_cdd_fail_cst_list_add;
extern C_CDD_EXPORT int g_cdd_fail_token_matches_string;
#endif

/**
 * @brief Helper to skip whitespace tokens.
 *
 * @param[in] tokens Token list.
 * @param[in] i Current token index.
 * @param[in] limit Index limit.
 * @param[out] _out_val Index of next non-whitespace token.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t skip_ws(const struct TokenList *tokens, size_t i, size_t limit,
                      size_t *_out_val) {
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_skip_ws && --g_cdd_fail_skip_ws == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  if (!tokens || !_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  while (i < limit && tokens->tokens[i].kind == TOKEN_WHITESPACE)
    i++;
  *_out_val = i;
  return CDD_C_SUCCESS;
}

/**
 * @brief Helper to skip whitespace tokens backwards.
 *
 * @param[in] tokens Token list.
 * @param[in] i Current token index.
 * @param[out] _out_val Index of previous non-whitespace token.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t skip_ws_back(const struct TokenList *tokens, size_t i,
                           size_t *_out_val) {
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_skip_ws_back && --g_cdd_fail_skip_ws_back == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  if (!tokens || !_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (i == 0) {
    *_out_val = 0;
    return CDD_C_SUCCESS;
  }
  i--;
  while (i > 0 && tokens->tokens[i].kind == TOKEN_WHITESPACE)
    i--;

  *_out_val = i;
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks if a token is a valid start of a type specifier.
 *
 * @param[in] tok The token to inspect.
 * @param[out] out_is_type Pointer to int set to 1 if type start, 0 otherwise.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t is_type_start(const struct Token *tok, int *out_is_type) {
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_is_type_start && --g_cdd_fail_is_type_start == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  if (!tok || !out_is_type)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  *out_is_type = 0;

  if (tok->kind == TOKEN_IDENTIFIER) {
    *out_is_type = 1;
    return CDD_C_SUCCESS;
  }
  switch (tok->kind) {
  case TOKEN_KEYWORD_VOID:
  case TOKEN_KEYWORD_CHAR:
  case TOKEN_KEYWORD_INT:
  case TOKEN_KEYWORD_FLOAT:
  case TOKEN_KEYWORD_DOUBLE:
  case TOKEN_KEYWORD_LONG:
  case TOKEN_KEYWORD_SHORT:
  case TOKEN_KEYWORD_SIGNED:
  case TOKEN_KEYWORD_UNSIGNED:
  case TOKEN_KEYWORD_STRUCT:
  case TOKEN_KEYWORD_ENUM:
  case TOKEN_KEYWORD_UNION:
  case TOKEN_KEYWORD_STATIC:
  case TOKEN_KEYWORD_INLINE:
  case TOKEN_KEYWORD_EXTERN:
  case TOKEN_KEYWORD_CONST:
  case TOKEN_KEYWORD_VOLATILE:
  case TOKEN_KEYWORD_AUTO:
  case TOKEN_KEYWORD_REGISTER:
  case TOKEN_KEYWORD_BOOL:
    *out_is_type = 1;
    return CDD_C_SUCCESS;
  default:
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Heuristic to detect function definitions.
 *
 * @param[in] tokens Token list.
 * @param[in] start_idx Start token index.
 * @param[in] limit Index limit.
 * @param[out] end_idx_out Pointer to receive end token index.
 * @param[out] out_is_match Pointer to receive 1 if match, 0 otherwise.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t match_function_definition(const struct TokenList *tokens,
                                        size_t start_idx, size_t limit,
                                        size_t *end_idx_out,
                                        int *out_is_match) {
  size_t _ast_skip_ws_0 = 0;
  size_t k = start_idx;
  int paren_depth;
  int brace_depth;
  int seen_lparen = 0;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_match_function_definition &&
      --g_cdd_fail_match_function_definition == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  if (!tokens || !end_idx_out || !out_is_match)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  *out_is_match = 0;

  while (k < limit) {
    int is_type = 0;
    const enum TokenKind kind = tokens->tokens[k].kind;

    if (kind == TOKEN_SEMICOLON || kind == TOKEN_LBRACE ||
        kind == TOKEN_RBRACE) {
      return CDD_C_SUCCESS;
    }

    if (kind == TOKEN_ASSIGN || kind == TOKEN_NUMBER_LITERAL) {
      return CDD_C_SUCCESS;
    }

    {
      cdd_c_error_t rc_cst = is_type_start(&tokens->tokens[k], &is_type);
      if (rc_cst != CDD_C_SUCCESS)
        return rc_cst;
    }

    if (kind == TOKEN_LPAREN) {
      seen_lparen = 1;
      break;
    }
    k++;
  }

  if (!seen_lparen)
    return CDD_C_SUCCESS;

  paren_depth = 1;
  k++;
  while (paren_depth > 0 && k < limit) {
    if (tokens->tokens[k].kind == TOKEN_LPAREN)
      paren_depth++;
    else if (tokens->tokens[k].kind == TOKEN_RPAREN)
      paren_depth--;
    k++;
  }
  if (paren_depth > 0)
    return CDD_C_SUCCESS;
  if (k >= limit)
    return CDD_C_SUCCESS;

  {
    cdd_c_error_t rc_cst = skip_ws(tokens, k, limit, &_ast_skip_ws_0);
    if (rc_cst != CDD_C_SUCCESS)
      return rc_cst;
  }
  k = _ast_skip_ws_0;
  if (tokens->tokens[k].kind != TOKEN_LBRACE)
    return CDD_C_SUCCESS;

  brace_depth = 1;
  k++;
  while (brace_depth > 0 && k < limit) {
    if (tokens->tokens[k].kind == TOKEN_LBRACE)
      brace_depth++;
    else if (tokens->tokens[k].kind == TOKEN_RBRACE)
      brace_depth--;
    k++;
  }
  if (brace_depth > 0)
    return CDD_C_SUCCESS;

  *end_idx_out = k;
  *out_is_match = 1;
  return CDD_C_SUCCESS;
}

/**
 * @brief Consume a balanced parenthesized block `( ... )`.
 *
 * @param[in] tokens Token list.
 * @param[in] start Start index (at LPAREN).
 * @param[in] limit Index limit.
 * @param[out] _out_val Index after closing RPAREN.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t consume_balanced_parens(const struct TokenList *tokens,
                                      size_t start, size_t limit,
                                      size_t *_out_val) {
  size_t i = start;
  int depth = 0;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_consume_balanced_parens &&
      --g_cdd_fail_consume_balanced_parens == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  if (!tokens || !_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (i >= limit || tokens->tokens[i].kind != TOKEN_LPAREN) {
    *_out_val = start;
    return CDD_C_SUCCESS;
  }

  depth = 1;
  i++;

  while (depth > 0 && i < limit) {
    if (tokens->tokens[i].kind == TOKEN_LPAREN) {
      depth++;
    } else if (tokens->tokens[i].kind == TOKEN_RPAREN) {
      depth--;
    }
    i++;
  }

  if (depth == 0) {
    *_out_val = i;
    return CDD_C_SUCCESS;
  }
  *_out_val = start;
  return CDD_C_SUCCESS;
}

/**
 * @brief Consume a C23 attribute block `[[ ... ]]`.
 *
 * @param[in] tokens Token list.
 * @param[in] start Start index (at first LBRACKET).
 * @param[in] limit Index limit.
 * @param[out] _out_val Index after closing RBRACKETs or start if unclosed.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t consume_attributes(const struct TokenList *tokens, size_t start,
                                 size_t limit, size_t *_out_val) {
  size_t i = start + 2;
  int depth = 2;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_consume_attributes && --g_cdd_fail_consume_attributes == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  if (!tokens || !_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  while (i < limit && depth > 0) {
    if (tokens->tokens[i].kind == TOKEN_LBRACKET) {
      depth++;
    } else if (tokens->tokens[i].kind == TOKEN_RBRACKET) {
      depth--;
    }
    i++;
  }

  if (depth == 0) {
    *_out_val = i;
    return CDD_C_SUCCESS;
  }
  *_out_val = start;
  return CDD_C_SUCCESS;
}

/**
 * @brief Consume a static assertion declaration.
 *
 * @param[in] tokens Token list.
 * @param[in] start Start index.
 * @param[in] limit Index limit.
 * @param[out] _out_val Index after semicolon, or start if invalid.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t consume_static_assert(const struct TokenList *tokens,
                                    size_t start, size_t limit,
                                    size_t *_out_val) {
  size_t _ast_skip_ws_1 = 0;
  size_t _ast_skip_ws_2 = 0;
  size_t i = start + 1;
  int paren_depth = 0;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_consume_static_assert &&
      --g_cdd_fail_consume_static_assert == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  if (!tokens || !_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  {
    cdd_c_error_t rc_cst = skip_ws(tokens, i, limit, &_ast_skip_ws_1);
    if (rc_cst != CDD_C_SUCCESS)
      return rc_cst;
  }
  i = _ast_skip_ws_1;
  if (i < limit && tokens->tokens[i].kind == TOKEN_LPAREN) {
    paren_depth = 1;
    i++;
  } else {
    *_out_val = start;
    return CDD_C_SUCCESS;
  }

  while (paren_depth > 0 && i < limit) {
    if (tokens->tokens[i].kind == TOKEN_LPAREN)
      paren_depth++;
    else if (tokens->tokens[i].kind == TOKEN_RPAREN)
      paren_depth--;
    i++;
  }

  {
    cdd_c_error_t rc_cst = skip_ws(tokens, i, limit, &_ast_skip_ws_2);
    if (rc_cst != CDD_C_SUCCESS)
      return rc_cst;
  }
  i = _ast_skip_ws_2;
  if (i < limit && tokens->tokens[i].kind == TOKEN_SEMICOLON) {
    *_out_val = i + 1;
    return CDD_C_SUCCESS;
  }
  *_out_val = start;
  return CDD_C_SUCCESS;
}

/**
 * @brief Consume a _Generic selection `_Generic ( ... )`.
 *
 * @param[in] tokens Token list.
 * @param[in] start Start index.
 * @param[in] limit Index limit.
 * @param[out] _out_val Index after balanced parens.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t consume_generic_selection(const struct TokenList *tokens,
                                        size_t start, size_t limit,
                                        size_t *_out_val) {
  size_t _ast_skip_ws_3 = 0;
  size_t _ast_consume_balanced_parens_4 = 0;
  size_t i = start + 1;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_consume_generic_selection &&
      --g_cdd_fail_consume_generic_selection == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  if (!tokens || !_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  {
    cdd_c_error_t rc_cst = skip_ws(tokens, i, limit, &_ast_skip_ws_3);
    if (rc_cst != CDD_C_SUCCESS)
      return rc_cst;
  }
  i = _ast_skip_ws_3;
  if (i >= limit || tokens->tokens[i].kind != TOKEN_LPAREN) {
    *_out_val = start;
    return CDD_C_SUCCESS;
  }
  {
    cdd_c_error_t rc_cst = consume_balanced_parens(
        tokens, i, limit, &_ast_consume_balanced_parens_4);
    if (rc_cst != CDD_C_SUCCESS)
      return rc_cst;
  }
  if (_ast_consume_balanced_parens_4 == i) {
    *_out_val = start;
    return CDD_C_SUCCESS;
  }
  *_out_val = _ast_consume_balanced_parens_4;
  return CDD_C_SUCCESS;
}

/**
 * @brief Identify if the LBRACE at `brace_idx` signifies an expression/init
 * list.
 *
 * @param[in] tokens Token list.
 * @param[in] brace_idx Index of LBRACE token.
 * @param[out] out_is_expr Pointer to receive 1 if expression brace, 0
 * otherwise.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t is_expression_brace(const struct TokenList *tokens,
                                  size_t brace_idx, int *out_is_expr) {
  size_t _ast_skip_ws_back_5 = 0;
  size_t _ast_skip_ws_back_6 = 0;
  size_t prev;
  enum TokenKind pk;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_is_expression_brace && --g_cdd_fail_is_expression_brace == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  if (!tokens || !out_is_expr)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  *out_is_expr = 0;

  {
    cdd_c_error_t rc_cst =
        skip_ws_back(tokens, brace_idx, &_ast_skip_ws_back_5);
    if (rc_cst != CDD_C_SUCCESS)
      return rc_cst;
  }
  prev = _ast_skip_ws_back_5;

  pk = tokens->tokens[prev].kind;

  if (pk == TOKEN_ASSIGN || pk == TOKEN_COMMA || pk == TOKEN_KEYWORD_RETURN) {
    *out_is_expr = 1;
    return CDD_C_SUCCESS;
  }

  if (pk == TOKEN_RPAREN) {
    int depth = 1;
    size_t k = prev;

    while (k > 0 && depth > 0) {
      k--;
      if (tokens->tokens[k].kind == TOKEN_LPAREN)
        depth--;
    }

    {
      size_t before_paren;
      enum TokenKind bpk;
      {
        cdd_c_error_t rc_cst = skip_ws_back(tokens, k, &_ast_skip_ws_back_6);
        if (rc_cst != CDD_C_SUCCESS)
          return rc_cst;
      }
      before_paren = _ast_skip_ws_back_6;
      bpk = tokens->tokens[before_paren].kind;

      if (bpk == TOKEN_KEYWORD_IF) {
        return CDD_C_SUCCESS;
      }

      *out_is_expr = 1;
      return CDD_C_SUCCESS;
    }
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Consume a brace-enclosed block, respecting nesting.
 *
 * @param[in] tokens Token list.
 * @param[in] start Start index (at LBRACE).
 * @param[in] limit Index limit.
 * @param[out] _out_val Index after closing RBRACE.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t consume_balanced_braces(const struct TokenList *tokens,
                                      size_t start, size_t limit,
                                      size_t *_out_val) {
  size_t i = start;
  int depth = 0;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_consume_balanced_braces &&
      --g_cdd_fail_consume_balanced_braces == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  if (!tokens || !_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  depth = 1;
  i++;

  while (i < limit && depth > 0) {
    if (tokens->tokens[i].kind == TOKEN_LBRACE) {
      depth++;
    } else if (tokens->tokens[i].kind == TOKEN_RBRACE) {
      depth--;
    }
    i++;
  }

  *_out_val = i;
  return CDD_C_SUCCESS;
}

#ifdef CDD_BUILD_TESTS
cdd_c_error_t cdd_test_cst_skip_ws(const struct TokenList *tokens, size_t i,
                                   size_t limit, size_t *out_val) {
  return skip_ws(tokens, i, limit, out_val);
}

cdd_c_error_t cdd_test_cst_skip_ws_back(const struct TokenList *tokens,
                                        size_t i, size_t *out_val) {
  return skip_ws_back(tokens, i, out_val);
}

cdd_c_error_t cdd_test_cst_is_type_start(const struct Token *tok,
                                         int *out_is_type) {
  return is_type_start(tok, out_is_type);
}

cdd_c_error_t
cdd_test_cst_match_function_definition(const struct TokenList *tokens,
                                       size_t start_idx, size_t limit,
                                       size_t *end_idx_out, int *out_is_match) {
  return match_function_definition(tokens, start_idx, limit, end_idx_out,
                                   out_is_match);
}

cdd_c_error_t
cdd_test_cst_consume_balanced_parens(const struct TokenList *tokens,
                                     size_t start, size_t limit,
                                     size_t *out_val) {
  return consume_balanced_parens(tokens, start, limit, out_val);
}

cdd_c_error_t cdd_test_cst_consume_attributes(const struct TokenList *tokens,
                                              size_t start, size_t limit,
                                              size_t *out_val) {
  return consume_attributes(tokens, start, limit, out_val);
}

cdd_c_error_t cdd_test_cst_consume_static_assert(const struct TokenList *tokens,
                                                 size_t start, size_t limit,
                                                 size_t *out_val) {
  return consume_static_assert(tokens, start, limit, out_val);
}

cdd_c_error_t
cdd_test_cst_consume_generic_selection(const struct TokenList *tokens,
                                       size_t start, size_t limit,
                                       size_t *out_val) {
  return consume_generic_selection(tokens, start, limit, out_val);
}

cdd_c_error_t cdd_test_cst_is_expression_brace(const struct TokenList *tokens,
                                               size_t brace_idx,
                                               int *out_is_expr) {
  return is_expression_brace(tokens, brace_idx, out_is_expr);
}

cdd_c_error_t
cdd_test_cst_consume_balanced_braces(const struct TokenList *tokens,
                                     size_t start, size_t limit,
                                     size_t *out_val) {
  return consume_balanced_braces(tokens, start, limit, out_val);
}

#endif /* CDD_BUILD_TESTS */
