#ifndef C_CDD_C_CDD_C_CDD_UTILS_H
#define C_CDD_C_CDD_C_CDD_UTILS_H

#ifdef __cplusplus
#include <cstdbool>
#include <cstdlib>
#include <cstring>
extern "C" {
#else
#include <stdlib.h>
#include <string.h>

#if __STDC_VERSION__ >= 199901L
#include <stdbool.h>
#else
#include <c_cdd_stdbool.h>
#endif /* __STDC_VERSION__ >= 199901L */

#endif /* __cplusplus */

#if defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__)
#include <BaseTsd.h>
#include <errno.h>
typedef SSIZE_T ssize_t;
#else
#include <sys/errno.h>
#endif
#include "c_cdd_export.h"

extern C_CDD_EXPORT void print_escaped(const char *name, char *s);

#endif /* C_CDD_C_CDD_C_CDD_UTILS_H */
