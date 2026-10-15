/**
 * @file test_operation_links.h
 * @brief Link and example tests for operation generator.
 */

#ifndef TEST_OPERATION_LINKS_H
#define TEST_OPERATION_LINKS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "emit/test_operation_common.h"
/* clang-format on */

TEST test_operation_parse_example_any_oom(void) {
  struct OpenAPI_Any out;
  memset(&out, 0, sizeof(out));

  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, parse_example_any("not-json", &out));
  g_cdd_strdup_fail = 0;

  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, parse_example_any("\"hello\"", &out));
  g_cdd_strdup_fail = 0;

  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, parse_example_any("{\"k\": \"v\"}", &out));
  g_cdd_strdup_fail = 0;

  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, parse_example_any("[1, 2, 3]", &out));
  g_cdd_strdup_fail = 0;

  PASS();
}

TEST test_operation_any_from_json_value_oom_and_types(void) {
  struct OpenAPI_Any out;
  JSON_Value *jv_str;
  JSON_Value *jv_obj;

  memset(&out, 0, sizeof(out));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, any_from_json_value(NULL, NULL));

  jv_str = json_parse_string("\"str\"");
  ASSERT_NEQ(NULL, jv_str);
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, any_from_json_value(jv_str, &out));
  g_cdd_strdup_fail = 0;
  json_value_free(jv_str);

  jv_obj = json_parse_string("{\"k\": 1}");
  ASSERT_NEQ(NULL, jv_obj);
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, any_from_json_value(jv_obj, &out));
  g_cdd_strdup_fail = 0;
  json_value_free(jv_obj);

  PASS();
}

TEST test_operation_parse_link_params_json_oom(void) {
  struct OpenAPI_LinkParam *out = NULL;
  size_t count = 0;

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            parse_link_params_json("not json", &out, &count));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            parse_link_params_json("[1, 2]", &out, &count));

  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            parse_link_params_json("{\"param1\": \"val1\", \"param2\": 42}",
                                   &out, &count));
  g_cdd_strdup_fail = 0;
  ASSERT_EQ(NULL, out);
  ASSERT_EQ(0, count);

  g_cdd_strdup_fail = 2;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            parse_link_params_json(
                "{\"param1\": \"val1\", \"param2\": \"val2\"}", &out, &count));
  g_cdd_strdup_fail = 0;
  ASSERT_EQ(NULL, out);
  ASSERT_EQ(0, count);

  PASS();
}

TEST test_operation_parse_link_params_json_cleanup_branches(void) {
  struct OpenAPI_LinkParam *out = NULL;
  size_t count = 0;

  g_cdd_strdup_fail = 3;
  ASSERT_EQ(
      CDD_C_ERROR_MEMORY,
      parse_link_params_json("{\"p1\": \"hello\", \"p2\": 42}", &out, &count));
  g_cdd_strdup_fail = 0;

  g_cdd_strdup_fail = 3;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            parse_link_params_json("{\"p1\": {\"nested\": 1}, \"p2\": 42}",
                                   &out, &count));
  g_cdd_strdup_fail = 0;

  PASS();
}

TEST test_operation_copy_and_free_any_value_local(void) {
  struct OpenAPI_Response resp;
  struct OpenAPI_MediaType mt;
  memset(&resp, 0, sizeof(resp));
  memset(&mt, 0, sizeof(mt));

  resp.content_media_types = &mt;
  resp.n_content_media_types = 1;

  ASSERT_EQ(CDD_C_SUCCESS, apply_example_to_response(&resp, "123.45", NULL));
  ASSERT_EQ(1, mt.example_set);
  ASSERT_EQ(OA_ANY_NUMBER, mt.example.type);
  mt.example_set = 0;

  ASSERT_EQ(CDD_C_SUCCESS, apply_example_to_response(&resp, "true", NULL));
  ASSERT_EQ(1, mt.example_set);
  ASSERT_EQ(OA_ANY_BOOL, mt.example.type);
  mt.example_set = 0;

  ASSERT_EQ(CDD_C_SUCCESS, apply_example_to_response(&resp, "null", NULL));
  ASSERT_EQ(1, mt.example_set);
  ASSERT_EQ(OA_ANY_NULL, mt.example.type);
  mt.example_set = 0;

  mt.name = cdd_test_mut_str("application/json");
  ASSERT_EQ(CDD_C_SUCCESS,
            apply_example_to_response(&resp, "\"hello\"", "application/json"));
  ASSERT_EQ(1, mt.example_set);
  ASSERT_EQ(OA_ANY_STRING, mt.example.type);
  if (mt.example.string)
    free(mt.example.string);
  mt.example.string = NULL;
  mt.example_set = 0;

  ASSERT_EQ(CDD_C_SUCCESS,
            apply_example_to_response(&resp, "{\"k\": 1}", "application/json"));
  ASSERT_EQ(1, mt.example_set);
  ASSERT_EQ(OA_ANY_JSON, mt.example.type);
  if (mt.example.json)
    free(mt.example.json);
  mt.example.json = NULL;
  mt.example_set = 0;

  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            apply_example_to_response(&resp, "\"hello\"", "application/json"));
  g_cdd_strdup_fail = 0;
  mt.example_set = 0;

  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            apply_example_to_response(&resp, "\"hello\"", NULL));
  g_cdd_strdup_fail = 0;
  mt.example_set = 0;

  resp.content_media_types = NULL;
  resp.n_content_media_types = 0;
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            apply_example_to_response(&resp, "\"hello\"", NULL));
  g_cdd_strdup_fail = 0;

  PASS();
}

