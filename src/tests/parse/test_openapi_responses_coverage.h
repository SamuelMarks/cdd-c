/**
 * @file test_openapi_responses_coverage.h
 * @brief Response code key validation, response ref resolution, Swagger 2.0
 * responses, inline response schemas, and parse_responses coverage tests.
 * @author Samuel Marks
 */

#ifndef TEST_OPENAPI_RESPONSES_COVERAGE_H
#define TEST_OPENAPI_RESPONSES_COVERAGE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "parse/test_openapi_loader_common.h"
/* clang-format on */

/**
 * @brief Tests response NULL checks, response code key validation, ref
 * resolution, and Swagger 2.0 response schema/headers/links.
 * @return Greatest test result.
 */
TEST test_openapi_response_codes_and_refs(void) {
  struct OpenAPI_Response resp;
  struct OpenAPI_Operation op;
  JSON_Value *jv = NULL;
  JSON_Object *jo = NULL;
  struct OpenAPI_Spec spec;

  /* 1. NULL checks */
  memset(&resp, 0, sizeof(resp));
  ASSERT_EQ(CDD_C_SUCCESS,
            parse_response_object(NULL, &resp, NULL, 0, NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, parse_response_object((const JSON_Object *)(size_t)1,
                                                 NULL, NULL, 0, NULL, NULL));

  memset(&op, 0, sizeof(op));
  ASSERT_EQ(CDD_C_SUCCESS, parse_responses(NULL, &op, NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS,
            parse_responses((const JSON_Object *)(size_t)1, NULL, NULL, NULL));

  /* 2. is_valid_response_code_key tests */
  {
    int is_val = 0;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              is_valid_response_code_key("200", NULL));
    ASSERT_EQ(CDD_C_SUCCESS, is_valid_response_code_key(NULL, &is_val));
    ASSERT_EQ(0, is_val);
    ASSERT_EQ(CDD_C_SUCCESS, is_valid_response_code_key("", &is_val));
    ASSERT_EQ(0, is_val);
    ASSERT_EQ(CDD_C_SUCCESS, is_valid_response_code_key("default", &is_val));
    ASSERT_EQ(1, is_val);
    ASSERT_EQ(CDD_C_SUCCESS, is_valid_response_code_key("20", &is_val));
    ASSERT_EQ(0, is_val);
    ASSERT_EQ(CDD_C_SUCCESS, is_valid_response_code_key("2000", &is_val));
    ASSERT_EQ(0, is_val);
    ASSERT_EQ(CDD_C_SUCCESS, is_valid_response_code_key("2XX", &is_val));
    ASSERT_EQ(1, is_val);
    ASSERT_EQ(CDD_C_SUCCESS, is_valid_response_code_key("6XX", &is_val));
    ASSERT_EQ(0, is_val);
    ASSERT_EQ(CDD_C_SUCCESS, is_valid_response_code_key("0XX", &is_val));
    ASSERT_EQ(0, is_val);
    ASSERT_EQ(CDD_C_SUCCESS, is_valid_response_code_key("200", &is_val));
    ASSERT_EQ(1, is_val);
    ASSERT_EQ(CDD_C_SUCCESS, is_valid_response_code_key("2a0", &is_val));
    ASSERT_EQ(0, is_val);
    ASSERT_EQ(CDD_C_SUCCESS, is_valid_response_code_key("2X0", &is_val));
    ASSERT_EQ(0, is_val);
  }

  /* 6. Response ref resolution and overrides */
  memset(&spec, 0, sizeof(spec));
  spec.n_component_responses = 1;
  spec.component_response_names = (char **)calloc(1, sizeof(char *));
  spec.component_response_names[0] = strdup("MyResp");
  spec.component_responses =
      (struct OpenAPI_Response *)calloc(1, sizeof(struct OpenAPI_Response));
  spec.component_responses[0].description = strdup("comp_resp_desc");

  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string(
        "{\"$ref\":\"#/components/responses/"
        "MyResp\",\"summary\":\"sum\",\"description\":\"override\"}",
        &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&resp, 0, sizeof(resp));
    ASSERT_EQ(CDD_C_SUCCESS,
              parse_response_object(jo, &resp, &spec, 1, "op", "200"));
    ASSERT_STR_EQ("override", resp.description);
    ASSERT_STR_EQ("sum", resp.summary);
    g_cdd_last_alloc_err = free_response(&resp);

    /* ref with resolve_refs = 0 and spec = NULL */
    memset(&resp, 0, sizeof(resp));
    ASSERT_EQ(CDD_C_SUCCESS,
              parse_response_object(jo, &resp, NULL, 0, "op", "200"));
    g_cdd_last_alloc_err = free_response(&resp);

    memset(&resp, 0, sizeof(resp));
    ASSERT_EQ(CDD_C_SUCCESS,
              parse_response_object(jo, &resp, NULL, 1, "op", "200"));
    g_cdd_last_alloc_err = free_response(&resp);
    g_cdd_last_alloc_err = json_value_free(jv);
  }

  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string(
        "{\"$ref\":\"#/components/responses/NotFound\"}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&resp, 0, sizeof(resp));
    ASSERT_EQ(CDD_C_SUCCESS,
              parse_response_object(jo, &resp, &spec, 1, "op", "200"));
    g_cdd_last_alloc_err = free_response(&resp);
    g_cdd_last_alloc_err = json_value_free(jv);
  }
  g_cdd_last_alloc_err = openapi_spec_free(&spec);

  /* 7. Response Swagger 2.0 schema and headers and links */
  memset(&spec, 0, sizeof(spec));
  spec.swagger_version = strdup("2.0");
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string(
        "{\"description\":\"desc\",\"schema\":{\"type\":\"string\"},"
        "\"headers\":{\"X-Rate\":{\"description\":\"rate\"}},"
        "\"links\":{\"GetItem\":{\"operationId\":\"getItem\"}},"
        "\"x-custom\":\"val\"}",
        &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&resp, 0, sizeof(resp));
    ASSERT_EQ(CDD_C_SUCCESS,
              parse_response_object(jo, &resp, &spec, 0, "op", "200"));
    ASSERT_EQ(1, resp.schema_set);
    ASSERT_EQ(1, resp.n_headers);
    ASSERT_EQ(1, resp.n_links);
    g_cdd_last_alloc_err = free_response(&resp);
    g_cdd_last_alloc_err = json_value_free(jv);
  }

  /* Swagger 2.0 with boolean schema */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string(
        "{\"description\":\"desc\",\"schema\":true}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&resp, 0, sizeof(resp));
    ASSERT_EQ(CDD_C_SUCCESS,
              parse_response_object(jo, &resp, &spec, 0, "op", "200"));
    g_cdd_last_alloc_err = free_response(&resp);
    g_cdd_last_alloc_err = json_value_free(jv);
  }

  /* Swagger 2.0 schema error */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string(
        "{\"description\":\"d\",\"schema\":{\"$ref\":\"#/definitions/Bad\"}}",
        &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  if (jv) {
    g_cdd_strdup_fail = 2;
    memset(&resp, 0, sizeof(resp));
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              parse_response_object(json_value_get_object(jv), &resp, &spec, 0,
                                    "op", "200"));
    g_cdd_strdup_fail = 0;
    g_cdd_last_alloc_err = json_value_free(jv);
  }
  g_cdd_last_alloc_err = openapi_spec_free(&spec);

  PASS();
}

