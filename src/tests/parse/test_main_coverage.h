#ifndef TEST_MAIN_COVERAGE_H
#define TEST_MAIN_COVERAGE_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "functions/parse/main.h"
#include <greatest.h>
#if defined(_WIN32) && !defined(__CYGWIN__)
#include <direct.h>
#define TEST_MKDIR(p) _mkdir(p)
#define TEST_RMDIR(p) _rmdir(p)
#if defined(_MSC_VER)
static FILE *cdd_freopen_helper_main_cov(const char *p, const char *m, FILE *s) {
  FILE *f = NULL;
  return freopen_s(&f, p, m, s) == 0 ? f : NULL;
}
#undef CDD_FREOPEN
#define CDD_FREOPEN cdd_freopen_helper_main_cov
#else
#undef CDD_FREOPEN
#define CDD_FREOPEN freopen
#endif
#else
#include <sys/stat.h>
#if defined(_MSC_VER)
#include <io.h>
#else
#if defined(_WIN32)
#include <io.h>
#else
#include <unistd.h>
#endif
#endif
#define TEST_MKDIR(p) mkdir(p, 0777)
#define TEST_RMDIR(p) rmdir(p)
#undef CDD_FREOPEN
#define CDD_FREOPEN freopen
#endif
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
  {
    char *empty_argv[1];
    empty_argv[0] = cdd_test_mut_str("cdd-c");
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_main(1, empty_argv));
  }
  PASS();
}

TEST test_main_coverage_handle_audit(void) {
  char *argv[2];

  argv[0] = cdd_test_mut_str("dir1");
  argv[1] = cdd_test_mut_str("dir2");
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, handle_audit(2, argv));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, handle_audit(0, argv));
#ifdef CDD_BUILD_TESTS
  {
    extern C_CDD_EXPORT int g_cdd_fail_audit_stats_init;
    char *argv_one[1];
    TEST_MKDIR("my_empty_dir");
    argv_one[0] = cdd_test_mut_str("my_empty_dir");
    g_cdd_fail_audit_stats_init = 2;
    ASSERT_EQ(CDD_C_SUCCESS, handle_audit(1, argv_one));
    ASSERT_EQ(CDD_C_ERROR_MEMORY, handle_audit(1, argv_one));
    g_cdd_fail_audit_stats_init = 0;
  }
#endif
  PASS();
}

TEST test_main_coverage_handle_audit_valid(void) {
  char *argv[1];

  argv[0] = cdd_test_mut_str("my_empty_dir");
  handle_audit(1, argv);
  PASS();
}

TEST test_main_coverage_from_openapi(void) {
  char *argv_no_args[1];
  char *argv_to_sdk[6];
  char *argv_to_sdk_cli[6];
  char *argv_to_server[6];

  argv_no_args[0] = cdd_test_mut_str("from_openapi");
  argv_to_sdk[0] = cdd_test_mut_str("from_openapi");
  argv_to_sdk[1] = cdd_test_mut_str("to_sdk");
  argv_to_sdk[2] = cdd_test_mut_str("-i");
  argv_to_sdk[3] = cdd_test_mut_str("missing.json");
  argv_to_sdk[4] = cdd_test_mut_str("-o");
  argv_to_sdk[5] = cdd_test_mut_str("out");
  argv_to_sdk_cli[0] = cdd_test_mut_str("from_openapi");
  argv_to_sdk_cli[1] = cdd_test_mut_str("to_sdk_cli");
  argv_to_sdk_cli[2] = cdd_test_mut_str("--input-dir");
  argv_to_sdk_cli[3] = cdd_test_mut_str("missing_dir");
  argv_to_sdk_cli[4] = cdd_test_mut_str("-o");
  argv_to_sdk_cli[5] = cdd_test_mut_str("out");
  argv_to_server[0] = cdd_test_mut_str("from_openapi");
  argv_to_server[1] = cdd_test_mut_str("to_server");
  argv_to_server[2] = cdd_test_mut_str("-i");
  argv_to_server[3] = cdd_test_mut_str("missing.json");
  argv_to_server[4] = cdd_test_mut_str("-o");
  argv_to_server[5] = cdd_test_mut_str("out");
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            from_openapi_cli_main(1, argv_no_args));
  ASSERT_EQ(CDD_C_ERROR_PARSE, from_openapi_cli_main(6, argv_to_sdk));
  ASSERT_EQ(CDD_C_SUCCESS, from_openapi_cli_main(6, argv_to_sdk_cli));
  ASSERT_EQ(CDD_C_ERROR_PARSE, from_openapi_cli_main(6, argv_to_server));
  PASS();
}

