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
#include <unistd.h>
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
  PASS();
}

TEST test_main_coverage_handle_audit(void) {
  char *argv[] = {(char *)(size_t)(size_t) "dir1",
                  (char *)(size_t)(size_t) "dir2"};
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, handle_audit(2, argv));
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, handle_audit(0, argv));
  PASS();
}

TEST test_main_coverage_handle_audit_valid(void) {
  char *argv[] = {(char *)(size_t)(size_t) "my_empty_dir"};
  handle_audit(1, argv);
  PASS();
}

TEST test_main_coverage_from_openapi(void) {
  char *argv_no_args[] = {(char *)(size_t)(size_t) "from_openapi"};
  char *argv_to_sdk[] = {(char *)(size_t)(size_t) "from_openapi",
                         (char *)(size_t)(size_t) "to_sdk",
                         (char *)(size_t)(size_t) "-i",
                         (char *)(size_t)(size_t) "missing.json",
                         (char *)(size_t)(size_t) "-o",
                         (char *)(size_t)(size_t) "out"};
  char *argv_to_sdk_cli[] = {(char *)(size_t)(size_t) "from_openapi",
                             (char *)(size_t)(size_t) "to_sdk_cli",
                             (char *)(size_t)(size_t) "--input-dir",
                             (char *)(size_t)(size_t) "missing_dir",
                             (char *)(size_t)(size_t) "-o",
                             (char *)(size_t)(size_t) "out"};
  char *argv_to_server[] = {(char *)(size_t)(size_t) "from_openapi",
                            (char *)(size_t)(size_t) "to_server",
                            (char *)(size_t)(size_t) "-i",
                            (char *)(size_t)(size_t) "missing.json",
                            (char *)(size_t)(size_t) "-o",
                            (char *)(size_t)(size_t) "out"};

  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, from_openapi_cli_main(1, argv_no_args));
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, from_openapi_cli_main(6, argv_to_sdk));
  ASSERT_EQ(CDD_C_SUCCESS, from_openapi_cli_main(6, argv_to_sdk_cli));
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, from_openapi_cli_main(6, argv_to_server));
  PASS();
}

TEST test_main_coverage_from_openapi_opts(void) {
  char *argv_help[] = {(char *)(size_t)(size_t) "from_openapi",
                       (char *)(size_t)(size_t) "--help"};
  char *argv_opts[] = {(char *)(size_t)(size_t) "from_openapi",
                       (char *)(size_t)(size_t) "to_sdk",
                       (char *)(size_t)(size_t) "-i",
                       (char *)(size_t)(size_t) "missing.json",
                       (char *)(size_t)(size_t) "--no-github-actions",
                       (char *)(size_t)(size_t) "--no-installable-package",
                       (char *)(size_t)(size_t) "--tests"};
  char *argv_out[] = {(char *)(size_t)(size_t) "from_openapi",
                      (char *)(size_t)(size_t) "to_sdk",
                      (char *)(size_t)(size_t) "-i",
                      (char *)(size_t)(size_t) "missing.json",
                      (char *)(size_t)(size_t) "-o",
                      (char *)(size_t)(size_t) "out_dir"};

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
    char *argv_to_sdk[] = {(char *)(size_t)(size_t) "from_openapi",
                           (char *)(size_t)(size_t) "to_sdk",
                           (char *)(size_t)(size_t) "-i",
                           (char *)(size_t)(size_t) "dummy_spec.json",
                           (char *)(size_t)(size_t) "-o",
                           (char *)(size_t)(size_t) "out_dir"};
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
    char *argv_invalid[] = {(char *)(size_t)(size_t) "from_openapi",
                            (char *)(size_t)(size_t) "to_sdk",
                            (char *)(size_t)(size_t) "-i",
                            (char *)(size_t)(size_t) "invalid_spec.json",
                            (char *)(size_t)(size_t) "-o",
                            (char *)(size_t)(size_t) "out_dir"};
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, from_openapi_cli_main(6, argv_invalid));
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
    char *argv_invalid2[] = {(char *)(size_t)(size_t) "from_openapi",
                             (char *)(size_t)(size_t) "to_sdk",
                             (char *)(size_t)(size_t) "-i",
                             (char *)(size_t)(size_t) "invalid_spec2.json",
                             (char *)(size_t)(size_t) "-o",
                             (char *)(size_t)(size_t) "out_dir"};
    char *argv_inp_last[] = {(char *)(size_t)(size_t) "from_openapi",
                             (char *)(size_t)(size_t) "--input"};
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              from_openapi_cli_main(6, argv_invalid2));
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, from_openapi_cli_main(2, argv_inp_last));
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
    char *argv_cli[] = {(char *)(size_t)(size_t) "from_openapi",
                        (char *)(size_t)(size_t) "to_sdk_cli",
                        (char *)(size_t)(size_t) "-i",
                        (char *)(size_t)(size_t) "dummy_spec.json",
                        (char *)(size_t)(size_t) "-o",
                        (char *)(size_t)(size_t) "out_dir"};
    char *argv_server[] = {(char *)(size_t)(size_t) "from_openapi",
                           (char *)(size_t)(size_t) "to_server",
                           (char *)(size_t)(size_t) "-i",
                           (char *)(size_t)(size_t) "dummy_spec.json",
                           (char *)(size_t)(size_t) "-o",
                           (char *)(size_t)(size_t) "out_dir"};

    ASSERT_EQ(CDD_C_SUCCESS, from_openapi_cli_main(6, argv_cli));
    ASSERT_EQ(CDD_C_SUCCESS, from_openapi_cli_main(6, argv_server));
    PASS();
  }
}

