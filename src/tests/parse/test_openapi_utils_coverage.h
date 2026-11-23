/**
 * @file test_openapi_utils_coverage.h
 * @brief Comprehensive 100% test coverage for openapi_utils.c.
 * @author Samuel Marks
 */

#ifndef TEST_OPENAPI_UTILS_COVERAGE_H
#define TEST_OPENAPI_UTILS_COVERAGE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "parse/test_openapi_loader_common.h"
/* clang-format on */

/**
 * @brief Tests parse_verb, is_fixed_operation_method, and parse_param_in.
 * @return GREATEST_TEST_RES
 */
TEST test_openapi_utils_verbs_and_params(void) {
  enum OpenAPI_Verb verb;
  enum OpenAPI_ParamIn p_in;
  cdd_c_error_t rc = 0;

  /* 1. parse_verb */
  rc = parse_verb(NULL, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = parse_verb(NULL, &verb);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_VERB_UNKNOWN, verb);

  rc = parse_verb("get", &verb);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_VERB_GET, verb);
  rc = parse_verb("post", &verb);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_VERB_POST, verb);
  rc = parse_verb("put", &verb);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_VERB_PUT, verb);
  rc = parse_verb("delete", &verb);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_VERB_DELETE, verb);
  rc = parse_verb("patch", &verb);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_VERB_PATCH, verb);
  rc = parse_verb("head", &verb);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_VERB_HEAD, verb);
  rc = parse_verb("options", &verb);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_VERB_OPTIONS, verb);
  rc = parse_verb("trace", &verb);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_VERB_TRACE, verb);
  rc = parse_verb("query", &verb);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_VERB_QUERY, verb);
  rc = parse_verb("custom_verb", &verb);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_VERB_UNKNOWN, verb);

  /* 2. is_fixed_operation_method */
  {
    int bval = 0;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              is_fixed_operation_method("get", NULL));
    ASSERT_EQ(CDD_C_SUCCESS, is_fixed_operation_method(NULL, &bval));
    ASSERT_EQ(0, bval);
    ASSERT_EQ(CDD_C_SUCCESS, is_fixed_operation_method("get", &bval));
    ASSERT_EQ(1, bval);
    ASSERT_EQ(CDD_C_SUCCESS, is_fixed_operation_method("POST", &bval));
    ASSERT_EQ(1, bval);
    ASSERT_EQ(CDD_C_SUCCESS, is_fixed_operation_method("Put", &bval));
    ASSERT_EQ(1, bval);
    ASSERT_EQ(CDD_C_SUCCESS, is_fixed_operation_method("DELETE", &bval));
    ASSERT_EQ(1, bval);
    ASSERT_EQ(CDD_C_SUCCESS, is_fixed_operation_method("patch", &bval));
    ASSERT_EQ(1, bval);
    ASSERT_EQ(CDD_C_SUCCESS, is_fixed_operation_method("HEAD", &bval));
    ASSERT_EQ(1, bval);
    ASSERT_EQ(CDD_C_SUCCESS, is_fixed_operation_method("options", &bval));
    ASSERT_EQ(1, bval);
    ASSERT_EQ(CDD_C_SUCCESS, is_fixed_operation_method("TRACE", &bval));
    ASSERT_EQ(1, bval);
    ASSERT_EQ(CDD_C_SUCCESS, is_fixed_operation_method("query", &bval));
    ASSERT_EQ(1, bval);
    ASSERT_EQ(CDD_C_SUCCESS, is_fixed_operation_method("foobar", &bval));
    ASSERT_EQ(0, bval);

    {
      int eq = 0;
      extern C_CDD_EXPORT int g_cdd_fail_str_iequal;
      g_cdd_fail_str_iequal = 1;
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                is_fixed_operation_method("get", &bval));
      g_cdd_fail_str_iequal = 0;

      g_cdd_fail_str_iequal = 2;
      ASSERT_EQ(CDD_C_SUCCESS, c_cdd_str_iequal("a", "b", &eq));
      g_cdd_fail_str_iequal = 0;
    }
  }

  /* 3. parse_param_in */
  rc = parse_param_in(NULL, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = parse_param_in(NULL, &p_in);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_PARAM_IN_UNKNOWN, p_in);

  rc = parse_param_in("path", &p_in);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_PARAM_IN_PATH, p_in);
  rc = parse_param_in("query", &p_in);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_PARAM_IN_QUERY, p_in);
  rc = parse_param_in("querystring", &p_in);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_PARAM_IN_QUERYSTRING, p_in);
  rc = parse_param_in("header", &p_in);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_PARAM_IN_HEADER, p_in);
  rc = parse_param_in("cookie", &p_in);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_PARAM_IN_COOKIE, p_in);
  rc = parse_param_in("body", &p_in);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_PARAM_IN_BODY, p_in);
  rc = parse_param_in("formData", &p_in);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_PARAM_IN_FORM_DATA, p_in);
  rc = parse_param_in("unknown", &p_in);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_PARAM_IN_UNKNOWN, p_in);

  PASS();
}

/**
 * @brief Tests parse_param_style, param_type_is_primitive, and
 * validate_parameter_style.
 * @return GREATEST_TEST_RES
 */
TEST test_openapi_utils_param_style_and_types(void) {
  enum OpenAPI_Style style;
  struct OpenAPI_Parameter p;
  cdd_c_error_t rc = 0;

  /* 1. parse_param_style */
  rc = parse_param_style(NULL, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = parse_param_style(NULL, &style);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_STYLE_UNKNOWN, style);

  rc = parse_param_style("form", &style);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_STYLE_FORM, style);
  rc = parse_param_style("simple", &style);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_STYLE_SIMPLE, style);
  rc = parse_param_style("matrix", &style);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_STYLE_MATRIX, style);
  rc = parse_param_style("label", &style);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_STYLE_LABEL, style);
  rc = parse_param_style("spaceDelimited", &style);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_STYLE_SPACE_DELIMITED, style);
  rc = parse_param_style("pipeDelimited", &style);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_STYLE_PIPE_DELIMITED, style);
  rc = parse_param_style("deepObject", &style);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_STYLE_DEEP_OBJECT, style);
  rc = parse_param_style("cookie", &style);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_STYLE_COOKIE, style);
  rc = parse_param_style("custom", &style);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_STYLE_UNKNOWN, style);

  /* 2. param_type_is_primitive */
  {
    int bval = 0;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              param_type_is_primitive("string", NULL));
    ASSERT_EQ(CDD_C_SUCCESS, param_type_is_primitive(NULL, &bval));
    ASSERT_EQ(0, bval);
    ASSERT_EQ(CDD_C_SUCCESS, param_type_is_primitive("string", &bval));
    ASSERT_EQ(1, bval);
    ASSERT_EQ(CDD_C_SUCCESS, param_type_is_primitive("integer", &bval));
    ASSERT_EQ(1, bval);
    ASSERT_EQ(CDD_C_SUCCESS, param_type_is_primitive("number", &bval));
    ASSERT_EQ(1, bval);
    ASSERT_EQ(CDD_C_SUCCESS, param_type_is_primitive("boolean", &bval));
    ASSERT_EQ(1, bval);
    ASSERT_EQ(CDD_C_SUCCESS, param_type_is_primitive("object", &bval));
    ASSERT_EQ(0, bval);
    ASSERT_EQ(CDD_C_SUCCESS, param_type_is_primitive("array", &bval));
    ASSERT_EQ(0, bval);

    /* 3. param_type_is_object_like */
    memset(&p, 0, sizeof(p));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              param_type_is_object_like(&p, NULL));
    ASSERT_EQ(CDD_C_SUCCESS, param_type_is_object_like(NULL, &bval));
    ASSERT_EQ(0, bval);
    ASSERT_EQ(CDD_C_SUCCESS, param_type_is_object_like(&p, &bval));
    ASSERT_EQ(0, bval);
    p.type = cdd_test_mut_str("array");
    ASSERT_EQ(CDD_C_SUCCESS, param_type_is_object_like(&p, &bval));
    ASSERT_EQ(0, bval);
    p.type = cdd_test_mut_str("string");
    ASSERT_EQ(CDD_C_SUCCESS, param_type_is_object_like(&p, &bval));
    ASSERT_EQ(0, bval);
    p.type = cdd_test_mut_str("MyModel");
    ASSERT_EQ(CDD_C_SUCCESS, param_type_is_object_like(&p, &bval));
    ASSERT_EQ(1, bval);
  }

  /* 4. validate_parameter_style */
  ASSERT_EQ(CDD_C_SUCCESS, validate_parameter_style(NULL, 0));
  ASSERT_EQ(CDD_C_SUCCESS, validate_parameter_style(&p, 1));

  p.in = OA_PARAM_IN_QUERYSTRING;
  ASSERT_EQ(CDD_C_SUCCESS, validate_parameter_style(&p, 0));

  /* Query: valid styles */
  p.in = OA_PARAM_IN_QUERY;
  p.style = OA_STYLE_FORM;
  ASSERT_EQ(CDD_C_SUCCESS, validate_parameter_style(&p, 0));
  p.style = OA_STYLE_SPACE_DELIMITED;
  p.is_array = 1;
  ASSERT_EQ(CDD_C_SUCCESS, validate_parameter_style(&p, 0));
  p.style = OA_STYLE_PIPE_DELIMITED;
  ASSERT_EQ(CDD_C_SUCCESS, validate_parameter_style(&p, 0));
  p.style = OA_STYLE_DEEP_OBJECT;
  p.is_array = 0;
  p.type = cdd_test_mut_str("MyObj");
  ASSERT_EQ(CDD_C_SUCCESS, validate_parameter_style(&p, 0));

  /* Query: invalid style */
  p.style = OA_STYLE_MATRIX;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, validate_parameter_style(&p, 0));

  /* Path: valid and invalid */
  p.in = OA_PARAM_IN_PATH;
  p.style = OA_STYLE_SIMPLE;
  ASSERT_EQ(CDD_C_SUCCESS, validate_parameter_style(&p, 0));
  p.style = OA_STYLE_MATRIX;
  ASSERT_EQ(CDD_C_SUCCESS, validate_parameter_style(&p, 0));
  p.style = OA_STYLE_LABEL;
  ASSERT_EQ(CDD_C_SUCCESS, validate_parameter_style(&p, 0));
  p.style = OA_STYLE_FORM;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, validate_parameter_style(&p, 0));

  /* Header: valid and invalid */
  p.in = OA_PARAM_IN_HEADER;
  p.style = OA_STYLE_SIMPLE;
  ASSERT_EQ(CDD_C_SUCCESS, validate_parameter_style(&p, 0));
  p.style = OA_STYLE_FORM;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, validate_parameter_style(&p, 0));

  /* Cookie: valid and invalid */
  p.in = OA_PARAM_IN_COOKIE;
  p.style = OA_STYLE_FORM;
  ASSERT_EQ(CDD_C_SUCCESS, validate_parameter_style(&p, 0));
  p.style = OA_STYLE_COOKIE;
  ASSERT_EQ(CDD_C_SUCCESS, validate_parameter_style(&p, 0));
  p.style = OA_STYLE_SIMPLE;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, validate_parameter_style(&p, 0));

  /* Default in branch */
  p.in = OA_PARAM_IN_BODY;
  p.style = OA_STYLE_FORM;
  ASSERT_EQ(CDD_C_SUCCESS, validate_parameter_style(&p, 0));

  /* DeepObject errors: array or primitive */
  p.in = OA_PARAM_IN_QUERY;
  p.style = OA_STYLE_DEEP_OBJECT;
  p.is_array = 1;
  p.type = cdd_test_mut_str("MyObj");
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, validate_parameter_style(&p, 0));
  p.is_array = 0;
  p.type = cdd_test_mut_str("string");
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, validate_parameter_style(&p, 0));

  /* spaceDelimited / pipeDelimited error: neither array nor object-like */
  p.style = OA_STYLE_SPACE_DELIMITED;
  p.is_array = 0;
  p.type = cdd_test_mut_str("integer");
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, validate_parameter_style(&p, 0));
  p.style = OA_STYLE_PIPE_DELIMITED;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, validate_parameter_style(&p, 0));

  /* spaceDelimited / pipeDelimited success: object-like */
  p.type = cdd_test_mut_str("MyObj");
  ASSERT_EQ(CDD_C_SUCCESS, validate_parameter_style(&p, 0));
  p.style = OA_STYLE_SPACE_DELIMITED;
  ASSERT_EQ(CDD_C_SUCCESS, validate_parameter_style(&p, 0));

  PASS();
}

