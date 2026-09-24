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

TEST test_url_builder_and_query_branches(void) {
  FILE *fp = cdd_test_tmpfile_global();
  struct OpenAPI_Operation op;
  struct OpenAPI_Parameter params[5];
  struct CodegenUrlConfig cfg;
  ASSERT(fp != NULL);

  /* codegen_url_write_builder invalid args */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            codegen_url_write_builder(NULL, NULL, NULL, 0, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            codegen_url_write_builder(fp, NULL, NULL, 0, NULL));

  /* Builder error from parse_segments */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            codegen_url_write_builder(fp, "/users/{unclosed", NULL, 0, NULL));

  /* Builder with config out_variable & custom base_variable */
  memset(&cfg, 0, sizeof(cfg));
  cfg.base_variable = "custom_base";
  cfg.out_variable = "custom_url";

  memset(params, 0, sizeof(params));

  /* Path param: boolean */
  params[0].name = (char *)(size_t) "flag";
  params[0].in = OA_PARAM_IN_PATH;
  params[0].type = (char *)(size_t) "boolean";

  /* Path param: number */
  params[1].name = (char *)(size_t) "score";
  params[1].in = OA_PARAM_IN_PATH;
  params[1].type = (char *)(size_t) "number";

  /* Path param: custom fallback */
  params[2].name = (char *)(size_t) "custom";
  params[2].in = OA_PARAM_IN_PATH;
  params[2].type = (char *)(size_t) "custom_type";

  ASSERT_EQ(CDD_C_SUCCESS, codegen_url_write_builder(
                               fp, "/api/{flag}/{score}/{custom}/{unmatched}",
                               params, 3, &cfg));

  /* codegen_url_write_query_params invalid args */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            codegen_url_write_query_params(NULL, NULL, 0));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            codegen_url_write_query_params(fp, NULL, 0));

  /* Query params operation tests */
  memset(&op, 0, sizeof(op));
  memset(params, 0, sizeof(params));

  /* Querystring param with integer json_item */
  params[0].name = (char *)(size_t) "qs";
  params[0].in = OA_PARAM_IN_QUERYSTRING;
  params[0].content_type = (char *)(size_t) "application/json";
  params[0].is_array = 1;
  params[0].items_type = (char *)(size_t) "integer";
  op.parameters = params;
  op.n_parameters = 1;
  ASSERT_EQ(CDD_C_SUCCESS, codegen_url_write_query_params(fp, &op, 0));

  /* Querystring param with number json_item */
  params[0].items_type = (char *)(size_t) "number";
  ASSERT_EQ(CDD_C_SUCCESS, codegen_url_write_query_params(fp, &op, 0));

  /* Querystring param with boolean json_item */
  params[0].items_type = (char *)(size_t) "boolean";
  ASSERT_EQ(CDD_C_SUCCESS, codegen_url_write_query_params(fp, &op, 0));

  /* Querystring param with other json_item */
  params[0].items_type = (char *)(size_t) "other";
  ASSERT_EQ(CDD_C_SUCCESS, codegen_url_write_query_params(fp, &op, 0));

  /* Querystring primitive: integer */
  params[0].is_array = 0;
  params[0].items_type = NULL;
  params[0].type = (char *)(size_t) "integer";
  ASSERT_EQ(CDD_C_SUCCESS, codegen_url_write_query_params(fp, &op, 0));

  /* Querystring primitive: number */
  params[0].type = (char *)(size_t) "number";
  ASSERT_EQ(CDD_C_SUCCESS, codegen_url_write_query_params(fp, &op, 0));

  /* Querystring primitive: boolean */
  params[0].type = (char *)(size_t) "boolean";
  ASSERT_EQ(CDD_C_SUCCESS, codegen_url_write_query_params(fp, &op, 0));

  /* Querystring raw: number */
  params[0].content_type = (char *)(size_t) "text/plain";
  params[0].type = (char *)(size_t) "number";
  ASSERT_EQ(CDD_C_SUCCESS, codegen_url_write_query_params(fp, &op, 0));

  /* Querystring raw: boolean */
  params[0].type = (char *)(size_t) "boolean";
  ASSERT_EQ(CDD_C_SUCCESS, codegen_url_write_query_params(fp, &op, 0));

  /* Querystring raw: unsupported */
  params[0].type = (char *)(size_t) "unsupported";
  ASSERT_EQ(CDD_C_SUCCESS, codegen_url_write_query_params(fp, &op, 0));

  /* Querystring raw with NULL name (fallback to "querystring") */
  params[0].name = NULL;
  params[0].content_type = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, codegen_url_write_query_params(fp, &op, 0));

  /* Query param array with pipeDelimited and allow_reserved=1 */
  memset(params, 0, sizeof(params));
  params[0].name = (char *)(size_t) "pipe_arr";
  params[0].in = OA_PARAM_IN_QUERY;
  params[0].is_array = 1;
  params[0].items_type = (char *)(size_t) "string";
  params[0].style = OA_STYLE_PIPE_DELIMITED;
  params[0].allow_reserved = 1;
  params[0].allow_reserved_set = 1;
  op.parameters = params;
  op.n_parameters = 1;
  ASSERT_EQ(CDD_C_SUCCESS, codegen_url_write_query_params(fp, &op, 0));

  /* Query param array with fallback explode=true: string with allow_reserved=1
   */
  params[0].style = OA_STYLE_DEEP_OBJECT;
  params[0].explode = 1;
  params[0].explode_set = 1;
  ASSERT_EQ(CDD_C_SUCCESS, codegen_url_write_query_params(fp, &op, 0));

  /* Query param array with fallback explode=true: string with allow_reserved=0
   */
  params[0].allow_reserved = 0;
  params[0].allow_reserved_set = 0;
  ASSERT_EQ(CDD_C_SUCCESS, codegen_url_write_query_params(fp, &op, 0));

  /* Query param array with fallback explode=true: integer */
  params[0].items_type = (char *)(size_t) "integer";
  ASSERT_EQ(CDD_C_SUCCESS, codegen_url_write_query_params(fp, &op, 0));

  /* Query param array with fallback explode=true: number */
  params[0].items_type = (char *)(size_t) "number";
  ASSERT_EQ(CDD_C_SUCCESS, codegen_url_write_query_params(fp, &op, 0));

  /* Query param array with fallback explode=true: boolean */
  params[0].items_type = (char *)(size_t) "boolean";
  ASSERT_EQ(CDD_C_SUCCESS, codegen_url_write_query_params(fp, &op, 0));

  /* Query param array with fallback explode=false (unsupported) */
  params[0].explode = 0;
  ASSERT_EQ(CDD_C_SUCCESS, codegen_url_write_query_params(fp, &op, 0));

  /* Query param scalar boolean */
  params[0].is_array = 0;
  params[0].items_type = NULL;
  params[0].type = (char *)(size_t) "boolean";
  params[0].style = OA_STYLE_FORM;
  ASSERT_EQ(CDD_C_SUCCESS, codegen_url_write_query_params(fp, &op, 0));

  /* Query param scalar string with allow_reserved=1 */
  params[0].type = (char *)(size_t) "string";
  params[0].allow_reserved = 1;
  params[0].allow_reserved_set = 1;
  ASSERT_EQ(CDD_C_SUCCESS, codegen_url_write_query_params(fp, &op, 0));

  /* Path param: array with OA_STYLE_MATRIX and explode=1 */
  memset(params, 0, sizeof(params));
  params[0].name = (char *)(size_t) "arr_mat";
  params[0].in = OA_PARAM_IN_PATH;
  params[0].is_array = 1;
  params[0].items_type = (char *)(size_t) "string";
  params[0].style = OA_STYLE_MATRIX;
  params[0].explode = 1;
  params[0].explode_set = 1;

  /* Path param: array with OA_STYLE_MATRIX and explode=0 */
  params[1].name = (char *)(size_t) "arr_mat_noexp";
  params[1].in = OA_PARAM_IN_PATH;
  params[1].is_array = 1;
  params[1].items_type = (char *)(size_t) "string";
  params[1].style = OA_STYLE_MATRIX;
  params[1].explode = 0;
  params[1].explode_set = 1;

  /* Path param: array with OA_STYLE_SIMPLE */
  params[2].name = (char *)(size_t) "arr_simple";
  params[2].in = OA_PARAM_IN_PATH;
  params[2].is_array = 1;
  params[2].items_type = (char *)(size_t) "string";
  params[2].style = OA_STYLE_SIMPLE;

  /* Path param: scalar string with OA_STYLE_LABEL */
  params[3].name = (char *)(size_t) "scalar_lbl";
  params[3].in = OA_PARAM_IN_PATH;
  params[3].type = (char *)(size_t) "string";
  params[3].style = OA_STYLE_LABEL;
  params[3].allow_reserved = 1;
  params[3].allow_reserved_set = 1;

  /* Path param: array of objects */
  params[4].name = (char *)(size_t) "arr_obj";
  params[4].in = OA_PARAM_IN_PATH;
  params[4].type = (char *)(size_t) "object";
  params[4].is_array = 1;
  params[4].items_type = (char *)(size_t) "object";

  ASSERT_EQ(
      CDD_C_SUCCESS,
      codegen_url_write_builder(
          fp,
          "/api/{arr_mat}/{arr_mat_noexp}/{arr_simple}/{scalar_lbl}/{arr_obj}",
          params, 5, NULL));

  /* Path param: array with OA_STYLE_LABEL and explode=1 */
  {
    struct OpenAPI_Parameter p_lbl;
    memset(&p_lbl, 0, sizeof(p_lbl));
    p_lbl.name = (char *)(size_t) "arr_lbl_exp";
    p_lbl.in = OA_PARAM_IN_PATH;
    p_lbl.is_array = 1;
    p_lbl.items_type = (char *)(size_t) "string";
    p_lbl.style = OA_STYLE_LABEL;
    p_lbl.explode = 1;
    p_lbl.explode_set = 1;
    ASSERT_EQ(CDD_C_SUCCESS, codegen_url_write_builder(fp, "/api/{arr_lbl_exp}",
                                                       &p_lbl, 1, NULL));
  }

  /* Querystring primitive: string */
  memset(&op, 0, sizeof(op));
  memset(params, 0, sizeof(params));
  params[0].name = (char *)(size_t) "qstr";
  params[0].in = OA_PARAM_IN_QUERYSTRING;
  params[0].content_type = (char *)(size_t) "application/json";
  params[0].type = (char *)(size_t) "string";
  op.parameters = params;
  op.n_parameters = 1;
  ASSERT_EQ(CDD_C_SUCCESS, codegen_url_write_query_params(fp, &op, 0));

  /* Query param scalar string with allow_reserved=0 */
  memset(params, 0, sizeof(params));
  params[0].name = (char *)(size_t) "plain_str";
  params[0].in = OA_PARAM_IN_QUERY;
  params[0].type = (char *)(size_t) "string";
  params[0].allow_reserved = 0;
  params[0].allow_reserved_set = 1;
  op.parameters = params;
  op.n_parameters = 1;
  ASSERT_EQ(CDD_C_SUCCESS, codegen_url_write_query_params(fp, &op, 0));

  fclose(fp);
  PASS();
}

/**
 * @brief Helper for exhaustive IO failure testing on query parameters.
 */
static cdd_c_error_t test_run_io_exhaustive_query(FILE *fp,
                                                  struct OpenAPI_Operation *op,
                                                  int qp_tracking) {
  int total_calls;
  int i;
  g_fail_io_after = 100000;
  g_io_calls = 0;
  codegen_url_write_query_params(fp, op, qp_tracking);
  total_calls = g_io_calls;
  for (i = 0; i <= total_calls; ++i) {
    g_io_calls = 0;
    g_fail_io_after = i;
    codegen_url_write_query_params(fp, op, qp_tracking);
  }
  g_fail_io_after = -1;
  return CDD_C_SUCCESS;
}

/**
 * @brief Helper for exhaustive IO failure testing on URL builder.
 */
static cdd_c_error_t test_run_io_exhaustive_builder(
    FILE *fp, const char *tmpl, struct OpenAPI_Parameter *params,
    size_t n_params, const struct CodegenUrlConfig *cfg) {
  int total_calls;
  int i;
  g_fail_io_after = 100000;
  g_io_calls = 0;
  codegen_url_write_builder(fp, tmpl, params, n_params, cfg);
  total_calls = g_io_calls;
  for (i = 0; i <= total_calls; ++i) {
    g_io_calls = 0;
    g_fail_io_after = i;
    codegen_url_write_builder(fp, tmpl, params, n_params, cfg);
  }
  g_fail_io_after = -1;
  return CDD_C_SUCCESS;
}

/**
 * @brief Helper for exhaustive IO failure testing on write_query_json_param.
 */
static cdd_c_error_t
test_run_io_exhaustive_json(FILE *fp, const struct OpenAPI_Parameter *p) {
  int total_calls;
  int i;
  g_fail_io_after = 100000;
  g_io_calls = 0;
  write_query_json_param(fp, p);
  total_calls = g_io_calls;
  for (i = 0; i <= total_calls; ++i) {
    g_io_calls = 0;
    g_fail_io_after = i;
    write_query_json_param(fp, p);
  }
  g_fail_io_after = -1;
  return CDD_C_SUCCESS;
}

/**
 * @brief Helper for exhaustive IO failure testing on write_query_object_param.
 */
static cdd_c_error_t
test_run_io_exhaustive_obj(FILE *fp, const struct OpenAPI_Parameter *p) {
  int total_calls;
  int i;
  g_fail_io_after = 100000;
  g_io_calls = 0;
  write_query_object_param(fp, p);
  total_calls = g_io_calls;
  for (i = 0; i <= total_calls; ++i) {
    g_io_calls = 0;
    g_fail_io_after = i;
    write_query_object_param(fp, p);
  }
  g_fail_io_after = -1;
  return CDD_C_SUCCESS;
}

/**
 * @brief Helper for exhaustive IO failure testing on path object serialization.
 */
static cdd_c_error_t
test_run_io_exhaustive_path_obj(FILE *fp, const struct OpenAPI_Parameter *p) {
  int total_calls;
  int i;
  g_fail_io_after = 100000;
  g_io_calls = 0;
  write_path_object_serialization(fp, p);
  total_calls = g_io_calls;
  for (i = 0; i <= total_calls; ++i) {
    g_io_calls = 0;
    g_fail_io_after = i;
    write_path_object_serialization(fp, p);
  }
  g_fail_io_after = -1;
  return CDD_C_SUCCESS;
}

/**
 * @brief Helper for exhaustive IO failure testing on path array serialization.
 */
static cdd_c_error_t
test_run_io_exhaustive_path_arr(FILE *fp, const struct OpenAPI_Parameter *p,
                                const char *prefix, const char *delim) {
  int total_calls;
  int i;
  g_fail_io_after = 100000;
  g_io_calls = 0;
  write_path_array_serialization(fp, p, prefix, delim);
  total_calls = g_io_calls;
  for (i = 0; i <= total_calls; ++i) {
    g_io_calls = 0;
    g_fail_io_after = i;
    write_path_array_serialization(fp, p, prefix, delim);
  }
  g_fail_io_after = -1;
  return CDD_C_SUCCESS;
}

/**
 * @brief Helper for exhaustive IO failure testing on write_joined_query_array.
 */
static cdd_c_error_t
test_run_io_exhaustive_joined(FILE *fp, const struct OpenAPI_Parameter *p,
                              char delim, const char *encode_fn,
                              int encode_delim) {
  int total_calls;
  int i;
  g_fail_io_after = 100000;
  g_io_calls = 0;
  write_joined_query_array(fp, p, delim, encode_fn, encode_delim);
  total_calls = g_io_calls;
  for (i = 0; i <= total_calls; ++i) {
    g_io_calls = 0;
    g_fail_io_after = i;
    write_joined_query_array(fp, p, delim, encode_fn, encode_delim);
  }
  g_fail_io_after = -1;
  return CDD_C_SUCCESS;
}

/**
 * @brief Helper for exhaustive IO failure testing on
 * write_joined_query_array_encoded_delim.
 */
static cdd_c_error_t
test_run_io_exhaustive_joined_enc(FILE *fp, const struct OpenAPI_Parameter *p,
                                  const char *delim_enc,
                                  const char *encode_fn) {
  int total_calls;
  int i;
  g_fail_io_after = 100000;
  g_io_calls = 0;
  write_joined_query_array_encoded_delim(fp, p, delim_enc, encode_fn);
  total_calls = g_io_calls;
  for (i = 0; i <= total_calls; ++i) {
    g_io_calls = 0;
    g_fail_io_after = i;
    write_joined_query_array_encoded_delim(fp, p, delim_enc, encode_fn);
  }
  g_fail_io_after = -1;
  return CDD_C_SUCCESS;
}

