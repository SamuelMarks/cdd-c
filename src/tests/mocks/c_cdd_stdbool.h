/*
 * stdbool isn't always included with every popular C89 implementation
 *
 * This variant is modified from MUSL
 * */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#if (defined(_MSC_VER) && _MSC_VER >= 1800) || (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L)
#include <stdbool.h>
#else
#if !defined(_STDBOOL_H) && !defined(__STDBOOL_H) && !defined(HAS_STDBOOL) &&  \
    !defined(__cplusplus)
#define _STDBOOL_H
#define __STDBOOL_H
#define HAS_STDBOOL

#ifdef bool
#undef bool
#endif /* bool */

#ifdef true
#undef true
#endif /* true */

#ifdef false
#undef false
#endif /* false */

#include <stdlib.h>

#ifndef __cplusplus
typedef unsigned char _c_cdd_bool;
#define bool _c_cdd_bool
#define true 1
#define false 0
#endif
#endif /* !defined(_STDBOOL_H) && !defined(HAS_STDBOOL) */
#endif
/* clang-format on */

#ifdef __cplusplus
}
#endif /* __cplusplus */
