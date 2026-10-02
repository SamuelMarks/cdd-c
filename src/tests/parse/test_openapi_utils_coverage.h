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
#include "openapi/parse/openapi_test_helpers.h"
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
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_is_fixed_operation_method(NULL));
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, cdd_test_is_fixed_operation_method("get"));
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, cdd_test_is_fixed_operation_method("POST"));
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, cdd_test_is_fixed_operation_method("Put"));
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, cdd_test_is_fixed_operation_method("DELETE"));
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, cdd_test_is_fixed_operation_method("patch"));
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, cdd_test_is_fixed_operation_method("HEAD"));
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, cdd_test_is_fixed_operation_method("options"));
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, cdd_test_is_fixed_operation_method("TRACE"));
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, cdd_test_is_fixed_operation_method("query"));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_is_fixed_operation_method("foobar"));

  {
    int eq = 0;
    extern C_CDD_EXPORT int g_cdd_fail_str_iequal;
    g_cdd_fail_str_iequal = 1;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_test_is_fixed_operation_method("get"));
    g_cdd_fail_str_iequal = 0;

    g_cdd_fail_str_iequal = 2;
    ASSERT_EQ(CDD_C_SUCCESS, c_cdd_str_iequal("a", "b", &eq));
    g_cdd_fail_str_iequal = 0;
  }

  /* 3. parse_param_in */
  rc = cdd_test_parse_param_in(NULL, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = cdd_test_parse_param_in(NULL, &p_in);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_PARAM_IN_UNKNOWN, p_in);

  rc = cdd_test_parse_param_in("path", &p_in);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_PARAM_IN_PATH, p_in);
  rc = cdd_test_parse_param_in("query", &p_in);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_PARAM_IN_QUERY, p_in);
  rc = cdd_test_parse_param_in("querystring", &p_in);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_PARAM_IN_QUERYSTRING, p_in);
  rc = cdd_test_parse_param_in("header", &p_in);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_PARAM_IN_HEADER, p_in);
  rc = cdd_test_parse_param_in("cookie", &p_in);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_PARAM_IN_COOKIE, p_in);
  rc = cdd_test_parse_param_in("body", &p_in);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_PARAM_IN_BODY, p_in);
  rc = cdd_test_parse_param_in("formData", &p_in);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_PARAM_IN_FORM_DATA, p_in);
  rc = cdd_test_parse_param_in("unknown", &p_in);
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
  rc = cdd_test_parse_param_style(NULL, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = cdd_test_parse_param_style(NULL, &style);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_STYLE_UNKNOWN, style);

  rc = cdd_test_parse_param_style("form", &style);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_STYLE_FORM, style);
  rc = cdd_test_parse_param_style("simple", &style);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_STYLE_SIMPLE, style);
  rc = cdd_test_parse_param_style("matrix", &style);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_STYLE_MATRIX, style);
  rc = cdd_test_parse_param_style("label", &style);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_STYLE_LABEL, style);
  rc = cdd_test_parse_param_style("spaceDelimited", &style);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_STYLE_SPACE_DELIMITED, style);
  rc = cdd_test_parse_param_style("pipeDelimited", &style);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_STYLE_PIPE_DELIMITED, style);
  rc = cdd_test_parse_param_style("deepObject", &style);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_STYLE_DEEP_OBJECT, style);
  rc = cdd_test_parse_param_style("cookie", &style);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_STYLE_COOKIE, style);
  rc = cdd_test_parse_param_style("custom", &style);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_STYLE_UNKNOWN, style);

  /* 2. param_type_is_primitive */
  ASSERT_EQ(CDD_C_SUCCESS, param_type_is_primitive(NULL));
  ASSERT(param_type_is_primitive("string") != 0);
  ASSERT(param_type_is_primitive("integer") != 0);
  ASSERT(param_type_is_primitive("number") != 0);
  ASSERT(param_type_is_primitive("boolean") != 0);
  ASSERT_EQ(0, param_type_is_primitive("object"));
  ASSERT_EQ(0, param_type_is_primitive("array"));

  /* 3. param_type_is_object_like */
  memset(&p, 0, sizeof(p));
  ASSERT_EQ(0, param_type_is_object_like(NULL));
  ASSERT_EQ(0, param_type_is_object_like(&p));
  p.type = (char *)(size_t) "array";
  ASSERT_EQ(0, param_type_is_object_like(&p));
  p.type = (char *)(size_t) "string";
  ASSERT_EQ(0, param_type_is_object_like(&p));
  p.type = (char *)(size_t) "MyModel";
  ASSERT(param_type_is_object_like(&p) != 0);

  /* 4. validate_parameter_style */
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_validate_parameter_style(NULL, 0));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_validate_parameter_style(&p, 1));

  p.in = OA_PARAM_IN_QUERYSTRING;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_validate_parameter_style(&p, 0));

  /* Query: valid styles */
  p.in = OA_PARAM_IN_QUERY;
  p.style = OA_STYLE_FORM;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_validate_parameter_style(&p, 0));
  p.style = OA_STYLE_SPACE_DELIMITED;
  p.is_array = 1;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_validate_parameter_style(&p, 0));
  p.style = OA_STYLE_PIPE_DELIMITED;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_validate_parameter_style(&p, 0));
  p.style = OA_STYLE_DEEP_OBJECT;
  p.is_array = 0;
  p.type = (char *)(size_t) "MyObj";
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_validate_parameter_style(&p, 0));

  /* Query: invalid style */
  p.style = OA_STYLE_MATRIX;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_validate_parameter_style(&p, 0));

  /* Path: valid and invalid */
  p.in = OA_PARAM_IN_PATH;
  p.style = OA_STYLE_SIMPLE;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_validate_parameter_style(&p, 0));
  p.style = OA_STYLE_MATRIX;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_validate_parameter_style(&p, 0));
  p.style = OA_STYLE_LABEL;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_validate_parameter_style(&p, 0));
  p.style = OA_STYLE_FORM;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_validate_parameter_style(&p, 0));

  /* Header: valid and invalid */
  p.in = OA_PARAM_IN_HEADER;
  p.style = OA_STYLE_SIMPLE;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_validate_parameter_style(&p, 0));
  p.style = OA_STYLE_FORM;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_validate_parameter_style(&p, 0));

  /* Cookie: valid and invalid */
  p.in = OA_PARAM_IN_COOKIE;
  p.style = OA_STYLE_FORM;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_validate_parameter_style(&p, 0));
  p.style = OA_STYLE_COOKIE;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_validate_parameter_style(&p, 0));
  p.style = OA_STYLE_SIMPLE;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_validate_parameter_style(&p, 0));

  /* Default in branch */
  p.in = OA_PARAM_IN_BODY;
  p.style = OA_STYLE_FORM;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_validate_parameter_style(&p, 0));

  /* DeepObject errors: array or primitive */
  p.in = OA_PARAM_IN_QUERY;
  p.style = OA_STYLE_DEEP_OBJECT;
  p.is_array = 1;
  p.type = (char *)(size_t) "MyObj";
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_validate_parameter_style(&p, 0));
  p.is_array = 0;
  p.type = (char *)(size_t) "string";
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_validate_parameter_style(&p, 0));

  /* spaceDelimited / pipeDelimited error: neither array nor object-like */
  p.style = OA_STYLE_SPACE_DELIMITED;
  p.is_array = 0;
  p.type = (char *)(size_t) "integer";
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_validate_parameter_style(&p, 0));
  p.style = OA_STYLE_PIPE_DELIMITED;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_validate_parameter_style(&p, 0));

  /* spaceDelimited / pipeDelimited success: object-like */
  p.type = (char *)(size_t) "MyObj";
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_validate_parameter_style(&p, 0));
  p.style = OA_STYLE_SPACE_DELIMITED;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_validate_parameter_style(&p, 0));

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

  /* 1. component_key_is_valid */
  ASSERT_EQ(CDD_C_SUCCESS, component_key_is_valid(NULL));
  ASSERT_EQ(CDD_C_SUCCESS, component_key_is_valid(""));
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, component_key_is_valid("Valid_Name.1-ok"));
  ASSERT_EQ(CDD_C_SUCCESS, component_key_is_valid("Invalid@Key"));

  /* 2. media_type_key_is_valid */
  ASSERT_EQ(CDD_C_SUCCESS, media_type_key_is_valid(NULL));
  ASSERT_EQ(CDD_C_SUCCESS, media_type_key_is_valid(""));
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            media_type_key_is_valid("application/json+vnd.*_1"));
  ASSERT_EQ(CDD_C_SUCCESS, media_type_key_is_valid("invalid#media"));

  /* 3. validate_component_key_map */
  ASSERT_EQ(CDD_C_SUCCESS, validate_component_key_map(NULL));
  jv = json_parse_string("{\"Key1\": 1, \"Key_2.ok\": 2}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, validate_component_key_map(jo));
  json_value_free(jv);

  jv = json_parse_string("{\"Key1\": 1, \"Bad@Key\": 2}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, validate_component_key_map(jo));
  json_value_free(jv);

  /* 4. validate_media_type_key_map */
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_validate_media_type_key_map(NULL));
  jv = json_parse_string("{\"application/json\": 1, \"text/*\": 2}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_validate_media_type_key_map(jo));
  json_value_free(jv);

  jv = json_parse_string("{\"bad!media\": 1}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_validate_media_type_key_map(jo));
  json_value_free(jv);

  /* 5. header_name_is_content_type */
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_header_name_is_content_type(NULL));
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_header_name_is_content_type("content-type"));
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_header_name_is_content_type("Content-Type"));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_header_name_is_content_type("Accept"));

  /* 6. header_param_is_reserved */
  memset(&p, 0, sizeof(p));
  ASSERT_EQ(CDD_C_SUCCESS, header_param_is_reserved(NULL));
  ASSERT_EQ(CDD_C_SUCCESS, header_param_is_reserved(&p));
  p.in = OA_PARAM_IN_QUERY;
  p.name = (char *)(size_t) "Accept";
  ASSERT_EQ(CDD_C_SUCCESS, header_param_is_reserved(&p));

  p.in = OA_PARAM_IN_HEADER;
  p.name = (char *)(size_t) "Accept";
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, header_param_is_reserved(&p));
  p.name = (char *)(size_t) "Content-Type";
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, header_param_is_reserved(&p));
  p.name = (char *)(size_t) "Authorization";
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, header_param_is_reserved(&p));
  p.name = (char *)(size_t) "X-Custom";
  ASSERT_EQ(CDD_C_SUCCESS, header_param_is_reserved(&p));

  p.name = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, header_param_is_reserved(&p));

  {
    extern C_CDD_EXPORT int g_cdd_fail_str_iequal;
    g_cdd_fail_str_iequal = 1;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_test_header_name_is_content_type("Content-Type"));
    g_cdd_fail_str_iequal = 0;

    g_cdd_fail_str_iequal = 1;
    p.name = (char *)(size_t) "Accept";
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, header_param_is_reserved(&p));
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
  rc = cdd_test_parse_security_type(NULL, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = cdd_test_parse_security_type(NULL, &sec_type);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_SEC_UNKNOWN, sec_type);

  rc = cdd_test_parse_security_type("apiKey", &sec_type);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_SEC_APIKEY, sec_type);
  rc = cdd_test_parse_security_type("http", &sec_type);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_SEC_HTTP, sec_type);
  rc = cdd_test_parse_security_type("mutualTLS", &sec_type);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_SEC_MUTUALTLS, sec_type);
  rc = cdd_test_parse_security_type("oauth2", &sec_type);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_SEC_OAUTH2, sec_type);
  rc = cdd_test_parse_security_type("openIdConnect", &sec_type);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_SEC_OPENID, sec_type);
  rc = cdd_test_parse_security_type("other", &sec_type);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_SEC_UNKNOWN, sec_type);

  /* 2. parse_security_in */
  rc = cdd_test_parse_security_in(NULL, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = cdd_test_parse_security_in(NULL, &sec_in);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_SEC_IN_UNKNOWN, sec_in);

  rc = cdd_test_parse_security_in("query", &sec_in);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_SEC_IN_QUERY, sec_in);
  rc = cdd_test_parse_security_in("header", &sec_in);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_SEC_IN_HEADER, sec_in);
  rc = cdd_test_parse_security_in("cookie", &sec_in);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_SEC_IN_COOKIE, sec_in);
  rc = cdd_test_parse_security_in("other", &sec_in);
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
  rc = cdd_test_parse_xml_node_type(NULL, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = cdd_test_parse_xml_node_type(NULL, &xml_type);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_XML_NODE_UNSET, xml_type);

  rc = cdd_test_parse_xml_node_type("element", &xml_type);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_XML_NODE_ELEMENT, xml_type);
  rc = cdd_test_parse_xml_node_type("attribute", &xml_type);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_XML_NODE_ATTRIBUTE, xml_type);
  rc = cdd_test_parse_xml_node_type("text", &xml_type);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_XML_NODE_TEXT, xml_type);
  rc = cdd_test_parse_xml_node_type("cdata", &xml_type);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_XML_NODE_CDATA, xml_type);
  rc = cdd_test_parse_xml_node_type("none", &xml_type);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_XML_NODE_NONE, xml_type);
  rc = cdd_test_parse_xml_node_type("other", &xml_type);
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
  jv = json_parse_string("\"test_str\"");
  memset(&any, 0, sizeof(any));
  rc = parse_any_value(jv, &any);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_ANY_STRING, any.type);
  ASSERT_STR_EQ("test_str", any.string);
  cdd_test_free_any_value(&any);
  json_value_free(jv);

  jv = json_parse_string("123.45");
  memset(&any, 0, sizeof(any));
  rc = parse_any_value(jv, &any);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_ANY_NUMBER, any.type);
  cdd_test_free_any_value(&any);
  json_value_free(jv);

  jv = json_parse_string("true");
  memset(&any, 0, sizeof(any));
  rc = parse_any_value(jv, &any);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_ANY_BOOL, any.type);
  ASSERT_EQ(1, any.boolean);
  cdd_test_free_any_value(&any);
  json_value_free(jv);

  jv = json_parse_string("null");
  memset(&any, 0, sizeof(any));
  rc = parse_any_value(jv, &any);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_ANY_NULL, any.type);
  cdd_test_free_any_value(&any);
  json_value_free(jv);

  jv = json_parse_string("{\"nested\": 1}");
  memset(&any, 0, sizeof(any));
  rc = parse_any_value(jv, &any);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(OA_ANY_JSON, any.type);
  ASSERT(any.json != NULL);
  cdd_test_free_any_value(&any);

  {
    JSON_Value *jv_arr = json_parse_string("[1, 2]");
    memset(&any, 0, sizeof(any));
    rc = parse_any_value(jv_arr, &any);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_EQ(OA_ANY_JSON, any.type);
    ASSERT(any.json != NULL);
    cdd_test_free_any_value(&any);
    json_value_free(jv_arr);
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
    cdd_test_free_any_value(&any);
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
  json_value_free(jv);

  /* 3. parse_any_field */
  ASSERT_EQ(CDD_C_SUCCESS, parse_any_field(NULL, NULL, NULL, NULL));
  jv = json_parse_string("{\"key1\": \"val1\"}");
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
  cdd_test_free_any_value(&any);

  g_cdd_strdup_fail = 1;
  rc = parse_any_field(jo, "key1", &any, &out_set);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  g_cdd_strdup_fail = 0;
  json_value_free(jv);

  /* 4. parse_any_array */
  ASSERT_EQ(CDD_C_SUCCESS, parse_any_array(NULL, NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, parse_any_array(NULL, &any_arr, &any_count));
  ASSERT(any_arr == NULL);
  ASSERT_EQ(0, any_count);

  jv = json_parse_string("[]");
  ja = json_value_get_array(jv);
  ASSERT_EQ(CDD_C_SUCCESS, parse_any_array(ja, NULL, &any_count));
  ASSERT_EQ(CDD_C_SUCCESS, parse_any_array(ja, &any_arr, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, parse_any_array(ja, &any_arr, &any_count));
  ASSERT(any_arr == NULL);
  ASSERT_EQ(0, any_count);
  json_value_free(jv);

  jv = json_parse_string("[\"item1\", 2, true]");
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
      cdd_test_free_any_value(&any_arr[k]);
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
  json_value_free(jv);

  {
    JSON_Value *jv2 = json_parse_string("[\"item1\", \"item2\"]");
    JSON_Array *ja2 = json_value_get_array(jv2);
    g_cdd_strdup_fail = 2;
    rc = parse_any_array(ja2, &any_arr, &any_count);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    g_cdd_strdup_fail = 0;
    json_value_free(jv2);
  }

  PASS();
}

/**
 * @brief Tests cdd_test_clone_json_value, collect_schema_extras, and
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

  /* 1. cdd_test_clone_json_value */
  rc = cdd_test_clone_json_value(NULL, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = cdd_test_clone_json_value(NULL, &clone);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(clone == NULL);

  jv = json_parse_string("{\"a\": 1, \"b\": \"two\"}");
  rc = cdd_test_clone_json_value(jv, &clone);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(clone != NULL);
  json_value_free(clone);
  clone = NULL;

  /* OOM on clone serialize */
  {
    extern C_CDD_EXPORT int g_cdd_fail_clone_json_serialize;
    g_cdd_fail_clone_json_serialize = 1;
    rc = cdd_test_clone_json_value(jv, &clone);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    ASSERT(clone == NULL);
    g_cdd_fail_clone_json_serialize = 0;

    g_cdd_fail_clone_json_serialize = 2;
    rc = cdd_test_clone_json_value(jv, &clone);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT(clone != NULL);
    json_value_free(clone);
    clone = NULL;
    g_cdd_fail_clone_json_serialize = 0;
  }

  /* OOM on clone parse */
  {
    extern C_CDD_EXPORT int g_cdd_fail_clone_json_parse;
    g_cdd_fail_clone_json_parse = 1;
    rc = cdd_test_clone_json_value(jv, &clone);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    ASSERT(clone == NULL);
    g_cdd_fail_clone_json_parse = 0;

    g_cdd_fail_clone_json_parse = 2;
    rc = cdd_test_clone_json_value(jv, &clone);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT(clone != NULL);
    json_value_free(clone);
    clone = NULL;
    g_cdd_fail_clone_json_parse = 0;
  }
  json_value_free(jv);

  /* 2. key_in_list */
  ASSERT_EQ(CDD_C_SUCCESS, key_in_list(NULL, NULL, 0));
  skip[0] = "skip1";
  skip[1] = "skip2";
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, key_in_list("skip1", skip, 2));
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, key_in_list("skip2", skip, 2));
  ASSERT_EQ(CDD_C_SUCCESS, key_in_list("keep", skip, 2));
  skip[0] = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, key_in_list("skip1", skip, 2));
  skip[0] = "skip1";

  /* 3. is_extension_key */
  ASSERT_EQ(CDD_C_SUCCESS, is_extension_key(NULL));
  ASSERT_EQ(CDD_C_SUCCESS, is_extension_key(""));
  ASSERT_EQ(CDD_C_SUCCESS, is_extension_key("x"));
  ASSERT_EQ(CDD_C_SUCCESS, is_extension_key("regularKey"));
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, is_extension_key("x-my-ext"));

  /* 4. collect_schema_extras */
  rc = cdd_test_collect_schema_extras(NULL, NULL, 0, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  jv = json_parse_string("{}");
  jo = json_value_get_object(jv);
  rc = cdd_test_collect_schema_extras(NULL, skip, 2, &out_json);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = cdd_test_collect_schema_extras(jo, skip, 2, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = cdd_test_collect_schema_extras(jo, skip, 2, &out_json);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(out_json == NULL);
  json_value_free(jv);

  jv = json_parse_string("{\"skip1\": 1, \"extra1\": \"val1\", \"extra2\": 2}");
  jo = json_value_get_object(jv);
  rc = cdd_test_collect_schema_extras(jo, skip, 2, &out_json);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(out_json != NULL);
  free(out_json);
  out_json = NULL;

  /* OOM on extras init */
  {
    extern C_CDD_EXPORT int g_cdd_fail_extras_init;
    g_cdd_fail_extras_init = 1;
    rc = cdd_test_collect_schema_extras(jo, skip, 2, &out_json);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    g_cdd_fail_extras_init = 0;

    g_cdd_fail_extras_init = 2;
    rc = cdd_test_collect_schema_extras(jo, skip, 2, &out_json);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    free(out_json);
    out_json = NULL;
    g_cdd_fail_extras_init = 0;
  }

  /* OOM on extras clone */
  {
    extern C_CDD_EXPORT int g_cdd_fail_clone_json_serialize;
    g_cdd_fail_clone_json_serialize = 1;
    rc = cdd_test_collect_schema_extras(jo, skip, 2, &out_json);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    g_cdd_fail_clone_json_serialize = 0;
  }

  /* OOM on extras set_value */
  {
    extern C_CDD_EXPORT volatile int g_cdd_fail_json_set_value;
    g_cdd_fail_json_set_value = 1;
    rc = cdd_test_collect_schema_extras(jo, skip, 2, &out_json);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    g_cdd_fail_json_set_value = 0;

    g_cdd_fail_json_set_value = 3;
    rc = cdd_test_collect_schema_extras(jo, skip, 2, &out_json);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    free(out_json);
    out_json = NULL;
    g_cdd_fail_json_set_value = 0;
  }

  /* OOM on extras serialize */
  {
    extern C_CDD_EXPORT int g_cdd_fail_extras_serialize;
    g_cdd_fail_extras_serialize = 1;
    rc = cdd_test_collect_schema_extras(jo, skip, 2, &out_json);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    g_cdd_fail_extras_serialize = 0;

    g_cdd_fail_extras_serialize = 2;
    rc = cdd_test_collect_schema_extras(jo, skip, 2, &out_json);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    free(out_json);
    out_json = NULL;
    g_cdd_fail_extras_serialize = 0;
  }

  /* OOM on extras strdup */
  g_cdd_strdup_fail = 1;
  rc = cdd_test_collect_schema_extras(jo, skip, 2, &out_json);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  g_cdd_strdup_fail = 0;
  json_value_free(jv);

  /* 5. collect_extensions */
  rc = cdd_test_collect_extensions(NULL, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = cdd_test_collect_extensions(NULL, &out_json);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  jv = json_parse_string("{\"non_ext\": 1}");
  jo = json_value_get_object(jv);
  rc = cdd_test_collect_extensions(jo, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = cdd_test_collect_extensions(jo, &out_json);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(out_json == NULL);
  json_value_free(jv);

  jv = json_parse_string("{\"x-tag\": \"val\", \"regular\": 2}");
  jo = json_value_get_object(jv);
  rc = cdd_test_collect_extensions(jo, &out_json);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(out_json != NULL);
  free(out_json);
  out_json = NULL;

  /* OOM on extensions init */
  {
    extern C_CDD_EXPORT int g_cdd_fail_extensions_init;
    g_cdd_fail_extensions_init = 1;
    rc = cdd_test_collect_extensions(jo, &out_json);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    g_cdd_fail_extensions_init = 0;

    g_cdd_fail_extensions_init = 2;
    rc = cdd_test_collect_extensions(jo, &out_json);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    free(out_json);
    out_json = NULL;
    g_cdd_fail_extensions_init = 0;
  }

  /* OOM on extensions clone */
  {
    extern C_CDD_EXPORT int g_cdd_fail_clone_json_serialize;
    g_cdd_fail_clone_json_serialize = 1;
    rc = cdd_test_collect_extensions(jo, &out_json);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    g_cdd_fail_clone_json_serialize = 0;
  }

  /* OOM on extensions set_value */
  {
    extern C_CDD_EXPORT volatile int g_cdd_fail_json_set_value;
    g_cdd_fail_json_set_value = 1;
    rc = cdd_test_collect_extensions(jo, &out_json);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    g_cdd_fail_json_set_value = 0;

    g_cdd_fail_json_set_value = 2;
    rc = cdd_test_collect_extensions(jo, &out_json);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    free(out_json);
    out_json = NULL;
    g_cdd_fail_json_set_value = 0;
  }

  /* OOM on extensions serialize */
  {
    extern C_CDD_EXPORT int g_cdd_fail_extensions_serialize;
    g_cdd_fail_extensions_serialize = 1;
    rc = cdd_test_collect_extensions(jo, &out_json);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    g_cdd_fail_extensions_serialize = 0;

    g_cdd_fail_extensions_serialize = 2;
    rc = cdd_test_collect_extensions(jo, &out_json);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    free(out_json);
    out_json = NULL;
    g_cdd_fail_extensions_serialize = 0;
  }

  /* OOM on extensions strdup */
  g_cdd_strdup_fail = 1;
  rc = cdd_test_collect_extensions(jo, &out_json);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  g_cdd_strdup_fail = 0;
  json_value_free(jv);

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

  /* 1. url_has_query_or_fragment */
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_url_has_query_or_fragment(NULL));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_url_has_query_or_fragment("https://example.com/api"));
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_url_has_query_or_fragment("https://example.com/api?a=1"));
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, cdd_test_url_has_query_or_fragment(
                                     "https://example.com/api#section"));

  /* 2. openapi_version_supported */
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_openapi_version_supported(NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_openapi_version_supported(""));
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, cdd_test_openapi_version_supported("3.0.0"));
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, cdd_test_openapi_version_supported("3.1.0"));
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, cdd_test_openapi_version_supported("3.2.0"));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_openapi_version_supported("2.0"));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_openapi_version_supported("4.0.0"));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_openapi_version_supported("3x"));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_openapi_version_supported("3.9"));

  /* 3. example_fields_valid */
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, cdd_test_example_fields_valid(NULL));
  memset(&ex, 0, sizeof(ex));
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, cdd_test_example_fields_valid(&ex));

  /* Conflicting data_value and value */
  ex.data_value_set = 1;
  ex.value_set = 1;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_example_fields_valid(&ex));

  /* Conflicting serialized and external */
  memset(&ex, 0, sizeof(ex));
  ex.serialized_value = (char *)(size_t) "ser";
  ex.external_value = (char *)(size_t) "ext";
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_example_fields_valid(&ex));

  /* Conflicting value and serialized */
  memset(&ex, 0, sizeof(ex));
  ex.value_set = 1;
  ex.serialized_value = (char *)(size_t) "ser";
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_example_fields_valid(&ex));

  /* Conflicting value and external */
  memset(&ex, 0, sizeof(ex));
  ex.value_set = 1;
  ex.external_value = (char *)(size_t) "ext";
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_example_fields_valid(&ex));

  /* 4. object_has_example_and_examples */
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_object_has_example_and_examples(NULL));
  jv = json_parse_string("{\"example\": 1}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_object_has_example_and_examples(jo));
  json_value_free(jv);

  jv = json_parse_string("{\"examples\": {}}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_object_has_example_and_examples(jo));
  json_value_free(jv);

  jv = json_parse_string("{\"example\": 1, \"examples\": {}}");
  jo = json_value_get_object(jv);
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, cdd_test_object_has_example_and_examples(jo));
  json_value_free(jv);

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
  RUN_TEST(test_openapi_utils_url_version_examples)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_OPENAPI_UTILS_COVERAGE_H */
