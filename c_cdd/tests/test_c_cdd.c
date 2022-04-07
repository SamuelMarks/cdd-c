#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS 1
#endif /* _MSC_VER */

#include <greatest.h>

#include "test_function.h"
#include "test_struct.h"

/* Add definitions that need to be in the test runner's main file. */
GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  /*RUN_SUITE(function_suite);*/
  RUN_SUITE(struct_suite);
  GREATEST_MAIN_END();
}
