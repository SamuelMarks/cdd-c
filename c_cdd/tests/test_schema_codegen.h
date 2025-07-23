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
  const char *argv2[] = {"bad_schema.json", "basename"};
  FILE *fp;

  ASSERT_EQ(EXIT_FAILURE, schema2code_main(2, (char **)argv0));

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  {
    errno_t err = fopen_s(&fp, broken_json, "w");
    if (err != 0 || fp == NULL) {
      fprintf(stderr, "Failed to open file %s\n", broken_json);
      return;
    }
  }
#else
  fp = fopen(broken_json, "w");
  ASSERT(fp);
#endif
  fputs("{not-a-json-file", fp);
  fclose(fp);
  ASSERT_EQ(EXIT_FAILURE, schema2code_main(2, (char **)argv1));
  remove(broken_json);

  /* Valid JSON but not a valid schema (e.g., not an object) */
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  {
    errno_t err = fopen_s(&fp, "bad_schema.json", "w");
    if (err != 0 || fp == NULL) {
      FAILm("Failed to open file");
    }
  }
#else
  fp = fopen("bad_schema.json", "w");
  ASSERT(fp);
#endif
  fputs("[]", fp);
  fclose(fp);
  ASSERT_EQ(EXIT_FAILURE, schema2code_main(2, (char **)argv2));
  remove("bad_schema.json");
  PASS();
}

TEST test_schema_codegen_argc_error(void) {
  char *argv[] = {"oneFile"};
  ASSERT_EQ(EXIT_FAILURE, schema2code_main(1, argv));
  PASS();
}

/* Bad/missing file */
TEST test_schema_codegen_bad_file(void) {
  char *argv[] = {"notfound.json", "basename"};
  ASSERT_EQ(EXIT_FAILURE, schema2code_main(2, argv));
  PASS();
}

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
      return;
    }
  }
#else
  fp = fopen(filename, "w");
  if (!fp) {
    fprintf(stderr, "Failed to open file: %s\n", filename);
    FAIL();
  }
#endif

  fputs("{broken", fp);
  fclose(fp);
  ASSERT_EQ(EXIT_FAILURE, schema2code_main(2, argv));
  remove("bad.json");
  remove("broken.json");
  PASS();
}

TEST test_schema_codegen_empty_schema(void) {
  FILE *fp;
  const char *const filename = "empty.json";
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  {
    errno_t err = fopen_s(&fp, filename, "w");
    if (err != 0 || fp == NULL) {
      fprintf(stderr, "Failed to open file %s\n", filename);
      return;
    }
  }
#else
  fp = fopen(filename, "w");
  if (!fp) {
    fprintf(stderr, "Failed to open file: %s\n", filename);
    FAIL();
  }
#endif
  fputs("{\"components\": {\"schemas\": {}}}", fp);
  fclose(fp);
  {
    char *argv[] = {"empty.json", "basename"};
    schema2code_main(2, argv);
  }
  remove("empty.json");
  remove("basename.h");
  remove("basename.c");
  PASS();
}

TEST test_schema_codegen_no_defs(void) {
  const char *const filename = "no_defs.json";
  const char *argv[] = {filename, "mybaz"};
  FILE *fp;
  int rc;

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  {
    errno_t err = fopen_s(&fp, filename, "w");
    if (err != 0 || fp == NULL) {
      FAILm("Failed to open file");
    }
  }
#else
  fp = fopen(filename, "w");
  ASSERT(fp);
#endif
  fputs("{\"title\": \"Some schema\"}", fp);
  fclose(fp);
  rc = schema2code_main(2, (char **)argv);
  ASSERT_EQ(EXIT_FAILURE, rc);
  remove(filename);
  PASS();
}

