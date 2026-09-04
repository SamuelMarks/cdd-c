#ifndef TEST_MAIN_COVERAGE_H
#define TEST_MAIN_COVERAGE_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "functions/parse/main.h"
#include <greatest.h>
/* clang-format on */

extern C_CDD_EXPORT cdd_c_error_t print_version(void);
extern C_CDD_EXPORT cdd_c_error_t print_help(const char *program_name);
extern C_CDD_EXPORT cdd_c_error_t handle_audit(int argc, char **argv);
extern C_CDD_EXPORT cdd_c_error_t from_openapi_cli_main(int argc, char **argv);
extern C_CDD_EXPORT cdd_c_error_t to_openapi_cli_main(int argc, char **argv);

TEST test_main_coverage_print_version(void) {
  ASSERT_EQ(CDD_C_SUCCESS, print_version());
  PASS();
}

TEST test_main_coverage_print_help(void) {
  ASSERT_EQ(CDD_C_SUCCESS, print_help("cdd-c"));
  PASS();
}

TEST test_main_coverage_handle_audit(void) {
  char *argv[] = {(char *)(size_t)"dir1", (char *)(size_t)"dir2"};
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, handle_audit(2, argv));
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, handle_audit(0, argv));
  PASS();
}

TEST test_main_coverage_handle_audit_valid(void) {
  char *argv[] = {(char *)(size_t)"my_empty_dir"};
  handle_audit(1, argv);
  PASS();
}

TEST test_main_coverage_from_openapi(void) {
  char *argv_no_args[] = {(char *)(size_t)"from_openapi"};
  char *argv_to_sdk[] = {
      (char *)(size_t)"from_openapi", (char *)(size_t)"to_sdk",
      (char *)(size_t)"-i",           (char *)(size_t)"missing.json",
      (char *)(size_t)"-o",           (char *)(size_t)"out"};
  char *argv_to_sdk_cli[] = {
      (char *)(size_t)"from_openapi", (char *)(size_t)"to_sdk_cli",
      (char *)(size_t)"--input-dir",  (char *)(size_t)"missing_dir",
      (char *)(size_t)"-o",           (char *)(size_t)"out"};
  char *argv_to_server[] = {
      (char *)(size_t)"from_openapi", (char *)(size_t)"to_server",
      (char *)(size_t)"-i",           (char *)(size_t)"missing.json",
      (char *)(size_t)"-o",           (char *)(size_t)"out"};

  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, from_openapi_cli_main(1, argv_no_args));
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, from_openapi_cli_main(6, argv_to_sdk));
  ASSERT_EQ(CDD_C_SUCCESS, from_openapi_cli_main(6, argv_to_sdk_cli));
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, from_openapi_cli_main(6, argv_to_server));
  PASS();
}

TEST test_main_coverage_from_openapi_opts(void) {
  char *argv_help[] = {(char *)(size_t)"from_openapi",
                       (char *)(size_t)"--help"};
  char *argv_opts[] = {(char *)(size_t)"from_openapi",
                       (char *)(size_t)"to_sdk",
                       (char *)(size_t)"-i",
                       (char *)(size_t)"missing.json",
                       (char *)(size_t)"--no-github-actions",
                       (char *)(size_t)"--no-installable-package",
                       (char *)(size_t)"--tests"};
  char *argv_out[] = {
      (char *)(size_t)"from_openapi", (char *)(size_t)"to_sdk",
      (char *)(size_t)"-i",           (char *)(size_t)"missing.json",
      (char *)(size_t)"-o",           (char *)(size_t)"out_dir"};

  ASSERT_EQ(CDD_C_SUCCESS, from_openapi_cli_main(2, argv_help));
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, from_openapi_cli_main(7, argv_opts));
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, from_openapi_cli_main(6, argv_out));
  PASS();
}

TEST test_main_coverage_from_openapi_valid(void) {
  FILE *f;
#if defined(_MSC_VER)
  if (fopen_s(&f, "dummy_spec.json", "w") != 0)
    f = NULL;
#else
  f = fopen("dummy_spec.json", "w");
#endif
  if (f) {
    fputs("{\"openapi\": \"3.0.0\", \"info\": {\"title\": \"A\", \"version\": "
          "\"1\"}, \"paths\": {}}",
          f);
    if (f)
      fclose(f);
  }
  {
    char *argv_to_sdk[] = {
        (char *)(size_t)"from_openapi", (char *)(size_t)"to_sdk",
        (char *)(size_t)"-i",           (char *)(size_t)"dummy_spec.json",
        (char *)(size_t)"-o",           (char *)(size_t)"out_dir"};
    ASSERT_EQ(CDD_C_SUCCESS, from_openapi_cli_main(6, argv_to_sdk));
    PASS();
  }
}

