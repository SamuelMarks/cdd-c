/* clang-format off */#include "c_cdd/safe_crt_msvc.h"

#include <stdlib.h>

#include "simple.h"
/* clang-format on */

cdd_c_error_t Haz_cleanup(struct Haz *haz) {
  if (!haz)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  /* wait, if haz is a struct Haz, and its members need freeing? In simple.c it
   * just frees haz */
  free(haz);
  return CDD_C_SUCCESS;
}

cdd_c_error_t Foo_cleanup(struct Foo *foo) {
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
