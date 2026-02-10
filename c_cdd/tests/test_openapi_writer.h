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

#include "openapi_loader.h"
#include "openapi_writer.h"

/* --- Helpers --- */

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
    ASSERT_STR_EQ("https://example.com/docs", json_object_get_string(ext, "url"));
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
  spec.info.license.url = "https://www.apache.org/licenses/LICENSE-2.0.html";

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);
  ASSERT(json != NULL);

  {
    JSON_Value *root = json_parse_string(json);
    JSON_Object *info = json_object_get_object(json_value_get_object(root),
                                               "info");
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
    ASSERT_STR_EQ("https://www.apache.org/licenses/LICENSE-2.0.html",
                  json_object_get_string(license, "url"));

    json_value_free(root);
  }

  free(json);
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
    JSON_Object *op_obj = json_object_get_object(json_value_get_object(root),
                                                 "paths");
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

TEST test_writer_info_license_fallback(void) {
  struct OpenAPI_Spec spec = {0};
  char *json = NULL;
  int rc;

  spec.info.title = "Example";
  spec.info.version = "1.0";
  spec.info.license.identifier = "Apache-2.0";

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root = json_parse_string(json);
    JSON_Object *license =
        json_object_get_object(json_object_get_object(root, "info"), "license");
    ASSERT_STR_EQ("Unknown", json_object_get_string(license, "name"));
    ASSERT_STR_EQ("Apache-2.0", json_object_get_string(license, "identifier"));
    json_value_free(root);
  }

  free(json);
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
    JSON_Object *verbs =
        json_object_get_object(json_object_get_object(root, "paths"), "/verbs");
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
    JSON_Object *paths = json_object_get_object(json_value_get_object(root), "paths");
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

    ASSERT_STR_EQ("https://api.example.com", json_object_get_string(srv, "url"));
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
      JSON_Object *media = json_object_get_object(
          content, "application/x-www-form-urlencoded");
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
    JSON_Object *paths = json_object_get_object(json_value_get_object(root), "paths");
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

  struct OpenAPI_SecurityScheme schemes[2];
  memset(&s1, 0, sizeof(s1));
  memset(&s2, 0, sizeof(s2));

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

  spec.security_schemes = schemes;
  spec.n_security_schemes = 2;

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
    JSON_Array *root_scopes =
        json_object_get_array(root_req_obj, "bearerAuth");

    ASSERT(root_scopes != NULL);

    {
      JSON_Object *op_obj = json_object_get_object(
          json_object_get_object(
              json_object_get_object(root_obj, "paths"), "/test/route"),
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

TEST test_writer_input_validation(void) {
  struct OpenAPI_Spec spec;
  char *json;

  ASSERT_EQ(EINVAL, openapi_write_spec_to_json(NULL, &json));
  ASSERT_EQ(EINVAL, openapi_write_spec_to_json(&spec, NULL));

  PASS();
}

SUITE(openapi_writer_suite) {
  RUN_TEST(test_writer_empty_spec);
  RUN_TEST(test_writer_basic_operation);
  RUN_TEST(test_writer_root_metadata_and_tags);
  RUN_TEST(test_writer_path_ref_and_servers);
  RUN_TEST(test_writer_webhooks);
  RUN_TEST(test_writer_params_responses);
  RUN_TEST(test_writer_parameter_metadata);
  RUN_TEST(test_writer_allow_empty_value);
  RUN_TEST(test_writer_request_body_metadata_and_response_description);
  RUN_TEST(test_writer_info_metadata);
  RUN_TEST(test_writer_operation_metadata);
  RUN_TEST(test_writer_response_content_type);
  RUN_TEST(test_writer_inline_response_schema_primitive);
  RUN_TEST(test_writer_inline_response_schema_array);
  RUN_TEST(test_writer_info_license_fallback);
  RUN_TEST(test_writer_options_trace_verbs);
  RUN_TEST(test_writer_query_and_external_docs);
  RUN_TEST(test_writer_parameter_styles);
  RUN_TEST(test_writer_parameter_style_matrix);
  RUN_TEST(test_writer_servers);
  RUN_TEST(test_writer_querystring_param);
  RUN_TEST(test_writer_parameter_content_any);
  RUN_TEST(test_writer_path_level_parameters);
  RUN_TEST(test_writer_server_variables);
  RUN_TEST(test_writer_security_schemes);
  RUN_TEST(test_writer_security_requirements);
  RUN_TEST(test_writer_multipart_schema);
  RUN_TEST(test_writer_components_schemas);
  RUN_TEST(test_writer_input_validation);
}

#endif /* TEST_OPENAPI_WRITER_H */
