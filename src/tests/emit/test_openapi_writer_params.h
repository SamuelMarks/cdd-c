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

/**
 * @brief Unit test test_writer_parameter_styles.
 * @return Greatest test result.
 */
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
        cdd_json_object_get_string(p_obj, "style", &_cdd_js_1);
        ASSERT_STR_EQ("form", _cdd_js_1);
      }
      {
        int _cdd_jb_1 = -1;
        if (cdd_json_object_get_boolean(p_obj, "explode", &_cdd_jb_1) !=
            CDD_C_SUCCESS)
          _cdd_jb_1 = -1;
        ASSERT_EQ(1, _cdd_jb_1);
      }
    }
    json_value_free(root);
  }
  free(json);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_writer_parameter_explode_false.
 * @return Greatest test result.
 */
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

    {
      JSON_Array *oa_params;
      JSON_Object *p_obj;
      {
        JSON_Array *_cdd_ja_1 = NULL;
        cdd_json_object_get_array(op_obj, "parameters", &_cdd_ja_1);
        oa_params = _cdd_ja_1;
      }
      p_obj = json_array_get_object(oa_params, 0);
      ASSERT(json_object_has_value(p_obj, "explode"));
      {
        int _cdd_jb_1 = -1;
        if (cdd_json_object_get_boolean(p_obj, "explode", &_cdd_jb_1) !=
            CDD_C_SUCCESS)
          _cdd_jb_1 = -1;
        ASSERT_EQ(0, _cdd_jb_1);
      }
    }
    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_writer_parameter_style_matrix.
 * @return Greatest test result.
 */
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
        cdd_json_object_get_string(p_obj, "style", &_cdd_js_1);
        ASSERT_STR_EQ("matrix", _cdd_js_1);
      }
    }
    json_value_free(root);
  }
  free(json);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_writer_parameter_content_any.
 * @return Greatest test result.
 */
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
  path.route = cdd_test_mut_str("/headers");
  path.operations = &op;
  path.n_operations = 1;
  op.verb = OA_VERB_GET;
  op.operation_id = cdd_test_mut_str("getHeader");
  op.parameters = &param;
  op.n_parameters = 1;
  param.name = cdd_test_mut_str("X-Foo");
  param.in = OA_PARAM_IN_HEADER;
  param.type = cdd_test_mut_str("string");
  param.content_type = cdd_test_mut_str("text/plain");

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
      cdd_json_object_get_object(op_obj, "/headers", &_cdd_jo_1);
      op_obj = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(op_obj, "get", &_cdd_jo_1);
      op_obj = _cdd_jo_1;
    }
    {
      JSON_Array *oa_params;
      JSON_Object *p_obj;
      JSON_Object *content;
      JSON_Object *media;
      JSON_Object *schema;
      {
        JSON_Array *_cdd_ja_1 = NULL;
        cdd_json_object_get_array(op_obj, "parameters", &_cdd_ja_1);
        oa_params = _cdd_ja_1;
      }
      p_obj = json_array_get_object(oa_params, 0);
      {
        JSON_Object *_cdd_jo_1 = NULL;
        cdd_json_object_get_object(p_obj, "content", &_cdd_jo_1);
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
      {
        const char *_cdd_js_1 = NULL;
        cdd_json_object_get_string(p_obj, "in", &_cdd_js_1);
        ASSERT_STR_EQ("header", _cdd_js_1);
      }
      {
        JSON_Object *_cdd_jo_1 = NULL;
        cdd_json_object_get_object(p_obj, "schema", &_cdd_jo_1);
        ASSERT(_cdd_jo_1 == NULL);
      }
      {
        const char *_cdd_js_1 = NULL;
        cdd_json_object_get_string(schema, "type", &_cdd_js_1);
        ASSERT_STR_EQ("string", _cdd_js_1);
      }
    }

    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_writer_parameter_and_header_content_media_type.
 * @return Greatest test result.
 */
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
  path.route = cdd_test_mut_str("/content");
  path.operations = &op;
  path.n_operations = 1;
  op.verb = OA_VERB_GET;
  op.operation_id = cdd_test_mut_str("getContent");
  op.parameters = &param;
  op.n_parameters = 1;
  param.name = cdd_test_mut_str("filter");
  param.in = OA_PARAM_IN_QUERY;
  param.content_media_types = &param_media;
  param.n_content_media_types = 1;
  param_media.name = cdd_test_mut_str("application/x-www-form-urlencoded");
  param_media.schema_set = 1;
  param_media.schema.inline_type = cdd_test_mut_str("object");
  param_media.encoding = &enc;
  param_media.n_encoding = 1;
  enc.name = cdd_test_mut_str("id");
  enc.content_type = cdd_test_mut_str("text/plain");
  enc.style = OA_STYLE_FORM;
  enc.style_set = 1;
  enc.explode = 1;
  enc.explode_set = 1;

  op.responses = &resp;
  op.n_responses = 1;
  resp.code = cdd_test_mut_str("200");
  resp.description = cdd_test_mut_str("ok");
  resp.headers = &header;
  resp.n_headers = 1;
  header.name = cdd_test_mut_str("X-Rate");
  header.content_media_types = &header_media;
  header.n_content_media_types = 1;
  header_media.name = cdd_test_mut_str("text/plain");
  header_media.schema_set = 1;
  header_media.schema.inline_type = cdd_test_mut_str("string");

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
      cdd_json_object_get_object(paths, "/content", &_cdd_jo_1);
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
      params = _cdd_ja_1;
    }
    p0 = json_array_get_object(params, 0);
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(p0, "content", &_cdd_jo_1);
      content = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(content, "application/x-www-form-urlencoded",
                                 &_cdd_jo_1);
      media = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(media, "encoding", &_cdd_jo_1);
      encoding = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(encoding, "id", &_cdd_jo_1);
      enc_id = _cdd_jo_1;
    }

    ASSERT(enc_id != NULL);
    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(enc_id, "contentType", &_cdd_js_1);
      ASSERT_STR_EQ("text/plain", _cdd_js_1);
    }
    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(enc_id, "style", &_cdd_js_1);
      ASSERT_STR_EQ("form", _cdd_js_1);
    }
    {
      int _cdd_jb_1 = -1;
      if (cdd_json_object_get_boolean(enc_id, "explode", &_cdd_jb_1) !=
          CDD_C_SUCCESS)
        _cdd_jb_1 = -1;
      ASSERT_EQ(1, _cdd_jb_1);
    }

    {
      JSON_Object *responses;
      JSON_Object *r200;
      JSON_Object *headers;
      JSON_Object *x_rate;
      JSON_Object *h_content;
      JSON_Object *h_media;
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
        cdd_json_object_get_object(r200, "headers", &_cdd_jo_1);
        headers = _cdd_jo_1;
      }
      {
        JSON_Object *_cdd_jo_1 = NULL;
        cdd_json_object_get_object(headers, "X-Rate", &_cdd_jo_1);
        x_rate = _cdd_jo_1;
      }
      {
        JSON_Object *_cdd_jo_1 = NULL;
        cdd_json_object_get_object(x_rate, "content", &_cdd_jo_1);
        h_content = _cdd_jo_1;
      }
      {
        JSON_Object *_cdd_jo_1 = NULL;
        cdd_json_object_get_object(h_content, "text/plain", &_cdd_jo_1);
        h_media = _cdd_jo_1;
      }
      {
        JSON_Object *_cdd_jo_1 = NULL;
        cdd_json_object_get_object(h_media, "schema", &_cdd_jo_1);
        schema = _cdd_jo_1;
      }
      {
        const char *_cdd_js_1 = NULL;
        cdd_json_object_get_string(schema, "type", &_cdd_js_1);
        ASSERT_STR_EQ("string", _cdd_js_1);
      }
    }

    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_writer_parameter_examples_object.
 * @return Greatest test result.
 */
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
  ex.name = cdd_test_mut_str("basic");
  ex.data_value.type = OA_ANY_STRING;
  ex.data_value.string = cdd_test_mut_str("hello");
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
    {
      JSON_Array *_cdd_ja_1 = NULL;
      cdd_json_object_get_array(op_obj, "parameters", &_cdd_ja_1);
      params = _cdd_ja_1;
    }
    p0 = json_array_get_object(params, 0);
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(p0, "examples", &_cdd_jo_1);
      examples = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(examples, "basic", &_cdd_jo_1);
      basic = _cdd_jo_1;
    }

    ASSERT(basic != NULL);
    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(basic, "dataValue", &_cdd_js_1);
      ASSERT_STR_EQ("hello", _cdd_js_1);
    }

    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_writer_parameter_examples_media.
 * @return Greatest test result.
 */
