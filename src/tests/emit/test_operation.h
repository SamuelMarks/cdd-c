extern int g_fail_io_after;
extern int g_io_calls;
/**
 * @file test_operation.h
 * @brief Unit tests for operation generator.
 */

#ifndef TEST_OPERATION_H
#define TEST_OPERATION_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/* clang-format off */
#include "greatest.h"

#include "routes/emit/operation.h"
/* clang-format on */
TEST test_operation_is_reserved_header_name(void) {
  ASSERT_EQ(0, is_reserved_header_name(NULL));
  ASSERT_EQ(0, is_reserved_header_name(""));
  ASSERT_EQ(1, is_reserved_header_name("accept"));
  ASSERT_EQ(1, is_reserved_header_name("Content-Type"));
  ASSERT_EQ(1, is_reserved_header_name("Authorization"));
  ASSERT_EQ(0, is_reserved_header_name("X-Custom"));
  g_fail_io_after = -1;
  PASS();
}

TEST test_operation_parse_example_any(void) {
  struct OpenAPI_Any out;

  /* NULLs */
  ASSERT_EQ(0, parse_example_any(NULL, &out));
  ASSERT_EQ(0, parse_example_any("test", NULL));

  /* Invalid JSON fallback to string */
  ASSERT_EQ(0, parse_example_any("invalid_json", &out));
  ASSERT_EQ(OA_ANY_STRING, out.type);
  ASSERT_STR_EQ("invalid_json", out.string);
  free(out.string);

  /* Strings */
  ASSERT_EQ(0, parse_example_any("\"hello\"", &out));
  ASSERT_EQ(OA_ANY_STRING, out.type);
  ASSERT_STR_EQ("hello", out.string);
  free(out.string);

  /* Numbers */
  ASSERT_EQ(0, parse_example_any("42.5", &out));
  ASSERT_EQ(OA_ANY_NUMBER, out.type);
  ASSERT(out.number == 42.5);

  /* Booleans */
  ASSERT_EQ(0, parse_example_any("true", &out));
  ASSERT_EQ(OA_ANY_BOOL, out.type);
  ASSERT(out.boolean == true);

  /* Nulls */
  ASSERT_EQ(0, parse_example_any("null", &out));
  ASSERT_EQ(OA_ANY_NULL, out.type);

  /* Objects/Arrays fallback to serialized json */
  ASSERT_EQ(0, parse_example_any("{\"a\": 1}", &out));
  ASSERT_EQ(OA_ANY_JSON, out.type);
  ASSERT(strstr(out.json, "a") != NULL);
  free(out.json);
  g_fail_io_after = -1;

  PASS();
}

TEST test_operation_any_from_json_value(void) {
  struct OpenAPI_Any out;
  JSON_Value *jv = NULL;

  /* NULL */
  ASSERT_EQ(EINVAL, any_from_json_value(NULL, NULL));
  ASSERT_EQ(0, any_from_json_value(NULL, &out));

  /* Types */
  jv = json_parse_string("\"hello\"");
  ASSERT_EQ(0, any_from_json_value(jv, &out));
  ASSERT_EQ(OA_ANY_STRING, out.type);
  ASSERT_STR_EQ("hello", out.string);
  free(out.string);
  json_value_free(jv);

  jv = json_parse_string("42.5");
  ASSERT_EQ(0, any_from_json_value(jv, &out));
  ASSERT_EQ(OA_ANY_NUMBER, out.type);
  ASSERT(out.number == 42.5);
  json_value_free(jv);

  jv = json_parse_string("true");
  ASSERT_EQ(0, any_from_json_value(jv, &out));
  ASSERT_EQ(OA_ANY_BOOL, out.type);
  ASSERT(out.boolean == true);
  json_value_free(jv);

  jv = json_parse_string("null");
  ASSERT_EQ(0, any_from_json_value(jv, &out));
  ASSERT_EQ(OA_ANY_NULL, out.type);
  json_value_free(jv);

  jv = json_parse_string("{\"a\": 1}");
  ASSERT_EQ(0, any_from_json_value(jv, &out));
  ASSERT_EQ(OA_ANY_JSON, out.type);
  ASSERT(strstr(out.json, "a") != NULL);
  free(out.json);
  json_value_free(jv);
  g_fail_io_after = -1;

  PASS();
}

