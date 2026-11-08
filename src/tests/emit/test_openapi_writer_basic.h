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

/**
 * @brief Unit test test_writer_empty_spec.
 * @return Greatest test result.
 */
TEST test_writer_empty_spec(void) {
  int rc = 0;
  char *json;
  struct OpenAPI_Spec spec = {0};
  json = NULL;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);
  ASSERT(json != NULL);

  {
    JSON_Value *root;
    JSON_Object *obj;
    {
      JSON_Value *_cdd_jv_1 = NULL;
      cdd_json_parse_string(json, &_cdd_jv_1);
      root = _cdd_jv_1;
    }
    obj = json_value_get_object(root);
    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(obj, "openapi", &_cdd_js_1);
      ASSERT_STR_EQ("3.2.0", _cdd_js_1);
    }
    ASSERT(json_object_has_value(obj, "info"));
    ASSERT(json_object_has_value(obj, "paths"));
    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_writer_basic_operation.
 * @return Greatest test result.
 */
TEST test_writer_basic_operation(void) {
  int rc = 0;
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

    ASSERT(op_obj != NULL);
    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(op_obj, "operationId", &_cdd_js_1);
      ASSERT_STR_EQ("testOp", _cdd_js_1);
    }

    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_writer_schema_document.
 * @return Greatest test result.
 */
TEST test_writer_schema_document(void) {
  int rc = 0;
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

/**
 * @brief Unit test test_writer_root_metadata_and_tags.
 * @return Greatest test result.
 */
TEST test_writer_root_metadata_and_tags(void) {
  int rc = 0;
  struct OpenAPI_Tag tags[1];
  char *json;
  struct OpenAPI_Spec spec = {0};
  json = NULL;

  memset(tags, 0, sizeof(tags));
  spec.openapi_version = cdd_test_mut_str("3.2.0");
  spec.self_uri = cdd_test_mut_str("https://example.com/openapi.json");
  spec.json_schema_dialect =
      cdd_test_mut_str("https://spec.openapis.org/oas/3.1/dialect/base");
  spec.external_docs.url = cdd_test_mut_str("https://example.com/docs");
  spec.external_docs.description = cdd_test_mut_str("Root docs");
  spec.tags = tags;
  spec.n_tags = 1;
  tags[0].name = cdd_test_mut_str("pets");
  tags[0].summary = cdd_test_mut_str("Pets");
  tags[0].description = cdd_test_mut_str("Pet ops");
  tags[0].parent = cdd_test_mut_str("animals");
  tags[0].kind = cdd_test_mut_str("nav");
  tags[0].external_docs.url = cdd_test_mut_str("https://example.com/tags/pets");
  tags[0].external_docs.description = cdd_test_mut_str("Tag docs");

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root;
    JSON_Object *obj;
    JSON_Object *ext;
    JSON_Array *tag_arr;
    JSON_Object *tag0;
    JSON_Object *tag_ext;
    {
      JSON_Value *_cdd_jv_1 = NULL;
      cdd_json_parse_string(json, &_cdd_jv_1);
      root = _cdd_jv_1;
    }
    obj = json_value_get_object(root);
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(obj, "externalDocs", &_cdd_jo_1);
      ext = _cdd_jo_1;
    }
    {
      JSON_Array *_cdd_ja_1 = NULL;
      cdd_json_object_get_array(obj, "tags", &_cdd_ja_1);
      tag_arr = _cdd_ja_1;
    }
    tag0 = json_array_get_object(tag_arr, 0);
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(tag0, "externalDocs", &_cdd_jo_1);
      tag_ext = _cdd_jo_1;
    }

    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(obj, "$self", &_cdd_js_1);
      ASSERT_STR_EQ("https://example.com/openapi.json", _cdd_js_1);
    }
    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(obj, "jsonSchemaDialect", &_cdd_js_1);
      ASSERT_STR_EQ("https://spec.openapis.org/oas/3.1/dialect/base",
                    _cdd_js_1);
    }
    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(ext, "url", &_cdd_js_1);
      ASSERT_STR_EQ("https://example.com/docs", _cdd_js_1);
    }
    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(ext, "description", &_cdd_js_1);
      ASSERT_STR_EQ("Root docs", _cdd_js_1);
    }
    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(tag0, "name", &_cdd_js_1);
      ASSERT_STR_EQ("pets", _cdd_js_1);
    }
    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(tag0, "summary", &_cdd_js_1);
      ASSERT_STR_EQ("Pets", _cdd_js_1);
    }
    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(tag0, "description", &_cdd_js_1);
      ASSERT_STR_EQ("Pet ops", _cdd_js_1);
    }
    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(tag0, "parent", &_cdd_js_1);
      ASSERT_STR_EQ("animals", _cdd_js_1);
    }
    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(tag0, "kind", &_cdd_js_1);
      ASSERT_STR_EQ("nav", _cdd_js_1);
    }
    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(tag_ext, "url", &_cdd_js_1);
      ASSERT_STR_EQ("https://example.com/tags/pets", _cdd_js_1);
    }
    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(tag_ext, "description", &_cdd_js_1);
      ASSERT_STR_EQ("Tag docs", _cdd_js_1);
    }

    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_writer_path_ref_and_servers.
 * @return Greatest test result.
 */
