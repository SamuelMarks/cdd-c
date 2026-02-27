/**
 * @file test_openapi_writer.h
 * @brief Unit tests for OpenAPI Writer module.
 *
 * Verifies that in-memory structures are correctly serialized to valid
 * JSON strings matching the OpenAPI 3.2 specification structure.
 * Includes coverage for Advanced features: Security Schemes, Parameter Styles,
 * and Multipart/Recursive schemes.
 *
 * @author Samuel Marks
 */

#ifndef TEST_OPENAPI_WRITER_H
#define TEST_OPENAPI_WRITER_H

#include <greatest.h>
#include <parson.h>
#include <stdlib.h>
#include <string.h>

#include "functions/parse_str.h"
#include "openapi/emit_openapi.h"
#include "openapi/parse_openapi.h"

/* --- Helpers --- */

static int load_spec_str2(const char *json_str, struct OpenAPI_Spec *spec) {
  JSON_Value *dyn = json_parse_string(json_str);
  int rc;
  if (!dyn)
    return -1;
  openapi_spec_init(spec);
  rc = openapi_load_from_json(dyn, spec);
  json_value_free(dyn);
  return rc;
}

static void setup_test_spec(struct OpenAPI_Spec *spec,
                            struct OpenAPI_Path *path,
                            struct OpenAPI_Operation *op,
                            struct OpenAPI_Parameter *param,
                            struct OpenAPI_Response *response) {
  /* Zero out stack structs */
  memset(spec, 0, sizeof(*spec));
  if (path)
    memset(path, 0, sizeof(*path));
  if (op)
    memset(op, 0, sizeof(*op));
  if (param)
    memset(param, 0, sizeof(*param));
  if (response)
    memset(response, 0, sizeof(*response));

  if (path) {
    spec->paths = path;
    spec->n_paths = 1;
    path->route = "/test/route";
    if (op) {
      path->operations = op;
      path->n_operations = 1;
      op->verb = OA_VERB_GET;
      op->operation_id = "testOp";
      if (param) {
        op->parameters = param;
        op->n_parameters = 1;
        param->name = "p1";
        param->in = OA_PARAM_IN_QUERY;
        param->type = "string";
      }
      if (response) {
        op->responses = response;
        op->n_responses = 1;
        response->code = "200";
        response->schema.ref_name = "TestModel";
      }
    }
  }
}

/* --- Tests --- */

TEST test_writer_empty_spec(void) {
  struct OpenAPI_Spec spec = {0};
  char *json = NULL;
  int rc;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);
  ASSERT(json != NULL);

  {
    JSON_Value *root = json_parse_string(json);
    JSON_Object *obj = json_value_get_object(root);
    ASSERT_STR_EQ("3.2.0", json_object_get_string(obj, "openapi"));
    ASSERT(json_object_has_value(obj, "info"));
    ASSERT(json_object_has_value(obj, "paths"));
    json_value_free(root);
  }

  free(json);
  PASS();
}

TEST test_writer_basic_operation(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Path path;
  struct OpenAPI_Operation op;
  char *json = NULL;
  int rc;

  setup_test_spec(&spec, &path, &op, NULL, NULL);

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root = json_parse_string(json);
    JSON_Object *paths =
        json_object_get_object(json_value_get_object(root), "paths");
    JSON_Object *p_item = json_object_get_object(paths, "/test/route");
    JSON_Object *op_obj = json_object_get_object(p_item, "get");

    ASSERT(op_obj != NULL);
    ASSERT_STR_EQ("testOp", json_object_get_string(op_obj, "operationId"));

    json_value_free(root);
  }

  free(json);
  PASS();
}

TEST test_writer_schema_document(void) {
  struct OpenAPI_Spec spec;
  char *json = NULL;
  int rc;

  openapi_spec_init(&spec);
  spec.is_schema_document = 1;
  spec.schema_root_json = c_cdd_strdup("{\"type\":\"string\"}");
  ASSERT(spec.schema_root_json != NULL);

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);
  ASSERT(json != NULL);
  ASSERT_STR_EQ("{\"type\":\"string\"}", json);

  free(json);
  openapi_spec_free(&spec);
  PASS();
}

TEST test_writer_root_metadata_and_tags(void) {
  struct OpenAPI_Spec spec = {0};
  struct OpenAPI_Tag tags[1];
  char *json = NULL;
  int rc;

  memset(tags, 0, sizeof(tags));
  spec.openapi_version = "3.2.0";
  spec.self_uri = "https://example.com/openapi.json";
  spec.json_schema_dialect = "https://spec.openapis.org/oas/3.1/dialect/base";
  spec.external_docs.url = "https://example.com/docs";
  spec.external_docs.description = "Root docs";
  spec.tags = tags;
  spec.n_tags = 1;
  tags[0].name = "pets";
  tags[0].summary = "Pets";
  tags[0].description = "Pet ops";
  tags[0].parent = "animals";
  tags[0].kind = "nav";
  tags[0].external_docs.url = "https://example.com/tags/pets";
  tags[0].external_docs.description = "Tag docs";

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root = json_parse_string(json);
    JSON_Object *obj = json_value_get_object(root);
    JSON_Object *ext = json_object_get_object(obj, "externalDocs");
    JSON_Array *tag_arr = json_object_get_array(obj, "tags");
    JSON_Object *tag0 = json_array_get_object(tag_arr, 0);
    JSON_Object *tag_ext = json_object_get_object(tag0, "externalDocs");

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
  PASS();
}

TEST test_writer_path_ref_and_servers(void) {
  struct OpenAPI_Spec spec = {0};
  struct OpenAPI_Path path = {0};
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Response resp = {0};
  struct OpenAPI_Server path_servers[1];
  struct OpenAPI_Server op_servers[1];
  char *json = NULL;
  int rc;

  memset(path_servers, 0, sizeof(path_servers));
  memset(op_servers, 0, sizeof(op_servers));

  spec.paths = &path;
  spec.n_paths = 1;

  path.route = "/pets";
  path.ref = "#/components/pathItems/Pets";
  path.servers = path_servers;
  path.n_servers = 1;
  path_servers[0].url = "https://path.example.com";

  path.operations = &op;
  path.n_operations = 1;
  op.verb = OA_VERB_GET;
  op.operation_id = "listPets";
  op.responses = &resp;
  op.n_responses = 1;
  resp.code = "200";
  resp.description = "OK";
  op.servers = op_servers;
  op.n_servers = 1;
  op_servers[0].url = "https://op.example.com";

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root = json_parse_string(json);
    JSON_Object *paths =
        json_object_get_object(json_value_get_object(root), "paths");
    JSON_Object *p_item = json_object_get_object(paths, "/pets");
    JSON_Array *p_servers = json_object_get_array(p_item, "servers");
    JSON_Object *p_srv0 = json_array_get_object(p_servers, 0);
    JSON_Object *op_obj = json_object_get_object(p_item, "get");
    JSON_Array *op_servers_arr = json_object_get_array(op_obj, "servers");
    JSON_Object *op_srv0 = json_array_get_object(op_servers_arr, 0);

    ASSERT_STR_EQ("#/components/pathItems/Pets",
                  json_object_get_string(p_item, "$ref"));
    ASSERT_STR_EQ("https://path.example.com",
                  json_object_get_string(p_srv0, "url"));
    ASSERT_STR_EQ("https://op.example.com",
                  json_object_get_string(op_srv0, "url"));

    json_value_free(root);
  }

  free(json);
  PASS();
}

TEST test_writer_webhooks(void) {
  struct OpenAPI_Spec spec = {0};
  struct OpenAPI_Path hook = {0};
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Response resp = {0};
  char *json = NULL;
  int rc;

  spec.webhooks = &hook;
  spec.n_webhooks = 1;

  hook.route = "petEvent";
  hook.operations = &op;
  hook.n_operations = 1;
  op.verb = OA_VERB_POST;
  op.operation_id = "onPetEvent";
  op.responses = &resp;
  op.n_responses = 1;
  resp.code = "200";
  resp.description = "OK";

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root = json_parse_string(json);
    JSON_Object *hooks =
        json_object_get_object(json_value_get_object(root), "webhooks");
    JSON_Object *hook_item = json_object_get_object(hooks, "petEvent");
    JSON_Object *op_obj = json_object_get_object(hook_item, "post");

    ASSERT(op_obj != NULL);
    ASSERT_STR_EQ("onPetEvent", json_object_get_string(op_obj, "operationId"));

    json_value_free(root);
  }

  free(json);
  PASS();
}

TEST test_writer_params_responses(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Path path;
  struct OpenAPI_Operation op;
  struct OpenAPI_Parameter param;
  struct OpenAPI_Response resp;
  char *json = NULL;
  int rc;

  setup_test_spec(&spec, &path, &op, &param, &resp);

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root = json_parse_string(json);
    JSON_Object *op_obj = json_value_get_object(root);
    op_obj = json_object_get_object(op_obj, "paths");
    op_obj = json_object_get_object(op_obj, "/test/route");
    op_obj = json_object_get_object(op_obj, "get");

    /* Param Check */
    {
      JSON_Array *oa_params = json_object_get_array(op_obj, "parameters");
      JSON_Object *p_obj = json_array_get_object(oa_params, 0);
      ASSERT_STR_EQ("p1", json_object_get_string(p_obj, "name"));
      ASSERT_STR_EQ("query", json_object_get_string(p_obj, "in"));
      {
        JSON_Object *sch = json_object_get_object(p_obj, "schema");
        ASSERT_STR_EQ("string", json_object_get_string(sch, "type"));
      }
    }

    /* Response Check */
    {
      JSON_Object *responses = json_object_get_object(op_obj, "responses");
      JSON_Object *r200 = json_object_get_object(responses, "200");
      JSON_Object *content = json_object_get_object(r200, "content");
      JSON_Object *media = json_object_get_object(content, "application/json");
      JSON_Object *schema = json_object_get_object(media, "schema");

      ASSERT_STR_EQ("#/components/schemas/TestModel",
                    json_object_get_string(schema, "$ref"));
    }

    json_value_free(root);
  }

  free(json);
  PASS();
}

TEST test_writer_parameter_metadata(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Path path;
  struct OpenAPI_Operation op;
  struct OpenAPI_Parameter param;
  char *json = NULL;
  int rc;

  setup_test_spec(&spec, &path, &op, &param, NULL);
  param.description = "Search term";
  param.deprecated_set = 1;
  param.deprecated = 1;
  param.allow_reserved_set = 1;
  param.allow_reserved = 1;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root = json_parse_string(json);
    JSON_Object *op_obj = json_object_get_object(
        json_object_get_object(
            json_object_get_object(json_value_get_object(root), "paths"),
            "/test/route"),
        "get");
    JSON_Array *oa_params = json_object_get_array(op_obj, "parameters");
    JSON_Object *p_obj = json_array_get_object(oa_params, 0);

    ASSERT_STR_EQ("Search term", json_object_get_string(p_obj, "description"));
    ASSERT_EQ(1, json_object_get_boolean(p_obj, "deprecated"));
    ASSERT_EQ(1, json_object_get_boolean(p_obj, "allowReserved"));
    json_value_free(root);
  }

  free(json);
  PASS();
}

TEST test_writer_allow_empty_value(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Path path;
  struct OpenAPI_Operation op;
  struct OpenAPI_Parameter param;
  char *json = NULL;
  int rc;

  setup_test_spec(&spec, &path, &op, &param, NULL);
  param.allow_empty_value_set = 1;
  param.allow_empty_value = 1;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root = json_parse_string(json);
    JSON_Object *op_obj = json_object_get_object(
        json_object_get_object(
            json_object_get_object(json_value_get_object(root), "paths"),
            "/test/route"),
        "get");
    JSON_Array *oa_params = json_object_get_array(op_obj, "parameters");
    JSON_Object *p_obj = json_array_get_object(oa_params, 0);

    ASSERT_EQ(1, json_object_get_boolean(p_obj, "allowEmptyValue"));
    json_value_free(root);
  }

  free(json);
  PASS();
}

TEST test_writer_request_body_metadata_and_response_description(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Path path;
  struct OpenAPI_Operation op;
  struct OpenAPI_Response resp;
  char *json = NULL;
  int rc;

  setup_test_spec(&spec, &path, &op, NULL, &resp);
  op.verb = OA_VERB_POST;
  op.req_body.ref_name = "User";
  op.req_body.content_type = "application/json";
  op.req_body_required_set = 1;
  op.req_body_required = 0;
  op.req_body_description = "Payload";
  resp.description = "Created";

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root = json_parse_string(json);
    JSON_Object *op_obj = json_value_get_object(root);
    op_obj = json_object_get_object(op_obj, "paths");
    op_obj = json_object_get_object(op_obj, "/test/route");
    op_obj = json_object_get_object(op_obj, "post");

    {
      JSON_Object *rb = json_object_get_object(op_obj, "requestBody");
      ASSERT_STR_EQ("Payload", json_object_get_string(rb, "description"));
      ASSERT_EQ(0, json_object_get_boolean(rb, "required"));
    }

    {
      JSON_Object *responses = json_object_get_object(op_obj, "responses");
      JSON_Object *r200 = json_object_get_object(responses, "200");
      ASSERT_STR_EQ("Created", json_object_get_string(r200, "description"));
    }

    json_value_free(root);
  }

  free(json);
  PASS();
}

TEST test_writer_info_metadata(void) {
  struct OpenAPI_Spec spec = {0};
  char *json = NULL;
  int rc;

  spec.info.title = "Example API";
  spec.info.summary = "Short";
  spec.info.description = "Long";
  spec.info.terms_of_service = "https://example.com/terms";
  spec.info.version = "2.1.0";
  spec.info.contact.name = "Support";
  spec.info.contact.url = "https://example.com";
  spec.info.contact.email = "support@example.com";
  spec.info.license.name = "Apache 2.0";
  spec.info.license.identifier = "Apache-2.0";

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);
  ASSERT(json != NULL);

  {
    JSON_Value *root = json_parse_string(json);
    JSON_Object *info =
        json_object_get_object(json_value_get_object(root), "info");
    JSON_Object *contact = json_object_get_object(info, "contact");
    JSON_Object *license = json_object_get_object(info, "license");

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
  PASS();
}

TEST test_writer_info_license_identifier_and_url_rejected(void) {
  struct OpenAPI_Spec spec = {0};
  char *json = NULL;
  int rc;

  spec.info.title = "Example API";
  spec.info.version = "1.0";
  spec.info.license.name = "Apache 2.0";
  spec.info.license.identifier = "Apache-2.0";
  spec.info.license.url = "https://www.apache.org/licenses/LICENSE-2.0.html";

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(EINVAL, rc);
  ASSERT(json == NULL);
  PASS();
}

TEST test_writer_server_url_query_rejected(void) {
  struct OpenAPI_Spec spec = {0};
  struct OpenAPI_Server server = {0};
  char *json = NULL;
  int rc;

  spec.info.title = "Example API";
  spec.info.version = "1.0";
  server.url = "https://example.com/api?x=1";
  spec.servers = &server;
  spec.n_servers = 1;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(EINVAL, rc);
  ASSERT(json == NULL);
  PASS();
}

TEST test_writer_operation_metadata(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Path path;
  struct OpenAPI_Operation op;
  char *json = NULL;
  int rc;

  setup_test_spec(&spec, &path, &op, NULL, NULL);
  op.summary = "Summary text";
  op.description = "Longer description";
  op.deprecated = 1;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root = json_parse_string(json);
    JSON_Object *op_obj =
        json_object_get_object(json_value_get_object(root), "paths");
    op_obj = json_object_get_object(op_obj, "/test/route");
    op_obj = json_object_get_object(op_obj, "get");

    ASSERT_STR_EQ("Summary text", json_object_get_string(op_obj, "summary"));
    ASSERT_STR_EQ("Longer description",
                  json_object_get_string(op_obj, "description"));
    ASSERT_EQ(1, json_object_get_boolean(op_obj, "deprecated"));

    json_value_free(root);
  }

  free(json);
  PASS();
}

