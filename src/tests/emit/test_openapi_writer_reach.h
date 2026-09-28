/**
 * @file test_openapi_writer_reach.h
 * @brief Unit tests for OpenAPI Writer (reach).
 */

#ifndef TEST_OPENAPI_WRITER_REACH_H
#define TEST_OPENAPI_WRITER_REACH_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "emit/test_openapi_writer_common.h"
/* clang-format on */

TEST test_openapi_writer_component_returns_and_root_oom(void) {
  JSON_Value *val;
  JSON_Object *obj;
  struct OpenAPI_Spec s;
  struct OpenAPI_Link link;
  struct OpenAPI_Callback cb;
  struct OpenAPI_Path path;
  struct OpenAPI_MediaType mt;
  struct OpenAPI_Example ex;
  struct OpenAPI_Header hdr;
  struct OpenAPI_Response resp;
  struct OpenAPI_Parameter param;
  struct OpenAPI_RequestBody rb;
  struct OpenAPI_SecurityScheme sec;
  struct OpenAPI_Server srv;
  struct OpenAPI_SecurityRequirementSet sec_set;
  struct OpenAPI_SecurityRequirement sec_req;
  char *names[1];
  char *json = NULL;
  int k_idx;

  val = json_value_init_object();
  obj = json_value_get_object(val);
  names[0] = (char *)(size_t) "item";

  memset(&link, 0, sizeof(link));
  memset(&cb, 0, sizeof(cb));
  memset(&path, 0, sizeof(path));
  memset(&mt, 0, sizeof(mt));
  memset(&ex, 0, sizeof(ex));
  memset(&hdr, 0, sizeof(hdr));
  memset(&resp, 0, sizeof(resp));
  memset(&param, 0, sizeof(param));
  memset(&rb, 0, sizeof(rb));
  memset(&sec, 0, sizeof(sec));
  memset(&srv, 0, sizeof(srv));
  memset(&sec_set, 0, sizeof(sec_set));
  memset(&sec_req, 0, sizeof(sec_req));

  path.route = (char *)(size_t) "/route";

  json_set_allocation_functions(mock_parson_oom_malloc, mock_parson_oom_free);

  /* 1. Each component failure across k = 0..12 in write_components */
  memset(&s, 0, sizeof(s));
  s.security_schemes = &sec;
  s.n_security_schemes = 1;
  for (k_idx = 0; k_idx < 12; ++k_idx) {
    g_parson_oom_fail_at = k_idx;
    write_components(obj, &s);
  }

  memset(&s, 0, sizeof(s));
  s.component_parameters = &param;
  s.component_parameter_names = names;
  s.n_component_parameters = 1;
  for (k_idx = 0; k_idx < 12; ++k_idx) {
    g_parson_oom_fail_at = k_idx;
    write_components(obj, &s);
  }

  memset(&s, 0, sizeof(s));
  s.component_responses = &resp;
  s.component_response_names = names;
  s.n_component_responses = 1;
  for (k_idx = 0; k_idx < 12; ++k_idx) {
    g_parson_oom_fail_at = k_idx;
    write_components(obj, &s);
  }

  memset(&s, 0, sizeof(s));
  s.component_headers = &hdr;
  s.component_header_names = names;
  s.n_component_headers = 1;
  for (k_idx = 0; k_idx < 12; ++k_idx) {
    g_parson_oom_fail_at = k_idx;
    write_components(obj, &s);
  }

  memset(&s, 0, sizeof(s));
  s.component_request_bodies = &rb;
  s.component_request_body_names = names;
  s.n_component_request_bodies = 1;
  for (k_idx = 0; k_idx < 12; ++k_idx) {
    g_parson_oom_fail_at = k_idx;
    write_components(obj, &s);
  }

  memset(&s, 0, sizeof(s));
  s.component_media_types = &mt;
  s.component_media_type_names = names;
  s.n_component_media_types = 1;
  for (k_idx = 0; k_idx < 12; ++k_idx) {
    g_parson_oom_fail_at = k_idx;
    write_components(obj, &s);
  }

  memset(&s, 0, sizeof(s));
  s.component_examples = &ex;
  s.component_example_names = names;
  s.n_component_examples = 1;
  for (k_idx = 0; k_idx < 12; ++k_idx) {
    g_parson_oom_fail_at = k_idx;
    write_components(obj, &s);
  }

  memset(&s, 0, sizeof(s));
  s.component_links = &link;
  s.n_component_links = 1;
  for (k_idx = 0; k_idx < 12; ++k_idx) {
    g_parson_oom_fail_at = k_idx;
    write_components(obj, &s);
  }

  memset(&s, 0, sizeof(s));
  s.component_callbacks = &cb;
  s.n_component_callbacks = 1;
  for (k_idx = 0; k_idx < 12; ++k_idx) {
    g_parson_oom_fail_at = k_idx;
    write_components(obj, &s);
  }

  memset(&s, 0, sizeof(s));
  s.component_path_items = &path;
  s.component_path_item_names = names;
  s.n_component_path_items = 1;
  for (k_idx = 0; k_idx < 12; ++k_idx) {
    g_parson_oom_fail_at = k_idx;
    write_components(obj, &s);
  }

  /* 2. Each root step failure in openapi_write_spec_to_json across k = 0..15 */
  memset(&s, 0, sizeof(s));
  sec_req.scheme = (char *)(size_t) "oauth2";
  sec_set.requirements = &sec_req;
  sec_set.n_requirements = 1;
  s.security = &sec_set;
  s.n_security = 1;
  s.security_set = 1;
  for (k_idx = 0; k_idx < 15; ++k_idx) {
    g_parson_oom_fail_at = k_idx;
    openapi_write_spec_to_json(&s, &json);
    if (json) {
      free(json);
      json = NULL;
    }
  }

  memset(&s, 0, sizeof(s));
  srv.url = (char *)(size_t) "https://example.com";
  s.servers = &srv;
  s.n_servers = 1;
  for (k_idx = 0; k_idx < 15; ++k_idx) {
    g_parson_oom_fail_at = k_idx;
    openapi_write_spec_to_json(&s, &json);
    if (json) {
      free(json);
      json = NULL;
    }
  }

  memset(&s, 0, sizeof(s));
  s.component_links = &link;
  s.n_component_links = 1;
  for (k_idx = 0; k_idx < 15; ++k_idx) {
    g_parson_oom_fail_at = k_idx;
    openapi_write_spec_to_json(&s, &json);
    if (json) {
      free(json);
      json = NULL;
    }
  }

  memset(&s, 0, sizeof(s));
  s.webhooks = &path;
  s.n_webhooks = 1;
  for (k_idx = 0; k_idx < 15; ++k_idx) {
    g_parson_oom_fail_at = k_idx;
    openapi_write_spec_to_json(&s, &json);
    if (json) {
      free(json);
      json = NULL;
    }
  }

  memset(&s, 0, sizeof(s));
  s.paths = &path;
  s.n_paths = 1;
  for (k_idx = 0; k_idx < 15; ++k_idx) {
    g_parson_oom_fail_at = k_idx;
    openapi_write_spec_to_json(&s, &json);
    if (json) {
      free(json);
      json = NULL;
    }
  }

  json_set_allocation_functions(malloc, free);
  json_value_free(val);
  PASS();
}

