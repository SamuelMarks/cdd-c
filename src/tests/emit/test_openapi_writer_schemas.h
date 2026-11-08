/**
 * @file test_openapi_writer_schemas.h
 * @brief Unit tests for OpenAPI Writer (schemas).
 */

#ifndef TEST_OPENAPI_WRITER_SCHEMAS_H
#define TEST_OPENAPI_WRITER_SCHEMAS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "emit/test_openapi_writer_common.h"
/* clang-format on */

/**
 * @brief Unit test test_writer_inline_response_schema_primitive.
 * @return Greatest test result.
 */
TEST test_writer_inline_response_schema_primitive(void) {
  int rc = 0;
  struct OpenAPI_Spec spec;
  char *json;
  struct OpenAPI_Path path = {0};
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Response resp = {0};
  json = NULL;

  setup_test_spec(&spec, &path, &op, NULL, &resp);
  resp.schema.inline_type = cdd_test_mut_str("string");

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root;
    JSON_Object *r200;
    JSON_Object *content;
    JSON_Object *media;
    JSON_Object *schema;
    JSON_Object *responses;
    {
      JSON_Value *_cdd_jv_1 = NULL;
      cdd_json_parse_string(json, &_cdd_jv_1);
      root = _cdd_jv_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      JSON_Object *_cdd_jo_2 = NULL;
      JSON_Object *_cdd_jo_3 = NULL;
      cdd_json_object_get_object(json_value_get_object(root), "paths",
                                 &_cdd_jo_1);
      cdd_json_object_get_object(_cdd_jo_1, "/test/route", &_cdd_jo_2);
      cdd_json_object_get_object(_cdd_jo_2, "get", &_cdd_jo_3);
      responses = _cdd_jo_3;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(responses, "responses", &_cdd_jo_1);
      responses = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(responses, "200", &_cdd_jo_1);
      r200 = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(r200, "content", &_cdd_jo_1);
      content = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(content, "application/json", &_cdd_jo_1);
      media = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(media, "schema", &_cdd_jo_1);
      schema = _cdd_jo_1;
    }

    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(schema, "type", &_cdd_js_1);
      ASSERT_STR_EQ("string", _cdd_js_1);
    }
    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_writer_inline_response_schema_array.
 * @return Greatest test result.
 */
TEST test_writer_inline_response_schema_array(void) {
  int rc = 0;
  struct OpenAPI_Spec spec;
  char *json;
  struct OpenAPI_Path path = {0};
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Response resp = {0};
  json = NULL;

  setup_test_spec(&spec, &path, &op, NULL, &resp);
  resp.schema.is_array = 1;
  resp.schema.inline_type = cdd_test_mut_str("integer");

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root;
    JSON_Object *r200;
    JSON_Object *content;
    JSON_Object *media;
    JSON_Object *schema;
    JSON_Object *items;
    JSON_Object *responses;
    {
      JSON_Value *_cdd_jv_1 = NULL;
      cdd_json_parse_string(json, &_cdd_jv_1);
      root = _cdd_jv_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      JSON_Object *_cdd_jo_2 = NULL;
      JSON_Object *_cdd_jo_3 = NULL;
      cdd_json_object_get_object(json_value_get_object(root), "paths",
                                 &_cdd_jo_1);
      cdd_json_object_get_object(_cdd_jo_1, "/test/route", &_cdd_jo_2);
      cdd_json_object_get_object(_cdd_jo_2, "get", &_cdd_jo_3);
      responses = _cdd_jo_3;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(responses, "responses", &_cdd_jo_1);
      responses = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(responses, "200", &_cdd_jo_1);
      r200 = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(r200, "content", &_cdd_jo_1);
      content = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(content, "application/json", &_cdd_jo_1);
      media = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(media, "schema", &_cdd_jo_1);
      schema = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(schema, "items", &_cdd_jo_1);
      items = _cdd_jo_1;
    }

    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(schema, "type", &_cdd_js_1);
      ASSERT_STR_EQ("array", _cdd_js_1);
    }
    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(items, "type", &_cdd_js_1);
      ASSERT_STR_EQ("integer", _cdd_js_1);
    }
    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_writer_inline_schema_format_and_content.
 * @return Greatest test result.
 */
