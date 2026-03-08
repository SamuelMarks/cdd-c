/**
 * @file weaver.h
 * @brief High-level Weaver Engine API for safe AST injection and patching.
 *
 * Provides functions to wrap tokens in conditional compilation blocks,
 * preserving indentation and structural integrity of the original source.
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_WEAVER_H
#define C_CDD_WEAVER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stddef.h>

#include "c_cdd_export.h"
#include "functions/emit/patcher.h"
#include "functions/parse/tokenizer.h"

/**
 * @brief Wrap a contiguous block of tokens in a conditional compilation block.
 *
 * Injects `#ifdef <condition>`, an optional `#else` block with `false_tokens`,
 * and `#endif` around the tokens in the range `[start_idx, end_idx)`.
 * Applies correct local indentation to the injected macros.
 *
 * @param[in,out] patches The patch list to populate.
 * @param[in] tokens The original token stream.
 * @param[in] start_idx Inclusive start index of the target tokens.
 * @param[in] end_idx Exclusive end index of the target tokens.
 * @param[in] condition The preprocessor condition (e.g., `_MSC_VER`).
 * @param[in] false_code Malloc'd string or NULL for the `#else` block.
 *                       If provided, it will be injected before `#endif`.
 * @return 0 on success, ENOMEM on allocation failure, EINVAL on invalid args.
 */
extern C_CDD_EXPORT int weaver_wrap_ifdef(struct PatchList *patches,
                                          const struct TokenList *tokens,
                                          size_t start_idx, size_t end_idx,
                                          const char *condition,
                                          const char *false_code);

/**
 * @brief Safely append MSVC headers adjacent to existing POSIX headers.
 *
 * Scans the token stream for the last `#include` directive. It appends
 * the requested MSVC headers (e.g., `<windows.h>`) after it within an `#ifdef _MSC_VER` block.
 * Automatically injects `#define WIN32_LEAN_AND_MEAN` and `#define NOMINMAX`
 * before `<windows.h>`, and ensures `<winsock2.h>` precedes `<windows.h>`.
 *
 * @param[in,out] patches The patch list to populate.
 * @param[in] tokens The original token stream.
 * @param[in] include_windows_h True to include <windows.h>.
 * @param[in] include_winsock2_h True to include <winsock2.h>.
 * @return 0 on success, ENOMEM on allocation failure, EINVAL on invalid args.
 */
extern C_CDD_EXPORT int weaver_inject_msvc_headers(struct PatchList *patches,
                                                   const struct TokenList *tokens,
                                                   bool include_windows_h,
                                                   bool include_winsock2_h);

/**
 * @brief Transform a Variable Length Array (VLA) declaration into _alloca().
 *
 * Example: `int arr[n];` -> `int *arr = (int *)_alloca(n * sizeof(int));`
 * If the user runs the interactive review mode, they will be prompted.
 *
 * @param[in,out] patches The patch list to populate.
 * @param[in] tokens The original token stream.
 * @param[in] start_idx Inclusive start index of the VLA declaration tokens.
 * @param[in] end_idx Exclusive end index of the VLA declaration tokens.
 * @param[in] type_str The string representation of the array type (e.g. "int").
 * @param[in] var_name The name of the array variable.
 * @param[in] size_expr The string representation of the size expression (e.g. "n").
 * @param[in] interactive True if the user should be prompted before applying.
 * @return 0 on success, ENOMEM on allocation failure, EINVAL on invalid args.
 */
extern C_CDD_EXPORT int weaver_vla_to_alloca(struct PatchList *patches,
                                             const struct TokenList *tokens,
                                             size_t start_idx, size_t end_idx,
                                             const char *type_str,
                                             const char *var_name,
                                             const char *size_expr,
                                             bool interactive);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_WEAVER_H */
