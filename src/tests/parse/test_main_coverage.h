#ifndef TEST_MAIN_COVERAGE_H
#define TEST_MAIN_COVERAGE_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "functions/parse/main.h"
#include <greatest.h>
/* clang-format on */

extern cdd_c_error_t print_version(void);
extern cdd_c_error_t print_help(const char *program_name);
extern cdd_c_error_t handle_audit(int argc, char **argv);
extern cdd_c_error_t from_openapi_cli_main(int argc, char **argv);
extern cdd_c_error_t to_openapi_cli_main(int argc, char **argv);

TEST test_main_coverage_print_version(void) {
  ASSERT_EQ(CDD_C_SUCCESS, print_version());
  PASS();
}

TEST test_main_coverage_print_help(void) {
  ASSERT_EQ(CDD_C_SUCCESS, print_help("cdd-c"));
  PASS();
}

TEST test_main_coverage_handle_audit(void) {
  char *argv[] = {"dir1", "dir2"};
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, handle_audit(2, argv));
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, handle_audit(0, argv));
  PASS();
}

TEST test_main_coverage_handle_audit_valid(void) {
  char *argv[] = {"my_empty_dir"};
  handle_audit(1, argv);
  PASS();
}

TEST test_main_coverage_from_openapi(void) {
  char *argv_no_args[] = {"from_openapi"};
  char *argv_to_sdk[] = {"from_openapi", "to_sdk", "-i",
                         "missing.json", "-o",     "out"};
  char *argv_to_sdk_cli[] = {"from_openapi", "to_sdk_cli", "--input-dir",
                             "missing_dir",  "-o",         "out"};
  char *argv_to_server[] = {"from_openapi", "to_server", "-i",
                            "missing.json", "-o",        "out"};

  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, from_openapi_cli_main(1, argv_no_args));
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, from_openapi_cli_main(6, argv_to_sdk));
  ASSERT_EQ(CDD_C_SUCCESS, from_openapi_cli_main(6, argv_to_sdk_cli));
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, from_openapi_cli_main(6, argv_to_server));
  PASS();
}

TEST test_main_coverage_from_openapi_opts(void) {
  char *argv_help[] = {"from_openapi", "--help"};
  char *argv_opts[] = {"from_openapi",
                       "to_sdk",
                       "-i",
                       "missing.json",
                       "--no-github-actions",
                       "--no-installable-package",
                       "--tests"};
  char *argv_out[] = {"from_openapi", "to_sdk", "-i",
                      "missing.json", "-o",     "out_dir"};

  ASSERT_EQ(CDD_C_SUCCESS, from_openapi_cli_main(2, argv_help));
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, from_openapi_cli_main(7, argv_opts));
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, from_openapi_cli_main(6, argv_out));
  PASS();
}

TEST test_main_coverage_from_openapi_valid(void) {
  FILE *f = fopen("dummy_spec.json", "w");
  if (f) {
    fputs("{\"openapi\": \"3.0.0\", \"info\": {\"title\": \"A\", \"version\": "
          "\"1\"}, \"paths\": {}}",
          f);
    fclose(f);
  }
  char *argv_to_sdk[] = {"from_openapi",    "to_sdk", "-i",
                         "dummy_spec.json", "-o",     "out_dir"};
  ASSERT_EQ(CDD_C_SUCCESS, from_openapi_cli_main(6, argv_to_sdk));
  PASS();
}

TEST test_main_coverage_from_openapi_invalid(void) {
  FILE *f = fopen("invalid_spec.json", "w");
  if (f) {
    fputs("invalid json", f);
    fclose(f);
  }
  char *argv_invalid[] = {"from_openapi",      "to_sdk", "-i",
                          "invalid_spec.json", "-o",     "out_dir"};
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, from_openapi_cli_main(6, argv_invalid));
  PASS();
}

TEST test_main_coverage_from_openapi_cli_server(void) {
  FILE *f = fopen("dummy_spec.json", "w");
  if (f) {
    fputs("{\"openapi\": \"3.0.0\", \"info\": {\"title\": \"A\", \"version\": "
          "\"1\"}, \"paths\": {}}",
          f);
    fclose(f);
  }
  char *argv_cli[] = {"from_openapi",    "to_sdk_cli", "-i",
                      "dummy_spec.json", "-o",         "out_dir"};
  char *argv_server[] = {"from_openapi",    "to_server", "-i",
                         "dummy_spec.json", "-o",        "out_dir"};

  ASSERT_EQ(CDD_C_SUCCESS, from_openapi_cli_main(6, argv_cli));
  ASSERT_EQ(CDD_C_SUCCESS, from_openapi_cli_main(6, argv_server));
  PASS();
}

TEST test_main_coverage_cdd_main(void) {
  char *argv_ver[] = {"cdd-c", "--version"};
  char *argv_help[] = {"cdd-c", "--help"};
  char *argv_err[] = {"cdd-c", "unknown"};

  ASSERT_EQ(CDD_C_SUCCESS, cdd_main(2, argv_ver));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_main(2, argv_help));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_main(2, argv_err));
  PASS();
}