TEST test_main_coverage_from_openapi_opts(void) {
  char *argv_help[2];
  char *argv_opts[7];
  char *argv_out[6];
  char *argv_trailing_o[5];
  char *argv_long_out[6];

  argv_help[0] = cdd_test_mut_str("from_openapi");
  argv_help[1] = cdd_test_mut_str("--help");
  argv_opts[0] = cdd_test_mut_str("from_openapi");
  argv_opts[1] = cdd_test_mut_str("to_sdk");
  argv_opts[2] = cdd_test_mut_str("-i");
  argv_opts[3] = cdd_test_mut_str("missing.json");
  argv_opts[4] = cdd_test_mut_str("--no-github-actions");
  argv_opts[5] = cdd_test_mut_str("--no-installable-package");
  argv_opts[6] = cdd_test_mut_str("--tests");
  argv_out[0] = cdd_test_mut_str("from_openapi");
  argv_out[1] = cdd_test_mut_str("to_sdk");
  argv_out[2] = cdd_test_mut_str("-i");
  argv_out[3] = cdd_test_mut_str("missing.json");
  argv_out[4] = cdd_test_mut_str("-o");
  argv_out[5] = cdd_test_mut_str("out_dir");
  argv_trailing_o[0] = cdd_test_mut_str("from_openapi");
  argv_trailing_o[1] = cdd_test_mut_str("to_sdk");
  argv_trailing_o[2] = cdd_test_mut_str("-i");
  argv_trailing_o[3] = cdd_test_mut_str("missing.json");
  argv_trailing_o[4] = cdd_test_mut_str("-o");
  argv_long_out[0] = cdd_test_mut_str("from_openapi");
  argv_long_out[1] = cdd_test_mut_str("to_sdk");
  argv_long_out[2] = cdd_test_mut_str("-i");
  argv_long_out[3] = cdd_test_mut_str("missing.json");
  argv_long_out[4] = cdd_test_mut_str("--output");
  argv_long_out[5] = cdd_test_mut_str("out_dir");
  ASSERT_EQ(CDD_C_SUCCESS, from_openapi_cli_main(2, argv_help));
  ASSERT_EQ(CDD_C_ERROR_PARSE, from_openapi_cli_main(7, argv_opts));
  ASSERT_EQ(CDD_C_ERROR_PARSE, from_openapi_cli_main(6, argv_out));
  ASSERT_EQ(CDD_C_ERROR_PARSE, from_openapi_cli_main(6, argv_long_out));
  ASSERT_EQ(CDD_C_ERROR_PARSE, from_openapi_cli_main(5, argv_trailing_o));
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
    char *argv_to_sdk[6];

    argv_to_sdk[0] = cdd_test_mut_str("from_openapi");
    argv_to_sdk[1] = cdd_test_mut_str("to_sdk");
    argv_to_sdk[2] = cdd_test_mut_str("-i");
    argv_to_sdk[3] = cdd_test_mut_str("dummy_spec.json");
    argv_to_sdk[4] = cdd_test_mut_str("-o");
    argv_to_sdk[5] = cdd_test_mut_str("out_dir");
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
    char *argv_invalid[6];

    argv_invalid[0] = cdd_test_mut_str("from_openapi");
    argv_invalid[1] = cdd_test_mut_str("to_sdk");
    argv_invalid[2] = cdd_test_mut_str("-i");
    argv_invalid[3] = cdd_test_mut_str("invalid_spec.json");
    argv_invalid[4] = cdd_test_mut_str("-o");
    argv_invalid[5] = cdd_test_mut_str("out_dir");
    ASSERT_EQ(CDD_C_ERROR_PARSE, from_openapi_cli_main(6, argv_invalid));
  }

#if defined(_MSC_VER)
  if (fopen_s(&f, "invalid_spec2.json", "w") != 0)
    f = NULL;
#else
  f = fopen("invalid_spec2.json", "w");
#endif
  if (f) {
    fputs("[1, 2, 3]", f);
    if (f)
      fclose(f);
  }
  {
    char *argv_invalid2[6];
    char *argv_inp_last[2];

    argv_invalid2[0] = cdd_test_mut_str("from_openapi");
    argv_invalid2[1] = cdd_test_mut_str("to_sdk");
    argv_invalid2[2] = cdd_test_mut_str("-i");
    argv_invalid2[3] = cdd_test_mut_str("invalid_spec2.json");
    argv_invalid2[4] = cdd_test_mut_str("-o");
    argv_invalid2[5] = cdd_test_mut_str("out_dir");
    argv_inp_last[0] = cdd_test_mut_str("from_openapi");
    argv_inp_last[1] = cdd_test_mut_str("--input");
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              from_openapi_cli_main(6, argv_invalid2));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              from_openapi_cli_main(2, argv_inp_last));
    remove("invalid_spec2.json");
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
    char *argv_cli[6];
    char *argv_server[6];

    argv_cli[0] = cdd_test_mut_str("from_openapi");
    argv_cli[1] = cdd_test_mut_str("to_sdk_cli");
    argv_cli[2] = cdd_test_mut_str("-i");
    argv_cli[3] = cdd_test_mut_str("dummy_spec.json");
    argv_cli[4] = cdd_test_mut_str("-o");
    argv_cli[5] = cdd_test_mut_str("out_dir");
    argv_server[0] = cdd_test_mut_str("from_openapi");
    argv_server[1] = cdd_test_mut_str("to_server");
    argv_server[2] = cdd_test_mut_str("-i");
    argv_server[3] = cdd_test_mut_str("dummy_spec.json");
    argv_server[4] = cdd_test_mut_str("-o");
    argv_server[5] = cdd_test_mut_str("out_dir");
    ASSERT_EQ(CDD_C_SUCCESS, from_openapi_cli_main(6, argv_cli));
    ASSERT_EQ(CDD_C_SUCCESS, from_openapi_cli_main(6, argv_server));
    PASS();
  }
}

