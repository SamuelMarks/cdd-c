#if defined(__clang__) || defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverlength-strings"
#pragma GCC diagnostic ignored "-Wlong-long"
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif
/* clang-format off */
#include "c_cdd/memory.h"
#include <stdlib.h>

#include "simple.h"
/* clang-format on */

/** \brief func */
/**
 * @brief Executes the Haz cleanup operation.
 */
enum cdd_c_error Haz_cleanup(struct Haz *haz) {
  C_CDD_FREE(haz);
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
  C_CDD_FREE(foo);
  return CDD_C_SUCCESS;
}

#if defined(__clang__) || defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