TEST test_openapi_writer_strike_last_19(void) {
  JSON_Value *val;
  JSON_Object *obj;
  struct OpenAPI_Encoding enc_parent;
  struct OpenAPI_Encoding enc_item;
  struct OpenAPI_Encoding enc_sub[1];
  struct OpenAPI_Encoding enc_map_arr[1];
  struct OpenAPI_MediaType mt_parent;
  struct OpenAPI_MediaType mt_map_arr[1];
  struct OpenAPI_Operation op_no_body;
  struct OpenAPI_Response resp_one[1];
  struct OpenAPI_Path path_ops;
  struct OpenAPI_Operation op_item[1];
  struct OpenAPI_Path path_only_add;
  struct OpenAPI_Operation op_add_item[1];
  struct OpenAPI_Spec spec_tags;
  struct OpenAPI_Tag tag[1];
  struct OpenAPI_Spec spec_comp_mt;
  char *names[1];
  char *json = NULL;
  int k;

  val = json_value_init_object();
  obj = json_value_get_object(val);
  names[0] = (char *)(size_t) "item";

  /* Setup multi-level encoding failure: enc_parent -> enc_item -> enc_sub */
  memset(&enc_parent, 0, sizeof(enc_parent));
  memset(&enc_item, 0, sizeof(enc_item));
  memset(enc_sub, 0, sizeof(enc_sub));
  enc_sub[0].name = (char *)(size_t) "leaf";
  enc_item.name = (char *)(size_t) "middle";
  enc_item.prefix_encoding = enc_sub;
  enc_item.n_prefix_encoding = 1;
  enc_parent.name = (char *)(size_t) "root";
  enc_parent.item_encoding = &enc_item;
  enc_parent.item_encoding_set = 1;

  memset(enc_map_arr, 0, sizeof(enc_map_arr));
  enc_map_arr[0] = enc_parent;

  memset(&mt_parent, 0, sizeof(mt_parent));
  mt_parent.name = (char *)(size_t) "application/json";
  mt_parent.item_encoding = &enc_item;
  mt_parent.item_encoding_set = 1;

  memset(mt_map_arr, 0, sizeof(mt_map_arr));
  mt_map_arr[0] = mt_parent;

  /* 1. Lines 1600-1601, 1643-1644, 1681-1682, 1727-1728, 1764-1766, 3005-3006
   */
  json_set_allocation_functions(mock_parson_oom_malloc, mock_parson_oom_free);
  for (k = 0; k < 12; ++k) {
    g_parson_oom_fail_at = k;
    write_encoding_object(obj, &enc_parent);
    write_encoding_map(obj, enc_map_arr, 1);
    write_encoding_array(obj, "arr", enc_map_arr, 1);
    write_media_type_object(obj, &mt_parent);
    write_media_type_map(obj, "content", mt_map_arr, 1);
  }

  memset(&spec_comp_mt, 0, sizeof(spec_comp_mt));
  spec_comp_mt.component_media_types = mt_map_arr;
  spec_comp_mt.component_media_type_names = names;
  spec_comp_mt.n_component_media_types = 1;
  for (k = 0; k < 12; ++k) {
    g_parson_oom_fail_at = k;
    write_component_media_types(obj, &spec_comp_mt);
  }
  json_set_allocation_functions(malloc, free);

  /* 2. Line 2325: write_operation_object responses failure */
  memset(&op_no_body, 0, sizeof(op_no_body));
  op_no_body.verb = OA_VERB_GET;
  memset(resp_one, 0, sizeof(resp_one));
  resp_one[0].code = (char *)(size_t) "200";
  resp_one[0].description = (char *)(size_t) "OK";
  op_no_body.responses = resp_one;
  op_no_body.n_responses = 1;

  json_set_allocation_functions(mock_parson_oom_malloc, mock_parson_oom_free);
  for (k = 0; k < 6; ++k) {
    g_parson_oom_fail_at = k;
    write_operation_object(obj, &op_no_body);
  }
  json_set_allocation_functions(malloc, free);

  /* 3. Line 2455: write_operations failure */
  memset(&path_ops, 0, sizeof(path_ops));
  path_ops.route = (char *)(size_t) "/ops";
  memset(op_item, 0, sizeof(op_item));
  op_item[0] = op_no_body;
  path_ops.operations = op_item;
  path_ops.n_operations = 1;

  json_set_allocation_functions(mock_parson_oom_malloc, mock_parson_oom_free);
  for (k = 0; k < 8; ++k) {
    g_parson_oom_fail_at = k;
    write_operations(obj, &path_ops);
  }
  json_set_allocation_functions(malloc, free);

  /* 4. Line 2464: write_path_item_object additional_operations failure */
  memset(&path_only_add, 0, sizeof(path_only_add));
  path_only_add.route = (char *)(size_t) "/add";
  memset(op_add_item, 0, sizeof(op_add_item));
  op_add_item[0] = op_no_body;
  path_only_add.additional_operations = op_add_item;
  path_only_add.n_additional_operations = 1;

  json_set_allocation_functions(mock_parson_oom_malloc, mock_parson_oom_free);
  for (k = 0; k < 8; ++k) {
    g_parson_oom_fail_at = k;
    write_path_item_object(obj, &path_only_add);
  }
  json_set_allocation_functions(malloc, free);

  /* 5. Line 2582-2583: write_tags array OOM & Line 3397: root write_tags
   * failure */
  memset(&spec_tags, 0, sizeof(spec_tags));
  memset(tag, 0, sizeof(tag));
  tag[0].name = (char *)(size_t) "TagA";
  spec_tags.tags = tag;
  spec_tags.n_tags = 1;

  json_set_allocation_functions(mock_parson_oom_malloc, mock_parson_oom_free);
  g_parson_oom_fail_at = 0;
  write_tags(obj, &spec_tags);

  for (k = 0; k < 15; ++k) {
    g_parson_oom_fail_at = k;
    openapi_write_spec_to_json(&spec_tags, &json);
    if (json) {
      free(json);
      json = NULL;
    }
  }
  json_set_allocation_functions(malloc, free);

  json_value_free(val);
  PASS();
}

