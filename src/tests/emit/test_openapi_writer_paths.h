/**
 * @file test_openapi_writer_paths.h
 * @brief Unit tests for OpenAPI Writer (paths).
 */

#ifndef TEST_OPENAPI_WRITER_PATHS_H
#define TEST_OPENAPI_WRITER_PATHS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "emit/test_openapi_writer_common.h"
/* clang-format on */

/**
 * @brief Unit test test_writer_component_path_items.
 * @return Greatest test result.
 */
TEST test_writer_component_path_items(void) {
  int rc = 0;
  char *path_item_names[1];
  char *json;
  struct OpenAPI_Spec spec = {0};
  struct OpenAPI_Path path_item = {0};
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Response resp = {0};
  json = NULL;

  path_item.route = cdd_test_mut_str("FooItem");
  path_item.summary = cdd_test_mut_str("foo");
  path_item.operations = &op;
  path_item.n_operations = 1;

  op.verb = OA_VERB_GET;
  op.operation_id = cdd_test_mut_str("getFoo");
  op.responses = &resp;
  op.n_responses = 1;
  resp.code = cdd_test_mut_str("200");
  resp.description = cdd_test_mut_str("ok");

  path_item_names[0] = cdd_test_mut_str("FooItem");
  spec.component_path_items = &path_item;
  spec.component_path_item_names = path_item_names;
  spec.n_component_path_items = 1;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root;
    JSON_Object *obj;
    JSON_Object *comps;
    JSON_Object *path_items;
    JSON_Object *pi_obj;
    JSON_Object *get_op;
    {
      JSON_Value *_cdd_jv_1 = NULL;
      cdd_json_parse_string(json, &_cdd_jv_1);
      root = _cdd_jv_1;
    }
    obj = json_value_get_object(root);
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(obj, "components", &_cdd_jo_1);
      comps = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(comps, "pathItems", &_cdd_jo_1);
      path_items = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(path_items, "FooItem", &_cdd_jo_1);
      pi_obj = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(pi_obj, "get", &_cdd_jo_1);
      get_op = _cdd_jo_1;
    }

    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(pi_obj, "summary", &_cdd_js_1);
      ASSERT_STR_EQ("foo", _cdd_js_1);
    }
    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(get_op, "operationId", &_cdd_js_1);
      ASSERT_STR_EQ("getFoo", _cdd_js_1);
    }

    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_writer_response_links.
 * @return Greatest test result.
 */
