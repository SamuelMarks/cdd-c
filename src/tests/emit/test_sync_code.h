/**
 * @file test_sync_code.h
 * @brief Unit tests and mocks in test_sync_code.h.
 */
#ifndef TEST_SYNC_CODE_H
#define TEST_SYNC_CODE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include "functions/emit/sync.h"
#include "functions/parse/fs.h"
#include <cdd_test_helpers/cdd_helpers.h>
#include <greatest.h>
#include <stdio.h>
/* clang-format on */

/* Moved extern declarations for C89 compliance */
/** @brief Test definition g_cdd_sync_fail_tokenize. */
extern C_CDD_EXPORT int g_cdd_sync_fail_tokenize;
/** @brief Test definition g_cdd_sync_fail_patch_list_apply. */
extern C_CDD_EXPORT int g_cdd_sync_fail_patch_list_apply;
/** @brief Test definition g_cdd_sync_fail_func_sig_init. */
extern C_CDD_EXPORT int g_cdd_sync_fail_func_sig_init;
/** @brief Test definition g_cdd_sync_fail_patch_list_init. */
extern C_CDD_EXPORT int g_cdd_sync_fail_patch_list_init;
extern C_CDD_EXPORT int g_cdd_fprintf_fail;
/** @brief Test definition g_cdd_sync_fail_extract. */
extern C_CDD_EXPORT int g_cdd_sync_fail_extract;
/** @brief Test definition g_cdd_sync_fail_fopen_write. */
extern C_CDD_EXPORT int g_cdd_sync_fail_fopen_write;

/**
 * @brief test_sync_code_wrong_args
 * @return TEST
 */
TEST test_sync_code_wrong_args(void) {
  char *argv[2];

  argv[0] = cdd_test_mut_str("program");
  argv[1] = NULL;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, sync_code_main(1, argv));
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief test_sync_code_main_argc
 * @return TEST
 */
TEST test_sync_code_main_argc(void) {
  char *argv[1];

  argv[0] = cdd_test_mut_str("foo.h");
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, sync_code_main(1, argv));
#ifdef CDD_BUILD_TESTS
  {
    char *argv2[2];
    extern C_CDD_EXPORT int g_cdd_fail_type_def_list_init;
    argv2[0] = cdd_test_mut_str("foo.h");
    argv2[1] = cdd_test_mut_str("foo.c");
    g_cdd_fail_type_def_list_init = 2;
    ASSERT_EQ(CDD_C_ERROR_NOT_FOUND, sync_code_main(2, argv2));
    ASSERT_EQ(CDD_C_ERROR_MEMORY, sync_code_main(2, argv2));
    g_cdd_fail_type_def_list_init = 0;
  }
#endif
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief test_sync_code_file_missing
 * @return TEST
 */
