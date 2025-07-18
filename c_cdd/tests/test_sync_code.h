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

/* File open error */
TEST test_sync_code_file_missing(void) {
  char *argv[] = {"notfound.h", "impl.c"};
  ASSERT_EQ(EXIT_FAILURE, sync_code_main(2, argv));
  PASS();
}

/* Working: header with struct + enum, then generate c impl file */
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

TEST test_sync_code_empty_header(void) {
  const char *const filename = "emptyheader.h";
  FILE *fp;

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

  fclose(fp);
  /* Should not crash */
  {
    char *argv[] = {NULL, "emptyimpl.c"};
    int rc;
    argv[0] = (char *)filename;
    rc = sync_code_main(2, argv);
    ASSERT(rc == 0 || rc == EXIT_SUCCESS);
  }
  remove(filename);
  remove("emptyimpl.c");
  PASS();
}

TEST test_sync_code_no_struct_or_enum(void) {
  const char *const filename = "nostructenum.h";
  FILE *fp;
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

  fputs("// just a comment\n", fp);
  fclose(fp);
  {
    char *argv[] = {NULL, "noimpl.c"};
    int rc;
    argv[0] = (char *)filename;
    rc = sync_code_main(2, argv);
    ASSERT(rc == 0);
  }
  remove(filename);
  remove("noimpl.c");
  PASS();
}

TEST test_sync_code_impl_file_cannot_open(void) {
  const char *const filename = "onlystruct.h";
  FILE *fp;
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
  fputs("struct X {int i;};\n", fp);
  fclose(fp);
  {
    char *argv[] = {NULL, "/"};
    int rc;
    argv[0] = (char *)filename;
    /* Try to write to directory (should fail) */
    rc = sync_code_main(2, argv);
    /* Should report failure */
    ASSERT(rc != 0);
  }
  remove(filename);
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
}

#endif /* !TEST_SYNC_CODE_H */
