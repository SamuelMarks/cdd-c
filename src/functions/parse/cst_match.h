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
 * @brief Checks if a token starts a type specification.
 *
 * @param[in] tok Token to inspect.
 * @param[out] out_is_type Pointer to int set to 1 if type start, 0 otherwise.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t is_type_start(const struct Token *tok, int *out_is_type);

/**
 * @brief Matches function definition patterns.
 *
 * @param[in] tokens Token list.
 * @param[in] start_idx Start token index.
 * @param[in] limit Limit token index.
 * @param[out] end_idx_out Output index after function signature.
 * @param[out] out_is_match Pointer to receive 1 if match, 0 otherwise.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t match_function_definition(const struct TokenList *tokens,
                                        size_t start_idx, size_t limit,
                                        size_t *end_idx_out, int *out_is_match);

/**
 * @brief Consumes balanced parentheses starting from LPAREN.
 *
 * @param[in] tokens Token list.
 * @param[in] start Index of LPAREN token.
 * @param[in] limit Upper bound token index.
 * @param[out] _out_val Index after closing RPAREN.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t consume_balanced_parens(const struct TokenList *tokens,
                                      size_t start, size_t limit,
                                      size_t *_out_val);

/**
 * @brief Consumes C23 standard attributes [[ ... ]].
 *
 * @param[in] tokens Token list.
 * @param[in] start Index of first LBRACKET token.
 * @param[in] limit Upper bound token index.
 * @param[out] _out_val Index after closing RBRACKETs or start if unclosed.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t consume_attributes(const struct TokenList *tokens, size_t start,
                                 size_t limit, size_t *_out_val);

/**
 * @brief Consumes C11 static assert directive: _Static_assert ( ... ) ;
 *
 * @param[in] tokens Token list.
 * @param[in] start Index of STATIC_ASSERT token.
 * @param[in] limit Upper bound token index.
 * @param[out] _out_val Index after semicolon, or start if invalid.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t consume_static_assert(const struct TokenList *tokens,
                                    size_t start, size_t limit,
                                    size_t *_out_val);

/**
 * @brief Consumes C11 generic selection: _Generic ( ... )
 *
 * @param[in] tokens Token list.
 * @param[in] start Index of GENERIC token.
 * @param[in] limit Upper bound token index.
 * @param[out] _out_val Index after balanced parens.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t consume_generic_selection(const struct TokenList *tokens,
                                        size_t start, size_t limit,
                                        size_t *_out_val);

/**
 * @brief Determines if an opening brace belongs to an expression rather than a
 * block.
 *
 * @param[in] tokens Token list.
 * @param[in] brace_idx Index of the LBRACE token.
 * @param[out] out_is_expr Output 1 if brace belongs to expression, 0
 * otherwise.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
cdd_c_error_t is_expression_brace(const struct TokenList *tokens,
                                  size_t brace_idx, int *out_is_expr);

/**
 * @brief Consumes balanced curly braces { ... }.
 *
 * @param[in] tokens Token list.
 * @param[in] start Index of opening LBRACE token.
 * @param[in] limit Upper bound token index.
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
