/* clang-format off */
#include <stdlib.h>

#include "simple.h"
/* clang-format on */

/** \brief func */
/**
 * @brief Executes the Haz cleanup operation.
 */
enum cdd_c_error Haz_cleanup(struct Haz *haz) {
  free(haz);
  return CDD_C_SUCCESS;
}

/** \brief func */
/**
 * @brief Executes the Foo cleanup operation.
 */
enum cdd_c_error Foo_cleanup(struct Foo *foo) {
  if (foo == NULL)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  (void)Haz_cleanup(foo->haz);
  free(foo);
  return CDD_C_SUCCESS;
}