TEST test_operation_parse_link_params_json(void) {
  struct OpenAPI_LinkParam *out = NULL;
  size_t count = 0;
  size_t i;

  /* NULLs */
  ASSERT_EQ(0, parse_link_params_json(NULL, &out, &count));
  ASSERT_EQ(0, count);

  /* Invalid JSON */
  ASSERT_EQ(EINVAL, parse_link_params_json("{invalid", &out, &count));

  /* Not object */
  ASSERT_EQ(EINVAL, parse_link_params_json("\"string\"", &out, &count));

  /* Empty object */
  ASSERT_EQ(0, parse_link_params_json("{}", &out, &count));
  ASSERT_EQ(0, count);

  /* Valid object */
  ASSERT_EQ(
      0, parse_link_params_json("{\"a\": 1, \"b\": \"test\"}", &out, &count));
  ASSERT_EQ(2, count);
  ASSERT_STR_EQ("a", out[0].name);
  ASSERT_EQ(OA_ANY_NUMBER, out[0].value.type);
  ASSERT_STR_EQ("b", out[1].name);
  ASSERT_EQ(OA_ANY_STRING, out[1].value.type);

  for (i = 0; i < count; i++) {
    free(out[i].name);
    if (out[i].value.type == OA_ANY_STRING)
      free(out[i].value.string);
    if (out[i].value.type == OA_ANY_JSON)
      free(out[i].value.json);
  }
  free(out);
  g_fail_io_after = -1;

  PASS();
}

TEST test_operation_free_openapi_server_variables_op(void) {
  struct OpenAPI_Server srv;
  /* NULL */
  free_openapi_server_variables_op(NULL);

  memset(&srv, 0, sizeof(srv));
  free_openapi_server_variables_op(&srv);

  srv.n_variables = 1;
  srv.variables =
      (struct OpenAPI_ServerVariable *)calloc(1, sizeof(*srv.variables));
  srv.variables[0].name = strdup("name");
  srv.variables[0].default_value = strdup("def");
  srv.variables[0].description = strdup("desc");
  srv.variables[0].n_enum_values = 2;
  srv.variables[0].enum_values = (char **)calloc(2, sizeof(char *));
  srv.variables[0].enum_values[0] = strdup("e1");
  srv.variables[0].enum_values[1] = strdup("e2");

  free_openapi_server_variables_op(&srv);
  ASSERT(srv.variables == NULL);
  ASSERT_EQ(0, srv.n_variables);
  g_fail_io_after = -1;

  PASS();
}

TEST test_operation_copy_doc_server_variables_op(void) {
  struct OpenAPI_Server dst;
  struct DocServer src;

  memset(&dst, 0, sizeof(dst));
  memset(&src, 0, sizeof(src));

  /* NULLs */
  ASSERT_EQ(0, copy_doc_server_variables_op(NULL, NULL));
  ASSERT_EQ(0, copy_doc_server_variables_op(&dst, &src));

  src.n_variables = 1;
  src.variables = (struct DocServerVar *)calloc(1, sizeof(struct DocServerVar));

  /* Missing name/default */
  ASSERT_EQ(EINVAL, copy_doc_server_variables_op(&dst, &src));

  src.variables[0].name = "name";
  src.variables[0].default_value = "def";
  src.variables[0].description = "desc";
  src.variables[0].n_enum_values = 2;
  src.variables[0].enum_values = (char **)calloc(2, sizeof(char *));
  src.variables[0].enum_values[0] = strdup("e1");
  src.variables[0].enum_values[1] = strdup("def");

  ASSERT_EQ(0, copy_doc_server_variables_op(&dst, &src));
  ASSERT_EQ(1, dst.n_variables);
  ASSERT_STR_EQ("name", dst.variables[0].name);
  ASSERT_STR_EQ("def", dst.variables[0].default_value);
  ASSERT_STR_EQ("desc", dst.variables[0].description);
  ASSERT_EQ(2, dst.variables[0].n_enum_values);
  ASSERT_STR_EQ("e1", dst.variables[0].enum_values[0]);
  ASSERT_STR_EQ("def", dst.variables[0].enum_values[1]);

  free_openapi_server_variables_op(&dst);

  /* Test validation for default_value in enum */
  src.variables[0].enum_values[1] = strdup("e2"); /* removed "def" */
  ASSERT_EQ(EINVAL, copy_doc_server_variables_op(&dst, &src));

  free(src.variables[0].enum_values[0]);
  free(src.variables[0].enum_values[1]);
  free(src.variables[0].enum_values);
  free(src.variables);
  g_fail_io_after = -1;

  PASS();
}

