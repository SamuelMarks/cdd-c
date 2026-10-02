/**
 * @file test_openapi_headers_links_coverage.h
 * @brief Comprehensive 100% test coverage for openapi_headers_links.c.
 * @author Samuel Marks
 */

#ifndef TEST_OPENAPI_HEADERS_LINKS_COVERAGE_H
#define TEST_OPENAPI_HEADERS_LINKS_COVERAGE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "parse/test_openapi_loader_common.h"
#include "openapi/parse/openapi_test_helpers.h"
#include "openapi/parse/openapi_internal_components.h"
/* clang-format on */

static void cleanup_link_params(struct OpenAPI_LinkParam *params,
                                size_t count) {
  size_t i;
  if (!params)
    return;
  for (i = 0; i < count; ++i) {
    if (params[i].name)
      free(params[i].name);
    cdd_test_free_any_value(&params[i].value);
  }
  free(params);
}

static void cleanup_links_array(struct OpenAPI_Link *links, size_t count) {
  size_t i;
  if (!links)
    return;
  for (i = 0; i < count; ++i)
    cdd_test_free_link(&links[i]);
  free(links);
}

static void cleanup_headers_array(struct OpenAPI_Header *headers,
                                  size_t count) {
  size_t i;
  if (!headers)
    return;
  for (i = 0; i < count; ++i)
    cdd_test_free_header(&headers[i]);
  free(headers);
}

static void cleanup_encodings_array(struct OpenAPI_Encoding *encs,
                                    size_t count) {
  size_t i;
  if (!encs)
    return;
  for (i = 0; i < count; ++i)
    cdd_test_free_encoding(&encs[i]);
  free(encs);
}

/**
 * @brief Test parse_header_object comprehensive branches.
 *
 * @return GREATEST_TEST_RES
 */
