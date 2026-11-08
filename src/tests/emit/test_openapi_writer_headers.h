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

/**
 * @brief Unit test test_writer_ignores_content_type_response_header.
 * @return Greatest test result.
 */
TEST test_writer_ignores_content_type_response_header(void) {
  int rc = 0;
  char *json;
  struct OpenAPI_Spec spec = {0};
  struct OpenAPI_Path path = {0};
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Response resp = {0};
  struct OpenAPI_Header headers[2] = {{0}};
  json = NULL;

  spec.paths = &path;
  spec.n_paths = 1;
  path.route = cdd_test_mut_str("/r");
  path.operations = &op;
  path.n_operations = 1;
  op.verb = OA_VERB_GET;
  op.operation_id = cdd_test_mut_str("getR");
  op.responses = &resp;
  op.n_responses = 1;

  resp.code = cdd_test_mut_str("200");
  resp.description = cdd_test_mut_str("ok");
  resp.headers = headers;
  resp.n_headers = 2;

  headers[0].name = cdd_test_mut_str("Content-Type");
  headers[0].type = cdd_test_mut_str("string");

  headers[1].name = cdd_test_mut_str("X-Rate");
  headers[1].type = cdd_test_mut_str("integer");

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root;
    JSON_Object *op_obj;
    JSON_Object *resp_obj;
    JSON_Object *headers_obj;
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
      cdd_json_object_get_object(op_obj, "/r", &_cdd_jo_1);
      op_obj = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(op_obj, "get", &_cdd_jo_1);
      op_obj = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(op_obj, "responses", &_cdd_jo_1);
      resp_obj = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(resp_obj, "200", &_cdd_jo_1);
      resp_obj = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(resp_obj, "headers", &_cdd_jo_1);
      headers_obj = _cdd_jo_1;
    }
    ASSERT(headers_obj != NULL);
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(headers_obj, "Content-Type", &_cdd_jo_1);
      ASSERT(_cdd_jo_1 == NULL);
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(headers_obj, "X-Rate", &_cdd_jo_1);
      ASSERT(_cdd_jo_1 != NULL);
    }
    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_writer_path_level_parameters.
 * @return Greatest test result.
 */
TEST test_writer_path_level_parameters(void) {
  int rc = 0;
  char *json;
  struct OpenAPI_Spec spec = {0};
  struct OpenAPI_Path path = {0};
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Parameter pparam = {0};
  json = NULL;

  spec.paths = &path;
  spec.n_paths = 1;
  path.route = cdd_test_mut_str("/pets");
  path.summary = cdd_test_mut_str("Pets");
  path.description = cdd_test_mut_str("All pets");
  path.parameters = &pparam;
  path.n_parameters = 1;
  path.operations = &op;
  path.n_operations = 1;
  op.verb = OA_VERB_GET;
  op.operation_id = cdd_test_mut_str("listPets");

  pparam.name = cdd_test_mut_str("x-trace");
  pparam.in = OA_PARAM_IN_HEADER;
  pparam.type = cdd_test_mut_str("string");

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root;
    JSON_Object *item;
    JSON_Array *oa_params;
    JSON_Object *p_obj;
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
      item = _cdd_jo_1;
    }
    {
      JSON_Array *_cdd_ja_1 = NULL;
      cdd_json_object_get_array(item, "parameters", &_cdd_ja_1);
      oa_params = _cdd_ja_1;
    }
    p_obj = json_array_get_object(oa_params, 0);

    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(item, "summary", &_cdd_js_1);
      ASSERT_STR_EQ("Pets", _cdd_js_1);
    }
    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(item, "description", &_cdd_js_1);
      ASSERT_STR_EQ("All pets", _cdd_js_1);
    }
    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(p_obj, "name", &_cdd_js_1);
      ASSERT_STR_EQ("x-trace", _cdd_js_1);
    }
    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(p_obj, "in", &_cdd_js_1);
      ASSERT_STR_EQ("header", _cdd_js_1);
    }

    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_writer_server_variables.
 * @return Greatest test result.
 */
