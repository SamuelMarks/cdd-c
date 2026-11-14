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
 * @param[out] out_val Optional output pointer receiving static buffer or NULL.
 * @return CDD_C_SUCCESS on success, or error code.
 */
static __inline cdd_c_error_t cdd_getenv_out(const char *name, char **out_val) {
  static char s_ring[8][512];
  static unsigned int s_idx = 0;
  static char *s_last = 0;
  char *buf = 0;
  size_t sz = 0;
  s_last = 0;
  if (!name) {
    if (out_val)
      *out_val = s_last;
    return CDD_C_SUCCESS;
  }
  if (_dupenv_s(&buf, &sz, name) == 0 && buf) {
    char *slot = s_ring[s_idx & 7u];
    s_idx++;
    strncpy_s(slot, sizeof(s_ring[0]), buf, _TRUNCATE);
    free(buf);
    s_last = slot;
  }
  if (out_val)
    *out_val = s_last;
  return CDD_C_SUCCESS;
}

/** @brief Scratch pointer storing the result of the most recent `getenv` call
 * on MSVC */
static char *g_cdd_msvc_last_getenv = 0;

#ifndef getenv
#define getenv(name)                                                           \
  (cdd_getenv_out((name), &g_cdd_msvc_last_getenv), g_cdd_msvc_last_getenv)
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
 * @brief Safe wrapper for fopen using fopen_s.
 * @param[in] path The path to the file.
 * @param[in] mode The mode string.
 * @param[out] out_f Output pointer receiving the opened FILE pointer, or NULL.
 * @return CDD_C_SUCCESS on success, or error code.
 */
static __inline cdd_c_error_t cdd_fopen_out(const char *path, const char *mode,
                                            FILE **out_f) {
  FILE *f = 0;
  if (!out_f)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_f = 0;
  if (!path || !mode)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (fopen_s(&f, path, mode) == 0 && f) {
    *out_f = f;
    return CDD_C_SUCCESS;
  }
  return CDD_C_ERROR_IO;
}

/** @brief Scratch pointer storing the result of the most recent `fopen` call on
 * MSVC */
static FILE *g_cdd_msvc_last_fopen = 0;

#ifndef fopen
#define fopen(path, mode)                                                      \
  (cdd_fopen_out((path), (mode), &g_cdd_msvc_last_fopen), g_cdd_msvc_last_fopen)
#endif

#endif /* defined(_MSC_VER) */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