TEST test_writer_path_ref_and_servers(void) {
  int rc = 0;
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

  path.route = cdd_test_mut_str("/pets");
  path.ref = cdd_test_mut_str("#/components/pathItems/Pets");
  path.servers = path_servers;
  path.n_servers = 1;
  path_servers[0].url = cdd_test_mut_str("https://path.example.com");

  path.operations = &op;
  path.n_operations = 1;
  op.verb = OA_VERB_GET;
  op.operation_id = cdd_test_mut_str("listPets");
  op.responses = &resp;
  op.n_responses = 1;
  resp.code = cdd_test_mut_str("200");
  resp.description = cdd_test_mut_str("OK");
  op.servers = op_servers;
  op.n_servers = 1;
  op_servers[0].url = cdd_test_mut_str("https://op.example.com");

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
      JSON_Array *_cdd_ja_1 = NULL;
      cdd_json_object_get_array(p_item, "servers", &_cdd_ja_1);
      p_servers = _cdd_ja_1;
    }
    p_srv0 = json_array_get_object(p_servers, 0);
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(p_item, "get", &_cdd_jo_1);
      op_obj = _cdd_jo_1;
    }
    {
      JSON_Array *_cdd_ja_1 = NULL;
      cdd_json_object_get_array(op_obj, "servers", &_cdd_ja_1);
      op_servers_arr = _cdd_ja_1;
    }
    op_srv0 = json_array_get_object(op_servers_arr, 0);

    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(p_item, "$ref", &_cdd_js_1);
      ASSERT_STR_EQ("#/components/pathItems/Pets", _cdd_js_1);
    }
    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(p_srv0, "url", &_cdd_js_1);
      ASSERT_STR_EQ("https://path.example.com", _cdd_js_1);
    }
    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(op_srv0, "url", &_cdd_js_1);
      ASSERT_STR_EQ("https://op.example.com", _cdd_js_1);
    }

    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_writer_webhooks.
 * @return Greatest test result.
 */
TEST test_writer_webhooks(void) {
  int rc = 0;
  char *json;
  struct OpenAPI_Spec spec = {0};
  struct OpenAPI_Path hook = {0};
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Response resp = {0};
  json = NULL;

  spec.webhooks = &hook;
  spec.n_webhooks = 1;

  hook.route = cdd_test_mut_str("petEvent");
  hook.operations = &op;
  hook.n_operations = 1;
  op.verb = OA_VERB_POST;
  op.operation_id = cdd_test_mut_str("onPetEvent");
  op.responses = &resp;
  op.n_responses = 1;
  resp.code = cdd_test_mut_str("200");
  resp.description = cdd_test_mut_str("OK");

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root;
    JSON_Object *hook_item;
    JSON_Object *op_obj;
    JSON_Object *hooks;
    {
      JSON_Value *_cdd_jv_1 = NULL;
      cdd_json_parse_string(json, &_cdd_jv_1);
      root = _cdd_jv_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(json_value_get_object(root), "webhooks",
                                 &_cdd_jo_1);
      hooks = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(hooks, "petEvent", &_cdd_jo_1);
      hook_item = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(hook_item, "post", &_cdd_jo_1);
      op_obj = _cdd_jo_1;
    }

    ASSERT(op_obj != NULL);
    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(op_obj, "operationId", &_cdd_js_1);
      ASSERT_STR_EQ("onPetEvent", _cdd_js_1);
    }

    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_writer_params_responses.
 * @return Greatest test result.
 */