TEST test_writer_inline_schema_format_and_content(void) {
  int rc = 0;
  struct OpenAPI_Spec spec;
  char *json;
  struct OpenAPI_Path path = {0};
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Response resp = {0};
  json = NULL;

  setup_test_spec(&spec, &path, &op, NULL, &resp);
  resp.schema.inline_type = cdd_test_mut_str("string");
  resp.schema.format = cdd_test_mut_str("uuid");
  resp.schema.content_media_type = cdd_test_mut_str("image/png");
  resp.schema.content_encoding = cdd_test_mut_str("base64");

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root;
    JSON_Object *r200;
    JSON_Object *content;
    JSON_Object *media;
    JSON_Object *schema;
    JSON_Object *responses;
    {
      JSON_Value *_cdd_jv_1 = NULL;
      cdd_json_parse_string(json, &_cdd_jv_1);
      root = _cdd_jv_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      JSON_Object *_cdd_jo_2 = NULL;
      JSON_Object *_cdd_jo_3 = NULL;
      cdd_json_object_get_object(json_value_get_object(root), "paths",
                                 &_cdd_jo_1);
      cdd_json_object_get_object(_cdd_jo_1, "/test/route", &_cdd_jo_2);
      cdd_json_object_get_object(_cdd_jo_2, "get", &_cdd_jo_3);
      responses = _cdd_jo_3;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(responses, "responses", &_cdd_jo_1);
      responses = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(responses, "200", &_cdd_jo_1);
      r200 = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(r200, "content", &_cdd_jo_1);
      content = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(content, "application/json", &_cdd_jo_1);
      media = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(media, "schema", &_cdd_jo_1);
      schema = _cdd_jo_1;
    }

    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(schema, "type", &_cdd_js_1);
      ASSERT_STR_EQ("string", _cdd_js_1);
    }
    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(schema, "format", &_cdd_js_1);
      ASSERT_STR_EQ("uuid", _cdd_js_1);
    }
    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(schema, "contentMediaType", &_cdd_js_1);
      ASSERT_STR_EQ("image/png", _cdd_js_1);
    }
    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(schema, "contentEncoding", &_cdd_js_1);
      ASSERT_STR_EQ("base64", _cdd_js_1);
    }
    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_writer_inline_schema_array_item_format_and_content.
 * @return Greatest test result.
 */
