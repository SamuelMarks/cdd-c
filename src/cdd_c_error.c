/* clang-format off */
#include "cdd_c_error.h"
#include <stddef.h>
/* clang-format on */

cdd_c_error_t cdd_c_strerror(cdd_c_error_t err, char **out) {
  if (out == NULL) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  switch (err) {
  case CDD_C_SUCCESS:
    *out = "Success";
    break;
  case CDD_C_ERROR_MEMORY:
    *out = "Memory allocation failed";
    break;
  case CDD_C_ERROR_INVALID_ARGUMENT:
    *out = "Invalid argument";
    break;
  case CDD_C_ERROR_IO:
    *out = "I/O error";
    break;
  case CDD_C_ERROR_SYSTEM:
    *out = "System error";
    break;
  case CDD_C_ERROR_NOT_FOUND:
    *out = "Not found";
    break;
  case CDD_C_ERROR_PARSE:
    *out = "Parse error";
    break;
  case CDD_C_ERROR_UNKNOWN:
  default:
    *out = "Unknown error";
    break;
  }
  return CDD_C_SUCCESS;
}
