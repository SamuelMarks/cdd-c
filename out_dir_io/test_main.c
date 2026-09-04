#include "test_io.h"
#include <greatest.h>

GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(enums_suite);
  RUN_SUITE(structs_suite);
  GREATEST_MAIN_END();
}
