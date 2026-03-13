/* clang-format off */
#include <greatest.h>

#include "test_simple_json.h"
/* clang-format on */

/* Add definitions that need to be in the test runner's main file. */
GREATEST_MAIN_DEFS();
#if defined(_MSC_VER)
#pragma warning(disable : 4551)
#endif

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(simple_mocks_suite);
  GREATEST_MAIN_END();
}
