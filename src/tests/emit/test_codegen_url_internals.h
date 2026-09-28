/**
 * @file test_codegen_url_internals.h
 * @brief Full branch and line coverage test suite for routes/emit/url.c
 * internals.
 */

#ifndef TEST_CODEGEN_URL_INTERNALS_H
#define TEST_CODEGEN_URL_INTERNALS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"
#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cdd_c_error.h"
#include "cdd_test_helpers_export.h"
#include "openapi/parse/openapi.h"
#include "routes/emit/url.h"
/* clang-format on */

extern C_CDD_EXPORT int g_cdd_alloc_fail;
extern C_CDD_EXPORT int g_fail_io_after;
extern C_CDD_EXPORT int g_cdd_fail_url_segment_alloc;
extern C_CDD_EXPORT int g_io_calls;
CDD_TEST_HELPERS_EXPORT FILE *cdd_test_tmpfile_global(void);

TEST test_url_primitive_and_object_predicates(void) {
  struct OpenAPI_Parameter p;
  memset(&p, 0, sizeof(p));

  /* is_primitive_type_url */
  ASSERT_EQ(CDD_C_SUCCESS, is_primitive_type_url(NULL));
  ASSERT_EQ(1, is_primitive_type_url("integer") != 0);
  ASSERT_EQ(1, is_primitive_type_url("string") != 0);
  ASSERT_EQ(1, is_primitive_type_url("boolean") != 0);
  ASSERT_EQ(1, is_primitive_type_url("number") != 0);
  ASSERT_EQ(0, is_primitive_type_url("object") != 0);
  ASSERT_EQ(0, is_primitive_type_url("unknown_type") != 0);

  /* param_is_object_kv_url */
  ASSERT_EQ(CDD_C_SUCCESS, param_is_object_kv_url(NULL));

  p.is_array = 1;
  ASSERT_EQ(CDD_C_SUCCESS, param_is_object_kv_url(&p));

  p.is_array = 0;
  p.in = OA_PARAM_IN_PATH;
  ASSERT_EQ(CDD_C_SUCCESS, param_is_object_kv_url(&p));

  p.in = OA_PARAM_IN_QUERY;
  p.type = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, param_is_object_kv_url(&p));

  p.type = (char *)(size_t) "string";
  ASSERT_EQ(0, param_is_object_kv_url(&p) != 0);

  p.type = (char *)(size_t) "object";
  ASSERT_EQ(1, param_is_object_kv_url(&p) != 0);

  PASS();
}

