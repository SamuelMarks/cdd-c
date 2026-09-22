/**
 * @file test_code2schema_internals.h
 * @brief Unit tests for internal functions and branch coverage in
 * code2schema.c.
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
extern C_CDD_EXPORT volatile int g_cdd_fail_c2s_collect_schema_extras;
extern C_CDD_EXPORT volatile int g_cdd_fail_json_set_value;
extern C_CDD_EXPORT int g_cdd_fail_json_serialize;
extern C_CDD_EXPORT volatile int g_cdd_fail_struct_fields_get;
extern C_CDD_EXPORT volatile int g_cdd_fail_struct_fields_add;
extern C_CDD_EXPORT int g_struct_fields_init_fail;

/**
 * @brief Tests c2s_read_line with various valid and invalid arguments.
 *
 * @return TEST_PASSED on success.
 */
TEST test_code2schema_c2s_read_line(void) {
  FILE *fp;
  char buf[128];
  int has_line = 0;
  cdd_c_error_t rc;

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
  cdd_c_error_t rc;

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
  cdd_c_error_t rc;

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
  cdd_c_error_t rc;

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
  cdd_c_error_t rc;

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
  cdd_c_error_t rc;

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
  cdd_c_error_t rc;

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
  cdd_c_error_t rc;

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
  cdd_c_error_t rc;

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
  cdd_c_error_t rc;

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
  cdd_c_error_t rc;

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
  cdd_c_error_t rc;

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
  cdd_c_error_t rc;

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

/**
 * @brief Tests c2s_json_object_to_struct_fields_internal comprehensively.
 *
 * @return TEST_PASSED on success.
 */
TEST test_code2schema_json_object_to_struct_fields_full(void) {
  JSON_Value *root_val;
  JSON_Object *root;
  JSON_Value *schema_val;
  JSON_Object *schema_obj;
  struct StructFields sf;
  cdd_c_error_t rc;

  root_val = json_parse_string(
      "{\"StatusEnum\":{\"type\":\"string\",\"enum\":[\"OK\",\"FAIL\"]},"
      "\"SubObj\":{\"type\":\"object\",\"properties\":{\"tag\":{\"type\":"
      "\"string\"}}}}");
  root = json_value_get_object(root_val);

  schema_val = json_value_init_object();
  schema_obj = json_value_get_object(schema_val);
  json_object_set_string(schema_obj, "type", "object");
  {
    JSON_Value *req_v = json_parse_string("[\"field_req\"]");
    json_object_set_value(schema_obj, "required", req_v);
  }
  {
    JSON_Value *p1 = json_parse_string(
        "{\"field_req\":{\"type\":\"string\"},"
        "\"field_int\":{\"type\":\"integer\",\"default\":42,\"minimum\":0,"
        "\"exclusiveMinimum\":false,\"maximum\":100,\"exclusiveMaximum\":true},"
        "\"field_float\":{\"type\":\"number\",\"default\":3.14,\"minimum\":0.5,"
        "\"maximum\":99.9},"
        "\"field_bool\":{\"type\":\"boolean\",\"default\":true}}");
    JSON_Value *p2 = json_parse_string(
        "{\"field_str\":{\"type\":\"string\",\"minLength\":1,\"maxLength\":50,"
        "\"pattern\":\"^[a-z]+$\",\"format\":\"email\",\"deprecated\":true,"
        "\"readOnly\":true,\"writeOnly\":false},"
        "\"field_enum_ref\":{\"$ref\":\"#/components/schemas/StatusEnum\"},"
        "\"field_obj_ref\":{\"$ref\":\"#/components/schemas/SubObj\"}}");
    JSON_Value *p3 = json_parse_string(
        "{\"field_arr_enum\":{\"type\":\"array\",\"items\":{\"$ref\":\"#/"
        "components/schemas/StatusEnum\"}},"
        "\"field_arr_union\":{\"type\":\"array\",\"items\":{\"type\":["
        "\"string\",\"null\"]}},"
        "\"field_arr_obj\":{\"type\":\"array\",\"items\":{\"type\":\"object\"},"
        "\"minItems\":1,\"maxItems\":5,\"uniqueItems\":true}}");
    JSON_Object *props_obj;
    JSON_Value *props_val = json_value_init_object();
    props_obj = json_value_get_object(props_val);
    c2s_merge_schema_extras_object(props_obj, json_serialize_to_string(p1));
    c2s_merge_schema_extras_object(props_obj, json_serialize_to_string(p2));
    c2s_merge_schema_extras_object(props_obj, json_serialize_to_string(p3));
    json_object_set_value(schema_obj, "properties", props_val);
    json_value_free(p1);
    json_value_free(p2);
    json_value_free(p3);
  }

  rc = struct_fields_init(&sf);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = c2s_json_object_to_struct_fields_internal(schema_obj, &sf, root,
                                                 "FullModel", 1);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(sf.size >= 10);

  struct_fields_free(&sf);
  json_value_free(schema_val);
  json_value_free(root_val);
  PASS();
}

/**
 * @brief Tests apply_union_to_struct_fields_ex with discriminator and inline
 * promotion.
 *
 * @return TEST_PASSED on success.
 */
TEST test_code2schema_apply_union_ex_full(void) {
  JSON_Value *root_val;
  JSON_Object *root;
  JSON_Value *union_val;
  JSON_Array *union_arr;
  JSON_Value *schema_val;
  JSON_Object *schema_obj;
  struct StructFields sf;
  cdd_c_error_t rc;

  root_val = json_value_init_object();
  root = json_value_get_object(root_val);

  schema_val =
      json_parse_string("{\"discriminator\":{\"propertyName\":\"kind\","
                        "\"mapping\":{\"cat\":\"CatModel\"}}}");
  schema_obj = json_value_get_object(schema_val);

  union_val = json_parse_string(
      "["
      "  "
      "{\"title\":\"CatModel\",\"type\":\"object\",\"properties\":{\"meow\":{"
      "\"type\":\"boolean\"}},\"required\":[\"meow\"]},"
      "  {\"type\":\"array\",\"items\":{\"type\":\"string\"}}"
      "]");
  union_arr = json_value_get_array(union_val);

  rc = struct_fields_init(&sf);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = apply_union_to_struct_fields_ex(union_arr, &sf, root, "PetUnion", 0,
                                       schema_obj, 1);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, sf.is_union);
  ASSERT_EQ(2, sf.n_union_variants);
  ASSERT_STR_EQ("kind", sf.union_discriminator);

  struct_fields_free(&sf);
  json_value_free(schema_val);
  json_value_free(union_val);
  json_value_free(root_val);
  PASS();
}

/**
 * @brief Tests code2schema_main CLI with error paths and rich C headers.
 *
 * @return TEST_PASSED on success.
 */
TEST test_code2schema_main_full_suite(void) {
  char *argv[3];
  char test_h[256];
  char test_json[256];
  FILE *f;
  cdd_c_error_t rc;

  /* Wrong argc */
  rc = code2schema_main(1, NULL);
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);

  /* Nonexistent input file */
  argv[0] = (char *)(size_t) "code2schema";
  argv[1] = (char *)(size_t) "/nonexistent/missing_file.h";
  rc = code2schema_main(2, argv);
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);

  /* Create temporary C header file */
  CDD_SNPRINTF(test_h, sizeof(test_h), "c2s_tmp_%u.h", (unsigned)rand());
  CDD_SNPRINTF(test_json, sizeof(test_json), "c2s_tmp_%u.json",
               (unsigned)rand());

  f = fopen(test_h, "w");
  ASSERT(f != NULL);
  fputs("/* C Header with struct, union, enum, and nested structs */\n"
        "union Shape {\n"
        "  int circle_radius;\n"
        "  char* label;\n"
        "};\n\n"
        "enum Color {\n"
        "  RED,\n"
        "  GREEN,\n"
        "  BLUE\n"
        "};\n\n"
        "struct UserProfile {\n"
        "  int id;\n"
        "  char* username;\n"
        "  struct {\n"
        "    char* street;\n"
        "    int zip;\n"
        "  } address;\n"
        "  size_t n_roles;\n"
        "  char* roles;\n"
        "};\n\n"
        "enum { ANON_A, ANON_B };\n\n"
        "enum Broken { VAL1\n",
        f);
  fclose(f);

  argv[0] = test_h;
  argv[1] = test_json;
  rc = code2schema_main(2, argv);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* Unwritable output path */
  argv[1] = (char *)(size_t) "/proc/nonexistent_dir/out.json";
  rc = code2schema_main(2, argv);
  ASSERT(rc != CDD_C_SUCCESS);

  remove(test_h);
  remove(test_json);
  PASS();
}

/**
 * @brief Tests c2s_union_array_items_supported branches.
 *
 * @return TEST_PASSED on success.
 */
TEST test_code2schema_c2s_union_array_items_branches(void) {
  JSON_Value *root_val;
  JSON_Object *root;
  JSON_Value *val;
  JSON_Object *obj;
  int supported = 0;
  cdd_c_error_t rc;

  root_val = json_parse_string(
      "{\"StatusEnum\":{\"type\":\"string\",\"enum\":[\"A\",\"B\"]}}");
  root = json_value_get_object(root_val);

  /* Items with $ref to enum */
  val = json_parse_string(
      "{\"items\":{\"$ref\":\"#/components/schemas/StatusEnum\"}}");
  obj = json_value_get_object(val);
  rc = c2s_union_array_items_supported(obj, root, 1, &supported);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, supported);
  json_value_free(val);

  /* Items with type union array */
  val = json_parse_string("{\"items\":{\"type\":[\"string\",\"null\"]}}");
  obj = json_value_get_object(val);
  rc = c2s_union_array_items_supported(obj, root, 1, &supported);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, supported);
  json_value_free(val);

  /* Items with anonymous properties */
  val = json_parse_string(
      "{\"items\":{\"properties\":{\"name\":{\"type\":\"string\"}}}}");
  obj = json_value_get_object(val);
  rc = c2s_union_array_items_supported(obj, root, 1, &supported);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, supported);
  json_value_free(val);

  /* Items with empty object (no type, ref, or properties) */
  val = json_parse_string("{\"items\":{}}");
  obj = json_value_get_object(val);
  rc = c2s_union_array_items_supported(obj, root, 1, &supported);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, supported);
  json_value_free(val);

  json_value_free(root_val);
  PASS();
}

/**
 * @brief Tests c2s_merge_schema_extras_strings with invalid JSON and key
 * collisions.
 *
 * @return TEST_PASSED on success.
 */
TEST test_code2schema_merge_schema_extras_strings_branches(void) {
  char *dest = NULL;
  cdd_c_error_t rc;

  /* Dest is not a JSON object */
  dest = (char *)malloc(32);
  CDD_STRCPY(dest, 32, "not_json");
  rc = c2s_merge_schema_extras_strings(&dest, "{\"a\":1}");
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  free(dest);
  dest = NULL;

  /* Dest is a JSON array, not object */
  dest = (char *)malloc(32);
  CDD_STRCPY(dest, 32, "[1, 2]");
  rc = c2s_merge_schema_extras_strings(&dest, "{\"a\":1}");
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  free(dest);
  dest = NULL;

  /* Key overwrite branch: both have key "key" */
  dest = (char *)malloc(64);
  CDD_STRCPY(dest, 64, "{\"key\":\"old_val\",\"other\":1}");
  rc = c2s_merge_schema_extras_strings(&dest, "{\"key\":\"new_val\"}");
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(strstr(dest, "new_val") != NULL);
  C_CDD_FREE(dest);
  dest = NULL;

  PASS();
}

/**
 * @brief Tests merge_struct_fields when src is an enum or has type unions.
 *
 * @return TEST_PASSED on success.
 */
TEST test_code2schema_merge_struct_fields_enums_and_extras(void) {
  struct StructFields src_sf;
  struct StructFields dst_sf;
  char *u1[1];
  char *u2[1];
  cdd_c_error_t rc;

  u1[0] = (char *)(size_t) "string";
  u2[0] = (char *)(size_t) "null";

  /* src is_enum */
  rc = struct_fields_init(&src_sf);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = struct_fields_init(&dst_sf);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  src_sf.is_enum = 1;
  rc = enum_members_init(&src_sf.enum_members);
  ASSERT_EQ(0, rc);
  rc = enum_members_add(&src_sf.enum_members, "VAL");
  ASSERT_EQ(0, rc);

  rc = merge_struct_fields(&dst_sf, &src_sf);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  struct_fields_free(&src_sf);
  struct_fields_free(&dst_sf);

  /* src field has type_union and items_type_union and items_extra_json */
  rc = struct_fields_init(&src_sf);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = struct_fields_init(&dst_sf);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = struct_fields_add(&src_sf, "rich_field", "array", "Item", NULL, NULL);
  ASSERT_EQ(0, rc);
  c_cdd_strdup("{\"x-item\":true}", &src_sf.fields[0].items_extra_json);
  copy_string_array_code2schema(&src_sf.fields[0].type_union,
                                &src_sf.fields[0].n_type_union, u1, 1);
  copy_string_array_code2schema(&src_sf.fields[0].items_type_union,
                                &src_sf.fields[0].n_items_type_union, u2, 1);

  rc = merge_struct_fields(&dst_sf, &src_sf);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, dst_sf.size);
  ASSERT(dst_sf.fields[0].items_extra_json != NULL);
  ASSERT(dst_sf.fields[0].type_union != NULL);
  ASSERT(dst_sf.fields[0].items_type_union != NULL);

  struct_fields_free(&src_sf);
  struct_fields_free(&dst_sf);
  PASS();
}

