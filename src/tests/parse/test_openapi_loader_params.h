/**
 * @file test_openapi_loader_params.h
 * @brief Parameter metadata, allowEmptyValue, querystring, and server tests.
 */

#ifndef TEST_OPENAPI_LOADER_PARAMS_H
#define TEST_OPENAPI_LOADER_PARAMS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "parse/test_openapi_loader_common.h"
#include "openapi/parse/openapi_test_helpers.h"
/* clang-format on */

TEST test_load_operation_tags(void) {

  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
      "\"paths\":{\"/"
      "tagged\":{\"get\":{\"tags\":[\"pet\",\"store\"],\"operationId\":"
      "\"getTagged\",\"responses\":{\"200\":{\"description\":\"OK\"}}}}}}";

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  {
    struct OpenAPI_Operation *op = &spec.paths[0].operations[0];
    ASSERT_EQ(2, op->n_tags);
    ASSERT_STR_EQ("pet", op->tags[0]);
    ASSERT_STR_EQ("store", op->tags[1]);
  }

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_load_parameter_metadata(void) {

  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
      "\"paths\":{\"/"
      "q\":{\"get\":{\"parameters\":[{\"name\":\"q\",\"in\":\"query\","
      "\"description\":\"Search "
      "term\",\"deprecated\":true,\"allowReserved\":true,\"schema\":{\"type\":"
      "\"string\"}}],\"responses\":{\"200\":{\"description\":\"OK\"}}}}}}";

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  {
    struct OpenAPI_Parameter *p = &spec.paths[0].operations[0].parameters[0];
    ASSERT_STR_EQ("Search term", p->description);
    ASSERT_EQ(1, p->deprecated_set);
    ASSERT_EQ(1, p->deprecated);
    ASSERT_EQ(1, p->allow_reserved_set);
    ASSERT_EQ(1, p->allow_reserved);
  }

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_load_allow_empty_value(void) {

  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
      "\"paths\":{\"/"
      "q\":{\"get\":{\"parameters\":[{\"name\":\"q\",\"in\":\"query\","
      "\"allowEmptyValue\":true,\"schema\":{\"type\":\"string\"}}],"
      "\"responses\":{\"200\":{\"description\":\"OK\"}}}}}}";

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  {
    struct OpenAPI_Parameter *p = &spec.paths[0].operations[0].parameters[0];
    ASSERT_EQ(1, p->allow_empty_value_set);
    ASSERT_EQ(1, p->allow_empty_value);
  }

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_load_allow_empty_value_non_query_rejected(void) {

  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
      "\"paths\":{\"/"
      "q\":{\"get\":{\"parameters\":[{\"name\":\"q\",\"in\":\"header\","
      "\"allowEmptyValue\":true,\"schema\":{\"type\":\"string\"}}],"
      "\"responses\":{\"200\":{\"description\":\"OK\"}}}}}}";

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  g_fail_io_after = -1;
  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_parameter_explode_false(void) {
  const char *json =
      "{\"paths\":{\"/"
      "q\":{\"get\":{\"parameters\":[{\"name\":\"ids\",\"in\":\"query\","
      "\"style\":\"form\",\"explode\":false,\"schema\":{\"type\":\"array\","
      "\"items\":{\"type\":\"string\"}}}],\"responses\":{\"200\":{"
      "\"description\":\"OK\"}}}}},\"openapi\":\"3.2.0\"}";

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  {
    struct OpenAPI_Parameter *p = &spec.paths[0].operations[0].parameters[0];
    ASSERT_EQ(1, p->explode_set);
    ASSERT_EQ(0, p->explode);
  }

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_load_querystring_parameter(void) {

  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
      "\"paths\":{\"/"
      "search\":{\"get\":{\"parameters\":[{\"name\":\"qs\",\"in\":"
      "\"querystring\",\"content\":{\"application/"
      "x-www-form-urlencoded\":{\"schema\":{\"type\":\"object\"}}}}],"
      "\"responses\":{\"200\":{\"description\":\"OK\"}}}}}}";

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  {
    struct OpenAPI_Parameter *p = &spec.paths[0].operations[0].parameters[0];
    ASSERT_EQ(OA_PARAM_IN_QUERYSTRING, p->in);
    ASSERT_STR_EQ("application/x-www-form-urlencoded", p->content_type);
    ASSERT_STR_EQ("object", p->type);
  }

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_load_querystring_json_inline_promoted(void) {

  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
      "\"paths\":{\"/"
      "search\":{\"get\":{\"parameters\":[{\"name\":\"qs\",\"in\":"
      "\"querystring\",\"content\":{\"application/"
      "json\":{\"schema\":{\"type\":\"object\",\"properties\":{\"q\":{\"type\":"
      "\"string\"}}}}}}],\"responses\":{\"200\":{\"description\":\"OK\"}}}}}}";

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  {
    struct OpenAPI_Parameter *p = &spec.paths[0].operations[0].parameters[0];
    ASSERT_EQ(OA_PARAM_IN_QUERYSTRING, p->in);
    ASSERT(p->schema.ref_name != NULL);
    ASSERT_EQ(1, spec.n_defined_schemas);
    ASSERT_STR_EQ(p->schema.ref_name, spec.defined_schema_names[0]);
  }

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_ignore_reserved_header_parameters(void) {

  const char json[] = {
      123, 34,  111, 112, 101, 110, 97,  112, 105, 34,  58,  34,  51,  46,  50,
      46,  48,  34,  44,  34,  105, 110, 102, 111, 34,  58,  123, 34,  116, 105,
      116, 108, 101, 34,  58,  34,  116, 34,  44,  34,  118, 101, 114, 115, 105,
      111, 110, 34,  58,  34,  49,  34,  125, 44,  34,  112, 97,  116, 104, 115,
      34,  58,  123, 34,  47,  104, 34,  58,  123, 34,  103, 101, 116, 34,  58,
      123, 34,  114, 101, 115, 112, 111, 110, 115, 101, 115, 34,  58,  123, 34,
      50,  48,  48,  34,  58,  123, 34,  100, 101, 115, 99,  114, 105, 112, 116,
      105, 111, 110, 34,  58,  34,  111, 107, 34,  125, 125, 44,  34,  112, 97,
      114, 97,  109, 101, 116, 101, 114, 115, 34,  58,  91,  123, 34,  110, 97,
      109, 101, 34,  58,  34,  65,  99,  99,  101, 112, 116, 34,  44,  34,  105,
      110, 34,  58,  34,  104, 101, 97,  100, 101, 114, 34,  44,  34,  115, 99,
      104, 101, 109, 97,  34,  58,  123, 34,  116, 121, 112, 101, 34,  58,  34,
      115, 116, 114, 105, 110, 103, 34,  125, 125, 44,  123, 34,  110, 97,  109,
      101, 34,  58,  34,  113, 34,  44,  34,  105, 110, 34,  58,  34,  113, 117,
      101, 114, 121, 34,  44,  34,  115, 99,  104, 101, 109, 97,  34,  58,  123,
      34,  116, 121, 112, 101, 34,  58,  34,  115, 116, 114, 105, 110, 103, 34,
      125, 125, 93,  125, 125, 125, 125, 0};

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  {
    struct OpenAPI_Operation *op = &spec.paths[0].operations[0];
    ASSERT_EQ(1, op->n_parameters);
    ASSERT_STR_EQ("q", op->parameters[0].name);
    ASSERT_EQ(OA_PARAM_IN_QUERY, op->parameters[0].in);
  }

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_ignore_content_type_response_header(void) {

  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
      "\"paths\":{\"/r\":{\"get\":{\"responses\":{\"200\":{"
      "\"description\":\"ok\","
      "\"headers\":{"
      "\"Content-Type\":{\"schema\":{\"type\":\"string\"}},"
      "\"X-Rate\":{\"schema\":{\"type\":\"integer\"}}"
      "}}}}}}}";

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  {
    struct OpenAPI_Response *resp = &spec.paths[0].operations[0].responses[0];
    ASSERT_EQ(1, resp->n_headers);
    ASSERT_STR_EQ("X-Rate", resp->headers[0].name);
  }

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_param_schema_and_content_conflict(void) {

  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
      "\"paths\":{\"/"
      "c\":{\"get\":{\"responses\":{\"200\":{\"description\":\"ok\"}},"
      "\"parameters\":[{"
      "\"name\":\"q\",\"in\":\"query\","
      "\"schema\":{\"type\":\"string\"},"
      "\"content\":{\"text/plain\":{\"schema\":{\"type\":\"string\"}}}"
      "}]}}}}";

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  g_fail_io_after = -1;
  openapi_spec_free(&spec);
  PASS();
}

TEST test_header_schema_and_content_conflict(void) {

  const char *json = "{\"paths\":{\"/"
                     "c\":{\"get\":{\"responses\":{\"200\":{\"description\":"
                     "\"ok\",\"headers\":{\"X-Foo\":{\"schema\":{\"type\":"
                     "\"string\"},\"content\":{\"text/"
                     "plain\":{\"schema\":{\"type\":\"string\"}}}}}}}}}},"
                     "\"openapi\":\"3.2.0\"}";

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  g_fail_io_after = -1;
  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_parameter_content_any(void) {

  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
      "\"paths\":{\"/"
      "h\":{\"get\":{\"responses\":{\"200\":{\"description\":\"ok\"}},"
      "\"parameters\":[{"
      "\"name\":\"X-Foo\",\"in\":\"header\","
      "\"content\":{\"text/plain\":{\"schema\":{\"type\":\"string\"}}}"
      "}]}}}}";

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  {
    struct OpenAPI_Parameter *p = &spec.paths[0].operations[0].parameters[0];
    ASSERT_STR_EQ("text/plain", p->content_type);
    ASSERT_STR_EQ("string", p->type);
  }

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_load_parameter_content_media_type_encoding(void) {

  const char json[] = {
      123, 34,  111, 112, 101, 110, 97,  112, 105, 34,  58,  34,  51,  46,  50,
      46,  48,  34,  44,  34,  105, 110, 102, 111, 34,  58,  123, 34,  116, 105,
      116, 108, 101, 34,  58,  34,  116, 34,  44,  34,  118, 101, 114, 115, 105,
      111, 110, 34,  58,  34,  49,  34,  125, 44,  34,  112, 97,  116, 104, 115,
      34,  58,  123, 34,  47,  112, 34,  58,  123, 34,  103, 101, 116, 34,  58,
      123, 34,  112, 97,  114, 97,  109, 101, 116, 101, 114, 115, 34,  58,  91,
      123, 34,  110, 97,  109, 101, 34,  58,  34,  102, 105, 108, 116, 101, 114,
      34,  44,  34,  105, 110, 34,  58,  34,  113, 117, 101, 114, 121, 34,  44,
      34,  99,  111, 110, 116, 101, 110, 116, 34,  58,  123, 34,  97,  112, 112,
      108, 105, 99,  97,  116, 105, 111, 110, 47,  120, 45,  119, 119, 119, 45,
      102, 111, 114, 109, 45,  117, 114, 108, 101, 110, 99,  111, 100, 101, 100,
      34,  58,  123, 34,  115, 99,  104, 101, 109, 97,  34,  58,  123, 34,  116,
      121, 112, 101, 34,  58,  34,  111, 98,  106, 101, 99,  116, 34,  44,  34,
      112, 114, 111, 112, 101, 114, 116, 105, 101, 115, 34,  58,  123, 34,  105,
      100, 34,  58,  123, 34,  116, 121, 112, 101, 34,  58,  34,  115, 116, 114,
      105, 110, 103, 34,  125, 125, 125, 44,  34,  101, 110, 99,  111, 100, 105,
      110, 103, 34,  58,  123, 34,  105, 100, 34,  58,  123, 34,  99,  111, 110,
      116, 101, 110, 116, 84,  121, 112, 101, 34,  58,  34,  116, 101, 120, 116,
      47,  112, 108, 97,  105, 110, 34,  44,  34,  115, 116, 121, 108, 101, 34,
      58,  34,  102, 111, 114, 109, 34,  44,  34,  101, 120, 112, 108, 111, 100,
      101, 34,  58,  116, 114, 117, 101, 125, 125, 125, 125, 125, 93,  44,  34,
      114, 101, 115, 112, 111, 110, 115, 101, 115, 34,  58,  123, 34,  50,  48,
      48,  34,  58,  123, 34,  100, 101, 115, 99,  114, 105, 112, 116, 105, 111,
      110, 34,  58,  34,  79,  75,  34,  125, 125, 125, 125, 125, 125, 0};

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  {
    struct OpenAPI_Parameter *p = &spec.paths[0].operations[0].parameters[0];
    ASSERT_STR_EQ("application/x-www-form-urlencoded", p->content_type);
    ASSERT(p->content_media_types != NULL);
    ASSERT_EQ((size_t)1, p->n_content_media_types);
    ASSERT_STR_EQ("application/x-www-form-urlencoded",
                  p->content_media_types[0].name);
    ASSERT_EQ((size_t)1, p->content_media_types[0].n_encoding);
    ASSERT_STR_EQ("id", p->content_media_types[0].encoding[0].name);
    ASSERT_STR_EQ("text/plain",
                  p->content_media_types[0].encoding[0].content_type);
    ASSERT_EQ(1, p->content_media_types[0].encoding[0].style_set);
    ASSERT_EQ(OA_STYLE_FORM, p->content_media_types[0].encoding[0].style);
    ASSERT_EQ(1, p->content_media_types[0].encoding[0].explode_set);
    ASSERT_EQ(1, p->content_media_types[0].encoding[0].explode);
  }

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_load_header_content_media_type(void) {

  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
      "\"paths\":{\"/p\":{\"get\":{\"responses\":{"
      "\"200\":{\"description\":\"ok\",\"headers\":{"
      "\"X-Rate\":{\"content\":{"
      "\"text/plain\":{\"schema\":{\"type\":\"string\"}}"
      "}}}}}}}}}}}";

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  {
    struct OpenAPI_Header *h =
        &spec.paths[0].operations[0].responses[0].headers[0];
    ASSERT_STR_EQ("text/plain", h->content_type);
    ASSERT(h->content_media_types != NULL);
    ASSERT_EQ((size_t)1, h->n_content_media_types);
    ASSERT_STR_EQ("text/plain", h->content_media_types[0].name);
  }

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_load_parameter_schema_ref(void) {

  const char json[] = {
      123, 34,  111, 112, 101, 110, 97,  112, 105, 34,  58,  34,  51,  46,  50,
      46,  48,  34,  44,  34,  112, 97,  116, 104, 115, 34,  58,  123, 34,  47,
      112, 101, 116, 115, 34,  58,  123, 34,  103, 101, 116, 34,  58,  123, 34,
      112, 97,  114, 97,  109, 101, 116, 101, 114, 115, 34,  58,  91,  123, 34,
      110, 97,  109, 101, 34,  58,  34,  112, 101, 116, 34,  44,  34,  105, 110,
      34,  58,  34,  113, 117, 101, 114, 121, 34,  44,  34,  115, 99,  104, 101,
      109, 97,  34,  58,  123, 34,  36,  114, 101, 102, 34,  58,  34,  35,  47,
      99,  111, 109, 112, 111, 110, 101, 110, 116, 115, 47,  115, 99,  104, 101,
      109, 97,  115, 47,  80,  101, 116, 34,  125, 125, 44,  123, 34,  110, 97,
      109, 101, 34,  58,  34,  116, 97,  103, 115, 34,  44,  34,  105, 110, 34,
      58,  34,  113, 117, 101, 114, 121, 34,  44,  34,  115, 99,  104, 101, 109,
      97,  34,  58,  123, 34,  116, 121, 112, 101, 34,  58,  34,  97,  114, 114,
      97,  121, 34,  44,  34,  105, 116, 101, 109, 115, 34,  58,  123, 34,  36,
      114, 101, 102, 34,  58,  34,  35,  47,  99,  111, 109, 112, 111, 110, 101,
      110, 116, 115, 47,  115, 99,  104, 101, 109, 97,  115, 47,  84,  97,  103,
      34,  125, 125, 125, 93,  44,  34,  114, 101, 115, 112, 111, 110, 115, 101,
      115, 34,  58,  123, 34,  50,  48,  48,  34,  58,  123, 34,  100, 101, 115,
      99,  114, 105, 112, 116, 105, 111, 110, 34,  58,  34,  79,  75,  34,  125,
      125, 125, 125, 125, 44,  34,  99,  111, 109, 112, 111, 110, 101, 110, 116,
      115, 34,  58,  123, 34,  115, 99,  104, 101, 109, 97,  115, 34,  58,  123,
      34,  80,  101, 116, 34,  58,  123, 34,  116, 121, 112, 101, 34,  58,  34,
      111, 98,  106, 101, 99,  116, 34,  125, 44,  34,  84,  97,  103, 34,  58,
      123, 34,  116, 121, 112, 101, 34,  58,  34,  111, 98,  106, 101, 99,  116,
      34,  125, 125, 125, 125, 0};

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  {
    struct OpenAPI_Parameter *p0 = &spec.paths[0].operations[0].parameters[0];
    struct OpenAPI_Parameter *p1 = &spec.paths[0].operations[0].parameters[1];
    ASSERT_STR_EQ("Pet", p0->type);
    ASSERT_EQ(0, p0->is_array);
    ASSERT_EQ(1, p1->is_array);
    ASSERT_STR_EQ("array", p1->type);
    ASSERT_STR_EQ("Tag", p1->items_type);
  }

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_load_header_schema_ref(void) {

  const char *json =
      "{\"openapi\":\"3.2.0\",\"paths\":{\"/pets\":{\"get\":{\"responses\":{"
      "\"200\":{\"description\":\"ok\",\"headers\":{"
      "\"X-Rate\":{\"schema\":{\"$ref\":\"#/components/schemas/Rate\"}}"
      "}}}}}},\"components\":{\"schemas\":{"
      "\"Rate\":{\"type\":\"integer\"}"
      "}}}";

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  {
    struct OpenAPI_Header *h =
        &spec.paths[0].operations[0].responses[0].headers[0];
    ASSERT_STR_EQ("Rate", h->type);
    ASSERT_EQ(0, h->is_array);
  }

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_load_path_level_parameters(void) {

  const char *json =
      "{\"paths\":{\"/pets\":{\"summary\":\"Pets\",\"description\":\"All "
      "pets\",\"parameters\":[{\"name\":\"x-trace\",\"in\":\"header\","
      "\"schema\":{\"type\":\"string\"}}],\"get\":{\"operationId\":"
      "\"listPets\",\"responses\":{\"200\":{\"description\":\"OK\"}}}}},"
      "\"openapi\":\"3.2.0\"}";

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  ASSERT_EQ(1, spec.n_paths);
  ASSERT_STR_EQ("Pets", spec.paths[0].summary);
  ASSERT_STR_EQ("All pets", spec.paths[0].description);
  ASSERT_EQ(1, spec.paths[0].n_parameters);
  ASSERT_STR_EQ("x-trace", spec.paths[0].parameters[0].name);
  ASSERT_EQ(OA_PARAM_IN_HEADER, spec.paths[0].parameters[0].in);
  ASSERT_EQ(0, spec.paths[0].operations[0].n_parameters);

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_load_server_variables(void) {

  const char *json = "{\"openapi\":\"3.2.0\",\"servers\":[{\"url\":\"https://"
                     "{env}.example.com\","
                     "\"variables\":{\"env\":{\"default\":\"prod\",\"enum\":["
                     "\"prod\",\"staging\"],"
                     "\"description\":\"Environment\"}}}],\"paths\":{}}";

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  ASSERT_EQ(1, spec.n_servers);
  ASSERT_EQ(1, spec.servers[0].n_variables);
  ASSERT_STR_EQ("env", spec.servers[0].variables[0].name);
  ASSERT_STR_EQ("prod", spec.servers[0].variables[0].default_value);
  ASSERT_STR_EQ("Environment", spec.servers[0].variables[0].description);
  ASSERT_EQ(2, spec.servers[0].variables[0].n_enum_values);
  ASSERT_STR_EQ("prod", spec.servers[0].variables[0].enum_values[0]);
  ASSERT_STR_EQ("staging", spec.servers[0].variables[0].enum_values[1]);

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_server_variable_default_required(void) {

  const char *json = "{\"openapi\":\"3.2.0\",\"servers\":[{\"url\":\"https://"
                     "{env}.example.com\","
                     "\"variables\":{\"env\":{\"enum\":[\"prod\",\"staging\"]}}"
                     "}],\"paths\":{}}";

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_load_openapi_version_and_servers(void) {

  const char *json =
      "{\"openapi\":\"3.2.0\",\"servers\":[{\"url\":\"https://"
      "api.example.com\","
      "\"description\":\"Prod\",\"name\":\"prod\"}],\"paths\":{}}";

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("3.2.0", spec.openapi_version);
  ASSERT_EQ(1, spec.n_servers);
  ASSERT_STR_EQ("https://api.example.com", spec.servers[0].url);
  ASSERT_STR_EQ("Prod", spec.servers[0].description);
  ASSERT_STR_EQ("prod", spec.servers[0].name);

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_load_server_duplicate_name_rejected(void) {

  const char *json =
      "{"
      "\"openapi\":\"3.2.0\","
      "\"info\":{\"title\":\"t\",\"version\":\"1\"},"
      "\"servers\":["
      "{\"url\":\"https://api.example.com\",\"name\":\"prod\"},"
      "{\"url\":\"https://staging.example.com\",\"name\":\"prod\"}"
      "],"
      "\"paths\":{}"
      "}";

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  g_fail_io_after = -1;
  openapi_spec_free(&spec);
  PASS();
}

TEST test_load_missing_openapi_and_swagger_rejected(void) {

  const char *json = "{\"info\":{\"title\":\"T\",\"version\":\"1\"},\"paths\":{"
                     "},\"openapi\":\"3.2.0\"}";

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);
  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_load_schema_root_document_with_id(void) {

  const char *json = "{"
                     "\"$id\":\"https://example.com/schema.json\","
                     "\"type\":\"object\","
                     "\"properties\":{\"id\":{\"type\":\"string\"}}"
                     "}";

  struct OpenAPI_DocRegistry registry;
  struct OpenAPI_Spec spec = {0};
  int rc = 0;
  rc += 0;

  ASSERT_EQ(CDD_C_SUCCESS, openapi_doc_registry_init(&registry));
  rc = load_spec_str_with_context(json, "https://example.com/schema.json",
                                  &registry, &spec);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(1, spec.is_schema_document);
  ASSERT(spec.schema_root_json != NULL);
  ASSERT_STR_EQ("https://example.com/schema.json", spec.document_uri);
  ASSERT_EQ(1, registry.count);
  ASSERT_EQ(&spec, registry.entries[0].spec);
  ASSERT_STR_EQ("https://example.com/schema.json",
                registry.entries[0].base_uri);

  {
    JSON_Value *val = json_parse_string(spec.schema_root_json);
    JSON_Object *obj = json_value_get_object(val);
    ASSERT_STR_EQ("object", json_object_get_string(obj, "type"));
    json_value_free(val);
  }

  openapi_spec_free(&spec);
  openapi_doc_registry_free(&registry);
  g_fail_io_after = -1;
  PASS();
}

TEST test_load_schema_root_boolean(void) {

  const char *json = (char *)(size_t)(size_t) "false";

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str_with_context(json, "https://example.com/boolean.json",
                                      NULL, &spec);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(1, spec.is_schema_document);
  ASSERT(spec.schema_root_json != NULL);
  ASSERT_STR_EQ("https://example.com/boolean.json", spec.document_uri);

  {
    JSON_Value *val = json_parse_string(spec.schema_root_json);
    ASSERT_EQ(JSONBoolean, json_value_get_type(val));
    ASSERT_EQ(0, json_value_get_boolean(val));
    json_value_free(val);
  }

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_load_swagger_root_allowed(void) {

  const char *json =
      "{\"swagger\":\"2.0\",\"info\":{\"title\":\"T\",\"version\":\"1\"},"
      "\"paths\":{}}";

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);
  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_load_openapi_version_unsupported_rejected(void) {

  const char *json =
      "{\"openapi\":\"4.0.0\",\"info\":{\"title\":\"T\",\"version\":\"1\"},"
      "\"paths\":{}}";

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  g_fail_io_after = -1;
  openapi_spec_free(&spec);
  PASS();
}

TEST test_openapi_parameters_full_coverage(void) {
  struct OpenAPI_Parameter param;
  struct OpenAPI_Parameter param2;
  struct OpenAPI_Parameter *out_params = NULL;
  size_t out_count = 0;
  JSON_Value *jv = NULL;
  JSON_Object *jo = NULL;
  struct OpenAPI_Spec spec;
  char q_buf[8];
  char other_buf[8];
  int k;
  size_t i;

  q_buf[0] = 'q';
  q_buf[1] = '\0';
  other_buf[0] = 'o';
  other_buf[1] = '\0';

  /* 1. NULL checks */
  memset(&param, 0, sizeof(param));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_parse_parameter_object(NULL, &param, NULL, 0));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_parameter_object(
                               (const JSON_Object *)(size_t)1, NULL, NULL, 0));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_parse_parameters_array(NULL, NULL, NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_parameters_array(NULL, &out_params,
                                                           &out_count, NULL));

  /* 2. param_key_equals branches */
  memset(&param, 0, sizeof(param));
  memset(&param2, 0, sizeof(param2));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_param_key_equals(NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_param_key_equals(&param, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_param_key_equals(NULL, &param2));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_param_key_equals(&param, &param2));

  param.name = q_buf;
  param.in = OA_PARAM_IN_QUERY;
  param2.name = q_buf;
  param2.in = OA_PARAM_IN_HEADER;
  ASSERT_EQ(0, cdd_test_param_key_equals(&param, &param2));

  param2.in = OA_PARAM_IN_QUERY;
  ASSERT_EQ(1, cdd_test_param_key_equals(&param, &param2));

  param2.name = other_buf;
  ASSERT_EQ(0, cdd_test_param_key_equals(&param, &param2));

  param2.name = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_param_key_equals(&param, &param2));

  /* 3. Ref with description and ref resolution */
  memset(&spec, 0, sizeof(spec));
  spec.n_component_parameters = 1;
  spec.component_parameter_names = (char **)calloc(1, sizeof(char *));
  spec.component_parameter_names[0] = strdup("MyParam");
  spec.component_parameters =
      (struct OpenAPI_Parameter *)calloc(1, sizeof(struct OpenAPI_Parameter));
  spec.component_parameters[0].name = strdup("resolved_name");
  spec.component_parameters[0].in = OA_PARAM_IN_QUERY;

  jv = json_parse_string("{\"$ref\":\"#/components/parameters/"
                         "MyParam\",\"description\":\"desc\"}");
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&param, 0, sizeof(param));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_parse_parameter_object(jo, &param, &spec, 1));
    ASSERT_STR_EQ("resolved_name", param.name);
    ASSERT_STR_EQ("desc", param.description);
    cdd_test_free_parameter(&param);
    json_value_free(jv);
  }
  openapi_spec_free(&spec);

  /* 4. Invalid name/in / schema and content / querystring without content */
  {
    const char *bad_params[11];
    size_t n_bad = 11;
    bad_params[0] = "{}";
    bad_params[1] = "{\"name\":\"\"}";
    bad_params[2] = "{\"name\":\"p\"}";
    bad_params[3] = "{\"name\":\"p\",\"in\":\"unknown\"}";
    bad_params[4] =
        "{\"name\":\"p\",\"in\":\"query\",\"schema\":{\"type\":\"string\"},"
        "\"content\":{\"application/"
        "json\":{\"schema\":{\"type\":\"string\"}}}}";
    bad_params[5] = "{\"name\":\"p\",\"in\":\"querystring\"}";
    bad_params[6] =
        "{\"name\":\"p\",\"in\":\"header\",\"allowEmptyValue\":true,"
        "\"schema\":{\"type\":\"string\"}}";
    bad_params[7] =
        "{\"name\":\"p\",\"in\":\"query\",\"content\":{\"a\":{},\"b\":{}}}";
    bad_params[8] = "{\"name\":\"p\",\"in\":\"query\",\"style\":\"unknown\","
                    "\"schema\":{\"type\":\"string\"}}";
    bad_params[9] = "{\"name\":\"p\",\"in\":\"query\",\"example\":\"ex\","
                    "\"examples\":{\"ex1\":{\"value\":\"v\"}},"
                    "\"schema\":{\"type\":\"string\"}}";
    bad_params[10] =
        "{\"name\":\"p_no_schema\",\"in\":\"query\",\"example\":\"a\","
        "\"examples\":{\"b\":{\"value\":\"c\"}}}";

    for (i = 0; i < n_bad; ++i) {
      jv = json_parse_string(bad_params[i]);
      if (jv) {
        jo = json_value_get_object(jv);
        memset(&param, 0, sizeof(param));
        ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                  cdd_test_parse_parameter_object(jo, &param, NULL, 0));
        cdd_test_free_parameter(&param);
        json_value_free(jv);
      }
    }
  }

  /* 5. Boolean schema */
  jv = json_parse_string("{\"name\":\"b\",\"in\":\"query\",\"schema\":false}");
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&param, 0, sizeof(param));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_parse_parameter_object(jo, &param, NULL, 0));
    ASSERT_EQ(1, param.schema.schema_is_boolean);
    ASSERT_EQ(0, param.schema.schema_boolean_value);
    cdd_test_free_parameter(&param);
    json_value_free(jv);
  }

  /* 6. Media ref resolution in content (schema_set, item_schema_set, neither)
   */
  memset(&spec, 0, sizeof(spec));
  spec.n_component_media_types = 3;
  spec.component_media_type_names = (char **)calloc(3, sizeof(char *));
  spec.component_media_type_names[0] = strdup("Mt1");
  spec.component_media_type_names[1] = strdup("Mt2");
  spec.component_media_type_names[2] = strdup("Mt3");
  spec.component_media_types =
      (struct OpenAPI_MediaType *)calloc(3, sizeof(struct OpenAPI_MediaType));
  spec.component_media_types[0].schema_set = 1;
  spec.component_media_types[0].schema.ref_name = strdup("SchemaRef1");
  spec.component_media_types[1].item_schema_set = 1;
  spec.component_media_types[1].item_schema.ref_name = strdup("SchemaRef2");

  jv = json_parse_string(
      "{\"name\":\"p\",\"in\":\"query\",\"content\":{\"application/json\":"
      "{\"$ref\":\"#/components/mediaTypes/Mt1\"}}}");
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&param, 0, sizeof(param));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_parse_parameter_object(jo, &param, &spec, 1));
    cdd_test_free_parameter(&param);
    json_value_free(jv);
  }

  jv = json_parse_string(
      "{\"name\":\"p\",\"in\":\"query\",\"content\":{\"application/json\":"
      "{\"$ref\":\"#/components/mediaTypes/Mt2\"}}}");
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&param, 0, sizeof(param));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_parse_parameter_object(jo, &param, &spec, 1));
    cdd_test_free_parameter(&param);
    json_value_free(jv);
  }

  jv = json_parse_string(
      "{\"name\":\"p\",\"in\":\"query\",\"content\":{\"application/json\":"
      "{\"$ref\":\"#/components/mediaTypes/Mt3\"}}}");
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&param, 0, sizeof(param));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_parse_parameter_object(jo, &param, &spec, 1));
    cdd_test_free_parameter(&param);
    json_value_free(jv);
  }
  openapi_spec_free(&spec);

  /* 7. Swagger 2.0 type on parameter without schema */
  memset(&spec, 0, sizeof(spec));
  spec.swagger_version = strdup("2.0");
  jv = json_parse_string(
      "{\"name\":\"sw_p\",\"in\":\"query\",\"type\":\"integer\"}");
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&param, 0, sizeof(param));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_parse_parameter_object(jo, &param, &spec, 0));
    cdd_test_free_parameter(&param);
    json_value_free(jv);
  }

  jv = json_parse_string(
      "{\"name\":\"sw_b\",\"in\":\"body\",\"type\":\"integer\"}");
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&param, 0, sizeof(param));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_parse_parameter_object(jo, &param, &spec, 0));
    cdd_test_free_parameter(&param);
    json_value_free(jv);
  }
  openapi_spec_free(&spec);

  /* 7b. Ref not found and Querystring variations */
  memset(&spec, 0, sizeof(spec));
  jv = json_parse_string("{\"$ref\":\"#/components/parameters/NotFound\"}");
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&param, 0, sizeof(param));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_parse_parameter_object(jo, &param, &spec, 1));
    cdd_test_free_parameter(&param);
    json_value_free(jv);
  }

  /* Querystring with form-urlencoded */
  jv = json_parse_string(
      "{\"name\":\"q_form\",\"in\":\"querystring\",\"content\":{\"application/"
      "x-www-form-urlencoded\":{\"schema\":{\"type\":\"object\","
      "\"properties\":{\"k\":{\"type\":\"string\"}}}}}}");
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&param, 0, sizeof(param));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_parse_parameter_object(jo, &param, &spec, 0));
    cdd_test_free_parameter(&param);
    json_value_free(jv);
  }

  /* Querystring with non-object json */
  jv = json_parse_string(
      "{\"name\":\"q_str\",\"in\":\"querystring\",\"content\":{\"application/"
      "json\":{\"schema\":{\"type\":\"string\"}}}}");
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&param, 0, sizeof(param));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_parse_parameter_object(jo, &param, &spec, 0));
    cdd_test_free_parameter(&param);
    json_value_free(jv);
  }
  openapi_spec_free(&spec);

  /* 8. Style defaults and explode variations */
  {
    const char *styles[6];
    size_t n_st = 6;
    styles[0] =
        "{\"name\":\"p\",\"in\":\"path\",\"schema\":{\"type\":\"string\"}}";
    styles[1] =
        "{\"name\":\"p\",\"in\":\"cookie\",\"schema\":{\"type\":\"string\"}}";
    styles[2] =
        "{\"name\":\"p\",\"in\":\"header\",\"schema\":{\"type\":\"string\"}}";
    styles[3] = "{\"name\":\"p\",\"in\":\"query\",\"explode\":true,\"schema\":{"
                "\"type\":"
                "\"string\"}}";
    styles[4] = "{\"name\":\"p\",\"in\":\"query\",\"explode\":false,\"schema\":"
                "{\"type\":"
                "\"string\"}}";
    styles[5] =
        "{\"name\":\"p\",\"in\":\"query\",\"content\":{\"application/json\":"
        "{\"schema\":{\"type\":\"string\"},\"example\":\"ex\"}}}";

    for (i = 0; i < n_st; ++i) {
      jv = json_parse_string(styles[i]);
      if (jv) {
        jo = json_value_get_object(jv);
        memset(&param, 0, sizeof(param));
        ASSERT_EQ(CDD_C_SUCCESS,
                  cdd_test_parse_parameter_object(jo, &param, NULL, 0));
        cdd_test_free_parameter(&param);
        json_value_free(jv);
      }
    }
  }

  /* 9. parse_parameters_array edge cases */
  {
    /* Empty array */
    jv = json_parse_string("[]");
    if (jv) {
      out_params = NULL;
      out_count = 0;
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_test_parse_parameters_array(json_value_get_array(jv),
                                                &out_params, &out_count, NULL));
      ASSERT_EQ(0, out_count);
      json_value_free(jv);
    }

    /* Array with null / non-object element */
    jv = json_parse_string("[null, 42]");
    if (jv) {
      out_params = NULL;
      out_count = 0;
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_test_parse_parameters_array(json_value_get_array(jv),
                                                &out_params, &out_count, NULL));
      ASSERT_EQ(0, out_count);
      json_value_free(jv);
    }

    /* Array with reserved header parameter only */
    jv = json_parse_string("[{\"name\":\"Authorization\",\"in\":\"header\","
                           "\"schema\":{\"type\":\"string\"}}]");
    if (jv) {
      out_params = NULL;
      out_count = 0;
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_test_parse_parameters_array(json_value_get_array(jv),
                                                &out_params, &out_count, NULL));
      ASSERT_EQ(0, out_count);
      json_value_free(jv);
    }

    /* Array with reserved header + valid query param (realloc valid < count) */
    jv = json_parse_string(
        "[{\"name\":\"Authorization\",\"in\":\"header\","
        "\"schema\":{\"type\":\"string\"}},{\"name\":\"q\",\"in\":\"query\","
        "\"schema\":{\"type\":\"string\"}}]");
    if (jv) {
      out_params = NULL;
      out_count = 0;
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_test_parse_parameters_array(json_value_get_array(jv),
                                                &out_params, &out_count, NULL));
      ASSERT_EQ(1, out_count);
      cdd_test_free_parameter(&out_params[0]);
      free(out_params);
      json_value_free(jv);
    }

    /* Array with duplicate parameters */
    jv = json_parse_string(
        "[{\"name\":\"q\",\"in\":\"query\",\"schema\":{\"type\":\"string\"}},"
        "{\"name\":\"q\",\"in\":\"query\",\"schema\":{\"type\":\"string\"}}]");
    if (jv) {
      out_params = NULL;
      out_count = 0;
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_test_parse_parameters_array(json_value_get_array(jv),
                                                &out_params, &out_count, NULL));
      free(out_params);
      json_value_free(jv);
    }

    /* Array with invalid parameter */
    jv = json_parse_string("[{\"name\":\"q\",\"in\":\"unknown\"}]");
    if (jv) {
      out_params = NULL;
      out_count = 0;
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_test_parse_parameters_array(json_value_get_array(jv),
                                                &out_params, &out_count, NULL));
      free(out_params);
      json_value_free(jv);
    }

    /* Array with ref parameter without name */
    jv = json_parse_string("[{\"$ref\":\"#/components/parameters/NotFound\"}]");
    if (jv) {
      out_params = NULL;
      out_count = 0;
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_test_parse_parameters_array(json_value_get_array(jv),
                                                &out_params, &out_count, NULL));
      if (out_params) {
        cdd_test_free_parameter(&out_params[0]);
        free(out_params);
      }
      json_value_free(jv);
    }

    /* Array with null out_count */
    jv = json_parse_string("[{\"name\":\"q\",\"in\":\"query\"}]");
    if (jv) {
      out_params = NULL;
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_test_parse_parameters_array(json_value_get_array(jv),
                                                &out_params, NULL, NULL));
      json_value_free(jv);
    }
  }

  /* 9b. Additional branches: swagger 2 without type, querystring with schema
   * ref */
  memset(&spec, 0, sizeof(spec));
  spec.swagger_version = strdup("2.0");
  jv = json_parse_string("{\"name\":\"p_notype\",\"in\":\"query\"}");
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&param, 0, sizeof(param));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_parse_parameter_object(jo, &param, &spec, 0));
    cdd_test_free_parameter(&param);
    json_value_free(jv);
  }
  openapi_spec_free(&spec);

  memset(&spec, 0, sizeof(spec));
  jv = json_parse_string(
      "{\"name\":\"q_ref\",\"in\":\"querystring\",\"content\":{\"application/"
      "json\":{\"schema\":{\"type\":\"object\",\"$ref\":\"#/components/schemas/"
      "Existing\"}}}}");
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&param, 0, sizeof(param));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_parse_parameter_object(jo, &param, &spec, 0));
    cdd_test_free_parameter(&param);
    json_value_free(jv);
  }
  openapi_spec_free(&spec);

  /* Media type ref with empty schema */
  memset(&spec, 0, sizeof(spec));
  spec.n_component_media_types = 1;
  spec.component_media_type_names = (char **)calloc(1, sizeof(char *));
  spec.component_media_type_names[0] = strdup("MtEmpty");
  spec.component_media_types =
      (struct OpenAPI_MediaType *)calloc(1, sizeof(struct OpenAPI_MediaType));
  jv = json_parse_string(
      "{\"name\":\"p\",\"in\":\"query\",\"content\":{\"application/json\":"
      "{\"$ref\":\"#/components/mediaTypes/MtEmpty\"}}}");
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&param, 0, sizeof(param));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_parse_parameter_object(jo, &param, &spec, 1));
    cdd_test_free_parameter(&param);
    json_value_free(jv);
  }
  openapi_spec_free(&spec);

  /* Querystring with resolved media ref */
  memset(&spec, 0, sizeof(spec));
  spec.n_component_media_types = 1;
  spec.component_media_type_names = (char **)calloc(1, sizeof(char *));
  spec.component_media_type_names[0] = strdup("MtQs");
  spec.component_media_types =
      (struct OpenAPI_MediaType *)calloc(1, sizeof(struct OpenAPI_MediaType));
  spec.component_media_types[0].schema_set = 1;
  spec.component_media_types[0].schema.ref_name = strdup("QsRef");
  jv = json_parse_string(
      "{\"name\":\"p_qs\",\"in\":\"querystring\",\"content\":{\"application/"
      "json\":{\"$ref\":\"#/components/mediaTypes/MtQs\"}}}");
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&param, 0, sizeof(param));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_parse_parameter_object(jo, &param, &spec, 1));
    cdd_test_free_parameter(&param);
    json_value_free(jv);
  }
  openapi_spec_free(&spec);

  /* Edge cases: ref with resolve_refs=0, content non-object value, media ref
   * resolve_refs=0 or spec=NULL or NotFound */
  jv = json_parse_string("{\"$ref\":\"#/components/parameters/"
                         "MyParam\",\"description\":\"desc\"}");
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&param, 0, sizeof(param));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_parse_parameter_object(jo, &param, NULL, 0));
    cdd_test_free_parameter(&param);
    json_value_free(jv);
  }

  jv = json_parse_string("{\"name\":\"p\",\"in\":\"query\",\"content\":{"
                         "\"application/json\":123}}");
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&param, 0, sizeof(param));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_parse_parameter_object(jo, &param, NULL, 0));
    cdd_test_free_parameter(&param);
    json_value_free(jv);
  }

  memset(&spec, 0, sizeof(spec));
  jv = json_parse_string(
      "{\"name\":\"p\",\"in\":\"query\",\"content\":{\"application/json\":"
      "{\"$ref\":\"#/components/mediaTypes/NotFound\"}}}");
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&param, 0, sizeof(param));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_parse_parameter_object(jo, &param, &spec, 1));
    cdd_test_free_parameter(&param);

    memset(&param, 0, sizeof(param));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_parse_parameter_object(jo, &param, &spec, 0));
    cdd_test_free_parameter(&param);

    memset(&param, 0, sizeof(param));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_parse_parameter_object(jo, &param, NULL, 1));
    cdd_test_free_parameter(&param);
    json_value_free(jv);
  }
  openapi_spec_free(&spec);

  memset(&spec, 0, sizeof(spec));
  jv = json_parse_string(
      "{\"name\":\"p_prop\",\"in\":\"querystring\",\"content\":{\"application/"
      "json\":{\"schema\":{\"properties\":{\"a\":{\"type\":\"string\"}}}}}}");
  if (jv) {
    jo = json_value_get_object(jv);
    memset(&param, 0, sizeof(param));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_parse_parameter_object(jo, &param, &spec, 0));
    cdd_test_free_parameter(&param);
    json_value_free(jv);
  }
  openapi_spec_free(&spec);

  /* 10. OOM paths */
  {
    const char *oom_jsons[9];
    size_t n_oom = 9;
    oom_jsons[0] = "{\"name\":\"q\",\"in\":\"query\",\"description\":\"desc\","
                   "\"schema\":{\"type\":\"string\"},\"x-ext\":\"val\"}";
    oom_jsons[1] = "{\"name\":\"p\",\"in\":\"query\",\"examples\":{\"ex1\":{"
                   "\"value\":\"v1\"},"
                   "\"ex2\":{\"value\":\"v2\"}}}";
    oom_jsons[2] =
        "{\"name\":\"p\",\"in\":\"query\",\"content\":{\"application/json\":"
        "{\"schema\":{\"type\":\"string\"},\"example\":\"ex\"}}}";
    oom_jsons[3] = "{\"name\":\"p\",\"in\":\"querystring\",\"content\":{"
                   "\"application/json\":"
                   "{\"schema\":{\"type\":\"object\",\"properties\":{\"a\":{"
                   "\"type\":\"string\"}}}}}}";
    oom_jsons[4] =
        "{\"name\":\"p\",\"in\":\"query\",\"content\":{\"application/json\":"
        "{\"$ref\":\"#/components/mediaTypes/Mt1\"}}}";
    oom_jsons[5] = "{\"name\":\"p_ex\",\"in\":\"query\",\"example\":\"foo\"}";
    oom_jsons[6] = "{\"name\":\"p_ex2\",\"in\":\"query\",\"example\":\"foo\","
                   "\"schema\":{\"type\":\"string\"}}";
    oom_jsons[7] =
        "{\"name\":\"q_no_schema\",\"in\":\"query\",\"x-ext\":\"val\"}";
    oom_jsons[8] =
        "{\"name\":\"p_no_sch\",\"in\":\"query\",\"content\":{\"application/"
        "json\":{\"example\":\"ex\"}}}";

    memset(&spec, 0, sizeof(spec));
    spec.n_component_media_types = 1;
    spec.component_media_type_names = (char **)calloc(1, sizeof(char *));
    spec.component_media_type_names[0] = strdup("Mt1");
    spec.component_media_types =
        (struct OpenAPI_MediaType *)calloc(1, sizeof(struct OpenAPI_MediaType));
    spec.component_media_types[0].schema_set = 1;
    spec.component_media_types[0].schema.ref_name = strdup("SchemaRef1");

    for (i = 0; i < n_oom; ++i) {
      jv = json_parse_string(oom_jsons[i]);
      if (jv) {
        jo = json_value_get_object(jv);
        for (k = 1; k <= 30; ++k) {
          g_cdd_alloc_fail = k;
          memset(&param, 0, sizeof(param));
          cdd_test_parse_parameter_object(jo, &param, &spec, 1);
          cdd_test_free_parameter(&param);
          g_cdd_alloc_fail = 0;

          g_cdd_strdup_fail = k;
          memset(&param, 0, sizeof(param));
          cdd_test_parse_parameter_object(jo, &param, &spec, 1);
          cdd_test_free_parameter(&param);
          g_cdd_strdup_fail = 0;
        }
        json_value_free(jv);
      }
    }
    openapi_spec_free(&spec);

    /* Array calloc failure */
    jv = json_parse_string("[{\"name\":\"q\",\"in\":\"query\"}]");
    if (jv) {
      g_cdd_alloc_fail = 1;
      out_params = NULL;
      out_count = 0;
      ASSERT_EQ(CDD_C_ERROR_MEMORY,
                cdd_test_parse_parameters_array(json_value_get_array(jv),
                                                &out_params, &out_count, NULL));
      g_cdd_alloc_fail = 0;
      json_value_free(jv);
    }

    /* Array shrinking realloc failure (keeps original pointer) */
    jv = json_parse_string(
        "[{\"name\":\"Authorization\",\"in\":\"header\","
        "\"schema\":{\"type\":\"string\"}},{\"name\":\"q\",\"in\":\"query\","
        "\"schema\":{\"type\":\"string\"}}]");
    if (jv) {
      g_cdd_alloc_fail = 2;
      out_params = NULL;
      out_count = 0;
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_test_parse_parameters_array(json_value_get_array(jv),
                                                &out_params, &out_count, NULL));
      g_cdd_alloc_fail = 0;
      ASSERT_EQ(1, out_count);
      cdd_test_free_parameter(&out_params[0]);
      free(out_params);
      json_value_free(jv);
    }
  }

  g_cdd_alloc_fail = 0;
  g_cdd_strdup_fail = 0;
  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_OPENAPI_LOADER_PARAMS_H */
