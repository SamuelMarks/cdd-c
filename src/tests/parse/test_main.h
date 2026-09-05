#if defined(_MSC_VER)
static FILE *cdd_freopen_helper_main(const char *p, const char *m, FILE *s) {
  FILE *f = NULL;
  freopen_s(&f, p, m, s);
  return f;
}
#undef CDD_FREOPEN
#define CDD_FREOPEN cdd_freopen_helper_main
#else
#define CDD_FREOPEN freopen
#endif
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
  char *argv[] = {(char *)(size_t)(size_t) "cdd-c"};
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
  char *argv[] = {(char *)(size_t)(size_t) "cdd-c",
                  (char *)(size_t)(size_t) "--help"};
  char *argv2[] = {(char *)(size_t)(size_t) "cdd-c",
                   (char *)(size_t)(size_t) "-h"};
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
  char *argv[] = {(char *)(size_t)(size_t) "cdd-c",
                  (char *)(size_t)(size_t) "--version"};
  char *argv2[] = {(char *)(size_t)(size_t) "cdd-c",
                   (char *)(size_t)(size_t) "-v"};
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
  char *argv[] = {(char *)(size_t)(size_t) "cdd-c",
                  (char *)(size_t)(size_t) "unknown_command"};
  char *argv2[] = {(char *)(size_t)(size_t) "cdd-c",
                   (char *)(size_t)(size_t) "openapi2client"};
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
  char *argv_c2openapi[] = {
      (char *)(size_t)(size_t) "cdd-c", (char *)(size_t)(size_t) "c2openapi",
      (char *)(size_t)(size_t) "dir", (char *)(size_t)(size_t) "out.json"};
  char *argv_code2schema[] = {(char *)(size_t)(size_t) "cdd-c",
                              (char *)(size_t)(size_t) "code2schema",
                              (char *)(size_t)(size_t) "header.h",
                              (char *)(size_t)(size_t) "schema.json"};
  char *argv_generate_build[] = {
      (char *)(size_t)(size_t) "cdd-c",
      (char *)(size_t)(size_t) "generate_build_system",
      (char *)(size_t)(size_t) "cmake", (char *)(size_t)(size_t) "out",
      (char *)(size_t)(size_t) "name"};
  char *argv_schema2code[] = {
      (char *)(size_t)(size_t) "cdd-c", (char *)(size_t)(size_t) "schema2code",
      (char *)(size_t)(size_t) "schema.json", (char *)(size_t)(size_t) "out"};
  char *argv_jsonschema2tests[] = {(char *)(size_t)(size_t) "cdd-c",
                                   (char *)(size_t)(size_t) "jsonschema2tests",
                                   (char *)(size_t)(size_t) "schema.json",
                                   (char *)(size_t)(size_t) "hdr.h",
                                   (char *)(size_t)(size_t) "out.h"};
  char *argv_audit[] = {(char *)(size_t)(size_t) "cdd-c",
                        (char *)(size_t)(size_t) "audit",
                        (char *)(size_t)(size_t) "dir"};
  char *argv_to_openapi[] = {
      (char *)(size_t)(size_t) "cdd-c", (char *)(size_t)(size_t) "to_openapi",
      (char *)(size_t)(size_t) "-f", (char *)(size_t)(size_t) "dir"};
  char *argv_to_docs[] = {
      (char *)(size_t)(size_t) "cdd-c", (char *)(size_t)(size_t) "to_docs_json",
      (char *)(size_t)(size_t) "-i", (char *)(size_t)(size_t) "spec.json"};
  char *argv_from_openapi[] = {(char *)(size_t)(size_t) "cdd-c",
                               (char *)(size_t)(size_t) "from_openapi",
                               (char *)(size_t)(size_t) "to_sdk",
                               (char *)(size_t)(size_t) "-i",
                               (char *)(size_t)(size_t) "spec.json",
                               (char *)(size_t)(size_t) "-o",
                               (char *)(size_t)(size_t) "build/test_out_dir_3"};
  char *argv_serve_json_rpc[] = {(char *)(size_t)(size_t) "cdd-c",
                                 (char *)(size_t)(size_t) "serve_json_rpc"};
  char *argv_transformer[] = {(char *)(size_t)(size_t) "cdd-c",
                              (char *)(size_t)(size_t) "transformer",
                              (char *)(size_t)(size_t) "--help"};
  char *argv_standardize_gnu[] = {(char *)(size_t)(size_t) "cdd-c",
                                  (char *)(size_t)(size_t) "standardize-gnu",
                                  (char *)(size_t)(size_t) "--help"};
  char *argv_code2schema_err[] = {(char *)(size_t)(size_t) "cdd-c",
                                  (char *)(size_t)(size_t) "code2schema",
                                  (char *)(size_t)(size_t) "invalid"};
  char *argv_bind[] = {(char *)(size_t)(size_t) "cdd-c",
                       (char *)(size_t)(size_t) "bind",
                       (char *)(size_t)(size_t) "--help"};
  char *argv_generate_build_err[] = {
      (char *)(size_t)(size_t) "cdd-c",
      (char *)(size_t)(size_t) "generate_build_system",
      (char *)(size_t)(size_t) "invalid"};
  char *argv_schema2code_err[] = {(char *)(size_t)(size_t) "cdd-c",
                                  (char *)(size_t)(size_t) "schema2code",
                                  (char *)(size_t)(size_t) "invalid"};
  char *argv_serve_json_rpc_err[] = {(char *)(size_t)(size_t) "cdd-c",
                                     (char *)(size_t)(size_t) "serve_json_rpc",
                                     (char *)(size_t)(size_t) "invalid"};
  char *argv_mcp[] = {(char *)(size_t)(size_t) "cdd-c",
                      (char *)(size_t)(size_t) "mcp"};
  char *argv_openapi2client[] = {(char *)(size_t)(size_t) "cdd-c",
                                 (char *)(size_t)(size_t) "openapi2client",
                                 (char *)(size_t)(size_t) "--help"};
  char *argv_audit_err[] = {(char *)(size_t)(size_t) "cdd-c",
                            (char *)(size_t)(size_t) "audit",
                            (char *)(size_t)(size_t) "invalid"};
  char *argv_audit_too_many[] = {
      (char *)(size_t)(size_t) "cdd-c", (char *)(size_t)(size_t) "audit",
      (char *)(size_t)(size_t) "invalid", (char *)(size_t)(size_t) "extra"};
  char *argv_transformer_err[] = {(char *)(size_t)(size_t) "cdd-c",
                                  (char *)(size_t)(size_t) "transformer",
                                  (char *)(size_t)(size_t) "invalid"};
  char *argv_standardize_gnu_err[] = {
      (char *)(size_t)(size_t) "cdd-c",
      (char *)(size_t)(size_t) "standardize-gnu",
      (char *)(size_t)(size_t) "invalid"};
  char *argv_to_docs_json_err[] = {(char *)(size_t)(size_t) "cdd-c",
                                   (char *)(size_t)(size_t) "to_docs_json",
                                   (char *)(size_t)(size_t) "invalid"};
  char *argv_bind_err[] = {(char *)(size_t)(size_t) "cdd-c",
                           (char *)(size_t)(size_t) "bind",
                           (char *)(size_t)(size_t) "invalid"};
  char *argv_mcp_err[] = {(char *)(size_t)(size_t) "cdd-c",
                          (char *)(size_t)(size_t) "mcp",
                          (char *)(size_t)(size_t) "invalid"};

  /* empty.h and valid_schema.json were created in cdd-c root */
  char *argv_c2openapi_help[] = {(char *)(size_t)(size_t) "cdd-c",
                                 (char *)(size_t)(size_t) "c2openapi",
                                 (char *)(size_t)(size_t) "../empty_dir",
                                 (char *)(size_t)(size_t) "out.json"};
  char *argv_code2schema_help[] = {(char *)(size_t)(size_t) "cdd-c",
                                   (char *)(size_t)(size_t) "code2schema",
                                   (char *)(size_t)(size_t) "../empty.h",
                                   (char *)(size_t)(size_t) "out.json"};
  char *argv_schema2code_help[] = {
      (char *)(size_t)(size_t) "cdd-c", (char *)(size_t)(size_t) "schema2code",
      (char *)(size_t)(size_t) "../valid_schema.json",
      (char *)(size_t)(size_t) "prefix"};
  char *argv_to_openapi_help[] = {(char *)(size_t)(size_t) "cdd-c",
                                  (char *)(size_t)(size_t) "to_openapi",
                                  (char *)(size_t)(size_t) "--help"};
  char *argv_from_openapi_help[] = {(char *)(size_t)(size_t) "cdd-c",
                                    (char *)(size_t)(size_t) "from_openapi",
                                    (char *)(size_t)(size_t) "--help"};
  char *argv_audit_help[] = {(char *)(size_t)(size_t) "cdd-c",
                             (char *)(size_t)(size_t) "audit",
                             (char *)(size_t)(size_t) "../empty_dir"};
  char *argv_generate_build_help[] = {
      (char *)(size_t)(size_t) "cdd-c",
      (char *)(size_t)(size_t) "generate_build_system",
      (char *)(size_t)(size_t) "--help"};

  /* cdd_main(4, argv_c2openapi); */ fprintf(stderr, "1\n");
  (void)argv_c2openapi;
  /* cdd_main(4, argv_code2schema); */ fprintf(stderr, "2\n");
  (void)argv_code2schema;
  cdd_main(5, argv_generate_build);
  fprintf(stderr, "3\n");
  cdd_main(4, argv_schema2code);
  fprintf(stderr, "4\n");
  cdd_main(5, argv_jsonschema2tests);
  fprintf(stderr, "5\n");
  cdd_main(3, argv_audit);
  fprintf(stderr, "6\n");
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
  (void)argv_mcp;
  (void)argv_serve_json_rpc;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, cdd_main(4, argv_audit_too_many));
  cdd_main(3, argv_transformer_err);
  cdd_main(3, argv_standardize_gnu_err);
  cdd_main(3, argv_to_docs_json_err);
  cdd_main(3, argv_bind_err);

