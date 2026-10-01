/**
 * @file test_openapi_components_coverage.h
 * @brief Comprehensive 100% test coverage for openapi_components.c.
 * @author Samuel Marks
 */

#ifndef TEST_OPENAPI_COMPONENTS_COVERAGE_H
#define TEST_OPENAPI_COMPONENTS_COVERAGE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "parse/test_openapi_loader_common.h"
#include "openapi/parse/openapi_test_helpers.h"
#include "openapi/parse/openapi_internal_components.h"
#include "routes/parse/cli.h"
/* clang-format on */

static int g_components_parson_oom_fail_at = -1;
static void *mock_components_parson_malloc(size_t sz) {
  if (g_components_parson_oom_fail_at == 0) {
    g_components_parson_oom_fail_at = -1;
    return NULL;
  }
  if (g_components_parson_oom_fail_at > 0)
    g_components_parson_oom_fail_at--;
  return malloc(sz);
}
static void mock_components_parson_free(void *ptr) { free(ptr); }

/**
 * @brief Test parse_component_parameters edge cases and branches.
 *
 * @return GREATEST_TEST_RES
 */
TEST test_openapi_component_parameters_branches(void) {
  struct OpenAPI_Spec spec;
  JSON_Value *jv = NULL;
  const JSON_Object *jo = NULL;
  cdd_c_error_t rc;

  memset(&spec, 0, sizeof(spec));

  /* 1. NULL components or NULL spec */
  ASSERT_EQ(CDD_C_SUCCESS, parse_component_parameters(NULL, &spec));

  jv = json_parse_string("{}");
  ASSERT_NEQ(NULL, jv);
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, parse_component_parameters(jo, NULL));

  /* 2. No parameters property */
  ASSERT_EQ(CDD_C_SUCCESS, parse_component_parameters(jo, &spec));
  json_value_free(jv);

  /* 3. Invalid component key map */
  jv = json_parse_string("{\"parameters\": {\"bad key!\": {}}}");
  ASSERT_NEQ(NULL, jv);
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            parse_component_parameters(jo, &spec));
  json_value_free(jv);

  /* 4. Empty parameters */
  jv = json_parse_string("{\"parameters\": {}}");
  ASSERT_NEQ(NULL, jv);
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, parse_component_parameters(jo, &spec));
  json_value_free(jv);

  /* 5. Non-object value in parameters (p_obj == NULL) */
  jv = json_parse_string("{\"parameters\": {\"p1\": 123}}");
  ASSERT_NEQ(NULL, jv);
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, parse_component_parameters(jo, &spec));
  openapi_spec_free(&spec);
  json_value_free(jv);

  /* 6. Invalid parameter object (parse_parameter_object error) */
  jv = json_parse_string("{\"parameters\": {\"p1\": {\"name\": \"foo\"}}}");
  ASSERT_NEQ(NULL, jv);
  jo = json_value_get_object(jv);
  ASSERT_NEQ(CDD_C_SUCCESS, parse_component_parameters(jo, &spec));
  openapi_spec_free(&spec);
  json_value_free(jv);

  /* 7. Valid parameter object */
  jv = json_parse_string(
      "{\"parameters\": {\"p1\": {\"name\": \"foo\", \"in\": \"query\", "
      "\"schema\": {\"type\": \"string\"}}}}");
  ASSERT_NEQ(NULL, jv);
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, parse_component_parameters(jo, &spec));
  openapi_spec_free(&spec);

  /* 8. OOM on calloc 1 */
  g_cdd_alloc_fail = 1;
  rc = parse_component_parameters(jo, &spec);
  g_cdd_alloc_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  openapi_spec_free(&spec);

  /* 9. OOM on calloc 2 */
  g_cdd_alloc_fail = 2;
  rc = parse_component_parameters(jo, &spec);
  g_cdd_alloc_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  openapi_spec_free(&spec);

  /* 10. OOM on strdup */
  g_cdd_strdup_fail = 1;
  rc = parse_component_parameters(jo, &spec);
  g_cdd_strdup_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  openapi_spec_free(&spec);

  json_value_free(jv);
  PASS();
}

/**
 * @brief Test parse_component_responses edge cases and branches.
 *
 * @return GREATEST_TEST_RES
 */