TEST test_writer_params_responses(void) {
  int rc = 0;
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
    {
      JSON_Value *_cdd_jv_1 = NULL;
      cdd_json_parse_string(json, &_cdd_jv_1);
      root = _cdd_jv_1;
    }
    op_obj = json_value_get_object(root);
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(op_obj, "paths", &_cdd_jo_1);
      op_obj = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(op_obj, "/test/route", &_cdd_jo_1);
      op_obj = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(op_obj, "get", &_cdd_jo_1);
      op_obj = _cdd_jo_1;
    }

    /* Param Check */
    {
      JSON_Array *oa_params;
      JSON_Object *p_obj;
      {
        JSON_Array *_cdd_ja_1 = NULL;
        cdd_json_object_get_array(op_obj, "parameters", &_cdd_ja_1);
        oa_params = _cdd_ja_1;
      }
      p_obj = json_array_get_object(oa_params, 0);
      {
        const char *_cdd_js_1 = NULL;
        cdd_json_object_get_string(p_obj, "name", &_cdd_js_1);
        ASSERT_STR_EQ("p1", _cdd_js_1);
      }
      {
        const char *_cdd_js_1 = NULL;
        cdd_json_object_get_string(p_obj, "in", &_cdd_js_1);
        ASSERT_STR_EQ("query", _cdd_js_1);
      }
      {
        JSON_Object *sch;
        {
          JSON_Object *_cdd_jo_1 = NULL;
          cdd_json_object_get_object(p_obj, "schema", &_cdd_jo_1);
          sch = _cdd_jo_1;
        }
        {
          const char *_cdd_js_1 = NULL;
          cdd_json_object_get_string(sch, "type", &_cdd_js_1);
          ASSERT_STR_EQ("string", _cdd_js_1);
        }
      }
    }

    /* Response Check */
    {
      JSON_Object *responses;
      JSON_Object *r200;
      JSON_Object *content;
      JSON_Object *media;
      JSON_Object *schema;
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
        const char *_cdd_js_1 = NULL;
        cdd_json_object_get_string(schema, "$ref", &_cdd_js_1);
        ASSERT_STR_EQ("#/components/schemas/TestModel", _cdd_js_1);
      }
    }

    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_writer_parameter_metadata.
 * @return Greatest test result.
 */
