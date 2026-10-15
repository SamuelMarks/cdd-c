/**
 * @file test_openapi_writer_extensions.h
 * @brief Unit tests for OpenAPI Writer (extensions).
 */

#ifndef TEST_OPENAPI_WRITER_EXTENSIONS_H
#define TEST_OPENAPI_WRITER_EXTENSIONS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "emit/test_openapi_writer_common.h"
/* clang-format on */

TEST test_writer_paths_webhooks_components_extensions(void) {
  int rc = 0;
  struct OpenAPI_Spec spec;
  char *json;
  struct OpenAPI_Path path = {0};
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Response resp = {0};
  json = NULL;

  setup_test_spec(&spec, &path, &op, NULL, &resp);
  spec.info.title = cdd_test_mut_str("Spec");
  spec.info.version = cdd_test_mut_str("1");
  resp.description = cdd_test_mut_str("ok");

  spec.paths_extensions_json = cdd_test_mut_str("{\"x-paths\":true}");
  spec.webhooks_extensions_json = cdd_test_mut_str("{\"x-hooks\":1}");
  spec.components_extensions_json =
      cdd_test_mut_str("{\"x-comps\":{\"meta\":\"yes\"}}");
  spec.webhooks = NULL;
  spec.n_webhooks = 0;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);
  ASSERT(json != NULL);

  {
    JSON_Value *root;
    JSON_Object *root_obj;
    JSON_Object *paths_obj;
    JSON_Object *hooks_obj;
    JSON_Object *comps_obj;
    JSON_Object *path_item;
    root = json_parse_string(json);
    root_obj = json_value_get_object(root);
    paths_obj = json_object_get_object(root_obj, "paths");
    hooks_obj = json_object_get_object(root_obj, "webhooks");
    comps_obj = json_object_get_object(root_obj, "components");
    path_item = json_object_get_object(paths_obj, "/test/route");

    ASSERT_EQ(1, json_object_get_boolean(paths_obj, "x-paths"));
    ASSERT(path_item != NULL);

    ASSERT_EQ(1, (int)json_object_get_number(hooks_obj, "x-hooks"));
    ASSERT_STR_EQ("yes",
                  json_object_get_string(
                      json_object_get_object(comps_obj, "x-comps"), "meta"));

    json_value_free(root);
  }

  free(json);
  g_fail_io_after = -1;
  PASS();
}

TEST test_writer_methods_and_styles(void) {
  int rc = 0;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Path path;
  char *json;
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Parameter param = {0};
  json = NULL;

  setup_test_spec(&spec, &path, &op, &param, NULL);

  /* Test all methods */
  path.n_operations = 7;
  path.operations = calloc(7, sizeof(struct OpenAPI_Operation));
  path.operations[0].verb = OA_VERB_PUT;
  path.operations[1].verb = OA_VERB_DELETE;
  path.operations[2].verb = OA_VERB_PATCH;
  path.operations[3].verb = OA_VERB_HEAD;
  path.operations[4].verb = OA_VERB_OPTIONS;
  path.operations[5].verb = OA_VERB_TRACE;
  path.operations[6].verb = OA_VERB_QUERY;

  /* Test parameter styles */
  path.operations[0].n_parameters = 6;
  path.operations[0].parameters = calloc(6, sizeof(struct OpenAPI_Parameter));
  path.operations[0].parameters[0].name = cdd_test_mut_str("p1");
  path.operations[0].parameters[0].in = OA_PARAM_IN_COOKIE;
  path.operations[0].parameters[0].style = OA_STYLE_COOKIE;

  path.operations[0].parameters[1].name = cdd_test_mut_str("p2");
  path.operations[0].parameters[1].in = OA_PARAM_IN_QUERY;
  path.operations[0].parameters[1].style = OA_STYLE_LABEL;

  path.operations[0].parameters[2].name = cdd_test_mut_str("p3");
  path.operations[0].parameters[2].in = OA_PARAM_IN_QUERY;
  path.operations[0].parameters[2].style = OA_STYLE_SPACE_DELIMITED;

  path.operations[0].parameters[3].name = cdd_test_mut_str("p4");
  path.operations[0].parameters[3].in = OA_PARAM_IN_QUERY;
  path.operations[0].parameters[3].style = OA_STYLE_PIPE_DELIMITED;

  path.operations[0].parameters[4].name = cdd_test_mut_str("p5");
  path.operations[0].parameters[4].in = OA_PARAM_IN_QUERY;
  path.operations[0].parameters[4].style = OA_STYLE_DEEP_OBJECT;

  path.operations[0].parameters[5].name = cdd_test_mut_str("p6");
  path.operations[0].parameters[5].in = OA_PARAM_IN_QUERYSTRING;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);
  ASSERT(json != NULL);
  free(json);

  free(path.operations[0].parameters);
  free(path.operations);
  g_fail_io_after = -1;
  PASS();
}

