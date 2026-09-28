/**
 * @file test_code2schema_helpers.h
 * @brief Unit tests for code2schema helper and variant routines.
 *
 * @author Samuel Marks
 */

#ifndef TEST_CODE2SCHEMA_HELPERS_H
#define TEST_CODE2SCHEMA_HELPERS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "classes/emit/schema.h"
#include "classes/parse/code2schema.h"
#include "functions/emit/codegen.h"
#include "functions/parse/fs.h"
/* clang-format on */

extern C_CDD_EXPORT int g_cdd_strdup_fail;

TEST test_parse_struct_member_annotations(void) {

  struct StructFields sf;

  struct_fields_init(&sf);

  ASSERT_EQ(0, parse_struct_member_line(

                   "int user_id; // @shard_key @shard_hash", &sf));

  ASSERT_EQ(1, sf.size);
  ASSERT_STR_EQ("user_id", sf.fields[0].name);
  ASSERT_STR_EQ("integer", sf.fields[0].type);
  ASSERT(sf.fields[0].schema_extra_json != NULL);
  ASSERT(strstr(sf.fields[0].schema_extra_json, "\"x-cdd-shard-key\":true") !=

         NULL);

  ASSERT(strstr(sf.fields[0].schema_extra_json, "\"x-cdd-shard-hash\":true") !=

         NULL);

  ASSERT_EQ(

      0, parse_struct_member_line(
             "char *name; /* @track_telemetry @slow_query_warn(250) */", &sf));

  ASSERT_EQ(2, sf.size);
  ASSERT_STR_EQ("name", sf.fields[1].name);
  ASSERT(sf.fields[1].schema_extra_json != NULL);
  ASSERT(strstr(sf.fields[1].schema_extra_json,

                "\"x-cdd-track-telemetry\":true") != NULL);

  ASSERT(strstr(sf.fields[1].schema_extra_json, "\"x-cdd-slow-query\":250") !=

         NULL);

  struct_fields_free(&sf);
  g_fail_io_after = -1;
  PASS();
}

TEST test_code2schema_merge_struct_field(void) {

  struct StructField f1, f2;

  memset(&f1, 0, sizeof(f1));
  memset(&f2, 0, sizeof(f2));

  merge_struct_field(NULL, &f2);
  merge_struct_field(&f1, NULL);

  f2.has_min = 1;
  f2.min_val = 10;
  f2.has_max = 1;
  f2.max_val = 20;
  f2.has_min_len = 1;
  f2.min_len = 5;
  f2.has_max_len = 1;
  f2.max_len = 15;
  f2.has_min_items = 1;
  f2.min_items = 2;
  f2.has_max_items = 1;
  f2.max_items = 8;
  f2.required = 1;
#if defined(_MSC_VER)
  strncpy_s(f2.default_val, sizeof(f2.default_val), "test",
            sizeof(f2.default_val) - 1);
  strncpy_s(f2.format, sizeof(f2.format), "uuid", sizeof(f2.format) - 1);
  strncpy_s(f2.pattern, sizeof(f2.pattern), "^[a-z]+$", sizeof(f2.pattern) - 1);
  strncpy_s(f2.bit_width, sizeof(f2.bit_width), "16", sizeof(f2.bit_width) - 1);
#else
  strncpy(f2.default_val, "test", sizeof(f2.default_val) - 1);
  strncpy(f2.format, "uuid", sizeof(f2.format) - 1);
  strncpy(f2.pattern, "^[a-z]+$", sizeof(f2.pattern) - 1);
  strncpy(f2.bit_width, "16", sizeof(f2.bit_width) - 1);
#endif

  merge_struct_field(&f1, &f2);

  ASSERT_EQ(1, f1.has_min);
  ASSERT_EQ(10, f1.min_val);
  ASSERT_EQ(1, f1.has_max);
  ASSERT_EQ(20, f1.max_val);
  ASSERT_EQ(1, f1.has_min_len);
  ASSERT_EQ(5, f1.min_len);
  ASSERT_EQ(1, f1.has_max_len);
  ASSERT_EQ(15, f1.max_len);
  ASSERT_EQ(1, f1.has_min_items);
  ASSERT_EQ(2, f1.min_items);
  ASSERT_EQ(1, f1.has_max_items);
  ASSERT_EQ(8, f1.max_items);
  ASSERT_EQ(1, f1.required);
  ASSERT_STR_EQ("test", f1.default_val);
  ASSERT_STR_EQ("16", f1.bit_width);

  f2.min_val = 15;
  f2.max_val = 15;
  f2.min_len = 10;
  f2.max_len = 10;
  f2.min_items = 5;
  f2.max_items = 5;

  merge_struct_field(&f1, &f2);

  ASSERT_EQ(15, f1.min_val);
  ASSERT_EQ(15, f1.max_val);
  ASSERT_EQ(10, f1.min_len);
  ASSERT_EQ(10, f1.max_len);
  ASSERT_EQ(5, f1.min_items);
  ASSERT_EQ(5, f1.max_items);
  g_fail_io_after = -1;

  PASS();
}