TEST test_main_coverage_to_openapi(void) {
  char *argv_help[] = {"to_openapi", "--help"};
  char *argv_no_args[] = {"to_openapi"};
  char *argv_args[] = {"to_openapi", "-i", "my_empty_dir", "-o", "out.json"};

  ASSERT_EQ(CDD_C_SUCCESS, to_openapi_cli_main(2, argv_help));
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, to_openapi_cli_main(1, argv_no_args));
  to_openapi_cli_main(5, argv_args);
  PASS();
}

TEST test_main_coverage_cdd_main_subcommands(void) {
  FILE *f = fopen("dummy_spec.json", "w");
  if (f) {
    fputs("{\"openapi\": \"3.0.0\", \"info\": {\"title\": \"A\", \"version\": "
          "\"1\"}, \"paths\": {}}",
          f);
    fclose(f);
  }
  char *argv_audit[] = {"cdd-c", "audit", "a", "b"};
  char *argv_c2openapi[] = {"cdd-c", "c2openapi", "a", "b"};
  char *argv_transformer[] = {"cdd-c", "transformer", "a"};
  char *argv_standardize[] = {"cdd-c", "standardize-gnu"};
  char *argv_code2schema[] = {"cdd-c", "code2schema", "a", "b"};
  char *argv_gen_build[] = {"cdd-c", "generate_build_system", "a"};
  char *argv_schema2code[] = {"cdd-c", "schema2code", "a"};
  char *argv_to_docs[] = {"cdd-c", "to_docs_json", "a"};
  char *argv_bind[] = {"cdd-c", "bind", "a"};
  char *argv_from_openapi[] = {"cdd-c", "from_openapi", "a"};
  char *argv_to_openapi[] = {"cdd-c", "to_openapi", "a"};

  cdd_main(2, argv_audit);
  cdd_main(2, argv_c2openapi);
  cdd_main(3, argv_transformer);
  cdd_main(2, argv_standardize);
  cdd_main(3, argv_code2schema);
  cdd_main(3, argv_gen_build);
  cdd_main(3, argv_schema2code);
  cdd_main(3, argv_to_docs);
  cdd_main(3, argv_bind);
  cdd_main(3, argv_from_openapi);
  cdd_main(3, argv_to_openapi);

  PASS();
}

TEST test_main_coverage_cdd_main_success(void) {
  FILE *f = fopen("dummy_spec.json", "w");
  if (f) {
    fputs("{\"openapi\": \"3.0.0\", \"info\": {\"title\": \"A\", \"version\": "
          "\"1\"}, \"paths\": {}}",
          f);
    fclose(f);
  }
  char *argv_to_openapi[] = {"cdd-c",        "to_openapi", "-i",
                             "my_empty_dir", "-o",         "out.json"};
  char *argv_from_openapi[] = {"cdd-c",  "from_openapi",    "to_sdk",
                               "-i",     "dummy_spec.json", "-o",
                               "out_dir"};
  char *argv_c2openapi[] = {"cdd-c", "c2openapi", "my_empty_dir", "out.json"};
  char *argv_code2schema[] = {"cdd-c", "code2schema", "my_empty_dir/empty.h",
                              "out_schema.json"};
  char *argv_transformer[] = {"cdd-c", "transformer", "safe_crt",
                              "my_empty_dir/empty.c"};
  char *argv_standardize[] = {"cdd-c", "standardize-gnu",
                              "my_empty_dir/empty.c"};
  char *argv_audit[] = {"cdd-c", "audit", "my_empty_dir"};
  char *argv_gen_build[] = {"cdd-c", "generate_build_system", "cmake",
                            "my_empty_dir", "test"};
  char *argv_schema2code[] = {"cdd-c", "schema2code", "dummy_spec.json",
                              "out_dir"};

  cdd_main(6, argv_to_openapi);
  cdd_main(7, argv_from_openapi);
  cdd_main(4, argv_c2openapi);
  cdd_main(4, argv_code2schema);
  cdd_main(4, argv_transformer);
  cdd_main(3, argv_standardize);
  cdd_main(3, argv_audit);
  cdd_main(5, argv_gen_build);
  cdd_main(4, argv_schema2code);

  PASS();
}

SUITE(main_coverage_suite) {
  RUN_TEST(test_main_coverage_print_version);
  RUN_TEST(test_main_coverage_print_help);
  RUN_TEST(test_main_coverage_handle_audit);
  RUN_TEST(test_main_coverage_handle_audit_valid);
  RUN_TEST(test_main_coverage_from_openapi);
  RUN_TEST(test_main_coverage_from_openapi_opts);
  RUN_TEST(test_main_coverage_from_openapi_valid);
  RUN_TEST(test_main_coverage_from_openapi_invalid);
  RUN_TEST(test_main_coverage_from_openapi_cli_server);
  RUN_TEST(test_main_coverage_cdd_main);
  RUN_TEST(test_main_coverage_to_openapi);
  RUN_TEST(test_main_coverage_cdd_main_subcommands);
  RUN_TEST(test_main_coverage_cdd_main_success);
}

#ifdef __cplusplus
}
#endif
#endif