TEST test_operation_find_response_by_code(void) {
  struct OpenAPI_Operation op;
  struct OpenAPI_Response *out = NULL;

  memset(&op, 0, sizeof(op));

  /* NULLs */
  ASSERT_EQ(0, find_response_by_code(NULL, NULL, &out));
  ASSERT(out == NULL);

  /* Empty responses */
  ASSERT_EQ(0, find_response_by_code(&op, "200", &out));
  ASSERT(out == NULL);

  op.n_responses = 2;
  op.responses = (struct OpenAPI_Response *)calloc(2, sizeof(*op.responses));
  op.responses[0].code = "404";
  op.responses[1].code = "200";

  ASSERT_EQ(0, find_response_by_code(&op, "200", &out));
  ASSERT(out == &op.responses[1]);

  ASSERT_EQ(0, find_response_by_code(&op, "500", &out));
  ASSERT(out == NULL);

  free(op.responses);
  g_fail_io_after = -1;
  PASS();
}

TEST test_operation_find_media_type_op(void) {
  struct OpenAPI_MediaType mts[2];
  struct OpenAPI_MediaType *out = NULL;

  memset(&mts, 0, sizeof(mts));

  /* NULLs */
  ASSERT_EQ(0, find_media_type_op(NULL, 0, "test", &out));
  ASSERT(out == NULL);

  mts[0].name = "application/json";
  mts[1].name = "text/plain";

  ASSERT_EQ(0, find_media_type_op(mts, 2, "application/json", &out));
  ASSERT(out == &mts[0]);

  ASSERT_EQ(0, find_media_type_op(mts, 2, "not-found", &out));
  ASSERT(out == NULL);
  g_fail_io_after = -1;

  PASS();
}

TEST test_operation_apply_example(void) {
  struct OpenAPI_MediaType mt;
  struct OpenAPI_Response resp;

  memset(&mt, 0, sizeof(mt));
  memset(&resp, 0, sizeof(resp));

  /* NULLs */
  ASSERT_EQ(0, apply_example_to_media_type(NULL, NULL));
  ASSERT_EQ(0, apply_example_to_media_type(&mt, NULL));
  ASSERT_EQ(0, apply_example_to_media_type(NULL, "test"));

  ASSERT_EQ(0, apply_example_to_response(NULL, NULL, NULL));
  ASSERT_EQ(0, apply_example_to_response(&resp, NULL, NULL));
  ASSERT_EQ(0, apply_example_to_response(NULL, "test", NULL));

  /* basic apply mt */
  ASSERT_EQ(0, apply_example_to_media_type(&mt, "test"));
  ASSERT_EQ(1, mt.example_set);
  ASSERT_STR_EQ("test", mt.example.string);

  /* already set */
  ASSERT_EQ(0, apply_example_to_media_type(&mt, "test2"));
  ASSERT_STR_EQ("test", mt.example.string);

  /* basic apply resp */
  ASSERT_EQ(0, apply_example_to_response(&resp, "test3", NULL));
  ASSERT_EQ(1, resp.example_set);
  ASSERT_STR_EQ("test3", resp.example.string);

  /* resp already set */
  ASSERT_EQ(0, apply_example_to_response(&resp, "test4", NULL));
  ASSERT_STR_EQ("test3", resp.example.string);

  /* Now what if content_type is provided? */
  resp.example_set = 0;
  resp.n_content_media_types = 1;
  resp.content_media_types =
      (struct OpenAPI_MediaType *)calloc(1, sizeof(struct OpenAPI_MediaType));
  resp.content_media_types[0].name = "application/json";

  /* Content type provided but doesn't match */
  ASSERT_EQ(0, apply_example_to_response(&resp, "test5", "text/plain"));
  ASSERT_EQ(0, resp.example_set);

  /* Content type provided and matches */
  ASSERT_EQ(0, apply_example_to_response(&resp, "test6", "application/json"));
  ASSERT_EQ(1, resp.content_media_types[0].example_set);
  ASSERT_STR_EQ("test6", resp.content_media_types[0].example.string);

  /* No content type, applies to all */
  resp.content_media_types[0].example_set = 0;
  ASSERT_EQ(0, apply_example_to_response(&resp, "test7", NULL));
  ASSERT_EQ(1, resp.content_media_types[0].example_set);
  ASSERT_STR_EQ("test7", resp.content_media_types[0].example.string);

  free(resp.content_media_types[0].example.string);
  free(resp.content_media_types);

  free(mt.example.string);
  free(resp.example.string);
  g_fail_io_after = -1;
  PASS();
}