TEST test_writer_server_variables(void) {
  int rc = 0;
  char *enum_vals[2];
  char *json;
  struct OpenAPI_Spec spec = {0};
  struct OpenAPI_Server server = {0};
  struct OpenAPI_ServerVariable var = {0};
  json = NULL;

  enum_vals[0] = cdd_test_mut_str("prod");
  enum_vals[1] = cdd_test_mut_str("staging");

  var.name = cdd_test_mut_str("env");
  var.default_value = cdd_test_mut_str("prod");
  var.description = cdd_test_mut_str("Environment");
  var.enum_values = enum_vals;
  var.n_enum_values = 2;

  server.url = cdd_test_mut_str("https://{env}.example.com");
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
    {
      JSON_Value *_cdd_jv_1 = NULL;
      cdd_json_parse_string(json, &_cdd_jv_1);
      root = _cdd_jv_1;
    }
    obj = json_value_get_object(root);
    {
      JSON_Array *_cdd_ja_1 = NULL;
      cdd_json_object_get_array(obj, "servers", &_cdd_ja_1);
      srv_arr = _cdd_ja_1;
    }
    srv_obj = json_array_get_object(srv_arr, 0);
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(srv_obj, "variables", &_cdd_jo_1);
      vars = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(vars, "env", &_cdd_jo_1);
      env = _cdd_jo_1;
    }
    {
      JSON_Array *_cdd_ja_1 = NULL;
      cdd_json_object_get_array(env, "enum", &_cdd_ja_1);
      enum_arr = _cdd_ja_1;
    }

    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(env, "default", &_cdd_js_1);
      ASSERT_STR_EQ("prod", _cdd_js_1);
    }
    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(env, "description", &_cdd_js_1);
      ASSERT_STR_EQ("Environment", _cdd_js_1);
    }
    ASSERT_STR_EQ("prod", json_array_get_string(enum_arr, 0));
    ASSERT_STR_EQ("staging", json_array_get_string(enum_arr, 1));

    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_writer_security_schemes.
 * @return Greatest test result.
 */
TEST test_writer_security_schemes(void) {
  int rc = 0;
  struct OpenAPI_SecurityScheme s1, s2;
  char *json;
  struct OpenAPI_Spec spec = {0};
  struct OpenAPI_SecurityScheme schemes[3];
  struct OpenAPI_SecurityScheme s3;
  json = NULL;

  memset(&s1, 0, sizeof(s1));
  memset(&s2, 0, sizeof(s2));
  memset(&s3, 0, sizeof(s3));

  s1.name = cdd_test_mut_str("bearerAuth");
  s1.type = OA_SEC_HTTP;
  s1.scheme = cdd_test_mut_str("bearer");
  s1.bearer_format = cdd_test_mut_str("Opaque");

  s2.name = cdd_test_mut_str("apiKeyAuth");
  s2.type = OA_SEC_APIKEY;
  s2.in = OA_SEC_IN_HEADER;
  s2.key_name = cdd_test_mut_str("X-Api-Key");

  schemes[0] = s1;
  schemes[1] = s2;
  s3.name = cdd_test_mut_str("mtlsAuth");
  s3.type = OA_SEC_MUTUALTLS;
  s3.description = cdd_test_mut_str("mTLS only");
  schemes[2] = s3;

  spec.security_schemes = schemes;
  spec.n_security_schemes = 3;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);

  {
    JSON_Value *root;
    JSON_Object *secs;
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
      secs = _cdd_jo_1;
    }

    /* Check Bearer */
    {
      JSON_Object *b;
      {
        JSON_Object *_cdd_jo_1 = NULL;
        cdd_json_object_get_object(secs, "bearerAuth", &_cdd_jo_1);
        b = _cdd_jo_1;
      }
      ASSERT(b != NULL);
      {
        const char *_cdd_js_1 = NULL;
        cdd_json_object_get_string(b, "type", &_cdd_js_1);
        ASSERT_STR_EQ("http", _cdd_js_1);
      }
      {
        const char *_cdd_js_1 = NULL;
        cdd_json_object_get_string(b, "scheme", &_cdd_js_1);
        ASSERT_STR_EQ("bearer", _cdd_js_1);
      }
      {
        const char *_cdd_js_1 = NULL;
        cdd_json_object_get_string(b, "bearerFormat", &_cdd_js_1);
        ASSERT_STR_EQ("Opaque", _cdd_js_1);
      }
    }

    /* Check ApiKey */
    {
      JSON_Object *k;
      {
        JSON_Object *_cdd_jo_1 = NULL;
        cdd_json_object_get_object(secs, "apiKeyAuth", &_cdd_jo_1);
        k = _cdd_jo_1;
      }
      ASSERT(k != NULL);
      {
        const char *_cdd_js_1 = NULL;
        cdd_json_object_get_string(k, "type", &_cdd_js_1);
        ASSERT_STR_EQ("apiKey", _cdd_js_1);
      }
      {
        const char *_cdd_js_1 = NULL;
        cdd_json_object_get_string(k, "in", &_cdd_js_1);
        ASSERT_STR_EQ("header", _cdd_js_1);
      }
      {
        const char *_cdd_js_1 = NULL;
        cdd_json_object_get_string(k, "name", &_cdd_js_1);
        ASSERT_STR_EQ("X-Api-Key", _cdd_js_1);
      }
    }

    /* Check MutualTLS */
    {
      JSON_Object *m;
      {
        JSON_Object *_cdd_jo_1 = NULL;
        cdd_json_object_get_object(secs, "mtlsAuth", &_cdd_jo_1);
        m = _cdd_jo_1;
      }
      ASSERT(m != NULL);
      {
        const char *_cdd_js_1 = NULL;
        cdd_json_object_get_string(m, "type", &_cdd_js_1);
        ASSERT_STR_EQ("mutualTLS", _cdd_js_1);
      }
      {
        const char *_cdd_js_1 = NULL;
        cdd_json_object_get_string(m, "description", &_cdd_js_1);
        ASSERT_STR_EQ("mTLS only", _cdd_js_1);
      }
    }

    json_value_free(root);
  }
  free(json);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_writer_security_requirements.
 * @return Greatest test result.
 */