TEST test_main_coverage_from_openapi_invalid(void) {
  FILE *f;
#if defined(_MSC_VER)
  if (fopen_s(&f, "invalid_spec.json", "w") != 0)
    f = NULL;
#else
  f = fopen("invalid_spec.json", "w");
#endif
  if (f) {
    fputs("invalid json", f);
    if (f)
      fclose(f);
  }
  {
    char *argv_invalid[] = {
        (char *)(size_t)"from_openapi", (char *)(size_t)"to_sdk",
        (char *)(size_t)"-i",           (char *)(size_t)"invalid_spec.json",
        (char *)(size_t)"-o",           (char *)(size_t)"out_dir"};
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, from_openapi_cli_main(6, argv_invalid));
    PASS();
  }
}

TEST test_main_coverage_from_openapi_cli_server(void) {
  FILE *f;
#if defined(_MSC_VER)
  if (fopen_s(&f, "dummy_spec.json", "w") != 0)
    f = NULL;
#else
  f = fopen("dummy_spec.json", "w");
#endif
  if (f) {
    fputs("{\"openapi\": \"3.0.0\", \"info\": {\"title\": \"A\", \"version\": "
          "\"1\"}, \"paths\": {}}",
          f);
    if (f)
      fclose(f);
  }
  {
    char *argv_cli[] = {
        (char *)(size_t)"from_openapi", (char *)(size_t)"to_sdk_cli",
        (char *)(size_t)"-i",           (char *)(size_t)"dummy_spec.json",
        (char *)(size_t)"-o",           (char *)(size_t)"out_dir"};
    char *argv_server[] = {
        (char *)(size_t)"from_openapi", (char *)(size_t)"to_server",
        (char *)(size_t)"-i",           (char *)(size_t)"dummy_spec.json",
        (char *)(size_t)"-o",           (char *)(size_t)"out_dir"};

    ASSERT_EQ(CDD_C_SUCCESS, from_openapi_cli_main(6, argv_cli));
    ASSERT_EQ(CDD_C_SUCCESS, from_openapi_cli_main(6, argv_server));
    PASS();
  }
}

TEST test_main_coverage_cdd_main(void) {
  char *argv_ver[] = {(char *)(size_t)"cdd-c", (char *)(size_t)"--version"};
  char *argv_help[] = {(char *)(size_t)"cdd-c", (char *)(size_t)"--help"};
  char *argv_err[] = {(char *)(size_t)"cdd-c", (char *)(size_t)"unknown"};

  ASSERT_EQ(CDD_C_SUCCESS, cdd_main(2, argv_ver));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_main(2, argv_help));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_main(2, argv_err));
  PASS();
}

TEST test_main_coverage_to_openapi(void) {
  char *argv_help[] = {(char *)(size_t)"to_openapi", (char *)(size_t)"--help"};
  char *argv_no_args[] = {(char *)(size_t)"to_openapi"};
  char *argv_args[] = {(char *)(size_t)"to_openapi", (char *)(size_t)"-i",
                       (char *)(size_t)"my_empty_dir", (char *)(size_t)"-o",
                       (char *)(size_t)"out.json"};

  ASSERT_EQ(CDD_C_SUCCESS, to_openapi_cli_main(2, argv_help));
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, to_openapi_cli_main(1, argv_no_args));
  to_openapi_cli_main(5, argv_args);
  PASS();
}

