/**
 * @file test_simple_json.c
 * @brief Test runner for simple json tests.
 */

/* clang-format off */
#include <greatest.h>

#include "test_simple_json.h"
/* clang-format on */

/* Add definitions that need to be in the test runner's main file. */
GREATEST_MAIN_DEFS();
#if defined(_MSC_VER)
#pragma warning(disable : 4551)
#endif

/**
 * @brief Main entry point for the test runner.
 *
 * @param[in] argc Argument count.
 * @param[in] argv Argument values.
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on failure.
 */
int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(simple_mocks_suite);
  GREATEST_MAIN_END();
}
