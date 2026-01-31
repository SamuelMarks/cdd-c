/**
 * @file c_cdd_stdbool.h
 * @brief Portable boolean type definition for C89 compliance.
 *
 * Provides logical types 'bool', 'true', and 'false' similar to C99 stdint.h,
 * adjusted for older compiler storage classes.
 *
 * @author Samuel Marks
 */

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

#include <stdlib.h>

/**
 * @brief Boolean type alias.
 * Using size_t ensures alignment with machine word size for efficient register
 * usage.
 */
typedef size_t bool;

/**
 * @brief Logical true.
 */
#define true 1

/**
 * @brief Logical false.
 * Defined as negation of true to ensure logical consistency.
 */
#define false (!true)

#endif /* !defined(_STDBOOL_H) && !defined(HAS_STDBOOL) */