TEST test_main_coverage_cdd_main(void) {
  char *argv_ver[] = {(char *)(size_t)(size_t) "cdd-c",
                      (char *)(size_t)(size_t) "--version"};
  char *argv_help[] = {(char *)(size_t)(size_t) "cdd-c",
                       (char *)(size_t)(size_t) "--help"};
  char *argv_err[] = {(char *)(size_t)(size_t) "cdd-c",
                      (char *)(size_t)(size_t) "unknown"};

  ASSERT_EQ(CDD_C_SUCCESS, cdd_main(2, argv_ver));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_main(2, argv_help));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_main(2, argv_err));
  PASS();
}

TEST test_main_coverage_to_openapi(void) {
  char *argv_help[] = {(char *)(size_t)(size_t) "to_openapi",
                       (char *)(size_t)(size_t) "--help"};
  char *argv_no_args[] = {(char *)(size_t)(size_t) "to_openapi"};
  char *argv_args[] = {
      (char *)(size_t)(size_t) "to_openapi", (char *)(size_t)(size_t) "-i",
      (char *)(size_t)(size_t) "my_empty_dir", (char *)(size_t)(size_t) "-o",
      (char *)(size_t)(size_t) "out.json"};

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
    char *argv_audit[] = {
        (char *)(size_t)(size_t) "cdd-c", (char *)(size_t)(size_t) "audit",
        (char *)(size_t)(size_t) "a", (char *)(size_t)(size_t) "b"};
    char *argv_c2openapi[] = {
        (char *)(size_t)(size_t) "cdd-c", (char *)(size_t)(size_t) "c2openapi",
        (char *)(size_t)(size_t) "a", (char *)(size_t)(size_t) "b"};
    char *argv_transformer[] = {(char *)(size_t)(size_t) "cdd-c",
                                (char *)(size_t)(size_t) "transformer",
                                (char *)(size_t)(size_t) "a"};
    char *argv_standardize[] = {(char *)(size_t)(size_t) "cdd-c",
                                (char *)(size_t)(size_t) "standardize-gnu"};
    char *argv_code2schema[] = {(char *)(size_t)(size_t) "cdd-c",
                                (char *)(size_t)(size_t) "code2schema",
                                (char *)(size_t)(size_t) "a",
                                (char *)(size_t)(size_t) "b"};
    char *argv_gen_build[] = {(char *)(size_t)(size_t) "cdd-c",
                              (char *)(size_t)(size_t) "generate_build_system",
                              (char *)(size_t)(size_t) "a"};
    char *argv_schema2code[] = {(char *)(size_t)(size_t) "cdd-c",
                                (char *)(size_t)(size_t) "schema2code",
                                (char *)(size_t)(size_t) "a"};
    char *argv_to_docs[] = {(char *)(size_t)(size_t) "cdd-c",
                            (char *)(size_t)(size_t) "to_docs_json",
                            (char *)(size_t)(size_t) "a"};
    char *argv_bind[] = {(char *)(size_t)(size_t) "cdd-c",
                         (char *)(size_t)(size_t) "bind",
                         (char *)(size_t)(size_t) "a"};
    char *argv_from_openapi[] = {(char *)(size_t)(size_t) "cdd-c",
                                 (char *)(size_t)(size_t) "from_openapi",
                                 (char *)(size_t)(size_t) "a"};
    char *argv_to_openapi[] = {(char *)(size_t)(size_t) "cdd-c",
                               (char *)(size_t)(size_t) "to_openapi",
                               (char *)(size_t)(size_t) "a"};

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
  char *argv_to_openapi[] = {
      (char *)(size_t)(size_t) "cdd-c", (char *)(size_t)(size_t) "to_openapi",
      (char *)(size_t)(size_t) "-i",    (char *)(size_t)(size_t) "my_empty_dir",
      (char *)(size_t)(size_t) "-o",    (char *)(size_t)(size_t) "out.json"};
  char *argv_from_openapi[] = {(char *)(size_t)(size_t) "cdd-c",
                               (char *)(size_t)(size_t) "from_openapi",
                               (char *)(size_t)(size_t) "to_sdk",
                               (char *)(size_t)(size_t) "-i",
                               (char *)(size_t)(size_t) "dummy_spec.json",
                               (char *)(size_t)(size_t) "-o",
                               (char *)(size_t)(size_t) "out_dir"};
  char *argv_c2openapi[] = {(char *)(size_t)(size_t) "cdd-c",
                            (char *)(size_t)(size_t) "c2openapi",
                            (char *)(size_t)(size_t) "my_empty_dir",
                            (char *)(size_t)(size_t) "out.json"};
  char *argv_code2schema[] = {(char *)(size_t)(size_t) "cdd-c",
                              (char *)(size_t)(size_t) "code2schema",
                              (char *)(size_t)(size_t) "my_empty_dir/empty.h",
                              (char *)(size_t)(size_t) "out_schema.json"};
  char *argv_transformer[] = {(char *)(size_t)(size_t) "cdd-c",
                              (char *)(size_t)(size_t) "transformer",
                              (char *)(size_t)(size_t) "safe_crt",
                              (char *)(size_t)(size_t) "my_empty_dir/empty.c"};
  char *argv_standardize[] = {(char *)(size_t)(size_t) "cdd-c",
                              (char *)(size_t)(size_t) "standardize-gnu",
                              (char *)(size_t)(size_t) "my_empty_dir/empty.c"};
  char *argv_audit[] = {(char *)(size_t)(size_t) "cdd-c",
                        (char *)(size_t)(size_t) "audit",
                        (char *)(size_t)(size_t) "my_empty_dir"};
  char *argv_gen_build[] = {(char *)(size_t)(size_t) "cdd-c",
                            (char *)(size_t)(size_t) "generate_build_system",
                            (char *)(size_t)(size_t) "cmake",
                            (char *)(size_t)(size_t) "my_empty_dir",
                            (char *)(size_t)(size_t) "test"};
  char *argv_schema2code[] = {
      (char *)(size_t)(size_t) "cdd-c", (char *)(size_t)(size_t) "schema2code",
      (char *)(size_t)(size_t) "src/tests/mocks/emit/simple.schema.json",
      (char *)(size_t)(size_t) "out_dir/simple"};

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
    char *argv_v[] = {(char *)(size_t)(size_t) "cdd-c",
                      (char *)(size_t)(size_t) "-v"};
    char *argv_h[] = {(char *)(size_t)(size_t) "cdd-c",
                      (char *)(size_t)(size_t) "-h"};
    char *argv_stub[] = {(char *)(size_t)(size_t) "cdd-c",
                         (char *)(size_t)(size_t) "openapi2client"};
    ASSERT_EQ(CDD_C_SUCCESS, cdd_main(2, argv_v));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_main(2, argv_h));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_main(2, argv_stub));
  }

  /* 3. to_docs_json, bind, serve_json_rpc, mcp --help */
  {
    char *argv_docs[] = {(char *)(size_t)(size_t) "cdd-c",
                         (char *)(size_t)(size_t) "to_docs_json",
                         (char *)(size_t)(size_t) "--help"};
    char *argv_bind[] = {(char *)(size_t)(size_t) "cdd-c",
                         (char *)(size_t)(size_t) "bind",
                         (char *)(size_t)(size_t) "--help"};
    char *argv_rpc[] = {(char *)(size_t)(size_t) "cdd-c",
                        (char *)(size_t)(size_t) "serve_json_rpc",
                        (char *)(size_t)(size_t) "--help"};
    char *argv_mcp[] = {(char *)(size_t)(size_t) "cdd-c",
                        (char *)(size_t)(size_t) "mcp",
                        (char *)(size_t)(size_t) "--help"};

    ASSERT_EQ(CDD_C_SUCCESS, cdd_main(3, argv_docs));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_main(3, argv_bind));