TEST test_openapi_header_object_full_branches(void) {
  struct OpenAPI_Header hdr;
  struct OpenAPI_Spec spec;
  JSON_Value *jv = NULL;
  const JSON_Object *jo = NULL;
  cdd_c_error_t rc = 0;

  memset(&hdr, 0, sizeof(hdr));
  memset(&spec, 0, sizeof(spec));

  /* 1. NULL input checks */
  ASSERT_EQ(CDD_C_SUCCESS, parse_header_object(NULL, &hdr, NULL, 0));
  jv = json_parse_string("{}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, parse_header_object(jo, NULL, NULL, 0));
  json_value_free(jv);

  /* 2. $ref branches */
  spec.component_headers =
      (struct OpenAPI_Header *)calloc(1, sizeof(struct OpenAPI_Header));
  spec.component_headers[0].name = strdup("MyHeader");
  spec.component_headers[0].description = strdup("comp desc");
  spec.component_headers[0].type = strdup("string");
  spec.n_component_headers = 1;
  spec.component_header_names = (char **)calloc(1, sizeof(char *));
  spec.component_header_names[0] = strdup("MyHeader");

  jv = json_parse_string("{\"$ref\": \"#/components/headers/MyHeader\", "
                         "\"description\": \"overridden desc\"}");
  jo = json_value_get_object(jv);
  memset(&hdr, 0, sizeof(hdr));
  ASSERT_EQ(CDD_C_SUCCESS, parse_header_object(jo, &hdr, &spec, 1));
  cdd_test_free_header(&hdr);

  /* 2b. $ref OOM on ref strdup */
  g_cdd_strdup_fail = 1;
  memset(&hdr, 0, sizeof(hdr));
  rc = parse_header_object(jo, &hdr, &spec, 1);
  g_cdd_strdup_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  cdd_test_free_header(&hdr);

  /* 2c. $ref OOM on description strdup */
  g_cdd_strdup_fail = 2;
  memset(&hdr, 0, sizeof(hdr));
  rc = parse_header_object(jo, &hdr, &spec, 0);
  g_cdd_strdup_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  cdd_test_free_header(&hdr);

  /* 2d. $ref copy_header_fields failure */
  g_cdd_strdup_fail = 2;
  memset(&hdr, 0, sizeof(hdr));
  rc = parse_header_object(jo, &hdr, &spec, 1);
  g_cdd_strdup_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  cdd_test_free_header(&hdr);
  json_value_free(jv);
  openapi_spec_free(&spec);

  /* 3. description without ref, required, deprecated, explode, style */
  jv = json_parse_string(
      "{\"description\": \"header desc\", \"required\": true, \"deprecated\": "
      "true, \"explode\": false, \"style\": \"simple\"}");
  jo = json_value_get_object(jv);
  memset(&hdr, 0, sizeof(hdr));
  ASSERT_EQ(CDD_C_SUCCESS, parse_header_object(jo, &hdr, NULL, 0));
  ASSERT_EQ(1, hdr.required);
  ASSERT_EQ(1, hdr.deprecated_set);
  ASSERT_EQ(1, hdr.deprecated);
  ASSERT_EQ(1, hdr.explode_set);
  ASSERT_EQ(0, hdr.explode);
  ASSERT_EQ(1, hdr.style_set);
  ASSERT_EQ(OA_STYLE_SIMPLE, hdr.style);
  cdd_test_free_header(&hdr);
  json_value_free(jv);

  /* 4. Invalid style (must be simple) */
  jv = json_parse_string("{\"style\": \"form\"}");
  jo = json_value_get_object(jv);
  memset(&hdr, 0, sizeof(hdr));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            parse_header_object(jo, &hdr, NULL, 0));
  json_value_free(jv);

  /* 5. Both schema and content present */
  jv = json_parse_string("{\"schema\": {\"type\": \"string\"}, \"content\": "
                         "{\"application/json\": {}}}");
  jo = json_value_get_object(jv);
  memset(&hdr, 0, sizeof(hdr));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            parse_header_object(jo, &hdr, NULL, 0));
  json_value_free(jv);

  /* 6. Content with count != 1 */
  jv = json_parse_string("{\"content\": {}}");
  jo = json_value_get_object(jv);
  memset(&hdr, 0, sizeof(hdr));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            parse_header_object(jo, &hdr, NULL, 0));
  json_value_free(jv);

  /* 7. Content with valid single media type */
  jv = json_parse_string("{\"content\": {\"application/json\": {\"schema\": "
                         "{\"type\": \"integer\"}}}}");
  jo = json_value_get_object(jv);
  memset(&hdr, 0, sizeof(hdr));
  ASSERT_EQ(CDD_C_SUCCESS, parse_header_object(jo, &hdr, NULL, 0));
  ASSERT_STR_EQ("application/json", hdr.content_type);
  cdd_test_free_header(&hdr);

  /* 7b. Content media_type strdup OOM */
  g_cdd_strdup_fail = 1;
  memset(&hdr, 0, sizeof(hdr));
  rc = parse_header_object(jo, &hdr, NULL, 0);
  g_cdd_strdup_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  cdd_test_free_header(&hdr);
  json_value_free(jv);

  /* 8. Content with media_ref ($ref in media_obj) targeting Json */
  memset(&spec, 0, sizeof(spec));
  spec.component_media_types =
      (struct OpenAPI_MediaType *)calloc(1, sizeof(struct OpenAPI_MediaType));
  spec.component_media_types[0].name = strdup("Json");
  spec.component_media_types[0].schema_set = 1;
  spec.component_media_types[0].schema.inline_type = strdup("number");
  spec.n_component_media_types = 1;
  spec.component_media_type_names = (char **)calloc(1, sizeof(char *));
  spec.component_media_type_names[0] = strdup("Json");

  jv = json_parse_string("{\"content\": {\"application/json\": {\"$ref\": "
                         "\"#/components/mediaTypes/Json\"}}}");
  jo = json_value_get_object(jv);
  memset(&hdr, 0, sizeof(hdr));
  ASSERT_EQ(CDD_C_SUCCESS, parse_header_object(jo, &hdr, &spec, 1));
  cdd_test_free_header(&hdr);

  /* 8b. Content with item_schema_set instead of schema_set */
  if (spec.component_media_types[0].schema.inline_type) {
    free(spec.component_media_types[0].schema.inline_type);
    spec.component_media_types[0].schema.inline_type = NULL;
  }
  spec.component_media_types[0].schema_set = 0;
  spec.component_media_types[0].item_schema_set = 1;
  spec.component_media_types[0].item_schema.inline_type = strdup("boolean");
  memset(&hdr, 0, sizeof(hdr));
  ASSERT_EQ(CDD_C_SUCCESS, parse_header_object(jo, &hdr, &spec, 1));
  cdd_test_free_header(&hdr);

  /* 8e. Content with neither schema_set nor item_schema_set */
  if (spec.component_media_types[0].item_schema.inline_type) {
    free(spec.component_media_types[0].item_schema.inline_type);
    spec.component_media_types[0].item_schema.inline_type = NULL;
  }
  spec.component_media_types[0].schema_set = 0;
  spec.component_media_types[0].item_schema_set = 0;
  memset(&hdr, 0, sizeof(hdr));
  ASSERT_EQ(CDD_C_SUCCESS, parse_header_object(jo, &hdr, &spec, 1));
  cdd_test_free_header(&hdr);

  /* 8d. Content with media_ref and resolve_refs = 0 */
  memset(&hdr, 0, sizeof(hdr));
  ASSERT_EQ(CDD_C_SUCCESS, parse_header_object(jo, &hdr, &spec, 0));
  cdd_test_free_header(&hdr);

  /* 8c. Content media_ref strdup OOM */
  g_cdd_strdup_fail = 2;
  memset(&hdr, 0, sizeof(hdr));
  rc = parse_header_object(jo, &hdr, &spec, 1);
  g_cdd_strdup_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  cdd_test_free_header(&hdr);
  json_value_free(jv);
  openapi_spec_free(&spec);

  /* 9. Swagger 2.0 type on header directly */
  memset(&spec, 0, sizeof(spec));
  spec.swagger_version = (char *)(size_t) "2.0";
  jv = json_parse_string("{\"type\": \"integer\", \"format\": \"int64\"}");
  jo = json_value_get_object(jv);
  memset(&hdr, 0, sizeof(hdr));
  ASSERT_EQ(CDD_C_SUCCESS, parse_header_object(jo, &hdr, &spec, 0));
  ASSERT_STR_EQ("integer", hdr.type);
  cdd_test_free_header(&hdr);
  spec.swagger_version = NULL;
  json_value_free(jv);

  /* 10. Boolean schema (JSONBoolean) */
  jv = json_parse_string("{\"schema\": true}");
  jo = json_value_get_object(jv);
  memset(&hdr, 0, sizeof(hdr));
  ASSERT_EQ(CDD_C_SUCCESS, parse_header_object(jo, &hdr, NULL, 0));
  ASSERT_EQ(1, hdr.schema_set);
  ASSERT_EQ(1, hdr.schema.schema_is_boolean);
  ASSERT_EQ(1, hdr.schema.schema_boolean_value);
  cdd_test_free_header(&hdr);
  json_value_free(jv);

  /* 11. Examples and example locations */
  /* 11a. Both example and examples */
  jv = json_parse_string(
      "{\"example\": 1, \"examples\": {\"ex1\": {\"value\": 2}}}");
  jo = json_value_get_object(jv);
  memset(&hdr, 0, sizeof(hdr));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            parse_header_object(jo, &hdr, NULL, 0));
  json_value_free(jv);

  /* 11b. Examples only */
  jv = json_parse_string("{\"examples\": {\"ex1\": {\"value\": \"sample\"}}}");
  jo = json_value_get_object(jv);
  memset(&hdr, 0, sizeof(hdr));
  ASSERT_EQ(CDD_C_SUCCESS, parse_header_object(jo, &hdr, NULL, 0));
  ASSERT_EQ(OA_EXAMPLE_LOC_OBJECT, hdr.example_location);
  cdd_test_free_header(&hdr);
  json_value_free(jv);

  /* 11c. Example only */
  jv = json_parse_string("{\"example\": \"val\"}");
  jo = json_value_get_object(jv);
  memset(&hdr, 0, sizeof(hdr));
  ASSERT_EQ(CDD_C_SUCCESS, parse_header_object(jo, &hdr, NULL, 0));
  ASSERT_EQ(OA_EXAMPLE_LOC_OBJECT, hdr.example_location);
  cdd_test_free_header(&hdr);
  json_value_free(jv);

  /* 11d. Media example location */
  jv = json_parse_string(
      "{\"content\": {\"application/json\": {\"example\": \"media_val\"}}}");
  jo = json_value_get_object(jv);
  memset(&hdr, 0, sizeof(hdr));
  ASSERT_EQ(CDD_C_SUCCESS, parse_header_object(jo, &hdr, NULL, 0));
  ASSERT_EQ(OA_EXAMPLE_LOC_MEDIA, hdr.example_location);
  cdd_test_free_header(&hdr);
  json_value_free(jv);

  /* 12. Extensions */
  jv = json_parse_string(
      "{\"schema\": {\"type\": \"string\"}, \"x-rate-limit\": 100}");
  jo = json_value_get_object(jv);
  memset(&hdr, 0, sizeof(hdr));
  ASSERT_EQ(CDD_C_SUCCESS, parse_header_object(jo, &hdr, NULL, 0));
  ASSERT_NEQ(NULL, hdr.extensions_json);
  cdd_test_free_header(&hdr);
  json_value_free(jv);

  /* 13. Schema error in header */
  jv = json_parse_string("{\"schema\": {\"items\": 123}}");
  jo = json_value_get_object(jv);
  memset(&hdr, 0, sizeof(hdr));
  rc = parse_header_object(jo, &hdr, NULL, 0);
  json_value_free(jv);

  /* 14. OOM sweeps for header schema and copy */
  jv = json_parse_string(
      "{\"schema\": {\"type\": \"string\", \"format\": \"date-time\"}}");
  jo = json_value_get_object(jv);
  {
    int k;
    for (k = 1; k <= 6; ++k) {
      memset(&hdr, 0, sizeof(hdr));
      g_cdd_strdup_fail = k;
      rc = parse_header_object(jo, &hdr, NULL, 0);
      g_cdd_strdup_fail = 0;
      cdd_test_free_header(&hdr);
    }
  }
  json_value_free(jv);

  /* 15. Header content with non-object media_obj */
  jv = json_parse_string("{\"content\": {\"application/json\": 123}}");
  jo = json_value_get_object(jv);
  memset(&hdr, 0, sizeof(hdr));
  rc = parse_header_object(jo, &hdr, NULL, 0);
  cdd_test_free_header(&hdr);
  json_value_free(jv);

  /* 16. Header content sweeps for content_ref, resolved schema, and type */
  {
    int k;
    memset(&spec, 0, sizeof(spec));
    spec.component_media_types =
        (struct OpenAPI_MediaType *)calloc(1, sizeof(struct OpenAPI_MediaType));
    spec.component_media_types[0].name = strdup("Json");
    spec.component_media_types[0].schema_set = 1;
    spec.component_media_types[0].schema.inline_type = strdup("string");
    spec.n_component_media_types = 1;
    spec.component_media_type_names = (char **)calloc(1, sizeof(char *));
    spec.component_media_type_names[0] = strdup("Json");

    jv = json_parse_string("{\"content\": {\"application/json\": {\"$ref\": "
                           "\"#/components/mediaTypes/Json\"} } }");
    jo = json_value_get_object(jv);
    for (k = 1; k <= 15; ++k) {
      memset(&hdr, 0, sizeof(hdr));
      g_cdd_strdup_fail = k;
      rc = parse_header_object(jo, &hdr, &spec, 1);
      g_cdd_strdup_fail = 0;
      cdd_test_free_header(&hdr);
    }
    json_value_free(jv);
    openapi_spec_free(&spec);
  }

  /* 17. parsed_schema_set cleanup branches */
  /* 17a. out_hdr->type strdup failure with empty schema */
  jv = json_parse_string("{\"schema\": {}}");
  jo = json_value_get_object(jv);
  memset(&hdr, 0, sizeof(hdr));
  g_cdd_strdup_fail = 1;
  rc = parse_header_object(jo, &hdr, NULL, 0);
  g_cdd_strdup_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  cdd_test_free_header(&hdr);
  json_value_free(jv);

  /* 17b. object_has_example_and_examples with parsed_schema_set */
  jv = json_parse_string("{\"schema\": {}, \"example\": 1, \"examples\": "
                         "{\"e\": {\"value\": 2}}}");
  jo = json_value_get_object(jv);
  memset(&hdr, 0, sizeof(hdr));
  rc = parse_header_object(jo, &hdr, NULL, 0);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  cdd_test_free_header(&hdr);
  json_value_free(jv);

  /* 17c. parse_any_field failure with parsed_schema_set */
  jv = json_parse_string("{\"schema\": {}, \"example\": \"val\"}");
  jo = json_value_get_object(jv);
  memset(&hdr, 0, sizeof(hdr));
  g_cdd_strdup_fail = 2;
  rc = parse_header_object(jo, &hdr, NULL, 0);
  g_cdd_strdup_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  cdd_test_free_header(&hdr);
  json_value_free(jv);

  /* 17d. parse_media_examples failure at line 270 */
  jv = json_parse_string("{\"content\": {\"application/json\": {\"schema\": "
                         "{}, \"examples\": {\"ex1\": {\"value\": 1}}}}}");
  jo = json_value_get_object(jv);
  {
    int k;
    for (k = 1; k <= 5; ++k) {
      memset(&hdr, 0, sizeof(hdr));
      g_cdd_alloc_fail = k;
      rc = parse_header_object(jo, &hdr, NULL, 0);
      g_cdd_alloc_fail = 0;
      cdd_test_free_header(&hdr);
    }
  }
  json_value_free(jv);

  /* 17e. collect_extensions failure with parsed_schema_set */
  jv = json_parse_string("{\"schema\": {}, \"x-ext\": 1}");
  jo = json_value_get_object(jv);
  memset(&hdr, 0, sizeof(hdr));
  g_cdd_strdup_fail = 2;
  rc = parse_header_object(jo, &hdr, NULL, 0);
  g_cdd_strdup_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  cdd_test_free_header(&hdr);
  json_value_free(jv);

  /* 18. Additional branches for header */
  /* 18a. $ref with resolve_refs = 1 and spec = NULL */
  jv = json_parse_string("{\"$ref\": \"#/components/headers/MyHeader\"}");
  jo = json_value_get_object(jv);
  memset(&hdr, 0, sizeof(hdr));
  ASSERT_EQ(CDD_C_SUCCESS, parse_header_object(jo, &hdr, NULL, 1));
  cdd_test_free_header(&hdr);

  /* 18b. $ref targeting non-existent component header */
  memset(&spec, 0, sizeof(spec));
  memset(&hdr, 0, sizeof(hdr));
  ASSERT_EQ(CDD_C_SUCCESS, parse_header_object(jo, &hdr, &spec, 1));
  cdd_test_free_header(&hdr);
  json_value_free(jv);

  /* 18c. content with media_ref and spec = NULL, and non-existent mediaType */
  jv = json_parse_string("{\"content\": {\"application/json\": {\"$ref\": "
                         "\"#/components/mediaTypes/NonExistent\"}}}");
  jo = json_value_get_object(jv);
  memset(&hdr, 0, sizeof(hdr));
  ASSERT_EQ(CDD_C_SUCCESS, parse_header_object(jo, &hdr, NULL, 1));
  cdd_test_free_header(&hdr);
  memset(&hdr, 0, sizeof(hdr));
  ASSERT_EQ(CDD_C_SUCCESS, parse_header_object(jo, &hdr, &spec, 1));
  cdd_test_free_header(&hdr);
  json_value_free(jv);

  /* 18d. parse_examples_object failure with parsed_schema_set */
  jv = json_parse_string("{\"schema\": {}, \"examples\": {\"ex1\": 123}}");
  jo = json_value_get_object(jv);
  memset(&hdr, 0, sizeof(hdr));
  rc = parse_header_object(jo, &hdr, NULL, 0);
  cdd_test_free_header(&hdr);
  json_value_free(jv);

  /* 18e. parse_examples_object failure with parsed_schema_set == 0 */
  jv = json_parse_string(
      "{\"examples\": {\"ex1\": {\"dataValue\": 1, \"value\": 2}}}");
  jo = json_value_get_object(jv);
  memset(&hdr, 0, sizeof(hdr));
  rc = parse_header_object(jo, &hdr, NULL, 0);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  cdd_test_free_header(&hdr);
  json_value_free(jv);

  /* 18f. parse_any_field failure with parsed_schema_set == 0 */
  jv = json_parse_string("{\"example\": \"val\"}");
  jo = json_value_get_object(jv);
  memset(&hdr, 0, sizeof(hdr));
  g_cdd_strdup_fail = 2;
  rc = parse_header_object(jo, &hdr, NULL, 0);
  g_cdd_strdup_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  cdd_test_free_header(&hdr);
  json_value_free(jv);

  /* 18g. collect_extensions failure with parsed_schema_set == 0 */
  jv = json_parse_string("{\"x-ext\": 1}");
  jo = json_value_get_object(jv);
  memset(&hdr, 0, sizeof(hdr));
  g_cdd_strdup_fail = 2;
  rc = parse_header_object(jo, &hdr, NULL, 0);
  g_cdd_strdup_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  cdd_test_free_header(&hdr);
  json_value_free(jv);

  /* 18h. parse_media_examples failure with parsed_schema_set == 0 */
  jv = json_parse_string("{\"content\": {\"application/json\": {\"examples\": "
                         "{\"ex1\": {\"value\": 1}}}}}");
  jo = json_value_get_object(jv);
  memset(&hdr, 0, sizeof(hdr));
  g_cdd_alloc_fail = 3;
  rc = parse_header_object(jo, &hdr, NULL, 0);
  g_cdd_alloc_fail = 0;
  cdd_test_free_header(&hdr);
  json_value_free(jv);

  PASS();
}