TEST test_writer_inline_schema_array_item_format_and_content(void) {
  int rc = 0;
  struct OpenAPI_Spec spec;
  char *json;
  struct OpenAPI_Path path = {0};
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Response resp = {0};
  json = NULL;

  setup_test_spec(&spec, &path, &op, NULL, &resp);
  resp.schema.is_array = 1;
  resp.schema.inline_type = cdd_test_mut_str("string");
  resp.schema.items_format = cdd_test_mut_str("uuid");
  resp.schema.items_content_media_type = cdd_test_mut_str("image/png");
  resp.schema.items_content_encoding = cdd_test_mut_str("base64");

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root;
    JSON_Object *r200;
    JSON_Object *content;
    JSON_Object *media;
    JSON_Object *schema;
    JSON_Object *items;
    JSON_Object *responses;
    {
      JSON_Value *_cdd_jv_1 = NULL;
      cdd_json_parse_string(json, &_cdd_jv_1);
      root = _cdd_jv_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      JSON_Object *_cdd_jo_2 = NULL;
      JSON_Object *_cdd_jo_3 = NULL;
      cdd_json_object_get_object(json_value_get_object(root), "paths",
                                 &_cdd_jo_1);
      cdd_json_object_get_object(_cdd_jo_1, "/test/route", &_cdd_jo_2);
      cdd_json_object_get_object(_cdd_jo_2, "get", &_cdd_jo_3);
      responses = _cdd_jo_3;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(responses, "responses", &_cdd_jo_1);
      responses = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(responses, "200", &_cdd_jo_1);
      r200 = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(r200, "content", &_cdd_jo_1);
      content = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(content, "application/json", &_cdd_jo_1);
      media = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(media, "schema", &_cdd_jo_1);
      schema = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(schema, "items", &_cdd_jo_1);
      items = _cdd_jo_1;
    }

    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(schema, "type", &_cdd_js_1);
      ASSERT_STR_EQ("array", _cdd_js_1);
    }
    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(items, "type", &_cdd_js_1);
      ASSERT_STR_EQ("string", _cdd_js_1);
    }
    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(items, "format", &_cdd_js_1);
      ASSERT_STR_EQ("uuid", _cdd_js_1);
    }
    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(items, "contentMediaType", &_cdd_js_1);
      ASSERT_STR_EQ("image/png", _cdd_js_1);
    }
    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(items, "contentEncoding", &_cdd_js_1);
      ASSERT_STR_EQ("base64", _cdd_js_1);
    }
    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_writer_schema_external_docs_discriminator_xml.
 * @return Greatest test result.
 */
TEST test_writer_schema_external_docs_discriminator_xml(void) {
  int rc = 0;
  struct OpenAPI_Spec spec;
  struct OpenAPI_DiscriminatorMap map;
  char *json;
  struct OpenAPI_Path path = {0};
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Response resp = {0};
  json = NULL;

  setup_test_spec(&spec, &path, &op, NULL, &resp);

  resp.schema.ref_name = NULL;
  resp.schema.inline_type = cdd_test_mut_str("string");
  resp.schema.external_docs_set = 1;
  resp.schema.external_docs.url =
      cdd_test_mut_str("https://example.com/schema-doc");
  resp.schema.external_docs.description =
      cdd_test_mut_str("Schema external docs");
  resp.schema.discriminator_set = 1;
  resp.schema.discriminator.property_name = cdd_test_mut_str("kind");
  resp.schema.discriminator.default_mapping =
      cdd_test_mut_str("#/components/schemas/Base");
  map.value = cdd_test_mut_str("cat");
  map.schema = cdd_test_mut_str("#/components/schemas/Cat");
  resp.schema.discriminator.mapping = &map;
  resp.schema.discriminator.n_mapping = 1;
  resp.schema.xml_set = 1;
  resp.schema.xml.node_type_set = 1;
  resp.schema.xml.node_type = OA_XML_NODE_ATTRIBUTE;
  resp.schema.xml.name = cdd_test_mut_str("id");
  resp.schema.xml.namespace_uri = cdd_test_mut_str("https://example.com/ns");
  resp.schema.xml.prefix = cdd_test_mut_str("p");

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);
  ASSERT(json != NULL);

  {
    JSON_Value *root;
    JSON_Object *p_item;
    JSON_Object *op_obj;
    JSON_Object *responses;
    JSON_Object *r200;
    JSON_Object *content;
    JSON_Object *media;
    JSON_Object *schema;
    JSON_Object *ext;
    JSON_Object *disc;
    JSON_Object *mapping;
    JSON_Object *xml;
    JSON_Object *paths;
    {
      JSON_Value *_cdd_jv_1 = NULL;
      cdd_json_parse_string(json, &_cdd_jv_1);
      root = _cdd_jv_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(json_value_get_object(root), "paths",
                                 &_cdd_jo_1);
      paths = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(paths, "/test/route", &_cdd_jo_1);
      p_item = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(p_item, "get", &_cdd_jo_1);
      op_obj = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(op_obj, "responses", &_cdd_jo_1);
      responses = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(responses, "200", &_cdd_jo_1);
      r200 = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(r200, "content", &_cdd_jo_1);
      content = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(content, "application/json", &_cdd_jo_1);
      media = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(media, "schema", &_cdd_jo_1);
      schema = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(schema, "externalDocs", &_cdd_jo_1);
      ext = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(schema, "discriminator", &_cdd_jo_1);
      disc = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(disc, "mapping", &_cdd_jo_1);
      mapping = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(schema, "xml", &_cdd_jo_1);
      xml = _cdd_jo_1;
    }

    ASSERT(ext != NULL);
    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(ext, "url", &_cdd_js_1);
      ASSERT_STR_EQ("https://example.com/schema-doc", _cdd_js_1);
    }
    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(ext, "description", &_cdd_js_1);
      ASSERT_STR_EQ("Schema external docs", _cdd_js_1);
    }
    ASSERT(disc != NULL);
    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(disc, "propertyName", &_cdd_js_1);
      ASSERT_STR_EQ("kind", _cdd_js_1);
    }
    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(disc, "defaultMapping", &_cdd_js_1);
      ASSERT_STR_EQ("#/components/schemas/Base", _cdd_js_1);
    }
    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(mapping, "cat", &_cdd_js_1);
      ASSERT_STR_EQ("#/components/schemas/Cat", _cdd_js_1);
    }
    ASSERT(xml != NULL);
    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(xml, "nodeType", &_cdd_js_1);
      ASSERT_STR_EQ("attribute", _cdd_js_1);
    }
    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(xml, "name", &_cdd_js_1);
      ASSERT_STR_EQ("id", _cdd_js_1);
    }
    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(xml, "namespace", &_cdd_js_1);
      ASSERT_STR_EQ("https://example.com/ns", _cdd_js_1);
    }
    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(xml, "prefix", &_cdd_js_1);
      ASSERT_STR_EQ("p", _cdd_js_1);
    }
    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_writer_inline_schema_const_examples_annotations.
 * @return Greatest test result.
 */
