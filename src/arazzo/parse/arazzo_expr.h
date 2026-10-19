/**
 * @file arazzo_expr.h
 * @brief Parser and formatter for Arazzo v1.1.0 Runtime Expressions.
 *
 * Implements strict C89 parsing and canonical formatting of all 17 Arazzo
 * v1.1.0 runtime expression roots (`$url`, `$method`, `$statusCode`, `$self`,
 * `$request.*`, `$response.*`, `$message.*`, `$inputs.*`, `$outputs.*`,
 * `$steps.*`, `$workflows.*`, `$sourceDescriptions.*`, `$components.*`) as
 * well as embedded curly-brace string templates (`Bearer
 * {$steps.s.outputs.t}`).
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_ARAZZO_EXPR_H
/** @brief Header guard for arazzo_expr.h */
#define C_CDD_ARAZZO_EXPR_H

/* clang-format off */
#include "arazzo/parse/arazzo_types.h"
#include "c_cdd_export.h"
#include "cdd_c_error.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief Parses an Arazzo 1.1.0 runtime expression or embedded template string.
 *
 * Populates `out_expr` with the classified `Arazzo_RuntimeExprKind`, raw
 * string, extracted identifiers (`source_name`, `step_or_workflow_id`,
 * `property_name`), and optional RFC 6901 `json_pointer` fragment (`#/...`).
 *
 * @param[in] expr_str Null-terminated input expression or literal string.
 * @param[out] out_expr Destination parsed expression structure.
 * @return `CDD_C_SUCCESS` on success, `CDD_C_ERROR_INVALID_ARGUMENT` if
 * malformed or NULL, or `CDD_C_ERROR_MEMORY` on OOM.
 */
C_CDD_EXPORT cdd_c_error_t arazzo_runtime_expr_parse(
    const char *expr_str, struct Arazzo_RuntimeExpr *out_expr);

/**
 * @brief Formats a parsed `Arazzo_RuntimeExpr` back into a canonical string.
 *
 * If `expr->raw` is non-NULL and matches the structural fields, or if `expr`
 * is a literal/template, allocates and returns the canonical representation.
 *
 * @param[in] expr Source parsed runtime expression.
 * @param[out] out_str Output pointer receiving newly allocated string.
 * @return `CDD_C_SUCCESS` on success, `CDD_C_ERROR_INVALID_ARGUMENT` if
 * invalid/NULL, or `CDD_C_ERROR_MEMORY` on OOM.
 */
C_CDD_EXPORT cdd_c_error_t arazzo_runtime_expr_format(
    const struct Arazzo_RuntimeExpr *expr, char **out_str);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_ARAZZO_EXPR_H */