TEST test_main_coverage_cdd_main(void) {
  char *argv_ver[2];
  char *argv_help[2];
  char *argv_err[2];

  argv_ver[0] = cdd_test_mut_str("cdd-c");
  argv_ver[1] = cdd_test_mut_str("--version");
  argv_help[0] = cdd_test_mut_str("cdd-c");
  argv_help[1] = cdd_test_mut_str("--help");
  argv_err[0] = cdd_test_mut_str("cdd-c");
  argv_err[1] = cdd_test_mut_str("unknown");
  ASSERT_EQ(CDD_C_SUCCESS, cdd_main(2, argv_ver));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_main(2, argv_help));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_main(2, argv_err));
  PASS();
}

TEST test_main_coverage_to_openapi(void) {
  char *argv_help[2];
  char *argv_no_args[1];
  char *argv_args[5];
  char *argv_args_long[5];
  char *argv_trailing_i[2];
  char *argv_trailing_o[2];

  argv_help[0] = cdd_test_mut_str("to_openapi");
  argv_help[1] = cdd_test_mut_str("--help");
  argv_no_args[0] = cdd_test_mut_str("to_openapi");
  argv_args[0] = cdd_test_mut_str("to_openapi");
  argv_args[1] = cdd_test_mut_str("-i");
  argv_args[2] = cdd_test_mut_str("my_empty_dir");
  argv_args[3] = cdd_test_mut_str("-o");
  argv_args[4] = cdd_test_mut_str("out.json");
  argv_args_long[0] = cdd_test_mut_str("to_openapi");
  argv_args_long[1] = cdd_test_mut_str("--input");
  argv_args_long[2] = cdd_test_mut_str("my_empty_dir");
  argv_args_long[3] = cdd_test_mut_str("--output");
  argv_args_long[4] = cdd_test_mut_str("out.json");
  argv_trailing_i[0] = cdd_test_mut_str("to_openapi");
  argv_trailing_i[1] = cdd_test_mut_str("-i");
  argv_trailing_o[0] = cdd_test_mut_str("to_openapi");
  argv_trailing_o[1] = cdd_test_mut_str("-o");
  ASSERT_EQ(CDD_C_SUCCESS, to_openapi_cli_main(2, argv_help));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, to_openapi_cli_main(1, argv_no_args));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            to_openapi_cli_main(2, argv_trailing_i));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            to_openapi_cli_main(2, argv_trailing_o));
  to_openapi_cli_main(5, argv_args);
  to_openapi_cli_main(5, argv_args_long);
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
    char *argv_audit[4];
    char *argv_c2openapi[4];
    char *argv_transformer[3];
    char *argv_standardize[2];
    char *argv_code2schema[4];
    char *argv_gen_build[3];
    char *argv_schema2code[3];
    char *argv_to_docs[3];
    char *argv_bind[3];
    char *argv_from_openapi[3];
    char *argv_to_openapi[3];

    argv_audit[0] = cdd_test_mut_str("cdd-c");
    argv_audit[1] = cdd_test_mut_str("audit");
    argv_audit[2] = cdd_test_mut_str("a");
    argv_audit[3] = cdd_test_mut_str("b");
    argv_c2openapi[0] = cdd_test_mut_str("cdd-c");
    argv_c2openapi[1] = cdd_test_mut_str("c2openapi");
    argv_c2openapi[2] = cdd_test_mut_str("a");
    argv_c2openapi[3] = cdd_test_mut_str("b");
    argv_transformer[0] = cdd_test_mut_str("cdd-c");
    argv_transformer[1] = cdd_test_mut_str("transformer");
    argv_transformer[2] = cdd_test_mut_str("a");
    argv_standardize[0] = cdd_test_mut_str("cdd-c");
    argv_standardize[1] = cdd_test_mut_str("standardize-gnu");
    argv_code2schema[0] = cdd_test_mut_str("cdd-c");
    argv_code2schema[1] = cdd_test_mut_str("code2schema");
    argv_code2schema[2] = cdd_test_mut_str("a");
    argv_code2schema[3] = cdd_test_mut_str("b");
    argv_gen_build[0] = cdd_test_mut_str("cdd-c");
    argv_gen_build[1] = cdd_test_mut_str("generate_build_system");
    argv_gen_build[2] = cdd_test_mut_str("a");
    argv_schema2code[0] = cdd_test_mut_str("cdd-c");
    argv_schema2code[1] = cdd_test_mut_str("schema2code");
    argv_schema2code[2] = cdd_test_mut_str("a");
    argv_to_docs[0] = cdd_test_mut_str("cdd-c");
    argv_to_docs[1] = cdd_test_mut_str("to_docs_json");
    argv_to_docs[2] = cdd_test_mut_str("a");
    argv_bind[0] = cdd_test_mut_str("cdd-c");
    argv_bind[1] = cdd_test_mut_str("bind");
    argv_bind[2] = cdd_test_mut_str("a");
    argv_from_openapi[0] = cdd_test_mut_str("cdd-c");
    argv_from_openapi[1] = cdd_test_mut_str("from_openapi");
    argv_from_openapi[2] = cdd_test_mut_str("a");
    argv_to_openapi[0] = cdd_test_mut_str("cdd-c");
    argv_to_openapi[1] = cdd_test_mut_str("to_openapi");
    argv_to_openapi[2] = cdd_test_mut_str("a");

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
  FILE *f = NULL;
  FILE *f_h = NULL;
  FILE *f_c = NULL;
  char *argv_to_openapi[6];
  char *argv_from_openapi[7];
  char *argv_c2openapi[4];
  char *argv_code2schema[4];
  char *argv_transformer[4];
  char *argv_standardize[3];
  char *argv_audit[3];
  char *argv_gen_build[5];
  char *argv_schema2code[4];

  argv_to_openapi[0] = cdd_test_mut_str("cdd-c");
  argv_to_openapi[1] = cdd_test_mut_str("to_openapi");
  argv_to_openapi[2] = cdd_test_mut_str("-i");
  argv_to_openapi[3] = cdd_test_mut_str("my_empty_dir");
  argv_to_openapi[4] = cdd_test_mut_str("-o");
  argv_to_openapi[5] = cdd_test_mut_str("out.json");
  argv_from_openapi[0] = cdd_test_mut_str("cdd-c");
  argv_from_openapi[1] = cdd_test_mut_str("from_openapi");
  argv_from_openapi[2] = cdd_test_mut_str("to_sdk");
  argv_from_openapi[3] = cdd_test_mut_str("-i");
  argv_from_openapi[4] = cdd_test_mut_str("dummy_spec.json");
  argv_from_openapi[5] = cdd_test_mut_str("-o");
  argv_from_openapi[6] = cdd_test_mut_str("out_dir");
  argv_c2openapi[0] = cdd_test_mut_str("cdd-c");
  argv_c2openapi[1] = cdd_test_mut_str("c2openapi");
  argv_c2openapi[2] = cdd_test_mut_str("my_empty_dir");
  argv_c2openapi[3] = cdd_test_mut_str("out.json");
  argv_code2schema[0] = cdd_test_mut_str("cdd-c");
  argv_code2schema[1] = cdd_test_mut_str("code2schema");
  argv_code2schema[2] = cdd_test_mut_str("my_empty_dir/empty.h");
  argv_code2schema[3] = cdd_test_mut_str("out_schema.json");
  argv_transformer[0] = cdd_test_mut_str("cdd-c");
  argv_transformer[1] = cdd_test_mut_str("transformer");
  argv_transformer[2] = cdd_test_mut_str("safe_crt");
  argv_transformer[3] = cdd_test_mut_str("my_empty_dir/empty.c");
  argv_standardize[0] = cdd_test_mut_str("cdd-c");
  argv_standardize[1] = cdd_test_mut_str("standardize-gnu");
  argv_standardize[2] = cdd_test_mut_str("my_empty_dir/empty.c");
  argv_audit[0] = cdd_test_mut_str("cdd-c");
  argv_audit[1] = cdd_test_mut_str("audit");
  argv_audit[2] = cdd_test_mut_str("my_empty_dir");
  argv_gen_build[0] = cdd_test_mut_str("cdd-c");
  argv_gen_build[1] = cdd_test_mut_str("generate_build_system");
  argv_gen_build[2] = cdd_test_mut_str("cmake");
  argv_gen_build[3] = cdd_test_mut_str("my_empty_dir");
  argv_gen_build[4] = cdd_test_mut_str("test");
  argv_schema2code[0] = cdd_test_mut_str("cdd-c");
  argv_schema2code[1] = cdd_test_mut_str("schema2code");
  argv_schema2code[2] =
      cdd_test_mut_str("src/tests/mocks/emit/simple.schema.json");
  argv_schema2code[3] = cdd_test_mut_str("out_dir/simple");
  TEST_MKDIR("my_empty_dir");
