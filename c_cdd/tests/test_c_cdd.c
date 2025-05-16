#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS 1
#endif /* _MSC_VER */

#include <greatest.h>

#include "test_codegen.h"
#include "test_dataclasses.h"
#include "test_json_from_and_to.h"
#include "test_parsing.h"

/* Add definitions that need to be in the test runner's main file. */
GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(codegen_suite);
  RUN_SUITE(dataclasses_suite);
  RUN_SUITE(json_from_and_to_suite);
  RUN_SUITE(parsing_suite);
  GREATEST_MAIN_END();
}
