/**
 * @file preprocessor_internal.h
 * @brief Internal declarations for preprocessor parsing and evaluation.
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_PREPROCESSOR_INTERNAL_H
#define C_CDD_PREPROCESSOR_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"
#include <stddef.h>

#include "c_cdd_export.h"
#include "cdd_c_error.h"
#include "functions/parse/preprocessor.h"
/* clang-format on */

/** @brief Symbol alias for pp_expr_skip_ws */
#define skip_ws pp_expr_skip_ws
/** @brief Symbol alias for pp_expr_match */
#define match pp_expr_match
/** @brief Symbol alias for pp_handle_has_include_embed */
#define handle_has_include_embed pp_handle_has_include_embed
/** @brief Symbol alias for pp_handle_has_c_attribute */
#define handle_has_c_attribute pp_handle_has_c_attribute
/** @brief Symbol alias for pp_parse_primary */
#define parse_primary pp_parse_primary
/** @brief Symbol alias for pp_parse_unary */
#define parse_unary pp_parse_unary
/** @brief Symbol alias for pp_parse_multiplicative */
#define parse_multiplicative pp_parse_multiplicative
/** @brief Symbol alias for pp_parse_additive */
#define parse_additive pp_parse_additive
/** @brief Symbol alias for pp_parse_shift */
#define parse_shift pp_parse_shift
/** @brief Symbol alias for pp_parse_relational */
#define parse_relational pp_parse_relational
/** @brief Symbol alias for pp_parse_equality */
#define parse_equality pp_parse_equality
/** @brief Symbol alias for pp_parse_logic_and */
#define parse_logic_and pp_parse_logic_and
/** @brief Symbol alias for pp_parse_logic_or */
#define parse_logic_or pp_parse_logic_or
/** @brief Symbol alias for pp_parse_expr */
#define parse_expr pp_parse_expr

/* Internal declarations for expression evaluation and directive handling */
/**
 * @brief Advances past whitespace tokens in an expression.
 *
 * @param[in,out] s Expression state.
 * @return CDD_C_SUCCESS.
 */
C_CDD_EXPORT cdd_c_error_t pp_expr_skip_ws(struct ExprState *s);

/**
 * @brief Matches and consumes a specific token kind if present.
 *
 * @param[in,out] s Expression state.
 * @param[in] kind Token kind to match.
 * @param[out] out_val Pointer to store 1 if matched, 0 otherwise.
 * @return CDD_C_SUCCESS.
 */
C_CDD_EXPORT cdd_c_error_t pp_expr_match(struct ExprState *s,
                                         enum TokenKind kind, int *out_val);

/**
 * @brief Handles __has_include and __has_embed preprocessor expressions.
 *
 * @param[in,out] s Expression state.
 * @param[out] out_val Pointer to store evaluation result (1 or 0).
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t pp_handle_has_include_embed(struct ExprState *s,
                                                       long *out_val);

/**
 * @brief Handles __has_c_attribute preprocessor expressions.
 *
 * @param[in,out] s Expression state.
 * @param[out] out_val Pointer to store attribute version or 0.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t pp_handle_has_c_attribute(struct ExprState *s,
                                                     long *out_val);

/**
 * @brief Parses a primary preprocessor expression.
 * @param[in,out] s Expression state.
 * @param[out] out_val Evaluated value.
 * @return CDD_C_SUCCESS on success or error code.
 */
C_CDD_EXPORT cdd_c_error_t pp_parse_primary(struct ExprState *s, long *out_val);

/**
 * @brief Parses a unary preprocessor expression.
 * @param[in,out] s Expression state.
 * @param[out] out_val Evaluated value.
 * @return CDD_C_SUCCESS on success or error code.
 */
C_CDD_EXPORT cdd_c_error_t pp_parse_unary(struct ExprState *s, long *out_val);

/**
 * @brief Parses a multiplicative preprocessor expression.
 * @param[in,out] s Expression state.
 * @param[out] out_val Evaluated value.
 * @return CDD_C_SUCCESS on success or error code.
 */
C_CDD_EXPORT cdd_c_error_t pp_parse_multiplicative(struct ExprState *s,
                                                   long *out_val);

/**
 * @brief Parses an additive preprocessor expression.
 * @param[in,out] s Expression state.
 * @param[out] out_val Evaluated value.
 * @return CDD_C_SUCCESS on success or error code.
 */
C_CDD_EXPORT cdd_c_error_t pp_parse_additive(struct ExprState *s,
                                             long *out_val);

/**
 * @brief Parses a bitwise shift preprocessor expression.
 * @param[in,out] s Expression state.
 * @param[out] out_val Evaluated value.
 * @return CDD_C_SUCCESS on success or error code.
 */
C_CDD_EXPORT cdd_c_error_t pp_parse_shift(struct ExprState *s, long *out_val);

/**
 * @brief Parses a relational preprocessor expression.
 * @param[in,out] s Expression state.
 * @param[out] out_val Evaluated value.
 * @return CDD_C_SUCCESS on success or error code.
 */
C_CDD_EXPORT cdd_c_error_t pp_parse_relational(struct ExprState *s,
                                               long *out_val);

/**
 * @brief Parses an equality preprocessor expression.
 * @param[in,out] s Expression state.
 * @param[out] out_val Evaluated value.
 * @return CDD_C_SUCCESS on success or error code.
 */
C_CDD_EXPORT cdd_c_error_t pp_parse_equality(struct ExprState *s,
                                             long *out_val);

/**
 * @brief Parses a logical AND preprocessor expression.
 * @param[in,out] s Expression state.
 * @param[out] out_val Evaluated value.
 * @return CDD_C_SUCCESS on success or error code.
 */
C_CDD_EXPORT cdd_c_error_t pp_parse_logic_and(struct ExprState *s,
                                              long *out_val);

/**
 * @brief Parses a logical OR preprocessor expression.
 * @param[in,out] s Expression state.
 * @param[out] out_val Evaluated value.
 * @return CDD_C_SUCCESS on success or error code.
 */
C_CDD_EXPORT cdd_c_error_t pp_parse_logic_or(struct ExprState *s,
                                             long *out_val);

/**
 * @brief Parses a top-level conditional preprocessor expression.
 * @param[in,out] s Expression state.
 * @param[out] out_val Evaluated value.
 * @return CDD_C_SUCCESS on success or error code.
 */
C_CDD_EXPORT cdd_c_error_t pp_parse_expr(struct ExprState *s, long *out_val);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_PREPROCESSOR_INTERNAL_H */
