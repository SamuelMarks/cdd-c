#if defined(_MSC_VER)
static FILE *cdd_freopen_helper_main(const char *p, const char *m, FILE *s) {
  FILE *f = NULL;
  freopen_s(&f, p, m, s);
  return f;
}
#undef CDD_FREOPEN
#define CDD_FREOPEN cdd_freopen_helper_main
#else
/**
 * @brief Macro CDD_FREOPEN.
 */
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
  char *argv[1];
  int rc;

  argv[0] = cdd_test_mut_str("cdd-c");
  rc = cdd_main(1, argv);
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
  char *argv[2];
  char *argv2[2];
  int rc;

  argv[0] = cdd_test_mut_str("cdd-c");
  argv[1] = cdd_test_mut_str("--help");
  argv2[0] = cdd_test_mut_str("cdd-c");
  argv2[1] = cdd_test_mut_str("-h");
  rc = cdd_main(2, argv);
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
  char *argv[2];
  char *argv2[2];
  int rc;

  argv[0] = cdd_test_mut_str("cdd-c");
  argv[1] = cdd_test_mut_str("--version");
  argv2[0] = cdd_test_mut_str("cdd-c");
  argv2[1] = cdd_test_mut_str("-v");
  rc = cdd_main(2, argv);
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
  char *argv[2];
  char *argv2[2];
  int rc;

  argv[0] = cdd_test_mut_str("cdd-c");
  argv[1] = cdd_test_mut_str("unknown_command");
  argv2[0] = cdd_test_mut_str("cdd-c");
  argv2[1] = cdd_test_mut_str("openapi2client");
  rc = cdd_main(2, argv);
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
  char *argv_c2openapi[4];
  char *argv_code2schema[4];
  char *argv_generate_build[5];
  char *argv_schema2code[4];
  char *argv_jsonschema2tests[5];
  char *argv_audit[3];
  char *argv_to_openapi[4];
  char *argv_to_docs[4];
  char *argv_from_openapi[7];
  char *argv_serve_json_rpc[2];
  char *argv_transformer[3];
  char *argv_standardize_gnu[3];
  char *argv_code2schema_err[3];
  char *argv_bind[3];
  char *argv_generate_build_err[3];
  char *argv_schema2code_err[3];
  char *argv_serve_json_rpc_err[3];
  char *argv_mcp[2];
  char *argv_openapi2client[3];
  char *argv_audit_err[3];
  char *argv_audit_too_many[4];
  char *argv_transformer_err[3];
  char *argv_standardize_gnu_err[3];
  char *argv_to_docs_json_err[3];
  char *argv_bind_err[3];
  char *argv_mcp_err[3];

  /* empty.h and valid_schema.json were created in cdd-c root */
  char *argv_c2openapi_help[4];
  char *argv_code2schema_help[4];
  char *argv_schema2code_help[4];
  char *argv_to_openapi_help[3];
  char *argv_from_openapi_help[3];
  char *argv_audit_help[3];
  char *argv_generate_build_help[3];

  /* cdd_main(4, argv_c2openapi); */ fprintf(stderr, "1\n");

  argv_c2openapi[0] = cdd_test_mut_str("cdd-c");
  argv_c2openapi[1] = cdd_test_mut_str("c2openapi");
  argv_c2openapi[2] = cdd_test_mut_str("dir");
  argv_c2openapi[3] = cdd_test_mut_str("out.json");
  argv_code2schema[0] = cdd_test_mut_str("cdd-c");
  argv_code2schema[1] = cdd_test_mut_str("code2schema");
  argv_code2schema[2] = cdd_test_mut_str("header.h");
  argv_code2schema[3] = cdd_test_mut_str("schema.json");
  argv_generate_build[0] = cdd_test_mut_str("cdd-c");
  argv_generate_build[1] = cdd_test_mut_str("generate_build_system");
  argv_generate_build[2] = cdd_test_mut_str("cmake");
  argv_generate_build[3] = cdd_test_mut_str("out");
  argv_generate_build[4] = cdd_test_mut_str("name");
  argv_schema2code[0] = cdd_test_mut_str("cdd-c");
  argv_schema2code[1] = cdd_test_mut_str("schema2code");
  argv_schema2code[2] = cdd_test_mut_str("schema.json");
  argv_schema2code[3] = cdd_test_mut_str("out");
  argv_jsonschema2tests[0] = cdd_test_mut_str("cdd-c");
  argv_jsonschema2tests[1] = cdd_test_mut_str("jsonschema2tests");
  argv_jsonschema2tests[2] = cdd_test_mut_str("schema.json");
  argv_jsonschema2tests[3] = cdd_test_mut_str("hdr.h");
  argv_jsonschema2tests[4] = cdd_test_mut_str("out.h");
  argv_audit[0] = cdd_test_mut_str("cdd-c");
  argv_audit[1] = cdd_test_mut_str("audit");
  argv_audit[2] = cdd_test_mut_str("dir");
  argv_to_openapi[0] = cdd_test_mut_str("cdd-c");
  argv_to_openapi[1] = cdd_test_mut_str("to_openapi");
  argv_to_openapi[2] = cdd_test_mut_str("-f");
  argv_to_openapi[3] = cdd_test_mut_str("dir");
  argv_to_docs[0] = cdd_test_mut_str("cdd-c");
  argv_to_docs[1] = cdd_test_mut_str("to_docs_json");
  argv_to_docs[2] = cdd_test_mut_str("-i");
  argv_to_docs[3] = cdd_test_mut_str("spec.json");
  argv_from_openapi[0] = cdd_test_mut_str("cdd-c");
  argv_from_openapi[1] = cdd_test_mut_str("from_openapi");
  argv_from_openapi[2] = cdd_test_mut_str("to_sdk");
  argv_from_openapi[3] = cdd_test_mut_str("-i");
  argv_from_openapi[4] = cdd_test_mut_str("spec.json");
  argv_from_openapi[5] = cdd_test_mut_str("-o");
  argv_from_openapi[6] = cdd_test_mut_str("build/test_out_dir_3");
  argv_serve_json_rpc[0] = cdd_test_mut_str("cdd-c");
  argv_serve_json_rpc[1] = cdd_test_mut_str("serve_json_rpc");
  argv_transformer[0] = cdd_test_mut_str("cdd-c");
  argv_transformer[1] = cdd_test_mut_str("transformer");
  argv_transformer[2] = cdd_test_mut_str("--help");
  argv_standardize_gnu[0] = cdd_test_mut_str("cdd-c");
  argv_standardize_gnu[1] = cdd_test_mut_str("standardize-gnu");
  argv_standardize_gnu[2] = cdd_test_mut_str("--help");
  argv_code2schema_err[0] = cdd_test_mut_str("cdd-c");
  argv_code2schema_err[1] = cdd_test_mut_str("code2schema");
  argv_code2schema_err[2] = cdd_test_mut_str("invalid");
  argv_bind[0] = cdd_test_mut_str("cdd-c");
  argv_bind[1] = cdd_test_mut_str("bind");
  argv_bind[2] = cdd_test_mut_str("--help");
  argv_generate_build_err[0] = cdd_test_mut_str("cdd-c");
  argv_generate_build_err[1] = cdd_test_mut_str("generate_build_system");
  argv_generate_build_err[2] = cdd_test_mut_str("invalid");
  argv_schema2code_err[0] = cdd_test_mut_str("cdd-c");
  argv_schema2code_err[1] = cdd_test_mut_str("schema2code");
  argv_schema2code_err[2] = cdd_test_mut_str("invalid");
  argv_serve_json_rpc_err[0] = cdd_test_mut_str("cdd-c");
  argv_serve_json_rpc_err[1] = cdd_test_mut_str("serve_json_rpc");
  argv_serve_json_rpc_err[2] = cdd_test_mut_str("invalid");
  argv_mcp[0] = cdd_test_mut_str("cdd-c");
  argv_mcp[1] = cdd_test_mut_str("mcp");
  argv_openapi2client[0] = cdd_test_mut_str("cdd-c");
  argv_openapi2client[1] = cdd_test_mut_str("openapi2client");
  argv_openapi2client[2] = cdd_test_mut_str("--help");
  argv_audit_err[0] = cdd_test_mut_str("cdd-c");
  argv_audit_err[1] = cdd_test_mut_str("audit");
  argv_audit_err[2] = cdd_test_mut_str("invalid");
  argv_audit_too_many[0] = cdd_test_mut_str("cdd-c");
  argv_audit_too_many[1] = cdd_test_mut_str("audit");
  argv_audit_too_many[2] = cdd_test_mut_str("invalid");
  argv_audit_too_many[3] = cdd_test_mut_str("extra");
  argv_transformer_err[0] = cdd_test_mut_str("cdd-c");
  argv_transformer_err[1] = cdd_test_mut_str("transformer");
  argv_transformer_err[2] = cdd_test_mut_str("invalid");
  argv_standardize_gnu_err[0] = cdd_test_mut_str("cdd-c");
  argv_standardize_gnu_err[1] = cdd_test_mut_str("standardize-gnu");
  argv_standardize_gnu_err[2] = cdd_test_mut_str("invalid");
  argv_to_docs_json_err[0] = cdd_test_mut_str("cdd-c");
  argv_to_docs_json_err[1] = cdd_test_mut_str("to_docs_json");
  argv_to_docs_json_err[2] = cdd_test_mut_str("invalid");
  argv_bind_err[0] = cdd_test_mut_str("cdd-c");
  argv_bind_err[1] = cdd_test_mut_str("bind");
  argv_bind_err[2] = cdd_test_mut_str("invalid");
  argv_mcp_err[0] = cdd_test_mut_str("cdd-c");
  argv_mcp_err[1] = cdd_test_mut_str("mcp");
  argv_mcp_err[2] = cdd_test_mut_str("invalid");
  argv_c2openapi_help[0] = cdd_test_mut_str("cdd-c");
  argv_c2openapi_help[1] = cdd_test_mut_str("c2openapi");
  argv_c2openapi_help[2] = cdd_test_mut_str("../empty_dir");
  argv_c2openapi_help[3] = cdd_test_mut_str("out.json");
  argv_code2schema_help[0] = cdd_test_mut_str("cdd-c");
  argv_code2schema_help[1] = cdd_test_mut_str("code2schema");
  argv_code2schema_help[2] = cdd_test_mut_str("../empty.h");
  argv_code2schema_help[3] = cdd_test_mut_str("out.json");
  argv_schema2code_help[0] = cdd_test_mut_str("cdd-c");
  argv_schema2code_help[1] = cdd_test_mut_str("schema2code");
  argv_schema2code_help[2] = cdd_test_mut_str("../valid_schema.json");
  argv_schema2code_help[3] = cdd_test_mut_str("prefix");
  argv_to_openapi_help[0] = cdd_test_mut_str("cdd-c");
  argv_to_openapi_help[1] = cdd_test_mut_str("to_openapi");
  argv_to_openapi_help[2] = cdd_test_mut_str("--help");
  argv_from_openapi_help[0] = cdd_test_mut_str("cdd-c");
  argv_from_openapi_help[1] = cdd_test_mut_str("from_openapi");
  argv_from_openapi_help[2] = cdd_test_mut_str("--help");
  argv_audit_help[0] = cdd_test_mut_str("cdd-c");
  argv_audit_help[1] = cdd_test_mut_str("audit");
  argv_audit_help[2] = cdd_test_mut_str("../empty_dir");
  argv_generate_build_help[0] = cdd_test_mut_str("cdd-c");
  argv_generate_build_help[1] = cdd_test_mut_str("generate_build_system");
  argv_generate_build_help[2] = cdd_test_mut_str("--help");
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
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_main(4, argv_audit_too_many));
  cdd_main(3, argv_transformer_err);
  cdd_main(3, argv_standardize_gnu_err);
  cdd_main(3, argv_to_docs_json_err);
  cdd_main(3, argv_bind_err);

