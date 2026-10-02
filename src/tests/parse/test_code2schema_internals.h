/**
 * @file test_code2schema_internals.h
 * @brief Unit tests for internal functions in code2schema.c.
 * @author Samuel Marks
 */

#ifndef TEST_CODE2SCHEMA_INTERNALS_H
#define TEST_CODE2SCHEMA_INTERNALS_H

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

TEST test_code2schema_c2s_read_line(void) {
  FILE *fp;
  char buf[128];
  int has_line = 0;
  cdd_c_error_t rc = 0;

  /* Invalid arguments */
  rc = c2s_read_line(NULL, buf, sizeof(buf), NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  rc = c2s_read_line(NULL, buf, sizeof(buf), &has_line);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  fp = tmpfile();
  ASSERT(fp != NULL);

  rc = c2s_read_line(fp, NULL, sizeof(buf), &has_line);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  rc = c2s_read_line(fp, buf, 0, &has_line);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  /* Empty file -> EOF */
  rc = c2s_read_line(fp, buf, sizeof(buf), &has_line);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, has_line);

  /* Write lines with various endings */
  fputs("line1\r\nline2\nline3\n", fp);
  rewind(fp);

  rc = c2s_read_line(fp, buf, sizeof(buf), &has_line);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, has_line);
  ASSERT_STR_EQ("line1", buf);

  rc = c2s_read_line(fp, buf, sizeof(buf), &has_line);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, has_line);
  ASSERT_STR_EQ("line2", buf);

  rc = c2s_read_line(fp, buf, sizeof(buf), &has_line);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, has_line);
  ASSERT_STR_EQ("line3", buf);

  rc = c2s_read_line(fp, buf, sizeof(buf), &has_line);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, has_line);

  fclose(fp);
  PASS();
}

/**
 * @brief Tests c2s_key_in_list for presence, absence, and edge cases.
 *
 * @return TEST_PASSED on success.
 */
TEST test_code2schema_c2s_key_in_list(void) {
  const char *list[4];
  int found = 0;
  cdd_c_error_t rc = 0;

  list[0] = "apple";
  list[1] = NULL;
  list[2] = "banana";
  list[3] = "cherry";

  /* Invalid argument: out_found NULL */
  rc = c2s_key_in_list("apple", list, 4, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  /* NULL key or NULL list */
  rc = c2s_key_in_list(NULL, list, 4, &found);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, found);

  rc = c2s_key_in_list("apple", NULL, 4, &found);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, found);

  /* Key present */
  rc = c2s_key_in_list("banana", list, 4, &found);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, found);

  /* Key absent */
  rc = c2s_key_in_list("orange", list, 4, &found);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, found);

  PASS();
}

/**
 * @brief Tests c2s_clone_json_value with normal values and OOM.
 *
 * @return TEST_PASSED on success.
 */
