/**
 * @file test_openapi_writer_coverage.h
 * @brief Unit tests for OpenAPI Writer (coverage).
 */

#ifndef TEST_OPENAPI_WRITER_COVERAGE_H
#define TEST_OPENAPI_WRITER_COVERAGE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "emit/test_openapi_writer_common.h"
/* clang-format on */

TEST test_openapi_writer_any_and_examples_coverage(void) {
  JSON_Value *val = NULL;
  JSON_Object *obj;
  JSON_Value *parent_val;
  JSON_Object *parent;
  struct OpenAPI_Any any;
  struct OpenAPI_Example ex;
  struct OpenAPI_Example examples[2];

  parent_val = json_value_init_object();
  parent = json_value_get_object(parent_val);

  /* any_to_json_value */
  memset(&any, 0, sizeof(any));
  any.type = OA_ANY_STRING;
  any.string = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, any_to_json_value(&any, &val));
  json_value_free(val);
  any.string = (char *)(size_t) "hello";
  ASSERT_EQ(CDD_C_SUCCESS, any_to_json_value(&any, &val));
  json_value_free(val);

  any.type = OA_ANY_NUMBER;
  any.number = 42.0;
  ASSERT_EQ(CDD_C_SUCCESS, any_to_json_value(&any, &val));
  json_value_free(val);

  any.type = OA_ANY_BOOL;
  any.boolean = 1;
  ASSERT_EQ(CDD_C_SUCCESS, any_to_json_value(&any, &val));
  json_value_free(val);
  any.boolean = 0;
  ASSERT_EQ(CDD_C_SUCCESS, any_to_json_value(&any, &val));
  json_value_free(val);

  any.type = OA_ANY_NULL;
  ASSERT_EQ(CDD_C_SUCCESS, any_to_json_value(&any, &val));
  json_value_free(val);

  any.type = OA_ANY_JSON;
  any.json = (char *)(size_t) "{\"k\": \"v\"}";
  ASSERT_EQ(CDD_C_SUCCESS, any_to_json_value(&any, &val));
  json_value_free(val);

  any.json = (char *)(size_t) "{invalid";
  ASSERT_EQ(CDD_C_SUCCESS, any_to_json_value(&any, &val));
  json_value_free(val);

  any.json = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, any_to_json_value(&any, &val));
  json_value_free(val);

  any.type = (enum OpenAPI_AnyType)999;
  ASSERT_EQ(CDD_C_SUCCESS, any_to_json_value(&any, &val));
  ASSERT(val == NULL);

  /* write_example_object */
  val = json_value_init_object();
  obj = json_value_get_object(val);

  memset(&ex, 0, sizeof(ex));
  ex.ref = (char *)(size_t) "#/components/examples/ref1";
  ex.summary = (char *)(size_t) "sum";
  ex.description = (char *)(size_t) "desc";
  write_example_object(obj, &ex);

  memset(&ex, 0, sizeof(ex));
  ex.summary = (char *)(size_t) "sum";
  ex.description = (char *)(size_t) "desc";
  ex.data_value_set = 1;
  ex.data_value.type = OA_ANY_STRING;
  ex.data_value.string = (char *)(size_t) "data";
  ex.serialized_value = (char *)(size_t) "ser";
  ex.external_value = (char *)(size_t) "http://example.com";
  ex.extensions_json = (char *)(size_t) "{\"x-ex\": 1}";
  write_example_object(obj, &ex);

  memset(&ex, 0, sizeof(ex));
  ex.value_set = 1;
  ex.value.type = OA_ANY_NUMBER;
  ex.value.number = 123.0;
  write_example_object(obj, &ex);

  json_value_free(val);

  /* write_examples_object */
  memset(examples, 0, sizeof(examples));
  examples[0].name = NULL;
  examples[1].name = (char *)(size_t) "ex2";
  examples[1].summary = (char *)(size_t) "example 2";
  ASSERT_EQ(CDD_C_SUCCESS,
            write_examples_object(parent, "examples", examples, 2));

  /* write_example_fields */
  write_example_fields(parent, NULL, 0, examples, 2);
  memset(&any, 0, sizeof(any));
  any.type = OA_ANY_STRING;
  any.string = (char *)(size_t) "single";
  write_example_fields(parent, &any, 1, NULL, 0);

  json_value_free(parent_val);
  PASS();
}