TEST test_writer_response_content_type(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Path path;
  struct OpenAPI_Operation op;
  struct OpenAPI_Response resp;
  char *json = NULL;
  int rc;

  setup_test_spec(&spec, &path, &op, NULL, &resp);
  resp.content_type = "text/plain";
  resp.schema.ref_name = "Message";

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root = json_parse_string(json);
    JSON_Object *responses = json_object_get_object(
        json_object_get_object(
            json_object_get_object(json_value_get_object(root), "paths"),
            "/test/route"),
        "get");
    responses = json_object_get_object(responses, "responses");
    JSON_Object *r200 = json_object_get_object(responses, "200");
    JSON_Object *content = json_object_get_object(r200, "content");
    JSON_Object *media = json_object_get_object(content, "text/plain");
    JSON_Object *schema = json_object_get_object(media, "schema");

    ASSERT(schema != NULL);
    ASSERT_STR_EQ("#/components/schemas/Message",
                  json_object_get_string(schema, "$ref"));
    json_value_free(root);
  }

  free(json);
  PASS();
}

TEST test_writer_inline_response_schema_primitive(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Path path;
  struct OpenAPI_Operation op;
  struct OpenAPI_Response resp;
  char *json = NULL;
  int rc;

  setup_test_spec(&spec, &path, &op, NULL, &resp);
  resp.schema.inline_type = "string";

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root = json_parse_string(json);
    JSON_Object *responses = json_object_get_object(
        json_object_get_object(
            json_object_get_object(json_value_get_object(root), "paths"),
            "/test/route"),
        "get");
    responses = json_object_get_object(responses, "responses");
    JSON_Object *r200 = json_object_get_object(responses, "200");
    JSON_Object *content = json_object_get_object(r200, "content");
    JSON_Object *media = json_object_get_object(content, "application/json");
    JSON_Object *schema = json_object_get_object(media, "schema");

    ASSERT_STR_EQ("string", json_object_get_string(schema, "type"));
    json_value_free(root);
  }

  free(json);
  PASS();
}

TEST test_writer_inline_response_schema_array(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Path path;
  struct OpenAPI_Operation op;
  struct OpenAPI_Response resp;
  char *json = NULL;
  int rc;

  setup_test_spec(&spec, &path, &op, NULL, &resp);
  resp.schema.is_array = 1;
  resp.schema.inline_type = "integer";

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root = json_parse_string(json);
    JSON_Object *responses = json_object_get_object(
        json_object_get_object(
            json_object_get_object(json_value_get_object(root), "paths"),
            "/test/route"),
        "get");
    responses = json_object_get_object(responses, "responses");
    JSON_Object *r200 = json_object_get_object(responses, "200");
    JSON_Object *content = json_object_get_object(r200, "content");
    JSON_Object *media = json_object_get_object(content, "application/json");
    JSON_Object *schema = json_object_get_object(media, "schema");
    JSON_Object *items = json_object_get_object(schema, "items");

    ASSERT_STR_EQ("array", json_object_get_string(schema, "type"));
    ASSERT_STR_EQ("integer", json_object_get_string(items, "type"));
    json_value_free(root);
  }

  free(json);
  PASS();
}

TEST test_writer_inline_schema_format_and_content(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Path path;
  struct OpenAPI_Operation op;
  struct OpenAPI_Response resp;
  char *json = NULL;
  int rc;

  setup_test_spec(&spec, &path, &op, NULL, &resp);
  resp.schema.inline_type = "string";
  resp.schema.format = "uuid";
  resp.schema.content_media_type = "image/png";
  resp.schema.content_encoding = "base64";

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root = json_parse_string(json);
    JSON_Object *responses = json_object_get_object(
        json_object_get_object(
            json_object_get_object(json_value_get_object(root), "paths"),
            "/test/route"),
        "get");
    responses = json_object_get_object(responses, "responses");
    JSON_Object *r200 = json_object_get_object(responses, "200");
    JSON_Object *content = json_object_get_object(r200, "content");
    JSON_Object *media = json_object_get_object(content, "application/json");
    JSON_Object *schema = json_object_get_object(media, "schema");

    ASSERT_STR_EQ("string", json_object_get_string(schema, "type"));
    ASSERT_STR_EQ("uuid", json_object_get_string(schema, "format"));
    ASSERT_STR_EQ("image/png",
                  json_object_get_string(schema, "contentMediaType"));
    ASSERT_STR_EQ("base64", json_object_get_string(schema, "contentEncoding"));
    json_value_free(root);
  }

  free(json);
  PASS();
}

TEST test_writer_inline_schema_array_item_format_and_content(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Path path;
  struct OpenAPI_Operation op;
  struct OpenAPI_Response resp;
  char *json = NULL;
  int rc;

  setup_test_spec(&spec, &path, &op, NULL, &resp);
  resp.schema.is_array = 1;
  resp.schema.inline_type = "string";
  resp.schema.items_format = "uuid";
  resp.schema.items_content_media_type = "image/png";
  resp.schema.items_content_encoding = "base64";

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root = json_parse_string(json);
    JSON_Object *responses = json_object_get_object(
        json_object_get_object(
            json_object_get_object(json_value_get_object(root), "paths"),
            "/test/route"),
        "get");
    responses = json_object_get_object(responses, "responses");
    JSON_Object *r200 = json_object_get_object(responses, "200");
    JSON_Object *content = json_object_get_object(r200, "content");
    JSON_Object *media = json_object_get_object(content, "application/json");
    JSON_Object *schema = json_object_get_object(media, "schema");
    JSON_Object *items = json_object_get_object(schema, "items");

    ASSERT_STR_EQ("array", json_object_get_string(schema, "type"));
    ASSERT_STR_EQ("string", json_object_get_string(items, "type"));
    ASSERT_STR_EQ("uuid", json_object_get_string(items, "format"));
    ASSERT_STR_EQ("image/png",
                  json_object_get_string(items, "contentMediaType"));
    ASSERT_STR_EQ("base64", json_object_get_string(items, "contentEncoding"));
    json_value_free(root);
  }

  free(json);
  PASS();
}

TEST test_writer_schema_external_docs_discriminator_xml(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Path path;
  struct OpenAPI_Operation op;
  struct OpenAPI_Response resp;
  struct OpenAPI_DiscriminatorMap map;
  char *json = NULL;
  int rc;

  setup_test_spec(&spec, &path, &op, NULL, &resp);

  resp.schema.ref_name = NULL;
  resp.schema.inline_type = "string";
  resp.schema.external_docs_set = 1;
  resp.schema.external_docs.url = "https://example.com/schema-doc";
  resp.schema.external_docs.description = "Schema external docs";
  resp.schema.discriminator_set = 1;
  resp.schema.discriminator.property_name = "kind";
  resp.schema.discriminator.default_mapping = "#/components/schemas/Base";
  map.value = "cat";
  map.schema = "#/components/schemas/Cat";
  resp.schema.discriminator.mapping = &map;
  resp.schema.discriminator.n_mapping = 1;
  resp.schema.xml_set = 1;
  resp.schema.xml.node_type_set = 1;
  resp.schema.xml.node_type = OA_XML_NODE_ATTRIBUTE;
  resp.schema.xml.name = "id";
  resp.schema.xml.namespace_uri = "https://example.com/ns";
  resp.schema.xml.prefix = "p";

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);
  ASSERT(json != NULL);

  {
    JSON_Value *root = json_parse_string(json);
    JSON_Object *paths =
        json_object_get_object(json_value_get_object(root), "paths");
    JSON_Object *p_item = json_object_get_object(paths, "/test/route");
    JSON_Object *op_obj = json_object_get_object(p_item, "get");
    JSON_Object *responses = json_object_get_object(op_obj, "responses");
    JSON_Object *r200 = json_object_get_object(responses, "200");
    JSON_Object *content = json_object_get_object(r200, "content");
    JSON_Object *media = json_object_get_object(content, "application/json");
    JSON_Object *schema = json_object_get_object(media, "schema");
    JSON_Object *ext = json_object_get_object(schema, "externalDocs");
    JSON_Object *disc = json_object_get_object(schema, "discriminator");
    JSON_Object *mapping = json_object_get_object(disc, "mapping");
    JSON_Object *xml = json_object_get_object(schema, "xml");

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
  PASS();
}

TEST test_writer_inline_schema_const_examples_annotations(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Path path;
  struct OpenAPI_Operation op;
  struct OpenAPI_Response resp;
  struct OpenAPI_Any examples[2];
  char *json = NULL;
  int rc;

  memset(examples, 0, sizeof(examples));
  examples[0].type = OA_ANY_STRING;
  examples[0].string = "fast";
  examples[1].type = OA_ANY_STRING;
  examples[1].string = "slow";

  setup_test_spec(&spec, &path, &op, NULL, &resp);
  resp.schema.inline_type = "string";
  resp.schema.description = "Mode";
  resp.schema.deprecated_set = 1;
  resp.schema.deprecated = 1;
  resp.schema.read_only_set = 1;
  resp.schema.read_only = 1;
  resp.schema.write_only_set = 1;
  resp.schema.write_only = 0;
  resp.schema.const_value_set = 1;
  resp.schema.const_value.type = OA_ANY_STRING;
  resp.schema.const_value.string = "fast";
  resp.schema.examples = examples;
  resp.schema.n_examples = 2;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root = json_parse_string(json);
    JSON_Object *responses = json_object_get_object(
        json_object_get_object(
            json_object_get_object(json_value_get_object(root), "paths"),
            "/test/route"),
        "get");
    responses = json_object_get_object(responses, "responses");
    JSON_Object *r200 = json_object_get_object(responses, "200");
    JSON_Object *content = json_object_get_object(r200, "content");
    JSON_Object *media = json_object_get_object(content, "application/json");
    JSON_Object *schema = json_object_get_object(media, "schema");
    JSON_Array *examples_arr = json_object_get_array(schema, "examples");

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
  PASS();
}

TEST test_writer_preserves_composed_component_schema(void) {
  const char *json =
      "{"
      "\"openapi\":\"3.2.0\","
      "\"info\":{\"title\":\"Spec\",\"version\":\"1\",\"license\":{\"name\":"
      "\"MIT\"}},"
      "\"components\":{"
      "\"schemas\":{"
      "\"Pet\":{"
      "\"oneOf\":["
      "{\"type\":\"object\",\"properties\":{\"id\":{\"type\":\"integer\"}}},"
      "{\"type\":\"object\",\"properties\":{\"name\":{\"type\":\"string\"}}}"
      "]"
      "}"
      "}"
      "}"
      "}";

  struct OpenAPI_Spec spec;
  char *out_json = NULL;
  int rc;

  rc = load_spec_str2(json, &spec);
  ASSERT_EQ(0, rc);

  rc = openapi_write_spec_to_json(&spec, &out_json);
  ASSERT_EQ(0, rc);
  ASSERT(out_json != NULL);

  {
    JSON_Value *root = json_parse_string(out_json);
    JSON_Object *root_obj = json_value_get_object(root);
    JSON_Object *comps = json_object_get_object(root_obj, "components");
    JSON_Object *schemas = json_object_get_object(comps, "schemas");
    JSON_Object *pet = json_object_get_object(schemas, "Pet");
    JSON_Array *one_of = json_object_get_array(pet, "oneOf");
    ASSERT(one_of != NULL);
    ASSERT_EQ(2, json_array_get_count(one_of));
    json_value_free(root);
  }

  free(out_json);
  openapi_spec_free(&spec);
  PASS();
}

TEST test_writer_preserves_inline_composed_schema(void) {
  SKIPm("fix me");

  const char *json =
      "{"
      "\"openapi\":\"3.2.0\","
      "\"info\":{\"title\":\"Spec\",\"version\":\"1\",\"license\":{\"name\":"
      "\"MIT\"}},"
      "\"paths\":{"
      "\"/pets\":{"
      "\"get\":{"
      "\"operationId\":\"GetPets\","
      "\"responses\":{"
      "\"200\":{"
      "\"description\":\"ok\","
      "\"content\":{"
      "\"application/json\":{"
      "\"schema\":{"
      "\"oneOf\":["
      "{\"type\":\"object\",\"properties\":{\"id\":{\"type\":\"integer\"}}},"
      "{\"type\":\"object\",\"properties\":{\"name\":{\"type\":\"string\"}}}"
      "]"
      "}"
      "}"
      "}"
      "}"
      "}"
      "}"
      "}"
      "}";

  struct OpenAPI_Spec spec;
  char *out_json = NULL;
  int rc;

  rc = load_spec_str2(json, &spec);
  ASSERT_EQ(0, rc);

  rc = openapi_write_spec_to_json(&spec, &out_json);
  ASSERT_EQ(0, rc);
  ASSERT(out_json != NULL);

  {
    JSON_Value *root = json_parse_string(out_json);
    JSON_Object *root_obj = json_value_get_object(root);
    JSON_Object *comps = json_object_get_object(root_obj, "components");
    JSON_Object *schemas = json_object_get_object(comps, "schemas");
    JSON_Object *inline_schema =
        json_object_get_object(schemas, "Inline_GetPets_Response_200");
    JSON_Array *one_of =
        inline_schema ? json_object_get_array(inline_schema, "oneOf") : NULL;
    ASSERT(inline_schema != NULL);
    ASSERT(one_of != NULL);
    ASSERT_EQ(2, json_array_get_count(one_of));
    json_value_free(root);
  }

  free(out_json);
  openapi_spec_free(&spec);
  PASS();
}

TEST test_writer_schema_ref_summary_description(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Path path;
  struct OpenAPI_Operation op;
  struct OpenAPI_Response resp;
  char *json = NULL;
  int rc;

  setup_test_spec(&spec, &path, &op, NULL, &resp);
  resp.schema.ref_name = "Mode";
  resp.schema.summary = "Mode summary";
  resp.schema.description = "Mode description";

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root = json_parse_string(json);
    JSON_Object *responses = json_object_get_object(
        json_object_get_object(
            json_object_get_object(json_value_get_object(root), "paths"),
            "/test/route"),
        "get");
    responses = json_object_get_object(responses, "responses");
    JSON_Object *r200 = json_object_get_object(responses, "200");
    JSON_Object *content = json_object_get_object(r200, "content");
    JSON_Object *media = json_object_get_object(content, "application/json");
    JSON_Object *schema = json_object_get_object(media, "schema");

    ASSERT_STR_EQ("#/components/schemas/Mode",
                  json_object_get_string(schema, "$ref"));
    ASSERT_STR_EQ("Mode summary", json_object_get_string(schema, "summary"));
    ASSERT_STR_EQ("Mode description",
                  json_object_get_string(schema, "description"));
    json_value_free(root);
  }

  free(json);
  PASS();
}

TEST test_writer_info_license_missing_name_rejected(void) {
  struct OpenAPI_Spec spec = {0};
  char *json = NULL;
  int rc;

  spec.info.title = "Example";
  spec.info.version = "1.0";
  spec.info.license.identifier = "Apache-2.0";

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(EINVAL, rc);
  ASSERT(json == NULL);
  PASS();
}

TEST test_writer_options_trace_verbs(void) {
  struct OpenAPI_Spec spec = {0};
  struct OpenAPI_Path path;
  struct OpenAPI_Operation ops[2];
  char *json = NULL;
  int rc;

  memset(&path, 0, sizeof(path));
  memset(ops, 0, sizeof(ops));
  path.route = "/verbs";
  path.operations = ops;
  path.n_operations = 2;
  ops[0].verb = OA_VERB_OPTIONS;
  ops[0].operation_id = "opt";
  ops[1].verb = OA_VERB_TRACE;
  ops[1].operation_id = "tr";
  spec.paths = &path;
  spec.n_paths = 1;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root = json_parse_string(json);
    JSON_Object *verbs = json_object_get_object(
        json_object_get_object(json_value_get_object(root), "paths"), "/verbs");
    ASSERT(json_object_get_object(verbs, "options") != NULL);
    ASSERT(json_object_get_object(verbs, "trace") != NULL);
    json_value_free(root);
  }

  free(json);
  PASS();
}

