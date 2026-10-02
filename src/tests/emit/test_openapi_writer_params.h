/**
 * @file test_openapi_writer_params.h
 * @brief Unit tests for OpenAPI Writer (params).
 */

#ifndef TEST_OPENAPI_WRITER_PARAMS_H
#define TEST_OPENAPI_WRITER_PARAMS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "emit/test_openapi_writer_common.h"
/* clang-format on */

TEST test_writer_parameter_styles(void) {
  int rc = 0;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Path path;
  char *json;
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Parameter param = {0};
  json = NULL;

  setup_test_spec(&spec, &path, &op, &param, NULL);
  /* Configure Advanced Params */
  param.in = OA_PARAM_IN_QUERY;
  param.style = OA_STYLE_FORM;
  param.explode = 1;

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

    {
      JSON_Array *oa_params;
      JSON_Object *p_obj;
      oa_params = json_object_get_array(op_obj, "parameters");
      p_obj = json_array_get_object(oa_params, 0);
      ASSERT_STR_EQ("form", json_object_get_string(p_obj, "style"));
      ASSERT_EQ(1, json_object_get_boolean(p_obj, "explode"));
    }
    json_value_free(root);
  }
  free(json);
  g_fail_io_after = -1;
  PASS();
}

TEST test_writer_parameter_explode_false(void) {
  int rc = 0;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Path path;
  char *json;
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Parameter param = {0};
  json = NULL;

  setup_test_spec(&spec, &path, &op, &param, NULL);
  param.in = OA_PARAM_IN_QUERY;
  param.style = OA_STYLE_FORM;
  param.explode_set = 1;
  param.explode = 0;

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

    {
      JSON_Array *oa_params;
      JSON_Object *p_obj;
      oa_params = json_object_get_array(op_obj, "parameters");
      p_obj = json_array_get_object(oa_params, 0);
      ASSERT(json_object_has_value(p_obj, "explode"));
      ASSERT_EQ(0, json_object_get_boolean(p_obj, "explode"));
    }
    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

TEST test_writer_parameter_style_matrix(void) {
  int rc = 0;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Path path;
  char *json;
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Parameter param = {0};
  json = NULL;

  setup_test_spec(&spec, &path, &op, &param, NULL);
  param.in = OA_PARAM_IN_PATH;
  param.style = OA_STYLE_MATRIX;

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

    {
      JSON_Array *oa_params;
      JSON_Object *p_obj;
      oa_params = json_object_get_array(op_obj, "parameters");
      p_obj = json_array_get_object(oa_params, 0);
      ASSERT_STR_EQ("matrix", json_object_get_string(p_obj, "style"));
    }
    json_value_free(root);
  }
  free(json);
  g_fail_io_after = -1;
  PASS();
}

