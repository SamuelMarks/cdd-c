/**
 * @file simple.c
 * @brief Unit tests and mocks in simple.c.
 */
/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "c_cdd/memory.h"
#include <stdlib.h>

#include "simple.h"
/* clang-format on */

/**
 * @brief Releases memory owned by a Haz instance.
 * @return Result status or value.
 */
enum cdd_c_error Haz_cleanup(struct Haz *haz) {
  C_CDD_FREE(haz);
  return CDD_C_SUCCESS;
}

/**
 * @brief Releases memory owned by a Foo instance.
 * @return Result status or value.
 */
enum cdd_c_error Foo_cleanup(struct Foo *foo) {
  enum cdd_c_error rc;
  if (foo == NULL)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = Haz_cleanup(foo->haz);
  if (rc != CDD_C_SUCCESS)
    return rc;
  C_CDD_FREE(foo);
  return CDD_C_SUCCESS;
}