TEST test_writer_query_and_external_docs(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Path path;
  struct OpenAPI_Operation op;
  char *json = NULL;
  int rc;

  setup_test_spec(&spec, &path, &op, NULL, NULL);
  op.verb = OA_VERB_QUERY;
  op.operation_id = "querySearch";
  op.external_docs.url = "https://example.com/op";
  op.external_docs.description = "Op docs";

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root = json_parse_string(json);
    JSON_Object *paths =
        json_object_get_object(json_value_get_object(root), "paths");
    JSON_Object *p_item = json_object_get_object(paths, "/test/route");
    JSON_Object *op_obj = json_object_get_object(p_item, "query");
    JSON_Object *ext = json_object_get_object(op_obj, "externalDocs");

    ASSERT(op_obj != NULL);
    ASSERT_STR_EQ("https://example.com/op", json_object_get_string(ext, "url"));
    ASSERT_STR_EQ("Op docs", json_object_get_string(ext, "description"));

    json_value_free(root);
  }

  free(json);
  PASS();
}

TEST test_writer_parameter_styles(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Path path;
  struct OpenAPI_Operation op;
  struct OpenAPI_Parameter param;
  char *json = NULL;
  int rc;

  setup_test_spec(&spec, &path, &op, &param, NULL);
  /* Configure Advanced Params */
  param.in = OA_PARAM_IN_QUERY;
  param.style = OA_STYLE_FORM;
  param.explode = 1;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root = json_parse_string(json);
    JSON_Object *op_obj = json_value_get_object(root);
    op_obj = json_object_get_object(op_obj, "paths");
    op_obj = json_object_get_object(op_obj, "/test/route");
    op_obj = json_object_get_object(op_obj, "get");

    {
      JSON_Array *oa_params = json_object_get_array(op_obj, "parameters");
      JSON_Object *p_obj = json_array_get_object(oa_params, 0);
      ASSERT_STR_EQ("form", json_object_get_string(p_obj, "style"));
      ASSERT_EQ(1, json_object_get_boolean(p_obj, "explode"));
    }
    json_value_free(root);
  }
  free(json);
  PASS();
}

TEST test_writer_parameter_explode_false(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Path path;
  struct OpenAPI_Operation op;
  struct OpenAPI_Parameter param;
  char *json = NULL;
  int rc;

  setup_test_spec(&spec, &path, &op, &param, NULL);
  param.in = OA_PARAM_IN_QUERY;
  param.style = OA_STYLE_FORM;
  param.explode_set = 1;
  param.explode = 0;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root = json_parse_string(json);
    JSON_Object *op_obj = json_value_get_object(root);
    op_obj = json_object_get_object(op_obj, "paths");
    op_obj = json_object_get_object(op_obj, "/test/route");
    op_obj = json_object_get_object(op_obj, "get");

    {
      JSON_Array *oa_params = json_object_get_array(op_obj, "parameters");
      JSON_Object *p_obj = json_array_get_object(oa_params, 0);
      ASSERT(json_object_has_value(p_obj, "explode"));
      ASSERT_EQ(0, json_object_get_boolean(p_obj, "explode"));
    }
    json_value_free(root);
  }

  free(json);
  PASS();
}

TEST test_writer_parameter_style_matrix(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Path path;
  struct OpenAPI_Operation op;
  struct OpenAPI_Parameter param;
  char *json = NULL;
  int rc;

  setup_test_spec(&spec, &path, &op, &param, NULL);
  param.in = OA_PARAM_IN_PATH;
  param.style = OA_STYLE_MATRIX;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root = json_parse_string(json);
    JSON_Object *op_obj = json_value_get_object(root);
    op_obj = json_object_get_object(op_obj, "paths");
    op_obj = json_object_get_object(op_obj, "/test/route");
    op_obj = json_object_get_object(op_obj, "get");

    {
      JSON_Array *oa_params = json_object_get_array(op_obj, "parameters");
      JSON_Object *p_obj = json_array_get_object(oa_params, 0);
      ASSERT_STR_EQ("matrix", json_object_get_string(p_obj, "style"));
    }
    json_value_free(root);
  }
  free(json);
  PASS();
}

TEST test_writer_parameter_content_any(void) {
  struct OpenAPI_Spec spec = {0};
  struct OpenAPI_Path path = {0};
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Parameter param = {0};
  char *json = NULL;
  int rc;

  spec.paths = &path;
  spec.n_paths = 1;
  path.route = "/headers";
  path.operations = &op;
  path.n_operations = 1;
  op.verb = OA_VERB_GET;
  op.operation_id = "getHeader";
  op.parameters = &param;
  op.n_parameters = 1;
  param.name = "X-Foo";
  param.in = OA_PARAM_IN_HEADER;
  param.type = "string";
  param.content_type = "text/plain";

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root = json_parse_string(json);
    JSON_Object *op_obj = json_value_get_object(root);
    op_obj = json_object_get_object(op_obj, "paths");
    op_obj = json_object_get_object(op_obj, "/headers");
    op_obj = json_object_get_object(op_obj, "get");
    {
      JSON_Array *oa_params = json_object_get_array(op_obj, "parameters");
      JSON_Object *p_obj = json_array_get_object(oa_params, 0);
      JSON_Object *content = json_object_get_object(p_obj, "content");
      JSON_Object *media = json_object_get_object(content, "text/plain");
      JSON_Object *schema = json_object_get_object(media, "schema");
      ASSERT_STR_EQ("header", json_object_get_string(p_obj, "in"));
      ASSERT(json_object_get_object(p_obj, "schema") == NULL);
      ASSERT_STR_EQ("string", json_object_get_string(schema, "type"));
    }

    json_value_free(root);
  }

  free(json);
  PASS();
}

TEST test_writer_parameter_and_header_content_media_type(void) {
  struct OpenAPI_Spec spec = {0};
  struct OpenAPI_Path path = {0};
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Parameter param = {0};
  struct OpenAPI_Response resp = {0};
  struct OpenAPI_Header header = {0};
  struct OpenAPI_MediaType param_media = {0};
  struct OpenAPI_MediaType header_media = {0};
  struct OpenAPI_Encoding enc = {0};
  char *json = NULL;
  int rc;

  spec.paths = &path;
  spec.n_paths = 1;
  path.route = "/content";
  path.operations = &op;
  path.n_operations = 1;
  op.verb = OA_VERB_GET;
  op.operation_id = "getContent";
  op.parameters = &param;
  op.n_parameters = 1;
  param.name = "filter";
  param.in = OA_PARAM_IN_QUERY;
  param.content_media_types = &param_media;
  param.n_content_media_types = 1;
  param_media.name = "application/x-www-form-urlencoded";
  param_media.schema_set = 1;
  param_media.schema.inline_type = "object";
  param_media.encoding = &enc;
  param_media.n_encoding = 1;
  enc.name = "id";
  enc.content_type = "text/plain";
  enc.style = OA_STYLE_FORM;
  enc.style_set = 1;
  enc.explode = 1;
  enc.explode_set = 1;

  op.responses = &resp;
  op.n_responses = 1;
  resp.code = "200";
  resp.description = "ok";
  resp.headers = &header;
  resp.n_headers = 1;
  header.name = "X-Rate";
  header.content_media_types = &header_media;
  header.n_content_media_types = 1;
  header_media.name = "text/plain";
  header_media.schema_set = 1;
  header_media.schema.inline_type = "string";

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root = json_parse_string(json);
    JSON_Object *paths =
        json_object_get_object(json_value_get_object(root), "paths");
    JSON_Object *p_item = json_object_get_object(paths, "/content");
    JSON_Object *op_obj = json_object_get_object(p_item, "get");
    JSON_Array *params = json_object_get_array(op_obj, "parameters");
    JSON_Object *p0 = json_array_get_object(params, 0);
    JSON_Object *content = json_object_get_object(p0, "content");
    JSON_Object *media =
        json_object_get_object(content, "application/x-www-form-urlencoded");
    JSON_Object *encoding = json_object_get_object(media, "encoding");
    JSON_Object *enc_id = json_object_get_object(encoding, "id");

    ASSERT(enc_id != NULL);
    ASSERT_STR_EQ("text/plain", json_object_get_string(enc_id, "contentType"));
    ASSERT_STR_EQ("form", json_object_get_string(enc_id, "style"));
    ASSERT_EQ(1, json_object_get_boolean(enc_id, "explode"));

    {
      JSON_Object *responses = json_object_get_object(op_obj, "responses");
      JSON_Object *r200 = json_object_get_object(responses, "200");
      JSON_Object *headers = json_object_get_object(r200, "headers");
      JSON_Object *x_rate = json_object_get_object(headers, "X-Rate");
      JSON_Object *h_content = json_object_get_object(x_rate, "content");
      JSON_Object *h_media = json_object_get_object(h_content, "text/plain");
      JSON_Object *schema = json_object_get_object(h_media, "schema");
      ASSERT_STR_EQ("string", json_object_get_string(schema, "type"));
    }

    json_value_free(root);
  }

  free(json);
  PASS();
}

TEST test_writer_parameter_examples_object(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Path path;
  struct OpenAPI_Operation op;
  struct OpenAPI_Parameter param;
  struct OpenAPI_Example ex;
  char *json = NULL;
  int rc;

  memset(&ex, 0, sizeof(ex));
  setup_test_spec(&spec, &path, &op, &param, NULL);
  ex.name = "basic";
  ex.data_value.type = OA_ANY_STRING;
  ex.data_value.string = "hello";
  ex.data_value_set = 1;
  param.examples = &ex;
  param.n_examples = 1;
  param.example_location = OA_EXAMPLE_LOC_OBJECT;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root = json_parse_string(json);
    JSON_Object *paths =
        json_object_get_object(json_value_get_object(root), "paths");
    JSON_Object *p_item = json_object_get_object(paths, "/test/route");
    JSON_Object *op_obj = json_object_get_object(p_item, "get");
    JSON_Array *params = json_object_get_array(op_obj, "parameters");
    JSON_Object *p0 = json_array_get_object(params, 0);
    JSON_Object *examples = json_object_get_object(p0, "examples");
    JSON_Object *basic = json_object_get_object(examples, "basic");

    ASSERT(basic != NULL);
    ASSERT_STR_EQ("hello", json_object_get_string(basic, "dataValue"));

    json_value_free(root);
  }

  free(json);
  PASS();
}

TEST test_writer_parameter_examples_media(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Path path;
  struct OpenAPI_Operation op;
  struct OpenAPI_Parameter param;
  char *json = NULL;
  int rc;

  setup_test_spec(&spec, &path, &op, &param, NULL);
  param.content_type = "application/json";
  param.example.type = OA_ANY_STRING;
  param.example.string = "hi";
  param.example_set = 1;
  param.example_location = OA_EXAMPLE_LOC_MEDIA;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root = json_parse_string(json);
    JSON_Object *paths =
        json_object_get_object(json_value_get_object(root), "paths");
    JSON_Object *p_item = json_object_get_object(paths, "/test/route");
    JSON_Object *op_obj = json_object_get_object(p_item, "get");
    JSON_Array *params = json_object_get_array(op_obj, "parameters");
    JSON_Object *p0 = json_array_get_object(params, 0);
    JSON_Object *content = json_object_get_object(p0, "content");
    JSON_Object *media = json_object_get_object(content, "application/json");

    ASSERT_STR_EQ("hi", json_object_get_string(media, "example"));

    json_value_free(root);
  }

  free(json);
  PASS();
}

TEST test_writer_component_examples(void) {
  struct OpenAPI_Spec spec = {0};
  struct OpenAPI_Example ex;
  char *json = NULL;
  char *names[1];
  int rc;

  memset(&ex, 0, sizeof(ex));
  names[0] = "ex1";
  spec.component_examples = &ex;
  spec.component_example_names = names;
  spec.n_component_examples = 1;
  ex.summary = "Example";
  ex.value.type = OA_ANY_STRING;
  ex.value.string = "v";
  ex.value_set = 1;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root = json_parse_string(json);
    JSON_Object *comps =
        json_object_get_object(json_value_get_object(root), "components");
    JSON_Object *examples = json_object_get_object(comps, "examples");
    JSON_Object *ex_obj = json_object_get_object(examples, "ex1");
    ASSERT_STR_EQ("Example", json_object_get_string(ex_obj, "summary"));
    ASSERT_STR_EQ("v", json_object_get_string(ex_obj, "value"));
    json_value_free(root);
  }

  free(json);
  PASS();
}

TEST test_writer_oauth2_flows(void) {
  struct OpenAPI_Spec spec = {0};
  struct OpenAPI_SecurityScheme scheme;
  struct OpenAPI_OAuthFlow flow;
  struct OpenAPI_OAuthScope scope;
  char *json = NULL;
  int rc;

  memset(&scheme, 0, sizeof(scheme));
  memset(&flow, 0, sizeof(flow));
  memset(&scope, 0, sizeof(scope));

  scope.name = "read";
  scope.description = "Read";
  flow.type = OA_OAUTH_FLOW_PASSWORD;
  flow.token_url = "https://token.example.com";
  flow.scopes = &scope;
  flow.n_scopes = 1;
  scheme.name = "oauth";
  scheme.type = OA_SEC_OAUTH2;
  scheme.flows = &flow;
  scheme.n_flows = 1;

  spec.security_schemes = &scheme;
  spec.n_security_schemes = 1;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root = json_parse_string(json);
    JSON_Object *comps =
        json_object_get_object(json_value_get_object(root), "components");
    JSON_Object *schemes = json_object_get_object(comps, "securitySchemes");
    JSON_Object *oauth = json_object_get_object(schemes, "oauth");
    JSON_Object *flows = json_object_get_object(oauth, "flows");
    JSON_Object *password = json_object_get_object(flows, "password");
    JSON_Object *scopes = json_object_get_object(password, "scopes");

    ASSERT_STR_EQ("https://token.example.com",
                  json_object_get_string(password, "tokenUrl"));
    ASSERT_STR_EQ("Read", json_object_get_string(scopes, "read"));
    json_value_free(root);
  }

  free(json);
  PASS();
}

TEST test_writer_servers(void) {
  struct OpenAPI_Spec spec = {0};
  struct OpenAPI_Server servers[1];
  char *json = NULL;
  int rc;

  memset(servers, 0, sizeof(servers));
  servers[0].url = "https://api.example.com";
  servers[0].description = "Prod";
  servers[0].name = "prod";

  spec.servers = servers;
  spec.n_servers = 1;
  spec.openapi_version = "3.1.2";

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root = json_parse_string(json);
    JSON_Object *root_obj = json_value_get_object(root);
    JSON_Array *srv_arr = json_object_get_array(root_obj, "servers");
    ASSERT_STR_EQ("3.1.2", json_object_get_string(root_obj, "openapi"));
    JSON_Object *srv = json_array_get_object(srv_arr, 0);

    ASSERT_STR_EQ("https://api.example.com",
                  json_object_get_string(srv, "url"));
    ASSERT_STR_EQ("Prod", json_object_get_string(srv, "description"));
    ASSERT_STR_EQ("prod", json_object_get_string(srv, "name"));
    json_value_free(root);
  }

  free(json);
  PASS();
}

TEST test_writer_querystring_param(void) {
  struct OpenAPI_Spec spec = {0};
  struct OpenAPI_Path path = {0};
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Parameter param = {0};
  char *json = NULL;
  int rc;

  spec.paths = &path;
  spec.n_paths = 1;
  path.route = "/search";
  path.operations = &op;
  path.n_operations = 1;
  op.verb = OA_VERB_GET;
  op.operation_id = "search";
  op.parameters = &param;
  op.n_parameters = 1;
  param.name = "qs";
  param.in = OA_PARAM_IN_QUERYSTRING;
  param.type = "string";
  param.content_type = "application/x-www-form-urlencoded";

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root = json_parse_string(json);
    JSON_Object *op_obj = json_value_get_object(root);
    op_obj = json_object_get_object(op_obj, "paths");
    op_obj = json_object_get_object(op_obj, "/search");
    op_obj = json_object_get_object(op_obj, "get");
    {
      JSON_Array *oa_params = json_object_get_array(op_obj, "parameters");
      JSON_Object *p_obj = json_array_get_object(oa_params, 0);
      JSON_Object *content = json_object_get_object(p_obj, "content");
      JSON_Object *media =
          json_object_get_object(content, "application/x-www-form-urlencoded");
      JSON_Object *schema = json_object_get_object(media, "schema");
      ASSERT_STR_EQ("querystring", json_object_get_string(p_obj, "in"));
      ASSERT(json_object_get_object(p_obj, "schema") == NULL);
      ASSERT_STR_EQ("string", json_object_get_string(schema, "type"));
    }

    json_value_free(root);
  }

  free(json);
  PASS();
}

