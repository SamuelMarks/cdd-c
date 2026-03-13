/* clang-format off */
#ifndef TEST_MAIN_H
#define TEST_MAIN_H

#include "functions/parse/main.h"
#include "greatest.h"
/* clang-format on */

TEST test_main_no_args(void) {
  char *argv[] = {"cdd-c"};
  int rc = cdd_main(1, argv);
  ASSERT_EQ_FMT(EXIT_FAILURE, rc, "%d");
  PASS();
}

TEST test_main_help(void) {
  char *argv[] = {"cdd-c", "--help"};
  int rc = cdd_main(2, argv);
  ASSERT_EQ_FMT(EXIT_SUCCESS, rc, "%d");
  PASS();
}

TEST test_main_version(void) {
  char *argv[] = {"cdd-c", "--version"};
  int rc = cdd_main(2, argv);
  ASSERT_EQ_FMT(EXIT_SUCCESS, rc, "%d");
  PASS();
}

TEST test_main_invalid_command(void) {
  char *argv[] = {"cdd-c", "unknown_command"};
  int rc = cdd_main(2, argv);
  ASSERT_EQ_FMT(EXIT_FAILURE, rc, "%d");
  PASS();
}

TEST test_main_subcommands(void) {
  char *argv_c2openapi[] = {"cdd-c", "c2openapi", "dir", "out.json"};
  char *argv_code2schema[] = {"cdd-c", "code2schema", "header.h",
                              "schema.json"};
  char *argv_generate_build[] = {"cdd-c", "generate_build_system", "cmake",
                                 "out", "name"};
  char *argv_schema2code[] = {"cdd-c", "schema2code", "schema.json", "out"};
  char *argv_jsonschema2tests[] = {"cdd-c", "jsonschema2tests", "schema.json",
                                   "hdr.h", "out.h"};
  char *argv_audit[] = {"cdd-c", "audit", "dir"};
  char *argv_to_openapi[] = {"cdd-c", "to_openapi", "-f", "dir"};
  char *argv_to_docs[] = {"cdd-c", "to_docs_json", "-i", "spec.json"};
  char *argv_from_openapi[] = {"cdd-c", "from_openapi", "to_sdk", "-i",
                               "spec.json"};

  cdd_main(4, argv_c2openapi);
  cdd_main(4, argv_code2schema);
  cdd_main(5, argv_generate_build);
  cdd_main(4, argv_schema2code);
  cdd_main(5, argv_jsonschema2tests);
  cdd_main(3, argv_audit);
  cdd_main(4, argv_to_openapi);
  cdd_main(4, argv_to_docs);
  cdd_main(5, argv_from_openapi);

  PASS();
}

SUITE(main_suite) {
  RUN_TEST(test_main_no_args);
  RUN_TEST(test_main_help);
  RUN_TEST(test_main_version);
  RUN_TEST(test_main_invalid_command);
  RUN_TEST(test_main_subcommands);
}
#endif
