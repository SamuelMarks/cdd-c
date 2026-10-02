/**
 * @file test_openapi_writer_components.h
 * @brief Unit tests for OpenAPI Writer (components).
 */

#ifndef TEST_OPENAPI_WRITER_COMPONENTS_H
#define TEST_OPENAPI_WRITER_COMPONENTS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "emit/test_openapi_writer_common.h"
/* clang-format on */

TEST test_writer_components_schemas_raw(void) {
  int rc = 0;
  char *json;
  struct OpenAPI_Spec spec = {0};
  char *names[3] = {(char *)(size_t)(size_t) "Token",
                    (char *)(size_t)(size_t) "Flag",
                    (char *)(size_t)(size_t) "Nums"};
  char *raw[3] = {(char *)(size_t)(size_t) "{\"type\":\"string\"}",
                  (char *)(size_t)(size_t) "true",
                  (char *)(size_t)(size_t) "{\"type\":\"array\",\"items\":{"
                                           "\"type\":\"integer\"}}"};
  json = NULL;

  spec.raw_schema_names = names;
  spec.raw_schema_json = raw;
  spec.n_raw_schemas = 3;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root;
    JSON_Object *schemas;
    JSON_Object *token;
    JSON_Value *flag_val;
    JSON_Object *nums;
    JSON_Object *items;
    JSON_Object *comps;
    root = json_parse_string(json);
    comps = json_object_get_object(json_value_get_object(root), "components");
    schemas = json_object_get_object(comps, "schemas");
    token = json_object_get_object(schemas, "Token");
    flag_val = json_object_get_value(schemas, "Flag");
    nums = json_object_get_object(schemas, "Nums");
    items = json_object_get_object(nums, "items");

    ASSERT_STR_EQ("string", json_object_get_string(token, "type"));
    ASSERT_EQ(JSONBoolean, json_value_get_type(flag_val));
    ASSERT_EQ(1, json_value_get_boolean(flag_val));
    ASSERT_STR_EQ("array", json_object_get_string(nums, "type"));
    ASSERT_STR_EQ("integer", json_object_get_string(items, "type"));

    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

TEST test_writer_schema_ref_external(void) {
  int rc = 0;
  struct OpenAPI_Spec spec;
  char *json;
  struct OpenAPI_Path path = {0};
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Response resp = {0};
  json = NULL;

  setup_test_spec(&spec, &path, &op, NULL, &resp);
  resp.schema.ref_name = NULL;
  resp.schema.ref = (char *)(size_t)(size_t) "https://example.com/schemas/Pet";

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root;
    JSON_Object *p_item;
    JSON_Object *get;
    JSON_Object *responses;
    JSON_Object *resp_obj;
    JSON_Object *content;
    JSON_Object *media;
    JSON_Object *schema;
    JSON_Object *paths;
    root = json_parse_string(json);
    paths = json_object_get_object(json_value_get_object(root), "paths");
    p_item = json_object_get_object(paths, "/test/route");
    get = json_object_get_object(p_item, "get");
    responses = json_object_get_object(get, "responses");
    resp_obj = json_object_get_object(responses, "200");
    content = json_object_get_object(resp_obj, "content");
    media = json_object_get_object(content, "application/json");
    schema = json_object_get_object(media, "schema");

    ASSERT_STR_EQ("https://example.com/schemas/Pet",
                  json_object_get_string(schema, "$ref"));

    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

TEST test_writer_schema_dynamic_ref_external(void) {
  int rc = 0;
  struct OpenAPI_Spec spec;
  char *json;
  struct OpenAPI_Path path = {0};
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Response resp = {0};
  json = NULL;

  setup_test_spec(&spec, &path, &op, NULL, &resp);
  resp.schema.ref_name = NULL;
  resp.schema.ref = (char *)(size_t)(size_t) "https://example.com/schemas/Pet";
  resp.schema.ref_is_dynamic = 1;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root;
    JSON_Object *p_item;
    JSON_Object *get;
    JSON_Object *responses;
    JSON_Object *resp_obj;
    JSON_Object *content;
    JSON_Object *media;
    JSON_Object *schema;
    JSON_Object *paths;
    root = json_parse_string(json);
    paths = json_object_get_object(json_value_get_object(root), "paths");
    p_item = json_object_get_object(paths, "/test/route");
    get = json_object_get_object(p_item, "get");
    responses = json_object_get_object(get, "responses");
    resp_obj = json_object_get_object(responses, "200");
    content = json_object_get_object(resp_obj, "content");
    media = json_object_get_object(content, "application/json");
    schema = json_object_get_object(media, "schema");

    ASSERT_STR_EQ("https://example.com/schemas/Pet",
                  json_object_get_string(schema, "$dynamicRef"));

    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

TEST test_writer_schema_items_ref_external(void) {
  int rc = 0;
  struct OpenAPI_Spec spec;
  char *json;
  struct OpenAPI_Path path = {0};
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Response resp = {0};
  json = NULL;

  setup_test_spec(&spec, &path, &op, NULL, &resp);
  resp.schema.ref_name = NULL;
  resp.schema.is_array = 1;
  resp.schema.items_ref =
      (char *)(size_t)(size_t) "https://example.com/schemas/Pet";

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root;
    JSON_Object *p_item;
    JSON_Object *get;
    JSON_Object *responses;
    JSON_Object *resp_obj;
    JSON_Object *content;
    JSON_Object *media;
    JSON_Object *schema;
    JSON_Object *items;
    JSON_Object *paths;
    root = json_parse_string(json);
    paths = json_object_get_object(json_value_get_object(root), "paths");
    p_item = json_object_get_object(paths, "/test/route");
    get = json_object_get_object(p_item, "get");
    responses = json_object_get_object(get, "responses");
    resp_obj = json_object_get_object(responses, "200");
    content = json_object_get_object(resp_obj, "content");
    media = json_object_get_object(content, "application/json");
    schema = json_object_get_object(media, "schema");
    items = json_object_get_object(schema, "items");

    ASSERT_STR_EQ("array", json_object_get_string(schema, "type"));
    ASSERT_STR_EQ("https://example.com/schemas/Pet",
                  json_object_get_string(items, "$ref"));

    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

TEST test_writer_schema_items_dynamic_ref_external(void) {
  int rc = 0;
  struct OpenAPI_Spec spec;
  char *json;
  struct OpenAPI_Path path = {0};
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Response resp = {0};
  json = NULL;

  setup_test_spec(&spec, &path, &op, NULL, &resp);
  resp.schema.ref_name = NULL;
  resp.schema.is_array = 1;
  resp.schema.items_ref =
      (char *)(size_t)(size_t) "https://example.com/schemas/Pet";
  resp.schema.items_ref_is_dynamic = 1;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root;
    JSON_Object *p_item;
    JSON_Object *get;
    JSON_Object *responses;
    JSON_Object *resp_obj;
    JSON_Object *content;
    JSON_Object *media;
    JSON_Object *schema;
    JSON_Object *items;
    JSON_Object *paths;
    root = json_parse_string(json);
    paths = json_object_get_object(json_value_get_object(root), "paths");
    p_item = json_object_get_object(paths, "/test/route");
    get = json_object_get_object(p_item, "get");
    responses = json_object_get_object(get, "responses");
    resp_obj = json_object_get_object(responses, "200");
    content = json_object_get_object(resp_obj, "content");
    media = json_object_get_object(content, "application/json");
    schema = json_object_get_object(media, "schema");
    items = json_object_get_object(schema, "items");

    ASSERT_STR_EQ("array", json_object_get_string(schema, "type"));
    ASSERT_STR_EQ("https://example.com/schemas/Pet",
                  json_object_get_string(items, "$dynamicRef"));

    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

TEST test_writer_additional_operations(void) {
  int rc = 0;
  char *json;
  struct OpenAPI_Spec spec = {0};
  struct OpenAPI_Path path = {0};
  struct OpenAPI_Operation add_op = {0};
  struct OpenAPI_Response resp = {0};
  json = NULL;

  spec.paths = &path;
  spec.n_paths = 1;
  path.route = (char *)(size_t)(size_t) "/copy";
  path.additional_operations = &add_op;
  path.n_additional_operations = 1;

  add_op.method = (char *)(size_t)(size_t) "COPY";
  add_op.is_additional = 1;
  add_op.operation_id = (char *)(size_t)(size_t) "copyItem";
  add_op.responses = &resp;
  add_op.n_responses = 1;

  resp.code = (char *)(size_t)(size_t) "200";
  resp.description = (char *)(size_t)(size_t) "ok";

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root;
    JSON_Object *p_item;
    JSON_Object *copy_op;
    JSON_Object *paths;
    JSON_Object *add_ops;
    root = json_parse_string(json);
    paths = json_object_get_object(json_value_get_object(root), "paths");
    p_item = json_object_get_object(paths, "/copy");
    add_ops = json_object_get_object(p_item, "additionalOperations");
    copy_op = json_object_get_object(add_ops, "COPY");

    ASSERT(copy_op != NULL);
    ASSERT_STR_EQ("copyItem", json_object_get_string(copy_op, "operationId"));

    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

TEST test_writer_component_media_types_and_content_ref(void) {
  int rc = 0;
  char *media_names[1];
  char *json;
  struct OpenAPI_Spec spec = {0};
  struct OpenAPI_Path path = {0};
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Response resp = {0};
  struct OpenAPI_MediaType mt = {0};
  json = NULL;

  media_names[0] = (char *)(size_t)(size_t) "application/vnd.acme+json";
  spec.component_media_types = &mt;
  spec.component_media_type_names = media_names;
  spec.n_component_media_types = 1;

  mt.name = (char *)(size_t)(size_t) "application/vnd.acme+json";
  mt.schema_set = 1;
  mt.schema.ref_name = (char *)(size_t)(size_t) "Pet";

  spec.paths = &path;
  spec.n_paths = 1;
  path.route = (char *)(size_t)(size_t) "/pets";
  path.operations = &op;
  path.n_operations = 1;
  op.verb = OA_VERB_GET;
  op.operation_id = (char *)(size_t)(size_t) "getPet";
  op.responses = &resp;
  op.n_responses = 1;

  resp.code = (char *)(size_t)(size_t) "200";
  resp.description = (char *)(size_t)(size_t) "ok";
  resp.content_type = (char *)(size_t)(size_t) "application/vnd.acme+json";
  resp.content_ref = (char *)(size_t)(size_t) "#/components/mediaTypes/"
                                              "application~1vnd.acme+json";

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root;
    JSON_Object *obj;
    JSON_Object *comps;
    JSON_Object *media;
    JSON_Object *schema_obj;
    JSON_Object *paths;
    JSON_Object *p_item;
    JSON_Object *get_op;
    JSON_Object *responses;
    JSON_Object *resp_obj;
    JSON_Object *content;
    JSON_Object *mt_obj;
    JSON_Object *mt_content;
    root = json_parse_string(json);
    obj = json_value_get_object(root);
    comps = json_object_get_object(obj, "components");
    media = json_object_get_object(comps, "mediaTypes");
    mt_obj = json_object_get_object(media, "application/vnd.acme+json");
    schema_obj = json_object_get_object(mt_obj, "schema");
    paths = json_object_get_object(obj, "paths");
    p_item = json_object_get_object(paths, "/pets");
    get_op = json_object_get_object(p_item, "get");
    responses = json_object_get_object(get_op, "responses");
    resp_obj = json_object_get_object(responses, "200");
    content = json_object_get_object(resp_obj, "content");
    mt_content = json_object_get_object(content, "application/vnd.acme+json");

    ASSERT_STR_EQ("#/components/schemas/Pet",
                  json_object_get_string(schema_obj, "$ref"));
    ASSERT_STR_EQ("#/components/mediaTypes/application~1vnd.acme+json",
                  json_object_get_string(mt_content, "$ref"));

    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

TEST test_writer_response_multiple_content(void) {
  int rc = 0;
  struct OpenAPI_MediaType contents[2];
  char *json;
  struct OpenAPI_Spec spec = {0};
  struct OpenAPI_Path path = {0};
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Response resp = {0};
  json = NULL;

  memset(contents, 0, sizeof(contents));

  setup_test_spec(&spec, &path, &op, NULL, &resp);
  resp.description = (char *)(size_t)(size_t) "ok";

  contents[0].name = (char *)(size_t)(size_t) "application/json";
  contents[0].schema_set = 1;
  contents[0].schema.ref_name = (char *)(size_t)(size_t) "TestModel";
  contents[1].name = (char *)(size_t)(size_t) "text/plain";
  contents[1].schema_set = 1;
  contents[1].schema.inline_type = (char *)(size_t)(size_t) "string";

  resp.content_media_types = contents;
  resp.n_content_media_types = 2;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root;
    JSON_Object *p_item;
    JSON_Object *op_obj;
    JSON_Object *responses;
    JSON_Object *r200;
    JSON_Object *content;
    JSON_Object *paths;
    root = json_parse_string(json);
    paths = json_object_get_object(json_value_get_object(root), "paths");
    p_item = json_object_get_object(paths, "/test/route");
    op_obj = json_object_get_object(p_item, "get");
    responses = json_object_get_object(op_obj, "responses");
    r200 = json_object_get_object(responses, "200");
    content = json_object_get_object(r200, "content");

    ASSERT(content != NULL);
    ASSERT(json_object_get_object(content, "application/json") != NULL);
    ASSERT(json_object_get_object(content, "text/plain") != NULL);

    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

TEST test_writer_request_body_multiple_content_and_encoding(void) {
  int rc = 0;
  struct OpenAPI_MediaType media[1];
  struct OpenAPI_Encoding enc[1];
  char *json;
  struct OpenAPI_Spec spec = {0};
  struct OpenAPI_Path path = {0};
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Header enc_hdr = {0};
  json = NULL;

  memset(media, 0, sizeof(media));
  memset(enc, 0, sizeof(enc));

  setup_test_spec(&spec, &path, &op, NULL, NULL);

  enc_hdr.name = (char *)(size_t)(size_t) "X-Rate-Limit-Limit";
  enc_hdr.type = (char *)(size_t)(size_t) "integer";

  enc[0].name = (char *)(size_t)(size_t) "file";
  enc[0].content_type = (char *)(size_t)(size_t) "image/png";
  enc[0].explode_set = 1;
  enc[0].explode = 1;
  enc[0].allow_reserved_set = 1;
  enc[0].allow_reserved = 1;
  enc[0].headers = &enc_hdr;
  enc[0].n_headers = 1;

  media[0].name = (char *)(size_t)(size_t) "multipart/form-data";
  media[0].schema_set = 1;
  media[0].schema.inline_type = (char *)(size_t)(size_t) "object";
  media[0].encoding = enc;
  media[0].n_encoding = 1;

  op.req_body_media_types = media;
  op.n_req_body_media_types = 1;
  op.req_body_required_set = 1;
  op.req_body_required = 1;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root;
    JSON_Object *p_item;
    JSON_Object *op_obj;
    JSON_Object *rb;
    JSON_Object *content;
    JSON_Object *mt;
    JSON_Object *encoding;
    JSON_Object *file_enc;
    JSON_Object *headers;
    JSON_Object *hdr;
    JSON_Object *hdr_schema;
    JSON_Object *paths;
    root = json_parse_string(json);
    paths = json_object_get_object(json_value_get_object(root), "paths");
    p_item = json_object_get_object(paths, "/test/route");
    op_obj = json_object_get_object(p_item, "get");
    rb = json_object_get_object(op_obj, "requestBody");
    content = json_object_get_object(rb, "content");
    mt = json_object_get_object(content, "multipart/form-data");
    encoding = json_object_get_object(mt, "encoding");
    file_enc = json_object_get_object(encoding, "file");
    headers = json_object_get_object(file_enc, "headers");
    hdr = json_object_get_object(headers, "X-Rate-Limit-Limit");
    hdr_schema = json_object_get_object(hdr, "schema");

    ASSERT_STR_EQ("image/png", json_object_get_string(file_enc, "contentType"));
    ASSERT_EQ(1, json_object_get_boolean(file_enc, "explode"));
    ASSERT_EQ(1, json_object_get_boolean(file_enc, "allowReserved"));
    ASSERT_STR_EQ("integer", json_object_get_string(hdr_schema, "type"));

    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

TEST test_writer_media_type_prefix_item_encoding(void) {
  int rc = 0;
  struct OpenAPI_MediaType media[1];
  char *media_names[1];
  struct OpenAPI_Encoding prefix[2];
  struct OpenAPI_Encoding nested[1];
  char *json;
  struct OpenAPI_Spec spec = {0};
  struct OpenAPI_Encoding item = {0};
  struct OpenAPI_Header prefix_hdr = {0};
  json = NULL;

  memset(media, 0, sizeof(media));
  memset(prefix, 0, sizeof(prefix));
  memset(nested, 0, sizeof(nested));

  media_names[0] = (char *)(size_t)(size_t) "multipart/mixed";
  spec.component_media_type_names = media_names;
  media[0].name = (char *)(size_t)(size_t) "multipart/mixed";
  media[0].schema_set = 1;
  media[0].schema.inline_type = (char *)(size_t)(size_t) "array";

  prefix[0].content_type = (char *)(size_t)(size_t) "application/json";
  prefix_hdr.name = (char *)(size_t)(size_t) "X-Pos";
  prefix_hdr.type = (char *)(size_t)(size_t) "string";
  prefix[1].content_type = (char *)(size_t)(size_t) "image/png";
  prefix[1].headers = &prefix_hdr;
  prefix[1].n_headers = 1;

  media[0].prefix_encoding = prefix;
  media[0].n_prefix_encoding = 2;

  nested[0].name = (char *)(size_t)(size_t) "meta";
  nested[0].content_type = (char *)(size_t)(size_t) "text/plain";
  item.content_type = (char *)(size_t)(size_t) "application/octet-stream";
  item.encoding = nested;
  item.n_encoding = 1;

  media[0].item_encoding = &item;
  media[0].item_encoding_set = 1;

  spec.component_media_types = media;
  spec.n_component_media_types = 1;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root;
    JSON_Object *media_types;
    JSON_Object *mt;
    JSON_Array *prefix_arr;
    JSON_Object *prefix0;
    JSON_Object *prefix1;
    JSON_Object *prefix1_headers;
    JSON_Object *prefix1_hdr;
    JSON_Object *prefix1_schema;
    JSON_Object *item_obj;
    JSON_Object *item_encoding;
    JSON_Object *meta_obj;
    JSON_Object *components;
    root = json_parse_string(json);
    components =
        json_object_get_object(json_value_get_object(root), "components");
    media_types = json_object_get_object(components, "mediaTypes");
    mt = json_object_get_object(media_types, "multipart/mixed");
    prefix_arr = json_object_get_array(mt, "prefixEncoding");
    prefix0 = json_array_get_object(prefix_arr, 0);
    prefix1 = json_array_get_object(prefix_arr, 1);
    prefix1_headers = json_object_get_object(prefix1, "headers");
    prefix1_hdr = json_object_get_object(prefix1_headers, "X-Pos");
    prefix1_schema = json_object_get_object(prefix1_hdr, "schema");
    item_obj = json_object_get_object(mt, "itemEncoding");
    item_encoding = json_object_get_object(item_obj, "encoding");
    meta_obj = json_object_get_object(item_encoding, "meta");

    ASSERT_EQ(2, json_array_get_count(prefix_arr));
    ASSERT_STR_EQ("application/json",
                  json_object_get_string(prefix0, "contentType"));
    ASSERT_STR_EQ("image/png", json_object_get_string(prefix1, "contentType"));
    ASSERT_STR_EQ("string", json_object_get_string(prefix1_schema, "type"));

    ASSERT_STR_EQ("application/octet-stream",
                  json_object_get_string(item_obj, "contentType"));
    ASSERT_STR_EQ("text/plain",
                  json_object_get_string(meta_obj, "contentType"));

    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_OPENAPI_WRITER_COMPONENTS_H */
