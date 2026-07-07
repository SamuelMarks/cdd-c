/* clang-format off */
#include <stdlib.h>

#include "simple.h"
/* clang-format on */

enum cdd_c_error Haz_cleanup(struct Haz *const haz) {
  free(haz);
  return CDD_C_SUCCESS;
}

enum cdd_c_error Foo_cleanup(struct Foo *const foo) {
  if (foo == NULL)
    return CDD_C_SUCCESS;
  Haz_cleanup(foo->haz);
  free(foo);
  return CDD_C_SUCCESS;
}