TEST test_writer_inline_schema_const_examples_annotations(void) {
  int rc = 0;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Any examples[2];
  char *json;
  struct OpenAPI_Path path = {0};
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Response resp = {0};
  json = NULL;

  memset(examples, 0, sizeof(examples));
  examples[0].type = OA_ANY_STRING;
  examples[0].string = cdd_test_mut_str("fast");
  examples[1].type = OA_ANY_STRING;
  examples[1].string = cdd_test_mut_str("slow");

  setup_test_spec(&spec, &path, &op, NULL, &resp);
  resp.schema.inline_type = cdd_test_mut_str("string");
  resp.schema.description = cdd_test_mut_str("Mode");
  resp.schema.deprecated_set = 1;
  resp.schema.deprecated = 1;
  resp.schema.read_only_set = 1;
  resp.schema.read_only = 1;
  resp.schema.write_only_set = 1;
  resp.schema.write_only = 0;
  resp.schema.const_value_set = 1;
  resp.schema.const_value.type = OA_ANY_STRING;
  resp.schema.const_value.string = cdd_test_mut_str("fast");
  resp.schema.examples = examples;
  resp.schema.n_examples = 2;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root;
    JSON_Object *r200;
    JSON_Object *content;
    JSON_Object *media;
    JSON_Object *schema;
    JSON_Array *examples_arr;
    JSON_Object *responses;
    {
      JSON_Value *_cdd_jv_1 = NULL;
      cdd_json_parse_string(json, &_cdd_jv_1);
      root = _cdd_jv_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      JSON_Object *_cdd_jo_2 = NULL;
      JSON_Object *_cdd_jo_3 = NULL;
      cdd_json_object_get_object(json_value_get_object(root), "paths",
                                 &_cdd_jo_1);
      cdd_json_object_get_object(_cdd_jo_1, "/test/route", &_cdd_jo_2);
      cdd_json_object_get_object(_cdd_jo_2, "get", &_cdd_jo_3);
      responses = _cdd_jo_3;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(responses, "responses", &_cdd_jo_1);
      responses = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(responses, "200", &_cdd_jo_1);
      r200 = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(r200, "content", &_cdd_jo_1);
      content = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(content, "application/json", &_cdd_jo_1);
      media = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(media, "schema", &_cdd_jo_1);
      schema = _cdd_jo_1;
    }
    {
      JSON_Array *_cdd_ja_1 = NULL;
      cdd_json_object_get_array(schema, "examples", &_cdd_ja_1);
      examples_arr = _cdd_ja_1;
    }

    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(schema, "const", &_cdd_js_1);
      ASSERT_STR_EQ("fast", _cdd_js_1);
    }
    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(schema, "description", &_cdd_js_1);
      ASSERT_STR_EQ("Mode", _cdd_js_1);
    }
    {
      int _cdd_jb_1 = -1;
      if (cdd_json_object_get_boolean(schema, "deprecated", &_cdd_jb_1) !=
          CDD_C_SUCCESS)
        _cdd_jb_1 = -1;
      ASSERT_EQ(1, _cdd_jb_1);
    }
    {
      int _cdd_jb_1 = -1;
      if (cdd_json_object_get_boolean(schema, "readOnly", &_cdd_jb_1) !=
          CDD_C_SUCCESS)
        _cdd_jb_1 = -1;
      ASSERT_EQ(1, _cdd_jb_1);
    }
    {
      int _cdd_jb_1 = -1;
      if (cdd_json_object_get_boolean(schema, "writeOnly", &_cdd_jb_1) !=
          CDD_C_SUCCESS)
        _cdd_jb_1 = -1;
      ASSERT_EQ(0, _cdd_jb_1);
    }
    ASSERT_EQ(2, json_array_get_count(examples_arr));
    ASSERT_STR_EQ("fast", json_array_get_string(examples_arr, 0));
    ASSERT_STR_EQ("slow", json_array_get_string(examples_arr, 1));
    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_writer_preserves_composed_component_schema.
 * @return Greatest test result.
 */