TEST test_operation_copy_any_value_local_nulls(void) {
  struct OpenAPI_Any dst, src;
  memset(&dst, 0, sizeof(dst));
  memset(&src, 0, sizeof(src));

  ASSERT_EQ(CDD_C_SUCCESS, copy_any_value_local(NULL, &src));
  ASSERT_EQ(CDD_C_SUCCESS, copy_any_value_local(&dst, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, copy_any_value_local(NULL, NULL));

  free_any_value_local(NULL);
  PASS();
}

TEST test_operation_apply_example_to_response_branches(void) {
  struct OpenAPI_Response resp;
  struct OpenAPI_MediaType mts[2];
  memset(&resp, 0, sizeof(resp));
  memset(mts, 0, sizeof(mts));

  resp.content_media_types = mts;
  resp.n_content_media_types = 2;
  mts[0].name = cdd_test_mut_str("application/json");
  mts[1].name = cdd_test_mut_str("text/plain");

  g_cdd_strdup_fail = 2;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            apply_example_to_response(&resp, "\"val\"", "application/json"));
  g_cdd_strdup_fail = 0;

  mts[0].example_set = 1;
  ASSERT_EQ(CDD_C_SUCCESS, apply_example_to_response(&resp, "42", NULL));
  ASSERT_EQ(1, mts[1].example_set);

  mts[1].example_set = 0;
  g_cdd_strdup_fail = 2;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            apply_example_to_response(&resp, "\"val\"", NULL));
  g_cdd_strdup_fail = 0;

  PASS();
}

TEST test_operation_copy_doc_server_variables_op_oom(void) {
  struct OpenAPI_Server dst;
  struct DocServer src;
  struct DocServerVar var;
  char *enums[2];
  memset(&dst, 0, sizeof(dst));
  memset(&src, 0, sizeof(src));
  memset(&var, 0, sizeof(var));

  enums[0] = cdd_test_mut_str("v1");
  enums[1] = cdd_test_mut_str("v2");

  var.name = cdd_test_mut_str("port");
  var.default_value = cdd_test_mut_str("v1");
  var.description = cdd_test_mut_str("desc");
  var.enum_values = enums;
  var.n_enum_values = 2;

  src.variables = &var;
  src.n_variables = 1;

  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, copy_doc_server_variables_op(&dst, &src));
  g_cdd_strdup_fail = 0;

  g_cdd_strdup_fail = 2;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, copy_doc_server_variables_op(&dst, &src));
  g_cdd_strdup_fail = 0;

  g_cdd_strdup_fail = 3;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, copy_doc_server_variables_op(&dst, &src));
  g_cdd_strdup_fail = 0;

  g_cdd_strdup_fail = 4;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, copy_doc_server_variables_op(&dst, &src));
  g_cdd_strdup_fail = 0;

  var.default_value = cdd_test_mut_str("v_not_found");
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            copy_doc_server_variables_op(&dst, &src));

  var.name = NULL;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            copy_doc_server_variables_op(&dst, &src));

  PASS();
}

