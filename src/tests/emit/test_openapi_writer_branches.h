/**
 * @file test_openapi_writer_branches.h
 * @brief Unit tests for OpenAPI Writer (branches).
 */

#ifndef TEST_OPENAPI_WRITER_BRANCHES_H
#define TEST_OPENAPI_WRITER_BRANCHES_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "emit/test_openapi_writer_common.h"
/* clang-format on */

TEST test_openapi_writer_branch_sweep(void) {
  JSON_Value *val;
  JSON_Object *obj;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Parameter p;
  struct OpenAPI_Header h;
  struct OpenAPI_Response resp;
  struct OpenAPI_Operation op;
  struct OpenAPI_Path path;
  struct OpenAPI_Tag tag;
  struct OpenAPI_Server srv;
  struct OpenAPI_ServerVariable var;
  char *var_enums[1];
  struct OpenAPI_Discriminator disc;
  struct OpenAPI_Xml xml;
  struct OpenAPI_Example ex;
  struct OpenAPI_Encoding enc;
  struct OpenAPI_MediaType mt;
  struct OpenAPI_Link link;
  struct OpenAPI_Callback cb;
  struct OpenAPI_RequestBody rb;
  struct OpenAPI_SchemaRef ref;
  struct OpenAPI_MultipartField mp;
  struct OpenAPI_SecurityScheme sec;
  struct OpenAPI_OAuthFlow flow;
  struct OpenAPI_OAuthScope scope_obj[1];
  struct OpenAPI_SecurityRequirementSet sec_set;
  struct OpenAPI_SecurityRequirement sec_req;

  val = json_value_init_object();
  obj = json_value_get_object(val);

  /* 1. param_is_reserved_header_openapi: p != NULL, header, but p->name == NULL
   */
  memset(&p, 0, sizeof(p));
  p.in = OA_PARAM_IN_HEADER;
  p.name = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, param_is_reserved_header_openapi(&p));

  /* 2. license_fields_invalid: only identifier, only url, only extensions */
  {
    struct OpenAPI_License lic;
    memset(&lic, 0, sizeof(lic));
    lic.identifier = (char *)(size_t) "MIT";
    license_fields_invalid(&lic);
    memset(&lic, 0, sizeof(lic));
    lic.url = (char *)(size_t) "http://url";
    license_fields_invalid(&lic);
    memset(&lic, 0, sizeof(lic));
    lic.extensions_json = (char *)(size_t) "{}";
    license_fields_invalid(&lic);
  }

  /* 3. Defensive calls: obj NULL vs arg NULL */
  memset(&ex, 0, sizeof(ex));
  write_example_object(obj, NULL);
  write_example_object(NULL, &ex);

  memset(&disc, 0, sizeof(disc));
  write_discriminator_object(obj, NULL, 1);
  write_discriminator_object(obj, &disc, 0);

  memset(&xml, 0, sizeof(xml));
  write_xml_object(obj, NULL, 1);
  write_xml_object(obj, &xml, 0);

  memset(&srv, 0, sizeof(srv));
  write_server_object(obj, NULL);
  write_server_object(NULL, &srv);

  memset(&p, 0, sizeof(p));
  write_parameter_object(obj, NULL);
  write_parameter_object(NULL, &p);

  memset(&h, 0, sizeof(h));
  write_header_object(obj, NULL);
  write_header_object(NULL, &h);

  memset(&enc, 0, sizeof(enc));
  write_encoding_object(obj, NULL);
  write_encoding_object(NULL, &enc);

  memset(&mt, 0, sizeof(mt));
  write_media_type_object(obj, NULL);
  write_media_type_object(NULL, &mt);

  memset(&link, 0, sizeof(link));
  write_link_object(obj, NULL);
  write_link_object(NULL, &link);

  memset(&resp, 0, sizeof(resp));
  write_response_object(obj, NULL);
  write_response_object(NULL, &resp);

  memset(&rb, 0, sizeof(rb));
  write_request_body_object(obj, NULL);
  write_request_body_object(NULL, &rb);

  memset(&cb, 0, sizeof(cb));
  write_callback_object(obj, NULL);
  write_callback_object(NULL, &cb);

  memset(&op, 0, sizeof(op));
  write_operation_object(obj, NULL);
  write_operation_object(NULL, &op);

  memset(&path, 0, sizeof(path));
  write_path_item_object(obj, NULL);
  write_path_item_object(NULL, &path);

  /* 4. Contact metadata: only url, only email */
  memset(&spec, 0, sizeof(spec));
  spec.info.contact.url = (char *)(size_t) "http://url";
  write_info(obj, &spec);
  memset(&spec, 0, sizeof(spec));
  spec.info.contact.email = (char *)(size_t) "e@test.com";
  write_info(obj, &spec);

  /* 5. License metadata: only identifier, only url, only extensions */
  memset(&spec, 0, sizeof(spec));
  spec.info.license.identifier = (char *)(size_t) "MIT";
  write_info(obj, &spec);
  memset(&spec, 0, sizeof(spec));
  spec.info.license.url = (char *)(size_t) "http://url";
  write_info(obj, &spec);
  memset(&spec, 0, sizeof(spec));
  spec.info.license.extensions_json = (char *)(size_t) "{}";
  write_info(obj, &spec);

  /* 6. Server variables with enum NULL */
  memset(&srv, 0, sizeof(srv));
  memset(&var, 0, sizeof(var));
  var.name = (char *)(size_t) "port";
  var_enums[0] = NULL;
  var.enum_values = var_enums;
  var.n_enum_values = 0;
  srv.variables = &var;
  srv.n_variables = 1;
  write_server_object(obj, &srv);

  /* 7. Multipart field name NULL */
  memset(&ref, 0, sizeof(ref));
  memset(&mp, 0, sizeof(mp));
  mp.name = NULL;
  ref.multipart_fields = &mp;
  ref.n_multipart_fields = 1;
  write_schema_ref(obj, "mp_noname", &ref);

  /* 8. Schema ref: all_of, any_of, one_of with n == 0 but ptr != NULL */
  memset(&ref, 0, sizeof(ref));
  ref.all_of = &ref;
  ref.n_all_of = 0;
  ref.any_of = &ref;
  ref.n_any_of = 0;
  ref.one_of = &ref;
  ref.n_one_of = 0;
  ref.summary = (char *)(size_t) "sum";
  ref.examples = NULL;
  ref.n_examples = 0;
  write_schema_ref(obj, "ref_zeros", &ref);

  /* 9. Parameter: allow_empty_value with non-query in */
  memset(&p, 0, sizeof(p));
  p.in = OA_PARAM_IN_HEADER;
  p.allow_empty_value_set = 1;
  p.allow_empty_value = 1;
  p.schema_set = 1;
  write_parameter_object(obj, &p);

  memset(&p, 0, sizeof(p));
  p.type = NULL;
  p.is_array = 1;
  write_parameter_object(obj, &p);

  /* 10. Header: type without schema_set */
  memset(&h, 0, sizeof(h));
  h.type = NULL;
  h.is_array = 1;
  h.content_type = (char *)(size_t) "application/json";
  write_header_object(obj, &h);

  memset(&h, 0, sizeof(h));
  h.type = (char *)(size_t) "integer";
  h.content_type = (char *)(size_t) "application/json";
  write_header_object(obj, &h);

  /* 11. Encoding / Media type: name == NULL */
  memset(&mt, 0, sizeof(mt));
  mt.name = NULL;
  write_media_type_map(obj, "content", &mt, 1);

  memset(&h, 0, sizeof(h));
  h.name = NULL;
  write_headers_map(obj, "headers", &h, 1, 0);

  /* 12. Link: parameter without name */
  memset(&link, 0, sizeof(link));
  link.summary = NULL;
  link.description = NULL;
  link.name = NULL;
  write_link_object(obj, &link);

  /* 13. Callback: summary NULL, description NULL, name NULL */
  memset(&cb, 0, sizeof(cb));
  cb.name = NULL;
  cb.summary = NULL;
  cb.description = NULL;
  write_callback_object(obj, &cb);

  /* 14. Operation: without ID, summary, description, docs, deprecated */
  memset(&op, 0, sizeof(op));
  op.operation_id = NULL;
  op.summary = NULL;
  op.description = NULL;
  write_operation_object(obj, &op);

  /* 15. Response: r->code == NULL -> defaults to "default" */
  memset(&resp, 0, sizeof(resp));
  resp.code = NULL;
  op.responses = &resp;
  op.n_responses = 1;
  write_responses(obj, &op);

  /* 16. Path route == NULL -> defaults to "/" */
  memset(&path, 0, sizeof(path));
  path.route = NULL;
  write_path_item_object(obj, &path);

  /* 17. Security scheme: OA_SEC_IN_UNKNOWN */
  memset(&sec, 0, sizeof(sec));
  sec.type = OA_SEC_APIKEY;
  sec.in = OA_SEC_IN_UNKNOWN;
  sec.scheme = (char *)(size_t) "basic";
  memset(&flow, 0, sizeof(flow));
  flow.type = OA_OAUTH_FLOW_IMPLICIT;
  memset(scope_obj, 0, sizeof(scope_obj));
  scope_obj[0].name = NULL;
  flow.scopes = scope_obj;
  flow.n_scopes = 1;
  sec.flows = &flow;
  sec.n_flows = 1;
  memset(&spec, 0, sizeof(spec));
  spec.security_schemes = &sec;
  spec.n_security_schemes = 1;
  write_security_schemes(obj, &spec);

  /* 18. Tag with name == NULL */
  memset(&tag, 0, sizeof(tag));
  tag.name = NULL;
  spec.tags = &tag;
  spec.n_tags = 1;
  write_tags(obj, &spec);

  /* 19. Security requirement with scheme == NULL */
  memset(&sec_req, 0, sizeof(sec_req));
  sec_req.scheme = NULL;
  memset(&sec_set, 0, sizeof(sec_set));
  sec_set.requirements = &sec_req;
  sec_set.n_requirements = 1;
  write_security_requirements(obj, "sec", &sec_set, 1, 1);

  /* 20. Component items with 0 count */
  memset(&spec, 0, sizeof(spec));
  write_component_parameters(obj, &spec);
  write_component_responses(obj, &spec);
  write_component_headers(obj, &spec);
  write_component_media_types(obj, &spec);
  write_component_examples(obj, &spec);
  write_component_links(obj, &spec);
  write_component_callbacks(obj, &spec);
  write_component_path_items(obj, &spec);

  /* 21. Paths empty */
  spec.is_schema_document = 0;
  spec.n_paths = 0;
  spec.paths_extensions_json = NULL;
  write_paths(obj, &spec);

  json_value_free(val);
  PASS();
}