TEST test_operation_ensure_response_for_code(void) {
  struct OpenAPI_Operation op;
  struct OpenAPI_Response *out = NULL;
  size_t i;

  memset(&op, 0, sizeof(op));

  /* NULLs */
  ASSERT_EQ(0, ensure_response_for_code(NULL, NULL, &out));
  ASSERT(out == NULL);

  /* Valid creation 200 */
  ASSERT_EQ(0, ensure_response_for_code(&op, "200", &out));
  ASSERT(out != NULL);
  ASSERT_STR_EQ("200", out->code);
  ASSERT_STR_EQ("Success", out->description);
  ASSERT_EQ(1, op.n_responses);

  /* Valid creation 404 */
  ASSERT_EQ(0, ensure_response_for_code(&op, "404", &out));
  ASSERT(out != NULL);
  ASSERT_STR_EQ("404", out->code);
  ASSERT_STR_EQ("Response", out->description);
  ASSERT_EQ(2, op.n_responses);

  /* Already exists */
  ASSERT_EQ(0, ensure_response_for_code(&op, "200", &out));
  ASSERT(out == &op.responses[0]);
  ASSERT_EQ(2, op.n_responses);

  for (i = 0; i < op.n_responses; i++) {
    free(op.responses[i].code);
    free(op.responses[i].description);
  }
  free(op.responses);
  g_fail_io_after = -1;

  PASS();
}

TEST test_operation_add_header_to_response(void) {
  struct OpenAPI_Response resp;
  struct DocResponseHeader dh;

  memset(&resp, 0, sizeof(resp));
  memset(&dh, 0, sizeof(dh));

  /* NULLs */
  ASSERT_EQ(EINVAL, add_header_to_response(NULL, NULL));
  ASSERT_EQ(EINVAL, add_header_to_response(&resp, &dh));

  dh.name = "X-Test";
  dh.description = "Test header";
  dh.type = "string";
  dh.content_type = "text/plain";
  dh.format = "uuid";
  dh.example = "test_ex";

  ASSERT_EQ(0, add_header_to_response(&resp, &dh));
  ASSERT_EQ(1, resp.n_headers);
  ASSERT_STR_EQ("X-Test", resp.headers[0].name);
  ASSERT_STR_EQ("Test header", resp.headers[0].description);
  ASSERT_STR_EQ("string", resp.headers[0].type);
  ASSERT_STR_EQ("text/plain", resp.headers[0].content_type);
  ASSERT_EQ(1, resp.headers[0].schema_set);
  ASSERT_STR_EQ("string", resp.headers[0].schema.inline_type);
  ASSERT_EQ(OA_ANY_STRING, resp.headers[0].example.type);
  ASSERT_STR_EQ("test_ex", resp.headers[0].example.string);

  /* Add again (merge) */
  dh.description = "New desc";
  dh.type = "int";
  dh.content_type = "app/json";

  /* Already set, won't override desc/type/content_type but WILL override format
   * maybe? */
  ASSERT_EQ(0, add_header_to_response(&resp, &dh));
  ASSERT_EQ(1, resp.n_headers);
  ASSERT_STR_EQ("Test header", resp.headers[0].description);

  free(resp.headers[0].name);
  free(resp.headers[0].description);
  free(resp.headers[0].type);
  free(resp.headers[0].content_type);
  free(resp.headers[0].schema.inline_type);
  free(resp.headers[0].example.string);
  free(resp.headers);
  g_fail_io_after = -1;

  PASS();
}

