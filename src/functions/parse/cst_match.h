/**
 * @file cst_match.h
 * @brief Token matching and scanning routines for CST parsing.
 *
 * @author Samuel Marks
 */

#ifndef CDD_FUNCTIONS_PARSE_CST_MATCH_H
#define CDD_FUNCTIONS_PARSE_CST_MATCH_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/* clang-format off */
#include "c_cdd_export.h"
#include <stddef.h>

#include "cdd_c_error.h"
#include "functions/parse/tokenizer.h"
/* clang-format on */

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
                      size_t *_out_val);

/**
 * @brief Helper to skip whitespace tokens backwards.
 *
 * @param[in] tokens Token list.
 * @param[in] i Current token index.
 * @param[out] _out_val Index of previous non-whitespace token.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t skip_ws_back(const struct TokenList *tokens, size_t i,
                           size_t *_out_val);

/**
 * @brief Checks if a token is a valid start of a type specifier.
 *
 * @param[in] tok The token to inspect.
 * @param[out] out_is_type Pointer to int set to 1 if type start, 0 otherwise.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t is_type_start(const struct Token *tok, int *out_is_type);

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
                                        size_t *end_idx_out, int *out_is_match);

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
                                      size_t *_out_val);

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
                                 size_t limit, size_t *_out_val);

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
                                    size_t *_out_val);

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
                                        size_t *_out_val);

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
                                  size_t brace_idx, int *out_is_expr);

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
                                      size_t *_out_val);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !CDD_FUNCTIONS_PARSE_CST_MATCH_H */
