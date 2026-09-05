#ifndef CDD_SAFE_CRT_MSVC_H
#define CDD_SAFE_CRT_MSVC_H

/**
 * @file safe_crt_msvc.h
 * @brief Provides MSVC Safe CRT wrappers and macros.
 */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include <stdio.h>
#include <string.h>
#include "c_cdd/safe_crt.h"
#include <stdlib.h>
/* clang-format on */

#if defined(_MSC_VER)

/**
 * @brief Safely wrapper for getenv using _dupenv_s.
 * @param name The environment variable name.
 * @return The value of the environment variable, or NULL.
 */
static __inline char *cdd_getenv(const char *name) {
  char *buf = 0;
  size_t sz = 0;
  if (_dupenv_s(&buf, &sz, name) == 0 && buf) {
    return buf;
  }
  return 0;
}
#ifndef getenv
#define getenv cdd_getenv
#endif
#ifndef strdup
#ifndef strdup
#define strdup _strdup
#endif
#endif
#ifndef stricmp
#define stricmp _stricmp
#endif
#ifndef strnicmp
#define strnicmp _strnicmp
#endif
#ifndef strcasecmp
#define strcasecmp _stricmp
#endif
#ifndef strncasecmp
#define strncasecmp _strnicmp
#endif

/**
 * @brief Safely wrapper for fopen using fopen_s.
 * @param path The path to the file.
 * @param mode The mode string.
 * @return The opened FILE pointer, or NULL.
 */
static __inline FILE *cdd_fopen(const char *path, const char *mode) {
  FILE *f = 0;
  if (fopen_s(&f, path, mode) == 0) {
    return f;
  }
  return 0;
}
#ifndef fopen
#define fopen cdd_fopen
#endif

#if _MSC_VER < 1900
#else
#endif

#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