TEST test_operation_add_link_to_response(void) {
  struct OpenAPI_Response resp;
  struct DocLink dl;

  memset(&resp, 0, sizeof(resp));
  memset(&dl, 0, sizeof(dl));

  /* NULLs */
  ASSERT_EQ(EINVAL, add_link_to_response(NULL, NULL));
  ASSERT_EQ(EINVAL, add_link_to_response(&resp, &dl));
  dl.name = "MyLink";

  /* Must have EXACTLY one of opId or opRef */
  ASSERT_EQ(EINVAL, add_link_to_response(&resp, &dl));
  dl.operation_id = "opId";
  dl.operation_ref = "opRef";
  ASSERT_EQ(EINVAL, add_link_to_response(&resp, &dl));

  dl.operation_ref = NULL;

  /* Valid */
  dl.description = "desc";
  dl.server_url = "http://test";
  dl.server_description = "server_desc";
  dl.request_body_json = "{\"a\": 1}";

  ASSERT_EQ(0, add_link_to_response(&resp, &dl));
  ASSERT_EQ(1, resp.n_links);
  ASSERT_STR_EQ("MyLink", resp.links[0].name);
  ASSERT_STR_EQ("opId", resp.links[0].operation_id);
  ASSERT_STR_EQ("desc", resp.links[0].description);
  ASSERT_STR_EQ("http://test", resp.links[0].server->url);
  ASSERT_STR_EQ("server_desc", resp.links[0].server->description);
  ASSERT_EQ(1, resp.links[0].request_body_set);
  ASSERT_EQ(OA_ANY_JSON, resp.links[0].request_body.type);

  /* Attempt duplicate */
  ASSERT_EQ(EINVAL, add_link_to_response(&resp, &dl));
  ASSERT_EQ(1, resp.n_links);

  /* clean up */
  free(resp.links[0].name);
  free(resp.links[0].operation_id);
  free(resp.links[0].description);
  free(resp.links[0].server->url);
  free(resp.links[0].server->description);
  free(resp.links[0].server);
  free(resp.links[0].request_body.json);
  free(resp.links);
  g_fail_io_after = -1;

  PASS();
}

TEST test_operation_add_param_to_op(void) {
  struct OpenAPI_Operation op;
  struct OpenAPI_Parameter p;

  memset(&op, 0, sizeof(op));
  memset(&p, 0, sizeof(p));

  p.name = "test_param";

  ASSERT_EQ(0, add_param_to_op(&op, &p));
  ASSERT_EQ(1, op.n_parameters);
  ASSERT_STR_EQ("test_param", op.parameters[0].name);

  free(op.parameters);
  g_fail_io_after = -1;
  PASS();
}

TEST test_operation_schema_ref_has_data_basic(void) {
  struct OpenAPI_SchemaRef ref;
  memset(&ref, 0, sizeof(ref));

  ASSERT_EQ(0, schema_ref_has_data_basic(NULL));
  ASSERT_EQ(0, schema_ref_has_data_basic(&ref));

  ref.ref_name = "test";
  ASSERT_EQ(1, schema_ref_has_data_basic(&ref));

  memset(&ref, 0, sizeof(ref));
  ref.ref = "#/components/schemas/test";
  ASSERT_EQ(1, schema_ref_has_data_basic(&ref));

  memset(&ref, 0, sizeof(ref));
  ref.inline_type = "string";
  ASSERT_EQ(1, schema_ref_has_data_basic(&ref));

  memset(&ref, 0, sizeof(ref));
  ref.is_array = 1;
  ASSERT_EQ(1, schema_ref_has_data_basic(&ref));
  g_fail_io_after = -1;

  PASS();
}

TEST test_operation_copy_schema_ref_basic(void) {
  struct OpenAPI_SchemaRef dst, src;
  memset(&dst, 0, sizeof(dst));
  memset(&src, 0, sizeof(src));

  /* NULLs */
  ASSERT_EQ(0, copy_schema_ref_basic(NULL, NULL));
  ASSERT_EQ(0, copy_schema_ref_basic(&dst, NULL));

  /* All fields */
  src.is_array = 1;
  src.ref_name = "test1";
  src.ref = "test2";
  src.inline_type = "test3";

  ASSERT_EQ(0, copy_schema_ref_basic(&dst, &src));
  ASSERT_EQ(1, dst.is_array);
  ASSERT_STR_EQ("test1", dst.ref_name);
  ASSERT_STR_EQ("test2", dst.ref);
  ASSERT_STR_EQ("test3", dst.inline_type);

  free(dst.ref_name);
  free(dst.ref);
  free(dst.inline_type);
  g_fail_io_after = -1;
  PASS();
}