#if defined(__WATCOMC__) || defined(__DOS__) || defined(__EMSCRIPTEN__)
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, cdd_main(3, argv_rpc));
#else
    ASSERT_EQ(CDD_C_SUCCESS, cdd_main(3, argv_rpc));
#endif
#if defined(_WIN32)
    (void)CDD_FREOPEN("NUL", "r", stdin);
#else
    if (CDD_FREOPEN("/dev/null", "r", stdin)) {
    }
#endif
    ASSERT_EQ(CDD_C_SUCCESS, cdd_main(3, argv_mcp));
  }

  /* 4. Failure paths (goto handle_err) for subcommands */
  {
    char *argv_audit_fail[] = {
        (char *)(size_t)(size_t) "cdd-c", (char *)(size_t)(size_t) "audit",
        (char *)(size_t)(size_t) "nonexistent_dir_12345"};
    char *argv_c2_fail[] = {(char *)(size_t)(size_t) "cdd-c",
                            (char *)(size_t)(size_t) "c2openapi",
                            (char *)(size_t)(size_t) "--bad-flag"};
    char *argv_tf_fail[] = {(char *)(size_t)(size_t) "cdd-c",
                            (char *)(size_t)(size_t) "transformer",
                            (char *)(size_t)(size_t) "--bad-flag"};
    char *argv_gnu_fail[] = {(char *)(size_t)(size_t) "cdd-c",
                             (char *)(size_t)(size_t) "standardize-gnu",
                             (char *)(size_t)(size_t) "--bad-flag"};
    char *argv_from_fail[] = {(char *)(size_t)(size_t) "cdd-c",
                              (char *)(size_t)(size_t) "from_openapi",
                              (char *)(size_t)(size_t) "--bad-flag"};
    char *argv_to_fail[] = {(char *)(size_t)(size_t) "cdd-c",
                            (char *)(size_t)(size_t) "to_openapi",
                            (char *)(size_t)(size_t) "--bad-flag"};
    char *argv_docs_fail[] = {(char *)(size_t)(size_t) "cdd-c",
                              (char *)(size_t)(size_t) "to_docs_json",
                              (char *)(size_t)(size_t) "--bad-flag"};
    char *argv_bind_fail[] = {(char *)(size_t)(size_t) "cdd-c",
                              (char *)(size_t)(size_t) "bind",
                              (char *)(size_t)(size_t) "--bad-flag"};
    char *argv_bld_fail[] = {(char *)(size_t)(size_t) "cdd-c",
                             (char *)(size_t)(size_t) "generate_build_system",
                             (char *)(size_t)(size_t) "--bad-flag"};
    char *argv_schema2code_fail[] = {(char *)(size_t)(size_t) "cdd-c",
                                     (char *)(size_t)(size_t) "schema2code",
                                     (char *)(size_t)(size_t) "--bad-flag"};
    char *argv_code2schema_fail[] = {
        (char *)(size_t)(size_t) "cdd-c",
        (char *)(size_t)(size_t) "code2schema",
        (char *)(size_t)(size_t) "my_empty_dir/empty.h",
        (char *)(size_t)(size_t) "my_empty_dir/empty.c/out.json"};

    (void)cdd_main(3, argv_audit_fail);
    (void)cdd_main(3, argv_c2_fail);
    (void)cdd_main(3, argv_tf_fail);
    (void)cdd_main(3, argv_gnu_fail);
    (void)cdd_main(3, argv_from_fail);
    (void)cdd_main(3, argv_to_fail);
    (void)cdd_main(3, argv_docs_fail);
    (void)cdd_main(3, argv_bind_fail);
    (void)cdd_main(3, argv_bld_fail);
    (void)cdd_main(3, argv_schema2code_fail);
    (void)cdd_main(4, argv_code2schema_fail);
  }

  /* 4b. Subcommand specific success and failure branches */
  {
    FILE *f_h;
    FILE *f_c;
    char *argv_std_ok[] = {(char *)(size_t)(size_t) "cdd-c",
                           (char *)(size_t)(size_t) "standardize-gnu",
                           (char *)(size_t)(size_t) "--audit",
                           (char *)(size_t)(size_t) "my_empty_dir/empty.c"};
    char *argv_std_fail[] = {(char *)(size_t)(size_t) "cdd-c",
                             (char *)(size_t)(size_t) "standardize-gnu",
                             (char *)(size_t)(size_t) "my_empty_dir/empty.c"};
    char *argv_rpc_ok[] = {(char *)(size_t)(size_t) "cdd-c",
                           (char *)(size_t)(size_t) "serve_json_rpc",
                           (char *)(size_t)(size_t) "--help"};
    char *argv_rpc_fail[] = {(char *)(size_t)(size_t) "cdd-c",
                             (char *)(size_t)(size_t) "serve_json_rpc",
                             (char *)(size_t)(size_t) "--port"};
    char *argv_code2schema_fail[] = {
        (char *)(size_t)(size_t) "cdd-c",
        (char *)(size_t)(size_t) "code2schema",
        (char *)(size_t)(size_t) "my_empty_dir/empty.h",
        (char *)(size_t)(size_t) "my_empty_dir/empty.c/out.json"};

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
    (void)cdd_main(4, argv_code2schema_fail);
  }

  /* 5. from_openapi with -h, --input, --output, without -o, and --input-dir at
   * end */
  {
    char *argv_h[] = {(char *)(size_t)(size_t) "from_openapi",
                      (char *)(size_t)(size_t) "-h"};
    char *argv_inp[] = {(char *)(size_t)(size_t) "from_openapi",
                        (char *)(size_t)(size_t) "to_sdk",
                        (char *)(size_t)(size_t) "--input",
                        (char *)(size_t)(size_t) "dummy_spec.json",
                        (char *)(size_t)(size_t) "-o",
                        (char *)(size_t)(size_t) "out_dir"};
    char *argv_out[] = {(char *)(size_t)(size_t) "from_openapi",
                        (char *)(size_t)(size_t) "to_sdk",
                        (char *)(size_t)(size_t) "--input",
                        (char *)(size_t)(size_t) "dummy_spec.json",
                        (char *)(size_t)(size_t) "--output",
                        (char *)(size_t)(size_t) "out_dir"};
    char *argv_dir_end[] = {(char *)(size_t)(size_t) "from_openapi",
                            (char *)(size_t)(size_t) "--input-dir"};
    char *argv_i_end[] = {(char *)(size_t)(size_t) "from_openapi",
                          (char *)(size_t)(size_t) "-i"};
    char *argv_no_o[] = {(char *)(size_t)(size_t) "from_openapi",
                         (char *)(size_t)(size_t) "to_sdk",
                         (char *)(size_t)(size_t) "-i",
                         (char *)(size_t)(size_t) "dummy_spec.json"};
    char *argv_bad_dir_sdk[] = {
        (char *)(size_t)(size_t) "from_openapi",
        (char *)(size_t)(size_t) "to_sdk",
        (char *)(size_t)(size_t) "-i",
        (char *)(size_t)(size_t) "dummy_spec.json",
        (char *)(size_t)(size_t) "-o",
        (char *)(size_t)(size_t) "my_empty_dir/empty.c/out"};
    char *argv_bad_dir_srv[] = {
        (char *)(size_t)(size_t) "from_openapi",
        (char *)(size_t)(size_t) "to_server",
        (char *)(size_t)(size_t) "-i",
        (char *)(size_t)(size_t) "dummy_spec.json",
        (char *)(size_t)(size_t) "-o",
        (char *)(size_t)(size_t) "my_empty_dir/empty.c/out"};
    char *argv_bad_dir_cli[] = {
        (char *)(size_t)(size_t) "from_openapi",
        (char *)(size_t)(size_t) "to_sdk_cli",
        (char *)(size_t)(size_t) "-i",
        (char *)(size_t)(size_t) "dummy_spec.json",
        (char *)(size_t)(size_t) "-o",
        (char *)(size_t)(size_t) "my_empty_dir/empty.c/out"};
    char *argv_inp_cli[] = {(char *)(size_t)(size_t) "from_openapi",
                            (char *)(size_t)(size_t) "to_sdk_cli",
                            (char *)(size_t)(size_t) "--input",
                            (char *)(size_t)(size_t) "dummy_spec.json",
                            (char *)(size_t)(size_t) "-o",
                            (char *)(size_t)(size_t) "out_dir"};
    char *argv_inp_srv[] = {(char *)(size_t)(size_t) "from_openapi",
                            (char *)(size_t)(size_t) "to_server",
                            (char *)(size_t)(size_t) "--input",
                            (char *)(size_t)(size_t) "dummy_spec.json",
                            (char *)(size_t)(size_t) "-o",
                            (char *)(size_t)(size_t) "out_dir"};

    ASSERT_EQ(CDD_C_SUCCESS, from_openapi_cli_main(2, argv_h));
    ASSERT_EQ(CDD_C_SUCCESS, from_openapi_cli_main(6, argv_inp));
    ASSERT_EQ(CDD_C_SUCCESS, from_openapi_cli_main(6, argv_out));
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, from_openapi_cli_main(2, argv_dir_end));
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, from_openapi_cli_main(2, argv_i_end));

    (void)from_openapi_cli_main(4, argv_no_o);
    (void)from_openapi_cli_main(6, argv_bad_dir_sdk);
    (void)from_openapi_cli_main(6, argv_bad_dir_srv);
    (void)from_openapi_cli_main(6, argv_bad_dir_cli);

    remove("out_dir/src/generated_client.h");
    TEST_MKDIR("out_dir/src/generated_client.h");
    (void)from_openapi_cli_main(6, argv_inp);
    TEST_RMDIR("out_dir/src/generated_client.h");

    remove("out_dir/src/generated_client_gui.h");
    TEST_MKDIR("out_dir/src/generated_client_gui.h");
    (void)from_openapi_cli_main(6, argv_inp);
    TEST_RMDIR("out_dir/src/generated_client_gui.h");

    remove("out_dir/src/generated_client_cli.c");
    TEST_MKDIR("out_dir/src/generated_client_cli.c");
    (void)from_openapi_cli_main(6, argv_inp_cli);
    TEST_RMDIR("out_dir/src/generated_client_cli.c");

    remove("out_dir/src/generated_client_server.c");
    TEST_MKDIR("out_dir/src/generated_client_server.c");
    (void)from_openapi_cli_main(6, argv_inp_srv);
    TEST_RMDIR("out_dir/src/generated_client_server.c");

    g_fail_io_after = 2;
    (void)from_openapi_cli_main(6, argv_bad_dir_cli);
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
      char *argv_snap[] = {(char *)(size_t)(size_t) "to_openapi",
                           (char *)(size_t)(size_t) "-i",
                           (char *)(size_t)(size_t) "my_empty_dir",
                           (char *)(size_t)(size_t) "-o",
                           (char *)(size_t)(size_t) "out_snap.json"};
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
