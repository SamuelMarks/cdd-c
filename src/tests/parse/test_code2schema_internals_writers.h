/**
 * @file test_code2schema_internals_writers.h
 * @brief Unit tests for constraint writers and schema generation in
 * code2schema.c.
 * @author Samuel Marks
 */

#ifndef TEST_CODE2SCHEMA_INTERNALS_WRITERS_H
#define TEST_CODE2SCHEMA_INTERNALS_WRITERS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "c_cdd/memory.h"
#include "c_cdd/safe_crt.h"
#include "cdd_c_error.h"
#include "classes/emit/struct.h"
#include "classes/parse/code2schema.h"
#include <greatest.h>
#include <parson.h>
/* clang-format on */

extern C_CDD_EXPORT volatile int g_c2s_helper_fail;
extern C_CDD_EXPORT volatile int g_cdd_fail_c2s_collect_schema_extras;
extern C_CDD_EXPORT volatile int g_cdd_fail_json_set_value;
extern C_CDD_EXPORT int g_cdd_fail_json_serialize;
extern C_CDD_EXPORT volatile int g_cdd_fail_struct_fields_get;
extern C_CDD_EXPORT volatile int g_cdd_fail_struct_fields_add;
extern C_CDD_EXPORT int g_struct_fields_init_fail;
extern C_CDD_EXPORT int g_enum_members_init_fail;
extern C_CDD_EXPORT int g_enum_members_add_strdup_fail;

/**
 * @brief Tests c2s_write_default_value, constraints, and c2s_write_type_union.
 *
 * @return TEST_PASSED on success.
 */
