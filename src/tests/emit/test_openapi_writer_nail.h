/**
 * @file test_openapi_writer_nail.h
 * @brief Unit tests for OpenAPI Writer (nail).
 */

#ifndef TEST_OPENAPI_WRITER_NAIL_H
#define TEST_OPENAPI_WRITER_NAIL_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "emit/test_openapi_writer_common.h"
/* clang-format on */

TEST test_openapi_writer_nail_all_remaining(void) {
  JSON_Value *val;
  JSON_Object *obj;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Discriminator disc;
  struct OpenAPI_SchemaRef ref_ex;
  struct OpenAPI_Any any_ex[1];
  struct OpenAPI_Parameter p;
  struct OpenAPI_Header h_arr[1];
  struct OpenAPI_Response resp;
  struct OpenAPI_Link link[1];
  struct OpenAPI_RequestBody rb_fail;
  struct OpenAPI_MediaType mt[1];
  struct OpenAPI_Operation op;
  struct OpenAPI_Response resp_one[1];
  struct OpenAPI_Callback cb[1];
  struct OpenAPI_Server srv[1];
  struct OpenAPI_Path path;
  struct StructFields defined_sch[1];
  char *def_names[1];
  struct OpenAPI_SecurityRequirementSet sec_set;
  struct OpenAPI_SecurityRequirement sec_req;
  char *scopes[1];
  char *names[1];
  char *json = NULL;
  int k;

  names[0] = (char *)(size_t) "test";
  def_names[0] = (char *)(size_t) "MyStruct";

  /* 1. Line 372-378 */
  for (k = 0; k < 50; ++k) {
    val = json_value_init_object();
    obj = json_value_get_object(val);
    json_set_allocation_functions(mock_parson_oom_malloc, mock_parson_oom_free);
    g_parson_oom_fail_at = k;
    merge_schema_extras_object_openapi(obj, "{\"foo\": 123}");
    json_set_allocation_functions(malloc, free);
    json_value_free(val);
  }

  /* 2. Line 713 */
  val = json_value_init_object();
  obj = json_value_get_object(val);
  memset(&disc, 0, sizeof(disc));
  disc.property_name = (char *)(size_t) "type";
  json_set_allocation_functions(mock_parson_oom_malloc, mock_parson_oom_free);
  g_parson_oom_fail_at = 0;
  write_discriminator_object(obj, &disc, 1);
  json_set_allocation_functions(malloc, free);
  json_value_free(val);

  /* 3. Line 1294 */
  memset(&ref_ex, 0, sizeof(ref_ex));
  memset(any_ex, 0, sizeof(any_ex));
  any_ex[0].type = OA_ANY_NUMBER;
  any_ex[0].number = 1.0;
  ref_ex.examples = any_ex;
  ref_ex.n_examples = 1;
  for (k = 0; k < 10; ++k) {
    val = json_value_init_object();
    obj = json_value_get_object(val);
    json_set_allocation_functions(mock_parson_oom_malloc, mock_parson_oom_free);
    g_parson_oom_fail_at = k;
    write_schema_ref(obj, "ex_ref", &ref_ex);
    json_set_allocation_functions(malloc, free);
    json_value_free(val);
  }

  /* 4. Line 1430 */
  val = json_value_init_object();
  obj = json_value_get_object(val);
  memset(&p, 0, sizeof(p));
  p.content_ref = (char *)(size_t) "#/ref";
  p.content_type = (char *)(size_t) "application/xml";
  write_parameter_object(obj, &p);
  json_value_free(val);

  /* 5. Lines 1664, 1748, 1851, 1889 */
  write_encoding_array(NULL, "k", NULL, 0);
  write_media_type_map(NULL, "k", NULL, 0);
  write_headers(NULL, NULL);
  val = json_value_init_object();
  obj = json_value_get_object(val);
  memset(&resp, 0, sizeof(resp));
  write_headers(obj, &resp);
  write_links(obj, &resp);
  json_value_free(val);

  /* 6. Lines 1855-1856 */
  memset(h_arr, 0, sizeof(h_arr));
  h_arr[0].name = (char *)(size_t) "X-H";
  for (k = 0; k < 5; ++k) {
    val = json_value_init_object();
    obj = json_value_get_object(val);
    json_set_allocation_functions(mock_parson_oom_malloc, mock_parson_oom_free);
    g_parson_oom_fail_at = k;
    write_headers_map(obj, "headers", h_arr, 1, 0);
    json_set_allocation_functions(malloc, free);
    json_value_free(val);
  }

  /* 7. Lines 1904, 1908-1909 */
  memset(&resp, 0, sizeof(resp));
  memset(link, 0, sizeof(link));
  link[0].ref = (char *)(size_t) "#/ref";
  resp.links = link;
  resp.n_links = 1;
  for (k = 0; k < 5; ++k) {
    val = json_value_init_object();
    obj = json_value_get_object(val);
    json_set_allocation_functions(mock_parson_oom_malloc, mock_parson_oom_free);
    g_parson_oom_fail_at = k;
    write_links(obj, &resp);
    json_set_allocation_functions(malloc, free);
    json_value_free(val);
  }

  /* 8. Lines 2051, 2063-2064 */
  memset(&rb_fail, 0, sizeof(rb_fail));
  rb_fail.content_ref = (char *)(size_t) "#/ref";
  for (k = 0; k < 6; ++k) {
    val = json_value_init_object();
    obj = json_value_get_object(val);
    json_set_allocation_functions(mock_parson_oom_malloc, mock_parson_oom_free);
    g_parson_oom_fail_at = k;
    write_request_body_object(obj, &rb_fail);
    json_set_allocation_functions(malloc, free);
    json_value_free(val);
  }

  memset(&rb_fail, 0, sizeof(rb_fail));
  memset(mt, 0, sizeof(mt));
  mt[0].name = (char *)(size_t) "application/json";
  rb_fail.content_media_types = mt;
  rb_fail.n_content_media_types = 1;
  for (k = 0; k < 6; ++k) {
    val = json_value_init_object();
    obj = json_value_get_object(val);
    json_set_allocation_functions(mock_parson_oom_malloc, mock_parson_oom_free);
    g_parson_oom_fail_at = k;
    write_request_body_object(obj, &rb_fail);
    json_set_allocation_functions(malloc, free);
    json_value_free(val);
  }

  /* 9. Lines 2127-2128, 2148-2149, 2163-2164 */
  memset(&op, 0, sizeof(op));
  op.req_body_ref = (char *)(size_t) "#/ref";
  for (k = 0; k < 5; ++k) {
    val = json_value_init_object();
    obj = json_value_get_object(val);
    json_set_allocation_functions(mock_parson_oom_malloc, mock_parson_oom_free);
    g_parson_oom_fail_at = k;
    write_request_body(obj, &op);
    json_set_allocation_functions(malloc, free);
    json_value_free(val);
  }

  memset(&op, 0, sizeof(op));
  op.req_body.ref_name = (char *)(size_t) "BodyModel";
  for (k = 0; k < 5; ++k) {
    val = json_value_init_object();
    obj = json_value_get_object(val);
    json_set_allocation_functions(mock_parson_oom_malloc, mock_parson_oom_free);
    g_parson_oom_fail_at = k;
    write_request_body(obj, &op);
    json_set_allocation_functions(malloc, free);
    json_value_free(val);
  }

  /* 10. Lines 2227-2228 */
  memset(&op, 0, sizeof(op));
  memset(cb, 0, sizeof(cb));
  cb[0].name = (char *)(size_t) "cb";
  op.callbacks = cb;
  op.n_callbacks = 1;
  for (k = 0; k < 5; ++k) {
    val = json_value_init_object();
    obj = json_value_get_object(val);
    json_set_allocation_functions(mock_parson_oom_malloc, mock_parson_oom_free);
    g_parson_oom_fail_at = k;
    write_callbacks(obj, &op);
    json_set_allocation_functions(malloc, free);
    json_value_free(val);
  }

  /* 11. Lines 2335, 2345, 2351 */
  memset(&op, 0, sizeof(op));
  op.verb = OA_VERB_GET;
  memset(resp_one, 0, sizeof(resp_one));
  resp_one[0].code = (char *)(size_t) "200";
  resp_one[0].description = (char *)(size_t) "OK";
  op.responses = resp_one;
  op.n_responses = 1;
  op.callbacks = cb;
  op.n_callbacks = 1;
  memset(srv, 0, sizeof(srv));
  srv[0].url = (char *)(size_t) "https://srv";
  op.servers = srv;
  op.n_servers = 1;

  for (k = 0; k < 30; ++k) {
    val = json_value_init_object();
    obj = json_value_get_object(val);
    json_set_allocation_functions(mock_parson_oom_malloc, mock_parson_oom_free);
    g_parson_oom_fail_at = k;
    write_operation_object(obj, &op);
    json_set_allocation_functions(malloc, free);
    json_value_free(val);
  }

  /* 12. Lines 2464, 2470 */
  memset(&path, 0, sizeof(path));
  path.route = (char *)(size_t) "/items";
  path.operations = &op;
  path.n_operations = 1;
  path.additional_operations = &op;
  path.n_additional_operations = 1;
  path.servers = srv;
  path.n_servers = 1;

  for (k = 0; k < 30; ++k) {
    val = json_value_init_object();
    obj = json_value_get_object(val);
    json_set_allocation_functions(mock_parson_oom_malloc, mock_parson_oom_free);
    g_parson_oom_fail_at = k;
    write_operations(obj, &path);
    write_path_item_object(obj, &path);
    json_set_allocation_functions(malloc, free);
    json_value_free(val);
  }

  /* 13. Lines 2522, 2528, 2559-2560 */
  memset(&spec, 0, sizeof(spec));
  spec.paths = &path;
  spec.n_paths = 1;
  for (k = 0; k < 20; ++k) {
    val = json_value_init_object();
    obj = json_value_get_object(val);
    json_set_allocation_functions(mock_parson_oom_malloc, mock_parson_oom_free);
    g_parson_oom_fail_at = k;
    write_paths(obj, &spec);
    json_set_allocation_functions(malloc, free);
    json_value_free(val);
  }

  /* 14. Lines 2647-2648, 2665, 2673 */
  memset(&spec, 0, sizeof(spec));
  spec.webhooks = &path;
  spec.n_webhooks = 1;
  for (k = 0; k < 20; ++k) {
    val = json_value_init_object();
    obj = json_value_get_object(val);
    json_set_allocation_functions(mock_parson_oom_malloc, mock_parson_oom_free);
    g_parson_oom_fail_at = k;
    write_webhooks(obj, &spec);
    json_set_allocation_functions(malloc, free);
    json_value_free(val);
  }

  /* 15. Lines 3168-3170 */
  memset(&spec, 0, sizeof(spec));
  spec.component_path_items = &path;
  spec.component_path_item_names = names;
  spec.n_component_path_items = 1;
  for (k = 0; k < 20; ++k) {
    val = json_value_init_object();
    obj = json_value_get_object(val);
    json_set_allocation_functions(mock_parson_oom_malloc, mock_parson_oom_free);
    g_parson_oom_fail_at = k;
    write_component_path_items(obj, &spec);
    json_set_allocation_functions(malloc, free);
    json_value_free(val);
  }

  /* 16. Lines 3267-3269 */
  memset(&spec, 0, sizeof(spec));
  struct_fields_init(&defined_sch[0]);
  spec.defined_schemas = defined_sch;
  spec.defined_schema_names = def_names;
  spec.n_defined_schemas = 1;
  for (k = 0; k < 20; ++k) {
    val = json_value_init_object();
    obj = json_value_get_object(val);
    json_set_allocation_functions(mock_parson_oom_malloc, mock_parson_oom_free);
    g_parson_oom_fail_at = k;
    write_components(obj, &spec);
    json_set_allocation_functions(malloc, free);
    json_value_free(val);
  }

  /* 17. Lines 3419-3445 */
  memset(&sec_req, 0, sizeof(sec_req));
  sec_req.scheme = (char *)(size_t) "bearer";
  scopes[0] = (char *)(size_t) "read";
  sec_req.scopes = scopes;
  sec_req.n_scopes = 1;
  memset(&sec_set, 0, sizeof(sec_set));
  sec_set.requirements = &sec_req;
  sec_set.n_requirements = 1;

  memset(&spec, 0, sizeof(spec));
  spec.openapi_version = (char *)(size_t) "3.2.0";
  spec.security = &sec_set;
  spec.n_security = 1;
  spec.security_set = 1;
  spec.servers = srv;
  spec.n_servers = 1;
  spec.webhooks = &path;
  spec.n_webhooks = 1;
  spec.paths = &path;
  spec.n_paths = 1;
  for (k = 0; k < 80; ++k) {
    json_set_allocation_functions(mock_parson_oom_malloc, mock_parson_oom_free);
    g_parson_oom_fail_at = k;
    openapi_write_spec_to_json(&spec, &json);
    if (json) {
      free(json);
      json = NULL;
    }
    json_set_allocation_functions(malloc, free);
  }

  /* Precise triggers for final 4 lines */
  /* Line 1851 */
  write_headers_map(NULL, "k", NULL, 0, 0);

  /* Line 2335: write_operation_object callbacks failure (k up to 35) */
  {
    struct OpenAPI_Operation op_cb;
    memset(&op_cb, 0, sizeof(op_cb));
    op_cb.callbacks = cb;
    op_cb.n_callbacks = 1;
    for (k = 0; k < 35; ++k) {
      val = json_value_init_object();
      obj = json_value_get_object(val);
      json_set_allocation_functions(mock_parson_oom_malloc,
                                    mock_parson_oom_free);
      g_parson_oom_fail_at = k;
      write_operation_object(obj, &op_cb);
      json_set_allocation_functions(malloc, free);
      json_value_free(val);
    }
  }

  /* Line 2351: write_operation_object servers failure */
  {
    struct OpenAPI_Operation op_srv;
    memset(&op_srv, 0, sizeof(op_srv));
    op_srv.servers = srv;
    op_srv.n_servers = 1;
    for (k = 0; k < 20; ++k) {
      val = json_value_init_object();
      obj = json_value_get_object(val);
      json_set_allocation_functions(mock_parson_oom_malloc,
                                    mock_parson_oom_free);
      g_parson_oom_fail_at = k;
      write_operation_object(obj, &op_srv);
      json_set_allocation_functions(malloc, free);
      json_value_free(val);
    }
  }

  /* Line 2464: write_path_item_object additional_operations failure (operations
   * must be NULL) */
  {
    struct OpenAPI_Path p_add_fail;
    struct OpenAPI_Operation op_f[1];
    memset(&p_add_fail, 0, sizeof(p_add_fail));
    memset(op_f, 0, sizeof(op_f));
    op_f[0].verb = OA_VERB_GET;
    op_f[0].callbacks = cb;
    op_f[0].n_callbacks = 1;
    p_add_fail.additional_operations = op_f;
    p_add_fail.n_additional_operations = 1;
    for (k = 0; k < 25; ++k) {
      val = json_value_init_object();
      obj = json_value_get_object(val);
      json_set_allocation_functions(mock_parson_oom_malloc,
                                    mock_parson_oom_free);
      g_parson_oom_fail_at = k;
      write_path_item_object(obj, &p_add_fail);
      json_set_allocation_functions(malloc, free);
      json_value_free(val);
    }
  }

  /* Line 3444-3445: openapi_write_spec_to_json paths failure (k up to 100) */
  {
    struct OpenAPI_Spec spec_only_paths;
    struct OpenAPI_Path p_item[1];
    memset(&spec_only_paths, 0, sizeof(spec_only_paths));
    memset(p_item, 0, sizeof(p_item));
    p_item[0].route = (char *)(size_t) "/items";
    spec_only_paths.paths = p_item;
    spec_only_paths.n_paths = 1;
    for (k = 10; k < 35; ++k) {
      json_set_allocation_functions(mock_parson_oom_malloc,
                                    mock_parson_oom_free);
      g_parson_oom_fail_at = k;
      openapi_write_spec_to_json(&spec_only_paths, &json);
      if (json) {
        free(json);
        json = NULL;
      }
      json_set_allocation_functions(malloc, free);
    }
  }

  /* Exact triggers for 2335, 2464, 3444-3445 */
  {
    struct OpenAPI_Operation op_rb;
    memset(&op_rb, 0, sizeof(op_rb));
    op_rb.req_body_ref = (char *)(size_t) "#/ref";
    val = json_value_init_object();
    obj = json_value_get_object(val);
    json_set_allocation_functions(mock_parson_oom_malloc, mock_parson_oom_free);
    g_parson_oom_fail_at = 0;
    write_operation_object(obj, &op_rb);
    json_set_allocation_functions(malloc, free);
    json_value_free(val);
  }

  {
    struct OpenAPI_Path p_param_fail;
    struct OpenAPI_Parameter param_item[1];
    memset(&p_param_fail, 0, sizeof(p_param_fail));
    memset(param_item, 0, sizeof(param_item));
    param_item[0].name = (char *)(size_t) "p";
    param_item[0].in = OA_PARAM_IN_QUERY;
    p_param_fail.parameters = param_item;
    p_param_fail.n_parameters = 1;
    val = json_value_init_object();
    obj = json_value_get_object(val);
    json_set_allocation_functions(mock_parson_oom_malloc, mock_parson_oom_free);
    g_parson_oom_fail_at = 0;
    write_path_item_object(obj, &p_param_fail);
    json_set_allocation_functions(malloc, free);
    json_value_free(val);
  }

  {
    struct OpenAPI_Spec s_p_fail;
    struct OpenAPI_Path p_p_fail[1];
    struct OpenAPI_Parameter param_item[1];
    memset(&s_p_fail, 0, sizeof(s_p_fail));
    memset(p_p_fail, 0, sizeof(p_p_fail));
    memset(param_item, 0, sizeof(param_item));
    param_item[0].name = (char *)(size_t) "p";
    param_item[0].in = OA_PARAM_IN_QUERY;
    p_p_fail[0].route = (char *)(size_t) "/items";
    p_p_fail[0].parameters = param_item;
    p_p_fail[0].n_parameters = 1;
    s_p_fail.paths = p_p_fail;
    s_p_fail.n_paths = 1;
    for (k = 0; k < 60; ++k) {
      json_set_allocation_functions(mock_parson_oom_malloc,
                                    mock_parson_oom_free);
      g_parson_oom_fail_at = k;
      openapi_write_spec_to_json(&s_p_fail, &json);
      if (json) {
        free(json);
        json = NULL;
      }
      json_set_allocation_functions(malloc, free);
    }
  }

  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_OPENAPI_WRITER_NAIL_H */