TEST test_writer_security_requirements(void) {
  int rc = 0;
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

  root_req.scheme = cdd_test_mut_str("bearerAuth");
  root_set.requirements = &root_req;
  root_set.n_requirements = 1;

  spec.security = &root_set;
  spec.n_security = 1;
  spec.security_set = 1;

  op_req.scheme = cdd_test_mut_str("ApiKeyAuth");
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
    {
      JSON_Value *_cdd_jv_1 = NULL;
      cdd_json_parse_string(json, &_cdd_jv_1);
      root = _cdd_jv_1;
    }
    root_obj = json_value_get_object(root);
    {
      JSON_Array *_cdd_ja_1 = NULL;
      cdd_json_object_get_array(root_obj, "security", &_cdd_ja_1);
      root_sec = _cdd_ja_1;
    }
    root_req_obj = json_array_get_object(root_sec, 0);
    {
      JSON_Array *_cdd_ja_1 = NULL;
      cdd_json_object_get_array(root_req_obj, "bearerAuth", &_cdd_ja_1);
      root_scopes = _cdd_ja_1;
    }

    ASSERT(root_scopes != NULL);

    {
      JSON_Array *op_sec;
      JSON_Object *op_req_obj;
      JSON_Object *op_obj;
      {
        JSON_Object *_cdd_jo_1 = NULL;
        JSON_Object *_cdd_jo_2 = NULL;
        JSON_Object *_cdd_jo_3 = NULL;
        cdd_json_object_get_object(root_obj, "paths", &_cdd_jo_1);
        cdd_json_object_get_object(_cdd_jo_1, "/test/route", &_cdd_jo_2);
        cdd_json_object_get_object(_cdd_jo_2, "get", &_cdd_jo_3);
        op_obj = _cdd_jo_3;
      }
      {
        JSON_Array *_cdd_ja_1 = NULL;
        cdd_json_object_get_array(op_obj, "security", &_cdd_ja_1);
        op_sec = _cdd_ja_1;
      }
      op_req_obj = json_array_get_object(op_sec, 0);
      {
        JSON_Array *_cdd_ja_1 = NULL;
        cdd_json_object_get_array(op_req_obj, "ApiKeyAuth", &_cdd_ja_1);
        ASSERT(_cdd_ja_1 != NULL);
      }
    }

    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_writer_multipart_schema.
 * @return Greatest test result.
 */
TEST test_writer_multipart_schema(void) {
  int rc = 0;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Path path;
  struct OpenAPI_MultipartField parts[2];
  char *json;
  struct OpenAPI_Operation op = {0};
  json = NULL;

  memset(&parts, 0, sizeof(parts));
  parts[0].name = cdd_test_mut_str("file");
  parts[0].is_binary = 1; /* File upload */
  parts[1].name = cdd_test_mut_str("desc");
  parts[1].type = cdd_test_mut_str("string");

  setup_test_spec(&spec, &path, &op, NULL, NULL);
  op.verb = OA_VERB_POST;
  op.req_body.content_type = cdd_test_mut_str("multipart/form-data");
  op.req_body.multipart_fields = parts;
  op.req_body.n_multipart_fields = 2;

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
      JSON_Object *req;
      JSON_Object *cnt;
      JSON_Object *mp;
      JSON_Object *sch;
      JSON_Object *props;
      {
        JSON_Object *_cdd_jo_1 = NULL;
        cdd_json_object_get_object(op_obj, "requestBody", &_cdd_jo_1);
        req = _cdd_jo_1;
      }
      {
        JSON_Object *_cdd_jo_1 = NULL;
        cdd_json_object_get_object(req, "content", &_cdd_jo_1);
        cnt = _cdd_jo_1;
      }
      {
        JSON_Object *_cdd_jo_1 = NULL;
        cdd_json_object_get_object(cnt, "multipart/form-data", &_cdd_jo_1);
        mp = _cdd_jo_1;
      }
      {
        JSON_Object *_cdd_jo_1 = NULL;
        cdd_json_object_get_object(mp, "schema", &_cdd_jo_1);
        sch = _cdd_jo_1;
      }
      {
        JSON_Object *_cdd_jo_1 = NULL;
        cdd_json_object_get_object(sch, "properties", &_cdd_jo_1);
        props = _cdd_jo_1;
      }

      {
        const char *_cdd_js_1 = NULL;
        cdd_json_object_get_string(sch, "type", &_cdd_js_1);
        ASSERT_STR_EQ("object", _cdd_js_1);
      }

      /* Check File */
      {
        JSON_Object *f;
        {
          JSON_Object *_cdd_jo_1 = NULL;
          cdd_json_object_get_object(props, "file", &_cdd_jo_1);
          f = _cdd_jo_1;
        }
        {
          const char *_cdd_js_1 = NULL;
          cdd_json_object_get_string(f, "type", &_cdd_js_1);
          ASSERT_STR_EQ("string", _cdd_js_1);
        }
        {
          const char *_cdd_js_1 = NULL;
          cdd_json_object_get_string(f, "format", &_cdd_js_1);
          ASSERT_STR_EQ("binary", _cdd_js_1);
        }
      }
      /* Check Desc */
      {
        JSON_Object *d;
        {
          JSON_Object *_cdd_jo_1 = NULL;
          cdd_json_object_get_object(props, "desc", &_cdd_jo_1);
          d = _cdd_jo_1;
        }
        {
          const char *_cdd_js_1 = NULL;
          cdd_json_object_get_string(d, "type", &_cdd_js_1);
          ASSERT_STR_EQ("string", _cdd_js_1);
        }
      }
    }
    json_value_free(root);
  }
  free(json);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_writer_components_and_response_headers.
 * @return Greatest test result.
 */
