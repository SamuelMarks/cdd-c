/**
 * @file test_openapi_request_body_coverage.h
 * @brief Request body ref resolution, validation, inline schema promotion, and
 * example coverage tests.
 * @author Samuel Marks
 */

#ifndef TEST_OPENAPI_REQUEST_BODY_COVERAGE_H
#define TEST_OPENAPI_REQUEST_BODY_COVERAGE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "parse/test_openapi_loader_common.h"
/* clang-format on */

/**
 * @brief Tests request body NULL checks, ref resolution, overrides, and invalid
 * content rejection.
 * @return Greatest test result.
 */
TEST test_openapi_request_body_refs_and_validation(void) {
  struct OpenAPI_RequestBody rb;
  JSON_Value *jv = NULL;
  JSON_Object *jo = NULL;
  struct OpenAPI_Spec spec;
  size_t i;

  /* 1. NULL checks */
  memset(&rb, 0, sizeof(rb));
  ASSERT_EQ(CDD_C_SUCCESS, parse_request_body_object(NULL, &rb, NULL, 0, NULL));
  ASSERT_EQ(CDD_C_SUCCESS,
            parse_request_body_object((const JSON_Object *)(size_t)1, NULL,
                                      NULL, 0, NULL));

  /* Pre-set description on request body */
  memset(&rb, 0, sizeof(rb));
  rb.description = strdup("existing");
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string(
        "{\"description\":\"new\",\"content\":{\"application/json\":{}}}",
        &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  if (jv) {
    jo = json_value_get_object(jv);
    ASSERT_EQ(CDD_C_SUCCESS, parse_request_body_object(jo, &rb, NULL, 0, NULL));
    g_cdd_last_alloc_err = free_request_body(&rb);
    g_cdd_last_alloc_err = json_value_free(jv);
  }

  /* 3. Request body ref resolution and overrides */
  memset(&spec, 0, sizeof(spec));
  spec.n_component_request_bodies = 1;
  spec.component_request_body_names = (char **)calloc(1, sizeof(char *));
  spec.component_request_body_names[0] = strdup("MyRb");
  spec.component_request_bodies = (struct OpenAPI_RequestBody *)calloc(
      1, sizeof(struct OpenAPI_RequestBody));
  spec.component_request_bodies[0].description = strdup("comp_desc");
  spec.component_request_bodies[0].required = 1;

  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err =
        cdd_json_parse_string("{\"$ref\":\"#/components/requestBodies/"
                              "MyRb\",\"description\":\"new_desc\"}",
                              &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&rb, 0, sizeof(rb));
    ASSERT_EQ(CDD_C_SUCCESS,
              parse_request_body_object(jo, &rb, &spec, 1, "op1"));
    ASSERT_STR_EQ("new_desc", rb.description);
    ASSERT_EQ(1, rb.required);
    g_cdd_last_alloc_err = free_request_body(&rb);

    /* ref with resolve_refs = 0 and spec = NULL */
    memset(&rb, 0, sizeof(rb));
    ASSERT_EQ(CDD_C_SUCCESS,
              parse_request_body_object(jo, &rb, NULL, 0, "op1"));
    g_cdd_last_alloc_err = free_request_body(&rb);

    memset(&rb, 0, sizeof(rb));
    ASSERT_EQ(CDD_C_SUCCESS,
              parse_request_body_object(jo, &rb, NULL, 1, "op1"));
    g_cdd_last_alloc_err = free_request_body(&rb);
    g_cdd_last_alloc_err = json_value_free(jv);
  }

  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string(
        "{\"$ref\":\"#/components/requestBodies/NotFound\"}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&rb, 0, sizeof(rb));
    ASSERT_EQ(CDD_C_SUCCESS,
              parse_request_body_object(jo, &rb, &spec, 1, "op1"));
    g_cdd_last_alloc_err = free_request_body(&rb);
    g_cdd_last_alloc_err = json_value_free(jv);
  }
  g_cdd_last_alloc_err = openapi_spec_free(&spec);

  /* 4. Request body invalid content */
  {
    const char *bad_rb[2];
    size_t n_bad = 2;
    bad_rb[0] = "{}";
    bad_rb[1] = "{\"content\":{}}";
    for (i = 0; i < n_bad; ++i) {
      {
        JSON_Value *_cdd_jv_1 = NULL;
        g_cdd_last_alloc_err = cdd_json_parse_string(bad_rb[i], &_cdd_jv_1);
        jv = _cdd_jv_1;
      }
      if (jv) {
        jo = json_value_get_object(jv);
        memset(&rb, 0, sizeof(rb));
        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  parse_request_body_object(jo, &rb, NULL, 0, "op1"));
        g_cdd_last_alloc_err = free_request_body(&rb);
        g_cdd_last_alloc_err = json_value_free(jv);
      }
    }
  }

  PASS();
}

/**
 * @brief Tests request body inline schema promotion, itemSchema, examples, and
 * media references.
 * @return Greatest test result.
 */