TEST test_writer_ignores_reserved_header_params(void) {
  struct OpenAPI_Spec spec = {0};
  struct OpenAPI_Path path = {0};
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Parameter params[2] = {{0}};
  char *json = NULL;
  int rc;

  spec.paths = &path;
  spec.n_paths = 1;
  path.route = "/h";
  path.operations = &op;
  path.n_operations = 1;
  op.verb = OA_VERB_GET;
  op.operation_id = "getH";
  op.parameters = params;
  op.n_parameters = 2;

  params[0].name = "Accept";
  params[0].in = OA_PARAM_IN_HEADER;
  params[0].type = "string";

  params[1].name = "q";
  params[1].in = OA_PARAM_IN_QUERY;
  params[1].type = "string";

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root = json_parse_string(json);
    JSON_Object *op_obj = json_value_get_object(root);
    JSON_Array *oa_params;
    JSON_Object *p_obj;
    op_obj = json_object_get_object(op_obj, "paths");
    op_obj = json_object_get_object(op_obj, "/h");
    op_obj = json_object_get_object(op_obj, "get");
    oa_params = json_object_get_array(op_obj, "parameters");
    ASSERT_EQ(1, (int)json_array_get_count(oa_params));
    p_obj = json_array_get_object(oa_params, 0);
    ASSERT_STR_EQ("q", json_object_get_string(p_obj, "name"));
    json_value_free(root);
  }

  free(json);
  PASS();
}

TEST test_writer_ignores_content_type_response_header(void) {
  struct OpenAPI_Spec spec = {0};
  struct OpenAPI_Path path = {0};
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Response resp = {0};
  struct OpenAPI_Header headers[2] = {{0}};
  char *json = NULL;
  int rc;

  spec.paths = &path;
  spec.n_paths = 1;
  path.route = "/r";
  path.operations = &op;
  path.n_operations = 1;
  op.verb = OA_VERB_GET;
  op.operation_id = "getR";
  op.responses = &resp;
  op.n_responses = 1;

  resp.code = "200";
  resp.description = "ok";
  resp.headers = headers;
  resp.n_headers = 2;

  headers[0].name = "Content-Type";
  headers[0].type = "string";

  headers[1].name = "X-Rate";
  headers[1].type = "integer";

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root = json_parse_string(json);
    JSON_Object *op_obj = json_value_get_object(root);
    JSON_Object *resp_obj;
    JSON_Object *headers_obj;
    op_obj = json_object_get_object(op_obj, "paths");
    op_obj = json_object_get_object(op_obj, "/r");
    op_obj = json_object_get_object(op_obj, "get");
    resp_obj = json_object_get_object(op_obj, "responses");
    resp_obj = json_object_get_object(resp_obj, "200");
    headers_obj = json_object_get_object(resp_obj, "headers");
    ASSERT(headers_obj != NULL);
    ASSERT(json_object_get_object(headers_obj, "Content-Type") == NULL);
    ASSERT(json_object_get_object(headers_obj, "X-Rate") != NULL);
    json_value_free(root);
  }

  free(json);
  PASS();
}

TEST test_writer_path_level_parameters(void) {
  struct OpenAPI_Spec spec = {0};
  struct OpenAPI_Path path = {0};
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Parameter pparam = {0};
  char *json = NULL;
  int rc;

  spec.paths = &path;
  spec.n_paths = 1;
  path.route = "/pets";
  path.summary = "Pets";
  path.description = "All pets";
  path.parameters = &pparam;
  path.n_parameters = 1;
  path.operations = &op;
  path.n_operations = 1;
  op.verb = OA_VERB_GET;
  op.operation_id = "listPets";

  pparam.name = "x-trace";
  pparam.in = OA_PARAM_IN_HEADER;
  pparam.type = "string";

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root = json_parse_string(json);
    JSON_Object *paths =
        json_object_get_object(json_value_get_object(root), "paths");
    JSON_Object *item = json_object_get_object(paths, "/pets");
    JSON_Array *oa_params = json_object_get_array(item, "parameters");
    JSON_Object *p_obj = json_array_get_object(oa_params, 0);

    ASSERT_STR_EQ("Pets", json_object_get_string(item, "summary"));
    ASSERT_STR_EQ("All pets", json_object_get_string(item, "description"));
    ASSERT_STR_EQ("x-trace", json_object_get_string(p_obj, "name"));
    ASSERT_STR_EQ("header", json_object_get_string(p_obj, "in"));

    json_value_free(root);
  }

  free(json);
  PASS();
}

TEST test_writer_server_variables(void) {
  struct OpenAPI_Spec spec = {0};
  struct OpenAPI_Server server = {0};
  struct OpenAPI_ServerVariable var = {0};
  char *enum_vals[2];
  char *json = NULL;
  int rc;

  enum_vals[0] = "prod";
  enum_vals[1] = "staging";

  var.name = "env";
  var.default_value = "prod";
  var.description = "Environment";
  var.enum_values = enum_vals;
  var.n_enum_values = 2;

  server.url = "https://{env}.example.com";
  server.variables = &var;
  server.n_variables = 1;

  spec.servers = &server;
  spec.n_servers = 1;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root = json_parse_string(json);
    JSON_Object *obj = json_value_get_object(root);
    JSON_Array *srv_arr = json_object_get_array(obj, "servers");
    JSON_Object *srv_obj = json_array_get_object(srv_arr, 0);
    JSON_Object *vars = json_object_get_object(srv_obj, "variables");
    JSON_Object *env = json_object_get_object(vars, "env");
    JSON_Array *enum_arr = json_object_get_array(env, "enum");

    ASSERT_STR_EQ("prod", json_object_get_string(env, "default"));
    ASSERT_STR_EQ("Environment", json_object_get_string(env, "description"));
    ASSERT_STR_EQ("prod", json_array_get_string(enum_arr, 0));
    ASSERT_STR_EQ("staging", json_array_get_string(enum_arr, 1));

    json_value_free(root);
  }

  free(json);
  PASS();
}

TEST test_writer_security_schemes(void) {
  struct OpenAPI_Spec spec = {0};
  struct OpenAPI_SecurityScheme s1, s2;
  char *json = NULL;
  int rc;

  struct OpenAPI_SecurityScheme schemes[3];
  struct OpenAPI_SecurityScheme s3;
  memset(&s1, 0, sizeof(s1));
  memset(&s2, 0, sizeof(s2));
  memset(&s3, 0, sizeof(s3));

  s1.name = "bearerAuth";
  s1.type = OA_SEC_HTTP;
  s1.scheme = "bearer";
  s1.bearer_format = "Opaque";

  s2.name = "apiKeyAuth";
  s2.type = OA_SEC_APIKEY;
  s2.in = OA_SEC_IN_HEADER;
  s2.key_name = "X-Api-Key";

  schemes[0] = s1;
  schemes[1] = s2;
  s3.name = "mtlsAuth";
  s3.type = OA_SEC_MUTUALTLS;
  s3.description = "mTLS only";
  schemes[2] = s3;

  spec.security_schemes = schemes;
  spec.n_security_schemes = 3;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root = json_parse_string(json);
    JSON_Object *comps =
        json_object_get_object(json_value_get_object(root), "components");
    JSON_Object *secs = json_object_get_object(comps, "securitySchemes");

    /* Check Bearer */
    {
      JSON_Object *b = json_object_get_object(secs, "bearerAuth");
      ASSERT(b != NULL);
      ASSERT_STR_EQ("http", json_object_get_string(b, "type"));
      ASSERT_STR_EQ("bearer", json_object_get_string(b, "scheme"));
      ASSERT_STR_EQ("Opaque", json_object_get_string(b, "bearerFormat"));
    }

    /* Check ApiKey */
    {
      JSON_Object *k = json_object_get_object(secs, "apiKeyAuth");
      ASSERT(k != NULL);
      ASSERT_STR_EQ("apiKey", json_object_get_string(k, "type"));
      ASSERT_STR_EQ("header", json_object_get_string(k, "in"));
      ASSERT_STR_EQ("X-Api-Key", json_object_get_string(k, "name"));
    }

    /* Check MutualTLS */
    {
      JSON_Object *m = json_object_get_object(secs, "mtlsAuth");
      ASSERT(m != NULL);
      ASSERT_STR_EQ("mutualTLS", json_object_get_string(m, "type"));
      ASSERT_STR_EQ("mTLS only", json_object_get_string(m, "description"));
    }

    json_value_free(root);
  }
  free(json);
  PASS();
}

TEST test_writer_security_requirements(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Path path;
  struct OpenAPI_Operation op;
  struct OpenAPI_SecurityRequirementSet root_set;
  struct OpenAPI_SecurityRequirement root_req;
  struct OpenAPI_SecurityRequirementSet op_set;
  struct OpenAPI_SecurityRequirement op_req;
  char *json = NULL;
  int rc;

  setup_test_spec(&spec, &path, &op, NULL, NULL);

  memset(&root_set, 0, sizeof(root_set));
  memset(&root_req, 0, sizeof(root_req));
  memset(&op_set, 0, sizeof(op_set));
  memset(&op_req, 0, sizeof(op_req));

  root_req.scheme = "bearerAuth";
  root_set.requirements = &root_req;
  root_set.n_requirements = 1;

  spec.security = &root_set;
  spec.n_security = 1;
  spec.security_set = 1;

  op_req.scheme = "ApiKeyAuth";
  op_set.requirements = &op_req;
  op_set.n_requirements = 1;
  op.security = &op_set;
  op.n_security = 1;
  op.security_set = 1;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root = json_parse_string(json);
    JSON_Object *root_obj = json_value_get_object(root);
    JSON_Array *root_sec = json_object_get_array(root_obj, "security");
    JSON_Object *root_req_obj = json_array_get_object(root_sec, 0);
    JSON_Array *root_scopes = json_object_get_array(root_req_obj, "bearerAuth");

    ASSERT(root_scopes != NULL);

    {
      JSON_Object *op_obj = json_object_get_object(
          json_object_get_object(json_object_get_object(root_obj, "paths"),
                                 "/test/route"),
          "get");
      JSON_Array *op_sec = json_object_get_array(op_obj, "security");
      JSON_Object *op_req_obj = json_array_get_object(op_sec, 0);
      ASSERT(json_object_get_array(op_req_obj, "ApiKeyAuth") != NULL);
    }

    json_value_free(root);
  }

  free(json);
  PASS();
}

TEST test_writer_multipart_schema(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Path path;
  struct OpenAPI_Operation op;
  struct OpenAPI_MultipartField parts[2];
  char *json = NULL;
  int rc;

  memset(&parts, 0, sizeof(parts));
  parts[0].name = "file";
  parts[0].is_binary = 1; /* File upload */
  parts[1].name = "desc";
  parts[1].type = "string";

  setup_test_spec(&spec, &path, &op, NULL, NULL);
  op.verb = OA_VERB_POST;
  op.req_body.content_type = "multipart/form-data";
  op.req_body.multipart_fields = parts;
  op.req_body.n_multipart_fields = 2;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root = json_parse_string(json);
    JSON_Object *op_obj = json_value_get_object(root);
    op_obj = json_object_get_object(op_obj, "paths");
    op_obj = json_object_get_object(op_obj, "/test/route");
    op_obj = json_object_get_object(op_obj, "post");

    {
      JSON_Object *req = json_object_get_object(op_obj, "requestBody");
      JSON_Object *cnt = json_object_get_object(req, "content");
      JSON_Object *mp = json_object_get_object(cnt, "multipart/form-data");
      JSON_Object *sch = json_object_get_object(mp, "schema");
      JSON_Object *props = json_object_get_object(sch, "properties");

      ASSERT_STR_EQ("object", json_object_get_string(sch, "type"));

      /* Check File */
      {
        JSON_Object *f = json_object_get_object(props, "file");
        ASSERT_STR_EQ("string", json_object_get_string(f, "type"));
        ASSERT_STR_EQ("binary", json_object_get_string(f, "format"));
      }
      /* Check Desc */
      {
        JSON_Object *d = json_object_get_object(props, "desc");
        ASSERT_STR_EQ("string", json_object_get_string(d, "type"));
      }
    }
    json_value_free(root);
  }
  free(json);
  PASS();
}

TEST test_writer_components_and_response_headers(void) {
  struct OpenAPI_Spec spec = {0};
  struct OpenAPI_Path path = {0};
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Response responses[2];
  struct OpenAPI_Header resp_hdr = {0};
  struct OpenAPI_Parameter comp_param = {0};
  struct OpenAPI_Response comp_resp = {0};
  struct OpenAPI_Header comp_hdr = {0};
  struct OpenAPI_Parameter op_param = {0};
  char *param_names[1];
  char *resp_names[1];
  char *hdr_names[1];
  char *json = NULL;
  int rc;

  memset(responses, 0, sizeof(responses));

  param_names[0] = "LimitParam";
  resp_names[0] = "NotFound";
  hdr_names[0] = "RateLimit";

  comp_param.name = "limit";
  comp_param.in = OA_PARAM_IN_QUERY;
  comp_param.type = "integer";
  spec.component_parameters = &comp_param;
  spec.component_parameter_names = param_names;
  spec.n_component_parameters = 1;

  comp_resp.description = "missing";
  spec.component_responses = &comp_resp;
  spec.component_response_names = resp_names;
  spec.n_component_responses = 1;

  comp_hdr.type = "integer";
  spec.component_headers = &comp_hdr;
  spec.component_header_names = hdr_names;
  spec.n_component_headers = 1;

  op_param.ref = "#/components/parameters/LimitParam";
  op.parameters = &op_param;
  op.n_parameters = 1;

  responses[0].code = "200";
  responses[0].description = "ok";
  resp_hdr.name = "X-Rate";
  resp_hdr.ref = "#/components/headers/RateLimit";
  responses[0].headers = &resp_hdr;
  responses[0].n_headers = 1;

  responses[1].code = "404";
  responses[1].ref = "#/components/responses/NotFound";

  op.responses = responses;
  op.n_responses = 2;
  op.verb = OA_VERB_GET;
  op.operation_id = "listItems";

  path.route = "/items";
  path.operations = &op;
  path.n_operations = 1;

  spec.paths = &path;
  spec.n_paths = 1;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root = json_parse_string(json);
    JSON_Object *root_obj = json_value_get_object(root);
    JSON_Object *components = json_object_get_object(root_obj, "components");
    JSON_Object *params = json_object_get_object(components, "parameters");
    JSON_Object *hdrs = json_object_get_object(components, "headers");
    JSON_Object *resps = json_object_get_object(components, "responses");
    JSON_Object *paths = json_object_get_object(root_obj, "paths");
    JSON_Object *item = json_object_get_object(paths, "/items");
    JSON_Object *get = json_object_get_object(item, "get");
    JSON_Array *params_arr = json_object_get_array(get, "parameters");
    JSON_Object *p0 = json_array_get_object(params_arr, 0);
    JSON_Object *resp200 =
        json_object_get_object(json_object_get_object(get, "responses"), "200");
    JSON_Object *resp404 =
        json_object_get_object(json_object_get_object(get, "responses"), "404");
    JSON_Object *resp200_hdrs = json_object_get_object(resp200, "headers");
    JSON_Object *x_rate = json_object_get_object(resp200_hdrs, "X-Rate");

    ASSERT(params != NULL);
    ASSERT(hdrs != NULL);
    ASSERT(resps != NULL);
    ASSERT_STR_EQ("limit",
                  json_object_get_string(
                      json_object_get_object(params, "LimitParam"), "name"));
    ASSERT_STR_EQ("integer",
                  json_object_get_string(
                      json_object_get_object(
                          json_object_get_object(hdrs, "RateLimit"), "schema"),
                      "type"));
    ASSERT_STR_EQ("missing", json_object_get_string(
                                 json_object_get_object(resps, "NotFound"),
                                 "description"));

    ASSERT_STR_EQ("#/components/parameters/LimitParam",
                  json_object_get_string(p0, "$ref"));
    ASSERT_STR_EQ("#/components/headers/RateLimit",
                  json_object_get_string(x_rate, "$ref"));
    ASSERT_STR_EQ("#/components/responses/NotFound",
                  json_object_get_string(resp404, "$ref"));

    json_value_free(root);
  }

  free(json);
  PASS();
}