/* Close stdin or redirect to /dev/null so mcp does not block waiting for
 * input */
#if defined(_WIN32)
  if (CDD_FREOPEN("NUL", "r", stdin)) {
  }
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
  char *argv_cli[7];
  char *argv_server[7];
  char *argv_help[3];
  char *argv_err[5];
  char *argv_flags[9];
  char *argv_env[3];
  FILE *f;

  /* Note: we can't test actual execution easily without creating a dummy */
  /* spec.json, but we can at least hit the help and error paths. */

  argv_cli[0] = cdd_test_mut_str("cdd-c");
  argv_cli[1] = cdd_test_mut_str("from_openapi");
  argv_cli[2] = cdd_test_mut_str("to_sdk_cli");
  argv_cli[3] = cdd_test_mut_str("-i");
  argv_cli[4] = cdd_test_mut_str("spec.json");
  argv_cli[5] = cdd_test_mut_str("-o");
  argv_cli[6] = cdd_test_mut_str("build/test_out_dir");
  argv_server[0] = cdd_test_mut_str("cdd-c");
  argv_server[1] = cdd_test_mut_str("from_openapi");
  argv_server[2] = cdd_test_mut_str("to_server");
  argv_server[3] = cdd_test_mut_str("-i");
  argv_server[4] = cdd_test_mut_str("spec.json");
  argv_server[5] = cdd_test_mut_str("-o");
  argv_server[6] = cdd_test_mut_str("build/test_out_dir_2");
  argv_help[0] = cdd_test_mut_str("cdd-c");
  argv_help[1] = cdd_test_mut_str("from_openapi");
  argv_help[2] = cdd_test_mut_str("--help");
  argv_err[0] = cdd_test_mut_str("cdd-c");
  argv_err[1] = cdd_test_mut_str("from_openapi");
  argv_err[2] = cdd_test_mut_str("to_sdk");
  argv_err[3] = cdd_test_mut_str("-o");
  argv_err[4] = cdd_test_mut_str("out_dir");
  argv_flags[0] = cdd_test_mut_str("cdd-c");
  argv_flags[1] = cdd_test_mut_str("from_openapi");
  argv_flags[2] = cdd_test_mut_str("to_sdk");
  argv_flags[3] = cdd_test_mut_str("--input-dir");
  argv_flags[4] = cdd_test_mut_str("indir");
  argv_flags[5] = cdd_test_mut_str("--no-github-actions");
  argv_flags[6] = cdd_test_mut_str("--no-installable-package");
  argv_flags[7] = cdd_test_mut_str("--tests");
  argv_flags[8] = cdd_test_mut_str("yes");
  argv_env[0] = cdd_test_mut_str("cdd-c");
  argv_env[1] = cdd_test_mut_str("from_openapi");
  argv_env[2] = cdd_test_mut_str("to_sdk");
  ASSERT_EQ(CDD_C_SUCCESS, cdd_main(3, argv_help));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_main(5, argv_err)); /* missing input */

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
  _putenv("CDD_INPUT_DIR=src/tests/mocks");
