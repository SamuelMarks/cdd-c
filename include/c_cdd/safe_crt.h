#ifndef C_CDD_SAFE_CRT_H
#define C_CDD_SAFE_CRT_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include <stdio.h>
#include <string.h>
/* clang-format on */

/**
 * @file safe_crt.h
 * @brief Cross-platform macros for MSVC Safe CRT functions.
 */

#if defined(_MSC_VER)
/** @brief Safe sprintf mapping for MSVC */
#define CDD_SNPRINTF sprintf_s
/** @brief Safe vsnprintf mapping for MSVC */
#define CDD_VSNPRINTF(buffer, sizeOfBuffer, format, argptr)                    \
  vsnprintf_s(buffer, sizeOfBuffer, _TRUNCATE, format, argptr)
/** @brief Safe strncpy mapping for MSVC */
#define CDD_STRNCPY(dest, dest_sz, src, count)                                 \
  strncpy_s(dest, dest_sz, src, count)
/** @brief Safe strcpy mapping for MSVC */
#define CDD_STRCPY(dest, dest_sz, src) strcpy_s(dest, dest_sz, src)
#else
/** @brief Safe sprintf mapping for POSIX/C99 */
#define CDD_SNPRINTF snprintf
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