TEST test_writer_components_request_bodies(void) {
  struct OpenAPI_Spec spec = {0};
  struct OpenAPI_Path path = {0};
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_RequestBody comp_rb = {0};
  char *rb_names[1];
  char *json = NULL;
  int rc;

  setup_test_spec(&spec, &path, &op, NULL, NULL);
  op.verb = OA_VERB_POST;
  op.req_body_ref = "#/components/requestBodies/CreatePet";

  comp_rb.description = "Create";
  comp_rb.required_set = 1;
  comp_rb.required = 1;
  comp_rb.schema.content_type = "application/json";
  comp_rb.schema.ref_name = "Pet";

  rb_names[0] = "CreatePet";
  spec.component_request_bodies = &comp_rb;
  spec.component_request_body_names = rb_names;
  spec.n_component_request_bodies = 1;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root = json_parse_string(json);
    JSON_Object *root_obj = json_value_get_object(root);
    JSON_Object *components = json_object_get_object(root_obj, "components");
    JSON_Object *rbs = json_object_get_object(components, "requestBodies");
    JSON_Object *create = json_object_get_object(rbs, "CreatePet");
    JSON_Object *content = json_object_get_object(create, "content");
    JSON_Object *media = json_object_get_object(content, "application/json");
    JSON_Object *schema = json_object_get_object(media, "schema");
    JSON_Object *paths = json_object_get_object(root_obj, "paths");
    JSON_Object *p_item = json_object_get_object(paths, "/test/route");
    JSON_Object *post = json_object_get_object(p_item, "post");
    JSON_Object *req = json_object_get_object(post, "requestBody");

    ASSERT_STR_EQ("Create", json_object_get_string(create, "description"));
    ASSERT_EQ(1, json_object_get_boolean(create, "required"));
    ASSERT_STR_EQ("#/components/schemas/Pet",
                  json_object_get_string(schema, "$ref"));
    ASSERT_STR_EQ("#/components/requestBodies/CreatePet",
                  json_object_get_string(req, "$ref"));

    json_value_free(root);
  }

  free(json);
  PASS();
}

TEST test_writer_components_schemas(void) {
  struct OpenAPI_Spec spec;
  struct StructFields sf;
  char *name = "MyModel";
  char *json = NULL;
  int rc;

  memset(&spec, 0, sizeof(spec));
  struct_fields_init(&sf);
  struct_fields_add(&sf, "id", "integer", NULL, NULL, NULL);

  spec.defined_schemas = &sf;
  spec.defined_schema_names = &name;
  spec.n_defined_schemas = 1;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root = json_parse_string(json);
    JSON_Object *comps =
        json_object_get_object(json_value_get_object(root), "components");
    JSON_Object *schemas = json_object_get_object(comps, "schemas");
    JSON_Object *model = json_object_get_object(schemas, "MyModel");
    JSON_Object *props = json_object_get_object(model, "properties");
    JSON_Object *id_prop = json_object_get_object(props, "id");

    ASSERT_STR_EQ("integer", json_object_get_string(id_prop, "type"));

    json_value_free(root);
  }

  free(json);
  struct_fields_free(&sf);
  PASS();
}

TEST test_writer_components_schemas_raw(void) {
  struct OpenAPI_Spec spec = {0};
  char *names[3] = {"Token", "Flag", "Nums"};
  char *raw[3] = {"{\"type\":\"string\"}", "true",
                  "{\"type\":\"array\",\"items\":{\"type\":\"integer\"}}"};
  char *json = NULL;
  int rc;

  spec.raw_schema_names = names;
  spec.raw_schema_json = raw;
  spec.n_raw_schemas = 3;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root = json_parse_string(json);
    JSON_Object *comps =
        json_object_get_object(json_value_get_object(root), "components");
    JSON_Object *schemas = json_object_get_object(comps, "schemas");
    JSON_Object *token = json_object_get_object(schemas, "Token");
    JSON_Value *flag_val = json_object_get_value(schemas, "Flag");
    JSON_Object *nums = json_object_get_object(schemas, "Nums");
    JSON_Object *items = json_object_get_object(nums, "items");

    ASSERT_STR_EQ("string", json_object_get_string(token, "type"));
    ASSERT_EQ(JSONBoolean, json_value_get_type(flag_val));
    ASSERT_EQ(1, json_value_get_boolean(flag_val));
    ASSERT_STR_EQ("array", json_object_get_string(nums, "type"));
    ASSERT_STR_EQ("integer", json_object_get_string(items, "type"));

    json_value_free(root);
  }

  free(json);
  PASS();
}

TEST test_writer_schema_ref_external(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Path path;
  struct OpenAPI_Operation op;
  struct OpenAPI_Response resp;
  char *json = NULL;
  int rc;

  setup_test_spec(&spec, &path, &op, NULL, &resp);
  resp.schema.ref_name = NULL;
  resp.schema.ref = "https://example.com/schemas/Pet";

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root = json_parse_string(json);
    JSON_Object *paths =
        json_object_get_object(json_value_get_object(root), "paths");
    JSON_Object *p_item = json_object_get_object(paths, "/test/route");
    JSON_Object *get = json_object_get_object(p_item, "get");
    JSON_Object *responses = json_object_get_object(get, "responses");
    JSON_Object *resp_obj = json_object_get_object(responses, "200");
    JSON_Object *content = json_object_get_object(resp_obj, "content");
    JSON_Object *media = json_object_get_object(content, "application/json");
    JSON_Object *schema = json_object_get_object(media, "schema");

    ASSERT_STR_EQ("https://example.com/schemas/Pet",
                  json_object_get_string(schema, "$ref"));

    json_value_free(root);
  }

  free(json);
  PASS();
}

TEST test_writer_schema_dynamic_ref_external(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Path path;
  struct OpenAPI_Operation op;
  struct OpenAPI_Response resp;
  char *json = NULL;
  int rc;

  setup_test_spec(&spec, &path, &op, NULL, &resp);
  resp.schema.ref_name = NULL;
  resp.schema.ref = "https://example.com/schemas/Pet";
  resp.schema.ref_is_dynamic = 1;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root = json_parse_string(json);
    JSON_Object *paths =
        json_object_get_object(json_value_get_object(root), "paths");
    JSON_Object *p_item = json_object_get_object(paths, "/test/route");
    JSON_Object *get = json_object_get_object(p_item, "get");
    JSON_Object *responses = json_object_get_object(get, "responses");
    JSON_Object *resp_obj = json_object_get_object(responses, "200");
    JSON_Object *content = json_object_get_object(resp_obj, "content");
    JSON_Object *media = json_object_get_object(content, "application/json");
    JSON_Object *schema = json_object_get_object(media, "schema");

    ASSERT_STR_EQ("https://example.com/schemas/Pet",
                  json_object_get_string(schema, "$dynamicRef"));

    json_value_free(root);
  }

  free(json);
  PASS();
}

TEST test_writer_schema_items_ref_external(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Path path;
  struct OpenAPI_Operation op;
  struct OpenAPI_Response resp;
  char *json = NULL;
  int rc;

  setup_test_spec(&spec, &path, &op, NULL, &resp);
  resp.schema.ref_name = NULL;
  resp.schema.is_array = 1;
  resp.schema.items_ref = "https://example.com/schemas/Pet";

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root = json_parse_string(json);
    JSON_Object *paths =
        json_object_get_object(json_value_get_object(root), "paths");
    JSON_Object *p_item = json_object_get_object(paths, "/test/route");
    JSON_Object *get = json_object_get_object(p_item, "get");
    JSON_Object *responses = json_object_get_object(get, "responses");
    JSON_Object *resp_obj = json_object_get_object(responses, "200");
    JSON_Object *content = json_object_get_object(resp_obj, "content");
    JSON_Object *media = json_object_get_object(content, "application/json");
    JSON_Object *schema = json_object_get_object(media, "schema");
    JSON_Object *items = json_object_get_object(schema, "items");

    ASSERT_STR_EQ("array", json_object_get_string(schema, "type"));
    ASSERT_STR_EQ("https://example.com/schemas/Pet",
                  json_object_get_string(items, "$ref"));

    json_value_free(root);
  }

  free(json);
  PASS();
}

TEST test_writer_schema_items_dynamic_ref_external(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Path path;
  struct OpenAPI_Operation op;
  struct OpenAPI_Response resp;
  char *json = NULL;
  int rc;

  setup_test_spec(&spec, &path, &op, NULL, &resp);
  resp.schema.ref_name = NULL;
  resp.schema.is_array = 1;
  resp.schema.items_ref = "https://example.com/schemas/Pet";
  resp.schema.items_ref_is_dynamic = 1;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root = json_parse_string(json);
    JSON_Object *paths =
        json_object_get_object(json_value_get_object(root), "paths");
    JSON_Object *p_item = json_object_get_object(paths, "/test/route");
    JSON_Object *get = json_object_get_object(p_item, "get");
    JSON_Object *responses = json_object_get_object(get, "responses");
    JSON_Object *resp_obj = json_object_get_object(responses, "200");
    JSON_Object *content = json_object_get_object(resp_obj, "content");
    JSON_Object *media = json_object_get_object(content, "application/json");
    JSON_Object *schema = json_object_get_object(media, "schema");
    JSON_Object *items = json_object_get_object(schema, "items");

    ASSERT_STR_EQ("array", json_object_get_string(schema, "type"));
    ASSERT_STR_EQ("https://example.com/schemas/Pet",
                  json_object_get_string(items, "$dynamicRef"));

    json_value_free(root);
  }

  free(json);
  PASS();
}

TEST test_writer_additional_operations(void) {
  struct OpenAPI_Spec spec = {0};
  struct OpenAPI_Path path = {0};
  struct OpenAPI_Operation add_op = {0};
  struct OpenAPI_Response resp = {0};
  char *json = NULL;
  int rc;

  spec.paths = &path;
  spec.n_paths = 1;
  path.route = "/copy";
  path.additional_operations = &add_op;
  path.n_additional_operations = 1;

  add_op.method = "COPY";
  add_op.is_additional = 1;
  add_op.operation_id = "copyItem";
  add_op.responses = &resp;
  add_op.n_responses = 1;

  resp.code = "200";
  resp.description = "ok";

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root = json_parse_string(json);
    JSON_Object *paths =
        json_object_get_object(json_value_get_object(root), "paths");
    JSON_Object *p_item = json_object_get_object(paths, "/copy");
    JSON_Object *add_ops =
        json_object_get_object(p_item, "additionalOperations");
    JSON_Object *copy_op = json_object_get_object(add_ops, "COPY");

    ASSERT(copy_op != NULL);
    ASSERT_STR_EQ("copyItem", json_object_get_string(copy_op, "operationId"));

    json_value_free(root);
  }

  free(json);
  PASS();
}

TEST test_writer_component_media_types_and_content_ref(void) {
  struct OpenAPI_Spec spec = {0};
  struct OpenAPI_Path path = {0};
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Response resp = {0};
  struct OpenAPI_MediaType mt = {0};
  char *media_names[1];
  char *json = NULL;
  int rc;

  media_names[0] = "application/vnd.acme+json";
  spec.component_media_types = &mt;
  spec.component_media_type_names = media_names;
  spec.n_component_media_types = 1;

  mt.name = "application/vnd.acme+json";
  mt.schema_set = 1;
  mt.schema.ref_name = "Pet";

  spec.paths = &path;
  spec.n_paths = 1;
  path.route = "/pets";
  path.operations = &op;
  path.n_operations = 1;
  op.verb = OA_VERB_GET;
  op.operation_id = "getPet";
  op.responses = &resp;
  op.n_responses = 1;

  resp.code = "200";
  resp.description = "ok";
  resp.content_type = "application/vnd.acme+json";
  resp.content_ref = "#/components/mediaTypes/application~1vnd.acme+json";

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root = json_parse_string(json);
    JSON_Object *obj = json_value_get_object(root);
    JSON_Object *comps = json_object_get_object(obj, "components");
    JSON_Object *media = json_object_get_object(comps, "mediaTypes");
    JSON_Object *mt_obj =
        json_object_get_object(media, "application/vnd.acme+json");
    JSON_Object *schema_obj = json_object_get_object(mt_obj, "schema");
    JSON_Object *paths = json_object_get_object(obj, "paths");
    JSON_Object *p_item = json_object_get_object(paths, "/pets");
    JSON_Object *get_op = json_object_get_object(p_item, "get");
    JSON_Object *responses = json_object_get_object(get_op, "responses");
    JSON_Object *resp_obj = json_object_get_object(responses, "200");
    JSON_Object *content = json_object_get_object(resp_obj, "content");
    JSON_Object *mt_content =
        json_object_get_object(content, "application/vnd.acme+json");

    ASSERT_STR_EQ("#/components/schemas/Pet",
                  json_object_get_string(schema_obj, "$ref"));
    ASSERT_STR_EQ("#/components/mediaTypes/application~1vnd.acme+json",
                  json_object_get_string(mt_content, "$ref"));

    json_value_free(root);
  }

  free(json);
  PASS();
}

TEST test_writer_response_multiple_content(void) {
  struct OpenAPI_Spec spec = {0};
  struct OpenAPI_Path path = {0};
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Response resp = {0};
  struct OpenAPI_MediaType contents[2];
  char *json = NULL;
  int rc;

  memset(contents, 0, sizeof(contents));

  setup_test_spec(&spec, &path, &op, NULL, &resp);
  resp.description = "ok";

  contents[0].name = "application/json";
  contents[0].schema_set = 1;
  contents[0].schema.ref_name = "TestModel";
  contents[1].name = "text/plain";
  contents[1].schema_set = 1;
  contents[1].schema.inline_type = "string";

  resp.content_media_types = contents;
  resp.n_content_media_types = 2;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root = json_parse_string(json);
    JSON_Object *paths =
        json_object_get_object(json_value_get_object(root), "paths");
    JSON_Object *p_item = json_object_get_object(paths, "/test/route");
    JSON_Object *op_obj = json_object_get_object(p_item, "get");
    JSON_Object *responses = json_object_get_object(op_obj, "responses");
    JSON_Object *r200 = json_object_get_object(responses, "200");
    JSON_Object *content = json_object_get_object(r200, "content");

    ASSERT(content != NULL);
    ASSERT(json_object_get_object(content, "application/json") != NULL);
    ASSERT(json_object_get_object(content, "text/plain") != NULL);

    json_value_free(root);
  }

  free(json);
  PASS();
}

TEST test_writer_request_body_multiple_content_and_encoding(void) {
  struct OpenAPI_Spec spec = {0};
  struct OpenAPI_Path path = {0};
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_MediaType media[1];
  struct OpenAPI_Encoding enc[1];
  struct OpenAPI_Header enc_hdr = {0};
  char *json = NULL;
  int rc;

  memset(media, 0, sizeof(media));
  memset(enc, 0, sizeof(enc));

  setup_test_spec(&spec, &path, &op, NULL, NULL);

  enc_hdr.name = "X-Rate-Limit-Limit";
  enc_hdr.type = "integer";

  enc[0].name = "file";
  enc[0].content_type = "image/png";
  enc[0].explode_set = 1;
  enc[0].explode = 1;
  enc[0].allow_reserved_set = 1;
  enc[0].allow_reserved = 1;
  enc[0].headers = &enc_hdr;
  enc[0].n_headers = 1;

  media[0].name = "multipart/form-data";
  media[0].schema_set = 1;
  media[0].schema.inline_type = "object";
  media[0].encoding = enc;
  media[0].n_encoding = 1;

  op.req_body_media_types = media;
  op.n_req_body_media_types = 1;
  op.req_body_required_set = 1;
  op.req_body_required = 1;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root = json_parse_string(json);
    JSON_Object *paths =
        json_object_get_object(json_value_get_object(root), "paths");
    JSON_Object *p_item = json_object_get_object(paths, "/test/route");
    JSON_Object *op_obj = json_object_get_object(p_item, "get");
    JSON_Object *rb = json_object_get_object(op_obj, "requestBody");
    JSON_Object *content = json_object_get_object(rb, "content");
    JSON_Object *mt = json_object_get_object(content, "multipart/form-data");
    JSON_Object *encoding = json_object_get_object(mt, "encoding");
    JSON_Object *file_enc = json_object_get_object(encoding, "file");
    JSON_Object *headers = json_object_get_object(file_enc, "headers");
    JSON_Object *hdr = json_object_get_object(headers, "X-Rate-Limit-Limit");
    JSON_Object *hdr_schema = json_object_get_object(hdr, "schema");

    ASSERT_STR_EQ("image/png", json_object_get_string(file_enc, "contentType"));
    ASSERT_EQ(1, json_object_get_boolean(file_enc, "explode"));
    ASSERT_EQ(1, json_object_get_boolean(file_enc, "allowReserved"));
    ASSERT_STR_EQ("integer", json_object_get_string(hdr_schema, "type"));

    json_value_free(root);
  }

  free(json);
  PASS();
}

