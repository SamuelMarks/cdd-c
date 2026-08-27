#ifndef CDD_SAFE_CRT_MSVC_H
#define CDD_SAFE_CRT_MSVC_H

/**
 * @file safe_crt_msvc.h
 * @brief Provides MSVC Safe CRT wrappers and macros.
 */

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include <stdio.h>
#include <string.h>
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
#define getenv cdd_getenv
#define strdup _strdup
#define stricmp _stricmp
#define strnicmp _strnicmp
#define strcasecmp _stricmp
#define strncasecmp _strnicmp

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
#define fopen cdd_fopen

/** @brief Macro to replace sprintf with sprintf_s */
#define sprintf(dest, ...)                                                     \
  sprintf_s((dest), (sizeof(dest) == sizeof(void *) ? 1024 : sizeof(dest)),    \
            __VA_ARGS__)
/** @brief Macro to replace strcpy with strcpy_s */
#define strcpy(dest, src)                                                      \
  strcpy_s((dest), (sizeof(dest) == sizeof(void *) ? 1024 : sizeof(dest)),     \
           (src))
/** @brief Macro to replace strncpy with strncpy_s */
#define strncpy(dest, src, count)                                              \
  strncpy_s((dest), (sizeof(dest) == sizeof(void *) ? 1024 : sizeof(dest)),    \
            (src), (count))
/** @brief Macro to replace strcat with strcat_s */
#define strcat(dest, src)                                                      \
  strcat_s((dest), (sizeof(dest) == sizeof(void *) ? 1024 : sizeof(dest)),     \
           (src))
/** @brief Macro to replace strncat with strncat_s */
#define strncat(dest, src, count)                                              \
  strncat_s((dest), (sizeof(dest) == sizeof(void *) ? 1024 : sizeof(dest)),    \
            (src), (count))
/** @brief Macro to replace vsprintf with vsprintf_s */
#define vsprintf(dest, fmt, args)                                              \
  vsprintf_s((dest), (sizeof(dest) == sizeof(void *) ? 1024 : sizeof(dest)),   \
             (fmt), (args))

#endif

#ifdef __cplusplus
}
#endif

#endif