TEST test_url_media_type_and_param_predicates(void) {
  struct OpenAPI_Parameter p;
  const char *out_val = NULL;
  size_t len = 0;

  /* media_type_base_len_url */
  ASSERT_EQ(CDD_C_SUCCESS, media_type_base_len_url(NULL, &len));
  ASSERT_EQ(0, len);
  ASSERT_EQ(CDD_C_SUCCESS,
            media_type_base_len_url("application/json; charset=utf-8", &len));
  ASSERT_EQ(16, len);

  /* media_type_ieq_url */
  ASSERT_EQ(CDD_C_SUCCESS, media_type_ieq_url(NULL, "test"));
  ASSERT_EQ(CDD_C_SUCCESS, media_type_ieq_url("test", NULL));
  ASSERT_EQ(CDD_C_SUCCESS, media_type_ieq_url("short", "longer_expected"));
  ASSERT_EQ(CDD_C_SUCCESS, media_type_ieq_url("diff", "diffX"));
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            media_type_ieq_url("application/json", "APPLICATION/JSON"));
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            media_type_ieq_url("APPLICATION/JSON", "application/json"));
  ASSERT_EQ(CDD_C_SUCCESS,
            media_type_ieq_url("applXcation/json", "application/json"));

  /* media_type_is_json_url */
  ASSERT_EQ(CDD_C_SUCCESS, media_type_is_json_url(NULL));
  ASSERT_EQ(CDD_C_SUCCESS, media_type_is_json_url("text"));
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, media_type_is_json_url("application/json"));
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            media_type_is_json_url("application/problem+json"));
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            media_type_is_json_url("APPLICATION/PROBLEM+JSON"));
  ASSERT_EQ(CDD_C_SUCCESS, media_type_is_json_url("application/problem+xml"));

  /* media_type_is_form_url */
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            media_type_is_form_url("application/x-www-form-urlencoded"));
  ASSERT_EQ(CDD_C_SUCCESS, media_type_is_form_url("application/json"));

  /* querystring_param_is_form_object */
  ASSERT_EQ(CDD_C_SUCCESS, querystring_param_is_form_object(NULL));
  memset(&p, 0, sizeof(p));
  p.in = OA_PARAM_IN_QUERY;
  ASSERT_EQ(CDD_C_SUCCESS, querystring_param_is_form_object(&p));

  p.in = OA_PARAM_IN_QUERYSTRING;
  p.content_type = (char *)(size_t) "application/json";
  ASSERT_EQ(CDD_C_SUCCESS, querystring_param_is_form_object(&p));

  p.content_type = (char *)(size_t) "application/x-www-form-urlencoded";
  p.schema.ref_name = (char *)(size_t) "MyModel";
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, querystring_param_is_form_object(&p));

  p.schema.ref_name = NULL;
  p.schema.inline_type = (char *)(size_t) "object";
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, querystring_param_is_form_object(&p));

  p.schema.inline_type = NULL;
  p.type = (char *)(size_t) "object";
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, querystring_param_is_form_object(&p));

  p.type = (char *)(size_t) "string";
  ASSERT_EQ(CDD_C_SUCCESS, querystring_param_is_form_object(&p));

  /* querystring_param_is_json_ref */
  ASSERT_EQ(CDD_C_SUCCESS, querystring_param_is_json_ref(NULL));
  memset(&p, 0, sizeof(p));
  p.in = OA_PARAM_IN_QUERY;
  ASSERT_EQ(CDD_C_SUCCESS, querystring_param_is_json_ref(&p));

  p.in = OA_PARAM_IN_QUERYSTRING;
  p.content_type = (char *)(size_t) "text/plain";
  ASSERT_EQ(CDD_C_SUCCESS, querystring_param_is_json_ref(&p));

  p.content_type = (char *)(size_t) "application/json";
  p.schema.is_array = 1;
  ASSERT_EQ(CDD_C_SUCCESS, querystring_param_is_json_ref(&p));

  p.schema.is_array = 0;
  p.type = (char *)(size_t) "array";
  ASSERT_EQ(CDD_C_SUCCESS, querystring_param_is_json_ref(&p));

  p.type = NULL;
  p.schema.ref_name = (char *)(size_t) "Model";
  ASSERT_EQ(1, querystring_param_is_json_ref(&p));

  p.schema.ref_name = NULL;
  ASSERT_EQ(0, querystring_param_is_json_ref(&p));

  /* querystring_param_json_primitive_type */
  ASSERT_EQ(CDD_C_SUCCESS,
            querystring_param_json_primitive_type(NULL, &out_val));
  ASSERT_EQ(NULL, out_val);

  memset(&p, 0, sizeof(p));
  p.in = OA_PARAM_IN_QUERY;
  ASSERT_EQ(CDD_C_SUCCESS, querystring_param_json_primitive_type(&p, &out_val));
  ASSERT_EQ(NULL, out_val);

  p.in = OA_PARAM_IN_QUERYSTRING;
  p.content_type = (char *)(size_t) "text/plain";
  ASSERT_EQ(CDD_C_SUCCESS, querystring_param_json_primitive_type(&p, &out_val));
  ASSERT_EQ(NULL, out_val);

  p.content_type = (char *)(size_t) "application/json";
  p.schema.is_array = 1;
  ASSERT_EQ(CDD_C_SUCCESS, querystring_param_json_primitive_type(&p, &out_val));
  ASSERT_EQ(NULL, out_val);

  p.schema.is_array = 0;
  p.type = (char *)(size_t) "array";
  ASSERT_EQ(CDD_C_SUCCESS, querystring_param_json_primitive_type(&p, &out_val));
  ASSERT_EQ(NULL, out_val);

  p.type = NULL;
  p.schema.inline_type = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, querystring_param_json_primitive_type(&p, &out_val));
  ASSERT_EQ(NULL, out_val);

  p.type = (char *)(size_t) "string";
  ASSERT_EQ(CDD_C_SUCCESS, querystring_param_json_primitive_type(&p, &out_val));
  ASSERT_STR_EQ("string", out_val);

  p.type = (char *)(size_t) "integer";
  ASSERT_EQ(CDD_C_SUCCESS, querystring_param_json_primitive_type(&p, &out_val));
  ASSERT_STR_EQ("integer", out_val);

  p.type = (char *)(size_t) "number";
  ASSERT_EQ(CDD_C_SUCCESS, querystring_param_json_primitive_type(&p, &out_val));
  ASSERT_STR_EQ("number", out_val);

  p.type = (char *)(size_t) "boolean";
  ASSERT_EQ(CDD_C_SUCCESS, querystring_param_json_primitive_type(&p, &out_val));
  ASSERT_STR_EQ("boolean", out_val);

  p.type = (char *)(size_t) "custom_object";
  ASSERT_EQ(CDD_C_SUCCESS, querystring_param_json_primitive_type(&p, &out_val));
  ASSERT_EQ(NULL, out_val);

  /* querystring_param_json_array_item_type */
  ASSERT_EQ(CDD_C_SUCCESS,
            querystring_param_json_array_item_type(NULL, &out_val));
  ASSERT_EQ(NULL, out_val);

  memset(&p, 0, sizeof(p));
  p.in = OA_PARAM_IN_QUERY;
  ASSERT_EQ(CDD_C_SUCCESS,
            querystring_param_json_array_item_type(&p, &out_val));
  ASSERT_EQ(NULL, out_val);

  p.in = OA_PARAM_IN_QUERYSTRING;
  p.content_type = (char *)(size_t) "text/plain";
  ASSERT_EQ(CDD_C_SUCCESS,
            querystring_param_json_array_item_type(&p, &out_val));
  ASSERT_EQ(NULL, out_val);

  p.content_type = (char *)(size_t) "application/json";
  ASSERT_EQ(CDD_C_SUCCESS,
            querystring_param_json_array_item_type(&p, &out_val));
  ASSERT_EQ(NULL, out_val);

  p.is_array = 1;
  ASSERT_EQ(CDD_C_SUCCESS,
            querystring_param_json_array_item_type(&p, &out_val));
  ASSERT_EQ(NULL, out_val);

  p.schema.inline_type = (char *)(size_t) "string";
  ASSERT_EQ(CDD_C_SUCCESS,
            querystring_param_json_array_item_type(&p, &out_val));
  ASSERT_STR_EQ("string", out_val);

  p.schema.inline_type = NULL;
  p.items_type = (char *)(size_t) "integer";
  ASSERT_EQ(CDD_C_SUCCESS,
            querystring_param_json_array_item_type(&p, &out_val));
  ASSERT_STR_EQ("integer", out_val);

  p.items_type = (char *)(size_t) "number";
  ASSERT_EQ(CDD_C_SUCCESS,
            querystring_param_json_array_item_type(&p, &out_val));
  ASSERT_STR_EQ("number", out_val);

  p.items_type = (char *)(size_t) "boolean";
  ASSERT_EQ(CDD_C_SUCCESS,
            querystring_param_json_array_item_type(&p, &out_val));
  ASSERT_STR_EQ("boolean", out_val);

  p.items_type = (char *)(size_t) "other_type";
  ASSERT_EQ(CDD_C_SUCCESS,
            querystring_param_json_array_item_type(&p, &out_val));
  ASSERT_EQ(NULL, out_val);

  /* querystring_param_json_array_item_ref */
  ASSERT_EQ(CDD_C_SUCCESS,
            querystring_param_json_array_item_ref(NULL, &out_val));
  ASSERT_EQ(NULL, out_val);

  memset(&p, 0, sizeof(p));
  p.in = OA_PARAM_IN_QUERY;
  ASSERT_EQ(CDD_C_SUCCESS, querystring_param_json_array_item_ref(&p, &out_val));
  ASSERT_EQ(NULL, out_val);

  p.in = OA_PARAM_IN_QUERYSTRING;
  p.content_type = (char *)(size_t) "text/plain";
  ASSERT_EQ(CDD_C_SUCCESS, querystring_param_json_array_item_ref(&p, &out_val));
  ASSERT_EQ(NULL, out_val);

  p.content_type = (char *)(size_t) "application/json";
  ASSERT_EQ(CDD_C_SUCCESS, querystring_param_json_array_item_ref(&p, &out_val));
  ASSERT_EQ(NULL, out_val);

  p.is_array = 1;
  ASSERT_EQ(CDD_C_SUCCESS, querystring_param_json_array_item_ref(&p, &out_val));
  ASSERT_EQ(NULL, out_val);

  p.schema.inline_type = (char *)(size_t) "string";
  ASSERT_EQ(CDD_C_SUCCESS, querystring_param_json_array_item_ref(&p, &out_val));
  ASSERT_EQ(NULL, out_val);

  p.schema.inline_type = (char *)(size_t) "object";
  ASSERT_EQ(CDD_C_SUCCESS, querystring_param_json_array_item_ref(&p, &out_val));
  ASSERT_EQ(NULL, out_val);

  p.schema.inline_type = (char *)(size_t) "MyModelRef";
  ASSERT_EQ(CDD_C_SUCCESS, querystring_param_json_array_item_ref(&p, &out_val));
  ASSERT_STR_EQ("MyModelRef", out_val);

  /* querystring_param_raw_primitive_type */
  ASSERT_EQ(CDD_C_SUCCESS,
            querystring_param_raw_primitive_type(NULL, &out_val));
  ASSERT_EQ(NULL, out_val);

  memset(&p, 0, sizeof(p));
  p.in = OA_PARAM_IN_QUERY;
  ASSERT_EQ(CDD_C_SUCCESS, querystring_param_raw_primitive_type(&p, &out_val));
  ASSERT_EQ(NULL, out_val);

  p.in = OA_PARAM_IN_QUERYSTRING;
  ASSERT_EQ(CDD_C_SUCCESS, querystring_param_raw_primitive_type(&p, &out_val));
  ASSERT_EQ(NULL, out_val);

  p.content_type = (char *)(size_t) "application/json";
  ASSERT_EQ(CDD_C_SUCCESS, querystring_param_raw_primitive_type(&p, &out_val));
  ASSERT_EQ(NULL, out_val);

  p.content_type = (char *)(size_t) "application/x-www-form-urlencoded";
  ASSERT_EQ(CDD_C_SUCCESS, querystring_param_raw_primitive_type(&p, &out_val));
  ASSERT_EQ(NULL, out_val);

  p.content_type = (char *)(size_t) "text/plain";
  p.schema.inline_type = NULL;
  p.type = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, querystring_param_raw_primitive_type(&p, &out_val));
  ASSERT_STR_EQ("string", out_val);

  p.type = (char *)(size_t) "integer";
  ASSERT_EQ(CDD_C_SUCCESS, querystring_param_raw_primitive_type(&p, &out_val));
  ASSERT_STR_EQ("integer", out_val);

  p.type = (char *)(size_t) "number";
  ASSERT_EQ(CDD_C_SUCCESS, querystring_param_raw_primitive_type(&p, &out_val));
  ASSERT_STR_EQ("number", out_val);

  p.type = (char *)(size_t) "boolean";
  ASSERT_EQ(CDD_C_SUCCESS, querystring_param_raw_primitive_type(&p, &out_val));
  ASSERT_STR_EQ("boolean", out_val);

  p.type = (char *)(size_t) "other_custom";
  ASSERT_EQ(CDD_C_SUCCESS, querystring_param_raw_primitive_type(&p, &out_val));
  ASSERT_STR_EQ("string", out_val);

  PASS();
}

