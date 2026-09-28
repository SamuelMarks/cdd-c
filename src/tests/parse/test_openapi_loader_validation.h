/**
 * @file test_openapi_loader_validation.h
 * @brief Validation tests for parameters, tags, operations, and encodings.
 */

#ifndef TEST_OPENAPI_LOADER_VALIDATION_H
#define TEST_OPENAPI_LOADER_VALIDATION_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "parse/test_openapi_loader_common.h"
/* clang-format on */

TEST test_load_parameter_array(void) {

  const char *json =
      "{\"paths\":{\"/"
      "q\":{\"get\":{\"parameters\":[{\"name\":\"tags\",\"in\":\"query\","
      "\"schema\":{\"type\":\"array\",\"items\":{\"type\":\"integer\"}},"
      "\"style\":\"form\",\"explode\":true}],\"responses\":{\"200\":{"
      "\"description\":\"OK\"}}}}},\"openapi\":\"3.2.0\"}";

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  {
    struct OpenAPI_Parameter *p = &spec.paths[0].operations[0].parameters[0];
    ASSERT_STR_EQ("tags", p->name);
    ASSERT_STR_EQ("array", p->type);
    ASSERT_EQ(1, p->is_array);
    ASSERT_STR_EQ("integer", p->items_type);
    ASSERT_EQ(OA_STYLE_FORM, p->style);
    ASSERT_EQ(1, p->explode);
  }

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_load_schema_parsing(void) {
  struct StructFields *_ast_openapi_spec_find_schema_0;

  /* Test that schemas are loaded into the global registry */
  const char *json = "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\","
                     "\"version\":\"1\"},\"components\":{\"schemas\":{"
                     "\"Login\":{\"type\":\"object\",\"properties\":{\"user\":{"
                     "\"type\":\"string\"}}}"
                     "}}}";
  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  ASSERT_EQ(1, spec.n_defined_schemas);
  ASSERT_STR_EQ("Login", spec.defined_schema_names[0]);

  {
    const struct StructFields *sf =
        (openapi_spec_find_schema(&spec, "Login",
                                  &_ast_openapi_spec_find_schema_0),
         _ast_openapi_spec_find_schema_0);
    ASSERT(sf != NULL);
    ASSERT_EQ(1, sf->size);
    ASSERT_STR_EQ("user", sf->fields[0].name);
    ASSERT_STR_EQ("string", sf->fields[0].type);
  }

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_load_schema_external_docs_discriminator_xml(void) {

  const char json[] = {
      123, 34,  111, 112, 101, 110, 97,  112, 105, 34,  58,  34,  51,  46,  50,
      46,  48,  34,  44,  34,  105, 110, 102, 111, 34,  58,  123, 34,  116, 105,
      116, 108, 101, 34,  58,  34,  116, 34,  44,  34,  118, 101, 114, 115, 105,
      111, 110, 34,  58,  34,  49,  34,  125, 44,  34,  112, 97,  116, 104, 115,
      34,  58,  123, 34,  47,  112, 34,  58,  123, 34,  103, 101, 116, 34,  58,
      123, 34,  112, 97,  114, 97,  109, 101, 116, 101, 114, 115, 34,  58,  91,
      123, 34,  110, 97,  109, 101, 34,  58,  34,  105, 100, 34,  44,  34,  105,
      110, 34,  58,  34,  113, 117, 101, 114, 121, 34,  44,  34,  115, 99,  104,
      101, 109, 97,  34,  58,  123, 34,  116, 121, 112, 101, 34,  58,  34,  115,
      116, 114, 105, 110, 103, 34,  44,  34,  101, 120, 116, 101, 114, 110, 97,
      108, 68,  111, 99,  115, 34,  58,  123, 34,  117, 114, 108, 34,  58,  34,
      104, 116, 116, 112, 115, 58,  47,  47,  101, 120, 97,  109, 112, 108, 101,
      46,  99,  111, 109, 47,  100, 111, 99,  115, 34,  44,  34,  100, 101, 115,
      99,  114, 105, 112, 116, 105, 111, 110, 34,  58,  34,  83,  99,  104, 101,
      109, 97,  32,  100, 111, 99,  115, 34,  125, 44,  34,  100, 105, 115, 99,
      114, 105, 109, 105, 110, 97,  116, 111, 114, 34,  58,  123, 34,  112, 114,
      111, 112, 101, 114, 116, 121, 78,  97,  109, 101, 34,  58,  34,  107, 105,
      110, 100, 34,  44,  34,  109, 97,  112, 112, 105, 110, 103, 34,  58,  123,
      34,  97,  34,  58,  34,  35,  47,  99,  111, 109, 112, 111, 110, 101, 110,
      116, 115, 47,  115, 99,  104, 101, 109, 97,  115, 47,  65,  34,  125, 44,
      34,  100, 101, 102, 97,  117, 108, 116, 77,  97,  112, 112, 105, 110, 103,
      34,  58,  34,  35,  47,  99,  111, 109, 112, 111, 110, 101, 110, 116, 115,
      47,  115, 99,  104, 101, 109, 97,  115, 47,  66,  97,  115, 101, 34,  125,
      44,  34,  120, 109, 108, 34,  58,  123, 34,  110, 97,  109, 101, 34,  58,
      34,  105, 100, 34,  44,  34,  110, 97,  109, 101, 115, 112, 97,  99,  101,
      34,  58,  34,  104, 116, 116, 112, 115, 58,  47,  47,  101, 120, 97,  109,
      112, 108, 101, 46,  99,  111, 109, 47,  110, 115, 34,  44,  34,  112, 114,
      101, 102, 105, 120, 34,  58,  34,  112, 34,  44,  34,  110, 111, 100, 101,
      84,  121, 112, 101, 34,  58,  34,  97,  116, 116, 114, 105, 98,  117, 116,
      101, 34,  125, 125, 125, 93,  44,  34,  114, 101, 115, 112, 111, 110, 115,
      101, 115, 34,  58,  123, 34,  50,  48,  48,  34,  58,  123, 34,  100, 101,
      115, 99,  114, 105, 112, 116, 105, 111, 110, 34,  58,  34,  79,  75,  34,
      125, 125, 125, 125, 125, 125, 0};

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  {
    struct OpenAPI_Parameter *p = &spec.paths[0].operations[0].parameters[0];
    ASSERT(p->schema.external_docs_set == 1);
    ASSERT_STR_EQ("https://example.com/docs", p->schema.external_docs.url);
    ASSERT_STR_EQ("Schema docs", p->schema.external_docs.description);
    ASSERT(p->schema.discriminator_set == 1);
    ASSERT_STR_EQ("kind", p->schema.discriminator.property_name);
    ASSERT_EQ(1, p->schema.discriminator.n_mapping);
    ASSERT_STR_EQ("a", p->schema.discriminator.mapping[0].value);
    ASSERT_STR_EQ("#/components/schemas/A",
                  p->schema.discriminator.mapping[0].schema);
    ASSERT_STR_EQ("#/components/schemas/Base",
                  p->schema.discriminator.default_mapping);
    ASSERT(p->schema.xml_set == 1);
    ASSERT_STR_EQ("id", p->schema.xml.name);
    ASSERT_STR_EQ("https://example.com/ns", p->schema.xml.namespace_uri);
    ASSERT_STR_EQ("p", p->schema.xml.prefix);
    ASSERT_EQ(OA_XML_NODE_ATTRIBUTE, p->schema.xml.node_type);
  }

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_load_form_content_type(void) {

  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
      "\"paths\":{\"/"
      "login\":{\"post\":{\"requestBody\":{\"content\":{\"application/"
      "x-www-form-urlencoded\":{\"schema\":{\"$ref\":\"#/components/schemas/"
      "Login\"}}}},\"responses\":{\"200\":{\"description\":\"OK\"}}}}}}";

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);

  ASSERT_STR_EQ("application/x-www-form-urlencoded",
                spec.paths[0].operations[0].req_body.content_type);
  ASSERT_STR_EQ("Login", spec.paths[0].operations[0].req_body.ref_name);

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_request_body_content_required(void) {

  const char *json = "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\","
                     "\"version\":\"1\"},\"paths\":{\"/login\":{\"post\":{"
                     "\"requestBody\":{},"
                     "\"responses\":{\"200\":{\"description\":\"OK\"}}"
                     "}}}}";
  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_param_content_multiple_entries_rejected(void) {

  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
      "\"paths\":{\"/"
      "q\":{\"get\":{\"parameters\":[{\"name\":\"q\",\"in\":\"query\","
      "\"content\":{\"application/json\":{},\"text/"
      "plain\":{}}}],\"responses\":{\"200\":{\"description\":\"OK\"}}}}}}";
  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_header_content_multiple_entries_rejected(void) {

  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
      "\"paths\":{\"/r\":{\"get\":{\"responses\":{\"200\":{"
      "\"description\":\"OK\","
      "\"headers\":{\"X-Rate\":{\"content\":{"
      "\"application/json\":{},\"text/plain\":{}}}}}}}}}}";
  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_response_description_required(void) {

  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
      "\"paths\":{\"/r\":{\"get\":{\"responses\":{\"200\":{}}}}}}";
  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);
  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_operation_responses_required(void) {
  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
      "\"paths\":{\"/"
      "r\":{\"get\":{}}}}";
  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);
  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_response_code_key_invalid_rejected(void) {
  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
      "\"paths\":{\"/r\":{\"get\":{\"responses\":{\"20X\":{"
      "\"description\":\"OK\"}}}}}}";
  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_response_code_range_valid(void) {
  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
      "\"paths\":{\"/r\":{\"get\":{\"responses\":{\"2XX\":{"
      "\"description\":\"OK\"}}}}}}";
  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);
  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_paths_require_leading_slash(void) {

  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
      "\"paths\":{\"pets\":{\"get\":{\"responses\":{\"200\":{"
      "\"description\":\"OK\"}}}}}}";
  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_paths_ambiguous_templates_rejected(void) {

  const char *json = "{\"paths\":{\"/pets/"
                     "{petId}\":{\"get\":{\"responses\":{\"200\":{"
                     "\"description\":\"OK\"}}}},\"/pets/"
                     "{name}\":{\"get\":{\"responses\":{\"200\":{"
                     "\"description\":\"OK\"}}}}},\"openapi\":\"3.2.0\"}";
  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_component_key_regex_rejected(void) {

  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
      "\"components\":{\"schemas\":{\"Bad/Name\":{\"type\":\"string\"}}}}";
  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_tag_duplicate_rejected(void) {

  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
      "\"tags\":[{\"name\":\"dup\"},{\"name\":\"dup\"}]}";
  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_tag_name_required(void) {

  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
      "\"tags\":[{\"description\":\"missing\"}]}";
  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_tag_parent_missing_rejected(void) {

  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
      "\"tags\":[{\"name\":\"child\",\"parent\":\"ghost\"}]}";
  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_tag_parent_cycle_rejected(void) {

  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
      "\"tags\":[{\"name\":\"a\",\"parent\":\"b\"},"
      "{\"name\":\"b\",\"parent\":\"a\"}]}";
  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_external_docs_url_required(void) {

  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
      "\"externalDocs\":{\"description\":\"Docs\"}}";
  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_operation_id_duplicate_rejected(void) {

  const char json[] = {
      123, 34,  111, 112, 101, 110, 97,  112, 105, 34,  58,  34,  51,  46,  50,
      46,  48,  34,  44,  34,  105, 110, 102, 111, 34,  58,  123, 34,  116, 105,
      116, 108, 101, 34,  58,  34,  116, 34,  44,  34,  118, 101, 114, 115, 105,
      111, 110, 34,  58,  34,  49,  34,  125, 44,  34,  112, 97,  116, 104, 115,
      34,  58,  123, 34,  47,  112, 101, 116, 115, 34,  58,  123, 34,  103, 101,
      116, 34,  58,  123, 34,  111, 112, 101, 114, 97,  116, 105, 111, 110, 73,
      100, 34,  58,  34,  108, 105, 115, 116, 80,  101, 116, 115, 34,  44,  34,
      114, 101, 115, 112, 111, 110, 115, 101, 115, 34,  58,  123, 34,  50,  48,
      48,  34,  58,  123, 34,  100, 101, 115, 99,  114, 105, 112, 116, 105, 111,
      110, 34,  58,  34,  79,  75,  34,  125, 125, 125, 125, 44,  34,  47,  99,
      97,  116, 115, 34,  58,  123, 34,  103, 101, 116, 34,  58,  123, 34,  111,
      112, 101, 114, 97,  116, 105, 111, 110, 73,  100, 34,  58,  34,  108, 105,
      115, 116, 80,  101, 116, 115, 34,  44,  34,  114, 101, 115, 112, 111, 110,
      115, 101, 115, 34,  58,  123, 34,  50,  48,  48,  34,  58,  123, 34,  100,
      101, 115, 99,  114, 105, 112, 116, 105, 111, 110, 34,  58,  34,  79,  75,
      34,  125, 125, 125, 125, 125, 125, 0};
  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_operation_id_duplicate_in_callback_rejected(void) {

  const char json[] = {
      123, 34,  111, 112, 101, 110, 97,  112, 105, 34,  58,  34,  51,  46,  50,
      46,  48,  34,  44,  34,  105, 110, 102, 111, 34,  58,  123, 34,  116, 105,
      116, 108, 101, 34,  58,  34,  116, 34,  44,  34,  118, 101, 114, 115, 105,
      111, 110, 34,  58,  34,  49,  34,  125, 44,  34,  112, 97,  116, 104, 115,
      34,  58,  123, 34,  47,  112, 101, 116, 115, 34,  58,  123, 34,  103, 101,
      116, 34,  58,  123, 34,  111, 112, 101, 114, 97,  116, 105, 111, 110, 73,
      100, 34,  58,  34,  100, 117, 112, 34,  44,  34,  114, 101, 115, 112, 111,
      110, 115, 101, 115, 34,  58,  123, 34,  50,  48,  48,  34,  58,  123, 34,
      100, 101, 115, 99,  114, 105, 112, 116, 105, 111, 110, 34,  58,  34,  79,
      75,  34,  125, 125, 44,  34,  99,  97,  108, 108, 98,  97,  99,  107, 115,
      34,  58,  123, 34,  99,  98,  34,  58,  123, 34,  123, 36,  114, 101, 113,
      117, 101, 115, 116, 46,  98,  111, 100, 121, 35,  47,  117, 114, 108, 125,
      34,  58,  123, 34,  112, 111, 115, 116, 34,  58,  123, 34,  111, 112, 101,
      114, 97,  116, 105, 111, 110, 73,  100, 34,  58,  34,  100, 117, 112, 34,
      44,  34,  114, 101, 115, 112, 111, 110, 115, 101, 115, 34,  58,  123, 34,
      50,  48,  48,  34,  58,  123, 34,  100, 101, 115, 99,  114, 105, 112, 116,
      105, 111, 110, 34,  58,  34,  79,  75,  34,  125, 125, 125, 125, 125, 125,
      125, 125, 125, 125, 0};
  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_parameter_duplicates_rejected(void) {

  const char *json =
      "{\"paths\":{\"/"
      "p\":{\"get\":{\"parameters\":[{\"name\":\"id\",\"in\":\"query\","
      "\"schema\":{\"type\":\"string\"}},{\"name\":\"id\",\"in\":\"query\","
      "\"schema\":{\"type\":\"string\"}}],\"responses\":{\"200\":{"
      "\"description\":\"OK\"}}}}},\"openapi\":\"3.2.0\"}";
  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_querystring_with_query_rejected(void) {

  const char *json =
      "{\"paths\":{\"/"
      "q\":{\"get\":{\"parameters\":[{\"name\":\"raw\",\"in\":\"querystring\","
      "\"content\":{\"application/"
      "x-www-form-urlencoded\":{}}},{\"name\":\"q\",\"in\":\"query\","
      "\"schema\":{\"type\":\"string\"}}],\"responses\":{\"200\":{"
      "\"description\":\"OK\"}}}}},\"openapi\":\"3.2.0\"}";
  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_querystring_duplicate_rejected(void) {

  const char json[] = {
      123, 34,  111, 112, 101, 110, 97,  112, 105, 34,  58,  34,  51,  46,  50,
      46,  48,  34,  44,  34,  105, 110, 102, 111, 34,  58,  123, 34,  116, 105,
      116, 108, 101, 34,  58,  34,  116, 34,  44,  34,  118, 101, 114, 115, 105,
      111, 110, 34,  58,  34,  49,  34,  125, 44,  34,  112, 97,  116, 104, 115,
      34,  58,  123, 34,  47,  113, 34,  58,  123, 34,  103, 101, 116, 34,  58,
      123, 34,  112, 97,  114, 97,  109, 101, 116, 101, 114, 115, 34,  58,  91,
      123, 34,  110, 97,  109, 101, 34,  58,  34,  114, 97,  119, 34,  44,  34,
      105, 110, 34,  58,  34,  113, 117, 101, 114, 121, 115, 116, 114, 105, 110,
      103, 34,  44,  34,  99,  111, 110, 116, 101, 110, 116, 34,  58,  123, 34,
      97,  112, 112, 108, 105, 99,  97,  116, 105, 111, 110, 47,  120, 45,  119,
      119, 119, 45,  102, 111, 114, 109, 45,  117, 114, 108, 101, 110, 99,  111,
      100, 101, 100, 34,  58,  123, 125, 125, 125, 44,  123, 34,  110, 97,  109,
      101, 34,  58,  34,  114, 97,  119, 50,  34,  44,  34,  105, 110, 34,  58,
      34,  113, 117, 101, 114, 121, 115, 116, 114, 105, 110, 103, 34,  44,  34,
      99,  111, 110, 116, 101, 110, 116, 34,  58,  123, 34,  97,  112, 112, 108,
      105, 99,  97,  116, 105, 111, 110, 47,  120, 45,  119, 119, 119, 45,  102,
      111, 114, 109, 45,  117, 114, 108, 101, 110, 99,  111, 100, 101, 100, 34,
      58,  123, 125, 125, 125, 93,  44,  34,  114, 101, 115, 112, 111, 110, 115,
      101, 115, 34,  58,  123, 34,  50,  48,  48,  34,  58,  123, 34,  100, 101,
      115, 99,  114, 105, 112, 116, 105, 111, 110, 34,  58,  34,  79,  75,  34,
      125, 125, 125, 125, 125, 125, 0};
  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_querystring_path_and_operation_mixed_rejected(void) {

  const char *json =
      "{\"paths\":{\"/"
      "q\":{\"parameters\":[{\"name\":\"raw\",\"in\":\"querystring\","
      "\"content\":{\"application/"
      "x-www-form-urlencoded\":{}}}],\"get\":{\"parameters\":[{\"name\":\"q\","
      "\"in\":\"query\",\"schema\":{\"type\":\"string\"}}],\"responses\":{"
      "\"200\":{\"description\":\"OK\"}}}}},\"openapi\":\"3.2.0\"}";
  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_querystring_with_query_in_callback_rejected(void) {

  const char json[] = {
      123, 34,  112, 97,  116, 104, 115, 34,  58,  123, 34,  47,  112, 101, 116,
      115, 34,  58,  123, 34,  103, 101, 116, 34,  58,  123, 34,  114, 101, 115,
      112, 111, 110, 115, 101, 115, 34,  58,  123, 34,  50,  48,  48,  34,  58,
      123, 34,  100, 101, 115, 99,  114, 105, 112, 116, 105, 111, 110, 34,  58,
      34,  79,  75,  34,  125, 125, 44,  34,  99,  97,  108, 108, 98,  97,  99,
      107, 115, 34,  58,  123, 34,  99,  98,  34,  58,  123, 34,  123, 36,  114,
      101, 113, 117, 101, 115, 116, 46,  98,  111, 100, 121, 35,  47,  117, 114,
      108, 125, 34,  58,  123, 34,  112, 111, 115, 116, 34,  58,  123, 34,  112,
      97,  114, 97,  109, 101, 116, 101, 114, 115, 34,  58,  91,  123, 34,  110,
      97,  109, 101, 34,  58,  34,  114, 97,  119, 34,  44,  34,  105, 110, 34,
      58,  34,  113, 117, 101, 114, 121, 115, 116, 114, 105, 110, 103, 34,  44,
      34,  99,  111, 110, 116, 101, 110, 116, 34,  58,  123, 34,  97,  112, 112,
      108, 105, 99,  97,  116, 105, 111, 110, 47,  120, 45,  119, 119, 119, 45,
      102, 111, 114, 109, 45,  117, 114, 108, 101, 110, 99,  111, 100, 101, 100,
      34,  58,  123, 125, 125, 125, 44,  123, 34,  110, 97,  109, 101, 34,  58,
      34,  113, 34,  44,  34,  105, 110, 34,  58,  34,  113, 117, 101, 114, 121,
      34,  44,  34,  115, 99,  104, 101, 109, 97,  34,  58,  123, 34,  116, 121,
      112, 101, 34,  58,  34,  115, 116, 114, 105, 110, 103, 34,  125, 125, 93,
      44,  34,  114, 101, 115, 112, 111, 110, 115, 101, 115, 34,  58,  123, 34,
      50,  48,  48,  34,  58,  123, 34,  100, 101, 115, 99,  114, 105, 112, 116,
      105, 111, 110, 34,  58,  34,  79,  75,  34,  125, 125, 125, 125, 125, 125,
      125, 125, 125, 44,  34,  111, 112, 101, 110, 97,  112, 105, 34,  58,  34,
      51,  46,  50,  46,  48,  34,  125, 0};
  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_parameter_missing_name_or_in_rejected(void) {

  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
      "\"paths\":{\"/p\":{\"get\":{\"parameters\":["
      "{\"in\":\"query\",\"schema\":{\"type\":\"string\"}}"
      "],\"responses\":{\"200\":{\"description\":\"OK\"}}}}}}";
  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_header_style_non_simple_rejected(void) {

  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
      "\"paths\":{\"/r\":{\"get\":{\"responses\":{\"200\":{"
      "\"description\":\"OK\","
      "\"headers\":{\"X-Test\":{\"schema\":{\"type\":\"string\"},"
      "\"style\":\"form\"}}}}}}}}";
  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_media_type_encoding_conflict_rejected(void) {

  const char json[] = {
      123, 34,  111, 112, 101, 110, 97,  112, 105, 34,  58,  34,  51,  46,  50,
      46,  48,  34,  44,  34,  105, 110, 102, 111, 34,  58,  123, 34,  116, 105,
      116, 108, 101, 34,  58,  34,  116, 34,  44,  34,  118, 101, 114, 115, 105,
      111, 110, 34,  58,  34,  49,  34,  125, 44,  34,  112, 97,  116, 104, 115,
      34,  58,  123, 34,  47,  117, 34,  58,  123, 34,  112, 111, 115, 116, 34,
      58,  123, 34,  114, 101, 113, 117, 101, 115, 116, 66,  111, 100, 121, 34,
      58,  123, 34,  99,  111, 110, 116, 101, 110, 116, 34,  58,  123, 34,  109,
      117, 108, 116, 105, 112, 97,  114, 116, 47,  102, 111, 114, 109, 45,  100,
      97,  116, 97,  34,  58,  123, 34,  115, 99,  104, 101, 109, 97,  34,  58,
      123, 34,  116, 121, 112, 101, 34,  58,  34,  111, 98,  106, 101, 99,  116,
      34,  44,  34,  112, 114, 111, 112, 101, 114, 116, 105, 101, 115, 34,  58,
      123, 34,  97,  34,  58,  123, 34,  116, 121, 112, 101, 34,  58,  34,  115,
      116, 114, 105, 110, 103, 34,  125, 125, 125, 44,  34,  101, 110, 99,  111,
      100, 105, 110, 103, 34,  58,  123, 34,  97,  34,  58,  123, 125, 125, 44,
      34,  112, 114, 101, 102, 105, 120, 69,  110, 99,  111, 100, 105, 110, 103,
      34,  58,  91,  123, 125, 93,  125, 125, 125, 44,  34,  114, 101, 115, 112,
      111, 110, 115, 101, 115, 34,  58,  123, 34,  50,  48,  48,  34,  58,  123,
      34,  100, 101, 115, 99,  114, 105, 112, 116, 105, 111, 110, 34,  58,  34,
      79,  75,  34,  125, 125, 125, 125, 125, 125, 0};
  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_encoding_object_conflict_rejected(void) {

  const char json[] = {
      123, 34,  111, 112, 101, 110, 97,  112, 105, 34,  58,  34,  51,  46,  50,
      46,  48,  34,  44,  34,  105, 110, 102, 111, 34,  58,  123, 34,  116, 105,
      116, 108, 101, 34,  58,  34,  116, 34,  44,  34,  118, 101, 114, 115, 105,
      111, 110, 34,  58,  34,  49,  34,  125, 44,  34,  112, 97,  116, 104, 115,
      34,  58,  123, 34,  47,  117, 34,  58,  123, 34,  112, 111, 115, 116, 34,
      58,  123, 34,  114, 101, 113, 117, 101, 115, 116, 66,  111, 100, 121, 34,
      58,  123, 34,  99,  111, 110, 116, 101, 110, 116, 34,  58,  123, 34,  109,
      117, 108, 116, 105, 112, 97,  114, 116, 47,  102, 111, 114, 109, 45,  100,
      97,  116, 97,  34,  58,  123, 34,  115, 99,  104, 101, 109, 97,  34,  58,
      123, 34,  116, 121, 112, 101, 34,  58,  34,  111, 98,  106, 101, 99,  116,
      34,  44,  34,  112, 114, 111, 112, 101, 114, 116, 105, 101, 115, 34,  58,
      123, 34,  97,  34,  58,  123, 34,  116, 121, 112, 101, 34,  58,  34,  115,
      116, 114, 105, 110, 103, 34,  125, 125, 125, 44,  34,  101, 110, 99,  111,
      100, 105, 110, 103, 34,  58,  123, 34,  97,  34,  58,  123, 34,  101, 110,
      99,  111, 100, 105, 110, 103, 34,  58,  123, 34,  98,  34,  58,  123, 125,
      125, 44,  34,  105, 116, 101, 109, 69,  110, 99,  111, 100, 105, 110, 103,
      34,  58,  123, 125, 125, 125, 125, 125, 125, 44,  34,  114, 101, 115, 112,
      111, 110, 115, 101, 115, 34,  58,  123, 34,  50,  48,  48,  34,  58,  123,
      34,  100, 101, 115, 99,  114, 105, 112, 116, 105, 111, 110, 34,  58,  34,
      79,  75,  34,  125, 125, 125, 125, 125, 125, 0};
  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_OPENAPI_LOADER_VALIDATION_H */
