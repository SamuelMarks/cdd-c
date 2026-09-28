/**
 * @file test_openapi_writer_oom.h
 * @brief Unit tests for OpenAPI Writer (oom).
 */

#ifndef TEST_OPENAPI_WRITER_OOM_H
#define TEST_OPENAPI_WRITER_OOM_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "emit/test_openapi_writer_common.h"
/* clang-format on */

TEST test_openapi_writer_extras_and_edge_branches(void) {
  JSON_Value *val;
  JSON_Object *obj;
  struct OpenAPI_SchemaRef ref;
  struct OpenAPI_MultipartField mp_fields[1];
  struct OpenAPI_Parameter p;
  struct OpenAPI_Header h;
  struct OpenAPI_Response resp;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Path c_paths[1];
  char *json = NULL;
  cdd_c_error_t rc;

  val = json_value_init_object();
  obj = json_value_get_object(val);

  /* 1. merge_schema_extras_object_openapi branches */
  ASSERT_EQ(CDD_C_SUCCESS, merge_schema_extras_object_openapi(NULL, "{}"));
  ASSERT_EQ(CDD_C_SUCCESS, merge_schema_extras_object_openapi(obj, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, merge_schema_extras_object_openapi(obj, ""));
  ASSERT_EQ(CDD_C_SUCCESS, merge_schema_extras_object_openapi(obj, "{invalid"));
  ASSERT_EQ(CDD_C_SUCCESS, merge_schema_extras_object_openapi(obj, "[1, 2]"));
  json_object_set_string(obj, "k1", "orig");
  ASSERT_EQ(CDD_C_SUCCESS, merge_schema_extras_object_openapi(
                               obj, "{\"k1\": \"override\", \"k2\": 42}"));
  ASSERT_STR_EQ("orig", json_object_get_string(obj, "k1"));
  ASSERT_EQ(42, (int)json_object_get_number(obj, "k2"));

  /* 2. write_multipart_schema with f->type == NULL and f->is_binary == 0 */
  memset(&ref, 0, sizeof(ref));
  memset(mp_fields, 0, sizeof(mp_fields));
  mp_fields[0].name = (char *)(size_t) "untyped_field";
  mp_fields[0].type = NULL;
  mp_fields[0].is_binary = 0;
  ref.multipart_fields = mp_fields;
  ref.n_multipart_fields = 1;
  write_schema_ref(obj, "mp_untyped", &ref);

  /* 3. write_schema_ref array with primitive ref_name and custom ref_name */
  memset(&ref, 0, sizeof(ref));
  ref.is_array = 1;
  ref.ref_name = (char *)(size_t) "integer";
  write_schema_ref(obj, "arr_prim", &ref);

  memset(&ref, 0, sizeof(ref));
  ref.is_array = 1;
  ref.ref_name = (char *)(size_t) "CustomEntity";
  write_schema_ref(obj, "arr_custom", &ref);

  /* 4. write_schema_ref minProperties and maxProperties */
  memset(&ref, 0, sizeof(ref));
  ref.has_min_properties = 1;
  ref.min_properties = 3;
  ref.has_max_properties = 1;
  ref.max_properties = 9;
  write_schema_ref(obj, "props_constraints", &ref);

  /* 5. write_parameter_object with in=QUERYSTRING and custom content_type */
  memset(&p, 0, sizeof(p));
  p.in = OA_PARAM_IN_QUERYSTRING;
  p.content_type = (char *)(size_t) "application/x-custom-qs";
  write_parameter_object(obj, &p);

  /* 6. write_header_object with content_type == NULL and schema_set = 1 */
  memset(&h, 0, sizeof(h));
  h.content_type = NULL;
  h.schema_set = 1;
  h.schema.ref_name = (char *)(size_t) "HeaderType";
  write_header_object(obj, &h);

  /* 7. write_response_object with summary and no ref */
  memset(&resp, 0, sizeof(resp));
  resp.ref = NULL;
  resp.summary = (char *)(size_t) "Summary for response";
  resp.description = (char *)(size_t) "Description";
  write_response_object(obj, &resp);

  /* 8. write_component_path_items with component_path_item_names == NULL */
  memset(&spec, 0, sizeof(spec));
  spec.openapi_version = (char *)(size_t) "3.2.0";
  spec.info.title = (char *)(size_t) "Path Items API";
  spec.info.version = (char *)(size_t) "1.0.0";
  memset(c_paths, 0, sizeof(c_paths));
  c_paths[0].route = (char *)(size_t) "/routed_component_path";
  spec.component_path_items = c_paths;
  spec.component_path_item_names = NULL;
  spec.n_component_path_items = 1;
  rc = openapi_write_spec_to_json(&spec, &json);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(json != NULL);
  free(json);
  json = NULL;

  json_value_free(val);
  PASS();
}

TEST test_openapi_writer_direct_oom(void) {
  JSON_Value *val;
  JSON_Object *obj;
  JSON_Value *out_val = NULL;
  char *types[2];
  struct OpenAPI_Any any_vals[1];
  struct OpenAPI_Example exs[1];
  struct OpenAPI_Encoding enc_item;
  struct OpenAPI_Encoding enc_arr[1];
  struct OpenAPI_MediaType mt;
  struct OpenAPI_RequestBody rb;
  int k;

  val = json_value_init_object();
  obj = json_value_get_object(val);

  /* 1. clone_json_value OOM */
  json_set_allocation_functions(mock_parson_oom_malloc, mock_parson_oom_free);
  g_parson_oom_fail_at = 0;
  clone_json_value(val, &out_val);
  json_set_allocation_functions(malloc, free);

  /* 2. merge_schema_extras_object_openapi OOM */
  json_set_allocation_functions(mock_parson_oom_malloc, mock_parson_oom_free);
  for (k = 0; k < 10; ++k) {
    g_parson_oom_fail_at = k;
    merge_schema_extras_object_openapi(obj, "{\"foo\": 123, \"bar\": 456}");
  }
  json_set_allocation_functions(malloc, free);

  /* 3. write_schema_type OOM */
  json_set_allocation_functions(mock_parson_oom_malloc, mock_parson_oom_free);
  g_parson_oom_fail_at = 0;
  write_schema_type(obj, "string", 1);
  json_set_allocation_functions(malloc, free);

  /* 4. write_schema_type_union OOM */
  types[0] = (char *)(size_t) "string";
  types[1] = (char *)(size_t) "integer";
  json_set_allocation_functions(mock_parson_oom_malloc, mock_parson_oom_free);
  g_parson_oom_fail_at = 0;
  write_schema_type_union(obj, "string", 1, types, 2);
  json_set_allocation_functions(malloc, free);

  /* 5. write_enum_any_values OOM */
  memset(any_vals, 0, sizeof(any_vals));
  any_vals[0].type = OA_ANY_NUMBER;
  any_vals[0].number = 1.0;
  json_set_allocation_functions(mock_parson_oom_malloc, mock_parson_oom_free);
  g_parson_oom_fail_at = 0;
  write_enum_any_values(obj, "enum", any_vals, 1);
  json_set_allocation_functions(malloc, free);

  /* 6. write_examples_object OOM */
  memset(exs, 0, sizeof(exs));
  exs[0].name = (char *)(size_t) "ex1";
  json_set_allocation_functions(mock_parson_oom_malloc, mock_parson_oom_free);
  g_parson_oom_fail_at = 0;
  write_examples_object(obj, "examples", exs, 1);
  json_set_allocation_functions(malloc, free);

  /* 7. write_encoding_object OOM branches */
  memset(&enc_item, 0, sizeof(enc_item));
  memset(enc_arr, 0, sizeof(enc_arr));
  enc_arr[0].name = (char *)(size_t) "sub";
  enc_item.encoding = enc_arr;
  enc_item.n_encoding = 1;
  enc_item.prefix_encoding = enc_arr;
  enc_item.n_prefix_encoding = 1;
  enc_item.item_encoding = &enc_arr[0];
  enc_item.item_encoding_set = 1;

  json_set_allocation_functions(mock_parson_oom_malloc, mock_parson_oom_free);
  g_parson_oom_fail_at = 0;
  write_encoding_object(obj, &enc_item);
  g_parson_oom_fail_at = 1;
  write_encoding_object(obj, &enc_item);
  g_parson_oom_fail_at = 2;
  write_encoding_object(obj, &enc_item);
  json_set_allocation_functions(malloc, free);

  /* 8. write_encoding_map OOM branches */
  json_set_allocation_functions(mock_parson_oom_malloc, mock_parson_oom_free);
  g_parson_oom_fail_at = 0;
  write_encoding_map(obj, enc_arr, 1);
  g_parson_oom_fail_at = 1;
  write_encoding_map(obj, enc_arr, 1);
  json_set_allocation_functions(malloc, free);

  /* 9. write_media_type_object OOM branches */
  memset(&mt, 0, sizeof(mt));
  mt.encoding = enc_arr;
  mt.n_encoding = 1;
  mt.prefix_encoding = enc_arr;
  mt.n_prefix_encoding = 1;
  mt.item_encoding = &enc_arr[0];
  mt.item_encoding_set = 1;

  json_set_allocation_functions(mock_parson_oom_malloc, mock_parson_oom_free);
  g_parson_oom_fail_at = 0;
  write_media_type_object(obj, &mt);
  g_parson_oom_fail_at = 1;
  write_media_type_object(obj, &mt);
  json_set_allocation_functions(malloc, free);

  /* 10. write_request_body_object OOM branches */
  memset(&rb, 0, sizeof(rb));
  rb.content_ref = (char *)(size_t) "#/components/media/Ref";
  json_set_allocation_functions(mock_parson_oom_malloc, mock_parson_oom_free);
  g_parson_oom_fail_at = 0;
  write_request_body_object(obj, &rb);
  g_parson_oom_fail_at = 1;
  write_request_body_object(obj, &rb);
  json_set_allocation_functions(malloc, free);

  memset(&rb, 0, sizeof(rb));
  json_set_allocation_functions(mock_parson_oom_malloc, mock_parson_oom_free);
  g_parson_oom_fail_at = 0;
  write_request_body_object(obj, &rb);
  g_parson_oom_fail_at = 1;
  write_request_body_object(obj, &rb);
  json_set_allocation_functions(malloc, free);

  json_value_free(val);
  PASS();
}

TEST test_openapi_writer_close_all_remaining_branches(void) {
  JSON_Value *val;
  JSON_Object *obj;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Spec empty_spec;
  struct OpenAPI_Path path;
  struct OpenAPI_Operation op;
  struct OpenAPI_Parameter param;
  struct OpenAPI_Response resp;
  struct OpenAPI_MediaType mt;
  struct OpenAPI_Callback cb;
  struct OpenAPI_Link link;
  struct OpenAPI_Server srv;
  struct OpenAPI_SecurityRequirementSet sec_set;
  struct OpenAPI_SecurityRequirement sec_req;
  struct OpenAPI_RequestBody rb;
  char *scopes[1];
  char *names[1];
  char *json = NULL;
  int k;

  val = json_value_init_object();
  obj = json_value_get_object(val);

  /* 1. Direct null spec calls */
  write_paths(obj, NULL);
  ASSERT_EQ(CDD_C_SUCCESS, write_servers(obj, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, write_webhooks(obj, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, write_request_body(obj, NULL));

  /* 2. Empty count on security requirements */
  ASSERT_EQ(CDD_C_SUCCESS, write_security_requirements(obj, "sec", NULL, 0, 1));

  /* 3. Empty security schemes */
  memset(&empty_spec, 0, sizeof(empty_spec));
  ASSERT_EQ(CDD_C_SUCCESS, write_security_schemes(obj, &empty_spec));

  /* 4. write_example_fields OOM on write_examples_object */
  {
    struct OpenAPI_Example exs[1];
    memset(exs, 0, sizeof(exs));
    exs[0].name = (char *)(size_t) "ex";
    json_set_allocation_functions(mock_parson_oom_malloc, mock_parson_oom_free);
    g_parson_oom_fail_at = 0;
    write_example_fields(obj, NULL, 0, exs, 1);
    json_set_allocation_functions(malloc, free);
  }

  /* 5. write_xml_object OOM */
  {
    struct OpenAPI_Xml xml;
    memset(&xml, 0, sizeof(xml));
    json_set_allocation_functions(mock_parson_oom_malloc, mock_parson_oom_free);
    g_parson_oom_fail_at = 0;
    write_xml_object(obj, &xml, 1);
    json_set_allocation_functions(malloc, free);
  }

  /* 6. write_schema_ref examples OOM */
  {
    struct OpenAPI_SchemaRef ref_ex;
    struct OpenAPI_Any any_ex[1];
    memset(&ref_ex, 0, sizeof(ref_ex));
    memset(any_ex, 0, sizeof(any_ex));
    any_ex[0].type = OA_ANY_NUMBER;
    any_ex[0].number = 1.0;
    ref_ex.examples = any_ex;
    ref_ex.n_examples = 1;
    json_set_allocation_functions(mock_parson_oom_malloc, mock_parson_oom_free);
    g_parson_oom_fail_at = 1;
    write_schema_ref(obj, "ex_ref", &ref_ex);
    json_set_allocation_functions(malloc, free);
  }

  /* 7. Item encoding and prefix encoding OOM */
  {
    struct OpenAPI_Encoding enc_item;
    struct OpenAPI_Encoding item_enc;
    memset(&enc_item, 0, sizeof(enc_item));
    memset(&item_enc, 0, sizeof(item_enc));
    item_enc.name = (char *)(size_t) "sub";
    enc_item.item_encoding = &item_enc;
    enc_item.item_encoding_set = 1;
    enc_item.prefix_encoding = &item_enc;
    enc_item.n_prefix_encoding = 1;

    json_set_allocation_functions(mock_parson_oom_malloc, mock_parson_oom_free);
    for (k = 0; k < 6; ++k) {
      g_parson_oom_fail_at = k;
      write_encoding_object(obj, &enc_item);
      write_encoding_map(obj, &enc_item, 1);
      write_encoding_array(obj, "enc_arr", &enc_item, 1);
    }
    json_set_allocation_functions(malloc, free);
  }

  /* 8. Media type item_encoding OOM and map fail */
  {
    struct OpenAPI_MediaType mt_item;
    struct OpenAPI_Encoding item_enc;
    memset(&mt_item, 0, sizeof(mt_item));
    memset(&item_enc, 0, sizeof(item_enc));
    mt_item.name = (char *)(size_t) "application/json";
    mt_item.item_encoding = &item_enc;
    mt_item.item_encoding_set = 1;
    mt_item.prefix_encoding = &item_enc;
    mt_item.n_prefix_encoding = 1;

    json_set_allocation_functions(mock_parson_oom_malloc, mock_parson_oom_free);
    for (k = 0; k < 6; ++k) {
      g_parson_oom_fail_at = k;
      write_media_type_object(obj, &mt_item);
      write_media_type_map(obj, "content", &mt_item, 1);
    }
    json_set_allocation_functions(malloc, free);
  }

  /* 9. write_request_body_object media_val failure */
  {
    struct OpenAPI_RequestBody rb_fail;
    memset(&rb_fail, 0, sizeof(rb_fail));
    rb_fail.content_ref = (char *)(size_t) "#/components/media/Ref";
    json_set_allocation_functions(mock_parson_oom_malloc, mock_parson_oom_free);
    g_parson_oom_fail_at = 1;
    write_request_body_object(obj, &rb_fail);
    json_set_allocation_functions(malloc, free);
  }

  /* 10. write_callback_object with failing write_path_item_object */
  {
    struct OpenAPI_Callback cb_fail;
    struct OpenAPI_Path cb_fail_path[1];
    struct OpenAPI_Operation cb_fail_op[1];
    struct OpenAPI_Parameter cb_fail_p[1];
    memset(&cb_fail, 0, sizeof(cb_fail));
    memset(cb_fail_path, 0, sizeof(cb_fail_path));
    memset(cb_fail_op, 0, sizeof(cb_fail_op));
    memset(cb_fail_p, 0, sizeof(cb_fail_p));
    cb_fail_p[0].name = (char *)(size_t) "q";
    cb_fail_op[0].verb = OA_VERB_GET;
    cb_fail_op[0].parameters = cb_fail_p;
    cb_fail_op[0].n_parameters = 1;
    cb_fail_path[0].route = (char *)(size_t) "/cb";
    cb_fail_path[0].operations = cb_fail_op;
    cb_fail_path[0].n_operations = 1;
    cb_fail.paths = cb_fail_path;
    cb_fail.n_paths = 1;

    json_set_allocation_functions(mock_parson_oom_malloc, mock_parson_oom_free);
    for (k = 0; k < 8; ++k) {
      g_parson_oom_fail_at = k;
      write_callback_object(obj, &cb_fail);
    }
    json_set_allocation_functions(malloc, free);
  }

  /* 11. Build full self-contained spec for all component and root returns */
  memset(&spec, 0, sizeof(spec));
  spec.openapi_version = (char *)(size_t) "3.2.0";
  spec.info.title = (char *)(size_t) "Complete Coverage API";
  spec.info.version = (char *)(size_t) "1.0.0";

  memset(&srv, 0, sizeof(srv));
  srv.url = (char *)(size_t) "https://srv.example.com";

  memset(&sec_req, 0, sizeof(sec_req));
  sec_req.scheme = (char *)(size_t) "bearer";
  scopes[0] = (char *)(size_t) "read";
  sec_req.scopes = scopes;
  sec_req.n_scopes = 1;
  memset(&sec_set, 0, sizeof(sec_set));
  sec_set.requirements = &sec_req;
  sec_set.n_requirements = 1;

  memset(&cb, 0, sizeof(cb));
  cb.name = (char *)(size_t) "callMe";

  memset(&link, 0, sizeof(link));
  link.name = (char *)(size_t) "linkMe";

  memset(&resp, 0, sizeof(resp));
  resp.code = (char *)(size_t) "200";
  resp.description = (char *)(size_t) "OK";

  memset(&param, 0, sizeof(param));
  param.name = (char *)(size_t) "p";
  param.in = OA_PARAM_IN_QUERY;

  memset(&rb, 0, sizeof(rb));
  rb.description = (char *)(size_t) "RB";

  memset(&op, 0, sizeof(op));
  op.verb = OA_VERB_GET;
  op.responses = &resp;
  op.n_responses = 1;
  op.parameters = &param;
  op.n_parameters = 1;
  op.security = &sec_set;
  op.n_security = 1;
  op.security_set = 1;
  op.servers = &srv;
  op.n_servers = 1;
  op.callbacks = &cb;
  op.n_callbacks = 1;
  op.req_body.ref_name = (char *)(size_t) "BodyModel";

  memset(&path, 0, sizeof(path));
  path.route = (char *)(size_t) "/items";
  path.operations = &op;
  path.n_operations = 1;
  path.additional_operations = &op;
  path.n_additional_operations = 1;
  path.parameters = &param;
  path.n_parameters = 1;
  path.servers = &srv;
  path.n_servers = 1;

  names[0] = (char *)(size_t) "elem";
  memset(&mt, 0, sizeof(mt));
  mt.name = (char *)(size_t) "application/json";

  spec.paths = &path;
  spec.n_paths = 1;
  spec.webhooks = &path;
  spec.n_webhooks = 1;
  spec.component_links = &link;
  spec.n_component_links = 1;
  spec.component_callbacks = &cb;
  spec.n_component_callbacks = 1;
  spec.component_path_items = &path;
  spec.component_path_item_names = names;
  spec.n_component_path_items = 1;
  spec.component_media_types = &mt;
  spec.component_media_type_names = names;
  spec.n_component_media_types = 1;

  /* Sweep write_operation_object and write_path_item_object error returns */
  json_set_allocation_functions(mock_parson_oom_malloc, mock_parson_oom_free);
  for (k = 0; k < 25; ++k) {
    g_parson_oom_fail_at = k;
    write_operation_object(obj, &op);
    write_path_item_object(obj, &path);
    write_component_media_types(obj, &spec);
    write_component_links(obj, &spec);
    write_component_callbacks(obj, &spec);
    write_component_path_items(obj, &spec);
    write_components(obj, &spec);
    openapi_write_spec_to_json(&spec, &json);
    if (json) {
      free(json);
      json = NULL;
    }
  }
  json_set_allocation_functions(malloc, free);

  json_value_free(val);
  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_OPENAPI_WRITER_OOM_H */
