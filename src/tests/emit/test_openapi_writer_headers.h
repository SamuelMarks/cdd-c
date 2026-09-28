/**
 * @file test_openapi_writer_headers.h
 * @brief Unit tests for OpenAPI Writer (headers).
 */

#ifndef TEST_OPENAPI_WRITER_HEADERS_H
#define TEST_OPENAPI_WRITER_HEADERS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "emit/test_openapi_writer_common.h"
/* clang-format on */

TEST test_writer_ignores_content_type_response_header(void) {
  int rc;
  char *json;
  struct OpenAPI_Spec spec = {0};
  struct OpenAPI_Path path = {0};
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Response resp = {0};
  struct OpenAPI_Header headers[2] = {{0}};
  json = NULL;

  spec.paths = &path;
  spec.n_paths = 1;
  path.route = (char *)(size_t)(size_t) "/r";
  path.operations = &op;
  path.n_operations = 1;
  op.verb = OA_VERB_GET;
  op.operation_id = (char *)(size_t)(size_t) "getR";
  op.responses = &resp;
  op.n_responses = 1;

  resp.code = (char *)(size_t)(size_t) "200";
  resp.description = (char *)(size_t)(size_t) "ok";
  resp.headers = headers;
  resp.n_headers = 2;

  headers[0].name = (char *)(size_t)(size_t) "Content-Type";
  headers[0].type = (char *)(size_t)(size_t) "string";

  headers[1].name = (char *)(size_t)(size_t) "X-Rate";
  headers[1].type = (char *)(size_t)(size_t) "integer";

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root;
    JSON_Object *op_obj;
    JSON_Object *resp_obj;
    JSON_Object *headers_obj;
    root = json_parse_string(json);
    op_obj = json_value_get_object(root);
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
  g_fail_io_after = -1;
  PASS();
}

