#if defined(__clang__) || defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverlength-strings"
#pragma GCC diagnostic ignored "-Wlong-long"
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif
/* clang-format off */
#include "c_cdd/log.h"
#include <stdarg.h>
#include <stdio.h>
/* clang-format on */
cdd_c_error_t c_cdd_log_debug(const char *fmt, ...) {
#ifdef DEBUG
  va_list args;
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);
#else
  /* Do nothing */
  (void)fmt;
#endif
  return CDD_C_SUCCESS;
}

#if defined(__clang__) || defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
