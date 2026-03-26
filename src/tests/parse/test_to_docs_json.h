
/**
 * @file test_to_docs_json.h
 * @brief Tests for the to_docs_json CLI functionality.
 */

#ifndef C_CDD_TEST_TO_DOCS_JSON_H
#define C_CDD_TEST_TO_DOCS_JSON_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "functions/parse/fs.h"
#include "greatest.h"
#include "routes/parse/cli.h"
#include <parson.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <io.h>
#else
#if !defined(_MSC_VER)
#include <unistd.h>
#endif
#endif
/* clang-format on */

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
  FILE *fp = NULL;
#if defined(_MSC_VER)
  if (fopen_s(&fp, TEMP_SPEC_FILE, "w") != 0)
    fp = NULL;
#else
  fp = fopen(TEMP_SPEC_FILE, "w");
#endif
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

  JSON_Object *root_obj = json_value_get_object(val);
  ASSERT(root_obj != NULL);

  JSON_Object *endpoints_obj = json_object_get_object(root_obj, "endpoints");
  ASSERT(endpoints_obj != NULL);

  JSON_Object *pet_obj = json_object_get_object(endpoints_obj, "/pet");
  ASSERT(pet_obj != NULL);

  const char *code_str = json_object_get_string(pet_obj, "get");
  ASSERT(code_str != NULL);

  ASSERT(strstr(code_str, "#include \"generated_client.h\"") != NULL);
  ASSERT(strstr(code_str, "int main(void)") != NULL);
  ASSERT(strstr(code_str, "api_getPet") != NULL);

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

  JSON_Object *root_obj = json_value_get_object(val);
  ASSERT(root_obj != NULL);

  JSON_Object *endpoints_obj = json_object_get_object(root_obj, "endpoints");
  ASSERT(endpoints_obj != NULL);

  JSON_Object *pet_obj = json_object_get_object(endpoints_obj, "/pet");
  ASSERT(pet_obj != NULL);

  const char *code_str = json_object_get_string(pet_obj, "get");
  ASSERT(code_str != NULL);

  ASSERT(strstr(code_str, "#include") == NULL);
  ASSERT(strstr(code_str, "int main(void)") == NULL);
  ASSERT(strstr(code_str, "api_getPet") != NULL);

  json_value_free(val);

  remove(TEMP_SPEC_FILE);
  remove(TEMP_OUT_FILE);
  PASS();
}

SUITE(to_docs_json_suite) {
  RUN_TEST(test_to_docs_json_basic);
  RUN_TEST(test_to_docs_json_no_imports_no_wrapping);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_TEST_TO_DOCS_JSON_H */