/**
 * @brief Tests component_key_is_valid, media_type_key_is_valid, and maps.
 * @return GREATEST_TEST_RES
 */
TEST test_openapi_utils_keys_and_maps(void) {
  JSON_Value *jv = NULL;
  JSON_Object *jo = NULL;
  struct OpenAPI_Parameter p;
  int bval = 0;

  /* 1. component_key_is_valid */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, component_key_is_valid("ok", NULL));
  ASSERT_EQ(CDD_C_SUCCESS, component_key_is_valid(NULL, &bval));
  ASSERT_EQ(0, bval);
  ASSERT_EQ(CDD_C_SUCCESS, component_key_is_valid("", &bval));
  ASSERT_EQ(0, bval);
  ASSERT_EQ(CDD_C_SUCCESS, component_key_is_valid("Valid_Name.1-ok", &bval));
  ASSERT_EQ(1, bval);
  ASSERT_EQ(CDD_C_SUCCESS, component_key_is_valid("Invalid@Key", &bval));
  ASSERT_EQ(0, bval);

  /* 2. media_type_key_is_valid */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, media_type_key_is_valid("ok", NULL));
  ASSERT_EQ(CDD_C_SUCCESS, media_type_key_is_valid(NULL, &bval));
  ASSERT_EQ(0, bval);
  ASSERT_EQ(CDD_C_SUCCESS, media_type_key_is_valid("", &bval));
  ASSERT_EQ(0, bval);
  ASSERT_EQ(CDD_C_SUCCESS,
            media_type_key_is_valid("application/json+vnd.*_1", &bval));
  ASSERT_EQ(1, bval);
  ASSERT_EQ(CDD_C_SUCCESS, media_type_key_is_valid("invalid#media", &bval));
  ASSERT_EQ(0, bval);

  /* 3. validate_component_key_map */
  ASSERT_EQ(CDD_C_SUCCESS, validate_component_key_map(NULL));
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err =
        cdd_json_parse_string("{\"Key1\": 1, \"Key_2.ok\": 2}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, validate_component_key_map(jo));
  g_cdd_last_alloc_err = json_value_free(jv);

  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err =
        cdd_json_parse_string("{\"Key1\": 1, \"Bad@Key\": 2}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, validate_component_key_map(jo));
  g_cdd_last_alloc_err = json_value_free(jv);

  /* 4. validate_media_type_key_map */
  ASSERT_EQ(CDD_C_SUCCESS, validate_media_type_key_map(NULL));
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string(
        "{\"application/json\": 1, \"text/*\": 2}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, validate_media_type_key_map(jo));
  g_cdd_last_alloc_err = json_value_free(jv);

  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err =
        cdd_json_parse_string("{\"bad!media\": 1}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, validate_media_type_key_map(jo));
  g_cdd_last_alloc_err = json_value_free(jv);

  /* 5. header_name_is_content_type */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            header_name_is_content_type("Content-Type", NULL));
  ASSERT_EQ(CDD_C_SUCCESS, header_name_is_content_type(NULL, &bval));
  ASSERT_EQ(0, bval);
  ASSERT_EQ(CDD_C_SUCCESS, header_name_is_content_type("content-type", &bval));
  ASSERT_EQ(1, bval);
  ASSERT_EQ(CDD_C_SUCCESS, header_name_is_content_type("Content-Type", &bval));
  ASSERT_EQ(1, bval);
  ASSERT_EQ(CDD_C_SUCCESS, header_name_is_content_type("Accept", &bval));
  ASSERT_EQ(0, bval);

  /* 6. header_param_is_reserved */
  memset(&p, 0, sizeof(p));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, header_param_is_reserved(&p, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, header_param_is_reserved(NULL, &bval));
  ASSERT_EQ(0, bval);
  ASSERT_EQ(CDD_C_SUCCESS, header_param_is_reserved(&p, &bval));
  ASSERT_EQ(0, bval);
  p.in = OA_PARAM_IN_QUERY;
  p.name = cdd_test_mut_str("Accept");
  ASSERT_EQ(CDD_C_SUCCESS, header_param_is_reserved(&p, &bval));
  ASSERT_EQ(0, bval);

  p.in = OA_PARAM_IN_HEADER;
  p.name = cdd_test_mut_str("Accept");
  ASSERT_EQ(CDD_C_SUCCESS, header_param_is_reserved(&p, &bval));
  ASSERT_EQ(1, bval);
  p.name = cdd_test_mut_str("Content-Type");
  ASSERT_EQ(CDD_C_SUCCESS, header_param_is_reserved(&p, &bval));
  ASSERT_EQ(1, bval);
  p.name = cdd_test_mut_str("Authorization");
  ASSERT_EQ(CDD_C_SUCCESS, header_param_is_reserved(&p, &bval));
  ASSERT_EQ(1, bval);
  p.name = cdd_test_mut_str("X-Custom");
  ASSERT_EQ(CDD_C_SUCCESS, header_param_is_reserved(&p, &bval));
  ASSERT_EQ(0, bval);

  p.name = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, header_param_is_reserved(&p, &bval));
  ASSERT_EQ(0, bval);

  {
    extern C_CDD_EXPORT int g_cdd_fail_str_iequal;
    g_cdd_fail_str_iequal = 1;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              header_name_is_content_type("Content-Type", &bval));
    g_cdd_fail_str_iequal = 0;

    g_cdd_fail_str_iequal = 1;
    p.name = cdd_test_mut_str("Accept");
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              header_param_is_reserved(&p, &bval));
    g_cdd_fail_str_iequal = 0;
  }

  PASS();
}

/**
 * @brief Tests parse_security_type, parse_security_in, parse_oauth_flow_type,
 * and xml.
 * @return GREATEST_TEST_RES
 */
TEST test_openapi_utils_security_and_xml(void) {
  enum OpenAPI_SecurityType sec_type;
  enum OpenAPI_SecurityIn sec_in;
  enum OpenAPI_OAuthFlowType flow_type;
  enum OpenAPI_XmlNodeType xml_type;
  cdd_c_error_t rc = 0;

  /* 1. parse_security_type */
  rc = parse_security_type(NULL, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = parse_security_type(NULL, &sec_type);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_SEC_UNKNOWN, sec_type);

  rc = parse_security_type("apiKey", &sec_type);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_SEC_APIKEY, sec_type);
  rc = parse_security_type("http", &sec_type);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_SEC_HTTP, sec_type);
  rc = parse_security_type("mutualTLS", &sec_type);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_SEC_MUTUALTLS, sec_type);
  rc = parse_security_type("oauth2", &sec_type);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_SEC_OAUTH2, sec_type);
  rc = parse_security_type("openIdConnect", &sec_type);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_SEC_OPENID, sec_type);
  rc = parse_security_type("other", &sec_type);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_SEC_UNKNOWN, sec_type);

  /* 2. parse_security_in */
  rc = parse_security_in(NULL, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = parse_security_in(NULL, &sec_in);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_SEC_IN_UNKNOWN, sec_in);

  rc = parse_security_in("query", &sec_in);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_SEC_IN_QUERY, sec_in);
  rc = parse_security_in("header", &sec_in);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_SEC_IN_HEADER, sec_in);
  rc = parse_security_in("cookie", &sec_in);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_SEC_IN_COOKIE, sec_in);
  rc = parse_security_in("other", &sec_in);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_SEC_IN_UNKNOWN, sec_in);

  /* 3. parse_oauth_flow_type */
  rc = parse_oauth_flow_type(NULL, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = parse_oauth_flow_type(NULL, &flow_type);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_OAUTH_FLOW_UNKNOWN, flow_type);

  rc = parse_oauth_flow_type("implicit", &flow_type);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_OAUTH_FLOW_IMPLICIT, flow_type);
  rc = parse_oauth_flow_type("password", &flow_type);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_OAUTH_FLOW_PASSWORD, flow_type);
  rc = parse_oauth_flow_type("clientCredentials", &flow_type);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_OAUTH_FLOW_CLIENT_CREDENTIALS, flow_type);
  rc = parse_oauth_flow_type("authorizationCode", &flow_type);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_OAUTH_FLOW_AUTHORIZATION_CODE, flow_type);
  rc = parse_oauth_flow_type("deviceAuthorization", &flow_type);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_OAUTH_FLOW_DEVICE_AUTHORIZATION, flow_type);
  rc = parse_oauth_flow_type("other", &flow_type);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_OAUTH_FLOW_UNKNOWN, flow_type);

  /* 4. parse_xml_node_type */
  rc = parse_xml_node_type(NULL, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = parse_xml_node_type(NULL, &xml_type);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_XML_NODE_UNSET, xml_type);

  rc = parse_xml_node_type("element", &xml_type);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_XML_NODE_ELEMENT, xml_type);
  rc = parse_xml_node_type("attribute", &xml_type);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_XML_NODE_ATTRIBUTE, xml_type);
  rc = parse_xml_node_type("text", &xml_type);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_XML_NODE_TEXT, xml_type);
  rc = parse_xml_node_type("cdata", &xml_type);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_XML_NODE_CDATA, xml_type);
  rc = parse_xml_node_type("none", &xml_type);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_XML_NODE_NONE, xml_type);
  rc = parse_xml_node_type("other", &xml_type);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_XML_NODE_UNSET, xml_type);

  PASS();
}

/**
 * @brief Tests parse_any_value, parse_any_field, and parse_any_array.
 * @return GREATEST_TEST_RES
 */
