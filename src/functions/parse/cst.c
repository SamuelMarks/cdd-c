/**
 * @file cst.c
 * @brief Implementation of the Concrete Syntax Tree logic.
 *
 * Implements recursive descent parsing to group tokens into semantic blocks.
 * Supports C23 attributes, Bit-fields, Static Assertions, C99 Compound
 * Literals, C23 Fixed Enum Types, and C11 _Generic selections.
 *
 * Update: Verified Bit-field support. The `parse_recursive` loop handling
 * `CST_NODE_OTHER` consumes tokens until `;`, `}`, or `{` (expression start).
 * Since bit-fields use `:` (TOKEN_COLON), and colons are treated as regular
 * punctuation within statements (unless they match `is_expression_brace`,
 * which only triggers on `{` preceded by specific tokens), bit-field
 * declarations like `int x : 3;` are correctly grouped into a single statement
 * node.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "c_cdd_export.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "c_cdd/log.h"
#include "c_cdd/memory.h"
#include "functions/parse/cst.h"
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
static cdd_c_error_t skip_ws(const struct TokenList *tokens, size_t i,
                             size_t limit, size_t *_out_val) {
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
static cdd_c_error_t skip_ws_back(const struct TokenList *tokens, size_t i,
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
 * @brief Add a node to the CST list.
 *
 * @param[in,out] list The list to append to.
 * @param[in] kind Node classification.
 * @param[in] start Byte pointer start.
 * @param[in] length Byte length.
 * @param[in] start_tok Token start index.
 * @param[in] end_tok Token end index (exclusive).
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t cst_list_add(struct CstNodeList *list, enum CstNodeKind kind,
                           const uint8_t *start, size_t length,
                           size_t start_tok, size_t end_tok) {
  struct CstNode *new_arr;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_cst_list_add && --g_cdd_fail_cst_list_add == 0)
    return CDD_C_ERROR_MEMORY;
#endif
  if (!list)
    return CDD_C_ERROR_INVALID_ARGUMENT;

#ifdef CDD_BUILD_TESTS
  if (g_cdd_fail_alloc && --g_cdd_fail_alloc == 0)
    return CDD_C_ERROR_MEMORY;
#endif
  if (list->size >= list->capacity) {
    const size_t new_cap = (list->capacity == 0) ? 64 : list->capacity * 2;
    new_arr = (struct CstNode *)C_CDD_REALLOC(list->nodes,
                                              new_cap * sizeof(struct CstNode));
    if (!new_arr) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      return CDD_C_ERROR_MEMORY;
    }
    list->nodes = new_arr;
    list->capacity = new_cap;
  }

  list->nodes[list->size].kind = kind;
  list->nodes[list->size].start = start;
  list->nodes[list->size].length = length;
  list->nodes[list->size].start_token = start_tok;
  list->nodes[list->size].end_token = end_tok;
  list->size++;

  return CDD_C_SUCCESS;
}

/**
 * @brief Checks if a token is a valid start of a type specifier.
 *
 * @param[in] tok The token to inspect.
 * @param[out] out_is_type Pointer to int set to 1 if type start, 0 otherwise.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
static cdd_c_error_t is_type_start(const struct Token *tok, int *out_is_type) {
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
static cdd_c_error_t match_function_definition(const struct TokenList *tokens,
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
static cdd_c_error_t consume_balanced_parens(const struct TokenList *tokens,
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
static cdd_c_error_t consume_attributes(const struct TokenList *tokens,
                                        size_t start, size_t limit,
                                        size_t *_out_val) {
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
static cdd_c_error_t consume_static_assert(const struct TokenList *tokens,
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
static cdd_c_error_t consume_generic_selection(const struct TokenList *tokens,
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
static cdd_c_error_t is_expression_brace(const struct TokenList *tokens,
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
static cdd_c_error_t consume_balanced_braces(const struct TokenList *tokens,
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

/**
 * @brief Recursive Parser core logic.
 *
 * @param[in] tokens Token list.
 * @param[in] start Start token index.
 * @param[in] end End token index.
 * @param[in,out] out CST node list to populate.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
static cdd_c_error_t parse_recursive(const struct TokenList *tokens,
                                     size_t start, size_t end,
                                     struct CstNodeList *out) {
  size_t _ast_consume_attributes_7 = 0;
  size_t _ast_consume_static_assert_8 = 0;
  int _ast_token_matches_string_9 = 0;
  int _ast_token_matches_string_10 = 0;
  size_t _ast_consume_generic_selection_11 = 0;
  size_t _ast_skip_ws_back_12 = 0;
  size_t _ast_consume_balanced_braces_13 = 0;
  size_t _ast_skip_ws_14 = 0;
  size_t _ast_consume_balanced_braces_15 = 0;
  size_t _ast_skip_ws_back_16 = 0;
  int _ast_token_matches_string_17 = 0;
  size_t i = start;
  cdd_c_error_t rc;

  while (i < end) {
    const struct Token *tok = &tokens->tokens[i];

    if (tok->kind == TOKEN_WHITESPACE) {
      i++;
      continue;
    }

    /* GCC Attributes */
    if (tok->kind == TOKEN_KEYWORD_ATTRIBUTE) {
      size_t attr_end = i + 1;
      int depth = 0;
      while (attr_end < end) {
        if (tokens->tokens[attr_end].kind == TOKEN_LPAREN)
          depth++;
        else if (tokens->tokens[attr_end].kind == TOKEN_RPAREN) {
          depth--;
          if (depth == 0) {
            attr_end++;
            break;
          }
        }
        attr_end++;
      }
      if (depth == 0) {
        const struct Token *last = &tokens->tokens[attr_end - 1];
        size_t byte_len = (size_t)((last->start + last->length) - tok->start);
        rc = cst_list_add(out, CST_NODE_GCC_ATTRIBUTE, tok->start, byte_len, i,
                          attr_end);
        if (rc != CDD_C_SUCCESS)
          return rc;
        i = attr_end;
        continue;
      }
    }

    /* MSVC Declspec */
    if (tok->kind == TOKEN_KEYWORD_DECLSPEC) {
      size_t attr_end = i + 1;
      int depth = 0;
      while (attr_end < end) {
        if (tokens->tokens[attr_end].kind == TOKEN_LPAREN)
          depth++;
        else if (tokens->tokens[attr_end].kind == TOKEN_RPAREN) {
          depth--;
          if (depth == 0) {
            attr_end++;
            break;
          }
        }
        attr_end++;
      }
      if (depth == 0) {
        const struct Token *last = &tokens->tokens[attr_end - 1];
        size_t byte_len = (size_t)((last->start + last->length) - tok->start);
        rc = cst_list_add(out, CST_NODE_DECLSPEC, tok->start, byte_len, i,
                          attr_end);
        if (rc != CDD_C_SUCCESS)
          return rc;
        i = attr_end;
        continue;
      }
    }

    /* C23 Attributes */
    if (tok->kind == TOKEN_LBRACKET && i + 1 < end &&
        tokens->tokens[i + 1].kind == TOKEN_LBRACKET) {
      size_t attr_end;
      {
        cdd_c_error_t rc_cst =
            consume_attributes(tokens, i, end, &_ast_consume_attributes_7);
        if (rc_cst != CDD_C_SUCCESS)
          return rc_cst;
      }
      attr_end = _ast_consume_attributes_7;
      if (attr_end > i) {
        const struct Token *last = &tokens->tokens[attr_end - 1];
        size_t byte_len = (size_t)((last->start + last->length) - tok->start);
        rc = cst_list_add(out, CST_NODE_ATTRIBUTE, tok->start, byte_len, i,
                          attr_end);
        if (rc != CDD_C_SUCCESS)
          return rc;
        i = attr_end;
        continue;
      }
    }

    /* Static Assert */
    if (tok->kind == TOKEN_KEYWORD_STATIC_ASSERT) {
      size_t sa_end;
      {
        cdd_c_error_t rc_cst = consume_static_assert(
            tokens, i, end, &_ast_consume_static_assert_8);
        if (rc_cst != CDD_C_SUCCESS)
          return rc_cst;
      }
      sa_end = _ast_consume_static_assert_8;
      if (sa_end > i) {
        const struct Token *last = &tokens->tokens[sa_end - 1];
        size_t byte_len = (size_t)((last->start + last->length) - tok->start);
        rc = cst_list_add(out, CST_NODE_STATIC_ASSERT, tok->start, byte_len, i,
                          sa_end);
        if (rc != CDD_C_SUCCESS)
          return rc;
        i = sa_end;
        continue;
      }
    }

    /* C11 _Generic */
    {
      cdd_c_error_t rc_cst =
          token_matches_string(tok, "_Generic", &_ast_token_matches_string_9);
      if (rc_cst != CDD_C_SUCCESS)
        return rc_cst;
    }
    {
      cdd_c_error_t rc_cst = token_matches_string(
          tok, "generic_selection", &_ast_token_matches_string_10);
      if (rc_cst != CDD_C_SUCCESS)
        return rc_cst;
    }
    if ((tok->kind == TOKEN_IDENTIFIER && _ast_token_matches_string_9) ||
        (tok->kind == TOKEN_IDENTIFIER && _ast_token_matches_string_10)) {
      size_t gen_end;
      {
        cdd_c_error_t rc_cst = consume_generic_selection(
            tokens, i, end, &_ast_consume_generic_selection_11);
        if (rc_cst != CDD_C_SUCCESS)
          return rc_cst;
      }
      gen_end = _ast_consume_generic_selection_11;

      if (gen_end > i) {
        const struct Token *last = &tokens->tokens[gen_end - 1];
        size_t byte_len = (size_t)((last->start + last->length) - tok->start);
        rc = cst_list_add(out, CST_NODE_GENERIC_SELECTION, tok->start, byte_len,
                          i, gen_end);
        if (rc != CDD_C_SUCCESS)
          return rc;
        i = gen_end;
        continue;
      }
    }

    /* Function Definitions */
    {
      int is_type = 0;
      {
        cdd_c_error_t rc_cst = is_type_start(tok, &is_type);
        if (rc_cst != CDD_C_SUCCESS)
          return rc_cst;
      }
      if (is_type || tok->kind == TOKEN_STAR) {
        size_t func_end = 0;
        int is_match = 0;
        {
          cdd_c_error_t rc_cst =
              match_function_definition(tokens, i, end, &func_end, &is_match);
          if (rc_cst != CDD_C_SUCCESS)
            return rc_cst;
        }
        if (is_match) {
          const struct Token *last = &tokens->tokens[func_end - 1];
          size_t byte_len = (size_t)((last->start + last->length) - tok->start);
          rc = cst_list_add(out, CST_NODE_FUNCTION, tok->start, byte_len, i,
                            func_end);
          if (rc != CDD_C_SUCCESS)
            return rc;
          i = func_end;
          continue;
        }
      }
    }

    /* Struct/Enum/Union Blocks */
    if (tok->kind == TOKEN_KEYWORD_STRUCT || tok->kind == TOKEN_KEYWORD_ENUM ||
        tok->kind == TOKEN_KEYWORD_UNION) {
      int is_literal = 0;
      {
        size_t prev;
        {
          cdd_c_error_t rc_cst = skip_ws_back(tokens, i, &_ast_skip_ws_back_12);
          if (rc_cst != CDD_C_SUCCESS)
            return rc_cst;
        }
        prev = _ast_skip_ws_back_12;
        if (prev < i && tokens->tokens[prev].kind == TOKEN_LPAREN) {
          is_literal = 1;
        }
      }

      if (!is_literal) {
        size_t k = i + 1;
        size_t block_end = 0;
        size_t body_start_idx = 0;
        int found_block = 0;

        while (k < end) {
          if (tokens->tokens[k].kind == TOKEN_SEMICOLON)
            break;
          if (tokens->tokens[k].kind == TOKEN_LBRACE) {
            cdd_c_error_t rc_cst;
            body_start_idx = k + 1;
            rc_cst = consume_balanced_braces(tokens, k, end,
                                             &_ast_consume_balanced_braces_13);
            if (rc_cst != CDD_C_SUCCESS)
              return rc_cst;
            block_end = _ast_consume_balanced_braces_13;
            found_block = (block_end > k);
            break;
          }
          k++;
        }

        if (found_block) {
          enum CstNodeKind nk;
          size_t byte_len;
          const struct Token *last;
          size_t next_probe;
          size_t brace_close_idx = _ast_consume_balanced_braces_13 - 1;
          {
            cdd_c_error_t rc_cst =
                skip_ws(tokens, block_end, end, &_ast_skip_ws_14);
            if (rc_cst != CDD_C_SUCCESS)
              return rc_cst;
          }
          next_probe = _ast_skip_ws_14;

          if (next_probe < end &&
              tokens->tokens[next_probe].kind == TOKEN_SEMICOLON) {
            block_end = next_probe + 1;
          }

          if (tok->kind == TOKEN_KEYWORD_STRUCT)
            nk = CST_NODE_STRUCT;
          else if (tok->kind == TOKEN_KEYWORD_ENUM)
            nk = CST_NODE_ENUM;
          else
            nk = CST_NODE_UNION;

          last = &tokens->tokens[block_end - 1];
          byte_len = (size_t)((last->start + last->length) - tok->start);

          rc = cst_list_add(out, nk, tok->start, byte_len, i, block_end);
          if (rc != CDD_C_SUCCESS)
            return rc;

          if (brace_close_idx > body_start_idx) {
            cdd_c_error_t rc_cst =
                parse_recursive(tokens, body_start_idx, brace_close_idx, out);
            if (rc_cst != CDD_C_SUCCESS)
              return rc_cst;
          }
          i = block_end;
          continue;
        } else {
          /* Forward Decl */
          size_t decl_end = k;
          if (decl_end < end)
            decl_end++;
          {
            enum CstNodeKind nk =
                (tok->kind == TOKEN_KEYWORD_STRUCT) ? CST_NODE_STRUCT
                : (tok->kind == TOKEN_KEYWORD_ENUM) ? CST_NODE_ENUM
                                                    : CST_NODE_UNION;
            const struct Token *last = &tokens->tokens[decl_end - 1];
            size_t byte_len =
                (size_t)((last->start + last->length) - tok->start);
            rc = cst_list_add(out, nk, tok->start, byte_len, i, decl_end);
            if (rc != CDD_C_SUCCESS)
              return rc;
          }
          i = decl_end;
          continue;
        }
      }
    }

    if (tok->kind == TOKEN_COMMENT) {
      rc = cst_list_add(out, CST_NODE_COMMENT, tok->start, tok->length, i,
                        i + 1);
      if (rc != CDD_C_SUCCESS)
        return rc;
      i++;
      continue;
    }
    if (tok->kind == TOKEN_MACRO || tok->kind == TOKEN_HASH) {
      size_t j = i + 1;
      while (j < end) {
        if (tokens->tokens[j - 1].kind == TOKEN_WHITESPACE &&
            memchr(tokens->tokens[j - 1].start, '\n',
                   tokens->tokens[j - 1].length)) {
          break;
        }
        j++;
      }
      rc = cst_list_add(out, CST_NODE_MACRO, tok->start, tok->length, i, j);
      if (rc != CDD_C_SUCCESS)
        return rc;
      i = j;
      continue;
    }

    /* Group CST_NODE_OTHER (Statements / Vars / Bitfields) */
    {
      size_t j = i + 1;
      while (j < end) {
        enum TokenKind kind = tokens->tokens[j].kind;

        if (kind == TOKEN_SEMICOLON) {
          j++;
          break;
        }

        if (kind == TOKEN_RBRACE) {
          break;
        }

        if (kind == TOKEN_LBRACE) {
          int is_expr = 0;
          {
            cdd_c_error_t rc_cst = is_expression_brace(tokens, j, &is_expr);
            if (rc_cst != CDD_C_SUCCESS)
              return rc_cst;
          }
          if (is_expr) {
            cdd_c_error_t rc_cst = consume_balanced_braces(
                tokens, j, end, &_ast_consume_balanced_braces_15);
            if (rc_cst != CDD_C_SUCCESS)
              return rc_cst;
            j = _ast_consume_balanced_braces_15;
            continue;
          } else {
            break;
          }
        }

        if (kind == TOKEN_KEYWORD_STRUCT || kind == TOKEN_KEYWORD_ENUM ||
            kind == TOKEN_KEYWORD_UNION) {
          size_t prev;
          {
            cdd_c_error_t rc_cst =
                skip_ws_back(tokens, j, &_ast_skip_ws_back_16);
            if (rc_cst != CDD_C_SUCCESS)
              return rc_cst;
          }
          prev = _ast_skip_ws_back_16;

          if (tokens->tokens[prev].kind == TOKEN_LPAREN) {
            j++;
            continue;
          }
          break;
        }

        if (kind == TOKEN_COMMENT || kind == TOKEN_MACRO ||
            kind == TOKEN_HASH || kind == TOKEN_KEYWORD_STATIC_ASSERT) {
          break;
        }
        {
          cdd_c_error_t rc_cst = token_matches_string(
              &tokens->tokens[j], "_Generic", &_ast_token_matches_string_17);
          if (rc_cst != CDD_C_SUCCESS)
            return rc_cst;
        }
        if (kind == TOKEN_IDENTIFIER && _ast_token_matches_string_17) {
          break;
        }

        if (kind == TOKEN_LBRACKET && j + 1 < end &&
            tokens->tokens[j + 1].kind == TOKEN_LBRACKET) {
          break;
        }
        j++;
      }

      {
        const struct Token *last = &tokens->tokens[j - 1];
        size_t byte_len = (size_t)((last->start + last->length) - tok->start);
        rc = cst_list_add(out, CST_NODE_OTHER, tok->start, byte_len, i, j);
        if (rc != CDD_C_SUCCESS)
          return rc;
        i = j;
      }
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Parses tokens from the given input.
 *
 * @param[in] tokens The token stream.
 * @param[out] out Destination structure.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t parse_tokens(const struct TokenList *tokens,
                           struct CstNodeList *out) {
  if (!tokens || !out)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (out->capacity == 0) {
    out->nodes = NULL;
    out->size = 0;
  }

  return parse_recursive(tokens, 0, tokens->size, out);
}

/**
 * @brief Frees the memory associated with cst node list.
 *
 * @param[in,out] list The list to clean.
 */
void free_cst_node_list(struct CstNodeList *list) {
  if (!list)
    return;
  if (list->nodes) {
    C_CDD_FREE(list->nodes);
    list->nodes = NULL;
  }
  list->size = 0;
  list->capacity = 0;
}

/**
 * @brief Executes the cst find first operation.
 *
 * @param[in] list The list to search.
 * @param[in] kind The kind to search for.
 * @param[out] out_node Pointer to store found node, or NULL.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t cst_find_first(struct CstNodeList *list,
                             const enum CstNodeKind kind,
                             struct CstNode **out_node) {
  size_t i;
  if (!out_node)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_node = NULL;
  if (!list) {
    return CDD_C_SUCCESS;
  }

  for (i = 0; i < list->size; i++) {
    if (list->nodes[i].kind == kind) {
      *out_node = &list->nodes[i];
      return CDD_C_SUCCESS;
    }
  }
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

cdd_c_error_t cdd_test_cst_parse_recursive(const struct TokenList *tokens,
                                           size_t start, size_t end,
                                           struct CstNodeList *out) {
  return parse_recursive(tokens, start, end, out);
}
#endif /* CDD_BUILD_TESTS */
