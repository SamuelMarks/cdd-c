
/**
 * @file test_to_docs_json.h
 * @brief Tests for the to_docs_json CLI functionality.
 */

#ifndef C_CDD_TEST_TO_DOCS_JSON_H
#define C_CDD_TEST_TO_DOCS_JSON_H

#include "functions/parse_fs.h"
#include "greatest.h"
#include "routes/parse_cli.h"
#include <parson.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

#define TEMP_OUT_FILE "to_docs_json_out.json"
#define TEMP_SPEC_FILE "test_spec.json"

static void write_test_spec(void) {
  const char *spec =
      "{\n"
      "  \"openapi\": \"3.2.0\",\n"
      "  \"info\": { \"title\": \"Test API\", \"version\": \"1.0.0\" },\n"
      "  \"paths\": {\n"
      "    \"/pet\": {\n"
      "      \"get\": {\n"
      "        \"operationId\": \"getPet\",\n"
      "        \"responses\": {\n"
      "          \"200\": { \"description\": \"OK\" }\n"
      "        }\n"
      "      }\n"
      "    }\n"
      "  }\n"
      "}";
  FILE *fp = fopen(TEMP_SPEC_FILE, "w");
  if (fp) {
    fputs(spec, fp);
    fclose(fp);
  }
}

TEST test_to_docs_json_basic(void) {
  char *argv[] = {"to_docs_json", "-i", TEMP_SPEC_FILE};
  int rc;
  int stdout_fd;
  fpos_t pos;
  JSON_Value *val;
  JSON_Array *arr;
  JSON_Object *lang_obj;
  JSON_Array *ops;
  JSON_Object *op_obj;
  JSON_Object *code_obj;

  write_test_spec();

  /* Redirect stdout to capture output */
  fflush(stdout);
  fgetpos(stdout, &pos);
  stdout_fd = dup(fileno(stdout));
  freopen(TEMP_OUT_FILE, "w", stdout);

  rc = to_docs_json_cli_main(3, argv);

  /* Restore stdout */
  fflush(stdout);
  dup2(stdout_fd, fileno(stdout));
  close(stdout_fd);
  clearerr(stdout);
  fsetpos(stdout, &pos);

  ASSERT_EQ(0, rc);

  val = json_parse_file(TEMP_OUT_FILE);
  ASSERT(val != NULL);

  arr = json_value_get_array(val);
  ASSERT(arr != NULL);
  ASSERT_EQ(1, json_array_get_count(arr));

  lang_obj = json_array_get_object(arr, 0);
  ASSERT_STR_EQ("c", json_object_get_string(lang_obj, "language"));

  ops = json_object_get_array(lang_obj, "operations");
  ASSERT(ops != NULL);
  ASSERT_EQ(1, json_array_get_count(ops));

  op_obj = json_array_get_object(ops, 0);
  ASSERT_STR_EQ("GET", json_object_get_string(op_obj, "method"));
  ASSERT_STR_EQ("/pet", json_object_get_string(op_obj, "path"));
  ASSERT_STR_EQ("getPet", json_object_get_string(op_obj, "operationId"));

  code_obj = json_object_get_object(op_obj, "code");
  ASSERT(code_obj != NULL);
  ASSERT(json_object_has_value(code_obj, "imports"));
  ASSERT(json_object_has_value(code_obj, "wrapper_start"));
  ASSERT(json_object_has_value(code_obj, "wrapper_end"));
  ASSERT(json_object_has_value(code_obj, "snippet"));

  json_value_free(val);

  remove(TEMP_SPEC_FILE);
  remove(TEMP_OUT_FILE);
  PASS();
}

TEST test_to_docs_json_no_imports_no_wrapping(void) {
  char *argv[] = {"to_docs_json", "--no-imports", "--no-wrapping", "-i",
                  TEMP_SPEC_FILE};
  int rc;
  int stdout_fd;
  fpos_t pos;
  JSON_Value *val;
  JSON_Array *arr;
  JSON_Object *lang_obj;
  JSON_Array *ops;
  JSON_Object *op_obj;
  JSON_Object *code_obj;

  write_test_spec();

  /* Redirect stdout */
  fflush(stdout);
  fgetpos(stdout, &pos);
  stdout_fd = dup(fileno(stdout));
  freopen(TEMP_OUT_FILE, "w", stdout);

  rc = to_docs_json_cli_main(5, argv);

  /* Restore stdout */
  fflush(stdout);
  dup2(stdout_fd, fileno(stdout));
  close(stdout_fd);
  clearerr(stdout);
  fsetpos(stdout, &pos);

  ASSERT_EQ(0, rc);

  val = json_parse_file(TEMP_OUT_FILE);
  ASSERT(val != NULL);

  arr = json_value_get_array(val);
  lang_obj = json_array_get_object(arr, 0);
  ops = json_object_get_array(lang_obj, "operations");
  op_obj = json_array_get_object(ops, 0);
  code_obj = json_object_get_object(op_obj, "code");

  ASSERT(!json_object_has_value(code_obj, "imports"));
  ASSERT(!json_object_has_value(code_obj, "wrapper_start"));
  ASSERT(!json_object_has_value(code_obj, "wrapper_end"));
  ASSERT(json_object_has_value(code_obj, "snippet"));

  json_value_free(val);

  remove(TEMP_SPEC_FILE);
  remove(TEMP_OUT_FILE);
  PASS();
}

SUITE(to_docs_json_suite) {
  RUN_TEST(test_to_docs_json_basic);
  RUN_TEST(test_to_docs_json_no_imports_no_wrapping);
}

#endif /* C_CDD_TEST_TO_DOCS_JSON_H */