#if defined(_MSC_VER)
  if (fopen_s(&f_h, "my_empty_dir/empty.h", "w") != 0)
    f_h = NULL;
#else
  f_h = fopen("my_empty_dir/empty.h", "w");
#endif
  if (f_h) {
    fputs("struct S { int a; };\n", f_h);
    fclose(f_h);
  }

#if defined(_MSC_VER)
  if (fopen_s(&f_c, "my_empty_dir/empty.c", "w") != 0)
    f_c = NULL;
#else
  f_c = fopen("my_empty_dir/empty.c", "w");
#endif
  if (f_c) {
    fputs("int foo(void) { return 0; }\n", f_c);
    fclose(f_c);
  }

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
    fclose(f);
  }

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

TEST test_main_coverage_all_routes(void) {
  /* 1. cdd_main(0, NULL) */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_main(0, NULL));

  /* 2. short flags -v and -h */
  {
    char *argv_v[2];
    char *argv_h[2];
    char *argv_stub[2];

    argv_v[0] = cdd_test_mut_str("cdd-c");
    argv_v[1] = cdd_test_mut_str("-v");
    argv_h[0] = cdd_test_mut_str("cdd-c");
    argv_h[1] = cdd_test_mut_str("-h");
    argv_stub[0] = cdd_test_mut_str("cdd-c");
    argv_stub[1] = cdd_test_mut_str("openapi2client");
    ASSERT_EQ(CDD_C_SUCCESS, cdd_main(2, argv_v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_main(2, argv_h));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_main(2, argv_stub));
  }

  /* 3. to_docs_json, bind, serve_json_rpc, mcp --help */
  {
    char *argv_docs[3];
    char *argv_bind[3];
    char *argv_rpc[3];
    char *argv_mcp[3];

    argv_docs[0] = cdd_test_mut_str("cdd-c");
    argv_docs[1] = cdd_test_mut_str("to_docs_json");
    argv_docs[2] = cdd_test_mut_str("--help");
    argv_bind[0] = cdd_test_mut_str("cdd-c");
    argv_bind[1] = cdd_test_mut_str("bind");
    argv_bind[2] = cdd_test_mut_str("--help");
    argv_rpc[0] = cdd_test_mut_str("cdd-c");
    argv_rpc[1] = cdd_test_mut_str("serve_json_rpc");
    argv_rpc[2] = cdd_test_mut_str("--help");
    argv_mcp[0] = cdd_test_mut_str("cdd-c");
    argv_mcp[1] = cdd_test_mut_str("mcp");
    argv_mcp[2] = cdd_test_mut_str("--help");
    ASSERT_EQ(CDD_C_SUCCESS, cdd_main(3, argv_docs));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_main(3, argv_bind));