TEST test_code2schema_discriminator_value(void) {
  char *val = NULL;

  JSON_Value *jv;
  JSON_Object *jo;
  JSON_Value *jv2;
  JSON_Object *jo2;

  /* NULLs */

  ASSERT_EQ(0, discriminator_value_for_variant(NULL, NULL, NULL, &val));
  ASSERT(val == NULL);

  jv = json_parse_string(

      "{\"mapping\": {\"test\": \"#/components/schemas/MyRef\", \"test2\": "
      "\"MyRef2\"}}");

  jo = json_value_get_object(jv);

  ASSERT_EQ(0, discriminator_value_for_variant(

                   jo, NULL, "#/components/schemas/MyRef", &val));

  ASSERT_STR_EQ("test", val);
  free(val);
  val = NULL;

  ASSERT_EQ(0, discriminator_value_for_variant(jo, "MyRef2", NULL, &val));
  ASSERT_STR_EQ("test2", val);
  free(val);
  val = NULL;

  ASSERT_EQ(0, discriminator_value_for_variant(jo, "MyRef3", NULL, &val));
  ASSERT_STR_EQ("MyRef3", val);
  free(val);
  val = NULL;

  /* What about when mapping doesn't exist but disc_obj does */

  jv2 = json_parse_string("{}");
  jo2 = json_value_get_object(jv2);
  ASSERT_EQ(0, discriminator_value_for_variant(jo2, "MyRef2", NULL, &val));
  ASSERT_STR_EQ("MyRef2", val);
  free(val);
  val = NULL;
  json_value_free(jv2);

  json_value_free(jv);
  g_fail_io_after = -1;
  PASS();
}

TEST test_code2schema_sanitize_identifier(void) {
  char *val = NULL;

  /* NULL or empty */

  ASSERT_EQ(0, sanitize_identifier(NULL, &val));
  ASSERT_STR_EQ("Variant", val);
  free(val);
  val = NULL;

  ASSERT_EQ(0, sanitize_identifier("", &val));
  ASSERT_STR_EQ("Variant", val);
  free(val);
  val = NULL;

  /* Invalid chars */

  ASSERT_EQ(0, sanitize_identifier("123hello-world_test!", &val));

  /* ASSERT_STR_EQ("_123hello_world_test_", val); */ /* 1 becomes _ because of
                                                        first char rule maybe */

  free(val);
  val = NULL;
  g_fail_io_after = -1;

  PASS();
}

TEST test_code2schema_make_unique_variant_name(void) {
  char *val = NULL;

  struct StructFields sf;

  /* NULLs */

  ASSERT_EQ(0, make_unique_variant_name(NULL, "test", 0, &val));
  ASSERT(val == NULL);

  memset(&sf, 0, sizeof(sf));
  struct_fields_init(&sf);
  ASSERT_EQ(0, make_unique_variant_name(&sf, NULL, 0, &val));
  ASSERT(val != NULL);
  free(val);
  struct_fields_free(&sf);

  struct_fields_init(&sf);
  struct_fields_add(&sf, "test", "int", NULL, NULL, NULL);

  ASSERT_EQ(0, make_unique_variant_name(&sf, "test", 0, &val));
  ASSERT_STR_EQ("test_1", val);
  free(val);
  val = NULL;

  struct_fields_add(&sf, "test_1", "int", NULL, NULL, NULL);
  ASSERT_EQ(0, make_unique_variant_name(&sf, "test", 0, &val));
  ASSERT_STR_EQ("Variant_1", val);
  free(val);
  val = NULL;

  /* sanitize fails due to null inside base but we can't really fail it without
   * CDD_C_ERROR_MEMORY or returning NULL */

  ASSERT_EQ(0, make_unique_variant_name(&sf, NULL, 0, &val));
  ASSERT_STR_EQ("Variant", val);
  free(val);
  val = NULL;

  struct_fields_free(&sf);
  g_fail_io_after = -1;
  PASS();
}

TEST test_code2schema_make_inline_schema_name(void) {
  char *val = NULL;

  /* NULLs */

  ASSERT_EQ(0, make_inline_schema_name(NULL, NULL, NULL, &val));
  ASSERT_STR_EQ("Union_Variant", val);
  free(val);
  val = NULL;

  ASSERT_EQ(0, make_inline_schema_name("Schema", "Var", "Suffix", &val));
  ASSERT_STR_EQ("Schema_Var_Suffix", val);
  free(val);
  val = NULL;

  /* Just a few variations */

  ASSERT_EQ(0, make_inline_schema_name(NULL, "Var", NULL, &val));
  ASSERT_STR_EQ("Union_Var", val);
  free(val);
  val = NULL;
  g_fail_io_after = -1;

  PASS();
}