TEST test_writer_parameter_metadata(void) {
  int rc = 0;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Path path;
  char *json;
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Parameter param = {0};
  json = NULL;

  setup_test_spec(&spec, &path, &op, &param, NULL);
  param.description = cdd_test_mut_str("Search term");
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
      op_obj = _cdd_jo_3;
    }
    {
      JSON_Array *_cdd_ja_1 = NULL;
      cdd_json_object_get_array(op_obj, "parameters", &_cdd_ja_1);
      oa_params = _cdd_ja_1;
    }
    p_obj = json_array_get_object(oa_params, 0);

    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(p_obj, "description", &_cdd_js_1);
      ASSERT_STR_EQ("Search term", _cdd_js_1);
    }
    {
      int _cdd_jb_1 = -1;
      if (cdd_json_object_get_boolean(p_obj, "deprecated", &_cdd_jb_1) !=
          CDD_C_SUCCESS)
        _cdd_jb_1 = -1;
      ASSERT_EQ(1, _cdd_jb_1);
    }
    {
      int _cdd_jb_1 = -1;
      if (cdd_json_object_get_boolean(p_obj, "allowReserved", &_cdd_jb_1) !=
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
 * @brief Unit test test_writer_allow_empty_value.
 * @return Greatest test result.
 */
TEST test_writer_allow_empty_value(void) {
  int rc = 0;
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
      op_obj = _cdd_jo_3;
    }
    {
      JSON_Array *_cdd_ja_1 = NULL;
      cdd_json_object_get_array(op_obj, "parameters", &_cdd_ja_1);
      oa_params = _cdd_ja_1;
    }
    p_obj = json_array_get_object(oa_params, 0);

    {
      int _cdd_jb_1 = -1;
      if (cdd_json_object_get_boolean(p_obj, "allowEmptyValue", &_cdd_jb_1) !=
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
 * @brief Unit test test_writer_request_body_metadata_and_response_description.
 * @return Greatest test result.
 */
TEST test_writer_request_body_metadata_and_response_description(void) {
  int rc = 0;
  struct OpenAPI_Spec spec;
  char *json;
  struct OpenAPI_Path path = {0};
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Response resp = {0};
  json = NULL;

  setup_test_spec(&spec, &path, &op, NULL, &resp);
  op.verb = OA_VERB_POST;
  op.req_body.ref_name = cdd_test_mut_str("User");
  op.req_body.content_type = cdd_test_mut_str("application/json");
  op.req_body_required_set = 1;
  op.req_body_required = 0;
  op.req_body_description = cdd_test_mut_str("Payload");
  resp.description = cdd_test_mut_str("Created");

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root;
    JSON_Object *op_obj;
    {
      JSON_Value *_cdd_jv_1 = NULL;
      cdd_json_parse_string(json, &_cdd_jv_1);
      root = _cdd_jv_1;
    }
    op_obj = json_value_get_object(root);
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(op_obj, "paths", &_cdd_jo_1);
      op_obj = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(op_obj, "/test/route", &_cdd_jo_1);
      op_obj = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(op_obj, "post", &_cdd_jo_1);
      op_obj = _cdd_jo_1;
    }

    {
      JSON_Object *rb;
      {
        JSON_Object *_cdd_jo_1 = NULL;
        cdd_json_object_get_object(op_obj, "requestBody", &_cdd_jo_1);
        rb = _cdd_jo_1;
      }
      {
        const char *_cdd_js_1 = NULL;
        cdd_json_object_get_string(rb, "description", &_cdd_js_1);
        ASSERT_STR_EQ("Payload", _cdd_js_1);
      }
      {
        int _cdd_jb_1 = -1;
        if (cdd_json_object_get_boolean(rb, "required", &_cdd_jb_1) !=
            CDD_C_SUCCESS)
          _cdd_jb_1 = -1;
        ASSERT_EQ(0, _cdd_jb_1);
      }
    }

    {
      JSON_Object *responses;
      JSON_Object *r200;
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
        const char *_cdd_js_1 = NULL;
        cdd_json_object_get_string(r200, "description", &_cdd_js_1);
        ASSERT_STR_EQ("Created", _cdd_js_1);
      }
    }

    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_writer_info_metadata.
 * @return Greatest test result.
 */
TEST test_writer_info_metadata(void) {
  int rc = 0;
  char *json;
  struct OpenAPI_Spec spec = {0};
  json = NULL;

  spec.info.title = cdd_test_mut_str("Example API");
  spec.info.summary = cdd_test_mut_str("Short");
  spec.info.description = cdd_test_mut_str("Long");
  spec.info.terms_of_service = cdd_test_mut_str("https://example.com/terms");
  spec.info.version = cdd_test_mut_str("2.1.0");
  spec.info.contact.name = cdd_test_mut_str("Support");
  spec.info.contact.url = cdd_test_mut_str("https://example.com");
  spec.info.contact.email = cdd_test_mut_str("support@example.com");
  spec.info.license.name = cdd_test_mut_str("Apache 2.0");
  spec.info.license.identifier = cdd_test_mut_str("Apache-2.0");

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);
  ASSERT(json != NULL);

  {
    JSON_Value *root;
    JSON_Object *contact;
    JSON_Object *license;
    JSON_Object *info;
    {
      JSON_Value *_cdd_jv_1 = NULL;
      cdd_json_parse_string(json, &_cdd_jv_1);
      root = _cdd_jv_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(json_value_get_object(root), "info",
                                 &_cdd_jo_1);
      info = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(info, "contact", &_cdd_jo_1);
      contact = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(info, "license", &_cdd_jo_1);
      license = _cdd_jo_1;
    }

    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(info, "title", &_cdd_js_1);
      ASSERT_STR_EQ("Example API", _cdd_js_1);
    }
    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(info, "summary", &_cdd_js_1);
      ASSERT_STR_EQ("Short", _cdd_js_1);
    }
    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(info, "description", &_cdd_js_1);
      ASSERT_STR_EQ("Long", _cdd_js_1);
    }
    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(info, "termsOfService", &_cdd_js_1);
      ASSERT_STR_EQ("https://example.com/terms", _cdd_js_1);
    }
    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(info, "version", &_cdd_js_1);
      ASSERT_STR_EQ("2.1.0", _cdd_js_1);
    }
    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(contact, "name", &_cdd_js_1);
      ASSERT_STR_EQ("Support", _cdd_js_1);
    }
    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(contact, "url", &_cdd_js_1);
      ASSERT_STR_EQ("https://example.com", _cdd_js_1);
    }
    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(contact, "email", &_cdd_js_1);
      ASSERT_STR_EQ("support@example.com", _cdd_js_1);
    }
    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(license, "name", &_cdd_js_1);
      ASSERT_STR_EQ("Apache 2.0", _cdd_js_1);
    }
    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(license, "identifier", &_cdd_js_1);
      ASSERT_STR_EQ("Apache-2.0", _cdd_js_1);
    }
    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(license, "url", &_cdd_js_1);
      ASSERT(_cdd_js_1 == NULL);
    }

    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_writer_info_license_identifier_and_url_rejected.
 * @return Greatest test result.
 */