TEST test_code2schema_c2s_clone_json_value(void) {
  JSON_Value *val;
  JSON_Value *copy = NULL;
  cdd_c_error_t rc = 0;

  /* Invalid argument: NULL destination */
  rc = c2s_clone_json_value(NULL, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  /* NULL value returns NULL copy and SUCCESS */
  rc = c2s_clone_json_value(NULL, &copy);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(NULL, copy);

  /* Normal value clone */
  val = json_parse_string("{\"k\":\"v\",\"n\":123}");
  ASSERT(val != NULL);

  rc = c2s_clone_json_value(val, &copy);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(copy != NULL);
  ASSERT_STR_EQ("v", json_object_get_string(json_value_get_object(copy), "k"));
  json_value_free(copy);

  json_value_free(val);
  PASS();
}

/**
 * @brief Tests c2s_collect_schema_extras with normal, skipped, and empty
 * objects.
 *
 * @return TEST_PASSED on success.
 */
TEST test_code2schema_c2s_collect_schema_extras(void) {
  JSON_Value *val;
  JSON_Object *obj;
  char *extras = NULL;
  const char *skip[2];
  cdd_c_error_t rc = 0;

  skip[0] = "type";
  skip[1] = "properties";

  /* Invalid arguments */
  rc = c2s_collect_schema_extras(NULL, skip, 2, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  val = json_parse_string("{\"type\":\"object\",\"x-c-custom\":\"val\"}");
  ASSERT(val != NULL);
  obj = json_value_get_object(val);

  rc = c2s_collect_schema_extras(obj, skip, 2, &extras);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(extras != NULL);
  ASSERT(strstr(extras, "x-c-custom") != NULL);
  ASSERT(strstr(extras, "\"type\"") == NULL);
  C_CDD_FREE(extras);
  extras = NULL;

  /* Empty object (only skip keys) -> returns SUCCESS with NULL */
  json_value_free(val);
  val = json_parse_string("{\"type\":\"object\"}");
  obj = json_value_get_object(val);
  rc = c2s_collect_schema_extras(obj, skip, 1, &extras);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(NULL, extras);

  json_value_free(val);
  PASS();
}

/**
 * @brief Tests c2s_merge_schema_extras_object and
 * c2s_merge_schema_extras_strings.
 *
 * @return TEST_PASSED on success.
 */
TEST test_code2schema_merge_extras(void) {
  JSON_Value *val;
  JSON_Object *obj;
  char *dest = NULL;
  cdd_c_error_t rc = 0;

  /* NULL target or NULL extras */
  rc = c2s_merge_schema_extras_object(NULL, "{\"a\":1}");
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  val = json_value_init_object();
  obj = json_value_get_object(val);

  rc = c2s_merge_schema_extras_object(obj, NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = c2s_merge_schema_extras_object(obj, "");
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = c2s_merge_schema_extras_object(obj, "{\"custom_key\":true}");
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, json_object_get_boolean(obj, "custom_key"));

  /* Existing key should not be overwritten */
  rc = c2s_merge_schema_extras_object(obj, "{\"custom_key\":false}");
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, json_object_get_boolean(obj, "custom_key"));

  json_value_free(val);

  /* merge_schema_extras_strings */
  rc = c2s_merge_schema_extras_strings(NULL, "{\"k\":\"v\"}");
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = c2s_merge_schema_extras_strings(&dest, NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = c2s_merge_schema_extras_strings(&dest, "{\"a\":1}");
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(dest != NULL);
  ASSERT(strstr(dest, "\"a\"") != NULL);

  rc = c2s_merge_schema_extras_strings(&dest, "{\"b\":2}");
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(strstr(dest, "\"b\"") != NULL);

  C_CDD_FREE(dest);
  PASS();
}

/**
 * @brief Tests c2s_openapi_type_is_primitive with various types.
 *
 * @return TEST_PASSED on success.
 */
TEST test_code2schema_c2s_openapi_type_is_primitive(void) {
  int is_prim = 0;
  cdd_c_error_t rc = 0;

  /* Invalid argument */
  rc = c2s_openapi_type_is_primitive("integer", NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  /* NULL type */
  rc = c2s_openapi_type_is_primitive(NULL, &is_prim);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, is_prim);

  /* Primitives */
  rc = c2s_openapi_type_is_primitive("integer", &is_prim);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, is_prim);

  rc = c2s_openapi_type_is_primitive("number", &is_prim);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, is_prim);

  rc = c2s_openapi_type_is_primitive("string", &is_prim);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, is_prim);

  rc = c2s_openapi_type_is_primitive("boolean", &is_prim);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, is_prim);

  /* Non-primitives */
  rc = c2s_openapi_type_is_primitive("object", &is_prim);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, is_prim);

  rc = c2s_openapi_type_is_primitive("array", &is_prim);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, is_prim);

  PASS();
}

/**
 * @brief Tests c2s_strip_quotes for quoted, unquoted, and edge cases.
 *
 * @return TEST_PASSED on success.
 */
