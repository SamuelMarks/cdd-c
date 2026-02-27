#include <greatest.h>

#include "test_simple_json.h"

/* Add definitions that need to be in the test runner's main file. */
GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(simple_mocks_suite);
  GREATEST_MAIN_END();
}