TEST test_openapi_utils_any_values(void) {
  struct OpenAPI_Any any;
  struct OpenAPI_Any *any_arr = NULL;
  size_t any_count = 0;
  JSON_Value *jv = NULL;
  JSON_Object *jo = NULL;
  JSON_Array *ja = NULL;
  int out_set = 0;
  cdd_c_error_t rc = 0;

  /* 1. parse_any_value NULL checks */
  ASSERT_EQ(CDD_C_SUCCESS, parse_any_value(NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, parse_any_value(NULL, &any));

  /* 2. parse_any_value types */
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string("\"test_str\"", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  memset(&any, 0, sizeof(any));
  rc = parse_any_value(jv, &any);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_ANY_STRING, any.type);
  ASSERT_STR_EQ("test_str", any.string);
  g_cdd_last_alloc_err = free_any_value(&any);
  g_cdd_last_alloc_err = json_value_free(jv);

  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string("123.45", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  memset(&any, 0, sizeof(any));
  rc = parse_any_value(jv, &any);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_ANY_NUMBER, any.type);
  g_cdd_last_alloc_err = free_any_value(&any);
  g_cdd_last_alloc_err = json_value_free(jv);

  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string("true", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  memset(&any, 0, sizeof(any));
  rc = parse_any_value(jv, &any);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_ANY_BOOL, any.type);
  ASSERT_EQ(1, any.boolean);
  g_cdd_last_alloc_err = free_any_value(&any);
  g_cdd_last_alloc_err = json_value_free(jv);

  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string("null", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  memset(&any, 0, sizeof(any));
  rc = parse_any_value(jv, &any);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_ANY_NULL, any.type);
  g_cdd_last_alloc_err = free_any_value(&any);
  g_cdd_last_alloc_err = json_value_free(jv);

  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string("{\"nested\": 1}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  memset(&any, 0, sizeof(any));
  rc = parse_any_value(jv, &any);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_ANY_JSON, any.type);
  ASSERT(any.json != NULL);
  g_cdd_last_alloc_err = free_any_value(&any);

  {
    JSON_Value *jv_arr = NULL;
    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err = cdd_json_parse_string("[1, 2]", &_cdd_jv_1);
      jv_arr = _cdd_jv_1;
    }
    memset(&any, 0, sizeof(any));
    rc = parse_any_value(jv_arr, &any);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_EQ(OA_ANY_JSON, any.type);
    ASSERT(any.json != NULL);
    g_cdd_last_alloc_err = free_any_value(&any);
    g_cdd_last_alloc_err = json_value_free(jv_arr);
  }

  /* OOM on any serialize */
  {
    extern C_CDD_EXPORT int g_cdd_fail_any_serialize;
    g_cdd_fail_any_serialize = 1;
    memset(&any, 0, sizeof(any));
    rc = parse_any_value(jv, &any);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    g_cdd_fail_any_serialize = 0;

    g_cdd_fail_any_serialize = 2;
    memset(&any, 0, sizeof(any));
    rc = parse_any_value(jv, &any);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    g_cdd_last_alloc_err = free_any_value(&any);
    g_cdd_fail_any_serialize = 0;
  }

  rc = parse_any_value(jv, NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* OOM on any strdup */
  g_cdd_strdup_fail = 1;
  memset(&any, 0, sizeof(any));
  rc = parse_any_value(jv, &any);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  g_cdd_strdup_fail = 0;
  g_cdd_last_alloc_err = json_value_free(jv);

  /* 3. parse_any_field */
  ASSERT_EQ(CDD_C_SUCCESS, parse_any_field(NULL, NULL, NULL, NULL));
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err =
        cdd_json_parse_string("{\"key1\": \"val1\"}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);

  ASSERT_EQ(CDD_C_SUCCESS, parse_any_field(jo, NULL, &any, &out_set));
  ASSERT_EQ(CDD_C_SUCCESS, parse_any_field(jo, "key1", NULL, &out_set));
  ASSERT_EQ(CDD_C_SUCCESS, parse_any_field(jo, "key1", &any, NULL));

  ASSERT_EQ(CDD_C_SUCCESS, parse_any_field(jo, "not_found", &any, &out_set));
  ASSERT_EQ(0, out_set);

  rc = parse_any_field(jo, "key1", &any, &out_set);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, out_set);
  ASSERT_STR_EQ("val1", any.string);
  g_cdd_last_alloc_err = free_any_value(&any);

  g_cdd_strdup_fail = 1;
  rc = parse_any_field(jo, "key1", &any, &out_set);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  g_cdd_strdup_fail = 0;
  g_cdd_last_alloc_err = json_value_free(jv);

  /* 4. parse_any_array */
  ASSERT_EQ(CDD_C_SUCCESS, parse_any_array(NULL, NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, parse_any_array(NULL, &any_arr, &any_count));
  ASSERT(any_arr == NULL);
  ASSERT_EQ(0, any_count);

  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string("[]", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  ja = json_value_get_array(jv);
  ASSERT_EQ(CDD_C_SUCCESS, parse_any_array(ja, NULL, &any_count));
  ASSERT_EQ(CDD_C_SUCCESS, parse_any_array(ja, &any_arr, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, parse_any_array(ja, &any_arr, &any_count));
  ASSERT(any_arr == NULL);
  ASSERT_EQ(0, any_count);
  g_cdd_last_alloc_err = json_value_free(jv);

  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err =
        cdd_json_parse_string("[\"item1\", 2, true]", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  ja = json_value_get_array(jv);
  rc = parse_any_array(ja, &any_arr, &any_count);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(3, any_count);
  ASSERT_STR_EQ("item1", any_arr[0].string);
  ASSERT_EQ(OA_ANY_NUMBER, any_arr[1].type);
  ASSERT_EQ(OA_ANY_BOOL, any_arr[2].type);
  {
    size_t k;
    for (k = 0; k < any_count; ++k)
      g_cdd_last_alloc_err = free_any_value(&any_arr[k]);
    free(any_arr);
    any_arr = NULL;
    any_count = 0;
  }

  /* OOM on calloc */
  g_cdd_alloc_fail = 1;
  rc = parse_any_array(ja, &any_arr, &any_count);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  g_cdd_alloc_fail = 0;

  /* OOM during parse_any_value in array */
  g_cdd_strdup_fail = 1;
  rc = parse_any_array(ja, &any_arr, &any_count);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  g_cdd_strdup_fail = 0;
  g_cdd_last_alloc_err = json_value_free(jv);

  {
    JSON_Value *jv2 = NULL;
    JSON_Array *ja2 = NULL;
    {
      JSON_Value *_cdd_jv_1 = NULL;
      g_cdd_last_alloc_err =
          cdd_json_parse_string("[\"item1\", \"item2\"]", &_cdd_jv_1);
      jv2 = _cdd_jv_1;
    }
    ja2 = json_value_get_array(jv2);
    g_cdd_strdup_fail = 2;
    rc = parse_any_array(ja2, &any_arr, &any_count);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    g_cdd_strdup_fail = 0;
    g_cdd_last_alloc_err = json_value_free(jv2);
  }

  PASS();
}

/**
 * @brief Tests clone_json_value, collect_schema_extras, and
 * collect_extensions.
 * @return GREATEST_TEST_RES
 */
TEST test_openapi_utils_clone_and_extras(void) {
  JSON_Value *jv = NULL;
  JSON_Value *clone = NULL;
  JSON_Object *jo = NULL;
  char *out_json = NULL;
  const char *skip[2];
  cdd_c_error_t rc = 0;

  /* 1. clone_json_value */
  rc = clone_json_value(NULL, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = clone_json_value(NULL, &clone);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(clone == NULL);

  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string(
        "{\"openapi\":\"3.2.0\",\"servers\":[{\"url\":\"https://"
        "api.example.com\"}],"
        "\"webhooks\":{\"newPet\":{\"post\":{\"operationId\":\"onNewPet\"}}},"
        "\"paths\":{\"/pets\":{\"get\":{\"operationId\":\"listPets\"}}},"
        "\"components\":{\"schemas\":{\"Pet\":{\"type\":\"object\"}}}}",
        &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  rc = clone_json_value(jv, &clone);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(clone != NULL);
  {
    JSON_Array *_cdd_ja_1 = NULL;
    const char *_cdd_js_2 = NULL;
    g_cdd_last_alloc_err = cdd_json_object_get_array(
        json_value_get_object(clone), "servers", &_cdd_ja_1);
    g_cdd_last_alloc_err = cdd_json_object_get_string(
        json_array_get_object(_cdd_ja_1, 0), "url", &_cdd_js_2);
    ASSERT_STR_EQ("https://api.example.com", _cdd_js_2);
  }
  {
    JSON_Value *_cdd_jv_1 = NULL;
    const char *_cdd_js_1 = NULL;
    if (cdd_json_object_dotget_value(json_value_get_object(clone),
                                     "webhooks.newPet.post.operationId",
                                     &_cdd_jv_1) == CDD_C_SUCCESS &&
        _cdd_jv_1 && _cdd_jv_1->type == JSONString)
      _cdd_js_1 = _cdd_jv_1->string_val;
    ASSERT_STR_EQ("onNewPet", _cdd_js_1);
  }
  {
    JSON_Value *_cdd_jv_1 = NULL;
    const char *_cdd_js_1 = NULL;
    if (cdd_json_object_dotget_value(json_value_get_object(clone),
                                     "paths./pets.get.operationId",
                                     &_cdd_jv_1) == CDD_C_SUCCESS &&
        _cdd_jv_1 && _cdd_jv_1->type == JSONString)
      _cdd_js_1 = _cdd_jv_1->string_val;
    ASSERT_STR_EQ("listPets", _cdd_js_1);
  }
  {
    JSON_Value *_cdd_jv_1 = NULL;
    const char *_cdd_js_1 = NULL;
    if (cdd_json_object_dotget_value(json_value_get_object(clone),
                                     "components.schemas.Pet.type",
                                     &_cdd_jv_1) == CDD_C_SUCCESS &&
        _cdd_jv_1 && _cdd_jv_1->type == JSONString)
      _cdd_js_1 = _cdd_jv_1->string_val;
    ASSERT_STR_EQ("object", _cdd_js_1);
  }
  g_cdd_last_alloc_err = json_value_free(clone);
  clone = NULL;

  /* OOM on clone serialize */
  {
    extern C_CDD_EXPORT int g_cdd_fail_clone_json_serialize;
    g_cdd_fail_clone_json_serialize = 1;
    rc = clone_json_value(jv, &clone);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    ASSERT(clone == NULL);
    g_cdd_fail_clone_json_serialize = 0;

    g_cdd_fail_clone_json_serialize = 2;
    rc = clone_json_value(jv, &clone);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT(clone != NULL);
    g_cdd_last_alloc_err = json_value_free(clone);
    clone = NULL;
    g_cdd_fail_clone_json_serialize = 0;
  }

  /* OOM on clone parse */
  {
    extern C_CDD_EXPORT int g_cdd_fail_clone_json_parse;
    g_cdd_fail_clone_json_parse = 1;
    rc = clone_json_value(jv, &clone);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    ASSERT(clone == NULL);
    g_cdd_fail_clone_json_parse = 0;

    g_cdd_fail_clone_json_parse = 2;
    rc = clone_json_value(jv, &clone);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT(clone != NULL);
    g_cdd_last_alloc_err = json_value_free(clone);
    clone = NULL;
    g_cdd_fail_clone_json_parse = 0;
  }
  g_cdd_last_alloc_err = json_value_free(jv);

  /* 2. key_in_list */
  {
    int bval = 0;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              key_in_list("skip1", skip, 2, NULL));
    ASSERT_EQ(CDD_C_SUCCESS, key_in_list(NULL, NULL, 0, &bval));
    ASSERT_EQ(0, bval);
    skip[0] = "skip1";
    skip[1] = "skip2";
    ASSERT_EQ(CDD_C_SUCCESS, key_in_list("skip1", skip, 2, &bval));
    ASSERT_EQ(1, bval);
    ASSERT_EQ(CDD_C_SUCCESS, key_in_list("skip2", skip, 2, &bval));
    ASSERT_EQ(1, bval);
    ASSERT_EQ(CDD_C_SUCCESS, key_in_list("keep", skip, 2, &bval));
    ASSERT_EQ(0, bval);
    skip[0] = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, key_in_list("skip1", skip, 2, &bval));
    ASSERT_EQ(0, bval);
    skip[0] = "skip1";

    /* 3. is_extension_key */
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, is_extension_key("x-my-ext", NULL));
    ASSERT_EQ(CDD_C_SUCCESS, is_extension_key(NULL, &bval));
    ASSERT_EQ(0, bval);
    ASSERT_EQ(CDD_C_SUCCESS, is_extension_key("", &bval));
    ASSERT_EQ(0, bval);
    ASSERT_EQ(CDD_C_SUCCESS, is_extension_key("x", &bval));
    ASSERT_EQ(0, bval);
    ASSERT_EQ(CDD_C_SUCCESS, is_extension_key("regularKey", &bval));
    ASSERT_EQ(0, bval);
    ASSERT_EQ(CDD_C_SUCCESS, is_extension_key("x-my-ext", &bval));
    ASSERT_EQ(1, bval);
  }

  /* 4. collect_schema_extras */
  rc = collect_schema_extras(NULL, NULL, 0, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string("{}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  rc = collect_schema_extras(NULL, skip, 2, &out_json);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = collect_schema_extras(jo, skip, 2, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = collect_schema_extras(jo, skip, 2, &out_json);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(out_json == NULL);
  g_cdd_last_alloc_err = json_value_free(jv);

  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string(
        "{\"skip1\": 1, \"extra1\": \"val1\", \"extra2\": 2}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  rc = collect_schema_extras(jo, skip, 2, &out_json);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(out_json != NULL);
  free(out_json);
  out_json = NULL;

  /* OOM on extras init */
  {
    extern C_CDD_EXPORT int g_cdd_fail_extras_init;
    g_cdd_fail_extras_init = 1;
    rc = collect_schema_extras(jo, skip, 2, &out_json);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    g_cdd_fail_extras_init = 0;

    g_cdd_fail_extras_init = 2;
    rc = collect_schema_extras(jo, skip, 2, &out_json);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    free(out_json);
    out_json = NULL;
    g_cdd_fail_extras_init = 0;
  }

  /* OOM on extras clone */
  {
    extern C_CDD_EXPORT int g_cdd_fail_clone_json_serialize;
    g_cdd_fail_clone_json_serialize = 1;
    rc = collect_schema_extras(jo, skip, 2, &out_json);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    g_cdd_fail_clone_json_serialize = 0;
  }

  /* OOM on extras set_value */
  {
    extern C_CDD_EXPORT volatile int g_cdd_fail_json_set_value;
    g_cdd_fail_json_set_value = 1;
    rc = collect_schema_extras(jo, skip, 2, &out_json);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    g_cdd_fail_json_set_value = 0;

    g_cdd_fail_json_set_value = 3;
    rc = collect_schema_extras(jo, skip, 2, &out_json);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    free(out_json);
    out_json = NULL;
    g_cdd_fail_json_set_value = 0;
  }

  /* OOM on extras serialize */
  {
    extern C_CDD_EXPORT int g_cdd_fail_extras_serialize;
    g_cdd_fail_extras_serialize = 1;
    rc = collect_schema_extras(jo, skip, 2, &out_json);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    g_cdd_fail_extras_serialize = 0;

    g_cdd_fail_extras_serialize = 2;
    rc = collect_schema_extras(jo, skip, 2, &out_json);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    free(out_json);
    out_json = NULL;
    g_cdd_fail_extras_serialize = 0;
  }

  /* OOM on extras strdup */
  g_cdd_strdup_fail = 1;
  rc = collect_schema_extras(jo, skip, 2, &out_json);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  g_cdd_strdup_fail = 0;
  g_cdd_last_alloc_err = json_value_free(jv);

  /* 5. collect_extensions */
  rc = collect_extensions(NULL, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = collect_extensions(NULL, &out_json);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err =
        cdd_json_parse_string("{\"non_ext\": 1}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  rc = collect_extensions(jo, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = collect_extensions(jo, &out_json);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(out_json == NULL);
  g_cdd_last_alloc_err = json_value_free(jv);

  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_parse_string(
        "{\"x-tag\": \"val\", \"regular\": 2}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  rc = collect_extensions(jo, &out_json);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(out_json != NULL);
  free(out_json);
  out_json = NULL;

  /* OOM on extensions init */
  {
    extern C_CDD_EXPORT int g_cdd_fail_extensions_init;
    g_cdd_fail_extensions_init = 1;
    rc = collect_extensions(jo, &out_json);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    g_cdd_fail_extensions_init = 0;

    g_cdd_fail_extensions_init = 2;
    rc = collect_extensions(jo, &out_json);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    free(out_json);
    out_json = NULL;
    g_cdd_fail_extensions_init = 0;
  }

  /* OOM on extensions clone */
  {
    extern C_CDD_EXPORT int g_cdd_fail_clone_json_serialize;
    g_cdd_fail_clone_json_serialize = 1;
    rc = collect_extensions(jo, &out_json);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    g_cdd_fail_clone_json_serialize = 0;
  }

  /* OOM on extensions set_value */
  {
    extern C_CDD_EXPORT volatile int g_cdd_fail_json_set_value;
    g_cdd_fail_json_set_value = 1;
    rc = collect_extensions(jo, &out_json);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    g_cdd_fail_json_set_value = 0;

    g_cdd_fail_json_set_value = 2;
    rc = collect_extensions(jo, &out_json);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    free(out_json);
    out_json = NULL;
    g_cdd_fail_json_set_value = 0;
  }

  /* OOM on extensions serialize */
  {
    extern C_CDD_EXPORT int g_cdd_fail_extensions_serialize;
    g_cdd_fail_extensions_serialize = 1;
    rc = collect_extensions(jo, &out_json);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    g_cdd_fail_extensions_serialize = 0;

    g_cdd_fail_extensions_serialize = 2;
    rc = collect_extensions(jo, &out_json);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    free(out_json);
    out_json = NULL;
    g_cdd_fail_extensions_serialize = 0;
  }

  /* OOM on extensions strdup */
  g_cdd_strdup_fail = 1;
  rc = collect_extensions(jo, &out_json);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  g_cdd_strdup_fail = 0;
  g_cdd_last_alloc_err = json_value_free(jv);

  PASS();
}

/**
 * @brief Tests url_has_query_or_fragment, openapi_version_supported, and
 * example validation.
 * @return GREATEST_TEST_RES
 */
TEST test_openapi_utils_url_version_examples(void) {
  struct OpenAPI_Example ex;
  JSON_Value *jv = NULL;
  JSON_Object *jo = NULL;
  int bval = 0;

  /* 1. url_has_query_or_fragment */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            url_has_query_or_fragment("https://example.com/api", NULL));
  ASSERT_EQ(CDD_C_SUCCESS, url_has_query_or_fragment(NULL, &bval));
  ASSERT_EQ(0, bval);
  ASSERT_EQ(CDD_C_SUCCESS,
            url_has_query_or_fragment("https://example.com/api", &bval));
  ASSERT_EQ(0, bval);
  ASSERT_EQ(CDD_C_SUCCESS,
            url_has_query_or_fragment("https://example.com/api?a=1", &bval));
  ASSERT_EQ(1, bval);
  ASSERT_EQ(CDD_C_SUCCESS, url_has_query_or_fragment(
                               "https://example.com/api#section", &bval));
  ASSERT_EQ(1, bval);

  /* 2. openapi_version_supported */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            openapi_version_supported("3.0.0", NULL));
  ASSERT_EQ(CDD_C_SUCCESS, openapi_version_supported(NULL, &bval));
  ASSERT_EQ(0, bval);
  ASSERT_EQ(CDD_C_SUCCESS, openapi_version_supported("", &bval));
  ASSERT_EQ(0, bval);
  ASSERT_EQ(CDD_C_SUCCESS, openapi_version_supported("3.0.0", &bval));
  ASSERT_EQ(1, bval);
  ASSERT_EQ(CDD_C_SUCCESS, openapi_version_supported("3.1.0", &bval));
  ASSERT_EQ(1, bval);
  ASSERT_EQ(CDD_C_SUCCESS, openapi_version_supported("3.2.0", &bval));
  ASSERT_EQ(1, bval);
  ASSERT_EQ(CDD_C_SUCCESS, openapi_version_supported("2.0", &bval));
  ASSERT_EQ(0, bval);
  ASSERT_EQ(CDD_C_SUCCESS, openapi_version_supported("4.0.0", &bval));
  ASSERT_EQ(0, bval);
  ASSERT_EQ(CDD_C_SUCCESS, openapi_version_supported("3x", &bval));
  ASSERT_EQ(0, bval);
  ASSERT_EQ(CDD_C_SUCCESS, openapi_version_supported("3.9", &bval));
  ASSERT_EQ(0, bval);

  /* 3. example_fields_valid */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, example_fields_valid(NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, example_fields_valid(NULL, &bval));
  ASSERT_EQ(1, bval);
  memset(&ex, 0, sizeof(ex));
  ASSERT_EQ(CDD_C_SUCCESS, example_fields_valid(&ex, &bval));
  ASSERT_EQ(1, bval);

  /* Conflicting data_value and value */
  ex.data_value_set = 1;
  ex.value_set = 1;
  ASSERT_EQ(CDD_C_SUCCESS, example_fields_valid(&ex, &bval));
  ASSERT_EQ(0, bval);

  /* Conflicting serialized and external */
  memset(&ex, 0, sizeof(ex));
  ex.serialized_value = cdd_test_mut_str("ser");
  ex.external_value = cdd_test_mut_str("ext");
  ASSERT_EQ(CDD_C_SUCCESS, example_fields_valid(&ex, &bval));
  ASSERT_EQ(0, bval);

  /* Conflicting value and serialized */
  memset(&ex, 0, sizeof(ex));
  ex.value_set = 1;
  ex.serialized_value = cdd_test_mut_str("ser");
  ASSERT_EQ(CDD_C_SUCCESS, example_fields_valid(&ex, &bval));
  ASSERT_EQ(0, bval);

  /* Conflicting value and external */
  memset(&ex, 0, sizeof(ex));
  ex.value_set = 1;
  ex.external_value = cdd_test_mut_str("ext");
  ASSERT_EQ(CDD_C_SUCCESS, example_fields_valid(&ex, &bval));
  ASSERT_EQ(0, bval);

  /* 4. object_has_example_and_examples */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            object_has_example_and_examples(NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, object_has_example_and_examples(NULL, &bval));
  ASSERT_EQ(0, bval);
  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err =
        cdd_json_parse_string("{\"example\": 1}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, object_has_example_and_examples(jo, &bval));
  ASSERT_EQ(0, bval);
  g_cdd_last_alloc_err = json_value_free(jv);

  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err =
        cdd_json_parse_string("{\"examples\": {}}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, object_has_example_and_examples(jo, &bval));
  ASSERT_EQ(0, bval);
  g_cdd_last_alloc_err = json_value_free(jv);

  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err =
        cdd_json_parse_string("{\"example\": 1, \"examples\": {}}", &_cdd_jv_1);
    jv = _cdd_jv_1;
  }
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, object_has_example_and_examples(jo, &bval));
  ASSERT_EQ(1, bval);
  g_cdd_last_alloc_err = json_value_free(jv);

  PASS();
}

/**
 * @brief Failing JSON allocator for testing cdd_json_serialize_to_string /
 * cdd_json_parse_string OOM paths.
 */
static int g_oa_json_fail_at = 0;

/**
 * @brief Mock malloc for Parson JSON DOM allocations.
 *
 * @param[in] sz Allocation size.
 * @return Allocated pointer or NULL on fault injection.
 */
static void *oa_mock_json_malloc(size_t sz) {
  if (g_oa_json_fail_at > 0 && --g_oa_json_fail_at == 0)
    return NULL;
  return malloc(sz);
}

/**
 * @brief Step-fail and OOM coverage for openapi_uri.c, openapi_utils.c,
 * openapi_examples.c, and openapi_media.c.
 * @return GREATEST_TEST_RES
 */
TEST test_openapi_percolation_batch1(void) {
  int sf;
  int bval = 0;
  size_t cnt = 0;
  const char *sname = NULL;
  JSON_Value *val_out = NULL;
  enum OpenAPI_OAuthFlowType ft = OA_OAUTH_FLOW_UNKNOWN;
  enum OpenAPI_XmlNodeType xt = OA_XML_NODE_UNSET;

  /* 1. openapi_uri.c branches */
  {
    char *doc_u = NULL;
    JSON_Value *self_jv = NULL;
    struct OpenAPI_Spec sp;

    g_cdd_strdup_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              compute_document_uri("https://a.com/x", NULL, &doc_u));
    g_cdd_strdup_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              compute_document_uri(NULL, "https://a.com/x", &doc_u));
    g_cdd_strdup_fail = 0;
    g_cdd_alloc_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              compute_document_uri("sub/a.json", "https://a.com/x", &doc_u));
    g_cdd_alloc_fail = 0;

    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_parse_string("{\"$self\":\"https://a.com\"}", &self_jv));
    ASSERT_EQ(CDD_C_SUCCESS,
              root_has_openapi_fields(json_value_get_object(self_jv), &bval));
    ASSERT_EQ(1, bval);

    {
      JSON_Value *dial_jv = NULL;
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_parse_string(
                                   "{\"jsonSchemaDialect\":\"x\"}", &dial_jv));
      ASSERT_EQ(CDD_C_SUCCESS,
                root_has_openapi_fields(json_value_get_object(dial_jv), &bval));
      ASSERT_EQ(1, bval);
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(dial_jv));
    }

    memset(&sp, 0, sizeof(sp));
    json_set_allocation_functions(oa_mock_json_malloc, free);
    g_oa_json_fail_at = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, store_schema_root_json(&sp, self_jv));
    g_oa_json_fail_at = 0;
    json_set_allocation_functions(malloc, free);

    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(self_jv));
  }

  /* 2. oa_json_* helpers including NULL and step-fail */
  {
    JSON_Value *jv = NULL;
    JSON_Object *jo = NULL;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_json_parse_string("{\"a\":\"x\",\"b\":1}", &jv));
    jo = json_value_get_object(jv);

    ASSERT_EQ(CDD_C_SUCCESS, oa_json_object_name(NULL, 0, &sname));
    ASSERT_EQ(NULL, sname);
    ASSERT_EQ(CDD_C_SUCCESS, oa_json_object_value_at(NULL, 0, &val_out));
    ASSERT_EQ(NULL, val_out);
    ASSERT_EQ(CDD_C_SUCCESS, oa_json_object_value_at(jo, 0, &val_out));
    ASSERT(val_out != NULL);
    g_cdd_step_fail = 2;
    ASSERT_EQ(CDD_C_SUCCESS, oa_json_object_value_at(jo, 0, &val_out));
    g_cdd_step_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, oa_json_object_value_at(jo, 0, &val_out));
    g_cdd_step_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, oa_json_object_count(jo, &cnt));
    g_cdd_step_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, oa_json_object_name(jo, 0, &sname));
    g_cdd_step_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, oa_json_has_value(jo, "a", &bval));
    g_cdd_step_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, oa_json_opt_value(jo, "a", &val_out));
    g_cdd_step_fail = 0;

    /* Leaf step-fail probes in openapi_utils.c */
    {
      struct OpenAPI_Parameter p;
      struct OpenAPI_Example ex;
      memset(&p, 0, sizeof(p));
      memset(&ex, 0, sizeof(ex));
      p.in = OA_PARAM_IN_QUERY;
      p.style = OA_STYLE_FORM;
      for (sf = 1; sf <= 2; ++sf) {
        g_cdd_step_fail = sf;
        g_cdd_last_alloc_err = validate_parameter_style(&p, 0);
        g_cdd_step_fail = sf;
        g_cdd_last_alloc_err = header_name_is_content_type("X", &bval);
        g_cdd_step_fail = sf;
        g_cdd_last_alloc_err = parse_oauth_flow_type("implicit", &ft);
        g_cdd_step_fail = sf;
        g_cdd_last_alloc_err = parse_xml_node_type("element", &xt);
        g_cdd_step_fail = sf;
        g_cdd_last_alloc_err = url_has_query_or_fragment("a", &bval);
        g_cdd_step_fail = sf;
        g_cdd_last_alloc_err = openapi_version_supported("3.1.0", &bval);
        g_cdd_step_fail = sf;
        g_cdd_last_alloc_err = example_fields_valid(&ex, &bval);
        g_cdd_step_fail = sf;
        g_cdd_last_alloc_err = object_has_example_and_examples(jo, &bval);
      }
      g_cdd_step_fail = 0;
    }

    for (sf = 1; sf <= 15; ++sf) {
      g_cdd_step_fail = sf;
      g_cdd_last_alloc_err = validate_component_key_map(jo);
      g_cdd_step_fail = sf;
      g_cdd_last_alloc_err = validate_media_type_key_map(jo);
      g_cdd_step_fail = 0;
    }

    /* parse_any_value / parse_any_field / parse_any_array / clone / extras */
    {
      JSON_Value *mix_jv = NULL;
      JSON_Object *mix_jo = NULL;
      JSON_Array *mix_ja = NULL;
      const char *skip_k[1] = {"s"};
      ASSERT_EQ(
          CDD_C_SUCCESS,
          cdd_json_parse_string("{\"s\":\"hi\",\"n\":1,\"b\":true,\"z\":null,"
                                "\"o\":{\"k\":1},\"x-ext\":2,"
                                "\"arr\":[\"hi\",1,true,null,{\"k\":1}]}",
                                &mix_jv));
      mix_jo = json_value_get_object(mix_jv);
      ASSERT_EQ(CDD_C_SUCCESS, oa_json_opt_array(mix_jo, "arr", &mix_ja));

      for (sf = 1; sf <= 35; ++sf) {
        struct OpenAPI_Any *arr_out = NULL;
        size_t arr_n = 0;
        g_cdd_step_fail = sf;
        g_cdd_last_alloc_err = parse_any_array(mix_ja, &arr_out, &arr_n);
        g_cdd_step_fail = 0;
        if (arr_out) {
          size_t ai;
          for (ai = 0; ai < arr_n; ++ai)
            g_cdd_last_alloc_err = free_any_value(&arr_out[ai]);
          free(arr_out);
        }
      }

      for (sf = 1; sf <= 25; ++sf) {
        struct OpenAPI_Any a_out;
        int a_set = 0;
        char *ext_json = NULL;
        memset(&a_out, 0, sizeof(a_out));
        g_cdd_step_fail = sf;
        g_cdd_last_alloc_err = parse_any_field(mix_jo, "s", &a_out, &a_set);
        g_cdd_step_fail = 0;
        g_cdd_last_alloc_err = free_any_value(&a_out);

        g_cdd_step_fail = sf;
        g_cdd_last_alloc_err =
            collect_schema_extras(mix_jo, skip_k, 1, &ext_json);
        g_cdd_step_fail = 0;
        free(ext_json);
        ext_json = NULL;

        g_cdd_step_fail = sf;
        g_cdd_last_alloc_err = collect_extensions(mix_jo, &ext_json);
        g_cdd_step_fail = 0;
        free(ext_json);
      }

      /* OOM on cdd_json_serialize_to_string / cdd_json_parse_string */
      json_set_allocation_functions(oa_mock_json_malloc, free);
      for (sf = 1; sf <= 20; ++sf) {
        char *ext_json = NULL;
        g_oa_json_fail_at = sf;
        g_cdd_last_alloc_err = collect_schema_extras(jo, NULL, 0, &ext_json);
        g_oa_json_fail_at = 0;
        free(ext_json);
      }
      for (sf = 1; sf <= 25; ++sf) {
        struct OpenAPI_Any a_out;
        JSON_Value *cl = NULL;
        char *ext_json = NULL;
        memset(&a_out, 0, sizeof(a_out));
        g_oa_json_fail_at = sf;
        g_cdd_last_alloc_err = parse_any_value(mix_jv, &a_out);
        g_oa_json_fail_at = 0;
        g_cdd_last_alloc_err = free_any_value(&a_out);

        g_oa_json_fail_at = sf;
        g_cdd_last_alloc_err = clone_json_value(mix_jv, &cl);
        g_oa_json_fail_at = 0;
        if (cl)
          g_cdd_last_alloc_err = cdd_json_value_free(cl);

        g_oa_json_fail_at = sf;
        g_cdd_last_alloc_err =
            collect_schema_extras(mix_jo, skip_k, 1, &ext_json);
        g_oa_json_fail_at = 0;
        free(ext_json);
        ext_json = NULL;

        g_oa_json_fail_at = sf;
        g_cdd_last_alloc_err = collect_extensions(mix_jo, &ext_json);
        g_oa_json_fail_at = 0;
        free(ext_json);
      }
      json_set_allocation_functions(malloc, free);

      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(mix_jv));
    }

    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(jv));
  }

  /* 3. openapi_examples.c & openapi_media.c step-fail sweeps */
  {
    JSON_Value *ex_jv = NULL;
    JSON_Object *ex_jo = NULL;
    JSON_Value *mt_jv = NULL;
    JSON_Object *mt_jo = NULL;
    const char *ex_json_str =
        "{\"refEx\":{\"$ref\":\"#/components/examples/E1\"},"
        "\"inlEx\":{\"summary\":\"s\",\"value\":\"v\",\"x-e\":1},"
        "\"singleMedia\":{\"example\":\"v\"},"
        "\"mapMedia\":{\"examples\":{\"e1\":{\"summary\":\"s\"}}},"
        "\"flows\":{\"authorizationCode\":{\"authorizationUrl\":\"https://a\","
        "\"tokenUrl\":\"https://t\",\"scopes\":{\"r\":\"read\"},\"x-f\":1}}}";
    const char *mt_json_str =
        "{\"refMt\":{\"$ref\":\"#/components/mediaTypes/M1\"},"
        "\"boolMt\":{\"schema\":true,\"itemSchema\":false},"
        "\"fullMt\":{\"schema\":{\"type\":\"string\"},"
        "\"itemSchema\":{\"type\":\"integer\"},"
        "\"prefixEncoding\":[{\"contentType\":\"text/plain\"}],"
        "\"itemEncoding\":{\"contentType\":\"text/plain\"},"
        "\"examples\":{\"e1\":{\"value\":\"x\"}},\"x-m\":1},"
        "\"encMt\":{\"encoding\":{\"f\":{\"contentType\":\"text/plain\"}}},"
        "\"contentMap\":{\"application/json; charset=utf-8\":{\"schema\":true},"
        "\"text/plain\":{\"schema\":false}}}";
    struct OpenAPI_Spec sp;
    struct OpenAPI_Example comp_ex;
    char *comp_ex_name = cdd_test_mut_str("E1");
    struct OpenAPI_MediaType comp_mt;
    char *comp_mt_name = cdd_test_mut_str("M1");

    memset(&sp, 0, sizeof(sp));
    memset(&comp_ex, 0, sizeof(comp_ex));
    memset(&comp_mt, 0, sizeof(comp_mt));
    comp_ex.summary = cdd_test_mut_str("s");
    sp.component_examples = &comp_ex;
    sp.component_example_names = &comp_ex_name;
    sp.n_component_examples = 1;
    sp.component_media_types = &comp_mt;
    sp.component_media_type_names = &comp_mt_name;
    sp.n_component_media_types = 1;

    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_parse_string(ex_json_str, &ex_jv));
    ex_jo = json_value_get_object(ex_jv);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_parse_string(mt_json_str, &mt_jv));
    mt_jo = json_value_get_object(mt_jv);

    for (sf = 1; sf <= 90; ++sf) {
      JSON_Object *sub = NULL;
      struct OpenAPI_Example ex_out;
      struct OpenAPI_MediaType mt_out;
      struct OpenAPI_MediaType *mt_arr = NULL;
      size_t mt_n = 0;
      const JSON_Object *found_mt = NULL;
      struct OpenAPI_SecurityScheme sec_out;

      /* refEx + inlEx */
      ASSERT_EQ(CDD_C_SUCCESS, oa_json_opt_object(ex_jo, "refEx", &sub));
      memset(&ex_out, 0, sizeof(ex_out));
      g_cdd_step_fail = sf;
      g_cdd_last_alloc_err = parse_example_object(sub, "r", &ex_out, &sp, 1);
      g_cdd_step_fail = 0;
      g_cdd_last_alloc_err = free_example(&ex_out);

      ASSERT_EQ(CDD_C_SUCCESS, oa_json_opt_object(ex_jo, "inlEx", &sub));
      memset(&ex_out, 0, sizeof(ex_out));
      g_cdd_step_fail = sf;
      g_cdd_last_alloc_err = parse_example_object(sub, "i", &ex_out, &sp, 1);
      g_cdd_step_fail = 0;
      g_cdd_last_alloc_err = free_example(&ex_out);

      /* singleMedia + mapMedia */
      ASSERT_EQ(CDD_C_SUCCESS, oa_json_opt_object(ex_jo, "singleMedia", &sub));
      memset(&mt_out, 0, sizeof(mt_out));
      g_cdd_step_fail = sf;
      g_cdd_last_alloc_err =
          parse_media_examples(sub, &mt_out.example, &mt_out.example_set,
                               &mt_out.examples, &mt_out.n_examples, &sp, 1);
      g_cdd_step_fail = 0;
      g_cdd_last_alloc_err = free_media_type(&mt_out);

      ASSERT_EQ(CDD_C_SUCCESS, oa_json_opt_object(ex_jo, "mapMedia", &sub));
      memset(&mt_out, 0, sizeof(mt_out));
      g_cdd_step_fail = sf;
      g_cdd_last_alloc_err =
          parse_media_examples(sub, &mt_out.example, &mt_out.example_set,
                               &mt_out.examples, &mt_out.n_examples, &sp, 1);
      g_cdd_step_fail = 0;
      g_cdd_last_alloc_err = free_media_type(&mt_out);

      /* refMt + boolMt + fullMt + encMt */
      ASSERT_EQ(CDD_C_SUCCESS, oa_json_opt_object(mt_jo, "refMt", &sub));
      memset(&mt_out, 0, sizeof(mt_out));
      g_cdd_step_fail = sf;
      g_cdd_last_alloc_err = parse_media_type_object(sub, &mt_out, &sp, 1);
      g_cdd_step_fail = 0;
      g_cdd_last_alloc_err = free_media_type(&mt_out);

      ASSERT_EQ(CDD_C_SUCCESS, oa_json_opt_object(mt_jo, "boolMt", &sub));
      memset(&mt_out, 0, sizeof(mt_out));
      g_cdd_step_fail = sf;
      g_cdd_last_alloc_err = parse_media_type_object(sub, &mt_out, &sp, 1);
      g_cdd_step_fail = 0;
      g_cdd_last_alloc_err = free_media_type(&mt_out);

      ASSERT_EQ(CDD_C_SUCCESS, oa_json_opt_object(mt_jo, "fullMt", &sub));
      memset(&mt_out, 0, sizeof(mt_out));
      g_cdd_step_fail = sf;
      g_cdd_last_alloc_err = parse_media_type_object(sub, &mt_out, &sp, 1);
      g_cdd_step_fail = 0;
      g_cdd_last_alloc_err = free_media_type(&mt_out);

      ASSERT_EQ(CDD_C_SUCCESS, oa_json_opt_object(mt_jo, "encMt", &sub));
      memset(&mt_out, 0, sizeof(mt_out));
      g_cdd_step_fail = sf;
      g_cdd_last_alloc_err = parse_media_type_object(sub, &mt_out, &sp, 1);
      g_cdd_step_fail = 0;
      g_cdd_last_alloc_err = free_media_type(&mt_out);

      /* contentMap + find_media_object_by_name + flows */
      ASSERT_EQ(CDD_C_SUCCESS, oa_json_opt_object(mt_jo, "contentMap", &sub));
      g_cdd_step_fail = sf;
      g_cdd_last_alloc_err = parse_content_object(sub, &mt_arr, &mt_n, &sp, 1);
      g_cdd_step_fail = 0;
      if (mt_arr) {
        size_t mi;
        for (mi = 0; mi < mt_n; ++mi)
          g_cdd_last_alloc_err = free_media_type(&mt_arr[mi]);
        free(mt_arr);
      }

      g_cdd_step_fail = sf;
      g_cdd_last_alloc_err =
          find_media_object_by_name(sub, "application/json", &found_mt);
      g_cdd_step_fail = 0;

      ASSERT_EQ(CDD_C_SUCCESS, oa_json_opt_object(ex_jo, "flows", &sub));
      memset(&sec_out, 0, sizeof(sec_out));
      g_cdd_step_fail = sf;
      g_cdd_last_alloc_err = parse_oauth_flows(sub, &sec_out);
      g_cdd_step_fail = 0;
      if (sec_out.flows) {
        struct OpenAPI_Spec dummy_sp;
        memset(&dummy_sp, 0, sizeof(dummy_sp));
        dummy_sp.security_schemes =
            calloc(1, sizeof(struct OpenAPI_SecurityScheme));
        dummy_sp.security_schemes[0] = sec_out;
        dummy_sp.n_security_schemes = 1;
        g_cdd_last_alloc_err = openapi_spec_free(&dummy_sp);
      }
    }

    /* select_primary_media_type_index + preference_rank step-fail */
    {
      struct OpenAPI_MediaType mts[4];
      int idx = -1;
      memset(mts, 0, sizeof(mts));
      mts[0].name = cdd_test_mut_str("application/json");
      mts[1].name = cdd_test_mut_str("application/x-www-form-urlencoded");
      mts[2].name = cdd_test_mut_str("multipart/form-data");
      mts[3].name = cdd_test_mut_str("text/plain");
      for (sf = 1; sf <= 35; ++sf) {
        g_cdd_step_fail = sf;
        g_cdd_last_alloc_err = select_primary_media_type_index(mts, 4, &idx);
        g_cdd_step_fail = 0;
      }
    }

    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(mt_jv));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(ex_jv));
  }

  PASS();
}