TEST test_writer_info_license_identifier_and_url_rejected(void) {
  int rc = 0;
  char *json;
  struct OpenAPI_Spec spec = {0};
  json = NULL;

  spec.info.title = cdd_test_mut_str("Example API");
  spec.info.version = cdd_test_mut_str("1.0");
  spec.info.license.name = cdd_test_mut_str("Apache 2.0");
  spec.info.license.identifier = cdd_test_mut_str("Apache-2.0");
  spec.info.license.url = cdd_test_mut_str("https://www.apache.org/"
                                           "licenses/LICENSE-2.0.html");

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  ASSERT(json == NULL);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_writer_server_url_query_rejected.
 * @return Greatest test result.
 */
TEST test_writer_server_url_query_rejected(void) {
  int rc = 0;
  char *json;
  struct OpenAPI_Spec spec = {0};
  struct OpenAPI_Server server = {0};
  json = NULL;

  spec.info.title = cdd_test_mut_str("Example API");
  spec.info.version = cdd_test_mut_str("1.0");
  server.url = cdd_test_mut_str("https://example.com/api?x=1");
  spec.servers = &server;
  spec.n_servers = 1;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  ASSERT(json == NULL);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_writer_operation_metadata.
 * @return Greatest test result.
 */
TEST test_writer_operation_metadata(void) {
  int rc = 0;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Path path;
  char *json;
  struct OpenAPI_Operation op = {0};
  json = NULL;

  setup_test_spec(&spec, &path, &op, NULL, NULL);
  op.summary = cdd_test_mut_str("Summary text");
  op.description = cdd_test_mut_str("Longer description");
  op.deprecated = 1;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root;
    JSON_Object *op_obj;
    {
      JSON_Value *_cdd_jv_1 = NULL;
      cdd_json_parse_string(json, &_cdd_jv_1);
      root = _cdd_jv_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(json_value_get_object(root), "paths",
                                 &_cdd_jo_1);
      op_obj = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(op_obj, "/test/route", &_cdd_jo_1);
      op_obj = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(op_obj, "get", &_cdd_jo_1);
      op_obj = _cdd_jo_1;
    }

    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(op_obj, "summary", &_cdd_js_1);
      ASSERT_STR_EQ("Summary text", _cdd_js_1);
    }
    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(op_obj, "description", &_cdd_js_1);
      ASSERT_STR_EQ("Longer description", _cdd_js_1);
    }
    {
      int _cdd_jb_1 = -1;
      if (cdd_json_object_get_boolean(op_obj, "deprecated", &_cdd_jb_1) !=
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
 * @brief Unit test test_writer_response_content_type.
 * @return Greatest test result.
 */
TEST test_writer_response_content_type(void) {
  int rc = 0;
  struct OpenAPI_Spec spec;
  char *json;
  struct OpenAPI_Path path = {0};
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Response resp = {0};
  json = NULL;

  setup_test_spec(&spec, &path, &op, NULL, &resp);
  resp.content_type = cdd_test_mut_str("text/plain");
  resp.schema.ref_name = cdd_test_mut_str("Message");

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
      cdd_json_object_get_object(content, "text/plain", &_cdd_jo_1);
      media = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(media, "schema", &_cdd_jo_1);
      schema = _cdd_jo_1;
    }

    ASSERT(schema != NULL);
    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(schema, "$ref", &_cdd_js_1);
      ASSERT_STR_EQ("#/components/schemas/Message", _cdd_js_1);
    }
    json_value_free(root);
  }

  free(json);
  json = NULL;

  /* Verify Swagger 2.0 emission with and without host/basePath */
  spec.swagger_version = cdd_test_mut_str("2.0");
  ASSERT_EQ(CDD_C_SUCCESS, openapi_write_spec_to_json(&spec, &json));
  ASSERT(strstr(json, "\"swagger\": \"2.0\"") != NULL);
  free(json);
  json = NULL;

  spec.host = cdd_test_mut_str("api.example.com");
  spec.basePath = cdd_test_mut_str("/v1");
  ASSERT_EQ(CDD_C_SUCCESS, openapi_write_spec_to_json(&spec, &json));
  ASSERT(strstr(json, "\"swagger\": \"2.0\"") != NULL);
  ASSERT(strstr(json, "\"host\": \"api.example.com\"") != NULL);
  ASSERT(strstr(json, "\"basePath\": \"\\/v1\"") != NULL);
  free(json);

  g_fail_io_after = -1;
  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_OPENAPI_WRITER_BASIC_H */
