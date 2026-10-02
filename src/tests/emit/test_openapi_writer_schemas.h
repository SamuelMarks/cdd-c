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

TEST test_writer_inline_response_schema_primitive(void) {
  int rc = 0;
  struct OpenAPI_Spec spec;
  char *json;
  struct OpenAPI_Path path = {0};
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Response resp = {0};
  json = NULL;

  setup_test_spec(&spec, &path, &op, NULL, &resp);
  resp.schema.inline_type = (char *)(size_t)(size_t) "string";

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root;
    JSON_Object *r200;
    JSON_Object *content;
    JSON_Object *media;
    JSON_Object *schema;
    JSON_Object *responses;
    root = json_parse_string(json);
    responses = json_object_get_object(
        json_object_get_object(
            json_object_get_object(json_value_get_object(root), "paths"),
            "/test/route"),
        "get");
    responses = json_object_get_object(responses, "responses");
    r200 = json_object_get_object(responses, "200");
    content = json_object_get_object(r200, "content");
    media = json_object_get_object(content, "application/json");
    schema = json_object_get_object(media, "schema");

    ASSERT_STR_EQ("string", json_object_get_string(schema, "type"));
    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

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
  resp.schema.inline_type = (char *)(size_t)(size_t) "integer";

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
    root = json_parse_string(json);
    responses = json_object_get_object(
        json_object_get_object(
            json_object_get_object(json_value_get_object(root), "paths"),
            "/test/route"),
        "get");
    responses = json_object_get_object(responses, "responses");
    r200 = json_object_get_object(responses, "200");
    content = json_object_get_object(r200, "content");
    media = json_object_get_object(content, "application/json");
    schema = json_object_get_object(media, "schema");
    items = json_object_get_object(schema, "items");

    ASSERT_STR_EQ("array", json_object_get_string(schema, "type"));
    ASSERT_STR_EQ("integer", json_object_get_string(items, "type"));
    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

TEST test_writer_inline_schema_format_and_content(void) {
  int rc = 0;
  struct OpenAPI_Spec spec;
  char *json;
  struct OpenAPI_Path path = {0};
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Response resp = {0};
  json = NULL;

  setup_test_spec(&spec, &path, &op, NULL, &resp);
  resp.schema.inline_type = (char *)(size_t)(size_t) "string";
  resp.schema.format = (char *)(size_t)(size_t) "uuid";
  resp.schema.content_media_type = (char *)(size_t)(size_t) "image/png";
  resp.schema.content_encoding = (char *)(size_t)(size_t) "base64";

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root;
    JSON_Object *r200;
    JSON_Object *content;
    JSON_Object *media;
    JSON_Object *schema;
    JSON_Object *responses;
    root = json_parse_string(json);
    responses = json_object_get_object(
        json_object_get_object(
            json_object_get_object(json_value_get_object(root), "paths"),
            "/test/route"),
        "get");
    responses = json_object_get_object(responses, "responses");
    r200 = json_object_get_object(responses, "200");
    content = json_object_get_object(r200, "content");
    media = json_object_get_object(content, "application/json");
    schema = json_object_get_object(media, "schema");

    ASSERT_STR_EQ("string", json_object_get_string(schema, "type"));
    ASSERT_STR_EQ("uuid", json_object_get_string(schema, "format"));
    ASSERT_STR_EQ("image/png",
                  json_object_get_string(schema, "contentMediaType"));
    ASSERT_STR_EQ("base64", json_object_get_string(schema, "contentEncoding"));
    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

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
  resp.schema.inline_type = (char *)(size_t)(size_t) "string";
  resp.schema.items_format = (char *)(size_t)(size_t) "uuid";
  resp.schema.items_content_media_type = (char *)(size_t)(size_t) "image/png";
  resp.schema.items_content_encoding = (char *)(size_t)(size_t) "base64";

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
    root = json_parse_string(json);
    responses = json_object_get_object(
        json_object_get_object(
            json_object_get_object(json_value_get_object(root), "paths"),
            "/test/route"),
        "get");
    responses = json_object_get_object(responses, "responses");
    r200 = json_object_get_object(responses, "200");
    content = json_object_get_object(r200, "content");
    media = json_object_get_object(content, "application/json");
    schema = json_object_get_object(media, "schema");
    items = json_object_get_object(schema, "items");

    ASSERT_STR_EQ("array", json_object_get_string(schema, "type"));
    ASSERT_STR_EQ("string", json_object_get_string(items, "type"));
    ASSERT_STR_EQ("uuid", json_object_get_string(items, "format"));
    ASSERT_STR_EQ("image/png",
                  json_object_get_string(items, "contentMediaType"));
    ASSERT_STR_EQ("base64", json_object_get_string(items, "contentEncoding"));
    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

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
  resp.schema.inline_type = (char *)(size_t)(size_t) "string";
  resp.schema.external_docs_set = 1;
  resp.schema.external_docs.url =
      (char *)(size_t)(size_t) "https://example.com/schema-doc";
  resp.schema.external_docs.description =
      (char *)(size_t)(size_t) "Schema external docs";
  resp.schema.discriminator_set = 1;
  resp.schema.discriminator.property_name = (char *)(size_t)(size_t) "kind";
  resp.schema.discriminator.default_mapping =
      (char *)(size_t)(size_t) "#/components/schemas/Base";
  map.value = (char *)(size_t)(size_t) "cat";
  map.schema = (char *)(size_t)(size_t) "#/components/schemas/Cat";
  resp.schema.discriminator.mapping = &map;
  resp.schema.discriminator.n_mapping = 1;
  resp.schema.xml_set = 1;
  resp.schema.xml.node_type_set = 1;
  resp.schema.xml.node_type = OA_XML_NODE_ATTRIBUTE;
  resp.schema.xml.name = (char *)(size_t)(size_t) "id";
  resp.schema.xml.namespace_uri =
      (char *)(size_t)(size_t) "https://example.com/ns";
  resp.schema.xml.prefix = (char *)(size_t)(size_t) "p";

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
    root = json_parse_string(json);
    paths = json_object_get_object(json_value_get_object(root), "paths");
    p_item = json_object_get_object(paths, "/test/route");
    op_obj = json_object_get_object(p_item, "get");
    responses = json_object_get_object(op_obj, "responses");
    r200 = json_object_get_object(responses, "200");
    content = json_object_get_object(r200, "content");
    media = json_object_get_object(content, "application/json");
    schema = json_object_get_object(media, "schema");
    ext = json_object_get_object(schema, "externalDocs");
    disc = json_object_get_object(schema, "discriminator");
    mapping = json_object_get_object(disc, "mapping");
    xml = json_object_get_object(schema, "xml");

    ASSERT(ext != NULL);
    ASSERT_STR_EQ("https://example.com/schema-doc",
                  json_object_get_string(ext, "url"));
    ASSERT_STR_EQ("Schema external docs",
                  json_object_get_string(ext, "description"));
    ASSERT(disc != NULL);
    ASSERT_STR_EQ("kind", json_object_get_string(disc, "propertyName"));
    ASSERT_STR_EQ("#/components/schemas/Base",
                  json_object_get_string(disc, "defaultMapping"));
    ASSERT_STR_EQ("#/components/schemas/Cat",
                  json_object_get_string(mapping, "cat"));
    ASSERT(xml != NULL);
    ASSERT_STR_EQ("attribute", json_object_get_string(xml, "nodeType"));
    ASSERT_STR_EQ("id", json_object_get_string(xml, "name"));
    ASSERT_STR_EQ("https://example.com/ns",
                  json_object_get_string(xml, "namespace"));
    ASSERT_STR_EQ("p", json_object_get_string(xml, "prefix"));
    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

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
  examples[0].string = (char *)(size_t)(size_t) "fast";
  examples[1].type = OA_ANY_STRING;
  examples[1].string = (char *)(size_t)(size_t) "slow";

  setup_test_spec(&spec, &path, &op, NULL, &resp);
  resp.schema.inline_type = (char *)(size_t)(size_t) "string";
  resp.schema.description = (char *)(size_t)(size_t) "Mode";
  resp.schema.deprecated_set = 1;
  resp.schema.deprecated = 1;
  resp.schema.read_only_set = 1;
  resp.schema.read_only = 1;
  resp.schema.write_only_set = 1;
  resp.schema.write_only = 0;
  resp.schema.const_value_set = 1;
  resp.schema.const_value.type = OA_ANY_STRING;
  resp.schema.const_value.string = (char *)(size_t)(size_t) "fast";
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
    root = json_parse_string(json);
    responses = json_object_get_object(
        json_object_get_object(
            json_object_get_object(json_value_get_object(root), "paths"),
            "/test/route"),
        "get");
    responses = json_object_get_object(responses, "responses");
    r200 = json_object_get_object(responses, "200");
    content = json_object_get_object(r200, "content");
    media = json_object_get_object(content, "application/json");
    schema = json_object_get_object(media, "schema");
    examples_arr = json_object_get_array(schema, "examples");

    ASSERT_STR_EQ("fast", json_object_get_string(schema, "const"));
    ASSERT_STR_EQ("Mode", json_object_get_string(schema, "description"));
    ASSERT_EQ(1, json_object_get_boolean(schema, "deprecated"));
    ASSERT_EQ(1, json_object_get_boolean(schema, "readOnly"));
    ASSERT_EQ(0, json_object_get_boolean(schema, "writeOnly"));
    ASSERT_EQ(2, json_array_get_count(examples_arr));
    ASSERT_STR_EQ("fast", json_array_get_string(examples_arr, 0));
    ASSERT_STR_EQ("slow", json_array_get_string(examples_arr, 1));
    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

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
    root = json_parse_string(out_json);
    root_obj = json_value_get_object(root);
    comps = json_object_get_object(root_obj, "components");
    schemas = json_object_get_object(comps, "schemas");
    pet = json_object_get_object(schemas, "Pet");
    one_of = json_object_get_array(pet, "oneOf");
    ASSERT(one_of != NULL);
    ASSERT_EQ(2, json_array_get_count(one_of));
    json_value_free(root);
  }

  free(out_json);
  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

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
    root = json_parse_string(out_json);
    root_obj = json_value_get_object(root);
    comps = json_object_get_object(root_obj, "components");
    schemas = json_object_get_object(comps, "schemas");
    inline_schema =
        json_object_get_object(schemas, "Inline_GetPets_Response_200");
    ASSERT(inline_schema != NULL);
    one_of = json_object_get_array(inline_schema, "oneOf");
    ASSERT(one_of != NULL);
    ASSERT_EQ(2, json_array_get_count(one_of));
    json_value_free(root);
  }

  free(out_json);
  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_writer_schema_ref_summary_description(void) {
  int rc = 0;
  struct OpenAPI_Spec spec;
  char *json;
  struct OpenAPI_Path path = {0};
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Response resp = {0};
  json = NULL;

  setup_test_spec(&spec, &path, &op, NULL, &resp);
  resp.schema.ref_name = (char *)(size_t)(size_t) "Mode";
  resp.schema.summary = (char *)(size_t)(size_t) "Mode summary";
  resp.schema.description = (char *)(size_t)(size_t) "Mode description";

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root;
    JSON_Object *r200;
    JSON_Object *content;
    JSON_Object *media;
    JSON_Object *schema;
    JSON_Object *responses;
    root = json_parse_string(json);
    responses = json_object_get_object(
        json_object_get_object(
            json_object_get_object(json_value_get_object(root), "paths"),
            "/test/route"),
        "get");
    responses = json_object_get_object(responses, "responses");
    r200 = json_object_get_object(responses, "200");
    content = json_object_get_object(r200, "content");
    media = json_object_get_object(content, "application/json");
    schema = json_object_get_object(media, "schema");

    ASSERT_STR_EQ("#/components/schemas/Mode",
                  json_object_get_string(schema, "$ref"));
    ASSERT_STR_EQ("Mode summary", json_object_get_string(schema, "summary"));
    ASSERT_STR_EQ("Mode description",
                  json_object_get_string(schema, "description"));
    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

TEST test_writer_info_license_missing_name_rejected(void) {
  int rc = 0;
  char *json;
  struct OpenAPI_Spec spec = {0};
  json = NULL;

  spec.info.title = (char *)(size_t)(size_t) "Example";
  spec.info.version = (char *)(size_t)(size_t) "1.0";
  spec.info.license.identifier = (char *)(size_t)(size_t) "Apache-2.0";

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  ASSERT(json == NULL);
  g_fail_io_after = -1;
  PASS();
}

TEST test_writer_options_trace_verbs(void) {
  int rc = 0;
  struct OpenAPI_Path path;
  struct OpenAPI_Operation ops[2];
  char *json;
  struct OpenAPI_Spec spec = {0};
  json = NULL;

  memset(&path, 0, sizeof(path));
  memset(ops, 0, sizeof(ops));
  path.route = (char *)(size_t)(size_t) "/verbs";
  path.operations = ops;
  path.n_operations = 2;
  ops[0].verb = OA_VERB_OPTIONS;
  ops[0].operation_id = (char *)(size_t)(size_t) "opt";
  ops[1].verb = OA_VERB_TRACE;
  ops[1].operation_id = (char *)(size_t)(size_t) "tr";
  spec.paths = &path;
  spec.n_paths = 1;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root;
    JSON_Object *verbs;
    root = json_parse_string(json);
    verbs = json_object_get_object(
        json_object_get_object(json_value_get_object(root), "paths"), "/verbs");
    ASSERT(json_object_get_object(verbs, "options") != NULL);
    ASSERT(json_object_get_object(verbs, "trace") != NULL);
    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

TEST test_writer_query_and_external_docs(void) {
  int rc = 0;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Path path;
  char *json;
  struct OpenAPI_Operation op = {0};
  json = NULL;

  setup_test_spec(&spec, &path, &op, NULL, NULL);
  op.verb = OA_VERB_QUERY;
  op.operation_id = (char *)(size_t)(size_t) "querySearch";
  op.external_docs.url = (char *)(size_t)(size_t) "https://example.com/op";
  op.external_docs.description = (char *)(size_t)(size_t) "Op docs";

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root;
    JSON_Object *p_item;
    JSON_Object *op_obj;
    JSON_Object *ext;
    JSON_Object *paths;
    root = json_parse_string(json);
    paths = json_object_get_object(json_value_get_object(root), "paths");
    p_item = json_object_get_object(paths, "/test/route");
    op_obj = json_object_get_object(p_item, "query");
    ext = json_object_get_object(op_obj, "externalDocs");

    ASSERT(op_obj != NULL);
    ASSERT_STR_EQ("https://example.com/op", json_object_get_string(ext, "url"));
    ASSERT_STR_EQ("Op docs", json_object_get_string(ext, "description"));

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