/**
 * @brief Tests apply_union_to_struct_fields_ex with all variant types and edge
 * cases.
 *
 * @return TEST_PASSED on success.
 */
TEST test_code2schema_apply_union_variants_all_types(void) {
  JSON_Value *root_val;
  JSON_Object *root;
  JSON_Value *union_val;
  JSON_Array *union_arr;
  struct StructFields sf;
  cdd_c_error_t rc;

  root_val =
      json_parse_string("{\"MyEnum\":{\"type\":\"string\",\"enum\":[\"X\"]}}");
  root = json_value_get_object(root_val);

  /* Variants covering: number, boolean, null, object with title, array with
   * enum items */
  union_val = json_parse_string(
      "["
      "  {\"type\":\"number\"},"
      "  {\"type\":\"boolean\"},"
      "  {\"type\":\"null\"},"
      "  "
      "{\"title\":\"MyObj\",\"type\":\"object\",\"properties\":{\"p\":{"
      "\"type\":\"integer\"}}},"
      "  "
      "{\"type\":\"array\",\"items\":{\"$ref\":\"#/components/schemas/"
      "MyEnum\"}},"
      "  {\"type\":\"array\",\"items\":{\"type\":[\"string\",\"null\"]}},"
      "  "
      "{\"type\":\"array\",\"items\":{\"properties\":{\"sub\":{\"type\":"
      "\"string\"}}}}"
      "]");
  union_arr = json_value_get_array(union_val);

  rc = struct_fields_init(&sf);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = apply_union_to_struct_fields_ex(union_arr, &sf, root, "AllUnion", 0,
                                       NULL, 1);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, sf.is_union);
  ASSERT_EQ(7, sf.n_union_variants);

  /* Already has fields -> returns success early */
  rc = apply_union_to_struct_fields_ex(union_arr, &sf, root, "AllUnion", 0,
                                       NULL, 1);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  struct_fields_free(&sf);
  json_value_free(union_val);

  /* Empty union array -> returns success early */
  union_val = json_parse_string("[]");
  union_arr = json_value_get_array(union_val);
  rc = struct_fields_init(&sf);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = apply_union_to_struct_fields_ex(union_arr, &sf, root, "EmptyUnion", 0,
                                       NULL, 1);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  struct_fields_free(&sf);
  json_value_free(union_val);

  /* Array variant when allow_inline is 0 */
  union_val = json_parse_string(
      "[{\"type\":\"array\",\"items\":{\"type\":\"string\"}}]");
  union_arr = json_value_get_array(union_val);
  rc = struct_fields_init(&sf);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = apply_union_to_struct_fields_ex(union_arr, &sf, root, "ArrayNoInline", 0,
                                       NULL, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  struct_fields_free(&sf);
  json_value_free(union_val);

  json_value_free(root_val);
  PASS();
}

/**
 * @brief Tests json_array_to_enum_members with non-string elements.
 *
 * @return TEST_PASSED on success.
 */
TEST test_code2schema_json_array_to_enum_members_nonstrings(void) {
  JSON_Value *val = json_parse_string("[123, \"VALID\", true, null]");
  JSON_Array *arr = json_value_get_array(val);
  struct EnumMembers em;
  cdd_c_error_t rc;

  rc = enum_members_init(&em);
  ASSERT_EQ(0, rc);

  rc = json_array_to_enum_members(arr, &em);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, em.size);
  ASSERT_STR_EQ("VALID", em.members[0]);

  enum_members_free(&em);
  json_value_free(val);
  PASS();
}

/**
 * @brief Tests out-of-memory error paths across code2schema functions using
 * g_cdd_alloc_fail.
 *
 * @return TEST_PASSED on success.
 */
TEST test_code2schema_alloc_failures(void) {
  char *src_arr[2];
  char **dst_arr = NULL;
  size_t dst_cnt = 0;
  JSON_Value *val;
  JSON_Array *arr;
  JSON_Object *obj;
  char *str_out = NULL;
  struct StructFields sf1;
  struct StructFields sf2;
  cdd_c_error_t rc;

  src_arr[0] = (char *)(size_t) "alpha";
  src_arr[1] = (char *)(size_t) "beta";

  /* copy_string_array_code2schema OOM */
  g_cdd_alloc_fail = 1;
  rc = copy_string_array_code2schema(&dst_arr, &dst_cnt, src_arr, 2);
  g_cdd_alloc_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);

  g_cdd_strdup_fail = 1;
  rc = copy_string_array_code2schema(&dst_arr, &dst_cnt, src_arr, 2);
  g_cdd_strdup_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);

  /* parse_type_union_array_code2schema OOM */
  val = json_parse_string("[\"string\",\"integer\"]");
  arr = json_value_get_array(val);

  g_cdd_alloc_fail = 1;
  rc = parse_type_union_array_code2schema(arr, &dst_arr, &dst_cnt, NULL, NULL);
  g_cdd_alloc_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);

  g_cdd_strdup_fail = 1;
  rc = parse_type_union_array_code2schema(arr, &dst_arr, &dst_cnt, NULL, NULL);
  g_cdd_strdup_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);

  json_value_free(val);

  /* c2s_collect_string_array OOM */
  val = json_parse_string("[\"one\",\"two\"]");
  arr = json_value_get_array(val);

  g_cdd_alloc_fail = 1;
  rc = c2s_collect_string_array(arr, &dst_arr, &dst_cnt);
  g_cdd_alloc_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);

  g_cdd_strdup_fail = 1;
  rc = c2s_collect_string_array(arr, &dst_arr, &dst_cnt);
  g_cdd_strdup_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);

  json_value_free(val);

  /* c2s_collect_property_names OOM */
  val = json_parse_string("{\"properties\":{\"a\":{},\"b\":{}}}");
  obj = json_value_get_object(val);

  g_cdd_alloc_fail = 1;
  rc = c2s_collect_property_names(obj, &dst_arr, &dst_cnt);
  g_cdd_alloc_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);

  g_cdd_strdup_fail = 1;
  rc = c2s_collect_property_names(obj, &dst_arr, &dst_cnt);
  g_cdd_strdup_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);

  json_value_free(val);

  /* sanitize_identifier OOM */
  g_cdd_alloc_fail = 1;
  rc = sanitize_identifier("my_var", &str_out);
  g_cdd_alloc_fail = 0;
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(NULL, str_out);

  g_cdd_strdup_fail = 1;
  rc = sanitize_identifier(NULL, &str_out);
  g_cdd_strdup_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);

  /* c2s_merge_schema_extras_strings OOM */
  str_out = NULL;
  g_cdd_strdup_fail = 1;
  rc = c2s_merge_schema_extras_strings(&str_out, "{\"a\":1}");
  g_cdd_strdup_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);

  /* merge_struct_fields OOM */
  struct_fields_init(&sf1);
  struct_fields_init(&sf2);
  struct_fields_add(&sf1, "src_f", "string", NULL, NULL, NULL);
  c_cdd_strdup("{\"extra\":1}", &sf1.schema_extra_json);

  g_cdd_strdup_fail = 1;
  rc = merge_struct_fields(&sf2, &sf1);
  g_cdd_strdup_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);

  struct_fields_free(&sf1);
  struct_fields_free(&sf2);

  /* apply_union_to_struct_fields_ex OOM */
  val = json_parse_string("[{\"type\":\"string\"}]");
  arr = json_value_get_array(val);
  struct_fields_init(&sf1);

  g_cdd_alloc_fail = 1;
  rc = apply_union_to_struct_fields_ex(arr, &sf1, NULL, "UnionOOM", 0, NULL, 0);
  g_cdd_alloc_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);

  struct_fields_free(&sf1);
  json_value_free(val);

  PASS();
}

/**
 * @brief Tests ref_points_to_string_enum and resolve_schema_ref_object empty
 * ref paths.
 *
 * @return TEST_PASSED on success.
 */
TEST test_code2schema_empty_refs_and_defaults(void) {
  JSON_Value *root_val = json_value_init_object();
  JSON_Object *root = json_value_get_object(root_val);
  JSON_Value *val = json_value_init_object();
  JSON_Object *obj = json_value_get_object(val);
  JSON_Object *res = NULL;
  struct StructField f;
  char *name = NULL;
  int is_enum = 0;
  int b = 0;
  cdd_c_error_t rc;

  /* Empty and slash refs */
  rc = ref_points_to_string_enum(root, "", &is_enum);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, is_enum);

  rc = ref_points_to_string_enum(root, "/", &is_enum);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, is_enum);

  rc = resolve_schema_ref_object(root, "", &res);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(NULL, res);

  rc = resolve_schema_ref_object(root, "/", &res);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(NULL, res);

  /* Test str_after_last failure */
  {
    extern C_CDD_EXPORT int g_cdd_fail_str_after_last;
    g_cdd_fail_str_after_last = 1;
    rc = ref_points_to_string_enum(root, "#/components/schemas/Status",
                                   &is_enum);
    g_cdd_fail_str_after_last = 0;
    ASSERT(rc != CDD_C_SUCCESS);

    g_cdd_fail_str_after_last = 1;
    rc = resolve_schema_ref_object(root, "#/components/schemas/Status", &res);
    g_cdd_fail_str_after_last = 0;
    ASSERT(rc != CDD_C_SUCCESS);
  }

  /* str_starts_with error branch */
  {
    extern C_CDD_EXPORT int g_cdd_fail_str_starts_with;
    g_cdd_fail_str_starts_with = 1;
    rc = str_starts_with("abc", "a", &b);
    g_cdd_fail_str_starts_with = 0;
    ASSERT(rc != CDD_C_SUCCESS);
  }

  /* make_inline_schema_name with NULL args */
  rc = make_inline_schema_name(NULL, NULL, NULL, &name);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("Union_Variant", name);
  C_CDD_FREE(name);
  name = NULL;

  /* c2s_write_default_value for object type */
  memset(&f, 0, sizeof(f));
  CDD_STRCPY(f.type, sizeof(f.type), "object");
  CDD_STRCPY(f.default_val, sizeof(f.default_val), "{}");
  rc = c2s_write_default_value(obj, &f);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* c2s_write_array_constraints for non-array */
  CDD_STRCPY(f.type, sizeof(f.type), "string");
  rc = c2s_write_array_constraints(obj, &f);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* c2s_write_type_union with NULL type and NULL union */
  rc = c2s_write_type_union(obj, NULL, NULL, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  json_value_free(val);
  json_value_free(root_val);
  PASS();
}

/**
 * @brief Tests c2s_json_object_to_struct_fields_internal with union array and
 * empty properties.
 *
 * @return TEST_PASSED on success.
 */