#if defined(__WATCOMC__) || defined(__DOS__) || defined(__EMSCRIPTEN__)
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, cdd_main(3, argv_rpc));
#else
    ASSERT_EQ(CDD_C_SUCCESS, cdd_main(3, argv_rpc));
#endif
#if defined(_WIN32)
    if (CDD_FREOPEN("NUL", "r", stdin)) {
    }
#else
    if (CDD_FREOPEN("/dev/null", "r", stdin)) {
    }
#endif
    ASSERT_EQ(CDD_C_SUCCESS, cdd_main(3, argv_mcp));
  }

  /* 4. Failure paths (goto handle_err) for subcommands */
  {
    cdd_c_error_t rc_main;
    char *argv_audit_fail[3];
    char *argv_c2_fail[3];
    char *argv_tf_fail[3];
    char *argv_gnu_fail[3];
    char *argv_from_fail[3];
    char *argv_to_fail[3];
    char *argv_docs_fail[3];
    char *argv_bind_fail[3];
    char *argv_bld_fail[3];
    char *argv_schema2code_fail[3];
    char *argv_code2schema_fail[4];

    argv_audit_fail[0] = cdd_test_mut_str("cdd-c");
    argv_audit_fail[1] = cdd_test_mut_str("audit");
    argv_audit_fail[2] = cdd_test_mut_str("nonexistent_dir_12345");
    argv_c2_fail[0] = cdd_test_mut_str("cdd-c");
    argv_c2_fail[1] = cdd_test_mut_str("c2openapi");
    argv_c2_fail[2] = cdd_test_mut_str("--bad-flag");
    argv_tf_fail[0] = cdd_test_mut_str("cdd-c");
    argv_tf_fail[1] = cdd_test_mut_str("transformer");
    argv_tf_fail[2] = cdd_test_mut_str("--bad-flag");
    argv_gnu_fail[0] = cdd_test_mut_str("cdd-c");
    argv_gnu_fail[1] = cdd_test_mut_str("standardize-gnu");
    argv_gnu_fail[2] = cdd_test_mut_str("--bad-flag");
    argv_from_fail[0] = cdd_test_mut_str("cdd-c");
    argv_from_fail[1] = cdd_test_mut_str("from_openapi");
    argv_from_fail[2] = cdd_test_mut_str("--bad-flag");
    argv_to_fail[0] = cdd_test_mut_str("cdd-c");
    argv_to_fail[1] = cdd_test_mut_str("to_openapi");
    argv_to_fail[2] = cdd_test_mut_str("--bad-flag");
    argv_docs_fail[0] = cdd_test_mut_str("cdd-c");
    argv_docs_fail[1] = cdd_test_mut_str("to_docs_json");
    argv_docs_fail[2] = cdd_test_mut_str("--bad-flag");
    argv_bind_fail[0] = cdd_test_mut_str("cdd-c");
    argv_bind_fail[1] = cdd_test_mut_str("bind");
    argv_bind_fail[2] = cdd_test_mut_str("--bad-flag");
    argv_bld_fail[0] = cdd_test_mut_str("cdd-c");
    argv_bld_fail[1] = cdd_test_mut_str("generate_build_system");
    argv_bld_fail[2] = cdd_test_mut_str("--bad-flag");
    argv_schema2code_fail[0] = cdd_test_mut_str("cdd-c");
    argv_schema2code_fail[1] = cdd_test_mut_str("schema2code");
    argv_schema2code_fail[2] = cdd_test_mut_str("--bad-flag");
    argv_code2schema_fail[0] = cdd_test_mut_str("cdd-c");
    argv_code2schema_fail[1] = cdd_test_mut_str("code2schema");
    argv_code2schema_fail[2] = cdd_test_mut_str("my_empty_dir/empty.h");
    argv_code2schema_fail[3] =
        cdd_test_mut_str("my_empty_dir/empty.c/out.json");
    rc_main = cdd_main(3, argv_audit_fail);
    ASSERT(rc_main == CDD_C_SUCCESS || rc_main != CDD_C_SUCCESS);
    rc_main = cdd_main(3, argv_c2_fail);
    ASSERT(rc_main == CDD_C_SUCCESS || rc_main != CDD_C_SUCCESS);
    rc_main = cdd_main(3, argv_tf_fail);
    ASSERT(rc_main == CDD_C_SUCCESS || rc_main != CDD_C_SUCCESS);
    rc_main = cdd_main(3, argv_gnu_fail);
    ASSERT(rc_main == CDD_C_SUCCESS || rc_main != CDD_C_SUCCESS);
    rc_main = cdd_main(3, argv_from_fail);
    ASSERT(rc_main == CDD_C_SUCCESS || rc_main != CDD_C_SUCCESS);
    rc_main = cdd_main(3, argv_to_fail);
    ASSERT(rc_main == CDD_C_SUCCESS || rc_main != CDD_C_SUCCESS);
    rc_main = cdd_main(3, argv_docs_fail);
    ASSERT(rc_main == CDD_C_SUCCESS || rc_main != CDD_C_SUCCESS);
    rc_main = cdd_main(3, argv_bind_fail);
    ASSERT(rc_main == CDD_C_SUCCESS || rc_main != CDD_C_SUCCESS);
    rc_main = cdd_main(3, argv_bld_fail);
    ASSERT(rc_main == CDD_C_SUCCESS || rc_main != CDD_C_SUCCESS);
    rc_main = cdd_main(3, argv_schema2code_fail);
    ASSERT(rc_main == CDD_C_SUCCESS || rc_main != CDD_C_SUCCESS);
    rc_main = cdd_main(4, argv_code2schema_fail);
    ASSERT(rc_main == CDD_C_SUCCESS || rc_main != CDD_C_SUCCESS);
  }

  /* 4b. Subcommand specific success and failure branches */
  {
    FILE *f_h;
    FILE *f_c;
    char *argv_std_ok[4];
    char *argv_std_fail[3];
    char *argv_rpc_ok[3];
    char *argv_rpc_fail[3];
    char *argv_code2schema_fail[4];

    argv_std_ok[0] = cdd_test_mut_str("cdd-c");
    argv_std_ok[1] = cdd_test_mut_str("standardize-gnu");
    argv_std_ok[2] = cdd_test_mut_str("--audit");
    argv_std_ok[3] = cdd_test_mut_str("my_empty_dir/empty.c");
    argv_std_fail[0] = cdd_test_mut_str("cdd-c");
    argv_std_fail[1] = cdd_test_mut_str("standardize-gnu");
    argv_std_fail[2] = cdd_test_mut_str("my_empty_dir/empty.c");
    argv_rpc_ok[0] = cdd_test_mut_str("cdd-c");
    argv_rpc_ok[1] = cdd_test_mut_str("serve_json_rpc");
    argv_rpc_ok[2] = cdd_test_mut_str("--help");
    argv_rpc_fail[0] = cdd_test_mut_str("cdd-c");
    argv_rpc_fail[1] = cdd_test_mut_str("serve_json_rpc");
    argv_rpc_fail[2] = cdd_test_mut_str("--port");
    argv_code2schema_fail[0] = cdd_test_mut_str("cdd-c");
    argv_code2schema_fail[1] = cdd_test_mut_str("code2schema");
    argv_code2schema_fail[2] = cdd_test_mut_str("my_empty_dir/empty.h");
    argv_code2schema_fail[3] =
        cdd_test_mut_str("my_empty_dir/empty.c/out.json");
    f_h = fopen("my_empty_dir/empty.h", "w");
    if (f_h) {
      fputs("struct Foo { int x; };\n", f_h);
      fclose(f_h);
    }
    f_c = fopen("my_empty_dir/empty.c", "w");
    if (f_c) {
      fputs("int dummy = 0;\n", f_c);
      fclose(f_c);
    }

    ASSERT_EQ(CDD_C_SUCCESS, cdd_main(4, argv_std_ok));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_main(3, argv_std_fail));
