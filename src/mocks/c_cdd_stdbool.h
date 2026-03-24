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

#ifdef __cplusplus
}
#endif /* __cplusplus */
