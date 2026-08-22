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
#include "c_cdd_export.h"
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
  ASSERT_EQ_FMT(CDD_C_ERROR_INVALID_ARGUMENT, rc, "%d");
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
  ASSERT_EQ_FMT(CDD_C_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = cdd_main(2, argv2);
  ASSERT_EQ_FMT(CDD_C_ERROR_INVALID_ARGUMENT, rc, "%d");
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
      "cdd-c", "from_openapi",        "to_sdk", "-i", "spec.json",
      "-o",    "build/test_out_dir_3"};
  char *argv_serve_json_rpc[] = {"cdd-c", "serve_json_rpc"};
  char *argv_transformer[] = {"cdd-c", "transformer", "--help"};
  char *argv_standardize_gnu[] = {"cdd-c", "standardize-gnu", "--help"};
  char *argv_code2schema_err[] = {"cdd-c", "code2schema", "invalid"};
  char *argv_bind[] = {"cdd-c", "bind", "--help"};
  char *argv_generate_build_err[] = {"cdd-c", "generate_build_system",
                                     "invalid"};
  char *argv_schema2code_err[] = {"cdd-c", "schema2code", "invalid"};
  char *argv_serve_json_rpc_err[] = {"cdd-c", "serve_json_rpc", "invalid"};
  char *argv_mcp[] = {"cdd-c", "mcp"};
  char *argv_openapi2client[] = {"cdd-c", "openapi2client", "--help"};
  char *argv_audit_err[] = {"cdd-c", "audit", "invalid"};
  char *argv_audit_too_many[] = {"cdd-c", "audit", "invalid", "extra"};
  char *argv_transformer_err[] = {"cdd-c", "transformer", "invalid"};
  char *argv_standardize_gnu_err[] = {"cdd-c", "standardize-gnu", "invalid"};
  char *argv_to_docs_json_err[] = {"cdd-c", "to_docs_json", "invalid"};
  char *argv_bind_err[] = {"cdd-c", "bind", "invalid"};
  char *argv_mcp_err[] = {"cdd-c", "mcp", "invalid"};

  /* empty.h and valid_schema.json were created in cdd-c root */
  char *argv_c2openapi_help[] = {"cdd-c", "c2openapi", "../empty_dir",
                                 "out.json"};
  char *argv_code2schema_help[] = {"cdd-c", "code2schema", "../empty.h",
                                   "out.json"};
  char *argv_schema2code_help[] = {"cdd-c", "schema2code",
                                   "../valid_schema.json", "prefix"};
  char *argv_to_openapi_help[] = {"cdd-c", "to_openapi", "--help"};
  char *argv_from_openapi_help[] = {"cdd-c", "from_openapi", "--help"};
  char *argv_audit_help[] = {"cdd-c", "audit", "../empty_dir"};
  char *argv_generate_build_help[] = {"cdd-c", "generate_build_system",
                                      "--help"};

  cdd_main(4, argv_c2openapi);
  cdd_main(4, argv_code2schema);
  cdd_main(5, argv_generate_build);
  cdd_main(4, argv_schema2code);
  cdd_main(5, argv_jsonschema2tests);
  cdd_main(3, argv_audit);
  cdd_main(4, argv_to_openapi);
  cdd_main(4, argv_to_docs);
  cdd_main(7, argv_from_openapi);
  cdd_main(3, argv_transformer);
  cdd_main(3, argv_standardize_gnu);
  cdd_main(3, argv_code2schema_err);
  cdd_main(3, argv_bind);
  cdd_main(3, argv_generate_build_err);
  cdd_main(3, argv_schema2code_err);
  cdd_main(3, argv_serve_json_rpc_err);
  cdd_main(3, argv_openapi2client);
  cdd_main(3, argv_audit_err);
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, cdd_main(4, argv_audit_too_many));
  cdd_main(3, argv_transformer_err);
  cdd_main(3, argv_standardize_gnu_err);
  cdd_main(3, argv_to_docs_json_err);
  cdd_main(3, argv_bind_err);

/* Close stdin or redirect to /dev/null so mcp does not block waiting for
 * input */
#if defined(_WIN32)
  (void)freopen("NUL", "r", stdin);
#else
  if (freopen("/dev/null", "r", stdin)) {
  }
#endif

  cdd_main(3, argv_mcp_err);

  cdd_main(3, argv_c2openapi_help);
  cdd_main(3, argv_code2schema_help);
  cdd_main(3, argv_schema2code_help);
  cdd_main(3, argv_to_openapi_help);
  cdd_main(3, argv_from_openapi_help);
  cdd_main(3, argv_audit_help);
  cdd_main(3, argv_generate_build_help);

  /* cdd_main(2, argv_mcp); */
  /* cdd_main(2, argv_serve_json_rpc); */

  g_fail_io_after = -1;

  PASS();
}