TEST test_openapi_component_responses_branches(void) {
  struct OpenAPI_Spec spec;
  JSON_Value *jv = NULL;
  const JSON_Object *jo = NULL;
  cdd_c_error_t rc;

  memset(&spec, 0, sizeof(spec));

  /* 1. NULL components or NULL spec */
  ASSERT_EQ(CDD_C_SUCCESS, parse_component_responses(NULL, &spec));

  jv = json_parse_string("{}");
  ASSERT_NEQ(NULL, jv);
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, parse_component_responses(jo, NULL));

  /* 2. No responses property */
  ASSERT_EQ(CDD_C_SUCCESS, parse_component_responses(jo, &spec));
  json_value_free(jv);

  /* 3. Invalid component key map */
  jv = json_parse_string("{\"responses\": {\"bad key!\": {}}}");
  ASSERT_NEQ(NULL, jv);
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, parse_component_responses(jo, &spec));
  json_value_free(jv);

  /* 4. Empty responses */
  jv = json_parse_string("{\"responses\": {}}");
  ASSERT_NEQ(NULL, jv);
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, parse_component_responses(jo, &spec));
  json_value_free(jv);

  /* 5. Non-object value in responses (r_obj == NULL) */
  jv = json_parse_string("{\"responses\": {\"r1\": 123}}");
  ASSERT_NEQ(NULL, jv);
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, parse_component_responses(jo, &spec));
  openapi_spec_free(&spec);
  json_value_free(jv);

  /* 6. Invalid response object (parse_response_object error) */
  jv = json_parse_string(
      "{\"responses\": {\"r1\": {\"headers\": {\"X-Header\": {\"style\": "
      "\"form\"}}}}}}");
  ASSERT_NEQ(NULL, jv);
  jo = json_value_get_object(jv);
  ASSERT_NEQ(CDD_C_SUCCESS, parse_component_responses(jo, &spec));
  openapi_spec_free(&spec);
  json_value_free(jv);

  /* 7. Valid response object */
  jv = json_parse_string(
      "{\"responses\": {\"r1\": {\"description\": \"Success response\"}}}");
  ASSERT_NEQ(NULL, jv);
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, parse_component_responses(jo, &spec));
  openapi_spec_free(&spec);

  /* 8. OOM on calloc 1 */
  g_cdd_alloc_fail = 1;
  rc = parse_component_responses(jo, &spec);
  g_cdd_alloc_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  openapi_spec_free(&spec);

  /* 9. OOM on calloc 2 */
  g_cdd_alloc_fail = 2;
  rc = parse_component_responses(jo, &spec);
  g_cdd_alloc_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  openapi_spec_free(&spec);

  /* 10. OOM on strdup */
  g_cdd_strdup_fail = 1;
  rc = parse_component_responses(jo, &spec);
  g_cdd_strdup_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  openapi_spec_free(&spec);

  json_value_free(jv);
  PASS();
}

/**
 * @brief Test parse_component_headers edge cases and branches.
 *
 * @return GREATEST_TEST_RES
 */
TEST test_openapi_component_headers_branches(void) {
  struct OpenAPI_Spec spec;
  JSON_Value *jv = NULL;
  const JSON_Object *jo = NULL;
  cdd_c_error_t rc;

  memset(&spec, 0, sizeof(spec));

  /* 1. NULL components or NULL spec */
  ASSERT_EQ(CDD_C_SUCCESS, parse_component_headers(NULL, &spec));

  jv = json_parse_string("{}");
  ASSERT_NEQ(NULL, jv);
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, parse_component_headers(jo, NULL));

  /* 2. No headers property */
  ASSERT_EQ(CDD_C_SUCCESS, parse_component_headers(jo, &spec));
  json_value_free(jv);

  /* 3. Invalid component key map */
  jv = json_parse_string("{\"headers\": {\"bad key!\": {}}}");
  ASSERT_NEQ(NULL, jv);
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, parse_component_headers(jo, &spec));
  json_value_free(jv);

  /* 4. Empty headers */
  jv = json_parse_string("{\"headers\": {}}");
  ASSERT_NEQ(NULL, jv);
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, parse_component_headers(jo, &spec));
  json_value_free(jv);

  /* 5. Non-object value in headers (h_obj == NULL) */
  jv = json_parse_string("{\"headers\": {\"h1\": 123}}");
  ASSERT_NEQ(NULL, jv);
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, parse_component_headers(jo, &spec));
  openapi_spec_free(&spec);
  json_value_free(jv);

  /* 6. Invalid header object (parse_header_object error: style form invalid) */
  jv = json_parse_string("{\"headers\": {\"h1\": {\"style\": \"form\"}}}");
  ASSERT_NEQ(NULL, jv);
  jo = json_value_get_object(jv);
  ASSERT_NEQ(CDD_C_SUCCESS, parse_component_headers(jo, &spec));
  openapi_spec_free(&spec);
  json_value_free(jv);

  /* 7. Valid header object */
  jv = json_parse_string(
      "{\"headers\": {\"X-Rate-Limit\": {\"description\": "
      "\"calls left\", \"schema\": {\"type\": \"integer\"}}}}");
  ASSERT_NEQ(NULL, jv);
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, parse_component_headers(jo, &spec));
  openapi_spec_free(&spec);

  /* 8. OOM on calloc 1 */
  g_cdd_alloc_fail = 1;
  rc = parse_component_headers(jo, &spec);
  g_cdd_alloc_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  openapi_spec_free(&spec);

  /* 9. OOM on calloc 2 */
  g_cdd_alloc_fail = 2;
  rc = parse_component_headers(jo, &spec);
  g_cdd_alloc_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  openapi_spec_free(&spec);

  /* 10. OOM on strdup */
  g_cdd_strdup_fail = 1;
  rc = parse_component_headers(jo, &spec);
  g_cdd_strdup_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  openapi_spec_free(&spec);

  json_value_free(jv);
  PASS();
}

