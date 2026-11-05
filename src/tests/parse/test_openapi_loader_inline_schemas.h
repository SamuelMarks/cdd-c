/**
 * @file test_openapi_loader_inline_schemas.h
 * @brief Inline response and parameter schema format, content, enum, and union
 * loader tests.
 * @author Samuel Marks
 */

#ifndef TEST_OPENAPI_LOADER_INLINE_SCHEMAS_H
#define TEST_OPENAPI_LOADER_INLINE_SCHEMAS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "parse/test_openapi_loader_common.h"
/* clang-format on */

/**
 * @brief Unit test test_load_inline_response_schema_primitive.
 * @return Greatest test result.
 */
TEST test_load_inline_response_schema_primitive(void) {

  const char *json =
      "{\"paths\":{\"/"
      "r\":{\"get\":{\"responses\":{\"200\":{\"description\":\"OK\","
      "\"content\":{\"application/"
      "json\":{\"schema\":{\"type\":\"string\"}}}}}}}},\"openapi\":\"3.2.0\"}";

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("string",
                spec.paths[0].operations[0].responses[0].schema.inline_type);
  ASSERT_EQ(0, spec.paths[0].operations[0].responses[0].schema.is_array);

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_load_inline_response_schema_array.
 * @return Greatest test result.
 */
TEST test_load_inline_response_schema_array(void) {

  const char *json =
      "{\"openapi\":\"3.2.0\",\"paths\":{\"/r\":{\"get\":{"
      "\"responses\":{\"200\":{\"description\":\"OK\","
      "\"content\":{\"application/json\":{"
      "\"schema\":{\"type\":\"array\",\"items\":{\"type\":\"integer\"}}}"
      "}}}}}}}";

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(1, spec.paths[0].operations[0].responses[0].schema.is_array);
  ASSERT_STR_EQ("integer",
                spec.paths[0].operations[0].responses[0].schema.inline_type);

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_load_inline_schema_format_and_content.
 * @return Greatest test result.
 */
TEST test_load_inline_schema_format_and_content(void) {

  const char json[] = {
      123, 34,  111, 112, 101, 110, 97,  112, 105, 34,  58,  34,  51,  46,  50,
      46,  48,  34,  44,  34,  105, 110, 102, 111, 34,  58,  123, 34,  116, 105,
      116, 108, 101, 34,  58,  34,  116, 34,  44,  34,  118, 101, 114, 115, 105,
      111, 110, 34,  58,  34,  49,  34,  125, 44,  34,  112, 97,  116, 104, 115,
      34,  58,  123, 34,  47,  114, 34,  58,  123, 34,  103, 101, 116, 34,  58,
      123, 34,  114, 101, 115, 112, 111, 110, 115, 101, 115, 34,  58,  123, 34,
      50,  48,  48,  34,  58,  123, 34,  100, 101, 115, 99,  114, 105, 112, 116,
      105, 111, 110, 34,  58,  34,  79,  75,  34,  44,  34,  99,  111, 110, 116,
      101, 110, 116, 34,  58,  123, 34,  97,  112, 112, 108, 105, 99,  97,  116,
      105, 111, 110, 47,  106, 115, 111, 110, 34,  58,  123, 34,  115, 99,  104,
      101, 109, 97,  34,  58,  123, 34,  116, 121, 112, 101, 34,  58,  34,  115,
      116, 114, 105, 110, 103, 34,  44,  34,  102, 111, 114, 109, 97,  116, 34,
      58,  34,  117, 117, 105, 100, 34,  44,  34,  99,  111, 110, 116, 101, 110,
      116, 77,  101, 100, 105, 97,  84,  121, 112, 101, 34,  58,  34,  105, 109,
      97,  103, 101, 47,  112, 110, 103, 34,  44,  34,  99,  111, 110, 116, 101,
      110, 116, 69,  110, 99,  111, 100, 105, 110, 103, 34,  58,  34,  98,  97,
      115, 101, 54,  52,  34,  125, 125, 125, 125, 125, 125, 125, 125, 125, 0};

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);
  {
    struct OpenAPI_SchemaRef *schema =
        &spec.paths[0].operations[0].responses[0].schema;
    ASSERT_STR_EQ("uuid", schema->format);
    ASSERT_STR_EQ("image/png", schema->content_media_type);
    ASSERT_STR_EQ("base64", schema->content_encoding);
  }

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_load_inline_schema_array_item_format_and_content.
 * @return Greatest test result.
 */
TEST test_load_inline_schema_array_item_format_and_content(void) {

  const char *json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
      "\"paths\":{\"/"
      "r\":{\"get\":{\"responses\":{\"200\":{\"description\":\"OK\","
      "\"content\":{\"application/json\":{\"schema\":{\"type\":\"array\","
      "\"items\":{\"type\":\"string\",\"format\":\"uuid\","
      "\"contentMediaType\":\"image/"
      "png\",\"contentEncoding\":\"base64\"}}}}}}}}}}";

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);
  {
    struct OpenAPI_SchemaRef *schema =
        &spec.paths[0].operations[0].responses[0].schema;
    ASSERT_EQ(1, schema->is_array);
    ASSERT_STR_EQ("uuid", schema->items_format);
    ASSERT_STR_EQ("image/png", schema->items_content_media_type);
    ASSERT_STR_EQ("base64", schema->items_content_encoding);
  }

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_load_inline_schema_const_examples_annotations.
 * @return Greatest test result.
 */
TEST test_load_inline_schema_const_examples_annotations(void) {

  const char json[] = {
      123, 34,  111, 112, 101, 110, 97,  112, 105, 34,  58,  34,  51,  46,  50,
      46,  48,  34,  44,  34,  105, 110, 102, 111, 34,  58,  123, 34,  116, 105,
      116, 108, 101, 34,  58,  34,  116, 34,  44,  34,  118, 101, 114, 115, 105,
      111, 110, 34,  58,  34,  49,  34,  125, 44,  34,  112, 97,  116, 104, 115,
      34,  58,  123, 34,  47,  112, 34,  58,  123, 34,  103, 101, 116, 34,  58,
      123, 34,  112, 97,  114, 97,  109, 101, 116, 101, 114, 115, 34,  58,  91,
      123, 34,  110, 97,  109, 101, 34,  58,  34,  109, 111, 100, 101, 34,  44,
      34,  105, 110, 34,  58,  34,  113, 117, 101, 114, 121, 34,  44,  34,  115,
      99,  104, 101, 109, 97,  34,  58,  123, 34,  116, 121, 112, 101, 34,  58,
      34,  115, 116, 114, 105, 110, 103, 34,  44,  34,  99,  111, 110, 115, 116,
      34,  58,  34,  102, 97,  115, 116, 34,  44,  34,  101, 120, 97,  109, 112,
      108, 101, 115, 34,  58,  91,  34,  102, 97,  115, 116, 34,  44,  34,  115,
      108, 111, 119, 34,  93,  44,  34,  100, 101, 115, 99,  114, 105, 112, 116,
      105, 111, 110, 34,  58,  34,  77,  111, 100, 101, 34,  44,  34,  100, 101,
      112, 114, 101, 99,  97,  116, 101, 100, 34,  58,  116, 114, 117, 101, 44,
      34,  114, 101, 97,  100, 79,  110, 108, 121, 34,  58,  116, 114, 117, 101,
      44,  34,  119, 114, 105, 116, 101, 79,  110, 108, 121, 34,  58,  102, 97,
      108, 115, 101, 125, 125, 93,  44,  34,  114, 101, 115, 112, 111, 110, 115,
      101, 115, 34,  58,  123, 34,  50,  48,  48,  34,  58,  123, 34,  100, 101,
      115, 99,  114, 105, 112, 116, 105, 111, 110, 34,  58,  34,  79,  75,  34,
      125, 125, 125, 125, 125, 125, 0};

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);
  {
    struct OpenAPI_SchemaRef *schema =
        &spec.paths[0].operations[0].parameters[0].schema;
    ASSERT_EQ(1, schema->const_value_set);
    ASSERT_EQ(OA_ANY_STRING, schema->const_value.type);
    ASSERT_STR_EQ("fast", schema->const_value.string);
    ASSERT_EQ(2, schema->n_examples);
    ASSERT_EQ(OA_ANY_STRING, schema->examples[0].type);
    ASSERT_STR_EQ("fast", schema->examples[0].string);
    ASSERT_EQ(OA_ANY_STRING, schema->examples[1].type);
    ASSERT_STR_EQ("slow", schema->examples[1].string);
    ASSERT_STR_EQ("Mode", schema->description);
    ASSERT_EQ(1, schema->deprecated_set);
    ASSERT_EQ(1, schema->deprecated);
    ASSERT_EQ(1, schema->read_only_set);
    ASSERT_EQ(1, schema->read_only);
    ASSERT_EQ(1, schema->write_only_set);
    ASSERT_EQ(0, schema->write_only);
  }

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_load_schema_ref_summary_description.
 * @return Greatest test result.
 */
TEST test_load_schema_ref_summary_description(void) {

  const char *json =
      "{\"paths\":{\"/"
      "p\":{\"get\":{\"parameters\":[{\"name\":\"mode\",\"in\":\"query\","
      "\"schema\":{\"$ref\":\"#/components/schemas/Mode\",\"summary\":\"Mode "
      "summary\",\"description\":\"Mode "
      "description\"}}],\"responses\":{\"200\":{\"description\":\"OK\"}}}}},"
      "\"components\":{\"schemas\":{\"Mode\":{\"type\":\"string\"}}},"
      "\"openapi\":\"3.2.0\"}";

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);
  {
    struct OpenAPI_SchemaRef *schema =
        &spec.paths[0].operations[0].parameters[0].schema;
    ASSERT_STR_EQ("Mode", schema->ref_name);
    ASSERT_STR_EQ("Mode summary", schema->summary);
    ASSERT_STR_EQ("Mode description", schema->description);
  }

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_load_parameter_schema_format_and_content.
 * @return Greatest test result.
 */
TEST test_load_parameter_schema_format_and_content(void) {

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
      116, 114, 105, 110, 103, 34,  44,  34,  102, 111, 114, 109, 97,  116, 34,
      58,  34,  117, 117, 105, 100, 34,  44,  34,  99,  111, 110, 116, 101, 110,
      116, 77,  101, 100, 105, 97,  84,  121, 112, 101, 34,  58,  34,  116, 101,
      120, 116, 47,  112, 108, 97,  105, 110, 34,  44,  34,  99,  111, 110, 116,
      101, 110, 116, 69,  110, 99,  111, 100, 105, 110, 103, 34,  58,  34,  98,
      97,  115, 101, 54,  52,  34,  125, 125, 93,  44,  34,  114, 101, 115, 112,
      111, 110, 115, 101, 115, 34,  58,  123, 34,  50,  48,  48,  34,  58,  123,
      34,  100, 101, 115, 99,  114, 105, 112, 116, 105, 111, 110, 34,  58,  34,
      79,  75,  34,  125, 125, 125, 125, 125, 125, 0};

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);
  {
    struct OpenAPI_Parameter *p = &spec.paths[0].operations[0].parameters[0];
    ASSERT_EQ(1, p->schema_set);
    ASSERT_STR_EQ("uuid", p->schema.format);
    ASSERT_STR_EQ("text/plain", p->schema.content_media_type);
    ASSERT_STR_EQ("base64", p->schema.content_encoding);
  }

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_load_inline_schema_enum_default_nullable.
 * @return Greatest test result.
 */
TEST test_load_inline_schema_enum_default_nullable(void) {

  const char json[] = {
      123, 34,  111, 112, 101, 110, 97,  112, 105, 34,  58,  34,  51,  46,  50,
      46,  48,  34,  44,  34,  105, 110, 102, 111, 34,  58,  123, 34,  116, 105,
      116, 108, 101, 34,  58,  34,  116, 34,  44,  34,  118, 101, 114, 115, 105,
      111, 110, 34,  58,  34,  49,  34,  125, 44,  34,  112, 97,  116, 104, 115,
      34,  58,  123, 34,  47,  112, 34,  58,  123, 34,  103, 101, 116, 34,  58,
      123, 34,  112, 97,  114, 97,  109, 101, 116, 101, 114, 115, 34,  58,  91,
      123, 34,  110, 97,  109, 101, 34,  58,  34,  115, 116, 97,  116, 117, 115,
      34,  44,  34,  105, 110, 34,  58,  34,  113, 117, 101, 114, 121, 34,  44,
      34,  115, 99,  104, 101, 109, 97,  34,  58,  123, 34,  116, 121, 112, 101,
      34,  58,  91,  34,  115, 116, 114, 105, 110, 103, 34,  44,  34,  110, 117,
      108, 108, 34,  93,  44,  34,  101, 110, 117, 109, 34,  58,  91,  34,  111,
      110, 34,  44,  34,  111, 102, 102, 34,  93,  44,  34,  100, 101, 102, 97,
      117, 108, 116, 34,  58,  34,  111, 110, 34,  125, 125, 93,  44,  34,  114,
      101, 115, 112, 111, 110, 115, 101, 115, 34,  58,  123, 34,  50,  48,  48,
      34,  58,  123, 34,  100, 101, 115, 99,  114, 105, 112, 116, 105, 111, 110,
      34,  58,  34,  79,  75,  34,  125, 125, 125, 125, 125, 125, 0};

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);
  {
    struct OpenAPI_Parameter *p = &spec.paths[0].operations[0].parameters[0];
    ASSERT_EQ(1, p->schema_set);
    ASSERT_STR_EQ("string", p->schema.inline_type);
    ASSERT_EQ(1, p->schema.nullable);
    ASSERT_EQ(2, p->schema.n_enum_values);
    ASSERT_EQ(OA_ANY_STRING, p->schema.enum_values[0].type);
    ASSERT_STR_EQ("on", p->schema.enum_values[0].string);
    ASSERT_EQ(OA_ANY_STRING, p->schema.enum_values[1].type);
    ASSERT_STR_EQ("off", p->schema.enum_values[1].string);
    ASSERT_EQ(1, p->schema.default_value_set);
    ASSERT_EQ(OA_ANY_STRING, p->schema.default_value.type);
    ASSERT_STR_EQ("on", p->schema.default_value.string);
  }

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_load_inline_schema_type_union.
 * @return Greatest test result.
 */
TEST test_load_inline_schema_type_union(void) {

  const char *json =
      "{\"paths\":{\"/"
      "p\":{\"get\":{\"parameters\":[{\"name\":\"mix\",\"in\":\"query\","
      "\"schema\":{\"type\":[\"string\",\"integer\",\"null\"]}}],\"responses\":"
      "{\"200\":{\"description\":\"OK\"}}}}},\"openapi\":\"3.2.0\"}";

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);
  {
    struct OpenAPI_Parameter *p = &spec.paths[0].operations[0].parameters[0];
    ASSERT_EQ(1, p->schema_set);
    ASSERT_STR_EQ("string", p->schema.inline_type);
    ASSERT_EQ(1, p->schema.nullable);
    ASSERT_EQ(3, p->schema.n_type_union);
    ASSERT_STR_EQ("string", p->schema.type_union[0]);
    ASSERT_STR_EQ("integer", p->schema.type_union[1]);
    ASSERT_STR_EQ("null", p->schema.type_union[2]);
  }

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_load_inline_schema_array_items_enum_nullable.
 * @return Greatest test result.
 */
TEST test_load_inline_schema_array_items_enum_nullable(void) {

  const char *json =
      "{\"paths\":{\"/"
      "p\":{\"get\":{\"parameters\":[{\"name\":\"tags\",\"in\":\"query\","
      "\"schema\":{\"type\":\"array\",\"items\":{\"type\":[\"string\",\"null\"]"
      ",\"enum\":[\"a\",\"b\"]}}}],\"responses\":{\"200\":{\"description\":"
      "\"OK\"}}}}},\"openapi\":\"3.2.0\"}";

  struct OpenAPI_Spec spec = {0};
  int rc = load_spec_str(json, &spec);
  ASSERT_EQ(0, rc);
  {
    struct OpenAPI_Parameter *p = &spec.paths[0].operations[0].parameters[0];
    ASSERT_EQ(1, p->schema_set);
    ASSERT_EQ(1, p->schema.is_array);
    ASSERT_STR_EQ("string", p->schema.inline_type);
    ASSERT_EQ(1, p->schema.items_nullable);
    ASSERT_EQ(2, p->schema.n_items_enum_values);
    ASSERT_EQ(OA_ANY_STRING, p->schema.items_enum_values[0].type);
    ASSERT_STR_EQ("a", p->schema.items_enum_values[0].string);
    ASSERT_EQ(OA_ANY_STRING, p->schema.items_enum_values[1].type);
    ASSERT_STR_EQ("b", p->schema.items_enum_values[1].string);
  }

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_OPENAPI_LOADER_INLINE_SCHEMAS_H */