TEST test_writer_response_links(void) {
  int rc = 0;
  struct OpenAPI_LinkParam params[2];
  char *json;
  struct OpenAPI_Spec spec = {0};
  struct OpenAPI_Path path = {0};
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Response resp = {0};
  struct OpenAPI_Link link = {0};
  struct OpenAPI_Server link_server = {0};
  json = NULL;

  memset(params, 0, sizeof(params));

  spec.paths = &path;
  spec.n_paths = 1;
  path.route = cdd_test_mut_str("/pets");
  path.operations = &op;
  path.n_operations = 1;
  op.verb = OA_VERB_GET;
  op.operation_id = cdd_test_mut_str("listPets");
  op.responses = &resp;
  op.n_responses = 1;

  resp.code = cdd_test_mut_str("200");
  resp.description = cdd_test_mut_str("ok");
  resp.links = &link;
  resp.n_links = 1;

  link.name = cdd_test_mut_str("next");
  link.operation_id = cdd_test_mut_str("listPets");
  link.parameters = params;
  link.n_parameters = 2;
  params[0].name = cdd_test_mut_str("limit");
  params[0].value.type = OA_ANY_NUMBER;
  params[0].value.number = 50;
  params[1].name = cdd_test_mut_str("offset");
  params[1].value.type = OA_ANY_STRING;
  params[1].value.string = cdd_test_mut_str("$response.body#/offset");

  link.request_body_set = 1;
  link.request_body.type = OA_ANY_STRING;
  link.request_body.string = cdd_test_mut_str("payload");

  link_server.url = cdd_test_mut_str("https://api.example.com");
  link.server = &link_server;
  link.server_set = 1;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root;
    JSON_Object *p_item;
    JSON_Object *op_obj;
    JSON_Object *resps;
    JSON_Object *resp_obj;
    JSON_Object *links;
    JSON_Object *link_obj;
    JSON_Object *params_obj;
    JSON_Object *srv_obj;
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
      cdd_json_object_get_object(paths, "/pets", &_cdd_jo_1);
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
      resps = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(resps, "200", &_cdd_jo_1);
      resp_obj = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(resp_obj, "links", &_cdd_jo_1);
      links = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(links, "next", &_cdd_jo_1);
      link_obj = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(link_obj, "parameters", &_cdd_jo_1);
      params_obj = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(link_obj, "server", &_cdd_jo_1);
      srv_obj = _cdd_jo_1;
    }

    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(link_obj, "operationId", &_cdd_js_1);
      ASSERT_STR_EQ("listPets", _cdd_js_1);
    }
    {
      double _cdd_jn_1 = 0.0;
      cdd_json_object_get_number(params_obj, "limit", &_cdd_jn_1);
      ASSERT_EQ(50, (int)_cdd_jn_1);
    }
    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(params_obj, "offset", &_cdd_js_1);
      ASSERT_STR_EQ("$response.body#/offset", _cdd_js_1);
    }
    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(link_obj, "requestBody", &_cdd_js_1);
      ASSERT_STR_EQ("payload", _cdd_js_1);
    }
    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(srv_obj, "url", &_cdd_js_1);
      ASSERT_STR_EQ("https://api.example.com", _cdd_js_1);
    }

    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_writer_callbacks.
 * @return Greatest test result.
 */
TEST test_writer_callbacks(void) {
  int rc = 0;
  char *json;
  struct OpenAPI_Spec spec = {0};
  struct OpenAPI_Path path = {0};
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Response resp = {0};
  struct OpenAPI_Callback cb = {0};
  struct OpenAPI_Path cb_path = {0};
  struct OpenAPI_Operation cb_op = {0};
  struct OpenAPI_Response cb_resp = {0};
  json = NULL;

  spec.paths = &path;
  spec.n_paths = 1;
  path.route = cdd_test_mut_str("/pets");
  path.operations = &op;
  path.n_operations = 1;
  op.verb = OA_VERB_GET;
  op.operation_id = cdd_test_mut_str("listPets");
  op.responses = &resp;
  op.n_responses = 1;
  resp.code = cdd_test_mut_str("200");
  resp.description = cdd_test_mut_str("ok");

  op.callbacks = &cb;
  op.n_callbacks = 1;
  cb.name = cdd_test_mut_str("onEvent");
  cb.paths = &cb_path;
  cb.n_paths = 1;

  cb_path.route = cdd_test_mut_str("{$request.body#/url}");
  cb_path.operations = &cb_op;
  cb_path.n_operations = 1;
  cb_op.verb = OA_VERB_POST;
  cb_op.operation_id = cdd_test_mut_str("cbPost");
  cb_op.responses = &cb_resp;
  cb_op.n_responses = 1;
  cb_resp.code = cdd_test_mut_str("200");
  cb_resp.description = cdd_test_mut_str("ok");

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root;
    JSON_Object *p_item;
    JSON_Object *op_obj;
    JSON_Object *cbs;
    JSON_Object *cb_obj;
    JSON_Object *cb_post;
    JSON_Object *paths;
    JSON_Object *cb_path_obj;
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
      cdd_json_object_get_object(paths, "/pets", &_cdd_jo_1);
      p_item = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(p_item, "get", &_cdd_jo_1);
      op_obj = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(op_obj, "callbacks", &_cdd_jo_1);
      cbs = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(cbs, "onEvent", &_cdd_jo_1);
      cb_obj = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(cb_obj, "{$request.body#/url}", &_cdd_jo_1);
      cb_path_obj = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(cb_path_obj, "post", &_cdd_jo_1);
      cb_post = _cdd_jo_1;
    }

    ASSERT(cb_post != NULL);
    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(cb_post, "operationId", &_cdd_js_1);
      ASSERT_STR_EQ("cbPost", _cdd_js_1);
    }

    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_writer_parameter_and_header_schema_ref.
 * @return Greatest test result.
 */
