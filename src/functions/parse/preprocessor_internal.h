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

#define skip_ws pp_expr_skip_ws
#define match pp_expr_match
#define handle_has_include_embed pp_handle_has_include_embed
#define handle_has_c_attribute pp_handle_has_c_attribute
#define parse_primary pp_parse_primary
#define parse_unary pp_parse_unary
#define parse_multiplicative pp_parse_multiplicative
#define parse_additive pp_parse_additive
#define parse_shift pp_parse_shift
#define parse_relational pp_parse_relational
#define parse_equality pp_parse_equality
#define parse_logic_and pp_parse_logic_and
#define parse_logic_or pp_parse_logic_or
#define parse_expr pp_parse_expr

/* Internal declarations for expression evaluation and directive handling */
C_CDD_EXPORT cdd_c_error_t pp_expr_skip_ws(struct ExprState *s);
C_CDD_EXPORT cdd_c_error_t pp_expr_match(struct ExprState *s,
                                         enum TokenKind kind, int *out_val);
C_CDD_EXPORT cdd_c_error_t pp_handle_has_include_embed(struct ExprState *s,
                                                       long *out_val);
C_CDD_EXPORT cdd_c_error_t pp_handle_has_c_attribute(struct ExprState *s,
                                                     long *out_val);
C_CDD_EXPORT cdd_c_error_t pp_parse_primary(struct ExprState *s, long *out_val);
C_CDD_EXPORT cdd_c_error_t pp_parse_unary(struct ExprState *s, long *out_val);
C_CDD_EXPORT cdd_c_error_t pp_parse_multiplicative(struct ExprState *s,
                                                   long *out_val);
C_CDD_EXPORT cdd_c_error_t pp_parse_additive(struct ExprState *s,
                                             long *out_val);
C_CDD_EXPORT cdd_c_error_t pp_parse_shift(struct ExprState *s, long *out_val);
C_CDD_EXPORT cdd_c_error_t pp_parse_relational(struct ExprState *s,
                                               long *out_val);
C_CDD_EXPORT cdd_c_error_t pp_parse_equality(struct ExprState *s,
                                             long *out_val);
C_CDD_EXPORT cdd_c_error_t pp_parse_logic_and(struct ExprState *s,
                                              long *out_val);
C_CDD_EXPORT cdd_c_error_t pp_parse_logic_or(struct ExprState *s,
                                             long *out_val);
C_CDD_EXPORT cdd_c_error_t pp_parse_expr(struct ExprState *s, long *out_val);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_PREPROCESSOR_INTERNAL_H */