TEST test_writer_path_level_parameters(void) {
  int rc;
  char *json;
  struct OpenAPI_Spec spec = {0};
  struct OpenAPI_Path path = {0};
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Parameter pparam = {0};
  json = NULL;

  spec.paths = &path;
  spec.n_paths = 1;
  path.route = (char *)(size_t)(size_t) "/pets";
  path.summary = (char *)(size_t)(size_t) "Pets";
  path.description = (char *)(size_t)(size_t) "All pets";
  path.parameters = &pparam;
  path.n_parameters = 1;
  path.operations = &op;
  path.n_operations = 1;
  op.verb = OA_VERB_GET;
  op.operation_id = (char *)(size_t)(size_t) "listPets";

  pparam.name = (char *)(size_t)(size_t) "x-trace";
  pparam.in = OA_PARAM_IN_HEADER;
  pparam.type = (char *)(size_t)(size_t) "string";

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root;
    JSON_Object *item;
    JSON_Array *oa_params;
    JSON_Object *p_obj;
    JSON_Object *paths;
    root = json_parse_string(json);
    paths = json_object_get_object(json_value_get_object(root), "paths");
    item = json_object_get_object(paths, "/pets");
    oa_params = json_object_get_array(item, "parameters");
    p_obj = json_array_get_object(oa_params, 0);

    ASSERT_STR_EQ("Pets", json_object_get_string(item, "summary"));
    ASSERT_STR_EQ("All pets", json_object_get_string(item, "description"));
    ASSERT_STR_EQ("x-trace", json_object_get_string(p_obj, "name"));
    ASSERT_STR_EQ("header", json_object_get_string(p_obj, "in"));

    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

TEST test_writer_server_variables(void) {
  int rc;
  char *enum_vals[2];
  char *json;
  struct OpenAPI_Spec spec = {0};
  struct OpenAPI_Server server = {0};
  struct OpenAPI_ServerVariable var = {0};
  json = NULL;

  enum_vals[0] = (char *)(size_t)(size_t) "prod";
  enum_vals[1] = (char *)(size_t)(size_t) "staging";

  var.name = (char *)(size_t)(size_t) "env";
  var.default_value = (char *)(size_t)(size_t) "prod";
  var.description = (char *)(size_t)(size_t) "Environment";
  var.enum_values = enum_vals;
  var.n_enum_values = 2;

  server.url = (char *)(size_t)(size_t) "https://{env}.example.com";
  server.variables = &var;
  server.n_variables = 1;

  spec.servers = &server;
  spec.n_servers = 1;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root;
    JSON_Object *obj;
    JSON_Array *srv_arr;
    JSON_Object *srv_obj;
    JSON_Object *vars;
    JSON_Object *env;
    JSON_Array *enum_arr;
    root = json_parse_string(json);
    obj = json_value_get_object(root);
    srv_arr = json_object_get_array(obj, "servers");
    srv_obj = json_array_get_object(srv_arr, 0);
    vars = json_object_get_object(srv_obj, "variables");
    env = json_object_get_object(vars, "env");
    enum_arr = json_object_get_array(env, "enum");

    ASSERT_STR_EQ("prod", json_object_get_string(env, "default"));
    ASSERT_STR_EQ("Environment", json_object_get_string(env, "description"));
    ASSERT_STR_EQ("prod", json_array_get_string(enum_arr, 0));
    ASSERT_STR_EQ("staging", json_array_get_string(enum_arr, 1));

    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

TEST test_writer_security_schemes(void) {
  int rc;
  struct OpenAPI_SecurityScheme s1, s2;
  char *json;
  struct OpenAPI_Spec spec = {0};
  struct OpenAPI_SecurityScheme schemes[3];
  struct OpenAPI_SecurityScheme s3;
  json = NULL;

  memset(&s1, 0, sizeof(s1));
  memset(&s2, 0, sizeof(s2));
  memset(&s3, 0, sizeof(s3));

  s1.name = (char *)(size_t)(size_t) "bearerAuth";
  s1.type = OA_SEC_HTTP;
  s1.scheme = (char *)(size_t)(size_t) "bearer";
  s1.bearer_format = (char *)(size_t)(size_t) "Opaque";

  s2.name = (char *)(size_t)(size_t) "apiKeyAuth";
  s2.type = OA_SEC_APIKEY;
  s2.in = OA_SEC_IN_HEADER;
  s2.key_name = (char *)(size_t)(size_t) "X-Api-Key";

  schemes[0] = s1;
  schemes[1] = s2;
  s3.name = (char *)(size_t)(size_t) "mtlsAuth";
  s3.type = OA_SEC_MUTUALTLS;
  s3.description = (char *)(size_t)(size_t) "mTLS only";
  schemes[2] = s3;

  spec.security_schemes = schemes;
  spec.n_security_schemes = 3;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root;
    JSON_Object *secs;
    JSON_Object *comps;
    root = json_parse_string(json);
    comps = json_object_get_object(json_value_get_object(root), "components");
    secs = json_object_get_object(comps, "securitySchemes");

    /* Check Bearer */
    {
      JSON_Object *b;
      b = json_object_get_object(secs, "bearerAuth");
      ASSERT(b != NULL);
      ASSERT_STR_EQ("http", json_object_get_string(b, "type"));
      ASSERT_STR_EQ("bearer", json_object_get_string(b, "scheme"));
      ASSERT_STR_EQ("Opaque", json_object_get_string(b, "bearerFormat"));
    }

    /* Check ApiKey */
    {
      JSON_Object *k;
      k = json_object_get_object(secs, "apiKeyAuth");
      ASSERT(k != NULL);
      ASSERT_STR_EQ("apiKey", json_object_get_string(k, "type"));
      ASSERT_STR_EQ("header", json_object_get_string(k, "in"));
      ASSERT_STR_EQ("X-Api-Key", json_object_get_string(k, "name"));
    }

    /* Check MutualTLS */
    {
      JSON_Object *m;
      m = json_object_get_object(secs, "mtlsAuth");
      ASSERT(m != NULL);
      ASSERT_STR_EQ("mutualTLS", json_object_get_string(m, "type"));
      ASSERT_STR_EQ("mTLS only", json_object_get_string(m, "description"));
    }

    json_value_free(root);
  }
  free(json);
  g_fail_io_after = -1;
  PASS();
}

TEST test_writer_security_requirements(void) {
  int rc;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Path path;
  struct OpenAPI_SecurityRequirementSet root_set;
  struct OpenAPI_SecurityRequirement root_req;
  struct OpenAPI_SecurityRequirementSet op_set;
  struct OpenAPI_SecurityRequirement op_req;
  char *json;
  struct OpenAPI_Operation op = {0};
  json = NULL;

  setup_test_spec(&spec, &path, &op, NULL, NULL);

  memset(&root_set, 0, sizeof(root_set));
  memset(&root_req, 0, sizeof(root_req));
  memset(&op_set, 0, sizeof(op_set));
  memset(&op_req, 0, sizeof(op_req));

  root_req.scheme = (char *)(size_t)(size_t) "bearerAuth";
  root_set.requirements = &root_req;
  root_set.n_requirements = 1;

  spec.security = &root_set;
  spec.n_security = 1;
  spec.security_set = 1;

  op_req.scheme = (char *)(size_t)(size_t) "ApiKeyAuth";
  op_set.requirements = &op_req;
  op_set.n_requirements = 1;
  op.security = &op_set;
  op.n_security = 1;
  op.security_set = 1;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root;
    JSON_Object *root_obj;
    JSON_Array *root_sec;
    JSON_Object *root_req_obj;
    JSON_Array *root_scopes;
    root = json_parse_string(json);
    root_obj = json_value_get_object(root);
    root_sec = json_object_get_array(root_obj, "security");
    root_req_obj = json_array_get_object(root_sec, 0);
    root_scopes = json_object_get_array(root_req_obj, "bearerAuth");

    ASSERT(root_scopes != NULL);

    {
      JSON_Array *op_sec;
      JSON_Object *op_req_obj;
      JSON_Object *op_obj;
      op_obj = json_object_get_object(
          json_object_get_object(json_object_get_object(root_obj, "paths"),
                                 "/test/route"),
          "get");
      op_sec = json_object_get_array(op_obj, "security");
      op_req_obj = json_array_get_object(op_sec, 0);
      ASSERT(json_object_get_array(op_req_obj, "ApiKeyAuth") != NULL);
    }

    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

TEST test_writer_multipart_schema(void) {
  int rc;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Path path;
  struct OpenAPI_MultipartField parts[2];
  char *json;
  struct OpenAPI_Operation op = {0};
  json = NULL;

  memset(&parts, 0, sizeof(parts));
  parts[0].name = (char *)(size_t)(size_t) "file";
  parts[0].is_binary = 1; /* File upload */
  parts[1].name = (char *)(size_t)(size_t) "desc";
  parts[1].type = (char *)(size_t)(size_t) "string";

  setup_test_spec(&spec, &path, &op, NULL, NULL);
  op.verb = OA_VERB_POST;
  op.req_body.content_type = (char *)(size_t)(size_t) "multipart/form-data";
  op.req_body.multipart_fields = parts;
  op.req_body.n_multipart_fields = 2;

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
      JSON_Object *req;
      JSON_Object *cnt;
      JSON_Object *mp;
      JSON_Object *sch;
      JSON_Object *props;
      req = json_object_get_object(op_obj, "requestBody");
      cnt = json_object_get_object(req, "content");
      mp = json_object_get_object(cnt, "multipart/form-data");
      sch = json_object_get_object(mp, "schema");
      props = json_object_get_object(sch, "properties");

      ASSERT_STR_EQ("object", json_object_get_string(sch, "type"));

      /* Check File */
      {
        JSON_Object *f;
        f = json_object_get_object(props, "file");
        ASSERT_STR_EQ("string", json_object_get_string(f, "type"));
        ASSERT_STR_EQ("binary", json_object_get_string(f, "format"));
      }
      /* Check Desc */
      {
        JSON_Object *d;
        d = json_object_get_object(props, "desc");
        ASSERT_STR_EQ("string", json_object_get_string(d, "type"));
      }
    }
    json_value_free(root);
  }
  free(json);
  g_fail_io_after = -1;
  PASS();
}

