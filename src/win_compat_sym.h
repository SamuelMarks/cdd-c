#ifndef C_CDD_WIN_COMPAT_SYM_H
#define C_CDD_WIN_COMPAT_SYM_H
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#if defined(_WIN32) && defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#if defined(_M_AMD64) && !defined(_AMD64_)
#define _AMD64_
#elif defined(_M_IX86) && !defined(_X86_)
#define _X86_
#elif defined(_M_ARM64) && !defined(_ARM64_)
#define _ARM64_
#elif defined(_M_ARM) && !defined(_ARM_)
#define _ARM_
#endif
/* clang-format off */
#include <string.h>
#if _MSC_VER < 1900
#define snprintf _snprintf
#endif
#endif

#if defined(_MSC_VER) && _MSC_VER < 1900
#define CDD_SIZE_T_FMT "Iu"
#define CDD_NUM_FORMAT "I64u"
#elif defined(_MSC_VER)
#define CDD_SIZE_T_FMT "zu"
#define CDD_NUM_FORMAT "llu"
#elif defined(__MINGW64__)
#define CDD_SIZE_T_FMT "llu"
#define CDD_NUM_FORMAT "llu"
#elif defined(__MINGW32__)
#define CDD_SIZE_T_FMT "u"
#define CDD_NUM_FORMAT "llu"
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#if !defined(_MSC_VER) || _MSC_VER >= 1800
#include <inttypes.h>
#endif
/* clang-format on */
#define CDD_SIZE_T_FMT "zu"
#define CDD_NUM_FORMAT PRIu64
#elif defined(__LP64__) || defined(_LP64) || defined(__wasm__) ||              \
    defined(__wasm32__)
#define CDD_SIZE_T_FMT "lu"
#define CDD_NUM_FORMAT "llu"
#else
#define CDD_SIZE_T_FMT "u"
#define CDD_NUM_FORMAT "llu"
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* C_CDD_WIN_COMPAT_SYM_H */