/**
 * @brief Test parse_component_request_bodies edge cases and branches.
 *
 * @return GREATEST_TEST_RES
 */
TEST test_openapi_component_request_bodies_branches(void) {
  struct OpenAPI_Spec spec;
  JSON_Value *jv = NULL;
  const JSON_Object *jo = NULL;
  cdd_c_error_t rc;

  memset(&spec, 0, sizeof(spec));

  /* 1. NULL components or NULL spec */
  ASSERT_EQ(CDD_C_SUCCESS, parse_component_request_bodies(NULL, &spec));

  jv = json_parse_string("{}");
  ASSERT_NEQ(NULL, jv);
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, parse_component_request_bodies(jo, NULL));

  /* 2. No requestBodies property */
  ASSERT_EQ(CDD_C_SUCCESS, parse_component_request_bodies(jo, &spec));
  json_value_free(jv);

  /* 3. Invalid component key map */
  jv = json_parse_string("{\"requestBodies\": {\"bad key!\": {}}}");
  ASSERT_NEQ(NULL, jv);
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            parse_component_request_bodies(jo, &spec));
  json_value_free(jv);

  /* 4. Empty requestBodies */
  jv = json_parse_string("{\"requestBodies\": {}}");
  ASSERT_NEQ(NULL, jv);
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, parse_component_request_bodies(jo, &spec));
  json_value_free(jv);

  /* 5. Non-object value in requestBodies (rb_obj == NULL) */
  jv = json_parse_string("{\"requestBodies\": {\"rb1\": 123}}");
  ASSERT_NEQ(NULL, jv);
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, parse_component_request_bodies(jo, &spec));
  openapi_spec_free(&spec);
  json_value_free(jv);

  /* 6. Invalid request body object (parse_request_body_object error: no
   * content) */
  jv = json_parse_string(
      "{\"requestBodies\": {\"rb1\": {\"description\": \"no content\"}}}");
  ASSERT_NEQ(NULL, jv);
  jo = json_value_get_object(jv);
  ASSERT_NEQ(CDD_C_SUCCESS, parse_component_request_bodies(jo, &spec));
  openapi_spec_free(&spec);
  json_value_free(jv);

  /* 7. Valid requestBody object */
  jv = json_parse_string(
      "{\"requestBodies\": {\"UserBody\": {\"description\": "
      "\"user body\", \"required\": true, \"content\": "
      "{\"application/json\": {\"schema\": {\"type\": \"string\"}}}}}}");
  ASSERT_NEQ(NULL, jv);
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, parse_component_request_bodies(jo, &spec));
  openapi_spec_free(&spec);

  /* 8. OOM on calloc 1 */
  g_cdd_alloc_fail = 1;
  rc = parse_component_request_bodies(jo, &spec);
  g_cdd_alloc_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  openapi_spec_free(&spec);

  /* 9. OOM on calloc 2 */
  g_cdd_alloc_fail = 2;
  rc = parse_component_request_bodies(jo, &spec);
  g_cdd_alloc_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  openapi_spec_free(&spec);

  /* 10. OOM on strdup */
  g_cdd_strdup_fail = 1;
  rc = parse_component_request_bodies(jo, &spec);
  g_cdd_strdup_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  openapi_spec_free(&spec);

  json_value_free(jv);
  PASS();
}

/**
 * @brief Test parse_component_media_types edge cases and branches.
 *
 * @return GREATEST_TEST_RES
 */
