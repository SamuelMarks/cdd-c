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
#include <string.h>
#endif

#endif /* C_CDD_WIN_COMPAT_SYM_H */