/** @brief Countdown counter for Parson OOM fault injection in OpenAPI parser
 * tests. */
static int g_oa_parson_oom_at = -1;

/**
 * @brief Mock Parson allocator that fails when `g_oa_parson_oom_at` reaches 0.
 *
 * @param[in] sz Allocation size in bytes.
 * @return Allocated buffer or NULL on simulated OOM.
 */
static void *oa_mock_parson_malloc(size_t sz) {
  if (g_oa_parson_oom_at >= 0) {
    if (g_oa_parson_oom_at == 0) {
      g_oa_parson_oom_at = -1;
      return NULL;
    }
    g_oa_parson_oom_at--;
  }
  return malloc(sz);
}

/**
 * @brief Mock Parson deallocator paired with `oa_mock_parson_malloc`.
 *
 * @param[in,out] p Pointer to free.
 */
static void oa_mock_parson_free(void *p) { free(p); }

/**
 * @brief Concatenates NULL-terminated string chunks (<509 chars each for C89)
 * into a heap buffer.
 *
 * @param[in] parts NULL-terminated array of C strings.
 * @return Newly allocated concatenated string.
 */
static char *oa_join_c89_chunks(const char *const *parts) {
  size_t total = 0;
  size_t i;
  size_t pos = 0;
  char *buf;
  for (i = 0; parts[i]; ++i)
    total += strlen(parts[i]);
  buf = (char *)C_CDD_MALLOC(total + 1);
  if (!buf)
    return NULL;
  for (i = 0; parts[i]; ++i) {
    size_t len = strlen(parts[i]);
    memcpy(buf + pos, parts[i], len);
    pos += len;
  }
  buf[pos] = '\0';
  return buf;
}

