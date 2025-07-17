#ifndef TEST_SCHEMA_CODEGEN_H
#define TEST_SCHEMA_CODEGEN_H

#include <greatest.h>

#include <schema_codegen.h>

TEST test_schema2code_wrong_args(void) {
  char *argv[] = {"program", NULL};
  const int rc = schema2code_main(1, argv);
  ASSERT_EQ(rc, EXIT_FAILURE);
  PASS();
}

TEST test_schema2code_input_errors(void) {
  const char *const broken_json = "broken.json";
  const char *argv0[] = {"missing_file.json", "basename"};
  const char *argv1[] = {broken_json, "basename"};
  FILE *fp;

  ASSERT_EQ(EXIT_FAILURE, schema2code_main(2, (char **)argv0));

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  {
    errno_t err = fopen_s(&fp, broken_json, "r");
    if (err != 0 || fp == NULL) {
      fprintf(stderr, "Failed to open file %s\n", broken_json);
      return -1;
    }
  }
#else
  fp = fopen(broken_json, "w");
#endif
  fputs("{not-a-json-file", fp);
  fclose(fp);
  ASSERT_EQ(EXIT_FAILURE, schema2code_main(2, (char **)argv1));
  remove(broken_json);
  PASS();
}

TEST test_schema_codegen_argc_error(void) {
  char *argv[] = {"oneFile"};
  ASSERT_EQ(EXIT_FAILURE, schema2code_main(1, argv));
  PASS();
}

// Bad/missing file
TEST test_schema_codegen_bad_file(void) {
  char *argv[] = {"notfound.json", "basename"};
  ASSERT_EQ(EXIT_FAILURE, schema2code_main(2, argv));
  PASS();
}

// Broken JSON
TEST test_schema_codegen_broken_json(void) {
  char *argv[] = {"bad.json", "basename"};
  const char *const filename = "broken.json";
  FILE *fp;
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  {
    errno_t err = fopen_s(&fp, filename, "w");
    if (err != 0 || fp == NULL) {
      fprintf(stderr, "Failed to open file %s\n", filename);
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

  fputs("{broken", fp);
  fclose(fp);
  ASSERT_EQ(EXIT_FAILURE, schema2code_main(2, argv));
  remove("bad.json");
  PASS();
}

// Empty schemas
TEST test_schema_codegen_empty_schema(void) {
  FILE *fp;
  const char *const filename = "empty.json";
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  {
    errno_t err = fopen_s(&fp, filename, "w");
    if (err != 0 || fp == NULL) {
      fprintf(stderr, "Failed to open file %s\n", filename);
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
  fputs("{\"components\": {\"schemas\": {}}}", fp);
  fclose(fp);
  {
    char *argv[] = {"empty.json", "basename"};
    schema2code_main(2, argv);
  }
  remove("empty.json");
  PASS();
}

// Valid: test enum and struct output
TEST test_schema_codegen_valid_struct_enum(void) {
  FILE *fp;
  const char *const filename = "myschema.json";
  const char *argv[] = {filename, "mybaz"};
  int rc;

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  {
    errno_t err = fopen_s(&fp, filename, "w");
    if (err != 0 || fp == NULL) {
      fprintf(stderr, "Failed to open file %s\n", filename);
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
  fputs("{\"components\": {\"schemas\": {"
        "\"Foo\": {\"type\": \"object\", \"properties\": { \"x\": {\"type\": "
        "\"string\"}, \"v\": {\"type\": \"integer\"}}},"
        "\"Bar\": {\"type\": \"string\", \"enum\": [\"A\", \"B\"]}}}}",
        fp);
  fclose(fp);
  rc = schema2code_main(2, (char **)argv);
  ASSERT_EQ(0, rc);
  remove(filename);
  remove("mybaz.h");
  remove("mybaz.c");
  PASS();
}

SUITE(schema_codegen_suite) {
  RUN_TEST(test_schema2code_wrong_args);
  RUN_TEST(test_schema2code_input_errors);
  RUN_TEST(test_schema_codegen_argc_error);
  RUN_TEST(test_schema_codegen_bad_file);
  RUN_TEST(test_schema_codegen_broken_json);
  RUN_TEST(test_schema_codegen_empty_schema);
  RUN_TEST(test_schema_codegen_valid_struct_enum);
}

#endif /* !TEST_SCHEMA_CODEGEN_H */