TEST test_code2schema_json_object_fields_more_branches(void) {
  JSON_Value *val;
  JSON_Object *obj;
  struct StructFields sf;
  cdd_c_error_t rc;

  /* Schema with boolean exclusiveMinimum and field without type/ref (empty
   * property) */
  val = json_parse_string("{\"type\":\"object\","
                          "\"properties\":{"
                          "  \"empty_p\":{},"
                          "  "
                          "\"num_bool_ex\":{\"type\":\"number\",\"minimum\":5."
                          "0,\"exclusiveMinimum\":true},"
                          "  "
                          "\"arr_union_type\":{\"type\":[\"array\",\"null\"],"
                          "\"items\":{\"type\":\"string\"}}"
                          "}}");
  obj = json_value_get_object(val);

  rc = struct_fields_init(&sf);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = c2s_json_object_to_struct_fields_internal(obj, &sf, NULL, "MoreBranches",
                                                 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(2, sf.size);
  ASSERT_EQ(1, sf.fields[0].exclusive_min);

  struct_fields_free(&sf);
  json_value_free(val);
  PASS();
}

/**
 * @brief Tests remaining branches in code2schema to achieve 100% test coverage.
 *
 * @return TEST_PASSED on success.
 */
TEST test_code2schema_exhaustive_100_percent_coverage(void) {
  /* 1. sanitize_identifier empty string */
  {
    char *out = NULL;
    cdd_c_error_t rc;
    rc = sanitize_identifier("", &out);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_STR_EQ("Variant", out);
    C_CDD_FREE(out);
  }

  /* 2. discriminator_value_for_variant all branches */
  {
    JSON_Value *val =
        json_parse_string("{\"mapping\":{\"k1\":\"#/components/schemas/"
                          "A\",\"k2\":\"B\",\"k3\":\"C\",\"k4\":123}}");
    JSON_Object *obj = json_value_get_object(val);
    char *out_val = NULL;
    cdd_c_error_t rc;

    /* Fallback on NULL disc_obj */
    rc = discriminator_value_for_variant(NULL, "Hint", NULL, &out_val);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_STR_EQ("Hint", out_val);
    C_CDD_FREE(out_val);

    /* Matching ref */
    rc = discriminator_value_for_variant(obj, "Hint", "#/components/schemas/A",
                                         &out_val);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_STR_EQ("k1", out_val);
    C_CDD_FREE(out_val);

    /* Matching ref_name */
    rc = discriminator_value_for_variant(obj, "Hint", "#/components/schemas/B",
                                         &out_val);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_STR_EQ("k2", out_val);
    C_CDD_FREE(out_val);

    /* Matching variant_name */
    rc = discriminator_value_for_variant(obj, "C", NULL, &out_val);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_STR_EQ("k3", out_val);
    C_CDD_FREE(out_val);

    /* Fallback variant_name */
    rc = discriminator_value_for_variant(obj, "SchemaX", NULL, &out_val);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_STR_EQ("SchemaX", out_val);
    C_CDD_FREE(out_val);

    /* Fallback ref_name */
    rc = discriminator_value_for_variant(obj, NULL, "#/components/schemas/RefX",
                                         &out_val);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_STR_EQ("RefX", out_val);
    C_CDD_FREE(out_val);

    /* Fallback NULL */
    rc = discriminator_value_for_variant(obj, NULL, NULL, &out_val);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_EQ(NULL, out_val);

    json_value_free(val);
  }

  /* 3. merge_struct_fields complete branches */
  {
    struct StructFields src;
    struct StructFields dest;
    cdd_c_error_t rc;
    char *types1[2];
    char *types2[2];

    rc = struct_fields_init(&src);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    rc = struct_fields_init(&dest);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    rc = struct_fields_add(&src, "field1", "string", NULL, NULL, NULL);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    c_cdd_strdup("{\"x-custom\":1}", &src.fields[0].schema_extra_json);
    c_cdd_strdup("{\"x-item-custom\":2}", &src.fields[0].items_extra_json);

    types1[0] = C_CDD_STR_LIT("string");
    types1[1] = C_CDD_STR_LIT("null");
    copy_string_array_code2schema(&src.fields[0].type_union,
                                  &src.fields[0].n_type_union, types1, 2);
    types2[0] = C_CDD_STR_LIT("integer");
    types2[1] = C_CDD_STR_LIT("null");
    copy_string_array_code2schema(&src.fields[0].items_type_union,
                                  &src.fields[0].n_items_type_union, types2, 2);

    /* First merge into empty dest (field added) */
    rc = merge_struct_fields(&dest, &src);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_EQ(1, dest.size);

    /* Second merge with matching field (field merged via merge_struct_field) */
    rc = merge_struct_fields(&dest, &src);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    struct_fields_free(&src);
    struct_fields_free(&dest);
  }

  /* 4. apply_union_to_struct_fields_ex variant types */
  {
    struct StructFields dest;
    JSON_Value *val = json_parse_string(
        "[42, {\"type\":\"null\"}, {\"type\":\"custom_unknown\"}, "
        "{\"type\":\"array\",\"items\":{\"properties\":{\"sub\":{\"type\":"
        "\"string\"}}}}, "
        "{\"type\":\"array\",\"items\":{\"type\":\"unknown_items\"}}, "
        "{}]");
    JSON_Array *arr = json_value_get_array(val);
    JSON_Value *root_val = json_value_init_object();
    JSON_Object *root = json_value_get_object(root_val);
    cdd_c_error_t rc;

    rc = struct_fields_init(&dest);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    /* Test with allow_inline = 1 */
    rc = apply_union_to_struct_fields_ex(arr, &dest, root, "TestUnion", 0, NULL,
                                         1);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    struct_fields_free(&dest);

    /* Test with allow_inline = 0 */
    rc = struct_fields_init(&dest);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    rc = apply_union_to_struct_fields_ex(arr, &dest, root, "TestUnion", 0, NULL,
                                         0);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    struct_fields_free(&dest);
    json_value_free(val);
    json_value_free(root_val);
  }

  /* 5. write_struct_to_json_schema all field attributes */
  {
    struct StructFields sf;
    JSON_Value *root = json_value_init_object();
    JSON_Object *schemas_obj = json_value_get_object(root);
    char *arr_types[2];
    cdd_c_error_t rc;

    rc = struct_fields_init(&sf);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    rc = struct_fields_add(&sf, "attr_field", "string", NULL, NULL, NULL);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    sf.fields[0].deprecated_set = 1;
    sf.fields[0].deprecated = 1;
    sf.fields[0].read_only_set = 1;
    sf.fields[0].read_only = 1;
    sf.fields[0].write_only_set = 1;
    sf.fields[0].write_only = 1;
    CDD_STRCPY(sf.fields[0].description, sizeof(sf.fields[0].description),
               "Description text");
    CDD_STRCPY(sf.fields[0].format, sizeof(sf.fields[0].format), "date-time");
    c_cdd_strdup("{\"x-fld\":1}", &sf.fields[0].schema_extra_json);

    /* Array field with items_type_union and $ref */
    rc = struct_fields_add(&sf, "arr_ref_field", "array", "MyCustomModel", NULL,
                           NULL);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    arr_types[0] = C_CDD_STR_LIT("string");
    arr_types[1] = C_CDD_STR_LIT("null");
    copy_string_array_code2schema(&sf.fields[1].items_type_union,
                                  &sf.fields[1].n_items_type_union, arr_types,
                                  2);
    c_cdd_strdup("{\"x-item\":2}", &sf.fields[1].items_extra_json);

    /* Enum field with direct ref */
    rc = struct_fields_add(&sf, "enum_field", "enum",
                           "#/components/schemas/MyEnum", NULL, NULL);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    c_cdd_strdup("{\"x-struct\":3}", &sf.schema_extra_json);

    rc = write_struct_to_json_schema(schemas_obj, "FullStruct", &sf);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    struct_fields_free(&sf);
    json_value_free(root);
  }

  /* 6. parse_struct_member_line formats & code2schema_main nested structs */
  {
    struct StructFields sf;
    cdd_c_error_t rc;
    FILE *fp;
    char out_path[256];
    char *argv[4];

    rc = struct_fields_init(&sf);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    rc = parse_struct_member_line("uint32_t numbers[10];", &sf);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_EQ(1, sf.size);
    ASSERT(sf.fields[0].items_extra_json != NULL);
    ASSERT(strstr(sf.fields[0].items_extra_json, "int32") != NULL);

    rc = parse_struct_member_line("long int big_nums[5];", &sf);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_EQ(2, sf.size);
    ASSERT(sf.fields[1].items_extra_json != NULL);
    ASSERT(strstr(sf.fields[1].items_extra_json, "int64") != NULL);

    struct_fields_free(&sf);

    /* Test code2schema_main with nested struct, union pointer member, and
     * leading spaces */
    fp = fopen("test_nested_code2schema.h", "w");
    ASSERT(fp != NULL);
    fprintf(fp, "   \n");
    fprintf(fp, "   struct Outer {\n");
    fprintf(fp, "     struct {\n");
    fprintf(fp, "       int nested_val;\n");
    fprintf(fp, "     } inner;\n");
    fprintf(fp, "     int plain_val;\n");
    fprintf(fp, "   };\n");
    fprintf(fp, "   union PtrUnion {\n");
    fprintf(fp, "     char *ptr_name;\n");
    fprintf(fp, "     int id;\n");
    fprintf(fp, "   };\n");
    fclose(fp);

    CDD_SNPRINTF(out_path, sizeof(out_path), "test_nested_code2schema.json");
    argv[0] = C_CDD_STR_LIT("test_nested_code2schema.h");
    argv[1] = out_path;
    argv[2] = NULL;

    rc = code2schema_main(2, argv);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    remove("test_nested_code2schema.h");
    remove(out_path);
  }

  /* 7. json_array_to_enum_members NULL args */
  {
    struct EnumMembers em;
    JSON_Value *val = json_parse_string("[\"A\", \"B\"]");
    JSON_Array *arr = json_value_get_array(val);
    cdd_c_error_t rc;

    enum_members_init(&em);

    rc = json_array_to_enum_members(NULL, &em);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

    rc = json_array_to_enum_members(arr, NULL);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

    enum_members_free(&em);
    json_value_free(val);
  }

  /* 8. c2s_merge_schema_extras_object & strings edge cases */
  {
    JSON_Value *val = json_value_init_object();
    JSON_Object *obj = json_value_get_object(val);
    char *dest = NULL;
    cdd_c_error_t rc;

    /* non-object json strings */
    rc = c2s_merge_schema_extras_object(obj, "[]");
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    rc = c2s_merge_schema_extras_object(obj, "123");
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    /* dest is valid json, src is invalid */
    c_cdd_strdup("{\"x\":1}", &dest);
    rc = c2s_merge_schema_extras_strings(&dest, "invalid_json");
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    C_CDD_FREE(dest);

    json_value_free(val);
  }

  /* 9. Union array in items */
  {
    struct StructFields sf;
    JSON_Value *val = json_parse_string(
        "{\"type\":\"object\",\"properties\":{\"tags\":{\"type\":\"array\","
        "\"items\":{\"type\":[\"string\",\"null\"]}}}}");
    JSON_Object *obj = json_value_get_object(val);
    cdd_c_error_t rc;

    rc = struct_fields_init(&sf);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    rc = c2s_json_object_to_struct_fields_internal(obj, &sf, NULL,
                                                   "UnionArrItems", 0);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_EQ(1, sf.size);
    ASSERT_STR_EQ("array", sf.fields[0].type);
    ASSERT_STR_EQ("string", sf.fields[0].ref);
    ASSERT(sf.fields[0].items_type_union != NULL);
    ASSERT_EQ(2, sf.fields[0].n_items_type_union);

    struct_fields_free(&sf);
    json_value_free(val);
  }

  /* 10. Null checks in helpers */
  {
    int is_prim = 0;
    char **props_out = NULL;
    size_t props_cnt = 0;
    cdd_c_error_t rc;

    rc = ref_points_to_string_enum(NULL, "#/components/schemas/Test", &is_prim);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_EQ(0, is_prim);

    rc = c2s_collect_property_names(NULL, &props_out, &props_cnt);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_EQ(NULL, props_out);
    ASSERT_EQ(0, props_cnt);
  }

  /* 11. Helper error and null arguments */
  {
    char *out_val = NULL;
    char **out_arr = NULL;
    size_t out_cnt = 0;
    cdd_c_error_t rc;

    /* sanitize_identifier invalid / OOM */
    rc = sanitize_identifier(NULL, &out_val);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_STR_EQ("Variant", out_val);
    C_CDD_FREE(out_val);
    rc = sanitize_identifier("foo", NULL);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_alloc_fail = 1;
    rc = sanitize_identifier("foo", &out_val);
    g_cdd_alloc_fail = 0;
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_EQ(NULL, out_val);
    g_cdd_strdup_fail = 1;
    rc = sanitize_identifier("", &out_val);
    g_cdd_strdup_fail = 0;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);

    /* make_unique_variant_name invalid */
    rc = make_unique_variant_name(NULL, "base", 0, &out_val);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_EQ(NULL, out_val);

    /* make_inline_schema_name invalid */
    rc = make_inline_schema_name("Schema", "Variant", "Suffix", NULL);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

    /* c2s_collect_string_array OOM */
    {
      JSON_Value *arr_val = json_parse_string("[\"A\", \"B\"]");
      JSON_Array *arr = json_value_get_array(arr_val);
      g_cdd_alloc_fail = 1;
      rc = c2s_collect_string_array(arr, &out_arr, &out_cnt);
      g_cdd_alloc_fail = 0;
      ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
      json_value_free(arr_val);
    }

    /* c2s_collect_property_names invalid & OOM */
    rc = c2s_collect_property_names(NULL, NULL, &out_cnt);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = c2s_collect_property_names(NULL, &out_arr, NULL);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    {
      JSON_Value *p_val =
          json_parse_string("{\"properties\":{\"x\":{\"type\":\"int\"}}}");
      JSON_Object *p_obj = json_value_get_object(p_val);
      g_cdd_alloc_fail = 1;
      rc = c2s_collect_property_names(p_obj, &out_arr, &out_cnt);
      g_cdd_alloc_fail = 0;
      ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
      json_value_free(p_val);
    }

    /* discriminator_value_for_variant invalid & failure */
    rc = discriminator_value_for_variant(NULL, "Hint", NULL, NULL);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

    {
      JSON_Value *d_val = json_parse_string(
          "{\"mapping\":{\"k1\":\"#/schemas/A\",\"k2\":\"B\",\"k3\":\"C\"}}");
      JSON_Object *d_obj = json_value_get_object(d_val);
      g_cdd_strdup_fail = 1;
      rc = discriminator_value_for_variant(d_obj, "Hint", "#/schemas/A",
                                           &out_val);
      g_cdd_strdup_fail = 0;
      ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);

      g_cdd_strdup_fail = 1;
      rc = discriminator_value_for_variant(d_obj, "Hint", "#/schemas/B",
                                           &out_val);
      g_cdd_strdup_fail = 0;
      ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);

      g_cdd_strdup_fail = 1;
      rc = discriminator_value_for_variant(d_obj, "C", NULL, &out_val);
      g_cdd_strdup_fail = 0;
      ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);

      g_cdd_strdup_fail = 1;
      rc = discriminator_value_for_variant(d_obj, "FallbackS", NULL, &out_val);
      g_cdd_strdup_fail = 0;
      ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);

      g_cdd_strdup_fail = 1;
      rc = discriminator_value_for_variant(d_obj, NULL, "#/schemas/FallbackR",
                                           &out_val);
      g_cdd_strdup_fail = 0;
      ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);

      json_value_free(d_val);
    }
  }

  /* 12. c2s_merge_schema_extras_strings exhaustive tests */
  {
    char *dest = NULL;
    cdd_c_error_t rc;

    /* dest_obj NULL */
    c_cdd_strdup("[]", &dest);
    rc = c2s_merge_schema_extras_strings(&dest, "{}");
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    C_CDD_FREE(dest);

    /* src_obj NULL */
    c_cdd_strdup("{}", &dest);
    rc = c2s_merge_schema_extras_strings(&dest, "[]");
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    C_CDD_FREE(dest);

    /* existing key replacement */
    c_cdd_strdup("{\"key\":1,\"other\":2}", &dest);
    rc = c2s_merge_schema_extras_strings(&dest, "{\"key\":3}");
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT(strstr(dest, "\"key\"") != NULL);
    C_CDD_FREE(dest);

    /* OOM on c_cdd_strdup */
    c_cdd_strdup("{\"a\":1}", &dest);
    g_cdd_strdup_fail = 1;
    rc = c2s_merge_schema_extras_strings(&dest, "{\"b\":2}");
    g_cdd_strdup_fail = 0;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    C_CDD_FREE(dest);
  }

  /* 13. Additional edge cases for 100% coverage */
  {
    cdd_c_error_t rc;
    char **out_arr = NULL;
    size_t out_cnt = 0;

    /* resolve_schema_ref_object NULL out */
    rc = resolve_schema_ref_object(NULL, "ref", NULL);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

    /* c2s_collect_property_names on object without properties */
    {
      JSON_Value *v = json_parse_string("{\"type\":\"string\"}");
      JSON_Object *o = json_value_get_object(v);
      rc = c2s_collect_property_names(o, &out_arr, &out_cnt);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      ASSERT_EQ(NULL, out_arr);
      ASSERT_EQ(0, out_cnt);
      json_value_free(v);
    }

    /* c2s_parse_union_and_write invalid */
    rc = c2s_parse_union_and_write(NULL, NULL, NULL);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

    /* apply_union_to_struct_fields_ex with non-object element, empty object and
     * boolean */
    {
      struct StructFields dest;
      JSON_Value *val = json_parse_string(
          "[{\"type\":\"string\"}, 42, {}, {\"type\":\"boolean\"}]");
      JSON_Array *arr = json_value_get_array(val);
      JSON_Value *root_val = json_value_init_object();
      JSON_Object *root = json_value_get_object(root_val);

      rc = struct_fields_init(&dest);
      ASSERT_EQ(CDD_C_SUCCESS, rc);

      rc = apply_union_to_struct_fields_ex(arr, &dest, root, "TestUnionEdges",
                                           0, NULL, 1);
      ASSERT_EQ(CDD_C_SUCCESS, rc);

      struct_fields_free(&dest);
      json_value_free(val);
      json_value_free(root_val);
    }

    /* apply_union_to_struct_fields_ex with allow_inline = 0 and array variant
     */
    {
      struct StructFields dest;
      JSON_Value *val = json_parse_string(
          "[{\"type\":\"array\",\"items\":{\"type\":\"string\"}}]");
      JSON_Array *arr = json_value_get_array(val);
      JSON_Value *root_val = json_value_init_object();
      JSON_Object *root = json_value_get_object(root_val);

      rc = struct_fields_init(&dest);
      ASSERT_EQ(CDD_C_SUCCESS, rc);

      rc = apply_union_to_struct_fields_ex(arr, &dest, root, "TestArrNoInline",
                                           0, NULL, 0);
      ASSERT_EQ(CDD_C_SUCCESS, rc);

      struct_fields_free(&dest);
      json_value_free(val);
      json_value_free(root_val);
    }
  }

  /* 14. c2s_json_object_to_struct_fields_internal property error paths */
  {
    struct StructFields sf;
    JSON_Value *val_root =
        json_parse_string("{\"components\":{\"schemas\":{\"MyEnum\":{\"type\":"
                          "\"string\",\"enum\":[\"A\"]}}}}");
    JSON_Object *root = json_value_get_object(val_root);
    JSON_Value *val_schema = json_parse_string(
        "{\"type\":\"object\",\"properties\":{"
        "  "
        "\"arr_ref\":{\"type\":\"array\",\"items\":{\"$ref\":\"#/components/"
        "schemas/MyEnum\"}},"
        "  \"direct_ref\":{\"$ref\":\"#/components/schemas/MyEnum\"},"
        "  \"plain_str\":{\"type\":\"string\",\"x-extra\":1}"
        "}}");
    JSON_Object *schema = json_value_get_object(val_schema);
    cdd_c_error_t rc;

    /* ref_points_to_string_enum on array items failure */
    struct_fields_init(&sf);
    g_cdd_fail_str_after_last = 1;
    rc = c2s_json_object_to_struct_fields_internal(schema, &sf, root, "TestErr",
                                                   0);
    g_cdd_fail_str_after_last = 0;
    ASSERT_NEQ(CDD_C_SUCCESS, rc);
    struct_fields_free(&sf);

    /* ref_points_to_string_enum on direct ref failure */
    struct_fields_init(&sf);
    g_cdd_fail_str_after_last = 2;
    rc = c2s_json_object_to_struct_fields_internal(schema, &sf, root, "TestErr",
                                                   0);
    g_cdd_fail_str_after_last = 0;
    ASSERT_NEQ(CDD_C_SUCCESS, rc);
    struct_fields_free(&sf);

    {
      int k;
      for (k = 1; k <= 3; ++k) {
        struct_fields_init(&sf);
        g_cdd_fail_struct_fields_add = k;
        rc = c2s_json_object_to_struct_fields_internal(schema, &sf, root,
                                                       "TestErr", 0);
        g_cdd_fail_struct_fields_add = 0;
        ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
        struct_fields_free(&sf);
      }
    }

    json_value_free(val_schema);
    json_value_free(val_root);
  }

  /* 15. c2s_merge_schema_extras_strings error branches */
  {
    char *dest = NULL;
    cdd_c_error_t rc;

    c_cdd_strdup("{\"a\":1}", &dest);
    g_c2s_helper_fail = 2;
    rc = c2s_merge_schema_extras_strings(&dest, "{\"b\":2}");
    g_c2s_helper_fail = 0;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    C_CDD_FREE(dest);

    c_cdd_strdup("{\"a\":1}", &dest);
    g_cdd_strdup_fail = 1;
    rc = c2s_merge_schema_extras_strings(&dest, "{\"b\":2}");
    g_cdd_strdup_fail = 0;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    C_CDD_FREE(dest);
  }

  /* 16. Bit-field trim_trailing failure */
  {
    struct StructFields sf;
    cdd_c_error_t rc;
    struct_fields_init(&sf);
    g_c2s_helper_fail = 2;
    rc = parse_struct_member_line("int flag : 1;", &sf);
    g_c2s_helper_fail = 0;
    ASSERT_NEQ(CDD_C_SUCCESS, rc);
    struct_fields_free(&sf);
  }

  /* 17. parse_struct_member_line array format error & enum_members_add error */
  {
    struct StructFields sf;
    struct EnumMembers em;
    JSON_Value *arr_val;
    JSON_Array *arr;
    cdd_c_error_t rc;

    struct_fields_init(&sf);
    g_cdd_strdup_fail = 1;
    rc = parse_struct_member_line("uint32_t arr[10];", &sf);
    g_cdd_strdup_fail = 0;
    ASSERT_NEQ(CDD_C_SUCCESS, rc);
    struct_fields_free(&sf);

    enum_members_init(&em);
    arr_val = json_parse_string("[\"A\", \"B\"]");
    arr = json_value_get_array(arr_val);
    g_enum_members_add_strdup_fail = 1;
    rc = json_array_to_enum_members(arr, &em);
    g_enum_members_add_strdup_fail = 0;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    json_value_free(arr_val);
    enum_members_free(&em);
  }

  /* 18. c2s_json_object_to_struct_fields_internal allOf / anyOf / oneOf error
   * percolation */
  {
    struct StructFields sf;
    JSON_Value *val_enum =
        json_parse_string("{\"type\":\"string\",\"enum\":[\"A\",\"B\"]}");
    JSON_Object *obj_enum = json_value_get_object(val_enum);
    JSON_Value *val_allof =
        json_parse_string("{\"allOf\":[{\"type\":\"object\",\"properties\":{"
                          "\"x\":{\"type\":\"int\"}}}]}");
    JSON_Object *obj_allof = json_value_get_object(val_allof);
    JSON_Value *val_anyof =
        json_parse_string("{\"anyOf\":[{\"type\":\"object\",\"properties\":{"
                          "\"y\":{\"type\":\"string\"}}}]}");
    JSON_Object *obj_anyof = json_value_get_object(val_anyof);
    JSON_Value *val_oneof =
        json_parse_string("{\"oneOf\":[{\"type\":\"object\",\"properties\":{"
                          "\"z\":{\"type\":\"boolean\"}}}]}");
    JSON_Object *obj_oneof = json_value_get_object(val_oneof);
    cdd_c_error_t rc;

    /* enum_members_init fail */
    struct_fields_init(&sf);
    g_enum_members_init_fail = 1;
    rc = c2s_json_object_to_struct_fields_internal(obj_enum, &sf, NULL,
                                                   "EnumTest", 1);
    g_enum_members_init_fail = 0;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    struct_fields_free(&sf);

    /* json_array_to_enum_members fail */
    struct_fields_init(&sf);
    g_enum_members_add_strdup_fail = 1;
    rc = c2s_json_object_to_struct_fields_internal(obj_enum, &sf, NULL,
                                                   "EnumTest", 1);
    g_enum_members_add_strdup_fail = 0;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    struct_fields_free(&sf);

    /* allOf fail with g_struct_fields_init_fail */
    struct_fields_init(&sf);
    g_struct_fields_init_fail = 1;
    rc = c2s_json_object_to_struct_fields_internal(obj_allof, &sf, NULL,
                                                   "AllOfTest", 1);
    g_struct_fields_init_fail = 0;
    ASSERT_NEQ(CDD_C_SUCCESS, rc);
    struct_fields_free(&sf);

    /* anyOf ex fail with g_c2s_helper_fail = 3 */
    struct_fields_init(&sf);
    g_c2s_helper_fail = 3;
    rc = c2s_json_object_to_struct_fields_internal(obj_anyof, &sf, NULL,
                                                   "AnyOfTest", 1);
    g_c2s_helper_fail = 0;
    ASSERT_NEQ(CDD_C_SUCCESS, rc);
    struct_fields_free(&sf);

    /* anyOf fallback fail with g_struct_fields_init_fail */
    struct_fields_init(&sf);
    g_struct_fields_init_fail = 1;
    rc = c2s_json_object_to_struct_fields_internal(obj_anyof, &sf, NULL,
                                                   "AnyOfTest", 0);
    g_struct_fields_init_fail = 0;
    ASSERT_NEQ(CDD_C_SUCCESS, rc);
    struct_fields_free(&sf);

    /* oneOf fallback fail with g_struct_fields_init_fail */
    struct_fields_init(&sf);
    g_struct_fields_init_fail = 1;
    rc = c2s_json_object_to_struct_fields_internal(obj_oneof, &sf, NULL,
                                                   "OneOfTest", 0);
    g_struct_fields_init_fail = 0;
    ASSERT_NEQ(CDD_C_SUCCESS, rc);
    struct_fields_free(&sf);

    json_value_free(val_enum);
    json_value_free(val_allof);
    json_value_free(val_anyof);
    json_value_free(val_oneof);
  }

  /* 19. Helpers error paths */
  {
    struct StructFields dest;
    char *out_val = NULL;
    char *out_name = NULL;
    cdd_c_error_t rc;

    /* make_unique_variant_name sanitize error */
    struct_fields_init(&dest);
    g_c2s_helper_fail = 1;
    rc = make_unique_variant_name(&dest, "variant", 0, &out_val);
    g_c2s_helper_fail = 0;
    ASSERT_NEQ(CDD_C_SUCCESS, rc);

    /* make_unique_variant_name strdup error */
    struct_fields_add(&dest, "variant", "string", NULL, NULL, NULL);
    g_cdd_strdup_fail = 1;
    rc = make_unique_variant_name(&dest, "variant", 0, &out_val);
    g_cdd_strdup_fail = 0;
    ASSERT_NEQ(CDD_C_SUCCESS, rc);
    struct_fields_free(&dest);

    /* make_inline_schema_name error */
    g_c2s_helper_fail = 1;
    rc = make_inline_schema_name("Schema", "Variant", "Suffix", &out_name);
    g_c2s_helper_fail = 0;
    ASSERT_NEQ(CDD_C_SUCCESS, rc);

    /* register_inline_schema_c2s clone error */
    {
      JSON_Value *root_val = json_value_init_object();
      JSON_Object *root = json_value_get_object(root_val);
      JSON_Value *sch_val = json_parse_string("{\"type\":\"string\"}");
      g_c2s_helper_fail = 1;
      rc = register_inline_schema_c2s(root, "Schema", "Variant", "Suffix",
                                      sch_val, &out_name);
      g_c2s_helper_fail = 0;
      ASSERT_NEQ(CDD_C_SUCCESS, rc);
      json_value_free(sch_val);
      json_value_free(root_val);
    }

    /* discriminator_value_for_variant strdup error on ref_name */
    {
      JSON_Value *d_val = json_parse_string("{\"mapping\":{}}");
      JSON_Object *d_obj = json_value_get_object(d_val);
      g_cdd_strdup_fail = 1;
      rc = discriminator_value_for_variant(
          d_obj, NULL, "#/components/schemas/MyRef", &out_val);
      g_cdd_strdup_fail = 0;
      ASSERT_NEQ(CDD_C_SUCCESS, rc);
      json_value_free(d_val);
    }
  }

  /* 20. c2s_parse_union_and_write edge cases */
  {
    FILE *fp;
    JSON_Value *root_val = json_value_init_object();
    JSON_Object *root = json_value_get_object(root_val);
    cdd_c_error_t rc;

    fp = tmpfile();
    if (fp) {
      fprintf(fp, "int a;\n}\n");
      rewind(fp);
      g_c2s_helper_fail = 1;
      rc = c2s_parse_union_and_write(fp, root, "MyUnion");
      g_c2s_helper_fail = 0;
      ASSERT_NEQ(CDD_C_SUCCESS, rc);
      fclose(fp);
    }
    json_value_free(root_val);
  }

  /* 21. Additional failure coverage tests */
  {
    /* c2s_collect_string_array strdup error */
    {
      char **out_arr = NULL;
      size_t out_cnt = 0;
      JSON_Value *v = json_parse_string("[\"item1\"]");
      JSON_Array *a = json_value_get_array(v);
      cdd_c_error_t rc;
      g_cdd_strdup_fail = 1;
      rc = c2s_collect_string_array(a, &out_arr, &out_cnt);
      g_cdd_strdup_fail = 0;
      ASSERT_NEQ(CDD_C_SUCCESS, rc);
      json_value_free(v);
    }

    /* c2s_collect_property_names strdup error */
    {
      char **out_arr = NULL;
      size_t out_cnt = 0;
      JSON_Value *v = json_parse_string(
          "{\"properties\":{\"prop1\":{\"type\":\"string\"} } }");
      JSON_Object *o = json_value_get_object(v);
      cdd_c_error_t rc;
      g_cdd_strdup_fail = 1;
      rc = c2s_collect_property_names(o, &out_arr, &out_cnt);
      g_cdd_strdup_fail = 0;
      ASSERT_NEQ(CDD_C_SUCCESS, rc);
      json_value_free(v);
    }

    /* merge_struct_fields struct_fields_add and strdup error */
    {
      struct StructFields src, dest;
      cdd_c_error_t rc;
      struct_fields_init(&src);
      struct_fields_init(&dest);
      struct_fields_add(&src, "fld", "string", NULL, NULL, NULL);
      c_cdd_strdup("{\"x\":1}", &src.fields[0].schema_extra_json);

      /* struct_fields_add failure in merge */
      g_cdd_fail_struct_fields_add = 1;
      rc = merge_struct_fields(&dest, &src);
      g_cdd_fail_struct_fields_add = 0;
      ASSERT_NEQ(CDD_C_SUCCESS, rc);

      /* strdup failure on schema_extra_json in merge */
      g_cdd_strdup_fail = 1;
      rc = merge_struct_fields(&dest, &src);
      g_cdd_strdup_fail = 0;
      ASSERT_NEQ(CDD_C_SUCCESS, rc);

      struct_fields_free(&src);
      struct_fields_free(&dest);
    }

    /* apply_union_to_struct_fields_ex variant struct_fields_add error */
    {
      struct StructFields dest;
      JSON_Value *val = json_parse_string("[{\"type\":\"string\"}]");
      JSON_Array *arr = json_value_get_array(val);
      JSON_Value *root_val = json_value_init_object();
      JSON_Object *root = json_value_get_object(root_val);
      cdd_c_error_t rc;

      struct_fields_init(&dest);
      g_cdd_fail_struct_fields_add = 1;
      rc = apply_union_to_struct_fields_ex(arr, &dest, root, "TestUnionFldAdd",
                                           0, NULL, 1);
      g_cdd_fail_struct_fields_add = 0;
      ASSERT_NEQ(CDD_C_SUCCESS, rc);

      struct_fields_free(&dest);
      json_value_free(val);
      json_value_free(root_val);
    }

    /* parse_struct_member_line error percolation in code2schema_main */
    {
      FILE *fp = fopen("test_err_main.h", "w");
      char *argv[3];
      cdd_c_error_t rc;
      if (fp) {
        fprintf(fp, "struct BadStruct {\n  uint32_t bad_field[10];\n};\n");
        fclose(fp);

        argv[0] = C_CDD_STR_LIT("test_err_main.h");
        argv[1] = C_CDD_STR_LIT("test_err_main.json");
        argv[2] = NULL;

        g_c2s_helper_fail = 4;
        rc = code2schema_main(2, argv);
        g_c2s_helper_fail = 0;
        ASSERT_NEQ(CDD_C_SUCCESS, rc);

        remove("test_err_main.h");
        remove("test_err_main.json");
      }
    }
  }

  /* 22. c2s_write_default_value parser failure paths */
  {
    JSON_Value *val = json_value_init_object();
    JSON_Object *obj = json_value_get_object(val);
    struct StructField f_str, f_bool, f_num;
    cdd_c_error_t rc;

    memset(&f_str, 0, sizeof(f_str));
    CDD_STRCPY(f_str.type, sizeof(f_str.type), "string");
    CDD_STRCPY(f_str.default_val, sizeof(f_str.default_val), "\"hello\"");

    memset(&f_bool, 0, sizeof(f_bool));
    CDD_STRCPY(f_bool.type, sizeof(f_bool.type), "boolean");
    CDD_STRCPY(f_bool.default_val, sizeof(f_bool.default_val), "true");

    memset(&f_num, 0, sizeof(f_num));
    CDD_STRCPY(f_num.type, sizeof(f_num.type), "integer");
    CDD_STRCPY(f_num.default_val, sizeof(f_num.default_val), "42");

    /* c2s_strip_quotes fail */
    g_c2s_helper_fail = 2;
    rc = c2s_write_default_value(obj, &f_str);
    g_c2s_helper_fail = 0;
    ASSERT_NEQ(CDD_C_SUCCESS, rc);

    /* c2s_parse_bool_default fail */
    g_c2s_helper_fail = 2;
    rc = c2s_write_default_value(obj, &f_bool);
    g_c2s_helper_fail = 0;
    ASSERT_NEQ(CDD_C_SUCCESS, rc);

    /* c2s_parse_number_default fail */
    g_c2s_helper_fail = 2;
    rc = c2s_write_default_value(obj, &f_num);
    g_c2s_helper_fail = 0;
    ASSERT_NEQ(CDD_C_SUCCESS, rc);

    json_value_free(val);
  }

  /* 23. c2s_parse_union_and_write error percolation in code2schema_main */
  {
    FILE *fp = fopen("test_union_err.h", "w");
    char *argv[3];
    cdd_c_error_t rc;
    if (fp) {
      fprintf(fp, "union FailUnion {\n  int x;\n};\n");
      fclose(fp);

      argv[0] = C_CDD_STR_LIT("test_union_err.h");
      argv[1] = C_CDD_STR_LIT("test_union_err.json");
      argv[2] = NULL;

      g_c2s_helper_fail = 2;
      rc = code2schema_main(2, argv);
      g_c2s_helper_fail = 0;
      ASSERT_NEQ(CDD_C_SUCCESS, rc);

      remove("test_union_err.h");
      remove("test_union_err.json");
    }
  }

  /* 24. Final targeted branch coverage tests */
  {
    const char *out_s = NULL;
    int has_bval = 0;
    cdd_c_error_t rc;

    /* c2s_strip_quotes null input */
    rc = c2s_strip_quotes(NULL, NULL, 0, &out_s);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    /* c2s_parse_bool_default null input */
    rc = c2s_parse_bool_default(NULL, NULL, &has_bval);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    /* resolve_schema_ref_object str_after_last error */
    {
      JSON_Value *rv = json_value_init_object();
      JSON_Object *ro = json_value_get_object(rv);
      JSON_Object *out_obj = NULL;
      g_cdd_fail_str_after_last = 1;
      rc = resolve_schema_ref_object(ro, "#/components/schemas/X", &out_obj);
      g_cdd_fail_str_after_last = 0;
      ASSERT_NEQ(CDD_C_SUCCESS, rc);
      json_value_free(rv);
    }

    /* make_unique_variant_name double collision and strdup fail */
    {
      struct StructFields dest;
      char *out_val = NULL;
      struct_fields_init(&dest);
      struct_fields_add(&dest, "variant", "string", NULL, NULL, NULL);
      struct_fields_add(&dest, "variant_1", "string", NULL, NULL, NULL);
      g_cdd_strdup_fail = 1;
      rc = make_unique_variant_name(&dest, "variant", 0, &out_val);
      g_cdd_strdup_fail = 0;
      ASSERT_NEQ(CDD_C_SUCCESS, rc);
      struct_fields_free(&dest);
    }

    /* merge_struct_fields items_extra_json strdup error */
    {
      struct StructFields src, dest;
      struct_fields_init(&src);
      struct_fields_init(&dest);
      struct_fields_add(&src, "arr_fld", "array", "string", NULL, NULL);
      c_cdd_strdup("{\"format\":\"int32\"}", &src.fields[0].items_extra_json);
      g_cdd_strdup_fail = 1;
      rc = merge_struct_fields(&dest, &src);
      g_cdd_strdup_fail = 0;
      ASSERT_NEQ(CDD_C_SUCCESS, rc);
      struct_fields_free(&src);
      struct_fields_free(&dest);
    }

    /* apply_union_to_struct_fields_ex array with allow_inline = 0 */
    {
      struct StructFields dest;
      JSON_Value *val = json_parse_string(
          "[{\"type\":\"array\",\"items\":{\"type\":\"string\"}}]");
      JSON_Array *arr = json_value_get_array(val);
      JSON_Value *root_val = json_value_init_object();
      JSON_Object *root = json_value_get_object(root_val);

      struct_fields_init(&dest);
      rc = apply_union_to_struct_fields_ex(arr, &dest, root, "TestArrNoInline2",
                                           0, NULL, 0);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      ASSERT_EQ(0, dest.is_union);

      struct_fields_free(&dest);
      json_value_free(val);
      json_value_free(root_val);
    }

    /* apply_union_to_struct_fields_ex discriminator error via str_after_last */
    {
      struct StructFields dest;
      JSON_Value *val =
          json_parse_string("{\"discriminator\":{\"propertyName\":\"type\"},"
                            "\"oneOf\":[{\"$ref\":\"#/components/schemas/"
                            "A\"},{\"$ref\":\"#/components/schemas/B\"}]}");
      JSON_Object *obj = json_value_get_object(val);
      JSON_Array *oneof_arr = json_object_get_array(obj, "oneOf");
      JSON_Value *root_val = json_value_init_object();
      JSON_Object *root = json_value_get_object(root_val);

      struct_fields_init(&dest);
      g_cdd_fail_str_after_last = 1;
      rc = apply_union_to_struct_fields_ex(oneof_arr, &dest, root,
                                           "TestDiscErr", 0, obj, 1);
      g_cdd_fail_str_after_last = 0;
      ASSERT_NEQ(CDD_C_SUCCESS, rc);

      struct_fields_free(&dest);
      json_value_free(val);
      json_value_free(root_val);
    }
  }

  /* 25. Direct helper fail hook tests */
  {
    const char *out_s = NULL;
    char buf[64];
    int b = 0, has_b = 0;
    double n = 0;
    int has_n = 0;
    JSON_Value *val = json_value_init_object();
    JSON_Object *obj = json_value_get_object(val);
    JSON_Object *out_obj = NULL;
    struct StructField fld;
    struct StructFields sf;
    cdd_c_error_t rc;

    memset(&fld, 0, sizeof(fld));
    struct_fields_init(&sf);

    g_c2s_helper_fail = 1;
    rc = c2s_strip_quotes("abc", buf, sizeof(buf), &out_s);
    g_c2s_helper_fail = 0;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

    g_c2s_helper_fail = 1;
    rc = c2s_parse_bool_default("true", &b, &has_b);
    g_c2s_helper_fail = 0;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

    g_c2s_helper_fail = 1;
    rc = c2s_parse_number_default("42", &n, &has_n);
    g_c2s_helper_fail = 0;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

    g_c2s_helper_fail = 1;
    rc = resolve_schema_ref_object(obj, "#/components/schemas/A", &out_obj);
    g_c2s_helper_fail = 0;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

    g_c2s_helper_fail = 1;
    rc = c2s_write_type_union(obj, "string", NULL, 0);
    g_c2s_helper_fail = 0;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

    g_c2s_helper_fail = 1;
    rc = c2s_write_numeric_constraints(obj, &fld);
    g_c2s_helper_fail = 0;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

    g_c2s_helper_fail = 1;
    rc = c2s_write_string_constraints(obj, &fld);
    g_c2s_helper_fail = 0;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

    g_c2s_helper_fail = 1;
    rc = c2s_write_array_constraints(obj, &fld);
    g_c2s_helper_fail = 0;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

    g_c2s_helper_fail = 1;
    rc = write_struct_to_json_schema(obj, "TestSt", &sf);
    g_c2s_helper_fail = 0;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

    g_c2s_helper_fail = 1;
    rc = c2s_collapse_arrays(&sf);
    g_c2s_helper_fail = 0;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

    struct_fields_free(&sf);
    json_value_free(val);
  }

  /* 26. struct_fields_get failure and make_unique_variant_name coverage */
  {
    struct StructFields dest, src;
    char *out_val = NULL;
    cdd_c_error_t rc;

    struct_fields_init(&dest);
    struct_fields_add(&dest, "variant", "string", NULL, NULL, NULL);
    struct_fields_add(&dest, "variant_1", "string", NULL, NULL, NULL);

    /* struct_fields_get failure in make_unique_variant_name call 1 */
    g_cdd_fail_struct_fields_get = 1;
    rc = make_unique_variant_name(&dest, "variant", 0, &out_val);
    g_cdd_fail_struct_fields_get = 0;
    ASSERT_NEQ(CDD_C_SUCCESS, rc);

    /* struct_fields_get failure in make_unique_variant_name call 2 */
    g_cdd_fail_struct_fields_get = 2;
    rc = make_unique_variant_name(&dest, "variant", 0, &out_val);
    g_cdd_fail_struct_fields_get = 0;
    ASSERT_NEQ(CDD_C_SUCCESS, rc);

    /* strdup failure on fallback in make_unique_variant_name */
    g_cdd_strdup_fail = 2;
    rc = make_unique_variant_name(&dest, "variant", 0, &out_val);
    g_cdd_strdup_fail = 0;
    ASSERT_NEQ(CDD_C_SUCCESS, rc);

    /* struct_fields_get failure in merge_struct_fields */
    struct_fields_init(&src);
    struct_fields_add(&src, "fld", "string", NULL, NULL, NULL);
    g_cdd_fail_struct_fields_get = 1;
    rc = merge_struct_fields(&dest, &src);
    g_cdd_fail_struct_fields_get = 0;
    ASSERT_NEQ(CDD_C_SUCCESS, rc);

    struct_fields_free(&src);
    struct_fields_free(&dest);
  }

  /* 27. apply_union_to_struct_fields_ex comprehensive failure loop */
  {
    int k;
    cdd_c_error_t rc;
    for (k = 1; k <= 25; ++k) {
      struct StructFields dest;
      JSON_Value *val = json_parse_string(
          "{\"discriminator\":{\"propertyName\":\"kind\"},"
          "\"oneOf\":["
          "  {\"$ref\":\"#/components/schemas/MyRef\"},"
          "  "
          "{\"type\":\"object\",\"properties\":{\"p\":{\"type\":\"string\"}}},"
          "  {\"type\":\"array\",\"items\":{\"type\":\"string\"}},"
          "  "
          "{\"type\":\"array\",\"items\":{\"properties\":{\"p2\":{\"type\":"
          "\"integer\"}}}},"
          "  {\"type\":\"string\"}"
          "]}");
      JSON_Object *obj = json_value_get_object(val);
      JSON_Array *oneof_arr = json_object_get_array(obj, "oneOf");
      JSON_Value *root_val =
          json_parse_string("{\"components\":{\"schemas\":{\"MyRef\":{\"type\":"
                            "\"string\",\"enum\":[\"A\"]}}}}");
      JSON_Object *root = json_value_get_object(root_val);

      struct_fields_init(&dest);
      g_c2s_helper_fail = k;
      rc = apply_union_to_struct_fields_ex(oneof_arr, &dest, root, "LoopUnion",
                                           0, obj, 1);
      g_c2s_helper_fail = 0;
      if (rc != CDD_C_SUCCESS) { /* expected failure */
      }

      struct_fields_free(&dest);
      json_value_free(val);
      json_value_free(root_val);
    }
  }

  /* 28. Additional coverage for merge_extras, merge_struct_field and
   * discriminator */
  {
    cdd_c_error_t rc;
    char *out_val = NULL;

    /* discriminator fallback strdup failure */
    g_cdd_strdup_fail = 1;
    rc =
        discriminator_value_for_variant(NULL, "FallbackSchema", NULL, &out_val);
    g_cdd_strdup_fail = 0;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);

    /* parse_struct_member_line struct_fields_add error (line 1786) */
    {
      struct StructFields sf;
      struct_fields_init(&sf);
      g_cdd_fail_struct_fields_add = 1;
      rc = parse_struct_member_line("int x;", &sf);
      g_cdd_fail_struct_fields_add = 0;
      ASSERT_NEQ(CDD_C_SUCCESS, rc);
      struct_fields_free(&sf);
    }

    /* merge_struct_fields with merge_struct_field failure */
    {
      struct StructFields src, dest;
      struct_fields_init(&src);
      struct_fields_init(&dest);
      struct_fields_add(&src, "shared_fld", "string", NULL, NULL, NULL);
      struct_fields_add(&dest, "shared_fld", "string", NULL, NULL, NULL);

      g_c2s_helper_fail = 2;
      rc = merge_struct_fields(&dest, &src);
      g_c2s_helper_fail = 0;
      ASSERT_NEQ(CDD_C_SUCCESS, rc);

      struct_fields_free(&src);
      struct_fields_free(&dest);
    }

    /* c2s_json_object_to_struct_fields_internal with items and prop extras
     * error */
    {
      struct StructFields sf;
      JSON_Value *v =
          json_parse_string("{\"type\":\"object\",\"properties\":{"
                            "  "
                            "\"arr\":{\"type\":\"array\",\"items\":{\"type\":"
                            "\"string\",\"x-item\":1}},"
                            "  \"p\":{\"type\":\"string\",\"x-prop\":2}"
                            "}}");
      JSON_Object *o = json_value_get_object(v);

      /* fail on items extras */
      struct_fields_init(&sf);
      g_c2s_helper_fail = 4;
      rc = c2s_json_object_to_struct_fields_internal(o, &sf, NULL, "ExtrasFail",
                                                     0);
      g_c2s_helper_fail = 0;
      ASSERT_NEQ(CDD_C_SUCCESS, rc);
      struct_fields_free(&sf);

      /* fail on prop extras */
      struct_fields_init(&sf);
      g_c2s_helper_fail = 3;
      rc = c2s_json_object_to_struct_fields_internal(o, &sf, NULL, "ExtrasFail",
                                                     0);
      g_c2s_helper_fail = 0;
      ASSERT_NEQ(CDD_C_SUCCESS, rc);
      struct_fields_free(&sf);

      json_value_free(v);
    }
  }

  /* 29. allOf and union fallback failure with invalid sub-schemas */
  {
    struct StructFields dest;
    JSON_Value *val_bad = json_parse_string(
        "{\"allOf\":[{\"type\":\"object\",\"properties\":{\"p\":{\"type\":"
        "\"array\",\"items\":{\"$ref\":\"#/components/schemas/Bad\"}}}}]}");
    JSON_Object *obj_bad = json_value_get_object(val_bad);
    JSON_Value *val_root = json_value_init_object();
    JSON_Object *root = json_value_get_object(val_root);
    cdd_c_error_t rc;

    /* allOf fails because sub-schema fails on unresolved ref with
     * fail_str_after_last */
    struct_fields_init(&dest);
    g_cdd_fail_str_after_last = 1;
    rc = c2s_json_object_to_struct_fields_internal(obj_bad, &dest, root,
                                                   "BadAllOf", 0);
    g_cdd_fail_str_after_last = 0;
    if (rc != CDD_C_SUCCESS) { /* expected failure */
    }
    struct_fields_free(&dest);

    /* oneOf fallback fails similarly */
    {
      JSON_Value *val_oneof_bad = json_parse_string(
          "{\"oneOf\":[{\"type\":\"object\",\"properties\":{\"p\":{\"type\":"
          "\"array\",\"items\":{\"$ref\":\"#/components/schemas/Bad\"}}}}]}");
      JSON_Object *obj_oneof_bad = json_value_get_object(val_oneof_bad);

      struct_fields_init(&dest);
      g_cdd_fail_str_after_last = 1;
      rc = c2s_json_object_to_struct_fields_internal(obj_oneof_bad, &dest, root,
                                                     "BadOneOf", 0);
      g_cdd_fail_str_after_last = 0;
      if (rc != CDD_C_SUCCESS) { /* expected failure */
      }
      struct_fields_free(&dest);

      json_value_free(val_oneof_bad);
    }

    json_value_free(val_bad);
    json_value_free(val_root);
  }

  /* 30. Exact branch coverage for apply_union_to_struct_fields_ex */
  {
    /* 1. sub == NULL in loop 2 (42 in array) */
    {
      struct StructFields dest;
      JSON_Value *val = json_parse_string("[{\"type\":\"string\"}, 42]");
      JSON_Array *arr = json_value_get_array(val);
      JSON_Value *root_val = json_value_init_object();
      JSON_Object *root = json_value_get_object(root_val);
      cdd_c_error_t rc;

      struct_fields_init(&dest);
      rc = apply_union_to_struct_fields_ex(arr, &dest, root, "TestSubNull", 0,
                                           NULL, 1);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      ASSERT_EQ(1, dest.is_union);

      struct_fields_free(&dest);
      json_value_free(val);
      json_value_free(root_val);
    }

    /* 2. name_hint fallback (object with properties but no type/title) */
    {
      struct StructFields dest;
      JSON_Value *val =
          json_parse_string("[{\"properties\":{\"x\":{\"type\":\"string\"}}}]");
      JSON_Array *arr = json_value_get_array(val);
      JSON_Value *root_val = json_value_init_object();
      JSON_Object *root = json_value_get_object(root_val);
      cdd_c_error_t rc;

      struct_fields_init(&dest);
      rc = apply_union_to_struct_fields_ex(arr, &dest, root, "FallbackHint", 0,
                                           NULL, 1);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      ASSERT_EQ(1, dest.is_union);

      struct_fields_free(&dest);
      json_value_free(val);
      json_value_free(root_val);
    }

    /* 3. default branch in switch(jtype) (boolean variant) */
    {
      struct StructFields dest;
      JSON_Value *val =
          json_parse_string("[{\"type\":\"boolean\"}, {\"type\":\"string\"}]");
      JSON_Array *arr = json_value_get_array(val);
      JSON_Value *root_val = json_value_init_object();
      JSON_Object *root = json_value_get_object(root_val);
      cdd_c_error_t rc;

      struct_fields_init(&dest);
      rc = apply_union_to_struct_fields_ex(arr, &dest, root,
                                           "TestDefaultSwitch", 0, NULL, 1);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      ASSERT_EQ(1, dest.is_union);

      struct_fields_free(&dest);
      json_value_free(val);
      json_value_free(root_val);
    }

    /* 4. unsupported array items (!supported in loop 1) */
    {
      struct StructFields dest;
      JSON_Value *val = json_parse_string(
          "[{\"type\":\"array\",\"items\":{\"type\":\"array\"}}]");
      JSON_Array *arr = json_value_get_array(val);
      JSON_Value *root_val = json_value_init_object();
      JSON_Object *root = json_value_get_object(root_val);
      cdd_c_error_t rc;

      struct_fields_init(&dest);
      rc = apply_union_to_struct_fields_ex(arr, &dest, root, "TestUnsupported",
                                           0, NULL, 1);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      ASSERT_EQ(0, dest.is_union);

      struct_fields_free(&dest);
      json_value_free(val);
      json_value_free(root_val);
    }
  }

  /* 31. Direct tests for remaining missing error branches */
  {
    char *dest = NULL;
    char *out_val = NULL;
    struct StructFields sf;
    cdd_c_error_t rc;

    /* discriminator line 4228: str_after_last failure when ref is present */
    {
      JSON_Value *d_val = json_parse_string("{\"mapping\":{}}");
      JSON_Object *d_obj = json_value_get_object(d_val);
      g_cdd_fail_str_after_last = 1;
      rc = discriminator_value_for_variant(d_obj, "MySchema",
                                           "#/components/schemas/A", &out_val);
      g_cdd_fail_str_after_last = 0;
      ASSERT_NEQ(CDD_C_SUCCESS, rc);
      json_value_free(d_val);
    }

    /* c2s_merge_schema_extras_strings: json_set_value error */
    c_cdd_strdup("{\"a\":1}", &dest);
    g_cdd_fail_json_set_value = 1;
    rc = c2s_merge_schema_extras_strings(&dest, "{\"b\":2}");
    g_cdd_fail_json_set_value = 0;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    C_CDD_FREE(dest);

    /* c2s_merge_schema_extras_strings: json_serialize error */
    c_cdd_strdup("{\"a\":1}", &dest);
    g_cdd_fail_json_serialize = 1;
    rc = c2s_merge_schema_extras_strings(&dest, "{\"b\":2}");
    g_cdd_fail_json_serialize = 0;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    C_CDD_FREE(dest);

    /* c2s_json_object_to_struct_fields_internal: items extras error (line 2185)
     */
    {
      JSON_Value *v = json_parse_string("{\"type\":\"object\",\"properties\":{"
                                        "  "
                                        "\"arr\":{\"type\":\"array\",\"items\":"
                                        "{\"type\":\"string\",\"x-item\":1}}"
                                        "}}");
      JSON_Object *o = json_value_get_object(v);

      struct_fields_init(&sf);
      g_cdd_fail_c2s_collect_schema_extras = 2;
      rc = c2s_json_object_to_struct_fields_internal(o, &sf, NULL,
                                                     "ExtrasFail2", 0);
      g_cdd_fail_c2s_collect_schema_extras = 0;
      ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
      struct_fields_free(&sf);
      json_value_free(v);
    }

    /* c2s_json_object_to_struct_fields_internal: prop extras error (line 2358)
     */
    {
      JSON_Value *v =
          json_parse_string("{\"type\":\"object\",\"properties\":{"
                            "  \"p\":{\"type\":\"string\",\"x-prop\":2}"
                            "}}");
      JSON_Object *o = json_value_get_object(v);

      struct_fields_init(&sf);
      g_cdd_fail_c2s_collect_schema_extras = 2;
      rc = c2s_json_object_to_struct_fields_internal(o, &sf, NULL,
                                                     "ExtrasFail3", 0);
      g_cdd_fail_c2s_collect_schema_extras = 0;
      ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
      struct_fields_free(&sf);
      json_value_free(v);
    }
  }

  /* 32. 100% line completion tests */
  {
    cdd_c_error_t rc;

    /* 1. c2s_clone_json_value g_cdd_fail_json_serialize */
    {
      JSON_Value *v = json_parse_string("{\"a\":1}");
      JSON_Value *c = NULL;
      g_cdd_fail_json_serialize = 1;
      rc = c2s_clone_json_value(v, &c);
      g_cdd_fail_json_serialize = 0;
      ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
      json_value_free(v);
    }

    /* 2. c2s_merge_schema_extras_strings json_serialize failure with empty
     * objects */
    {
      char *dest = NULL;
      c_cdd_strdup("{}", &dest);
      g_cdd_fail_json_serialize = 1;
      rc = c2s_merge_schema_extras_strings(&dest, "{}");
      g_cdd_fail_json_serialize = 0;
      ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
      C_CDD_FREE(dest);
    }

    /* 3. copy_string_array_code2schema direct fail hook */
    {
      char *src_arr[1];
      char **out_arr = NULL;
      size_t out_cnt = 0;
      src_arr[0] = C_CDD_STR_LIT("a");
      g_c2s_helper_fail = 1;
      rc = copy_string_array_code2schema(&out_arr, &out_cnt, src_arr, 1);
      g_c2s_helper_fail = 0;
      ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    }

    /* 4. parse_type_union_array_code2schema direct fail hook */
    {
      JSON_Value *v = json_parse_string("[\"string\"]");
      JSON_Array *arr = json_value_get_array(v);
      char **out_arr = NULL;
      size_t out_cnt = 0;
      const char *primary = NULL;
      g_c2s_helper_fail = 1;
      rc = parse_type_union_array_code2schema(arr, &out_arr, &out_cnt, &primary,
                                              NULL);
      g_c2s_helper_fail = 0;
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
      json_value_free(v);
    }

    /* 5. required_name_in_list direct and internal failure */
    {
      JSON_Value *av = json_parse_string("[\"fld\"]");
      JSON_Array *arr = json_value_get_array(av);
      int in_list = 0;
      g_c2s_helper_fail = 1;
      rc = required_name_in_list(arr, "fld", &in_list);
      g_c2s_helper_fail = 0;
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
      json_value_free(av);

      /* internal percolation in c2s_json_object_to_struct_fields_internal (Line
       * 2261) */
      {
        struct StructFields sf;
        JSON_Value *v = json_parse_string(
            "{\"type\":\"object\",\"required\":[\"fld\"],\"properties\":{"
            "\"fld\":{\"type\":\"string\"} } }");
        JSON_Object *o = json_value_get_object(v);
        int k;

        for (k = 1; k <= 5; ++k) {
          struct_fields_init(&sf);
          g_c2s_helper_fail = k;
          rc = c2s_json_object_to_struct_fields_internal(o, &sf, NULL,
                                                         "ReqFail", 0);
          g_c2s_helper_fail = 0;
          struct_fields_free(&sf);
        }
        json_value_free(v);
      }
    }

    /* 6. merge_struct_fields struct_fields_get call 2 (Line 4483) */
    {
      struct StructFields src, dest;
      struct_fields_init(&src);
      struct_fields_init(&dest);
      struct_fields_add(&src, "fld1", "string", NULL, NULL, NULL);
      g_cdd_fail_struct_fields_get = 2;
      rc = merge_struct_fields(&dest, &src);
      g_cdd_fail_struct_fields_get = 0;
      ASSERT_NEQ(CDD_C_SUCCESS, rc);
      struct_fields_free(&src);
      struct_fields_free(&dest);
    }

    /* 7. merge_struct_fields items_type_union copy failure (Line 4521) */
    {
      struct StructFields src, dest;
      char *arr_types[2];
      arr_types[0] = C_CDD_STR_LIT("string");
      arr_types[1] = C_CDD_STR_LIT("null");
      struct_fields_init(&src);
      struct_fields_init(&dest);
      struct_fields_add(&src, "fld2", "array", "string", NULL, NULL);
      copy_string_array_code2schema(&src.fields[0].items_type_union,
                                    &src.fields[0].n_items_type_union,
                                    arr_types, 2);
      g_c2s_helper_fail = 2;
      rc = merge_struct_fields(&dest, &src);
      g_c2s_helper_fail = 0;
      ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
      struct_fields_free(&src);
      struct_fields_free(&dest);
    }

    /* 8. apply_allof_to_struct_fields merge failure (Line 4764) */
    {
      struct StructFields sf;
      JSON_Value *allof_val =
          json_parse_string("[{\"type\":\"object\",\"properties\":{\"f1\":{"
                            "\"type\":\"string\"}}}]");
      JSON_Array *allof_arr = json_value_get_array(allof_val);
      struct_fields_init(&sf);
      g_cdd_fail_struct_fields_add = 2;
      rc = apply_allof_to_struct_fields(allof_arr, &sf, NULL);
      g_cdd_fail_struct_fields_add = 0;
      ASSERT_NEQ(CDD_C_SUCCESS, rc);
      struct_fields_free(&sf);
      json_value_free(allof_val);
    }

    /* 9. apply_union_to_struct_fields_ex variant error paths */
    {
      struct StructFields dest;
      JSON_Value *val = json_parse_string(
          "{\"discriminator\":{\"propertyName\":\"type\"},"
          "\"oneOf\":["
          "  {\"$ref\":\"#/components/schemas/RefA\"},"
          "  "
          "{\"type\":\"array\",\"items\":{\"$ref\":\"#/components/schemas/"
          "EnumItem\"}},"
          "  {\"type\":\"array\",\"items\":{\"type\":[\"string\",\"null\"]}}"
          "]}");
      JSON_Object *obj = json_value_get_object(val);
      JSON_Array *oneof_arr = json_object_get_array(obj, "oneOf");
      JSON_Value *root_val = json_parse_string(
          "{\"RefA\":{\"type\":\"object\",\"properties\":{\"a\":{\"type\":"
          "\"string\"}}},"
          "\"EnumItem\":{\"type\":\"string\",\"enum\":[\"E1\"]}}");
      JSON_Object *root = json_value_get_object(root_val);
      int k;

      for (k = 1; k <= 15; ++k) {
        struct_fields_init(&dest);
        g_c2s_helper_fail = k;
        rc = apply_union_to_struct_fields_ex(oneof_arr, &dest, root,
                                             "UnionExErr", 0, obj, 1);
        g_c2s_helper_fail = 0;
        struct_fields_free(&dest);
      }

      for (k = 1; k <= 10; ++k) {
        struct_fields_init(&dest);
        g_cdd_fail_str_after_last = k;
        rc = apply_union_to_struct_fields_ex(oneof_arr, &dest, root,
                                             "UnionExErr", 0, obj, 1);
        g_cdd_fail_str_after_last = 0;
        struct_fields_free(&dest);
      }

      json_value_free(val);
      json_value_free(root_val);
    }
  }

  /* 33. Comprehensive branch coverage for parameter validations and C parsing
   */
  {
    cdd_c_error_t rc;
    char *src_arr[2];
    char **out_arr = NULL;
    size_t out_cnt = 0;
    struct StructFields sf;

    src_arr[0] = C_CDD_STR_LIT("val1");
    src_arr[1] = NULL;

    /* copy_string_array_code2schema branch permutations */
    rc = copy_string_array_code2schema(NULL, &out_cnt, src_arr, 1);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = copy_string_array_code2schema(&out_arr, NULL, src_arr, 1);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = copy_string_array_code2schema(&out_arr, &out_cnt, NULL, 1);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    rc = copy_string_array_code2schema(&out_arr, &out_cnt, src_arr, 0);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    rc = copy_string_array_code2schema(&out_arr, &out_cnt, src_arr, 2);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    free_string_array_code2schema(out_arr, out_cnt);

    /* parse_type_union_array_code2schema branch permutations */
    {
      JSON_Value *v = json_parse_string("[\"null\"]");
      JSON_Array *arr = json_value_get_array(v);
      int is_null = 0;
      rc = parse_type_union_array_code2schema(arr, &out_arr, &out_cnt, NULL,
                                              &is_null);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      ASSERT_EQ(1, is_null);
      free_string_array_code2schema(out_arr, out_cnt);
      json_value_free(v);
    }

    /* parse_struct_member_line branch permutations */
    struct_fields_init(&sf);
    rc = parse_struct_member_line(NULL, &sf);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = parse_struct_member_line("int a;", NULL);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

    /* int* p without space before pointer */
    rc = parse_struct_member_line("int* p;", &sf);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    /* Flexible array member */
    rc = parse_struct_member_line("int fam[];", &sf);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    /* Flexible array member of strings */
    rc = parse_struct_member_line("char fam_str[][32];", &sf);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    /* Shard key, hash, track telemetry, and slow query */
    rc = parse_struct_member_line("int id; // @shard_key @shard_hash "
                                  "@track_telemetry @slow_query_warn(100)",
                                  &sf);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    struct_fields_free(&sf);

    /* c2s_collect_schema_extras NULL checks */
    {
      char *out_j = NULL;
      JSON_Value *v = json_parse_string("{\"a\":1}");
      JSON_Object *o = json_value_get_object(v);
      rc = c2s_collect_schema_extras(NULL, NULL, 0, &out_j);
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
      rc = c2s_collect_schema_extras(o, NULL, 0, NULL);
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
      json_value_free(v);
    }

    /* c2s_json_object_to_struct_fields_internal NULL checks */
    struct_fields_init(&sf);
    rc = c2s_json_object_to_struct_fields_internal(NULL, &sf, NULL, "A", 0);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    {
      JSON_Value *v = json_parse_string("{\"type\":\"object\"}");
      JSON_Object *o = json_value_get_object(v);
      rc = c2s_json_object_to_struct_fields_internal(o, NULL, NULL, "A", 0);
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
      json_value_free(v);
    }
    struct_fields_free(&sf);
  }

  /* 34. Exhaustive branch coverage for remaining untaken conditions */
  {
    cdd_c_error_t rc;

    /* str_starts_with with NULL _out_val */
    rc = str_starts_with("abc", "a", NULL);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    /* parse_type_union_array_code2schema with NULL out pointers */
    {
      JSON_Value *v = json_parse_string("[\"string\", \"null\"]");
      JSON_Array *arr = json_value_get_array(v);
      rc = parse_type_union_array_code2schema(arr, NULL, NULL, NULL, NULL);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      json_value_free(v);
    }

    /* c2s_merge_schema_extras_object with existing key collision */
    {
      JSON_Value *val = json_parse_string("{\"x-existing\":1}");
      JSON_Object *target = json_value_get_object(val);
      rc = c2s_merge_schema_extras_object(target,
                                          "{\"x-existing\":2,\"x-new\":3}");
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      json_value_free(val);
    }

    /* c2s_merge_schema_extras_strings with empty src_json */
    {
      char *dest = NULL;
      c_cdd_strdup("{}", &dest);
      rc = c2s_merge_schema_extras_strings(&dest, "");
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      C_CDD_FREE(dest);
    }

    /* parse_struct_member_line variations */
    {
      struct StructFields sf;
      struct_fields_init(&sf);

      /* Bit-width with spaces after colon */
      rc = parse_struct_member_line("int width :   5;", &sf);
      ASSERT_EQ(CDD_C_SUCCESS, rc);

      /* Array of objects */
      rc = parse_struct_member_line("struct SubObj items[5];", &sf);
      ASSERT_EQ(CDD_C_SUCCESS, rc);

      /* Array of primitives without format */
      rc = parse_struct_member_line("int plain_items[5];", &sf);
      ASSERT_EQ(CDD_C_SUCCESS, rc);

      /* Individual annotations */
      rc = parse_struct_member_line("int a; // @shard_key", &sf);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      rc = parse_struct_member_line("int b; // @shard_hash", &sf);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      rc = parse_struct_member_line("int c; // @track_telemetry", &sf);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      rc = parse_struct_member_line("int d; // @slow_query_warn(50)", &sf);
      ASSERT_EQ(CDD_C_SUCCESS, rc);

      /* FAM variations */
      rc = parse_struct_member_line("int fam_int[];", &sf);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      rc = parse_struct_member_line("char fam_char[];", &sf);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      rc = parse_struct_member_line("struct SubObj fam_obj[];", &sf);
      ASSERT_EQ(CDD_C_SUCCESS, rc);

      struct_fields_free(&sf);
    }

    /* c2s_json_object_to_struct_fields_internal root == NULL variations */
    {
      struct StructFields sf;
      JSON_Value *v = json_parse_string(
          "{\"type\":\"object\",\"properties\":{"
          "  "
          "\"arr_ref\":{\"type\":\"array\",\"items\":{\"$ref\":\"#/components/"
          "schemas/Item\"}},"
          "  "
          "\"arr_anon\":{\"type\":\"array\",\"items\":{\"properties\":{\"sub\":"
          "{\"type\":\"string\"}}}},"
          "  \"dir_ref\":{\"$ref\":\"#/components/schemas/Item\"},"
          "  \"int_prop\":{\"type\":\"integer\",\"minimum\":1}"
          "}}");
      JSON_Object *o = json_value_get_object(v);

      struct_fields_init(&sf);
      rc = c2s_json_object_to_struct_fields_internal(o, &sf, NULL,
                                                     "NullRootTest", 0);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      struct_fields_free(&sf);

      json_value_free(v);
    }
  }

  PASS();
}