TEST test_sync_code_file_missing(void) {
  char *argv[2];

  argv[0] = cdd_test_mut_str("notfound.h");
  argv[1] = cdd_test_mut_str("impl.c");
  ASSERT_EQ(CDD_C_ERROR_NOT_FOUND, sync_code_main(2, argv));
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief test_sync_code_simple_struct_enum
 * @return TEST
 */
TEST test_sync_code_simple_struct_enum(void) {
  const char *const filename = "test30.h";
  char *argv[2];
  argv[0] = cdd_test_mut_str(filename);
  argv[1] = cdd_test_mut_str("impl30.c");
  ASSERT_EQ(
      EXIT_SUCCESS,
      write_to_file(filename,
                    "enum ABC { X, Y, Z, };\n"
                    "enum DEF{A,B=5,C};\n"
                    "struct S { int foo; double bar; struct Foo *baz; };\n"
                    "struct T {};\n"
                    "struct U;\n"
                    "typedef int MyInt;\n"
                    "union MyUnion { int i; float f; };"));
  ASSERT_EQ(0, sync_code_main(2, argv));

  {
    extern C_CDD_EXPORT int g_io_calls;
    int k;
    cdd_c_error_t rc_sync;
    for (k = 0; k <= 120; k++) {
      g_io_calls = 0;
      g_fail_io_after = k;
      rc_sync = sync_code_main(2, argv);
      if (rc_sync != CDD_C_SUCCESS) {
        /* I/O failure injected */
      }
      g_fail_io_after = -1;
    }
  }

  remove(filename);
  remove("impl30.c");
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief test_sync_code_empty_header
 * @return TEST
 */
TEST test_sync_code_empty_header(void) {
  const char *const filename = "emptyheader.h";
  char *argv[2];
  argv[0] = cdd_test_mut_str(filename);
  argv[1] = cdd_test_mut_str("emptyimpl.c");
  ASSERT_EQ(0, write_to_file(filename, ""));
  ASSERT_EQ(0, sync_code_main(2, argv));
  remove(filename);
  remove("emptyimpl.c");
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief test_sync_code_no_struct_or_enum
 * @return TEST
 */
TEST test_sync_code_no_struct_or_enum(void) {
  const char *const filename = "nostructenum.h";
  char *argv[2];
  argv[0] = cdd_test_mut_str(filename);
  argv[1] = cdd_test_mut_str("noimpl.c");
  ASSERT_EQ(0, write_to_file(filename, "// just a comment\n"));
  ASSERT_EQ(0, sync_code_main(2, argv));
  remove(filename);
  remove("noimpl.c");
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief test_sync_code_impl_file_cannot_open
 * @return TEST
 */
TEST test_sync_code_impl_file_cannot_open(void) {
  const char *const filename = "onlystruct.h";
  char *argv[2];
  argv[0] = cdd_test_mut_str(filename);
  argv[1] = cdd_test_mut_str("/");
  ASSERT_EQ(0, write_to_file(filename, "struct X {int i;};\n"));
  ASSERT(sync_code_main(2, argv) != 0);
  remove(filename);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief test_sync_code_too_many_defs
 * @return TEST
 */
TEST test_sync_code_too_many_defs(void) {
  char *argv[2];
  const char *filename;
  FILE *f;
  int i;
  argv[0] = cdd_test_mut_str("too_many.h");
  argv[1] = cdd_test_mut_str("too_many.c");
  filename = argv[0];
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  ASSERT_EQ(0, fopen_s(&f, filename, "w"));
  ASSERT(f);
#elif defined(_MSC_VER)
  fopen_s(&f, filename, "w");
  ASSERT(f);
#else
#if defined(_MSC_VER)
  if (fopen_s(&f, filename, "w") != 0)
    f = NULL;
#else
  f = fopen(filename, "w");
#endif
  ASSERT(f);
#endif
  for (i = 0; i < 70; i++)
    fprintf(f, "struct S%d { int i; };\n", i);
  if (f)
    fclose(f);
  sync_code_main(2, argv);
  remove(filename);
  remove("too_many.c");
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief test_sync_code_unterminated_defs
 * @return TEST
 */
TEST test_sync_code_unterminated_defs(void) {
  char *argv[2];
  const char *filename;

  argv[0] = cdd_test_mut_str("unterminated.h");
  argv[1] = cdd_test_mut_str("unterminated.c");
  filename = argv[0];
  ASSERT_EQ(0, write_to_file(filename, "struct MyStruct { int x;"));
  ASSERT_EQ(0, sync_code_main(2, argv));

  ASSERT_EQ(0, write_to_file(filename, "enum MyEnum { A, B"));
  ASSERT_EQ(0, sync_code_main(2, argv));

  remove(filename);
  remove("unterminated.c");
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief test_patch_header_basic
 * @return TEST
 */
TEST test_patch_header_basic(void) {
  /*
     Header: void foo();
     Source: int foo() { return 0; }
     Expected Header: int foo();
  */
  const char *h_path = cdd_test_mut_str("basic_patch.h");
  const char *src = ""
                    "int foo() { return 0; }";
  char *content = NULL;
  size_t sz;
  int rc = 0;

  write_to_file(h_path, ""
                        "void foo();\n");

  rc = patch_header_from_source(h_path, src);
  ASSERT_EQ(0, rc);

  rc = read_to_file(h_path, "r", &content, &sz);
  ASSERT_EQ(0, rc);

  ASSERT(strstr(content, "int foo") != NULL);
  ASSERT(strstr(content, "void foo") == NULL);

  free(content);
  remove(h_path);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief test_patch_header_ptr_arg
 * @return TEST
 */
TEST test_patch_header_ptr_arg(void) {
  /*
    Header: char* bar(int x);
    Source: int bar(int x, char **out) { ... }
  */
  const char *h_path = cdd_test_mut_str("ptr_patch.h");
  const char *src = ""
                    "int bar(int x, char **out) { *out=0;return 0; }";
  char *content = NULL;
  size_t sz;
  int rc = 0;

  write_to_file(h_path, "char* bar(int x);\n");

  rc = patch_header_from_source(h_path, src);
  ASSERT_EQ(0, rc);

  rc = read_to_file(h_path, "r", &content, &sz);
  ASSERT_EQ(0, rc);

  ASSERT(strstr(content, "int bar") != NULL);
  /* Use lenient check for whitespace in generated output */
  ASSERT(strstr(content, "char * * out") != NULL ||
         strstr(content, "char **out") != NULL ||
         strstr(content, "char * *out") != NULL);

  free(content);
  remove(h_path);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief test_patch_header_ignore_others
 * @return TEST
 */
TEST test_patch_header_ignore_others(void) {
  /*
    Header contains irrelevant function.
    Source contains only 'foo'.
    Header 'other' should be untouched.
  */
  const char *h_path = cdd_test_mut_str("ignore_others.h");
  const char *src = ""
                    "int foo(void) { return 0; }";
  char *content = NULL;
  size_t sz;
  int rc = 0;

  write_to_file(h_path, ""
                        "void other();\nvoid foo();\n");

  rc = patch_header_from_source(h_path, src);
  ASSERT_EQ(0, rc);

  rc = read_to_file(h_path, "r", &content, &sz);
  ASSERT_EQ(0, rc);

  ASSERT(strstr(content, "void other") != NULL);
  ASSERT(strstr(content, "int foo") != NULL);

  free(content);
  remove(h_path);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_patch_header_bounds.
 * @return Greatest test result.
 */
TEST test_patch_header_bounds(void) {
  const char *h_path = cdd_test_mut_str("bounds_patch.h");
  const char *src = cdd_test_mut_str("int foo() { return 0; }");
  int rc = 0;

  /* End of file while looking for semicolon */
  write_to_file(h_path, "void foo()");
  rc = patch_header_from_source(h_path, src);
  ASSERT_EQ(0, rc);

  /* End of file while looking for paren */
  write_to_file(h_path, "void foo");
  rc = patch_header_from_source(h_path, src);
  ASSERT_EQ(0, rc);

  write_to_file(h_path, "void foo ");
  rc = patch_header_from_source(h_path, src);
  ASSERT_EQ(0, rc);

  write_to_file(h_path, "void foo bar");
  rc = patch_header_from_source(h_path, src);
  ASSERT_EQ(0, rc);

  /* Semicolon bounds looking backward */
  write_to_file(h_path, "{ void foo();");
  rc = patch_header_from_source(h_path, src);
  ASSERT_EQ(0, rc);

  write_to_file(h_path, "} void foo();");
  rc = patch_header_from_source(h_path, src);
  ASSERT_EQ(0, rc);

  remove(h_path);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_patch_header_failures.
 * @return Greatest test result.
 */
TEST test_patch_header_failures(void) {
#ifdef CDD_BUILD_TESTS
  const char *h_path = cdd_test_mut_str("fail_patch.h");
  const char *src = cdd_test_mut_str("int foo() { return 0; }");
  int rc = 0;
  /* extern C_CDD_EXPORT int g_cdd_sync_fail_func_sig_init; (moved to global) */
  /* extern C_CDD_EXPORT int g_cdd_sync_fail_patch_list_init; (moved to global)
   */
  /* extern C_CDD_EXPORT int g_cdd_sync_fail_extract; (moved to global) */
  /* extern C_CDD_EXPORT int g_cdd_sync_fail_tokenize; (moved to global) */
  /* extern C_CDD_EXPORT int g_cdd_sync_fail_patch_list_apply; (moved to global)
   */
  /* extern C_CDD_EXPORT int g_cdd_sync_fail_fopen_write; (moved to global) */

  write_to_file(h_path, "void foo();\n");

  /* Test func_sig_list_init failure */
  g_cdd_sync_fail_func_sig_init = 1;
  rc = patch_header_from_source(h_path, src);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  g_cdd_sync_fail_func_sig_init = 0;

  /* Test patch_list_init failure */
  g_cdd_sync_fail_patch_list_init = 1;
  rc = patch_header_from_source(h_path, src);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  g_cdd_sync_fail_patch_list_init = 0;

  /* Test extract failure */
  g_cdd_sync_fail_extract = 1;
  rc = patch_header_from_source(h_path, src);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  g_cdd_sync_fail_extract = 0;

  /* Test tokenize failure */
  g_cdd_sync_fail_tokenize = 1;
  rc = patch_header_from_source(h_path, src);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  g_cdd_sync_fail_tokenize = 0;

  /* Test patch_list_apply failure */
  g_cdd_sync_fail_patch_list_apply = 1;
  rc = patch_header_from_source(h_path, src);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  g_cdd_sync_fail_patch_list_apply = 0;

  /* Test fopen write failure */
  g_cdd_sync_fail_fopen_write = 1;
  rc = patch_header_from_source(h_path, src);
  ASSERT(rc != 0); /* will be EIO or CDD_C_ERROR_SYSTEM via errno */
  g_cdd_sync_fail_fopen_write = 0;

  /* Test read_to_file failure inside patch_header_from_source */
  {
    /* read_to_file fails if it can't open file. So give it a missing file. */
    rc = patch_header_from_source("missing_file_abc123.h", src);
    ASSERT(rc != 0);
  }

  {
    extern C_CDD_EXPORT int g_cdd_fail_sync_matches;
    write_to_file(h_path, "void foo();\n");
    g_cdd_fail_sync_matches = 1;
    rc = patch_header_from_source(h_path, src);
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
    g_cdd_fail_sync_matches = 0;
  }

  {
    extern C_CDD_EXPORT int g_cdd_strdup_fail;
    g_cdd_strdup_fail = 1;
    rc = patch_header_from_source(h_path, src);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    g_cdd_strdup_fail = 0;
  }

  {
    extern C_CDD_EXPORT int g_cdd_fail_sync_patch_add;
    g_cdd_fail_sync_patch_add = 1;
    rc = patch_header_from_source(h_path, src);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    g_cdd_fail_sync_patch_add = 0;
  }

  remove(h_path);
#endif
  PASS();
}

/**
 * @brief test_sync_oom
 * @return Test or helper result.
 */
TEST test_sync_oom(void) {
#ifdef CDD_BUILD_TESTS
  {
    char *argv[2];
    FILE *f;
    /*  (moved to global) */
    /* extern C_CDD_EXPORT int g_cdd_fprintf_fail; (moved to global) */
    int rc_s;
    int rc_s2;

    argv[0] = cdd_test_mut_str("header.h");
    argv[1] = cdd_test_mut_str("impl.c");
    remove("header.h");

#if defined(_MSC_VER)
    if (fopen_s(&f, "header.h", "w") != 0)
      f = NULL;
#else
    f = fopen("header.h", "w");
#endif
    if (f) {
      fprintf(f, "enum E { VAL1 };\nstruct A { int a; };\n");
      if (f)
        fclose(f);
    }

    {
      int k;
      for (k = 0; k <= 30; k++) {
        g_fail_io_after = k;
        rc_s = sync_code_main(2, (char **)(size_t)argv);
        if (rc_s != CDD_C_SUCCESS) {
          /* I/O failure injected */
        }
        g_fail_io_after = -1;
      }
    }

    {
      extern C_CDD_EXPORT int g_cdd_fail_get_basename;
      g_cdd_fail_get_basename = 1;
      rc_s = sync_code_main(2, (char **)(size_t)argv);
      ASSERT_EQ(CDD_C_ERROR_MEMORY, rc_s);
      g_cdd_fail_get_basename = 0;
    }

    g_cdd_alloc_fail = 1;
    rc_s = sync_code_main(2, (char **)(size_t)argv);
    if (rc_s != CDD_C_SUCCESS) {
      ASSERT_EQ(CDD_C_ERROR_MEMORY, rc_s);
    }
    g_cdd_alloc_fail = 0;

    g_cdd_fprintf_fail = 8001;
    rc_s = sync_code_main(2, (char **)(size_t)argv);
    ASSERT_EQ(CDD_C_ERROR_IO, rc_s);

    g_cdd_fprintf_fail = 8002;
    rc_s = sync_code_main(2, (char **)(size_t)argv);
    ASSERT_EQ(CDD_C_ERROR_NOT_FOUND, rc_s);

    g_cdd_fprintf_fail = 8003;
    rc_s = sync_code_main(2, (char **)(size_t)argv);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc_s);

    g_cdd_fprintf_fail = 8004;
    rc_s = sync_code_main(2, (char **)(size_t)argv);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc_s);

    g_cdd_fprintf_fail = 8005;
    rc_s2 = sync_code_main(2, (char **)(size_t)argv);
    g_cdd_fprintf_fail = 0;
    ASSERT_EQ(0, rc_s2);
    g_cdd_alloc_fail = 0;

    remove("header.h");
  }
#endif
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief sync_code_suite
 */
SUITE(sync_code_suite) {
  RUN_TEST(test_sync_code_wrong_args);
  RUN_TEST(test_sync_code_main_argc);
  RUN_TEST(test_sync_oom);
  RUN_TEST(test_sync_code_file_missing);
  RUN_TEST(test_sync_code_simple_struct_enum);
  RUN_TEST(test_sync_code_empty_header);
  RUN_TEST(test_sync_code_no_struct_or_enum);
  RUN_TEST(test_sync_code_impl_file_cannot_open);
  RUN_TEST(test_sync_code_too_many_defs);
  RUN_TEST(test_sync_code_unterminated_defs);
  RUN_TEST(test_patch_header_basic);
  RUN_TEST(test_patch_header_ptr_arg);
  RUN_TEST(test_patch_header_ignore_others);
  RUN_TEST(test_patch_header_bounds);
  RUN_TEST(test_patch_header_failures);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !TEST_SYNC_CODE_H */