TEST test_writer_parameter_and_header_schema_ref(void) {
  int rc = 0;
  char *json;
  struct OpenAPI_Spec spec = {0};
  struct OpenAPI_Path path = {0};
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Parameter params[2] = {0};
  struct OpenAPI_Response resp = {0};
  struct OpenAPI_Header hdr = {0};
  json = NULL;

  spec.paths = &path;
  spec.n_paths = 1;

  path.route = cdd_test_mut_str("/pets");
  path.operations = &op;
  path.n_operations = 1;

  op.verb = OA_VERB_GET;
  op.parameters = params;
  op.n_parameters = 2;

  params[0].name = cdd_test_mut_str("pet");
  params[0].in = OA_PARAM_IN_QUERY;
  params[0].type = cdd_test_mut_str("Pet");

  params[1].name = cdd_test_mut_str("tags");
  params[1].in = OA_PARAM_IN_QUERY;
  params[1].is_array = 1;
  params[1].type = cdd_test_mut_str("array");
  params[1].items_type = cdd_test_mut_str("Tag");

  op.responses = &resp;
  op.n_responses = 1;
  resp.code = cdd_test_mut_str("200");
  resp.description = cdd_test_mut_str("ok");
  resp.headers = &hdr;
  resp.n_headers = 1;
  hdr.name = cdd_test_mut_str("X-Rate");
  hdr.type = cdd_test_mut_str("Rate");

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root;
    JSON_Object *p_item;
    JSON_Object *op_obj;
    JSON_Array *params_arr;
    JSON_Object *p0;
    JSON_Object *p1;
    JSON_Object *p0_schema;
    JSON_Object *p1_schema;
    JSON_Object *p1_items;
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
      cdd_json_object_get_object(paths, "/pets", &_cdd_jo_1);
      p_item = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(p_item, "get", &_cdd_jo_1);
      op_obj = _cdd_jo_1;
    }
    {
      JSON_Array *_cdd_ja_1 = NULL;
      cdd_json_object_get_array(op_obj, "parameters", &_cdd_ja_1);
      params_arr = _cdd_ja_1;
    }
    p0 = json_array_get_object(params_arr, 0);
    p1 = json_array_get_object(params_arr, 1);
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(p0, "schema", &_cdd_jo_1);
      p0_schema = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(p1, "schema", &_cdd_jo_1);
      p1_schema = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(p1_schema, "items", &_cdd_jo_1);
      p1_items = _cdd_jo_1;
    }

    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(p0_schema, "$ref", &_cdd_js_1);
      ASSERT_STR_EQ("#/components/schemas/Pet", _cdd_js_1);
    }
    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(p1_items, "$ref", &_cdd_js_1);
      ASSERT_STR_EQ("#/components/schemas/Tag", _cdd_js_1);
    }

    {
      JSON_Object *responses;
      JSON_Object *resp200;
      JSON_Object *headers;
      JSON_Object *hdr_obj;
      JSON_Object *hdr_schema;
      {
        JSON_Object *_cdd_jo_1 = NULL;
        cdd_json_object_get_object(op_obj, "responses", &_cdd_jo_1);
        responses = _cdd_jo_1;
      }
      {
        JSON_Object *_cdd_jo_1 = NULL;
        cdd_json_object_get_object(responses, "200", &_cdd_jo_1);
        resp200 = _cdd_jo_1;
      }
      {
        JSON_Object *_cdd_jo_1 = NULL;
        cdd_json_object_get_object(resp200, "headers", &_cdd_jo_1);
        headers = _cdd_jo_1;
      }
      {
        JSON_Object *_cdd_jo_1 = NULL;
        cdd_json_object_get_object(headers, "X-Rate", &_cdd_jo_1);
        hdr_obj = _cdd_jo_1;
      }
      {
        JSON_Object *_cdd_jo_1 = NULL;
        cdd_json_object_get_object(hdr_obj, "schema", &_cdd_jo_1);
        hdr_schema = _cdd_jo_1;
      }
      {
        const char *_cdd_js_1 = NULL;
        cdd_json_object_get_string(hdr_schema, "$ref", &_cdd_js_1);
        ASSERT_STR_EQ("#/components/schemas/Rate", _cdd_js_1);
      }
    }

    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_writer_parameter_schema_format_and_content.
 * @return Greatest test result.
 */
