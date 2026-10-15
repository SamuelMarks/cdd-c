/**
 * @file stdbool.h
 * @brief MSVC stdbool port.
 */
#ifndef _STDBOOL_H
#define _STDBOOL_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#ifndef __cplusplus
/** @brief C89 boolean type fallback */
typedef unsigned char bool;
/** @brief Boolean true constant */
#define true 1
/** @brief Boolean false constant */
#define false 0
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
