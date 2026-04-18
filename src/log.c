#include "c_cdd/log.h"
#include <stdarg.h>
#include <stdio.h>

void c_cdd_log_debug(const char *fmt, ...) {
#ifdef DEBUG
  va_list args;
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);
#else
  /* Do nothing */
  (void)fmt;
#endif
}
