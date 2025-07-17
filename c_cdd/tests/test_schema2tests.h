#ifndef TEST_SCHEMA2TESTS_H
#define TEST_SCHEMA2TESTS_H

#include <greatest.h>

#include <schema2tests.h>

TEST test_jsonschema2tests_wrong_args(void) {
  char *argv[] = {"program", NULL};
  const int rc = jsonschema2tests_main(1, argv);
  ASSERT_EQ(rc, EXIT_FAILURE);
  PASS();
}

TEST test_schema2tests_argc_error(void) {
  char *argv[] = {"prog", "a.json"};
  ASSERT_EQ(EXIT_FAILURE, jsonschema2tests_main(1, argv));
  PASS();
}

TEST test_schema2tests_bad_json(void) {
  FILE *fp;
  const char *const filename = "testfs.txt";
  const char *const filename1 = "bad_s2t.json";
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  {
    errno_t err = fopen_s(&fp, filename, "w");
    if (err != 0 || fp == NULL) {
      fprintf(stderr, "Failed to open header file %s\n", filename);
      return -1;
    }
  }
#else
  fp = fopen(filename, "w");
  if (!fp) {
    fprintf(stderr, "Failed to open file: %s\n", filename);
    return EXIT_FAILURE;
  }
#endif

  {
    FILE *fp1;
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
    errno_t err = fopen_s(&fp1, filename1, "w");
    if (err != 0 || fp1 == NULL) {
      fprintf(stderr, "Failed to open file %s\n", filename1);
      return -1;
    }
#else
    fp1 = fopen(filename1, "w");
    if (!fp1) {
      fprintf(stderr, "Failed to open file: %s\n", filename1);
      return EXIT_FAILURE;
    }
#endif
    fputs("{bad json", fp1);
    fclose(fp1);
  }
  {
    const char *argv[] = {filename1, "header.h", "out.h"};
    ASSERT_EQ(EXIT_FAILURE, jsonschema2tests_main(3, (char **)argv));
  }
  remove(filename1);
  PASS();
}

TEST test_schema2tests_success(void) {
  FILE *f = fopen("min_schema.json", "w");
  fputs("{\"components\": {\"schemas\": "
        "{\"E\":{\"type\":\"string\",\"enum\":[\"X\",\"Y\"]},"
        "\"S\":{\"type\":\"object\",\"properties\":{\"foo\": "
        "{\"type\":\"string\"}}}}}}",
        f);
  fclose(f);
  {
    char *argv[] = {"min_schema.json", "header.h", "test_s2t.h"};
    const int rc = jsonschema2tests_main(3, argv);
    ASSERT_EQ(0, rc);
  }
  remove("min_schema.json");
  remove("test_s2t.h");
  remove("test_main.c");
  PASS();
}

TEST test_schema2tests_output_file_open_fail(void) {
  /* Output file which can't be written
   * (write to a directory - not always portable, but usually fails) */
  FILE *fh;
  const char *const filename = "schema.2tests.json";
  int rc;
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  errno_t err = fopen_s(&fh, filename, "w");
  if (err != 0 || fh == NULL) {
    fprintf(stderr, "Failed to open %s for writing", filename);
    free(fh);
    return EXIT_FAILURE;
  }
#else
  fh = fopen(filename, "w");
#endif
  fputs("{\"$defs\":{}}", fh);
  fclose(fh);
  {
    char *argv[] = {"schema.2tests.json", "header.h", PATH_SEP};
    rc = jsonschema2tests_main(3, (char **)argv);
  }
  ASSERT(rc == EXIT_FAILURE || rc == -1);
  remove("schema.2tests.json");
  PASS();
}

TEST test_schema2tests_defs_fallback(void) {
  FILE *fh;
  const char *const filename = "defs_schema.json";
  int rc;
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  errno_t err = fopen_s(&fh, filename, "w");
  if (err != 0 || fh == NULL) {
    fprintf(stderr, "Failed to open %s for writing", filename);
    free(fh);
    return EXIT_FAILURE;
  }
#else
  fh = fopen(filename, "w");
#endif

  fputs("{\"$defs\":{\"E\":{\"type\":\"string\",\"enum\":[\"X\"]}}}", fh);
  fclose(fh);
  {
    char *argv[] = {"defs_schema.json", "header.h", "defs_out.h"};
    rc = jsonschema2tests_main(3, argv);
  }
  ASSERT_EQ(0, rc);
  remove("defs_schema.json");
  remove("defs_out.h");
  remove("test_main.c");
  PASS();
}

SUITE(schema2tests_suite) {
  RUN_TEST(test_jsonschema2tests_wrong_args);
  RUN_TEST(test_schema2tests_argc_error);
  RUN_TEST(test_schema2tests_bad_json);
  RUN_TEST(test_schema2tests_success);
  RUN_TEST(test_schema2tests_output_file_open_fail);
  RUN_TEST(test_schema2tests_defs_fallback);
}

#endif /* !TEST_SCHEMA2TESTS_H */
