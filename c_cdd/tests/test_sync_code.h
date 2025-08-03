#ifndef TEST_SYNC_CODE_H
#define TEST_SYNC_CODE_H

#include <cdd_test_helpers/cdd_helpers.h>
#include <greatest.h>
#include <stdio.h>
#include <sync_code.h>

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
  ASSERT_EQ(EXIT_FAILURE, sync_code_main(2, argv));
  PASS();
}

TEST test_sync_code_simple_struct_enum(void) {
  const char *const filename = "test30.h";
  char *argv[] = {(char *)filename, "impl30.c"};
  ASSERT_EQ(
      0, write_to_file(filename,
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
  char *argv[] = {(char *)filename, "emptyimpl.c"};
  ASSERT_EQ(0, write_to_file(filename, ""));
  ASSERT_EQ(0, sync_code_main(2, argv));
  remove(filename);
  remove("emptyimpl.c");
  PASS();
}

TEST test_sync_code_no_struct_or_enum(void) {
  const char *const filename = "nostructenum.h";
  char *argv[] = {(char *)filename, "noimpl.c"};
  ASSERT_EQ(0, write_to_file(filename, "// just a comment\n"));
  ASSERT_EQ(0, sync_code_main(2, argv));
  remove(filename);
  remove("noimpl.c");
  PASS();
}

TEST test_sync_code_impl_file_cannot_open(void) {
  const char *const filename = "onlystruct.h";
  char *argv[] = {(char *)filename, "/"};
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
  ASSERT_EQ(0, fopen_s(&f, filename, "w"));
  ASSERT(f);
#else
  f = fopen(filename, "w");
  ASSERT(f);
#endif
  for (i = 0; i < 70; i++)
    fprintf(f, "struct S%d { int i; };\n", i);
  fclose(f);
  ASSERT_EQ(0, sync_code_main(2, argv));
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

TEST test_sync_code_messy_decls(void) {
  const char *const filename = "messy_header_sync.h";
  char *argv[] = {(char *)filename, "messy_impl_sync.c"};
  const char *header_content = "enum E1 { A, B, };\n"
                               "enum E2 { C,,D };\n" /* empty item */
                               "struct S1 {\n"
                               "  int field1;\n"
                               "  unparseable_line;\n"
                               "};\n";

  ASSERT_EQ(0, write_to_file(filename, header_content));
  ASSERT_EQ(0, sync_code_main(2, argv));
  remove(filename);
  remove(argv[1]);
  PASS();
}

TEST test_sync_code_single_line_defs(void) {
  const char *const filename = "sync_oneline.h";
  char *argv[] = {(char *)filename, "sync_oneline.c"};
  ASSERT_EQ(0,
            write_to_file(
                filename,
                "enum E { A, B, C }; struct S { int x; const char *s; };\n"));
  ASSERT_EQ(0, sync_code_main(2, argv));
  remove(filename);
  remove("sync_oneline.c");
  PASS();
}

TEST test_sync_code_compact_defs(void) {
  const char *const filename = "compact_defs.h";
  char *argv[] = {(char *)filename, "compact_defs.c"};
  ASSERT_EQ(0, write_to_file(filename, "struct S {int i;}; enum E{A,B};"));
  ASSERT_EQ(0, sync_code_main(2, argv));
  remove(filename);
  remove("compact_defs.c");
  PASS();
}

TEST test_sync_code_forward_declarations(void) {
  const char *const filename = "fwd_sync.h";
  char *argv[] = {(char *)filename, "fwd_sync.c"};
  ASSERT_EQ(0, write_to_file(filename, "struct MyStruct;\nenum MyEnum;\n"));
  ASSERT_EQ(0, sync_code_main(2, argv));
  /* Check that the impl file is generated but is empty of functions */
  {
    int err;
    size_t size;
    char *content = c_read_file(argv[1], &err, &size, "r");
    ASSERT_EQ(0, err);
    ASSERT_NEQ(NULL, content);
    /* Should only contain includes */
    ASSERT(strstr(content, "#include <stdlib.h>"));
    ASSERT(strstr(content, "/*") == NULL); /* No functions generated */
    free(content);
  }
  remove(filename);
  remove(argv[1]);
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
  RUN_TEST(test_sync_code_messy_decls);
  RUN_TEST(test_sync_code_single_line_defs);
  RUN_TEST(test_sync_code_compact_defs);
  RUN_TEST(test_sync_code_forward_declarations);
}

#endif /* !TEST_SYNC_CODE_H */