TEST test_openapi_request_body_inline_schemas_and_examples(void) {
  struct OpenAPI_RequestBody rb;
  JSON_Value *jv = NULL;
  JSON_Object *jo = NULL;
  struct OpenAPI_Spec spec;
  int k;

  /* 5. Request body inline schemas: array items, object, itemSchema */
  memset(&spec, 0, sizeof(spec));
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string(
        "{\"description\":\"desc\",\"required\":false,\"content\":{"
        "\"application/"
        "json\":"
        "{\"schema\":{\"type\":\"array\",\"items\":{\"type\":\"object\","
        "\"properties\":{\"x\":{\"type\":\"string\"}}}}}}}",
        &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&rb, 0, sizeof(rb));
    ASSERT_EQ(CDD_C_SUCCESS,
              parse_request_body_object(jo, &rb, &spec, 0, "myOp"));
    g_cdd_last_alloc_err = free_request_body(&rb);

    for (k = 1; k < 30; ++k) {
      cdd_c_error_t rc_rb;
      g_cdd_strdup_fail = k;
      memset(&rb, 0, sizeof(rb));
      rc_rb = parse_request_body_object(jo, &rb, &spec, 0, "myOp");
      g_cdd_last_alloc_err = free_request_body(&rb);
      g_cdd_strdup_fail = 0;
      if (rc_rb == CDD_C_SUCCESS)
        break;
    }

    {
      extern C_CDD_EXPORT int g_cdd_fail_assign_ref_name;
      g_cdd_fail_assign_ref_name = 1;
      memset(&rb, 0, sizeof(rb));
      ASSERT_EQ(CDD_C_ERROR_MEMORY,
                parse_request_body_object(jo, &rb, &spec, 0, "myOp"));
      g_cdd_last_alloc_err = free_request_body(&rb);
      g_cdd_fail_assign_ref_name = 0;
    }

    g_cdd_last_alloc_err = json_value_free(jv);
  }

  /* Non-object schemas / non-object items in request body */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string(
        "{\"content\":{\"application/"
        "json\":{\"schema\":{\"type\":\"array\",\"items\":{"
        "\"type\":\"string\"}}}}}",
        &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&rb, 0, sizeof(rb));
    ASSERT_EQ(CDD_C_SUCCESS,
              parse_request_body_object(jo, &rb, &spec, 0, "myOp"));
    g_cdd_last_alloc_err = free_request_body(&rb);
    g_cdd_last_alloc_err = json_value_free(jv);
  }

  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err =
        cdd_json_parse_string("{\"content\":{\"application/"
                              "json\":{\"schema\":{\"type\":\"string\"}}}}",
                              &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&rb, 0, sizeof(rb));
    ASSERT_EQ(CDD_C_SUCCESS,
              parse_request_body_object(jo, &rb, &spec, 0, "myOp"));
    g_cdd_last_alloc_err = free_request_body(&rb);
    g_cdd_last_alloc_err = json_value_free(jv);
  }

  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err =
        cdd_json_parse_string("{\"content\":{\"application/"
                              "json\":{\"itemSchema\":{\"type\":\"string\"}}}}",
                              &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&rb, 0, sizeof(rb));
    ASSERT_EQ(CDD_C_SUCCESS,
              parse_request_body_object(jo, &rb, &spec, 0, "myOp"));
    g_cdd_last_alloc_err = free_request_body(&rb);
    g_cdd_last_alloc_err = json_value_free(jv);
  }

  /* Array schema without items */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err =
        cdd_json_parse_string("{\"content\":{\"application/"
                              "json\":{\"schema\":{\"type\":\"array\"}}}}",
                              &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&rb, 0, sizeof(rb));
    ASSERT_EQ(CDD_C_SUCCESS,
              parse_request_body_object(jo, &rb, &spec, 0, "myOp"));
    g_cdd_last_alloc_err = free_request_body(&rb);
    g_cdd_last_alloc_err = json_value_free(jv);
  }

  /* Properties without type */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string(
        "{\"content\":{\"application/"
        "json\":{\"schema\":{\"properties\":{\"x\":{\"type\":\"string\"}}}}}}",
        &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&rb, 0, sizeof(rb));
    ASSERT_EQ(CDD_C_SUCCESS,
              parse_request_body_object(jo, &rb, &spec, 0, "myOp"));
    g_cdd_last_alloc_err = free_request_body(&rb);
    g_cdd_last_alloc_err = json_value_free(jv);
  }

  /* itemSchema boolean true and itemSchema properties without type */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string(
        "{\"content\":{\"application/json\":{\"itemSchema\":true}}}",
        &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&rb, 0, sizeof(rb));
    ASSERT_EQ(CDD_C_SUCCESS,
              parse_request_body_object(jo, &rb, &spec, 0, "myOp"));
    g_cdd_last_alloc_err = free_request_body(&rb);
    g_cdd_last_alloc_err = json_value_free(jv);
  }

  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err =
        cdd_json_parse_string("{\"content\":{\"application/"
                              "json\":{\"itemSchema\":{\"properties\":{\"x\":{"
                              "\"type\":\"string\"}}}}}}",
                              &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&rb, 0, sizeof(rb));
    ASSERT_EQ(CDD_C_SUCCESS,
              parse_request_body_object(jo, &rb, &spec, 0, "myOp"));
    g_cdd_last_alloc_err = free_request_body(&rb);
    g_cdd_last_alloc_err = json_value_free(jv);
  }

  /* Request body with boolean schema */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string(
        "{\"content\":{\"application/json\":{\"schema\":true}}}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&rb, 0, sizeof(rb));
    ASSERT_EQ(CDD_C_SUCCESS,
              parse_request_body_object(jo, &rb, &spec, 0, "myOp"));
    g_cdd_last_alloc_err = free_request_body(&rb);
    g_cdd_last_alloc_err = json_value_free(jv);
  }

  /* Request body without description, and with invalid media value */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string(
        "{\"content\":{\"application/json\":123}}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&rb, 0, sizeof(rb));
    ASSERT_EQ(CDD_C_SUCCESS,
              parse_request_body_object(jo, &rb, &spec, 0, "myOp"));
    g_cdd_last_alloc_err = free_request_body(&rb);
    g_cdd_last_alloc_err = json_value_free(jv);
  }

  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string(
        "{\"content\":{\"application/json\":{}}}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&rb, 0, sizeof(rb));
    ASSERT_EQ(CDD_C_SUCCESS,
              parse_request_body_object(jo, &rb, &spec, 0, "myOp"));
    g_cdd_last_alloc_err = free_request_body(&rb);
    g_cdd_last_alloc_err = json_value_free(jv);
  }

  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string(
        "{\"content\":{\"application/"
        "json\":{\"schema\":{\"type\":\"object\","
        "\"properties\":{\"x\":{\"type\":\"string\"}}}}}}",
        &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&rb, 0, sizeof(rb));
    ASSERT_EQ(CDD_C_SUCCESS,
              parse_request_body_object(jo, &rb, &spec, 0, "myOp"));
    g_cdd_last_alloc_err = free_request_body(&rb);
    g_cdd_last_alloc_err = json_value_free(jv);
  }

  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string(
        "{\"content\":{\"application/"
        "json\":{\"itemSchema\":{\"type\":\"object\","
        "\"properties\":{\"x\":{\"type\":\"string\"}}}}}}",
        &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&rb, 0, sizeof(rb));
    ASSERT_EQ(CDD_C_SUCCESS,
              parse_request_body_object(jo, &rb, &spec, 0, "myOp"));
    g_cdd_last_alloc_err = free_request_body(&rb);

    {
      extern C_CDD_EXPORT int g_cdd_fail_assign_ref_name;
      g_cdd_fail_assign_ref_name = 1;
      memset(&rb, 0, sizeof(rb));
      ASSERT_EQ(CDD_C_ERROR_MEMORY,
                parse_request_body_object(jo, &rb, &spec, 0, "myOp"));
      g_cdd_last_alloc_err = free_request_body(&rb);
      g_cdd_fail_assign_ref_name = 0;
    }

    g_cdd_last_alloc_err = json_value_free(jv);
  }

  /* Request body with examples array and single example */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string(
        "{\"content\":{\"application/json\":{\"schema\":{\"type\":\"string\"},"
        "\"examples\":{\"e1\":{\"value\":\"v1\"},\"e2\":{\"value\":\"v2\"}}}}}",
        &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&rb, 0, sizeof(rb));
    ASSERT_EQ(CDD_C_SUCCESS,
              parse_request_body_object(jo, &rb, &spec, 0, "myOp"));
    g_cdd_last_alloc_err = free_request_body(&rb);
    g_cdd_last_alloc_err = json_value_free(jv);
  }

  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string(
        "{\"content\":{\"application/json\":{\"schema\":{\"type\":\"string\"},"
        "\"example\":\"single_val\"}}}",
        &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&rb, 0, sizeof(rb));
    ASSERT_EQ(CDD_C_SUCCESS,
              parse_request_body_object(jo, &rb, &spec, 0, "myOp"));
    g_cdd_last_alloc_err = free_request_body(&rb);
    g_cdd_last_alloc_err = json_value_free(jv);
  }

  /* Request body with content media ref */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string(
        "{\"content\":{\"application/json\":{\"$ref\":\"#/"
        "components/mediaTypes/Mt\"}}}",
        &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&rb, 0, sizeof(rb));
    ASSERT_EQ(CDD_C_SUCCESS,
              parse_request_body_object(jo, &rb, &spec, 0, "myOp"));
    g_cdd_last_alloc_err = free_request_body(&rb);
    g_cdd_last_alloc_err = json_value_free(jv);
  }
  g_cdd_last_alloc_err = openapi_spec_free(&spec);

  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_OPENAPI_REQUEST_BODY_COVERAGE_H */