TEST test_writer_xml_and_oauth(void) {
  int rc = 0;
  struct OpenAPI_Spec spec;
  char *json;
  json = NULL;

  ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
  spec.openapi_version = cdd_test_mut_str("3.2.0");
  spec.info.title = cdd_test_mut_str("test");
  spec.info.version = cdd_test_mut_str("1");

  spec.n_security_schemes = 1;
  spec.security_schemes = calloc(1, sizeof(struct OpenAPI_SecurityScheme));
  spec.security_schemes[0].name = cdd_test_mut_str("oauth2_all");
  spec.security_schemes[0].type = OA_SEC_OAUTH2;

  spec.security_schemes[0].n_flows = 3;
  spec.security_schemes[0].flows = calloc(3, sizeof(struct OpenAPI_OAuthFlow));
  spec.security_schemes[0].flows[0].type = OA_OAUTH_FLOW_CLIENT_CREDENTIALS;
  spec.security_schemes[0].flows[0].token_url = cdd_test_mut_str("https://a.b");
  spec.security_schemes[0].flows[1].type = OA_OAUTH_FLOW_AUTHORIZATION_CODE;
  spec.security_schemes[0].flows[1].token_url = cdd_test_mut_str("https://a.b");
  spec.security_schemes[0].flows[1].authorization_url =
      cdd_test_mut_str("https://a.b");
  spec.security_schemes[0].flows[2].type = OA_OAUTH_FLOW_DEVICE_AUTHORIZATION;
  spec.security_schemes[0].flows[2].device_authorization_url =
      cdd_test_mut_str("https://a.b");

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);
  ASSERT(json != NULL);
  free(json);

  free(spec.security_schemes[0].flows);
  free(spec.security_schemes);
  g_fail_io_after = -1;
  PASS();
}

TEST test_writer_xml_types(void) {
  int rc = 0;
  struct OpenAPI_Spec spec;
  char *json;
  struct OpenAPI_Path path = {0};
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Response resp = {0};
  json = NULL;

  ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
  spec.openapi_version = cdd_test_mut_str("3.2.0");
  spec.info.title = cdd_test_mut_str("test");
  spec.info.version = cdd_test_mut_str("1");

  spec.n_paths = 1;
  spec.paths = &path;
  path.route = cdd_test_mut_str("/xml");
  path.n_operations = 1;
  path.operations = &op;
  op.verb = OA_VERB_GET;
  op.operation_id = cdd_test_mut_str("xmlOp");

  op.n_responses = 1;
  op.responses = &resp;
  resp.code = cdd_test_mut_str("200");
  resp.description = cdd_test_mut_str("OK");
  resp.n_content_media_types = 4;
  resp.content_media_types = calloc(4, sizeof(struct OpenAPI_MediaType));

  resp.content_media_types[0].name = cdd_test_mut_str("application/xml");
  resp.content_media_types[0].schema_set = 1;
  resp.content_media_types[0].schema.xml.node_type_set = 1;
  resp.content_media_types[0].schema.xml.node_type = OA_XML_NODE_ELEMENT;

  resp.content_media_types[1].name = cdd_test_mut_str("text/xml");
  resp.content_media_types[1].schema_set = 1;
  resp.content_media_types[1].schema.xml.node_type_set = 1;
  resp.content_media_types[1].schema.xml.node_type = OA_XML_NODE_TEXT;

  resp.content_media_types[2].name = cdd_test_mut_str("application/cdata");
  resp.content_media_types[2].schema_set = 1;
  resp.content_media_types[2].schema.xml.node_type_set = 1;
  resp.content_media_types[2].schema.xml.node_type = OA_XML_NODE_CDATA;

  resp.content_media_types[3].name = cdd_test_mut_str("application/none");
  resp.content_media_types[3].schema_set = 1;
  resp.content_media_types[3].schema.xml.node_type_set = 1;
  resp.content_media_types[3].schema.xml.node_type = OA_XML_NODE_NONE;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);
  ASSERT(json != NULL);
  free(json);

  free(resp.content_media_types);
  g_fail_io_after = -1;
  PASS();
}

