#ifndef TEST_SCHEMA2TESTS_H
#define TEST_SCHEMA2TESTS_H

#include "cdd_test_helpers/cdd_helpers.h"
#include <greatest.h>

#include "fs.h"
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
  const char *const filename = "bad_s2t.json";
  int rc = write_to_file(filename, "{bad json");
  char *argv[] = {(char *)filename, "header.h", "out.h"};
  ASSERT_EQ(rc, EXIT_SUCCESS);
  ASSERT_EQ(EXIT_FAILURE, jsonschema2tests_main(3, argv));
  remove(filename);
  PASS();
}

TEST test_schema2tests_success(void) {
  FILE *f;
  char *argv[] = {"min_schema.json", "header.h", "build" PATH_SEP "test_s2t.h"};
  int rc_main;
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  errno_t err = fopen_s(&f, "min_schema.json", "w");
  if (err != 0 || f == NULL) {
    fprintf(stderr, "Failed to open header file %s\n", "min_schema.json");
    FAIL();
  }
#else
  f = fopen("min_schema.json", "w");
  ASSERT(f);
#endif
  fputs("{\"components\": {\"schemas\": "
        "{\"E\":{\"type\":\"string\",\"enum\":[\"X\",\"Y\"]},"
        "\"S\":{\"type\":\"object\",\"properties\":{\"foo\": "
        "{\"type\":\"string\"}}}}}}",
        f);
  fclose(f);

  rc_main = jsonschema2tests_main(3, argv);
  ASSERT_EQ(0, rc_main);

  remove("min_schema.json");
  remove("build" PATH_SEP "test_s2t.h");
  remove("build" PATH_SEP "test_main.c");
  rmdir("build");
  PASS();
}

TEST test_schema2tests_output_file_open_fail(void) {
  /* Output file which can't be written
   * (write to a directory - not always portable, but usually fails) */
  const char *const schema_filename = "schema.2tests.json";
  char *argv[] = {(char *)schema_filename, "header.h", "a_dir"};
  int rc = write_to_file(schema_filename, "{\"$defs\":{}}");
  ASSERT_EQ(EXIT_SUCCESS, rc);
  makedir("a_dir");
  rc = jsonschema2tests_main(3, argv);
  ASSERT(rc == EXIT_FAILURE || rc == -1);
  remove(schema_filename);
  rmdir("a_dir");

  /* Test makedirs failure by creating a file with the same name as output dir
   */
  {
    const char *out_dir_as_file = "out_dir_file.tmp";
    char out_path[256];
    FILE *f;
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
    ASSERT_EQ(0, fopen_s(&f, out_dir_as_file, "w"));
    ASSERT(f);
#else
    f = fopen(out_dir_as_file, "w");
    ASSERT(f);
#endif
    fclose(f);

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
    sprintf_s(out_path, sizeof(out_path), "%s%sout.h", out_dir_as_file,
              PATH_SEP);
#else
    sprintf(out_path, "%s%sout.h", out_dir_as_file, PATH_SEP);
#endif
    argv[2] = out_path;

    rc = jsonschema2tests_main(3, argv);
    ASSERT(rc != 0);
    remove(out_dir_as_file);
  }

  PASS();
}

TEST test_schema2tests_defs_fallback(void) {
  const char *const filename = "defs_schema.json";
  char *argv[] = {"defs_schema.json", "header.h",
                  "build" PATH_SEP "defs_out.h"};
  int rc = write_to_file(
      filename, "{\"$defs\":{\"E\":{\"type\":\"string\",\"enum\":[\"X\"]}}}");
  ASSERT_EQ(EXIT_SUCCESS, rc);
  rc = jsonschema2tests_main(3, argv);
  ASSERT_EQ(0, rc);
  remove("defs_schema.json");
  remove("build" PATH_SEP "defs_out.h");
  remove("build" PATH_SEP "test_main.c");
  rmdir("build");
  PASS();
}