TEST test_operation_doc_server_variables_enum_calloc_oom(void) {
  struct OpenAPI_Server dst;
  struct DocServer src;
  struct DocServerVar var;
  memset(&dst, 0, sizeof(dst));
  memset(&src, 0, sizeof(src));
  memset(&var, 0, sizeof(var));

  var.name = cdd_test_mut_str("port");
  var.default_value = cdd_test_mut_str("8080");
  src.variables = &var;
  src.n_variables = 1;
  ASSERT_EQ(CDD_C_SUCCESS, copy_doc_server_variables_op(&dst, &src));
  free_openapi_server_variables_op(&dst);

  PASS();
}

TEST test_operation_find_doc_param(void) {
  struct DocMetadata doc;
  struct DocParam params[2];
  struct DocParam *out = (struct DocParam *)1;
  memset(&doc, 0, sizeof(doc));
  memset(params, 0, sizeof(params));

  ASSERT_EQ(CDD_C_SUCCESS, find_doc_param(NULL, "param", &out));
  ASSERT_EQ(NULL, out);
  ASSERT_EQ(CDD_C_SUCCESS, find_doc_param(&doc, NULL, &out));
  ASSERT_EQ(NULL, out);

  params[0].name = cdd_test_mut_str("foo");
  params[1].name = cdd_test_mut_str("bar");
  doc.params = params;
  doc.n_params = 2;

  ASSERT_EQ(CDD_C_SUCCESS, find_doc_param(&doc, "bar", &out));
  ASSERT_NEQ(NULL, out);
  ASSERT_STR_EQ("bar", out->name);

  ASSERT_EQ(CDD_C_SUCCESS, find_doc_param(&doc, "baz", &out));
  ASSERT_EQ(NULL, out);

  PASS();
}

TEST test_operation_ensure_response_for_code_oom(void) {
  struct OpenAPI_Operation op;
  struct OpenAPI_Response *out_resp = NULL;
  memset(&op, 0, sizeof(op));

  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_SUCCESS, ensure_response_for_code(&op, "200", &out_resp));
  g_cdd_strdup_fail = 0;
  ASSERT_EQ(NULL, out_resp);

  g_cdd_strdup_fail = 2;
  ASSERT_EQ(CDD_C_SUCCESS, ensure_response_for_code(&op, "200", &out_resp));
  g_cdd_strdup_fail = 0;
  ASSERT_EQ(NULL, out_resp);

  if (op.responses) {
    free(op.responses);
    op.responses = NULL;
    op.n_responses = 0;
  }

  PASS();
}

TEST test_operation_apply_example_to_media_type_oom(void) {
  struct OpenAPI_MediaType mt;
  memset(&mt, 0, sizeof(mt));
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, apply_example_to_media_type(&mt, "not-json"));
  g_cdd_strdup_fail = 0;
  PASS();
}