TEST test_writer_components_and_response_headers(void) {
  int rc;
  struct OpenAPI_Response responses[2];
  char *param_names[1];
  char *resp_names[1];
  char *hdr_names[1];
  char *json;
  struct OpenAPI_Spec spec = {0};
  struct OpenAPI_Path path = {0};
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Header resp_hdr = {0};
  struct OpenAPI_Parameter comp_param = {0};
  struct OpenAPI_Response comp_resp = {0};
  struct OpenAPI_Header comp_hdr = {0};
  struct OpenAPI_Parameter op_param = {0};
  json = NULL;

  memset(responses, 0, sizeof(responses));

  param_names[0] = (char *)(size_t)(size_t) "LimitParam";
  resp_names[0] = (char *)(size_t)(size_t) "NotFound";
  hdr_names[0] = (char *)(size_t)(size_t) "RateLimit";

  comp_param.name = (char *)(size_t)(size_t) "limit";
  comp_param.in = OA_PARAM_IN_QUERY;
  comp_param.type = (char *)(size_t)(size_t) "integer";
  spec.component_parameters = &comp_param;
  spec.component_parameter_names = param_names;
  spec.n_component_parameters = 1;

  comp_resp.description = (char *)(size_t)(size_t) "missing";
  spec.component_responses = &comp_resp;
  spec.component_response_names = resp_names;
  spec.n_component_responses = 1;

  comp_hdr.type = (char *)(size_t)(size_t) "integer";
  spec.component_headers = &comp_hdr;
  spec.component_header_names = hdr_names;
  spec.n_component_headers = 1;

  op_param.ref = (char *)(size_t)(size_t) "#/components/parameters/LimitParam";
  op.parameters = &op_param;
  op.n_parameters = 1;

  responses[0].code = (char *)(size_t)(size_t) "200";
  responses[0].description = (char *)(size_t)(size_t) "ok";
  resp_hdr.name = (char *)(size_t)(size_t) "X-Rate";
  resp_hdr.ref = (char *)(size_t)(size_t) "#/components/headers/RateLimit";
  responses[0].headers = &resp_hdr;
  responses[0].n_headers = 1;

  responses[1].code = (char *)(size_t)(size_t) "404";
  responses[1].ref = (char *)(size_t)(size_t) "#/components/responses/NotFound";

  op.responses = responses;
  op.n_responses = 2;
  op.verb = OA_VERB_GET;
  op.operation_id = (char *)(size_t)(size_t) "listItems";

  path.route = (char *)(size_t)(size_t) "/items";
  path.operations = &op;
  path.n_operations = 1;

  spec.paths = &path;
  spec.n_paths = 1;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root;
    JSON_Object *root_obj;
    JSON_Object *components;
    JSON_Object *params;
    JSON_Object *hdrs;
    JSON_Object *resps;
    JSON_Object *paths;
    JSON_Object *item;
    JSON_Object *get;
    JSON_Array *params_arr;
    JSON_Object *p0;
    JSON_Object *resp200_hdrs;
    JSON_Object *x_rate;
    JSON_Object *resp200;
    JSON_Object *resp404;
    root = json_parse_string(json);
    root_obj = json_value_get_object(root);
    components = json_object_get_object(root_obj, "components");
    params = json_object_get_object(components, "parameters");
    hdrs = json_object_get_object(components, "headers");
    resps = json_object_get_object(components, "responses");
    paths = json_object_get_object(root_obj, "paths");
    item = json_object_get_object(paths, "/items");
    get = json_object_get_object(item, "get");
    params_arr = json_object_get_array(get, "parameters");
    p0 = json_array_get_object(params_arr, 0);
    resp200 =
        json_object_get_object(json_object_get_object(get, "responses"), "200");
    resp404 =
        json_object_get_object(json_object_get_object(get, "responses"), "404");
    resp200_hdrs = json_object_get_object(resp200, "headers");
    x_rate = json_object_get_object(resp200_hdrs, "X-Rate");

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
  g_fail_io_after = -1;
  PASS();
}

