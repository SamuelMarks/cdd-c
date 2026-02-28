#ifndef C_CDD_WIN_COMPAT_SYM_H
#define C_CDD_WIN_COMPAT_SYM_H

#if defined(_WIN32) && defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#include <string.h>
#define strtok_r strtok_s
#endif

#endif /* C_CDD_WIN_COMPAT_SYM_H */