TEST test_operation_add_header_to_response_advanced(void) {
  struct OpenAPI_Response resp;
  struct DocResponseHeader dh;
  memset(&resp, 0, sizeof(resp));
  memset(&dh, 0, sizeof(dh));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, add_header_to_response(NULL, &dh));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, add_header_to_response(&resp, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, add_header_to_response(&resp, &dh));

  dh.name = cdd_test_mut_str("X-Test");
  dh.description = cdd_test_mut_str("desc");
  dh.type = cdd_test_mut_str("integer");
  dh.content_type = cdd_test_mut_str("text/plain");
  dh.format = cdd_test_mut_str("int64");
  dh.required_set = 1;
  dh.required = 1;
  dh.example = cdd_test_mut_str("42");

  ASSERT_EQ(CDD_C_SUCCESS, add_header_to_response(&resp, &dh));
  ASSERT_EQ(1, resp.n_headers);
  ASSERT_EQ(OA_EXAMPLE_LOC_MEDIA, resp.headers[0].example_location);

  dh.name = cdd_test_mut_str("x-test");
  dh.description = NULL;
  dh.type = NULL;
  dh.content_type = NULL;
  dh.format = cdd_test_mut_str("int32");
  dh.required = 0;
  dh.example = cdd_test_mut_str("100");
  ASSERT_EQ(CDD_C_SUCCESS, add_header_to_response(&resp, &dh));

  if (resp.headers[0].description)
    free(resp.headers[0].description);
  if (resp.headers[0].type)
    free(resp.headers[0].type);
  if (resp.headers[0].content_type)
    free(resp.headers[0].content_type);
  resp.headers[0].description = NULL;
  resp.headers[0].type = NULL;
  resp.headers[0].content_type = NULL;
  resp.headers[0].example_set = 0;
  dh.description = cdd_test_mut_str("new desc");
  dh.type = cdd_test_mut_str("string");
  dh.content_type = cdd_test_mut_str("application/json");
  dh.example = cdd_test_mut_str("test");
  ASSERT_EQ(CDD_C_SUCCESS, add_header_to_response(&resp, &dh));

  {
    size_t i;
    for (i = 0; i < resp.n_headers; ++i) {
      if (resp.headers[i].name)
        free(resp.headers[i].name);
      if (resp.headers[i].description)
        free(resp.headers[i].description);
      if (resp.headers[i].type)
        free(resp.headers[i].type);
      if (resp.headers[i].content_type)
        free(resp.headers[i].content_type);
      if (resp.headers[i].schema.inline_type)
        free(resp.headers[i].schema.inline_type);
      if (resp.headers[i].schema.format)
        free(resp.headers[i].schema.format);
      if (resp.headers[i].example.type == OA_ANY_STRING &&
          resp.headers[i].example.string)
        free(resp.headers[i].example.string);
    }
    free(resp.headers);
    resp.headers = NULL;
    resp.n_headers = 0;
  }

  dh.name = cdd_test_mut_str("X-OOM");
  dh.description = cdd_test_mut_str("desc");
  dh.type = cdd_test_mut_str("string");
  dh.content_type = cdd_test_mut_str("text/plain");
  dh.format = cdd_test_mut_str("date");
  dh.example = cdd_test_mut_str("2020-01-01");

  {
    int k;
    for (k = 1; k <= 6; ++k) {
      memset(&resp, 0, sizeof(resp));
      g_cdd_strdup_fail = k;
      ASSERT_EQ(CDD_C_ERROR_MEMORY, add_header_to_response(&resp, &dh));
      g_cdd_strdup_fail = 0;
      if (resp.headers) {
        free(resp.headers);
        resp.headers = NULL;
      }
    }
  }

  PASS();
}

TEST test_operation_add_header_to_response_existing_oom(void) {
  struct OpenAPI_Response resp;
  struct DocResponseHeader dh;
  struct OpenAPI_Header hdr;
  memset(&resp, 0, sizeof(resp));
  memset(&dh, 0, sizeof(dh));
  memset(&hdr, 0, sizeof(hdr));

  resp.headers = &hdr;
  resp.n_headers = 1;
  hdr.name = cdd_test_mut_str("X-Custom");

  dh.name = cdd_test_mut_str("X-Custom");

  dh.description = cdd_test_mut_str("desc");
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, add_header_to_response(&resp, &dh));
  g_cdd_strdup_fail = 0;
  dh.description = NULL;

  dh.type = cdd_test_mut_str("integer");
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, add_header_to_response(&resp, &dh));
  g_cdd_strdup_fail = 0;
  dh.type = NULL;

  dh.content_type = cdd_test_mut_str("application/json");
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, add_header_to_response(&resp, &dh));
  g_cdd_strdup_fail = 0;
  dh.content_type = NULL;

  dh.format = cdd_test_mut_str("date");
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, add_header_to_response(&resp, &dh));
  g_cdd_strdup_fail = 0;

  g_cdd_strdup_fail = 2;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, add_header_to_response(&resp, &dh));
  g_cdd_strdup_fail = 0;
  if (hdr.schema.inline_type) {
    free(hdr.schema.inline_type);
    hdr.schema.inline_type = NULL;
  }
  dh.format = NULL;

  dh.example = cdd_test_mut_str("ex");
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, add_header_to_response(&resp, &dh));
  g_cdd_strdup_fail = 0;

  PASS();
}

