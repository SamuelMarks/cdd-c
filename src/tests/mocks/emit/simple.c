/* clang-format off */
#include <stdlib.h>

#include "simple.h"
/* clang-format on */

cdd_c_error_t Haz_cleanup(struct Haz *const haz) {
  if (!haz)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  free(haz);
  return CDD_C_SUCCESS;
}

cdd_c_error_t Foo_cleanup(struct Foo *const foo) {
  if (foo == NULL)
    return CDD_C_SUCCESS;
  {
    cdd_c_error_t rc = Haz_cleanup(foo->haz);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  free(foo);
  return CDD_C_SUCCESS;
}
