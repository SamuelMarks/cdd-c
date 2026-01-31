/**
 * @file rewriter_sig.h
 * @brief Logic to transform C function signatures to use integer error codes.
 *
 * Robustly parses function definitions/declarations token streams and
 * rewrites them to follow the pattern: `int function_name(args, Type *out)`.
 *
 * Supports:
 * - `void func(...)` -> `int func(...)`
 * - `Type func(...)` -> `int func(..., Type *out)`
 * - `Type* func(...)` -> `int func(..., Type **out)`
 * - Preservation of storage specifiers (`static`, `extern`).
 * - Preservation of `const`, `volatile`, and complex pointer types.
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
 * 1. If return type is `void` (and not pointer), change to `int`.
 * 2. If return type is `int` (and not a pointer), keep as is (assumed error
 * code).
 * 3. Otherwise (e.g. `double`, `char*`, `struct S`), change return type to
 * `int` and append the original type as a pointer argument named `out`.
 *
 * @param[in] tokens Valid token list containing the function header.
 * @param[out] out_code Pointer to char* where the allocated result string will
 * be stored.
 * @return 0 on success, ENOMEM/EINVAL on failure.
 */
extern C_CDD_EXPORT int rewrite_signature(const struct TokenList *tokens,
                                          char **out_code);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* REWRITER_SIG_H */