TEST test_code2schema_c2s_strip_quotes(void) {
  char buf[64];
  const char *out = NULL;
  cdd_c_error_t rc = 0;

  /* Invalid args */
  rc = c2s_strip_quotes(NULL, buf, sizeof(buf), &out);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(NULL, out);

  /* Unquoted string */
  rc = c2s_strip_quotes("hello", buf, sizeof(buf), &out);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("hello", out);

  /* Quoted string */
  rc = c2s_strip_quotes("\"quoted\"", buf, sizeof(buf), &out);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("quoted", out);

  /* Single quote char */
  rc = c2s_strip_quotes("\"", buf, sizeof(buf), &out);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("\"", out);

  /* Small buffer truncation */
  rc = c2s_strip_quotes("\"longer_string\"", buf, 4, &out);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("lon", out);

  PASS();
}

/**
 * @brief Tests c2s_parse_bool_default and c2s_parse_number_default.
 *
 * @return TEST_PASSED on success.
 */
TEST test_code2schema_parse_defaults(void) {
  int bval = -1;
  int has_val = 0;
  double nval = 0.0;
  cdd_c_error_t rc = 0;

  /* parse_bool_default NULL checks */
  rc = c2s_parse_bool_default("1", &bval, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  rc = c2s_parse_bool_default(NULL, &bval, &has_val);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, has_val);

  rc = c2s_parse_bool_default("1", &bval, &has_val);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, has_val);
  ASSERT_EQ(1, bval);

  rc = c2s_parse_bool_default("true", &bval, &has_val);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, has_val);
  ASSERT_EQ(1, bval);

  rc = c2s_parse_bool_default("0", &bval, &has_val);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, has_val);
  ASSERT_EQ(0, bval);

  rc = c2s_parse_bool_default("false", &bval, &has_val);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, has_val);
  ASSERT_EQ(0, bval);

  rc = c2s_parse_bool_default("invalid", &bval, &has_val);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, has_val);

  /* parse_number_default NULL checks */
  rc = c2s_parse_number_default("123", &nval, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  rc = c2s_parse_number_default(NULL, &nval, &has_val);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, has_val);

  rc = c2s_parse_number_default("42", &nval, &has_val);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, has_val);
  ASSERT_EQ(42.0, nval);

  rc = c2s_parse_number_default("3.14", &nval, &has_val);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, has_val);

  rc = c2s_parse_number_default("not_a_number", &nval, &has_val);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, has_val);

  PASS();
}

/**
 * @brief Tests c2s_detect_union_json_type for all type variants.
 *
 * @return TEST_PASSED on success.
 */
