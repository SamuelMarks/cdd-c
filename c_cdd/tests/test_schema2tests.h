#ifndef TEST_SCHEMA2TESTS_H
#define TEST_SCHEMA2TESTS_H

#include "cdd_test_helpers/cdd_helpers.h"
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
  const char *const filename = "bad_s2t.json";
  int rc = write_to_file(filename, "{bad json");
  ASSERT_EQ(rc, EXIT_SUCCESS);
  {
    const char *argv[] = {filename, "header.h", "out.h"};
    ASSERT_EQ(EXIT_FAILURE, jsonschema2tests_main(3, (char **)argv));
  }
  remove(filename);
  PASS();
}

TEST test_schema2tests_success(void) {
  FILE *f;
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  {
    errno_t err = fopen_s(&f, "min_schema.json", "w");
    if (err != 0 || f == NULL) {
      fprintf(stderr, "Failed to open header file %s\n", "min_schema.json");
      return;
    }
  }
#else
  f = fopen("min_schema.json", "w");
#endif
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
  const char *const schema_filename = "schema.2tests.json";
  char *argv[] = {(char *)schema_filename, "header.h", PATH_SEP};
  int rc = write_to_file(schema_filename, "{\"$defs\":{}}");
  ASSERT_EQ(EXIT_SUCCESS, rc);
  rc = jsonschema2tests_main(3, argv);
  ASSERT(rc == EXIT_FAILURE || rc == -1);
  remove(schema_filename);

  /* Test makedirs failure by creating a file with the same name as output dir
   */
  {
    const char *out_dir_as_file = "out_dir_file.tmp";
    char out_path[256];
    FILE *f;
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
    ASSERT_EQ(0, fopen_s(&f, out_dir_as_file, "w"));
#else
    f = fopen(out_dir_as_file, "w");
    ASSERT(f);
#endif
    fclose(f);

    sprintf(out_path, "%s%sout.h", out_dir_as_file, PATH_SEP);
    argv[2] = out_path;

    rc = jsonschema2tests_main(3, argv);
    ASSERT(rc != 0);
    remove(out_dir_as_file);
  }

  PASS();
}

TEST test_schema2tests_defs_fallback(void) {
  const char *const filename = "defs_schema.json";
  char *argv[] = {"defs_schema.json", "header.h", "defs_out.h"};
  int rc = write_to_file(
      filename, "{\"$defs\":{\"E\":{\"type\":\"string\",\"enum\":[\"X\"]}}}");
  ASSERT_EQ(EXIT_SUCCESS, rc);
  rc = jsonschema2tests_main(3, argv);
  ASSERT_EQ(0, rc);
  remove("defs_schema.json");
  remove("defs_out.h");
  remove("test_main.c");
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
  char *argv[] = {(char *)schema_file, "header.h", "out.h"};
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
  remove("out.h");
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
  RUN_TEST(test_schema2tests_invalid_schema_root);
  RUN_TEST(test_schema2tests_no_schemas_object);
  RUN_TEST(test_schema2tests_malformed_schemas);
}

#endif /* !TEST_SCHEMA2TESTS_H */