/**
 * @brief Exhaustive branch testing for merge_struct_field.
 */
TEST test_code2schema_merge_struct_field_exhaustive(void) {
  struct StructField dest, src;
  cdd_c_error_t rc;

  memset(&dest, 0, sizeof(dest));
  memset(&src, 0, sizeof(src));

  /* Invalid args */
  rc = merge_struct_field(NULL, &src);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = merge_struct_field(&dest, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  /* Default val, desc, format, pattern when dest empty vs dest non-empty */
  memset(&dest, 0, sizeof(dest));
  memset(&src, 0, sizeof(src));
  CDD_STRCPY(src.default_val, sizeof(src.default_val), "def1");
  CDD_STRCPY(src.pattern, sizeof(src.pattern), "pat1");
  CDD_STRCPY(src.bit_width, sizeof(src.bit_width), "4");
  src.required = 1;
  src.unique_items = 1;
  src.is_flexible_array = 1;

  rc = merge_struct_field(&dest, &src);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("def1", dest.default_val);
  ASSERT_STR_EQ("pat1", dest.pattern);
  ASSERT_STR_EQ("4", dest.bit_width);
  ASSERT_EQ(1, dest.required);
  ASSERT_EQ(1, dest.unique_items);
  ASSERT_EQ(1, dest.is_flexible_array);

  /* Already non-empty dest (should not overwrite) */
  CDD_STRCPY(src.default_val, sizeof(src.default_val), "def2");
  CDD_STRCPY(src.pattern, sizeof(src.pattern), "pat2");
  CDD_STRCPY(src.bit_width, sizeof(src.bit_width), "8");
  rc = merge_struct_field(&dest, &src);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("def1", dest.default_val);
  ASSERT_STR_EQ("pat1", dest.pattern);
  ASSERT_STR_EQ("4", dest.bit_width);

  /* Min / Max comparisons */
  memset(&dest, 0, sizeof(dest));
  memset(&src, 0, sizeof(src));
  dest.has_min = 1;
  dest.min_val = 10.0;
  dest.exclusive_min = 0;
  src.has_min = 1;
  src.min_val = 5.0; /* smaller min, not taken */
  rc = merge_struct_field(&dest, &src);
  ASSERT_EQ(10.0, dest.min_val);

  src.min_val = 20.0; /* larger min, taken */
  rc = merge_struct_field(&dest, &src);
  ASSERT_EQ(20.0, dest.min_val);

  /* equal min, exclusive takes precedence */
  dest.has_min = 1;
  dest.min_val = 20.0;
  dest.exclusive_min = 0;
  src.has_min = 1;
  src.min_val = 20.0;
  src.exclusive_min = 1;
  rc = merge_struct_field(&dest, &src);
  ASSERT_EQ(1, dest.exclusive_min);

  /* equal min, non-exclusive does not overwrite exclusive */
  src.exclusive_min = 0;
  rc = merge_struct_field(&dest, &src);
  ASSERT_EQ(1, dest.exclusive_min);

  /* Max comparisons */
  memset(&dest, 0, sizeof(dest));
  memset(&src, 0, sizeof(src));
  dest.has_max = 1;
  dest.max_val = 50.0;
  dest.exclusive_max = 0;
  src.has_max = 1;
  src.max_val = 100.0; /* larger max, not taken */
  rc = merge_struct_field(&dest, &src);
  ASSERT_EQ(50.0, dest.max_val);

  src.max_val = 25.0; /* smaller max, taken */
  rc = merge_struct_field(&dest, &src);
  ASSERT_EQ(25.0, dest.max_val);

  /* equal max, exclusive takes precedence */
  dest.has_max = 1;
  dest.max_val = 25.0;
  dest.exclusive_max = 0;
  src.has_max = 1;
  src.max_val = 25.0;
  src.exclusive_max = 1;
  rc = merge_struct_field(&dest, &src);
  ASSERT_EQ(1, dest.exclusive_max);

  src.exclusive_max = 0;
  rc = merge_struct_field(&dest, &src);
  ASSERT_EQ(1, dest.exclusive_max);

  /* Length comparisons */
  memset(&dest, 0, sizeof(dest));
  memset(&src, 0, sizeof(src));
  dest.has_min_len = 1;
  dest.min_len = 5;
  src.has_min_len = 1;
  src.min_len = 2; /* not taken */
  rc = merge_struct_field(&dest, &src);
  ASSERT_EQ(5, dest.min_len);
  src.min_len = 10; /* taken */
  rc = merge_struct_field(&dest, &src);
  ASSERT_EQ(10, dest.min_len);

  dest.has_max_len = 1;
  dest.max_len = 20;
  src.has_max_len = 1;
  src.max_len = 50; /* not taken */
  rc = merge_struct_field(&dest, &src);
  ASSERT_EQ(20, dest.max_len);
  src.max_len = 15; /* taken */
  rc = merge_struct_field(&dest, &src);
  ASSERT_EQ(15, dest.max_len);

  /* Items comparisons */
  memset(&dest, 0, sizeof(dest));
  memset(&src, 0, sizeof(src));
  dest.has_min_items = 1;
  dest.min_items = 3;
  src.has_min_items = 1;
  src.min_items = 1; /* not taken */
  rc = merge_struct_field(&dest, &src);
  ASSERT_EQ(3, dest.min_items);
  src.min_items = 6; /* taken */
  rc = merge_struct_field(&dest, &src);
  ASSERT_EQ(6, dest.min_items);

  dest.has_max_items = 1;
  dest.max_items = 12;
  src.has_max_items = 1;
  src.max_items = 20; /* not taken */
  rc = merge_struct_field(&dest, &src);
  ASSERT_EQ(12, dest.max_items);
  src.max_items = 8; /* taken */
  rc = merge_struct_field(&dest, &src);
  ASSERT_EQ(8, dest.max_items);

  PASS();
}

SUITE(code2schema_internals_suite) {
  RUN_TEST(test_code2schema_merge_struct_field_exhaustive);
  RUN_TEST(test_code2schema_exhaustive_100_percent_coverage);
  RUN_TEST(test_code2schema_empty_refs_and_defaults);
  RUN_TEST(test_code2schema_json_object_fields_more_branches);
  RUN_TEST(test_code2schema_alloc_failures);
  RUN_TEST(test_code2schema_c2s_union_array_items_branches);
  RUN_TEST(test_code2schema_merge_schema_extras_strings_branches);
  RUN_TEST(test_code2schema_merge_struct_fields_enums_and_extras);
  RUN_TEST(test_code2schema_apply_union_variants_all_types);
  RUN_TEST(test_code2schema_json_array_to_enum_members_nonstrings);
  RUN_TEST(test_code2schema_sanitize_identifier_edge_cases);
  RUN_TEST(test_code2schema_make_unique_variant_name_fallbacks);
  RUN_TEST(test_code2schema_register_inline_schema_branches);
  RUN_TEST(test_code2schema_allof_and_fallback_branches);
  RUN_TEST(test_code2schema_collect_property_names_empty);
  RUN_TEST(test_code2schema_parse_struct_member_line_extended_types);
  RUN_TEST(test_code2schema_merge_struct_field_bounds_and_unions);
  RUN_TEST(test_code2schema_discriminator_more_branches);
  RUN_TEST(test_code2schema_c2s_parse_union_and_write_internals);
  RUN_TEST(test_code2schema_json_object_to_struct_fields_full);
  RUN_TEST(test_code2schema_apply_union_ex_full);
  RUN_TEST(test_code2schema_main_full_suite);
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
  RUN_TEST(test_code2schema_constraint_writers);
  RUN_TEST(test_code2schema_c2s_collapse_arrays);
  RUN_TEST(test_code2schema_merge_struct_fields_branches);
  RUN_TEST(test_code2schema_allof_and_unions);
  RUN_TEST(test_code2schema_parse_struct_member_line_internals);
  RUN_TEST(test_code2schema_write_struct_to_json_schema_internals);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CODE2SCHEMA_INTERNALS_H */