TEST test_code2schema_c2s_detect_union_json_type(void) {
  JSON_Value *val;
  JSON_Object *obj;
  enum UnionVariantJsonType jtype;
  cdd_c_error_t rc = 0;

  /* Invalid argument */
  rc = c2s_detect_union_json_type(NULL, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  /* NULL object -> UNKNOWN */
  rc = c2s_detect_union_json_type(NULL, &jtype);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(UNION_JSON_UNKNOWN, jtype);

  /* Test string enum */
  val = json_parse_string("{\"enum\":[\"A\",\"B\"]}");
  obj = json_value_get_object(val);
  rc = c2s_detect_union_json_type(obj, &jtype);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(UNION_JSON_STRING, jtype);
  json_value_free(val);

  /* Test object type */
  val = json_parse_string("{\"type\":\"object\"}");
  obj = json_value_get_object(val);
  rc = c2s_detect_union_json_type(obj, &jtype);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(UNION_JSON_OBJECT, jtype);
  json_value_free(val);

  /* Test string type */
  val = json_parse_string("{\"type\":\"string\"}");
  obj = json_value_get_object(val);
  rc = c2s_detect_union_json_type(obj, &jtype);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(UNION_JSON_STRING, jtype);
  json_value_free(val);

  /* Test integer type */
  val = json_parse_string("{\"type\":\"integer\"}");
  obj = json_value_get_object(val);
  rc = c2s_detect_union_json_type(obj, &jtype);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(UNION_JSON_INTEGER, jtype);
  json_value_free(val);

  /* Test number type */
  val = json_parse_string("{\"type\":\"number\"}");
  obj = json_value_get_object(val);
  rc = c2s_detect_union_json_type(obj, &jtype);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(UNION_JSON_NUMBER, jtype);
  json_value_free(val);

  /* Test boolean type */
  val = json_parse_string("{\"type\":\"boolean\"}");
  obj = json_value_get_object(val);
  rc = c2s_detect_union_json_type(obj, &jtype);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(UNION_JSON_BOOLEAN, jtype);
  json_value_free(val);

  /* Test array type */
  val = json_parse_string("{\"type\":\"array\"}");
  obj = json_value_get_object(val);
  rc = c2s_detect_union_json_type(obj, &jtype);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(UNION_JSON_ARRAY, jtype);
  json_value_free(val);

  /* Test null type */
  val = json_parse_string("{\"type\":\"null\"}");
  obj = json_value_get_object(val);
  rc = c2s_detect_union_json_type(obj, &jtype);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(UNION_JSON_NULL, jtype);
  json_value_free(val);

  /* Test schema with properties but no type */
  val = json_parse_string("{\"properties\":{\"id\":{\"type\":\"integer\"}}}");
  obj = json_value_get_object(val);
  rc = c2s_detect_union_json_type(obj, &jtype);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(UNION_JSON_OBJECT, jtype);
  json_value_free(val);

  /* Test schema with unknown type */
  val = json_parse_string("{\"type\":\"custom_unknown\"}");
  obj = json_value_get_object(val);
  rc = c2s_detect_union_json_type(obj, &jtype);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(UNION_JSON_UNKNOWN, jtype);
  json_value_free(val);

  PASS();
}

/**
 * @brief Tests c2s_collect_string_array and c2s_collect_property_names.
 *
 * @return TEST_PASSED on success.
 */
TEST test_code2schema_collect_arrays(void) {
  JSON_Value *val;
  JSON_Array *arr;
  JSON_Object *obj;
  char **strs = NULL;
  size_t count = 0;
  cdd_c_error_t rc = 0;

  /* Invalid args */
  rc = c2s_collect_string_array(NULL, NULL, &count);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  rc = c2s_collect_string_array(NULL, &strs, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  /* NULL arr returns SUCCESS with 0 count */
  rc = c2s_collect_string_array(NULL, &strs, &count);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, count);
  ASSERT_EQ(NULL, strs);

  /* Empty array */
  val = json_parse_string("[]");
  arr = json_value_get_array(val);
  rc = c2s_collect_string_array(arr, &strs, &count);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, count);
  json_value_free(val);

  /* Populated array */
  val = json_parse_string("[\"one\",\"two\",\"three\"]");
  arr = json_value_get_array(val);
  rc = c2s_collect_string_array(arr, &strs, &count);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(3, count);
  ASSERT_STR_EQ("one", strs[0]);
  ASSERT_STR_EQ("two", strs[1]);
  ASSERT_STR_EQ("three", strs[2]);
  free_string_array_code2schema(strs, count);
  json_value_free(val);

  /* collect_property_names */
  rc = c2s_collect_property_names(NULL, NULL, &count);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  rc = c2s_collect_property_names(NULL, &strs, &count);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, count);

  val = json_parse_string("{\"properties\":{\"first\":{},\"second\":{}}}");
  obj = json_value_get_object(val);
  rc = c2s_collect_property_names(obj, &strs, &count);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(2, count);
  free_string_array_code2schema(strs, count);
  json_value_free(val);

  PASS();
}

/**
 * @brief Tests c2s_union_array_items_supported.
 *
 * @return TEST_PASSED on success.
 */