TEST test_url_write_query_json_param_all_types(void) {
  FILE *fp = cdd_test_tmpfile_global();
  struct OpenAPI_Parameter p;
  memset(&p, 0, sizeof(p));
  ASSERT(fp != NULL);

  /* Invalid args */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, write_query_json_param(NULL, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, write_query_json_param(fp, NULL));

  memset(&p, 0, sizeof(p));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, write_query_json_param(fp, &p));

  p.content_type = (char *)(size_t) "text/plain";
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, write_query_json_param(fp, &p));

  p.content_type = (char *)(size_t) "application/json";

  /* Array with NULL item_type */
  p.name = (char *)(size_t) "tags";
  p.is_array = 1;
  p.items_type = NULL;
  p.schema.inline_type = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, write_query_json_param(fp, &p));

  /* Array with primitive item types */
  p.items_type = (char *)(size_t) "string";
  ASSERT_EQ(CDD_C_SUCCESS, write_query_json_param(fp, &p));

  p.items_type = (char *)(size_t) "integer";
  ASSERT_EQ(CDD_C_SUCCESS, write_query_json_param(fp, &p));

  p.items_type = (char *)(size_t) "number";
  ASSERT_EQ(CDD_C_SUCCESS, write_query_json_param(fp, &p));

  p.items_type = (char *)(size_t) "boolean";
  ASSERT_EQ(CDD_C_SUCCESS, write_query_json_param(fp, &p));

  /* Array with item_type object */
  p.items_type = (char *)(size_t) "object";
  ASSERT_EQ(CDD_C_SUCCESS, write_query_json_param(fp, &p));

  /* Array with custom model ref */
  p.items_type = (char *)(size_t) "UserTag";
  ASSERT_EQ(CDD_C_SUCCESS, write_query_json_param(fp, &p));

  /* Non-array with schema.ref_name */
  p.is_array = 0;
  p.items_type = NULL;
  p.schema.ref_name = (char *)(size_t) "UserFilter";
  ASSERT_EQ(CDD_C_SUCCESS, write_query_json_param(fp, &p));

  /* Non-array with object type */
  p.schema.ref_name = NULL;
  p.type = (char *)(size_t) "object";
  ASSERT_EQ(CDD_C_SUCCESS, write_query_json_param(fp, &p));

  /* Non-array with string primitive */
  p.type = (char *)(size_t) "string";
  ASSERT_EQ(CDD_C_SUCCESS, write_query_json_param(fp, &p));

  /* Non-array with integer primitive */
  p.type = (char *)(size_t) "integer";
  ASSERT_EQ(CDD_C_SUCCESS, write_query_json_param(fp, &p));

  /* Non-array with number primitive */
  p.type = (char *)(size_t) "number";
  ASSERT_EQ(CDD_C_SUCCESS, write_query_json_param(fp, &p));

  /* Non-array with boolean primitive */
  p.type = (char *)(size_t) "boolean";
  ASSERT_EQ(CDD_C_SUCCESS, write_query_json_param(fp, &p));

  /* Non-array with unsupported type */
  p.type = (char *)(size_t) "unsupported_type";
  ASSERT_EQ(CDD_C_SUCCESS, write_query_json_param(fp, &p));

  fclose(fp);
  PASS();
}

