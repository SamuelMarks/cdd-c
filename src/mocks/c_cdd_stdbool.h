/**
 * @file c_cdd_stdbool.h
 * @brief Portable boolean type definition for C89 compliance.
 *
 * Provides logical types 'bool', 'true', and 'false' similar to C99 stdint.h,
 * adjusted for older compiler storage classes.
 *
 * @author Samuel Marks
 */

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

/**
 * @brief Boolean type alias.
 * Using unsigned char for compatibility with other headers like c_orm.
 */
typedef unsigned char _c_cdd_bool;
#define bool _c_cdd_bool

/**
 * @brief Logical true.
 */
#define true 1

/**
 * @brief Logical false.
 * Defined as negation of true to ensure logical consistency.
 */
#define false 0

#endif /* !defined(_STDBOOL_H) && !defined(HAS_STDBOOL) */
#endif
/* clang-format on */

#ifdef __cplusplus
}
#endif /* __cplusplus */
