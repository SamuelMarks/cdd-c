/**
 * @file test_openapi_writer_finish.h
 * @brief Unit tests for OpenAPI Writer (finish).
 */

#ifndef TEST_OPENAPI_WRITER_FINISH_H
#define TEST_OPENAPI_WRITER_FINISH_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "emit/test_openapi_writer_common.h"
/* clang-format on */

TEST test_openapi_writer_close_100_percent(void) {
  JSON_Value *val;
  JSON_Object *obj;
  JSON_Value *parent_val;
  JSON_Object *parent;
  struct OpenAPI_Any any_vals[2];
  struct OpenAPI_Example ex;
  struct OpenAPI_Example ex_arr[1];
  struct OpenAPI_ExternalDocs ed;
  struct OpenAPI_Discriminator disc;
  struct OpenAPI_DiscriminatorMap dmap[1];
  struct OpenAPI_Xml xml;
  struct OpenAPI_Server srv[1];
  struct OpenAPI_ServerVariable svar;
  struct OpenAPI_SchemaRef sr;
  struct OpenAPI_SchemaRef sub_sr[1];
  struct OpenAPI_Parameter p;
  struct OpenAPI_Header h;
  struct OpenAPI_Header h_arr[1];
  struct OpenAPI_Encoding enc;
  struct OpenAPI_MediaType mt_dummy;
  struct OpenAPI_Link lk;
  struct OpenAPI_LinkParam lp[2];
  struct OpenAPI_Response r_dummy;
  struct OpenAPI_RequestBody rb_d;
  struct OpenAPI_Operation op;
  struct OpenAPI_Callback cb_d;
  struct OpenAPI_Path path;
  struct OpenAPI_SecurityRequirementSet sec_set;
  struct OpenAPI_SecurityScheme ss;
  struct OpenAPI_OAuthFlow fl;
  struct OpenAPI_OAuthScope scp[1];
  struct OpenAPI_Spec sp_d;
  struct StructFields sf[1];
  char *dn[1];
  char *tu[1];
  char *json = NULL;
  int k;

  parent_val = json_value_init_object();
  parent = json_value_get_object(parent_val);
  val = json_value_init_object();
  obj = json_value_get_object(val);

  /* 1. L367: duplicate key in merge_schema_extras_object_openapi */
  json_object_set_string(obj, "k", "orig");
  merge_schema_extras_object_openapi(obj, "{\"k\": \"dup\", \"k2\": \"new\"}");

  /* 2. L422: write_schema_type with type == NULL */
  write_schema_type(obj, NULL, 0);

  /* 3. L463: write_schema_type_union with type_union != NULL && n_type_union ==
   * 0 */
  tu[0] = (char *)(size_t) "string";
  write_schema_type_union(obj, "string", 0, tu, 0);

  /* 4. L490, L503: write_enum_any_values variations */
  memset(any_vals, 0, sizeof(any_vals));
  any_vals[0].type = (enum OpenAPI_AnyType)999;
  write_enum_any_values(obj, "k", NULL, 1);
  write_enum_any_values(obj, "k", any_vals, 0);
  write_enum_any_values(obj, NULL, any_vals, 1);
  write_enum_any_values(NULL, "k", any_vals, 1);
  write_enum_any_values(obj, "bad_enum", any_vals, 1);
  any_vals[0].type = OA_ANY_STRING;
  any_vals[0].string = (char *)(size_t) "good";
  write_enum_any_values(obj, "good_enum", any_vals, 1);

  /* 5. L579, L581, L594, L599: write_example_object branches */
  memset(&ex, 0, sizeof(ex));
  ex.ref = (char *)(size_t) "#/ref";
  ex.summary = NULL;
  ex.description = NULL;
  write_example_object(obj, &ex);

  memset(&ex, 0, sizeof(ex));
  ex.data_value_set = 1;
  ex.data_value.type = (enum OpenAPI_AnyType)999;
  write_example_object(obj, &ex);

  memset(&ex, 0, sizeof(ex));
  ex.value_set = 1;
  ex.value.type = (enum OpenAPI_AnyType)999;
  write_example_object(obj, &ex);

  /* 6. L621: write_examples_object null/empty checks */
  memset(ex_arr, 0, sizeof(ex_arr));
  ex_arr[0].name = (char *)(size_t) "ex1";
  write_examples_object(parent, "k", NULL, 1);
  write_examples_object(parent, "k", ex_arr, 0);
  write_examples_object(parent, NULL, ex_arr, 1);
  write_examples_object(NULL, "k", ex_arr, 1);
  write_examples_object(parent, "valid_ex", ex_arr, 1);

  /* 7. L658, L665, L668: write_example_fields branches */
  write_example_fields(parent, NULL, 0, ex_arr, 0);
  write_example_fields(parent, NULL, 1, NULL, 0);
  memset(&any_vals[0], 0, sizeof(any_vals[0]));
  any_vals[0].type = (enum OpenAPI_AnyType)999;
  write_example_fields(parent, &any_vals[0], 1, NULL, 0);

  /* 8. L681: write_external_docs with docs->url == NULL and docs == NULL */
  memset(&ed, 0, sizeof(ed));
  ed.url = NULL;
  write_external_docs(parent, "ext", &ed);
  write_external_docs(parent, "ext", NULL);

  /* 9. L711, L719, L721, L724, L729: write_discriminator_object branches */
  memset(&disc, 0, sizeof(disc));
  disc.property_name = NULL;
  disc.n_mapping = 0;
  disc.default_mapping = NULL;
  write_discriminator_object(parent, &disc, 1);

  memset(&disc, 0, sizeof(disc));
  disc.property_name = (char *)(size_t) "type";
  disc.n_mapping = 0;
  disc.default_mapping = NULL;
  write_discriminator_object(parent, &disc, 1);

  memset(&disc, 0, sizeof(disc));
  memset(dmap, 0, sizeof(dmap));
  dmap[0].value = (char *)(size_t) "val";
  dmap[0].schema = (char *)(size_t) "sch";
  disc.property_name = NULL;
  disc.mapping = dmap;
  disc.n_mapping = 1;
  disc.default_mapping = NULL;
  write_discriminator_object(parent, &disc, 1);

  memset(&disc, 0, sizeof(disc));
  disc.property_name = NULL;
  disc.default_mapping = (char *)(size_t) "def";
  write_discriminator_object(parent, &disc, 1);

  memset(&disc, 0, sizeof(disc));
  disc.property_name = (char *)(size_t) "type";
  disc.mapping = NULL;
  disc.n_mapping = 1;
  write_discriminator_object(parent, &disc, 1);

  disc.mapping = dmap;
  disc.n_mapping = 0;
  write_discriminator_object(parent, &disc, 1);

  dmap[0].value = (char *)(size_t) "val";
  dmap[0].schema = NULL;
  disc.mapping = dmap;
  disc.n_mapping = 1;
  write_discriminator_object(parent, &disc, 1);

  /* 10. L762: write_xml_object with invalid node_type */
  memset(&xml, 0, sizeof(xml));
  xml.node_type_set = 1;
  xml.node_type = (enum OpenAPI_XmlNodeType)999;
  write_xml_object(parent, &xml, 1);

  /* 11. L856, L868: write_server_object branches */
  memset(srv, 0, sizeof(srv));
  srv[0].n_variables = 1;
  srv[0].variables = NULL;
  write_server_object(obj, &srv[0]);

  memset(&svar, 0, sizeof(svar));
  svar.n_enum_values = 1;
  svar.enum_values = NULL;
  srv[0].variables = &svar;
  srv[0].n_variables = 1;
  write_server_object(obj, &srv[0]);

  /* 12. L937: write_schema_example with invalid any */
  memset(&sr, 0, sizeof(sr));
  sr.example_set = 1;
  sr.example.type = (enum OpenAPI_AnyType)999;
  write_schema_ref(parent, "sch_ex", &sr);

  /* 13. L1009, L1044, L1051: write_items_schema_fields branches */
  write_items_schema_fields(obj, NULL);
  memset(&sr, 0, sizeof(sr));
  sr.is_array = 1;
  sr.inline_type = (char *)(size_t) "string";
  sr.items_const_value_set = 1;
  sr.items_const_value.type = (enum OpenAPI_AnyType)999;
  write_schema_ref(parent, "items_const", &sr);

  memset(&sr, 0, sizeof(sr));
  sr.is_array = 1;
  sr.inline_type = (char *)(size_t) "string";
  sr.items_default_value_set = 1;
  sr.items_default_value.type = (enum OpenAPI_AnyType)999;
  write_schema_ref(parent, "items_def", &sr);

  /* 14. L1085, L1127, L1207, L1224, L1241, L1263, L1282, L1286, L1310 */
  write_schema_ref(parent, NULL, &sr);
  write_schema_ref(parent, "sch", NULL);

  memset(&sr, 0, sizeof(sr));
  sr.is_array = 1;
  write_schema_ref(parent, "arr_empty", &sr);

  memset(&sr, 0, sizeof(sr));
  sr.summary = (char *)(size_t) "sum";
  sr.ref_name = NULL;
  sr.ref = NULL;
  write_schema_ref(parent, "sch_sum_no_ref", &sr);

  memset(&sr, 0, sizeof(sr));
  sr.summary = (char *)(size_t) "sum";
  sr.ref_name = NULL;
  sr.ref = (char *)(size_t) "#/ref";
  write_schema_ref(parent, "sch_sum_ref", &sr);

  memset(&sr, 0, sizeof(sr));
  sr.const_value_set = 1;
  sr.const_value.type = (enum OpenAPI_AnyType)999;
  write_schema_ref(parent, "sch_const", &sr);

  memset(&sr, 0, sizeof(sr));
  sr.n_examples = 1;
  sr.examples = NULL;
  write_schema_ref(parent, "sch_ex_null", &sr);

  memset(&sr, 0, sizeof(sr));
  sr.default_value_set = 1;
  sr.default_value.type = (enum OpenAPI_AnyType)999;
  write_schema_ref(parent, "sch_def", &sr);

  memset(sub_sr, 0, sizeof(sub_sr));
  sub_sr[0].ref_name = (char *)(size_t) "Sub";
  memset(&sr, 0, sizeof(sr));
  sr.all_of = sub_sr;
  sr.n_all_of = 1;
  sr.any_of = sub_sr;
  sr.n_any_of = 1;
  sr.one_of = sub_sr;
  sr.n_one_of = 1;
  for (k = 0; k < 25; ++k) {
    JSON_Value *oom_val = json_value_init_object();
    JSON_Object *oom_obj = json_value_get_object(oom_val);
    json_set_allocation_functions(mock_parson_oom_malloc, mock_parson_oom_free);
    g_parson_oom_fail_at = k;
    write_schema_ref(oom_obj, "sch_composed", &sr);
    json_set_allocation_functions(malloc, free);
    json_value_free(oom_val);
  }

  memset(&sr, 0, sizeof(sr));
  sr.any_of = sub_sr;
  sr.n_any_of = 1;
  for (k = 0; k < 25; ++k) {
    JSON_Value *oom_val = json_value_init_object();
    JSON_Object *oom_obj = json_value_get_object(oom_val);
    json_set_allocation_functions(mock_parson_oom_malloc, mock_parson_oom_free);
    g_parson_oom_fail_at = k;
    write_schema_ref(oom_obj, "sch_any", &sr);
    json_set_allocation_functions(malloc, free);
    json_value_free(oom_val);
  }

  memset(&sr, 0, sizeof(sr));
  sr.one_of = sub_sr;
  sr.n_one_of = 1;
  for (k = 0; k < 25; ++k) {
    JSON_Value *oom_val = json_value_init_object();
    JSON_Object *oom_obj = json_value_get_object(oom_val);
    json_set_allocation_functions(mock_parson_oom_malloc, mock_parson_oom_free);
    g_parson_oom_fail_at = k;
    write_schema_ref(oom_obj, "sch_one", &sr);
    json_set_allocation_functions(malloc, free);
    json_value_free(oom_val);
  }

  /* 15. L1398, L1417, L1428, L1453, L1455, L1458: write_parameter_object */
  memset(&p, 0, sizeof(p));
  p.in = (enum OpenAPI_ParamIn)999;
  p.style = (enum OpenAPI_Style)999;
  write_parameter_object(obj, &p);

  memset(&p, 0, sizeof(p));
  p.content_media_types = (struct OpenAPI_MediaType *)(size_t)0x1;
  p.n_content_media_types = 0;
  write_parameter_object(obj, &p);

  memset(&p, 0, sizeof(p));
  p.content_type = (char *)(size_t) "application/json";
  p.schema_set = 1;
  write_parameter_object(obj, &p);

  memset(&p, 0, sizeof(p));
  p.content_type = (char *)(size_t) "application/json";
  p.schema_set = 0;
  p.type = NULL;
  p.is_array = 0;
  write_parameter_object(obj, &p);

  memset(&p, 0, sizeof(p));
  p.content_type = (char *)(size_t) "application/json";
  p.schema_set = 0;
  p.type = NULL;
  p.is_array = 1;
  write_parameter_object(obj, &p);

  /* 16. L1508, L1518, L1529, L1538, L1540, L1552: write_header_object */
  memset(&h, 0, sizeof(h));
  h.style_set = 1;
  h.style = (enum OpenAPI_Style)999;
  write_header_object(obj, &h);

  memset(&h, 0, sizeof(h));
  h.content_media_types = (struct OpenAPI_MediaType *)(size_t)0x1;
  h.n_content_media_types = 0;
  write_header_object(obj, &h);

  memset(&h, 0, sizeof(h));
  h.content_ref = (char *)(size_t) "#/ref";
  h.content_type = (char *)(size_t) "application/json";
  write_header_object(obj, &h);

  memset(&h, 0, sizeof(h));
  h.content_type = (char *)(size_t) "application/json";
  h.schema_set = 1;
  write_header_object(obj, &h);

  memset(&h, 0, sizeof(h));
  h.content_type = (char *)(size_t) "application/json";
  h.schema_set = 0;
  h.type = NULL;
  h.is_array = 0;
  write_header_object(obj, &h);

  memset(&h, 0, sizeof(h));
  h.content_type = NULL;
  h.schema_set = 1;
  write_header_object(obj, &h);

  /* 17. L1578, L1586, L1590, L1594, L1599, L1629, L1670: write_encoding_* */
  memset(&enc, 0, sizeof(enc));
  enc.style_set = 1;
  enc.style = (enum OpenAPI_Style)999;
  write_encoding_object(obj, &enc);

  memset(&enc, 0, sizeof(enc));
  enc.headers = (struct OpenAPI_Header *)(size_t)0x1;
  enc.n_headers = 0;
  write_encoding_object(obj, &enc);

  memset(&enc, 0, sizeof(enc));
  enc.encoding = (struct OpenAPI_Encoding *)(size_t)0x1;
  enc.n_encoding = 0;
  write_encoding_object(obj, &enc);

  memset(&enc, 0, sizeof(enc));
  enc.prefix_encoding = (struct OpenAPI_Encoding *)(size_t)0x1;
  enc.n_prefix_encoding = 0;
  write_encoding_object(obj, &enc);

  memset(&enc, 0, sizeof(enc));
  enc.item_encoding = &enc;
  enc.item_encoding_set = 0;
  write_encoding_object(obj, &enc);

  write_encoding_map(obj, NULL, 1);
  write_encoding_map(obj, &enc, 0);
  write_encoding_array(parent, NULL, &enc, 1);
  write_encoding_array(parent, "k", NULL, 1);
  write_encoding_array(parent, "k", &enc, 0);

  /* 18. L1709, L1712, L1717, L1721, L1726, L1754: write_media_type_* */
  memset(&mt_dummy, 0, sizeof(mt_dummy));
  mt_dummy.encoding = &enc;
  mt_dummy.n_encoding = 0;
  mt_dummy.prefix_encoding = &enc;
  mt_dummy.n_prefix_encoding = 0;
  mt_dummy.item_encoding = &enc;
  mt_dummy.item_encoding_set = 0;
  write_media_type_object(obj, &mt_dummy);
  write_media_type_map(parent, NULL, &mt_dummy, 1);
  write_media_type_map(parent, "k", NULL, 1);
  write_media_type_map(parent, "k", &mt_dummy, 0);

  memset(&mt_dummy, 0, sizeof(mt_dummy));
  mt_dummy.schema_set = 0;
  mt_dummy.schema.ref_name = (char *)(size_t) "Model";
  write_media_type_object(obj, &mt_dummy);

  memset(&mt_dummy, 0, sizeof(mt_dummy));
  mt_dummy.item_schema_set = 0;
  mt_dummy.item_schema.ref_name = (char *)(size_t) "ItemModel";
  write_media_type_object(obj, &mt_dummy);

  /* 19. L1808, L1817, L1819, L1830, L1834, L1856, L1894, L1909, L1955, L1958,
   * L1962 */
  memset(&lk, 0, sizeof(lk));
  lk.n_parameters = 1;
  lk.parameters = NULL;
  write_link_object(obj, &lk);

  memset(&lk, 0, sizeof(lk));
  memset(lp, 0, sizeof(lp));
  lp[0].value.type = OA_ANY_STRING;
  lp[0].value.string = (char *)(size_t) "val";
  lp[0].name = NULL;
  lp[1].value.type = (enum OpenAPI_AnyType)999;
  lp[1].name = (char *)(size_t) "p";
  lk.parameters = lp;
  lk.n_parameters = 2;
  lk.request_body_set = 1;
  lk.request_body.type = (enum OpenAPI_AnyType)999;
  lk.server_set = 1;
  lk.server = NULL;
  write_link_object(obj, &lk);

  memset(h_arr, 0, sizeof(h_arr));
  h_arr[0].name = (char *)(size_t) "X-H";
  write_headers_map(parent, NULL, h_arr, 1, 0);
  write_headers_map(parent, "k", NULL, 1, 0);
  write_headers_map(parent, "k", h_arr, 0, 0);

  memset(&r_dummy, 0, sizeof(r_dummy));
  r_dummy.n_headers = 0;
  write_headers(parent, &r_dummy);
  r_dummy.n_headers = 1;
  r_dummy.headers = NULL;
  write_headers(parent, &r_dummy);
  write_headers(obj, NULL);

  write_links(NULL, &r_dummy);
  r_dummy.n_links = 0;
  write_links(parent, &r_dummy);
  r_dummy.n_links = 1;
  r_dummy.links = NULL;
  write_links(parent, &r_dummy);
  write_links(obj, NULL);

  memset(&r_dummy, 0, sizeof(r_dummy));
  r_dummy.headers = h_arr;
  r_dummy.n_headers = 0;
  r_dummy.links = &lk;
  r_dummy.n_links = 0;
  r_dummy.content_media_types = &mt_dummy;
  r_dummy.n_content_media_types = 0;
  write_response_object(obj, &r_dummy);

  /* 20. L2009, L2054, L2076, L2127, L2148: parameters & request_body */
  write_parameters(NULL, &p, 1);
  write_parameters(parent, &p, 0);

  memset(&rb_d, 0, sizeof(rb_d));
  rb_d.content_media_types = &mt_dummy;
  rb_d.n_content_media_types = 0;
  write_request_body_object(obj, &rb_d);

  memset(&rb_d, 0, sizeof(rb_d));
  rb_d.content_ref = (char *)(size_t) "#/ref";
  rb_d.schema.content_type = (char *)(size_t) "application/json";
  write_request_body_object(obj, &rb_d);

  memset(&op, 0, sizeof(op));
  write_request_body(NULL, &op);

  memset(&op, 0, sizeof(op));
  op.req_body.ref_name = (char *)(size_t) "Body";
  op.req_body.content_type = NULL;
  op.n_req_body_media_types = 0;
  write_request_body(obj, &op);

  memset(&op, 0, sizeof(op));
  op.req_body.content_type = (char *)(size_t) "application/json";
  write_request_body(obj, &op);

  /* 21. L2191, L2193, L2198, L2228, L2240, L2261: callbacks & responses */
  memset(&cb_d, 0, sizeof(cb_d));
  cb_d.ref = (char *)(size_t) "#/ref";
  cb_d.summary = NULL;
  cb_d.description = NULL;
  write_callback_object(obj, &cb_d);

  memset(&cb_d, 0, sizeof(cb_d));
  memset(&path, 0, sizeof(path));
  cb_d.paths = &path;
  cb_d.n_paths = 0;
  write_callback_object(obj, &cb_d);

  memset(&op, 0, sizeof(op));
  write_callbacks(NULL, &op);
  write_callbacks(obj, NULL);
  op.n_callbacks = 0;
  write_callbacks(obj, &op);
  op.n_callbacks = 1;
  op.callbacks = NULL;
  write_callbacks(obj, &op);

  cb_d.name = NULL;
  op.callbacks = &cb_d;
  op.n_callbacks = 1;
  write_callbacks(obj, &op);

  write_responses(obj, NULL);

  /* 22. L2354, L2411, L2472, L2514: operations & paths */
  memset(&op, 0, sizeof(op));
  op.n_servers = 1;
  op.servers = NULL;
  write_operation_object(obj, &op);

  write_additional_operations(NULL, &path);
  write_additional_operations(obj, NULL);
  memset(&path, 0, sizeof(path));
  path.n_additional_operations = 0;
  write_additional_operations(obj, &path);
  path.n_additional_operations = 1;
  path.additional_operations = NULL;
  write_additional_operations(obj, &path);

  memset(&path, 0, sizeof(path));
  path.n_servers = 1;
  path.servers = NULL;
  write_path_item_object(obj, &path);

  memset(&sp_d, 0, sizeof(sp_d));
  memset(&path, 0, sizeof(path));
  path.route = NULL;
  sp_d.paths = &path;
  sp_d.n_paths = 1;
  write_paths(obj, &sp_d);

  /* 23. L2560, L2598, L2695: server array, tags, security requirements */
  write_server_array(NULL, "k", srv, 1);
  write_server_array(parent, NULL, srv, 1);
  write_tags(obj, NULL);
  memset(&sec_set, 0, sizeof(sec_set));
  write_security_requirements(NULL, "k", &sec_set, 1, 1);
  write_security_requirements(parent, NULL, &sec_set, 1, 1);

  /* 24. L2799, L2818, L2853, L2869: security schemes */
  memset(&sp_d, 0, sizeof(sp_d));
  memset(&ss, 0, sizeof(ss));
  ss.type = OA_SEC_HTTP;
  ss.scheme = (char *)(size_t) "basic";
  sp_d.security_schemes = &ss;
  sp_d.n_security_schemes = 1;
  write_security_schemes(obj, &sp_d);

  memset(&ss, 0, sizeof(ss));
  ss.type = OA_SEC_OAUTH2;
  ss.flows = (struct OpenAPI_OAuthFlow *)(size_t)0x1;
  ss.n_flows = 0;
  sp_d.security_schemes = &ss;
  sp_d.n_security_schemes = 1;
  write_security_schemes(obj, &sp_d);

  memset(&ss, 0, sizeof(ss));
  memset(&fl, 0, sizeof(fl));
  memset(scp, 0, sizeof(scp));
  scp[0].name = NULL;
  fl.scopes = scp;
  fl.n_scopes = 1;
  fl.type = OA_OAUTH_FLOW_IMPLICIT;
  ss.type = OA_SEC_OAUTH2;
  ss.flows = &fl;
  ss.n_flows = 1;
  sp_d.security_schemes = &ss;
  sp_d.n_security_schemes = 1;
  write_security_schemes(obj, &sp_d);

  memset(&ss, 0, sizeof(ss));
  ss.type = OA_SEC_OPENID;
  ss.open_id_connect_url = NULL;
  sp_d.security_schemes = &ss;
  sp_d.n_security_schemes = 1;
  write_security_schemes(obj, &sp_d);

  /* 25. L2898, L2934, L2970, L3006, L3047, L3083, L3116, L3150: component_* */
  write_component_parameters(obj, NULL);
  write_component_responses(obj, NULL);
  write_component_headers(obj, NULL);
  write_component_media_types(obj, NULL);
  write_component_examples(obj, NULL);
  memset(&sp_d, 0, sizeof(sp_d));
  sp_d.n_component_examples = 1;
  sp_d.component_examples = NULL;
  write_component_examples(obj, &sp_d);

  write_component_links(obj, NULL);
  memset(&sp_d, 0, sizeof(sp_d));
  sp_d.n_component_links = 1;
  sp_d.component_links = NULL;
  write_component_links(obj, &sp_d);

  write_component_callbacks(obj, NULL);
  memset(&sp_d, 0, sizeof(sp_d));
  sp_d.n_component_callbacks = 1;
  sp_d.component_callbacks = NULL;
  write_component_callbacks(obj, &sp_d);

  write_component_path_items(obj, NULL);

  /* 26. L3269: write_components with NULL name */
  memset(&sp_d, 0, sizeof(sp_d));
  dn[0] = NULL;
  memset(sf, 0, sizeof(sf));
  sp_d.defined_schemas = sf;
  sp_d.defined_schema_names = dn;
  sp_d.n_defined_schemas = 1;
  write_components(obj, &sp_d);

  /* 27. L3448: openapi_write_spec_to_json with n_paths == 0 and
   * paths_extensions_json */
  memset(&sp_d, 0, sizeof(sp_d));
  sp_d.n_paths = 0;
  sp_d.paths_extensions_json = (char *)(size_t) "{\"x-paths\": 1}";
  openapi_write_spec_to_json(&sp_d, &json);
  if (json) {
    free(json);
    json = NULL;
  }

  json_value_free(val);
  json_value_free(parent_val);
  PASS();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_OPENAPI_WRITER_FINISH_H */