/**
 * @brief Tests response inline schemas, itemSchema, media refs, examples, and
 * parse_responses branches.
 * @return Greatest test result.
 */
TEST test_openapi_response_inline_schemas_and_parse_responses(void) {
  struct OpenAPI_Response resp;
  struct OpenAPI_Operation op;
  JSON_Value *jv = NULL;
  JSON_Object *jo = NULL;
  struct OpenAPI_Spec spec;
  int k;

  /* 8. Response inline schemas, itemSchema, and media ref */
  memset(&spec, 0, sizeof(spec));
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string(
        "{\"description\":\"desc\",\"content\":{\"application/json\":"
        "{\"schema\":{\"type\":\"array\",\"items\":{\"type\":\"object\","
        "\"properties\":{\"k\":{\"type\":\"string\"}}}}}}}",
        &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&resp, 0, sizeof(resp));
    ASSERT_EQ(CDD_C_SUCCESS,
              parse_response_object(jo, &resp, &spec, 0, "op", "200"));
    g_cdd_last_alloc_err = free_response(&resp);
    g_cdd_last_alloc_err = json_value_free(jv);
  }

  /* Array schema without items */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string(
        "{\"description\":\"d\",\"content\":{\"application/"
        "json\":{\"schema\":{\"type\":\"array\"}}}}",
        &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&resp, 0, sizeof(resp));
    ASSERT_EQ(CDD_C_SUCCESS,
              parse_response_object(jo, &resp, &spec, 0, "op", "200"));
    g_cdd_last_alloc_err = free_response(&resp);
    g_cdd_last_alloc_err = json_value_free(jv);
  }

  /* Properties without type */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string(
        "{\"description\":\"d\",\"content\":{\"application/"
        "json\":{\"schema\":{\"properties\":{\"x\":{\"type\":\"string\"}}}}}}",
        &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&resp, 0, sizeof(resp));
    ASSERT_EQ(CDD_C_SUCCESS,
              parse_response_object(jo, &resp, &spec, 0, "op", "200"));
    g_cdd_last_alloc_err = free_response(&resp);

    for (k = 1; k < 30; ++k) {
      cdd_c_error_t rc_resp;
      g_cdd_strdup_fail = k;
      memset(&resp, 0, sizeof(resp));
      rc_resp = parse_response_object(jo, &resp, &spec, 0, "op", "200");
      g_cdd_last_alloc_err = free_response(&resp);
      g_cdd_strdup_fail = 0;
      if (rc_resp == CDD_C_SUCCESS)
        break;
    }

    {
      extern C_CDD_EXPORT int g_cdd_fail_assign_ref_name;
      g_cdd_fail_assign_ref_name = 1;
      memset(&resp, 0, sizeof(resp));
      ASSERT_EQ(CDD_C_ERROR_MEMORY,
                parse_response_object(jo, &resp, &spec, 0, "op", "200"));
      g_cdd_last_alloc_err = free_response(&resp);
      g_cdd_fail_assign_ref_name = 0;
    }

    g_cdd_last_alloc_err = json_value_free(jv);
  }

  /* itemSchema boolean true and itemSchema properties without type */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string(
        "{\"description\":\"d\",\"content\":{\"application/"
        "json\":{\"itemSchema\":true}}}",
        &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&resp, 0, sizeof(resp));
    ASSERT_EQ(CDD_C_SUCCESS,
              parse_response_object(jo, &resp, &spec, 0, "op", "200"));
    g_cdd_last_alloc_err = free_response(&resp);
    g_cdd_last_alloc_err = json_value_free(jv);
  }

  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string(
        "{\"description\":\"d\",\"content\":{\"application/"
        "json\":{\"itemSchema\":{\"properties\":{\"x\":{"
        "\"type\":\"string\"}}}}}}",
        &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&resp, 0, sizeof(resp));
    ASSERT_EQ(CDD_C_SUCCESS,
              parse_response_object(jo, &resp, &spec, 0, "op", "200"));
    g_cdd_last_alloc_err = free_response(&resp);

    for (k = 1; k < 30; ++k) {
      cdd_c_error_t rc_resp;
      g_cdd_strdup_fail = k;
      memset(&resp, 0, sizeof(resp));
      rc_resp = parse_response_object(jo, &resp, &spec, 0, "op", "200");
      g_cdd_last_alloc_err = free_response(&resp);
      g_cdd_strdup_fail = 0;
      if (rc_resp == CDD_C_SUCCESS)
        break;
    }

    {
      extern C_CDD_EXPORT int g_cdd_fail_assign_ref_name;
      g_cdd_fail_assign_ref_name = 1;
      memset(&resp, 0, sizeof(resp));
      ASSERT_EQ(CDD_C_ERROR_MEMORY,
                parse_response_object(jo, &resp, &spec, 0, "op", "200"));
      g_cdd_last_alloc_err = free_response(&resp);
      g_cdd_fail_assign_ref_name = 0;
    }

    g_cdd_last_alloc_err = json_value_free(jv);
  }

  /* Non-object schemas in response */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string(
        "{\"description\":\"desc\",\"content\":{\"application/json\":"
        "{\"schema\":{\"type\":\"array\",\"items\":{\"type\":\"string\"}}}}}",
        &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&resp, 0, sizeof(resp));
    ASSERT_EQ(CDD_C_SUCCESS,
              parse_response_object(jo, &resp, &spec, 0, "op", "200"));
    g_cdd_last_alloc_err = free_response(&resp);
    g_cdd_last_alloc_err = json_value_free(jv);
  }

  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string(
        "{\"description\":\"desc\",\"content\":{\"application/json\":"
        "{\"schema\":{\"type\":\"string\"}}}}",
        &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&resp, 0, sizeof(resp));
    ASSERT_EQ(CDD_C_SUCCESS,
              parse_response_object(jo, &resp, &spec, 0, "op", "200"));
    g_cdd_last_alloc_err = free_response(&resp);

    memset(&resp, 0, sizeof(resp));
    ASSERT_EQ(CDD_C_SUCCESS,
              parse_response_object(jo, &resp, NULL, 0, "op", "200"));
    g_cdd_last_alloc_err = free_response(&resp);

    memset(&resp, 0, sizeof(resp));
    ASSERT_EQ(CDD_C_SUCCESS,
              parse_response_object(jo, &resp, &spec, 0, "op", NULL));
    g_cdd_last_alloc_err = free_response(&resp);
    g_cdd_last_alloc_err = json_value_free(jv);
  }

  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string(
        "{\"description\":\"desc\",\"content\":{\"application/json\":"
        "{\"itemSchema\":{\"type\":\"string\"}}}}",
        &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&resp, 0, sizeof(resp));
    ASSERT_EQ(CDD_C_SUCCESS,
              parse_response_object(jo, &resp, &spec, 0, "op", "200"));
    g_cdd_last_alloc_err = free_response(&resp);
    g_cdd_last_alloc_err = json_value_free(jv);
  }

  /* Response with boolean schema and invalid media content */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string(
        "{\"description\":\"desc\",\"content\":{\"application/"
        "json\":{\"schema\":true}}}",
        &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&resp, 0, sizeof(resp));
    ASSERT_EQ(CDD_C_SUCCESS,
              parse_response_object(jo, &resp, &spec, 0, "op", "200"));
    g_cdd_last_alloc_err = free_response(&resp);
    g_cdd_last_alloc_err = json_value_free(jv);
  }

  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string(
        "{\"description\":\"desc\",\"content\":{\"application/json\":123}}",
        &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&resp, 0, sizeof(resp));
    ASSERT_EQ(CDD_C_SUCCESS,
              parse_response_object(jo, &resp, &spec, 0, "op", "200"));
    g_cdd_last_alloc_err = free_response(&resp);
    g_cdd_last_alloc_err = json_value_free(jv);
  }

  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string(
        "{\"description\":\"desc\",\"content\":{\"application/json\":"
        "{\"schema\":{\"type\":\"object\",\"properties\":{\"k\":{\"type\":"
        "\"string\"}}}}}}",
        &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&resp, 0, sizeof(resp));
    ASSERT_EQ(CDD_C_SUCCESS,
              parse_response_object(jo, &resp, &spec, 0, "op", "200"));
    g_cdd_last_alloc_err = free_response(&resp);
    g_cdd_last_alloc_err = json_value_free(jv);
  }

  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string(
        "{\"description\":\"desc\",\"content\":{\"application/json\":"
        "{\"itemSchema\":{\"type\":\"object\",\"properties\":{\"k\":{\"type\":"
        "\"string\"}}}}}}",
        &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&resp, 0, sizeof(resp));
    ASSERT_EQ(CDD_C_SUCCESS,
              parse_response_object(jo, &resp, &spec, 0, "op", "200"));
    g_cdd_last_alloc_err = free_response(&resp);
    g_cdd_last_alloc_err = json_value_free(jv);
  }

  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string(
        "{\"description\":\"desc\",\"content\":{\"application/json\":"
        "{\"$ref\":\"#/components/mediaTypes/Mt\"}}}",
        &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&resp, 0, sizeof(resp));
    ASSERT_EQ(CDD_C_SUCCESS,
              parse_response_object(jo, &resp, &spec, 0, "op", "200"));
    g_cdd_last_alloc_err = free_response(&resp);
    g_cdd_last_alloc_err = json_value_free(jv);
  }

  /* Response with examples array and single example */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string(
        "{\"description\":\"desc\",\"content\":{\"application/"
        "json\":{\"schema\":{\"type\":\"string\"},"
        "\"examples\":{\"e1\":{\"value\":\"v1\"}}}}}",
        &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&resp, 0, sizeof(resp));
    ASSERT_EQ(CDD_C_SUCCESS,
              parse_response_object(jo, &resp, &spec, 0, "op", "200"));
    g_cdd_last_alloc_err = free_response(&resp);
    g_cdd_last_alloc_err = json_value_free(jv);
  }

  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string(
        "{\"description\":\"desc\",\"content\":{\"application/"
        "json\":{\"schema\":{\"type\":\"string\"},"
        "\"example\":\"val\"}}}",
        &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&resp, 0, sizeof(resp));
    ASSERT_EQ(CDD_C_SUCCESS,
              parse_response_object(jo, &resp, &spec, 0, "op", "200"));
    g_cdd_last_alloc_err = free_response(&resp);
    g_cdd_last_alloc_err = json_value_free(jv);
  }
  g_cdd_last_alloc_err = openapi_spec_free(&spec);

  /* 9. parse_responses error and success branches */
  memset(&op, 0, sizeof(op));
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string("{}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  if (jv) {
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              parse_responses(json_value_get_object(jv), &op, NULL, "op"));
    g_cdd_last_alloc_err = json_value_free(jv);
  }

  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err =
        cdd_json_parse_string("{\"x-ext\":\"custom\"}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  if (jv) {
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              parse_responses(json_value_get_object(jv), &op, NULL, "op"));
    g_cdd_last_alloc_err = json_value_free(jv);
  }

  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string(
        "{\"bad_code\":{\"description\":\"desc\"}}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  if (jv) {
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              parse_responses(json_value_get_object(jv), &op, NULL, "op"));
    g_cdd_last_alloc_err = json_value_free(jv);
  }

  /* Responses with non-object response value (resp_obj == NULL) */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string(
        "{\"200\": 123, \"x-custom\": \"val\"}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  if (jv) {
    memset(&op, 0, sizeof(op));
    ASSERT_EQ(CDD_C_SUCCESS,
              parse_responses(json_value_get_object(jv), &op, NULL, "op"));
    g_cdd_last_alloc_err = free_operation(&op);
    g_cdd_last_alloc_err = json_value_free(jv);
  }

  /* Responses without extensions */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string(
        "{\"200\": {\"description\": \"OK\"}}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  if (jv) {
    memset(&op, 0, sizeof(op));
    ASSERT_EQ(CDD_C_SUCCESS,
              parse_responses(json_value_get_object(jv), &op, NULL, "op"));
    g_cdd_last_alloc_err = free_operation(&op);
    g_cdd_last_alloc_err = json_value_free(jv);
  }

  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_OPENAPI_RESPONSES_COVERAGE_H */
