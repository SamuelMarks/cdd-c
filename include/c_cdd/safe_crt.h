#ifndef C_CDD_SAFE_CRT_H
#define C_CDD_SAFE_CRT_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/* clang-format off */
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
/* clang-format on */

/**
 * @file safe_crt.h
 * @brief Cross-platform macros for MSVC Safe CRT functions.
 */

#if defined(_MSC_VER)
/**
 * @brief Truncating MSVC Safe CRT `snprintf` wrapper using `vsnprintf_s` with
 * `_TRUNCATE` so output exceeding `sz - 1` safely truncates instead of invoking
 * the invalid parameter handler.
 *
 * @param[out] buf Destination character buffer.
 * @param[in] sz Capacity of `buf` in bytes.
 * @param[in] fmt `printf`-style format string.
 * @return Number of characters written (clamped to `sz - 1` on truncation), or
 * required length when `buf == NULL` or `sz == 0`.
 */
static __inline int cdd_msvc_snprintf(char *buf, size_t sz, const char *fmt,
                                      ...) {
  int n;
  va_list ap;
  if (!buf || sz == 0) {
    va_start(ap, fmt);
    n = _vscprintf(fmt, ap);
    va_end(ap);
    return n;
  }
  va_start(ap, fmt);
  n = vsnprintf_s(buf, sz, _TRUNCATE, fmt, ap);
  va_end(ap);
  return (n < 0) ? (int)(sz - 1) : n;
}

/** @brief Safe sprintf mapping for MSVC */
#define CDD_SNPRINTF cdd_msvc_snprintf
/** @brief Safe strcat mapping for MSVC */
#define CDD_STRCAT(dest, sz, src) strncat_s((dest), (sz), (src), _TRUNCATE)
/** @brief Safe strncat mapping for MSVC */
#define CDD_STRNCAT strncat_s
/** @brief Safe vsnprintf mapping for MSVC */
#define CDD_VSNPRINTF(buffer, sizeOfBuffer, format, argptr)                    \
  vsnprintf_s(buffer, sizeOfBuffer, _TRUNCATE, format, argptr)
/** @brief Safe strncpy mapping for MSVC */
#define CDD_STRNCPY(dest, dest_sz, src, count)                                 \
  strncpy_s(dest, dest_sz, src, count)
/** @brief Safe strcpy mapping for MSVC */
#define CDD_STRCPY(dest, dest_sz, src)                                         \
  strncpy_s((dest), (dest_sz), (src), _TRUNCATE)
#else
/** @brief Safe sprintf mapping for POSIX/C99 */
#define CDD_SNPRINTF snprintf
/** @brief Safe strcat mapping for POSIX/C99 */
#define CDD_STRCAT(dest, sz, src) strcat(dest, src)
/** @brief Safe strncat mapping for POSIX/C99 */
#define CDD_STRNCAT(dest, sz, src, n) strncat(dest, src, n)
/** @brief Safe vsnprintf mapping for POSIX/C99 */
#define CDD_VSNPRINTF(buffer, sizeOfBuffer, format, argptr)                    \
  vsnprintf(buffer, sizeOfBuffer, format, argptr)
/** @brief Safe strncpy mapping for POSIX/C99 */
#define CDD_STRNCPY(dest, dest_sz, src, count) strncpy(dest, src, count)
/** @brief Safe strcpy mapping for POSIX/C99 */
#define CDD_STRCPY(dest, dest_sz, src) strcpy(dest, src)
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_SAFE_CRT_H */