TEST test_code2schema_c2s_union_array_items_supported(void) {
  JSON_Value *val;
  JSON_Object *obj;
  int supported = 0;
  cdd_c_error_t rc = 0;

  /* Invalid argument */
  rc = c2s_union_array_items_supported(NULL, NULL, 1, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  /* NULL schema_obj */
  rc = c2s_union_array_items_supported(NULL, NULL, 1, &supported);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, supported);

  /* Without items property */
  val = json_parse_string("{\"type\":\"array\"}");
  obj = json_value_get_object(val);
  rc = c2s_union_array_items_supported(obj, NULL, 1, &supported);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, supported);
  json_value_free(val);

  /* Items with primitive type */
  val = json_parse_string("{\"items\":{\"type\":\"string\"}}");
  obj = json_value_get_object(val);
  rc = c2s_union_array_items_supported(obj, NULL, 1, &supported);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, supported);
  json_value_free(val);

  /* Items with nested array (unsupported) */
  val = json_parse_string("{\"items\":{\"type\":\"array\"}}");
  obj = json_value_get_object(val);
  rc = c2s_union_array_items_supported(obj, NULL, 1, &supported);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, supported);
  json_value_free(val);

  /* Items with object and allow_inline = 0 */
  val = json_parse_string("{\"items\":{\"type\":\"object\"}}");
  obj = json_value_get_object(val);
  rc = c2s_union_array_items_supported(obj, NULL, 0, &supported);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, supported);
  json_value_free(val);

  /* Items with $ref */
  val =
      json_parse_string("{\"items\":{\"$ref\":\"#/components/schemas/Item\"}}");
  obj = json_value_get_object(val);
  rc = c2s_union_array_items_supported(obj, NULL, 1, &supported);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, supported);
  json_value_free(val);

  PASS();
}

/**
 * @brief Tests schema_object_is_string_enum, ref_points_to_string_enum, and
 * required_name_in_list.
 *
 * @return TEST_PASSED on success.
 */
TEST test_code2schema_enum_and_refs(void) {
  JSON_Value *root_val;
  JSON_Object *root;
  JSON_Value *val;
  JSON_Object *obj;
  const JSON_Array *enum_arr = NULL;
  int is_enum = 0;
  int is_req = 0;
  cdd_c_error_t rc = 0;

  /* schema_object_is_string_enum NULL check */
  rc = schema_object_is_string_enum(NULL, NULL, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  rc = schema_object_is_string_enum(NULL, NULL, &is_enum);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, is_enum);

  /* Non-enum schema */
  val = json_parse_string("{\"type\":\"integer\"}");
  obj = json_value_get_object(val);
  rc = schema_object_is_string_enum(obj, &enum_arr, &is_enum);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, is_enum);
  json_value_free(val);

  /* Empty enum array */
  val = json_parse_string("{\"enum\":[]}");
  obj = json_value_get_object(val);
  rc = schema_object_is_string_enum(obj, &enum_arr, &is_enum);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, is_enum);
  json_value_free(val);

  /* Integer enum (non-string type) */
  val = json_parse_string("{\"type\":\"integer\",\"enum\":[1,2]}");
  obj = json_value_get_object(val);
  rc = schema_object_is_string_enum(obj, &enum_arr, &is_enum);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, is_enum);
  json_value_free(val);

  /* Enum containing non-strings */
  val = json_parse_string("{\"enum\":[\"OK\", 123]}");
  obj = json_value_get_object(val);
  rc = schema_object_is_string_enum(obj, &enum_arr, &is_enum);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, is_enum);
  json_value_free(val);

  /* Valid string enum */
  val = json_parse_string(
      "{\"type\":\"string\",\"enum\":[\"active\",\"inactive\"]}");
  obj = json_value_get_object(val);
  rc = schema_object_is_string_enum(obj, &enum_arr, &is_enum);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, is_enum);
  ASSERT(enum_arr != NULL);

  /* ref_points_to_string_enum */
  root_val = json_parse_string(
      "{\"Status\":{\"type\":\"string\",\"enum\":[\"A\",\"B\"]},"
      "\"User\":{\"type\":\"object\"}}");
  root = json_value_get_object(root_val);

  rc = ref_points_to_string_enum(NULL, NULL, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  rc = ref_points_to_string_enum(root, "#/components/schemas/Status", &is_enum);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, is_enum);

  rc = ref_points_to_string_enum(root, "#/components/schemas/User", &is_enum);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, is_enum);

  rc = ref_points_to_string_enum(root, "#/components/schemas/NotFound",
                                 &is_enum);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, is_enum);

  json_value_free(root_val);
  json_value_free(val);

  /* required_name_in_list */
  val = json_parse_string("[\"id\",\"name\"]");
  enum_arr = json_value_get_array(val);

  rc = required_name_in_list(enum_arr, "id", NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  rc = required_name_in_list(NULL, "id", &is_req);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, is_req);

  rc = required_name_in_list(enum_arr, "id", &is_req);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, is_req);

  rc = required_name_in_list(enum_arr, "missing", &is_req);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, is_req);

  json_value_free(val);
  PASS();
}