TEST test_operation_response_has_media_type(void) {
  struct OpenAPI_Response resp;
  memset(&resp, 0, sizeof(resp));

  /* NULLs */
  ASSERT_EQ(0, response_has_media_type(NULL, "test"));
  ASSERT_EQ(0, response_has_media_type(&resp, NULL));
  ASSERT_EQ(0, response_has_media_type(&resp, "test"));

  resp.n_content_media_types = 1;
  resp.content_media_types =
      (struct OpenAPI_MediaType *)calloc(1, sizeof(struct OpenAPI_MediaType));
  resp.content_media_types[0].name = "test";

  ASSERT_EQ(1, response_has_media_type(&resp, "test"));
  ASSERT_EQ(0, response_has_media_type(&resp, "test2"));

  free(resp.content_media_types);
  g_fail_io_after = -1;
  PASS();
}

TEST test_operation_is_struct_pointer(void) {
  int is_dp = 0;

  ASSERT_EQ(0, is_struct_pointer(NULL, &is_dp));
  ASSERT_EQ(0, is_struct_pointer("int *", &is_dp));
  ASSERT_EQ(0, is_struct_pointer("struct MyStruct", &is_dp));

  ASSERT_EQ(1, is_struct_pointer("struct MyStruct *", &is_dp));
  ASSERT_EQ(0, is_dp);

  ASSERT_EQ(1, is_struct_pointer("struct MyStruct **", &is_dp));
  ASSERT_EQ(1, is_dp);
  g_fail_io_after = -1;

  PASS();
}

TEST test_operation_doc_style_to_openapi(void) {
  enum OpenAPI_Style out;

  ASSERT_EQ(0, doc_style_to_openapi(DOC_PARAM_STYLE_FORM, &out));
  ASSERT_EQ(OA_STYLE_FORM, out);

  ASSERT_EQ(0, doc_style_to_openapi(DOC_PARAM_STYLE_SIMPLE, &out));
  ASSERT_EQ(OA_STYLE_SIMPLE, out);

  ASSERT_EQ(0, doc_style_to_openapi(DOC_PARAM_STYLE_MATRIX, &out));
  ASSERT_EQ(OA_STYLE_MATRIX, out);

  ASSERT_EQ(0, doc_style_to_openapi(DOC_PARAM_STYLE_LABEL, &out));
  ASSERT_EQ(OA_STYLE_LABEL, out);

  ASSERT_EQ(0, doc_style_to_openapi(DOC_PARAM_STYLE_SPACE_DELIMITED, &out));
  ASSERT_EQ(OA_STYLE_SPACE_DELIMITED, out);

  ASSERT_EQ(0, doc_style_to_openapi(DOC_PARAM_STYLE_PIPE_DELIMITED, &out));
  ASSERT_EQ(OA_STYLE_PIPE_DELIMITED, out);

  ASSERT_EQ(0, doc_style_to_openapi(DOC_PARAM_STYLE_DEEP_OBJECT, &out));
  ASSERT_EQ(OA_STYLE_DEEP_OBJECT, out);

  ASSERT_EQ(0, doc_style_to_openapi(DOC_PARAM_STYLE_COOKIE, &out));
  ASSERT_EQ(OA_STYLE_COOKIE, out);

  ASSERT_EQ(0, doc_style_to_openapi(DOC_PARAM_STYLE_UNSET, &out));
  ASSERT_EQ(OA_STYLE_UNKNOWN, out);
  g_fail_io_after = -1;

  PASS();
}

SUITE(operation_suite) {
  RUN_TEST(test_operation_is_reserved_header_name);
  RUN_TEST(test_operation_parse_example_any);
  RUN_TEST(test_operation_any_from_json_value);
  RUN_TEST(test_operation_parse_link_params_json);
  RUN_TEST(test_operation_free_openapi_server_variables_op);
  RUN_TEST(test_operation_copy_doc_server_variables_op);
  RUN_TEST(test_operation_find_response_by_code);
  RUN_TEST(test_operation_find_media_type_op);
  RUN_TEST(test_operation_ensure_response_for_code);
  RUN_TEST(test_operation_apply_example);
  RUN_TEST(test_operation_add_header_to_response);
  RUN_TEST(test_operation_add_link_to_response);
  RUN_TEST(test_operation_add_param_to_op);
  RUN_TEST(test_operation_schema_ref_has_data_basic);
  RUN_TEST(test_operation_copy_schema_ref_basic);
  RUN_TEST(test_operation_response_has_media_type);
  RUN_TEST(test_operation_is_struct_pointer);
  RUN_TEST(test_operation_doc_style_to_openapi);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_OPERATION_H */
