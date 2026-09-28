/**
 * @file test_openapi_writer_basic.h
 * @brief Unit tests for OpenAPI Writer (basic).
 */

#ifndef TEST_OPENAPI_WRITER_BASIC_H
#define TEST_OPENAPI_WRITER_BASIC_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "emit/test_openapi_writer_common.h"
/* clang-format on */

TEST test_writer_empty_spec(void) {
  int rc;
  char *json;
  struct OpenAPI_Spec spec = {0};
  json = NULL;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);
  ASSERT(json != NULL);

  {
    JSON_Value *root;
    JSON_Object *obj;
    root = json_parse_string(json);
    obj = json_value_get_object(root);
    ASSERT_STR_EQ("3.2.0", json_object_get_string(obj, "openapi"));
    ASSERT(json_object_has_value(obj, "info"));
    ASSERT(json_object_has_value(obj, "paths"));
    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

TEST test_writer_basic_operation(void) {
  int rc;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Path path;
  char *json;
  struct OpenAPI_Operation op = {0};
  json = NULL;

  setup_test_spec(&spec, &path, &op, NULL, NULL);

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root;
    JSON_Object *p_item;
    JSON_Object *op_obj;
    JSON_Object *paths;
    root = json_parse_string(json);
    paths = json_object_get_object(json_value_get_object(root), "paths");
    p_item = json_object_get_object(paths, "/test/route");
    op_obj = json_object_get_object(p_item, "get");

    ASSERT(op_obj != NULL);
    ASSERT_STR_EQ("testOp", json_object_get_string(op_obj, "operationId"));

    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

TEST test_writer_schema_document(void) {
  int rc;
  char *_ast_strdup_0 = NULL;
  struct OpenAPI_Spec spec;
  char *json;
  _ast_strdup_0 = NULL;
  json = NULL;

  ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
  spec.is_schema_document = 1;
  spec.schema_root_json =
      (c_cdd_strdup("{\"type\":\"string\"}", &_ast_strdup_0), _ast_strdup_0);
  ASSERT(spec.schema_root_json != NULL);

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);
  ASSERT(json != NULL);
  ASSERT_STR_EQ("{\"type\":\"string\"}", json);

  free(json);
  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_writer_root_metadata_and_tags(void) {
  int rc;
  struct OpenAPI_Tag tags[1];
  char *json;
  struct OpenAPI_Spec spec = {0};
  json = NULL;

  memset(tags, 0, sizeof(tags));
  spec.openapi_version = (char *)(size_t)(size_t) "3.2.0";
  spec.self_uri = (char *)(size_t)(size_t) "https://example.com/openapi.json";
  spec.json_schema_dialect =
      (char *)(size_t)(size_t) "https://spec.openapis.org/oas/3.1/dialect/base";
  spec.external_docs.url = (char *)(size_t)(size_t) "https://example.com/docs";
  spec.external_docs.description = (char *)(size_t)(size_t) "Root docs";
  spec.tags = tags;
  spec.n_tags = 1;
  tags[0].name = (char *)(size_t)(size_t) "pets";
  tags[0].summary = (char *)(size_t)(size_t) "Pets";
  tags[0].description = (char *)(size_t)(size_t) "Pet ops";
  tags[0].parent = (char *)(size_t)(size_t) "animals";
  tags[0].kind = (char *)(size_t)(size_t) "nav";
  tags[0].external_docs.url =
      (char *)(size_t)(size_t) "https://example.com/tags/pets";
  tags[0].external_docs.description = (char *)(size_t)(size_t) "Tag docs";

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root;
    JSON_Object *obj;
    JSON_Object *ext;
    JSON_Array *tag_arr;
    JSON_Object *tag0;
    JSON_Object *tag_ext;
    root = json_parse_string(json);
    obj = json_value_get_object(root);
    ext = json_object_get_object(obj, "externalDocs");
    tag_arr = json_object_get_array(obj, "tags");
    tag0 = json_array_get_object(tag_arr, 0);
    tag_ext = json_object_get_object(tag0, "externalDocs");

    ASSERT_STR_EQ("https://example.com/openapi.json",
                  json_object_get_string(obj, "$self"));
    ASSERT_STR_EQ("https://spec.openapis.org/oas/3.1/dialect/base",
                  json_object_get_string(obj, "jsonSchemaDialect"));
    ASSERT_STR_EQ("https://example.com/docs",
                  json_object_get_string(ext, "url"));
    ASSERT_STR_EQ("Root docs", json_object_get_string(ext, "description"));
    ASSERT_STR_EQ("pets", json_object_get_string(tag0, "name"));
    ASSERT_STR_EQ("Pets", json_object_get_string(tag0, "summary"));
    ASSERT_STR_EQ("Pet ops", json_object_get_string(tag0, "description"));
    ASSERT_STR_EQ("animals", json_object_get_string(tag0, "parent"));
    ASSERT_STR_EQ("nav", json_object_get_string(tag0, "kind"));
    ASSERT_STR_EQ("https://example.com/tags/pets",
                  json_object_get_string(tag_ext, "url"));
    ASSERT_STR_EQ("Tag docs", json_object_get_string(tag_ext, "description"));

    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

TEST test_writer_path_ref_and_servers(void) {
  int rc;
  struct OpenAPI_Server path_servers[1];
  struct OpenAPI_Server op_servers[1];
  char *json;
  struct OpenAPI_Spec spec = {0};
  struct OpenAPI_Path path = {0};
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Response resp = {0};
  json = NULL;

  memset(path_servers, 0, sizeof(path_servers));
  memset(op_servers, 0, sizeof(op_servers));

  spec.paths = &path;
  spec.n_paths = 1;

  path.route = (char *)(size_t)(size_t) "/pets";
  path.ref = (char *)(size_t)(size_t) "#/components/pathItems/Pets";
  path.servers = path_servers;
  path.n_servers = 1;
  path_servers[0].url = (char *)(size_t)(size_t) "https://path.example.com";

  path.operations = &op;
  path.n_operations = 1;
  op.verb = OA_VERB_GET;
  op.operation_id = (char *)(size_t)(size_t) "listPets";
  op.responses = &resp;
  op.n_responses = 1;
  resp.code = (char *)(size_t)(size_t) "200";
  resp.description = (char *)(size_t)(size_t) "OK";
  op.servers = op_servers;
  op.n_servers = 1;
  op_servers[0].url = (char *)(size_t)(size_t) "https://op.example.com";

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root;
    JSON_Object *p_item;
    JSON_Array *p_servers;
    JSON_Object *p_srv0;
    JSON_Object *op_obj;
    JSON_Array *op_servers_arr;
    JSON_Object *op_srv0;
    JSON_Object *paths;
    root = json_parse_string(json);
    paths = json_object_get_object(json_value_get_object(root), "paths");
    p_item = json_object_get_object(paths, "/pets");
    p_servers = json_object_get_array(p_item, "servers");
    p_srv0 = json_array_get_object(p_servers, 0);
    op_obj = json_object_get_object(p_item, "get");
    op_servers_arr = json_object_get_array(op_obj, "servers");
    op_srv0 = json_array_get_object(op_servers_arr, 0);

    ASSERT_STR_EQ("#/components/pathItems/Pets",
                  json_object_get_string(p_item, "$ref"));
    ASSERT_STR_EQ("https://path.example.com",
                  json_object_get_string(p_srv0, "url"));
    ASSERT_STR_EQ("https://op.example.com",
                  json_object_get_string(op_srv0, "url"));

    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

TEST test_writer_webhooks(void) {
  int rc;
  char *json;
  struct OpenAPI_Spec spec = {0};
  struct OpenAPI_Path hook = {0};
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Response resp = {0};
  json = NULL;

  spec.webhooks = &hook;
  spec.n_webhooks = 1;

  hook.route = (char *)(size_t)(size_t) "petEvent";
  hook.operations = &op;
  hook.n_operations = 1;
  op.verb = OA_VERB_POST;
  op.operation_id = (char *)(size_t)(size_t) "onPetEvent";
  op.responses = &resp;
  op.n_responses = 1;
  resp.code = (char *)(size_t)(size_t) "200";
  resp.description = (char *)(size_t)(size_t) "OK";

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root;
    JSON_Object *hook_item;
    JSON_Object *op_obj;
    JSON_Object *hooks;
    root = json_parse_string(json);
    hooks = json_object_get_object(json_value_get_object(root), "webhooks");
    hook_item = json_object_get_object(hooks, "petEvent");
    op_obj = json_object_get_object(hook_item, "post");

    ASSERT(op_obj != NULL);
    ASSERT_STR_EQ("onPetEvent", json_object_get_string(op_obj, "operationId"));

    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

TEST test_writer_params_responses(void) {
  int rc;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Path path;
  char *json;
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Parameter param = {0};
  struct OpenAPI_Response resp = {0};
  json = NULL;

  setup_test_spec(&spec, &path, &op, &param, &resp);

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root;
    JSON_Object *op_obj;
    root = json_parse_string(json);
    op_obj = json_value_get_object(root);
    op_obj = json_object_get_object(op_obj, "paths");
    op_obj = json_object_get_object(op_obj, "/test/route");
    op_obj = json_object_get_object(op_obj, "get");

    /* Param Check */
    {
      JSON_Array *oa_params;
      JSON_Object *p_obj;
      oa_params = json_object_get_array(op_obj, "parameters");
      p_obj = json_array_get_object(oa_params, 0);
      ASSERT_STR_EQ("p1", json_object_get_string(p_obj, "name"));
      ASSERT_STR_EQ("query", json_object_get_string(p_obj, "in"));
      {
        JSON_Object *sch;
        sch = json_object_get_object(p_obj, "schema");
        ASSERT_STR_EQ("string", json_object_get_string(sch, "type"));
      }
    }

    /* Response Check */
    {
      JSON_Object *responses;
      JSON_Object *r200;
      JSON_Object *content;
      JSON_Object *media;
      JSON_Object *schema;
      responses = json_object_get_object(op_obj, "responses");
      r200 = json_object_get_object(responses, "200");
      content = json_object_get_object(r200, "content");
      media = json_object_get_object(content, "application/json");
      schema = json_object_get_object(media, "schema");

      ASSERT_STR_EQ("#/components/schemas/TestModel",
                    json_object_get_string(schema, "$ref"));
    }

    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

TEST test_writer_parameter_metadata(void) {
  int rc;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Path path;
  char *json;
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Parameter param = {0};
  json = NULL;

  setup_test_spec(&spec, &path, &op, &param, NULL);
  param.description = (char *)(size_t)(size_t) "Search term";
  param.deprecated_set = 1;
  param.deprecated = 1;
  param.allow_reserved_set = 1;
  param.allow_reserved = 1;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root;
    JSON_Array *oa_params;
    JSON_Object *p_obj;
    JSON_Object *op_obj;
    root = json_parse_string(json);
    op_obj = json_object_get_object(
        json_object_get_object(
            json_object_get_object(json_value_get_object(root), "paths"),
            "/test/route"),
        "get");
    oa_params = json_object_get_array(op_obj, "parameters");
    p_obj = json_array_get_object(oa_params, 0);

    ASSERT_STR_EQ("Search term", json_object_get_string(p_obj, "description"));
    ASSERT_EQ(1, json_object_get_boolean(p_obj, "deprecated"));
    ASSERT_EQ(1, json_object_get_boolean(p_obj, "allowReserved"));
    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

TEST test_writer_allow_empty_value(void) {
  int rc;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Path path;
  char *json;
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Parameter param = {0};
  json = NULL;

  setup_test_spec(&spec, &path, &op, &param, NULL);
  param.allow_empty_value_set = 1;
  param.allow_empty_value = 1;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root;
    JSON_Array *oa_params;
    JSON_Object *p_obj;
    JSON_Object *op_obj;
    root = json_parse_string(json);
    op_obj = json_object_get_object(
        json_object_get_object(
            json_object_get_object(json_value_get_object(root), "paths"),
            "/test/route"),
        "get");
    oa_params = json_object_get_array(op_obj, "parameters");
    p_obj = json_array_get_object(oa_params, 0);

    ASSERT_EQ(1, json_object_get_boolean(p_obj, "allowEmptyValue"));
    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

TEST test_writer_request_body_metadata_and_response_description(void) {
  int rc;
  struct OpenAPI_Spec spec;
  char *json;
  struct OpenAPI_Path path = {0};
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Response resp = {0};
  json = NULL;

  setup_test_spec(&spec, &path, &op, NULL, &resp);
  op.verb = OA_VERB_POST;
  op.req_body.ref_name = (char *)(size_t)(size_t) "User";
  op.req_body.content_type = (char *)(size_t)(size_t) "application/json";
  op.req_body_required_set = 1;
  op.req_body_required = 0;
  op.req_body_description = (char *)(size_t)(size_t) "Payload";
  resp.description = (char *)(size_t)(size_t) "Created";

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root;
    JSON_Object *op_obj;
    root = json_parse_string(json);
    op_obj = json_value_get_object(root);
    op_obj = json_object_get_object(op_obj, "paths");
    op_obj = json_object_get_object(op_obj, "/test/route");
    op_obj = json_object_get_object(op_obj, "post");

    {
      JSON_Object *rb;
      rb = json_object_get_object(op_obj, "requestBody");
      ASSERT_STR_EQ("Payload", json_object_get_string(rb, "description"));
      ASSERT_EQ(0, json_object_get_boolean(rb, "required"));
    }

    {
      JSON_Object *responses;
      JSON_Object *r200;
      responses = json_object_get_object(op_obj, "responses");
      r200 = json_object_get_object(responses, "200");
      ASSERT_STR_EQ("Created", json_object_get_string(r200, "description"));
    }

    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

TEST test_writer_info_metadata(void) {
  int rc;
  char *json;
  struct OpenAPI_Spec spec = {0};
  json = NULL;

  spec.info.title = (char *)(size_t)(size_t) "Example API";
  spec.info.summary = (char *)(size_t)(size_t) "Short";
  spec.info.description = (char *)(size_t)(size_t) "Long";
  spec.info.terms_of_service =
      (char *)(size_t)(size_t) "https://example.com/terms";
  spec.info.version = (char *)(size_t)(size_t) "2.1.0";
  spec.info.contact.name = (char *)(size_t)(size_t) "Support";
  spec.info.contact.url = (char *)(size_t)(size_t) "https://example.com";
  spec.info.contact.email = (char *)(size_t)(size_t) "support@example.com";
  spec.info.license.name = (char *)(size_t)(size_t) "Apache 2.0";
  spec.info.license.identifier = (char *)(size_t)(size_t) "Apache-2.0";

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);
  ASSERT(json != NULL);

  {
    JSON_Value *root;
    JSON_Object *contact;
    JSON_Object *license;
    JSON_Object *info;
    root = json_parse_string(json);
    info = json_object_get_object(json_value_get_object(root), "info");
    contact = json_object_get_object(info, "contact");
    license = json_object_get_object(info, "license");

    ASSERT_STR_EQ("Example API", json_object_get_string(info, "title"));
    ASSERT_STR_EQ("Short", json_object_get_string(info, "summary"));
    ASSERT_STR_EQ("Long", json_object_get_string(info, "description"));
    ASSERT_STR_EQ("https://example.com/terms",
                  json_object_get_string(info, "termsOfService"));
    ASSERT_STR_EQ("2.1.0", json_object_get_string(info, "version"));
    ASSERT_STR_EQ("Support", json_object_get_string(contact, "name"));
    ASSERT_STR_EQ("https://example.com",
                  json_object_get_string(contact, "url"));
    ASSERT_STR_EQ("support@example.com",
                  json_object_get_string(contact, "email"));
    ASSERT_STR_EQ("Apache 2.0", json_object_get_string(license, "name"));
    ASSERT_STR_EQ("Apache-2.0", json_object_get_string(license, "identifier"));
    ASSERT(json_object_get_string(license, "url") == NULL);

    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

TEST test_writer_info_license_identifier_and_url_rejected(void) {
  int rc;
  char *json;
  struct OpenAPI_Spec spec = {0};
  json = NULL;

  spec.info.title = (char *)(size_t)(size_t) "Example API";
  spec.info.version = (char *)(size_t)(size_t) "1.0";
  spec.info.license.name = (char *)(size_t)(size_t) "Apache 2.0";
  spec.info.license.identifier = (char *)(size_t)(size_t) "Apache-2.0";
  spec.info.license.url = (char *)(size_t)(size_t) "https://www.apache.org/"
                                                   "licenses/LICENSE-2.0.html";

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  ASSERT(json == NULL);
  g_fail_io_after = -1;
  PASS();
}

TEST test_writer_server_url_query_rejected(void) {
  int rc;
  char *json;
  struct OpenAPI_Spec spec = {0};
  struct OpenAPI_Server server = {0};
  json = NULL;

  spec.info.title = (char *)(size_t)(size_t) "Example API";
  spec.info.version = (char *)(size_t)(size_t) "1.0";
  server.url = (char *)(size_t)(size_t) "https://example.com/api?x=1";
  spec.servers = &server;
  spec.n_servers = 1;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  ASSERT(json == NULL);
  g_fail_io_after = -1;
  PASS();
}

TEST test_writer_operation_metadata(void) {
  int rc;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Path path;
  char *json;
  struct OpenAPI_Operation op = {0};
  json = NULL;

  setup_test_spec(&spec, &path, &op, NULL, NULL);
  op.summary = (char *)(size_t)(size_t) "Summary text";
  op.description = (char *)(size_t)(size_t) "Longer description";
  op.deprecated = 1;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root;
    JSON_Object *op_obj;
    root = json_parse_string(json);
    op_obj = json_object_get_object(json_value_get_object(root), "paths");
    op_obj = json_object_get_object(op_obj, "/test/route");
    op_obj = json_object_get_object(op_obj, "get");

    ASSERT_STR_EQ("Summary text", json_object_get_string(op_obj, "summary"));
    ASSERT_STR_EQ("Longer description",
                  json_object_get_string(op_obj, "description"));
    ASSERT_EQ(1, json_object_get_boolean(op_obj, "deprecated"));

    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

TEST test_writer_response_content_type(void) {
  int rc;
  struct OpenAPI_Spec spec;
  char *json;
  struct OpenAPI_Path path = {0};
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Response resp = {0};
  json = NULL;

  setup_test_spec(&spec, &path, &op, NULL, &resp);
  resp.content_type = (char *)(size_t)(size_t) "text/plain";
  resp.schema.ref_name = (char *)(size_t)(size_t) "Message";

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
    media = json_object_get_object(content, "text/plain");
    schema = json_object_get_object(media, "schema");

    ASSERT(schema != NULL);
    ASSERT_STR_EQ("#/components/schemas/Message",
                  json_object_get_string(schema, "$ref"));
    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_OPENAPI_WRITER_BASIC_H */