/**
 * @brief Tests resolve_schema_ref_object and discriminator_value_for_variant.
 *
 * @return TEST_PASSED on success.
 */
TEST test_code2schema_ref_and_discriminator(void) {
  JSON_Value *root_val;
  JSON_Object *root;
  JSON_Object *resolved = NULL;
  JSON_Value *disc_val;
  JSON_Object *disc_obj;
  char *dval = NULL;
  cdd_c_error_t rc = 0;

  root_val = json_parse_string("{\"MySchema\":{\"type\":\"object\"}}");
  root = json_value_get_object(root_val);

  /* resolve_schema_ref_object */
  rc = resolve_schema_ref_object(NULL, NULL, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  rc = resolve_schema_ref_object(NULL, NULL, &resolved);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(NULL, resolved);

  rc = resolve_schema_ref_object(root, "#/components/schemas/MySchema",
                                 &resolved);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(resolved != NULL);

  rc = resolve_schema_ref_object(root, "#/components/schemas/Nonexistent",
                                 &resolved);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(NULL, resolved);

  json_value_free(root_val);

  /* discriminator_value_for_variant */
  disc_val =
      json_parse_string("{\"mapping\":{\"cat\":\"#/components/schemas/Cat\","
                        "\"dog\":\"Dog\"}}");
  disc_obj = json_value_get_object(disc_val);

  rc = discriminator_value_for_variant(NULL, NULL, NULL, &dval);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(NULL, dval);

  /* Match by ref */
  rc = discriminator_value_for_variant(disc_obj, "Cat",
                                       "#/components/schemas/Cat", &dval);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(dval != NULL);
  ASSERT_STR_EQ("cat", dval);
  C_CDD_FREE(dval);
  dval = NULL;

  /* Match by name */
  rc = discriminator_value_for_variant(disc_obj, "Dog", NULL, &dval);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(dval != NULL);
  ASSERT_STR_EQ("dog", dval);
  C_CDD_FREE(dval);
  dval = NULL;

  /* Fallback without match */
  rc = discriminator_value_for_variant(disc_obj, "Bird", NULL, &dval);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(dval != NULL);
  ASSERT_STR_EQ("Bird", dval);
  C_CDD_FREE(dval);
  dval = NULL;

  json_value_free(disc_val);
  PASS();
}

SUITE(code2schema_internals_suite) {
  RUN_TEST(test_code2schema_c2s_read_line);
  RUN_TEST(test_code2schema_c2s_key_in_list);
  RUN_TEST(test_code2schema_c2s_clone_json_value);
  RUN_TEST(test_code2schema_c2s_collect_schema_extras);
  RUN_TEST(test_code2schema_merge_extras);
  RUN_TEST(test_code2schema_c2s_openapi_type_is_primitive);
  RUN_TEST(test_code2schema_c2s_strip_quotes);
  RUN_TEST(test_code2schema_parse_defaults);
  RUN_TEST(test_code2schema_c2s_detect_union_json_type);
  RUN_TEST(test_code2schema_collect_arrays);
  RUN_TEST(test_code2schema_c2s_union_array_items_supported);
  RUN_TEST(test_code2schema_enum_and_refs);
  RUN_TEST(test_code2schema_ref_and_discriminator);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CODE2SCHEMA_INTERNALS_H */
