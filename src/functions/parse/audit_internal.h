/**
 * @file audit_internal.h
 * @brief Internal declarations for C project auditing helpers.
 * @author Samuel Marks
 */

#ifndef PROJECT_AUDIT_INTERNAL_H
#define PROJECT_AUDIT_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/* clang-format off */
#include <c_cdd_export.h>
#include "cdd_c_error.h"
#include "classes/parse/cdd_token.h"
#include "functions/parse/audit.h"
#include <stddef.h>
#if defined(_MSC_VER) && _MSC_VER < 1600
#include "c_cdd/msvc/stdint.h"
#else
#include <stdint.h>
#endif
/* clang-format on */

/**
 * @brief Calculate 1-based line and column numbers from a pointer into a source
 * buffer.
 *
 * @param[in] content Start of the file content buffer.
 * @param[in] token_ptr Pointer to the token within the buffer.
 * @param[out] line 1-based line number.
 * @param[out] col 1-based column number.
 * @return `CDD_C_SUCCESS` on success.
 */
extern C_CDD_EXPORT cdd_c_error_t audit_get_line_col(const char *content,
                                                     const uint8_t *token_ptr,
                                                     size_t *line, size_t *col);

/**
 * @brief Helper to detect functions returning allocations directly.
 *
 * @param[in] tokens Token stream to scan.
 * @param[out] out_count Output pointer receiving the number of direct-return
 * allocations.
 * @return `CDD_C_SUCCESS` on success, or an error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t
count_returning_allocs(const struct TokenList *tokens, int *out_count);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* PROJECT_AUDIT_INTERNAL_H */