TEST test_writer_preserves_composed_component_schema(void) {
  int rc = 0;
  const char json[] = {
      123, 34,  111, 112, 101, 110, 97,  112, 105, 34,  58,  34,  51,  46,  50,
      46,  48,  34,  44,  34,  105, 110, 102, 111, 34,  58,  123, 34,  116, 105,
      116, 108, 101, 34,  58,  34,  83,  112, 101, 99,  34,  44,  34,  118, 101,
      114, 115, 105, 111, 110, 34,  58,  34,  49,  34,  44,  34,  108, 105, 99,
      101, 110, 115, 101, 34,  58,  123, 34,  110, 97,  109, 101, 34,  58,  34,
      77,  73,  84,  34,  125, 125, 44,  34,  99,  111, 109, 112, 111, 110, 101,
      110, 116, 115, 34,  58,  123, 34,  115, 99,  104, 101, 109, 97,  115, 34,
      58,  123, 34,  80,  101, 116, 34,  58,  123, 34,  111, 110, 101, 79,  102,
      34,  58,  91,  123, 34,  116, 121, 112, 101, 34,  58,  34,  111, 98,  106,
      101, 99,  116, 34,  44,  34,  112, 114, 111, 112, 101, 114, 116, 105, 101,
      115, 34,  58,  123, 34,  105, 100, 34,  58,  123, 34,  116, 121, 112, 101,
      34,  58,  34,  105, 110, 116, 101, 103, 101, 114, 34,  125, 125, 125, 44,
      123, 34,  116, 121, 112, 101, 34,  58,  34,  111, 98,  106, 101, 99,  116,
      34,  44,  34,  112, 114, 111, 112, 101, 114, 116, 105, 101, 115, 34,  58,
      123, 34,  110, 97,  109, 101, 34,  58,  123, 34,  116, 121, 112, 101, 34,
      58,  34,  115, 116, 114, 105, 110, 103, 34,  125, 125, 125, 93,  125, 125,
      125, 125, 0};

  struct OpenAPI_Spec spec;
  char *out_json;
  out_json = NULL;

  ASSERT(load_spec_str2("{", &spec) != 0);
  ASSERT(load_spec_str2("{}", NULL) != 0);
  rc = load_spec_str2(json, &spec);
  ASSERT_EQ(0, rc);

  rc = openapi_write_spec_to_json(&spec, &out_json);
  ASSERT_EQ(0, rc);
  ASSERT(out_json != NULL);

  {
    JSON_Value *root;
    JSON_Object *root_obj;
    JSON_Object *comps;
    JSON_Object *schemas;
    JSON_Object *pet;
    JSON_Array *one_of;
    {
      JSON_Value *_cdd_jv_1 = NULL;
      cdd_json_parse_string(out_json, &_cdd_jv_1);
      root = _cdd_jv_1;
    }
    root_obj = json_value_get_object(root);
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(root_obj, "components", &_cdd_jo_1);
      comps = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(comps, "schemas", &_cdd_jo_1);
      schemas = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(schemas, "Pet", &_cdd_jo_1);
      pet = _cdd_jo_1;
    }
    {
      JSON_Array *_cdd_ja_1 = NULL;
      cdd_json_object_get_array(pet, "oneOf", &_cdd_ja_1);
      one_of = _cdd_ja_1;
    }
    ASSERT(one_of != NULL);
    ASSERT_EQ(2, json_array_get_count(one_of));
    json_value_free(root);
  }

  free(out_json);
  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_writer_preserves_inline_composed_schema.
 * @return Greatest test result.
 */