TEST test_schema_codegen_complex_schema(void) {
  const char *const filename = "complex_schema.json";
  const char *argv[] = {filename, "complex"};
  int rc;

  /* string literal of length 629 exceeds maximum length
   * 509 that C90 compilers are required to support */
  enum { COUNT = 630 };
  char *const complex_schema_json = malloc(COUNT * sizeof(complex_schema_json));
  strcpy(
      complex_schema_json,
      "{\"components\": {\"schemas\": {"
      "\"EnumNoUnknown\": {\"type\": \"string\", \"enum\": [\"A\", null, "
      "\"B\"]},"
      "\"EnumWithUnknown\": {\"type\": \"string\", \"enum\": [\"C\", "
      "\"UNKNOWN\"]},"
      "\"StructEmpty\": {\"type\": \"object\"},"
      "\"StructWithBadProp\": {\"type\": \"object\", \"properties\": {\"p1\": "
      "null}},"
      "\"StructWithNoTypeProp\": {\"type\": \"object\", \"properties\": "
      "{\"p1\": {}}},"
      "\"StructWithArray\": {\"type\": \"object\", \"properties\": {\"arr\": "
      "{\"type\": \"array\", \"items\": {\"type\": \"string\"}}}},"
      "\"StringNotEnum\": {\"type\": \"string\"},"
      "\"TopLevelArray\": {\"type\": \"array\", \"ite");
  strcat(complex_schema_json,
         "ms\": {\"type\": \"integer\"}},"
         "\"StructWithBadRef\": {\"type\": \"object\", \"properties\": "
         "{\"ref_prop\": {\"$ref\": \"#/c/s/Other\"}}}"
         "}}}");
  rc = write_to_file(filename, complex_schema_json);
  free(complex_schema_json);
  ASSERT_EQ(0, rc);

  rc = schema2code_main(2, (char **)argv);
  ASSERT_EQ(0, rc);

  remove(filename);
  remove("complex.h");
  remove("complex.c");

  PASS();
}

/* Valid: test enum and struct output */
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
      return;
    }
  }
#else
  fp = fopen(filename, "w");
  if (!fp) {
    fprintf(stderr, "Failed to open file: %s\n", filename);
    FAIL();
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

TEST test_schema_codegen_output_file_open_fail(void) {
  const char *const filename = "codegen_fail.json";
  FILE *fp;
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  {
    errno_t err = fopen_s(&fp, filename, "w");
    if (err != 0 || fp == NULL) {
      fprintf(stderr, "Failed to open file %s\n", filename);
      return;
    }
  }
#else
  fp = fopen(filename, "w");
  if (!fp) {
    fprintf(stderr, "Failed to open file: %s\n", filename);
    FAIL();
  }
#endif
  fputs("{\"components\":{\"schemas\":{}}}", fp);
  fclose(fp);
  /* Simulate unable to open output by passing directory (usually fails) */
  {
    int rc;
    char *argv[] = {NULL, "/"};
    argv[0] = (char *)filename;
    rc = schema2code_main(2, argv);
    /* Should fail as it can't open '/' for writing headers */
    ASSERT(rc != 0);
  }
  remove(filename);
  PASS();
}

TEST test_schema_codegen_types_unhandled(void) {
  const char *const filename = "codegen_type.json";
  char *argv[] = {NULL, "typeout"};
  int rc = write_to_file(
      filename, "{\"components\":{\"schemas\":{"
                "\"T\":{\"type\":\"array\",\"items\":{\"type\":\"object\"}},"
                "\"Q\":{\"properties\":{\"f\":{}}}}}");
  argv[0] = (char *)filename;
  rc = schema2code_main(2, argv);
  ASSERT_EQ(rc, rc == 0 ? 0 : 1);
  /* Should not crash, should just skip type or print "unknown type" */
  remove(filename);
  remove("typeout.h");
  remove("typeout.c");
  PASS();
}

TEST test_schema_codegen_missing_type_object(void) {
  const char *const filename = "missingtype.json";
  char *argv[] = {NULL, "typeout2"};
  int rc = write_to_file(filename,
                         "{\"components\":{\"schemas\":{"
                         "\"Q\":{\"properties\":{\"f\":{}}}}}"); /* no type */
  ASSERT_EQ(EXIT_SUCCESS, rc);
  argv[0] = (char *)filename;
  rc = schema2code_main(2, argv);
  ASSERT(rc == 0 || rc == 1);
  remove(filename);
  remove("typeout2.h");
  remove("typeout2.c");
  PASS();
}

TEST test_schema_codegen_header_null_basename(void) {
  /* This will try to fopen(NULL), want to ensure it handles gracefully */
  char *argv[] = {NULL, NULL};
  ASSERT_EQ(EXIT_FAILURE, schema2code_main(2, argv));
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
  RUN_TEST(test_schema_codegen_no_defs);
  RUN_TEST(test_schema_codegen_complex_schema);
  RUN_TEST(test_schema_codegen_output_file_open_fail);
  RUN_TEST(test_schema_codegen_types_unhandled);
  RUN_TEST(test_schema_codegen_missing_type_object);
  RUN_TEST(test_schema_codegen_header_null_basename);
}

#endif /* !TEST_SCHEMA_CODEGEN_H */