TEST test_openapi_utils(void) {
  struct OpenAPI_Parameter p;
  char *out = NULL;

  /* Hit the default missing branches */
  ASSERT_EQ(0, verb_to_str_openapi((enum OpenAPI_Verb) - 1, &out));
  ASSERT(out == NULL);

  ASSERT_EQ(0, param_in_to_str_openapi((enum OpenAPI_ParamIn) - 1, &out));
  ASSERT(out == NULL);

  ASSERT_EQ(0, style_to_str_openapi((enum OpenAPI_Style) - 1, &out));
  ASSERT(out == NULL);

  ASSERT_EQ(0, xml_node_type_to_str_openapi(OA_XML_NODE_ELEMENT, &out));
  ASSERT_EQ(0, xml_node_type_to_str_openapi(OA_XML_NODE_ATTRIBUTE, &out));
  ASSERT_EQ(0, xml_node_type_to_str_openapi(OA_XML_NODE_TEXT, &out));
  ASSERT_EQ(0, xml_node_type_to_str_openapi(OA_XML_NODE_CDATA, &out));
  ASSERT_EQ(0, xml_node_type_to_str_openapi(OA_XML_NODE_NONE, &out));
  ASSERT_EQ(0,
            xml_node_type_to_str_openapi((enum OpenAPI_XmlNodeType) - 1, &out));

  ASSERT_EQ(0, header_name_is_content_type_openapi(NULL));
  ASSERT_EQ(1, header_name_is_content_type_openapi("Content-Type"));

  memset(&p, 0, sizeof(p));
  ASSERT_EQ(0, param_is_reserved_header_openapi(NULL));
  p.in = OA_PARAM_IN_HEADER;
  p.name = cdd_test_mut_str("Accept");
  ASSERT_EQ(1, param_is_reserved_header_openapi(&p));

  ASSERT_EQ(0, param_is_reserved_header_openapi(NULL));
  p.in = OA_PARAM_IN_HEADER;
  p.name = cdd_test_mut_str("Accept");
  ASSERT_EQ(1, param_is_reserved_header_openapi(&p));
  p.name = cdd_test_mut_str("Content-Type");
  ASSERT_EQ(1, param_is_reserved_header_openapi(&p));
  p.name = cdd_test_mut_str("Authorization");
  ASSERT_EQ(1, param_is_reserved_header_openapi(&p));
  p.name = cdd_test_mut_str("Content-Type");
  ASSERT_EQ(1, param_is_reserved_header_openapi(&p));

  ASSERT_EQ(0, param_is_reserved_header_openapi(NULL));
  p.in = OA_PARAM_IN_HEADER;
  p.name = cdd_test_mut_str("Accept");
  ASSERT_EQ(1, param_is_reserved_header_openapi(&p));
  p.name = cdd_test_mut_str("Content-Type");
  ASSERT_EQ(1, param_is_reserved_header_openapi(&p));
  p.name = cdd_test_mut_str("Authorization");
  ASSERT_EQ(1, param_is_reserved_header_openapi(&p));
  p.name = cdd_test_mut_str("Authorization");
  ASSERT_EQ(1, param_is_reserved_header_openapi(&p));

  ASSERT_EQ(0, param_is_reserved_header_openapi(NULL));
  p.in = OA_PARAM_IN_HEADER;
  p.name = cdd_test_mut_str("Accept");
  ASSERT_EQ(1, param_is_reserved_header_openapi(&p));
  p.name = cdd_test_mut_str("Content-Type");
  ASSERT_EQ(1, param_is_reserved_header_openapi(&p));
  p.name = cdd_test_mut_str("Authorization");
  ASSERT_EQ(1, param_is_reserved_header_openapi(&p));

  /* OAuth Flow string conversion */
  ASSERT_EQ(0, oauth_flow_type_to_str_openapi(OA_OAUTH_FLOW_IMPLICIT, &out));
  ASSERT_STR_EQ("implicit", out);
  ASSERT_EQ(0, oauth_flow_type_to_str_openapi(OA_OAUTH_FLOW_PASSWORD, &out));
  ASSERT_STR_EQ("password", out);
  ASSERT_EQ(0, oauth_flow_type_to_str_openapi(OA_OAUTH_FLOW_CLIENT_CREDENTIALS,
                                              &out));
  ASSERT_STR_EQ("clientCredentials", out);
  ASSERT_EQ(0, oauth_flow_type_to_str_openapi(OA_OAUTH_FLOW_AUTHORIZATION_CODE,
                                              &out));
  ASSERT_STR_EQ("authorizationCode", out);
  ASSERT_EQ(0, oauth_flow_type_to_str_openapi(
                   OA_OAUTH_FLOW_DEVICE_AUTHORIZATION, &out));
  ASSERT_STR_EQ("deviceAuthorization", out);
  ASSERT_EQ(0, oauth_flow_type_to_str_openapi((enum OpenAPI_OAuthFlowType) - 1,
                                              &out));
  ASSERT(out == NULL);

  /* is_schema_primitive_openapi */
  ASSERT_EQ(0, is_schema_primitive_openapi(NULL));
  ASSERT_EQ(1, is_schema_primitive_openapi("string"));
  ASSERT_EQ(1, is_schema_primitive_openapi("integer"));
  ASSERT_EQ(1, is_schema_primitive_openapi("boolean"));
  ASSERT_EQ(1, is_schema_primitive_openapi("number"));
  ASSERT_EQ(1, is_schema_primitive_openapi("object"));
  ASSERT_EQ(1, is_schema_primitive_openapi("null"));
  ASSERT_EQ(0, is_schema_primitive_openapi("custom_type"));

  g_fail_io_after = -1;

  PASS();
}