TEST test_openapi_writer_null_and_defensive(void) {
  JSON_Value *val = NULL;
  char *str = NULL;

  ASSERT_EQ(CDD_C_SUCCESS, license_fields_invalid(NULL));
  ASSERT_EQ(CDD_C_SUCCESS, server_url_has_query_or_fragment(NULL));
  ASSERT_EQ(1, server_url_has_query_or_fragment("https://example.com#frag"));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, clone_json_value(NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, clone_json_value(NULL, &val));
  ASSERT(val == NULL);
  ASSERT_EQ(CDD_C_SUCCESS, schema_ref_has_data(NULL));

  {
    struct OpenAPI_Spec s_webhooks;
    char *out_j = NULL;
    memset(&s_webhooks, 0, sizeof(s_webhooks));
    s_webhooks.openapi_version = (char *)(size_t)(size_t) "3.1.0";
    s_webhooks.info.title = (char *)(size_t)(size_t) "Test";
    s_webhooks.info.version = (char *)(size_t)(size_t) "1.0";
    s_webhooks.n_webhooks = 1;
    s_webhooks.webhooks = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, openapi_write_spec_to_json(&s_webhooks, &out_j));
    if (out_j) {
      free(out_j);
      out_j = NULL;
    }
  }

  {
    extern C_CDD_EXPORT int g_cdd_strdup_fail;
    struct OpenAPI_Spec s_doc;
    char *out_j = NULL;
    memset(&s_doc, 0, sizeof(s_doc));
    s_doc.is_schema_document = 1;
    s_doc.schema_root_json = (char *)(size_t)(size_t) "{}";
    g_cdd_strdup_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, openapi_write_spec_to_json(&s_doc, &out_j));
    g_cdd_strdup_fail = 0;
  }

  {
    int k;
    struct OpenAPI_Spec s_empty;
    memset(&s_empty, 0, sizeof(s_empty));
    s_empty.openapi_version = (char *)(size_t)(size_t) "3.1.0";
    s_empty.info.title = (char *)(size_t)(size_t) "A";
    s_empty.info.version = (char *)(size_t)(size_t) "1";
    json_set_allocation_functions(mock_parson_oom_malloc, mock_parson_oom_free);
    for (k = 0; k < 20; ++k) {
      char *out_j = NULL;
      g_parson_oom_fail_at = k;
      openapi_write_spec_to_json(&s_empty, &out_j);
      if (out_j) {
        free(out_j);
        out_j = NULL;
      }
    }
    json_set_allocation_functions(malloc, free);
  }
  ASSERT_EQ(CDD_C_SUCCESS, schema_ref_keyword(0, &str));
  ASSERT_STR_EQ("$ref", str);
  ASSERT_EQ(CDD_C_SUCCESS, schema_ref_keyword(1, &str));
  ASSERT_STR_EQ("$dynamicRef", str);
  ASSERT_EQ(CDD_C_SUCCESS, any_to_json_value(NULL, &val));
  ASSERT(val == NULL);

  write_schema_type(NULL, NULL, 0);
  ASSERT_EQ(CDD_C_SUCCESS, type_union_contains(NULL, 0, NULL));
  write_schema_type_union(NULL, NULL, 0, NULL, 0);
  write_enum_any_values(NULL, NULL, NULL, 0);
  write_any_array_values(NULL, NULL, NULL, 0);
  write_example_object(NULL, NULL);
  ASSERT_EQ(CDD_C_SUCCESS, write_examples_object(NULL, NULL, NULL, 0));
  write_example_fields(NULL, NULL, 0, NULL, 0);
  write_external_docs(NULL, NULL, NULL);
  write_discriminator_object(NULL, NULL, 0);
  write_xml_object(NULL, NULL, 0);
  write_server_object(NULL, NULL);
  write_numeric_constraints(NULL, 0, 0.0, 0, 0, 0.0, 0);
  write_string_constraints(NULL, 0, 0, 0, 0, NULL);
  write_array_constraints(NULL, 0, 0, 0, 0, 0);
  write_items_schema_fields(NULL, NULL);
  write_schema_ref(NULL, NULL, NULL);
  write_schema_from_type_fields(NULL, NULL, NULL, 0, NULL);
  write_parameter_object(NULL, NULL);
  write_header_object(NULL, NULL);
  ASSERT_EQ(CDD_C_SUCCESS, write_encoding_object(NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, write_encoding_map(NULL, NULL, 0));
  write_link_object(NULL, NULL);
  write_response_object(NULL, NULL);
  ASSERT_EQ(CDD_C_SUCCESS, write_request_body_object(NULL, NULL));
  write_callback_object(NULL, NULL);
  ASSERT_EQ(CDD_C_SUCCESS, write_media_type_object(NULL, NULL));

  PASS();
}

