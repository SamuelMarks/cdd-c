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

/**
 * @brief Cross-platform leak-free environment variable lookup that duplicates
 * the environment variable into an owned heap string (`*out_val`), or sets
 * `*out_val = NULL` if the variable is not set.
 *
 * @param[in] name Environment variable name.
 * @param[out] out_val Output pointer receiving a newly allocated string (caller
 * frees with `free`) or NULL if unset.
 * @return CDD_C_SUCCESS on success (including when variable is unset), or an
 * error code.
 */
#if defined(_MSC_VER)
static __inline cdd_c_error_t cdd_env_dup(const char *name, char **out_val) {
  char *buf = 0;
  size_t sz = 0;
  if (!name || !out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_val = 0;
  if (_dupenv_s(&buf, &sz, name) != 0)
    return CDD_C_ERROR_MEMORY;
  *out_val = buf;
  return CDD_C_SUCCESS;
}
#else
#if defined(__GNUC__) || defined(__clang__)
__attribute__((unused))
#endif
static cdd_c_error_t
cdd_env_dup(const char *name, char **out_val) {
  const char *v;
  size_t len;
  char *copy;
  if (!name || !out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_val = 0;
  v = getenv(name);
  if (!v)
    return CDD_C_SUCCESS;
  len = strlen(v);
  copy = (char *)malloc(len + 1);
  if (!copy)
    return CDD_C_ERROR_MEMORY;
  memcpy(copy, v, len + 1);
  *out_val = copy;
  return CDD_C_SUCCESS;
}
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
