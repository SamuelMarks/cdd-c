#ifndef C_CDD_STR_INCLUDES_H
#define C_CDD_STR_INCLUDES_H

#if defined(_BSD_SOURCE) || defined(_GNU_SOURCE) || defined(HAVE_ASPRINTF)
/* clang-format off */
#include <stdio.h>
#else
#include <c89stringutils_string_extras.h>
/* clang-format on */
#endif

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#define NUM_LONG_FMT "z"
#else
#define NUM_LONG_FMT "l"
#endif /* defined(WIN32) || defined(_WIN32) || defined(__WIN32__) ||           \
          defined(__NT__) */

#endif /* !C_CDD_STR_INCLUDES_H */
