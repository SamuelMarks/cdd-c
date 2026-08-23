/**
 * @file no_discard.h
 * @brief Cross-platform compiler macro for the nodiscard attribute.
 *
 * This header provides a generic macro to mark functions and types such that
 * discarding their return values emits a compiler warning. It serves as a
 * compatibility layer across various C and C++ standards and compilers.
 */

#ifndef NO_DISCARD_H
#define NO_DISCARD_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @def NO_DISCARD
 * @brief Marks a return type or function such that discarding its return value
 * emits a compiler warning.
 *
 * This macro expands to the appropriate compiler-specific attribute or standard
 * directive based on the language version and compiler being used:
 * - C23 / C++17: Uses the standard `[[nodiscard]]` attribute.
 * - GCC / Clang: Uses `__attribute__((warn_unused_result))`.
 * - MSVC: Uses the `_Check_return_` SAL annotation.
 * - Fallback: Expands to nothing if unsupported.
 *
 * @par Example Usage on a typedef:
 * @code
 *   typedef enum NO_DISCARD { OK, ERR } my_error_t;
 * @endcode
 *
 * @par Example Usage on a function:
 * @code
 *   NO_DISCARD my_error_t do_something(void);
 * @endcode
 *
 * @note If NO_DISCARD is already defined prior to including this header,
 *       the existing definition will be preserved.
 */
#ifdef NO_DISCARD
/* pass */
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 202311L
/* C23 and later */
#define NO_DISCARD [[nodiscard]]
#elif defined(__cplusplus) && __cplusplus >= 201703L
/* C++17 and later */
#define NO_DISCARD [[nodiscard]]
#elif defined(__GNUC__) || defined(__clang__)
/* GCC / Clang */
#define NO_DISCARD __attribute__((warn_unused_result))
#elif defined(_MSC_VER) && _MSC_VER >= 1700
/* MSVC (Requires SAL) */
/* clang-format off */
#include <sal.h>
/* clang-format on */
#define NO_DISCARD _Check_return_
#else
/* Fallback for unsupported compilers */
#define NO_DISCARD
#endif

#ifdef __cplusplus
}
#endif

#endif /* NO_DISCARD_H */