TEST test_writer_parameter_schema_format_and_content(void) {
  int rc = 0;
  char *json;
  struct OpenAPI_Spec spec = {0};
  struct OpenAPI_Path path = {0};
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Parameter param = {0};
  struct OpenAPI_Response resp = {0};
  json = NULL;

  spec.paths = &path;
  spec.n_paths = 1;

  path.route = cdd_test_mut_str("/pets");
  path.operations = &op;
  path.n_operations = 1;

  op.verb = OA_VERB_GET;
  op.operation_id = cdd_test_mut_str("getPets");
  op.parameters = &param;
  op.n_parameters = 1;

  param.name = cdd_test_mut_str("id");
  param.in = OA_PARAM_IN_QUERY;
  param.schema_set = 1;
  param.schema.inline_type = cdd_test_mut_str("string");
  param.schema.format = cdd_test_mut_str("uuid");
  param.schema.content_media_type = cdd_test_mut_str("text/plain");
  param.schema.content_encoding = cdd_test_mut_str("base64");

  op.responses = &resp;
  op.n_responses = 1;
  resp.code = cdd_test_mut_str("200");
  resp.description = cdd_test_mut_str("ok");

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root;
    JSON_Object *p_item;
    JSON_Object *op_obj;
    JSON_Array *params_arr;
    JSON_Object *p_obj;
    JSON_Object *schema;
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
      cdd_json_object_get_object(paths, "/pets", &_cdd_jo_1);
      p_item = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(p_item, "get", &_cdd_jo_1);
      op_obj = _cdd_jo_1;
    }
    {
      JSON_Array *_cdd_ja_1 = NULL;
      cdd_json_object_get_array(op_obj, "parameters", &_cdd_ja_1);
      params_arr = _cdd_ja_1;
    }
    p_obj = json_array_get_object(params_arr, 0);
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(p_obj, "schema", &_cdd_jo_1);
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
      ASSERT_STR_EQ("text/plain", _cdd_js_1);
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
 * @brief Unit test test_writer_request_body_ref_with_description.
 * @return Greatest test result.
 */
TEST test_writer_request_body_ref_with_description(void) {
  int rc = 0;
  char *json;
  struct OpenAPI_Spec spec = {0};
  struct OpenAPI_Path path = {0};
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Response resp = {0};
  json = NULL;

  spec.paths = &path;
  spec.n_paths = 1;

  path.route = cdd_test_mut_str("/pets");
  path.operations = &op;
  path.n_operations = 1;

  op.verb = OA_VERB_POST;
  op.operation_id = cdd_test_mut_str("createPet");
  op.req_body_ref = cdd_test_mut_str("#/components/requestBodies/CreatePet");
  op.req_body_description = cdd_test_mut_str("Override");
  op.responses = &resp;
  op.n_responses = 1;
  resp.code = cdd_test_mut_str("200");
  resp.description = cdd_test_mut_str("ok");

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root;
    JSON_Object *p_item;
    JSON_Object *op_obj;
    JSON_Object *rb;
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
      cdd_json_object_get_object(paths, "/pets", &_cdd_jo_1);
      p_item = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(p_item, "post", &_cdd_jo_1);
      op_obj = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(op_obj, "requestBody", &_cdd_jo_1);
      rb = _cdd_jo_1;
    }

    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(rb, "$ref", &_cdd_js_1);
      ASSERT_STR_EQ("#/components/requestBodies/CreatePet", _cdd_js_1);
    }
    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(rb, "description", &_cdd_js_1);
      ASSERT_STR_EQ("Override", _cdd_js_1);
    }

    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_writer_security_scheme_deprecated.
 * @return Greatest test result.
 */