TEST test_main_coverage_cdd_main_subcommands(void) {
  FILE *f;
#if defined(_MSC_VER)
  if (fopen_s(&f, "dummy_spec.json", "w") != 0)
    f = NULL;
#else
  f = fopen("dummy_spec.json", "w");
#endif
  if (f) {
    fputs("{\"openapi\": \"3.0.0\", \"info\": {\"title\": \"A\", \"version\": "
          "\"1\"}, \"paths\": {}}",
          f);
    if (f)
      fclose(f);
  }
  {
    char *argv_audit[] = {(char *)(size_t)"cdd-c", (char *)(size_t)"audit",
                          (char *)(size_t)"a", (char *)(size_t)"b"};
    char *argv_c2openapi[] = {(char *)(size_t)"cdd-c",
                              (char *)(size_t)"c2openapi", (char *)(size_t)"a",
                              (char *)(size_t)"b"};
    char *argv_transformer[] = {(char *)(size_t)"cdd-c",
                                (char *)(size_t)"transformer",
                                (char *)(size_t)"a"};
    char *argv_standardize[] = {(char *)(size_t)"cdd-c",
                                (char *)(size_t)"standardize-gnu"};
    char *argv_code2schema[] = {(char *)(size_t)"cdd-c",
                                (char *)(size_t)"code2schema",
                                (char *)(size_t)"a", (char *)(size_t)"b"};
    char *argv_gen_build[] = {(char *)(size_t)"cdd-c",
                              (char *)(size_t)"generate_build_system",
                              (char *)(size_t)"a"};
    char *argv_schema2code[] = {(char *)(size_t)"cdd-c",
                                (char *)(size_t)"schema2code",
                                (char *)(size_t)"a"};
    char *argv_to_docs[] = {(char *)(size_t)"cdd-c",
                            (char *)(size_t)"to_docs_json",
                            (char *)(size_t)"a"};
    char *argv_bind[] = {(char *)(size_t)"cdd-c", (char *)(size_t)"bind",
                         (char *)(size_t)"a"};
    char *argv_from_openapi[] = {(char *)(size_t)"cdd-c",
                                 (char *)(size_t)"from_openapi",
                                 (char *)(size_t)"a"};
    char *argv_to_openapi[] = {(char *)(size_t)"cdd-c",
                               (char *)(size_t)"to_openapi",
                               (char *)(size_t)"a"};

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
}

TEST test_main_coverage_cdd_main_success(void) {
  FILE *f;
#if defined(_MSC_VER)
  if (fopen_s(&f, "dummy_spec.json", "w") != 0)
    f = NULL;
#else
  f = fopen("dummy_spec.json", "w");
#endif
  if (f) {
    fputs("{\"openapi\": \"3.0.0\", \"info\": {\"title\": \"A\", \"version\": "
          "\"1\"}, \"paths\": {}}",
          f);
    if (f)
      fclose(f);
  }
  {
    char *argv_to_openapi[] = {
        (char *)(size_t)"cdd-c", (char *)(size_t)"to_openapi",
        (char *)(size_t)"-i",    (char *)(size_t)"my_empty_dir",
        (char *)(size_t)"-o",    (char *)(size_t)"out.json"};
    char *argv_from_openapi[] = {
        (char *)(size_t)"cdd-c",           (char *)(size_t)"from_openapi",
        (char *)(size_t)"to_sdk",          (char *)(size_t)"-i",
        (char *)(size_t)"dummy_spec.json", (char *)(size_t)"-o",
        (char *)(size_t)"out_dir"};
    char *argv_c2openapi[] = {
        (char *)(size_t)"cdd-c", (char *)(size_t)"c2openapi",
        (char *)(size_t)"my_empty_dir", (char *)(size_t)"out.json"};
    char *argv_code2schema[] = {(char *)(size_t)"cdd-c",
                                (char *)(size_t)"code2schema",
                                (char *)(size_t)"my_empty_dir/empty.h",
                                (char *)(size_t)"out_schema.json"};
    char *argv_transformer[] = {
        (char *)(size_t)"cdd-c", (char *)(size_t)"transformer",
        (char *)(size_t)"safe_crt", (char *)(size_t)"my_empty_dir/empty.c"};
    char *argv_standardize[] = {(char *)(size_t)"cdd-c",
                                (char *)(size_t)"standardize-gnu",
                                (char *)(size_t)"my_empty_dir/empty.c"};
    char *argv_audit[] = {(char *)(size_t)"cdd-c", (char *)(size_t)"audit",
                          (char *)(size_t)"my_empty_dir"};
    char *argv_gen_build[] = {
        (char *)(size_t)"cdd-c", (char *)(size_t)"generate_build_system",
        (char *)(size_t)"cmake", (char *)(size_t)"my_empty_dir",
        (char *)(size_t)"test"};
    char *argv_schema2code[] = {
        (char *)(size_t)"cdd-c", (char *)(size_t)"schema2code",
        (char *)(size_t)"dummy_spec.json", (char *)(size_t)"out_dir"};

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