TEST test_url_write_query_and_path_object_serialization(void) {
  FILE *fp = cdd_test_tmpfile_global();
  struct OpenAPI_Parameter p;
  ASSERT(fp != NULL);

  /* write_query_object_param invalid arguments */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, write_query_object_param(NULL, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, write_query_object_param(fp, NULL));

  memset(&p, 0, sizeof(p));
  p.name = (char *)(size_t) "filter";

  /* Form style with explode=1 and allow_reserved=0 */
  p.style = OA_STYLE_FORM;
  p.explode = 1;
  p.allow_reserved = 0;
  p.allow_reserved_set = 0;
  ASSERT_EQ(CDD_C_SUCCESS, write_query_object_param(fp, &p));

  /* Form style with explode=1 and allow_reserved=1 */
  p.allow_reserved = 1;
  p.allow_reserved_set = 1;
  ASSERT_EQ(CDD_C_SUCCESS, write_query_object_param(fp, &p));

  /* Form style with explode=0 and allow_reserved=0 */
  p.explode = 0;
  p.allow_reserved = 0;
  p.allow_reserved_set = 0;
  ASSERT_EQ(CDD_C_SUCCESS, write_query_object_param(fp, &p));

  /* Form style with explode=0 and allow_reserved=1 */
  p.allow_reserved = 1;
  p.allow_reserved_set = 1;
  ASSERT_EQ(CDD_C_SUCCESS, write_query_object_param(fp, &p));

  /* DeepObject style with allow_reserved=0 */
  p.style = OA_STYLE_DEEP_OBJECT;
  p.allow_reserved = 0;
  p.allow_reserved_set = 0;
  ASSERT_EQ(CDD_C_SUCCESS, write_query_object_param(fp, &p));

  /* DeepObject style with allow_reserved=1 */
  p.allow_reserved = 1;
  p.allow_reserved_set = 1;
  ASSERT_EQ(CDD_C_SUCCESS, write_query_object_param(fp, &p));

  /* SpaceDelimited style with allow_reserved=0 */
  p.style = OA_STYLE_SPACE_DELIMITED;
  p.allow_reserved = 0;
  p.allow_reserved_set = 0;
  ASSERT_EQ(CDD_C_SUCCESS, write_query_object_param(fp, &p));

  /* SpaceDelimited style with allow_reserved=1 */
  p.allow_reserved = 1;
  p.allow_reserved_set = 1;
  ASSERT_EQ(CDD_C_SUCCESS, write_query_object_param(fp, &p));

  /* PipeDelimited style with allow_reserved=0 */
  p.style = OA_STYLE_PIPE_DELIMITED;
  p.allow_reserved = 0;
  p.allow_reserved_set = 0;
  ASSERT_EQ(CDD_C_SUCCESS, write_query_object_param(fp, &p));

  /* PipeDelimited style with allow_reserved=1 */
  p.allow_reserved = 1;
  p.allow_reserved_set = 1;
  ASSERT_EQ(CDD_C_SUCCESS, write_query_object_param(fp, &p));

  /* Query object with allow_reserved=0 and allow_reserved_set=1 */
  p.allow_reserved = 0;
  p.allow_reserved_set = 1;
  ASSERT_EQ(CDD_C_SUCCESS, write_query_object_param(fp, &p));

  /* Unsupported object style */
  p.style = OA_STYLE_MATRIX;
  ASSERT_EQ(CDD_C_SUCCESS, write_query_object_param(fp, &p));

  /* write_path_object_serialization */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            write_path_object_serialization(NULL, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            write_path_object_serialization(fp, NULL));

  /* Simple style, explode=0 */
  p.style = OA_STYLE_SIMPLE;
  p.explode = 0;
  p.explode_set = 1;
  p.allow_reserved = 0;
  p.allow_reserved_set = 0;
  ASSERT_EQ(CDD_C_SUCCESS, write_path_object_serialization(fp, &p));

  /* Simple style, explode=1 */
  p.explode = 1;
  ASSERT_EQ(CDD_C_SUCCESS, write_path_object_serialization(fp, &p));

  /* Label style, explode=0 */
  p.style = OA_STYLE_LABEL;
  p.explode = 0;
  ASSERT_EQ(CDD_C_SUCCESS, write_path_object_serialization(fp, &p));

  /* Label style, explode=1 */
  p.explode = 1;
  ASSERT_EQ(CDD_C_SUCCESS, write_path_object_serialization(fp, &p));

  /* Matrix style, explode=0 */
  p.style = OA_STYLE_MATRIX;
  p.explode = 0;
  ASSERT_EQ(CDD_C_SUCCESS, write_path_object_serialization(fp, &p));

  /* Matrix style, explode=1 */
  p.explode = 1;
  ASSERT_EQ(CDD_C_SUCCESS, write_path_object_serialization(fp, &p));

  /* Path object with cookie style and default explode */
  memset(&p, 0, sizeof(p));
  p.name = (char *)(size_t) "cookie_obj";
  p.style = OA_STYLE_COOKIE;
  p.explode_set = 0;
  ASSERT_EQ(CDD_C_SUCCESS, write_path_object_serialization(fp, &p));

  /* Path object with allow_reserved=1 */
  p.allow_reserved = 1;
  p.allow_reserved_set = 1;
  ASSERT_EQ(CDD_C_SUCCESS, write_path_object_serialization(fp, &p));

  /* Path object with allow_reserved=0 and allow_reserved_set=1 */
  p.allow_reserved = 0;
  p.allow_reserved_set = 1;
  ASSERT_EQ(CDD_C_SUCCESS, write_path_object_serialization(fp, &p));

  fclose(fp);
  PASS();
}

