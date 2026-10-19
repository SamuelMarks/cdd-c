/**
 * @file log.c
 * @brief Core log definitions and implementation.
 */
/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "c_cdd/log.h"
#include <stdarg.h>
#include <stdio.h>
/* clang-format on */

/**
 * @brief Logs a debug message to stderr when DEBUG is enabled.
 *
 * @return CDD_C_SUCCESS on success.
 */
C_CDD_EXPORT cdd_c_error_t c_cdd_log_debug(const char *fmt, ...) {
#ifdef DEBUG
  va_list args;
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);
  return CDD_C_SUCCESS;
#else
  return (cdd_c_error_t)((size_t)fmt & 0);
#endif
}
