/* clang-format off */
#include <stdlib.h>

#include "simple.h"
/* clang-format on */

/** \brief func */
/**
 * @brief Executes the Haz cleanup operation.
 */
void Haz_cleanup(struct Haz *haz) { free(haz); }

/** \brief func */
/**
 * @brief Executes the Foo cleanup operation.
 */
void Foo_cleanup(struct Foo *foo) {
  if (foo == NULL)
    return;
  Haz_cleanup(foo->haz);
  free(foo);
}