TEST test_openapi_writer_objects_and_maps_coverage(void) {
  JSON_Value *parent_val;
  JSON_Object *parent;
  JSON_Value *val;
  JSON_Object *obj;
  struct OpenAPI_ExternalDocs docs;
  struct OpenAPI_Discriminator disc;
  struct OpenAPI_DiscriminatorMap map[2];
  struct OpenAPI_Xml xml;
  struct OpenAPI_Server srv;
  struct OpenAPI_ServerVariable vars[2];
  char *var_enums[2];
  struct OpenAPI_Parameter p;
  struct OpenAPI_Header h;
  struct OpenAPI_Encoding enc;
  struct OpenAPI_Encoding enc_arr[2];
  struct OpenAPI_Link link;
  struct OpenAPI_LinkParam lparam[2];
  struct OpenAPI_Response resp;
  struct OpenAPI_RequestBody rb;
  struct OpenAPI_Callback cb;
  struct OpenAPI_Path cb_paths[2];
  struct OpenAPI_MediaType mt;

  parent_val = json_value_init_object();
  parent = json_value_get_object(parent_val);
  val = json_value_init_object();
  obj = json_value_get_object(val);

  /* write_external_docs */
  memset(&docs, 0, sizeof(docs));
  write_external_docs(parent, "ext", &docs);
  docs.url = (char *)(size_t) "https://docs.example.com";
  docs.description = (char *)(size_t) "Documentation";
  docs.extensions_json = (char *)(size_t) "{\"x-doc\": true}";
  write_external_docs(parent, "ext", &docs);

  /* write_discriminator_object */
  memset(&disc, 0, sizeof(disc));
  write_discriminator_object(parent, &disc, 0);
  write_discriminator_object(parent, &disc, 1);
  disc.property_name = (char *)(size_t) "petType";
  disc.default_mapping = (char *)(size_t) "#/components/schemas/Pet";
  memset(map, 0, sizeof(map));
  map[0].value = NULL;
  map[0].schema = NULL;
  map[1].value = (char *)(size_t) "dog";
  map[1].schema = (char *)(size_t) "#/components/schemas/Dog";
  disc.mapping = map;
  disc.n_mapping = 2;
  disc.extensions_json = (char *)(size_t) "{\"x-disc\": 1}";
  write_discriminator_object(parent, &disc, 1);

  /* write_xml_object */
  memset(&xml, 0, sizeof(xml));
  write_xml_object(parent, &xml, 0);
  xml.attribute_set = 1;
  xml.attribute = 1;
  xml.wrapped_set = 1;
  xml.wrapped = 1;
  xml.extensions_json = (char *)(size_t) "{\"x-xml\": 1}";
  write_xml_object(parent, &xml, 1);
  xml.attribute = 0;
  xml.wrapped = 0;
  write_xml_object(parent, &xml, 1);

  /* write_server_object */
  memset(&srv, 0, sizeof(srv));
  srv.url = NULL;
  srv.description = (char *)(size_t) "Main server";
  srv.name = (char *)(size_t) "prod";
  memset(vars, 0, sizeof(vars));
  vars[0].name = NULL;
  vars[1].name = (char *)(size_t) "port";
  vars[1].default_value = (char *)(size_t) "8080";
  vars[1].description = (char *)(size_t) "Port number";
  var_enums[0] = NULL;
  var_enums[1] = (char *)(size_t) "8080";
  vars[1].enum_values = var_enums;
  vars[1].n_enum_values = 2;
  vars[1].extensions_json = (char *)(size_t) "{\"x-var\": 1}";
  srv.variables = vars;
  srv.n_variables = 2;
  srv.extensions_json = (char *)(size_t) "{\"x-srv\": 1}";
  write_server_object(obj, &srv);

  /* write_parameter_object */
  memset(&p, 0, sizeof(p));
  p.ref = (char *)(size_t) "#/components/parameters/ParamRef";
  p.summary = (char *)(size_t) "Param summary";
  p.description = (char *)(size_t) "Param description";
  write_parameter_object(obj, &p);

  memset(&p, 0, sizeof(p));
  p.in = OA_PARAM_IN_QUERY;
  p.allow_empty_value_set = 1;
  p.allow_empty_value = 1;
  p.explode_set = 0;
  p.explode = 1;
  p.allow_reserved_set = 1;
  p.allow_reserved = 1;
  p.content_ref = (char *)(size_t) "#/components/media/Media1";
  p.content_type = NULL;
  write_parameter_object(obj, &p);

  memset(&p, 0, sizeof(p));
  p.in = OA_PARAM_IN_QUERYSTRING;
  p.content_ref = (char *)(size_t) "#/components/media/Media2";
  p.content_type = NULL;
  write_parameter_object(obj, &p);

  memset(&p, 0, sizeof(p));
  p.content_type = (char *)(size_t) "application/custom";
  p.item_schema_set = 1;
  p.schema.ref_name = (char *)(size_t) "ItemModel";
  write_parameter_object(obj, &p);

  memset(&p, 0, sizeof(p));
  p.content_type = (char *)(size_t) "application/custom";
  p.schema_set = 1;
  p.schema.ref_name = (char *)(size_t) "Model";
  write_parameter_object(obj, &p);

  memset(&p, 0, sizeof(p));
  p.content_type = (char *)(size_t) "application/custom";
  p.type = (char *)(size_t) "string";
  p.example_location = OA_EXAMPLE_LOC_MEDIA;
  p.extensions_json = (char *)(size_t) "{\"x-p\": 1}";
  write_parameter_object(obj, &p);

  /* write_header_object */
  memset(&h, 0, sizeof(h));
  h.ref = (char *)(size_t) "#/components/headers/HRef";
  h.description = (char *)(size_t) "H desc";
  write_header_object(obj, &h);

  memset(&h, 0, sizeof(h));
  h.content_ref = (char *)(size_t) "#/components/media/HM1";
  h.content_type = NULL;
  write_header_object(obj, &h);

  memset(&h, 0, sizeof(h));
  h.content_type = (char *)(size_t) "application/json";
  h.schema_set = 1;
  h.schema.ref_name = (char *)(size_t) "HSchema";
  write_header_object(obj, &h);

  memset(&h, 0, sizeof(h));
  h.content_type = (char *)(size_t) "application/json";
  h.type = (char *)(size_t) "string";
  h.example_location = OA_EXAMPLE_LOC_MEDIA;
  h.extensions_json = (char *)(size_t) "{\"x-h\": 1}";
  write_header_object(obj, &h);

  /* write_encoding_object and write_encoding_map */
  memset(&enc, 0, sizeof(enc));
  memset(enc_arr, 0, sizeof(enc_arr));
  enc.prefix_encoding = enc_arr;
  enc.n_prefix_encoding = 1;
  enc.item_encoding = &enc_arr[1];
  enc.item_encoding_set = 1;
  enc.extensions_json = (char *)(size_t) "{\"x-enc\": 1}";
  ASSERT_EQ(CDD_C_SUCCESS, write_encoding_object(obj, &enc));

  memset(enc_arr, 0, sizeof(enc_arr));
  enc_arr[0].name = NULL;
  enc_arr[1].name = (char *)(size_t) "field2";
  ASSERT_EQ(CDD_C_SUCCESS, write_encoding_map(obj, enc_arr, 2));

  /* write_link_object */
  memset(&link, 0, sizeof(link));
  link.ref = (char *)(size_t) "#/components/links/LRef";
  link.summary = (char *)(size_t) "L summary";
  link.description = (char *)(size_t) "L desc";
  write_link_object(obj, &link);

  memset(&link, 0, sizeof(link));
  link.operation_ref = (char *)(size_t) "/paths/~1users/get";
  link.operation_id = (char *)(size_t) "getUser";
  link.description = (char *)(size_t) "User link";
  memset(lparam, 0, sizeof(lparam));
  lparam[0].name = NULL;
  lparam[0].value.type = OA_ANY_STRING;
  lparam[0].value.string = (char *)(size_t) "val1";
  lparam[1].name = (char *)(size_t) "userId";
  lparam[1].value.type = OA_ANY_NUMBER;
  lparam[1].value.number = 100.0;
  link.parameters = lparam;
  link.n_parameters = 2;
  link.request_body_set = 1;
  link.request_body.type = OA_ANY_STRING;
  link.request_body.string = (char *)(size_t) "body";
  link.server_set = 1;
  link.server = &srv;
  link.extensions_json = (char *)(size_t) "{\"x-l\": 1}";
  write_link_object(obj, &link);

  /* write_response_object */
  memset(&resp, 0, sizeof(resp));
  resp.ref = (char *)(size_t) "#/components/responses/RRef";
  resp.summary = (char *)(size_t) "R summary";
  resp.description = (char *)(size_t) "R desc";
  write_response_object(obj, &resp);

  memset(&resp, 0, sizeof(resp));
  resp.content_ref = (char *)(size_t) "#/components/media/RM1";
  resp.content_type = NULL;
  write_response_object(obj, &resp);

  memset(&resp, 0, sizeof(resp));
  resp.content_type = (char *)(size_t) "application/json";
  resp.extensions_json = (char *)(size_t) "{\"x-resp\": 1}";
  write_response_object(obj, &resp);

  /* write_request_body_object */
  memset(&rb, 0, sizeof(rb));
  rb.content_ref = (char *)(size_t) "#/components/media/RBM1";
  rb.schema.content_type = NULL;
  rb.extensions_json = (char *)(size_t) "{\"x-rb\": 1}";
  ASSERT_EQ(CDD_C_SUCCESS, write_request_body_object(obj, &rb));

  /* write_callback_object */
  memset(&cb, 0, sizeof(cb));
  cb.ref = (char *)(size_t) "#/components/callbacks/CBRef";
  cb.summary = (char *)(size_t) "CB summary";
  cb.description = (char *)(size_t) "CB desc";
  write_callback_object(obj, &cb);

  memset(&cb, 0, sizeof(cb));
  memset(cb_paths, 0, sizeof(cb_paths));
  cb_paths[0].route = NULL;
  cb_paths[1].route = (char *)(size_t) "http://example.com/hook";
  cb.paths = cb_paths;
  cb.n_paths = 2;
  cb.extensions_json = (char *)(size_t) "{\"x-cb\": 1}";
  write_callback_object(obj, &cb);

  /* write_media_type_object */
  memset(&mt, 0, sizeof(mt));
  mt.ref = (char *)(size_t) "#/components/mediaTypes/MTRef";
  ASSERT_EQ(CDD_C_SUCCESS, write_media_type_object(obj, &mt));

  memset(&mt, 0, sizeof(mt));
  mt.item_schema_set = 1;
  mt.item_schema.ref_name = (char *)(size_t) "MTItem";
  mt.prefix_encoding = enc_arr;
  mt.n_prefix_encoding = 1;
  mt.item_encoding = &enc_arr[1];
  mt.item_encoding_set = 1;
  mt.extensions_json = (char *)(size_t) "{\"x-mt\": 1}";
  ASSERT_EQ(CDD_C_SUCCESS, write_media_type_object(obj, &mt));

  json_value_free(val);
  json_value_free(parent_val);
  PASS();
}