TEST test_writer_preserves_inline_composed_schema(void) {
  int rc = 0;
  /* */

  const char json[] = {
      123, 34,  111, 112, 101, 110, 97,  112, 105, 34,  58,  34,  51,  46,  50,
      46,  48,  34,  44,  34,  105, 110, 102, 111, 34,  58,  123, 34,  116, 105,
      116, 108, 101, 34,  58,  34,  83,  112, 101, 99,  34,  44,  34,  118, 101,
      114, 115, 105, 111, 110, 34,  58,  34,  49,  34,  44,  34,  108, 105, 99,
      101, 110, 115, 101, 34,  58,  123, 34,  110, 97,  109, 101, 34,  58,  34,
      77,  73,  84,  34,  125, 125, 44,  34,  112, 97,  116, 104, 115, 34,  58,
      123, 34,  47,  112, 101, 116, 115, 34,  58,  123, 34,  103, 101, 116, 34,
      58,  123, 34,  111, 112, 101, 114, 97,  116, 105, 111, 110, 73,  100, 34,
      58,  34,  71,  101, 116, 80,  101, 116, 115, 34,  44,  34,  114, 101, 115,
      112, 111, 110, 115, 101, 115, 34,  58,  123, 34,  50,  48,  48,  34,  58,
      123, 34,  100, 101, 115, 99,  114, 105, 112, 116, 105, 111, 110, 34,  58,
      34,  111, 107, 34,  44,  34,  99,  111, 110, 116, 101, 110, 116, 34,  58,
      123, 34,  97,  112, 112, 108, 105, 99,  97,  116, 105, 111, 110, 47,  106,
      115, 111, 110, 34,  58,  123, 34,  115, 99,  104, 101, 109, 97,  34,  58,
      123, 34,  111, 110, 101, 79,  102, 34,  58,  91,  123, 34,  116, 121, 112,
      101, 34,  58,  34,  111, 98,  106, 101, 99,  116, 34,  44,  34,  112, 114,
      111, 112, 101, 114, 116, 105, 101, 115, 34,  58,  123, 34,  105, 100, 34,
      58,  123, 34,  116, 121, 112, 101, 34,  58,  34,  105, 110, 116, 101, 103,
      101, 114, 34,  125, 125, 125, 44,  123, 34,  116, 121, 112, 101, 34,  58,
      34,  111, 98,  106, 101, 99,  116, 34,  44,  34,  112, 114, 111, 112, 101,
      114, 116, 105, 101, 115, 34,  58,  123, 34,  110, 97,  109, 101, 34,  58,
      123, 34,  116, 121, 112, 101, 34,  58,  34,  115, 116, 114, 105, 110, 103,
      34,  125, 125, 125, 93,  125, 125, 125, 125, 125, 125, 125, 125, 125, 0};

  struct OpenAPI_Spec spec;
  char *out_json;
  out_json = NULL;

  rc = load_spec_str2(json, &spec);
  ASSERT_EQ(0, rc);

  rc = openapi_write_spec_to_json(&spec, &out_json);
  ASSERT_EQ(0, rc);
  ASSERT(out_json != NULL);

  {
    JSON_Value *root;
    JSON_Object *root_obj;
    JSON_Object *comps;
    JSON_Object *schemas;
    JSON_Object *inline_schema;
    JSON_Array *one_of;
    {
      JSON_Value *_cdd_jv_1 = NULL;
      cdd_json_parse_string(out_json, &_cdd_jv_1);
      root = _cdd_jv_1;
    }
    root_obj = json_value_get_object(root);
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(root_obj, "components", &_cdd_jo_1);
      comps = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(comps, "schemas", &_cdd_jo_1);
      schemas = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(schemas, "Inline_GetPets_Response_200",
                                 &_cdd_jo_1);
      inline_schema = _cdd_jo_1;
    }
    ASSERT(inline_schema != NULL);
    {
      JSON_Array *_cdd_ja_1 = NULL;
      cdd_json_object_get_array(inline_schema, "oneOf", &_cdd_ja_1);
      one_of = _cdd_ja_1;
    }
    ASSERT(one_of != NULL);
    ASSERT_EQ(2, json_array_get_count(one_of));
    json_value_free(root);
  }

  free(out_json);
  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_writer_schema_ref_summary_description.
 * @return Greatest test result.
 */
