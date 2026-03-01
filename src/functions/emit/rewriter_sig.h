/**
 * @file rewriter_sig.h
 * @brief Logic to transform C function signatures to use integer error codes.
 *
 * Robustly parses function definitions/declarations token streams and
 * rewrites them to follow the pattern: `int function_name(args, Type *out)`.
 *
 * Capabilities:
 * - Robust handling of C23 Attributes `[[nodiscard]]`.
 * - Deep parsing of declarators to support array arguments `int a[]` and
 *   function pointer arguments `int (*cb)(void)`.
 * - Preservation of storage specifiers and qualifiers.
 * - Support for K&R style obsolescent function definitions:
 *   `int foo(a) int a; { ... }`.
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_REWRITER_SIG_H
#define C_CDD_REWRITER_SIG_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "functions/parse/tokenizer.h"
#include <c_cdd_export.h>

/**
 * @brief Rewrite a function signature token stream into a C string.
 *
 * Analyzes the provided token list (representing a function header) to
 * decompose it into Attributes, Storage Class, Return Type, Name, Arguments,
 * and optional K&R Declarations.
 *
 * Transformation Rules:
 * 1. `void func(...)` -> `int func(...)`
 * 2. `Type func(...)` -> `int func(..., Type *out)`
 * 3. Preserves `[[...]]` attributes and storage specifiers like `static`.
 * 4. Preserves K&R declaration lists, injecting `out` parameter declarations
 *    if necessary (e.g., `int f(a, out) int a; Type *out;`).
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

#endif /* C_CDD_REWRITER_SIG_H */