TEST test_writer_parameter_examples_media(void) {
  int rc = 0;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Path path;
  char *json;
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Parameter param = {0};
  json = NULL;

  setup_test_spec(&spec, &path, &op, &param, NULL);
  param.content_type = cdd_test_mut_str("application/json");
  param.example.type = OA_ANY_STRING;
  param.example.string = cdd_test_mut_str("hi");
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
    {
      JSON_Array *_cdd_ja_1 = NULL;
      cdd_json_object_get_array(op_obj, "parameters", &_cdd_ja_1);
      params = _cdd_ja_1;
    }
    p0 = json_array_get_object(params, 0);
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(p0, "content", &_cdd_jo_1);
      content = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(content, "application/json", &_cdd_jo_1);
      media = _cdd_jo_1;
    }

    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(media, "example", &_cdd_js_1);
      ASSERT_STR_EQ("hi", _cdd_js_1);
    }

    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_writer_component_examples.
 * @return Greatest test result.
 */
TEST test_writer_component_examples(void) {
  int rc = 0;
  struct OpenAPI_Example ex;
  char *json;
  char *names[1];
  struct OpenAPI_Spec spec = {0};
  json = NULL;

  memset(&ex, 0, sizeof(ex));
  names[0] = cdd_test_mut_str("ex1");
  spec.component_examples = &ex;
  spec.component_example_names = names;
  spec.n_component_examples = 1;
  ex.summary = cdd_test_mut_str("Example");
  ex.value.type = OA_ANY_STRING;
  ex.value.string = cdd_test_mut_str("v");
  ex.value_set = 1;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root;
    JSON_Object *examples;
    JSON_Object *ex_obj;
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
      cdd_json_object_get_object(comps, "examples", &_cdd_jo_1);
      examples = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(examples, "ex1", &_cdd_jo_1);
      ex_obj = _cdd_jo_1;
    }
    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(ex_obj, "summary", &_cdd_js_1);
      ASSERT_STR_EQ("Example", _cdd_js_1);
    }
    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(ex_obj, "value", &_cdd_js_1);
      ASSERT_STR_EQ("v", _cdd_js_1);
    }
    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_writer_oauth2_flows.
 * @return Greatest test result.
 */
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

  scope.name = cdd_test_mut_str("read");
  scope.description = cdd_test_mut_str("Read");
  flow.type = OA_OAUTH_FLOW_PASSWORD;
  flow.token_url = cdd_test_mut_str("https://token.example.com");
  flow.scopes = &scope;
  flow.n_scopes = 1;
  scheme.name = cdd_test_mut_str("oauth");
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
      cdd_json_object_get_object(schemes, "oauth", &_cdd_jo_1);
      oauth = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(oauth, "flows", &_cdd_jo_1);
      flows = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(flows, "password", &_cdd_jo_1);
      password = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(password, "scopes", &_cdd_jo_1);
      scopes = _cdd_jo_1;
    }

    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(password, "tokenUrl", &_cdd_js_1);
      ASSERT_STR_EQ("https://token.example.com", _cdd_js_1);
    }
    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(scopes, "read", &_cdd_js_1);
      ASSERT_STR_EQ("Read", _cdd_js_1);
    }
    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_writer_servers.
 * @return Greatest test result.
 */
