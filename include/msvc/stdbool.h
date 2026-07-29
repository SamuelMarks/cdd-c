/* clang-format off */
/* clang-format on */
#ifdef __cplusplus
extern "C" {
#endif
/**
 * @file stdbool.h
 * @brief MSVC stdbool port.
 */
#ifndef _STDBOOL_H
#define _STDBOOL_H

#ifndef __cplusplus
typedef unsigned char bool;
#define true 1
#define false 0
#endif

#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */
