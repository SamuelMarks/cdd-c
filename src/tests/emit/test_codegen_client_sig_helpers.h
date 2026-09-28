/**
 * @file test_codegen_client_sig_helpers.h
 * @brief Unit tests for C Client Signature Generation (Helper predicates and
 * codegen branches).
 */

#ifndef TEST_CODEGEN_CLIENT_SIG_HELPERS_H
#define TEST_CODEGEN_CLIENT_SIG_HELPERS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "emit/test_codegen_client_sig_common.h"
/* clang-format on */

TEST test_sig_helpers_coverage_1(void) {
  const char *str_val = NULL;
  int int_val = 0;
  size_t sz_val = 0;

  /* map_type_to_c_arg */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_sig_map_type_to_c_arg("int", NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_map_type_to_c_arg(NULL, &str_val));
  ASSERT_STR_EQ("const void *", str_val);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_map_type_to_c_arg("integer", &str_val));
  ASSERT_STR_EQ("int ", str_val);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_map_type_to_c_arg("string", &str_val));
  ASSERT_STR_EQ("const char *", str_val);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_map_type_to_c_arg("boolean", &str_val));
  ASSERT_STR_EQ("int ", str_val);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_map_type_to_c_arg("number", &str_val));
  ASSERT_STR_EQ("double ", str_val);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_map_type_to_c_arg("other", &str_val));
  ASSERT_STR_EQ("const void *", str_val);

  /* is_primitive_type */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_sig_is_primitive_type("int", NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_is_primitive_type(NULL, &int_val));
  ASSERT_EQ(0, int_val);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_is_primitive_type("integer", &int_val));
  ASSERT_EQ(1, int_val);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_is_primitive_type("string", &int_val));
  ASSERT_EQ(1, int_val);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_is_primitive_type("boolean", &int_val));
  ASSERT_EQ(1, int_val);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_is_primitive_type("number", &int_val));
  ASSERT_EQ(1, int_val);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_is_primitive_type("custom", &int_val));
  ASSERT_EQ(0, int_val);

  /* param_is_object_kv */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_sig_param_is_object_kv(NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_param_is_object_kv(NULL, &int_val));
  ASSERT_EQ(0, int_val);
  {
    struct OpenAPI_Parameter p;
    memset(&p, 0, sizeof(p));
    p.type = (char *)(size_t)(size_t) "string";
    ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_param_is_object_kv(&p, &int_val));
    ASSERT_EQ(0, int_val);

    p.type = (char *)(size_t)(size_t) "object";
    p.in = OA_PARAM_IN_QUERY;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_param_is_object_kv(&p, &int_val));
    ASSERT_EQ(1, int_val);
    p.in = OA_PARAM_IN_PATH;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_param_is_object_kv(&p, &int_val));
    ASSERT_EQ(1, int_val);
    p.in = OA_PARAM_IN_HEADER;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_param_is_object_kv(&p, &int_val));
    ASSERT_EQ(1, int_val);
    p.in = OA_PARAM_IN_COOKIE;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_param_is_object_kv(&p, &int_val));
    ASSERT_EQ(1, int_val);
    p.in = OA_PARAM_IN_QUERYSTRING;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_param_is_object_kv(&p, &int_val));
    ASSERT_EQ(0, int_val);
    p.is_array = 1;
    p.in = OA_PARAM_IN_QUERY;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_param_is_object_kv(&p, &int_val));
    ASSERT_EQ(0, int_val);
  }

  /* media_type_base_len */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_sig_media_type_base_len("test", NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_media_type_base_len(NULL, &sz_val));
  ASSERT_EQ((size_t)0, sz_val);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_media_type_base_len(
                               "application/json;charset=utf-8", &sz_val));
  ASSERT_EQ((size_t)16, sz_val);

  /* media_type_has_prefix */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_sig_media_type_has_prefix("a", "b", NULL));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_media_type_has_prefix(NULL, "a", &int_val));
  ASSERT_EQ(0, int_val);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_media_type_has_prefix("a", NULL, &int_val));
  ASSERT_EQ(0, int_val);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_media_type_has_prefix(
                               "multipart/form-data", "multipart/", &int_val));
  ASSERT_EQ(1, int_val);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_media_type_has_prefix(
                               "text/plain", "multipart/", &int_val));
  ASSERT_EQ(0, int_val);

  /* media_type_has_suffix */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_sig_media_type_has_suffix("a", "b", NULL));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_media_type_has_suffix(NULL, "a", &int_val));
  ASSERT_EQ(0, int_val);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_media_type_has_suffix("a", NULL, &int_val));
  ASSERT_EQ(0, int_val);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_media_type_has_suffix("a", "longersuffix", &int_val));
  ASSERT_EQ(0, int_val);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_media_type_has_suffix(
                               "app+json;charset=utf8", "+json", &int_val));
  ASSERT_EQ(1, int_val);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_media_type_has_suffix("app+xml", "+json", &int_val));
  ASSERT_EQ(0, int_val);

  /* media_type_ieq */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_sig_media_type_ieq("a", "b", NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_media_type_ieq(NULL, "a", &int_val));
  ASSERT_EQ(0, int_val);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_media_type_ieq("a", NULL, &int_val));
  ASSERT_EQ(0, int_val);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_media_type_ieq(
                               "text/plain", "text/plain;param", &int_val));
  ASSERT_EQ(0, int_val);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_media_type_ieq("text/plain;charset=utf8", "text/plain",
                                        &int_val));
  ASSERT_EQ(1, int_val);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_media_type_ieq("text/plain", "text/html", &int_val));
  ASSERT_EQ(0, int_val);

  /* media_type_is_json */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_sig_media_type_is_json("a", NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_media_type_is_json(NULL, &int_val));
  ASSERT_EQ(0, int_val);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_media_type_is_json("application/json", &int_val));
  ASSERT_EQ(1, int_val);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_media_type_is_json(
                               "application/vnd.api+json", &int_val));
  ASSERT_EQ(1, int_val);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_media_type_is_json("text/plain", &int_val));
  ASSERT_EQ(0, int_val);

  /* media_type_is_form */
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_media_type_is_form(
                               "application/x-www-form-urlencoded", &int_val));
  ASSERT_EQ(1, int_val);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_media_type_is_form("text/plain", &int_val));
  ASSERT_EQ(0, int_val);

  /* media_type_is_text_plain */
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_media_type_is_text_plain("text/plain", &int_val));
  ASSERT_EQ(1, int_val);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_media_type_is_text_plain(
                               "application/json", &int_val));
  ASSERT_EQ(0, int_val);

  /* media_type_is_multipart */
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_media_type_is_multipart("multipart/mixed", &int_val));
  ASSERT_EQ(1, int_val);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_media_type_is_multipart("text/plain", &int_val));
  ASSERT_EQ(0, int_val);

  /* media_type_is_multipart_form */
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_media_type_is_multipart_form(
                               "multipart/form-data", &int_val));
  ASSERT_EQ(1, int_val);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_media_type_is_multipart_form(
                               "multipart/mixed", &int_val));
  ASSERT_EQ(0, int_val);

  PASS();
}

TEST test_sig_helpers_coverage_2(void) {
  const char *str_val = NULL;
  int int_val = 0;
  struct OpenAPI_MediaType mts[2];
  const struct OpenAPI_MediaType *found_mt = NULL;
  struct OpenAPI_Parameter p;

  /* find_media_type */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_sig_find_media_type(NULL, 0, "name", NULL));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_find_media_type(NULL, 0, "name", &found_mt));
  ASSERT(found_mt == NULL);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_find_media_type(mts, 2, NULL, &found_mt));
  ASSERT(found_mt == NULL);
  memset(mts, 0, sizeof(mts));
  mts[0].name = (char *)(size_t)(size_t) "application/json";
  mts[1].name = (char *)(size_t)(size_t) "multipart/form-data";
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_find_media_type(
                               mts, 2, "multipart/form-data", &found_mt));
  ASSERT(found_mt == &mts[1]);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_find_media_type(mts, 2, "text/plain", &found_mt));
  ASSERT(found_mt == NULL);

  /* media_type_is_textual */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_sig_media_type_is_textual("text/plain", NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_media_type_is_textual(NULL, &int_val));
  ASSERT_EQ(0, int_val);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_media_type_is_textual("text/plain", &int_val));
  ASSERT_EQ(1, int_val);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_media_type_is_textual("text/html", &int_val));
  ASSERT_EQ(1, int_val);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_media_type_is_textual("application/xml", &int_val));
  ASSERT_EQ(1, int_val);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_media_type_is_textual(
                               "application/atom+xml", &int_val));
  ASSERT_EQ(1, int_val);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_media_type_is_textual(
                               "application/octet-stream", &int_val));
  ASSERT_EQ(0, int_val);

  /* media_type_is_binary */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_sig_media_type_is_binary("bin", NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_media_type_is_binary(NULL, &int_val));
  ASSERT_EQ(0, int_val);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_media_type_is_binary("application/json", &int_val));
  ASSERT_EQ(0, int_val);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_media_type_is_binary(
                               "application/x-www-form-urlencoded", &int_val));
  ASSERT_EQ(0, int_val);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_media_type_is_binary("multipart/form-data", &int_val));
  ASSERT_EQ(0, int_val);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_media_type_is_binary("text/plain", &int_val));
  ASSERT_EQ(0, int_val);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_media_type_is_binary("application/pdf", &int_val));
  ASSERT_EQ(1, int_val);

  /* querystring_param_is_form_object */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_sig_querystring_param_is_form_object(NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_querystring_param_is_form_object(NULL, &int_val));
  ASSERT_EQ(0, int_val);
  memset(&p, 0, sizeof(p));
  p.schema.ref_name = (char *)(size_t)(size_t) "MyRef";
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_querystring_param_is_form_object(&p, &int_val));
  ASSERT_EQ(1, int_val);
  p.schema.ref_name = NULL;
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_querystring_param_is_form_object(&p, &int_val));
  ASSERT_EQ(0, int_val);

  /* querystring_param_is_json_ref */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_sig_querystring_param_is_json_ref(NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_querystring_param_is_json_ref(NULL, &int_val));
  ASSERT_EQ(0, int_val);
  memset(&p, 0, sizeof(p));
  p.schema.ref_name = (char *)(size_t)(size_t) "MyRef";
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_querystring_param_is_json_ref(&p, &int_val));
  ASSERT_EQ(1, int_val);
  p.schema.is_array = 1;
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_querystring_param_is_json_ref(&p, &int_val));
  ASSERT_EQ(0, int_val);
  p.schema.is_array = 0;
  p.type = (char *)(size_t)(size_t) "array";
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_querystring_param_is_json_ref(&p, &int_val));
  ASSERT_EQ(0, int_val);

  /* querystring_param_json_primitive_type */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_sig_querystring_param_json_primitive_type(NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_querystring_param_json_primitive_type(NULL, &str_val));
  ASSERT(str_val == NULL);
  memset(&p, 0, sizeof(p));
  p.in = OA_PARAM_IN_HEADER;
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_querystring_param_json_primitive_type(&p, &str_val));
  ASSERT(str_val == NULL);
  p.in = OA_PARAM_IN_QUERYSTRING;
  p.content_type = (char *)(size_t)(size_t) "text/plain";
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_querystring_param_json_primitive_type(&p, &str_val));
  ASSERT(str_val == NULL);
  p.content_type = (char *)(size_t)(size_t) "application/json";
  p.schema.is_array = 1;
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_querystring_param_json_primitive_type(&p, &str_val));
  ASSERT(str_val == NULL);
  p.schema.is_array = 0;
  p.type = (char *)(size_t)(size_t) "array";
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_querystring_param_json_primitive_type(&p, &str_val));
  ASSERT(str_val == NULL);
  p.type = NULL;
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_querystring_param_json_primitive_type(&p, &str_val));
  ASSERT(str_val == NULL);
  p.schema.inline_type = (char *)(size_t)(size_t) "integer";
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_querystring_param_json_primitive_type(&p, &str_val));
  ASSERT_STR_EQ("integer", str_val);
  p.schema.inline_type = (char *)(size_t)(size_t) "custom";
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_querystring_param_json_primitive_type(&p, &str_val));
  ASSERT(str_val == NULL);
  p.schema.inline_type = NULL;
  p.type = (char *)(size_t)(size_t) "string";
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_querystring_param_json_primitive_type(&p, &str_val));
  ASSERT_STR_EQ("string", str_val);
  p.type = (char *)(size_t)(size_t) "number";
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_querystring_param_json_primitive_type(&p, &str_val));
  ASSERT_STR_EQ("number", str_val);
  p.type = (char *)(size_t)(size_t) "boolean";
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_querystring_param_json_primitive_type(&p, &str_val));
  ASSERT_STR_EQ("boolean", str_val);

  /* querystring_param_json_array_item_type */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_sig_querystring_param_json_array_item_type(NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_querystring_param_json_array_item_type(
                               NULL, &str_val));
  ASSERT(str_val == NULL);
  memset(&p, 0, sizeof(p));
  p.in = OA_PARAM_IN_QUERYSTRING;
  p.content_type = (char *)(size_t)(size_t) "application/json";
  p.is_array = 1;
  p.items_type = (char *)(size_t)(size_t) "integer";
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_querystring_param_json_array_item_type(&p, &str_val));
  ASSERT_STR_EQ("integer", str_val);
  p.items_type = (char *)(size_t)(size_t) "custom";
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_querystring_param_json_array_item_type(&p, &str_val));
  ASSERT(str_val == NULL);
  p.items_type = NULL;
  p.schema.inline_type = (char *)(size_t)(size_t) "string";
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_querystring_param_json_array_item_type(&p, &str_val));
  ASSERT_STR_EQ("string", str_val);
  p.schema.inline_type = (char *)(size_t)(size_t) "number";
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_querystring_param_json_array_item_type(&p, &str_val));
  ASSERT_STR_EQ("number", str_val);
  p.schema.inline_type = (char *)(size_t)(size_t) "boolean";
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_querystring_param_json_array_item_type(&p, &str_val));
  ASSERT_STR_EQ("boolean", str_val);
  p.schema.inline_type = NULL;
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_querystring_param_json_array_item_type(&p, &str_val));
  ASSERT(str_val == NULL);
  p.is_array = 0;
  p.type = (char *)(size_t)(size_t) "string";
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_querystring_param_json_array_item_type(&p, &str_val));
  ASSERT(str_val == NULL);
  p.type = (char *)(size_t)(size_t) "array";
  p.items_type = (char *)(size_t)(size_t) "integer";
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_querystring_param_json_array_item_type(&p, &str_val));
  ASSERT_STR_EQ("integer", str_val);
  p.items_type = NULL;
  p.type = NULL;
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_querystring_param_json_array_item_type(&p, &str_val));
  ASSERT(str_val == NULL);

  /* querystring_param_json_array_item_ref */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_sig_querystring_param_json_array_item_ref(NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_querystring_param_json_array_item_ref(NULL, &str_val));
  ASSERT(str_val == NULL);
  memset(&p, 0, sizeof(p));
  p.in = OA_PARAM_IN_QUERYSTRING;
  p.content_type = (char *)(size_t)(size_t) "application/json";
  p.is_array = 1;
  p.items_type = (char *)(size_t)(size_t) "MyItemRef";
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_querystring_param_json_array_item_ref(&p, &str_val));
  ASSERT_STR_EQ("MyItemRef", str_val);
  p.items_type = (char *)(size_t)(size_t) "object";
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_querystring_param_json_array_item_ref(&p, &str_val));
  ASSERT(str_val == NULL);
  p.items_type = (char *)(size_t)(size_t) "string";
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_querystring_param_json_array_item_ref(&p, &str_val));
  ASSERT(str_val == NULL);
  p.items_type = NULL;
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_querystring_param_json_array_item_ref(&p, &str_val));
  ASSERT(str_val == NULL);
  p.is_array = 0;
  p.type = (char *)(size_t)(size_t) "string";
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_querystring_param_json_array_item_ref(&p, &str_val));
  ASSERT(str_val == NULL);
  p.type = (char *)(size_t)(size_t) "array";
  p.items_type = (char *)(size_t)(size_t) "MyItemRef";
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_querystring_param_json_array_item_ref(&p, &str_val));
  ASSERT_STR_EQ("MyItemRef", str_val);
  p.items_type = NULL;
  p.type = NULL;

  /* querystring_param_raw_primitive_type */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_sig_querystring_param_raw_primitive_type(NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_querystring_param_raw_primitive_type(NULL, &str_val));
  ASSERT(str_val == NULL);
  memset(&p, 0, sizeof(p));
  p.in = OA_PARAM_IN_QUERYSTRING;
  p.content_type = (char *)(size_t)(size_t) "text/plain";
  p.type = (char *)(size_t)(size_t) "integer";
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_querystring_param_raw_primitive_type(&p, &str_val));
  ASSERT_STR_EQ("integer", str_val);
  p.type = (char *)(size_t)(size_t) "number";
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_querystring_param_raw_primitive_type(&p, &str_val));
  ASSERT_STR_EQ("number", str_val);
  p.type = (char *)(size_t)(size_t) "boolean";
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_querystring_param_raw_primitive_type(&p, &str_val));
  ASSERT_STR_EQ("boolean", str_val);
  p.type = (char *)(size_t)(size_t) "custom";
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_querystring_param_raw_primitive_type(&p, &str_val));
  ASSERT_STR_EQ("string", str_val);
  p.type = NULL;
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_querystring_param_raw_primitive_type(&p, &str_val));
  ASSERT_STR_EQ("string", str_val);
  p.schema.inline_type = (char *)(size_t)(size_t) "string";
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_querystring_param_raw_primitive_type(&p, &str_val));
  ASSERT_STR_EQ("string", str_val);

  PASS();
}

TEST test_sig_helpers_coverage_3(void) {
  const char *str_val = NULL;
  int int_val = 0;
  char buf[64];
  struct OpenAPI_Operation op;
  struct OpenAPI_Response resp[3];
  const struct OpenAPI_Response *out_resp = NULL;
  const struct OpenAPI_SchemaRef *out_schema = NULL;
  struct OpenAPI_SchemaRef schema;

  /* map_array_item_type */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_sig_map_array_item_type("int", NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_map_array_item_type(NULL, &str_val));
  ASSERT_STR_EQ("const void *", str_val);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_map_array_item_type("integer", &str_val));
  ASSERT_STR_EQ("const int *", str_val);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_map_array_item_type("boolean", &str_val));
  ASSERT_STR_EQ("const int *", str_val);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_map_array_item_type("string", &str_val));
  ASSERT_STR_EQ("const char **", str_val);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_map_array_item_type("number", &str_val));
  ASSERT_STR_EQ("const double *", str_val);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_map_array_item_type("custom", &str_val));
  ASSERT_STR_EQ("const void *", str_val);

  /* sanitize_ident */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_sig_sanitize_ident(NULL, 10, "abc"));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_sig_sanitize_ident(buf, 0, "abc"));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_sig_sanitize_ident(buf, sizeof(buf), NULL));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_sanitize_ident(buf, sizeof(buf), "123-abc.xyz"));
  ASSERT_STR_EQ("_123_abc_xyz", buf);
  {
    char small_buf[2];
    ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_sanitize_ident(small_buf, 2, "1"));
    ASSERT_STR_EQ("_", small_buf);
  }

  /* multipart_header_param_name */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_sig_multipart_header_param_name(NULL, 10, "f", "h"));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_sig_multipart_header_param_name(buf, 0, "f", "h"));
  ASSERT_EQ(
      CDD_C_ERROR_INVALID_ARGUMENT,
      cdd_test_sig_multipart_header_param_name(buf, sizeof(buf), NULL, "h"));
  ASSERT_EQ(
      CDD_C_ERROR_INVALID_ARGUMENT,
      cdd_test_sig_multipart_header_param_name(buf, sizeof(buf), "f", NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_multipart_header_param_name(
                               buf, sizeof(buf), "photo", "X-Custom-Hdr"));
  ASSERT_STR_EQ("photo_hdr_X_Custom_Hdr", buf);

  /* header_name_is_content_type */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_sig_header_name_is_content_type("ct", NULL));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_header_name_is_content_type(NULL, &int_val));
  ASSERT_EQ(0, int_val);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_header_name_is_content_type("Content-Type", &int_val));
  ASSERT_EQ(1, int_val);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_header_name_is_content_type(
                               "Authorization", &int_val));
  ASSERT_EQ(0, int_val);

  /* map_type_to_c_out */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_sig_map_type_to_c_out("int", NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_map_type_to_c_out(NULL, &str_val));
  ASSERT_STR_EQ("void *", str_val);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_map_type_to_c_out("integer", &str_val));
  ASSERT_STR_EQ("int *", str_val);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_map_type_to_c_out("boolean", &str_val));
  ASSERT_STR_EQ("int *", str_val);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_map_type_to_c_out("string", &str_val));
  ASSERT_STR_EQ("char **", str_val);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_map_type_to_c_out("number", &str_val));
  ASSERT_STR_EQ("double *", str_val);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_map_type_to_c_out("custom", &str_val));
  ASSERT_STR_EQ("void *", str_val);

  /* map_array_item_type_out */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_sig_map_array_item_type_out("int", NULL));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_map_array_item_type_out(NULL, &str_val));
  ASSERT_STR_EQ("void **", str_val);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_map_array_item_type_out("integer", &str_val));
  ASSERT_STR_EQ("int **", str_val);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_map_array_item_type_out("boolean", &str_val));
  ASSERT_STR_EQ("int **", str_val);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_map_array_item_type_out("string", &str_val));
  ASSERT_STR_EQ("char ***", str_val);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_map_array_item_type_out("number", &str_val));
  ASSERT_STR_EQ("double **", str_val);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_map_array_item_type_out("custom", &str_val));
  ASSERT_STR_EQ("void **", str_val);

  /* schema_has_inline */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_sig_schema_has_inline(NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_schema_has_inline(NULL, &int_val));
  ASSERT_EQ(0, int_val);
  memset(&schema, 0, sizeof(schema));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_schema_has_inline(&schema, &int_val));
  ASSERT_EQ(0, int_val);
  schema.inline_type = (char *)(size_t)(size_t) "string";
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_schema_has_inline(&schema, &int_val));
  ASSERT_EQ(1, int_val);

  /* get_success_response */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_sig_get_success_response(NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_get_success_response(NULL, &out_resp));
  ASSERT(out_resp == NULL);
  memset(&op, 0, sizeof(op));
  memset(resp, 0, sizeof(resp));
  op.responses = resp;
  op.n_responses = 3;
  resp[0].code = (char *)(size_t)(size_t) "default";
  resp[1].code = (char *)(size_t)(size_t) "404";
  resp[2].code = (char *)(size_t)(size_t) "200";
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_get_success_response(&op, &out_resp));
  ASSERT(out_resp == &resp[2]);
  resp[2].code = (char *)(size_t)(size_t) "2XX";
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_get_success_response(&op, &out_resp));
  ASSERT(out_resp == &resp[2]);
  resp[2].code = (char *)(size_t)(size_t) "500";
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_get_success_response(&op, &out_resp));
  ASSERT(out_resp == &resp[0]);

  /* response_is_binary_success */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_sig_response_is_binary_success(NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_response_is_binary_success(NULL, &int_val));
  ASSERT_EQ(0, int_val);
  resp[2].code = (char *)(size_t)(size_t) "200";
  resp[2].content_type = (char *)(size_t)(size_t) "application/octet-stream";
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_response_is_binary_success(&op, &int_val));
  ASSERT_EQ(1, int_val);

  /* get_success_schema */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_sig_get_success_schema(NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_get_success_schema(NULL, &out_schema));
  ASSERT(out_schema == NULL);
  resp[2].schema.ref_name = (char *)(size_t)(size_t) "SuccessModel";
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_get_success_schema(&op, &out_schema));
  ASSERT(out_schema == &resp[2].schema);

  PASS();
}

TEST test_sig_codegen_branches_coverage(void) {
  struct OpenAPI_Operation op;
  struct OpenAPI_Parameter param;
  struct OpenAPI_MediaType mt;
  struct OpenAPI_Encoding enc;
  struct OpenAPI_Header hdr;
  struct CodegenSigConfig cfg;
  char *code = NULL;

  /* op.operation_id = NULL, group_name = "" */
  memset(&op, 0, sizeof(op));
  memset(&cfg, 0, sizeof(cfg));
  cfg.group_name = "";
  cfg.include_semicolon = 0;
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, &cfg, &code));
  ASSERT(code != NULL);
  ASSERT(strstr(code, "unnamed_op(struct HttpClient *ctx, struct ApiError "
                      "**api_error) {\n") != NULL);
  free(code);

  /* Parameter is JSON array with object item */
  memset(&op, 0, sizeof(op));
  memset(&param, 0, sizeof(param));
  op.operation_id = (char *)(size_t)(size_t) "testArrayObj";
  op.n_parameters = 1;
  op.parameters = &param;
  param.name = (char *)(size_t)(size_t) "items";
  param.content_type = (char *)(size_t)(size_t) "application/json";
  param.is_array = 1;
  param.items_type = (char *)(size_t)(size_t) "object";
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  ASSERT(code != NULL);
  ASSERT(strstr(code, "const void *items, size_t items_len") != NULL);
  free(code);

  /* Parameter is JSON array with custom struct item */
  param.items_type = (char *)(size_t)(size_t) "CustomItem";
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  ASSERT(code != NULL);
  ASSERT(strstr(code, "const struct CustomItem **items, size_t items_len") !=
         NULL);
  free(code);

  /* Parameter is JSON object with type "object" */
  param.is_array = 0;
  param.items_type = NULL;
  param.type = (char *)(size_t)(size_t) "object";
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  ASSERT(code != NULL);
  ASSERT(strstr(code, "const struct OpenAPI_KV *items, size_t items_len") !=
         NULL);
  free(code);

  /* Request body is multipart/form-data with encodings and headers */
  memset(&op, 0, sizeof(op));
  memset(&mt, 0, sizeof(mt));
  memset(&enc, 0, sizeof(enc));
  memset(&hdr, 0, sizeof(hdr));
  op.operation_id = (char *)(size_t)(size_t) "testMultipartHdr";
  op.req_body.content_type = (char *)(size_t)(size_t) "multipart/form-data";
  op.n_req_body_media_types = 1;
  op.req_body_media_types = &mt;
  mt.name = (char *)(size_t)(size_t) "multipart/form-data";
  mt.n_encoding = 1;
  mt.encoding = &enc;
  enc.name = (char *)(size_t)(size_t) "avatar";
  enc.n_headers = 1;
  enc.headers = &hdr;
  hdr.name = (char *)(size_t)(size_t) "X-Meta";
  hdr.type = (char *)(size_t)(size_t) "object";
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  ASSERT(code != NULL);
  ASSERT(strstr(code, "const struct OpenAPI_KV *avatar_hdr_X_Meta, size_t "
                      "avatar_hdr_X_Meta_len") != NULL);
  free(code);

  /* Header is array of string */
  hdr.type = (char *)(size_t)(size_t) "array";
  hdr.items_type = (char *)(size_t)(size_t) "string";
  hdr.is_array = 1;
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  ASSERT(code != NULL);
  ASSERT(
      strstr(code,
             "const char **avatar_hdr_X_Meta, size_t avatar_hdr_X_Meta_len") !=
      NULL);
  free(code);

  /* Header is integer */
  hdr.type = (char *)(size_t)(size_t) "integer";
  hdr.is_array = 0;
  hdr.items_type = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  ASSERT(code != NULL);
  ASSERT(strstr(code, "int avatar_hdr_X_Meta") != NULL);
  free(code);

  /* Request body array of integers in req_body */
  memset(&op, 0, sizeof(op));
  op.operation_id = (char *)(size_t)(size_t) "testReqBodyIntArray";
  op.req_body.content_type = (char *)(size_t)(size_t) "application/json";
  op.req_body.ref_name = (char *)(size_t)(size_t) "integer";
  op.req_body.is_array = 1;
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  ASSERT(code != NULL);
  ASSERT(strstr(code, "const int *body, size_t body_len") != NULL);
  free(code);

  /* Success schema array of numbers */
  memset(&op, 0, sizeof(op));
  op.operation_id = (char *)(size_t)(size_t) "testSuccessNumArray";
  op.req_body.content_type = (char *)(size_t)(size_t) "application/json";
  op.req_body.inline_type = (char *)(size_t)(size_t) "number";
  op.req_body.is_array = 1;
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  ASSERT(code != NULL);
  ASSERT(strstr(code, "double **out, size_t *out_len") != NULL);
  free(code);

  /* Success schema inline number not array */
  memset(&op, 0, sizeof(op));
  op.operation_id = (char *)(size_t)(size_t) "testSuccessNum";
  op.req_body.content_type = (char *)(size_t)(size_t) "application/json";
  op.req_body.inline_type = (char *)(size_t)(size_t) "number";
  op.req_body.is_array = 0;
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  ASSERT(code != NULL);
  ASSERT(strstr(code, "double *out") != NULL);
  free(code);

  PASS();
}

SUITE(client_sig_helpers_suite) {
  RUN_TEST(test_sig_helpers_coverage_1);
  RUN_TEST(test_sig_helpers_coverage_2);
  RUN_TEST(test_sig_helpers_coverage_3);
  RUN_TEST(test_sig_codegen_branches_coverage);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CODEGEN_CLIENT_SIG_HELPERS_H */