TEST test_code2schema_constraint_writers(void) {
  JSON_Value *val = json_value_init_object();
  JSON_Object *obj = json_value_get_object(val);
  struct StructField f;
  char u_str[16];
  char u_null[16];
  char *union_types[2];
  cdd_c_error_t rc;

  CDD_STRCPY(u_str, sizeof(u_str), "string");
  CDD_STRCPY(u_null, sizeof(u_null), "null");
  union_types[0] = u_str;
  union_types[1] = u_null;

  memset(&f, 0, sizeof(f));

  /* Invalid arguments */
  rc = c2s_write_default_value(NULL, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = c2s_write_numeric_constraints(NULL, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = c2s_write_string_constraints(NULL, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = c2s_write_array_constraints(NULL, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  /* nullptr default */
  CDD_STRCPY(f.type, sizeof(f.type), "string");
  CDD_STRCPY(f.default_val, sizeof(f.default_val), "nullptr");
  rc = c2s_write_default_value(obj, &f);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(json_object_has_value_of_type(obj, "default", JSONNull));

  /* boolean default */
  CDD_STRCPY(f.type, sizeof(f.type), "boolean");
  CDD_STRCPY(f.default_val, sizeof(f.default_val), "true");
  rc = c2s_write_default_value(obj, &f);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, json_object_get_boolean(obj, "default"));

  /* numeric constraints: min & max */
  CDD_STRCPY(f.type, sizeof(f.type), "integer");
  f.has_min = 1;
  f.min_val = 10;
  f.exclusive_min = 1;
  f.has_max = 1;
  f.max_val = 100;
  f.exclusive_max = 0;
  rc = c2s_write_numeric_constraints(obj, &f);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(10, json_object_get_number(obj, "exclusiveMinimum"));
  ASSERT_EQ(100, json_object_get_number(obj, "maximum"));

  /* string constraints */
  CDD_STRCPY(f.type, sizeof(f.type), "string");
  f.has_min_len = 1;
  f.min_len = 5;
  f.has_max_len = 1;
  f.max_len = 25;
  CDD_STRCPY(f.pattern, sizeof(f.pattern), "^[A-Z]+$");
  rc = c2s_write_string_constraints(obj, &f);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(5, json_object_get_number(obj, "minLength"));
  ASSERT_EQ(25, json_object_get_number(obj, "maxLength"));
  ASSERT_STR_EQ("^[A-Z]+$", json_object_get_string(obj, "pattern"));

  /* array constraints */
  CDD_STRCPY(f.type, sizeof(f.type), "array");
  f.has_min_items = 1;
  f.min_items = 2;
  f.has_max_items = 1;
  f.max_items = 10;
  f.unique_items = 1;
  rc = c2s_write_array_constraints(obj, &f);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(2, json_object_get_number(obj, "minItems"));
  ASSERT_EQ(10, json_object_get_number(obj, "maxItems"));
  ASSERT_EQ(1, json_object_get_boolean(obj, "uniqueItems"));

  /* write_type_union */
  rc = c2s_write_type_union(NULL, "string", NULL, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = c2s_write_type_union(obj, "string", union_types, 2);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(json_object_get_array(obj, "type") != NULL);

  rc = c2s_write_type_union(obj, "boolean", NULL, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("boolean", json_object_get_string(obj, "type"));

  json_value_free(val);
  PASS();
}

/**
 * @brief Tests c2s_collapse_arrays on struct fields.
 *
 * @return TEST_PASSED on success.
 */
TEST test_code2schema_c2s_collapse_arrays(void) {
  struct StructFields sf;
  cdd_c_error_t rc;

  /* Invalid argument */
  rc = c2s_collapse_arrays(NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  rc = struct_fields_init(&sf);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* Add n_items and items (already array type) */
  rc = struct_fields_add(&sf, "n_items", "size_t", NULL, NULL, NULL);
  ASSERT_EQ(0, rc);
  rc = struct_fields_add(&sf, "items", "array", "Item", NULL, NULL);
  ASSERT_EQ(0, rc);

  rc = c2s_collapse_arrays(&sf);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* Add n_data and data of non-array type (gets converted to array) */
  rc = struct_fields_add(&sf, "n_data", "size_t", NULL, NULL, NULL);
  ASSERT_EQ(0, rc);
  rc = struct_fields_add(&sf, "data", "char*", NULL, NULL, NULL);
  ASSERT_EQ(0, rc);

  rc = c2s_collapse_arrays(&sf);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  struct_fields_free(&sf);
  PASS();
}

/**
 * @brief Tests merge_struct_field and merge_struct_fields branches.
 *
 * @return TEST_PASSED on success.
 */
TEST test_code2schema_merge_struct_fields_branches(void) {
  struct StructFields src_sf;
  struct StructFields dst_sf;
  struct StructField src_f;
  struct StructField dst_f;
  cdd_c_error_t rc;

  /* Invalid argument NULL checks */
  rc = merge_struct_field(NULL, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  rc = merge_struct_fields(NULL, NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* merge_struct_field constraint overwriting */
  memset(&src_f, 0, sizeof(src_f));
  memset(&dst_f, 0, sizeof(dst_f));

  CDD_STRCPY(src_f.default_val, sizeof(src_f.default_val), "default_test");
  src_f.required = 1;
  src_f.has_min = 1;
  src_f.min_val = 50;
  src_f.exclusive_min = 1;
  src_f.has_max = 1;
  src_f.max_val = 200;
  src_f.exclusive_max = 1;
  src_f.has_min_len = 1;
  src_f.min_len = 10;
  src_f.has_max_len = 1;
  src_f.max_len = 50;
  src_f.has_min_items = 1;
  src_f.min_items = 3;
  src_f.has_max_items = 1;
  src_f.max_items = 15;
  src_f.unique_items = 1;
  src_f.is_flexible_array = 1;
  CDD_STRCPY(src_f.pattern, sizeof(src_f.pattern), "^[a-z]+$");
  CDD_STRCPY(src_f.bit_width, sizeof(src_f.bit_width), "4");

  rc = merge_struct_field(&dst_f, &src_f);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("default_test", dst_f.default_val);
  ASSERT_EQ(1, dst_f.required);
  ASSERT_EQ(1, dst_f.has_min);
  ASSERT_EQ(50, dst_f.min_val);
  ASSERT_EQ(1, dst_f.exclusive_min);
  ASSERT_EQ(1, dst_f.has_max);
  ASSERT_EQ(200, dst_f.max_val);
  ASSERT_EQ(1, dst_f.exclusive_max);
  ASSERT_EQ(10, dst_f.min_len);
  ASSERT_EQ(50, dst_f.max_len);
  ASSERT_EQ(3, dst_f.min_items);
  ASSERT_EQ(15, dst_f.max_items);
  ASSERT_EQ(1, dst_f.unique_items);
  ASSERT_EQ(1, dst_f.is_flexible_array);
  ASSERT_STR_EQ("^[a-z]+$", dst_f.pattern);
  ASSERT_STR_EQ("4", dst_f.bit_width);

  /* merge_struct_fields container */
  rc = struct_fields_init(&src_sf);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = struct_fields_init(&dst_sf);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = struct_fields_add(&src_sf, "common_field", "integer", NULL, "0", NULL);
  ASSERT_EQ(0, rc);
  rc = struct_fields_add(&dst_sf, "common_field", "integer", NULL, NULL, NULL);
  ASSERT_EQ(0, rc);

  rc = struct_fields_add(&src_sf, "new_field", "string", NULL, "\"test\"", "8");
  ASSERT_EQ(0, rc);

  rc = merge_struct_fields(&dst_sf, &src_sf);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(2, dst_sf.size);
  ASSERT_STR_EQ("0", dst_sf.fields[0].default_val);
  ASSERT_STR_EQ("new_field", dst_sf.fields[1].name);

  struct_fields_free(&src_sf);
  struct_fields_free(&dst_sf);
  PASS();
}

/**
 * @brief Tests apply_allof_to_struct_fields,
 * apply_union_to_struct_fields_fallback, and apply_union_to_struct_fields_ex.
 *
 * @return TEST_PASSED on success.
 */
TEST test_code2schema_allof_and_unions(void) {
  JSON_Value *root_val;
  JSON_Object *root;
  JSON_Value *allof_val;
  JSON_Array *allof_arr;
  JSON_Value *union_val;
  JSON_Array *union_arr;
  struct StructFields sf;
  cdd_c_error_t rc;

  /* apply_allof_to_struct_fields NULL check */
  rc = apply_allof_to_struct_fields(NULL, NULL, NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  root_val = json_parse_string("{\"Base\":{\"type\":\"object\",\"properties\":{"
                               "\"id\":{\"type\":\"integer\"}}}}");
  root = json_value_get_object(root_val);

  allof_val = json_parse_string(
      "[{\"$ref\":\"#/components/schemas/"
      "Base\"},{\"properties\":{\"name\":{\"type\":\"string\"}}}]");
  allof_arr = json_value_get_array(allof_val);

  rc = struct_fields_init(&sf);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = apply_allof_to_struct_fields(allof_arr, &sf, root);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(2, sf.size);

  struct_fields_free(&sf);
  json_value_free(allof_val);

  /* apply_union_to_struct_fields_fallback */
  rc = apply_union_to_struct_fields_fallback(NULL, NULL, NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  union_val =
      json_parse_string("[{\"type\":\"string\"},{\"type\":\"integer\"}]");
  union_arr = json_value_get_array(union_val);

  rc = struct_fields_init(&sf);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = apply_union_to_struct_fields_fallback(union_arr, &sf, root);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  struct_fields_free(&sf);

  /* apply_union_to_struct_fields_ex */
  rc = apply_union_to_struct_fields_ex(NULL, NULL, NULL, NULL, 0, NULL, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = struct_fields_init(&sf);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = apply_union_to_struct_fields_ex(union_arr, &sf, root, "TestUnion", 1,
                                       NULL, 1);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, sf.is_union);
  ASSERT_EQ(1, sf.union_is_anyof);
  ASSERT_EQ(2, sf.n_union_variants);

  struct_fields_free(&sf);
  json_value_free(union_val);
  json_value_free(root_val);
  PASS();
}

/**
 * @brief Tests parse_struct_member_line with comments, bitfields, pointers, and
 * FAM.
 *
 * @return TEST_PASSED on success.
 */
TEST test_code2schema_parse_struct_member_line_internals(void) {
  struct StructFields sf;
  cdd_c_error_t rc;

  /* Invalid arguments */
  rc = parse_struct_member_line(NULL, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  rc = struct_fields_init(&sf);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* Line with @shard_key and @shard_hash */
  rc = parse_struct_member_line("int shard_id; /* @shard_key @shard_hash */",
                                &sf);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(sf.fields[sf.size - 1].schema_extra_json != NULL);

  /* Line with @slow_query_warn(250) and @track_telemetry */
  rc = parse_struct_member_line(
      "long query_time; /* @slow_query_warn(250) @track_telemetry */", &sf);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* Bitfields */
  rc = parse_struct_member_line("unsigned int flag : 1;", &sf);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("1", sf.fields[sf.size - 1].bit_width);

  rc = parse_struct_member_line("unsigned int mask : (8);", &sf);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("(8)", sf.fields[sf.size - 1].bit_width);

  /* Pointers */
  rc = parse_struct_member_line("char *title;", &sf);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = parse_struct_member_line("char* body;", &sf);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* FAM (Flexible array member) */
  rc = parse_struct_member_line("int numbers[];", &sf);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, sf.fields[sf.size - 1].is_flexible_array);

  /* Invalid line without space or asterisk */
  rc = parse_struct_member_line("invalid_line_without_space_or_star;", &sf);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  struct_fields_free(&sf);
  PASS();
}

/**
 * @brief Tests write_struct_to_json_schema with enum and complex struct models.
 *
 * @return TEST_PASSED on success.
 */
TEST test_code2schema_write_struct_to_json_schema_internals(void) {
  JSON_Value *schemas_val = json_value_init_object();
  JSON_Object *schemas_obj = json_value_get_object(schemas_val);
  struct StructFields sf;
  cdd_c_error_t rc;

  /* Invalid argument NULL checks */
  rc = write_struct_to_json_schema(NULL, NULL, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  /* Enum serialization */
  rc = struct_fields_init(&sf);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  sf.is_enum = 1;
  rc = enum_members_init(&sf.enum_members);
  ASSERT_EQ(0, rc);
  rc = enum_members_add(&sf.enum_members, "ACTIVE");
  ASSERT_EQ(0, rc);
  rc = enum_members_add(&sf.enum_members, "PENDING");
  ASSERT_EQ(0, rc);

  rc = write_struct_to_json_schema(schemas_obj, "UserStatus", &sf);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(json_object_get_object(schemas_obj, "UserStatus") != NULL);

  struct_fields_free(&sf);

  /* Struct serialization with bitwidth, format, deprecated, array ref */
  rc = struct_fields_init(&sf);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = struct_fields_add(&sf, "tags", "array", "string", NULL, NULL);
  ASSERT_EQ(0, rc);
  rc = struct_fields_add(&sf, "user_ref", "object", "User", NULL, NULL);
  ASSERT_EQ(0, rc);
  CDD_STRCPY(sf.fields[1].bit_width, sizeof(sf.fields[1].bit_width), "16");
  CDD_STRCPY(sf.fields[1].description, sizeof(sf.fields[1].description),
             "User reference");
  CDD_STRCPY(sf.fields[1].format, sizeof(sf.fields[1].format), "uuid");
  sf.fields[1].deprecated_set = 1;
  sf.fields[1].deprecated = 1;
  sf.fields[1].read_only_set = 1;
  sf.fields[1].read_only = 1;
  sf.fields[1].write_only_set = 1;
  sf.fields[1].write_only = 0;
  sf.fields[1].required = 1;

  rc = write_struct_to_json_schema(schemas_obj, "Container", &sf);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(json_object_get_object(schemas_obj, "Container") != NULL);

  struct_fields_free(&sf);
  json_value_free(schemas_val);
  PASS();
}

/**
 * @brief Suite definition grouping all code2schema internals unit tests.
 */

/**
 * @brief Tests sanitize_identifier edge cases including empty and NULL.
 *
 * @return TEST_PASSED on success.
 */
TEST test_code2schema_sanitize_identifier_edge_cases(void) {
  char *out = NULL;
  cdd_c_error_t rc;

  /* NULL or empty string -> "Variant" */
  rc = sanitize_identifier(NULL, &out);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("Variant", out);
  C_CDD_FREE(out);
  out = NULL;

  rc = sanitize_identifier("", &out);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("Variant", out);
  C_CDD_FREE(out);
  out = NULL;

  /* Special characters become _ */
  rc = sanitize_identifier("my-var.name@123", &out);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("my_var_name_123", out);
  C_CDD_FREE(out);
  out = NULL;

  PASS();
}

/**
 * @brief Tests make_unique_variant_name fallbacks when sanitized and numbered
 * names conflict.
 *
 * @return TEST_PASSED on success.
 */
TEST test_code2schema_make_unique_variant_name_fallbacks(void) {
  struct StructFields sf;
  char *out = NULL;
  cdd_c_error_t rc;

  /* NULL dest */
  rc = make_unique_variant_name(NULL, "Test", 0, &out);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(NULL, out);

  rc = struct_fields_init(&sf);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* Add "Item" and "Item_1" to force fallback to Variant_1 */
  rc = struct_fields_add(&sf, "Item", "integer", NULL, NULL, NULL);
  ASSERT_EQ(0, rc);
  rc = struct_fields_add(&sf, "Item_1", "integer", NULL, NULL, NULL);
  ASSERT_EQ(0, rc);

  rc = make_unique_variant_name(&sf, "Item", 0, &out);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("Variant_1", out);
  C_CDD_FREE(out);
  out = NULL;

  struct_fields_free(&sf);
  PASS();
}

/**
 * @brief Tests register_inline_schema_c2s invalid arguments and duplicate
 * registration.
 *
 * @return TEST_PASSED on success.
 */
TEST test_code2schema_register_inline_schema_branches(void) {
  JSON_Value *root_val = json_value_init_object();
  JSON_Object *root = json_value_get_object(root_val);
  JSON_Value *schema_val = json_parse_string("{\"type\":\"string\"}");
  char *name1 = NULL;
  char *name2 = NULL;
  cdd_c_error_t rc;

  /* Invalid arguments */
  rc = register_inline_schema_c2s(NULL, "Schema", "Variant", NULL, schema_val,
                                  &name1);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  rc =
      register_inline_schema_c2s(root, "Schema", "Variant", NULL, NULL, &name1);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  rc = register_inline_schema_c2s(root, "Schema", "Variant", NULL, schema_val,
                                  NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  /* First registration */
  rc = register_inline_schema_c2s(root, "Schema", "Variant", "Suffix",
                                  schema_val, &name1);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(name1 != NULL);
  ASSERT(json_object_has_value(root, name1));

  /* Second registration of the same name (branch: already exists in root) */
  rc = register_inline_schema_c2s(root, "Schema", "Variant", "Suffix",
                                  schema_val, &name2);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ(name1, name2);

  C_CDD_FREE(name1);
  C_CDD_FREE(name2);
  json_value_free(schema_val);
  json_value_free(root_val);
  PASS();
}

/**
 * @brief Tests apply_allof_to_struct_fields and fallback with unresolved refs
 * and non-objects.
 *
 * @return TEST_PASSED on success.
 */
TEST test_code2schema_allof_and_fallback_branches(void) {
  JSON_Value *root_val = json_value_init_object();
  JSON_Object *root = json_value_get_object(root_val);
  JSON_Value *arr_val;
  JSON_Array *arr;
  struct StructFields sf;
  cdd_c_error_t rc;

  rc = struct_fields_init(&sf);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* Array with non-object (integer 42) and unresolved ref */
  arr_val =
      json_parse_string("[42, {\"$ref\":\"#/components/schemas/Unresolved\"}]");
  arr = json_value_get_array(arr_val);

  rc = apply_allof_to_struct_fields(arr, &sf, root);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = apply_union_to_struct_fields_fallback(arr, &sf, root);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* Dest already has size > 0 in fallback */
  rc = struct_fields_add(&sf, "existing", "string", NULL, NULL, NULL);
  ASSERT_EQ(0, rc);

  rc = apply_union_to_struct_fields_fallback(arr, &sf, root);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  struct_fields_free(&sf);
  json_value_free(arr_val);
  json_value_free(root_val);
  PASS();
}

/**
 * @brief Tests c2s_collect_property_names with empty properties object.
 *
 * @return TEST_PASSED on success.
 */
TEST test_code2schema_collect_property_names_empty(void) {
  JSON_Value *val = json_parse_string("{\"properties\":{}}");
  JSON_Object *obj = json_value_get_object(val);
  char **names = NULL;
  size_t count = 0;
  cdd_c_error_t rc;

  rc = c2s_collect_property_names(obj, &names, &count);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, count);
  ASSERT_EQ(NULL, names);

  json_value_free(val);
  PASS();
}

/**
 * @brief Tests parse_struct_member_line with long type names and fixed array
 * types.
 *
 * @return TEST_PASSED on success.
 */
TEST test_code2schema_parse_struct_member_line_extended_types(void) {
  struct StructFields sf;
  cdd_c_error_t rc;

  rc = struct_fields_init(&sf);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* Long type name > 63 chars */
  rc = parse_struct_member_line("struct "
                                "ThisIsAnExtremelyLongTypeNameThatExceedsSixtyT"
                                "hreeCharactersInLengthHere my_field;",
                                &sf);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* Fixed size array: int arr[10]; */
  rc = parse_struct_member_line("int arr[10];", &sf);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  struct_fields_free(&sf);
  PASS();
}

/**
 * @brief Tests merge_struct_field exclusive bounds and union copying.
 *
 * @return TEST_PASSED on success.
 */
TEST test_code2schema_merge_struct_field_bounds_and_unions(void) {
  struct StructField dest;
  struct StructField src;
  char *u1[1];
  char *u2[1];
  cdd_c_error_t rc;

  u1[0] = (char *)(size_t) "string";
  u2[0] = (char *)(size_t) "integer";

  memset(&dest, 0, sizeof(dest));
  memset(&src, 0, sizeof(src));

  /* Test equal min_val but src is exclusive_min */
  dest.has_min = 1;
  dest.min_val = 100.0;
  dest.exclusive_min = 0;

  src.has_min = 1;
  src.min_val = 100.0;
  src.exclusive_min = 1;

  /* Test equal max_val but src is exclusive_max */
  dest.has_max = 1;
  dest.max_val = 500.0;
  dest.exclusive_max = 0;

  src.has_max = 1;
  src.max_val = 500.0;
  src.exclusive_max = 1;

  /* Type unions */
  src.type_union = u1;
  src.n_type_union = 1;
  src.items_type_union = u2;
  src.n_items_type_union = 1;

  rc = merge_struct_field(&dest, &src);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, dest.exclusive_min);
  ASSERT_EQ(1, dest.exclusive_max);
  ASSERT(dest.type_union != NULL);
  ASSERT(dest.items_type_union != NULL);

  free_string_array_code2schema(dest.type_union, dest.n_type_union);
  free_string_array_code2schema(dest.items_type_union, dest.n_items_type_union);
  PASS();
}

/**
 * @brief Tests discriminator_value_for_variant with ref_name match and
 * fallback.
 *
 * @return TEST_PASSED on success.
 */
TEST test_code2schema_discriminator_more_branches(void) {
  JSON_Value *disc_val =
      json_parse_string("{\"mapping\":{\"dog\":\"DogClass\"}}");
  JSON_Object *disc_obj = json_value_get_object(disc_val);
  char *out = NULL;
  cdd_c_error_t rc;

  /* Match by ref_name (after last slash) */
  rc = discriminator_value_for_variant(disc_obj, NULL,
                                       "#/components/schemas/DogClass", &out);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(out != NULL);
  ASSERT_STR_EQ("dog", out);
  C_CDD_FREE(out);
  out = NULL;

  /* Fallback with ref_name when schema_name is NULL */
  rc = discriminator_value_for_variant(NULL, NULL,
                                       "#/components/schemas/CatClass", &out);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(out != NULL);
  ASSERT_STR_EQ("CatClass", out);
  C_CDD_FREE(out);
  out = NULL;

  /* Fallback with schema_name */
  rc = discriminator_value_for_variant(NULL, "FoxClass", NULL, &out);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(out != NULL);
  ASSERT_STR_EQ("FoxClass", out);
  C_CDD_FREE(out);
  out = NULL;

  json_value_free(disc_val);
  PASS();
}

/**
 * @brief Tests c2s_parse_union_and_write with pointer members and whitespace
 * lines.
 *
 * @return TEST_PASSED on success.
 */
TEST test_code2schema_c2s_parse_union_and_write_internals(void) {
  FILE *fp = tmpfile();
  JSON_Value *val = json_value_init_object();
  JSON_Object *obj = json_value_get_object(val);
  cdd_c_error_t rc;

  ASSERT(fp != NULL);
  fputs("\n   \n  int id;\n  char str_val;\n  float num_val;\n  char* name;\n  "
        "struct Node* next;\n}\n",
        fp);
  rewind(fp);

  rc = c2s_parse_union_and_write(fp, obj, "MyUnion");
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(json_object_has_value(obj, "MyUnion"));

  fclose(fp);
  json_value_free(val);
  PASS();
}

SUITE(code2schema_internals_writers_suite) {
  RUN_TEST(test_code2schema_constraint_writers);
  RUN_TEST(test_code2schema_c2s_collapse_arrays);
  RUN_TEST(test_code2schema_merge_struct_fields_branches);
  RUN_TEST(test_code2schema_allof_and_unions);
  RUN_TEST(test_code2schema_parse_struct_member_line_internals);
  RUN_TEST(test_code2schema_write_struct_to_json_schema_internals);
  RUN_TEST(test_code2schema_sanitize_identifier_edge_cases);
  RUN_TEST(test_code2schema_make_unique_variant_name_fallbacks);
  RUN_TEST(test_code2schema_register_inline_schema_branches);
  RUN_TEST(test_code2schema_allof_and_fallback_branches);
  RUN_TEST(test_code2schema_collect_property_names_empty);
  RUN_TEST(test_code2schema_parse_struct_member_line_extended_types);
  RUN_TEST(test_code2schema_merge_struct_field_bounds_and_unions);
  RUN_TEST(test_code2schema_discriminator_more_branches);
  RUN_TEST(test_code2schema_c2s_parse_union_and_write_internals);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CODE2SCHEMA_INTERNALS_WRITERS_H */
