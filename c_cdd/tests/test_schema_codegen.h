#ifndef TEST_SCHEMA_CODEGEN_H
#define TEST_SCHEMA_CODEGEN_H

#include <greatest.h>
#include <schema_codegen.h>

#include "fs.h"

TEST test_schema2code_wrong_args(void) {
  char *argv[] = {"program", NULL};
  const int rc = schema2code_main(1, argv);
  ASSERT_EQ(EXIT_FAILURE, rc);
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
    errno_t err = fopen_s(&fp, broken_json, "w, ccs=UTF-8");
    if (err != 0 || fp == NULL) {
      fprintf(stderr, "Failed to open file %s\n", broken_json);
      FAIL();
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
    errno_t err = fopen_s(&fp, "bad_schema.json", "w, ccs=UTF-8");
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
  errno_t err = fopen_s(&fp, filename, "w, ccs=UTF-8");
  if (err != 0 || fp == NULL) {
    fprintf(stderr, "Failed to open file %s\n", filename);
    FAIL();
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
  argv[0] = (char *)filename;
  ASSERT_EQ(EXIT_FAILURE, schema2code_main(2, argv));
  remove(filename);
  PASS();
}

TEST test_schema_codegen_empty_schema(void) {
  FILE *fp;
  const char *const filename = "empty.json";
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  errno_t err = fopen_s(&fp, filename, "w, ccs=UTF-8");
  if (err != 0 || fp == NULL) {
    fprintf(stderr, "Failed to open file %s\n", filename);
    FAIL();
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
    ASSERT_EQ(0, schema2code_main(2, argv));
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
  errno_t err = fopen_s(&fp, filename, "w, ccs=UTF-8");
  if (err != 0 || fp == NULL) {
    FAILm("Failed to open file");
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
  enum { COUNT = 1000, str0 = 345, str1 = 289 };
  char *const schema_str = calloc(COUNT, sizeof(char));
  const char *add_str0 =
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
      "\"StructWithArray\": ";
  const char *add_str1 =
      "{\"type\": \"object\", \"properties\": {\"arr\": "
      "{\"type\": \"array\", \"items\": {\"type\": \"string\"}}}},"
      "\"StringNotEnum\": {\"type\": \"string\"},"
      "\"TopLevelArray\": {\"type\": \"array\", \"items\": {\"type\": "
      "\"integer\"}},"
      "\"StructWithBadRef\": {\"type\": \"object\", \"properties\": "
      "{\"ref_prop\": {\"$ref\": \"#/c/s/Other\"}}}"
      "}}}\0";

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  strcpy_s((char *)schema_str, COUNT, add_str0);
  strcat_s((char *)schema_str, COUNT, add_str1);
#else
  strcpy((char *)schema_str, add_str0);
  strcat((char *)schema_str, add_str1);
#endif

  rc = write_to_file(filename, (char *)schema_str);
  ASSERT_EQ(0, rc);

  rc = schema2code_main(2, (char **)argv);
  ASSERT_EQ(0, rc);

  free(schema_str);
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
  errno_t err = fopen_s(&fp, filename, "w, ccs=UTF-8");
  if (err != 0 || fp == NULL) {
    fprintf(stderr, "Failed to open file %s\n", filename);
    FAIL();
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
  const char *const schema_filename = "codegen_fail_schema.json";
  const char *const out_dir_as_file = "codegen_fail_dir_is_file";
  const char out_path_base[] = "codegen_fail_dir_is_file" PATH_SEP "out_base";
  FILE *fp;

  /* 1. Create a dummy schema file */
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  errno_t err = fopen_s(&fp, schema_filename, "w, ccs=UTF-8");
  if (err != 0 || fp == NULL) {
    FAIL();
  }
#else
  fp = fopen(schema_filename, "w");
  if (!fp) {
    FAIL();
  }
#endif
  fputs("{\"components\":{\"schemas\":{}}}", fp);
  fclose(fp);

  /* 2. Create a file that we will try to use as a directory */
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  err = fopen_s(&fp, out_dir_as_file, "w, ccs=UTF-8");
  if (err != 0 || fp == NULL) {
    remove(schema_filename);
    FAIL();
  }
#else
  fp = fopen(out_dir_as_file, "w");
  if (!fp) {
    remove(schema_filename);
    FAIL();
  }
#endif
  fclose(fp);

  /* 3. Run the function and assert it fails */
  {
    int rc;
    char *argv[] = {(char *)schema_filename, NULL};
    argv[1] = (char *)out_path_base;
    rc = schema2code_main(2, argv);
    /* Should fail as 'out_dir_as_file' in the path is a file, not a directory
     */
    ASSERT(rc != 0);
  }

  /* 5. Cleanup */
  remove(schema_filename);
  remove(out_dir_as_file);
  PASS();
}

TEST test_schema_codegen_types_unhandled(void) {
  const char *const filename = "codegen_type.json";
  char *argv[] = {NULL, "typeout"};
  ASSERT_EQ(0, write_to_file(
                   filename,
                   "{\"components\":{\"schemas\":{"
                   "\"T\":{\"type\":\"array\",\"items\":{\"type\":\"object\"}},"
                   "\"Q\":{\"properties\":{\"f\":{}}}}}}"));
  argv[0] = (char *)filename;
  ASSERT_EQ(0, schema2code_main(2, argv));
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
                         "\"Q\":{\"properties\":{\"f\":{}}}}}}"); /* no type */
  ASSERT_EQ(EXIT_SUCCESS, rc);
  argv[0] = (char *)filename;
  rc = schema2code_main(2, argv);
  ASSERT_EQ(0, rc);
  remove(filename);
  remove("typeout2.h");
  remove("typeout2.c");
  PASS();
}

TEST test_schema_codegen_header_null_basename(void) {
  /* This will try to fopen(NULL), want to ensure it handles gracefully */
  char *argv[] = {"schema.json", "."};
  write_to_file("schema.json", "{}");
  /* It might fail with parsing or file opening, but shouldn't crash */
  ASSERT_EQ(EXIT_FAILURE, schema2code_main(2, argv));
  remove("schema.json");
  PASS();
}

TEST test_schema_codegen_various_types(void) {
  const char *const filename = "various_types.json";
  const char *argv[] = {filename, "various_types_out"};
  int rc;
  const char *schema =
      "{\"components\": {\"schemas\": {"
      "\"S1\": {\"type\": \"object\", \"properties\": {"
      "\"p_str\": {\"type\": \"string\"},"
      "\"p_int\": {\"type\": \"integer\"},"
      "\"p_num\": {\"type\": \"number\"},"
      "\"p_bool\": {\"type\": \"boolean\"},"
      "\"p_obj_unresolved\": {\"type\": \"object\"},"
      "\"p_arr\": {\"type\": \"array\", \"items\": "
      "{\"$ref\": \"#/c/s/Other\"}},"
      "\"p_arr_no_ref\": {\"type\": \"array\"},"
      "\"p_arr_no_items\": {\"type\": \"array\", \"items\": {}},"
      "\"p_unhandled\": {\"type\": \"null\"}"
      "  }},"
      "\"E1\": {\"type\": \"string\", \"enum\": [\"A\", null, \"B\"]},"
      "\"ToplevelArray\": {\"type\": \"array\"}"
      "}}}";

  rc = write_to_file(filename, schema);
  ASSERT_EQ(0, rc);

  rc = schema2code_main(2, (char **)argv);
  ASSERT_EQ(0, rc);

  remove(filename);
  remove("various_types_out.h");
  remove("various_types_out.c");
  PASS();
}

TEST test_schema_codegen_malformed_props(void) {
  const char *const filename = "malformed_props.json";
  const char *argv[] = {filename, "malformed_props_out"};
  int rc;
  const char *schema =
      "{\"components\":{\"schemas\":{"
      "\"S1\": {\"type\": \"object\", \"properties\": {\"p1\": null, \"p2\": "
      "{\"type\": \"string\"}}}"
      "}}}";
  rc = write_to_file(filename, schema);
  ASSERT_EQ(0, rc);
  rc = schema2code_main(2, (char **)argv);
  ASSERT_EQ(0, rc);
  remove(filename);
  remove("malformed_props_out.h");
  remove("malformed_props_out.c");
  PASS();
}

TEST test_schema_codegen_empty_properties(void) {
  const char *argv[] = {"empty_props.json", "empty_props_out"};
  const char *const filename = argv[0];
  int rc;
  const char *schema = "{\"components\":{\"schemas\":{"
                       "\"S1\": {\"type\": \"object\", \"properties\": {}}"
                       "}}}";

  rc = write_to_file(filename, schema);
  ASSERT_EQ(0, rc);

  rc = schema2code_main(2, (char **)argv);
  ASSERT_EQ(0, rc);

  /* Check generated files. Should be a struct with no fields. */
  {
    int err;
    size_t fsize;
    char *content = NULL;
    err = read_to_file("empty_props_out.h", "r", &content, &fsize);
    ASSERT_EQ(0, err);
    ASSERT(strstr(content, "struct LIB_EXPORT S1 {\n};") != NULL);
    free(content);
  }

  remove(filename);
  remove("empty_props_out.h");
  remove("empty_props_out.c");
  PASS();
}

SUITE(schema_codegen_suite) {
  RUN_TEST(test_schema2code_wrong_args);
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  /* TODO: Get them to work on MSVC */
#else
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
  RUN_TEST(test_schema_codegen_various_types);
  RUN_TEST(test_schema_codegen_malformed_props);
  RUN_TEST(test_schema_codegen_empty_properties);
#endif
}

#endif /* !TEST_SCHEMA_CODEGEN_H */