TEST test_writer_components_and_response_headers(void) {
  int rc = 0;
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

  param_names[0] = cdd_test_mut_str("LimitParam");
  resp_names[0] = cdd_test_mut_str("NotFound");
  hdr_names[0] = cdd_test_mut_str("RateLimit");

  comp_param.name = cdd_test_mut_str("limit");
  comp_param.in = OA_PARAM_IN_QUERY;
  comp_param.type = cdd_test_mut_str("integer");
  spec.component_parameters = &comp_param;
  spec.component_parameter_names = param_names;
  spec.n_component_parameters = 1;

  comp_resp.description = cdd_test_mut_str("missing");
  spec.component_responses = &comp_resp;
  spec.component_response_names = resp_names;
  spec.n_component_responses = 1;

  comp_hdr.type = cdd_test_mut_str("integer");
  spec.component_headers = &comp_hdr;
  spec.component_header_names = hdr_names;
  spec.n_component_headers = 1;

  op_param.ref = cdd_test_mut_str("#/components/parameters/LimitParam");
  op.parameters = &op_param;
  op.n_parameters = 1;

  responses[0].code = cdd_test_mut_str("200");
  responses[0].description = cdd_test_mut_str("ok");
  resp_hdr.name = cdd_test_mut_str("X-Rate");
  resp_hdr.ref = cdd_test_mut_str("#/components/headers/RateLimit");
  responses[0].headers = &resp_hdr;
  responses[0].n_headers = 1;

  responses[1].code = cdd_test_mut_str("404");
  responses[1].ref = cdd_test_mut_str("#/components/responses/NotFound");

  op.responses = responses;
  op.n_responses = 2;
  op.verb = OA_VERB_GET;
  op.operation_id = cdd_test_mut_str("listItems");

  path.route = cdd_test_mut_str("/items");
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
    {
      JSON_Value *_cdd_jv_1 = NULL;
      cdd_json_parse_string(json, &_cdd_jv_1);
      root = _cdd_jv_1;
    }
    root_obj = json_value_get_object(root);
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(root_obj, "components", &_cdd_jo_1);
      components = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(components, "parameters", &_cdd_jo_1);
      params = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(components, "headers", &_cdd_jo_1);
      hdrs = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(components, "responses", &_cdd_jo_1);
      resps = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(root_obj, "paths", &_cdd_jo_1);
      paths = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(paths, "/items", &_cdd_jo_1);
      item = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(item, "get", &_cdd_jo_1);
      get = _cdd_jo_1;
    }
    {
      JSON_Array *_cdd_ja_1 = NULL;
      cdd_json_object_get_array(get, "parameters", &_cdd_ja_1);
      params_arr = _cdd_ja_1;
    }
    p0 = json_array_get_object(params_arr, 0);
    {
      JSON_Object *_cdd_jo_1 = NULL;
      JSON_Object *_cdd_jo_2 = NULL;
      cdd_json_object_get_object(get, "responses", &_cdd_jo_1);
      cdd_json_object_get_object(_cdd_jo_1, "200", &_cdd_jo_2);
      resp200 = _cdd_jo_2;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      JSON_Object *_cdd_jo_2 = NULL;
      cdd_json_object_get_object(get, "responses", &_cdd_jo_1);
      cdd_json_object_get_object(_cdd_jo_1, "404", &_cdd_jo_2);
      resp404 = _cdd_jo_2;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(resp200, "headers", &_cdd_jo_1);
      resp200_hdrs = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(resp200_hdrs, "X-Rate", &_cdd_jo_1);
      x_rate = _cdd_jo_1;
    }

    ASSERT(params != NULL);
    ASSERT(hdrs != NULL);
    ASSERT(resps != NULL);
    {
      JSON_Object *_cdd_jo_1 = NULL;
      const char *_cdd_js_2 = NULL;
      cdd_json_object_get_object(params, "LimitParam", &_cdd_jo_1);
      cdd_json_object_get_string(_cdd_jo_1, "name", &_cdd_js_2);
      ASSERT_STR_EQ("limit", _cdd_js_2);
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      JSON_Object *_cdd_jo_2 = NULL;
      const char *_cdd_js_3 = NULL;
      cdd_json_object_get_object(hdrs, "RateLimit", &_cdd_jo_1);
      cdd_json_object_get_object(_cdd_jo_1, "schema", &_cdd_jo_2);
      cdd_json_object_get_string(_cdd_jo_2, "type", &_cdd_js_3);
      ASSERT_STR_EQ("integer", _cdd_js_3);
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      const char *_cdd_js_2 = NULL;
      cdd_json_object_get_object(resps, "NotFound", &_cdd_jo_1);
      cdd_json_object_get_string(_cdd_jo_1, "description", &_cdd_js_2);
      ASSERT_STR_EQ("missing", _cdd_js_2);
    }

    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(p0, "$ref", &_cdd_js_1);
      ASSERT_STR_EQ("#/components/parameters/LimitParam", _cdd_js_1);
    }
    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(x_rate, "$ref", &_cdd_js_1);
      ASSERT_STR_EQ("#/components/headers/RateLimit", _cdd_js_1);
    }
    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(resp404, "$ref", &_cdd_js_1);
      ASSERT_STR_EQ("#/components/responses/NotFound", _cdd_js_1);
    }

    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_writer_components_request_bodies.
 * @return Greatest test result.
 */
