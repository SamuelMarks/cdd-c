#ifndef C_CDD_FORMAT_SPECIFIERS_H
#define C_CDD_FORMAT_SPECIFIERS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include <stddef.h>
/* clang-format on */

/**
 * @file format_specifiers.h
 * @brief Cross-platform format specifiers for printf and formatting strings.
 *
 * Provides macros like CDD_PRId64, CDD_PRIu64, CDD_PRIz to abstract over MSVC
 * vs GCC/Clang differences in formatting 64-bit integers and size_t.
 */

#if defined(_MSC_VER)
#define CDD_PRId64 "%I64d"
#define CDD_PRIu64 "%I64u"
#define CDD_PRIx64 "%I64x"
#if _MSC_VER < 1900
/* Older MSVC does not strictly support %zu for size_t. It uses %Iu */
#define CDD_PRIz "%Iu"
#else
#define CDD_PRIz "%zu"
#endif
#else
#define CDD_PRId64 "%lld"
#define CDD_PRIu64 "%llu"
#define CDD_PRIx64 "%llx"
#define CDD_PRIz "%zu"
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_FORMAT_SPECIFIERS_H */
