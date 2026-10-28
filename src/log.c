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
 * @param[in] fmt Printf-style format string.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_INVALID_ARGUMENT if fmt is
 * NULL.
 */
C_CDD_EXPORT cdd_c_error_t c_cdd_log_debug(const char *fmt, ...) {
  if (!fmt) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
#ifdef DEBUG
  {
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
  }
#endif
  return CDD_C_SUCCESS;
}