TEST test_writer_schema_ref_summary_description(void) {
  int rc = 0;
  struct OpenAPI_Spec spec;
  char *json;
  struct OpenAPI_Path path = {0};
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Response resp = {0};
  json = NULL;

  setup_test_spec(&spec, &path, &op, NULL, &resp);
  resp.schema.ref_name = cdd_test_mut_str("Mode");
  resp.schema.summary = cdd_test_mut_str("Mode summary");
  resp.schema.description = cdd_test_mut_str("Mode description");

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root;
    JSON_Object *r200;
    JSON_Object *content;
    JSON_Object *media;
    JSON_Object *schema;
    JSON_Object *responses;
    {
      JSON_Value *_cdd_jv_1 = NULL;
      cdd_json_parse_string(json, &_cdd_jv_1);
      root = _cdd_jv_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      JSON_Object *_cdd_jo_2 = NULL;
      JSON_Object *_cdd_jo_3 = NULL;
      cdd_json_object_get_object(json_value_get_object(root), "paths",
                                 &_cdd_jo_1);
      cdd_json_object_get_object(_cdd_jo_1, "/test/route", &_cdd_jo_2);
      cdd_json_object_get_object(_cdd_jo_2, "get", &_cdd_jo_3);
      responses = _cdd_jo_3;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(responses, "responses", &_cdd_jo_1);
      responses = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(responses, "200", &_cdd_jo_1);
      r200 = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(r200, "content", &_cdd_jo_1);
      content = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(content, "application/json", &_cdd_jo_1);
      media = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(media, "schema", &_cdd_jo_1);
      schema = _cdd_jo_1;
    }

    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(schema, "$ref", &_cdd_js_1);
      ASSERT_STR_EQ("#/components/schemas/Mode", _cdd_js_1);
    }
    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(schema, "summary", &_cdd_js_1);
      ASSERT_STR_EQ("Mode summary", _cdd_js_1);
    }
    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(schema, "description", &_cdd_js_1);
      ASSERT_STR_EQ("Mode description", _cdd_js_1);
    }
    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_writer_info_license_missing_name_rejected.
 * @return Greatest test result.
 */