TEST test_writer_parameter_content_any(void) {
  int rc = 0;
  char *json;
  struct OpenAPI_Spec spec = {0};
  struct OpenAPI_Path path = {0};
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Parameter param = {0};
  json = NULL;

  spec.paths = &path;
  spec.n_paths = 1;
  path.route = (char *)(size_t)(size_t) "/headers";
  path.operations = &op;
  path.n_operations = 1;
  op.verb = OA_VERB_GET;
  op.operation_id = (char *)(size_t)(size_t) "getHeader";
  op.parameters = &param;
  op.n_parameters = 1;
  param.name = (char *)(size_t)(size_t) "X-Foo";
  param.in = OA_PARAM_IN_HEADER;
  param.type = (char *)(size_t)(size_t) "string";
  param.content_type = (char *)(size_t)(size_t) "text/plain";

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root;
    JSON_Object *op_obj;
    root = json_parse_string(json);
    op_obj = json_value_get_object(root);
    op_obj = json_object_get_object(op_obj, "paths");
    op_obj = json_object_get_object(op_obj, "/headers");
    op_obj = json_object_get_object(op_obj, "get");
    {
      JSON_Array *oa_params;
      JSON_Object *p_obj;
      JSON_Object *content;
      JSON_Object *media;
      JSON_Object *schema;
      oa_params = json_object_get_array(op_obj, "parameters");
      p_obj = json_array_get_object(oa_params, 0);
      content = json_object_get_object(p_obj, "content");
      media = json_object_get_object(content, "text/plain");
      schema = json_object_get_object(media, "schema");
      ASSERT_STR_EQ("header", json_object_get_string(p_obj, "in"));
      ASSERT(json_object_get_object(p_obj, "schema") == NULL);
      ASSERT_STR_EQ("string", json_object_get_string(schema, "type"));
    }

    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

TEST test_writer_parameter_and_header_content_media_type(void) {
  int rc = 0;
  char *json;
  struct OpenAPI_Spec spec = {0};
  struct OpenAPI_Path path = {0};
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Parameter param = {0};
  struct OpenAPI_Response resp = {0};
  struct OpenAPI_Header header = {0};
  struct OpenAPI_MediaType param_media = {0};
  struct OpenAPI_MediaType header_media = {0};
  struct OpenAPI_Encoding enc = {0};
  json = NULL;

  spec.paths = &path;
  spec.n_paths = 1;
  path.route = (char *)(size_t)(size_t) "/content";
  path.operations = &op;
  path.n_operations = 1;
  op.verb = OA_VERB_GET;
  op.operation_id = (char *)(size_t)(size_t) "getContent";
  op.parameters = &param;
  op.n_parameters = 1;
  param.name = (char *)(size_t)(size_t) "filter";
  param.in = OA_PARAM_IN_QUERY;
  param.content_media_types = &param_media;
  param.n_content_media_types = 1;
  param_media.name =
      (char *)(size_t)(size_t) "application/x-www-form-urlencoded";
  param_media.schema_set = 1;
  param_media.schema.inline_type = (char *)(size_t)(size_t) "object";
  param_media.encoding = &enc;
  param_media.n_encoding = 1;
  enc.name = (char *)(size_t)(size_t) "id";
  enc.content_type = (char *)(size_t)(size_t) "text/plain";
  enc.style = OA_STYLE_FORM;
  enc.style_set = 1;
  enc.explode = 1;
  enc.explode_set = 1;

  op.responses = &resp;
  op.n_responses = 1;
  resp.code = (char *)(size_t)(size_t) "200";
  resp.description = (char *)(size_t)(size_t) "ok";
  resp.headers = &header;
  resp.n_headers = 1;
  header.name = (char *)(size_t)(size_t) "X-Rate";
  header.content_media_types = &header_media;
  header.n_content_media_types = 1;
  header_media.name = (char *)(size_t)(size_t) "text/plain";
  header_media.schema_set = 1;
  header_media.schema.inline_type = (char *)(size_t)(size_t) "string";

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root;
    JSON_Object *p_item;
    JSON_Object *op_obj;
    JSON_Array *params;
    JSON_Object *p0;
    JSON_Object *content;
    JSON_Object *encoding;
    JSON_Object *enc_id;
    JSON_Object *paths;
    JSON_Object *media;
    root = json_parse_string(json);
    paths = json_object_get_object(json_value_get_object(root), "paths");
    p_item = json_object_get_object(paths, "/content");
    op_obj = json_object_get_object(p_item, "get");
    params = json_object_get_array(op_obj, "parameters");
    p0 = json_array_get_object(params, 0);
    content = json_object_get_object(p0, "content");
    media =
        json_object_get_object(content, "application/x-www-form-urlencoded");
    encoding = json_object_get_object(media, "encoding");
    enc_id = json_object_get_object(encoding, "id");

    ASSERT(enc_id != NULL);
    ASSERT_STR_EQ("text/plain", json_object_get_string(enc_id, "contentType"));
    ASSERT_STR_EQ("form", json_object_get_string(enc_id, "style"));
    ASSERT_EQ(1, json_object_get_boolean(enc_id, "explode"));

    {
      JSON_Object *responses;
      JSON_Object *r200;
      JSON_Object *headers;
      JSON_Object *x_rate;
      JSON_Object *h_content;
      JSON_Object *h_media;
      JSON_Object *schema;
      responses = json_object_get_object(op_obj, "responses");
      r200 = json_object_get_object(responses, "200");
      headers = json_object_get_object(r200, "headers");
      x_rate = json_object_get_object(headers, "X-Rate");
      h_content = json_object_get_object(x_rate, "content");
      h_media = json_object_get_object(h_content, "text/plain");
      schema = json_object_get_object(h_media, "schema");
      ASSERT_STR_EQ("string", json_object_get_string(schema, "type"));
    }

    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

TEST test_writer_parameter_examples_object(void) {
  int rc = 0;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Path path;
  struct OpenAPI_Example ex;
  char *json;
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Parameter param = {0};
  json = NULL;

  memset(&ex, 0, sizeof(ex));
  setup_test_spec(&spec, &path, &op, &param, NULL);
  ex.name = (char *)(size_t)(size_t) "basic";
  ex.data_value.type = OA_ANY_STRING;
  ex.data_value.string = (char *)(size_t)(size_t) "hello";
  ex.data_value_set = 1;
  param.examples = &ex;
  param.n_examples = 1;
  param.example_location = OA_EXAMPLE_LOC_OBJECT;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root;
    JSON_Object *p_item;
    JSON_Object *op_obj;
    JSON_Array *params;
    JSON_Object *p0;
    JSON_Object *examples;
    JSON_Object *basic;
    JSON_Object *paths;
    root = json_parse_string(json);
    paths = json_object_get_object(json_value_get_object(root), "paths");
    p_item = json_object_get_object(paths, "/test/route");
    op_obj = json_object_get_object(p_item, "get");
    params = json_object_get_array(op_obj, "parameters");
    p0 = json_array_get_object(params, 0);
    examples = json_object_get_object(p0, "examples");
    basic = json_object_get_object(examples, "basic");

    ASSERT(basic != NULL);
    ASSERT_STR_EQ("hello", json_object_get_string(basic, "dataValue"));

    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

TEST test_writer_parameter_examples_media(void) {
  int rc = 0;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Path path;
  char *json;
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Parameter param = {0};
  json = NULL;

  setup_test_spec(&spec, &path, &op, &param, NULL);
  param.content_type = (char *)(size_t)(size_t) "application/json";
  param.example.type = OA_ANY_STRING;
  param.example.string = (char *)(size_t)(size_t) "hi";
  param.example_set = 1;
  param.example_location = OA_EXAMPLE_LOC_MEDIA;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root;
    JSON_Object *p_item;
    JSON_Object *op_obj;
    JSON_Array *params;
    JSON_Object *p0;
    JSON_Object *content;
    JSON_Object *media;
    JSON_Object *paths;
    root = json_parse_string(json);
    paths = json_object_get_object(json_value_get_object(root), "paths");
    p_item = json_object_get_object(paths, "/test/route");
    op_obj = json_object_get_object(p_item, "get");
    params = json_object_get_array(op_obj, "parameters");
    p0 = json_array_get_object(params, 0);
    content = json_object_get_object(p0, "content");
    media = json_object_get_object(content, "application/json");

    ASSERT_STR_EQ("hi", json_object_get_string(media, "example"));

    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

TEST test_writer_component_examples(void) {
  int rc = 0;
  struct OpenAPI_Example ex;
  char *json;
  char *names[1];
  struct OpenAPI_Spec spec = {0};
  json = NULL;

  memset(&ex, 0, sizeof(ex));
  names[0] = (char *)(size_t)(size_t) "ex1";
  spec.component_examples = &ex;
  spec.component_example_names = names;
  spec.n_component_examples = 1;
  ex.summary = (char *)(size_t)(size_t) "Example";
  ex.value.type = OA_ANY_STRING;
  ex.value.string = (char *)(size_t)(size_t) "v";
  ex.value_set = 1;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root;
    JSON_Object *examples;
    JSON_Object *ex_obj;
    JSON_Object *comps;
    root = json_parse_string(json);
    comps = json_object_get_object(json_value_get_object(root), "components");
    examples = json_object_get_object(comps, "examples");
    ex_obj = json_object_get_object(examples, "ex1");
    ASSERT_STR_EQ("Example", json_object_get_string(ex_obj, "summary"));
    ASSERT_STR_EQ("v", json_object_get_string(ex_obj, "value"));
    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

TEST test_writer_oauth2_flows(void) {
  int rc = 0;
  struct OpenAPI_SecurityScheme scheme;
  struct OpenAPI_OAuthFlow flow;
  struct OpenAPI_OAuthScope scope;
  char *json;
  struct OpenAPI_Spec spec = {0};
  json = NULL;

  memset(&scheme, 0, sizeof(scheme));
  memset(&flow, 0, sizeof(flow));
  memset(&scope, 0, sizeof(scope));

  scope.name = (char *)(size_t)(size_t) "read";
  scope.description = (char *)(size_t)(size_t) "Read";
  flow.type = OA_OAUTH_FLOW_PASSWORD;
  flow.token_url = (char *)(size_t)(size_t) "https://token.example.com";
  flow.scopes = &scope;
  flow.n_scopes = 1;
  scheme.name = (char *)(size_t)(size_t) "oauth";
  scheme.type = OA_SEC_OAUTH2;
  scheme.flows = &flow;
  scheme.n_flows = 1;

  spec.security_schemes = &scheme;
  spec.n_security_schemes = 1;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root;
    JSON_Object *schemes;
    JSON_Object *oauth;
    JSON_Object *flows;
    JSON_Object *password;
    JSON_Object *scopes;
    JSON_Object *comps;
    root = json_parse_string(json);
    comps = json_object_get_object(json_value_get_object(root), "components");
    schemes = json_object_get_object(comps, "securitySchemes");
    oauth = json_object_get_object(schemes, "oauth");
    flows = json_object_get_object(oauth, "flows");
    password = json_object_get_object(flows, "password");
    scopes = json_object_get_object(password, "scopes");

    ASSERT_STR_EQ("https://token.example.com",
                  json_object_get_string(password, "tokenUrl"));
    ASSERT_STR_EQ("Read", json_object_get_string(scopes, "read"));
    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

TEST test_writer_servers(void) {
  int rc = 0;
  struct OpenAPI_Server servers[1];
  char *json;
  struct OpenAPI_Spec spec = {0};
  json = NULL;

  memset(servers, 0, sizeof(servers));
  servers[0].url = (char *)(size_t)(size_t) "https://api.example.com";
  servers[0].description = (char *)(size_t)(size_t) "Prod";
  servers[0].name = (char *)(size_t)(size_t) "prod";

  spec.servers = servers;
  spec.n_servers = 1;
  spec.openapi_version = (char *)(size_t)(size_t) "3.1.2";

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root;
    JSON_Object *root_obj;
    JSON_Array *srv_arr;
    JSON_Object *srv;
    root = json_parse_string(json);
    root_obj = json_value_get_object(root);
    srv_arr = json_object_get_array(root_obj, "servers");
    ASSERT_STR_EQ("3.1.2", json_object_get_string(root_obj, "openapi"));
    srv = json_array_get_object(srv_arr, 0);

    ASSERT_STR_EQ("https://api.example.com",
                  json_object_get_string(srv, "url"));
    ASSERT_STR_EQ("Prod", json_object_get_string(srv, "description"));
    ASSERT_STR_EQ("prod", json_object_get_string(srv, "name"));
    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

TEST test_writer_querystring_param(void) {
  int rc = 0;
  char *json;
  struct OpenAPI_Spec spec = {0};
  struct OpenAPI_Path path = {0};
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Parameter param = {0};
  json = NULL;

  spec.paths = &path;
  spec.n_paths = 1;
  path.route = (char *)(size_t)(size_t) "/search";
  path.operations = &op;
  path.n_operations = 1;
  op.verb = OA_VERB_GET;
  op.operation_id = (char *)(size_t)(size_t) "search";
  op.parameters = &param;
  op.n_parameters = 1;
  param.name = (char *)(size_t)(size_t) "qs";
  param.in = OA_PARAM_IN_QUERYSTRING;
  param.type = (char *)(size_t)(size_t) "string";
  param.content_type =
      (char *)(size_t)(size_t) "application/x-www-form-urlencoded";

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root;
    JSON_Object *op_obj;
    root = json_parse_string(json);
    op_obj = json_value_get_object(root);
    op_obj = json_object_get_object(op_obj, "paths");
    op_obj = json_object_get_object(op_obj, "/search");
    op_obj = json_object_get_object(op_obj, "get");
    {
      JSON_Array *oa_params;
      JSON_Object *p_obj;
      JSON_Object *content;
      JSON_Object *schema;
      JSON_Object *media;
      oa_params = json_object_get_array(op_obj, "parameters");
      p_obj = json_array_get_object(oa_params, 0);
      content = json_object_get_object(p_obj, "content");
      media =
          json_object_get_object(content, "application/x-www-form-urlencoded");
      schema = json_object_get_object(media, "schema");
      ASSERT_STR_EQ("querystring", json_object_get_string(p_obj, "in"));
      ASSERT(json_object_get_object(p_obj, "schema") == NULL);
      ASSERT_STR_EQ("string", json_object_get_string(schema, "type"));
    }

    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

TEST test_writer_ignores_reserved_header_params(void) {
  int rc = 0;
  char *json;
  struct OpenAPI_Spec spec = {0};
  struct OpenAPI_Path path = {0};
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Parameter params[2] = {{0}};
  json = NULL;

  spec.paths = &path;
  spec.n_paths = 1;
  path.route = (char *)(size_t)(size_t) "/h";
  path.operations = &op;
  path.n_operations = 1;
  op.verb = OA_VERB_GET;
  op.operation_id = (char *)(size_t)(size_t) "getH";
  op.parameters = params;
  op.n_parameters = 2;

  params[0].name = (char *)(size_t)(size_t) "Accept";
  params[0].in = OA_PARAM_IN_HEADER;
  params[0].type = (char *)(size_t)(size_t) "string";

  params[1].name = (char *)(size_t)(size_t) "q";
  params[1].in = OA_PARAM_IN_QUERY;
  params[1].type = (char *)(size_t)(size_t) "string";

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root;
    JSON_Object *op_obj;
    JSON_Array *oa_params;
    JSON_Object *p_obj;
    root = json_parse_string(json);
    op_obj = json_value_get_object(root);
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
  g_fail_io_after = -1;
  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_OPENAPI_WRITER_PARAMS_H */
