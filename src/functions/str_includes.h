/**
 * @file str_includes.h
 * @brief Core str includes definitions and implementation.
 */
#ifndef C_CDD_STR_INCLUDES_H
#define C_CDD_STR_INCLUDES_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#if defined(_BSD_SOURCE) || defined(_GNU_SOURCE) || defined(HAVE_ASPRINTF)
/* clang-format off */
#include <stdio.h>
#else
#include <c89stringutils_string_extras.h>
/* clang-format on */
#endif

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
/** @brief Length modifier for size_t / long formatting */
#define NUM_LONG_FMT "z"
#else
/** @brief Length modifier for size_t / long formatting */
#define NUM_LONG_FMT "l"
#endif /* defined(WIN32) || defined(_WIN32) || defined(__WIN32__) ||           \
          defined(__NT__) */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !C_CDD_STR_INCLUDES_H */