TEST test_openapi_writer_nail_last_11(void) {
  JSON_Value *val;
  JSON_Object *obj;
  struct OpenAPI_Encoding enc_parent;
  struct OpenAPI_Encoding enc_item;
  struct OpenAPI_Encoding enc_sub;
  struct OpenAPI_MediaType mt_parent;
  struct OpenAPI_Operation op_no_body;
  struct OpenAPI_Response resp_one[1];
  struct OpenAPI_Path path_ops;
  struct OpenAPI_Operation op_item[1];
  struct OpenAPI_Spec spec_tags;
  struct OpenAPI_Tag tag[1];
  char *json = NULL;
  int k;

  /* Setup multi-level encoding */
  memset(&enc_parent, 0, sizeof(enc_parent));
  memset(&enc_item, 0, sizeof(enc_item));
  memset(&enc_sub, 0, sizeof(enc_sub));
  enc_sub.name = (char *)(size_t) "leaf";
  enc_item.name = (char *)(size_t) "middle";
  enc_item.prefix_encoding = &enc_sub;
  enc_item.n_prefix_encoding = 1;
  enc_parent.name = (char *)(size_t) "root";
  enc_parent.prefix_encoding = &enc_sub;
  enc_parent.n_prefix_encoding = 1;
  enc_parent.item_encoding = &enc_item;
  enc_parent.item_encoding_set = 1;
  enc_parent.encoding = &enc_sub;
  enc_parent.n_encoding = 1;

  memset(&mt_parent, 0, sizeof(mt_parent));
  mt_parent.name = (char *)(size_t) "application/json";
  mt_parent.prefix_encoding = &enc_sub;
  mt_parent.n_prefix_encoding = 1;
  mt_parent.item_encoding = &enc_item;
  mt_parent.item_encoding_set = 1;
  mt_parent.encoding = &enc_sub;
  mt_parent.n_encoding = 1;

  /* 1. Lines 1643-1644, 1681-1682, 1764-1766 with fresh JSON object each time
   */
  for (k = 0; k < 20; ++k) {
    val = json_value_init_object();
    obj = json_value_get_object(val);
    json_set_allocation_functions(mock_parson_oom_malloc, mock_parson_oom_free);
    g_parson_oom_fail_at = k;
    write_encoding_map(obj, &enc_parent, 1);
    json_set_allocation_functions(malloc, free);
    json_value_free(val);
  }
  for (k = 0; k < 20; ++k) {
    val = json_value_init_object();
    obj = json_value_get_object(val);
    json_set_allocation_functions(mock_parson_oom_malloc, mock_parson_oom_free);
    g_parson_oom_fail_at = k;
    write_encoding_array(obj, "arr", &enc_parent, 1);
    json_set_allocation_functions(malloc, free);
    json_value_free(val);
  }
  for (k = 0; k < 20; ++k) {
    val = json_value_init_object();
    obj = json_value_get_object(val);
    json_set_allocation_functions(mock_parson_oom_malloc, mock_parson_oom_free);
    g_parson_oom_fail_at = k;
    write_media_type_map(obj, "content", &mt_parent, 1);
    json_set_allocation_functions(malloc, free);
    json_value_free(val);
  }

  /* 2. Line 2325: write_operation_object responses failure */
  memset(&op_no_body, 0, sizeof(op_no_body));
  op_no_body.verb = OA_VERB_GET;
  memset(resp_one, 0, sizeof(resp_one));
  resp_one[0].code = (char *)(size_t) "200";
  resp_one[0].description = (char *)(size_t) "OK";
  op_no_body.responses = resp_one;
  op_no_body.n_responses = 1;

  for (k = 0; k < 10; ++k) {
    val = json_value_init_object();
    obj = json_value_get_object(val);
    json_set_allocation_functions(mock_parson_oom_malloc, mock_parson_oom_free);
    g_parson_oom_fail_at = k;
    write_operation_object(obj, &op_no_body);
    json_set_allocation_functions(malloc, free);
    json_value_free(val);
  }

  /* 3. Line 2455: write_operations failure */
  memset(&path_ops, 0, sizeof(path_ops));
  path_ops.route = (char *)(size_t) "/ops";
  memset(op_item, 0, sizeof(op_item));
  op_item[0] = op_no_body;
  path_ops.operations = op_item;
  path_ops.n_operations = 1;

  for (k = 0; k < 10; ++k) {
    val = json_value_init_object();
    obj = json_value_get_object(val);
    json_set_allocation_functions(mock_parson_oom_malloc, mock_parson_oom_free);
    g_parson_oom_fail_at = k;
    write_operations(obj, &path_ops);
    json_set_allocation_functions(malloc, free);
    json_value_free(val);
  }

  /* 4. Line 3397: root write_tags failure */
  memset(&spec_tags, 0, sizeof(spec_tags));
  memset(tag, 0, sizeof(tag));
  tag[0].name = (char *)(size_t) "TagA";
  spec_tags.tags = tag;
  spec_tags.n_tags = 1;

  for (k = 0; k < 25; ++k) {
    json_set_allocation_functions(mock_parson_oom_malloc, mock_parson_oom_free);
    g_parson_oom_fail_at = k;
    openapi_write_spec_to_json(&spec_tags, &json);
    if (json) {
      free(json);
      json = NULL;
    }
    json_set_allocation_functions(malloc, free);
  }

  /* 5. Line 2256: write_responses null check */
  write_responses(NULL, NULL);

  /* 6. Line 2470: write_path_item_object write_server_array failure */
  {
    struct OpenAPI_Path path_srv_fail;
    struct OpenAPI_Server srv_dummy[1];
    memset(&path_srv_fail, 0, sizeof(path_srv_fail));
    memset(srv_dummy, 0, sizeof(srv_dummy));
    path_srv_fail.servers = srv_dummy;
    path_srv_fail.n_servers = 1;
    json_set_allocation_functions(mock_parson_oom_malloc, mock_parson_oom_free);
    g_parson_oom_fail_at = 0;
    val = json_value_init_object();
    obj = json_value_get_object(val);
    write_path_item_object(obj, &path_srv_fail);
    json_value_free(val);
    json_set_allocation_functions(malloc, free);
  }

  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_OPENAPI_WRITER_REACH_H */
