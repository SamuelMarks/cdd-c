#if defined(__clang__) || defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverlength-strings"
#pragma GCC diagnostic ignored "-Wlong-long"
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif
/* clang-format off */
/* clang-format on */
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

#if defined(__clang__) || defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
