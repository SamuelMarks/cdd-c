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

/**
 * @brief Unit test test_openapi_writer_branch_sweep.
 * @return Greatest test result.
 */
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

  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_value_init_object(&_cdd_jv_1);
    val = _cdd_jv_1;
  }
  obj = json_value_get_object(val);

  /* 1. param_is_reserved_header_openapi: p != NULL, header, but p->name == NULL
   */
  {
    int is_res = 0;
    memset(&p, 0, sizeof(p));
    p.in = OA_PARAM_IN_HEADER;
    p.name = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, param_is_reserved_header_openapi(&p, &is_res));
    ASSERT_EQ(0, is_res);
  }

  /* 2. license_fields_invalid: only identifier, only url, only extensions */
  {
    struct OpenAPI_License lic;
    int inv = 0;
    memset(&lic, 0, sizeof(lic));
    lic.identifier = cdd_test_mut_str("MIT");
    ASSERT_EQ(CDD_C_SUCCESS, license_fields_invalid(&lic, &inv));
    ASSERT_EQ(1, inv);
    memset(&lic, 0, sizeof(lic));
    lic.url = cdd_test_mut_str("http://url");
    ASSERT_EQ(CDD_C_SUCCESS, license_fields_invalid(&lic, &inv));
    ASSERT_EQ(1, inv);
    memset(&lic, 0, sizeof(lic));
    lic.extensions_json = cdd_test_mut_str("{}");
    ASSERT_EQ(CDD_C_SUCCESS, license_fields_invalid(&lic, &inv));
    ASSERT_EQ(1, inv);
  }

  /* 3. Defensive calls: obj NULL vs arg NULL */
  memset(&ex, 0, sizeof(ex));
  g_cdd_last_alloc_err = write_example_object(obj, NULL);
  g_cdd_last_alloc_err = write_example_object(NULL, &ex);

  memset(&disc, 0, sizeof(disc));
  g_cdd_last_alloc_err = write_discriminator_object(obj, NULL, 1);
  g_cdd_last_alloc_err = write_discriminator_object(obj, &disc, 0);

  memset(&xml, 0, sizeof(xml));
  g_cdd_last_alloc_err = write_xml_object(obj, NULL, 1);
  g_cdd_last_alloc_err = write_xml_object(obj, &xml, 0);

  memset(&srv, 0, sizeof(srv));
  g_cdd_last_alloc_err = write_server_object(obj, NULL);
  g_cdd_last_alloc_err = write_server_object(NULL, &srv);

  memset(&p, 0, sizeof(p));
  g_cdd_last_alloc_err = write_parameter_object(obj, NULL);
  g_cdd_last_alloc_err = write_parameter_object(NULL, &p);

  memset(&h, 0, sizeof(h));
  g_cdd_last_alloc_err = write_header_object(obj, NULL);
  g_cdd_last_alloc_err = write_header_object(NULL, &h);

  memset(&enc, 0, sizeof(enc));
  g_cdd_last_alloc_err = write_encoding_object(obj, NULL);
  g_cdd_last_alloc_err = write_encoding_object(NULL, &enc);

  memset(&mt, 0, sizeof(mt));
  g_cdd_last_alloc_err = write_media_type_object(obj, NULL);
  g_cdd_last_alloc_err = write_media_type_object(NULL, &mt);

  memset(&link, 0, sizeof(link));
  g_cdd_last_alloc_err = write_link_object(obj, NULL);
  g_cdd_last_alloc_err = write_link_object(NULL, &link);

  memset(&resp, 0, sizeof(resp));
  g_cdd_last_alloc_err = write_response_object(obj, NULL);
  g_cdd_last_alloc_err = write_response_object(NULL, &resp);

  memset(&rb, 0, sizeof(rb));
  g_cdd_last_alloc_err = write_request_body_object(obj, NULL);
  g_cdd_last_alloc_err = write_request_body_object(NULL, &rb);

  memset(&cb, 0, sizeof(cb));
  g_cdd_last_alloc_err = write_callback_object(obj, NULL);
  g_cdd_last_alloc_err = write_callback_object(NULL, &cb);

  memset(&op, 0, sizeof(op));
  g_cdd_last_alloc_err = write_operation_object(obj, NULL);
  g_cdd_last_alloc_err = write_operation_object(NULL, &op);

  memset(&path, 0, sizeof(path));
  g_cdd_last_alloc_err = write_path_item_object(obj, NULL);
  g_cdd_last_alloc_err = write_path_item_object(NULL, &path);

  /* 4. Contact metadata: only url, only email */
  memset(&spec, 0, sizeof(spec));
  spec.info.contact.url = cdd_test_mut_str("http://url");
  g_cdd_last_alloc_err = write_info(obj, &spec);
  memset(&spec, 0, sizeof(spec));
  spec.info.contact.email = cdd_test_mut_str("e@test.com");
  g_cdd_last_alloc_err = write_info(obj, &spec);

  /* 5. License metadata: only identifier, only url, only extensions */
  memset(&spec, 0, sizeof(spec));
  spec.info.license.identifier = cdd_test_mut_str("MIT");
  g_cdd_last_alloc_err = write_info(obj, &spec);
  memset(&spec, 0, sizeof(spec));
  spec.info.license.url = cdd_test_mut_str("http://url");
  g_cdd_last_alloc_err = write_info(obj, &spec);
  memset(&spec, 0, sizeof(spec));
  spec.info.license.extensions_json = cdd_test_mut_str("{}");
  g_cdd_last_alloc_err = write_info(obj, &spec);

  /* 6. Server variables with enum NULL */
  memset(&srv, 0, sizeof(srv));
  memset(&var, 0, sizeof(var));
  var.name = cdd_test_mut_str("port");
  var_enums[0] = NULL;
  var.enum_values = var_enums;
  var.n_enum_values = 0;
  srv.variables = &var;
  srv.n_variables = 1;
  g_cdd_last_alloc_err = write_server_object(obj, &srv);

  /* 7. Multipart field name NULL */
  memset(&ref, 0, sizeof(ref));
  memset(&mp, 0, sizeof(mp));
  mp.name = NULL;
  ref.multipart_fields = &mp;
  ref.n_multipart_fields = 1;
  g_cdd_last_alloc_err = write_schema_ref(obj, "mp_noname", &ref);

  /* 8. Schema ref: all_of, any_of, one_of with n == 0 but ptr != NULL */
  memset(&ref, 0, sizeof(ref));
  ref.all_of = &ref;
  ref.n_all_of = 0;
  ref.any_of = &ref;
  ref.n_any_of = 0;
  ref.one_of = &ref;
  ref.n_one_of = 0;
  ref.summary = cdd_test_mut_str("sum");
  ref.examples = NULL;
  ref.n_examples = 0;
  g_cdd_last_alloc_err = write_schema_ref(obj, "ref_zeros", &ref);

  /* 9. Parameter: allow_empty_value with non-query in */
  memset(&p, 0, sizeof(p));
  p.in = OA_PARAM_IN_HEADER;
  p.allow_empty_value_set = 1;
  p.allow_empty_value = 1;
  p.schema_set = 1;
  g_cdd_last_alloc_err = write_parameter_object(obj, &p);

  memset(&p, 0, sizeof(p));
  p.type = NULL;
  p.is_array = 1;
  g_cdd_last_alloc_err = write_parameter_object(obj, &p);

  /* 10. Header: type without schema_set */
  memset(&h, 0, sizeof(h));
  h.type = NULL;
  h.is_array = 1;
  h.content_type = cdd_test_mut_str("application/json");
  g_cdd_last_alloc_err = write_header_object(obj, &h);

  memset(&h, 0, sizeof(h));
  h.type = cdd_test_mut_str("integer");
  h.content_type = cdd_test_mut_str("application/json");
  g_cdd_last_alloc_err = write_header_object(obj, &h);

  /* 11. Encoding / Media type: name == NULL */
  memset(&mt, 0, sizeof(mt));
  mt.name = NULL;
  g_cdd_last_alloc_err = write_media_type_map(obj, "content", &mt, 1);

  memset(&h, 0, sizeof(h));
  h.name = NULL;
  g_cdd_last_alloc_err = write_headers_map(obj, "headers", &h, 1, 0);

  /* 12. Link: parameter without name */
  memset(&link, 0, sizeof(link));
  link.summary = NULL;
  link.description = NULL;
  link.name = NULL;
  g_cdd_last_alloc_err = write_link_object(obj, &link);

  /* 13. Callback: summary NULL, description NULL, name NULL */
  memset(&cb, 0, sizeof(cb));
  cb.name = NULL;
  cb.summary = NULL;
  cb.description = NULL;
  g_cdd_last_alloc_err = write_callback_object(obj, &cb);

  /* 14. Operation: without ID, summary, description, docs, deprecated */
  memset(&op, 0, sizeof(op));
  op.operation_id = NULL;
  op.summary = NULL;
  op.description = NULL;
  g_cdd_last_alloc_err = write_operation_object(obj, &op);

  /* 15. Response: r->code == NULL -> defaults to "default" */
  memset(&resp, 0, sizeof(resp));
  resp.code = NULL;
  op.responses = &resp;
  op.n_responses = 1;
  g_cdd_last_alloc_err = write_responses(obj, &op);

  /* 16. Path route == NULL -> defaults to "/" */
  memset(&path, 0, sizeof(path));
  path.route = NULL;
  g_cdd_last_alloc_err = write_path_item_object(obj, &path);

  /* 17. Security scheme: OA_SEC_IN_UNKNOWN */
  memset(&sec, 0, sizeof(sec));
  sec.type = OA_SEC_APIKEY;
  sec.in = OA_SEC_IN_UNKNOWN;
  sec.scheme = cdd_test_mut_str("basic");
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
  g_cdd_last_alloc_err = write_security_schemes(obj, &spec);

  /* 18. Tag with name == NULL */
  memset(&tag, 0, sizeof(tag));
  tag.name = NULL;
  spec.tags = &tag;
  spec.n_tags = 1;
  g_cdd_last_alloc_err = write_tags(obj, &spec);

  /* 19. Security requirement with scheme == NULL */
  memset(&sec_req, 0, sizeof(sec_req));
  sec_req.scheme = NULL;
  memset(&sec_set, 0, sizeof(sec_set));
  sec_set.requirements = &sec_req;
  sec_set.n_requirements = 1;
  g_cdd_last_alloc_err =
      write_security_requirements(obj, "sec", &sec_set, 1, 1);

  /* 20. Component items with 0 count */
  memset(&spec, 0, sizeof(spec));
  g_cdd_last_alloc_err = write_component_parameters(obj, &spec);
  g_cdd_last_alloc_err = write_component_responses(obj, &spec);
  g_cdd_last_alloc_err = write_component_headers(obj, &spec);
  g_cdd_last_alloc_err = write_component_media_types(obj, &spec);
  g_cdd_last_alloc_err = write_component_examples(obj, &spec);
  g_cdd_last_alloc_err = write_component_links(obj, &spec);
  g_cdd_last_alloc_err = write_component_callbacks(obj, &spec);
  g_cdd_last_alloc_err = write_component_path_items(obj, &spec);

  /* 21. Paths empty */
  spec.is_schema_document = 0;
  spec.n_paths = 0;
  spec.paths_extensions_json = NULL;
  g_cdd_last_alloc_err = write_paths(obj, &spec);

  g_cdd_last_alloc_err = json_value_free(val);
  PASS();
}