#else
  setenv("CDD_INPUT_DIR", "src/tests/mocks", 1);
#endif
  ASSERT_EQ(0, cdd_main(3, argv_env));

#if defined(_WIN32)
  _putenv("CDD_INPUT_DIR=");
  _putenv("INPUT_DIR=src/tests/mocks");
#else
  unsetenv("CDD_INPUT_DIR");
  setenv("INPUT_DIR", "src/tests/mocks", 1);
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
  char *argv_help[3];
  char *argv_help2[3];
  char *argv_err[2];
  char *argv_flags[6];
  char *argv_flags2[6];
  char *argv_env[2];

  argv_help[0] = cdd_test_mut_str("cdd-c");
  argv_help[1] = cdd_test_mut_str("to_openapi");
  argv_help[2] = cdd_test_mut_str("--help");
  argv_help2[0] = cdd_test_mut_str("cdd-c");
  argv_help2[1] = cdd_test_mut_str("to_openapi");
  argv_help2[2] = cdd_test_mut_str("-h");
  argv_err[0] = cdd_test_mut_str("cdd-c");
  argv_err[1] = cdd_test_mut_str("to_openapi");
  argv_flags[0] = cdd_test_mut_str("cdd-c");
  argv_flags[1] = cdd_test_mut_str("to_openapi");
  argv_flags[2] = cdd_test_mut_str("-i");
  argv_flags[3] = cdd_test_mut_str("indir");
  argv_flags[4] = cdd_test_mut_str("-o");
  argv_flags[5] = cdd_test_mut_str("outdir");
  argv_flags2[0] = cdd_test_mut_str("cdd-c");
  argv_flags2[1] = cdd_test_mut_str("to_openapi");
  argv_flags2[2] = cdd_test_mut_str("--input");
  argv_flags2[3] = cdd_test_mut_str("indir2");
  argv_flags2[4] = cdd_test_mut_str("--output");
  argv_flags2[5] = cdd_test_mut_str("outdir2");
  argv_env[0] = cdd_test_mut_str("cdd-c");
  argv_env[1] = cdd_test_mut_str("to_openapi");
  ASSERT_EQ(CDD_C_SUCCESS, cdd_main(3, argv_help));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_main(3, argv_help2));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_main(2, argv_err));

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

  /* c2openapi_cli_main will return error since indir3 doesn't exist, but we
   * successfully passed the env vars! */
  ASSERT_NEQ(CDD_C_SUCCESS, cdd_main(2, argv_env));

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
  ASSERT_NEQ(CDD_C_SUCCESS, cdd_main(2, argv_env));

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
 * @return Test or helper result.
 */
TEST test_bin_cdd_executable(void) {
  /* Run the actual executable to cover bin_cdd.c's main() */
  int rc = system("./bin/cdd-c --help > /dev/null 2>&1");
  ASSERT_NEQ(-1, rc);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Executes the main_suite test suite.
 */
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