/**
 * @brief Test parse_link_parameters comprehensive branches.
 *
 * @return GREATEST_TEST_RES
 */
TEST test_openapi_link_parameters_full_branches(void) {
  struct OpenAPI_LinkParam *params = NULL;
  size_t count = 0;
  JSON_Value *jv = NULL;
  const JSON_Object *jo = NULL;
  cdd_c_error_t rc = 0;

  /* 1. NULL checks */
  ASSERT_EQ(CDD_C_SUCCESS, parse_link_parameters(NULL, &params, &count));
  ASSERT_EQ(0, count);
  ASSERT_EQ(NULL, params);

  jv = json_parse_string("{}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, parse_link_parameters(jo, NULL, &count));
  ASSERT_EQ(CDD_C_SUCCESS, parse_link_parameters(jo, &params, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, parse_link_parameters(jo, &params, &count));
  ASSERT_EQ(0, count);
  ASSERT_EQ(NULL, params);
  json_value_free(jv);

  /* 2. Valid link parameters */
  jv = json_parse_string("{\"userId\": \"$response.body#/id\", \"limit\": 10}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, parse_link_parameters(jo, &params, &count));
  ASSERT_EQ(2, count);
  ASSERT_STR_EQ("userId", params[0].name);
  ASSERT_STR_EQ("limit", params[1].name);
  cleanup_link_params(params, count);
  params = NULL;

  /* 3. OOM on calloc */
  g_cdd_alloc_fail = 1;
  rc = parse_link_parameters(jo, &params, &count);
  g_cdd_alloc_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);

  /* 4. OOM on strdup */
  g_cdd_strdup_fail = 1;
  rc = parse_link_parameters(jo, &params, &count);
  g_cdd_strdup_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);

  json_value_free(jv);
  PASS();
}

/**
 * @brief Test parse_link_object comprehensive branches.
 *
 * @return GREATEST_TEST_RES
 */
TEST test_openapi_link_object_full_branches(void) {
  struct OpenAPI_Link link;
  struct OpenAPI_Spec spec;
  JSON_Value *jv = NULL;
  const JSON_Object *jo = NULL;
  cdd_c_error_t rc = 0;

  memset(&link, 0, sizeof(link));
  memset(&spec, 0, sizeof(spec));

  /* 1. NULL checks */
  ASSERT_EQ(CDD_C_SUCCESS, parse_link_object(NULL, &link, NULL, 0));
  jv = json_parse_string("{}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, parse_link_object(jo, NULL, NULL, 0));
  json_value_free(jv);

  /* 2. $ref branches */
  spec.component_links =
      (struct OpenAPI_Link *)calloc(1, sizeof(struct OpenAPI_Link));
  spec.component_links[0].name = strdup("MyLink");
  spec.component_links[0].operation_id = strdup("op1");
  spec.n_component_links = 1;

  jv = json_parse_string(
      "{\"$ref\": \"#/components/links/MyLink\", \"summary\": \"link sum\", "
      "\"description\": \"link desc\"}");
  jo = json_value_get_object(jv);
  memset(&link, 0, sizeof(link));
  ASSERT_EQ(CDD_C_SUCCESS, parse_link_object(jo, &link, &spec, 1));
  ASSERT_STR_EQ("#/components/links/MyLink", link.ref);
  ASSERT_STR_EQ("link sum", link.summary);
  ASSERT_STR_EQ("link desc", link.description);
  cdd_test_free_link(&link);

  /* 2b. $ref OOM on ref strdup */
  g_cdd_strdup_fail = 1;
  memset(&link, 0, sizeof(link));
  rc = parse_link_object(jo, &link, &spec, 1);
  g_cdd_strdup_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  cdd_test_free_link(&link);

  /* 2c. $ref OOM on summary */
  g_cdd_strdup_fail = 2;
  memset(&link, 0, sizeof(link));
  rc = parse_link_object(jo, &link, &spec, 0);
  g_cdd_strdup_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  cdd_test_free_link(&link);

  /* 2d. $ref OOM on description */
  g_cdd_strdup_fail = 3;
  memset(&link, 0, sizeof(link));
  rc = parse_link_object(jo, &link, &spec, 0);
  g_cdd_strdup_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  cdd_test_free_link(&link);

  /* 2e. $ref copy_link_fields failure */
  g_cdd_strdup_fail = 2;
  memset(&link, 0, sizeof(link));
  rc = parse_link_object(jo, &link, &spec, 1);
  g_cdd_strdup_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  cdd_test_free_link(&link);
  json_value_free(jv);
  openapi_spec_free(&spec);

  /* 3. Without $ref: summary, description, operationRef, parameters,
   * requestBody, server */
  jv = json_parse_string(
      "{\"summary\": \"link summary\", \"description\": \"link description\", "
      "\"operationRef\": \"#/paths/~1users/get\", \"parameters\": {\"id\": 1}, "
      "\"requestBody\": {\"data\": 42}, \"server\": {\"url\": "
      "\"http://api.example.com\"}, \"x-custom\": \"val\"}");
  jo = json_value_get_object(jv);
  memset(&link, 0, sizeof(link));
  ASSERT_EQ(CDD_C_SUCCESS, parse_link_object(jo, &link, NULL, 0));
  ASSERT_STR_EQ("link summary", link.summary);
  ASSERT_STR_EQ("link description", link.description);
  ASSERT_STR_EQ("#/paths/~1users/get", link.operation_ref);
  ASSERT_EQ(1, link.request_body_set);
  ASSERT_EQ(1, link.server_set);
  ASSERT_NEQ(NULL, link.server);
  cdd_test_free_link(&link);
  json_value_free(jv);

  /* 4. operationId instead of operationRef */
  jv = json_parse_string("{\"operationId\": \"getUser\"}");
  jo = json_value_get_object(jv);
  memset(&link, 0, sizeof(link));
  ASSERT_EQ(CDD_C_SUCCESS, parse_link_object(jo, &link, NULL, 0));
  ASSERT_STR_EQ("getUser", link.operation_id);
  cdd_test_free_link(&link);
  json_value_free(jv);

  /* 5. Invalid: both operationId and operationRef */
  jv = json_parse_string(
      "{\"operationId\": \"getUser\", \"operationRef\": \"#/users\"}");
  jo = json_value_get_object(jv);
  memset(&link, 0, sizeof(link));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            parse_link_object(jo, &link, NULL, 0));
  json_value_free(jv);

  /* 6. Invalid: neither operationId nor operationRef */
  jv = json_parse_string("{\"summary\": \"no op\"}");
  jo = json_value_get_object(jv);
  memset(&link, 0, sizeof(link));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            parse_link_object(jo, &link, NULL, 0));
  cdd_test_free_link(&link);
  json_value_free(jv);

  /* 7. Server calloc OOM */
  jv = json_parse_string("{\"operationId\": \"op\", \"server\": {\"url\": "
                         "\"http://example.com\"}}");
  jo = json_value_get_object(jv);
  g_cdd_alloc_fail = 1;
  memset(&link, 0, sizeof(link));
  rc = parse_link_object(jo, &link, NULL, 0);
  g_cdd_alloc_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  cdd_test_free_link(&link);
  json_value_free(jv);

  /* 8. Server object error and link copy error */
  jv = json_parse_string("{\"operationId\": \"op\", \"server\": {\"url\": "
                         "\"http://example.com/{var_missing_def}\"}}");
  jo = json_value_get_object(jv);
  memset(&link, 0, sizeof(link));
  rc = parse_link_object(jo, &link, NULL, 0);
  cdd_test_free_link(&link);
  json_value_free(jv);

  /* 8b. Link parameters error */
  jv = json_parse_string(
      "{\"operationId\": \"op\", \"parameters\": {\"p1\": 123}}");
  jo = json_value_get_object(jv);
  g_cdd_strdup_fail = 1;
  memset(&link, 0, sizeof(link));
  rc = parse_link_object(jo, &link, NULL, 0);
  g_cdd_strdup_fail = 0;
  cdd_test_free_link(&link);
  json_value_free(jv);

  /* 9. summary OOM without ref */
  jv =
      json_parse_string("{\"summary\": \"link sum\", \"operationId\": \"op\"}");
  jo = json_value_get_object(jv);
  memset(&link, 0, sizeof(link));
  g_cdd_strdup_fail = 1;
  rc = parse_link_object(jo, &link, NULL, 0);
  g_cdd_strdup_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  cdd_test_free_link(&link);
  json_value_free(jv);

  /* 10. operationRef OOM without ref */
  jv = json_parse_string("{\"operationRef\": \"#/users\"}");
  jo = json_value_get_object(jv);
  memset(&link, 0, sizeof(link));
  g_cdd_strdup_fail = 1;
  rc = parse_link_object(jo, &link, NULL, 0);
  g_cdd_strdup_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  cdd_test_free_link(&link);
  json_value_free(jv);

  /* 11. collect_extensions error */
  jv = json_parse_string("{\"operationId\": \"op\", \"x-ext\": 1}");
  jo = json_value_get_object(jv);
  memset(&link, 0, sizeof(link));
  g_cdd_strdup_fail = 2;
  rc = parse_link_object(jo, &link, NULL, 0);
  g_cdd_strdup_fail = 0;
  cdd_test_free_link(&link);
  json_value_free(jv);

  /* 12. Additional branches for link */
  /* 12a. $ref with resolve_refs = 1 and spec = NULL */
  jv = json_parse_string("{\"$ref\": \"#/components/links/MyLink\"}");
  jo = json_value_get_object(jv);
  memset(&link, 0, sizeof(link));
  ASSERT_EQ(CDD_C_SUCCESS, parse_link_object(jo, &link, NULL, 1));
  cdd_test_free_link(&link);

  /* 12b. $ref targeting non-existent component link */
  memset(&spec, 0, sizeof(spec));
  memset(&link, 0, sizeof(link));
  ASSERT_EQ(CDD_C_SUCCESS, parse_link_object(jo, &link, &spec, 1));
  cdd_test_free_link(&link);
  json_value_free(jv);

  PASS();
}

