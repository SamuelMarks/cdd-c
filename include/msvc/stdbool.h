/**
 * @file stdbool.h
 * @brief MSVC stdbool port.
 */
#ifndef _STDBOOL_H
#define _STDBOOL_H

/* clang-format off */
/* No includes required for C89 boolean port */
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#else
/** @brief C89 boolean type fallback */
typedef unsigned char bool;
/** @brief Boolean true constant */
#define true 1
/** @brief Boolean false constant */
#define false 0
#endif /* __cplusplus */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _STDBOOL_H */