TEST test_writer_info_license_missing_name_rejected(void) {
  int rc = 0;
  char *json;
  struct OpenAPI_Spec spec = {0};
  json = NULL;

  spec.info.title = cdd_test_mut_str("Example");
  spec.info.version = cdd_test_mut_str("1.0");
  spec.info.license.identifier = cdd_test_mut_str("Apache-2.0");

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  ASSERT(json == NULL);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_writer_options_trace_verbs.
 * @return Greatest test result.
 */
TEST test_writer_options_trace_verbs(void) {
  int rc = 0;
  struct OpenAPI_Path path;
  struct OpenAPI_Operation ops[2];
  char *json;
  struct OpenAPI_Spec spec = {0};
  json = NULL;

  memset(&path, 0, sizeof(path));
  memset(ops, 0, sizeof(ops));
  path.route = cdd_test_mut_str("/verbs");
  path.operations = ops;
  path.n_operations = 2;
  ops[0].verb = OA_VERB_OPTIONS;
  ops[0].operation_id = cdd_test_mut_str("opt");
  ops[1].verb = OA_VERB_TRACE;
  ops[1].operation_id = cdd_test_mut_str("tr");
  spec.paths = &path;
  spec.n_paths = 1;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root;
    JSON_Object *verbs;
    {
      JSON_Value *_cdd_jv_1 = NULL;
      cdd_json_parse_string(json, &_cdd_jv_1);
      root = _cdd_jv_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      JSON_Object *_cdd_jo_2 = NULL;
      cdd_json_object_get_object(json_value_get_object(root), "paths",
                                 &_cdd_jo_1);
      cdd_json_object_get_object(_cdd_jo_1, "/verbs", &_cdd_jo_2);
      verbs = _cdd_jo_2;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(verbs, "options", &_cdd_jo_1);
      ASSERT(_cdd_jo_1 != NULL);
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(verbs, "trace", &_cdd_jo_1);
      ASSERT(_cdd_jo_1 != NULL);
    }
    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_writer_query_and_external_docs.
 * @return Greatest test result.
 */
TEST test_writer_query_and_external_docs(void) {
  int rc = 0;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Path path;
  char *json;
  struct OpenAPI_Operation op = {0};
  json = NULL;

  setup_test_spec(&spec, &path, &op, NULL, NULL);
  op.verb = OA_VERB_QUERY;
  op.operation_id = cdd_test_mut_str("querySearch");
  op.external_docs.url = cdd_test_mut_str("https://example.com/op");
  op.external_docs.description = cdd_test_mut_str("Op docs");

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root;
    JSON_Object *p_item;
    JSON_Object *op_obj;
    JSON_Object *ext;
    JSON_Object *paths;
    {
      JSON_Value *_cdd_jv_1 = NULL;
      cdd_json_parse_string(json, &_cdd_jv_1);
      root = _cdd_jv_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(json_value_get_object(root), "paths",
                                 &_cdd_jo_1);
      paths = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(paths, "/test/route", &_cdd_jo_1);
      p_item = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(p_item, "query", &_cdd_jo_1);
      op_obj = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(op_obj, "externalDocs", &_cdd_jo_1);
      ext = _cdd_jo_1;
    }

    ASSERT(op_obj != NULL);
    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(ext, "url", &_cdd_js_1);
      ASSERT_STR_EQ("https://example.com/op", _cdd_js_1);
    }
    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(ext, "description", &_cdd_js_1);
      ASSERT_STR_EQ("Op docs", _cdd_js_1);
    }

    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_OPENAPI_WRITER_SCHEMAS_H */