/**
 * @brief Test parse_links_object comprehensive branches.
 *
 * @return GREATEST_TEST_RES
 */
TEST test_openapi_links_object_full_branches(void) {
  struct OpenAPI_Link *links = NULL;
  size_t count = 0;
  JSON_Value *jv = NULL;
  const JSON_Object *jo = NULL;
  cdd_c_error_t rc = 0;

  /* 1. NULL checks */
  ASSERT_EQ(CDD_C_SUCCESS, parse_links_object(NULL, &links, &count, NULL, 0));
  ASSERT_EQ(0, count);
  ASSERT_EQ(NULL, links);

  jv = json_parse_string("{}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, parse_links_object(jo, NULL, &count, NULL, 0));
  ASSERT_EQ(CDD_C_SUCCESS, parse_links_object(jo, &links, NULL, NULL, 0));
  ASSERT_EQ(CDD_C_SUCCESS, parse_links_object(jo, &links, &count, NULL, 0));
  ASSERT_EQ(0, count);
  ASSERT_EQ(NULL, links);
  json_value_free(jv);

  /* 2. Valid links */
  jv = json_parse_string(
      "{\"LinkA\": {\"operationId\": \"opA\"}, \"LinkB\": 123}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, parse_links_object(jo, &links, &count, NULL, 0));
  ASSERT_EQ(2, count);
  ASSERT_STR_EQ("LinkA", links[0].name);
  ASSERT_STR_EQ("LinkB", links[1].name);
  cleanup_links_array(links, count);
  links = NULL;

  /* 3. OOM on calloc */
  g_cdd_alloc_fail = 1;
  rc = parse_links_object(jo, &links, &count, NULL, 0);
  g_cdd_alloc_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);

  /* 4. OOM on strdup */
  g_cdd_strdup_fail = 1;
  rc = parse_links_object(jo, &links, &count, NULL, 0);
  g_cdd_strdup_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  json_value_free(jv);

  /* 5. Error in parse_link_object */
  jv = json_parse_string("{\"LinkErr\": {}}");
  jo = json_value_get_object(jv);
  rc = parse_links_object(jo, &links, &count, NULL, 0);
  ASSERT_NEQ(CDD_C_SUCCESS, rc);
  json_value_free(jv);

  PASS();
}

/**
 * @brief Test parse_headers_object comprehensive branches.
 *
 * @return GREATEST_TEST_RES
 */
TEST test_openapi_headers_object_full_branches(void) {
  struct OpenAPI_Header *headers = NULL;
  size_t count = 0;
  JSON_Value *jv = NULL;
  const JSON_Object *jo = NULL;
  cdd_c_error_t rc = 0;

  /* 1. NULL checks */
  ASSERT_EQ(CDD_C_SUCCESS,
            parse_headers_object(NULL, &headers, &count, NULL, 0, 0));
  ASSERT_EQ(0, count);
  ASSERT_EQ(NULL, headers);

  jv = json_parse_string("{}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, parse_headers_object(jo, NULL, &count, NULL, 0, 0));
  ASSERT_EQ(CDD_C_SUCCESS,
            parse_headers_object(jo, &headers, NULL, NULL, 0, 0));
  ASSERT_EQ(CDD_C_SUCCESS,
            parse_headers_object(jo, &headers, &count, NULL, 0, 0));
  ASSERT_EQ(0, count);
  ASSERT_EQ(NULL, headers);
  json_value_free(jv);

  /* 2. Valid headers with ignore_content_type (all Content-Type -> valid == 0)
   */
  jv = json_parse_string(
      "{\"Content-Type\": {\"schema\": {\"type\": \"string\"}}}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS,
            parse_headers_object(jo, &headers, &count, NULL, 0, 1));
  ASSERT_EQ(0, count);
  ASSERT_EQ(NULL, headers);
  json_value_free(jv);

  /* 3. Valid headers with some Content-Type (valid < count -> realloc) */
  jv = json_parse_string(
      "{\"X-Custom\": {\"schema\": {\"type\": \"string\"}}, \"Content-Type\": "
      "{\"schema\": {\"type\": \"string\"}}}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS,
            parse_headers_object(jo, &headers, &count, NULL, 0, 1));
  ASSERT_EQ(1, count);
  ASSERT_STR_EQ("X-Custom", headers[0].name);
  cleanup_headers_array(headers, count);
  headers = NULL;

  /* 4. OOM on calloc */
  g_cdd_alloc_fail = 1;
  rc = parse_headers_object(jo, &headers, &count, NULL, 0, 0);
  g_cdd_alloc_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);

  /* 5. OOM on strdup */
  g_cdd_strdup_fail = 1;
  rc = parse_headers_object(jo, &headers, &count, NULL, 0, 0);
  g_cdd_strdup_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  json_value_free(jv);

  /* 6. Error in parse_header_object */
  jv = json_parse_string("{\"X-Bad\": {\"style\": \"form\"}}");
  jo = json_value_get_object(jv);
  rc = parse_headers_object(jo, &headers, &count, NULL, 0, 0);
  ASSERT_NEQ(CDD_C_SUCCESS, rc);
  json_value_free(jv);

  /* 7. Additional branches for headers_object */
  /* 7a. h_obj == NULL (non-object in headers) */
  jv = json_parse_string("{\"X-Num\": 123}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS,
            parse_headers_object(jo, &headers, &count, NULL, 0, 0));
  ASSERT_EQ(1, count);
  cleanup_headers_array(headers, count);
  headers = NULL;
  json_value_free(jv);

  /* 7b. realloc failure when valid < count */
  jv = json_parse_string(
      "{\"X-Custom\": {\"schema\": {\"type\": \"string\"}}, \"Content-Type\": "
      "{\"schema\": {\"type\": \"string\"}}}");
  jo = json_value_get_object(jv);
  g_cdd_alloc_fail = 2;
  rc = parse_headers_object(jo, &headers, &count, NULL, 0, 1);
  g_cdd_alloc_fail = 0;
  cleanup_headers_array(headers, count);
  headers = NULL;
  json_value_free(jv);

  PASS();
}

/**
 * @brief Test parse_encoding_object comprehensive branches.
 *
 * @return GREATEST_TEST_RES
 */
TEST test_openapi_encoding_object_full_branches(void) {
  struct OpenAPI_Encoding enc;
  JSON_Value *jv = NULL;
  const JSON_Object *jo = NULL;
  cdd_c_error_t rc = 0;

  memset(&enc, 0, sizeof(enc));

  /* 1. NULL checks */
  ASSERT_EQ(CDD_C_SUCCESS, parse_encoding_object(NULL, &enc, NULL, 0));
  jv = json_parse_string("{}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, parse_encoding_object(jo, NULL, NULL, 0));
  json_value_free(jv);

  /* 2. Conflicting encodings: encoding AND prefixEncoding */
  jv = json_parse_string("{\"encoding\": {}, \"prefixEncoding\": []}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            parse_encoding_object(jo, &enc, NULL, 0));
  json_value_free(jv);

  /* 2b. Conflicting encodings: encoding AND itemEncoding */
  jv = json_parse_string("{\"encoding\": {}, \"itemEncoding\": {}}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            parse_encoding_object(jo, &enc, NULL, 0));
  json_value_free(jv);

  /* 3. Valid encoding with all fields */
  jv = json_parse_string(
      "{\"contentType\": \"application/json\", \"style\": \"form\", "
      "\"explode\": true, \"allowReserved\": true, \"headers\": "
      "{\"X-Part-Id\": {\"schema\": {\"type\": \"string\"}}}, \"encoding\": "
      "{\"field1\": {\"contentType\": \"text/plain\"}}, \"x-ext\": 1}");
  jo = json_value_get_object(jv);
  memset(&enc, 0, sizeof(enc));
  ASSERT_EQ(CDD_C_SUCCESS, parse_encoding_object(jo, &enc, NULL, 0));
  ASSERT_STR_EQ("application/json", enc.content_type);
  ASSERT_EQ(1, enc.style_set);
  ASSERT_EQ(1, enc.explode_set);
  ASSERT_EQ(1, enc.explode);
  ASSERT_EQ(1, enc.allow_reserved_set);
  ASSERT_EQ(1, enc.allow_reserved);
  ASSERT_EQ(1, enc.n_headers);
  ASSERT_EQ(1, enc.n_encoding);
  ASSERT_NEQ(NULL, enc.extensions_json);
  cdd_test_free_encoding(&enc);

  /* 3b. OOM on contentType strdup */
  g_cdd_strdup_fail = 1;
  memset(&enc, 0, sizeof(enc));
  rc = parse_encoding_object(jo, &enc, NULL, 0);
  g_cdd_strdup_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  cdd_test_free_encoding(&enc);
  json_value_free(jv);

  /* 4. prefixEncoding array and itemEncoding */
  jv = json_parse_string(
      "{\"prefixEncoding\": [{\"contentType\": \"text/plain\"}], "
      "\"itemEncoding\": {\"contentType\": \"application/octet-stream\"} }");
  jo = json_value_get_object(jv);
  memset(&enc, 0, sizeof(enc));
  ASSERT_EQ(CDD_C_SUCCESS, parse_encoding_object(jo, &enc, NULL, 0));
  ASSERT_EQ(1, enc.n_prefix_encoding);
  ASSERT_EQ(1, enc.item_encoding_set);
  ASSERT_NEQ(NULL, enc.item_encoding);
  cdd_test_free_encoding(&enc);

  /* 4b. OOM on item_encoding calloc */
  g_cdd_alloc_fail = 1;
  memset(&enc, 0, sizeof(enc));
  rc = parse_encoding_object(jo, &enc, NULL, 0);
  g_cdd_alloc_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  cdd_test_free_encoding(&enc);
  json_value_free(jv);

  /* 5. Child encoding errors */
  /* 5a. Nested encoding map error */
  jv = json_parse_string(
      "{\"encoding\": {\"f1\": {\"encoding\": {}, \"prefixEncoding\": []}}}");
  jo = json_value_get_object(jv);
  memset(&enc, 0, sizeof(enc));
  ASSERT_NEQ(CDD_C_SUCCESS, parse_encoding_object(jo, &enc, NULL, 0));
  json_value_free(jv);

  /* 5b. Nested prefixEncoding array error */
  jv = json_parse_string(
      "{\"prefixEncoding\": [{\"encoding\": {}, \"prefixEncoding\": []}]}");
  jo = json_value_get_object(jv);
  memset(&enc, 0, sizeof(enc));
  ASSERT_NEQ(CDD_C_SUCCESS, parse_encoding_object(jo, &enc, NULL, 0));
  json_value_free(jv);

  /* 5c. Nested itemEncoding object error */
  jv = json_parse_string(
      "{\"itemEncoding\": {\"encoding\": {}, \"prefixEncoding\": []}}");
  jo = json_value_get_object(jv);
  memset(&enc, 0, sizeof(enc));
  ASSERT_NEQ(CDD_C_SUCCESS, parse_encoding_object(jo, &enc, NULL, 0));
  json_value_free(jv);

  /* 5d. Headers error */
  jv = json_parse_string("{\"headers\": {\"X-Bad\": {\"style\": \"form\"}}}");
  jo = json_value_get_object(jv);
  memset(&enc, 0, sizeof(enc));
  ASSERT_NEQ(CDD_C_SUCCESS, parse_encoding_object(jo, &enc, NULL, 0));
  json_value_free(jv);

  /* 6. itemEncoding calloc OOM without prefixEncoding */
  jv = json_parse_string(
      "{\"itemEncoding\": {\"contentType\": \"text/plain\"}}");
  jo = json_value_get_object(jv);
  memset(&enc, 0, sizeof(enc));
  g_cdd_alloc_fail = 1;
  rc = parse_encoding_object(jo, &enc, NULL, 0);
  g_cdd_alloc_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  cdd_test_free_encoding(&enc);
  json_value_free(jv);

  /* 7. collect_extensions error */
  jv = json_parse_string("{\"x-ext\": 1}");
  jo = json_value_get_object(jv);
  memset(&enc, 0, sizeof(enc));
  g_cdd_strdup_fail = 1;
  rc = parse_encoding_object(jo, &enc, NULL, 0);
  g_cdd_strdup_fail = 0;
  cdd_test_free_encoding(&enc);
  json_value_free(jv);

  PASS();
}

/**
 * @brief Test parse_encoding_map and parse_encoding_array comprehensive
 * branches.
 *
 * @return GREATEST_TEST_RES
 */
TEST test_openapi_encoding_map_and_array_full_branches(void) {
  struct OpenAPI_Encoding *encs = NULL;
  size_t count = 0;
  JSON_Value *jv = NULL;
  const JSON_Object *jo = NULL;
  const JSON_Array *arr = NULL;
  cdd_c_error_t rc = 0;

  /* 1. Map NULL checks */
  ASSERT_EQ(CDD_C_SUCCESS, parse_encoding_map(NULL, &encs, &count, NULL, 0));
  ASSERT_EQ(0, count);
  ASSERT_EQ(NULL, encs);

  jv = json_parse_string("{}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, parse_encoding_map(jo, NULL, &count, NULL, 0));
  ASSERT_EQ(CDD_C_SUCCESS, parse_encoding_map(jo, &encs, NULL, NULL, 0));
  ASSERT_EQ(CDD_C_SUCCESS, parse_encoding_map(jo, &encs, &count, NULL, 0));
  ASSERT_EQ(0, count);
  ASSERT_EQ(NULL, encs);
  json_value_free(jv);

  /* 2. Map with non-object element (!enc_def skipped -> valid == 0) */
  jv = json_parse_string("{\"e1\": 123}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, parse_encoding_map(jo, &encs, &count, NULL, 0));
  ASSERT_EQ(0, count);
  ASSERT_EQ(NULL, encs);
  json_value_free(jv);

  /* 3. Valid map */
  jv = json_parse_string("{\"e1\": {\"contentType\": \"text/plain\"}, \"e2\": "
                         "{\"contentType\": \"application/json\"}}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, parse_encoding_map(jo, &encs, &count, NULL, 0));
  ASSERT_EQ(2, count);
  ASSERT_STR_EQ("e1", encs[0].name);
  ASSERT_STR_EQ("e2", encs[1].name);
  cleanup_encodings_array(encs, count);
  encs = NULL;

  /* 3b. Map OOM on calloc */
  g_cdd_alloc_fail = 1;
  rc = parse_encoding_map(jo, &encs, &count, NULL, 0);
  g_cdd_alloc_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);

  /* 3c. Map OOM on strdup */
  g_cdd_strdup_fail = 1;
  rc = parse_encoding_map(jo, &encs, &count, NULL, 0);
  g_cdd_strdup_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);

  /* 3d. Map error in child */
  json_value_free(jv);
  jv =
      json_parse_string("{\"e1\": {\"encoding\": {}, \"prefixEncoding\": []}}");
  jo = json_value_get_object(jv);
  rc = parse_encoding_map(jo, &encs, &count, NULL, 0);
  ASSERT_NEQ(CDD_C_SUCCESS, rc);
  json_value_free(jv);

  /* 4. Array NULL checks */
  ASSERT_EQ(CDD_C_SUCCESS, parse_encoding_array(NULL, &encs, &count, NULL, 0));
  ASSERT_EQ(0, count);
  ASSERT_EQ(NULL, encs);

  jv = json_parse_string("[]");
  arr = json_value_get_array(jv);
  ASSERT_EQ(CDD_C_SUCCESS, parse_encoding_array(arr, NULL, &count, NULL, 0));
  ASSERT_EQ(CDD_C_SUCCESS, parse_encoding_array(arr, &encs, NULL, NULL, 0));
  ASSERT_EQ(CDD_C_SUCCESS, parse_encoding_array(arr, &encs, &count, NULL, 0));
  ASSERT_EQ(0, count);
  ASSERT_EQ(NULL, encs);
  json_value_free(jv);

  /* 5. Array with non-object element (!enc_def skipped -> valid == 0) */
  jv = json_parse_string("[123]");
  arr = json_value_get_array(jv);
  ASSERT_EQ(CDD_C_SUCCESS, parse_encoding_array(arr, &encs, &count, NULL, 0));
  ASSERT_EQ(0, count);
  ASSERT_EQ(NULL, encs);
  json_value_free(jv);

  /* 6. Valid array */
  jv = json_parse_string(
      "[{\"contentType\": \"text/plain\"}, {\"contentType\": \"image/png\"}]");
  arr = json_value_get_array(jv);
  ASSERT_EQ(CDD_C_SUCCESS, parse_encoding_array(arr, &encs, &count, NULL, 0));
  ASSERT_EQ(2, count);
  ASSERT_STR_EQ("text/plain", encs[0].content_type);
  ASSERT_STR_EQ("image/png", encs[1].content_type);
  cleanup_encodings_array(encs, count);
  encs = NULL;

  /* 6b. Array OOM on calloc */
  g_cdd_alloc_fail = 1;
  rc = parse_encoding_array(arr, &encs, &count, NULL, 0);
  g_cdd_alloc_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);

  /* 6c. Array error in child */
  json_value_free(jv);
  jv = json_parse_string("[{\"encoding\": {}, \"prefixEncoding\": []}]");
  arr = json_value_get_array(jv);
  rc = parse_encoding_array(arr, &encs, &count, NULL, 0);
  ASSERT_NEQ(CDD_C_SUCCESS, rc);
  json_value_free(jv);

  PASS();
}

/**
 * @brief Test cleanup helpers for NULL and populated edge cases.
 *
 * @return GREATEST_TEST_RES
 */
TEST test_openapi_headers_links_cleanups(void) {
  struct OpenAPI_LinkParam *lp = NULL;
  cleanup_link_params(NULL, 0);
  cleanup_links_array(NULL, 0);
  cleanup_headers_array(NULL, 0);
  cleanup_encodings_array(NULL, 0);

  lp = (struct OpenAPI_LinkParam *)calloc(2, sizeof(*lp));
  ASSERT_NEQ(NULL, lp);
  lp[0].name = (char *)malloc(8);
  ASSERT_NEQ(NULL, lp[0].name);
#if defined(_MSC_VER)
  strcpy_s(lp[0].name, 8, "foo");
#else
  strcpy(lp[0].name, "foo");
#endif
  lp[1].name = NULL;
  cleanup_link_params(lp, 2);
  PASS();
}

#define OPENAPI_HEADERS_LINKS_COVERAGE_TESTS()                                 \
  RUN_TEST(test_openapi_header_object_full_branches);                          \
  RUN_TEST(test_openapi_link_parameters_full_branches);                        \
  RUN_TEST(test_openapi_link_object_full_branches);                            \
  RUN_TEST(test_openapi_links_object_full_branches);                           \
  RUN_TEST(test_openapi_headers_object_full_branches);                         \
  RUN_TEST(test_openapi_encoding_object_full_branches);                        \
  RUN_TEST(test_openapi_encoding_map_and_array_full_branches);                 \
  RUN_TEST(test_openapi_headers_links_cleanups)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_OPENAPI_HEADERS_LINKS_COVERAGE_H */