TEST test_openapi_writer_schema_ref_branches(void) {
  struct OpenAPI_SchemaRef ref;

  memset(&ref, 0, sizeof(ref));
  ASSERT_EQ(CDD_C_SUCCESS, schema_ref_has_data(&ref));

  ref.schema_is_boolean = 1;
  ASSERT(schema_ref_has_data(&ref) != CDD_C_SUCCESS);
  ref.schema_is_boolean = 0;
  ref.ref_name = (char *)(size_t) "test";
  ASSERT(schema_ref_has_data(&ref) != CDD_C_SUCCESS);
  ref.ref_name = NULL;
  ref.ref = (char *)(size_t) "test";
  ASSERT(schema_ref_has_data(&ref) != CDD_C_SUCCESS);
  ref.ref = NULL;
  ref.inline_type = (char *)(size_t) "string";
  ASSERT(schema_ref_has_data(&ref) != CDD_C_SUCCESS);
  ref.inline_type = NULL;
  ref.n_type_union = 1;
  ASSERT(schema_ref_has_data(&ref) != CDD_C_SUCCESS);
  ref.n_type_union = 0;
  ref.is_array = 1;
  ASSERT(schema_ref_has_data(&ref) != CDD_C_SUCCESS);
  ref.is_array = 0;
  ref.format = (char *)(size_t) "email";
  ASSERT(schema_ref_has_data(&ref) != CDD_C_SUCCESS);
  ref.format = NULL;
  ref.content_media_type = (char *)(size_t) "app/json";
  ASSERT(schema_ref_has_data(&ref) != CDD_C_SUCCESS);
  ref.content_media_type = NULL;
  ref.content_encoding = (char *)(size_t) "base64";
  ASSERT(schema_ref_has_data(&ref) != CDD_C_SUCCESS);
  ref.content_encoding = NULL;
  ref.items_format = (char *)(size_t) "int32";
  ASSERT(schema_ref_has_data(&ref) != CDD_C_SUCCESS);
  ref.items_format = NULL;
  ref.n_items_type_union = 1;
  ASSERT(schema_ref_has_data(&ref) != CDD_C_SUCCESS);
  ref.n_items_type_union = 0;
  ref.items_content_media_type = (char *)(size_t) "text/plain";
  ASSERT(schema_ref_has_data(&ref) != CDD_C_SUCCESS);
  ref.items_content_media_type = NULL;
  ref.items_content_encoding = (char *)(size_t) "binary";
  ASSERT(schema_ref_has_data(&ref) != CDD_C_SUCCESS);
  ref.items_content_encoding = NULL;
  ref.n_multipart_fields = 1;
  ASSERT(schema_ref_has_data(&ref) != CDD_C_SUCCESS);
  ref.n_multipart_fields = 0;
  ref.nullable = 1;
  ASSERT(schema_ref_has_data(&ref) != CDD_C_SUCCESS);
  ref.nullable = 0;
  ref.items_nullable = 1;
  ASSERT(schema_ref_has_data(&ref) != CDD_C_SUCCESS);
  ref.items_nullable = 0;
  ref.default_value_set = 1;
  ASSERT(schema_ref_has_data(&ref) != CDD_C_SUCCESS);
  ref.default_value_set = 0;
  ref.n_enum_values = 1;
  ASSERT(schema_ref_has_data(&ref) != CDD_C_SUCCESS);
  ref.n_enum_values = 0;
  ref.n_items_enum_values = 1;
  ASSERT(schema_ref_has_data(&ref) != CDD_C_SUCCESS);
  ref.n_items_enum_values = 0;
  ref.summary = (char *)(size_t) "sum";
  ASSERT(schema_ref_has_data(&ref) != CDD_C_SUCCESS);
  ref.summary = NULL;
  ref.description = (char *)(size_t) "desc";
  ASSERT(schema_ref_has_data(&ref) != CDD_C_SUCCESS);
  ref.description = NULL;
  ref.deprecated_set = 1;
  ASSERT(schema_ref_has_data(&ref) != CDD_C_SUCCESS);
  ref.deprecated_set = 0;
  ref.read_only_set = 1;
  ASSERT(schema_ref_has_data(&ref) != CDD_C_SUCCESS);
  ref.read_only_set = 0;
  ref.write_only_set = 1;
  ASSERT(schema_ref_has_data(&ref) != CDD_C_SUCCESS);
  ref.write_only_set = 0;
  ref.const_value_set = 1;
  ASSERT(schema_ref_has_data(&ref) != CDD_C_SUCCESS);
  ref.const_value_set = 0;
  ref.n_examples = 1;
  ASSERT(schema_ref_has_data(&ref) != CDD_C_SUCCESS);
  ref.n_examples = 0;
  ref.example_set = 1;
  ASSERT(schema_ref_has_data(&ref) != CDD_C_SUCCESS);
  ref.example_set = 0;
  ref.has_min = 1;
  ASSERT(schema_ref_has_data(&ref) != CDD_C_SUCCESS);
  ref.has_min = 0;
  ref.has_max = 1;
  ASSERT(schema_ref_has_data(&ref) != CDD_C_SUCCESS);
  ref.has_max = 0;
  ref.has_min_len = 1;
  ASSERT(schema_ref_has_data(&ref) != CDD_C_SUCCESS);
  ref.has_min_len = 0;
  ref.has_max_len = 1;
  ASSERT(schema_ref_has_data(&ref) != CDD_C_SUCCESS);
  ref.has_max_len = 0;
  ref.pattern = (char *)(size_t) "^a";
  ASSERT(schema_ref_has_data(&ref) != CDD_C_SUCCESS);
  ref.pattern = NULL;
  ref.has_min_items = 1;
  ASSERT(schema_ref_has_data(&ref) != CDD_C_SUCCESS);
  ref.has_min_items = 0;
  ref.has_max_items = 1;
  ASSERT(schema_ref_has_data(&ref) != CDD_C_SUCCESS);
  ref.has_max_items = 0;
  ref.unique_items = 1;
  ASSERT(schema_ref_has_data(&ref) != CDD_C_SUCCESS);
  ref.unique_items = 0;
  ref.items_has_min = 1;
  ASSERT(schema_ref_has_data(&ref) != CDD_C_SUCCESS);
  ref.items_has_min = 0;
  ref.items_has_max = 1;
  ASSERT(schema_ref_has_data(&ref) != CDD_C_SUCCESS);
  ref.items_has_max = 0;
  ref.items_has_min_len = 1;
  ASSERT(schema_ref_has_data(&ref) != CDD_C_SUCCESS);
  ref.items_has_min_len = 0;
  ref.items_has_max_len = 1;
  ASSERT(schema_ref_has_data(&ref) != CDD_C_SUCCESS);
  ref.items_has_max_len = 0;
  ref.items_pattern = (char *)(size_t) "^b";
  ASSERT(schema_ref_has_data(&ref) != CDD_C_SUCCESS);
  ref.items_pattern = NULL;
  ref.items_has_min_items = 1;
  ASSERT(schema_ref_has_data(&ref) != CDD_C_SUCCESS);
  ref.items_has_min_items = 0;
  ref.items_has_max_items = 1;
  ASSERT(schema_ref_has_data(&ref) != CDD_C_SUCCESS);
  ref.items_has_max_items = 0;
  ref.items_unique_items = 1;
  ASSERT(schema_ref_has_data(&ref) != CDD_C_SUCCESS);
  ref.items_unique_items = 0;
  ref.items_example_set = 1;
  ASSERT(schema_ref_has_data(&ref) != CDD_C_SUCCESS);
  ref.items_example_set = 0;
  ref.n_items_examples = 1;
  ASSERT(schema_ref_has_data(&ref) != CDD_C_SUCCESS);
  ref.n_items_examples = 0;
  ref.items_schema_is_boolean = 1;
  ASSERT(schema_ref_has_data(&ref) != CDD_C_SUCCESS);
  ref.items_schema_is_boolean = 0;
  ref.schema_extra_json = (char *)(size_t) "{}";
  ASSERT(schema_ref_has_data(&ref) != CDD_C_SUCCESS);
  ref.schema_extra_json = NULL;
  ref.external_docs_set = 1;
  ASSERT(schema_ref_has_data(&ref) != CDD_C_SUCCESS);
  ref.external_docs_set = 0;
  ref.discriminator_set = 1;
  ASSERT(schema_ref_has_data(&ref) != CDD_C_SUCCESS);
  ref.discriminator_set = 0;
  ref.xml_set = 1;
  ASSERT(schema_ref_has_data(&ref) != CDD_C_SUCCESS);
  ref.xml_set = 0;
  ref.items_extra_json = (char *)(size_t) "{}";
  ASSERT(schema_ref_has_data(&ref) != CDD_C_SUCCESS);
  ref.items_extra_json = NULL;
  ref.items_const_value_set = 1;
  ASSERT(schema_ref_has_data(&ref) != CDD_C_SUCCESS);
  ref.items_const_value_set = 0;
  ref.items_default_value_set = 1;
  ASSERT(schema_ref_has_data(&ref) != CDD_C_SUCCESS);
  ref.items_default_value_set = 0;

  PASS();
}

