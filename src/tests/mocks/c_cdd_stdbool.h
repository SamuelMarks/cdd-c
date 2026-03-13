/*
 * stdbool isn't always included with every popular C89 implementation
 *
 * This variant is modified from MUSL
 * */

#if !defined(_STDBOOL_H) && !defined(HAS_STDBOOL)
#define _STDBOOL_H

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
typedef size_t bool;
#define true 1
#define false (!true)
#endif

#endif /* !defined(_STDBOOL_H) && !defined(HAS_STDBOOL) */
