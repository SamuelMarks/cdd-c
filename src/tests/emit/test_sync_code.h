#ifndef TEST_SYNC_CODE_H
#define TEST_SYNC_CODE_H

#include "functions/emit/sync.h"
#include <cdd_test_helpers/cdd_helpers.h>
#include <greatest.h>
#include <stdio.h>

TEST test_sync_code_wrong_args(void) {
  char *argv[] = {"program", NULL};
  ASSERT_EQ(EXIT_FAILURE, sync_code_main(1, argv));
  PASS();
}

TEST test_sync_code_main_argc(void) {
  char *argv[] = {"foo.h"};
  ASSERT_EQ(EXIT_FAILURE, sync_code_main(1, argv));
  PASS();
}

TEST test_sync_code_file_missing(void) {
  char *argv[] = {"notfound.h", "impl.c"};
  ASSERT_EQ(ENOENT, sync_code_main(2, argv));
  PASS();
}

TEST test_sync_code_simple_struct_enum(void) {
  const char *const filename = "test30.h";
  char *argv[2];
  argv[0] = (char *)filename;
  argv[1] = "impl30.c";
  ASSERT_EQ(
      EXIT_SUCCESS,
      write_to_file(filename,
                    "enum ABC { X, Y, Z, };\n"
                    "enum DEF{A,B=5,C};\n"
                    "struct S { int foo; double bar; struct Foo *baz; };\n"
                    "struct T {};\n"
                    "struct U;"));
  ASSERT_EQ(0, sync_code_main(2, argv));
  remove(filename);
  remove("impl30.c");
  PASS();
}

TEST test_sync_code_empty_header(void) {
  const char *const filename = "emptyheader.h";
  char *argv[2];
  argv[0] = (char *)filename;
  argv[1] = "emptyimpl.c";
  ASSERT_EQ(0, write_to_file(filename, ""));
  ASSERT_EQ(0, sync_code_main(2, argv));
  remove(filename);
  remove("emptyimpl.c");
  PASS();
}

TEST test_sync_code_no_struct_or_enum(void) {
  const char *const filename = "nostructenum.h";
  char *argv[2];
  argv[0] = (char *)filename;
  argv[1] = "noimpl.c";
  ASSERT_EQ(0, write_to_file(filename, "// just a comment\n"));
  ASSERT_EQ(0, sync_code_main(2, argv));
  remove(filename);
  remove("noimpl.c");
  PASS();
}

TEST test_sync_code_impl_file_cannot_open(void) {
  const char *const filename = "onlystruct.h";
  char *argv[2];
  argv[0] = (char *)filename;
  argv[1] = "/";
  ASSERT_EQ(0, write_to_file(filename, "struct X {int i;};\n"));
  ASSERT(sync_code_main(2, argv) != 0);
  remove(filename);
  PASS();
}

TEST test_sync_code_too_many_defs(void) {
  char *argv[] = {"too_many.h", "too_many.c"};
  const char *const filename = argv[0];
  FILE *f;
  int i;
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  ASSERT_EQ(0, fopen_s(&f, filename, "w, ccs=UTF-8"));
  ASSERT(f);
#else
  f = fopen(filename, "w");
  ASSERT(f);
#endif
  for (i = 0; i < 70; i++)
    fprintf(f, "struct S%d { int i; };\n", i);
  fclose(f);
  sync_code_main(2, argv);
  remove(filename);
  remove("too_many.c");
  PASS();
}

TEST test_sync_code_unterminated_defs(void) {
  char *argv[] = {"unterminated.h", "unterminated.c"};
  const char *const filename = argv[0];

  ASSERT_EQ(0, write_to_file(filename, "struct MyStruct { int x;"));
  ASSERT_EQ(0, sync_code_main(2, argv));

  ASSERT_EQ(0, write_to_file(filename, "enum MyEnum { A, B"));
  ASSERT_EQ(0, sync_code_main(2, argv));

  remove(filename);
  remove("unterminated.c");
  PASS();
}

TEST test_patch_header_basic(void) {
  /*
     Header: void foo();
     Source: int foo() { return 0; }
     Expected Header: int foo();
  */
  const char *h_path = "basic_patch.h";
  const char *src = "int foo() { return 0; }";
  char *content;
  size_t sz;
  int rc;

  write_to_file(h_path, "void foo();\n");

  rc = patch_header_from_source(h_path, src);
  ASSERT_EQ(0, rc);

  rc = read_to_file(h_path, "r", &content, &sz);
  ASSERT_EQ(0, rc);

  ASSERT(strstr(content, "int foo") != NULL);
  ASSERT(strstr(content, "void foo") == NULL);

  free(content);
  remove(h_path);
  PASS();
}

TEST test_patch_header_ptr_arg(void) {
  /*
    Header: char* bar(int x);
    Source: int bar(int x, char **out) { ... }
  */
  const char *h_path = "ptr_patch.h";
  const char *src = "int bar(int x, char **out) { *out=0;return 0; }";
  char *content;
  size_t sz;
  int rc;

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
  PASS();
}

TEST test_patch_header_ignore_others(void) {
  /*
    Header contains irrelevant function.
    Source contains only 'foo'.
    Header 'other' should be untouched.
  */
  const char *h_path = "ignore_others.h";
  const char *src = "int foo(void) { return 0; }";
  char *content;
  size_t sz;
  int rc;

  write_to_file(h_path, "void other();\nvoid foo();\n");

  rc = patch_header_from_source(h_path, src);
  ASSERT_EQ(0, rc);

  rc = read_to_file(h_path, "r", &content, &sz);
  ASSERT_EQ(0, rc);

  ASSERT(strstr(content, "void other") != NULL);
  ASSERT(strstr(content, "int foo") != NULL);

  free(content);
  remove(h_path);
  PASS();
}

SUITE(sync_code_suite) {
  RUN_TEST(test_sync_code_wrong_args);
  RUN_TEST(test_sync_code_main_argc);
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
}

#endif /* !TEST_SYNC_CODE_H */