TEST test_writer_components_request_bodies(void) {
  int rc;
  char *rb_names[1];
  char *json;
  struct OpenAPI_Spec spec = {0};
  struct OpenAPI_Path path = {0};
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_RequestBody comp_rb = {0};
  json = NULL;

  setup_test_spec(&spec, &path, &op, NULL, NULL);
  op.verb = OA_VERB_POST;
  op.req_body_ref =
      (char *)(size_t)(size_t) "#/components/requestBodies/CreatePet";

  comp_rb.description = (char *)(size_t)(size_t) "Create";
  comp_rb.required_set = 1;
  comp_rb.required = 1;
  comp_rb.schema.content_type = (char *)(size_t)(size_t) "application/json";
  comp_rb.schema.ref_name = (char *)(size_t)(size_t) "Pet";

  rb_names[0] = (char *)(size_t)(size_t) "CreatePet";
  spec.component_request_bodies = &comp_rb;
  spec.component_request_body_names = rb_names;
  spec.n_component_request_bodies = 1;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root;
    JSON_Object *root_obj;
    JSON_Object *components;
    JSON_Object *rbs;
    JSON_Object *create;
    JSON_Object *content;
    JSON_Object *media;
    JSON_Object *schema;
    JSON_Object *paths;
    JSON_Object *p_item;
    JSON_Object *post;
    JSON_Object *req;
    root = json_parse_string(json);
    root_obj = json_value_get_object(root);
    components = json_object_get_object(root_obj, "components");
    rbs = json_object_get_object(components, "requestBodies");
    create = json_object_get_object(rbs, "CreatePet");
    content = json_object_get_object(create, "content");
    media = json_object_get_object(content, "application/json");
    schema = json_object_get_object(media, "schema");
    paths = json_object_get_object(root_obj, "paths");
    p_item = json_object_get_object(paths, "/test/route");
    post = json_object_get_object(p_item, "post");
    req = json_object_get_object(post, "requestBody");

    ASSERT_STR_EQ("Create", json_object_get_string(create, "description"));
    ASSERT_EQ(1, json_object_get_boolean(create, "required"));
    ASSERT_STR_EQ("#/components/schemas/Pet",
                  json_object_get_string(schema, "$ref"));
    ASSERT_STR_EQ("#/components/requestBodies/CreatePet",
                  json_object_get_string(req, "$ref"));

    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

TEST test_writer_components_schemas(void) {
  int rc;
  struct OpenAPI_Spec spec;
  struct StructFields sf;
  char *name;
  char *json;
  name = (char *)(size_t)(size_t) "MyModel";
  json = NULL;

  memset(&spec, 0, sizeof(spec));
  struct_fields_init(&sf);
  struct_fields_add(&sf, "id", "integer", NULL, NULL, NULL);

  spec.defined_schemas = &sf;
  spec.defined_schema_names = &name;
  spec.n_defined_schemas = 1;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root;
    JSON_Object *schemas;
    JSON_Object *model;
    JSON_Object *props;
    JSON_Object *id_prop;
    JSON_Object *comps;
    root = json_parse_string(json);
    comps = json_object_get_object(json_value_get_object(root), "components");
    schemas = json_object_get_object(comps, "schemas");
    model = json_object_get_object(schemas, "MyModel");
    props = json_object_get_object(model, "properties");
    id_prop = json_object_get_object(props, "id");

    ASSERT_STR_EQ("integer", json_object_get_string(id_prop, "type"));

    json_value_free(root);
  }

  free(json);
  struct_fields_free(&sf);
  g_fail_io_after = -1;
  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_OPENAPI_WRITER_HEADERS_H */