TEST test_writer_security_scheme_deprecated(void) {
  int rc = 0;
  char *json;
  struct OpenAPI_Spec spec = {0};
  struct OpenAPI_SecurityScheme scheme = {0};
  json = NULL;

  spec.security_schemes = &scheme;
  spec.n_security_schemes = 1;

  scheme.name = cdd_test_mut_str("oldKey");
  scheme.type = OA_SEC_APIKEY;
  scheme.in = OA_SEC_IN_HEADER;
  scheme.key_name = cdd_test_mut_str("X-Old");
  scheme.deprecated_set = 1;
  scheme.deprecated = 1;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root;
    JSON_Object *schemes;
    JSON_Object *old_key;
    JSON_Object *comps;
    {
      JSON_Value *_cdd_jv_1 = NULL;
      cdd_json_parse_string(json, &_cdd_jv_1);
      root = _cdd_jv_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(json_value_get_object(root), "components",
                                 &_cdd_jo_1);
      comps = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(comps, "securitySchemes", &_cdd_jo_1);
      schemes = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(schemes, "oldKey", &_cdd_jo_1);
      old_key = _cdd_jo_1;
    }
    {
      int _cdd_jb_1 = -1;
      if (cdd_json_object_get_boolean(old_key, "deprecated", &_cdd_jb_1) !=
          CDD_C_SUCCESS)
        _cdd_jb_1 = -1;
      ASSERT_EQ(1, _cdd_jb_1);
    }
    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_writer_schema_enum_default_nullable.
 * @return Greatest test result.
 */
TEST test_writer_schema_enum_default_nullable(void) {
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
  param.schema.inline_type = cdd_test_mut_str("string");
  param.schema.nullable = 1;
  enum_vals[0].type = OA_ANY_STRING;
  enum_vals[0].string = cdd_test_mut_str("on");
  enum_vals[1].type = OA_ANY_STRING;
  enum_vals[1].string = cdd_test_mut_str("off");
  param.schema.enum_values = enum_vals;
  param.schema.n_enum_values = 2;
  param.schema.default_value.type = OA_ANY_STRING;
  param.schema.default_value.string = cdd_test_mut_str("on");
  param.schema.default_value_set = 1;

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
    JSON_Array *type_arr;
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
      JSON_Array *_cdd_ja_2 = NULL;
      cdd_json_object_get_object(p_item, "get", &_cdd_jo_1);
      cdd_json_object_get_array(_cdd_jo_1, "parameters", &_cdd_ja_2);
      params = _cdd_ja_2;
    }
    p0 = json_array_get_object(params, 0);
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(p0, "schema", &_cdd_jo_1);
      schema = _cdd_jo_1;
    }
    {
      JSON_Value *_cdd_jv_1 = NULL;
      cdd_json_object_get_value(schema, "type", &_cdd_jv_1);
      type_arr = json_value_get_array(_cdd_jv_1);
    }
    {
      JSON_Array *_cdd_ja_1 = NULL;
      cdd_json_object_get_array(schema, "enum", &_cdd_ja_1);
      enum_arr = _cdd_ja_1;
    }
    ASSERT(type_arr != NULL);
    ASSERT_EQ(2, json_array_get_count(type_arr));
    ASSERT_STR_EQ("string", json_array_get_string(type_arr, 0));
    ASSERT_STR_EQ("null", json_array_get_string(type_arr, 1));
    ASSERT(enum_arr != NULL);
    ASSERT_EQ(2, json_array_get_count(enum_arr));
    ASSERT_STR_EQ("on", json_array_get_string(enum_arr, 0));
    ASSERT_STR_EQ("off", json_array_get_string(enum_arr, 1));
    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(schema, "default", &_cdd_js_1);
      ASSERT_STR_EQ("on", _cdd_js_1);
    }
    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_writer_schema_type_union.
 * @return Greatest test result.
 */