TEST test_writer_media_type_prefix_item_encoding(void) {
  struct OpenAPI_Spec spec = {0};
  struct OpenAPI_MediaType media[1];
  char *media_names[1];
  struct OpenAPI_Encoding prefix[2];
  struct OpenAPI_Encoding item = {0};
  struct OpenAPI_Encoding nested[1];
  struct OpenAPI_Header prefix_hdr = {0};
  char *json = NULL;
  int rc;

  memset(media, 0, sizeof(media));
  memset(prefix, 0, sizeof(prefix));
  memset(nested, 0, sizeof(nested));

  media_names[0] = "multipart/mixed";
  spec.component_media_type_names = media_names;
  media[0].name = "multipart/mixed";
  media[0].schema_set = 1;
  media[0].schema.inline_type = "array";

  prefix[0].content_type = "application/json";
  prefix_hdr.name = "X-Pos";
  prefix_hdr.type = "string";
  prefix[1].content_type = "image/png";
  prefix[1].headers = &prefix_hdr;
  prefix[1].n_headers = 1;

  media[0].prefix_encoding = prefix;
  media[0].n_prefix_encoding = 2;

  nested[0].name = "meta";
  nested[0].content_type = "text/plain";
  item.content_type = "application/octet-stream";
  item.encoding = nested;
  item.n_encoding = 1;

  media[0].item_encoding = &item;
  media[0].item_encoding_set = 1;

  spec.component_media_types = media;
  spec.n_component_media_types = 1;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root = json_parse_string(json);
    JSON_Object *components =
        json_object_get_object(json_value_get_object(root), "components");
    JSON_Object *media_types = json_object_get_object(components, "mediaTypes");
    JSON_Object *mt = json_object_get_object(media_types, "multipart/mixed");
    JSON_Array *prefix_arr = json_object_get_array(mt, "prefixEncoding");
    JSON_Object *prefix0 = json_array_get_object(prefix_arr, 0);
    JSON_Object *prefix1 = json_array_get_object(prefix_arr, 1);
    JSON_Object *prefix1_headers = json_object_get_object(prefix1, "headers");
    JSON_Object *prefix1_hdr = json_object_get_object(prefix1_headers, "X-Pos");
    JSON_Object *prefix1_schema = json_object_get_object(prefix1_hdr, "schema");
    JSON_Object *item_obj = json_object_get_object(mt, "itemEncoding");
    JSON_Object *item_encoding = json_object_get_object(item_obj, "encoding");
    JSON_Object *meta_obj = json_object_get_object(item_encoding, "meta");

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
  PASS();
}

TEST test_writer_component_path_items(void) {
  struct OpenAPI_Spec spec = {0};
  struct OpenAPI_Path path_item = {0};
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Response resp = {0};
  char *path_item_names[1];
  char *json = NULL;
  int rc;

  path_item.route = "FooItem";
  path_item.summary = "foo";
  path_item.operations = &op;
  path_item.n_operations = 1;

  op.verb = OA_VERB_GET;
  op.operation_id = "getFoo";
  op.responses = &resp;
  op.n_responses = 1;
  resp.code = "200";
  resp.description = "ok";

  path_item_names[0] = "FooItem";
  spec.component_path_items = &path_item;
  spec.component_path_item_names = path_item_names;
  spec.n_component_path_items = 1;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root = json_parse_string(json);
    JSON_Object *obj = json_value_get_object(root);
    JSON_Object *comps = json_object_get_object(obj, "components");
    JSON_Object *path_items = json_object_get_object(comps, "pathItems");
    JSON_Object *pi_obj = json_object_get_object(path_items, "FooItem");
    JSON_Object *get_op = json_object_get_object(pi_obj, "get");

    ASSERT_STR_EQ("foo", json_object_get_string(pi_obj, "summary"));
    ASSERT_STR_EQ("getFoo", json_object_get_string(get_op, "operationId"));

    json_value_free(root);
  }

  free(json);
  PASS();
}

TEST test_writer_response_links(void) {
  struct OpenAPI_Spec spec = {0};
  struct OpenAPI_Path path = {0};
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Response resp = {0};
  struct OpenAPI_Link link = {0};
  struct OpenAPI_LinkParam params[2];
  struct OpenAPI_Server link_server = {0};
  char *json = NULL;
  int rc;

  memset(params, 0, sizeof(params));

  spec.paths = &path;
  spec.n_paths = 1;
  path.route = "/pets";
  path.operations = &op;
  path.n_operations = 1;
  op.verb = OA_VERB_GET;
  op.operation_id = "listPets";
  op.responses = &resp;
  op.n_responses = 1;

  resp.code = "200";
  resp.description = "ok";
  resp.links = &link;
  resp.n_links = 1;

  link.name = "next";
  link.operation_id = "listPets";
  link.parameters = params;
  link.n_parameters = 2;
  params[0].name = "limit";
  params[0].value.type = OA_ANY_NUMBER;
  params[0].value.number = 50;
  params[1].name = "offset";
  params[1].value.type = OA_ANY_STRING;
  params[1].value.string = "$response.body#/offset";

  link.request_body_set = 1;
  link.request_body.type = OA_ANY_STRING;
  link.request_body.string = "payload";

  link_server.url = "https://api.example.com";
  link.server = &link_server;
  link.server_set = 1;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root = json_parse_string(json);
    JSON_Object *paths =
        json_object_get_object(json_value_get_object(root), "paths");
    JSON_Object *p_item = json_object_get_object(paths, "/pets");
    JSON_Object *op_obj = json_object_get_object(p_item, "get");
    JSON_Object *resps = json_object_get_object(op_obj, "responses");
    JSON_Object *resp_obj = json_object_get_object(resps, "200");
    JSON_Object *links = json_object_get_object(resp_obj, "links");
    JSON_Object *link_obj = json_object_get_object(links, "next");
    JSON_Object *params_obj = json_object_get_object(link_obj, "parameters");
    JSON_Object *srv_obj = json_object_get_object(link_obj, "server");

    ASSERT_STR_EQ("listPets", json_object_get_string(link_obj, "operationId"));
    ASSERT_EQ(50, (int)json_object_get_number(params_obj, "limit"));
    ASSERT_STR_EQ("$response.body#/offset",
                  json_object_get_string(params_obj, "offset"));
    ASSERT_STR_EQ("payload", json_object_get_string(link_obj, "requestBody"));
    ASSERT_STR_EQ("https://api.example.com",
                  json_object_get_string(srv_obj, "url"));

    json_value_free(root);
  }

  free(json);
  PASS();
}

TEST test_writer_callbacks(void) {
  struct OpenAPI_Spec spec = {0};
  struct OpenAPI_Path path = {0};
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Response resp = {0};
  struct OpenAPI_Callback cb = {0};
  struct OpenAPI_Path cb_path = {0};
  struct OpenAPI_Operation cb_op = {0};
  struct OpenAPI_Response cb_resp = {0};
  char *json = NULL;
  int rc;

  spec.paths = &path;
  spec.n_paths = 1;
  path.route = "/pets";
  path.operations = &op;
  path.n_operations = 1;
  op.verb = OA_VERB_GET;
  op.operation_id = "listPets";
  op.responses = &resp;
  op.n_responses = 1;
  resp.code = "200";
  resp.description = "ok";

  op.callbacks = &cb;
  op.n_callbacks = 1;
  cb.name = "onEvent";
  cb.paths = &cb_path;
  cb.n_paths = 1;

  cb_path.route = "{$request.body#/url}";
  cb_path.operations = &cb_op;
  cb_path.n_operations = 1;
  cb_op.verb = OA_VERB_POST;
  cb_op.operation_id = "cbPost";
  cb_op.responses = &cb_resp;
  cb_op.n_responses = 1;
  cb_resp.code = "200";
  cb_resp.description = "ok";

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root = json_parse_string(json);
    JSON_Object *paths =
        json_object_get_object(json_value_get_object(root), "paths");
    JSON_Object *p_item = json_object_get_object(paths, "/pets");
    JSON_Object *op_obj = json_object_get_object(p_item, "get");
    JSON_Object *cbs = json_object_get_object(op_obj, "callbacks");
    JSON_Object *cb_obj = json_object_get_object(cbs, "onEvent");
    JSON_Object *cb_path_obj =
        json_object_get_object(cb_obj, "{$request.body#/url}");
    JSON_Object *cb_post = json_object_get_object(cb_path_obj, "post");

    ASSERT(cb_post != NULL);
    ASSERT_STR_EQ("cbPost", json_object_get_string(cb_post, "operationId"));

    json_value_free(root);
  }

  free(json);
  PASS();
}

TEST test_writer_parameter_and_header_schema_ref(void) {
  struct OpenAPI_Spec spec = {0};
  struct OpenAPI_Path path = {0};
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Parameter params[2] = {0};
  struct OpenAPI_Response resp = {0};
  struct OpenAPI_Header hdr = {0};
  char *json = NULL;
  int rc;

  spec.paths = &path;
  spec.n_paths = 1;

  path.route = "/pets";
  path.operations = &op;
  path.n_operations = 1;

  op.verb = OA_VERB_GET;
  op.parameters = params;
  op.n_parameters = 2;

  params[0].name = "pet";
  params[0].in = OA_PARAM_IN_QUERY;
  params[0].type = "Pet";

  params[1].name = "tags";
  params[1].in = OA_PARAM_IN_QUERY;
  params[1].is_array = 1;
  params[1].type = "array";
  params[1].items_type = "Tag";

  op.responses = &resp;
  op.n_responses = 1;
  resp.code = "200";
  resp.description = "ok";
  resp.headers = &hdr;
  resp.n_headers = 1;
  hdr.name = "X-Rate";
  hdr.type = "Rate";

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root = json_parse_string(json);
    JSON_Object *paths =
        json_object_get_object(json_value_get_object(root), "paths");
    JSON_Object *p_item = json_object_get_object(paths, "/pets");
    JSON_Object *op_obj = json_object_get_object(p_item, "get");
    JSON_Array *params_arr = json_object_get_array(op_obj, "parameters");
    JSON_Object *p0 = json_array_get_object(params_arr, 0);
    JSON_Object *p1 = json_array_get_object(params_arr, 1);
    JSON_Object *p0_schema = json_object_get_object(p0, "schema");
    JSON_Object *p1_schema = json_object_get_object(p1, "schema");
    JSON_Object *p1_items = json_object_get_object(p1_schema, "items");

    ASSERT_STR_EQ("#/components/schemas/Pet",
                  json_object_get_string(p0_schema, "$ref"));
    ASSERT_STR_EQ("#/components/schemas/Tag",
                  json_object_get_string(p1_items, "$ref"));

    {
      JSON_Object *responses = json_object_get_object(op_obj, "responses");
      JSON_Object *resp200 = json_object_get_object(responses, "200");
      JSON_Object *headers = json_object_get_object(resp200, "headers");
      JSON_Object *hdr_obj = json_object_get_object(headers, "X-Rate");
      JSON_Object *hdr_schema = json_object_get_object(hdr_obj, "schema");
      ASSERT_STR_EQ("#/components/schemas/Rate",
                    json_object_get_string(hdr_schema, "$ref"));
    }

    json_value_free(root);
  }

  free(json);
  PASS();
}

TEST test_writer_parameter_schema_format_and_content(void) {
  struct OpenAPI_Spec spec = {0};
  struct OpenAPI_Path path = {0};
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Parameter param = {0};
  struct OpenAPI_Response resp = {0};
  char *json = NULL;
  int rc;

  spec.paths = &path;
  spec.n_paths = 1;

  path.route = "/pets";
  path.operations = &op;
  path.n_operations = 1;

  op.verb = OA_VERB_GET;
  op.operation_id = "getPets";
  op.parameters = &param;
  op.n_parameters = 1;

  param.name = "id";
  param.in = OA_PARAM_IN_QUERY;
  param.schema_set = 1;
  param.schema.inline_type = "string";
  param.schema.format = "uuid";
  param.schema.content_media_type = "text/plain";
  param.schema.content_encoding = "base64";

  op.responses = &resp;
  op.n_responses = 1;
  resp.code = "200";
  resp.description = "ok";

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root = json_parse_string(json);
    JSON_Object *paths =
        json_object_get_object(json_value_get_object(root), "paths");
    JSON_Object *p_item = json_object_get_object(paths, "/pets");
    JSON_Object *op_obj = json_object_get_object(p_item, "get");
    JSON_Array *params_arr = json_object_get_array(op_obj, "parameters");
    JSON_Object *p_obj = json_array_get_object(params_arr, 0);
    JSON_Object *schema = json_object_get_object(p_obj, "schema");

    ASSERT_STR_EQ("string", json_object_get_string(schema, "type"));
    ASSERT_STR_EQ("uuid", json_object_get_string(schema, "format"));
    ASSERT_STR_EQ("text/plain",
                  json_object_get_string(schema, "contentMediaType"));
    ASSERT_STR_EQ("base64", json_object_get_string(schema, "contentEncoding"));

    json_value_free(root);
  }

  free(json);
  PASS();
}

TEST test_writer_request_body_ref_with_description(void) {
  struct OpenAPI_Spec spec = {0};
  struct OpenAPI_Path path = {0};
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Response resp = {0};
  char *json = NULL;
  int rc;

  spec.paths = &path;
  spec.n_paths = 1;

  path.route = "/pets";
  path.operations = &op;
  path.n_operations = 1;

  op.verb = OA_VERB_POST;
  op.operation_id = "createPet";
  op.req_body_ref = "#/components/requestBodies/CreatePet";
  op.req_body_description = "Override";
  op.responses = &resp;
  op.n_responses = 1;
  resp.code = "200";
  resp.description = "ok";

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root = json_parse_string(json);
    JSON_Object *paths =
        json_object_get_object(json_value_get_object(root), "paths");
    JSON_Object *p_item = json_object_get_object(paths, "/pets");
    JSON_Object *op_obj = json_object_get_object(p_item, "post");
    JSON_Object *rb = json_object_get_object(op_obj, "requestBody");

    ASSERT_STR_EQ("#/components/requestBodies/CreatePet",
                  json_object_get_string(rb, "$ref"));
    ASSERT_STR_EQ("Override", json_object_get_string(rb, "description"));

    json_value_free(root);
  }

  free(json);
  PASS();
}

TEST test_writer_security_scheme_deprecated(void) {
  struct OpenAPI_Spec spec = {0};
  struct OpenAPI_SecurityScheme scheme = {0};
  char *json = NULL;
  int rc;

  spec.security_schemes = &scheme;
  spec.n_security_schemes = 1;

  scheme.name = "oldKey";
  scheme.type = OA_SEC_APIKEY;
  scheme.in = OA_SEC_IN_HEADER;
  scheme.key_name = "X-Old";
  scheme.deprecated_set = 1;
  scheme.deprecated = 1;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root = json_parse_string(json);
    JSON_Object *comps =
        json_object_get_object(json_value_get_object(root), "components");
    JSON_Object *schemes = json_object_get_object(comps, "securitySchemes");
    JSON_Object *old_key = json_object_get_object(schemes, "oldKey");
    ASSERT_EQ(1, json_object_get_boolean(old_key, "deprecated"));
    json_value_free(root);
  }

  free(json);
  PASS();
}

