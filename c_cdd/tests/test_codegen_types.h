/**
 * @file test_codegen_types.h
 * @brief Unit tests for Advanced Types (Unions/Arrays) generation.
 *
 * @author Samuel Marks
 */

#ifndef TEST_CODEGEN_TYPES_H
#define TEST_CODEGEN_TYPES_H

#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "codegen_struct.h" /* For struct_fields init helpers */
#include "codegen_types.h"

/* --- Union Tests --- */

TEST test_write_union_to_json(void) {
  struct StructFields sf;
  struct CodegenTypesConfig config = {0};
  FILE *tmp = tmpfile();
  char *content;
  long sz;

  ASSERT(tmp);
  struct_fields_init(&sf);
  struct_fields_add(&sf, "id", "integer", NULL, NULL, NULL);
  struct_fields_add(&sf, "name", "string", NULL, NULL, NULL);

  /* Generate */
  ASSERT_EQ(0, write_union_to_json_func(tmp, "MyUnion", &sf, &config));

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);
  content = (char *)calloc(1, sz + 1);
  fread(content, 1, sz, tmp);

  /* Check for switch on tag */
  ASSERT(strstr(content, "switch (obj->tag)"));
  /* Check case for id */
  ASSERT(strstr(content, "case MyUnion_id:"));
  ASSERT(strstr(content, "obj->data.id"));
  /* Check case for name */
  ASSERT(strstr(content, "case MyUnion_name:"));
  ASSERT(strstr(content, "obj->data.name"));

  free(content);
  struct_fields_free(&sf);
  fclose(tmp);
  PASS();
}

TEST test_write_union_from_json(void) {
  struct StructFields sf;
  FILE *tmp = tmpfile();
  char *content;
  long sz;

  ASSERT(tmp);
  struct_fields_init(&sf);
  struct_fields_add(&sf, "val", "integer", NULL, NULL, NULL);

  /* Generate */
  ASSERT_EQ(0, write_union_from_jsonObject_func(tmp, "NumU", &sf, NULL));

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);
  content = (char *)calloc(1, sz + 1);
  fread(content, 1, sz, tmp);

  ASSERT(strstr(content, "malloc(sizeof(struct NumU))"));
  /* Check discriminator check */
  ASSERT(strstr(content, "if (json_object_has_value(jsonObject, \"val\"))"));
  ASSERT(strstr(content, "ret->tag = NumU_val;"));
  ASSERT(strstr(content, "ret->data.val = (int)json_object_get_number"));

  free(content);
  struct_fields_free(&sf);
  fclose(tmp);
  PASS();
}

TEST test_write_union_cleanup_switch(void) {
  struct StructFields sf;
  FILE *tmp = tmpfile();
  char *content;
  long sz;

  ASSERT(tmp);
  struct_fields_init(&sf);
  struct_fields_add(&sf, "str", "string", NULL, NULL, NULL);
  struct_fields_add(&sf, "num", "integer", NULL, NULL, NULL);

  ASSERT_EQ(0, write_union_cleanup_func(tmp, "U", &sf, NULL));

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);
  content = (char *)calloc(1, sz + 1);
  fread(content, 1, sz, tmp);

  ASSERT(strstr(content, "switch (obj->tag)"));
  /* Integer should do nothing implicit */
  /* String should free */
  ASSERT(strstr(content, "case U_str:\n      free((void*)obj->data.str);"));

  free(content);
  struct_fields_free(&sf);
  fclose(tmp);
  PASS();
}

/* --- Root Array Tests --- */

TEST test_root_array_string_cleanup(void) {
  FILE *tmp = tmpfile();
  char *content;
  long sz;

  ASSERT(tmp);
  ASSERT_EQ(0,
            write_root_array_cleanup_func(tmp, "StrArr", "string", NULL, NULL));

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);
  content = (char *)calloc(1, sz + 1);
  fread(content, 1, sz, tmp);

  ASSERT(strstr(content, "void StrArr_cleanup(char **in, size_t len)"));
  ASSERT(strstr(content, "free(in[i])"));
  ASSERT(strstr(content, "free(in)"));

  free(content);
  fclose(tmp);
  PASS();
}

TEST test_root_array_int_from_json(void) {
  FILE *tmp = tmpfile();
  char *content;
  long sz;

  ASSERT(tmp);
  ASSERT_EQ(
      0, write_root_array_from_json_func(tmp, "IntArr", "integer", NULL, NULL));

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);
  content = (char *)calloc(1, sz + 1);
  fread(content, 1, sz, tmp);

  ASSERT(
      strstr(content,
             "int IntArr_from_json(const char *json, int **out, size_t *len)"));
  ASSERT(strstr(content, "malloc(count * sizeof(int))"));
  ASSERT(strstr(content, "json_array_get_number"));

  free(content);
  fclose(tmp);
  PASS();
}

TEST test_root_array_obj_to_json(void) {
  FILE *tmp = tmpfile();
  char *content;
  long sz;

  ASSERT(tmp);
  ASSERT_EQ(
      0, write_root_array_to_json_func(tmp, "ObjArr", "object", "Obj", NULL));

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);
  content = (char *)calloc(1, sz + 1);
  fread(content, 1, sz, tmp);

  ASSERT(strstr(content, "Obj_to_json(in[i], &tmp)"));
  ASSERT(strstr(content, "jasprintf(json_out, \"[\")"));

  free(content);
  fclose(tmp);
  PASS();
}

/* Guard Logic */
TEST test_union_guards(void) {
  struct StructFields sf;
  struct CodegenTypesConfig cfg;
  FILE *tmp = tmpfile();
  char *content;
  long sz;

  ASSERT(tmp);
  struct_fields_init(&sf);
  struct_fields_add(&sf, "x", "integer", NULL, NULL, NULL);

  cfg.json_guard = "JSON_G";
  cfg.utils_guard = NULL;

  ASSERT_EQ(0, write_union_to_json_func(tmp, "GuardedU", &sf, &cfg));

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);
  content = (char *)calloc(1, sz + 1);
  fread(content, 1, sz, tmp);

  ASSERT(strstr(content, "#ifdef JSON_G"));
  ASSERT(strstr(content, "#endif /* JSON_G */"));

  free(content);
  struct_fields_free(&sf);
  fclose(tmp);
  PASS();
}

TEST test_types_null_args(void) {
  ASSERT_EQ(EINVAL, write_union_cleanup_func(NULL, "U", NULL, NULL));
  ASSERT_EQ(EINVAL, write_root_array_cleanup_func(NULL, "A", "T", NULL, NULL));
  PASS();
}

SUITE(codegen_types_suite) {
  RUN_TEST(test_write_union_to_json);
  RUN_TEST(test_write_union_from_json);
  RUN_TEST(test_write_union_cleanup_switch);
  RUN_TEST(test_root_array_string_cleanup);
  RUN_TEST(test_root_array_int_from_json);
  RUN_TEST(test_root_array_obj_to_json);
  RUN_TEST(test_union_guards);
  RUN_TEST(test_types_null_args);
}

#endif /* TEST_CODEGEN_TYPES_H */
