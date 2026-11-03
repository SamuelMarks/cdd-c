#ifndef CDD_SAFE_CRT_MSVC_H
#define CDD_SAFE_CRT_MSVC_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/**
 * @file safe_crt_msvc.h
 * @brief Provides MSVC Safe CRT wrappers and macros.
 */

/* clang-format off */
#include <stdio.h>
#include <string.h>
#include "c_cdd/safe_crt.h"
#include "cdd_c_error.h"
#include <stdlib.h>
/* clang-format on */

#if defined(_MSC_VER)

/**
 * @brief Leak-free wrapper for `getenv` using `_dupenv_s` and a static ring
 * buffer so callers expecting standard C89 non-owning `getenv` semantics do not
 * leak heap memory on MSVC.
 *
 * @param[in] name The environment variable name.
 * @return Pointer to static buffer containing the value, or NULL if unset.
 */
static __inline char *cdd_getenv(const char *name) {
  static char s_ring[8][512];
  static unsigned int s_idx = 0;
  char *buf = 0;
  size_t sz = 0;
  if (!name)
    return 0;
  if (_dupenv_s(&buf, &sz, name) == 0 && buf) {
    char *slot = s_ring[s_idx & 7u];
    s_idx++;
    strncpy_s(slot, sizeof(s_ring[0]), buf, _TRUNCATE);
    free(buf);
    return slot;
  }
  return 0;
}
#ifndef getenv
#define getenv cdd_getenv
#endif
#ifndef strdup
#define strdup _strdup
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
 * @param[in] path The path to the file.
 * @param[in] mode The mode string.
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

#endif /* defined(_MSC_VER) */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
