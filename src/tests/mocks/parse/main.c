#include "cdd_c_error.h"
/**
 * @file main.c
 * @brief Main mock runner for simple JSON serialization testing.
 */

/* clang-format off */
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__)
#else
#include <errno.h>
#endif

#include "../emit/simple_json.h"
/* clang-format on */

static const char *const haz_e_mock0 =
    "{\"bzr\": \"some_bzr\",\"tank\": \"SMALL\"}";
static const char *const foo_e_mock0 =
    "{\"bar\": \"some_bar\",\"can\": 5,\"haz\":{\"bzr\": "
    "\"some_bzr\",\"tank\": \"SMALL\"}}";

/**
 * @brief Entry point for the JSON serialization mock runner.
 *
 * @return EXIT_SUCCESS on success, error code otherwise.
 */
/* LCOV_EXCL_START */
int main(void) {
  const enum Tank t = Tank_BIG;
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  char *tank_as_str = NULL;
  struct HazE haz_e = {"some_bzr", Tank_SMALL};
  struct FooE foo_e = {"some_bar", 5, NULL};
  char *haz_e_json = NULL;
  char *foo_e_json = NULL;
  struct HazE *haz_e0 = (struct HazE *)malloc(sizeof(*haz_e0));
  struct FooE *foo_e0 = (struct FooE *)malloc(sizeof(*foo_e0));
  if (!haz_e0 || !foo_e0)
    return CDD_C_ERROR_MEMORY;
  /* LCOV_EXCL_STOP */
  (void)t;
  /* LCOV_EXCL_START */
  foo_e.haz = &haz_e;
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  if (haz_e0 == NULL || foo_e0 == NULL)
    return CDD_C_ERROR_MEMORY;
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  assert(Tank_to_str(t, &tank_as_str) == CDD_C_SUCCESS);
  assert(strcmp(tank_as_str, "BIG") == 0);
  free(tank_as_str);
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  assert(HazE_to_json(&haz_e, &haz_e_json) == CDD_C_SUCCESS);
  assert(strcmp(haz_e_json, haz_e_mock0) == 0);
  free(haz_e_json);
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  assert(HazE_from_json(haz_e_mock0, &haz_e0) == CDD_C_SUCCESS);
  assert(HazE_eq(haz_e0, &haz_e) == CDD_C_SUCCESS);
  free(haz_e0);
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  assert(FooE_to_json(&foo_e, &foo_e_json) == CDD_C_SUCCESS);
  assert(strcmp(foo_e_json, foo_e_mock0) == 0);
  free(foo_e_json);
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  assert(FooE_from_json(foo_e_mock0, &foo_e0) == CDD_C_SUCCESS);
  assert(FooE_eq(foo_e0, &foo_e) == CDD_C_SUCCESS);
  free(foo_e0);
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  return EXIT_SUCCESS;
  /* LCOV_EXCL_STOP */
}
