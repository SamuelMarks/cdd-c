/**
 * @file rewriter_sig.h
 * @brief Logic to transform C function signatures to use integer error codes.
 *
 * Converts signatures like:
 * - `void func(...)` -> `int func(...)`
 * - `Type func(...)` -> `int func(..., Type *out)`
 * - `Type* func(...)` -> `int func(..., Type **out)`
 *
 * @author Samuel Marks
 */

#ifndef REWRITER_SIG_H
#define REWRITER_SIG_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "tokenizer.h"
#include <c_cdd_export.h>

/**
 * @brief Rewrite a function signature token stream into a C string.
 *
 * Scans the provided token list (expected to represent a single function
 * declaration/definition up to the closing parenthesis) and generates
 * a refactored C code string with an `int` return type.
 *
 * Rules:
 * 1. If return type is `void`, change to `int`.
 * 2. If return type is `int` (and not a pointer), keep as is.
 * 3. Otherwise (e.g. `double`, `char*`, `struct S`), change return type to
 * `int` and append the original type as a pointer-to-pointer (or pointer)
 * argument named `out`.
 *
 * @param[in] tokens Valid token list containing the function header.
 * @param[out] out_code Pointer to char* where the allocated string will be
 * stored.
 * @return 0 on success, ENOMEM/EINVAL/ENOTSUP on failure.
 */
extern C_CDD_EXPORT int rewrite_signature(const struct TokenList *tokens,
                                          char **out_code);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* REWRITER_SIG_H */
