extern int g_fail_io_after;
extern int g_io_calls;
/**
 * @file test_main.h
 * @brief Unit tests for the main application entry point router.
 */

#ifndef TEST_MAIN_H
#define TEST_MAIN_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "functions/parse/main.h"
#include "greatest.h"
#include <stdlib.h>
/* clang-format on */

/**
 * @brief Tests main with no arguments.
 *
 * @return The result of the test.
 */
TEST test_main_no_args(void) {
  char *argv[] = {"cdd-c"};
  int rc = cdd_main(1, argv);
  ASSERT_EQ_FMT(EXIT_FAILURE, rc, "%d");
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Tests main with the --help argument.
 *
 * @return The result of the test.
 */
TEST test_main_help(void) {
  char *argv[] = {"cdd-c", "--help"};
  char *argv2[] = {"cdd-c", "-h"};
  int rc = cdd_main(2, argv);
  ASSERT_EQ_FMT(EXIT_SUCCESS, rc, "%d");

  rc = cdd_main(2, argv2);
  ASSERT_EQ_FMT(EXIT_SUCCESS, rc, "%d");
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Tests main with the --version argument.
 *
 * @return The result of the test.
 */
TEST test_main_version(void) {
  char *argv[] = {"cdd-c", "--version"};
  char *argv2[] = {"cdd-c", "-v"};
  int rc = cdd_main(2, argv);
  ASSERT_EQ_FMT(EXIT_SUCCESS, rc, "%d");

  rc = cdd_main(2, argv2);
  ASSERT_EQ_FMT(EXIT_SUCCESS, rc, "%d");
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Tests main with an invalid command.
 *
 * @return The result of the test.
 */
TEST test_main_invalid_command(void) {
  char *argv[] = {"cdd-c", "unknown_command"};
  char *argv2[] = {"cdd-c", "openapi2client"};
  int rc = cdd_main(2, argv);
  ASSERT_EQ_FMT(EXIT_FAILURE, rc, "%d");

  rc = cdd_main(2, argv2);
  ASSERT_EQ_FMT(EXIT_FAILURE, rc, "%d");
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Tests routing for main subcommands.
 *
 * @return The result of the test.
 */
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
  char *argv_from_openapi[] = {
      "cdd-c",     "from_openapi", "to_sdk",        "-i",
      "spec.json", "-o",           "test_out_dir_3"};
  char *argv_serve_json_rpc[] = {"cdd-c", "serve_json_rpc"};
  char *argv_transformer[] = {"cdd-c", "transformer", "--help"};

  cdd_main(4, argv_c2openapi);
  cdd_main(4, argv_code2schema);
  cdd_main(5, argv_generate_build);
  cdd_main(4, argv_schema2code);
  cdd_main(5, argv_jsonschema2tests);
  cdd_main(3, argv_audit);
  cdd_main(4, argv_to_openapi);
  cdd_main(4, argv_to_docs);
  cdd_main(7, argv_from_openapi);
  cdd_main(2, argv_serve_json_rpc);
  cdd_main(3, argv_transformer);
  g_fail_io_after = -1;

  PASS();
}

/**
 * @brief Tests from_openapi CLI options and env vars.
 *
 * @return The result of the test.
 */
TEST test_main_from_openapi_cli_options(void) {
  char *argv_cli[] = {"cdd-c",     "from_openapi", "to_sdk_cli",  "-i",
                      "spec.json", "-o",           "test_out_dir"};
  char *argv_server[] = {"cdd-c",     "from_openapi", "to_server",     "-i",
                         "spec.json", "-o",           "test_out_dir_2"};
  char *argv_help[] = {"cdd-c", "from_openapi", "--help"};
  char *argv_err[] = {"cdd-c", "from_openapi", "to_sdk", "-o", "out_dir"};
  char *argv_flags[] = {"cdd-c",
                        "from_openapi",
                        "to_sdk",
                        "--input-dir",
                        "indir",
                        "--no-github-actions",
                        "--no-installable-package",
                        "--tests",
                        "yes"};
  char *argv_env[] = {"cdd-c", "from_openapi", "to_sdk"};
  FILE *f;

  /* Note: we can't test actual execution easily without creating a dummy */
  /* spec.json, but we can at least hit the help and error paths. */
  ASSERT_EQ(EXIT_SUCCESS, cdd_main(3, argv_help));
  ASSERT_EQ(EXIT_FAILURE, cdd_main(5, argv_err)); /* missing input */

  cdd_main(9, argv_flags);

  /* Set ENV vars */
#if defined(_WIN32)
  _putenv("CDD_INPUT_FILE=spec.json");
  _putenv("CDD_OUT_DIR=test_out_dir_env");
#else
  setenv("CDD_INPUT_FILE", "spec.json", 1);
  setenv("CDD_OUT_DIR", "test_out_dir_env", 1);
#endif

  /* Create a dummy spec to test the execution */
  f = fopen("spec.json", "w");
  fprintf(f, "{\"openapi\": \"3.1.0\", \"info\": {\"title\": \"Test\", "
             "\"version\": \"1.0\"}, \"paths\": {}}");
  fclose(f);

  ASSERT_EQ(0, cdd_main(7, argv_cli));
  ASSERT_EQ(0, cdd_main(7, argv_server));
  ASSERT_EQ(0, cdd_main(3, argv_env));

  remove("spec.json");

  /* Unset ENV vars */
#if defined(_WIN32)
  _putenv("CDD_INPUT_FILE=");
  _putenv("CDD_OUT_DIR=");
#else
  unsetenv("CDD_INPUT_FILE");
  unsetenv("CDD_OUT_DIR");
#endif
  g_fail_io_after = -1;

  PASS();
}

/**
 * @brief Tests to_openapi CLI options and env vars.
 *
 * @return The result of the test.
 */
TEST test_main_to_openapi_cli_options(void) {
  char *argv_help[] = {"cdd-c", "to_openapi", "--help"};
  char *argv_help2[] = {"cdd-c", "to_openapi", "-h"};
  char *argv_err[] = {"cdd-c", "to_openapi"};
  char *argv_flags[] = {"cdd-c", "to_openapi", "-i", "indir", "-o", "outdir"};
  char *argv_flags2[] = {"cdd-c",  "to_openapi", "--input",
                         "indir2", "--output",   "outdir2"};
  char *argv_env[] = {"cdd-c", "to_openapi"};

  ASSERT_EQ(EXIT_SUCCESS, cdd_main(3, argv_help));
  ASSERT_EQ(EXIT_SUCCESS, cdd_main(3, argv_help2));
  ASSERT_EQ(EXIT_FAILURE, cdd_main(2, argv_err));

  cdd_main(6, argv_flags);
  cdd_main(6, argv_flags2);

  /* Set ENV vars */
#if defined(_WIN32)
  _putenv("CDD_INPUT_DIR=indir3");
  _putenv("CDD_OUT_FILE=outdir3");
#else
  setenv("CDD_INPUT_DIR", "indir3", 1);
  setenv("CDD_OUT_FILE", "outdir3", 1);
#endif

  /* still fails because it's not implemented, but we hit the env var branch */
  cdd_main(2, argv_env);

  /* Unset ENV vars */
#if defined(_WIN32)
  _putenv("CDD_INPUT_DIR=");
  _putenv("CDD_OUT_FILE=");
#else
  unsetenv("CDD_INPUT_DIR");
  unsetenv("CDD_OUT_FILE");
#endif
  g_fail_io_after = -1;

  PASS();
}

/**
 * @brief Main logic test suite.
 */
TEST test_bin_cdd_executable(void) {
  /* Run the actual executable to cover bin_cdd.c's main() */
  /* The tests might be run from build dir or root dir, so we check both */
  int rc;
  rc = system("./bin/cdd-c --help > /dev/null 2>&1");
  if (rc != 0) {
    rc = system("../bin/cdd-c --help > /dev/null 2>&1");
  }
  if (rc != 0) {
    rc = system("../../bin/cdd-c --help > /dev/null 2>&1");
  }
  g_fail_io_after = -1;
  PASS();
}

SUITE(main_suite) {
  RUN_TEST(test_main_no_args);
  RUN_TEST(test_main_help);
  RUN_TEST(test_main_version);
  RUN_TEST(test_main_invalid_command);
  RUN_TEST(test_main_subcommands);
  RUN_TEST(test_main_from_openapi_cli_options);
  RUN_TEST(test_main_to_openapi_cli_options);
  RUN_TEST(test_bin_cdd_executable);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_MAIN_H */