TEST test_writer_schema_enum_default_nullable(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Path path;
  struct OpenAPI_Operation op;
  struct OpenAPI_Parameter param;
  char *json = NULL;
  int rc;
  struct OpenAPI_Any enum_vals[2];

  setup_test_spec(&spec, &path, &op, &param, NULL);
  param.schema_set = 1;
  param.schema.inline_type = "string";
  param.schema.nullable = 1;
  enum_vals[0].type = OA_ANY_STRING;
  enum_vals[0].string = "on";
  enum_vals[1].type = OA_ANY_STRING;
  enum_vals[1].string = "off";
  param.schema.enum_values = enum_vals;
  param.schema.n_enum_values = 2;
  param.schema.default_value.type = OA_ANY_STRING;
  param.schema.default_value.string = "on";
  param.schema.default_value_set = 1;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root = json_parse_string(json);
    JSON_Object *paths =
        json_object_get_object(json_value_get_object(root), "paths");
    JSON_Object *p_item = json_object_get_object(paths, "/test/route");
    JSON_Array *params = json_object_get_array(
        json_object_get_object(p_item, "get"), "parameters");
    JSON_Object *p0 = json_array_get_object(params, 0);
    JSON_Object *schema = json_object_get_object(p0, "schema");
    JSON_Array *type_arr =
        json_value_get_array(json_object_get_value(schema, "type"));
    JSON_Array *enum_arr = json_object_get_array(schema, "enum");
    ASSERT(type_arr != NULL);
    ASSERT_EQ(2, json_array_get_count(type_arr));
    ASSERT_STR_EQ("string", json_array_get_string(type_arr, 0));
    ASSERT_STR_EQ("null", json_array_get_string(type_arr, 1));
    ASSERT(enum_arr != NULL);
    ASSERT_EQ(2, json_array_get_count(enum_arr));
    ASSERT_STR_EQ("on", json_array_get_string(enum_arr, 0));
    ASSERT_STR_EQ("off", json_array_get_string(enum_arr, 1));
    ASSERT_STR_EQ("on", json_object_get_string(schema, "default"));
    json_value_free(root);
  }

  free(json);
  PASS();
}

TEST test_writer_schema_type_union(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Path path;
  struct OpenAPI_Operation op;
  struct OpenAPI_Parameter param;
  char *json = NULL;
  int rc;
  char *types[] = {"string", "integer", "null"};

  setup_test_spec(&spec, &path, &op, &param, NULL);
  param.schema_set = 1;
  param.schema.inline_type = "string";
  param.schema.nullable = 1;
  param.schema.type_union = types;
  param.schema.n_type_union = 3;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root = json_parse_string(json);
    JSON_Object *paths =
        json_object_get_object(json_value_get_object(root), "paths");
    JSON_Object *p_item = json_object_get_object(paths, "/test/route");
    JSON_Array *params = json_object_get_array(
        json_object_get_object(p_item, "get"), "parameters");
    JSON_Object *p0 = json_array_get_object(params, 0);
    JSON_Object *schema = json_object_get_object(p0, "schema");
    JSON_Array *type_arr =
        json_value_get_array(json_object_get_value(schema, "type"));
    ASSERT(type_arr != NULL);
    ASSERT_EQ(3, json_array_get_count(type_arr));
    ASSERT_STR_EQ("string", json_array_get_string(type_arr, 0));
    ASSERT_STR_EQ("integer", json_array_get_string(type_arr, 1));
    ASSERT_STR_EQ("null", json_array_get_string(type_arr, 2));
    json_value_free(root);
  }

  free(json);
  PASS();
}

TEST test_writer_schema_array_items_enum_nullable(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Path path;
  struct OpenAPI_Operation op;
  struct OpenAPI_Parameter param;
  char *json = NULL;
  int rc;
  struct OpenAPI_Any enum_vals[2];

  setup_test_spec(&spec, &path, &op, &param, NULL);
  param.schema_set = 1;
  param.schema.is_array = 1;
  param.schema.inline_type = "string";
  param.schema.items_nullable = 1;
  enum_vals[0].type = OA_ANY_STRING;
  enum_vals[0].string = "a";
  enum_vals[1].type = OA_ANY_STRING;
  enum_vals[1].string = "b";
  param.schema.items_enum_values = enum_vals;
  param.schema.n_items_enum_values = 2;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root = json_parse_string(json);
    JSON_Object *paths =
        json_object_get_object(json_value_get_object(root), "paths");
    JSON_Object *p_item = json_object_get_object(paths, "/test/route");
    JSON_Array *params = json_object_get_array(
        json_object_get_object(p_item, "get"), "parameters");
    JSON_Object *p0 = json_array_get_object(params, 0);
    JSON_Object *schema = json_object_get_object(p0, "schema");
    JSON_Object *items = json_object_get_object(schema, "items");
    JSON_Array *type_arr =
        json_value_get_array(json_object_get_value(items, "type"));
    JSON_Array *enum_arr = json_object_get_array(items, "enum");
    ASSERT(type_arr != NULL);
    ASSERT_EQ(2, json_array_get_count(type_arr));
    ASSERT_STR_EQ("string", json_array_get_string(type_arr, 0));
    ASSERT_STR_EQ("null", json_array_get_string(type_arr, 1));
    ASSERT(enum_arr != NULL);
    ASSERT_EQ(2, json_array_get_count(enum_arr));
    ASSERT_STR_EQ("a", json_array_get_string(enum_arr, 0));
    ASSERT_STR_EQ("b", json_array_get_string(enum_arr, 1));
    json_value_free(root);
  }

  free(json);
  PASS();
}

TEST test_writer_schema_items_type_union(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Path path;
  struct OpenAPI_Operation op;
  struct OpenAPI_Parameter param;
  char *json = NULL;
  int rc;
  char *types[] = {"string", "integer"};

  setup_test_spec(&spec, &path, &op, &param, NULL);
  param.schema_set = 1;
  param.schema.is_array = 1;
  param.schema.inline_type = "string";
  param.schema.items_type_union = types;
  param.schema.n_items_type_union = 2;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root = json_parse_string(json);
    JSON_Object *paths =
        json_object_get_object(json_value_get_object(root), "paths");
    JSON_Object *p_item = json_object_get_object(paths, "/test/route");
    JSON_Array *params = json_object_get_array(
        json_object_get_object(p_item, "get"), "parameters");
    JSON_Object *p0 = json_array_get_object(params, 0);
    JSON_Object *schema = json_object_get_object(p0, "schema");
    JSON_Object *items = json_object_get_object(schema, "items");
    JSON_Array *type_arr =
        json_value_get_array(json_object_get_value(items, "type"));
    ASSERT(type_arr != NULL);
    ASSERT_EQ(2, json_array_get_count(type_arr));
    ASSERT_STR_EQ("string", json_array_get_string(type_arr, 0));
    ASSERT_STR_EQ("integer", json_array_get_string(type_arr, 1));
    json_value_free(root);
  }

  free(json);
  PASS();
}

TEST test_writer_schema_boolean(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Path path;
  struct OpenAPI_Operation op;
  struct OpenAPI_Parameter param;
  char *json = NULL;
  int rc;

  setup_test_spec(&spec, &path, &op, &param, NULL);
  param.schema_set = 1;
  param.schema.schema_is_boolean = 1;
  param.schema.schema_boolean_value = 1;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root = json_parse_string(json);
    JSON_Object *paths =
        json_object_get_object(json_value_get_object(root), "paths");
    JSON_Object *p_item = json_object_get_object(paths, "/test/route");
    JSON_Array *params = json_object_get_array(
        json_object_get_object(p_item, "get"), "parameters");
    JSON_Object *p0 = json_array_get_object(params, 0);
    JSON_Value *schema_val = json_object_get_value(p0, "schema");
    ASSERT(schema_val != NULL);
    ASSERT_EQ(JSONBoolean, json_value_get_type(schema_val));
    ASSERT_EQ(1, json_value_get_boolean(schema_val));
    json_value_free(root);
  }

  free(json);
  PASS();
}

TEST test_writer_schema_numeric_enum(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Path path;
  struct OpenAPI_Operation op;
  struct OpenAPI_Parameter param;
  char *json = NULL;
  int rc;
  struct OpenAPI_Any enum_vals[2];

  setup_test_spec(&spec, &path, &op, &param, NULL);
  param.schema_set = 1;
  param.schema.inline_type = "integer";
  enum_vals[0].type = OA_ANY_NUMBER;
  enum_vals[0].number = 1.0;
  enum_vals[1].type = OA_ANY_NUMBER;
  enum_vals[1].number = 2.0;
  param.schema.enum_values = enum_vals;
  param.schema.n_enum_values = 2;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root = json_parse_string(json);
    JSON_Object *paths =
        json_object_get_object(json_value_get_object(root), "paths");
    JSON_Object *p_item = json_object_get_object(paths, "/test/route");
    JSON_Array *params = json_object_get_array(
        json_object_get_object(p_item, "get"), "parameters");
    JSON_Object *p0 = json_array_get_object(params, 0);
    JSON_Object *schema = json_object_get_object(p0, "schema");
    JSON_Array *enum_arr = json_object_get_array(schema, "enum");
    ASSERT(enum_arr != NULL);
    ASSERT_EQ(2, json_array_get_count(enum_arr));
    ASSERT_EQ(1.0, json_array_get_number(enum_arr, 0));
    ASSERT_EQ(2.0, json_array_get_number(enum_arr, 1));
    json_value_free(root);
  }

  free(json);
  PASS();
}

TEST test_writer_schema_items_examples(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Path path;
  struct OpenAPI_Operation op;
  struct OpenAPI_Parameter param;
  char *json = NULL;
  int rc;
  struct OpenAPI_Any item_examples[2];

  setup_test_spec(&spec, &path, &op, &param, NULL);
  param.schema_set = 1;
  param.schema.is_array = 1;
  param.schema.inline_type = "string";
  item_examples[0].type = OA_ANY_STRING;
  item_examples[0].string = "a";
  item_examples[1].type = OA_ANY_STRING;
  item_examples[1].string = "b";
  param.schema.items_examples = item_examples;
  param.schema.n_items_examples = 2;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root = json_parse_string(json);
    JSON_Object *paths =
        json_object_get_object(json_value_get_object(root), "paths");
    JSON_Object *p_item = json_object_get_object(paths, "/test/route");
    JSON_Array *params = json_object_get_array(
        json_object_get_object(p_item, "get"), "parameters");
    JSON_Object *p0 = json_array_get_object(params, 0);
    JSON_Object *schema = json_object_get_object(p0, "schema");
    JSON_Object *items = json_object_get_object(schema, "items");
    JSON_Array *examples = json_object_get_array(items, "examples");
    ASSERT(examples != NULL);
    ASSERT_EQ(2, json_array_get_count(examples));
    ASSERT_STR_EQ("a", json_array_get_string(examples, 0));
    ASSERT_STR_EQ("b", json_array_get_string(examples, 1));
    json_value_free(root);
  }

  free(json);
  PASS();
}

TEST test_writer_schema_items_boolean(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Path path;
  struct OpenAPI_Operation op;
  struct OpenAPI_Parameter param;
  char *json = NULL;
  int rc;

  setup_test_spec(&spec, &path, &op, &param, NULL);
  param.schema_set = 1;
  param.schema.is_array = 1;
  param.schema.items_schema_is_boolean = 1;
  param.schema.items_schema_boolean_value = 0;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root = json_parse_string(json);
    JSON_Object *paths =
        json_object_get_object(json_value_get_object(root), "paths");
    JSON_Object *p_item = json_object_get_object(paths, "/test/route");
    JSON_Array *params = json_object_get_array(
        json_object_get_object(p_item, "get"), "parameters");
    JSON_Object *p0 = json_array_get_object(params, 0);
    JSON_Object *schema = json_object_get_object(p0, "schema");
    JSON_Value *items_val = json_object_get_value(schema, "items");
    ASSERT(items_val != NULL);
    ASSERT_EQ(JSONBoolean, json_value_get_type(items_val));
    ASSERT_EQ(0, json_value_get_boolean(items_val));
    json_value_free(root);
  }

  free(json);
  PASS();
}

TEST test_writer_schema_example_and_numeric_constraints(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Path path;
  struct OpenAPI_Operation op;
  struct OpenAPI_Parameter param;
  char *json = NULL;
  int rc;

  setup_test_spec(&spec, &path, &op, &param, NULL);
  param.schema_set = 1;
  param.schema.inline_type = "number";
  param.schema.has_min = 1;
  param.schema.min_val = 1;
  param.schema.has_max = 1;
  param.schema.max_val = 9;
  param.schema.exclusive_max = 1;
  param.schema.example_set = 1;
  param.schema.example.type = OA_ANY_NUMBER;
  param.schema.example.number = 2.5;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root = json_parse_string(json);
    JSON_Object *paths =
        json_object_get_object(json_value_get_object(root), "paths");
    JSON_Object *p_item = json_object_get_object(paths, "/test/route");
    JSON_Array *params = json_object_get_array(
        json_object_get_object(p_item, "get"), "parameters");
    JSON_Object *p0 = json_array_get_object(params, 0);
    JSON_Object *schema = json_object_get_object(p0, "schema");
    ASSERT_EQ(1.0, json_object_get_number(schema, "minimum"));
    ASSERT_EQ(9.0, json_object_get_number(schema, "exclusiveMaximum"));
    ASSERT_EQ(2.5, json_object_get_number(schema, "example"));
    json_value_free(root);
  }

  free(json);
  PASS();
}

TEST test_writer_schema_array_constraints_and_items_example(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Path path;
  struct OpenAPI_Operation op;
  struct OpenAPI_Parameter param;
  char *json = NULL;
  int rc;

  setup_test_spec(&spec, &path, &op, &param, NULL);
  param.schema_set = 1;
  param.schema.is_array = 1;
  param.schema.inline_type = "string";
  param.schema.has_min_items = 1;
  param.schema.min_items = 1;
  param.schema.has_max_items = 1;
  param.schema.max_items = 3;
  param.schema.unique_items = 1;
  param.schema.items_has_min_len = 1;
  param.schema.items_min_len = 2;
  param.schema.items_has_max_len = 1;
  param.schema.items_max_len = 5;
  param.schema.items_pattern = "^[a-z]+$";
  param.schema.items_example_set = 1;
  param.schema.items_example.type = OA_ANY_STRING;
  param.schema.items_example.string = "ab";

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root = json_parse_string(json);
    JSON_Object *paths =
        json_object_get_object(json_value_get_object(root), "paths");
    JSON_Object *p_item = json_object_get_object(paths, "/test/route");
    JSON_Array *params = json_object_get_array(
        json_object_get_object(p_item, "get"), "parameters");
    JSON_Object *p0 = json_array_get_object(params, 0);
    JSON_Object *schema = json_object_get_object(p0, "schema");
    JSON_Object *items = json_object_get_object(schema, "items");
    ASSERT_EQ(1.0, json_object_get_number(schema, "minItems"));
    ASSERT_EQ(3.0, json_object_get_number(schema, "maxItems"));
    ASSERT_EQ(1, json_object_get_boolean(schema, "uniqueItems"));
    ASSERT_EQ(2.0, json_object_get_number(items, "minLength"));
    ASSERT_EQ(5.0, json_object_get_number(items, "maxLength"));
    ASSERT_STR_EQ("^[a-z]+$", json_object_get_string(items, "pattern"));
    ASSERT_STR_EQ("ab", json_object_get_string(items, "example"));
    json_value_free(root);
  }

  free(json);
  PASS();
}

