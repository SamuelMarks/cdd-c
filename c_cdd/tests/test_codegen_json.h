/**
 * @file test_codegen_json.h
 * @brief Unit tests for JSON Serialization generator.
 *
 * Verifies that the correct `parson` calls and `jasprintf` patterns
 * are emitted for various struct layouts.
 *
 * @author Samuel Marks
 */

#ifndef TEST_CODEGEN_JSON_H
#define TEST_CODEGEN_JSON_H

#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "codegen_json.h"
#include "codegen_struct.h" /* For struct_fields_init etc */

/* Common Setup */
static void setup_fields(struct StructFields *sf) {
  struct_fields_init(sf);
  struct_fields_add(sf, "id", "integer", NULL, "0", NULL);
  struct_fields_add(sf, "data", "string", NULL, NULL, NULL);
}

TEST test_json_to_plain(void) {
  FILE *tmp = tmpfile();
  struct StructFields sf;
  long sz;
  char *content;

  ASSERT(tmp);
  setup_fields(&sf);

  ASSERT_EQ(0, write_struct_to_json_func(tmp, "Data", &sf, NULL));

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);
  content = (char *)calloc(1, sz + 1);
  fread(content, 1, sz, tmp);

  /* Check structure */
  ASSERT(strstr(content, "jasprintf(json, \"{\");"));
  ASSERT(strstr(content, "jasprintf(json, \"\\\"id\\\": %d\", obj->id)"));
  ASSERT(strstr(content,
                "jasprintf(json, \"\\\"data\\\": \\\"%s\\\"\", obj->data)"));
  ASSERT(strstr(content, "jasprintf(json, \"}\");"));

  free(content);
  struct_fields_free(&sf);
  fclose(tmp);
  PASS();
}

TEST test_json_from_plain(void) {
  FILE *tmp = tmpfile();
  struct StructFields sf;
  long sz;
  char *content;

  ASSERT(tmp);
  setup_fields(&sf);

  ASSERT_EQ(0, write_struct_from_jsonObject_func(tmp, "Data", &sf, NULL));

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);
  content = (char *)calloc(1, sz + 1);
  fread(content, 1, sz, tmp);

  /* Check parson usage */
  ASSERT(strstr(content,
                "ret->id = (int)json_object_get_number(jsonObject, \"id\")"));
  ASSERT(strstr(content, "json_object_get_string(jsonObject, \"data\")"));
  ASSERT(strstr(content, "strdup(s)"));

  free(content);
  struct_fields_free(&sf);
  fclose(tmp);
  PASS();
}

TEST test_json_recursive_obj(void) {
  FILE *tmp = tmpfile();
  struct StructFields sf;
  char *content;
  long sz;

  ASSERT(tmp);
  struct_fields_init(&sf);
  struct_fields_add(&sf, "child", "object", "ChildType", NULL, NULL);

  ASSERT_EQ(0, write_struct_to_json_func(tmp, "Parent", &sf, NULL));

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);
  content = (char *)calloc(1, sz + 1);
  fread(content, 1, sz, tmp);

  /* Check recursive call pattern */
  ASSERT(strstr(content, "rc = ChildType_to_json(obj->child, &s);"));
  ASSERT(strstr(content, "jasprintf(json, \"\\\"child\\\": %s\", s);"));

  free(content);
  struct_fields_free(&sf);
  fclose(tmp);
  PASS();
}

TEST test_json_array_logic(void) {
  FILE *tmp = tmpfile();
  struct StructFields sf;
  char *content;
  long sz;

  ASSERT(tmp);
  struct_fields_init(&sf);
  /* Array of strings */
  struct_fields_add(&sf, "tags", "array", "string", NULL, NULL);

  ASSERT_EQ(0, write_struct_from_jsonObject_func(tmp, "Post", &sf, NULL));

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);
  content = (char *)calloc(1, sz + 1);
  fread(content, 1, sz, tmp);

  /* Check array loop extraction */
  ASSERT(strstr(content, "json_object_get_array(jsonObject, \"tags\")"));
  ASSERT(strstr(content, "json_array_get_count(arr)"));
  ASSERT(strstr(content, "calloc(ret->n_tags, sizeof(char*))"));

  free(content);
  struct_fields_free(&sf);
  fclose(tmp);
  PASS();
}

TEST test_json_guards(void) {
  FILE *tmp = tmpfile();
  struct StructFields sf;
  struct CodegenJsonConfig config;
  char *content;
  long sz;

  ASSERT(tmp);
  setup_fields(&sf);
  config.guard_macro = "JSON_ENABLED";

  ASSERT_EQ(0, write_struct_from_json_func(tmp, "Data", &config));

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);
  content = (char *)calloc(1, sz + 1);
  fread(content, 1, sz, tmp);

  ASSERT(strstr(content, "#ifdef JSON_ENABLED"));
  ASSERT(strstr(content, "#endif /* JSON_ENABLED */"));

  free(content);
  struct_fields_free(&sf);
  fclose(tmp);
  PASS();
}

TEST test_json_null_args(void) {
  ASSERT_EQ(EINVAL, write_struct_to_json_func(NULL, "S", NULL, NULL));
  ASSERT_EQ(EINVAL, write_struct_from_json_func(NULL, "S", NULL));
  ASSERT_EQ(EINVAL, write_struct_from_jsonObject_func(NULL, "S", NULL, NULL));
  PASS();
}

SUITE(codegen_json_suite) {
  RUN_TEST(test_json_to_plain);
  RUN_TEST(test_json_from_plain);
  RUN_TEST(test_json_recursive_obj);
  RUN_TEST(test_json_array_logic);
  RUN_TEST(test_json_guards);
  RUN_TEST(test_json_null_args);
}

#endif /* TEST_CODEGEN_JSON_H */