TEST test_writer_extended_coverage(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Header hdr;
  struct OpenAPI_Link link;
  struct OpenAPI_MediaType mt;
  struct OpenAPI_Encoding enc;
  char *json = NULL;
  int rc = 0;

  memset(&spec, 0, sizeof(spec));
  memset(&hdr, 0, sizeof(hdr));
  memset(&link, 0, sizeof(link));
  memset(&mt, 0, sizeof(mt));
  memset(&enc, 0, sizeof(enc));

  /* Component Header with content_ref, required, deprecated, style, explode */
  hdr.name = cdd_test_mut_str("X-Custom-Hdr");
  hdr.description = cdd_test_mut_str("Custom header description");
  hdr.required = 1;
  hdr.deprecated_set = 1;
  hdr.deprecated = 1;
  hdr.style_set = 1;
  hdr.style = OA_STYLE_SIMPLE;
  hdr.explode_set = 1;
  hdr.explode = 0;
  hdr.content_ref = cdd_test_mut_str("#/components/headers/OtherHdr");

  spec.component_headers = &hdr;
  spec.component_header_names = (char **)(size_t)(size_t)&hdr.name;
  spec.n_component_headers = 1;

  /* Component Link with ref */
  link.name = cdd_test_mut_str("LinkRef");
  link.ref = cdd_test_mut_str("#/components/links/TargetLink");
  link.summary = cdd_test_mut_str("Link summary");
  link.description = cdd_test_mut_str("Link desc");

  spec.component_links = &link;
  spec.n_component_links = 1;

  /* Component Media Types with prefixEncoding and itemEncoding */
  mt.name = cdd_test_mut_str("application/octet-stream");
  enc.name = cdd_test_mut_str("itemEnc");
  enc.content_type = cdd_test_mut_str("text/plain");
  mt.item_encoding = &enc;
  mt.item_encoding_set = 1;
  mt.prefix_encoding = &enc;
  mt.n_prefix_encoding = 1;

  spec.component_media_types = &mt;
  spec.component_media_type_names = (char **)(size_t)(size_t)&mt.name;
  spec.n_component_media_types = 1;

  /* Component Callbacks */
  {
    struct OpenAPI_Callback cb;
    struct OpenAPI_Path cb_path;
    struct OpenAPI_Operation cb_op;
    struct OpenAPI_Response cb_resp;
    memset(&cb, 0, sizeof(cb));
    memset(&cb_path, 0, sizeof(cb_path));
    memset(&cb_op, 0, sizeof(cb_op));
    memset(&cb_resp, 0, sizeof(cb_resp));

    cb.name = cdd_test_mut_str("compCallback");
    cb.paths = &cb_path;
    cb.n_paths = 1;
    cb_path.route = cdd_test_mut_str("{$request.query.queryUrl}");
    cb_path.operations = &cb_op;
    cb_path.n_operations = 1;
    cb_op.verb = OA_VERB_POST;
    cb_op.operation_id = cdd_test_mut_str("compCbPost");
    cb_op.responses = &cb_resp;
    cb_op.n_responses = 1;
    cb_resp.code = cdd_test_mut_str("200");
    cb_resp.description = cdd_test_mut_str("ok");

    spec.component_callbacks = &cb;
    spec.n_component_callbacks = 1;

    rc = openapi_write_spec_to_json(&spec, &json);
    ASSERT_EQ(0, rc);
    ASSERT(json != NULL);
    free(json);
    json = NULL;
    spec.component_callbacks = NULL;
    spec.n_component_callbacks = 0;
  }

  /* Schema composition (allOf, anyOf, oneOf, not, if, then, else,
   * contentSchema) */
  {
    struct OpenAPI_Path comp_path;
    struct OpenAPI_Operation comp_op;
    struct OpenAPI_Response comp_resp;
    struct OpenAPI_SchemaRef root_ref;
    struct OpenAPI_SchemaRef sub_refs[3];
    struct OpenAPI_SchemaRef not_s;
    struct OpenAPI_SchemaRef if_s;
    struct OpenAPI_SchemaRef then_s;
    struct OpenAPI_SchemaRef else_s;
    struct OpenAPI_SchemaRef content_s;

    memset(&comp_path, 0, sizeof(comp_path));
    memset(&comp_op, 0, sizeof(comp_op));
    memset(&comp_resp, 0, sizeof(comp_resp));
    memset(&root_ref, 0, sizeof(root_ref));
    memset(sub_refs, 0, sizeof(sub_refs));
    memset(&not_s, 0, sizeof(not_s));
    memset(&if_s, 0, sizeof(if_s));
    memset(&then_s, 0, sizeof(then_s));
    memset(&else_s, 0, sizeof(else_s));
    memset(&content_s, 0, sizeof(content_s));

    sub_refs[0].ref_name = cdd_test_mut_str("string");
    sub_refs[1].ref_name = cdd_test_mut_str("integer");
    sub_refs[2].ref = cdd_test_mut_str("#/components/schemas/External");

    not_s.ref_name = cdd_test_mut_str("boolean");
    if_s.ref_name = cdd_test_mut_str("string");
    then_s.ref_name = cdd_test_mut_str("number");
    else_s.ref_name = cdd_test_mut_str("null");
    content_s.ref_name = cdd_test_mut_str("object");

    root_ref.all_of = sub_refs;
    root_ref.n_all_of = 3;
    root_ref.any_of = sub_refs;
    root_ref.n_any_of = 3;
    root_ref.one_of = sub_refs;
    root_ref.n_one_of = 3;
    root_ref.not_schema = &not_s;
    root_ref.if_schema = &if_s;
    root_ref.then_schema = &then_s;
    root_ref.else_schema = &else_s;
    root_ref.content_schema = &content_s;
    root_ref.content_media_type = cdd_test_mut_str("application/json");
    root_ref.content_encoding = cdd_test_mut_str("base64");

    comp_path.route = cdd_test_mut_str("/composition");
    comp_path.operations = &comp_op;
    comp_path.n_operations = 1;
    comp_op.verb = OA_VERB_GET;
    comp_op.operation_id = cdd_test_mut_str("getComposition");
    comp_op.responses = &comp_resp;
    comp_op.n_responses = 1;
    comp_resp.code = cdd_test_mut_str("200");
    comp_resp.description = cdd_test_mut_str("ok");
    comp_resp.schema = root_ref;
    comp_resp.schema_set = 1;

    spec.paths = &comp_path;
    spec.n_paths = 1;

    rc = openapi_write_spec_to_json(&spec, &json);
    ASSERT_EQ(0, rc);
    ASSERT(json != NULL);
    free(json);
    json = NULL;
    spec.paths = NULL;
    spec.n_paths = 0;
  }

  /* Parameter content_ref and content_type variations */
  {
    struct OpenAPI_Path p_path;
    struct OpenAPI_Operation p_op;
    struct OpenAPI_Response p_resp;
    struct OpenAPI_Parameter params[3];

    memset(&p_path, 0, sizeof(p_path));
    memset(&p_op, 0, sizeof(p_op));
    memset(&p_resp, 0, sizeof(p_resp));
    memset(params, 0, sizeof(params));

    /* param 0: content_ref */
    params[0].name = cdd_test_mut_str("p_ref");
    params[0].in = OA_PARAM_IN_HEADER;
    params[0].content_ref =
        cdd_test_mut_str("#/components/headers/CustomHeader");

    /* param 1: content_type with item_schema_set */
    params[1].name = cdd_test_mut_str("p_items");
    params[1].in = OA_PARAM_IN_QUERY;
    params[1].content_type = cdd_test_mut_str("application/json");
    params[1].item_schema_set = 1;
    params[1].type = cdd_test_mut_str("array");
    params[1].is_array = 1;
    params[1].items_type = cdd_test_mut_str("string");
    params[1].example_location = OA_EXAMPLE_LOC_MEDIA;
    params[1].example_set = 1;
    params[1].example.type = OA_ANY_STRING;
    params[1].example.string = cdd_test_mut_str("itemExample");

    /* param 2: OA_PARAM_IN_QUERYSTRING fallback */
    params[2].name = cdd_test_mut_str("p_qs");
    params[2].in = OA_PARAM_IN_QUERYSTRING;
    params[2].type = cdd_test_mut_str("string");

    p_path.route = cdd_test_mut_str("/params_coverage");
    p_path.operations = &p_op;
    p_path.n_operations = 1;
    p_op.verb = OA_VERB_GET;
    p_op.operation_id = cdd_test_mut_str("getParamsCoverage");
    p_op.parameters = params;
    p_op.n_parameters = 3;
    p_op.responses = &p_resp;
    p_op.n_responses = 1;
    p_resp.code = cdd_test_mut_str("200");
    p_resp.description = cdd_test_mut_str("ok");

    spec.paths = &p_path;
    spec.n_paths = 1;

    rc = openapi_write_spec_to_json(&spec, &json);
    ASSERT_EQ(0, rc);
    ASSERT(json != NULL);
    free(json);
    json = NULL;
    spec.paths = NULL;
    spec.n_paths = 0;
  }

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(0, rc);
  ASSERT(json != NULL);
  free(json);

  g_fail_io_after = -1;
  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_OPENAPI_WRITER_EXTENSIONS_H */
