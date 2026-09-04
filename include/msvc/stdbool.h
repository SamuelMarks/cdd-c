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
typedef unsigned char bool;
#define true 1
#define false 0
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