TEST test_url_write_path_array_and_joined_array(void) {
  FILE *fp = cdd_test_tmpfile_global();
  struct OpenAPI_Parameter p;
  memset(&p, 0, sizeof(p));
  ASSERT(fp != NULL);

  /* write_path_array_serialization invalid args */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            write_path_array_serialization(NULL, NULL, NULL, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            write_path_array_serialization(fp, NULL, NULL, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            write_path_array_serialization(fp, &p, NULL, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            write_path_array_serialization(fp, &p, "", NULL));

  memset(&p, 0, sizeof(p));
  p.name = (char *)(size_t) "arr";

  /* String array with allow_reserved=0 */
  p.items_type = (char *)(size_t) "string";
  p.allow_reserved = 0;
  p.allow_reserved_set = 0;
  ASSERT_EQ(CDD_C_SUCCESS, write_path_array_serialization(fp, &p, ".", ","));

  /* String array with allow_reserved=1 */
  p.allow_reserved = 1;
  p.allow_reserved_set = 1;
  ASSERT_EQ(CDD_C_SUCCESS, write_path_array_serialization(fp, &p, ".", ","));

  /* Integer array */
  p.items_type = (char *)(size_t) "integer";
  ASSERT_EQ(CDD_C_SUCCESS, write_path_array_serialization(fp, &p, ".", ","));

  /* Number array */
  p.items_type = (char *)(size_t) "number";
  ASSERT_EQ(CDD_C_SUCCESS, write_path_array_serialization(fp, &p, ".", ","));

  /* Boolean array */
  p.items_type = (char *)(size_t) "boolean";
  ASSERT_EQ(CDD_C_SUCCESS, write_path_array_serialization(fp, &p, ".", ","));

  /* Other items_type */
  p.items_type = (char *)(size_t) "custom";
  ASSERT_EQ(CDD_C_SUCCESS, write_path_array_serialization(fp, &p, ".", ","));

  /* write_joined_query_array invalid args */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            write_joined_query_array(NULL, NULL, ',', NULL, 0));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            write_joined_query_array(fp, NULL, ',', NULL, 0));

  p.items_type = (char *)(size_t) "string";
  ASSERT_EQ(CDD_C_SUCCESS,
            write_joined_query_array(fp, &p, ',', "url_encode", 1));
  ASSERT_EQ(CDD_C_SUCCESS, write_joined_query_array(fp, &p, ' ', NULL, 0));
  ASSERT_EQ(CDD_C_SUCCESS, write_joined_query_array(fp, &p, ',', "", 0));

  /* write_joined_query_array_encoded_delim invalid args */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            write_joined_query_array_encoded_delim(NULL, NULL, NULL, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            write_joined_query_array_encoded_delim(fp, NULL, NULL, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            write_joined_query_array_encoded_delim(fp, &p, NULL, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            write_joined_query_array_encoded_delim(fp, &p, "%20", NULL));

  /* Integer */
  p.items_type = (char *)(size_t) "integer";
  ASSERT_EQ(CDD_C_SUCCESS, write_joined_query_array_encoded_delim(
                               fp, &p, "%20", "url_encode"));

  /* Number */
  p.items_type = (char *)(size_t) "number";
  ASSERT_EQ(CDD_C_SUCCESS, write_joined_query_array_encoded_delim(
                               fp, &p, "%20", "url_encode"));

  /* Boolean */
  p.items_type = (char *)(size_t) "boolean";
  ASSERT_EQ(CDD_C_SUCCESS, write_joined_query_array_encoded_delim(
                               fp, &p, "%20", "url_encode"));

  /* String/Default */
  p.items_type = (char *)(size_t) "string";
  ASSERT_EQ(CDD_C_SUCCESS, write_joined_query_array_encoded_delim(
                               fp, &p, "%20", "url_encode"));

  fclose(fp);
  PASS();
}

TEST test_url_segments_and_find_param(void) {
  struct UrlSegment *segs = NULL;
  size_t count = 0;
  struct OpenAPI_Parameter params[2];
  const struct OpenAPI_Parameter *found = NULL;
  size_t i;

  /* parse_segments errors */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            parse_segments("/users/{id", &segs, &count));

  /* parse_segments success with literals and variables */
  ASSERT_EQ(CDD_C_SUCCESS,
            parse_segments("/api/v1/users/{id}/orders/{order_id}/summary",
                           &segs, &count));
  ASSERT_EQ(5, count);
  ASSERT_EQ(0, segs[0].is_var);
  ASSERT_STR_EQ("/api/v1/users/", segs[0].text);
  ASSERT_EQ(1, segs[1].is_var);
  ASSERT_STR_EQ("id", segs[1].text);
  ASSERT_EQ(0, segs[2].is_var);
  ASSERT_STR_EQ("/orders/", segs[2].text);
  ASSERT_EQ(1, segs[3].is_var);
  ASSERT_STR_EQ("order_id", segs[3].text);
  ASSERT_EQ(0, segs[4].is_var);
  ASSERT_STR_EQ("/summary", segs[4].text);

  for (i = 0; i < count; ++i)
    free(segs[i].text);
  free(segs);
  segs = NULL;

  /* parse_segments with > 8 variable segments to trigger realloc on variable
   * path */
  ASSERT_EQ(CDD_C_SUCCESS,
            parse_segments("{a}{b}{c}{d}{e}{f}{g}{h}{i}{j}", &segs, &count));
  ASSERT_EQ(10, count);
  for (i = 0; i < count; ++i)
    free(segs[i].text);
  free(segs);
  segs = NULL;

  /* parse_segments verification */

  /* parse_segments OOM tests */
  for (i = 1; i <= 20; ++i) {
    struct UrlSegment *fail_segs = NULL;
    size_t fail_count = 0;
    g_cdd_fail_url_segment_alloc = (int)i;
    parse_segments("/api/{param}/test", &fail_segs, &fail_count);
    g_cdd_fail_url_segment_alloc = 0;
  }

  for (i = 1; i <= 25; ++i) {
    struct UrlSegment *fail_segs = NULL;
    size_t fail_count = 0;
    g_cdd_fail_url_segment_alloc = (int)i;
    parse_segments("{a}{b}{c}{d}{e}{f}{g}{h}{i}{j}", &fail_segs, &fail_count);
    g_cdd_fail_url_segment_alloc = 0;
  }

  for (i = 1; i <= 40; ++i) {
    struct UrlSegment *fail_segs = NULL;
    size_t fail_count = 0;
    g_cdd_fail_url_segment_alloc = (int)i;
    parse_segments("{a}{b}{c}{d}{e}{f}{g}{h}tail", &fail_segs, &fail_count);
    g_cdd_fail_url_segment_alloc = 0;
  }

  for (i = 1; i <= 40; ++i) {
    struct UrlSegment *fail_segs = NULL;
    size_t fail_count = 0;
    g_cdd_fail_url_segment_alloc = (int)i;
    parse_segments("s1{v1}s2{v2}s3{v3}s4{v4}s5{v5}s6{v6}", &fail_segs,
                   &fail_count);
    g_cdd_fail_url_segment_alloc = 0;
  }

  /* find_param */
  memset(params, 0, sizeof(params));
  params[0].name = (char *)(size_t) "user_id";
  params[0].in = OA_PARAM_IN_QUERY;
  params[1].name = (char *)(size_t) "id";
  params[1].in = OA_PARAM_IN_PATH;

  ASSERT_EQ(CDD_C_SUCCESS, find_param("user_id", params, 2, &found));
  ASSERT_EQ(NULL, found);

  ASSERT_EQ(CDD_C_SUCCESS, find_param("not_found", params, 2, &found));
  ASSERT_EQ(NULL, found);

  /* Parameter with name == NULL */
  params[0].name = NULL;
  params[0].in = OA_PARAM_IN_PATH;
  ASSERT_EQ(CDD_C_SUCCESS, find_param("id", params, 2, &found));
  ASSERT_EQ(&params[1], found);

  PASS();
}

SUITE(codegen_url_internals_suite) {
  RUN_TEST(test_url_primitive_and_object_predicates);
  RUN_TEST(test_url_media_type_and_param_predicates);
  RUN_TEST(test_url_write_query_json_param_all_types);
  RUN_TEST(test_url_write_query_and_path_object_serialization);
  RUN_TEST(test_url_write_path_array_and_joined_array);
  RUN_TEST(test_url_segments_and_find_param);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CODEGEN_URL_INTERNALS_H */
