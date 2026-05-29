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