TEST test_writer_servers(void) {
  int rc = 0;
  struct OpenAPI_Server servers[1];
  char *json;
  struct OpenAPI_Spec spec = {0};
  json = NULL;

  memset(servers, 0, sizeof(servers));
  servers[0].url = cdd_test_mut_str("https://api.example.com");
  servers[0].description = cdd_test_mut_str("Prod");
  servers[0].name = cdd_test_mut_str("prod");

  spec.servers = servers;
  spec.n_servers = 1;
  spec.openapi_version = cdd_test_mut_str("3.1.2");

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root;
    JSON_Object *root_obj;
    JSON_Array *srv_arr;
    JSON_Object *srv;
    {
      JSON_Value *_cdd_jv_1 = NULL;
      cdd_json_parse_string(json, &_cdd_jv_1);
      root = _cdd_jv_1;
    }
    root_obj = json_value_get_object(root);
    {
      JSON_Array *_cdd_ja_1 = NULL;
      cdd_json_object_get_array(root_obj, "servers", &_cdd_ja_1);
      srv_arr = _cdd_ja_1;
    }
    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(root_obj, "openapi", &_cdd_js_1);
      ASSERT_STR_EQ("3.1.2", _cdd_js_1);
    }
    srv = json_array_get_object(srv_arr, 0);

    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(srv, "url", &_cdd_js_1);
      ASSERT_STR_EQ("https://api.example.com", _cdd_js_1);
    }
    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(srv, "description", &_cdd_js_1);
      ASSERT_STR_EQ("Prod", _cdd_js_1);
    }
    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(srv, "name", &_cdd_js_1);
      ASSERT_STR_EQ("prod", _cdd_js_1);
    }
    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_writer_querystring_param.
 * @return Greatest test result.
 */
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
  path.route = cdd_test_mut_str("/search");
  path.operations = &op;
  path.n_operations = 1;
  op.verb = OA_VERB_GET;
  op.operation_id = cdd_test_mut_str("search");
  op.parameters = &param;
  op.n_parameters = 1;
  param.name = cdd_test_mut_str("qs");
  param.in = OA_PARAM_IN_QUERYSTRING;
  param.type = cdd_test_mut_str("string");
  param.content_type = cdd_test_mut_str("application/x-www-form-urlencoded");

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
      cdd_json_object_get_object(op_obj, "/search", &_cdd_jo_1);
      op_obj = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(op_obj, "get", &_cdd_jo_1);
      op_obj = _cdd_jo_1;
    }
    {
      JSON_Array *oa_params;
      JSON_Object *p_obj;
      JSON_Object *content;
      JSON_Object *schema;
      JSON_Object *media;
      {
        JSON_Array *_cdd_ja_1 = NULL;
        cdd_json_object_get_array(op_obj, "parameters", &_cdd_ja_1);
        oa_params = _cdd_ja_1;
      }
      p_obj = json_array_get_object(oa_params, 0);
      {
        JSON_Object *_cdd_jo_1 = NULL;
        cdd_json_object_get_object(p_obj, "content", &_cdd_jo_1);
        content = _cdd_jo_1;
      }
      {
        JSON_Object *_cdd_jo_1 = NULL;
        cdd_json_object_get_object(content, "application/x-www-form-urlencoded",
                                   &_cdd_jo_1);
        media = _cdd_jo_1;
      }
      {
        JSON_Object *_cdd_jo_1 = NULL;
        cdd_json_object_get_object(media, "schema", &_cdd_jo_1);
        schema = _cdd_jo_1;
      }
      {
        const char *_cdd_js_1 = NULL;
        cdd_json_object_get_string(p_obj, "in", &_cdd_js_1);
        ASSERT_STR_EQ("querystring", _cdd_js_1);
      }
      {
        JSON_Object *_cdd_jo_1 = NULL;
        cdd_json_object_get_object(p_obj, "schema", &_cdd_jo_1);
        ASSERT(_cdd_jo_1 == NULL);
      }
      {
        const char *_cdd_js_1 = NULL;
        cdd_json_object_get_string(schema, "type", &_cdd_js_1);
        ASSERT_STR_EQ("string", _cdd_js_1);
      }
    }

    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_writer_ignores_reserved_header_params.
 * @return Greatest test result.
 */
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
  path.route = cdd_test_mut_str("/h");
  path.operations = &op;
  path.n_operations = 1;
  op.verb = OA_VERB_GET;
  op.operation_id = cdd_test_mut_str("getH");
  op.parameters = params;
  op.n_parameters = 2;

  params[0].name = cdd_test_mut_str("Accept");
  params[0].in = OA_PARAM_IN_HEADER;
  params[0].type = cdd_test_mut_str("string");

  params[1].name = cdd_test_mut_str("q");
  params[1].in = OA_PARAM_IN_QUERY;
  params[1].type = cdd_test_mut_str("string");

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root;
    JSON_Object *op_obj;
    JSON_Array *oa_params;
    JSON_Object *p_obj;
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
      cdd_json_object_get_object(op_obj, "/h", &_cdd_jo_1);
      op_obj = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(op_obj, "get", &_cdd_jo_1);
      op_obj = _cdd_jo_1;
    }
    {
      JSON_Array *_cdd_ja_1 = NULL;
      cdd_json_object_get_array(op_obj, "parameters", &_cdd_ja_1);
      oa_params = _cdd_ja_1;
    }
    ASSERT_EQ(1, (int)json_array_get_count(oa_params));
    p_obj = json_array_get_object(oa_params, 0);
    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(p_obj, "name", &_cdd_js_1);
      ASSERT_STR_EQ("q", _cdd_js_1);
    }
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