TEST test_openapi_writer_schema_and_types_coverage(void) {
  JSON_Value *val;
  JSON_Object *obj;
  char *types[3];
  struct OpenAPI_SchemaRef ref;

  val = json_value_init_object();
  obj = json_value_get_object(val);

  /* write_schema_type */
  write_schema_type(obj, "null", 1);
  ASSERT_STR_EQ("null", json_object_get_string(obj, "type"));
  write_schema_type(obj, "string", 1);
  ASSERT(json_object_get_array(obj, "type") != NULL);
  write_schema_type(obj, "string", 0);
  ASSERT_STR_EQ("string", json_object_get_string(obj, "type"));

  /* type_union_contains */
  types[0] = NULL;
  types[1] = (char *)(size_t) "string";
  types[2] = (char *)(size_t) "null";
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, type_union_contains(types, 3, "null"));
  ASSERT_EQ(CDD_C_SUCCESS, type_union_contains(types, 3, "integer"));
  ASSERT_EQ(CDD_C_SUCCESS, type_union_contains(NULL, 0, "null"));
  ASSERT_EQ(CDD_C_SUCCESS, type_union_contains(types, 3, NULL));

  /* write_schema_type_union */
  write_schema_type_union(obj, "string", 1, types, 3);
  write_schema_type_union(obj, "string", 1, types, 2);
  write_schema_type_union(obj, "integer", 0, NULL, 0);
  write_schema_type_union(obj, NULL, 0, NULL, 0);

  /* write_numeric_constraints */
  write_numeric_constraints(obj, 1, 5.0, 1, 1, 10.0, 1);
  write_numeric_constraints(obj, 1, 5.0, 0, 1, 10.0, 0);
  write_numeric_constraints(obj, 0, 0.0, 1, 0, 0.0, 1);

  /* write_string_constraints */
  write_string_constraints(obj, 1, 2, 1, 20, "^abc$");

  /* write_array_constraints */
  write_array_constraints(obj, 1, 1, 1, 10, 1);

  /* write_schema_from_type_fields */
  write_schema_from_type_fields(obj, "s1", "string", 1, "string");
  write_schema_from_type_fields(obj, "s2", "string", 1, "CustomType");
  write_schema_from_type_fields(obj, "s3", "string", 1, NULL);
  write_schema_from_type_fields(obj, "s4", "string", 0, NULL);
  write_schema_from_type_fields(obj, "s5", "array", 0, NULL);
  write_schema_from_type_fields(obj, "s6", "CustomType", 0, NULL);
  write_schema_from_type_fields(obj, "s7", NULL, 0, NULL);

  /* write_items_schema_fields */
  memset(&ref, 0, sizeof(ref));
  ref.items_content_schema =
      (struct OpenAPI_SchemaRef *)malloc(sizeof(struct OpenAPI_SchemaRef));
  memset(ref.items_content_schema, 0, sizeof(struct OpenAPI_SchemaRef));
  ref.items_content_schema->ref_name = (char *)(size_t) "SubSchema";
  ref.items_format = (char *)(size_t) "date";
  ref.items_content_media_type = (char *)(size_t) "application/json";
  ref.items_content_encoding = (char *)(size_t) "base64";
  ref.items_extra_json = (char *)(size_t) "{\"x-field\": true}";
  write_items_schema_fields(obj, &ref);
  free(ref.items_content_schema);

  /* write_schema_ref cases */
  memset(&ref, 0, sizeof(ref));
  ref.has_multiple_of = 1;
  ref.multiple_of = 2.5;
  write_schema_ref(obj, "s_mult", &ref);

  json_value_free(val);
  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_OPENAPI_WRITER_BRANCHES_H */