TEST test_url_io_failure_branches(void) {
  FILE *fp = cdd_test_tmpfile_global();
  struct OpenAPI_Operation op;
  struct OpenAPI_Parameter p;
  struct OpenAPI_Parameter builder_params[12];
  struct CodegenUrlConfig cfg;
  cdd_c_error_t rc;
  ASSERT(fp != NULL);

  memset(&op, 0, sizeof(op));
  memset(&p, 0, sizeof(p));

  /* 1. write_query_json_param exhaustive IO */
  p.name = (char *)(size_t) "json_arr_str";
  p.in = OA_PARAM_IN_QUERY;
  p.content_type = (char *)(size_t) "application/json";
  p.is_array = 1;
  p.items_type = (char *)(size_t) "string";
  rc = test_run_io_exhaustive_json(fp, &p);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.items_type = (char *)(size_t) "integer";
  rc = test_run_io_exhaustive_json(fp, &p);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.items_type = (char *)(size_t) "number";
  rc = test_run_io_exhaustive_json(fp, &p);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.items_type = (char *)(size_t) "boolean";
  rc = test_run_io_exhaustive_json(fp, &p);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.items_type = (char *)(size_t) "object";
  rc = test_run_io_exhaustive_json(fp, &p);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* Array with custom model Tag */
  p.items_type = (char *)(size_t) "Tag";
  rc = test_run_io_exhaustive_json(fp, &p);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.items_type = NULL;
  p.schema.inline_type = NULL;
  rc = test_run_io_exhaustive_json(fp, &p);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* Scalar JSON primitives */
  p.is_array = 0;
  p.type = (char *)(size_t) "string";
  rc = test_run_io_exhaustive_json(fp, &p);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.type = (char *)(size_t) "integer";
  rc = test_run_io_exhaustive_json(fp, &p);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.type = (char *)(size_t) "number";
  rc = test_run_io_exhaustive_json(fp, &p);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.type = (char *)(size_t) "boolean";
  rc = test_run_io_exhaustive_json(fp, &p);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.type = (char *)(size_t) "object";
  rc = test_run_io_exhaustive_json(fp, &p);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.type = (char *)(size_t) "CustomModel";
  p.schema.ref_name = (char *)(size_t) "CustomModel";
  rc = test_run_io_exhaustive_json(fp, &p);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.schema.ref_name = NULL;
  p.type = (char *)(size_t) "unknown";
  rc = test_run_io_exhaustive_json(fp, &p);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* JSON param with name == NULL */
  p.name = NULL;
  p.type = (char *)(size_t) "string";
  rc = test_run_io_exhaustive_json(fp, &p);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* JSON param with type == NULL */
  p.name = (char *)(size_t) "null_type";
  p.type = NULL;
  rc = test_run_io_exhaustive_json(fp, &p);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* 2. write_query_object_param exhaustive IO */
  memset(&p, 0, sizeof(p));
  p.name = (char *)(size_t) "obj_p";
  p.type = (char *)(size_t) "object";
  p.style = OA_STYLE_DEEP_OBJECT;
  rc = test_run_io_exhaustive_obj(fp, &p);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.style = OA_STYLE_FORM;
  p.explode = 1;
  p.explode_set = 1;
  p.allow_reserved = 0;
  rc = test_run_io_exhaustive_obj(fp, &p);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.allow_reserved = 1;
  p.allow_reserved_set = 1;
  rc = test_run_io_exhaustive_obj(fp, &p);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.style = OA_STYLE_FORM;
  p.explode = 0;
  p.explode_set = 1;
  rc = test_run_io_exhaustive_obj(fp, &p);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.style = OA_STYLE_SPACE_DELIMITED;
  p.allow_reserved = 0;
  p.allow_reserved_set = 0;
  rc = test_run_io_exhaustive_obj(fp, &p);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.allow_reserved = 1;
  p.allow_reserved_set = 1;
  rc = test_run_io_exhaustive_obj(fp, &p);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.style = OA_STYLE_PIPE_DELIMITED;
  p.allow_reserved = 0;
  p.allow_reserved_set = 0;
  rc = test_run_io_exhaustive_obj(fp, &p);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.allow_reserved = 1;
  p.allow_reserved_set = 1;
  rc = test_run_io_exhaustive_obj(fp, &p);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.style = OA_STYLE_LABEL;
  rc = test_run_io_exhaustive_obj(fp, &p);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* Object with name == NULL and style == OA_STYLE_UNKNOWN */
  p.name = NULL;
  p.style = OA_STYLE_UNKNOWN;
  rc = test_run_io_exhaustive_obj(fp, &p);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* 3. write_path_object_serialization exhaustive IO */
  memset(&p, 0, sizeof(p));
  p.name = (char *)(size_t) "path_obj";
  p.type = (char *)(size_t) "object";
  p.style = OA_STYLE_SIMPLE;
  p.explode_set = 1;
  p.explode = 1;
  rc = test_run_io_exhaustive_path_obj(fp, &p);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.explode = 0;
  rc = test_run_io_exhaustive_path_obj(fp, &p);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.style = OA_STYLE_MATRIX;
  p.explode = 1;
  rc = test_run_io_exhaustive_path_obj(fp, &p);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.explode = 0;
  rc = test_run_io_exhaustive_path_obj(fp, &p);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.style = OA_STYLE_LABEL;
  p.explode = 1;
  rc = test_run_io_exhaustive_path_obj(fp, &p);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.explode = 0;
  rc = test_run_io_exhaustive_path_obj(fp, &p);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.style = OA_STYLE_COOKIE;
  rc = test_run_io_exhaustive_path_obj(fp, &p);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.style = OA_STYLE_FORM;
  rc = test_run_io_exhaustive_path_obj(fp, &p);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* Path object with name == NULL and style == OA_STYLE_UNKNOWN */
  p.name = NULL;
  p.style = OA_STYLE_UNKNOWN;
  rc = test_run_io_exhaustive_path_obj(fp, &p);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* 4. write_path_array_serialization exhaustive IO */
  memset(&p, 0, sizeof(p));
  p.name = (char *)(size_t) "path_arr";
  p.is_array = 1;
  p.items_type = (char *)(size_t) "string";
  p.allow_reserved = 0;
  p.allow_reserved_set = 0;
  rc = test_run_io_exhaustive_path_arr(fp, &p, ";arr=", ";arr=");
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.allow_reserved = 1;
  p.allow_reserved_set = 1;
  rc = test_run_io_exhaustive_path_arr(fp, &p, ";arr=", ",");
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.items_type = (char *)(size_t) "integer";
  rc = test_run_io_exhaustive_path_arr(fp, &p, ".", ".");
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.items_type = (char *)(size_t) "number";
  rc = test_run_io_exhaustive_path_arr(fp, &p, ".", ",");
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.items_type = (char *)(size_t) "boolean";
  rc = test_run_io_exhaustive_path_arr(fp, &p, "", ",");
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.items_type = (char *)(size_t) "string";
  rc = test_run_io_exhaustive_path_arr(fp, &p, ".", ".");
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = test_run_io_exhaustive_path_arr(fp, &p, ".", ",");
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = test_run_io_exhaustive_path_arr(fp, &p, "", ",");
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = test_run_io_exhaustive_path_arr(fp, &p, "", "");
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* Path array with name == NULL, items_type == NULL, allow_reserved_set == 1,
   * allow_reserved == 0 */
  p.name = NULL;
  p.items_type = NULL;
  p.allow_reserved_set = 1;
  p.allow_reserved = 0;
  rc = test_run_io_exhaustive_path_arr(fp, &p, "", "");
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* 5. write_joined_query_array & encoded_delim exhaustive IO */
  memset(&p, 0, sizeof(p));
  p.name = (char *)(size_t) "joined_arr";
  p.is_array = 1;
  p.items_type = (char *)(size_t) "string";
  p.allow_reserved = 0;
  p.allow_reserved_set = 0;
  rc = test_run_io_exhaustive_joined(fp, &p, ',', "url_encode", 1);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.allow_reserved = 1;
  p.allow_reserved_set = 1;
  rc = test_run_io_exhaustive_joined(fp, &p, ',', "url_encode_allow_reserved",
                                     1);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.items_type = (char *)(size_t) "integer";
  rc = test_run_io_exhaustive_joined(fp, &p, ',', "url_encode", 1);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.items_type = (char *)(size_t) "number";
  rc = test_run_io_exhaustive_joined(fp, &p, ',', "url_encode", 1);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.items_type = (char *)(size_t) "boolean";
  rc = test_run_io_exhaustive_joined(fp, &p, ',', "url_encode", 1);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.name = NULL;
  p.items_type = NULL;
  rc = test_run_io_exhaustive_joined(fp, &p, ',', "url_encode", 1);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.name = (char *)(size_t) "joined_enc";
  p.items_type = (char *)(size_t) "string";
  rc = test_run_io_exhaustive_joined_enc(fp, &p, "%7C", "url_encode");
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.items_type = (char *)(size_t) "integer";
  rc = test_run_io_exhaustive_joined_enc(fp, &p, "%7C", "url_encode");
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.items_type = (char *)(size_t) "number";
  rc = test_run_io_exhaustive_joined_enc(fp, &p, "%7C", "url_encode");
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.items_type = (char *)(size_t) "boolean";
  rc = test_run_io_exhaustive_joined_enc(fp, &p, "%7C", "url_encode");
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.name = NULL;
  p.items_type = NULL;
  rc = test_run_io_exhaustive_joined_enc(fp, &p, "%7C", "url_encode");
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* 6. codegen_url_write_builder exhaustive IO */
  memset(builder_params, 0, sizeof(builder_params));
  memset(&cfg, 0, sizeof(cfg));
  cfg.out_variable = "custom_url";
  cfg.base_variable = "base_url";

  builder_params[0].name = (char *)(size_t) "p_str";
  builder_params[0].in = OA_PARAM_IN_PATH;
  builder_params[0].type = (char *)(size_t) "string";
  builder_params[0].allow_reserved = 1;
  builder_params[0].allow_reserved_set = 1;

  builder_params[1].name = (char *)(size_t) "p_int";
  builder_params[1].in = OA_PARAM_IN_PATH;
  builder_params[1].type = (char *)(size_t) "integer";

  builder_params[2].name = (char *)(size_t) "p_num";
  builder_params[2].in = OA_PARAM_IN_PATH;
  builder_params[2].type = (char *)(size_t) "number";

  builder_params[3].name = (char *)(size_t) "p_bool";
  builder_params[3].in = OA_PARAM_IN_PATH;
  builder_params[3].type = (char *)(size_t) "boolean";

  builder_params[4].name = (char *)(size_t) "p_mat";
  builder_params[4].in = OA_PARAM_IN_PATH;
  builder_params[4].type = (char *)(size_t) "string";
  builder_params[4].style = OA_STYLE_MATRIX;

  builder_params[5].name = (char *)(size_t) "p_lbl";
  builder_params[5].in = OA_PARAM_IN_PATH;
  builder_params[5].type = (char *)(size_t) "string";
  builder_params[5].style = OA_STYLE_LABEL;

  builder_params[6].name = (char *)(size_t) "p_obj";
  builder_params[6].in = OA_PARAM_IN_PATH;
  builder_params[6].type = (char *)(size_t) "object";
  builder_params[6].is_array = 0;
  builder_params[6].style = OA_STYLE_SIMPLE;

  builder_params[7].name = (char *)(size_t) "p_arr";
  builder_params[7].in = OA_PARAM_IN_PATH;
  builder_params[7].is_array = 1;
  builder_params[7].items_type = (char *)(size_t) "string";
  builder_params[7].style = OA_STYLE_MATRIX;
  builder_params[7].explode = 1;

  builder_params[8].name = (char *)(size_t) "p_arr_lbl";
  builder_params[8].in = OA_PARAM_IN_PATH;
  builder_params[8].is_array = 1;
  builder_params[8].items_type = (char *)(size_t) "string";
  builder_params[8].style = OA_STYLE_LABEL;
  builder_params[8].explode = 0;

  builder_params[9].name = (char *)(size_t) "p_obj_arr";
  builder_params[9].in = OA_PARAM_IN_PATH;
  builder_params[9].type = (char *)(size_t) "object";
  builder_params[9].is_array = 1;

  builder_params[10].name = (char *)(size_t) "p_unknown";
  builder_params[10].in = OA_PARAM_IN_PATH;
  builder_params[10].type = (char *)(size_t) "string";
  builder_params[10].style = OA_STYLE_UNKNOWN;
  builder_params[10].allow_reserved_set = 1;
  builder_params[10].allow_reserved = 0;

  builder_params[11].name = (char *)(size_t) "p_custom";
  builder_params[11].in = OA_PARAM_IN_PATH;
  builder_params[11].type = (char *)(size_t) "custom_type";

  rc = test_run_io_exhaustive_builder(
      fp,
      "/v1/{p_str}/{p_int}/{p_num}/{p_bool}/{p_mat}/{p_lbl}/{p_obj}/{p_arr}/{"
      "p_arr_lbl}/{p_obj_arr}/{p_unknown}/{p_custom}/{missing}",
      builder_params, 12, &cfg);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  cfg.out_variable = NULL;
  rc = test_run_io_exhaustive_builder(fp, "/v1/{p_str}/{p_obj}/{p_arr}",
                                      builder_params, 12, &cfg);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* 7. codegen_url_write_query_params exhaustive IO */
  /* Query operation with raw querystring */
  memset(&op, 0, sizeof(op));
  memset(&p, 0, sizeof(p));
  p.name = (char *)(size_t) "raw_qs";
  p.in = OA_PARAM_IN_QUERYSTRING;
  op.parameters = &p;
  op.n_parameters = 1;
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* Query operation with form-object querystring */
  p.type = (char *)(size_t) "object";
  p.content_type = (char *)(size_t) "application/x-www-form-urlencoded";
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* Query operation with JSON array querystring (all primitive types) */
  p.content_type = (char *)(size_t) "application/json";
  p.is_array = 1;
  p.items_type = (char *)(size_t) "string";
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.items_type = (char *)(size_t) "integer";
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.items_type = (char *)(size_t) "number";
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.items_type = (char *)(size_t) "boolean";
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.items_type = (char *)(size_t) "MyObj";
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* Query operation with scalar JSON querystring (all primitive types) */
  p.is_array = 0;
  p.items_type = NULL;
  p.type = (char *)(size_t) "string";
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.type = (char *)(size_t) "integer";
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.type = (char *)(size_t) "number";
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.type = (char *)(size_t) "boolean";
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.type = (char *)(size_t) "MyModel";
  p.schema.ref_name = (char *)(size_t) "MyModel";
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* Query operation with raw primitive querystring */
  p.schema.ref_name = NULL;
  p.content_type = (char *)(size_t) "text/plain";
  p.type = (char *)(size_t) "string";
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.type = (char *)(size_t) "integer";
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.type = (char *)(size_t) "number";
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.type = (char *)(size_t) "boolean";
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* Query operation with 0 params and qp_tracking = 1 */
  op.n_parameters = 0;
  rc = test_run_io_exhaustive_query(fp, &op, 1);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* Query operation with OA_PARAM_IN_QUERY: non-json content_type */
  memset(&p, 0, sizeof(p));
  p.name = (char *)(size_t) "plain_param";
  p.in = OA_PARAM_IN_QUERY;
  p.content_type = (char *)(size_t) "text/plain";
  p.type = (char *)(size_t) "string";
  op.parameters = &p;
  op.n_parameters = 1;
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* Query operation with OA_PARAM_IN_QUERY: form explode=1 */
  memset(&p, 0, sizeof(p));
  p.name = (char *)(size_t) "q_arr";
  p.in = OA_PARAM_IN_QUERY;
  p.is_array = 1;
  p.items_type = (char *)(size_t) "string";
  p.style = OA_STYLE_FORM;
  p.explode = 1;
  p.explode_set = 1;
  p.allow_reserved = 0;
  p.allow_reserved_set = 0;
  op.parameters = &p;
  op.n_parameters = 1;
  rc = test_run_io_exhaustive_query(fp, &op, 1);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.allow_reserved = 1;
  p.allow_reserved_set = 1;
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.allow_reserved = 0;
  p.allow_reserved_set = 1;
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.allow_reserved_set = 0;
  p.items_type = (char *)(size_t) "integer";
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.items_type = (char *)(size_t) "number";
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.items_type = (char *)(size_t) "boolean";
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.allow_reserved = 0;
  p.allow_reserved_set = 1;
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.allow_reserved_set = 0;
  p.items_type = (char *)(size_t) "custom";
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.items_type = NULL;
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* Form explode=0 with allow_reserved=1 and allow_reserved=0 (set=1) */
  p.style = OA_STYLE_FORM;
  p.explode = 0;
  p.explode_set = 1;
  p.items_type = (char *)(size_t) "string";
  p.allow_reserved = 1;
  p.allow_reserved_set = 1;
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.allow_reserved = 0;
  p.allow_reserved_set = 1;
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* PipeDelimited array with allow_reserved=1, allow_reserved=0 (set=1), and
   * allow_reserved=0 (set=0) */
  p.items_type = (char *)(size_t) "string";
  p.style = OA_STYLE_PIPE_DELIMITED;
  p.allow_reserved = 1;
  p.allow_reserved_set = 1;
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.allow_reserved = 0;
  p.allow_reserved_set = 1;
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.allow_reserved = 0;
  p.allow_reserved_set = 0;
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* SpaceDelimited array with allow_reserved=1, allow_reserved=0 (set=1), and
   * allow_reserved=0 (set=0) */
  p.style = OA_STYLE_SPACE_DELIMITED;
  p.allow_reserved = 1;
  p.allow_reserved_set = 1;
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.allow_reserved = 0;
  p.allow_reserved_set = 1;
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.allow_reserved = 0;
  p.allow_reserved_set = 0;
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* Query operation with OA_PARAM_IN_QUERY: fallback explode=1 */
  p.style = OA_STYLE_DEEP_OBJECT;
  p.explode = 1;
  p.explode_set = 1;
  p.items_type = (char *)(size_t) "string";
  p.allow_reserved = 0;
  p.allow_reserved_set = 0;
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.allow_reserved = 1;
  p.allow_reserved_set = 1;
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.allow_reserved = 0;
  p.allow_reserved_set = 1;
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.items_type = (char *)(size_t) "integer";
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.items_type = (char *)(size_t) "number";
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.items_type = (char *)(size_t) "boolean";
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.items_type = (char *)(size_t) "custom";
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.items_type = NULL;
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* Fallback explode=0 */
  p.explode = 0;
  p.explode_set = 1;
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* Query operation with OA_PARAM_IN_QUERY: scalars */
  p.is_array = 0;
  p.items_type = NULL;
  p.type = (char *)(size_t) "string";
  p.allow_reserved = 0;
  p.allow_reserved_set = 0;
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.allow_reserved = 1;
  p.allow_reserved_set = 1;
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.type = (char *)(size_t) "integer";
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.type = (char *)(size_t) "number";
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.type = (char *)(size_t) "boolean";
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.type = (char *)(size_t) "unknown";
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.style = OA_STYLE_UNKNOWN;
  p.type = (char *)(size_t) "custom";
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.name = NULL;
  p.type = (char *)(size_t) "string";
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  g_fail_io_after = -1;
  fclose(fp);
  PASS();
}

SUITE(codegen_url_internals_suite) {
  RUN_TEST(test_url_io_failure_branches);
  RUN_TEST(test_url_primitive_and_object_predicates);
  RUN_TEST(test_url_media_type_and_param_predicates);
  RUN_TEST(test_url_write_query_json_param_all_types);
  RUN_TEST(test_url_write_query_and_path_object_serialization);
  RUN_TEST(test_url_write_path_array_and_joined_array);
  RUN_TEST(test_url_segments_and_find_param);
  RUN_TEST(test_url_builder_and_query_branches);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CODEGEN_URL_INTERNALS_H */
