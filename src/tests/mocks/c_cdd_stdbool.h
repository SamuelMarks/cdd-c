/*
 * stdbool isn't always included with every popular C89 implementation
 *
 * This variant is modified from MUSL
 * */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#if !defined(_STDBOOL_H) && !defined(HAS_STDBOOL) && !defined(__cplusplus)
#define _STDBOOL_H
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

/* clang-format off */
#include <stdlib.h>
/* clang-format on */

#ifndef __cplusplus
typedef unsigned char _c_cdd_bool;
#define bool _c_cdd_bool
#define true 1
#define false 0
#endif
#endif /* !defined(_STDBOOL_H) && !defined(HAS_STDBOOL) */

#ifdef __cplusplus
}
#endif /* __cplusplus */