TEST test_openapi_component_media_types_branches(void) {
  struct OpenAPI_Spec spec;
  JSON_Value *jv = NULL;
  const JSON_Object *jo = NULL;
  cdd_c_error_t rc;

  memset(&spec, 0, sizeof(spec));

  /* 1. NULL components or NULL spec */
  ASSERT_EQ(CDD_C_SUCCESS, parse_component_media_types(NULL, &spec));

  jv = json_parse_string("{}");
  ASSERT_NEQ(NULL, jv);
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, parse_component_media_types(jo, NULL));

  /* 2. No mediaTypes property */
  ASSERT_EQ(CDD_C_SUCCESS, parse_component_media_types(jo, &spec));
  json_value_free(jv);

  /* 3. Invalid media type key map (with space) */
  jv = json_parse_string(
      "{\"mediaTypes\": {\"bad media type with spaces\": {}}}");
  ASSERT_NEQ(NULL, jv);
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            parse_component_media_types(jo, &spec));
  json_value_free(jv);

  /* 4. Empty mediaTypes */
  jv = json_parse_string("{\"mediaTypes\": {}}");
  ASSERT_NEQ(NULL, jv);
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, parse_component_media_types(jo, &spec));
  json_value_free(jv);

  /* 5. Non-object value in mediaTypes (mt_obj == NULL) */
  jv = json_parse_string("{\"mediaTypes\": {\"application/json\": 123}}");
  ASSERT_NEQ(NULL, jv);
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, parse_component_media_types(jo, &spec));
  openapi_spec_free(&spec);
  json_value_free(jv);

  /* 6. Invalid media type object (conflicting encoding and prefixEncoding) */
  jv = json_parse_string(
      "{\"mediaTypes\": {\"application/json\": {\"encoding\": {}, "
      "\"prefixEncoding\": []}}}");
  ASSERT_NEQ(NULL, jv);
  jo = json_value_get_object(jv);
  ASSERT_NEQ(CDD_C_SUCCESS, parse_component_media_types(jo, &spec));
  openapi_spec_free(&spec);
  json_value_free(jv);

  /* 7. Valid media type object */
  jv = json_parse_string("{\"mediaTypes\": {\"application/json\": {\"schema\": "
                         "{\"type\": \"string\"}}}}");
  ASSERT_NEQ(NULL, jv);
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, parse_component_media_types(jo, &spec));
  openapi_spec_free(&spec);

  /* 8. OOM on calloc 1 */
  g_cdd_alloc_fail = 1;
  rc = parse_component_media_types(jo, &spec);
  g_cdd_alloc_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  openapi_spec_free(&spec);

  /* 9. OOM on calloc 2 */
  g_cdd_alloc_fail = 2;
  rc = parse_component_media_types(jo, &spec);
  g_cdd_alloc_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  openapi_spec_free(&spec);

  /* 10. OOM on strdup 1 */
  g_cdd_strdup_fail = 1;
  rc = parse_component_media_types(jo, &spec);
  g_cdd_strdup_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  openapi_spec_free(&spec);

  /* 11. OOM on strdup 2 */
  g_cdd_strdup_fail = 2;
  rc = parse_component_media_types(jo, &spec);
  g_cdd_strdup_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  openapi_spec_free(&spec);

  json_value_free(jv);
  PASS();
}

/**
 * @brief Test parse_component_examples edge cases and branches.
 *
 * @return GREATEST_TEST_RES
 */
