#ifndef C_CDD_FORMAT_SPECIFIERS_H
#define C_CDD_FORMAT_SPECIFIERS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include <stddef.h>
/* clang-format on */

#if defined(_MSC_VER)
#define CDD_PRId64 "I64d"
#define CDD_PRIu64 "I64u"
#define CDD_PRIx64 "I64x"
#if defined(_WIN64)
#define CDD_PRIz "I64u"
#else
#define CDD_PRIz "u"
#endif
#elif defined(__APPLE__)
#define CDD_PRId64 "lld"
#define CDD_PRIu64 "llu"
#define CDD_PRIx64 "llx"
#define CDD_PRIz "lu"
#else
#if (defined(__x86_64__) || defined(__ppc64__) || defined(__aarch64__) ||      \
     defined(__LP64__) || defined(_LP64)) &&                                   \
    !defined(_WIN32)
#define CDD_PRId64 "ld"
#define CDD_PRIu64 "lu"
#define CDD_PRIx64 "lx"
#else
#define CDD_PRId64 "lld"
#define CDD_PRIu64 "llu"
#define CDD_PRIx64 "llx"
#endif

#if defined(__x86_64__) || defined(__ppc64__) || defined(__aarch64__)
#if defined(_WIN32) /* MinGW64 */
#define CDD_PRIz "llu"
#else
#define CDD_PRIz "lu"
#endif
#elif defined(__wasm__) || defined(__wasm32__)
#define CDD_PRIz "lu"
#else
#define CDD_PRIz "u"
#endif
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_FORMAT_SPECIFIERS_H */