TEST test_writer_components_request_bodies(void) {
  int rc = 0;
  char *rb_names[1];
  char *json;
  struct OpenAPI_Spec spec = {0};
  struct OpenAPI_Path path = {0};
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_RequestBody comp_rb = {0};
  json = NULL;

  setup_test_spec(&spec, &path, &op, NULL, NULL);
  op.verb = OA_VERB_POST;
  op.req_body_ref = cdd_test_mut_str("#/components/requestBodies/CreatePet");

  comp_rb.description = cdd_test_mut_str("Create");
  comp_rb.required_set = 1;
  comp_rb.required = 1;
  comp_rb.schema.content_type = cdd_test_mut_str("application/json");
  comp_rb.schema.ref_name = cdd_test_mut_str("Pet");

  rb_names[0] = cdd_test_mut_str("CreatePet");
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
    {
      JSON_Value *_cdd_jv_1 = NULL;
      cdd_json_parse_string(json, &_cdd_jv_1);
      root = _cdd_jv_1;
    }
    root_obj = json_value_get_object(root);
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(root_obj, "components", &_cdd_jo_1);
      components = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(components, "requestBodies", &_cdd_jo_1);
      rbs = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(rbs, "CreatePet", &_cdd_jo_1);
      create = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(create, "content", &_cdd_jo_1);
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
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(root_obj, "paths", &_cdd_jo_1);
      paths = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(paths, "/test/route", &_cdd_jo_1);
      p_item = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(p_item, "post", &_cdd_jo_1);
      post = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(post, "requestBody", &_cdd_jo_1);
      req = _cdd_jo_1;
    }

    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(create, "description", &_cdd_js_1);
      ASSERT_STR_EQ("Create", _cdd_js_1);
    }
    {
      int _cdd_jb_1 = -1;
      if (cdd_json_object_get_boolean(create, "required", &_cdd_jb_1) !=
          CDD_C_SUCCESS)
        _cdd_jb_1 = -1;
      ASSERT_EQ(1, _cdd_jb_1);
    }
    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(schema, "$ref", &_cdd_js_1);
      ASSERT_STR_EQ("#/components/schemas/Pet", _cdd_js_1);
    }
    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(req, "$ref", &_cdd_js_1);
      ASSERT_STR_EQ("#/components/requestBodies/CreatePet", _cdd_js_1);
    }

    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_writer_components_schemas.
 * @return Greatest test result.
 */
TEST test_writer_components_schemas(void) {
  int rc = 0;
  struct OpenAPI_Spec spec;
  struct StructFields sf;
  char *name;
  char *json;
  name = cdd_test_mut_str("MyModel");
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
      cdd_json_object_get_object(comps, "schemas", &_cdd_jo_1);
      schemas = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(schemas, "MyModel", &_cdd_jo_1);
      model = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(model, "properties", &_cdd_jo_1);
      props = _cdd_jo_1;
    }
    {
      JSON_Object *_cdd_jo_1 = NULL;
      cdd_json_object_get_object(props, "id", &_cdd_jo_1);
      id_prop = _cdd_jo_1;
    }

    {
      const char *_cdd_js_1 = NULL;
      cdd_json_object_get_string(id_prop, "type", &_cdd_js_1);
      ASSERT_STR_EQ("integer", _cdd_js_1);
    }

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