TEST test_openapi_writer_full_spec_edges(void) {
  struct OpenAPI_Spec spec;
  char *json = NULL;
  cdd_c_error_t rc;
  struct OpenAPI_Path paths[2];
  struct OpenAPI_Operation ops[4];
  struct OpenAPI_Parameter params[3];
  struct OpenAPI_Header hdrs[2];
  struct OpenAPI_Response resps[2];
  struct OpenAPI_Path webhooks[1];
  struct OpenAPI_SecurityScheme sec_schemes[5];
  struct OpenAPI_OAuthFlow flows[1];
  struct OpenAPI_Parameter comp_params[1];
  struct OpenAPI_Response comp_resps[1];
  struct OpenAPI_Header comp_hdrs[1];
  struct OpenAPI_MediaType comp_mts[1];
  struct OpenAPI_Example comp_exs[1];
  struct OpenAPI_Link comp_links[1];
  struct OpenAPI_Callback comp_cbs[1];
  struct OpenAPI_Path comp_paths[2];
  struct OpenAPI_RequestBody comp_rbs[2];
  char *raw_names[3];
  char *raw_jsons[3];
  char *c_pnames[1];
  char *c_rnames[1];
  char *c_hnames[1];
  char *c_mtnames[1];
  char *c_exnames[1];
  char *c_pathnames[2];
  char *c_rbnames[2];

  /* 1. NULL checks on openapi_write_spec_to_json */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            openapi_write_spec_to_json(NULL, &json));
  memset(&spec, 0, sizeof(spec));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            openapi_write_spec_to_json(&spec, NULL));

  /* 2. Schema document mode */
  spec.is_schema_document = 1;
  spec.schema_root_json = NULL;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            openapi_write_spec_to_json(&spec, &json));

  spec.schema_root_json = (char *)(size_t) "{\"type\": \"object\"}";
  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(json != NULL);
  free(json);
  json = NULL;
  spec.is_schema_document = 0;

  /* 3. License with empty name rejected */
  spec.info.license.name = (char *)(size_t) "";
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            openapi_write_spec_to_json(&spec, &json));
  spec.info.license.name = NULL;

  /* 4. Full spec with all optional root and component fields */
  spec.openapi_version = NULL;
  spec.self_uri = (char *)(size_t) "https://api.example.com/openapi.json";
  spec.json_schema_dialect =
      (char *)(size_t) "https://json-schema.org/draft/2020-12/schema";
  spec.extensions_json = (char *)(size_t) "{\"x-root\": 1}";
  spec.external_docs.url = (char *)(size_t) "https://docs.example.com";
  spec.info.license.name = (char *)(size_t) "MIT";
  spec.info.license.url =
      (char *)(size_t) "https://opensource.org/licenses/MIT";

  /* Paths with duplicate routes, unknown verb, empty body, reserved headers,
   * etc. */
  memset(paths, 0, sizeof(paths));
  paths[0].route = (char *)(size_t) "/test";
  paths[1].route = (char *)(size_t) "/test";

  memset(ops, 0, sizeof(ops));
  ops[0].verb = OA_VERB_UNKNOWN;
  ops[1].verb = OA_VERB_GET;
  ops[1].req_body_ref = (char *)(size_t) "#/components/requestBodies/MyRB";
  ops[1].req_body_description = (char *)(size_t) "RB desc";
  ops[1].req_body_extensions_json = (char *)(size_t) "{\"x-rb-ext\": 1}";

  memset(params, 0, sizeof(params));
  params[0].name = (char *)(size_t) "Accept";
  params[0].in = OA_PARAM_IN_HEADER;
  params[1].name = (char *)(size_t) "Content-Type";
  params[1].in = OA_PARAM_IN_HEADER;
  params[2].name = (char *)(size_t) "Authorization";
  params[2].in = OA_PARAM_IN_HEADER;
  ops[1].parameters = params;
  ops[1].n_parameters = 3;

  memset(hdrs, 0, sizeof(hdrs));
  hdrs[0].name = (char *)(size_t) "Content-Type";
  hdrs[1].name = (char *)(size_t) "content-type";
  memset(resps, 0, sizeof(resps));
  resps[0].code = (char *)(size_t) "200";
  resps[0].description = (char *)(size_t) "OK";
  resps[0].headers = hdrs;
  resps[0].n_headers = 2;
  ops[1].responses = resps;
  ops[1].n_responses = 1;

  paths[0].operations = ops;
  paths[0].n_operations = 2;

  memset(&ops[2], 0, sizeof(ops[2]));
  ops[2].verb = OA_VERB_GET;
  ops[2].method = NULL;
  memset(&ops[3], 0, sizeof(ops[3]));
  ops[3].verb = OA_VERB_UNKNOWN;
  ops[3].method = NULL;
  paths[0].additional_operations = &ops[2];
  paths[0].n_additional_operations = 2;

  spec.paths = paths;
  spec.n_paths = 2;

  memset(webhooks, 0, sizeof(webhooks));
  webhooks[0].route = NULL;
  spec.webhooks = webhooks;
  spec.n_webhooks = 1;

  memset(sec_schemes, 0, sizeof(sec_schemes));
  sec_schemes[0].type = OA_SEC_APIKEY;
  sec_schemes[0].in = OA_SEC_IN_COOKIE;
  sec_schemes[0].key_name = (char *)(size_t) "session_id";

  sec_schemes[1].type = OA_SEC_OAUTH2;
  sec_schemes[1].oauth2_metadata_url =
      (char *)(size_t) "https://oauth.example.com/.well-known/oauth";
  memset(flows, 0, sizeof(flows));
  flows[0].type = (enum OpenAPI_OAuthFlowType)99;
  sec_schemes[1].flows = flows;
  sec_schemes[1].n_flows = 1;

  sec_schemes[2].type = OA_SEC_OPENID;
  sec_schemes[2].open_id_connect_url =
      (char *)(size_t) "https://auth.example.com/.well-known/"
                       "openid-configuration";

  sec_schemes[3].type = OA_SEC_UNKNOWN;

  spec.security_schemes = sec_schemes;
  spec.n_security_schemes = 4;

  c_pnames[0] = NULL;
  memset(comp_params, 0, sizeof(comp_params));
  spec.component_parameters = comp_params;
  spec.component_parameter_names = c_pnames;
  spec.n_component_parameters = 1;

  c_rnames[0] = NULL;
  memset(comp_resps, 0, sizeof(comp_resps));
  spec.component_responses = comp_resps;
  spec.component_response_names = c_rnames;
  spec.n_component_responses = 1;

  c_hnames[0] = NULL;
  memset(comp_hdrs, 0, sizeof(comp_hdrs));
  spec.component_headers = comp_hdrs;
  spec.component_header_names = c_hnames;
  spec.n_component_headers = 1;

  c_mtnames[0] = NULL;
  memset(comp_mts, 0, sizeof(comp_mts));
  spec.component_media_types = comp_mts;
  spec.component_media_type_names = c_mtnames;
  spec.n_component_media_types = 1;

  c_exnames[0] = NULL;
  memset(comp_exs, 0, sizeof(comp_exs));
  spec.component_examples = comp_exs;
  spec.component_example_names = c_exnames;
  spec.n_component_examples = 1;

  memset(comp_links, 0, sizeof(comp_links));
  comp_links[0].name = NULL;
  spec.component_links = comp_links;
  spec.n_component_links = 1;

  memset(comp_cbs, 0, sizeof(comp_cbs));
  comp_cbs[0].name = NULL;
  spec.component_callbacks = comp_cbs;
  spec.n_component_callbacks = 1;

  c_pathnames[0] = NULL;
  c_pathnames[1] = NULL;
  memset(comp_paths, 0, sizeof(comp_paths));
  comp_paths[0].route = (char *)(size_t) "/subpath";
  comp_paths[1].route = NULL;
  spec.component_path_items = comp_paths;
  spec.component_path_item_names = c_pathnames;
  spec.n_component_path_items = 2;

  c_rbnames[0] = NULL;
  c_rbnames[1] = (char *)(size_t) "RefBody";
  memset(comp_rbs, 0, sizeof(comp_rbs));
  comp_rbs[1].ref = (char *)(size_t) "#/components/requestBodies/TargetBody";
  spec.component_request_bodies = comp_rbs;
  spec.component_request_body_names = c_rbnames;
  spec.n_component_request_bodies = 2;

  raw_names[0] = NULL;
  raw_jsons[0] = (char *)(size_t) "{}";
  raw_names[1] = (char *)(size_t) "NullJson";
  raw_jsons[1] = NULL;
  raw_names[2] = (char *)(size_t) "ValidSchema";
  raw_jsons[2] = (char *)(size_t) "{\"type\": \"string\"}";
  spec.raw_schema_names = raw_names;
  spec.raw_schema_json = raw_jsons;
  spec.n_raw_schemas = 3;

  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(json != NULL);
  free(json);
  json = NULL;

  raw_names[0] = (char *)(size_t) "BadJson";
  raw_jsons[0] = (char *)(size_t) "{not valid json";
  spec.n_raw_schemas = 1;
  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_OPENAPI_WRITER_COVERAGE_H */
