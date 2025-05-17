#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS 1
#endif /* _MSC_VER */

#include <greatest.h>

#include "test_code2schema.h"
#include "test_cst_parser.h"
#include "test_dataclasses.h"
#include "test_fs.h"
#include "test_json_from_and_to.h"
#include "test_parsing.h"
#include "test_schema2tests.h"
#include "test_schema_codegen.h"
#include "test_sync_code.h"
#include "test_tokenizer.h"

/* Add definitions that need to be in the test runner's main file. */
GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(code2schema_suite);
  RUN_SUITE(cst_parser_suite);
  RUN_SUITE(dataclasses_suite);
  RUN_SUITE(fs_suite);
  RUN_SUITE(json_from_and_to_suite);
  RUN_SUITE(parsing_suite);
  RUN_SUITE(schema2tests_suite);
  RUN_SUITE(schema_codegen_suite);
  RUN_SUITE(sync_code_suite);
  RUN_SUITE(tokenizer_suite);
  GREATEST_MAIN_END();
}