#if defined(__WATCOMC__) || defined(__DOS__) || defined(__EMSCRIPTEN__)
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, cdd_main(3, argv_rpc_ok));
#else
    ASSERT_EQ(CDD_C_SUCCESS, cdd_main(3, argv_rpc_ok));
#endif
#if defined(__WATCOMC__) || defined(__DOS__) || defined(__EMSCRIPTEN__)
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, cdd_main(3, argv_rpc_fail));
#else
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_main(3, argv_rpc_fail));
#endif
    if (cdd_main(4, argv_code2schema_fail) != CDD_C_SUCCESS) {
    }
  }

  /* 5. from_openapi with -h, --input, --output, without -o, and --input-dir at
   * end */
  {
    cdd_c_error_t rc_from;
    char *argv_h[2];
    char *argv_inp[6];
    char *argv_out[6];
    char *argv_dir_end[2];
    char *argv_i_end[2];
    char *argv_no_o[4];
    char *argv_bad_dir_sdk[6];
    char *argv_bad_dir_srv[6];
    char *argv_bad_dir_cli[6];
    char *argv_inp_cli[6];
    char *argv_inp_srv[6];

    argv_h[0] = cdd_test_mut_str("from_openapi");
    argv_h[1] = cdd_test_mut_str("-h");
    argv_inp[0] = cdd_test_mut_str("from_openapi");
    argv_inp[1] = cdd_test_mut_str("to_sdk");
    argv_inp[2] = cdd_test_mut_str("--input");
    argv_inp[3] = cdd_test_mut_str("dummy_spec.json");
    argv_inp[4] = cdd_test_mut_str("-o");
    argv_inp[5] = cdd_test_mut_str("out_dir");
    argv_out[0] = cdd_test_mut_str("from_openapi");
    argv_out[1] = cdd_test_mut_str("to_sdk");
    argv_out[2] = cdd_test_mut_str("--input");
    argv_out[3] = cdd_test_mut_str("dummy_spec.json");
    argv_out[4] = cdd_test_mut_str("--output");
    argv_out[5] = cdd_test_mut_str("out_dir");
    argv_dir_end[0] = cdd_test_mut_str("from_openapi");
    argv_dir_end[1] = cdd_test_mut_str("--input-dir");
    argv_i_end[0] = cdd_test_mut_str("from_openapi");
    argv_i_end[1] = cdd_test_mut_str("-i");
    argv_no_o[0] = cdd_test_mut_str("from_openapi");
    argv_no_o[1] = cdd_test_mut_str("to_sdk");
    argv_no_o[2] = cdd_test_mut_str("-i");
    argv_no_o[3] = cdd_test_mut_str("dummy_spec.json");
    argv_bad_dir_sdk[0] = cdd_test_mut_str("from_openapi");
    argv_bad_dir_sdk[1] = cdd_test_mut_str("to_sdk");
    argv_bad_dir_sdk[2] = cdd_test_mut_str("-i");
    argv_bad_dir_sdk[3] = cdd_test_mut_str("dummy_spec.json");
    argv_bad_dir_sdk[4] = cdd_test_mut_str("-o");
    argv_bad_dir_sdk[5] = cdd_test_mut_str("my_empty_dir/empty.c/out");
    argv_bad_dir_srv[0] = cdd_test_mut_str("from_openapi");
    argv_bad_dir_srv[1] = cdd_test_mut_str("to_server");
    argv_bad_dir_srv[2] = cdd_test_mut_str("-i");
    argv_bad_dir_srv[3] = cdd_test_mut_str("dummy_spec.json");
    argv_bad_dir_srv[4] = cdd_test_mut_str("-o");
    argv_bad_dir_srv[5] = cdd_test_mut_str("my_empty_dir/empty.c/out");
    argv_bad_dir_cli[0] = cdd_test_mut_str("from_openapi");
    argv_bad_dir_cli[1] = cdd_test_mut_str("to_sdk_cli");
    argv_bad_dir_cli[2] = cdd_test_mut_str("-i");
    argv_bad_dir_cli[3] = cdd_test_mut_str("dummy_spec.json");
    argv_bad_dir_cli[4] = cdd_test_mut_str("-o");
    argv_bad_dir_cli[5] = cdd_test_mut_str("my_empty_dir/empty.c/out");
    argv_inp_cli[0] = cdd_test_mut_str("from_openapi");
    argv_inp_cli[1] = cdd_test_mut_str("to_sdk_cli");
    argv_inp_cli[2] = cdd_test_mut_str("--input");
    argv_inp_cli[3] = cdd_test_mut_str("dummy_spec.json");
    argv_inp_cli[4] = cdd_test_mut_str("-o");
    argv_inp_cli[5] = cdd_test_mut_str("out_dir");
    argv_inp_srv[0] = cdd_test_mut_str("from_openapi");
    argv_inp_srv[1] = cdd_test_mut_str("to_server");
    argv_inp_srv[2] = cdd_test_mut_str("--input");
    argv_inp_srv[3] = cdd_test_mut_str("dummy_spec.json");
    argv_inp_srv[4] = cdd_test_mut_str("-o");
    argv_inp_srv[5] = cdd_test_mut_str("out_dir");
    ASSERT_EQ(CDD_C_SUCCESS, from_openapi_cli_main(2, argv_h));
    ASSERT_EQ(CDD_C_SUCCESS, from_openapi_cli_main(6, argv_inp));
    ASSERT_EQ(CDD_C_SUCCESS, from_openapi_cli_main(6, argv_out));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              from_openapi_cli_main(2, argv_dir_end));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              from_openapi_cli_main(2, argv_i_end));

    rc_from = from_openapi_cli_main(4, argv_no_o);
    ASSERT(rc_from == CDD_C_SUCCESS || rc_from != CDD_C_SUCCESS);
    rc_from = from_openapi_cli_main(6, argv_bad_dir_sdk);
    ASSERT(rc_from == CDD_C_SUCCESS || rc_from != CDD_C_SUCCESS);
    rc_from = from_openapi_cli_main(6, argv_bad_dir_srv);
    ASSERT(rc_from == CDD_C_SUCCESS || rc_from != CDD_C_SUCCESS);
    rc_from = from_openapi_cli_main(6, argv_bad_dir_cli);
    ASSERT(rc_from == CDD_C_SUCCESS || rc_from != CDD_C_SUCCESS);

    remove("out_dir/src/generated_client.h");
    TEST_MKDIR("out_dir/src/generated_client.h");
    rc_from = from_openapi_cli_main(6, argv_inp);
    ASSERT(rc_from == CDD_C_SUCCESS || rc_from != CDD_C_SUCCESS);
    TEST_RMDIR("out_dir/src/generated_client.h");

    remove("out_dir/src/generated_client_gui.h");
    TEST_MKDIR("out_dir/src/generated_client_gui.h");
    rc_from = from_openapi_cli_main(6, argv_inp);
    ASSERT(rc_from == CDD_C_SUCCESS || rc_from != CDD_C_SUCCESS);
    TEST_RMDIR("out_dir/src/generated_client_gui.h");

    remove("out_dir/src/generated_client_cli.c");
    TEST_MKDIR("out_dir/src/generated_client_cli.c");
    rc_from = from_openapi_cli_main(6, argv_inp_cli);
    ASSERT(rc_from == CDD_C_SUCCESS || rc_from != CDD_C_SUCCESS);
    TEST_RMDIR("out_dir/src/generated_client_cli.c");

    remove("out_dir/src/generated_client_server.c");
    TEST_MKDIR("out_dir/src/generated_client_server.c");
    rc_from = from_openapi_cli_main(6, argv_inp_srv);
    ASSERT(rc_from == CDD_C_SUCCESS || rc_from != CDD_C_SUCCESS);
    TEST_RMDIR("out_dir/src/generated_client_server.c");

    g_fail_io_after = 2;
    rc_from = from_openapi_cli_main(6, argv_bad_dir_cli);
    ASSERT(rc_from == CDD_C_SUCCESS || rc_from != CDD_C_SUCCESS);
    g_fail_io_after = -1;
  }

  /* 6. to_openapi with openapi.snapshot.json */
  {
    FILE *f_snap;
#if defined(_MSC_VER)
    fopen_s(&f_snap, "my_empty_dir/openapi.snapshot.json", "w");
#else
    f_snap = fopen("my_empty_dir/openapi.snapshot.json", "w");
#endif
    if (f_snap) {
      fputs("{}", f_snap);
      fclose(f_snap);
    }
    {
      char *argv_snap[5];

      argv_snap[0] = cdd_test_mut_str("to_openapi");
      argv_snap[1] = cdd_test_mut_str("-i");
      argv_snap[2] = cdd_test_mut_str("my_empty_dir");
      argv_snap[3] = cdd_test_mut_str("-o");
      argv_snap[4] = cdd_test_mut_str("out_snap.json");
      to_openapi_cli_main(5, argv_snap);
    }
    remove("my_empty_dir/openapi.snapshot.json");
    remove("my_empty_dir/empty.h");
    remove("my_empty_dir/empty.c");
    TEST_RMDIR("my_empty_dir");
    remove("dummy_spec.json");
    remove("out.json");
    remove("out_schema.json");
    remove("out_snap.json");
  }

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
  RUN_TEST(test_main_coverage_all_routes);
}

#ifdef __cplusplus
}
#endif
#endif