TEST test_openapi_component_examples_branches(void) {
  struct OpenAPI_Spec spec;
  JSON_Value *jv = NULL;
  const JSON_Object *jo = NULL;
  cdd_c_error_t rc;

  memset(&spec, 0, sizeof(spec));

  /* 1. NULL components or NULL spec */
  ASSERT_EQ(CDD_C_SUCCESS, parse_component_examples(NULL, &spec));

  jv = json_parse_string("{}");
  ASSERT_NEQ(NULL, jv);
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, parse_component_examples(jo, NULL));

  /* 2. No examples property */
  ASSERT_EQ(CDD_C_SUCCESS, parse_component_examples(jo, &spec));
  json_value_free(jv);

  /* 3. Invalid component key map */
  jv = json_parse_string("{\"examples\": {\"bad key!\": {}}}");
  ASSERT_NEQ(NULL, jv);
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, parse_component_examples(jo, &spec));
  json_value_free(jv);

  /* 4. Empty examples */
  jv = json_parse_string("{\"examples\": {}}");
  ASSERT_NEQ(NULL, jv);
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, parse_component_examples(jo, &spec));
  json_value_free(jv);

  /* 5. Non-object value in examples (ex_obj == NULL) */
  jv = json_parse_string("{\"examples\": {\"ex1\": 123}}");
  ASSERT_NEQ(NULL, jv);
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, parse_component_examples(jo, &spec));
  openapi_spec_free(&spec);
  json_value_free(jv);

  /* 6. Invalid example object (parse_example_object error: both dataValue and
   * value) */
  jv = json_parse_string("{\"examples\": {\"ex1\": {\"dataValue\": \"foo\", "
                         "\"value\": \"bar\"}}}");
  ASSERT_NEQ(NULL, jv);
  jo = json_value_get_object(jv);
  ASSERT_NEQ(CDD_C_SUCCESS, parse_component_examples(jo, &spec));
  openapi_spec_free(&spec);
  json_value_free(jv);

  /* 7. Valid example object */
  jv = json_parse_string(
      "{\"examples\": {\"ex1\": {\"summary\": \"sample\", \"value\": 42}}}");
  ASSERT_NEQ(NULL, jv);
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, parse_component_examples(jo, &spec));
  openapi_spec_free(&spec);

  /* 8. OOM on calloc 1 */
  g_cdd_alloc_fail = 1;
  rc = parse_component_examples(jo, &spec);
  g_cdd_alloc_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  openapi_spec_free(&spec);

  /* 9. OOM on calloc 2 */
  g_cdd_alloc_fail = 2;
  rc = parse_component_examples(jo, &spec);
  g_cdd_alloc_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  openapi_spec_free(&spec);

  /* 10. OOM on strdup */
  g_cdd_strdup_fail = 1;
  rc = parse_component_examples(jo, &spec);
  g_cdd_strdup_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  openapi_spec_free(&spec);

  json_value_free(jv);
  PASS();
}

/**
 * @brief Test parse_component_links edge cases and branches.
 *
 * @return GREATEST_TEST_RES
 */
TEST test_openapi_component_links_branches(void) {
  struct OpenAPI_Spec spec;
  JSON_Value *jv = NULL;
  const JSON_Object *jo = NULL;
  cdd_c_error_t rc;

  memset(&spec, 0, sizeof(spec));

  /* 1. NULL components or NULL spec */
  ASSERT_EQ(CDD_C_SUCCESS, parse_component_links(NULL, &spec));

  jv = json_parse_string("{}");
  ASSERT_NEQ(NULL, jv);
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, parse_component_links(jo, NULL));

  /* 2. No links property */
  ASSERT_EQ(CDD_C_SUCCESS, parse_component_links(jo, &spec));
  json_value_free(jv);

  /* 3. Invalid component key map */
  jv = json_parse_string("{\"links\": {\"bad key!\": {}}}");
  ASSERT_NEQ(NULL, jv);
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, parse_component_links(jo, &spec));
  json_value_free(jv);

  /* 4. Empty links */
  jv = json_parse_string("{\"links\": {}}");
  ASSERT_NEQ(NULL, jv);
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, parse_component_links(jo, &spec));
  json_value_free(jv);

  /* 5. Non-object value in links (link_obj == NULL) */
  jv = json_parse_string("{\"links\": {\"l1\": 123}}");
  ASSERT_NEQ(NULL, jv);
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, parse_component_links(jo, &spec));
  openapi_spec_free(&spec);
  json_value_free(jv);

  /* 6. Invalid link object (both operationId and operationRef) */
  jv = json_parse_string(
      "{\"links\": {\"l1\": {\"operationId\": \"op\", \"operationRef\": "
      "\"ref\"}}}");
  ASSERT_NEQ(NULL, jv);
  jo = json_value_get_object(jv);
  ASSERT_NEQ(CDD_C_SUCCESS, parse_component_links(jo, &spec));
  openapi_spec_free(&spec);
  json_value_free(jv);

  /* 7. Valid link object */
  jv = json_parse_string(
      "{\"links\": {\"GetUser\": {\"operationId\": \"getUser\"}}}");
  ASSERT_NEQ(NULL, jv);
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, parse_component_links(jo, &spec));
  openapi_spec_free(&spec);

  /* 8. OOM on calloc */
  g_cdd_alloc_fail = 1;
  rc = parse_component_links(jo, &spec);
  g_cdd_alloc_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  openapi_spec_free(&spec);

  /* 9. OOM on strdup */
  g_cdd_strdup_fail = 1;
  rc = parse_component_links(jo, &spec);
  g_cdd_strdup_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  openapi_spec_free(&spec);

  json_value_free(jv);
  PASS();
}

/**
 * @brief Test parse_component_callbacks edge cases and branches.
 *
 * @return GREATEST_TEST_RES
 */
