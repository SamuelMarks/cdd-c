/**
 * @file main.c
 * @brief Main mock runner for simple JSON serialization testing.
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"
#include "c_cdd/suppress_gui.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "c_cdd_export.h"
#include "cdd_c_error.h"
#include "../emit/simple_json.h"
#include "simple_mocks_export.h"
#include "../../cdd_test_helpers/cdd_crt_bridge.h"
/* clang-format on */

/** @brief Test definition g_simple_json_fail_alloc. */
extern SIMPLE_MOCKS_EXPORT int g_simple_json_fail_alloc;

/**
 * @brief Entry point for the JSON serialization mock runner.
 *
 * @param[in] argc Argument count.
 * @param[in] argv Argument array.
 * @return EXIT_SUCCESS on success, error code otherwise.
 */
int main(int argc, char **argv) {
  cdd_c_error_t rc = 0;

  rc = cdd_suppress_gui_popups();
  if (rc != CDD_C_SUCCESS)
    return (int)rc;
  rc = c_cdd_crt_init_runtime();
  if (rc != CDD_C_SUCCESS)
    return (int)rc;
#if defined(_MSC_VER)
  json_set_allocation_functions(test_crt_malloc, c_cdd_crt_free);
#endif

  if (argc > 1 && strcmp(argv[1], "--fail-alloc") == 0) {
    g_simple_json_fail_alloc = 1;
  }

  rc = run_mocks_test();
  if (rc != CDD_C_SUCCESS)
    return (int)rc;

  return EXIT_SUCCESS;
}