/**
 * @brief Tests from_openapi CLI options and env vars.
 *
 * @return The result of the test.
 */
TEST test_main_from_openapi_cli_options(void) {
  char *argv_cli[] = {"cdd-c",     "from_openapi", "to_sdk_cli",        "-i",
                      "spec.json", "-o",           "build/test_out_dir"};
  char *argv_server[] = {
      "cdd-c",     "from_openapi", "to_server",           "-i",
      "spec.json", "-o",           "build/test_out_dir_2"};
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
  ASSERT_EQ(CDD_C_SUCCESS, cdd_main(3, argv_help));
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, cdd_main(5, argv_err)); /* missing input */

  cdd_main(9, argv_flags);

  /* Set ENV vars */
#if defined(_WIN32)
  _putenv("CDD_INPUT=spec.json");
  _putenv("CDD_OUTPUT=test_out_dir_env");
#else
  setenv("CDD_INPUT", "spec.json", 1);
  setenv("CDD_OUTPUT", "build/test_out_dir_env", 1);
#endif

/* Create a dummy spec to test the execution */
#if defined(_MSC_VER)
  if (fopen_s(&f, "spec.json", "w") != 0)
    f = NULL;
#else
  f = fopen("spec.json", "w");
#endif
  fprintf(f, "{\"openapi\": \"3.1.0\", \"info\": {\"title\": \"Test\", "
             "\"version\": \"1.0\"}, \"paths\": {}}");
  fclose(f);

  ASSERT_EQ(0, cdd_main(7, argv_cli));
  ASSERT_EQ(0, cdd_main(7, argv_server));
  ASSERT_EQ(0, cdd_main(3, argv_env));

  /* Unset CDD env vars and test fallbacks */
#if defined(_WIN32)
  _putenv("CDD_INPUT=");
  _putenv("CDD_OUTPUT=");
  _putenv("INPUT_FILE=spec.json");
  _putenv("OUT_DIR=build/test_out_dir_env2");
#else
  unsetenv("CDD_INPUT");
  unsetenv("CDD_OUTPUT");
  setenv("INPUT_FILE", "spec.json", 1);
  setenv("OUT_DIR", "build/test_out_dir_env2", 1);
#endif

  ASSERT_EQ(0, cdd_main(3, argv_env));

  /* Set input dir to cover that branch */
#if defined(_WIN32)
  _putenv("CDD_INPUT_DIR=.");
#else
  setenv("CDD_INPUT_DIR", ".", 1);
#endif
  ASSERT_EQ(0, cdd_main(3, argv_env));

#if defined(_WIN32)
  _putenv("CDD_INPUT_DIR=");
  _putenv("INPUT_DIR=.");
#else
  unsetenv("CDD_INPUT_DIR");
  setenv("INPUT_DIR", ".", 1);
#endif
  ASSERT_EQ(0, cdd_main(3, argv_env));

  remove("spec.json");

  /* Unset ENV vars */
#if defined(_WIN32)
  _putenv("INPUT_FILE=");
  _putenv("OUT_DIR=");
  _putenv("INPUT_DIR=");
#else
  unsetenv("INPUT_FILE");
  unsetenv("OUT_DIR");
  unsetenv("INPUT_DIR");
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

  ASSERT_EQ(CDD_C_SUCCESS, cdd_main(3, argv_help));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_main(3, argv_help2));
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, cdd_main(2, argv_err));

  cdd_main(6, argv_flags);
  cdd_main(6, argv_flags2);

  /* Set ENV vars */
#if defined(_WIN32)
  _putenv("CDD_INPUT=indir3");
  _putenv("CDD_OUTPUT=outdir3");
#else
  setenv("CDD_INPUT", "indir3", 1);
  setenv("CDD_OUTPUT", "outdir3", 1);
#endif

  /* still fails because it's not implemented, but we hit the env var branch */
  cdd_main(2, argv_env);

#if defined(_WIN32)
  _putenv("CDD_INPUT=");
  _putenv("CDD_OUTPUT=");
  _putenv("INPUT_DIR=indir3");
  _putenv("OUT_FILE=outdir3");
#else
  unsetenv("CDD_INPUT");
  unsetenv("CDD_OUTPUT");
  setenv("INPUT_DIR", "indir3", 1);
  setenv("OUT_FILE", "outdir3", 1);
#endif
  cdd_main(2, argv_env);

  /* Unset ENV vars */
#if defined(_WIN32)
  _putenv("INPUT_DIR=");
  _putenv("OUT_FILE=");
#else
  unsetenv("INPUT_DIR");
  unsetenv("OUT_FILE");
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
  rc = system("build_coverage/bin/cdd-c --help > /dev/null 2>&1");
  if (rc != 0) {
    rc = system("./bin/cdd-c --help > /dev/null 2>&1");
  }
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
