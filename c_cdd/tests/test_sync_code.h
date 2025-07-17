#ifndef TEST_SYNC_CODE_H
#define TEST_SYNC_CODE_H

#include <greatest.h>

#include <sync_code.h>

TEST test_sync_code_wrong_args(void) {
  char *argv[] = {"program", NULL};
  const int rc = sync_code_main(1, argv);
  ASSERT_EQ(rc, EXIT_FAILURE);
  PASS();
}

TEST test_sync_code_main_argc(void) {
  char *argv[] = {"foo.h"};
  ASSERT_EQ(EXIT_FAILURE, sync_code_main(1, argv));
  PASS();
}

// File open error
TEST test_sync_code_file_missing(void) {
  char *argv[] = {"notfound.h", "impl.c"};
  ASSERT_EQ(EXIT_FAILURE, sync_code_main(2, argv));
  PASS();
}

// Working: header with struct + enum, then generate c impl file
TEST test_sync_code_simple_struct_enum(void) {
  FILE *fp;
  int rc;
  const char *const filename = "test30.h";
  const char *argv[] = {filename, "impl30.c"};
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  errno_t err = fopen_s(&fp, filename, "w");
  if (err != 0 || fp == NULL) {
    fprintf(stderr, "Failed to open file %s\n", filename);
    return -1;
  }
#else
  fp = fopen(filename, "w");
  if (!fp) {
    fprintf(stderr, "Failed to open file: %s\n", filename);
    return EXIT_FAILURE;
  }
#endif
  fputs("enum ABC { X, Y, Z };\n"
        "struct S { int foo; double bar; struct Foo *baz; };\n",
        fp);
  fclose(fp);

  rc = sync_code_main(2, (char **)argv);
  ASSERT_EQ(0, rc);
  remove(filename);
  remove("impl30.c");
  PASS();
}

SUITE(sync_code_suite) {
  RUN_TEST(test_sync_code_wrong_args);
  RUN_TEST(test_sync_code_main_argc);
  RUN_TEST(test_sync_code_file_missing);
  RUN_TEST(test_sync_code_simple_struct_enum);
}

#endif /* !TEST_SYNC_CODE_H */