TEST test_openapi_component_callbacks_branches(void) {
  struct OpenAPI_Spec spec;
  JSON_Value *jv = NULL;
  const JSON_Object *jo = NULL;
  cdd_c_error_t rc;

  memset(&spec, 0, sizeof(spec));

  /* 1. NULL components or NULL spec */
  ASSERT_EQ(CDD_C_SUCCESS, parse_component_callbacks(NULL, &spec));

  jv = json_parse_string("{}");
  ASSERT_NEQ(NULL, jv);
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, parse_component_callbacks(jo, NULL));

  /* 2. No callbacks property */
  ASSERT_EQ(CDD_C_SUCCESS, parse_component_callbacks(jo, &spec));
  json_value_free(jv);

  /* 3. Invalid component key map */
  jv = json_parse_string("{\"callbacks\": {\"bad key!\": {}}}");
  ASSERT_NEQ(NULL, jv);
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, parse_component_callbacks(jo, &spec));
  json_value_free(jv);

  /* 4. Empty callbacks */
  jv = json_parse_string("{\"callbacks\": {}}");
  ASSERT_NEQ(NULL, jv);
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, parse_component_callbacks(jo, &spec));
  json_value_free(jv);

  /* 5. Non-object value in callbacks (cb_obj == NULL) */
  jv = json_parse_string("{\"callbacks\": {\"cb1\": 123}}");
  ASSERT_NEQ(NULL, jv);
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, parse_component_callbacks(jo, &spec));
  openapi_spec_free(&spec);
  json_value_free(jv);

  /* 6. Invalid callback object (parse_callback_object error: empty responses in
   * post) */
  jv = json_parse_string(
      "{\"callbacks\": {\"cb1\": {\"{$request.query.url}\": {\"post\": "
      "{\"responses\": {}}}}}}");
  ASSERT_NEQ(NULL, jv);
  jo = json_value_get_object(jv);
  ASSERT_NEQ(CDD_C_SUCCESS, parse_component_callbacks(jo, &spec));
  openapi_spec_free(&spec);
  json_value_free(jv);

  /* 7. Valid callback object */
  jv = json_parse_string(
      "{\"callbacks\": {\"cb1\": {\"{$request.query.url}\": {\"post\": "
      "{\"responses\": {\"200\": {\"description\": \"ok\"}}}}}}}");
  ASSERT_NEQ(NULL, jv);
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, parse_component_callbacks(jo, &spec));
  openapi_spec_free(&spec);

  /* 8. OOM on calloc */
  g_cdd_alloc_fail = 1;
  rc = parse_component_callbacks(jo, &spec);
  g_cdd_alloc_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  openapi_spec_free(&spec);

  /* 9. OOM on strdup */
  g_cdd_strdup_fail = 1;
  rc = parse_component_callbacks(jo, &spec);
  g_cdd_strdup_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  openapi_spec_free(&spec);

  json_value_free(jv);
  PASS();
}

/**
 * @brief Test parse_component_path_items edge cases and branches.
 *
 * @return GREATEST_TEST_RES
 */
TEST test_openapi_component_path_items_branches(void) {
  struct OpenAPI_Spec spec;
  JSON_Value *jv = NULL;
  const JSON_Object *jo = NULL;
  cdd_c_error_t rc;

  memset(&spec, 0, sizeof(spec));

  /* 1. NULL components or NULL spec */
  ASSERT_EQ(CDD_C_SUCCESS, parse_component_path_items(NULL, &spec));

  jv = json_parse_string("{}");
  ASSERT_NEQ(NULL, jv);
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, parse_component_path_items(jo, NULL));

  /* 2. No pathItems property */
  ASSERT_EQ(CDD_C_SUCCESS, parse_component_path_items(jo, &spec));
  json_value_free(jv);

  /* 3. Invalid component key map (starts with slash, invalid for component key)
   */
  jv = json_parse_string("{\"pathItems\": {\"/invalid_slash_key\": {}}}");
  ASSERT_NEQ(NULL, jv);
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            parse_component_path_items(jo, &spec));
  json_value_free(jv);

  /* 4. parse_paths_object error */
  jv = json_parse_string("{\"pathItems\": {\"item\": {\"get\": 123}}}");
  ASSERT_NEQ(NULL, jv);
  jo = json_value_get_object(jv);
  ASSERT_NEQ(CDD_C_SUCCESS, parse_component_path_items(jo, &spec));
  openapi_spec_free(&spec);
  json_value_free(jv);

  /* 5. Empty pathItems */
  jv = json_parse_string("{\"pathItems\": {}}");
  ASSERT_NEQ(NULL, jv);
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, parse_component_path_items(jo, &spec));
  json_value_free(jv);

  /* 6. Valid pathItems object */
  jv = json_parse_string("{\"pathItems\": {\"item\": {\"get\": {\"responses\": "
                         "{\"200\": {\"description\": \"ok\"}}}}}}");
  ASSERT_NEQ(NULL, jv);
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, parse_component_path_items(jo, &spec));
  openapi_spec_free(&spec);

  /* 7. OOM on calloc for component_path_item_names */
  g_cdd_alloc_fail = 2;
  rc = parse_component_path_items(jo, &spec);
  g_cdd_alloc_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  openapi_spec_free(&spec);

  /* 8. OOM on strdup for component_path_item_names[i] */
  g_cdd_strdup_fail = 2;
  rc = parse_component_path_items(jo, &spec);
  g_cdd_strdup_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  openapi_spec_free(&spec);

  json_value_free(jv);
  PASS();
}