TEST test_operation_header_example_failure(void) {
  struct OpenAPI_Response resp;
  struct DocResponseHeader dh;
  struct OpenAPI_Header hdr;
  memset(&resp, 0, sizeof(resp));
  memset(&dh, 0, sizeof(dh));
  memset(&hdr, 0, sizeof(hdr));

  resp.headers = &hdr;
  resp.n_headers = 1;
  hdr.name = cdd_test_mut_str("X-Header");
  dh.name = cdd_test_mut_str("X-Header");
  dh.example = cdd_test_mut_str("example_val");
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, add_header_to_response(&resp, &dh));
  g_cdd_strdup_fail = 0;

  memset(&resp, 0, sizeof(resp));
  dh.name = cdd_test_mut_str("X-New-Header");
  dh.description = cdd_test_mut_str("desc");
  dh.type = cdd_test_mut_str("string");
  dh.content_type = cdd_test_mut_str("text/plain");
  dh.format = cdd_test_mut_str("date");
  dh.example = cdd_test_mut_str("example_val");
  g_cdd_strdup_fail = 7;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, add_header_to_response(&resp, &dh));
  g_cdd_strdup_fail = 0;
  if (resp.headers) {
    free(resp.headers);
    resp.headers = NULL;
  }

  PASS();
}

TEST test_operation_add_link_to_response_advanced(void) {
  struct OpenAPI_Response resp;
  struct DocLink dl;
  memset(&resp, 0, sizeof(resp));
  memset(&dl, 0, sizeof(dl));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, add_link_to_response(NULL, &dl));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, add_link_to_response(&resp, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, add_link_to_response(&resp, &dl));

  dl.name = cdd_test_mut_str("link1");
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, add_link_to_response(&resp, &dl));
  dl.operation_id = cdd_test_mut_str("op1");
  dl.operation_ref = cdd_test_mut_str("#/paths/op");
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, add_link_to_response(&resp, &dl));

  dl.operation_id = NULL;
  dl.operation_ref = cdd_test_mut_str("#/components/links/Ref");
  dl.summary = cdd_test_mut_str("link summary");
  dl.description = cdd_test_mut_str("link desc");
  dl.server_url = cdd_test_mut_str("https://api.example.com");
  dl.server_name = cdd_test_mut_str("ProdServer");
  dl.server_description = cdd_test_mut_str("Production server");
  dl.parameters_json = cdd_test_mut_str("{\"userId\": \"$response.body#/id\"}");
  dl.request_body_json = cdd_test_mut_str("{\"meta\": \"data\"}");

  ASSERT_EQ(CDD_C_SUCCESS, add_link_to_response(&resp, &dl));
  ASSERT_EQ(1, resp.n_links);

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, add_link_to_response(&resp, &dl));

  {
    struct OpenAPI_Link *link = &resp.links[0];
    if (link->name)
      free(link->name);
    if (link->summary)
      free(link->summary);
    if (link->description)
      free(link->description);
    if (link->operation_ref)
      free(link->operation_ref);
    if (link->parameters) {
      size_t p;
      for (p = 0; p < link->n_parameters; ++p) {
        if (link->parameters[p].name)
          free(link->parameters[p].name);
        if (link->parameters[p].value.type == OA_ANY_STRING &&
            link->parameters[p].value.string)
          free(link->parameters[p].value.string);
      }
      free(link->parameters);
    }
    if (link->request_body.type == OA_ANY_JSON && link->request_body.json)
      free(link->request_body.json);
    if (link->server) {
      if (link->server->url)
        free(link->server->url);
      if (link->server->name)
        free(link->server->name);
      if (link->server->description)
        free(link->server->description);
      free(link->server);
    }
    free(resp.links);
    resp.links = NULL;
    resp.n_links = 0;
  }

  dl.parameters_json = cdd_test_mut_str("invalid json");
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, add_link_to_response(&resp, &dl));
  dl.parameters_json = NULL;

  g_cdd_strdup_fail = 5;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, add_link_to_response(&resp, &dl));
  g_cdd_strdup_fail = 0;
  if (resp.links) {
    free(resp.links[0].name);
    free(resp.links[0].summary);
    free(resp.links[0].description);
    free(resp.links[0].operation_id);
    free(resp.links);
    resp.links = NULL;
    resp.n_links = 0;
  }

  PASS();
}