TEST test_writer_inline_schema_items_const_default_and_extras(void) {
  const char *json =
      "{\"paths\":{\"/"
      "q\":{\"get\":{\"parameters\":[{\"name\":\"tags\",\"in\":\"query\","
      "\"schema\":{\"type\":\"array\",\"x-top\":true,\"items\":{\"type\":"
      "\"string\",\"const\":\"x\",\"default\":\"y\",\"x-custom\":99}}}],"
      "\"responses\":{\"200\":{\"description\":\"OK\"}}}}},\"openapi\":\"3.2."
      "0\"}";

  struct OpenAPI_Spec spec;
  char *out_json = NULL;
  int rc;

  rc = load_spec_str2(json, &spec);
  ASSERT_EQ(0, rc);

  rc = openapi_write_spec_to_json(&spec, &out_json);
  ASSERT_EQ(0, rc);
  ASSERT(out_json != NULL);

  {
    JSON_Value *root = json_parse_string(out_json);
    JSON_Object *root_obj = json_value_get_object(root);
    JSON_Object *paths = json_object_get_object(root_obj, "paths");
    JSON_Object *path = json_object_get_object(paths, "/q");
    JSON_Object *get = json_object_get_object(path, "get");
    JSON_Array *params = json_object_get_array(get, "parameters");
    JSON_Object *param0 = json_array_get_object(params, 0);
    JSON_Object *schema = json_object_get_object(param0, "schema");
    JSON_Object *items = json_object_get_object(schema, "items");

    ASSERT_EQ(1, json_object_get_boolean(schema, "x-top"));
    ASSERT_STR_EQ("x", json_object_get_string(items, "const"));
    ASSERT_STR_EQ("y", json_object_get_string(items, "default"));
    ASSERT_EQ(99, (int)json_object_get_number(items, "x-custom"));

    json_value_free(root);
  }

  free(out_json);
  openapi_spec_free(&spec);
  PASS();
}

TEST test_writer_input_validation(void) {
  struct OpenAPI_Spec spec;
  char *json;

  ASSERT_EQ(EINVAL, openapi_write_spec_to_json(NULL, &json));
  ASSERT_EQ(EINVAL, openapi_write_spec_to_json(&spec, NULL));

  PASS();
}

TEST test_writer_extensions_non_schema(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Path path;
  struct OpenAPI_Operation op;
  struct OpenAPI_Parameter param;
  struct OpenAPI_Response resp;
  struct OpenAPI_Callback cb;
  struct OpenAPI_Path cb_path;
  struct OpenAPI_Operation cb_op;
  struct OpenAPI_Response cb_resp;
  struct OpenAPI_Tag tag;
  struct OpenAPI_SecurityScheme scheme;
  struct OpenAPI_SecurityRequirement sec_req;
  struct OpenAPI_SecurityRequirementSet sec_set;
  struct OpenAPI_OAuthFlow flow;
  struct OpenAPI_OAuthScope scope;
  struct OpenAPI_RequestBody comp_rb;
  char *rb_names[1];
  char *json = NULL;
  int rc;

  setup_test_spec(&spec, &path, &op, &param, &resp);

  spec.extensions_json = "{\"x-root\":1}";
  spec.info.title = "Spec";
  spec.info.version = "1";
  spec.info.extensions_json = "{\"x-info\":\"info\"}";
  spec.info.contact.name = "Support";
  spec.info.contact.extensions_json = "{\"x-contact\":true}";
  spec.info.license.name = "MIT";
  spec.info.license.extensions_json = "{\"x-license\":\"lic\"}";
  spec.external_docs.url = "https://example.com";
  spec.external_docs.extensions_json = "{\"x-ext\":\"ext\"}";

  path.extensions_json = "{\"x-path\":6}";
  op.extensions_json = "{\"x-op\":7}";
  op.responses_extensions_json = "{\"x-responses\":1}";
  op.req_body.inline_type = "string";
  op.req_body.content_type = "application/json";
  op.req_body_required_set = 1;
  op.req_body_required = 1;
  op.req_body_extensions_json = "{\"x-rb-op\":true}";
  param.extensions_json = "{\"x-param\":\"param\"}";
  resp.extensions_json = "{\"x-resp\":true}";

  memset(&cb, 0, sizeof(cb));
  memset(&cb_path, 0, sizeof(cb_path));
  memset(&cb_op, 0, sizeof(cb_op));
  memset(&cb_resp, 0, sizeof(cb_resp));
  op.callbacks = &cb;
  op.n_callbacks = 1;
  cb.name = "onEvent";
  cb.extensions_json = "{\"x-cb\":\"cb\"}";
  cb.paths = &cb_path;
  cb.n_paths = 1;
  cb_path.route = "{$request.body#/url}";
  cb_path.operations = &cb_op;
  cb_path.n_operations = 1;
  cb_op.verb = OA_VERB_POST;
  cb_op.responses = &cb_resp;
  cb_op.n_responses = 1;
  cb_resp.code = "200";
  cb_resp.description = "ok";

  memset(&tag, 0, sizeof(tag));
  tag.name = "pet";
  tag.extensions_json = "{\"x-tag\":\"tag\"}";
  spec.tags = &tag;
  spec.n_tags = 1;

  memset(&scheme, 0, sizeof(scheme));
  memset(&flow, 0, sizeof(flow));
  memset(&scope, 0, sizeof(scope));
  scope.name = "read";
  scope.description = "Read";
  flow.type = OA_OAUTH_FLOW_PASSWORD;
  flow.token_url = "https://token.example.com";
  flow.scopes = &scope;
  flow.n_scopes = 1;
  flow.extensions_json = "{\"x-flow\":1}";
  scheme.name = "oauth";
  scheme.type = OA_SEC_OAUTH2;
  scheme.flows = &flow;
  scheme.n_flows = 1;
  scheme.extensions_json = "{\"x-sec\":1}";
  spec.security_schemes = &scheme;
  spec.n_security_schemes = 1;

  memset(&sec_req, 0, sizeof(sec_req));
  memset(&sec_set, 0, sizeof(sec_set));
  sec_req.scheme = "oauth";
  sec_set.requirements = &sec_req;
  sec_set.n_requirements = 1;
  sec_set.extensions_json = "{\"x-sec-req\":1}";
  spec.security = &sec_set;
  spec.n_security = 1;
  spec.security_set = 1;

  memset(&comp_rb, 0, sizeof(comp_rb));
  rb_names[0] = "CompRB";
  comp_rb.description = "desc";
  comp_rb.schema.inline_type = "string";
  comp_rb.extensions_json = "{\"x-rb\":1}";
  spec.component_request_bodies = &comp_rb;
  spec.component_request_body_names = rb_names;
  spec.n_component_request_bodies = 1;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root = json_parse_string(json);
    JSON_Object *root_obj = json_value_get_object(root);
    JSON_Object *info_obj = json_object_get_object(root_obj, "info");
    JSON_Object *contact_obj = json_object_get_object(info_obj, "contact");
    JSON_Object *license_obj = json_object_get_object(info_obj, "license");
    JSON_Object *ext_docs_obj =
        json_object_get_object(root_obj, "externalDocs");
    JSON_Array *tags_arr = json_object_get_array(root_obj, "tags");
    JSON_Object *tag_obj = json_array_get_object(tags_arr, 0);
    JSON_Object *paths = json_object_get_object(root_obj, "paths");
    JSON_Object *path_obj = json_object_get_object(paths, "/test/route");
    JSON_Object *op_obj = json_object_get_object(path_obj, "get");
    JSON_Object *rb_op_obj = json_object_get_object(op_obj, "requestBody");
    JSON_Array *params_arr = json_object_get_array(op_obj, "parameters");
    JSON_Object *param_obj = json_array_get_object(params_arr, 0);
    JSON_Object *responses = json_object_get_object(op_obj, "responses");
    JSON_Object *resp_obj = json_object_get_object(responses, "200");
    JSON_Object *callbacks = json_object_get_object(op_obj, "callbacks");
    JSON_Object *cb_obj = json_object_get_object(callbacks, "onEvent");
    JSON_Object *components = json_object_get_object(root_obj, "components");
    JSON_Object *sec = json_object_get_object(components, "securitySchemes");
    JSON_Object *scheme_obj = json_object_get_object(sec, "oauth");
    JSON_Object *flows = json_object_get_object(scheme_obj, "flows");
    JSON_Object *flow_obj = json_object_get_object(flows, "password");
    JSON_Object *rbs = json_object_get_object(components, "requestBodies");
    JSON_Object *rb_obj = json_object_get_object(rbs, "CompRB");
    JSON_Array *security_arr = json_object_get_array(root_obj, "security");
    JSON_Object *security_obj = json_array_get_object(security_arr, 0);

    ASSERT_EQ(1, (int)json_object_get_number(root_obj, "x-root"));
    ASSERT_STR_EQ("info", json_object_get_string(info_obj, "x-info"));
    ASSERT_EQ(1, json_object_get_boolean(contact_obj, "x-contact"));
    ASSERT_STR_EQ("lic", json_object_get_string(license_obj, "x-license"));
    ASSERT_STR_EQ("ext", json_object_get_string(ext_docs_obj, "x-ext"));
    ASSERT_STR_EQ("tag", json_object_get_string(tag_obj, "x-tag"));
    ASSERT_EQ(6, (int)json_object_get_number(path_obj, "x-path"));
    ASSERT_EQ(7, (int)json_object_get_number(op_obj, "x-op"));
    ASSERT(rb_op_obj != NULL);
    ASSERT_EQ(1, json_object_get_boolean(rb_op_obj, "x-rb-op"));
    ASSERT_STR_EQ("param", json_object_get_string(param_obj, "x-param"));
    ASSERT_EQ(1, json_object_get_boolean(resp_obj, "x-resp"));
    ASSERT_EQ(1, (int)json_object_get_number(responses, "x-responses"));
    ASSERT_STR_EQ("cb", json_object_get_string(cb_obj, "x-cb"));
    ASSERT_EQ(1, (int)json_object_get_number(scheme_obj, "x-sec"));
    ASSERT_EQ(1, (int)json_object_get_number(flow_obj, "x-flow"));
    ASSERT_EQ(1, (int)json_object_get_number(rb_obj, "x-rb"));
    ASSERT_EQ(1, (int)json_object_get_number(security_obj, "x-sec-req"));

    json_value_free(root);
  }

  free(json);
  PASS();
}

TEST test_writer_paths_webhooks_components_extensions(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Path path;
  struct OpenAPI_Operation op;
  struct OpenAPI_Response resp;
  char *json = NULL;
  int rc;

  setup_test_spec(&spec, &path, &op, NULL, &resp);
  spec.info.title = "Spec";
  spec.info.version = "1";
  resp.description = "ok";

  spec.paths_extensions_json = "{\"x-paths\":true}";
  spec.webhooks_extensions_json = "{\"x-hooks\":1}";
  spec.components_extensions_json = "{\"x-comps\":{\"meta\":\"yes\"}}";
  spec.webhooks = NULL;
  spec.n_webhooks = 0;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);
  ASSERT(json != NULL);

  {
    JSON_Value *root = json_parse_string(json);
    JSON_Object *root_obj = json_value_get_object(root);
    JSON_Object *paths_obj = json_object_get_object(root_obj, "paths");
    JSON_Object *hooks_obj = json_object_get_object(root_obj, "webhooks");
    JSON_Object *comps_obj = json_object_get_object(root_obj, "components");
    JSON_Object *path_item = json_object_get_object(paths_obj, "/test/route");

    ASSERT_EQ(1, json_object_get_boolean(paths_obj, "x-paths"));
    ASSERT(path_item != NULL);

    ASSERT_EQ(1, (int)json_object_get_number(hooks_obj, "x-hooks"));
    ASSERT_STR_EQ("yes",
                  json_object_get_string(
                      json_object_get_object(comps_obj, "x-comps"), "meta"));

    json_value_free(root);
  }

  free(json);
  PASS();
}

SUITE(openapi_writer_suite) {
  RUN_TEST(test_writer_empty_spec);
  RUN_TEST(test_writer_basic_operation);
  RUN_TEST(test_writer_schema_document);
  RUN_TEST(test_writer_root_metadata_and_tags);
  RUN_TEST(test_writer_path_ref_and_servers);
  RUN_TEST(test_writer_webhooks);
  RUN_TEST(test_writer_params_responses);
  RUN_TEST(test_writer_parameter_metadata);
  RUN_TEST(test_writer_allow_empty_value);
  RUN_TEST(test_writer_request_body_metadata_and_response_description);
  RUN_TEST(test_writer_info_metadata);
  RUN_TEST(test_writer_info_license_identifier_and_url_rejected);
  RUN_TEST(test_writer_server_url_query_rejected);
  RUN_TEST(test_writer_operation_metadata);
  RUN_TEST(test_writer_response_content_type);
  RUN_TEST(test_writer_inline_response_schema_primitive);
  RUN_TEST(test_writer_inline_response_schema_array);
  RUN_TEST(test_writer_inline_schema_format_and_content);
  RUN_TEST(test_writer_inline_schema_array_item_format_and_content);
  RUN_TEST(test_writer_schema_external_docs_discriminator_xml);
  RUN_TEST(test_writer_inline_schema_const_examples_annotations);
  RUN_TEST(test_writer_preserves_composed_component_schema);
  RUN_TEST(test_writer_preserves_inline_composed_schema);
  RUN_TEST(test_writer_schema_ref_summary_description);
  RUN_TEST(test_writer_info_license_missing_name_rejected);
  RUN_TEST(test_writer_options_trace_verbs);
  RUN_TEST(test_writer_query_and_external_docs);
  RUN_TEST(test_writer_parameter_styles);
  RUN_TEST(test_writer_parameter_style_matrix);
  RUN_TEST(test_writer_servers);
  RUN_TEST(test_writer_querystring_param);
  RUN_TEST(test_writer_ignores_reserved_header_params);
  RUN_TEST(test_writer_ignores_content_type_response_header);
  RUN_TEST(test_writer_parameter_content_any);
  RUN_TEST(test_writer_parameter_and_header_content_media_type);
  RUN_TEST(test_writer_parameter_examples_object);
  RUN_TEST(test_writer_parameter_examples_media);
  RUN_TEST(test_writer_component_examples);
  RUN_TEST(test_writer_oauth2_flows);
  RUN_TEST(test_writer_path_level_parameters);
  RUN_TEST(test_writer_server_variables);
  RUN_TEST(test_writer_components_and_response_headers);
  RUN_TEST(test_writer_components_request_bodies);
  RUN_TEST(test_writer_security_schemes);
  RUN_TEST(test_writer_security_requirements);
  RUN_TEST(test_writer_multipart_schema);
  RUN_TEST(test_writer_components_schemas);
  RUN_TEST(test_writer_components_schemas_raw);
  RUN_TEST(test_writer_schema_ref_external);
  RUN_TEST(test_writer_schema_dynamic_ref_external);
  RUN_TEST(test_writer_schema_items_ref_external);
  RUN_TEST(test_writer_schema_items_dynamic_ref_external);
  RUN_TEST(test_writer_additional_operations);
  RUN_TEST(test_writer_component_media_types_and_content_ref);
  RUN_TEST(test_writer_response_multiple_content);
  RUN_TEST(test_writer_request_body_multiple_content_and_encoding);
  RUN_TEST(test_writer_media_type_prefix_item_encoding);
  RUN_TEST(test_writer_component_path_items);
  RUN_TEST(test_writer_response_links);
  RUN_TEST(test_writer_callbacks);
  RUN_TEST(test_writer_parameter_and_header_schema_ref);
  RUN_TEST(test_writer_parameter_schema_format_and_content);
  RUN_TEST(test_writer_request_body_ref_with_description);
  RUN_TEST(test_writer_security_scheme_deprecated);
  RUN_TEST(test_writer_schema_enum_default_nullable);
  RUN_TEST(test_writer_schema_type_union);
  RUN_TEST(test_writer_schema_array_items_enum_nullable);
  RUN_TEST(test_writer_schema_items_type_union);
  RUN_TEST(test_writer_schema_example_and_numeric_constraints);
  RUN_TEST(test_writer_schema_array_constraints_and_items_example);
  RUN_TEST(test_writer_inline_schema_items_const_default_and_extras);
  RUN_TEST(test_writer_input_validation);
  RUN_TEST(test_writer_extensions_non_schema);
  RUN_TEST(test_writer_paths_webhooks_components_extensions);
}

#endif /* TEST_OPENAPI_WRITER_H */