/**
 * @brief Test parse_components full error percolation and branches.
 *
 * @return GREATEST_TEST_RES
 */
TEST test_openapi_components_full_coverage(void) {
  struct OpenAPI_Spec spec;
  JSON_Value *jv = NULL;
  const JSON_Object *jo = NULL;
  cdd_c_error_t rc;
  int k;

  memset(&spec, 0, sizeof(spec));

  /* 1. NULL components or spec */
  ASSERT_EQ(CDD_C_SUCCESS, parse_components(NULL, &spec));

  jv = json_parse_string("{}");
  ASSERT_NEQ(NULL, jv);
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, parse_components(jo, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, parse_components(jo, &spec));
  json_value_free(jv);

  /* 2. Error percolation for each sub-parser in parse_components */
  /* 2a. parse_security_schemes error */
  jv = json_parse_string("{\"securitySchemes\": {\"bad key!\": {}}}");
  jo = json_value_get_object(jv);
  ASSERT_NEQ(CDD_C_SUCCESS, parse_components(jo, &spec));
  json_value_free(jv);

  /* 2b. parse_component_parameters error */
  jv = json_parse_string("{\"parameters\": {\"bad key!\": {}}}");
  jo = json_value_get_object(jv);
  ASSERT_NEQ(CDD_C_SUCCESS, parse_components(jo, &spec));
  json_value_free(jv);

  /* 2c. parse_component_responses error */
  jv = json_parse_string("{\"responses\": {\"bad key!\": {}}}");
  jo = json_value_get_object(jv);
  ASSERT_NEQ(CDD_C_SUCCESS, parse_components(jo, &spec));
  json_value_free(jv);

  /* 2d. parse_component_headers error */
  jv = json_parse_string("{\"headers\": {\"bad key!\": {}}}");
  jo = json_value_get_object(jv);
  ASSERT_NEQ(CDD_C_SUCCESS, parse_components(jo, &spec));
  json_value_free(jv);

  /* 2e. parse_component_request_bodies error */
  jv = json_parse_string("{\"requestBodies\": {\"bad key!\": {}}}");
  jo = json_value_get_object(jv);
  ASSERT_NEQ(CDD_C_SUCCESS, parse_components(jo, &spec));
  json_value_free(jv);

  /* 2f. parse_component_media_types error */
  jv = json_parse_string("{\"mediaTypes\": {\"bad key with spaces\": {}}}");
  jo = json_value_get_object(jv);
  ASSERT_NEQ(CDD_C_SUCCESS, parse_components(jo, &spec));
  json_value_free(jv);

  /* 2g. parse_component_examples error */
  jv = json_parse_string("{\"examples\": {\"bad key!\": {}}}");
  jo = json_value_get_object(jv);
  ASSERT_NEQ(CDD_C_SUCCESS, parse_components(jo, &spec));
  json_value_free(jv);

  /* 2h. parse_component_links error */
  jv = json_parse_string("{\"links\": {\"bad key!\": {}}}");
  jo = json_value_get_object(jv);
  ASSERT_NEQ(CDD_C_SUCCESS, parse_components(jo, &spec));
  json_value_free(jv);

  /* 2i. parse_component_callbacks error */
  jv = json_parse_string("{\"callbacks\": {\"bad key!\": {}}}");
  jo = json_value_get_object(jv);
  ASSERT_NEQ(CDD_C_SUCCESS, parse_components(jo, &spec));
  json_value_free(jv);

  /* 2j. parse_component_path_items error */
  jv = json_parse_string("{\"pathItems\": {\"bad key!\": {}}}");
  jo = json_value_get_object(jv);
  ASSERT_NEQ(CDD_C_SUCCESS, parse_components(jo, &spec));
  json_value_free(jv);

  /* 3. schemas key validation error */
  jv = json_parse_string("{\"schemas\": {\"bad key!\": {}}}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, parse_components(jo, &spec));
  json_value_free(jv);

  /* 4. schemas empty */
  jv = json_parse_string("{\"schemas\": {}}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, parse_components(jo, &spec));
  json_value_free(jv);

  /* 5. swagger definitions */
  spec.swagger_version = (char *)(size_t) "2.0";
  jv = json_parse_string(
      "{\"definitions\": {\"User\": {\"type\": \"object\", \"properties\": "
      "{\"id\": {\"type\": \"integer\"}}}}}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, parse_components(jo, &spec));
  spec.swagger_version = NULL;
  openapi_spec_free(&spec);
  json_value_free(jv);

  /* 6. schemas with struct_compatible + raw (composition) + primitive (raw
   * only) */
  /* and $id, $anchor, $dynamicAnchor */
  jv = json_parse_string(
      "{\"schemas\": {"
      "\"StructOnly\": {\"type\": \"object\", \"$id\": \"http://id\", "
      "\"$anchor\": "
      "\"anc\", \"$dynamicAnchor\": \"danc\", \"properties\": {\"a\": "
      "{\"type\": "
      "\"string\"}}},"
      "\"RawOnly\": {\"type\": \"string\"},"
      "\"Both\": {\"type\": \"object\", \"allOf\": [{\"type\": \"object\"}]}"
      "}}");
  ASSERT_NEQ(NULL, jv);
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, parse_components(jo, &spec));
  openapi_spec_free(&spec);

  /* 7. OOM sweeps across schema allocations */
  for (k = 1; k <= 10; ++k) {
    memset(&spec, 0, sizeof(spec));
    g_cdd_alloc_fail = k;
    rc = parse_components(jo, &spec);
    g_cdd_alloc_fail = 0;
    ASSERT(rc == CDD_C_SUCCESS || rc == CDD_C_ERROR_MEMORY);
    openapi_spec_free(&spec);
  }

  /* 8. strdup OOM sweeps */
  for (k = 1; k <= 8; ++k) {
    memset(&spec, 0, sizeof(spec));
    g_cdd_strdup_fail = k;
    rc = parse_components(jo, &spec);
    g_cdd_strdup_fail = 0;
    ASSERT(rc == CDD_C_SUCCESS || rc == CDD_C_ERROR_MEMORY);
    openapi_spec_free(&spec);
  }

  /* 9. raw_json serialize failure */
  {
    JSON_Value *jv_raw = json_parse_string(
        "{\"schemas\": {\"RawOnly\": {\"type\": \"string\"}}}");
    const JSON_Object *jo_raw = json_value_get_object(jv_raw);
    json_set_allocation_functions(mock_components_parson_malloc,
                                  mock_components_parson_free);
    c2openapi_set_json_allocators(mock_components_parson_malloc,
                                  mock_components_parson_free);
    g_components_parson_oom_fail_at = 0;
    memset(&spec, 0, sizeof(spec));
    rc = parse_components(jo_raw, &spec);
    g_components_parson_oom_fail_at = -1;
    json_set_allocation_functions(malloc, free);
    c2openapi_set_json_allocators(malloc, free);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    openapi_spec_free(&spec);
    json_value_free(jv_raw);
  }

  /* 10. Exercise mock allocator branches directly */
  {
    void *p;
    g_components_parson_oom_fail_at = 1;
    p = mock_components_parson_malloc(16);
    ASSERT_NEQ(NULL, p);
    mock_components_parson_free(p);
    p = mock_components_parson_malloc(16);
    ASSERT_EQ(NULL, p);
    g_components_parson_oom_fail_at = -1;
    p = mock_components_parson_malloc(16);
    ASSERT_NEQ(NULL, p);
    mock_components_parson_free(p);
  }

  json_value_free(jv);
  PASS();
}

#define OPENAPI_COMPONENTS_COVERAGE_TESTS()                                    \
  RUN_TEST(test_openapi_component_parameters_branches);                        \
  RUN_TEST(test_openapi_component_responses_branches);                         \
  RUN_TEST(test_openapi_component_headers_branches);                           \
  RUN_TEST(test_openapi_component_request_bodies_branches);                    \
  RUN_TEST(test_openapi_component_media_types_branches);                       \
  RUN_TEST(test_openapi_component_examples_branches);                          \
  RUN_TEST(test_openapi_component_links_branches);                             \
  RUN_TEST(test_openapi_component_callbacks_branches);                         \
  RUN_TEST(test_openapi_component_path_items_branches);                        \
  RUN_TEST(test_openapi_components_full_coverage)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_OPENAPI_COMPONENTS_COVERAGE_H */