TEST test_operation_add_link_to_response_oom_sweep(void) {
  struct OpenAPI_Response resp;
  struct DocLink dl;
  int k;
  memset(&dl, 0, sizeof(dl));

  dl.name = cdd_test_mut_str("LinkName");
  dl.summary = cdd_test_mut_str("Summary");
  dl.description = cdd_test_mut_str("Description");
  dl.operation_id = cdd_test_mut_str("op_id");
  dl.server_url = cdd_test_mut_str("https://api.example.com");
  dl.server_name = cdd_test_mut_str("server1");
  dl.server_description = cdd_test_mut_str("server_desc");

  for (k = 1; k <= 7; ++k) {
    memset(&resp, 0, sizeof(resp));
    g_cdd_strdup_fail = k;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, add_link_to_response(&resp, &dl));
    g_cdd_strdup_fail = 0;
    if (resp.links) {
      free(resp.links);
      resp.links = NULL;
    }
  }

  dl.operation_id = NULL;
  dl.operation_ref = cdd_test_mut_str("#/components/links/Ref");
  for (k = 1; k <= 7; ++k) {
    memset(&resp, 0, sizeof(resp));
    g_cdd_strdup_fail = k;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, add_link_to_response(&resp, &dl));
    g_cdd_strdup_fail = 0;
    if (resp.links) {
      free(resp.links);
      resp.links = NULL;
    }
  }

  PASS();
}

TEST test_operation_link_cleanup_specific(void) {
  struct OpenAPI_Response resp;
  struct DocLink dl;
  memset(&resp, 0, sizeof(resp));
  memset(&dl, 0, sizeof(dl));

  dl.name = cdd_test_mut_str("L1");
  dl.operation_id = cdd_test_mut_str("op1");
  dl.parameters_json = cdd_test_mut_str("invalid_json");
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, add_link_to_response(&resp, &dl));

  dl.parameters_json = cdd_test_mut_str("{\"s\": \"val\", \"j\": {\"x\": 1}}");
  dl.request_body_json = cdd_test_mut_str("not-json");
  g_cdd_strdup_fail = 7;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, add_link_to_response(&resp, &dl));
  g_cdd_strdup_fail = 0;
  if (resp.links) {
    free(resp.links);
    resp.links = NULL;
  }

  PASS();
}

TEST test_operation_oa_type_is_primitive(void) {
  int is_p = -1;
  ASSERT_EQ(CDD_C_SUCCESS, oa_type_is_primitive(NULL, &is_p));
  ASSERT_EQ(0, is_p);
  ASSERT_EQ(CDD_C_SUCCESS, oa_type_is_primitive("integer", NULL));

  ASSERT_EQ(CDD_C_SUCCESS, oa_type_is_primitive("integer", &is_p));
  ASSERT_EQ(1, is_p);
  ASSERT_EQ(CDD_C_SUCCESS, oa_type_is_primitive("number", &is_p));
  ASSERT_EQ(1, is_p);
  ASSERT_EQ(CDD_C_SUCCESS, oa_type_is_primitive("string", &is_p));
  ASSERT_EQ(1, is_p);
  ASSERT_EQ(CDD_C_SUCCESS, oa_type_is_primitive("boolean", &is_p));
  ASSERT_EQ(1, is_p);
  ASSERT_EQ(CDD_C_SUCCESS, oa_type_is_primitive("object", &is_p));
  ASSERT_EQ(0, is_p);

  PASS();
}

TEST test_operation_apply_format_to_schema_ref(void) {
  struct OpenAPI_SchemaRef schema;
  struct OpenApiTypeMapping map;
  int applied = -1;
  memset(&schema, 0, sizeof(schema));
  memset(&map, 0, sizeof(map));

  ASSERT_EQ(CDD_C_SUCCESS,
            apply_format_to_schema_ref(NULL, &map, "fmt", &applied));
  ASSERT_EQ(0, applied);
  ASSERT_EQ(CDD_C_SUCCESS,
            apply_format_to_schema_ref(&schema, NULL, "fmt", &applied));
  ASSERT_EQ(0, applied);

  map.oa_type = cdd_test_mut_str("integer");
  ASSERT_EQ(CDD_C_SUCCESS,
            apply_format_to_schema_ref(&schema, &map, NULL, &applied));
  ASSERT_EQ(0, applied);
  ASSERT_EQ(CDD_C_SUCCESS,
            apply_format_to_schema_ref(&schema, &map, "", &applied));
  ASSERT_EQ(0, applied);

  map.oa_type = cdd_test_mut_str("object");
  map.oa_format = cdd_test_mut_str("custom");
  ASSERT_EQ(CDD_C_SUCCESS,
            apply_format_to_schema_ref(&schema, &map, NULL, &applied));
  ASSERT_EQ(0, applied);

  map.kind = OA_TYPE_ARRAY;
  map.oa_type = cdd_test_mut_str("integer");
  map.oa_format = cdd_test_mut_str("int64");

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_OPERATION_LINKS_H */