/**
 * @brief Sweeps step-fail and OOM fault injection across all OpenAPI 3.2,
 * Swagger 2.0, and standalone JSON Schema parser modules.
 * @return Greatest test result.
 */
TEST test_openapi_percolation_batch2(void) {
  static const char *const oas3_chunks[] = {
      "{\"openapi\":\"3.2.0\",\"$self\":\"https://api.example.com/openapi\","
      "\"jsonSchemaDialect\":\"https://spec.openapis.org/oas/3.1/dialect/base\""
      ",\"x-root\":1,\"info\":{\"title\":\"API\",\"summary\":\"Sum\","
      "\"description\":\"Desc\",\"termsOfService\":\"https://example.com/"
      "tos\",",
      "\"version\":\"1.0\",\"x-info\":1,\"contact\":{\"name\":\"N\",\"url\":"
      "\"https://example.com\",\"email\":\"a@b.c\",\"x-c\":1},\"license\":{"
      "\"name\":\"MIT\",\"identifier\":\"MIT\",\"x-l\":1}},",
      "\"externalDocs\":{\"url\":\"https://docs.example.com\",\"description\":"
      "\"Docs\",\"x-ed\":1},\"security\":[{\"OAuth\":[\"read\"]}],",
      "\"servers\":[{\"url\":\"https://{env}.example.com/{port}\",\"name\":"
      "\"prod\",\"description\":\"Prod\",\"x-s\":1,\"variables\":{\"env\":{"
      "\"default\":\"api\",\"enum\":[\"api\",\"stg\"],\"description\":\"Env\","
      "\"x-v\":1},\"port\":{\"default\":\"443\"}}},{\"url\":\"https://b.co\","
      "\"name\":\"backup\"}],",
      "\"tags\":[{\"name\":\"rootTag\",\"description\":\"R\",\"x-t\":1,"
      "\"externalDocs\":{\"url\":\"https://t.co\"}},{\"name\":\"childTag\","
      "\"parent\":\"rootTag\"}],",
      "\"components\":{\"securitySchemes\":{\"OAuth\":{\"type\":\"oauth2\","
      "\"flows\":{\"clientCredentials\":{\"tokenUrl\":\"https://auth/token\","
      "\"scopes\":{\"read\":\"R\"}}}}},",
      "\"parameters\":{\"P1\":{\"name\":\"qref\",\"in\":\"query\",\"schema\":{"
      "\"type\":\"string\"}}},\"responses\":{\"R1\":{\"description\":\"RefR\","
      "\"summary\":\"RS\"}},\"headers\":{\"H1\":{\"description\":\"RefH\","
      "\"schema\":{\"type\":\"string\"}}},",
      "\"requestBodies\":{\"RB1\":{\"description\":\"RefRB\",\"required\":true,"
      "\"content\":{\"application/json\":{\"schema\":{\"type\":\"string\"}}}}},"
      "\"mediaTypes\":{\"MT1\":{\"schema\":{\"type\":\"string\"}}},",
      "\"examples\":{\"EX1\":{\"summary\":\"Ex\",\"value\":\"v\"}},\"links\":{"
      "\"L1\":{\"operationId\":\"opGet\",\"description\":\"LD\"}},\"callbacks\""
      ":{\"CB1\":{\"{$request.body#/u}\":{\"post\":{\"responses\":{\"200\":{"
      "\"description\":\"ok\"}}}}}},",
      "\"pathItems\":{\"PI1\":{\"summary\":\"PS\",\"description\":\"PD\","
      "\"get\":{\"operationId\":\"piGet\",\"responses\":{\"200\":{"
      "\"description\":\"ok\"}}}}},",
      "\"schemas\":{\"RawBool\":true,\"Pet\":{\"$id\":\"https://e.co/Pet\","
      "\"$anchor\":\"petA\",\"$dynamicAnchor\":\"petDA\",\"type\":\"object\","
      "\"summary\":\"PetS\",\"description\":\"PetD\",\"format\":\"obj\",",
      "\"contentMediaType\":\"application/json\",\"contentEncoding\":\"utf-8\","
      "\"contentSchema\":{\"type\":\"string\"},\"deprecated\":true,"
      "\"readOnly\":false,\"writeOnly\":false,\"default\":{\"id\":\"1\"},"
      "\"const\":{\"id\":\"1\"},\"enum\":[{\"id\":\"1\"}],\"examples\":[{\"id\""
      ":\"1\"}],",
      "\"externalDocs\":{\"url\":\"https://p.co\"},\"discriminator\":{"
      "\"propertyName\":\"kind\",\"defaultMapping\":\"Pet\",\"x-d\":1,"
      "\"mapping\":{\"x-m\":1,\"dog\":\"#/components/schemas/Pet\"}},",
      "\"xml\":{\"name\":\"pet\",\"namespace\":\"urn:x\",\"prefix\":\"p\","
      "\"nodeType\":\"element\",\"attribute\":false,\"wrapped\":true,\"x-x\":1}"
      ",\"allOf\":[{\"type\":\"object\"}],\"anyOf\":[{\"type\":\"object\"}],"
      "\"oneOf\":[{\"type\":\"object\"}],\"not\":{\"type\":\"null\"},",
      "\"if\":{\"type\":\"object\"},\"then\":{\"type\":\"object\"},\"else\":{"
      "\"type\":\"object\"},\"properties\":{\"id\":{\"type\":\"string\"},"
      "\"kind\":{\"type\":[\"string\",\"null\"]},\"arrRef\":{\"type\":\"array\""
      ",\"items\":{\"$ref\":\"#/components/schemas/RawBool\"}},",
      "\"arrDyn\":{\"type\":\"array\",\"items\":{\"$dynamicRef\":\"#petDA\"}},"
      "\"arrBool\":{\"type\":\"array\",\"items\":true},\"arrFull\":{\"type\":"
      "\"array\",\"items\":{\"type\":[\"string\",\"null\"],\"format\":\"uri\","
      "\"collectionFormat\":\"csv\",\"contentMediaType\":\"text/plain\",",
      "\"contentEncoding\":\"7bit\",\"contentSchema\":{\"type\":\"string\"},"
      "\"enum\":[\"a\"],\"examples\":[\"a\"],\"const\":\"a\",\"default\":\"a\","
      "\"minLength\":1}}}}}},",
      "\"webhooks\":{\"newPet\":{\"post\":{\"operationId\":\"whPost\","
      "\"responses\":{\"200\":{\"description\":\"ok\"}}}}},",
      "\"paths\":{\"x-paths\":1,\"/ref-path\":{\"$ref\":"
      "\"#/components/pathItems/PI1\",\"summary\":\"OS\",\"description\":\"OD\""
      "},\"/items/{id}\":{\"summary\":\"IS\",\"description\":\"ID\",\"x-p\":1,"
      "\"servers\":[{\"url\":\"https://p.example.com\"}],",
      "\"parameters\":[{\"name\":\"id\",\"in\":\"path\",\"required\":true,"
      "\"schema\":{\"type\":\"string\"}}],\"additionalOperations\":{\"COPY\":{"
      "\"operationId\":\"copyItem\",\"responses\":{\"200\":{\"description\":"
      "\"ok\"}}}},",
      "\"post\":{\"operationId\":\"opGet\",\"summary\":\"OS\",\"description\":"
      "\"OD\",\"deprecated\":true,\"externalDocs\":{\"url\":\"https://o.co\"},"
      "\"security\":[],\"tags\":[\"rootTag\"],\"servers\":[{\"url\":"
      "\"https://op.example.com\"}],\"x-op\":1,",
      "\"parameters\":[{\"$ref\":\"#/components/parameters/P1\",\"description\""
      ":\"PDesc\"},{\"name\":\"Accept\",\"in\":\"header\",\"schema\":{\"type\":"
      "\"string\"}},{\"name\":\"q1\",\"in\":\"query\",\"description\":\"QD\","
      "\"required\":false,\"deprecated\":true,\"allowReserved\":true,"
      "\"allowEmptyValue\":true,\"style\":\"form\",\"explode\":true,",
      "\"schema\":{\"type\":[\"string\",\"null\"]},\"examples\":{\"e1\":{"
      "\"value\":\"a\"}}},{\"name\":\"qb\",\"in\":\"header\",\"schema\":true,"
      "\"example\":\"x\"},{\"name\":\"qc\",\"in\":\"cookie\",\"content\":{"
      "\"text/plain\":{\"schema\":{\"type\":\"string\"},\"example\":\"y\"}}},",
      "{\"name\":\"qm\",\"in\":\"header\",\"content\":{\"text/plain\":{\"$ref\""
      ":\"#/components/mediaTypes/MT1\"}}}],",
      "\"requestBody\":{\"description\":\"RBD\",\"required\":true,\"x-rb\":1,"
      "\"content\":{\"application/json\":{\"schema\":{\"type\":\"object\","
      "\"properties\":{\"f\":{\"type\":\"string\"}}},\"examples\":{\"e1\":{"
      "\"value\":{\"f\":\"a\"}}},\"encoding\":{\"f\":{\"contentType\":"
      "\"text/plain\",\"style\":\"form\",\"explode\":true,\"allowReserved\":"
      "true,\"x-e\":1,\"headers\":{\"Content-Type\":{\"schema\":{\"type\":"
      "\"string\"}},\"X-E\":{\"schema\":{\"type\":\"string\"}}},\"encoding\":{"
      "\"sub\":{\"contentType\":\"text/plain\"}}}}}}},",
      "\"callbacks\":{\"cbRef\":{\"$ref\":\"#/components/callbacks/CB1\","
      "\"summary\":\"CBS\",\"description\":\"CBD\"},\"cbInline\":{"
      "\"x-cb\":1,\"{$url}\":{\"post\":{\"responses\":{\"200\":{\"description\""
      ":\"ok\"}}}}}},",
      "\"responses\":{\"x-r\":1,\"200\":{\"summary\":\"S200\",\"description\":"
      "\"D200\",\"x-200\":1,\"headers\":{\"Content-Type\":{\"schema\":{\"type\""
      ":\"string\"}},\"X-Ref\":{\"$ref\":\"#/components/headers/H1\","
      "\"description\":\"HD\"},\"X-Bool\":{\"description\":\"HB\",\"required\":"
      "true,\"deprecated\":true,\"style\":\"simple\",\"explode\":false,"
      "\"schema\":true,\"example\":\"1\"},",
      "\"X-Content\":{\"content\":{\"text/plain\":{\"schema\":{\"type\":"
      "\"string\"},\"example\":\"z\"}}},\"X-MtRef\":{\"content\":{\"text/"
      "plain\""
      ":{\"$ref\":\"#/components/mediaTypes/MT1\"}}},\"X-Exs\":{\"schema\":{"
      "\"type\":\"string\"},\"examples\":{\"e1\":{\"value\":\"v\"}}}},",
      "\"links\":{\"lRef\":{\"$ref\":\"#/components/links/L1\",\"summary\":"
      "\"LS\",\"description\":\"LD\"},\"lIn\":{\"operationId\":\"opGet\","
      "\"parameters\":{\"id\":\"$response.body#/id\"},\"requestBody\":\"x\","
      "\"server\":{\"url\":\"https://l.co\"},\"x-lk\":1}},",
      "\"content\":{\"application/json\":{\"schema\":{\"type\":\"array\","
      "\"items\":{\"type\":\"object\",\"properties\":{\"a\":{\"type\":"
      "\"string\""
      "}}}},\"example\":[{\"a\":\"1\"}],\"prefixEncoding\":[{\"contentType\":"
      "\"text/plain\"}],\"itemEncoding\":{\"contentType\":\"text/plain\"}}}},"
      "\"201\":{\"$ref\":\"#/components/responses/R1\",\"summary\":\"S201\","
      "\"description\":\"D201\"},\"2XX\":{\"description\":\"Wild\","
      "\"content\":{\"application/jsonl\":{\"itemSchema\":{\"type\":\"object\","
      "\"properties\":{\"m\":{\"type\":\"string\"}}}}}}}}},",
      "\"/"
      "alt\":{\"get\":{\"operationId\":\"opAltGet\",\"parameters\":[{\"name\""
      ":\"qs\",\"in\":\"querystring\",\"content\":{\"application/json\":{"
      "\"schema\":{\"type\":\"object\",\"properties\":{\"k\":{\"type\":"
      "\"string\"}}}}}}],\"responses\":{\"200\":{\"description\":\"ok\"}}},",
      "\"put\":{\"operationId\":\"opPut\",\"parameters\":[{\"name\":"
      "\"qs2\",\"in\":\"querystring\",\"content\":{"
      "\"application/x-www-form-urlencoded\":{\"schema\":{\"type\":\"object\","
      "\"properties\":{\"a\":{\"type\":\"string\"}}}}}}],\"requestBody\":{"
      "\"$ref\":\"#/components/requestBodies/RB1\",\"description\":\"ORBD\"},"
      "\"responses\":{\"default\":{\"description\":\"Def\",\"content\":{"
      "\"text/plain\":{\"$ref\":\"#/components/mediaTypes/MT1\"}}}}}}}}",
      NULL};
  static const char *const sw2_chunks[] = {
      "{\"swagger\":\"2.0\",\"info\":{\"title\":\"S2\",\"version\":\"1.0\","
      "\"license\":{\"name\":\"Apache\",\"url\":\"https://a.org\"}},"
      "\"host\":\"api.example.com\",\"basePath\":\"/v1\",\"schemes\":[\"https\""
      "],\"consumes\":[\"application/json\"],\"produces\":[\"application/json\""
      "],\"definitions\":{\"Item\":{\"type\":\"object\",\"properties\":{\"id\":"
      "{\"type\":\"string\"}}}},",
      "\"parameters\":{\"SP\":{\"name\":\"sp\",\"in\":\"query\",\"type\":"
      "\"array\",\"items\":{\"type\":\"string\"},\"collectionFormat\":\"ssv\"}}"
      ","
      "\"responses\":{\"SR\":{\"description\":\"ok\",\"schema\":{\"$ref\":"
      "\"#/definitions/Item\"},\"headers\":{\"X-Sw\":{\"type\":\"array\","
      "\"items\":{\"type\":\"string\"},\"collectionFormat\":\"csv\"}}}},",
      "\"securityDefinitions\":{\"Basic\":{\"type\":\"basic\"}},\"paths\":{"
      "\"/s\":{\"get\":{\"operationId\":\"swGet\",\"parameters\":[{\"name\":"
      "\"p1\",\"in\":\"query\",\"type\":\"array\",\"items\":{\"type\":"
      "\"string\""
      "},\"collectionFormat\":\"pipes\"},{\"name\":\"p2\",\"in\":\"query\","
      "\"type\":\"array\",\"items\":{\"type\":\"string\"},\"collectionFormat\":"
      "\"multi\"},",
      "{\"name\":\"p3\",\"in\":\"cookie\",\"type\":\"array\",\"items\":{"
      "\"type\""
      ":\"string\"},\"collectionFormat\":\"csv\"},{\"name\":\"p4\",\"in\":"
      "\"header\",\"type\":\"array\",\"items\":{\"type\":\"string\"},"
      "\"collectionFormat\":\"csv\"}],\"responses\":{\"200\":{\"description\":"
      "\"ok\",\"schema\":{\"type\":\"string\"},\"headers\":{\"X-H\":{\"type\":"
      "\"string\"}}}}}}}}",
      NULL};
  static const char *const mini_cov_chunks[] = {
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"M\",\"version\":\"1\"},"
      "\"components\":{\"requestBodies\":{\"RB\":{\"description\":\"RBD\","
      "\"content\":{\"application/json\":{\"schema\":{\"type\":\"string\"}}}}},"
      "\"schemas\":{\"a_b\":{\"type\":\"string\",\"enum\":[\"x\",\"y\"]},"
      "\"PropsOnly\":{\"properties\":{\"p\":{\"type\":\"string\"}}},"
      "\"CompOnly\":{\"allOf\":[{\"$ref\":\"#/components/schemas/"
      "PropsOnly\"}]}}},",
      "\"paths\":{\"/p/{a}/{b}\":{\"parameters\":[{\"name\":\"a\",\"in\":"
      "\"path\",\"required\":true,\"schema\":{\"type\":\"string\"}}],"
      "\"additionalOperations\":{\"LOCK\":{\"parameters\":[{\"name\":\"b\","
      "\"in\":\"path\",\"required\":true,\"schema\":{\"type\":\"string\"}}],"
      "\"responses\":{\"200\":{\"description\":\"ok\"}}}},",
      "\"post\":{\"operationId\":\"mPost\",\"parameters\":[{\"name\":\"b\","
      "\"in\":\"path\",\"required\":true,\"schema\":{\"type\":\"string\"}}],"
      "\"requestBody\":{\"$ref\":\"#/components/requestBodies/RB\","
      "\"description\":\"Ovr\"},\"responses\":{\"200\":{\"description\":\"ok\","
      "\"content\":{\"application/json\":{\"schema\":{\"type\":\"object\","
      "\"contentSchema\":{\"type\":\"string\"},\"allOf\":[{\"type\":\"string\"}"
      ","
      "{\"type\":\"string\"}],\"not\":{\"type\":\"null\"},",
      "\"discriminator\":{\"propertyName\":\"k\",\"mapping\":{\"x-m\":1,\"d\":"
      "\"#/components/schemas/PropsOnly\"}},\"xml\":{\"name\":\"n\","
      "\"nodeType\":\"element\",\"attribute\":true,\"wrapped\":false}}}}},"
      "\"201\":{\"description\":\"arr\",\"content\":{\"application/json\":{"
      "\"schema\":{\"type\":\"array\",\"items\":{\"$ref\":"
      "\"#/components/schemas/a~1b\",\"contentSchema\":{\"type\":\"string\"}"
      "}}}}}}}},",
      "\"/q/{x}/"
      "z\":{\"get\":{\"operationId\":\"qGet\",\"parameters\":[{\"name\""
      ":\"x\",\"in\":\"path\",\"required\":true,\"schema\":{\"type\":\"string\""
      "}}],\"responses\":{\"200\":{\"description\":\"ok\"}}}},\"/qs\":{\"get\":"
      "{\"operationId\":\"qsGet\",\"parameters\":[{\"name\":\"qs\",\"in\":"
      "\"querystring\",\"content\":{\"application/json\":{\"schema\":{\"type\":"
      "\"object\",\"properties\":{\"f\":{\"type\":\"string\"}}}}}}],"
      "\"responses\":{\"200\":{\"description\":\"ok\"}}}}}}",
      NULL};
  static const char *const mini_tail_chunks[] = {
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"T\",\"version\":\"1\"},"
      "\"components\":{\"responses\":{\"R1\":{\"description\":\"RD\"}},"
      "\"headers\":{\"H1\":{\"description\":\"HD\",\"schema\":{"
      "\"type\":\"string\"}}},\"mediaTypes\":{\"M1\":{\"schema\":{\"type\":"
      "\"string\"}}},\"links\":{\"L1\":{\"operationId\":\"tOp\"}},"
      "\"callbacks\":{\"C1\":{\"{$u}\":{\"post\":{\"responses\":{\"200\":{"
      "\"description\":\"ok\"}}}}}}},",
      "\"paths\":{\"/t\":{\"post\":{\"operationId\":\"tOp\",\"tags\":[\"tg\"],"
      "\"callbacks\":{\"cRef\":{\"$ref\":\"#/components/callbacks/C1\","
      "\"summary\":\"CS\",\"description\":\"CD\"},\"cIn\":{\"{$v}\":{\"post\":{"
      "\"responses\":{\"200\":{\"description\":\"ok\"}}}}}},",
      "\"responses\":{\"200\":{\"description\":\"ok\",\"headers\":{\"XB\":{"
      "\"schema\":true},\"XR\":{"
      "\"$ref\":\"#/components/headers/H1\",\"description\":\"OD\"},\"XS\":{"
      "\"style\":\"simple\",\"content\":{\"text/plain\":{\"schema\":{\"type\":"
      "\"string\"}}}},\"XM\":{\"content\":{\"text/plain\":{\"$ref\":"
      "\"#/components/mediaTypes/M1\"}}}},",
      "\"links\":{\"lR\":{\"$ref\":\"#/components/links/L1\",\"summary\":\"S\","
      "\"description\":\"D\"},\"lI\":{\"operationId\":\"tOp\",\"parameters\":{"
      "\"p\":\"1\"}}},\"content\":{\"application/jsonl\":{\"itemSchema\":{"
      "\"type\":\"object\",\"properties\":{\"x\":{\"type\":\"string\"}}},"
      "\"prefixEncoding\":[{\"contentType\":\"text/plain\"}]}}},"
      "\"201\":{\"$ref\":\"#/components/responses/R1\"}}}}}}",
      NULL};
  static const char *const oauth2_mini =
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"O\",\"version\":\"1\"},"
      "\"components\":{\"securitySchemes\":{\"OA\":{\"type\":\"oauth2\","
      "\"flows\":{\"password\":{\"tokenUrl\":\"https://e.co/t\",\"scopes\":{}}"
      "}}}},\"paths\":{}}";
  static const char *const schema_doc =
      "{\"$schema\":\"https://json-schema.org/draft/2020-12/schema\",\"$id\":"
      "\"https://e.co/root.json\",\"title\":\"RootModel\",\"description\":"
      "\"Desc\",\"$defs\":{\"Sub\":{\"type\":\"object\",\"properties\":{\"k\":"
      "{\"type\":\"string\"}}}},\"type\":\"object\",\"properties\":{\"s\":{"
      "\"$ref\":\"#/$defs/Sub\"}}}";
  const char *docs[6];
  char *oas3_json = oa_join_c89_chunks(oas3_chunks);
  char *sw2_json = oa_join_c89_chunks(sw2_chunks);
  char *mini_json = oa_join_c89_chunks(mini_cov_chunks);
  char *tail_json = oa_join_c89_chunks(mini_tail_chunks);
  size_t d;
  int k;
  struct OpenAPI_Spec spec;

  ASSERT(oas3_json != NULL);
  ASSERT(sw2_json != NULL);
  ASSERT(mini_json != NULL);
  ASSERT(tail_json != NULL);
  docs[0] = mini_json;
  docs[1] = tail_json;
  docs[2] = oas3_json;
  docs[3] = sw2_json;
  docs[4] = schema_doc;
  docs[5] = "true";

  for (d = 0; d < 6; ++d) {
    JSON_Value *doc_jv = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_parse_string(docs[d], &doc_jv));
    ASSERT(doc_jv != NULL);
    memset(&spec, 0, sizeof(spec));
    ASSERT_EQ(CDD_C_SUCCESS, openapi_load_from_json(doc_jv, &spec));
    ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_free(&spec));

    for (k = 1; k <= 2500; ++k) {
      memset(&spec, 0, sizeof(spec));
      g_cdd_step_fail = k;
      if (openapi_load_from_json(doc_jv, &spec) == CDD_C_SUCCESS) {
        g_cdd_step_fail = 0;
        ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_free(&spec));
        continue;
      }
      if (g_cdd_step_fail > 0) {
        g_cdd_step_fail = 0;
        break;
      }
    }
    g_cdd_step_fail = 0;

    for (k = 1; k <= 2500; ++k) {
      memset(&spec, 0, sizeof(spec));
      g_cdd_alloc_fail = k;
      if (openapi_load_from_json(doc_jv, &spec) == CDD_C_SUCCESS) {
        g_cdd_alloc_fail = 0;
        ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_free(&spec));
        break;
      }
      g_cdd_alloc_fail = 0;
    }

    for (k = 1; k <= 2500; ++k) {
      memset(&spec, 0, sizeof(spec));
      g_cdd_strdup_fail = k;
      if (openapi_load_from_json(doc_jv, &spec) == CDD_C_SUCCESS) {
        g_cdd_strdup_fail = 0;
        ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_free(&spec));
        break;
      }
      g_cdd_strdup_fail = 0;
    }
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(doc_jv));
  }

  for (k = 1; k <= 300; ++k) {
    JSON_Value *oa_jv = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_parse_string(oauth2_mini, &oa_jv));
    memset(&spec, 0, sizeof(spec));
    g_cdd_step_fail = k;
    if (openapi_load_from_json(oa_jv, &spec) == CDD_C_SUCCESS) {
      g_cdd_step_fail = 0;
      ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_free(&spec));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(oa_jv));
      continue;
    }
    g_cdd_last_alloc_err = cdd_json_value_free(oa_jv);
    if (g_cdd_step_fail > 0) {
      g_cdd_step_fail = 0;
      break;
    }
  }
  g_cdd_step_fail = 0;

  for (k = 0; k <= 60; ++k) {
    JSON_Value *oa_jv = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_parse_string(oauth2_mini, &oa_jv));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_set_allocation_functions(
                                 oa_mock_parson_malloc, oa_mock_parson_free));
    memset(&spec, 0, sizeof(spec));
    g_oa_parson_oom_at = k;
    if (openapi_load_from_json(oa_jv, &spec) == CDD_C_SUCCESS) {
      g_oa_parson_oom_at = -1;
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_set_allocation_functions(NULL, NULL));
      ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_free(&spec));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(oa_jv));
      break;
    }
    g_oa_parson_oom_at = -1;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_set_allocation_functions(NULL, NULL));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_json_value_free(oa_jv));
  }

  C_CDD_FREE(tail_json);
  C_CDD_FREE(mini_json);

  C_CDD_FREE(oas3_json);
  C_CDD_FREE(sw2_json);
  PASS();
}

/**
 * @brief Registers openapi utils coverage tests.
 */
#define OPENAPI_UTILS_COVERAGE_TESTS()                                         \
  RUN_TEST(test_openapi_utils_verbs_and_params);                               \
  RUN_TEST(test_openapi_utils_param_style_and_types);                          \
  RUN_TEST(test_openapi_utils_keys_and_maps);                                  \
  RUN_TEST(test_openapi_utils_security_and_xml);                               \
  RUN_TEST(test_openapi_utils_any_values);                                     \
  RUN_TEST(test_openapi_utils_clone_and_extras);                               \
  RUN_TEST(test_openapi_utils_url_version_examples);                           \
  RUN_TEST(test_openapi_percolation_batch1);                                   \
  RUN_TEST(test_openapi_percolation_batch2)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_OPENAPI_UTILS_COVERAGE_H */
