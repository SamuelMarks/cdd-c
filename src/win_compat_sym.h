#ifndef C_CDD_WIN_COMPAT_SYM_H
#define C_CDD_WIN_COMPAT_SYM_H

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

#if defined(_MSC_VER)
#define SIZE_T_FMT "Iu"
#define NUM_FORMAT "I64u"
#elif defined(__MINGW64__)
#define SIZE_T_FMT "llu"
#define NUM_FORMAT "llu"
#elif defined(__MINGW32__)
#define SIZE_T_FMT "u"
#define NUM_FORMAT "llu"
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
#include <inttypes.h>
/* clang-format on */
#define SIZE_T_FMT "zu"
#define NUM_FORMAT PRIu64
#elif defined(__LP64__) || defined(_LP64)
#define SIZE_T_FMT "lu"
#define NUM_FORMAT "lu"
#else
#define SIZE_T_FMT "u"
#define NUM_FORMAT "llu"
#endif

#endif /* C_CDD_WIN_COMPAT_SYM_H */