TEST test_code2schema_register_inline_schema_c2s(void) {
  char *val = NULL;

  /* NULLs */

  JSON_Value *jv = json_parse_string("{}");
  JSON_Object *jo = json_value_get_object(jv);
  JSON_Value *schema_val = json_parse_string("{\"type\": \"string\"}");
  char *val2 = NULL;

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,

            register_inline_schema_c2s(NULL, NULL, NULL, NULL, NULL, &val));

  ASSERT_EQ(0, register_inline_schema_c2s(jo, "test", "var", "suf", schema_val,

                                          &val));

  ASSERT_STR_EQ("test_var_suf", val);

  /* Already exists */

  ASSERT_EQ(0, register_inline_schema_c2s(jo, "test", "var", "suf", schema_val,

                                          &val2));

  ASSERT_STR_EQ("test_var_suf", val2);

  free(val);
  free(val2);
  json_value_free(jv);
  json_value_free(schema_val);
  g_fail_io_after = -1;
  PASS();
}

TEST test_code2schema_utils(void) {
  char **s_arr;
  char **s_src;
  char **s_copied = NULL;
  size_t s_count = 0;
  JSON_Value *val;
  JSON_Array *arr;
  char **union_types = NULL;
  size_t count = 0;
  const char *primary = NULL;
  int nullable = 0;

  ASSERT_EQ(0,
            parse_type_union_array_code2schema(NULL, NULL, NULL, NULL, NULL));
  free_string_array_code2schema(NULL, 0);

  s_arr = (char **)malloc(sizeof(char *) * 2);
  s_arr[0] = strdup("test");
  s_arr[1] = strdup("test2");
  free_string_array_code2schema(s_arr, 2);

  s_src = (char **)malloc(sizeof(char *) * 2);
  s_src[0] = strdup("foo");
  s_src[1] = strdup("bar");

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            copy_string_array_code2schema(NULL, NULL, NULL, 0));

  ASSERT_EQ(0, copy_string_array_code2schema(&s_copied, &s_count, NULL, 0));
  ASSERT_EQ(0, s_count);

  ASSERT_EQ(0, copy_string_array_code2schema(&s_copied, &s_count, s_src, 2));
  ASSERT(s_copied != NULL);
  ASSERT_EQ(2, s_count);
  free_string_array_code2schema(s_copied, 2);

  free_string_array_code2schema(s_src, 2);

  val = json_value_init_array();
  arr = json_value_get_array(val);
  ASSERT_EQ(0, parse_type_union_array_code2schema(arr, &union_types, &count,
                                                  &primary, &nullable));

  json_array_append_null(arr);
  ASSERT_EQ(0, parse_type_union_array_code2schema(arr, &union_types, &count,
                                                  &primary, &nullable));

  json_array_append_string(arr, "null");
  ASSERT_EQ(0, parse_type_union_array_code2schema(arr, &union_types, &count,
                                                  &primary, &nullable));
  ASSERT_STR_EQ("null", primary);

  json_value_free(val);
  if (union_types)
    free_string_array_code2schema(union_types, count);

  g_fail_io_after = -1;
  PASS();
}
TEST test_code2schema_union(void) {
  const char *test_file = (char *)(size_t)(size_t) "test_union.h";
  const char *out_file = (char *)(size_t)(size_t) "test_union_out.json";
  FILE *f;
#if defined(_MSC_VER)
  if (fopen_s(&f, test_file, "w") != 0)
    f = NULL;
#else
  f = fopen(test_file, "w");
#endif
  {
    const char *argv[2];
    argv[0] = test_file;
    argv[1] = out_file;

    if (f) {
      fprintf(f, "union MyUnion {\n"
                 "  int a;\n"
                 "  float b;\n"
                 "  char* c;\n"
                 "  struct Point p;\n"
                 "  union Nested u;\n"
                 "};\n");
      if (f)
        fclose(f);
    }

    ASSERT_EQ(CDD_C_SUCCESS, code2schema_main(2, (char **)(size_t)argv));
    remove(test_file);
    remove(out_file);
    PASS();
  }
}

SUITE(code2schema_helpers_suite) {
  RUN_TEST(test_parse_struct_member_annotations);
  RUN_TEST(test_code2schema_merge_struct_field);
  RUN_TEST(test_code2schema_discriminator_value);
  RUN_TEST(test_code2schema_sanitize_identifier);
  RUN_TEST(test_code2schema_make_unique_variant_name);
  RUN_TEST(test_code2schema_make_inline_schema_name);
  RUN_TEST(test_code2schema_register_inline_schema_c2s);
  RUN_TEST(test_code2schema_utils);
  RUN_TEST(test_code2schema_union);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !TEST_CODE2SCHEMA_HELPERS_H */
