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

/* clang-format off */
#include "functions/parse/tokenizer.h"
#include "cdd_c_error.h"
#include <c_cdd_export.h>
/* clang-format on */

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
extern C_CDD_EXPORT cdd_c_error_t
rewrite_signature(const struct TokenList *tokens, char **out_code);

#ifdef CDD_BUILD_TESTS
/**
 * @brief Test helper to check if args represent void.
 * @param[in] args Arguments string.
 * @param[out] out_is_empty Set to 1 if args represent void or empty.
 * @return CDD_C_SUCCESS on success, error enum on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t test_args_represent_void(const char *args,
                                                           int *out_is_empty);

/**
 * @brief Test helper to verify if a token range represents a void return type.
 * @param[in] tokens Token list.
 * @param[in] start Start index in token list.
 * @param[in] end End index in token list.
 * @param[out] out_is_void Set to 1 if return type is void.
 * @return CDD_C_SUCCESS on success, error enum on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t test_check_is_void(
    const struct TokenList *tokens, size_t start, size_t end, int *out_is_void);
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_REWRITER_SIG_H */
