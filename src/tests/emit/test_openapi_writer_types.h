/**
 * @file test_openapi_writer_types.h
 * @brief Unit tests for OpenAPI Writer (types).
 */

#ifndef TEST_OPENAPI_WRITER_TYPES_H
#define TEST_OPENAPI_WRITER_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "emit/test_openapi_writer_common.h"
/* clang-format on */

TEST test_writer_schema_items_type_union(void) {
  int rc = 0;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Path path;
  char *json;
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Parameter param = {0};
  char *types[] = {(char *)(size_t)(size_t) "string",
                   (char *)(size_t)(size_t) "integer"};
  json = NULL;

  setup_test_spec(&spec, &path, &op, &param, NULL);
  param.schema_set = 1;
  param.schema.is_array = 1;
  param.schema.inline_type = (char *)(size_t)(size_t) "string";
  param.schema.items_type_union = types;
  param.schema.n_items_type_union = 2;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root;
    JSON_Object *p_item;
    JSON_Object *p0;
    JSON_Object *schema;
    JSON_Object *items;
    JSON_Object *paths;
    JSON_Array *params;
    JSON_Array *type_arr;
    root = json_parse_string(json);
    paths = json_object_get_object(json_value_get_object(root), "paths");
    p_item = json_object_get_object(paths, "/test/route");
    params = json_object_get_array(json_object_get_object(p_item, "get"),
                                   "parameters");
    p0 = json_array_get_object(params, 0);
    schema = json_object_get_object(p0, "schema");
    items = json_object_get_object(schema, "items");
    type_arr = json_value_get_array(json_object_get_value(items, "type"));
    ASSERT(type_arr != NULL);
    ASSERT_EQ(2, json_array_get_count(type_arr));
    ASSERT_STR_EQ("string", json_array_get_string(type_arr, 0));
    ASSERT_STR_EQ("integer", json_array_get_string(type_arr, 1));
    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

TEST test_writer_schema_boolean(void) {
  int rc = 0;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Path path;
  char *json;
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Parameter param = {0};
  json = NULL;

  setup_test_spec(&spec, &path, &op, &param, NULL);
  param.schema_set = 1;
  param.schema.schema_is_boolean = 1;
  param.schema.schema_boolean_value = 1;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root;
    JSON_Object *p_item;
    JSON_Object *p0;
    JSON_Value *schema_val;
    JSON_Object *paths;
    JSON_Array *params;
    root = json_parse_string(json);
    paths = json_object_get_object(json_value_get_object(root), "paths");
    p_item = json_object_get_object(paths, "/test/route");
    params = json_object_get_array(json_object_get_object(p_item, "get"),
                                   "parameters");
    p0 = json_array_get_object(params, 0);
    schema_val = json_object_get_value(p0, "schema");
    ASSERT(schema_val != NULL);
    ASSERT_EQ(JSONBoolean, json_value_get_type(schema_val));
    ASSERT_EQ(1, json_value_get_boolean(schema_val));
    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

TEST test_writer_schema_numeric_enum(void) {
  int rc = 0;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Path path;
  char *json;
  struct OpenAPI_Any enum_vals[2];
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Parameter param = {0};
  json = NULL;

  setup_test_spec(&spec, &path, &op, &param, NULL);
  param.schema_set = 1;
  param.schema.inline_type = (char *)(size_t)(size_t) "integer";
  enum_vals[0].type = OA_ANY_NUMBER;
  enum_vals[0].number = 1.0;
  enum_vals[1].type = OA_ANY_NUMBER;
  enum_vals[1].number = 2.0;
  param.schema.enum_values = enum_vals;
  param.schema.n_enum_values = 2;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root;
    JSON_Object *p_item;
    JSON_Object *p0;
    JSON_Object *schema;
    JSON_Array *enum_arr;
    JSON_Object *paths;
    JSON_Array *params;
    root = json_parse_string(json);
    paths = json_object_get_object(json_value_get_object(root), "paths");
    p_item = json_object_get_object(paths, "/test/route");
    params = json_object_get_array(json_object_get_object(p_item, "get"),
                                   "parameters");
    p0 = json_array_get_object(params, 0);
    schema = json_object_get_object(p0, "schema");
    enum_arr = json_object_get_array(schema, "enum");
    ASSERT(enum_arr != NULL);
    ASSERT_EQ(2, json_array_get_count(enum_arr));
    ASSERT_EQ(1.0, json_array_get_number(enum_arr, 0));
    ASSERT_EQ(2.0, json_array_get_number(enum_arr, 1));
    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

TEST test_writer_schema_items_examples(void) {
  int rc = 0;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Path path;
  char *json;
  struct OpenAPI_Any item_examples[2];
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Parameter param = {0};
  json = NULL;

  setup_test_spec(&spec, &path, &op, &param, NULL);
  param.schema_set = 1;
  param.schema.is_array = 1;
  param.schema.inline_type = (char *)(size_t)(size_t) "string";
  item_examples[0].type = OA_ANY_STRING;
  item_examples[0].string = (char *)(size_t)(size_t) "a";
  item_examples[1].type = OA_ANY_STRING;
  item_examples[1].string = (char *)(size_t)(size_t) "b";
  param.schema.items_examples = item_examples;
  param.schema.n_items_examples = 2;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root;
    JSON_Object *p_item;
    JSON_Object *p0;
    JSON_Object *schema;
    JSON_Object *items;
    JSON_Array *examples;
    JSON_Object *paths;
    JSON_Array *params;
    root = json_parse_string(json);
    paths = json_object_get_object(json_value_get_object(root), "paths");
    p_item = json_object_get_object(paths, "/test/route");
    params = json_object_get_array(json_object_get_object(p_item, "get"),
                                   "parameters");
    p0 = json_array_get_object(params, 0);
    schema = json_object_get_object(p0, "schema");
    items = json_object_get_object(schema, "items");
    examples = json_object_get_array(items, "examples");
    ASSERT(examples != NULL);
    ASSERT_EQ(2, json_array_get_count(examples));
    ASSERT_STR_EQ("a", json_array_get_string(examples, 0));
    ASSERT_STR_EQ("b", json_array_get_string(examples, 1));
    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

TEST test_writer_schema_items_boolean(void) {
  int rc = 0;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Path path;
  char *json;
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Parameter param = {0};
  json = NULL;

  setup_test_spec(&spec, &path, &op, &param, NULL);
  param.schema_set = 1;
  param.schema.is_array = 1;
  param.schema.items_schema_is_boolean = 1;
  param.schema.items_schema_boolean_value = 0;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root;
    JSON_Object *p_item;
    JSON_Object *p0;
    JSON_Object *schema;
    JSON_Value *items_val;
    JSON_Object *paths;
    JSON_Array *params;
    root = json_parse_string(json);
    paths = json_object_get_object(json_value_get_object(root), "paths");
    p_item = json_object_get_object(paths, "/test/route");
    params = json_object_get_array(json_object_get_object(p_item, "get"),
                                   "parameters");
    p0 = json_array_get_object(params, 0);
    schema = json_object_get_object(p0, "schema");
    items_val = json_object_get_value(schema, "items");
    ASSERT(items_val != NULL);
    ASSERT_EQ(JSONBoolean, json_value_get_type(items_val));
    ASSERT_EQ(0, json_value_get_boolean(items_val));
    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

TEST test_writer_schema_example_and_numeric_constraints(void) {
  int rc = 0;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Path path;
  char *json;
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Parameter param = {0};
  json = NULL;

  setup_test_spec(&spec, &path, &op, &param, NULL);
  param.schema_set = 1;
  param.schema.inline_type = (char *)(size_t)(size_t) "number";
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
    JSON_Value *root;
    JSON_Object *p_item;
    JSON_Object *p0;
    JSON_Object *schema;
    JSON_Object *paths;
    JSON_Array *params;
    root = json_parse_string(json);
    paths = json_object_get_object(json_value_get_object(root), "paths");
    p_item = json_object_get_object(paths, "/test/route");
    params = json_object_get_array(json_object_get_object(p_item, "get"),
                                   "parameters");
    p0 = json_array_get_object(params, 0);
    schema = json_object_get_object(p0, "schema");
    ASSERT_EQ(1.0, json_object_get_number(schema, "minimum"));
    ASSERT_EQ(9.0, json_object_get_number(schema, "exclusiveMaximum"));
    ASSERT_EQ(2.5, json_object_get_number(schema, "example"));
    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

TEST test_writer_schema_array_constraints_and_items_example(void) {
  int rc = 0;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Path path;
  char *json;
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Parameter param = {0};
  json = NULL;

  setup_test_spec(&spec, &path, &op, &param, NULL);
  param.schema_set = 1;
  param.schema.is_array = 1;
  param.schema.inline_type = (char *)(size_t)(size_t) "string";
  param.schema.has_min_items = 1;
  param.schema.min_items = 1;
  param.schema.has_max_items = 1;
  param.schema.max_items = 3;
  param.schema.unique_items = 1;
  param.schema.items_has_min_len = 1;
  param.schema.items_min_len = 2;
  param.schema.items_has_max_len = 1;
  param.schema.items_max_len = 5;
  param.schema.items_pattern = (char *)(size_t)(size_t) "^[a-z]+$";
  param.schema.items_example_set = 1;
  param.schema.items_example.type = OA_ANY_STRING;
  param.schema.items_example.string = (char *)(size_t)(size_t) "ab";

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root;
    JSON_Object *p_item;
    JSON_Object *p0;
    JSON_Object *schema;
    JSON_Object *items;
    JSON_Object *paths;
    JSON_Array *params;
    root = json_parse_string(json);
    paths = json_object_get_object(json_value_get_object(root), "paths");
    p_item = json_object_get_object(paths, "/test/route");
    params = json_object_get_array(json_object_get_object(p_item, "get"),
                                   "parameters");
    p0 = json_array_get_object(params, 0);
    schema = json_object_get_object(p0, "schema");
    items = json_object_get_object(schema, "items");
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
  g_fail_io_after = -1;
  PASS();
}

TEST test_writer_inline_schema_items_const_default_and_extras(void) {
  int rc = 0;
  const char *json =
      "{\"paths\":{\"/"
      "q\":{\"get\":{\"parameters\":[{\"name\":\"tags\",\"in\":\"query\","
      "\"schema\":{\"type\":\"array\",\"x-top\":true,\"items\":{\"type\":"
      "\"string\",\"const\":\"x\",\"default\":\"y\",\"x-custom\":99}}}],"
      "\"responses\":{\"200\":{\"description\":\"OK\"}}}}},\"openapi\":\"3.2."
      "0\"}";

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
    JSON_Object *paths;
    JSON_Object *path;
    JSON_Object *get;
    JSON_Array *params;
    JSON_Object *param0;
    JSON_Object *schema;
    JSON_Object *items;
    root = json_parse_string(out_json);
    root_obj = json_value_get_object(root);
    paths = json_object_get_object(root_obj, "paths");
    path = json_object_get_object(paths, "/q");
    get = json_object_get_object(path, "get");
    params = json_object_get_array(get, "parameters");
    param0 = json_array_get_object(params, 0);
    schema = json_object_get_object(param0, "schema");
    items = json_object_get_object(schema, "items");

    ASSERT_EQ(1, json_object_get_boolean(schema, "x-top"));
    ASSERT_STR_EQ("x", json_object_get_string(items, "const"));
    ASSERT_STR_EQ("y", json_object_get_string(items, "default"));
    ASSERT_EQ(99, (int)json_object_get_number(items, "x-custom"));

    json_value_free(root);
  }

  free(out_json);
  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

TEST test_writer_input_validation(void) {
  struct OpenAPI_Spec spec = {0};
  char *json;

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            openapi_write_spec_to_json(NULL, &json));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            openapi_write_spec_to_json(&spec, NULL));
  g_fail_io_after = -1;

  PASS();
}

TEST test_writer_extensions_non_schema(void) {
  int rc = 0;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Path path;
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
  char *rb_names[1];
  char *json;
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Parameter param = {0};
  struct OpenAPI_Response resp = {0};
  struct OpenAPI_RequestBody comp_rb = {0};
  json = NULL;

  setup_test_spec(&spec, &path, &op, &param, &resp);

  spec.extensions_json = (char *)(size_t)(size_t) "{\"x-root\":1}";
  spec.info.title = (char *)(size_t)(size_t) "Spec";
  spec.info.version = (char *)(size_t)(size_t) "1";
  spec.info.extensions_json = (char *)(size_t)(size_t) "{\"x-info\":\"info\"}";
  spec.info.contact.name = (char *)(size_t)(size_t) "Support";
  spec.info.contact.extensions_json =
      (char *)(size_t)(size_t) "{\"x-contact\":true}";
  spec.info.license.name = (char *)(size_t)(size_t) "MIT";
  spec.info.license.extensions_json =
      (char *)(size_t)(size_t) "{\"x-license\":\"lic\"}";
  spec.external_docs.url = (char *)(size_t)(size_t) "https://example.com";
  spec.external_docs.extensions_json =
      (char *)(size_t)(size_t) "{\"x-ext\":\"ext\"}";

  path.extensions_json = (char *)(size_t)(size_t) "{\"x-path\":6}";
  op.extensions_json = (char *)(size_t)(size_t) "{\"x-op\":7}";
  op.responses_extensions_json = (char *)(size_t)(size_t) "{\"x-responses\":1}";
  op.req_body.inline_type = (char *)(size_t)(size_t) "string";
  op.req_body.content_type = (char *)(size_t)(size_t) "application/json";
  op.req_body_required_set = 1;
  op.req_body_required = 1;
  op.req_body_extensions_json = (char *)(size_t)(size_t) "{\"x-rb-op\":true}";
  param.extensions_json = (char *)(size_t)(size_t) "{\"x-param\":\"param\"}";
  resp.extensions_json = (char *)(size_t)(size_t) "{\"x-resp\":true}";

  memset(&cb, 0, sizeof(cb));
  memset(&cb_path, 0, sizeof(cb_path));
  memset(&cb_op, 0, sizeof(cb_op));
  memset(&cb_resp, 0, sizeof(cb_resp));
  op.callbacks = &cb;
  op.n_callbacks = 1;
  cb.name = (char *)(size_t)(size_t) "onEvent";
  cb.extensions_json = (char *)(size_t)(size_t) "{\"x-cb\":\"cb\"}";
  cb.paths = &cb_path;
  cb.n_paths = 1;
  cb_path.route = (char *)(size_t)(size_t) "{$request.body#/url}";
  cb_path.operations = &cb_op;
  cb_path.n_operations = 1;
  cb_op.verb = OA_VERB_POST;
  cb_op.responses = &cb_resp;
  cb_op.n_responses = 1;
  cb_resp.code = (char *)(size_t)(size_t) "200";
  cb_resp.description = (char *)(size_t)(size_t) "ok";

  memset(&tag, 0, sizeof(tag));
  tag.name = (char *)(size_t)(size_t) "pet";
  tag.extensions_json = (char *)(size_t)(size_t) "{\"x-tag\":\"tag\"}";
  spec.tags = &tag;
  spec.n_tags = 1;

  memset(&scheme, 0, sizeof(scheme));
  memset(&flow, 0, sizeof(flow));
  memset(&scope, 0, sizeof(scope));
  scope.name = (char *)(size_t)(size_t) "read";
  scope.description = (char *)(size_t)(size_t) "Read";
  flow.type = OA_OAUTH_FLOW_PASSWORD;
  flow.token_url = (char *)(size_t)(size_t) "https://token.example.com";
  flow.scopes = &scope;
  flow.n_scopes = 1;
  flow.extensions_json = (char *)(size_t)(size_t) "{\"x-flow\":1}";
  scheme.name = (char *)(size_t)(size_t) "oauth";
  scheme.type = OA_SEC_OAUTH2;
  scheme.flows = &flow;
  scheme.n_flows = 1;
  scheme.extensions_json = (char *)(size_t)(size_t) "{\"x-sec\":1}";
  spec.security_schemes = &scheme;
  spec.n_security_schemes = 1;

  memset(&sec_req, 0, sizeof(sec_req));
  memset(&sec_set, 0, sizeof(sec_set));
  sec_req.scheme = (char *)(size_t)(size_t) "oauth";
  sec_set.requirements = &sec_req;
  sec_set.n_requirements = 1;
  sec_set.extensions_json = (char *)(size_t)(size_t) "{\"x-sec-req\":1}";
  spec.security = &sec_set;
  spec.n_security = 1;
  spec.security_set = 1;

  memset(&comp_rb, 0, sizeof(comp_rb));
  rb_names[0] = (char *)(size_t)(size_t) "CompRB";
  comp_rb.description = (char *)(size_t)(size_t) "desc";
  comp_rb.schema.inline_type = (char *)(size_t)(size_t) "string";
  comp_rb.extensions_json = (char *)(size_t)(size_t) "{\"x-rb\":1}";
  spec.component_request_bodies = &comp_rb;
  spec.component_request_body_names = rb_names;
  spec.n_component_request_bodies = 1;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root;
    JSON_Object *root_obj;
    JSON_Object *info_obj;
    JSON_Object *contact_obj;
    JSON_Object *license_obj;
    JSON_Array *tags_arr;
    JSON_Object *tag_obj;
    JSON_Object *paths;
    JSON_Object *path_obj;
    JSON_Object *op_obj;
    JSON_Object *rb_op_obj;
    JSON_Array *params_arr;
    JSON_Object *param_obj;
    JSON_Object *responses;
    JSON_Object *resp_obj;
    JSON_Object *callbacks;
    JSON_Object *cb_obj;
    JSON_Object *components;
    JSON_Object *sec;
    JSON_Object *scheme_obj;
    JSON_Object *flows;
    JSON_Object *flow_obj;
    JSON_Object *rbs;
    JSON_Object *rb_obj;
    JSON_Array *security_arr;
    JSON_Object *security_obj;
    JSON_Object *ext_docs_obj;
    root = json_parse_string(json);
    root_obj = json_value_get_object(root);
    info_obj = json_object_get_object(root_obj, "info");
    contact_obj = json_object_get_object(info_obj, "contact");
    license_obj = json_object_get_object(info_obj, "license");
    ext_docs_obj = json_object_get_object(root_obj, "externalDocs");
    tags_arr = json_object_get_array(root_obj, "tags");
    tag_obj = json_array_get_object(tags_arr, 0);
    paths = json_object_get_object(root_obj, "paths");
    path_obj = json_object_get_object(paths, "/test/route");
    op_obj = json_object_get_object(path_obj, "get");
    rb_op_obj = json_object_get_object(op_obj, "requestBody");
    params_arr = json_object_get_array(op_obj, "parameters");
    param_obj = json_array_get_object(params_arr, 0);
    responses = json_object_get_object(op_obj, "responses");
    resp_obj = json_object_get_object(responses, "200");
    callbacks = json_object_get_object(op_obj, "callbacks");
    cb_obj = json_object_get_object(callbacks, "onEvent");
    components = json_object_get_object(root_obj, "components");
    sec = json_object_get_object(components, "securitySchemes");
    scheme_obj = json_object_get_object(sec, "oauth");
    flows = json_object_get_object(scheme_obj, "flows");
    flow_obj = json_object_get_object(flows, "password");
    rbs = json_object_get_object(components, "requestBodies");
    rb_obj = json_object_get_object(rbs, "CompRB");
    security_arr = json_object_get_array(root_obj, "security");
    security_obj = json_array_get_object(security_arr, 0);

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
  g_fail_io_after = -1;
  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_OPENAPI_WRITER_TYPES_H */