TEST test_schema2tests_invalid_schema_root(void) {
  const char *const schema_file = "bad_root.json";
  char *argv[] = {(char *)schema_file, "header.h", "out.h"};
  write_to_file(schema_file, "[]");
  ASSERT_EQ(EXIT_FAILURE, jsonschema2tests_main(3, argv));
  remove(schema_file);
  PASS();
}
TEST test_schema2tests_no_schemas_object(void) {
  const char *const schema_file = "no_schemas.json";
  char *argv[] = {(char *)schema_file, "header.h", "out.h"};
  write_to_file(schema_file, "{}");
  ASSERT_EQ(EXIT_FAILURE, jsonschema2tests_main(3, argv));
  remove(schema_file);
  PASS();
}
TEST test_schema2tests_malformed_schemas(void) {
  const char *const schema_file = "malformed.json";
  char *argv[] = {(char *)schema_file, "header.h", "build" PATH_SEP "out.h"};
  int rc;
  /* non-object schema, no type, non-string enum member */
  write_to_file(schema_file, "{\"components\":{\"schemas\":{"
                             "\"E1\":{\"type\":\"string\",\"enum\":[\"X\",1]},"
                             "\"S1\":null,"
                             "\"S2\":{\"properties\":{}},"
                             "\"S3\":{\"type\":\"object\"}"
                             "}}}");
  rc = jsonschema2tests_main(3, argv);
  ASSERT_EQ(0, rc); /* Should succeed but generate empty/partial tests */
  remove(schema_file);
  remove("build" PATH_SEP "out.h");
  remove("build" PATH_SEP "test_main.c");
  rmdir("build");
  PASS();
}

TEST test_schema2tests_with_null_enum_val(void) {
  const char *const filename = "null_enum.json";
  char *argv[] = {"null_enum.json", "header.h",
                  "build" PATH_SEP "null_enum_out.h"};
  int rc = write_to_file(filename, "{\"$defs\":{\"E\":{\"type\":\"string\","
                                   "\"enum\":[\"X\", null, \"Y\"]}}}");
  ASSERT_EQ(EXIT_SUCCESS, rc);
  rc = jsonschema2tests_main(3, argv);
  ASSERT_EQ(0, rc); /* Should succeed, just skipping the null value */
  remove(filename);
  remove("build" PATH_SEP "null_enum_out.h");
  remove("build" PATH_SEP "test_main.c");
  rmdir("build");
  PASS();
}

TEST test_schema2tests_generated_output(void) {
#define OUT_DIR "build_s2t_test_output"
  char *argv[] = {OUT_DIR PATH_SEP "check_test_output.json",
                  OUT_DIR PATH_SEP "check_header_output.h",
                  OUT_DIR PATH_SEP "check_test_gen.h"};
  const char *const schema_file = argv[0];
  /* const char *const header_name = argv[1]; */
  const char *const output_file = argv[2];
  const char main_c_path[] = OUT_DIR PATH_SEP "test_main.c";
  int err;
  size_t size;
  char *test_content;

  makedirs(OUT_DIR);

  write_to_file(schema_file,
                "{\"components\":{\"schemas\":{"
                "\"MyEnum\":{\"type\":\"string\",\"enum\":[\"VAL1\","
                "\"VAL2\"]},"
                "\"MyStruct\":{\"type\":\"object\",\"properties\":{"
                "\"num\":{\"type\":\"integer\"}}}}}}");

  ASSERT_EQ(0, jsonschema2tests_main(3, argv));

  test_content = c_read_file(output_file, &err, &size, "r");
  ASSERT_EQ(0, err);
  ASSERT(test_content != NULL);

  ASSERT(strstr(test_content, "TEST test_MyEnum_to_str_from_str(void)"));
  ASSERT(strstr(test_content, "ASSERT_STR_EQ(\"VAL1\", str);"));
  ASSERT(strstr(test_content, "RUN_TEST(test_MyEnum_to_str_from_str);"));
  ASSERT(strstr(test_content,
                "TEST test_MyStruct_default_deepcopy_eq_cleanup(void)"));
  ASSERT(strstr(test_content, "RUN_TEST(test_MyStruct_json_roundtrip);"));

  free(test_content);

  remove(main_c_path);
  remove(schema_file);
  remove(output_file);
  rmdir(OUT_DIR);

  PASS();
#undef OUT_DIR
}

SUITE(schema2tests_suite) {
  RUN_TEST(test_jsonschema2tests_wrong_args);
  RUN_TEST(test_schema2tests_argc_error);
  RUN_TEST(test_schema2tests_bad_json);
  RUN_TEST(test_schema2tests_success);
  RUN_TEST(test_schema2tests_output_file_open_fail);
  RUN_TEST(test_schema2tests_defs_fallback);
  RUN_TEST(test_schema2tests_invalid_schema_root);
  RUN_TEST(test_schema2tests_no_schemas_object);
  RUN_TEST(test_schema2tests_malformed_schemas);
  RUN_TEST(test_schema2tests_with_null_enum_val);
  RUN_TEST(test_schema2tests_generated_output);
}

#endif /* !TEST_SCHEMA2TESTS_H */