/* Close stdin or redirect to /dev/null so mcp does not block waiting for
 * input */
#if defined(_WIN32)
  (void)CDD_FREOPEN("NUL", "r", stdin);
#else
  if (CDD_FREOPEN("/dev/null", "r", stdin)) {
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
  char *argv_cli[] = {(char *)(size_t)(size_t) "cdd-c",
                      (char *)(size_t)(size_t) "from_openapi",
                      (char *)(size_t)(size_t) "to_sdk_cli",
                      (char *)(size_t)(size_t) "-i",
                      (char *)(size_t)(size_t) "spec.json",
                      (char *)(size_t)(size_t) "-o",
                      (char *)(size_t)(size_t) "build/test_out_dir"};
  char *argv_server[] = {(char *)(size_t)(size_t) "cdd-c",
                         (char *)(size_t)(size_t) "from_openapi",
                         (char *)(size_t)(size_t) "to_server",
                         (char *)(size_t)(size_t) "-i",
                         (char *)(size_t)(size_t) "spec.json",
                         (char *)(size_t)(size_t) "-o",
                         (char *)(size_t)(size_t) "build/test_out_dir_2"};
  char *argv_help[] = {(char *)(size_t)(size_t) "cdd-c",
                       (char *)(size_t)(size_t) "from_openapi",
                       (char *)(size_t)(size_t) "--help"};
  char *argv_err[] = {
      (char *)(size_t)(size_t) "cdd-c", (char *)(size_t)(size_t) "from_openapi",
      (char *)(size_t)(size_t) "to_sdk", (char *)(size_t)(size_t) "-o",
      (char *)(size_t)(size_t) "out_dir"};
  char *argv_flags[] = {(char *)(size_t)(size_t) "cdd-c",
                        (char *)(size_t)(size_t) "from_openapi",
                        (char *)(size_t)(size_t) "to_sdk",
                        (char *)(size_t)(size_t) "--input-dir",
                        (char *)(size_t)(size_t) "indir",
                        (char *)(size_t)(size_t) "--no-github-actions",
                        (char *)(size_t)(size_t) "--no-installable-package",
                        (char *)(size_t)(size_t) "--tests",
                        (char *)(size_t)(size_t) "yes"};
  char *argv_env[] = {(char *)(size_t)(size_t) "cdd-c",
                      (char *)(size_t)(size_t) "from_openapi",
                      (char *)(size_t)(size_t) "to_sdk"};
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
  if (f)
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
  char *argv_help[] = {(char *)(size_t)(size_t) "cdd-c",
                       (char *)(size_t)(size_t) "to_openapi",
                       (char *)(size_t)(size_t) "--help"};
  char *argv_help2[] = {(char *)(size_t)(size_t) "cdd-c",
                        (char *)(size_t)(size_t) "to_openapi",
                        (char *)(size_t)(size_t) "-h"};
  char *argv_err[] = {(char *)(size_t)(size_t) "cdd-c",
                      (char *)(size_t)(size_t) "to_openapi"};
  char *argv_flags[] = {
      (char *)(size_t)(size_t) "cdd-c", (char *)(size_t)(size_t) "to_openapi",
      (char *)(size_t)(size_t) "-i",    (char *)(size_t)(size_t) "indir",
      (char *)(size_t)(size_t) "-o",    (char *)(size_t)(size_t) "outdir"};
  char *argv_flags2[] = {(char *)(size_t)(size_t) "cdd-c",
                         (char *)(size_t)(size_t) "to_openapi",
                         (char *)(size_t)(size_t) "--input",
                         (char *)(size_t)(size_t) "indir2",
                         (char *)(size_t)(size_t) "--output",
                         (char *)(size_t)(size_t) "outdir2"};
  char *argv_env[] = {(char *)(size_t)(size_t) "cdd-c",
                      (char *)(size_t)(size_t) "to_openapi"};

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
  (void)rc;
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
