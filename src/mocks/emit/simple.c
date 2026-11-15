/**
 * @file simple.c
 * @brief Mock cleanup implementations in simple.c.
 */
/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include <stdlib.h>

#include "simple.h"
#include "cdd_test_helpers/cdd_crt_bridge.h"
/* clang-format on */

/**
 * @brief Releases memory owned by a Haz instance.
 */
cdd_c_error_t Haz_cleanup(struct Haz *haz) {
  if (!haz)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  free(haz);
  return CDD_C_SUCCESS;
}

/**
 * @brief Releases memory owned by a Foo instance.
 */
cdd_c_error_t Foo_cleanup(struct Foo *foo) {
  cdd_c_error_t rc;
  if (foo == NULL)
    return CDD_C_SUCCESS;
  rc = Haz_cleanup(foo->haz);
  if (rc != CDD_C_SUCCESS)
    return rc;
  free(foo);
  return CDD_C_SUCCESS;
}
