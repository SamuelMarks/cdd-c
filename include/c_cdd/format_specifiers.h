/**
 * @file format_specifiers.h
 * @brief Core format specifiers definitions and implementation.
 */
#ifndef C_CDD_FORMAT_SPECIFIERS_H
#define C_CDD_FORMAT_SPECIFIERS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/* clang-format off */
#include <stddef.h>
/* clang-format on */

#if defined(_MSC_VER) ||                                                       \
    (defined(_WIN32) &&                                                        \
     (!defined(__USE_MINGW_ANSI_STDIO) || (__USE_MINGW_ANSI_STDIO + 0) == 0))
/** @brief 64-bit signed integer printf format specifier */
#define CDD_PRId64 "I64d"
/** @brief 64-bit unsigned integer printf format specifier */
#define CDD_PRIu64 "I64u"
/** @brief 64-bit hexadecimal integer printf format specifier */
#define CDD_PRIx64 "I64x"
#if defined(_WIN64)
/** @brief size_t printf format specifier */
#define CDD_PRIz "I64u"
#else
/** @brief size_t printf format specifier */
#define CDD_PRIz "u"
#endif
#elif defined(__APPLE__)
/** @brief 64-bit signed integer printf format specifier */
#define CDD_PRId64 "lld"
/** @brief 64-bit unsigned integer printf format specifier */
#define CDD_PRIu64 "llu"
/** @brief 64-bit hexadecimal integer printf format specifier */
#define CDD_PRIx64 "llx"
/** @brief size_t printf format specifier */
#define CDD_PRIz "lu"
#else
#if (defined(__x86_64__) || defined(__ppc64__) || defined(__aarch64__) ||      \
     defined(__LP64__) || defined(_LP64)) &&                                   \
    !defined(_WIN32)
/** @brief 64-bit signed integer printf format specifier */
#define CDD_PRId64 "ld"
/** @brief 64-bit unsigned integer printf format specifier */
#define CDD_PRIu64 "lu"
/** @brief 64-bit hexadecimal integer printf format specifier */
#define CDD_PRIx64 "lx"
#else
/** @brief 64-bit signed integer printf format specifier */
#define CDD_PRId64 "lld"
/** @brief 64-bit unsigned integer printf format specifier */
#define CDD_PRIu64 "llu"
/** @brief 64-bit hexadecimal integer printf format specifier */
#define CDD_PRIx64 "llx"
#endif

#if defined(__x86_64__) || defined(__ppc64__) || defined(__aarch64__)
#if defined(_WIN32) /* MinGW64 */
/** @brief size_t printf format specifier */
#define CDD_PRIz "llu"
#else
/** @brief size_t printf format specifier */
#define CDD_PRIz "lu"
#endif
#elif defined(__wasm__) || defined(__wasm32__)
/** @brief size_t printf format specifier */
#define CDD_PRIz "lu"
#else
/** @brief size_t printf format specifier */
#define CDD_PRIz "u"
#endif
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_FORMAT_SPECIFIERS_H */