TEST test_writer_schema_type_union(void) {
  int rc = 0;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Path path;
  char *json;
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Parameter param = {0};
  char *types[3];

  types[0] = cdd_test_mut_str("string");
  types[1] = cdd_test_mut_str("integer");
  types[2] = cdd_test_mut_str("null");
  json = NULL;

  setup_test_spec(&spec, &path, &op, &param, NULL);
  param.schema_set = 1;
  param.schema.inline_type = cdd_test_mut_str("string");
  param.schema.nullable = 1;
  param.schema.type_union = types;
  param.schema.n_type_union = 3;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root;
    JSON_Object *p_item;
    JSON_Object *p0;
    JSON_Object *schema;
    JSON_Object *paths;
    JSON_Array *params;
    JSON_Array *type_arr;
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
      JSON_Array *_cdd_ja_2 = NULL;
      cdd_json_object_get_object(p_item, "get", &_cdd_jo_1);
      cdd_json_object_get_array(_cdd_jo_1, "parameters", &_cdd_ja_2);
      params = _cdd_ja_2;
    }
    p0 = json_array_get_object(params, 0);
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(p0, "schema", &_cdd_jo_1);
      schema = _cdd_jo_1;
    }
    {
      JSON_Value *_cdd_jv_1 = NULL;
      cdd_json_object_get_value(schema, "type", &_cdd_jv_1);
      type_arr = json_value_get_array(_cdd_jv_1);
    }
    ASSERT(type_arr != NULL);
    ASSERT_EQ(3, json_array_get_count(type_arr));
    ASSERT_STR_EQ("string", json_array_get_string(type_arr, 0));
    ASSERT_STR_EQ("integer", json_array_get_string(type_arr, 1));
    ASSERT_STR_EQ("null", json_array_get_string(type_arr, 2));
    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_writer_schema_array_items_enum_nullable.
 * @return Greatest test result.
 */
TEST test_writer_schema_array_items_enum_nullable(void) {
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
  param.schema.is_array = 1;
  param.schema.inline_type = cdd_test_mut_str("string");
  param.schema.items_nullable = 1;
  enum_vals[0].type = OA_ANY_STRING;
  enum_vals[0].string = cdd_test_mut_str("a");
  enum_vals[1].type = OA_ANY_STRING;
  enum_vals[1].string = cdd_test_mut_str("b");
  param.schema.items_enum_values = enum_vals;
  param.schema.n_items_enum_values = 2;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root;
    JSON_Object *p_item;
    JSON_Object *p0;
    JSON_Object *schema;
    JSON_Object *items;
    JSON_Array *enum_arr;
    JSON_Object *paths;
    JSON_Array *params;
    JSON_Array *type_arr;
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
      JSON_Array *_cdd_ja_2 = NULL;
      cdd_json_object_get_object(p_item, "get", &_cdd_jo_1);
      cdd_json_object_get_array(_cdd_jo_1, "parameters", &_cdd_ja_2);
      params = _cdd_ja_2;
    }
    p0 = json_array_get_object(params, 0);
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(p0, "schema", &_cdd_jo_1);
      schema = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(schema, "items", &_cdd_jo_1);
      items = _cdd_jo_1;
    }
    {
      JSON_Value *_cdd_jv_1 = NULL;
      cdd_json_object_get_value(items, "type", &_cdd_jv_1);
      type_arr = json_value_get_array(_cdd_jv_1);
    }
    {
      JSON_Array *_cdd_ja_1 = NULL;
      cdd_json_object_get_array(items, "enum", &_cdd_ja_1);
      enum_arr = _cdd_ja_1;
    }
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
  g_fail_io_after = -1;
  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_OPENAPI_WRITER_PATHS_H */
