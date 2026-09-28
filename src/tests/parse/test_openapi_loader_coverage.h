/**
 * @file test_openapi_loader_coverage.h
 * @brief Exhaustive coverage tests for OpenAPI loader.
 */

#ifndef TEST_OPENAPI_LOADER_COVERAGE_H
#define TEST_OPENAPI_LOADER_COVERAGE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "parse/test_openapi_loader_common.h"
/* clang-format on */

TEST test_openapi_loader_100_percent_final(void) {
  enum OpenAPI_SecurityType stype = OA_SEC_UNKNOWN;
  enum OpenAPI_SecurityIn sin = OA_SEC_IN_UNKNOWN;
  enum OpenAPI_ParamIn pin = OA_PARAM_IN_QUERY;
  enum OpenAPI_Style pstyle = OA_STYLE_SIMPLE;
  enum OpenAPI_XmlNodeType xnode = OA_XML_NODE_NONE;
  size_t blen = 0;
  size_t qs_count = 0;
  int has_query = 0;
  struct OpenAPI_Parameter p_arr[2];
  struct OpenAPI_Path path_arr[2];
  JSON_Value *val_tmp = NULL;
  JSON_Object *obj_tmp = NULL;
  struct OpenAPI_Example ex_tmp;
  char s_name[16];
  char s_parent[16];
  char s_pname[16];
  char *norm_route = NULL;
  struct OpenAPI_Parameter *found_p = NULL;
  int k = 0;

  /* 1. Predicates and helpers */
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_param_in("body", &pin));
  ASSERT_EQ(OA_PARAM_IN_BODY, pin);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_param_in("formData", &pin));
  ASSERT_EQ(OA_PARAM_IN_FORM_DATA, pin);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_param_in("unknown_in", &pin));
  ASSERT_EQ(OA_PARAM_IN_UNKNOWN, pin);

  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_param_style(NULL, &pstyle));
  ASSERT_EQ(OA_STYLE_UNKNOWN, pstyle);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_param_style("matrix", &pstyle));
  ASSERT_EQ(OA_STYLE_MATRIX, pstyle);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_param_style("label", &pstyle));
  ASSERT_EQ(OA_STYLE_LABEL, pstyle);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_param_style("form", &pstyle));
  ASSERT_EQ(OA_STYLE_FORM, pstyle);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_param_style("simple", &pstyle));
  ASSERT_EQ(OA_STYLE_SIMPLE, pstyle);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_parse_param_style("spaceDelimited", &pstyle));
  ASSERT_EQ(OA_STYLE_SPACE_DELIMITED, pstyle);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_parse_param_style("pipeDelimited", &pstyle));
  ASSERT_EQ(OA_STYLE_PIPE_DELIMITED, pstyle);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_param_style("deepObject", &pstyle));
  ASSERT_EQ(OA_STYLE_DEEP_OBJECT, pstyle);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_parse_param_style("unknown_style", &pstyle));
  ASSERT_EQ(OA_STYLE_UNKNOWN, pstyle);

  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_security_type(NULL, &stype));
  ASSERT_EQ(OA_SEC_UNKNOWN, stype);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_security_type("apiKey", &stype));
  ASSERT_EQ(OA_SEC_APIKEY, stype);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_security_type("http", &stype));
  ASSERT_EQ(OA_SEC_HTTP, stype);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_security_type("oauth2", &stype));
  ASSERT_EQ(OA_SEC_OAUTH2, stype);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_parse_security_type("openIdConnect", &stype));
  ASSERT_EQ(OA_SEC_OPENID, stype);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_security_type("mutualTLS", &stype));
  ASSERT_EQ(OA_SEC_MUTUALTLS, stype);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_security_type("bogus", &stype));
  ASSERT_EQ(OA_SEC_UNKNOWN, stype);

  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_security_in(NULL, &sin));
  ASSERT_EQ(OA_SEC_IN_UNKNOWN, sin);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_security_in("header", &sin));
  ASSERT_EQ(OA_SEC_IN_HEADER, sin);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_security_in("query", &sin));
  ASSERT_EQ(OA_SEC_IN_QUERY, sin);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_security_in("cookie", &sin));
  ASSERT_EQ(OA_SEC_IN_COOKIE, sin);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_security_in("other", &sin));
  ASSERT_EQ(OA_SEC_IN_UNKNOWN, sin);

  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_xml_node_type(NULL, &xnode));
  ASSERT_EQ(OA_XML_NODE_UNSET, xnode);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_xml_node_type("element", &xnode));
  ASSERT_EQ(OA_XML_NODE_ELEMENT, xnode);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_xml_node_type("attribute", &xnode));
  ASSERT_EQ(OA_XML_NODE_ATTRIBUTE, xnode);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_xml_node_type("text", &xnode));
  ASSERT_EQ(OA_XML_NODE_TEXT, xnode);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_xml_node_type("cdata", &xnode));
  ASSERT_EQ(OA_XML_NODE_CDATA, xnode);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_xml_node_type("none", &xnode));
  ASSERT_EQ(OA_XML_NODE_NONE, xnode);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_xml_node_type("bogus", &xnode));
  ASSERT_EQ(OA_XML_NODE_UNSET, xnode);

  ASSERT(cdd_test_header_name_is_content_type("Content-Type") != CDD_C_SUCCESS);
  ASSERT(cdd_test_header_name_is_content_type("content-type") != CDD_C_SUCCESS);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_header_name_is_content_type(NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_header_name_is_content_type("Accept"));

  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_param_type_is_primitive(NULL));
  ASSERT(cdd_test_param_type_is_primitive("string") != CDD_C_SUCCESS);
  ASSERT(cdd_test_param_type_is_primitive("integer") != CDD_C_SUCCESS);
  ASSERT(cdd_test_param_type_is_primitive("number") != CDD_C_SUCCESS);
  ASSERT(cdd_test_param_type_is_primitive("boolean") != CDD_C_SUCCESS);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_param_type_is_primitive("object"));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_param_type_is_primitive("array"));

  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_url_has_query_or_fragment(NULL));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_url_has_query_or_fragment("https://example.com/api"));
  ASSERT(cdd_test_url_has_query_or_fragment("https://example.com/api?v=1") !=
         CDD_C_SUCCESS);
  ASSERT(cdd_test_url_has_query_or_fragment("https://example.com/api#sec") !=
         CDD_C_SUCCESS);

  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_openapi_version_supported(NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_openapi_version_supported(""));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_openapi_version_supported("2.0"));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_openapi_version_supported("4.0.0"));
  ASSERT(cdd_test_openapi_version_supported("3.0.0") != CDD_C_SUCCESS);
  ASSERT(cdd_test_openapi_version_supported("3.1.0") != CDD_C_SUCCESS);
  ASSERT(cdd_test_openapi_version_supported("3.2.0") != CDD_C_SUCCESS);

  memset(&ex_tmp, 0, sizeof(ex_tmp));
  ASSERT(cdd_test_example_fields_valid(NULL) != CDD_C_SUCCESS);
  ex_tmp.data_value_set = 1;
  ex_tmp.value_set = 1;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_example_fields_valid(&ex_tmp));
  memset(&ex_tmp, 0, sizeof(ex_tmp));
  CDD_STRCPY(s_name, sizeof(s_name), "foo");
  CDD_STRCPY(s_parent, sizeof(s_parent), "bar");
  ex_tmp.serialized_value = s_name;
  ex_tmp.external_value = s_parent;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_example_fields_valid(&ex_tmp));
  memset(&ex_tmp, 0, sizeof(ex_tmp));
  ex_tmp.value_set = 1;
  ex_tmp.serialized_value = s_name;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_example_fields_valid(&ex_tmp));
  memset(&ex_tmp, 0, sizeof(ex_tmp));
  ASSERT(cdd_test_example_fields_valid(&ex_tmp) != CDD_C_SUCCESS);

  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_object_has_example_and_examples(NULL));
  val_tmp = json_parse_string("{\"example\":\"a\",\"examples\":{}}");
  if (val_tmp) {
    obj_tmp = json_value_get_object(val_tmp);
    ASSERT(cdd_test_object_has_example_and_examples(obj_tmp) != CDD_C_SUCCESS);
    json_value_free(val_tmp);
  }
  val_tmp = json_parse_string("{\"example\":\"a\"}");
  if (val_tmp) {
    obj_tmp = json_value_get_object(val_tmp);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_test_object_has_example_and_examples(obj_tmp));
    json_value_free(val_tmp);
  }

  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_uri_has_scheme_prefix(NULL, 0));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_uri_has_scheme_prefix("http://example.com", 0));
  ASSERT(cdd_test_uri_has_scheme_prefix("http://example.com", 18) !=
         CDD_C_SUCCESS);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_uri_has_scheme_prefix("/api/users", 10));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_uri_has_scheme_prefix("?query=1", 8));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_uri_has_scheme_prefix("#fragment", 9));

  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_root_is_schema_document(NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_root_has_openapi_fields(NULL));
  val_tmp = json_value_init_boolean(1);
  ASSERT(cdd_test_root_is_schema_document(val_tmp, NULL) != CDD_C_SUCCESS);
  json_value_free(val_tmp);

  val_tmp = json_parse_string("{\"type\":\"string\"}");
  if (val_tmp) {
    obj_tmp = json_value_get_object(val_tmp);
    ASSERT(cdd_test_root_is_schema_document(val_tmp, obj_tmp) != CDD_C_SUCCESS);
    json_value_free(val_tmp);
  }
  val_tmp = json_parse_string("{\"openapi\":\"3.0.0\"}");
  if (val_tmp) {
    obj_tmp = json_value_get_object(val_tmp);
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_root_is_schema_document(val_tmp, obj_tmp));
    json_value_free(val_tmp);
  }
  val_tmp = json_parse_string("{\"swagger\":\"2.0\"}");
  if (val_tmp) {
    obj_tmp = json_value_get_object(val_tmp);
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_root_is_schema_document(val_tmp, obj_tmp));
    json_value_free(val_tmp);
  }
  val_tmp = json_parse_string("{\"info\":{}}");
  if (val_tmp) {
    obj_tmp = json_value_get_object(val_tmp);
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_root_is_schema_document(val_tmp, obj_tmp));
    json_value_free(val_tmp);
  }

  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_is_valid_response_code_key(NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_is_valid_response_code_key(""));
  ASSERT(cdd_test_is_valid_response_code_key("default") != CDD_C_SUCCESS);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_is_valid_response_code_key("2000"));
  ASSERT(cdd_test_is_valid_response_code_key("2XX") != CDD_C_SUCCESS);
  ASSERT(cdd_test_is_valid_response_code_key("5XX") != CDD_C_SUCCESS);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_is_valid_response_code_key("6XX"));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_is_valid_response_code_key("0XX"));
  ASSERT(cdd_test_is_valid_response_code_key("200") != CDD_C_SUCCESS);
  ASSERT(cdd_test_is_valid_response_code_key("404") != CDD_C_SUCCESS);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_is_valid_response_code_key("20A"));

  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_media_type_base_len(NULL, &blen));
  ASSERT_EQ(0, blen);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_media_type_base_len("text/plain; charset=utf-8", &blen));
  ASSERT_EQ(10, blen);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_media_type_base_equal(NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_media_type_base_equal("text/plain", "application/json"));
  ASSERT(cdd_test_media_type_base_equal("text/plain; a=1", "text/plain; b=2") !=
         CDD_C_SUCCESS);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_media_type_is_json(NULL));
  ASSERT(cdd_test_media_type_is_json("application/json") != CDD_C_SUCCESS);
  ASSERT(cdd_test_media_type_is_json("application/vnd.api+json") !=
         CDD_C_SUCCESS);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_media_type_is_json("text/plain"));

  memset(p_arr, 0, sizeof(p_arr));
  CDD_STRCPY(s_pname, sizeof(s_pname), "id");
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_param_key_equals(NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_param_key_equals(&p_arr[0], NULL));
  p_arr[0].name = s_pname;
  p_arr[0].in = OA_PARAM_IN_QUERY;
  p_arr[1].name = s_pname;
  p_arr[1].in = OA_PARAM_IN_HEADER;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_param_key_equals(&p_arr[0], &p_arr[1]));
  p_arr[1].in = OA_PARAM_IN_QUERY;
  ASSERT(cdd_test_param_key_equals(&p_arr[0], &p_arr[1]) != CDD_C_SUCCESS);
  CDD_STRCPY(s_name, sizeof(s_name), "other");
  p_arr[1].name = s_name;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_param_key_equals(&p_arr[0], &p_arr[1]));

  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_schema_has_composition(NULL));
  val_tmp = json_parse_string("{\"allOf\":[]}");
  if (val_tmp) {
    obj_tmp = json_value_get_object(val_tmp);
    ASSERT(cdd_test_schema_has_composition(obj_tmp) != CDD_C_SUCCESS);
    json_value_free(val_tmp);
  }
  val_tmp = json_parse_string("{\"anyOf\":[]}");
  if (val_tmp) {
    obj_tmp = json_value_get_object(val_tmp);
    ASSERT(cdd_test_schema_has_composition(obj_tmp) != CDD_C_SUCCESS);
    json_value_free(val_tmp);
  }
  val_tmp = json_parse_string("{\"oneOf\":[]}");
  if (val_tmp) {
    obj_tmp = json_value_get_object(val_tmp);
    ASSERT(cdd_test_schema_has_composition(obj_tmp) != CDD_C_SUCCESS);
    json_value_free(val_tmp);
  }
  val_tmp = json_parse_string("{\"type\":\"string\"}");
  if (val_tmp) {
    obj_tmp = json_value_get_object(val_tmp);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_test_schema_has_composition(obj_tmp));
    json_value_free(val_tmp);
  }

  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_is_fixed_operation_method(NULL));
  ASSERT(cdd_test_is_fixed_operation_method("get") != CDD_C_SUCCESS);
  ASSERT(cdd_test_is_fixed_operation_method("post") != CDD_C_SUCCESS);
  ASSERT(cdd_test_is_fixed_operation_method("put") != CDD_C_SUCCESS);
  ASSERT(cdd_test_is_fixed_operation_method("delete") != CDD_C_SUCCESS);
  ASSERT(cdd_test_is_fixed_operation_method("patch") != CDD_C_SUCCESS);
  ASSERT(cdd_test_is_fixed_operation_method("head") != CDD_C_SUCCESS);
  ASSERT(cdd_test_is_fixed_operation_method("options") != CDD_C_SUCCESS);
  ASSERT(cdd_test_is_fixed_operation_method("trace") != CDD_C_SUCCESS);
  ASSERT(cdd_test_is_fixed_operation_method("query") != CDD_C_SUCCESS);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_is_fixed_operation_method("custom_method"));

  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_scan_querystring_usage(NULL, 0, &qs_count, &has_query));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_validate_querystring_usage(NULL, 0));
  memset(p_arr, 0, sizeof(p_arr));
  p_arr[0].in = OA_PARAM_IN_QUERYSTRING;
  p_arr[1].in = OA_PARAM_IN_QUERY;
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_scan_querystring_usage(p_arr, 2, &qs_count, &has_query));
  ASSERT_EQ(1, qs_count);
  ASSERT_EQ(1, has_query);
  memset(path_arr, 0, sizeof(path_arr));
  path_arr[0].route = s_name;
  path_arr[0].parameters = p_arr;
  path_arr[0].n_parameters = 2;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_validate_querystring_usage(path_arr, 1));
  p_arr[1].in = OA_PARAM_IN_QUERYSTRING;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_validate_querystring_usage(path_arr, 1));

  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_validate_media_type_key_map(NULL));
  val_tmp = json_parse_string("{\"application/json\":{}}");
  if (val_tmp) {
    obj_tmp = json_value_get_object(val_tmp);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_test_validate_media_type_key_map(obj_tmp));
    json_value_free(val_tmp);
  }
  val_tmp = json_parse_string("{\"bad@key\":{}}");
  if (val_tmp) {
    obj_tmp = json_value_get_object(val_tmp);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_test_validate_media_type_key_map(obj_tmp));
    json_value_free(val_tmp);
  }

  /* 5. find_path_param */
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_find_path_param(p_arr, 2, "nonexistent", &found_p));
  ASSERT_EQ(NULL, found_p);

  /* 6. normalize_path_template_route error & collect_path_template_names */
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_normalize_path_template_route("{unclosed", &norm_route));
  ASSERT_EQ(NULL, norm_route);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_normalize_path_template_route("/users/{id}", &norm_route));
  if (norm_route) {
    free(norm_route);
    norm_route = NULL;
  }

  /* 7. free helpers edge cases */
  cdd_test_free_parameter(NULL);
  cdd_test_free_header(NULL);

  /* 2. Tag cycles and parent validation */
  {
    struct OpenAPI_Spec spec_test;
    struct OpenAPI_Tag tags_arr[2];
    size_t tag_idx = 0;
    int tag_states[2];
    memset(&spec_test, 0, sizeof(spec_test));
    memset(tags_arr, 0, sizeof(tags_arr));
    CDD_STRCPY(s_name, sizeof(s_name), "tagA");
    CDD_STRCPY(s_parent, sizeof(s_parent), "tagB");
    tags_arr[0].name = s_name;
    tags_arr[0].parent = s_parent;
    tags_arr[1].name = s_parent;
    tags_arr[1].parent = s_name;
    spec_test.tags = tags_arr;
    spec_test.n_tags = 2;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_tag_index_by_name(&spec_test, "tagA", &tag_idx));
    ASSERT_EQ(0, tag_idx);
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_tag_index_by_name(&spec_test, "tagB", &tag_idx));
    ASSERT_EQ(1, tag_idx);
    ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
              cdd_test_tag_index_by_name(&spec_test, "missing", &tag_idx));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_test_tag_index_by_name(NULL, "tagA", &tag_idx));
    tag_states[0] = 0;
    tag_states[1] = 0;
    ASSERT(cdd_test_detect_tag_cycle(&spec_test, 0, tag_states) !=
           CDD_C_SUCCESS);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_test_validate_tag_parents(&spec_test));
    CDD_STRCPY(s_parent, sizeof(s_parent), "nonexistent");
    tags_arr[0].parent = s_parent;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_test_validate_tag_parents(&spec_test));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_test_validate_tag_parents(NULL));
  }

  /* 3. Server variables defined & seen */
  {
    struct OpenAPI_Server srv_meta;
    struct OpenAPI_ServerVariable svar;
    char *tag_names[2];
    memset(&srv_meta, 0, sizeof(srv_meta));
    memset(&svar, 0, sizeof(svar));
    CDD_STRCPY(s_name, sizeof(s_name), "port");
    svar.name = s_name;
    srv_meta.variables = &svar;
    srv_meta.n_variables = 1;
    ASSERT(cdd_test_server_variable_defined(&srv_meta, "port") !=
           CDD_C_SUCCESS);
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_server_variable_defined(&srv_meta, "missing"));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_test_server_variable_defined(NULL, "port"));
    tag_names[0] = s_name;
    tag_names[1] = s_parent;
    ASSERT(cdd_test_server_variable_seen(tag_names, 2, "port") !=
           CDD_C_SUCCESS);
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_server_variable_seen(tag_names, 2, "host"));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_test_server_variable_seen(NULL, 0, "env"));
  }

  /* 4. Schema name in use */
  {
    struct OpenAPI_Spec spec_test;
    char s_def[16];
    char s_raw[16];
    char *def_names[1];
    char *raw_names[1];
    memset(&spec_test, 0, sizeof(spec_test));
    CDD_STRCPY(s_def, sizeof(s_def), "MySchema");
    CDD_STRCPY(s_raw, sizeof(s_raw), "RawSchema");
    def_names[0] = s_def;
    raw_names[0] = s_raw;
    spec_test.defined_schema_names = def_names;
    spec_test.n_defined_schemas = 1;
    spec_test.raw_schema_names = raw_names;
    spec_test.n_raw_schemas = 1;
    ASSERT(cdd_test_schema_name_in_use(&spec_test, "MySchema") !=
           CDD_C_SUCCESS);
    ASSERT(cdd_test_schema_name_in_use(&spec_test, "RawSchema") !=
           CDD_C_SUCCESS);
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_schema_name_in_use(&spec_test, "OtherSchema"));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_test_schema_name_in_use(NULL, "MySchema"));
  }

  /* 5. json_pointer_unescape with ~0 and ~1 */
  {
    char *unesc = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_test_json_pointer_unescape(NULL, &unesc));
    ASSERT_EQ(NULL, unesc);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_test_json_pointer_unescape("a~0b~1c", &unesc));
    if (unesc) {
      free(unesc);
      unesc = NULL;
    }
  }

  /* 6. parse_schema_type with null */
  {
    JSON_Value *jv_stype = json_parse_string("{\"type\":[\"null\"]}");
    if (jv_stype) {
      char *stype_str = NULL;
      int s_null = 0;
      cdd_test_parse_schema_type(json_value_get_object(jv_stype), &s_null,
                                 &stype_str);
      ASSERT_EQ(1, s_null);
      json_value_free(jv_stype);
    }
  }

  /* 7. ref_name_from_prefix branches */
  {
    char *rn = NULL;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_ref_name_from_prefix(NULL, NULL, NULL, &rn));
    ASSERT_EQ(NULL, rn);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_test_ref_name_from_prefix(
                                 NULL, "no_prefix_match", "#/prefix/", &rn));
    ASSERT_EQ(NULL, rn);
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_ref_name_from_prefix(NULL, "http://other.com/ref",
                                            "#/prefix/", &rn));
    ASSERT_EQ(NULL, rn);
  }

  /* 8. validate_parameter_style */
  {
    struct OpenAPI_Parameter p_style_test;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_test_validate_parameter_style(NULL, 0));
    memset(&p_style_test, 0, sizeof(p_style_test));
    p_style_test.in = OA_PARAM_IN_QUERY;
    p_style_test.style = OA_STYLE_FORM;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_validate_parameter_style(&p_style_test, 0));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_validate_parameter_style(&p_style_test, 1));
    p_style_test.in = OA_PARAM_IN_QUERYSTRING;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_validate_parameter_style(&p_style_test, 0));
    p_style_test.in = OA_PARAM_IN_QUERY;
    p_style_test.style = OA_STYLE_UNKNOWN;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_test_validate_parameter_style(&p_style_test, 0));
    p_style_test.style = (enum OpenAPI_Style)999;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_test_validate_parameter_style(&p_style_test, 0));
  }

  /* 9. Swagger 2.0 host, basePath, schemes, consumes, produces */
  {
    struct OpenAPI_Spec sw_spec;
    memset(&sw_spec, 0, sizeof(sw_spec));
    ASSERT_EQ(
        CDD_C_SUCCESS,
        load_spec_str(
            "{\"swagger\":\"2.0\",\"info\":{\"title\":\"T\",\"version\":\"1\"},"
            "\"host\":\"api.com\",\"basePath\":\"/v1\","
            "\"schemes\":[\"https\"],\"consumes\":[\"application/"
            "json\"],\"produces\":[\"application/json\"],"
            "\"paths\":{}}",
            &sw_spec));
    openapi_spec_free(&sw_spec);

    for (k = 1; k <= 10; ++k) {
      memset(&sw_spec, 0, sizeof(sw_spec));
      g_cdd_alloc_fail = k;
      load_spec_str(
          "{\"swagger\":\"2.0\",\"info\":{\"title\":\"T\",\"version\":\"1\"},"
          "\"host\":\"api.com\",\"basePath\":\"/v1\","
          "\"schemes\":[\"https\"],\"consumes\":[\"application/"
          "json\"],\"produces\":[\"application/json\"],"
          "\"paths\":{}}",
          &sw_spec);
      g_cdd_alloc_fail = 0;
      openapi_spec_free(&sw_spec);
    }
  }

  /* 10. Querystring validation failure in webhooks, component path items,
   * component callbacks, and duplicate opId */
  {
    struct OpenAPI_Spec s_err;
    memset(&s_err, 0, sizeof(s_err));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              load_spec_str(
                  "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"T\","
                  "\"version\":\"1\"},"
                  "\"webhooks\":{\"/"
                  "wh\":{\"get\":{\"parameters\":[{\"name\":\"q1\",\"in\":"
                  "\"querystring\"},{\"name\":\"q2\",\"in\":\"querystring\"}],"
                  "\"responses\":{\"200\":{\"description\":\"ok\"}}}}}}",
                  &s_err));
    memset(&s_err, 0, sizeof(s_err));
    ASSERT_EQ(
        CDD_C_ERROR_INVALID_ARGUMENT,
        load_spec_str(
            "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"T\",\"version\":"
            "\"1\"},"
            "\"components\":{\"pathItems\":{\"pi\":{\"route\":\"/"
            "pi\",\"parameters\":[{\"name\":\"q1\",\"in\":\"querystring\"},{"
            "\"name\":\"q2\",\"in\":\"querystring\"}]}}}}",
            &s_err));
    memset(&s_err, 0, sizeof(s_err));
    ASSERT_EQ(
        CDD_C_ERROR_INVALID_ARGUMENT,
        load_spec_str(
            "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"T\",\"version\":"
            "\"1\"},"
            "\"components\":{\"callbacks\":{\"cb\":{\"{$url}\":{\"route\":\"/"
            "cb\",\"get\":{\"parameters\":[{\"name\":\"q1\",\"in\":"
            "\"querystring\"},{\"name\":\"q2\",\"in\":\"querystring\"}],"
            "\"responses\":{\"200\":{\"description\":\"ok\"}}}}}}}}",
            &s_err));
    memset(&s_err, 0, sizeof(s_err));
    ASSERT_EQ(
        CDD_C_ERROR_INVALID_ARGUMENT,
        load_spec_str("{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"T\","
                      "\"version\":\"1\"},"
                      "\"paths\":{\"/"
                      "a\":{\"get\":{\"operationId\":\"dupOp\",\"responses\":{"
                      "\"200\":{\"description\":\"ok\"}}}},"
                      "\"/"
                      "b\":{\"get\":{\"operationId\":\"dupOp\",\"responses\":{"
                      "\"200\":{\"description\":\"ok\"}}}}}}",
                      &s_err));
  }

  /* 11. parse_operation with invalid verb */
  {
    struct OpenAPI_Operation op_dst;
    val_tmp =
        json_parse_string("{\"responses\":{\"200\":{\"description\":\"ok\"}}}");
    if (val_tmp) {
      obj_tmp = json_value_get_object(val_tmp);
      memset(&op_dst, 0, sizeof(op_dst));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_test_parse_operation("invalid_verb", obj_tmp,
                                                        &op_dst, NULL, 0));
      ASSERT_EQ(OA_VERB_UNKNOWN, op_dst.verb);
      cdd_test_free_operation(&op_dst);
      json_value_free(val_tmp);
    }
  }

  /* 12. openapi_load_from_json invalid roots & specs */
  {
    struct OpenAPI_Spec s_inv;
    memset(&s_inv, 0, sizeof(s_inv));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              load_spec_str("\"not_an_object\"", &s_inv));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, load_spec_str("[1, 2, 3]", &s_inv));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              load_spec_str("{\"paths\":{}}", &s_inv));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              load_spec_str("{\"openapi\":\"4.0.0\"}", &s_inv));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              load_spec_str(
                  "{\"openapi\":\"3.0.0\",\"tags\":[{\"name\":\"t1\","
                  "\"parent\":\"t2\"},{\"name\":\"t2\",\"parent\":\"t1\"}]}",
                  &s_inv));
    ASSERT_EQ(
        CDD_C_ERROR_INVALID_ARGUMENT,
        load_spec_str("{\"openapi\":\"3.0.0\",\"info\":{\"title\":\"T\","
                      "\"version\":\"1\"},\"paths\":{\"no_leading_slash\":{}}}",
                      &s_inv));
  }

  /* 13. ref_base_matches_self comprehensive tests */
  {
    struct OpenAPI_Spec s_self;
    char doc_u[64];
    char self_u[64];
    char ref_buf[64];
    char *h_ptr;
    memset(&s_self, 0, sizeof(s_self));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_test_ref_base_matches_self(NULL, NULL, NULL));
    CDD_STRCPY(ref_buf, sizeof(ref_buf), "a");
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
              cdd_test_ref_base_matches_self(&s_self, ref_buf, ref_buf));

    CDD_STRCPY(doc_u, sizeof(doc_u), "https://example.com/spec.json");
    s_self.document_uri = doc_u;
    CDD_STRCPY(ref_buf, sizeof(ref_buf),
               "https://example.com/spec.json#anchor");
    h_ptr = strchr(ref_buf, '#');
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
              cdd_test_ref_base_matches_self(&s_self, ref_buf, h_ptr));

    CDD_STRCPY(ref_buf, sizeof(ref_buf), "https://other.com/spec.json#anchor");
    h_ptr = strchr(ref_buf, '#');
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_ref_base_matches_self(&s_self, ref_buf, h_ptr));

    CDD_STRCPY(doc_u, sizeof(doc_u), "./rel/spec.json");
    CDD_STRCPY(ref_buf, sizeof(ref_buf), "/rel/spec.json#anchor");
    h_ptr = strchr(ref_buf, '#');
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
              cdd_test_ref_base_matches_self(&s_self, ref_buf, h_ptr));

    CDD_STRCPY(ref_buf, sizeof(ref_buf), "rel/spec.json#anchor");
    h_ptr = strchr(ref_buf, '#');
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
              cdd_test_ref_base_matches_self(&s_self, ref_buf, h_ptr));

    CDD_STRCPY(ref_buf, sizeof(ref_buf), "foo/rel/spec.json#anchor");
    h_ptr = strchr(ref_buf, '#');
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
              cdd_test_ref_base_matches_self(&s_self, ref_buf, h_ptr));

    CDD_STRCPY(doc_u, sizeof(doc_u), "./");
    CDD_STRCPY(ref_buf, sizeof(ref_buf), "spec.json#anchor");
    h_ptr = strchr(ref_buf, '#');
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_ref_base_matches_self(&s_self, ref_buf, h_ptr));

    s_self.document_uri = NULL;
    CDD_STRCPY(self_u, sizeof(self_u), "https://example.com/spec.json");
    s_self.self_uri = self_u;
    CDD_STRCPY(ref_buf, sizeof(ref_buf),
               "https://example.com/spec.json#anchor");
    h_ptr = strchr(ref_buf, '#');
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
              cdd_test_ref_base_matches_self(&s_self, ref_buf, h_ptr));

    CDD_STRCPY(ref_buf, sizeof(ref_buf), "https://other.com/spec.json#anchor");
    h_ptr = strchr(ref_buf, '#');
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_ref_base_matches_self(&s_self, ref_buf, h_ptr));

    CDD_STRCPY(self_u, sizeof(self_u), "./rel/spec.json");
    CDD_STRCPY(ref_buf, sizeof(ref_buf), "/rel/spec.json#anchor");
    h_ptr = strchr(ref_buf, '#');
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
              cdd_test_ref_base_matches_self(&s_self, ref_buf, h_ptr));
  }

  /* 14. collect_schema_extras and collect_extensions */
  {
    char *ext_json = NULL;
    val_tmp = json_parse_string("{\"x-custom\":\"val\",\"custom_extra\":123}");
    if (val_tmp) {
      obj_tmp = json_value_get_object(val_tmp);
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_test_collect_schema_extras(NULL, NULL, 0, &ext_json));
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_test_collect_schema_extras(obj_tmp, NULL, 0, &ext_json));
      if (ext_json) {
        free(ext_json);
        ext_json = NULL;
      }
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                cdd_test_collect_extensions(NULL, &ext_json));
      ASSERT_EQ(CDD_C_SUCCESS, cdd_test_collect_extensions(obj_tmp, &ext_json));
      if (ext_json) {
        free(ext_json);
        ext_json = NULL;
      }
      for (k = 1; k <= 10; ++k) {
        g_cdd_alloc_fail = k;
        cdd_test_collect_schema_extras(obj_tmp, NULL, 0, &ext_json);
        g_cdd_alloc_fail = 0;
        if (ext_json) {
          free(ext_json);
          ext_json = NULL;
        }
        g_cdd_alloc_fail = k;
        cdd_test_collect_extensions(obj_tmp, &ext_json);
        g_cdd_alloc_fail = 0;
        if (ext_json) {
          free(ext_json);
          ext_json = NULL;
        }
      }
      json_value_free(val_tmp);
    }
  }

  /* 15. URI reference and path normalization */
  {
    char *u_res = NULL;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_resolve_uri_reference(NULL, NULL, &u_res));
    ASSERT_EQ(NULL, u_res);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_test_resolve_uri_reference(NULL, "", &u_res));
    if (u_res) {
      free(u_res);
      u_res = NULL;
    }
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_resolve_uri_reference("http://base.com",
                                             "http://abs.com/path", &u_res));
    if (u_res) {
      free(u_res);
      u_res = NULL;
    }
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_resolve_uri_reference("urn:isbn:123", "/book", &u_res));
    if (u_res) {
      free(u_res);
      u_res = NULL;
    }
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_resolve_uri_reference("rel_base", "/root_path", &u_res));
    if (u_res) {
      free(u_res);
      u_res = NULL;
    }
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_resolve_uri_reference("/foo", "bar", &u_res));
    if (u_res) {
      free(u_res);
      u_res = NULL;
    }

    ASSERT_EQ(CDD_C_SUCCESS, cdd_test_normalize_path("../../a/b/../c", &u_res));
    if (u_res) {
      free(u_res);
      u_res = NULL;
    }
    ASSERT_EQ(CDD_C_SUCCESS, cdd_test_normalize_path(".", &u_res));
    if (u_res) {
      free(u_res);
      u_res = NULL;
    }
  }

  /* 16. parse_info OOM loops */
  {
    JSON_Value *jv_info = json_parse_string(
        "{\"title\":\"T\",\"version\":\"1\",\"summary\":\"S\",\"description\":"
        "\"D\","
        "\"termsOfService\":\"http://"
        "tos\",\"contact\":{\"name\":\"cn\",\"url\":\"http://"
        "cu\",\"email\":\"ce\"},"
        "\"license\":{\"name\":\"ln\",\"url\":\"http://lu\"}}");
    if (jv_info) {
      JSON_Object *jio = json_value_get_object(jv_info);
      struct OpenAPI_Spec info_spec;
      for (k = 1; k <= 15; ++k) {
        memset(&info_spec, 0, sizeof(info_spec));
        g_cdd_alloc_fail = k;
        cdd_test_parse_info(jio, &info_spec);
        g_cdd_alloc_fail = 0;
        openapi_spec_free(&info_spec);
      }
      json_value_free(jv_info);
    }
  }

  g_fail_io_after = -1;
  g_cdd_alloc_fail = 0;
  g_cdd_strdup_fail = 0;
  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_OPENAPI_LOADER_COVERAGE_H */
