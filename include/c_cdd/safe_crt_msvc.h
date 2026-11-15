/**
 * @file safe_crt_msvc.h
 * @brief Provides MSVC Safe CRT wrappers and macros.
 */
#ifndef CDD_SAFE_CRT_MSVC_H
#define CDD_SAFE_CRT_MSVC_H

/* clang-format off */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "c_cdd/no_discard.h"
#include "c_cdd/safe_crt.h"
#include "cdd_c_error.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#if defined(_MSC_VER) || defined(CDD_TEST_SAFE_CRT_MSVC)

#if defined(_MSC_VER)
/** @brief Inline specifier for MSVC Safe CRT helper functions. */
#define CDD_SAFE_CRT_MSVC_INLINE static __inline
#else
/** @brief Static linkage specifier for non-MSVC unit testing of Safe CRT
 * helpers. */
#define CDD_SAFE_CRT_MSVC_INLINE static
#endif

#ifndef CDD_SAFE_CRT_DUPENV_S
/** @brief Default Safe CRT environment duplication primitive on MSVC. */
#define CDD_SAFE_CRT_DUPENV_S(pbuf, psz, name) _dupenv_s((pbuf), (psz), (name))
#endif

#ifndef CDD_SAFE_CRT_STRNCPY_S
/** @brief Default Safe CRT bounded string copy primitive on MSVC. */
#define CDD_SAFE_CRT_STRNCPY_S(dst, cap, src)                                  \
  strncpy_s((dst), (cap), (src), _TRUNCATE)
#endif

#ifndef CDD_SAFE_CRT_FREE
/** @brief Default heap release primitive for `_dupenv_s` buffers. */
#define CDD_SAFE_CRT_FREE(ptr) free((ptr))
#endif

#ifndef CDD_SAFE_CRT_FOPEN_S
/** @brief Default Safe CRT file open primitive on MSVC. */
#define CDD_SAFE_CRT_FOPEN_S(pf, path, mode) fopen_s((pf), (path), (mode))
#endif

/**
 * @brief Leak-free wrapper for `getenv` using `_dupenv_s` and a static ring
 * buffer so callers expecting standard C89 non-owning `getenv` semantics do not
 * leak heap memory on MSVC.
 *
 * @param[in] name The environment variable name.
 * @param[out] out_val Optional output pointer receiving static buffer or NULL.
 * @return CDD_C_SUCCESS on success, or error code.
 */
CDD_SAFE_CRT_MSVC_INLINE NO_DISCARD cdd_c_error_t
cdd_getenv_out(const char *name, char **out_val) {
  static char s_ring[8][512];
  static unsigned int s_idx = 0;
  char *s_last = 0;
  char *buf = 0;
  size_t sz = 0;

  if (!name) {
    if (out_val)
      *out_val = 0;
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
  if (CDD_SAFE_CRT_DUPENV_S(&buf, &sz, name) != 0) {
    if (out_val)
      *out_val = 0;
    return CDD_C_ERROR_MEMORY;
  }
  if (buf) {
    char *slot = s_ring[s_idx & 7u];
    s_idx++;
    CDD_SAFE_CRT_STRNCPY_S(slot, sizeof(s_ring[0]), buf);
    CDD_SAFE_CRT_FREE(buf);
    s_last = slot;
  }
  if (out_val)
    *out_val = s_last;
  return CDD_C_SUCCESS;
}

/**
 * @brief Safe wrapper for `fopen` using `fopen_s`.
 *
 * @param[in] path The path to the file.
 * @param[in] mode The mode string.
 * @param[out] out_f Output pointer receiving the opened FILE pointer, or NULL.
 * @return CDD_C_SUCCESS on success, or error code.
 */
CDD_SAFE_CRT_MSVC_INLINE NO_DISCARD cdd_c_error_t
cdd_fopen_out(const char *path, const char *mode, FILE **out_f) {
  FILE *f = 0;
  if (!out_f)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_f = 0;
  if (!path || !mode)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (CDD_SAFE_CRT_FOPEN_S(&f, path, mode) == 0 && f) {
    *out_f = f;
    return CDD_C_SUCCESS;
  }
  return CDD_C_ERROR_IO;
}

#endif /* defined(_MSC_VER) || defined(CDD_TEST_SAFE_CRT_MSVC) */

#if defined(_MSC_VER)

/** @brief Scratch pointer storing the result of the most recent `getenv` call
 * on MSVC. */
static char *g_cdd_msvc_last_getenv = 0;

#ifndef getenv
/** @brief Redirects `getenv` to leak-free Safe CRT `cdd_getenv_out` on MSVC. */
#define getenv(name)                                                           \
  ((cdd_getenv_out((name), &g_cdd_msvc_last_getenv) == CDD_C_SUCCESS)          \
       ? g_cdd_msvc_last_getenv                                                \
       : (char *)0)
#endif
#ifndef strdup
/** @brief Maps POSIX `strdup` to ISO-conformant `_strdup` on MSVC. */
#define strdup _strdup
#endif
#ifndef stricmp
/** @brief Maps `stricmp` to `_stricmp` on MSVC. */
#define stricmp _stricmp
#endif
#ifndef strnicmp
/** @brief Maps `strnicmp` to `_strnicmp` on MSVC. */
#define strnicmp _strnicmp
#endif
#ifndef strcasecmp
/** @brief Maps POSIX `strcasecmp` to `_stricmp` on MSVC. */
#define strcasecmp _stricmp
#endif
#ifndef strncasecmp
/** @brief Maps POSIX `strncasecmp` to `_strnicmp` on MSVC. */
#define strncasecmp _strnicmp
#endif

/** @brief Scratch pointer storing the result of the most recent `fopen` call on
 * MSVC. */
static FILE *g_cdd_msvc_last_fopen = 0;

#ifndef fopen
/** @brief Redirects `fopen` to Safe CRT `cdd_fopen_out` on MSVC. */
#define fopen(path, mode)                                                      \
  ((cdd_fopen_out((path), (mode), &g_cdd_msvc_last_fopen) == CDD_C_SUCCESS)    \
       ? g_cdd_msvc_last_fopen                                                 \
       : (FILE *)0)
#endif

#endif /* defined(_MSC_VER) */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CDD_SAFE_CRT_MSVC_H */