/**
 * @brief Unit test test_openapi_writer_null_and_defensive.
 * @return Greatest test result.
 */
TEST test_openapi_writer_null_and_defensive(void) {
  JSON_Value *val = NULL;
  char *str = NULL;
  int bval = 0;

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, license_fields_invalid(NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, license_fields_invalid(NULL, &bval));
  ASSERT_EQ(0, bval);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            server_url_has_query_or_fragment(NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, server_url_has_query_or_fragment(NULL, &bval));
  ASSERT_EQ(0, bval);
  ASSERT_EQ(CDD_C_SUCCESS, server_url_has_query_or_fragment(
                               "https://example.com#frag", &bval));
  ASSERT_EQ(1, bval);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, clone_json_value(NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, clone_json_value(NULL, &val));
  ASSERT(val == NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, schema_ref_has_data(NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, schema_ref_has_data(NULL, &bval));
  ASSERT_EQ(0, bval);

  {
    struct OpenAPI_Spec s_webhooks;
    char *out_j = NULL;
    memset(&s_webhooks, 0, sizeof(s_webhooks));
    s_webhooks.openapi_version = cdd_test_mut_str("3.1.0");
    s_webhooks.info.title = cdd_test_mut_str("Test");
    s_webhooks.info.version = cdd_test_mut_str("1.0");
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
    s_doc.schema_root_json = cdd_test_mut_str("{}");
    g_cdd_strdup_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, openapi_write_spec_to_json(&s_doc, &out_j));
    g_cdd_strdup_fail = 0;
  }

  {
    int k;
    struct OpenAPI_Spec s_empty;
    memset(&s_empty, 0, sizeof(s_empty));
    s_empty.openapi_version = cdd_test_mut_str("3.1.0");
    s_empty.info.title = cdd_test_mut_str("A");
    s_empty.info.version = cdd_test_mut_str("1");
    json_set_allocation_functions(mock_parson_oom_malloc, mock_parson_oom_free);
    for (k = 0; k < 20; ++k) {
      char *out_j = NULL;
      g_parson_oom_fail_at = k;
      g_cdd_last_alloc_err = openapi_write_spec_to_json(&s_empty, &out_j);
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

  g_cdd_last_alloc_err = write_schema_type(NULL, NULL, 0);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            type_union_contains(NULL, 0, NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, type_union_contains(NULL, 0, NULL, &bval));
  ASSERT_EQ(0, bval);
  g_cdd_last_alloc_err = write_schema_type_union(NULL, NULL, 0, NULL, 0);
  g_cdd_last_alloc_err = write_enum_any_values(NULL, NULL, NULL, 0);
  g_cdd_last_alloc_err = write_any_array_values(NULL, NULL, NULL, 0);
  g_cdd_last_alloc_err = write_example_object(NULL, NULL);
  ASSERT_EQ(CDD_C_SUCCESS, write_examples_object(NULL, NULL, NULL, 0));
  g_cdd_last_alloc_err = write_example_fields(NULL, NULL, 0, NULL, 0);
  g_cdd_last_alloc_err = write_external_docs(NULL, NULL, NULL);
  g_cdd_last_alloc_err = write_discriminator_object(NULL, NULL, 0);
  g_cdd_last_alloc_err = write_xml_object(NULL, NULL, 0);
  g_cdd_last_alloc_err = write_server_object(NULL, NULL);
  g_cdd_last_alloc_err = write_numeric_constraints(NULL, 0, 0.0, 0, 0, 0.0, 0);
  g_cdd_last_alloc_err = write_string_constraints(NULL, 0, 0, 0, 0, NULL);
  g_cdd_last_alloc_err = write_array_constraints(NULL, 0, 0, 0, 0, 0);
  g_cdd_last_alloc_err = write_items_schema_fields(NULL, NULL);
  g_cdd_last_alloc_err = write_schema_ref(NULL, NULL, NULL);
  g_cdd_last_alloc_err =
      write_schema_from_type_fields(NULL, NULL, NULL, 0, NULL);
  g_cdd_last_alloc_err = write_parameter_object(NULL, NULL);
  g_cdd_last_alloc_err = write_header_object(NULL, NULL);
  ASSERT_EQ(CDD_C_SUCCESS, write_encoding_object(NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, write_encoding_map(NULL, NULL, 0));
  g_cdd_last_alloc_err = write_link_object(NULL, NULL);
  g_cdd_last_alloc_err = write_response_object(NULL, NULL);
  ASSERT_EQ(CDD_C_SUCCESS, write_request_body_object(NULL, NULL));
  g_cdd_last_alloc_err = write_callback_object(NULL, NULL);
  ASSERT_EQ(CDD_C_SUCCESS, write_media_type_object(NULL, NULL));

  PASS();
}

/**
 * @brief Helper macro to test schema_ref_has_data for a single field.
 * @param[in] stmt_set Statement to set the field.
 * @param[in] stmt_clear Statement to clear the field.
 */
#define CHECK_HAS_DATA(stmt_set, stmt_clear)                                   \
  do {                                                                         \
    stmt_set;                                                                  \
    ASSERT_EQ(CDD_C_SUCCESS, schema_ref_has_data(&ref, &has_data));            \
    ASSERT_EQ(1, has_data);                                                    \
    stmt_clear;                                                                \
  } while (0)

/**
 * @brief Unit test test_openapi_writer_schema_ref_branches.
 * @return Greatest test result.
 */
TEST test_openapi_writer_schema_ref_branches(void) {
  struct OpenAPI_SchemaRef ref;
  int has_data = 0;

  memset(&ref, 0, sizeof(ref));
  ASSERT_EQ(CDD_C_SUCCESS, schema_ref_has_data(&ref, &has_data));
  ASSERT_EQ(0, has_data);

  CHECK_HAS_DATA(ref.schema_is_boolean = 1, ref.schema_is_boolean = 0);
  CHECK_HAS_DATA(ref.ref_name = cdd_test_mut_str("test"), ref.ref_name = NULL);
  CHECK_HAS_DATA(ref.ref = cdd_test_mut_str("test"), ref.ref = NULL);
  CHECK_HAS_DATA(ref.inline_type = cdd_test_mut_str("string"),
                 ref.inline_type = NULL);
  CHECK_HAS_DATA(ref.n_type_union = 1, ref.n_type_union = 0);
  CHECK_HAS_DATA(ref.is_array = 1, ref.is_array = 0);
  CHECK_HAS_DATA(ref.format = cdd_test_mut_str("email"), ref.format = NULL);
  CHECK_HAS_DATA(ref.content_media_type = cdd_test_mut_str("app/json"),
                 ref.content_media_type = NULL);
  CHECK_HAS_DATA(ref.content_encoding = cdd_test_mut_str("base64"),
                 ref.content_encoding = NULL);
  CHECK_HAS_DATA(ref.items_format = cdd_test_mut_str("int32"),
                 ref.items_format = NULL);
  CHECK_HAS_DATA(ref.n_items_type_union = 1, ref.n_items_type_union = 0);
  CHECK_HAS_DATA(ref.items_content_media_type = cdd_test_mut_str("text/plain"),
                 ref.items_content_media_type = NULL);
  CHECK_HAS_DATA(ref.items_content_encoding = cdd_test_mut_str("binary"),
                 ref.items_content_encoding = NULL);
  CHECK_HAS_DATA(ref.n_multipart_fields = 1, ref.n_multipart_fields = 0);
  CHECK_HAS_DATA(ref.nullable = 1, ref.nullable = 0);
  CHECK_HAS_DATA(ref.items_nullable = 1, ref.items_nullable = 0);
  CHECK_HAS_DATA(ref.default_value_set = 1, ref.default_value_set = 0);
  CHECK_HAS_DATA(ref.n_enum_values = 1, ref.n_enum_values = 0);
  CHECK_HAS_DATA(ref.n_items_enum_values = 1, ref.n_items_enum_values = 0);
  CHECK_HAS_DATA(ref.summary = cdd_test_mut_str("sum"), ref.summary = NULL);
  CHECK_HAS_DATA(ref.description = cdd_test_mut_str("desc"),
                 ref.description = NULL);
  CHECK_HAS_DATA(ref.deprecated_set = 1, ref.deprecated_set = 0);
  CHECK_HAS_DATA(ref.read_only_set = 1, ref.read_only_set = 0);
  CHECK_HAS_DATA(ref.write_only_set = 1, ref.write_only_set = 0);
  CHECK_HAS_DATA(ref.const_value_set = 1, ref.const_value_set = 0);
  CHECK_HAS_DATA(ref.n_examples = 1, ref.n_examples = 0);
  CHECK_HAS_DATA(ref.example_set = 1, ref.example_set = 0);
  CHECK_HAS_DATA(ref.has_min = 1, ref.has_min = 0);
  CHECK_HAS_DATA(ref.has_max = 1, ref.has_max = 0);
  CHECK_HAS_DATA(ref.has_min_len = 1, ref.has_min_len = 0);
  CHECK_HAS_DATA(ref.has_max_len = 1, ref.has_max_len = 0);
  CHECK_HAS_DATA(ref.pattern = cdd_test_mut_str("^a"), ref.pattern = NULL);
  CHECK_HAS_DATA(ref.has_min_items = 1, ref.has_min_items = 0);
  CHECK_HAS_DATA(ref.has_max_items = 1, ref.has_max_items = 0);
  CHECK_HAS_DATA(ref.unique_items = 1, ref.unique_items = 0);
  CHECK_HAS_DATA(ref.items_has_min = 1, ref.items_has_min = 0);
  CHECK_HAS_DATA(ref.items_has_max = 1, ref.items_has_max = 0);
  CHECK_HAS_DATA(ref.items_has_min_len = 1, ref.items_has_min_len = 0);
  CHECK_HAS_DATA(ref.items_has_max_len = 1, ref.items_has_max_len = 0);
  CHECK_HAS_DATA(ref.items_pattern = cdd_test_mut_str("^b"),
                 ref.items_pattern = NULL);
  CHECK_HAS_DATA(ref.items_has_min_items = 1, ref.items_has_min_items = 0);
  CHECK_HAS_DATA(ref.items_has_max_items = 1, ref.items_has_max_items = 0);
  CHECK_HAS_DATA(ref.items_unique_items = 1, ref.items_unique_items = 0);
  CHECK_HAS_DATA(ref.items_example_set = 1, ref.items_example_set = 0);
  CHECK_HAS_DATA(ref.n_items_examples = 1, ref.n_items_examples = 0);
  CHECK_HAS_DATA(ref.items_schema_is_boolean = 1,
                 ref.items_schema_is_boolean = 0);
  CHECK_HAS_DATA(ref.schema_extra_json = cdd_test_mut_str("{}"),
                 ref.schema_extra_json = NULL);
  CHECK_HAS_DATA(ref.external_docs_set = 1, ref.external_docs_set = 0);
  CHECK_HAS_DATA(ref.discriminator_set = 1, ref.discriminator_set = 0);
  CHECK_HAS_DATA(ref.xml_set = 1, ref.xml_set = 0);
  CHECK_HAS_DATA(ref.items_extra_json = cdd_test_mut_str("{}"),
                 ref.items_extra_json = NULL);
  CHECK_HAS_DATA(ref.items_const_value_set = 1, ref.items_const_value_set = 0);
  CHECK_HAS_DATA(ref.items_default_value_set = 1,
                 ref.items_default_value_set = 0);

#undef CHECK_HAS_DATA

  PASS();
}

/**
 * @brief Unit test test_openapi_writer_schema_and_types_coverage.
 * @return Greatest test result.
 */
TEST test_openapi_writer_schema_and_types_coverage(void) {
  JSON_Value *val;
  JSON_Object *obj;
  char *types[3];
  struct OpenAPI_SchemaRef ref;

  {
    JSON_Value *_cdd_jv_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_value_init_object(&_cdd_jv_1);
    val = _cdd_jv_1;
  }
  obj = json_value_get_object(val);

  /* write_schema_type */
  g_cdd_last_alloc_err = write_schema_type(obj, "null", 1);
  {
    const char *_cdd_js_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_object_get_string(obj, "type", &_cdd_js_1);
    ASSERT_STR_EQ("null", _cdd_js_1);
  }
  g_cdd_last_alloc_err = write_schema_type(obj, "string", 1);
  {
    JSON_Array *_cdd_ja_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_object_get_array(obj, "type", &_cdd_ja_1);
    ASSERT(_cdd_ja_1 != NULL);
  }
  g_cdd_last_alloc_err = write_schema_type(obj, "string", 0);
  {
    const char *_cdd_js_1 = NULL;
    g_cdd_last_alloc_err = cdd_json_object_get_string(obj, "type", &_cdd_js_1);
    ASSERT_STR_EQ("string", _cdd_js_1);
  }

  /* type_union_contains */
  {
    int contains = 0;
    types[0] = NULL;
    types[1] = cdd_test_mut_str("string");
    types[2] = cdd_test_mut_str("null");
    ASSERT_EQ(CDD_C_SUCCESS, type_union_contains(types, 3, "null", &contains));
    ASSERT_EQ(1, contains);
    ASSERT_EQ(CDD_C_SUCCESS,
              type_union_contains(types, 3, "integer", &contains));
    ASSERT_EQ(0, contains);
    ASSERT_EQ(CDD_C_SUCCESS, type_union_contains(NULL, 0, "null", &contains));
    ASSERT_EQ(0, contains);
    ASSERT_EQ(CDD_C_SUCCESS, type_union_contains(types, 3, NULL, &contains));
    ASSERT_EQ(0, contains);
  }

  /* write_schema_type_union */
  g_cdd_last_alloc_err = write_schema_type_union(obj, "string", 1, types, 3);
  g_cdd_last_alloc_err = write_schema_type_union(obj, "string", 1, types, 2);
  g_cdd_last_alloc_err = write_schema_type_union(obj, "integer", 0, NULL, 0);
  g_cdd_last_alloc_err = write_schema_type_union(obj, NULL, 0, NULL, 0);

  /* write_numeric_constraints */
  g_cdd_last_alloc_err = write_numeric_constraints(obj, 1, 5.0, 1, 1, 10.0, 1);
  g_cdd_last_alloc_err = write_numeric_constraints(obj, 1, 5.0, 0, 1, 10.0, 0);
  g_cdd_last_alloc_err = write_numeric_constraints(obj, 0, 0.0, 1, 0, 0.0, 1);

  /* write_string_constraints */
  g_cdd_last_alloc_err = write_string_constraints(obj, 1, 2, 1, 20, "^abc$");

  /* write_array_constraints */
  g_cdd_last_alloc_err = write_array_constraints(obj, 1, 1, 1, 10, 1);

  /* write_schema_from_type_fields */
  g_cdd_last_alloc_err =
      write_schema_from_type_fields(obj, "s1", "string", 1, "string");
  g_cdd_last_alloc_err =
      write_schema_from_type_fields(obj, "s2", "string", 1, "CustomType");
  g_cdd_last_alloc_err =
      write_schema_from_type_fields(obj, "s3", "string", 1, NULL);
  g_cdd_last_alloc_err =
      write_schema_from_type_fields(obj, "s4", "string", 0, NULL);
  g_cdd_last_alloc_err =
      write_schema_from_type_fields(obj, "s5", "array", 0, NULL);
  g_cdd_last_alloc_err =
      write_schema_from_type_fields(obj, "s6", "CustomType", 0, NULL);
  g_cdd_last_alloc_err =
      write_schema_from_type_fields(obj, "s7", NULL, 0, NULL);

  /* write_items_schema_fields */
  memset(&ref, 0, sizeof(ref));
  ref.items_content_schema =
      (struct OpenAPI_SchemaRef *)malloc(sizeof(struct OpenAPI_SchemaRef));
  memset(ref.items_content_schema, 0, sizeof(struct OpenAPI_SchemaRef));
  ref.items_content_schema->ref_name = cdd_test_mut_str("SubSchema");
  ref.items_format = cdd_test_mut_str("date");
  ref.items_content_media_type = cdd_test_mut_str("application/json");
  ref.items_content_encoding = cdd_test_mut_str("base64");
  ref.items_extra_json = cdd_test_mut_str("{\"x-field\": true}");
  g_cdd_last_alloc_err = write_items_schema_fields(obj, &ref);
  free(ref.items_content_schema);

  /* write_schema_ref cases */
  memset(&ref, 0, sizeof(ref));
  ref.has_multiple_of = 1;
  ref.multiple_of = 2.5;
  g_cdd_last_alloc_err = write_schema_ref(obj, "s_mult", &ref);

  g_cdd_last_alloc_err = json_value_free(val);
  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_OPENAPI_WRITER_BRANCHES_H */
