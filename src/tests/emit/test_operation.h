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
#include "c_cdd_export.h"
#include "c_cdd/memory.h"
#include "c_cdd/safe_crt.h"
#include "greatest.h"

#include "routes/emit/operation.h"
/* clang-format on */

extern C_CDD_EXPORT int g_cdd_fail_schema_ref_has_data;
extern C_CDD_EXPORT int g_cdd_fail_json_serialize;
extern C_CDD_EXPORT int g_cdd_fail_apply_format;
extern C_CDD_EXPORT int g_cdd_fail_stricmp;
extern C_CDD_EXPORT int g_op_fail_find_media_type_op;
extern C_CDD_EXPORT int g_op_fail_oa_type_is_primitive;
extern C_CDD_EXPORT int g_mapping_fail_init;
extern C_CDD_EXPORT int g_op_fail_find_doc_param;
extern C_CDD_EXPORT int g_op_fail_is_path_param;
extern C_CDD_EXPORT int g_op_fail_is_struct_pointer;
extern C_CDD_EXPORT int g_op_fail_doc_style_to_openapi;
extern C_CDD_EXPORT int g_op_fail_ensure_response_null;
extern C_CDD_EXPORT int g_op_fail_ensure_response_for_code;

static void reset_operation_test(struct OpenAPI_Operation *op);

TEST test_operation_is_reserved_header_name(void) {
  int out = -1;
  reset_operation_test(NULL);
  ASSERT_EQ(CDD_C_SUCCESS, is_reserved_header_name(NULL, &out));
  ASSERT_EQ(0, out);
  ASSERT_EQ(CDD_C_SUCCESS, is_reserved_header_name("", &out));
  ASSERT_EQ(0, out);
  ASSERT_EQ(CDD_C_SUCCESS, is_reserved_header_name("accept", &out));
  ASSERT_EQ(1, out);
  ASSERT_EQ(CDD_C_SUCCESS, is_reserved_header_name("Content-Type", &out));
  ASSERT_EQ(1, out);
  ASSERT_EQ(CDD_C_SUCCESS, is_reserved_header_name("Authorization", &out));
  ASSERT_EQ(1, out);
  ASSERT_EQ(CDD_C_SUCCESS, is_reserved_header_name("X-Custom", &out));
  ASSERT_EQ(0, out);
  ASSERT_EQ(CDD_C_SUCCESS, is_reserved_header_name("accept", NULL));
  ASSERT_EQ(CDD_C_SUCCESS, is_reserved_header_name(NULL, NULL));
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
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, any_from_json_value(NULL, NULL));
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
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            parse_link_params_json("{invalid", &out, &count));

  /* Not object */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            parse_link_params_json("\"string\"", &out, &count));

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
    free(out[i].value.string);
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
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            copy_doc_server_variables_op(&dst, &src));

  src.variables[0].name = (char *)(size_t)(size_t) "name";
  src.variables[0].default_value = (char *)(size_t)(size_t) "def";
  src.variables[0].description = (char *)(size_t)(size_t) "desc";
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
  free(src.variables[0].enum_values[1]);
  src.variables[0].enum_values[1] = strdup("e2"); /* removed "def" */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            copy_doc_server_variables_op(&dst, &src));

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
  op.responses[0].code = (char *)(size_t)(size_t) "404";
  op.responses[1].code = (char *)(size_t)(size_t) "200";

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

  mts[0].name = (char *)(size_t)(size_t) "application/json";
  mts[1].name = (char *)(size_t)(size_t) "text/plain";

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
  if (resp.example.string)
    free(resp.example.string);
  resp.example.string = NULL;
  resp.n_content_media_types = 1;
  resp.content_media_types =
      (struct OpenAPI_MediaType *)calloc(1, sizeof(struct OpenAPI_MediaType));
  resp.content_media_types[0].name =
      (char *)(size_t)(size_t) "application/json";

  /* Content type provided but doesn't match */
  ASSERT_EQ(0, apply_example_to_response(&resp, "test5", "text/plain"));
  ASSERT_EQ(0, resp.example_set);

  /* Content type provided and matches */
  free(resp.content_media_types[0].example.string);
  resp.content_media_types[0].example.string = NULL;
  ASSERT_EQ(0, apply_example_to_response(&resp, "test6", "application/json"));
  ASSERT_EQ(1, resp.content_media_types[0].example_set);
  ASSERT_STR_EQ("test6", resp.content_media_types[0].example.string);

  /* No content type, applies to all */
  resp.content_media_types[0].example_set = 0;
  if (resp.content_media_types[0].example.string)
    free(resp.content_media_types[0].example.string);
  resp.content_media_types[0].example.string = NULL;
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
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, add_header_to_response(NULL, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, add_header_to_response(&resp, &dh));

  dh.name = (char *)(size_t)(size_t) "X-Test";
  dh.description = (char *)(size_t)(size_t) "Test header";
  dh.type = (char *)(size_t)(size_t) "string";
  dh.content_type = (char *)(size_t)(size_t) "text/plain";
  dh.format = (char *)(size_t)(size_t) "uuid";
  dh.example = (char *)(size_t)(size_t) "test_ex";

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
  dh.description = (char *)(size_t)(size_t) "New desc";
  dh.type = (char *)(size_t)(size_t) "int";
  dh.content_type = (char *)(size_t)(size_t) "app/json";

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
  if (resp.headers[0].schema.format)
    free(resp.headers[0].schema.format);
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
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, add_link_to_response(NULL, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, add_link_to_response(&resp, &dl));
  dl.name = (char *)(size_t)(size_t) "MyLink";

  /* Must have EXACTLY one of opId or opRef */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, add_link_to_response(&resp, &dl));
  dl.operation_id = (char *)(size_t)(size_t) "opId";
  dl.operation_ref = (char *)(size_t)(size_t) "opRef";
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, add_link_to_response(&resp, &dl));

  dl.operation_ref = NULL;

  /* Valid */
  dl.description = (char *)(size_t)(size_t) "desc";
  dl.server_url = (char *)(size_t)(size_t) "http://test";
  dl.server_description = (char *)(size_t)(size_t) "server_desc";
  dl.request_body_json = (char *)(size_t)(size_t) "{\"a\": 1}";

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
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, add_link_to_response(&resp, &dl));
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

  p.name = (char *)(size_t)(size_t) "test_param";

  ASSERT_EQ(0, add_param_to_op(&op, &p));
  ASSERT_EQ(1, op.n_parameters);
  ASSERT_STR_EQ("test_param", op.parameters[0].name);

  free(op.parameters);
  g_fail_io_after = -1;
  PASS();
}

TEST test_operation_schema_ref_has_data_basic(void) {
  struct OpenAPI_SchemaRef ref;
  int has_data = -1;
  memset(&ref, 0, sizeof(ref));

  ASSERT_EQ(CDD_C_SUCCESS, schema_ref_has_data_basic(NULL, &has_data));
  ASSERT_EQ(0, has_data);
  ASSERT_EQ(CDD_C_SUCCESS, schema_ref_has_data_basic(&ref, &has_data));
  ASSERT_EQ(0, has_data);
  ASSERT_EQ(CDD_C_SUCCESS, schema_ref_has_data_basic(&ref, NULL));

  ref.ref_name = (char *)(size_t)(size_t) "test";
  ASSERT_EQ(CDD_C_SUCCESS, schema_ref_has_data_basic(&ref, &has_data));
  ASSERT_EQ(1, has_data);

  memset(&ref, 0, sizeof(ref));
  ref.ref = (char *)(size_t)(size_t) "#/components/schemas/test";
  ASSERT_EQ(CDD_C_SUCCESS, schema_ref_has_data_basic(&ref, &has_data));
  ASSERT_EQ(1, has_data);

  memset(&ref, 0, sizeof(ref));
  ref.inline_type = (char *)(size_t)(size_t) "string";
  ASSERT_EQ(CDD_C_SUCCESS, schema_ref_has_data_basic(&ref, &has_data));
  ASSERT_EQ(1, has_data);

  memset(&ref, 0, sizeof(ref));
  ref.is_array = 1;
  ASSERT_EQ(CDD_C_SUCCESS, schema_ref_has_data_basic(&ref, &has_data));
  ASSERT_EQ(1, has_data);
  g_fail_io_after = -1;

  PASS();
}

TEST test_operation_copy_schema_ref_basic(void) {
  struct OpenAPI_SchemaRef dst, src;
  memset(&dst, 0, sizeof(dst));
  memset(&src, 0, sizeof(src));

  ASSERT_EQ(0, copy_schema_ref_basic(NULL, NULL));
  ASSERT_EQ(0, copy_schema_ref_basic(&dst, NULL));

  src.is_array = 1;
  src.ref_name = (char *)(size_t)(size_t) "test1";
  src.ref = (char *)(size_t)(size_t) "test2";
  src.inline_type = (char *)(size_t)(size_t) "test3";
  src.items_ref = (char *)(size_t)(size_t) "test4";
  src.format = (char *)(size_t)(size_t) "test5";
  src.items_format = (char *)(size_t)(size_t) "test6";

  ASSERT_EQ(0, copy_schema_ref_basic(&dst, &src));
  ASSERT_EQ(1, dst.is_array);
  ASSERT_STR_EQ("test1", dst.ref_name);
  ASSERT_STR_EQ("test2", dst.ref);
  ASSERT_STR_EQ("test3", dst.inline_type);
  ASSERT_STR_EQ("test4", dst.items_ref);
  ASSERT_STR_EQ("test5", dst.format);
  ASSERT_STR_EQ("test6", dst.items_format);

  free(dst.ref_name);
  free(dst.ref);
  free(dst.inline_type);
  free(dst.items_ref);
  free(dst.format);
  free(dst.items_format);

  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, copy_schema_ref_basic(&dst, &src));
  g_cdd_strdup_fail = 2;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, copy_schema_ref_basic(&dst, &src));
  g_cdd_strdup_fail = 3;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, copy_schema_ref_basic(&dst, &src));
  g_cdd_strdup_fail = 4;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, copy_schema_ref_basic(&dst, &src));
  g_cdd_strdup_fail = 5;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, copy_schema_ref_basic(&dst, &src));
  g_cdd_strdup_fail = 6;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, copy_schema_ref_basic(&dst, &src));
  g_cdd_strdup_fail = 0;

  g_fail_io_after = -1;
  PASS();
}

TEST test_operation_response_has_media_type(void) {
  struct OpenAPI_Response resp;
  int out = -1;
  memset(&resp, 0, sizeof(resp));

  ASSERT_EQ(CDD_C_SUCCESS, response_has_media_type(NULL, "test", &out));
  ASSERT_EQ(0, out);
  ASSERT_EQ(CDD_C_SUCCESS, response_has_media_type(&resp, NULL, &out));
  ASSERT_EQ(0, out);
  ASSERT_EQ(CDD_C_SUCCESS, response_has_media_type(&resp, "test", &out));
  ASSERT_EQ(0, out);
  ASSERT_EQ(CDD_C_SUCCESS, response_has_media_type(&resp, "test", NULL));
  ASSERT_EQ(CDD_C_SUCCESS, response_has_media_type(NULL, "test", NULL));

  resp.n_content_media_types = 1;
  resp.content_media_types =
      (struct OpenAPI_MediaType *)calloc(1, sizeof(struct OpenAPI_MediaType));
  resp.content_media_types[0].name = (char *)(size_t)(size_t) "test";

  ASSERT_EQ(CDD_C_SUCCESS, response_has_media_type(&resp, "test", &out));
  ASSERT_EQ(1, out);
  resp.content_type = (char *)(size_t)(size_t) "fast";
  ASSERT_EQ(CDD_C_SUCCESS, response_has_media_type(&resp, "fast", &out));
  ASSERT_EQ(1, out);
  ASSERT_EQ(CDD_C_SUCCESS, response_has_media_type(&resp, "test2", &out));
  ASSERT_EQ(0, out);
  resp.content_type = NULL;

  free(resp.content_media_types);
  g_fail_io_after = -1;
  PASS();
}

TEST test_operation_is_struct_pointer(void) {
  int is_dp = 0;
  int out = -1;
  ASSERT_EQ(CDD_C_SUCCESS, is_struct_pointer(NULL, &is_dp, &out));
  ASSERT_EQ(0, out);
  ASSERT_EQ(CDD_C_SUCCESS, is_struct_pointer("int *", &is_dp, &out));
  ASSERT_EQ(0, out);
  ASSERT_EQ(CDD_C_SUCCESS, is_struct_pointer("struct MyStruct", &is_dp, &out));
  ASSERT_EQ(0, out);
  ASSERT_EQ(CDD_C_SUCCESS, is_struct_pointer("struct MyStruct", NULL, &out));
  ASSERT_EQ(0, out);
  ASSERT_EQ(CDD_C_SUCCESS, is_struct_pointer("int", &is_dp, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, is_struct_pointer(NULL, NULL, NULL));

  ASSERT_EQ(CDD_C_SUCCESS,
            is_struct_pointer("struct MyStruct *", &is_dp, &out));
  ASSERT_EQ(1, out);
  ASSERT_EQ(0, is_dp);

  ASSERT_EQ(CDD_C_SUCCESS,
            is_struct_pointer("struct MyStruct **", &is_dp, &out));
  ASSERT_EQ(1, out);
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

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            doc_style_to_openapi(DOC_PARAM_STYLE_FORM, NULL));

  g_fail_io_after = -1;
  PASS();
}

static void reset_operation_test(struct OpenAPI_Operation *op) {
  size_t i;
  if (!op)
    return;
  free(op->operation_id);
  free(op->summary);
  free(op->description);
  free(op->method);
  if (op->parameters) {
    for (i = 0; i < op->n_parameters; i++) {
      free(op->parameters[i].name);
      free(op->parameters[i].type);
      free(op->parameters[i].description);
      free(op->parameters[i].content_type);
      free(op->parameters[i].items_type);
      free(op->parameters[i].example.string);
      free(op->parameters[i].example.json);
      free(op->parameters[i].schema.ref_name);
      free(op->parameters[i].schema.ref);
      free(op->parameters[i].schema.inline_type);
      free(op->parameters[i].schema.items_ref);
      free(op->parameters[i].schema.format);
      free(op->parameters[i].schema.items_format);
      free(op->parameters[i].schema.content_media_type);
      free(op->parameters[i].schema.content_encoding);
      free(op->parameters[i].schema.items_content_media_type);
      free(op->parameters[i].schema.items_content_encoding);
    }
    free(op->parameters);
  }
  if (op->tags) {
    for (i = 0; i < op->n_tags; ++i) {
      free(op->tags[i]);
    }
    free(op->tags);
  }
  free(op->external_docs.url);
  free(op->external_docs.description);
  free(op->req_body.ref_name);
  free(op->req_body.inline_type);
  free(op->req_body.content_type);
  free(op->req_body.format);
  free(op->req_body.items_ref);
  free(op->req_body.items_format);
  if (op->req_body_media_types) {
    for (i = 0; i < op->n_req_body_media_types; ++i) {
      struct OpenAPI_MediaType *mt = &op->req_body_media_types[i];
      size_t e;
      free(mt->name);
      free(mt->ref);
      free(mt->extensions_json);
      free(mt->schema.ref_name);
      free(mt->schema.ref);
      free(mt->schema.inline_type);
      free(mt->schema.items_ref);
      free(mt->schema.format);
      free(mt->schema.items_format);
      free(mt->schema.content_media_type);
      free(mt->schema.content_encoding);
      free(mt->schema.items_content_media_type);
      free(mt->schema.items_content_encoding);
      free(mt->example.string);
      free(mt->example.json);
      if (mt->encoding) {
        for (e = 0; e < mt->n_encoding; ++e) {
          free(mt->encoding[e].name);
          free(mt->encoding[e].content_type);
        }
        free(mt->encoding);
      }
      if (mt->prefix_encoding) {
        for (e = 0; e < mt->n_prefix_encoding; ++e) {
          free(mt->prefix_encoding[e].name);
          free(mt->prefix_encoding[e].content_type);
        }
        free(mt->prefix_encoding);
      }
      if (mt->item_encoding) {
        free(mt->item_encoding->name);
        free(mt->item_encoding->content_type);
        free(mt->item_encoding);
      }
    }
    free(op->req_body_media_types);
  }
  free(op->req_body_description);
  free(op->req_body_extensions_json);
  free(op->req_body_ref);
  free(op->req_body.example.string);
  free(op->req_body.example.json);
  if (op->responses) {
    for (i = 0; i < op->n_responses; i++) {
      size_t m;
      free(op->responses[i].code);
      free(op->responses[i].summary);
      free(op->responses[i].description);
      free(op->responses[i].content_type);
      free(op->responses[i].example.string);
      free(op->responses[i].example.json);
      free(op->responses[i].schema.ref_name);
      free(op->responses[i].schema.ref);
      free(op->responses[i].schema.inline_type);
      free(op->responses[i].schema.items_ref);
      free(op->responses[i].schema.format);
      free(op->responses[i].schema.items_format);
      if (op->responses[i].content_media_types) {
        for (m = 0; m < op->responses[i].n_content_media_types; ++m) {
          struct OpenAPI_MediaType *mt =
              &op->responses[i].content_media_types[m];
          free(mt->name);
          free(mt->schema.ref_name);
          free(mt->schema.ref);
          free(mt->schema.inline_type);
          free(mt->schema.items_ref);
          free(mt->schema.format);
          free(mt->schema.items_format);
          free(mt->item_schema.ref_name);
          free(mt->item_schema.ref);
          free(mt->item_schema.inline_type);
          free(mt->item_schema.items_ref);
          free(mt->item_schema.format);
          free(mt->item_schema.items_format);
          free(mt->example.string);
          free(mt->example.json);
        }
        free(op->responses[i].content_media_types);
      }
      if (op->responses[i].headers) {
        size_t h;
        for (h = 0; h < op->responses[i].n_headers; ++h) {
          struct OpenAPI_Header *hdr = &op->responses[i].headers[h];
          free(hdr->name);
          free(hdr->description);
          free(hdr->content_type);
          free(hdr->type);
          free(hdr->schema.inline_type);
          free(hdr->schema.format);
          free(hdr->example.string);
          free(hdr->example.json);
        }
        free(op->responses[i].headers);
      }
      if (op->responses[i].links) {
        size_t l;
        for (l = 0; l < op->responses[i].n_links; ++l) {
          struct OpenAPI_Link *lnk = &op->responses[i].links[l];
          free(lnk->name);
          free(lnk->summary);
          free(lnk->description);
          free(lnk->operation_id);
          free(lnk->operation_ref);
          if (lnk->parameters) {
            size_t p;
            for (p = 0; p < lnk->n_parameters; ++p) {
              free(lnk->parameters[p].name);
              free(lnk->parameters[p].value.string);
              free(lnk->parameters[p].value.json);
            }
            free(lnk->parameters);
          }
          free(lnk->request_body.string);
          free(lnk->request_body.json);
          if (lnk->server) {
            free(lnk->server->url);
            free(lnk->server->name);
            free(lnk->server->description);
            free(lnk->server);
          }
        }
        free(op->responses[i].links);
      }
    }
    free(op->responses);
  }
  if (op->security) {
    for (i = 0; i < op->n_security; ++i) {
      struct OpenAPI_SecurityRequirementSet *set = &op->security[i];
      if (set->requirements) {
        size_t r;
        for (r = 0; r < set->n_requirements; ++r) {
          size_t s;
          free(set->requirements[r].scheme);
          if (set->requirements[r].scopes) {
            for (s = 0; s < set->requirements[r].n_scopes; ++s) {
              free(set->requirements[r].scopes[s]);
            }
            free(set->requirements[r].scopes);
          }
        }
        free(set->requirements);
      }
    }
    free(op->security);
  }
  if (op->servers) {
    for (i = 0; i < op->n_servers; ++i) {
      free_openapi_server_variables_op(&op->servers[i]);
      free(op->servers[i].url);
      free(op->servers[i].name);
      free(op->servers[i].description);
    }
    free(op->servers);
  }
  memset(op, 0, sizeof(*op));
}

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

  mt.name = (char *)(size_t) "application/json";
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
  mts[0].name = (char *)(size_t) "application/json";
  mts[1].name = (char *)(size_t) "text/plain";

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

  enums[0] = (char *)(size_t) "v1";
  enums[1] = (char *)(size_t) "v2";

  var.name = (char *)(size_t) "port";
  var.default_value = (char *)(size_t) "v1";
  var.description = (char *)(size_t) "desc";
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

  var.default_value = (char *)(size_t) "v_not_found";
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

  var.name = (char *)(size_t) "port";
  var.default_value = (char *)(size_t) "8080";
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

  params[0].name = (char *)(size_t) "foo";
  params[1].name = (char *)(size_t) "bar";
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

  dh.name = (char *)(size_t) "X-Test";
  dh.description = (char *)(size_t) "desc";
  dh.type = (char *)(size_t) "integer";
  dh.content_type = (char *)(size_t) "text/plain";
  dh.format = (char *)(size_t) "int64";
  dh.required_set = 1;
  dh.required = 1;
  dh.example = (char *)(size_t) "42";

  ASSERT_EQ(CDD_C_SUCCESS, add_header_to_response(&resp, &dh));
  ASSERT_EQ(1, resp.n_headers);
  ASSERT_EQ(OA_EXAMPLE_LOC_MEDIA, resp.headers[0].example_location);

  dh.name = (char *)(size_t) "x-test";
  dh.description = NULL;
  dh.type = NULL;
  dh.content_type = NULL;
  dh.format = (char *)(size_t) "int32";
  dh.required = 0;
  dh.example = (char *)(size_t) "100";
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
  dh.description = (char *)(size_t) "new desc";
  dh.type = (char *)(size_t) "string";
  dh.content_type = (char *)(size_t) "application/json";
  dh.example = (char *)(size_t) "test";
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

  dh.name = (char *)(size_t) "X-OOM";
  dh.description = (char *)(size_t) "desc";
  dh.type = (char *)(size_t) "string";
  dh.content_type = (char *)(size_t) "text/plain";
  dh.format = (char *)(size_t) "date";
  dh.example = (char *)(size_t) "2020-01-01";

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
  hdr.name = (char *)(size_t) "X-Custom";

  dh.name = (char *)(size_t) "X-Custom";

  dh.description = (char *)(size_t) "desc";
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, add_header_to_response(&resp, &dh));
  g_cdd_strdup_fail = 0;
  dh.description = NULL;

  dh.type = (char *)(size_t) "integer";
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, add_header_to_response(&resp, &dh));
  g_cdd_strdup_fail = 0;
  dh.type = NULL;

  dh.content_type = (char *)(size_t) "application/json";
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, add_header_to_response(&resp, &dh));
  g_cdd_strdup_fail = 0;
  dh.content_type = NULL;

  dh.format = (char *)(size_t) "date";
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

  dh.example = (char *)(size_t) "ex";
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
  hdr.name = (char *)(size_t) "X-Header";
  dh.name = (char *)(size_t) "X-Header";
  dh.example = (char *)(size_t) "example_val";
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, add_header_to_response(&resp, &dh));
  g_cdd_strdup_fail = 0;

  memset(&resp, 0, sizeof(resp));
  dh.name = (char *)(size_t) "X-New-Header";
  dh.description = (char *)(size_t) "desc";
  dh.type = (char *)(size_t) "string";
  dh.content_type = (char *)(size_t) "text/plain";
  dh.format = (char *)(size_t) "date";
  dh.example = (char *)(size_t) "example_val";
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

  dl.name = (char *)(size_t) "link1";
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, add_link_to_response(&resp, &dl));
  dl.operation_id = (char *)(size_t) "op1";
  dl.operation_ref = (char *)(size_t) "#/paths/op";
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, add_link_to_response(&resp, &dl));

  dl.operation_id = NULL;
  dl.operation_ref = (char *)(size_t) "#/components/links/Ref";
  dl.summary = (char *)(size_t) "link summary";
  dl.description = (char *)(size_t) "link desc";
  dl.server_url = (char *)(size_t) "https://api.example.com";
  dl.server_name = (char *)(size_t) "ProdServer";
  dl.server_description = (char *)(size_t) "Production server";
  dl.parameters_json = (char *)(size_t) "{\"userId\": \"$response.body#/id\"}";
  dl.request_body_json = (char *)(size_t) "{\"meta\": \"data\"}";

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

  dl.parameters_json = (char *)(size_t) "invalid json";
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

  dl.name = (char *)(size_t) "LinkName";
  dl.summary = (char *)(size_t) "Summary";
  dl.description = (char *)(size_t) "Description";
  dl.operation_id = (char *)(size_t) "op_id";
  dl.server_url = (char *)(size_t) "https://api.example.com";
  dl.server_name = (char *)(size_t) "server1";
  dl.server_description = (char *)(size_t) "server_desc";

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
  dl.operation_ref = (char *)(size_t) "#/components/links/Ref";
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

  dl.name = (char *)(size_t) "L1";
  dl.operation_id = (char *)(size_t) "op1";
  dl.parameters_json = (char *)(size_t) "invalid_json";
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, add_link_to_response(&resp, &dl));

  dl.parameters_json = (char *)(size_t) "{\"s\": \"val\", \"j\": {\"x\": 1}}";
  dl.request_body_json = (char *)(size_t) "not-json";
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

  map.oa_type = (char *)(size_t) "integer";
  ASSERT_EQ(CDD_C_SUCCESS,
            apply_format_to_schema_ref(&schema, &map, NULL, &applied));
  ASSERT_EQ(0, applied);
  ASSERT_EQ(CDD_C_SUCCESS,
            apply_format_to_schema_ref(&schema, &map, "", &applied));
  ASSERT_EQ(0, applied);

  map.oa_type = (char *)(size_t) "object";
  map.oa_format = (char *)(size_t) "custom";
  ASSERT_EQ(CDD_C_SUCCESS,
            apply_format_to_schema_ref(&schema, &map, NULL, &applied));
  ASSERT_EQ(0, applied);

  map.kind = OA_TYPE_ARRAY;
  map.oa_type = (char *)(size_t) "integer";
  map.oa_format = (char *)(size_t) "int64";
  ASSERT_EQ(CDD_C_SUCCESS,
            apply_format_to_schema_ref(&schema, &map, NULL, &applied));
  ASSERT_EQ(1, applied);
  ASSERT_EQ(1, schema.is_array);
  ASSERT_STR_EQ("integer", schema.inline_type);
  ASSERT_STR_EQ("int64", schema.items_format);

  ASSERT_EQ(CDD_C_SUCCESS,
            apply_format_to_schema_ref(&schema, &map, "int32", &applied));
  ASSERT_EQ(1, applied);
  ASSERT_STR_EQ("int32", schema.items_format);

  free(schema.inline_type);
  free(schema.items_format);
  schema.inline_type = NULL;
  schema.items_format = NULL;
  schema.is_array = 0;

  map.kind = OA_TYPE_PRIMITIVE;
  ASSERT_EQ(CDD_C_SUCCESS,
            apply_format_to_schema_ref(&schema, &map, NULL, &applied));
  ASSERT_EQ(1, applied);
  ASSERT_STR_EQ("integer", schema.inline_type);
  ASSERT_STR_EQ("int64", schema.format);

  ASSERT_EQ(CDD_C_SUCCESS,
            apply_format_to_schema_ref(&schema, &map, "int32", &applied));
  ASSERT_EQ(1, applied);
  ASSERT_STR_EQ("int32", schema.format);

  free(schema.inline_type);
  free(schema.format);
  schema.inline_type = NULL;
  schema.format = NULL;

  map.kind = OA_TYPE_ARRAY;
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            apply_format_to_schema_ref(&schema, &map, NULL, &applied));
  g_cdd_strdup_fail = 0;

  g_cdd_strdup_fail = 2;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            apply_format_to_schema_ref(&schema, &map, NULL, &applied));
  g_cdd_strdup_fail = 0;
  if (schema.inline_type) {
    free(schema.inline_type);
    schema.inline_type = NULL;
  }

  map.kind = OA_TYPE_PRIMITIVE;
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            apply_format_to_schema_ref(&schema, &map, NULL, &applied));
  g_cdd_strdup_fail = 0;

  g_cdd_strdup_fail = 2;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            apply_format_to_schema_ref(&schema, &map, NULL, &applied));
  g_cdd_strdup_fail = 0;
  if (schema.inline_type) {
    free(schema.inline_type);
    schema.inline_type = NULL;
  }

  PASS();
}

TEST test_operation_set_querystring_schema_from_type_map(void) {
  struct OpenAPI_Parameter param;
  struct OpenApiTypeMapping tm;
  memset(&param, 0, sizeof(param));
  memset(&tm, 0, sizeof(tm));

  ASSERT_EQ(CDD_C_SUCCESS, set_querystring_schema_from_type_map(NULL, &tm));
  ASSERT_EQ(CDD_C_SUCCESS, set_querystring_schema_from_type_map(&param, NULL));

  tm.ref_name = (char *)(size_t) "UserRef";
  ASSERT_EQ(CDD_C_SUCCESS, set_querystring_schema_from_type_map(&param, &tm));
  ASSERT_EQ(1, param.schema_set);
  ASSERT_EQ(0, param.schema.is_array);
  ASSERT_STR_EQ("UserRef", param.schema.ref_name);
  free(param.schema.ref_name);
  param.schema.ref_name = NULL;

  tm.kind = OA_TYPE_ARRAY;
  ASSERT_EQ(CDD_C_SUCCESS, set_querystring_schema_from_type_map(&param, &tm));
  ASSERT_EQ(1, param.schema.is_array);
  ASSERT_STR_EQ("UserRef", param.schema.ref_name);
  free(param.schema.ref_name);
  param.schema.ref_name = NULL;
  tm.ref_name = NULL;

  tm.oa_type = (char *)(size_t) "integer";
  ASSERT_EQ(CDD_C_SUCCESS, set_querystring_schema_from_type_map(&param, &tm));
  ASSERT_STR_EQ("array", param.type);
  ASSERT_STR_EQ("integer", param.items_type);
  free(param.type);
  free(param.items_type);
  param.type = NULL;
  param.items_type = NULL;

  tm.oa_type = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, set_querystring_schema_from_type_map(&param, &tm));
  ASSERT_STR_EQ("array", param.type);
  ASSERT_EQ(NULL, param.items_type);
  free(param.type);
  param.type = NULL;

  tm.kind = OA_TYPE_PRIMITIVE;
  tm.oa_type = (char *)(size_t) "boolean";
  ASSERT_EQ(CDD_C_SUCCESS, set_querystring_schema_from_type_map(&param, &tm));
  ASSERT_STR_EQ("boolean", param.type);
  free(param.type);
  param.type = NULL;

  tm.oa_type = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, set_querystring_schema_from_type_map(&param, &tm));
  ASSERT_STR_EQ("string", param.type);
  free(param.type);
  param.type = NULL;

  tm.ref_name = (char *)(size_t) "Ref";
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            set_querystring_schema_from_type_map(&param, &tm));
  g_cdd_strdup_fail = 0;
  tm.ref_name = NULL;

  tm.kind = OA_TYPE_ARRAY;
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            set_querystring_schema_from_type_map(&param, &tm));
  g_cdd_strdup_fail = 0;

  tm.oa_type = (char *)(size_t) "integer";
  g_cdd_strdup_fail = 2;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            set_querystring_schema_from_type_map(&param, &tm));
  g_cdd_strdup_fail = 0;
  if (param.type) {
    free(param.type);
    param.type = NULL;
  }

  PASS();
}

TEST test_operation_init_and_add_response_media_type(void) {
  struct OpenAPI_MediaType mt;
  struct OpenAPI_Response resp;
  memset(&mt, 0, sizeof(mt));
  memset(&resp, 0, sizeof(resp));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            init_media_type_from_response(NULL, "app/json", &resp, 0));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            init_media_type_from_response(&mt, NULL, &resp, 0));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            init_media_type_from_response(&mt, "app/json", NULL, 0));

  ASSERT_EQ(CDD_C_SUCCESS,
            init_media_type_from_response(&mt, "app/json", &resp, 0));
  ASSERT_STR_EQ("app/json", mt.name);
  ASSERT_EQ(0, mt.schema_set);
  free(mt.name);

  resp.schema.ref_name = (char *)(size_t) "User";
  ASSERT_EQ(CDD_C_SUCCESS,
            init_media_type_from_response(&mt, "app/json", &resp, 1));
  ASSERT_EQ(1, mt.item_schema_set);
  ASSERT_STR_EQ("User", mt.item_schema.ref_name);
  free(mt.name);
  free(mt.item_schema.ref_name);

  ASSERT_EQ(CDD_C_SUCCESS,
            init_media_type_from_response(&mt, "app/json", &resp, 0));
  ASSERT_EQ(1, mt.schema_set);
  ASSERT_STR_EQ("User", mt.schema.ref_name);
  free(mt.name);
  free(mt.schema.ref_name);

  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            init_media_type_from_response(&mt, "app/json", &resp, 0));
  g_cdd_strdup_fail = 0;

  g_cdd_strdup_fail = 2;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            init_media_type_from_response(&mt, "app/json", &resp, 0));
  g_cdd_strdup_fail = 0;
  free(mt.name);
  mt.name = NULL;

  ASSERT_EQ(CDD_C_SUCCESS, add_response_media_type(NULL, "app/json", 0));
  ASSERT_EQ(CDD_C_SUCCESS, add_response_media_type(&resp, NULL, 0));
  ASSERT_EQ(CDD_C_SUCCESS, add_response_media_type(&resp, "", 0));

  resp.content_type = (char *)(size_t) "application/xml";
  ASSERT_EQ(CDD_C_SUCCESS,
            add_response_media_type(&resp, "application/json", 0));
  ASSERT_EQ(2, resp.n_content_media_types);

  ASSERT_EQ(CDD_C_SUCCESS,
            add_response_media_type(&resp, "application/json", 0));
  ASSERT_EQ(2, resp.n_content_media_types);

  ASSERT_EQ(CDD_C_SUCCESS, add_response_media_type(&resp, "text/plain", 0));
  ASSERT_EQ(3, resp.n_content_media_types);

  {
    size_t i;
    for (i = 0; i < resp.n_content_media_types; ++i) {
      if (resp.content_media_types[i].name)
        free(resp.content_media_types[i].name);
      if (resp.content_media_types[i].schema.ref_name)
        free(resp.content_media_types[i].schema.ref_name);
    }
    free(resp.content_media_types);
    resp.content_media_types = NULL;
    resp.n_content_media_types = 0;
    resp.content_type = NULL;
    resp.schema.ref_name = NULL;
  }

  ASSERT_EQ(CDD_C_SUCCESS,
            add_response_media_type(&resp, "application/json", 0));
  ASSERT_EQ(1, resp.n_content_media_types);
  free(resp.content_media_types[0].name);
  free(resp.content_media_types);
  resp.content_media_types = NULL;
  resp.n_content_media_types = 0;

  resp.content_type = (char *)(size_t) "application/xml";
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            add_response_media_type(&resp, "application/json", 0));
  g_cdd_strdup_fail = 0;
  if (resp.content_media_types) {
    free(resp.content_media_types);
    resp.content_media_types = NULL;
  }

  PASS();
}

TEST test_operation_request_body_media_types(void) {
  struct OpenAPI_Operation op;
  struct OpenAPI_MediaType mt;
  int has = -1;

  memset(&op, 0, sizeof(op));
  memset(&mt, 0, sizeof(mt));

  ASSERT_EQ(CDD_C_SUCCESS, request_body_has_media_type(NULL, "app/json", &has));
  ASSERT_EQ(0, has);
  ASSERT_EQ(CDD_C_SUCCESS, request_body_has_media_type(&op, NULL, &has));
  ASSERT_EQ(0, has);
  ASSERT_EQ(CDD_C_SUCCESS, request_body_has_media_type(&op, "app/json", NULL));

  ASSERT_EQ(CDD_C_SUCCESS, request_body_has_media_type(&op, "app/json", &has));
  ASSERT_EQ(0, has);

  op.req_body.content_type = (char *)(size_t) "application/json";
  ASSERT_EQ(CDD_C_SUCCESS,
            request_body_has_media_type(&op, "application/json", &has));
  ASSERT_EQ(1, has);

  op.req_body.content_type = NULL;
  op.req_body_media_types = &mt;
  op.n_req_body_media_types = 1;
  mt.name = (char *)(size_t) "application/xml";
  ASSERT_EQ(CDD_C_SUCCESS,
            request_body_has_media_type(&op, "application/xml", &has));
  ASSERT_EQ(1, has);
  ASSERT_EQ(CDD_C_SUCCESS,
            request_body_has_media_type(&op, "application/json", &has));
  ASSERT_EQ(0, has);
  op.req_body_media_types = NULL;
  op.n_req_body_media_types = 0;

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            init_media_type_from_request_body(NULL, "app/json", &op, 0));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            init_media_type_from_request_body(&mt, NULL, &op, 0));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            init_media_type_from_request_body(&mt, "app/json", NULL, 0));

  ASSERT_EQ(CDD_C_SUCCESS,
            init_media_type_from_request_body(&mt, "app/json", &op, 0));
  free(mt.name);

  op.req_body.ref_name = (char *)(size_t) "Payload";
  ASSERT_EQ(CDD_C_SUCCESS,
            init_media_type_from_request_body(&mt, "app/json", &op, 1));
  ASSERT_EQ(1, mt.item_schema_set);
  ASSERT_STR_EQ("Payload", mt.item_schema.ref_name);
  free(mt.name);
  free(mt.item_schema.ref_name);

  ASSERT_EQ(CDD_C_SUCCESS,
            init_media_type_from_request_body(&mt, "app/json", &op, 0));
  ASSERT_EQ(1, mt.schema_set);
  ASSERT_STR_EQ("Payload", mt.schema.ref_name);
  free(mt.name);
  free(mt.schema.ref_name);

  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            init_media_type_from_request_body(&mt, "app/json", &op, 0));
  g_cdd_strdup_fail = 2;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            init_media_type_from_request_body(&mt, "app/json", &op, 0));
  g_cdd_strdup_fail = 0;
  free(mt.name);
  mt.name = NULL;
  op.req_body.ref_name = NULL;

  ASSERT_EQ(CDD_C_SUCCESS, add_request_body_media_type(NULL, "app/json", 0));
  ASSERT_EQ(CDD_C_SUCCESS, add_request_body_media_type(&op, NULL, 0));
  ASSERT_EQ(CDD_C_SUCCESS, add_request_body_media_type(&op, "", 0));

  op.req_body.content_type = (char *)(size_t) "application/xml";
  ASSERT_EQ(CDD_C_SUCCESS,
            add_request_body_media_type(&op, "application/json", 0));
  ASSERT_EQ(2, op.n_req_body_media_types);

  ASSERT_EQ(CDD_C_SUCCESS,
            add_request_body_media_type(&op, "application/json", 0));
  ASSERT_EQ(2, op.n_req_body_media_types);

  ASSERT_EQ(CDD_C_SUCCESS, add_request_body_media_type(&op, "text/plain", 0));
  ASSERT_EQ(3, op.n_req_body_media_types);

  {
    size_t i;
    for (i = 0; i < op.n_req_body_media_types; ++i) {
      if (op.req_body_media_types[i].name)
        free(op.req_body_media_types[i].name);
    }
    free(op.req_body_media_types);
    op.req_body_media_types = NULL;
    op.n_req_body_media_types = 0;
    op.req_body.content_type = NULL;
  }

  ASSERT_EQ(CDD_C_SUCCESS,
            add_request_body_media_type(&op, "application/json", 0));
  ASSERT_EQ(1, op.n_req_body_media_types);
  free(op.req_body_media_types[0].name);
  free(op.req_body_media_types);
  op.req_body_media_types = NULL;
  op.n_req_body_media_types = 0;

  op.req_body.content_type = (char *)(size_t) "application/xml";
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            add_request_body_media_type(&op, "application/json", 0));
  g_cdd_strdup_fail = 0;
  if (op.req_body_media_types) {
    free(op.req_body_media_types);
    op.req_body_media_types = NULL;
  }

  PASS();
}

TEST test_operation_media_types_item_schema_oom(void) {
  struct OpenAPI_Response resp;
  struct OpenAPI_MediaType mt;
  struct OpenAPI_Operation op;
  memset(&resp, 0, sizeof(resp));
  memset(&mt, 0, sizeof(mt));
  memset(&op, 0, sizeof(op));

  resp.schema.ref_name = (char *)(size_t) "SchemaRef";
  g_cdd_strdup_fail = 2;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            init_media_type_from_response(&mt, "app/json", &resp, 1));
  g_cdd_strdup_fail = 0;
  free(mt.name);
  mt.name = NULL;

  op.req_body.ref_name = (char *)(size_t) "BodyRef";
  g_cdd_strdup_fail = 2;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            init_media_type_from_request_body(&mt, "app/json", &op, 1));
  g_cdd_strdup_fail = 0;
  free(mt.name);
  mt.name = NULL;

  resp.schema.ref_name = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, add_response_media_type(&resp, "app/json", 0));
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, add_response_media_type(&resp, "app/xml", 0));
  g_cdd_strdup_fail = 0;
  if (resp.content_media_types) {
    if (resp.content_media_types[0].name)
      free(resp.content_media_types[0].name);
    free(resp.content_media_types);
    resp.content_media_types = NULL;
    resp.n_content_media_types = 0;
  }

  op.req_body.ref_name = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, add_request_body_media_type(&op, "app/json", 0));
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, add_request_body_media_type(&op, "app/xml", 0));
  g_cdd_strdup_fail = 0;
  if (op.req_body_media_types) {
    if (op.req_body_media_types[0].name)
      free(op.req_body_media_types[0].name);
    free(op.req_body_media_types);
    op.req_body_media_types = NULL;
    op.n_req_body_media_types = 0;
  }

  PASS();
}

TEST test_operation_c2openapi_build_operation_nulls(void) {
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct OpenAPI_Operation op;
  memset(&ctx, 0, sizeof(ctx));
  memset(&sig, 0, sizeof(sig));
  memset(&op, 0, sizeof(op));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, c2openapi_build_operation(NULL, &op));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            c2openapi_build_operation(&ctx, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, c2openapi_build_operation(&ctx, &op));

  PASS();
}

TEST test_operation_c2openapi_build_operation_verbs(void) {
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct DocMetadata doc;
  struct OpenAPI_Operation op;

  memset(&ctx, 0, sizeof(ctx));
  memset(&sig, 0, sizeof(sig));
  memset(&doc, 0, sizeof(doc));
  memset(&op, 0, sizeof(op));
  ctx.sig = &sig;
  ctx.doc = &doc;
  ctx.func_name = "test_func";

  doc.verb = (char *)(size_t) "GET";
  ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
  ASSERT_EQ(OA_VERB_GET, op.verb);
  reset_operation_test(&op);

  doc.verb = (char *)(size_t) "POST";
  ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
  ASSERT_EQ(OA_VERB_POST, op.verb);
  reset_operation_test(&op);

  doc.verb = (char *)(size_t) "PUT";
  ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
  ASSERT_EQ(OA_VERB_PUT, op.verb);
  reset_operation_test(&op);

  doc.verb = (char *)(size_t) "DELETE";
  ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
  ASSERT_EQ(OA_VERB_DELETE, op.verb);
  reset_operation_test(&op);

  doc.verb = (char *)(size_t) "PATCH";
  ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
  ASSERT_EQ(OA_VERB_PATCH, op.verb);
  reset_operation_test(&op);

  doc.verb = (char *)(size_t) "HEAD";
  ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
  ASSERT_EQ(OA_VERB_HEAD, op.verb);
  reset_operation_test(&op);

  doc.verb = (char *)(size_t) "OPTIONS";
  ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
  ASSERT_EQ(OA_VERB_OPTIONS, op.verb);
  reset_operation_test(&op);

  doc.verb = (char *)(size_t) "TRACE";
  ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
  ASSERT_EQ(OA_VERB_TRACE, op.verb);
  reset_operation_test(&op);

  doc.verb = (char *)(size_t) "QUERY";
  ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
  ASSERT_EQ(OA_VERB_QUERY, op.verb);
  reset_operation_test(&op);

  doc.verb = (char *)(size_t) "CUSTOM";
  ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
  ASSERT_EQ(OA_VERB_UNKNOWN, op.verb);
  ASSERT_EQ(1, op.is_additional);
  ASSERT_STR_EQ("CUSTOM", op.method);
  reset_operation_test(&op);

  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, c2openapi_build_operation(&ctx, &op));
  g_cdd_strdup_fail = 0;
  reset_operation_test(&op);

  doc.verb = NULL;
  ctx.func_name = "api_post_item";
  ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
  ASSERT_EQ(OA_VERB_POST, op.verb);
  reset_operation_test(&op);

  ctx.func_name = "item_create";
  ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
  ASSERT_EQ(OA_VERB_POST, op.verb);
  reset_operation_test(&op);

  ctx.func_name = "api_put_item";
  ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
  ASSERT_EQ(OA_VERB_PUT, op.verb);
  reset_operation_test(&op);

  ctx.func_name = "item_update";
  ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
  ASSERT_EQ(OA_VERB_PUT, op.verb);
  reset_operation_test(&op);

  ctx.func_name = "api_delete_item";
  ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
  ASSERT_EQ(OA_VERB_DELETE, op.verb);
  reset_operation_test(&op);

  ctx.func_name = "item_delete";
  ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
  ASSERT_EQ(OA_VERB_DELETE, op.verb);
  reset_operation_test(&op);

  ctx.func_name = "api_other_op";
  ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
  ASSERT_EQ(OA_VERB_GET, op.verb);
  reset_operation_test(&op);

  PASS();
}

TEST test_operation_c2openapi_build_operation_metadata(void) {
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct DocMetadata doc;
  struct DocSecurityRequirement sec;
  struct DocServer srv;
  struct OpenAPI_Operation op;
  char *tags[2];
  char *scopes[2];

  memset(&ctx, 0, sizeof(ctx));
  memset(&sig, 0, sizeof(sig));
  memset(&doc, 0, sizeof(doc));
  memset(&sec, 0, sizeof(sec));
  memset(&srv, 0, sizeof(srv));
  memset(&op, 0, sizeof(op));

  tags[0] = (char *)(size_t) "tagA";
  tags[1] = (char *)(size_t) "tagB";
  scopes[0] = (char *)(size_t) "read";
  scopes[1] = (char *)(size_t) "write";

  sec.scheme = (char *)(size_t) "OAuth2";
  sec.scopes = scopes;
  sec.n_scopes = 2;

  srv.url = (char *)(size_t) "https://example.com/v1";
  srv.name = (char *)(size_t) "Primary";
  srv.description = (char *)(size_t) "Primary server";

  ctx.sig = &sig;
  ctx.doc = &doc;
  ctx.func_name = "api_widget_get";

  doc.operation_id = (char *)(size_t) "getWidget";
  doc.summary = (char *)(size_t) "Widget Summary";
  doc.description = (char *)(size_t) "Widget Description";
  doc.deprecated_set = 1;
  doc.deprecated = 1;
  doc.external_docs_url = (char *)(size_t) "https://docs.example.com";
  doc.external_docs_description = (char *)(size_t) "Full docs";
  doc.tags = tags;
  doc.n_tags = 2;
  doc.security = &sec;
  doc.n_security = 1;
  doc.servers = &srv;
  doc.n_servers = 1;

  ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
  ASSERT_STR_EQ("getWidget", op.operation_id);
  ASSERT_STR_EQ("Widget Summary", op.summary);
  ASSERT_STR_EQ("Widget Description", op.description);
  ASSERT_EQ(1, op.deprecated);
  ASSERT_STR_EQ("https://docs.example.com", op.external_docs.url);
  ASSERT_STR_EQ("Full docs", op.external_docs.description);
  ASSERT_EQ(2, op.n_tags);
  ASSERT_EQ(1, op.n_security);
  ASSERT_EQ(1, op.n_servers);
  reset_operation_test(&op);

  doc.n_tags = 0;
  doc.tags = NULL;
  ctx.func_name = "api_pet_get";
  ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
  ASSERT_EQ(1, op.n_tags);
  ASSERT_STR_EQ("Pet", op.tags[0]);
  reset_operation_test(&op);

  PASS();
}

TEST test_operation_c2openapi_build_operation_arguments(void) {
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct C2OpenAPI_ParsedArg args[8];
  struct DocMetadata doc;
  struct DocParam params[8];
  struct OpenAPI_Operation op;

  memset(&ctx, 0, sizeof(ctx));
  memset(&sig, 0, sizeof(sig));
  memset(args, 0, sizeof(args));
  memset(&doc, 0, sizeof(doc));
  memset(params, 0, sizeof(params));
  memset(&op, 0, sizeof(op));

  ctx.sig = &sig;
  ctx.doc = &doc;
  ctx.func_name = "api_user_update";
  doc.verb = (char *)(size_t) "PUT";
  doc.route = (char *)(size_t) "/user/{uid}";

  sig.args = args;
  sig.n_args = 8;
  doc.params = params;
  doc.n_params = 8;

  args[0].name = (char *)(size_t) "uid";
  args[0].type = (char *)(size_t) "int";

  args[1].name = (char *)(size_t) "session_id";
  args[1].type = (char *)(size_t) "char *";
  params[1].name = (char *)(size_t) "session_id";
  params[1].in_loc = (char *)(size_t) "cookie";
  params[1].description = (char *)(size_t) "Session cookie";
  params[1].required = 1;
  params[1].style_set = 1;
  params[1].style = DOC_PARAM_STYLE_FORM;
  params[1].explode_set = 1;
  params[1].explode = 1;
  params[1].allow_reserved_set = 1;
  params[1].allow_reserved = 1;
  params[1].allow_empty_value_set = 1;
  params[1].allow_empty_value = 1;
  params[1].deprecated_set = 1;
  params[1].deprecated = 1;
  params[1].example = (char *)(size_t) "abc123xyz";

  args[2].name = (char *)(size_t) "Accept";
  args[2].type = (char *)(size_t) "char *";
  params[2].name = (char *)(size_t) "Accept";
  params[2].in_loc = (char *)(size_t) "header";
  params[2].description = (char *)(size_t) "Reserved accept header";

  args[3].name = (char *)(size_t) "X-Trace-Id";
  args[3].type = (char *)(size_t) "char *";
  params[3].name = (char *)(size_t) "X-Trace-Id";
  params[3].in_loc = (char *)(size_t) "header";
  params[3].content_type = (char *)(size_t) "text/plain";
  params[3].example = (char *)(size_t) "tr-999";

  args[4].name = (char *)(size_t) "filter";
  args[4].type = (char *)(size_t) "char *";
  params[4].name = (char *)(size_t) "filter";
  params[4].in_loc = (char *)(size_t) "querystring";

  args[5].name = (char *)(size_t) "tags";
  args[5].type = (char *)(size_t) "int[]";
  params[5].name = (char *)(size_t) "tags";
  params[5].format = (char *)(size_t) "int32";
  params[5].item_schema = 1;

  args[6].name = (char *)(size_t) "body";
  args[6].type = (char *)(size_t) "struct User *";

  args[7].name = (char *)(size_t) "out_result";
  args[7].type = (char *)(size_t) "struct Result **";

  ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
  ASSERT_EQ(1, op.req_body_required);
  ASSERT_STR_EQ("application/json", op.req_body.content_type);
  ASSERT(op.n_responses >= 1);
  reset_operation_test(&op);

  args[6].type = (char *)(size_t) "const struct User *";
  sig.n_args = 7;
  ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
  ASSERT_EQ(1, op.req_body_required);
  reset_operation_test(&op);

  params[0].name = (char *)(size_t) "uid";
  params[0].in_loc = (char *)(size_t) "body";
  sig.n_args = 1;
  ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
  reset_operation_test(&op);

  PASS();
}

TEST test_operation_c2openapi_build_operation_bodies_returns_links(void) {
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct C2OpenAPI_ParsedArg args[1];
  struct DocMetadata doc;
  struct DocRequestBody rbs[1];
  struct DocEncoding encs[3];
  struct DocResponse rets[2];
  struct DocResponseHeader headers[2];
  struct DocLink links[2];
  struct OpenAPI_Operation op;

  memset(&ctx, 0, sizeof(ctx));
  memset(&sig, 0, sizeof(sig));
  memset(args, 0, sizeof(args));
  memset(&doc, 0, sizeof(doc));
  memset(rbs, 0, sizeof(rbs));
  memset(encs, 0, sizeof(encs));
  memset(rets, 0, sizeof(rets));
  memset(headers, 0, sizeof(headers));
  memset(links, 0, sizeof(links));
  memset(&op, 0, sizeof(op));

  ctx.sig = &sig;
  ctx.doc = &doc;
  ctx.func_name = "api_upload_post";
  doc.verb = (char *)(size_t) "POST";

  sig.args = args;
  sig.n_args = 1;
  args[0].name = (char *)(size_t) "out_data";
  args[0].type = (char *)(size_t) "struct UploadResponse **";

  rbs[0].content_type = (char *)(size_t) "multipart/form-data";
  rbs[0].item_schema = 0;
  rbs[0].example = (char *)(size_t) "{\"file\": \"data\"}";

  encs[0].name = (char *)(size_t) "avatar";
  encs[0].content_type = (char *)(size_t) "image/jpeg";
  encs[0].kind = 0;
  encs[0].style = DOC_PARAM_STYLE_FORM;
  encs[0].explode = 1;
  encs[0].explode_set = 1;
  encs[0].allow_reserved = 1;
  encs[0].allow_reserved_set = 1;

  encs[1].name = (char *)(size_t) "prefix";
  encs[1].kind = 1;

  encs[2].name = (char *)(size_t) "items";
  encs[2].kind = 2;

  doc.request_bodies = rbs;
  doc.n_request_bodies = 1;
  doc.encodings = encs;
  doc.n_encodings = 3;
  doc.request_body_description = (char *)(size_t) "Upload payload";
  doc.request_body_required_set = 1;
  doc.request_body_required = 1;

  rets[0].code = (char *)(size_t) "200";
  rets[0].summary = (char *)(size_t) "Success summary";
  rets[0].description = (char *)(size_t) "Success description";
  rets[0].content_type = (char *)(size_t) "application/json";
  rets[0].example = (char *)(size_t) "{\"status\": \"ok\"}";

  rets[1].code = (char *)(size_t) "400";
  rets[1].summary = (char *)(size_t) "Error summary";
  rets[1].description = (char *)(size_t) "Error description";
  rets[1].content_type = (char *)(size_t) "application/problem+json";
  rets[1].example = (char *)(size_t) "{\"error\": \"bad request\"}";

  doc.returns = rets;
  doc.n_returns = 2;

  headers[0].code = (char *)(size_t) "200";
  headers[0].name = (char *)(size_t) "X-Upload-Rate";
  headers[0].type = (char *)(size_t) "integer";

  headers[1].code = (char *)(size_t) "404";
  headers[1].name = (char *)(size_t) "X-Missing-Id";
  headers[1].type = (char *)(size_t) "string";

  doc.response_headers = headers;
  doc.n_response_headers = 2;

  links[0].code = (char *)(size_t) "200";
  links[0].name = (char *)(size_t) "GetFile";
  links[0].operation_id = (char *)(size_t) "api_file_get";

  links[1].code = (char *)(size_t) "500";
  links[1].name = (char *)(size_t) "SupportTicket";
  links[1].operation_ref = (char *)(size_t) "#/components/links/Support";

  doc.links = links;
  doc.n_links = 2;

  ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
  ASSERT_STR_EQ("Upload payload", op.req_body_description);
  ASSERT(op.n_responses >= 4);
  reset_operation_test(&op);

  doc.n_request_bodies = 0;
  doc.request_bodies = NULL;
  doc.n_encodings = 0;
  doc.request_body_content_type = (char *)(size_t) "text/plain";
  ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
  ASSERT_STR_EQ("text/plain", op.req_body.content_type);
  reset_operation_test(&op);

  PASS();
}

TEST test_operation_c2openapi_build_operation_coverage_extensions(void) {
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct C2OpenAPI_ParsedArg args[4];
  struct DocMetadata doc;
  struct DocParam params[4];
  struct DocResponse rets[1];
  struct OpenAPI_Operation op;
  int k;

  memset(&ctx, 0, sizeof(ctx));
  memset(&sig, 0, sizeof(sig));
  memset(args, 0, sizeof(args));
  memset(&doc, 0, sizeof(doc));
  memset(params, 0, sizeof(params));
  memset(rets, 0, sizeof(rets));
  memset(&op, 0, sizeof(op));

  ctx.sig = &sig;
  ctx.doc = &doc;
  ctx.func_name = "api_user_patch";
  doc.verb = (char *)(size_t) "PATCH";

  sig.args = args;
  sig.n_args = 4;
  doc.params = params;
  doc.n_params = 4;

  args[0].name = (char *)(size_t) "in_patch";
  args[0].type = (char *)(size_t) "const struct User *";

  args[1].name = (char *)(size_t) "out";
  args[1].type = (char *)(size_t) "struct Output **";

  args[2].name = (char *)(size_t) "list";
  args[2].type = (char *)(size_t) "int[]";
  params[2].name = (char *)(size_t) "list";
  params[2].in_loc = (char *)(size_t) "query";
  params[2].format = (char *)(size_t) "int64";

  args[3].name = (char *)(size_t) "X-Key";
  args[3].type = (char *)(size_t) "char *";
  params[3].name = (char *)(size_t) "X-Key";
  params[3].in_loc = (char *)(size_t) "header";
  params[3].example = (char *)(size_t) "key123";

  rets[0].code = (char *)(size_t) "200";
  rets[0].description = (char *)(size_t) "User updated successfully";
  doc.returns = rets;
  doc.n_returns = 1;

  ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
  reset_operation_test(&op);

  for (k = 1; k <= 30; ++k) {
    memset(&op, 0, sizeof(op));
    g_cdd_strdup_fail = k;
    (void)c2openapi_build_operation(&ctx, &op);
    g_cdd_strdup_fail = 0;
    reset_operation_test(&op);
  }

  sig.n_args = 0;
  doc.n_returns = 0;
  doc.returns = NULL;
  doc.n_params = 0;
  doc.params = NULL;
  memset(&op, 0, sizeof(op));
  ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
  ASSERT_EQ(1, op.n_responses);
  ASSERT_STR_EQ("200", op.responses[0].code);
  reset_operation_test(&op);

  memset(&op, 0, sizeof(op));
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, c2openapi_build_operation(&ctx, &op));
  g_cdd_strdup_fail = 0;
  reset_operation_test(&op);

  memset(&op, 0, sizeof(op));
  g_cdd_strdup_fail = 2;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, c2openapi_build_operation(&ctx, &op));
  g_cdd_strdup_fail = 0;
  reset_operation_test(&op);

  PASS();
}

TEST test_operation_c2openapi_build_operation_mega_oom(void) {
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct C2OpenAPI_ParsedArg args[6];
  struct DocMetadata doc;
  struct DocParam params[6];
  struct DocSecurityRequirement sec;
  struct DocServer srv;
  struct DocServerVar svar;
  struct DocRequestBody rbs[1];
  struct DocEncoding encs[3];
  struct DocResponse rets[2];
  struct DocResponseHeader hdrs[2];
  struct DocLink links[2];
  struct OpenAPI_Operation op;
  char *tags[2];
  char *scopes[2];
  char *enums[2];
  int k;

  memset(&ctx, 0, sizeof(ctx));
  memset(&sig, 0, sizeof(sig));
  memset(args, 0, sizeof(args));
  memset(&doc, 0, sizeof(doc));
  memset(params, 0, sizeof(params));
  memset(&sec, 0, sizeof(sec));
  memset(&srv, 0, sizeof(srv));
  memset(&svar, 0, sizeof(svar));
  memset(rbs, 0, sizeof(rbs));
  memset(encs, 0, sizeof(encs));
  memset(rets, 0, sizeof(rets));
  memset(hdrs, 0, sizeof(hdrs));
  memset(links, 0, sizeof(links));
  memset(&op, 0, sizeof(op));

  ctx.sig = &sig;
  ctx.doc = &doc;
  ctx.func_name = "api_mega_post";

  doc.operation_id = (char *)(size_t) "megaPost";
  doc.summary = (char *)(size_t) "Mega summary";
  doc.description = (char *)(size_t) "Mega description";
  doc.external_docs_url = (char *)(size_t) "https://docs.example.com";
  doc.external_docs_description = (char *)(size_t) "Docs";

  tags[0] = (char *)(size_t) "Tag1";
  tags[1] = (char *)(size_t) "Tag2";
  doc.tags = tags;
  doc.n_tags = 2;

  scopes[0] = (char *)(size_t) "read";
  scopes[1] = (char *)(size_t) "write";
  sec.scheme = (char *)(size_t) "Bearer";
  sec.scopes = scopes;
  sec.n_scopes = 2;
  doc.security = &sec;
  doc.n_security = 1;

  enums[0] = (char *)(size_t) "v1";
  enums[1] = (char *)(size_t) "v2";
  svar.name = (char *)(size_t) "env";
  svar.default_value = (char *)(size_t) "v1";
  svar.enum_values = enums;
  svar.n_enum_values = 2;
  srv.url = (char *)(size_t) "https://api.example.com";
  srv.name = (char *)(size_t) "Server1";
  srv.description = (char *)(size_t) "Primary";
  srv.variables = &svar;
  srv.n_variables = 1;
  doc.servers = &srv;
  doc.n_servers = 1;

  sig.args = args;
  sig.n_args = 6;
  doc.params = params;
  doc.n_params = 6;

  args[0].name = (char *)(size_t) "body_in";
  args[0].type = (char *)(size_t) "const struct Input *";

  args[1].name = (char *)(size_t) "data_out";
  args[1].type = (char *)(size_t) "struct Output **";

  args[2].name = (char *)(size_t) "num_list";
  args[2].type = (char *)(size_t) "int[]";
  params[2].name = (char *)(size_t) "num_list";
  params[2].in_loc = (char *)(size_t) "query";
  params[2].format = (char *)(size_t) "int32";

  args[3].name = (char *)(size_t) "query_str";
  args[3].type = (char *)(size_t) "char *";
  params[3].name = (char *)(size_t) "query_str";
  params[3].in_loc = (char *)(size_t) "querystring";

  args[4].name = (char *)(size_t) "sess_id";
  args[4].type = (char *)(size_t) "char *";
  params[4].name = (char *)(size_t) "sess_id";
  params[4].in_loc = (char *)(size_t) "cookie";

  args[5].name = (char *)(size_t) "X-Custom-Hdr";
  args[5].type = (char *)(size_t) "char *";
  params[5].name = (char *)(size_t) "X-Custom-Hdr";
  params[5].in_loc = (char *)(size_t) "header";
  params[5].content_type = (char *)(size_t) "text/plain";
  params[5].example = (char *)(size_t) "hdr_val";

  rbs[0].content_type = (char *)(size_t) "application/json";
  rbs[0].example = (char *)(size_t) "{\"id\": 1}";
  doc.request_bodies = rbs;
  doc.n_request_bodies = 1;

  encs[0].name = (char *)(size_t) "avatar";
  encs[0].content_type = (char *)(size_t) "image/png";
  encs[0].kind = 0;
  doc.encodings = encs;
  doc.n_encodings = 1;

  rets[0].code = (char *)(size_t) "200";
  rets[0].summary = (char *)(size_t) "Ok";
  rets[0].description = (char *)(size_t) "Success desc";
  rets[0].content_type = (char *)(size_t) "application/json";
  rets[0].example = (char *)(size_t) "{\"res\": 1}";

  rets[1].code = (char *)(size_t) "404";
  rets[1].summary = (char *)(size_t) "NotFound";
  rets[1].description = (char *)(size_t) "NotFound desc";
  rets[1].content_type = (char *)(size_t) "application/problem+json";
  rets[1].example = (char *)(size_t) "{\"err\": 1}";
  doc.returns = rets;
  doc.n_returns = 2;

  hdrs[0].code = (char *)(size_t) "200";
  hdrs[0].name = (char *)(size_t) "X-Rate-Limit";
  hdrs[0].type = (char *)(size_t) "integer";

  hdrs[1].code = (char *)(size_t) "400";
  hdrs[1].name = (char *)(size_t) "X-Bad-Request";
  hdrs[1].type = (char *)(size_t) "string";
  doc.response_headers = hdrs;
  doc.n_response_headers = 2;

  links[0].code = (char *)(size_t) "200";
  links[0].name = (char *)(size_t) "GetDetails";
  links[0].operation_id = (char *)(size_t) "getDetailsOp";

  links[1].code = (char *)(size_t) "500";
  links[1].name = (char *)(size_t) "ErrorReport";
  links[1].operation_ref = (char *)(size_t) "#/components/links/Error";
  doc.links = links;
  doc.n_links = 2;

  ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
  reset_operation_test(&op);

  for (k = 1; k <= 70; ++k) {
    memset(&op, 0, sizeof(op));
    g_cdd_strdup_fail = k;
    (void)c2openapi_build_operation(&ctx, &op);
    g_cdd_strdup_fail = 0;
    reset_operation_test(&op);
  }

  for (k = 1; k <= 40; ++k) {
    memset(&op, 0, sizeof(op));
    g_cdd_alloc_fail = k;
    (void)c2openapi_build_operation(&ctx, &op);
    g_cdd_alloc_fail = 0;
    reset_operation_test(&op);
  }

  PASS();
}

TEST test_operation_c2openapi_build_operation_oom(void) {
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct C2OpenAPI_ParsedArg args[2];
  struct DocMetadata doc;
  struct OpenAPI_Operation op;
  int i;

  memset(&ctx, 0, sizeof(ctx));
  memset(&sig, 0, sizeof(sig));
  memset(args, 0, sizeof(args));
  memset(&doc, 0, sizeof(doc));
  memset(&op, 0, sizeof(op));

  ctx.sig = &sig;
  ctx.doc = &doc;
  ctx.func_name = "api_user_get";
  doc.operation_id = (char *)(size_t) "getUser";
  doc.summary = (char *)(size_t) "Get user summary";
  doc.description = (char *)(size_t) "Get user desc";
  doc.external_docs_url = (char *)(size_t) "https://docs.example.com";
  doc.external_docs_description = (char *)(size_t) "Docs";

  sig.args = args;
  sig.n_args = 2;
  args[0].name = (char *)(size_t) "id";
  args[0].type = (char *)(size_t) "int";
  args[1].name = (char *)(size_t) "out";
  args[1].type = (char *)(size_t) "struct User **";

  for (i = 1; i <= 25; ++i) {
    memset(&op, 0, sizeof(op));
    g_cdd_strdup_fail = i;
    (void)c2openapi_build_operation(&ctx, &op);
    g_cdd_strdup_fail = 0;
    reset_operation_test(&op);
  }

  PASS();
}

TEST test_operation_final_gaps(void) {
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct C2OpenAPI_ParsedArg args[4];
  struct DocMetadata doc;
  struct DocResponse rets[4];
  struct DocResponseHeader hdrs[1];
  struct DocLink links[1];
  struct OpenAPI_Operation op;

  memset(&ctx, 0, sizeof(ctx));
  memset(&sig, 0, sizeof(sig));
  memset(args, 0, sizeof(args));
  memset(&doc, 0, sizeof(doc));
  memset(rets, 0, sizeof(rets));
  memset(hdrs, 0, sizeof(hdrs));
  memset(links, 0, sizeof(links));
  memset(&op, 0, sizeof(op));

  ctx.sig = &sig;
  ctx.doc = &doc;
  ctx.func_name = "api_test_final";

  sig.args = args;
  sig.n_args = 4;

  args[0].name = (char *)(size_t) "list[]";
  args[0].type = (char *)(size_t) "int";

  args[1].name = (char *)(size_t) "users[]";
  args[1].type = (char *)(size_t) "struct User";

  args[2].name = (char *)(size_t) "single_user";
  args[2].type = (char *)(size_t) "struct User";

  args[3].name = (char *)(size_t) "out_val";
  args[3].type = (char *)(size_t) "int **";

  rets[0].code = (char *)(size_t) "200";
  rets[0].description = NULL;

  rets[1].code = (char *)(size_t) "200";
  rets[1].description = (char *)(size_t) "Merged desc";

  rets[2].code = (char *)(size_t) "404";
  rets[2].description = NULL;

  rets[3].code = (char *)(size_t) "500";
  rets[3].description = NULL;

  doc.returns = rets;
  doc.n_returns = 4;

  hdrs[0].code = (char *)(size_t) "404";
  hdrs[0].name = (char *)(size_t) "X-Missing";
  hdrs[0].type = (char *)(size_t) "string";
  doc.response_headers = hdrs;
  doc.n_response_headers = 1;

  links[0].code = (char *)(size_t) "500";
  links[0].name = (char *)(size_t) "L500";
  links[0].operation_id = (char *)(size_t) "op500";
  doc.links = links;
  doc.n_links = 1;

  ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
  reset_operation_test(&op);

  /* Test path param doc override & request body content type override without
   * request bodies */
  {
    struct OpBuilderContext ctx2;
    struct C2OpenAPI_ParsedSig sig2;
    struct C2OpenAPI_ParsedArg args2[1];
    struct DocMetadata doc2;
    struct DocParam dp2;
    struct OpenAPI_Operation op2;

    memset(&ctx2, 0, sizeof(ctx2));
    memset(&sig2, 0, sizeof(sig2));
    memset(&doc2, 0, sizeof(doc2));
    memset(&dp2, 0, sizeof(dp2));
    memset(&op2, 0, sizeof(op2));

    ctx2.sig = &sig2;
    ctx2.doc = &doc2;
    ctx2.func_name = "api_path_test";

    sig2.args = args2;
    sig2.n_args = 1;
    args2[0].name = (char *)(size_t) "user_id";
    args2[0].type = (char *)(size_t) "int";

    dp2.name = (char *)(size_t) "user_id";
    dp2.in_loc = (char *)(size_t) "path";
    dp2.content_type = (char *)(size_t) "text/plain";
    doc2.params = &dp2;
    doc2.n_params = 1;

    doc2.request_body_content_type = (char *)(size_t) "application/xml";
    doc2.request_body_required_set = 1;
    doc2.request_body_required = 1;

    ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx2, &op2));
    reset_operation_test(&op2);
  }

  PASS();
}

TEST test_operation_all_null_and_boundary_checks(void) {
  struct OpenAPI_Any any_val;
  struct OpenAPI_LinkParam *lp = NULL;
  size_t count = 0;
  int bool_out = 0;
  struct OpenAPI_Server srv;
  struct DocServer doc_srv;
  struct OpenAPI_Operation op;
  struct OpenAPI_Response resp;
  struct OpenAPI_Response *resp_ptr = NULL;
  struct OpenAPI_MediaType mt;
  struct OpenAPI_MediaType *mt_ptr = NULL;
  struct OpenAPI_Parameter param;
  struct OpenApiTypeMapping tm;
  struct OpenAPI_SchemaRef schema;
  JSON_Value *jv = NULL;

  memset(&any_val, 0, sizeof(any_val));
  memset(&srv, 0, sizeof(srv));
  memset(&doc_srv, 0, sizeof(doc_srv));
  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));
  memset(&mt, 0, sizeof(mt));
  memset(&param, 0, sizeof(param));
  memset(&tm, 0, sizeof(tm));
  memset(&schema, 0, sizeof(schema));

  /* parse_example_any */
  ASSERT_EQ(CDD_C_SUCCESS, parse_example_any(NULL, &any_val));
  ASSERT_EQ(CDD_C_SUCCESS, parse_example_any("ex", NULL));
  ASSERT_EQ(CDD_C_SUCCESS, parse_example_any(NULL, NULL));

  /* any_from_json_value */
  jv = json_parse_string("123");
  ASSERT_EQ(CDD_C_SUCCESS, any_from_json_value(NULL, &any_val));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, any_from_json_value(jv, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, any_from_json_value(NULL, NULL));
  json_value_free(jv);

  /* parse_link_params_json */
  ASSERT_EQ(CDD_C_SUCCESS, parse_link_params_json(NULL, &lp, &count));
  ASSERT_EQ(CDD_C_SUCCESS, parse_link_params_json("{}", NULL, &count));
  ASSERT_EQ(CDD_C_SUCCESS, parse_link_params_json("{}", &lp, NULL));

  /* is_path_param */
  ASSERT_EQ(CDD_C_SUCCESS, is_path_param(NULL, "id", &bool_out));
  ASSERT_EQ(CDD_C_SUCCESS, is_path_param("/user/{id}", NULL, &bool_out));
  ASSERT_EQ(CDD_C_SUCCESS, is_path_param("/user/{id}", "id", NULL));

  /* copy_doc_server_variables_op */
  ASSERT_EQ(CDD_C_SUCCESS, copy_doc_server_variables_op(NULL, &doc_srv));
  ASSERT_EQ(CDD_C_SUCCESS, copy_doc_server_variables_op(&srv, NULL));
  doc_srv.n_variables = 0;
  ASSERT_EQ(CDD_C_SUCCESS, copy_doc_server_variables_op(&srv, &doc_srv));

  /* find_response_by_code */
  ASSERT_EQ(CDD_C_SUCCESS, find_response_by_code(NULL, "200", &resp_ptr));
  ASSERT_EQ(CDD_C_SUCCESS, find_response_by_code(&op, NULL, &resp_ptr));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            find_response_by_code(&op, "200", NULL));

  /* find_media_type_op */
  ASSERT_EQ(CDD_C_SUCCESS, find_media_type_op(NULL, 0, "app/json", &mt_ptr));
  ASSERT_EQ(CDD_C_SUCCESS, find_media_type_op(&mt, 0, NULL, &mt_ptr));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            find_media_type_op(&mt, 0, "app/json", NULL));

  /* apply_example_to_media_type */
  ASSERT_EQ(CDD_C_SUCCESS, apply_example_to_media_type(NULL, "ex"));
  ASSERT_EQ(CDD_C_SUCCESS, apply_example_to_media_type(&mt, NULL));

  /* apply_example_to_response */
  ASSERT_EQ(CDD_C_SUCCESS, apply_example_to_response(NULL, "ex", NULL));
  ASSERT_EQ(CDD_C_SUCCESS, apply_example_to_response(&resp, NULL, NULL));

  /* ensure_response_for_code */
  ASSERT_EQ(CDD_C_SUCCESS, ensure_response_for_code(NULL, "200", &resp_ptr));
  ASSERT_EQ(CDD_C_SUCCESS, ensure_response_for_code(&op, NULL, &resp_ptr));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            ensure_response_for_code(&op, "200", NULL));

  /* add_param_to_op */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, add_param_to_op(NULL, &param));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, add_param_to_op(&op, NULL));

  /* copy_schema_ref_basic */
  ASSERT_EQ(CDD_C_SUCCESS, copy_schema_ref_basic(NULL, &schema));
  ASSERT_EQ(CDD_C_SUCCESS, copy_schema_ref_basic(&schema, NULL));

  /* set_querystring_schema_from_type_map */
  ASSERT_EQ(CDD_C_SUCCESS, set_querystring_schema_from_type_map(NULL, &tm));
  ASSERT_EQ(CDD_C_SUCCESS, set_querystring_schema_from_type_map(&param, NULL));

  /* apply_format_to_schema_ref */
  ASSERT_EQ(CDD_C_SUCCESS,
            apply_format_to_schema_ref(NULL, &tm, "fmt", &bool_out));
  ASSERT_EQ(CDD_C_SUCCESS,
            apply_format_to_schema_ref(&schema, NULL, "fmt", &bool_out));
  ASSERT_EQ(CDD_C_SUCCESS,
            apply_format_to_schema_ref(&schema, &tm, "fmt", NULL));

  PASS();
}

TEST test_operation_100_percent_coverage(void) {
  struct OpenAPI_Link link;
  struct OpenAPI_Parameter param;
  struct OpenAPI_Response resp;
  struct OpenAPI_Operation op;
  struct OpenAPI_MediaType mt;
  struct OpenAPI_Any any;
  struct OpenAPI_Server srv;
  struct DocResponseHeader dh;
  struct DocLink dl;
  struct DocServer ds;
  struct DocServerVar dsv;
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct C2OpenAPI_ParsedArg args[2];
  struct DocMetadata doc;
  struct DocResponse ret;
  char *enums[2];
  JSON_Value *val_null;
  JSON_Value *val_obj;
  struct OpenAPI_LinkParam *lp = NULL;
  size_t lp_count = 0;

  memset(&link, 0, sizeof(link));
  memset(&param, 0, sizeof(param));
  memset(&resp, 0, sizeof(resp));
  memset(&op, 0, sizeof(op));
  memset(&mt, 0, sizeof(mt));
  memset(&any, 0, sizeof(any));
  memset(&srv, 0, sizeof(srv));
  memset(&dh, 0, sizeof(dh));
  memset(&dl, 0, sizeof(dl));
  memset(&ds, 0, sizeof(ds));
  memset(&dsv, 0, sizeof(dsv));
  memset(&ctx, 0, sizeof(ctx));
  memset(&sig, 0, sizeof(sig));
  memset(args, 0, sizeof(args));
  memset(&doc, 0, sizeof(doc));
  memset(&ret, 0, sizeof(ret));

  /* 1. cleanup_link_fields */

  ASSERT_EQ(CDD_C_SUCCESS, cleanup_link_fields(NULL));
  link.server =
      (struct OpenAPI_Server *)calloc(1, sizeof(struct OpenAPI_Server));
  ASSERT(link.server != NULL);
  link.server->name = (char *)malloc(12);
  ASSERT(link.server->name != NULL);
  CDD_STRCPY(link.server->name, 12, "ServerName");
  link.server->url = (char *)malloc(20);
  ASSERT(link.server->url != NULL);
  CDD_STRCPY(link.server->url, 20, "http://example.com");
  link.server->description = (char *)malloc(12);
  ASSERT(link.server->description != NULL);
  CDD_STRCPY(link.server->description, 12, "ServerDesc");
  link.server_set = 1;
  ASSERT_EQ(CDD_C_SUCCESS, cleanup_link_fields(&link));

  /* 2. free_param_fields */
  ASSERT_EQ(CDD_C_SUCCESS, free_param_fields(NULL));
  param.name = (char *)malloc(5);
  ASSERT(param.name != NULL);
  CDD_STRCPY(param.name, 5, "name");
  param.type = (char *)malloc(5);
  ASSERT(param.type != NULL);
  CDD_STRCPY(param.type, 5, "type");
  param.description = (char *)malloc(5);
  ASSERT(param.description != NULL);
  CDD_STRCPY(param.description, 5, "desc");
  param.items_type = (char *)malloc(5);
  ASSERT(param.items_type != NULL);
  CDD_STRCPY(param.items_type, 5, "item");
  param.content_type = (char *)malloc(5);
  ASSERT(param.content_type != NULL);
  CDD_STRCPY(param.content_type, 5, "text");
  param.schema.ref_name = (char *)malloc(5);
  ASSERT(param.schema.ref_name != NULL);
  CDD_STRCPY(param.schema.ref_name, 5, "rnam");
  param.schema.ref = (char *)malloc(5);
  ASSERT(param.schema.ref != NULL);
  CDD_STRCPY(param.schema.ref, 5, "href");
  param.schema.inline_type = (char *)malloc(5);
  ASSERT(param.schema.inline_type != NULL);
  CDD_STRCPY(param.schema.inline_type, 5, "int");
  param.schema.items_ref = (char *)malloc(5);
  ASSERT(param.schema.items_ref != NULL);
  CDD_STRCPY(param.schema.items_ref, 5, "iref");
  param.schema.format = (char *)malloc(5);
  ASSERT(param.schema.format != NULL);
  CDD_STRCPY(param.schema.format, 5, "fmt");
  param.schema.items_format = (char *)malloc(5);
  ASSERT(param.schema.items_format != NULL);
  CDD_STRCPY(param.schema.items_format, 5, "ifmt");
  param.schema.content_media_type = (char *)malloc(5);
  ASSERT(param.schema.content_media_type != NULL);
  CDD_STRCPY(param.schema.content_media_type, 5, "cmed");
  param.schema.content_encoding = (char *)malloc(5);
  ASSERT(param.schema.content_encoding != NULL);
  CDD_STRCPY(param.schema.content_encoding, 5, "cenc");
  param.schema.items_content_media_type = (char *)malloc(5);
  ASSERT(param.schema.items_content_media_type != NULL);
  CDD_STRCPY(param.schema.items_content_media_type, 5, "imed");
  param.schema.items_content_encoding = (char *)malloc(5);
  ASSERT(param.schema.items_content_encoding != NULL);
  CDD_STRCPY(param.schema.items_content_encoding, 5, "ienc");
  param.example_set = 1;
  param.example.type = OA_ANY_STRING;
  param.example.string = (char *)malloc(5);
  ASSERT(param.example.string != NULL);
  CDD_STRCPY(param.example.string, 5, "str");
  ASSERT_EQ(CDD_C_SUCCESS, free_param_fields(&param));

  param.example_set = 1;
  param.example.type = OA_ANY_JSON;
  param.example.json = (char *)malloc(5);
  ASSERT(param.example.json != NULL);
  CDD_STRCPY(param.example.json, 5, "{}");
  ASSERT_EQ(CDD_C_SUCCESS, free_param_fields(&param));

  /* 3. any_from_json_value */
  val_null = json_value_init_null();
  ASSERT_EQ(CDD_C_SUCCESS, any_from_json_value(val_null, &any));
  ASSERT_EQ(OA_ANY_NULL, any.type);
  json_value_free(val_null);

  val_obj = json_parse_string("{\"key\": \"value\"}");
  ASSERT(val_obj != NULL);
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, any_from_json_value(val_obj, &any));
  g_cdd_strdup_fail = 0;
  json_value_free(val_obj);

  /* 4. parse_link_params_json calloc OOM */
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            parse_link_params_json("{\"param1\": \"value1\"}", &lp, &lp_count));
  g_cdd_alloc_fail = 0;

  /* 5. copy_doc_server_variables_op calloc OOM */
  dsv.name = (char *)(size_t) "env";
  dsv.default_value = (char *)(size_t) "dev";
  enums[0] = (char *)(size_t) "dev";
  enums[1] = (char *)(size_t) "prod";
  dsv.enum_values = enums;
  dsv.n_enum_values = 2;
  ds.url = (char *)(size_t) "https://api.example.com";
  ds.variables = &dsv;
  ds.n_variables = 1;

  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, copy_doc_server_variables_op(&srv, &ds));
  g_cdd_alloc_fail = 0;

  g_cdd_alloc_fail = 2;
  (void)copy_doc_server_variables_op(&srv, &ds);
  g_cdd_alloc_fail = 0;

  /* 6. add_header_to_response realloc OOM */
  dh.name = (char *)(size_t) "X-Custom";
  dh.type = (char *)(size_t) "string";
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, add_header_to_response(&resp, &dh));
  g_cdd_alloc_fail = 0;

  /* 7. add_link_to_response realloc & calloc OOM */
  dl.name = (char *)(size_t) "LinkName";
  dl.operation_id = (char *)(size_t) "opId";
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, add_link_to_response(&resp, &dl));
  g_cdd_alloc_fail = 0;

  dl.server_url = (char *)(size_t) "http://server.com";
  g_cdd_alloc_fail = 2;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, add_link_to_response(&resp, &dl));
  g_cdd_alloc_fail = 0;

  /* 8. add_param_to_op realloc OOM */
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, add_param_to_op(&op, &param));
  g_cdd_alloc_fail = 0;

  /* 9. schema_ref_has_data_basic failure hook */
  resp.content_type = (char *)(size_t) "application/json";
  g_cdd_fail_schema_ref_has_data = 1;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            init_media_type_from_response(&mt, "application/json", &resp, 0));
  g_cdd_fail_schema_ref_has_data = 0;

  op.req_body.content_type = (char *)(size_t) "application/json";
  g_cdd_fail_schema_ref_has_data = 1;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            init_media_type_from_request_body(&mt, "application/json", &op, 0));
  g_cdd_fail_schema_ref_has_data = 0;

  /* 10. add_response_media_type & add_request_body_media_type calloc & realloc
   * OOM */
  memset(&resp, 0, sizeof(resp));
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            add_response_media_type(&resp, "application/json", 0));
  g_cdd_alloc_fail = 0;

  ASSERT_EQ(CDD_C_SUCCESS,
            add_response_media_type(&resp, "application/json", 0));
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            add_response_media_type(&resp, "text/plain", 0));
  g_cdd_alloc_fail = 0;

  memset(&op, 0, sizeof(op));
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            add_request_body_media_type(&op, "application/json", 0));
  g_cdd_alloc_fail = 0;

  ASSERT_EQ(CDD_C_SUCCESS,
            add_request_body_media_type(&op, "application/json", 0));
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            add_request_body_media_type(&op, "text/plain", 0));
  g_cdd_alloc_fail = 0;

  /* 11. Primitive output argument (hits lines 2100-2102) */
  memset(&op, 0, sizeof(op));
  ctx.sig = &sig;
  ctx.doc = &doc;
  ctx.func_name = "api_test_primitive_out";
  sig.args = args;
  sig.n_args = 1;
  args[0].name = (char *)(size_t) "out_val";
  args[0].type = (char *)(size_t) "int **";
  ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
  ASSERT_EQ(1, op.n_responses);
  ASSERT_STR_EQ("integer", op.responses[0].schema.inline_type);
  reset_operation_test(&op);

  /* 12. ensure_response_for_code realloc OOM */
  memset(&op, 0, sizeof(op));
  {
    struct OpenAPI_Response *out_resp = NULL;
    g_cdd_alloc_fail = 1;
    ASSERT_EQ(CDD_C_SUCCESS, ensure_response_for_code(&op, "200", &out_resp));
    ASSERT_EQ(NULL, out_resp);
    g_cdd_alloc_fail = 0;
  }

  /* 13. New response in doc.returns realloc OOM (line 2495) */
  memset(&op, 0, sizeof(op));
  ret.code = (char *)(size_t) "404";
  doc.returns = &ret;
  doc.n_returns = 1;
  sig.n_args = 0;
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, c2openapi_build_operation(&ctx, &op));
  g_cdd_alloc_fail = 0;
  reset_operation_test(&op);

  /* 14. Fallback 200 response realloc OOM (line 2608) */
  memset(&op, 0, sizeof(op));
  doc.returns = NULL;
  doc.n_returns = 0;
  sig.n_args = 0;
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, c2openapi_build_operation(&ctx, &op));
  g_cdd_alloc_fail = 0;
  reset_operation_test(&op);

  /* 15. Doc response header with non-200 code (hits line 2565 branch) */
  {
    struct DocResponseHeader drh;
    memset(&drh, 0, sizeof(drh));
    drh.code = (char *)(size_t) "400";
    drh.name = (char *)(size_t) "X-Err";
    drh.type = (char *)(size_t) "string";
    doc.response_headers = &drh;
    doc.n_response_headers = 1;
    ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
    reset_operation_test(&op);
    doc.response_headers = NULL;
    doc.n_response_headers = 0;
  }

  /* 16. Doc link with non-200 code (hits line 2594 branch) */
  {
    struct DocLink dlk;
    memset(&dlk, 0, sizeof(dlk));
    dlk.code = (char *)(size_t) "400";
    dlk.name = (char *)(size_t) "ErrLink";
    dlk.operation_id = (char *)(size_t) "handleErr";
    doc.links = &dlk;
    doc.n_links = 1;
    ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
    reset_operation_test(&op);
    doc.links = NULL;
    doc.n_links = 0;
  }

  /* 17. any_from_json_value json_serialize failure (line 106) */
  val_obj = json_parse_string("{\"key\": \"value\"}");
  ASSERT(val_obj != NULL);
  g_cdd_fail_json_serialize = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, any_from_json_value(val_obj, &any));
  g_cdd_fail_json_serialize = 0;
  json_value_free(val_obj);

  /* 18. any_from_json_value default branch (line 113) */
  {
    char dummy_val_buf[64];
    memset(dummy_val_buf, 0, sizeof(dummy_val_buf));
    ASSERT_EQ(CDD_C_SUCCESS,
              any_from_json_value((const JSON_Value *)dummy_val_buf, &any));
  }

  /* 19. add_header_to_response existing header content_type OOM (line 667) &
   * loop exit (line 676) */
  memset(&resp, 0, sizeof(resp));
  dh.name = (char *)(size_t) "X-Custom";
  dh.type = (char *)(size_t) "string";
  dh.content_type = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, add_header_to_response(&resp, &dh));
  /* Loop exit when header name differs (hits line 676) */
  dh.name = (char *)(size_t) "X-Different";
  ASSERT_EQ(CDD_C_SUCCESS, add_header_to_response(&resp, &dh));
  /* Matching header OOM on content_type (hits line 667) */
  dh.name = (char *)(size_t) "X-Custom";
  dh.content_type = (char *)(size_t) "application/json";
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, add_header_to_response(&resp, &dh));
  g_cdd_strdup_fail = 0;

  /* 20. add_link_to_response duplicate link name (line 854) & loop exit (line
   * 863) */
  memset(&resp, 0, sizeof(resp));
  dl.name = (char *)(size_t) "LinkName";
  dl.operation_id = (char *)(size_t) "opId";
  ASSERT_EQ(CDD_C_SUCCESS, add_link_to_response(&resp, &dl));
  /* Loop exit when link name differs (hits line 863) */
  dl.name = (char *)(size_t) "AnotherLink";
  ASSERT_EQ(CDD_C_SUCCESS, add_link_to_response(&resp, &dl));
  /* Duplicate link name (hits line 854) */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, add_link_to_response(&resp, &dl));

  /* 21. Output arg with apply_format_to_schema_ref failure (lines 2097-2098) */
  memset(&op, 0, sizeof(op));
  ctx.sig = &sig;
  ctx.doc = &doc;
  ctx.func_name = "api_test_out_fmt_fail";
  sig.args = args;
  sig.n_args = 1;
  args[0].name = (char *)(size_t) "out_val";
  args[0].type = (char *)(size_t) "int **";
  g_cdd_fail_apply_format = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, c2openapi_build_operation(&ctx, &op));
  g_cdd_fail_apply_format = 0;
  reset_operation_test(&op);

  /* 22. Body arg with apply_format_to_schema_ref failure (lines 2126-2150) */
  memset(&op, 0, sizeof(op));
  ctx.func_name = "api_post_user";
  args[0].name = (char *)(size_t) "in_body";
  args[0].type = (char *)(size_t) "const struct User *";
  g_cdd_fail_apply_format = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, c2openapi_build_operation(&ctx, &op));
  g_cdd_fail_apply_format = 0;
  reset_operation_test(&op);

  /* Body arg with inline_type format failure (lines 2146-2148) */
  memset(&op, 0, sizeof(op));
  args[0].type = (char *)(size_t) "int";
  {
    struct DocParam dp_body;
    memset(&dp_body, 0, sizeof(dp_body));
    dp_body.name = (char *)(size_t) "in_body";
    dp_body.in_loc = (char *)(size_t) "body";
    doc.params = &dp_body;
    doc.n_params = 1;
    g_cdd_fail_apply_format = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, c2openapi_build_operation(&ctx, &op));
    g_cdd_fail_apply_format = 0;
    doc.params = NULL;
    doc.n_params = 0;
    reset_operation_test(&op);
  }

  /* 23. Param description OOM (lines 2169-2171) */
  memset(&op, 0, sizeof(op));
  ctx.func_name = "api_user_get";
  args[0].name = (char *)(size_t) "user_id";
  args[0].type = (char *)(size_t) "int";
  {
    struct DocParam dp_desc;
    int k_step;
    memset(&dp_desc, 0, sizeof(dp_desc));
    dp_desc.name = (char *)(size_t) "user_id";
    dp_desc.description = (char *)(size_t) "User identifier";
    doc.params = &dp_desc;
    doc.n_params = 1;
    for (k_step = 1; k_step <= 6; ++k_step) {
      g_cdd_strdup_fail = k_step;
      (void)c2openapi_build_operation(&ctx, &op);
      g_cdd_strdup_fail = 0;
      reset_operation_test(&op);
    }
    doc.params = NULL;
    doc.n_params = 0;
  }

  /* 24. Param example error percolation (lines 2303-2305) */
  memset(&op, 0, sizeof(op));
  ctx.func_name = "api_user_get";
  args[0].name = (char *)(size_t) "page";
  args[0].type = (char *)(size_t) "int";
  {
    struct DocParam dp_ex;
    memset(&dp_ex, 0, sizeof(dp_ex));
    dp_ex.name = (char *)(size_t) "page";
    dp_ex.example = (char *)(size_t) "{invalid_json";
    doc.params = &dp_ex;
    doc.n_params = 1;
    /* In parse_example_any, invalid JSON falls back to OA_ANY_STRING unless
     * strdup fails */
    /* If 4th strdup fails (1: op_id, 2: name, 3: example in parse_example_any):
     */
    g_cdd_strdup_fail = 3;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, c2openapi_build_operation(&ctx, &op));
    g_cdd_strdup_fail = 0;
    doc.params = NULL;
    doc.n_params = 0;
    reset_operation_test(&op);
  }

  /* 25. Existing response description update (lines 2445-2451) */
  memset(&op, 0, sizeof(op));
  ctx.func_name = "api_user_get";
  sig.n_args = 1;
  args[0].name = (char *)(size_t) "out_user";
  args[0].type = (char *)(size_t) "struct User **";
  {
    struct DocResponse ret_desc;
    memset(&ret_desc, 0, sizeof(ret_desc));
    ret_desc.code = (char *)(size_t) "200";
    ret_desc.description = (char *)(size_t) "Successful retrieval";
    doc.returns = &ret_desc;
    doc.n_returns = 1;
    ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
    ASSERT_EQ(1, op.n_responses);
    ASSERT_STR_EQ("Successful retrieval", op.responses[0].description);
    reset_operation_test(&op);

    /* OOM when duplicating description: 1st is op_id, 2nd is code 200, 3rd is
     * "Success", 4th is return description */
    g_cdd_strdup_fail = 4;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, c2openapi_build_operation(&ctx, &op));
    g_cdd_strdup_fail = 0;
    reset_operation_test(&op);

    doc.returns = NULL;
    doc.n_returns = 0;
  }

  /* 26. Request body description and content_type (lines 2417, 2431) */
  memset(&op, 0, sizeof(op));
  ctx.func_name = "api_post_user";
  sig.n_args = 0;
  {
    int k_step;
    doc.request_body_description = (char *)(size_t) "Request payload";
    doc.request_body_content_type = (char *)(size_t) "application/json";
    doc.n_request_bodies = 0;
    ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
    reset_operation_test(&op);

    for (k_step = 1; k_step <= 4; ++k_step) {
      g_cdd_strdup_fail = k_step;
      (void)c2openapi_build_operation(&ctx, &op);
      g_cdd_strdup_fail = 0;
      reset_operation_test(&op);
    }

    doc.request_body_description = NULL;
    doc.request_body_content_type = NULL;
  }

  /* 27. Response header description OOM (line 2566) */
  {
    struct DocResponseHeader drh;
    int k_step;
    memset(&drh, 0, sizeof(drh));
    drh.code = (char *)(size_t) "200";
    drh.name = (char *)(size_t) "X-Res";
    drh.type = (char *)(size_t) "string";
    doc.response_headers = &drh;
    doc.n_response_headers = 1;
    for (k_step = 1; k_step <= 5; ++k_step) {
      g_cdd_strdup_fail = k_step;
      (void)c2openapi_build_operation(&ctx, &op);
      g_cdd_strdup_fail = 0;
      reset_operation_test(&op);
    }
    doc.response_headers = NULL;
    doc.n_response_headers = 0;
  }

  /* 28. Link description OOM (line 2595) */
  {
    struct DocLink dlk;
    int k_step;
    memset(&dlk, 0, sizeof(dlk));
    dlk.code = (char *)(size_t) "200";
    dlk.name = (char *)(size_t) "MyLink";
    dlk.operation_id = (char *)(size_t) "myOp";
    doc.links = &dlk;
    doc.n_links = 1;
    for (k_step = 1; k_step <= 5; ++k_step) {
      g_cdd_strdup_fail = k_step;
      (void)c2openapi_build_operation(&ctx, &op);
      g_cdd_strdup_fail = 0;
      reset_operation_test(&op);
    }
    doc.links = NULL;
    doc.n_links = 0;
  }

  /* 29. Fallback 200 description OOM (line 2623) */
  memset(&op, 0, sizeof(op));
  sig.n_args = 0;
  doc.returns = NULL;
  doc.n_returns = 0;
  /* 1st is op_id, 2nd is code "200", 3rd is description "Success" */
  g_cdd_strdup_fail = 3;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, c2openapi_build_operation(&ctx, &op));
  g_cdd_strdup_fail = 0;
  reset_operation_test(&op);

  PASS();
}

TEST test_operation_100_percent_coverage_part2(void) {
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, find_doc_param(NULL, "x", NULL));

  {
    struct OpenAPI_Any a;
    memset(&a, 0, sizeof(a));
    a.type = OA_ANY_STRING;
    a.string = NULL;
    free_any_value_local(&a);
    a.type = OA_ANY_JSON;
    a.json = NULL;
    free_any_value_local(&a);
  }

  {
    struct OpenAPI_Operation op_resp;
    struct OpenAPI_Response resps[2];
    struct OpenAPI_Response *found_r = NULL;
    memset(&op_resp, 0, sizeof(op_resp));
    memset(resps, 0, sizeof(resps));
    resps[0].code = NULL;
    resps[1].code = (char *)(size_t) "200";
    op_resp.responses = resps;
    op_resp.n_responses = 2;
    ASSERT_EQ(CDD_C_SUCCESS, find_response_by_code(&op_resp, "200", &found_r));
    ASSERT(found_r != NULL);
  }

  {
    struct OpenAPI_MediaType mts[2];
    struct OpenAPI_MediaType *found_mt = NULL;
    memset(mts, 0, sizeof(mts));
    mts[0].name = NULL;
    mts[1].name = (char *)(size_t) "text/plain";
    ASSERT_EQ(CDD_C_SUCCESS,
              find_media_type_op(mts, 2, "text/plain", &found_mt));
    ASSERT(found_mt != NULL);
  }

  {
    struct OpenAPI_Response resp_hdr;
    struct DocResponseHeader dh;
    memset(&resp_hdr, 0, sizeof(resp_hdr));
    resp_hdr.headers =
        (struct OpenAPI_Header *)calloc(1, sizeof(struct OpenAPI_Header));
    resp_hdr.headers[0].name = NULL;
    resp_hdr.n_headers = 1;
    memset(&dh, 0, sizeof(dh));
    dh.name = (char *)(size_t) "X-Custom";
    dh.type = (char *)(size_t) "string";
    dh.description = (char *)(size_t) "desc";
    dh.content_type = (char *)(size_t) "text/plain";
    ASSERT_EQ(CDD_C_SUCCESS, add_header_to_response(&resp_hdr, &dh));
    free(resp_hdr.headers[1].name);
    free(resp_hdr.headers[1].type);
    free(resp_hdr.headers[1].description);
    free(resp_hdr.headers[1].content_type);
    free(resp_hdr.headers);
  }

  {
    struct OpenAPI_Response resp_lnk;
    struct DocLink dl;
    memset(&resp_lnk, 0, sizeof(resp_lnk));
    resp_lnk.links =
        (struct OpenAPI_Link *)calloc(1, sizeof(struct OpenAPI_Link));
    resp_lnk.links[0].name = NULL;
    resp_lnk.n_links = 1;
    memset(&dl, 0, sizeof(dl));
    dl.name = (char *)(size_t) "Link1";
    dl.operation_id = (char *)(size_t) "op1";
    ASSERT_EQ(CDD_C_SUCCESS, add_link_to_response(&resp_lnk, &dl));
    g_cdd_strdup_fail = 1;
    dl.name = (char *)(size_t) "Link2";
    ASSERT_EQ(CDD_C_ERROR_MEMORY, add_link_to_response(&resp_lnk, &dl));
    g_cdd_strdup_fail = 0;
    cleanup_link_fields(&resp_lnk.links[1]);
    free(resp_lnk.links);
  }

  {
    int d_ptr = 0, is_st = 0;
    ASSERT_EQ(CDD_C_SUCCESS, is_struct_pointer("struct Foo*", NULL, NULL));
    ASSERT_EQ(CDD_C_SUCCESS, is_struct_pointer("struct Foo*", &d_ptr, &is_st));
    ASSERT_EQ(CDD_C_SUCCESS, is_struct_pointer("struct Foo**", &d_ptr, &is_st));
    ASSERT_EQ(CDD_C_SUCCESS, is_struct_pointer("*struct Foo", &d_ptr, &is_st));
  }

  {
    struct OpenAPI_MediaType mt;
    struct OpenAPI_Operation op;
    memset(&op, 0, sizeof(op));
    g_cdd_fail_schema_ref_has_data = 1;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              init_media_type_from_request_body(&mt, "app/json", &op, 0));
    g_cdd_fail_schema_ref_has_data = 0;
  }

  {
    struct OpBuilderContext ctx;
    struct C2OpenAPI_ParsedSig sig;
    struct OpenAPI_Operation op;
    memset(&ctx, 0, sizeof(ctx));
    memset(&sig, 0, sizeof(sig));
    memset(&op, 0, sizeof(op));
    ctx.sig = &sig;
    ctx.func_name = "do_something";
    ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
    ASSERT_EQ(OA_VERB_GET, op.verb);
    reset_operation_test(&op);
  }

  {
    struct OpBuilderContext ctx;
    struct C2OpenAPI_ParsedSig sig;
    struct C2OpenAPI_ParsedArg arg;
    struct OpenAPI_Operation op;
    memset(&ctx, 0, sizeof(ctx));
    memset(&sig, 0, sizeof(sig));
    memset(&arg, 0, sizeof(arg));
    memset(&op, 0, sizeof(op));
    arg.name = (char *)(size_t) "out";
    arg.type = (char *)(size_t) "struct Foo **";
    sig.args = &arg;
    sig.n_args = 1;
    ctx.sig = &sig;
    ctx.func_name = "api_user_get";
    g_cdd_fail_apply_format = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, c2openapi_build_operation(&ctx, &op));
    g_cdd_fail_apply_format = 0;
    reset_operation_test(&op);
  }

  {
    struct OpBuilderContext ctx;
    struct C2OpenAPI_ParsedSig sig;
    struct C2OpenAPI_ParsedArg arg;
    struct DocMetadata doc;
    struct DocParam dp;
    struct OpenAPI_Operation op;
    memset(&ctx, 0, sizeof(ctx));
    memset(&sig, 0, sizeof(sig));
    memset(&arg, 0, sizeof(arg));
    memset(&doc, 0, sizeof(doc));
    memset(&dp, 0, sizeof(dp));
    memset(&op, 0, sizeof(op));
    arg.name = (char *)(size_t) "in";
    arg.type = (char *)(size_t) "struct Foo *";
    sig.args = &arg;
    sig.n_args = 1;
    dp.name = (char *)(size_t) "in";
    dp.in_loc = (char *)(size_t) "body";
    doc.params = &dp;
    doc.n_params = 1;
    ctx.sig = &sig;
    ctx.doc = &doc;
    ctx.func_name = "api_post_user";
    g_cdd_fail_apply_format = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, c2openapi_build_operation(&ctx, &op));
    g_cdd_fail_apply_format = 0;
    reset_operation_test(&op);
  }

  {
    struct OpBuilderContext ctx;
    struct C2OpenAPI_ParsedSig sig;
    struct C2OpenAPI_ParsedArg arg;
    struct OpenAPI_Operation op;
    memset(&ctx, 0, sizeof(ctx));
    memset(&sig, 0, sizeof(sig));
    memset(&arg, 0, sizeof(arg));
    memset(&op, 0, sizeof(op));
    arg.name = (char *)(size_t) "items";
    arg.type = (char *)(size_t) "int[]";
    sig.args = &arg;
    sig.n_args = 1;
    ctx.sig = &sig;
    ctx.func_name = "api_user_get";
    g_cdd_strdup_fail = 3;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, c2openapi_build_operation(&ctx, &op));
    g_cdd_strdup_fail = 0;
    reset_operation_test(&op);

    g_cdd_strdup_fail = 4;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, c2openapi_build_operation(&ctx, &op));
    g_cdd_strdup_fail = 0;
    reset_operation_test(&op);
  }

  {
    struct OpBuilderContext ctx;
    struct C2OpenAPI_ParsedSig sig;
    struct DocMetadata doc;
    struct DocRequestBody rb;
    struct DocEncoding encs[2];
    struct OpenAPI_Operation op;
    memset(&ctx, 0, sizeof(ctx));
    memset(&sig, 0, sizeof(sig));
    memset(&doc, 0, sizeof(doc));
    memset(&rb, 0, sizeof(rb));
    memset(encs, 0, sizeof(encs));
    memset(&op, 0, sizeof(op));

    rb.content_type = (char *)(size_t) "application/json";
    doc.request_bodies = &rb;
    doc.n_request_bodies = 1;

    encs[0].name = (char *)(size_t) "field1";
    encs[0].content_type = (char *)(size_t) "application/xml";
    encs[0].kind = 1;

    encs[1].name = (char *)(size_t) "field2";
    encs[1].content_type = (char *)(size_t) "text/plain";
    encs[1].kind = 2;

    doc.encodings = encs;
    doc.n_encodings = 2;

    ctx.sig = &sig;
    ctx.doc = &doc;
    ctx.func_name = "api_user_post";

    ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
    reset_operation_test(&op);
  }

  {
    struct OpBuilderContext ctx;
    struct C2OpenAPI_ParsedSig sig;
    struct OpenAPI_Operation op;
    char *custom_tag = (char *)(size_t) "Custom";
    memset(&ctx, 0, sizeof(ctx));
    memset(&sig, 0, sizeof(sig));
    memset(&op, 0, sizeof(op));

    ctx.sig = &sig;
    ctx.func_name = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
    reset_operation_test(&op);

    memset(&op, 0, sizeof(op));
    op.tags = &custom_tag;
    op.n_tags = 1;
    ctx.func_name = "api_user_get";
    ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
    op.tags = NULL;
    op.n_tags = 0;
    reset_operation_test(&op);

    memset(&op, 0, sizeof(op));
    ctx.func_name = "api_user_get";
    g_cdd_alloc_fail = 2;
    ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
    g_cdd_alloc_fail = 0;
    reset_operation_test(&op);
  }

  {
    struct OpBuilderContext ctx;
    struct C2OpenAPI_ParsedSig sig;
    struct DocMetadata doc;
    struct DocServer srv;
    struct OpenAPI_Operation op;
    memset(&ctx, 0, sizeof(ctx));
    memset(&sig, 0, sizeof(sig));
    memset(&doc, 0, sizeof(doc));
    memset(&srv, 0, sizeof(srv));
    memset(&op, 0, sizeof(op));

    doc.external_docs_url = (char *)(size_t) "https://example.com/docs";
    doc.external_docs_description = NULL;

    srv.url = NULL;
    srv.name = NULL;
    srv.description = NULL;
    srv.n_variables = 0;
    doc.servers = &srv;
    doc.n_servers = 1;

    ctx.sig = &sig;
    ctx.doc = &doc;
    ctx.func_name = "api_user_get";

    ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
    reset_operation_test(&op);
  }

  /* Case 1: copy_any_value_local types */
  {
    struct OpenAPI_Any src, dst;
    memset(&src, 0, sizeof(src));
    src.type = OA_ANY_NUMBER;
    src.number = 42;
    ASSERT_EQ(CDD_C_SUCCESS, copy_any_value_local(&dst, &src));
    ASSERT_EQ(42, (int)dst.number);
    free_any_value_local(&dst);

    memset(&src, 0, sizeof(src));
    src.type = OA_ANY_BOOL;
    src.boolean = 1;
    ASSERT_EQ(CDD_C_SUCCESS, copy_any_value_local(&dst, &src));
    ASSERT_EQ(1, dst.boolean);
    free_any_value_local(&dst);

    memset(&src, 0, sizeof(src));
    src.type = OA_ANY_NULL;
    ASSERT_EQ(CDD_C_SUCCESS, copy_any_value_local(&dst, &src));
    ASSERT_EQ(OA_ANY_NULL, dst.type);
    free_any_value_local(&dst);

    memset(&src, 0, sizeof(src));
    src.type = OA_ANY_UNSET;
    ASSERT_EQ(CDD_C_SUCCESS, copy_any_value_local(&dst, &src));
    ASSERT_EQ(OA_ANY_UNSET, dst.type);
    free_any_value_local(&dst);
  }

  /* Case 2: DocServerVar with default_value == NULL and enum_values == NULL
   * with count > 0 */
  {
    struct OpenAPI_Server srv;
    struct DocServer dserver;
    struct DocServerVar svar;
    memset(&srv, 0, sizeof(srv));
    memset(&dserver, 0, sizeof(dserver));
    memset(&svar, 0, sizeof(svar));
    svar.name = (char *)(size_t) "env";
    svar.default_value = NULL;
    dserver.variables = &svar;
    dserver.n_variables = 1;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              copy_doc_server_variables_op(&srv, &dserver));

    svar.default_value = (char *)(size_t) "prod";
    svar.enum_values = NULL;
    svar.n_enum_values = 1;
    ASSERT_EQ(CDD_C_SUCCESS, copy_doc_server_variables_op(&srv, &dserver));
    free_openapi_server_variables_op(&srv);
  }

  /* Case 3: apply_example_to_response when content_type not found in
   * resp->content_media_types */
  {
    struct OpenAPI_Response resp;
    struct OpenAPI_MediaType mt;
    memset(&resp, 0, sizeof(resp));
    memset(&mt, 0, sizeof(mt));
    mt.name = (char *)(size_t) "application/json";
    resp.content_media_types = &mt;
    resp.n_content_media_types = 1;
    ASSERT_EQ(CDD_C_SUCCESS,
              apply_example_to_response(&resp, "ex", "nonexistent/type"));
  }

  /* Case 4: add_header_to_response when hdr->type == NULL and dh->type == NULL
   */
  {
    struct OpenAPI_Response resp;
    struct DocResponseHeader dh;
    memset(&resp, 0, sizeof(resp));
    memset(&dh, 0, sizeof(dh));
    dh.name = (char *)(size_t) "X-Test";
    dh.type = NULL;
    dh.format = (char *)(size_t) "int32";
    ASSERT_EQ(CDD_C_SUCCESS, add_header_to_response(&resp, &dh));
    dh.example = (char *)(size_t) "123";
    resp.headers[0].example_set = 1;
    ASSERT_EQ(CDD_C_SUCCESS, add_header_to_response(&resp, &dh));
    free(resp.headers[0].name);
    free(resp.headers[0].type);
    free(resp.headers[0].schema.inline_type);
    free(resp.headers[0].schema.format);
    free(resp.headers);
  }

  /* Case 5: cleanup_link_fields with link->parameters[p].name == NULL */
  {
    struct OpenAPI_Link lnk;
    memset(&lnk, 0, sizeof(lnk));
    lnk.parameters =
        (struct OpenAPI_LinkParam *)calloc(1, sizeof(struct OpenAPI_LinkParam));
    lnk.n_parameters = 1;
    cleanup_link_fields(&lnk);
  }

  /* Case 6: schema_ref_has_data_basic with is_array = 1 and ref_name == NULL */
  {
    struct OpenAPI_SchemaRef sref;
    int has_d = 0;
    memset(&sref, 0, sizeof(sref));
    sref.is_array = 1;
    ASSERT_EQ(CDD_C_SUCCESS, schema_ref_has_data_basic(&sref, &has_d));
    ASSERT_EQ(1, has_d);
  }

  /* Case 7: copy_schema_ref_basic with ref_name != NULL */
  {
    struct OpenAPI_SchemaRef src, dst;
    memset(&src, 0, sizeof(src));
    src.ref_name = (char *)(size_t) "MyRef";
    ASSERT_EQ(CDD_C_SUCCESS, copy_schema_ref_basic(&dst, &src));
    ASSERT(dst.ref_name != NULL);
    free(dst.ref_name);
  }

  /* Case 8: response_has_media_type and request_body_has_media_type name
   * mismatch */
  {
    struct OpenAPI_Response resp;
    struct OpenAPI_Operation op;
    struct OpenAPI_MediaType mt;
    int has_mt = 0;
    memset(&resp, 0, sizeof(resp));
    memset(&op, 0, sizeof(op));
    memset(&mt, 0, sizeof(mt));
    mt.name = (char *)(size_t) "application/json";
    resp.content_media_types = &mt;
    resp.n_content_media_types = 1;
    ASSERT_EQ(CDD_C_SUCCESS,
              response_has_media_type(&resp, "text/xml", &has_mt));
    ASSERT_EQ(0, has_mt);

    op.req_body_media_types = &mt;
    op.n_req_body_media_types = 1;
    ASSERT_EQ(CDD_C_SUCCESS,
              request_body_has_media_type(&op, "text/xml", &has_mt));
    ASSERT_EQ(0, has_mt);
  }

  /* Case 9: apply_format_to_schema_ref with override_format = "" and
   * out_applied = NULL */
  {
    struct OpenAPI_SchemaRef sref;
    struct OpenApiTypeMapping map;
    memset(&sref, 0, sizeof(sref));
    memset(&map, 0, sizeof(map));
    map.oa_type = (char *)(size_t) "string";
    map.oa_format = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, apply_format_to_schema_ref(&sref, &map, "", NULL));
    ASSERT_EQ(CDD_C_SUCCESS,
              apply_format_to_schema_ref(&sref, &map, "uuid", NULL));
    free(sref.inline_type);
    free(sref.format);
  }

  /* Case 10: doc_style_to_openapi DOC_PARAM_STYLE_UNSET */
  {
    enum OpenAPI_Style st = OA_STYLE_FORM;
    ASSERT_EQ(CDD_C_SUCCESS, doc_style_to_openapi(DOC_PARAM_STYLE_UNSET, &st));
    ASSERT_EQ(OA_STYLE_UNKNOWN, st);
  }

  /* Case 11: PUT verb with non-const struct pointer */
  {
    struct OpBuilderContext ctx;
    struct C2OpenAPI_ParsedSig sig;
    struct C2OpenAPI_ParsedArg arg;
    struct OpenAPI_Operation op;
    memset(&ctx, 0, sizeof(ctx));
    memset(&sig, 0, sizeof(sig));
    memset(&arg, 0, sizeof(arg));
    memset(&op, 0, sizeof(op));
    arg.name = (char *)(size_t) "in";
    arg.type = (char *)(size_t) "struct Foo *";
    sig.args = &arg;
    sig.n_args = 1;
    ctx.sig = &sig;
    ctx.func_name = "api_put_user";
    ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
    reset_operation_test(&op);
  }

  /* Case 12: Header parameter default style and query param style unset */
  {
    struct OpBuilderContext ctx;
    struct C2OpenAPI_ParsedSig sig;
    struct C2OpenAPI_ParsedArg arg;
    struct DocMetadata doc;
    struct DocParam dp;
    struct OpenAPI_Operation op;
    memset(&ctx, 0, sizeof(ctx));
    memset(&sig, 0, sizeof(sig));
    memset(&arg, 0, sizeof(arg));
    memset(&doc, 0, sizeof(doc));
    memset(&dp, 0, sizeof(dp));
    memset(&op, 0, sizeof(op));

    arg.name = (char *)(size_t) "X-Header";
    arg.type = (char *)(size_t) "char *";
    sig.args = &arg;
    sig.n_args = 1;
    dp.name = (char *)(size_t) "X-Header";
    dp.in_loc = (char *)(size_t) "header";
    dp.style = DOC_PARAM_STYLE_UNSET;
    dp.style_set = 1;
    doc.params = &dp;
    doc.n_params = 1;
    ctx.sig = &sig;
    ctx.doc = &doc;
    ctx.func_name = "api_user_get";

    ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
    reset_operation_test(&op);
  }

  /* Case 13: Querystring parameter example without content_type */
  {
    struct OpBuilderContext ctx;
    struct C2OpenAPI_ParsedSig sig;
    struct C2OpenAPI_ParsedArg arg;
    struct DocMetadata doc;
    struct DocParam dp;
    struct OpenAPI_Operation op;
    memset(&ctx, 0, sizeof(ctx));
    memset(&sig, 0, sizeof(sig));
    memset(&arg, 0, sizeof(arg));
    memset(&doc, 0, sizeof(doc));
    memset(&dp, 0, sizeof(dp));
    memset(&op, 0, sizeof(op));

    arg.name = (char *)(size_t) "qs";
    arg.type = (char *)(size_t) "char *";
    sig.args = &arg;
    sig.n_args = 1;
    dp.name = (char *)(size_t) "qs";
    dp.in_loc = (char *)(size_t) "querystring";
    dp.example = (char *)(size_t) "a=1";
    doc.params = &dp;
    doc.n_params = 1;
    ctx.sig = &sig;
    ctx.doc = &doc;
    ctx.func_name = "api_user_get";

    ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
    reset_operation_test(&op);
  }

  /* Case 14: Encoding with name == NULL and multiple item encodings */
  {
    struct OpBuilderContext ctx;
    struct C2OpenAPI_ParsedSig sig;
    struct DocMetadata doc;
    struct DocRequestBody rb;
    struct DocEncoding encs[2];
    struct OpenAPI_Operation op;
    memset(&ctx, 0, sizeof(ctx));
    memset(&sig, 0, sizeof(sig));
    memset(&doc, 0, sizeof(doc));
    memset(&rb, 0, sizeof(rb));
    memset(encs, 0, sizeof(encs));
    memset(&op, 0, sizeof(op));

    rb.content_type = (char *)(size_t) "application/json";
    doc.request_bodies = &rb;
    doc.n_request_bodies = 1;

    encs[0].name = NULL;
    encs[0].kind = 2;
    encs[1].name = NULL;
    encs[1].kind = 2;

    doc.encodings = encs;
    doc.n_encodings = 2;

    ctx.sig = &sig;
    ctx.doc = &doc;
    ctx.func_name = "api_user_post";

    ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
    reset_operation_test(&op);
  }

  /* Case 15: Returns with existing summary, code == NULL, and description ==
   * NULL */
  {
    struct OpBuilderContext ctx;
    struct C2OpenAPI_ParsedSig sig;
    struct DocMetadata doc;
    struct DocResponse rets[2];
    struct OpenAPI_Operation op;
    memset(&ctx, 0, sizeof(ctx));
    memset(&sig, 0, sizeof(sig));
    memset(&doc, 0, sizeof(doc));
    memset(rets, 0, sizeof(rets));
    memset(&op, 0, sizeof(op));

    rets[0].code = (char *)(size_t) "200";
    rets[0].summary = (char *)(size_t) "First summary";
    rets[0].description = NULL;
    rets[1].code = (char *)(size_t) "200";
    rets[1].summary = (char *)(size_t) "Second summary";
    rets[1].description = (char *)(size_t) "New desc";

    doc.returns = rets;
    doc.n_returns = 2;

    ctx.sig = &sig;
    ctx.doc = &doc;
    ctx.func_name = "api_user_get";

    ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
    reset_operation_test(&op);
  }

  /* Case 16: Return with code == NULL */
  {
    struct OpBuilderContext ctx;
    struct C2OpenAPI_ParsedSig sig;
    struct DocMetadata doc;
    struct DocResponse rets[2];
    struct OpenAPI_Operation op;
    memset(&ctx, 0, sizeof(ctx));
    memset(&sig, 0, sizeof(sig));
    memset(&doc, 0, sizeof(doc));
    memset(rets, 0, sizeof(rets));
    memset(&op, 0, sizeof(op));

    rets[0].code = (char *)(size_t) "200";
    rets[1].code = NULL;
    doc.returns = rets;
    doc.n_returns = 2;

    ctx.sig = &sig;
    ctx.doc = &doc;
    ctx.func_name = "api_user_get";

    ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
    reset_operation_test(&op);
  }

  /* Case 17: Tags strdup failure */
  {
    struct OpBuilderContext ctx;
    struct C2OpenAPI_ParsedSig sig;
    struct OpenAPI_Operation op;
    memset(&ctx, 0, sizeof(ctx));
    memset(&sig, 0, sizeof(sig));
    memset(&op, 0, sizeof(op));

    ctx.sig = &sig;
    ctx.func_name = "api_user_get";
    g_cdd_strdup_fail = 5;
    ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
    g_cdd_strdup_fail = 0;
    reset_operation_test(&op);
  }

  /* Case 18: copy_any_value_local and doc_style_to_openapi default cases */
  {
    struct OpenAPI_Any src, dst;
    enum OpenAPI_Style st = OA_STYLE_FORM;
    memset(&src, 0, sizeof(src));
    src.type = (enum OpenAPI_AnyType)99;
    ASSERT_EQ(CDD_C_SUCCESS, copy_any_value_local(&dst, &src));
    ASSERT_EQ(CDD_C_SUCCESS, doc_style_to_openapi((enum DocParamStyle)99, &st));
    ASSERT_EQ(OA_STYLE_UNKNOWN, st);
  }

  /* Case 19: copy_doc_server_variables_op enum_values != NULL && n_enum_values
   * == 0 */
  {
    struct OpenAPI_Server srv;
    struct DocServer dserver;
    struct DocServerVar svar;
    char *enums[1];
    memset(&srv, 0, sizeof(srv));
    memset(&dserver, 0, sizeof(dserver));
    memset(&svar, 0, sizeof(svar));
    enums[0] = (char *)(size_t) "v1";
    svar.name = (char *)(size_t) "env";
    svar.default_value = (char *)(size_t) "v1";
    svar.enum_values = enums;
    svar.n_enum_values = 0;
    dserver.variables = &svar;
    dserver.n_variables = 1;
    ASSERT_EQ(CDD_C_SUCCESS, copy_doc_server_variables_op(&srv, &dserver));
    free_openapi_server_variables_op(&srv);
  }

  /* Case 20: apply_example_to_response with mt->example_set == 1 and
   * n_content_media_types == 0 */
  {
    struct OpenAPI_Response resp;
    struct OpenAPI_MediaType mt;
    memset(&resp, 0, sizeof(resp));
    memset(&mt, 0, sizeof(mt));
    mt.name = (char *)(size_t) "application/json";
    mt.example_set = 1;
    resp.content_media_types = &mt;
    resp.n_content_media_types = 1;
    ASSERT_EQ(CDD_C_SUCCESS,
              apply_example_to_response(&resp, "ex", "application/json"));

    resp.n_content_media_types = 0;
    ASSERT_EQ(CDD_C_SUCCESS, apply_example_to_response(&resp, "ex", NULL));
  }

  /* Case 21: schema_ref_has_data_basic empty strings */
  {
    struct OpenAPI_SchemaRef sref;
    int has_d = 0;
    memset(&sref, 0, sizeof(sref));
    sref.inline_type = (char *)(size_t) "";
    ASSERT_EQ(CDD_C_SUCCESS, schema_ref_has_data_basic(&sref, &has_d));
    ASSERT_EQ(0, has_d);

    sref.inline_type = NULL;
    sref.ref_name = (char *)(size_t) "";
    ASSERT_EQ(CDD_C_SUCCESS, schema_ref_has_data_basic(&sref, &has_d));
    ASSERT_EQ(0, has_d);

    sref.ref_name = NULL;
    sref.ref = (char *)(size_t) "";
    ASSERT_EQ(CDD_C_SUCCESS, schema_ref_has_data_basic(&sref, &has_d));
    ASSERT_EQ(0, has_d);
  }

  /* Case 22: copy_schema_ref_basic strdup failure */
  {
    struct OpenAPI_SchemaRef src, dst;
    memset(&src, 0, sizeof(src));
    src.ref_name = (char *)(size_t) "MyRef";
    g_cdd_strdup_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, copy_schema_ref_basic(&dst, &src));
    g_cdd_strdup_fail = 0;
  }

  /* Case 23: response_has_media_type and request_body_has_media_type with
   * mt.name == NULL */
  {
    struct OpenAPI_Response resp;
    struct OpenAPI_Operation op;
    struct OpenAPI_MediaType mt;
    int has_mt = 0;
    memset(&resp, 0, sizeof(resp));
    memset(&op, 0, sizeof(op));
    memset(&mt, 0, sizeof(mt));
    mt.name = NULL;
    resp.content_media_types = &mt;
    resp.n_content_media_types = 1;
    ASSERT_EQ(CDD_C_SUCCESS,
              response_has_media_type(&resp, "text/xml", &has_mt));
    ASSERT_EQ(0, has_mt);

    op.req_body_media_types = &mt;
    op.n_req_body_media_types = 1;
    ASSERT_EQ(CDD_C_SUCCESS,
              request_body_has_media_type(&op, "text/xml", &has_mt));
    ASSERT_EQ(0, has_mt);
  }

  /* Case 24: apply_format_to_schema_ref with map->oa_format = "" */
  {
    struct OpenAPI_SchemaRef sref;
    struct OpenApiTypeMapping map;
    int applied = 0;
    memset(&sref, 0, sizeof(sref));
    memset(&map, 0, sizeof(map));
    map.oa_type = (char *)(size_t) "string";
    map.oa_format = (char *)(size_t) "";
    ASSERT_EQ(CDD_C_SUCCESS,
              apply_format_to_schema_ref(&sref, &map, NULL, &applied));
    ASSERT_EQ(0, applied);
  }

  /* Case 25: GET with non-const struct pointer */
  {
    struct OpBuilderContext ctx;
    struct C2OpenAPI_ParsedSig sig;
    struct C2OpenAPI_ParsedArg arg;
    struct OpenAPI_Operation op;
    memset(&ctx, 0, sizeof(ctx));
    memset(&sig, 0, sizeof(sig));
    memset(&arg, 0, sizeof(arg));
    memset(&op, 0, sizeof(op));
    arg.name = (char *)(size_t) "in";
    arg.type = (char *)(size_t) "struct Foo *";
    sig.args = &arg;
    sig.n_args = 1;
    ctx.sig = &sig;
    ctx.func_name = "api_user_get";
    ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
    reset_operation_test(&op);
  }

  /* Case 26: Output param and Body param with primitive oa_type and OOM */
  {
    struct OpBuilderContext ctx;
    struct C2OpenAPI_ParsedSig sig;
    struct C2OpenAPI_ParsedArg arg;
    struct DocMetadata doc;
    struct DocParam dp;
    struct OpenAPI_Operation op;

    memset(&ctx, 0, sizeof(ctx));
    memset(&sig, 0, sizeof(sig));
    memset(&arg, 0, sizeof(arg));
    memset(&op, 0, sizeof(op));
    arg.name = (char *)(size_t) "out";
    arg.type = (char *)(size_t) "int **";
    sig.args = &arg;
    sig.n_args = 1;
    ctx.sig = &sig;
    ctx.func_name = "api_user_get";
    ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
    reset_operation_test(&op);

    memset(&ctx, 0, sizeof(ctx));
    memset(&sig, 0, sizeof(sig));
    memset(&arg, 0, sizeof(arg));
    memset(&doc, 0, sizeof(doc));
    memset(&dp, 0, sizeof(dp));
    memset(&op, 0, sizeof(op));
    arg.name = (char *)(size_t) "in";
    arg.type = (char *)(size_t) "int *";
    sig.args = &arg;
    sig.n_args = 1;
    dp.name = (char *)(size_t) "in";
    dp.in_loc = (char *)(size_t) "body";
    doc.params = &dp;
    doc.n_params = 1;
    ctx.sig = &sig;
    ctx.doc = &doc;
    ctx.func_name = "api_post_user";
    ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
    reset_operation_test(&op);
  }

  /* Case 27: Querystring param with example and no style */
  {
    struct OpBuilderContext ctx;
    struct C2OpenAPI_ParsedSig sig;
    struct C2OpenAPI_ParsedArg arg;
    struct DocMetadata doc;
    struct DocParam dp;
    struct OpenAPI_Operation op;
    memset(&ctx, 0, sizeof(ctx));
    memset(&sig, 0, sizeof(sig));
    memset(&arg, 0, sizeof(arg));
    memset(&doc, 0, sizeof(doc));
    memset(&dp, 0, sizeof(dp));
    memset(&op, 0, sizeof(op));

    arg.name = (char *)(size_t) "qs";
    arg.type = (char *)(size_t) "int";
    sig.args = &arg;
    sig.n_args = 1;
    dp.name = (char *)(size_t) "qs";
    dp.in_loc = (char *)(size_t) "querystring";
    dp.example = (char *)(size_t) "10";
    doc.params = &dp;
    doc.n_params = 1;
    ctx.sig = &sig;
    ctx.doc = &doc;
    ctx.func_name = "api_user_get";
    ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
    reset_operation_test(&op);
  }

  /* Case 28: Doc with request_body_content_type and n_request_bodies > 0 */
  {
    struct OpBuilderContext ctx;
    struct C2OpenAPI_ParsedSig sig;
    struct DocMetadata doc;
    struct DocRequestBody rb;
    struct OpenAPI_Operation op;
    memset(&ctx, 0, sizeof(ctx));
    memset(&sig, 0, sizeof(sig));
    memset(&doc, 0, sizeof(doc));
    memset(&rb, 0, sizeof(rb));
    memset(&op, 0, sizeof(op));

    rb.content_type = (char *)(size_t) "application/json";
    doc.request_bodies = &rb;
    doc.n_request_bodies = 1;
    doc.request_body_content_type = (char *)(size_t) "application/xml";

    ctx.sig = &sig;
    ctx.doc = &doc;
    ctx.func_name = "api_user_post";
    ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
    reset_operation_test(&op);
  }

  /* Case 29: Allocation failure on prefix_encoding and item_encoding */
  {
    struct OpBuilderContext ctx;
    struct C2OpenAPI_ParsedSig sig;
    struct DocMetadata doc;
    struct DocRequestBody rb;
    struct DocEncoding enc;
    struct OpenAPI_Operation op;

    memset(&ctx, 0, sizeof(ctx));
    memset(&sig, 0, sizeof(sig));
    memset(&doc, 0, sizeof(doc));
    memset(&rb, 0, sizeof(rb));
    memset(&enc, 0, sizeof(enc));
    memset(&op, 0, sizeof(op));

    rb.content_type = (char *)(size_t) "application/json";
    doc.request_bodies = &rb;
    doc.n_request_bodies = 1;

    enc.name = (char *)(size_t) "f1";
    enc.content_type = (char *)(size_t) "text/plain";
    enc.kind = 1;
    doc.encodings = &enc;
    doc.n_encodings = 1;

    ctx.sig = &sig;
    ctx.doc = &doc;
    ctx.func_name = "api_user_post";

    g_cdd_alloc_fail = 2;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, c2openapi_build_operation(&ctx, &op));
    g_cdd_alloc_fail = 0;
    reset_operation_test(&op);

    enc.kind = 2;
    g_cdd_alloc_fail = 2;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, c2openapi_build_operation(&ctx, &op));
    g_cdd_alloc_fail = 0;
    reset_operation_test(&op);
  }

  /* Case 30: free_encoding_fields with non-null and null */
  {
    struct OpenAPI_Encoding test_enc;
    memset(&test_enc, 0, sizeof(test_enc));
    test_enc.name = (char *)malloc(10);
    test_enc.content_type = (char *)malloc(10);
    free_encoding_fields(&test_enc);
    free_encoding_fields(&test_enc);
    free_encoding_fields(NULL);
  }

  /* Case 31: add_header_to_response existing header with type == NULL and
   * example == NULL */
  {
    struct OpenAPI_Response resp_ex;
    struct OpenAPI_Header h_ex;
    struct DocResponseHeader dh_ex;
    memset(&resp_ex, 0, sizeof(resp_ex));
    memset(&h_ex, 0, sizeof(h_ex));
    memset(&dh_ex, 0, sizeof(dh_ex));
    h_ex.name = (char *)(size_t) "X-Null-Type";
    h_ex.type = NULL;
    resp_ex.headers = &h_ex;
    resp_ex.n_headers = 1;

    dh_ex.name = (char *)(size_t) "X-Null-Type";
    dh_ex.format = (char *)(size_t) "int32";
    dh_ex.example = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, add_header_to_response(&resp_ex, &dh_ex));
    free(h_ex.schema.inline_type);
    free(h_ex.schema.format);

    h_ex.type = (char *)(size_t) "string";
    h_ex.schema.inline_type = NULL;
    h_ex.schema.format = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, add_header_to_response(&resp_ex, &dh_ex));
    free(h_ex.schema.inline_type);
    free(h_ex.schema.format);
  }

  /* Case 32: copy_schema_ref_basic with ref_name == NULL */
  {
    struct OpenAPI_SchemaRef src_no_ref, dst_no_ref;
    memset(&src_no_ref, 0, sizeof(src_no_ref));
    src_no_ref.ref_name = NULL;
    src_no_ref.ref = (char *)(size_t) "SomeRef";
    ASSERT_EQ(CDD_C_SUCCESS, copy_schema_ref_basic(&dst_no_ref, &src_no_ref));
    free(dst_no_ref.ref);
  }

  /* Case 33: output param strdup failure loop */
  {
    struct OpBuilderContext ctx;
    struct C2OpenAPI_ParsedSig sig;
    struct C2OpenAPI_ParsedArg arg;
    struct OpenAPI_Operation op;
    int k;
    memset(&ctx, 0, sizeof(ctx));
    memset(&sig, 0, sizeof(sig));
    memset(&arg, 0, sizeof(arg));
    arg.name = (char *)(size_t) "out";
    arg.type = (char *)(size_t) "int **";
    sig.args = &arg;
    sig.n_args = 1;
    ctx.sig = &sig;
    ctx.func_name = "api_user_get";

    for (k = 1; k <= 8; ++k) {
      memset(&op, 0, sizeof(op));
      g_cdd_strdup_fail = k;
      (void)c2openapi_build_operation(&ctx, &op);
      g_cdd_strdup_fail = 0;
      reset_operation_test(&op);
    }
  }

  /* Case 34: body param strdup failure loop */
  {
    struct OpBuilderContext ctx;
    struct C2OpenAPI_ParsedSig sig;
    struct C2OpenAPI_ParsedArg arg;
    struct DocMetadata doc;
    struct DocParam dp;
    struct OpenAPI_Operation op;
    int k;
    memset(&ctx, 0, sizeof(ctx));
    memset(&sig, 0, sizeof(sig));
    memset(&arg, 0, sizeof(arg));
    memset(&doc, 0, sizeof(doc));
    memset(&dp, 0, sizeof(dp));
    arg.name = (char *)(size_t) "in";
    arg.type = (char *)(size_t) "int *";
    sig.args = &arg;
    sig.n_args = 1;
    dp.name = (char *)(size_t) "in";
    dp.in_loc = (char *)(size_t) "body";
    doc.params = &dp;
    doc.n_params = 1;
    ctx.sig = &sig;
    ctx.doc = &doc;
    ctx.func_name = "api_post_user";

    for (k = 1; k <= 8; ++k) {
      memset(&op, 0, sizeof(op));
      g_cdd_strdup_fail = k;
      (void)c2openapi_build_operation(&ctx, &op);
      g_cdd_strdup_fail = 0;
      reset_operation_test(&op);
    }
  }

  /* Case 35: array param strdup failure loop */
  {
    struct OpBuilderContext ctx;
    struct C2OpenAPI_ParsedSig sig;
    struct C2OpenAPI_ParsedArg arg;
    struct OpenAPI_Operation op;
    int k;

    memset(&ctx, 0, sizeof(ctx));
    memset(&sig, 0, sizeof(sig));
    memset(&arg, 0, sizeof(arg));
    arg.name = (char *)(size_t) "items[]";
    arg.type = (char *)(size_t) "int";
    sig.args = &arg;
    sig.n_args = 1;
    ctx.sig = &sig;
    ctx.func_name = "api_user_get";

    for (k = 0; k <= 10; ++k) {
      memset(&op, 0, sizeof(op));
      g_cdd_strdup_fail = k;
      (void)c2openapi_build_operation(&ctx, &op);
      g_cdd_strdup_fail = 0;
      reset_operation_test(&op);
    }

    arg.name = (char *)(size_t) "users[]";
    arg.type = (char *)(size_t) "struct User";
    for (k = 0; k <= 10; ++k) {
      memset(&op, 0, sizeof(op));
      g_cdd_strdup_fail = k;
      (void)c2openapi_build_operation(&ctx, &op);
      g_cdd_strdup_fail = 0;
      reset_operation_test(&op);
    }
  }

  PASS();
}

TEST test_operation_reach_100_percent(void) {
  /* 1-3. is_reserved_header_name stricmp fails */
  {
    int is_res = 0;
    g_cdd_fail_stricmp = 1;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              is_reserved_header_name("foo", &is_res));
    g_cdd_fail_stricmp = 2;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              is_reserved_header_name("foo", &is_res));
    g_cdd_fail_stricmp = 3;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              is_reserved_header_name("foo", &is_res));
    g_cdd_fail_stricmp = 0;
  }

  /* 4. find_response stricmp fail */
  {
    struct OpenAPI_Operation op;
    struct OpenAPI_Response r;
    struct OpenAPI_Response *out = NULL;
    memset(&op, 0, sizeof(op));
    memset(&r, 0, sizeof(r));
    r.code = (char *)(size_t) "200";
    op.responses = &r;
    op.n_responses = 1;
    g_cdd_fail_stricmp = 1;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              find_response_by_code(&op, "200", &out));
    g_cdd_fail_stricmp = 0;
  }

  /* 5. apply_example_to_response find_media_type_op fail */
  {
    struct OpenAPI_Response resp;
    struct OpenAPI_MediaType mt;
    memset(&resp, 0, sizeof(resp));
    memset(&mt, 0, sizeof(mt));
    mt.name = (char *)(size_t) "application/json";
    resp.content_media_types = &mt;
    resp.n_content_media_types = 1;
    g_op_fail_find_media_type_op = 1;
    ASSERT_EQ(
        CDD_C_ERROR_INVALID_ARGUMENT,
        apply_example_to_response(&resp, "{\"a\":1}", "application/json"));
    g_op_fail_find_media_type_op = 0;
  }

  /* 6. ensure_response_for_code find_response_by_code fail */
  {
    struct OpenAPI_Operation op;
    struct OpenAPI_Response r;
    struct OpenAPI_Response *out = NULL;
    memset(&op, 0, sizeof(op));
    memset(&r, 0, sizeof(r));
    r.code = (char *)(size_t) "200";
    op.responses = &r;
    op.n_responses = 1;
    g_cdd_fail_stricmp = 1;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              ensure_response_for_code(&op, "200", &out));
    g_cdd_fail_stricmp = 0;
  }

  /* 7. ensure_response_for_code stricmp 200 fail */
  {
    struct OpenAPI_Operation op;
    struct OpenAPI_Response *out = NULL;
    memset(&op, 0, sizeof(op));
    g_cdd_fail_stricmp = 1;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              ensure_response_for_code(&op, "200", &out));
    g_cdd_fail_stricmp = 0;
    if (op.responses)
      free(op.responses);
  }

  /* 8. add_header_to_response stricmp fail */
  {
    struct OpenAPI_Response resp;
    struct OpenAPI_Header hdr;
    struct DocResponseHeader dh;
    memset(&resp, 0, sizeof(resp));
    memset(&hdr, 0, sizeof(hdr));
    memset(&dh, 0, sizeof(dh));
    hdr.name = (char *)(size_t) "X-Test";
    resp.headers = &hdr;
    resp.n_headers = 1;
    dh.name = (char *)(size_t) "X-Test";
    g_cdd_fail_stricmp = 1;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, add_header_to_response(&resp, &dh));
    g_cdd_fail_stricmp = 0;
  }

  /* 9. apply_format_to_schema_ref oa_type_is_primitive fail */
  {
    struct OpenAPI_SchemaRef schema;
    struct OpenApiTypeMapping map;
    int applied = 0;
    memset(&schema, 0, sizeof(schema));
    memset(&map, 0, sizeof(map));
    map.oa_format = (char *)(size_t) "int32";
    map.oa_type = (char *)(size_t) "integer";
    g_op_fail_oa_type_is_primitive = 1;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              apply_format_to_schema_ref(&schema, &map, NULL, &applied));
    g_op_fail_oa_type_is_primitive = 0;
  }

  /* 10. c_mapping_init fail in c2openapi_build_operation */
  {
    struct OpBuilderContext ctx;
    struct C2OpenAPI_ParsedSig sig;
    struct C2OpenAPI_ParsedArg arg;
    struct OpenAPI_Operation op;
    memset(&ctx, 0, sizeof(ctx));
    memset(&sig, 0, sizeof(sig));
    memset(&arg, 0, sizeof(arg));
    memset(&op, 0, sizeof(op));
    arg.name = (char *)(size_t) "x";
    arg.type = (char *)(size_t) "int";
    sig.args = &arg;
    sig.n_args = 1;
    ctx.sig = &sig;
    ctx.func_name = "test_fn";
    g_mapping_fail_init = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, c2openapi_build_operation(&ctx, &op));
    g_mapping_fail_init = 0;
    reset_operation_test(&op);
  }

  /* 11. find_doc_param fail in c2openapi_build_operation */
  {
    struct OpBuilderContext ctx;
    struct C2OpenAPI_ParsedSig sig;
    struct C2OpenAPI_ParsedArg arg;
    struct OpenAPI_Operation op;
    memset(&ctx, 0, sizeof(ctx));
    memset(&sig, 0, sizeof(sig));
    memset(&arg, 0, sizeof(arg));
    memset(&op, 0, sizeof(op));
    arg.name = (char *)(size_t) "x";
    arg.type = (char *)(size_t) "int";
    sig.args = &arg;
    sig.n_args = 1;
    ctx.sig = &sig;
    ctx.func_name = "test_fn";
    g_op_fail_find_doc_param = 1;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              c2openapi_build_operation(&ctx, &op));
    g_op_fail_find_doc_param = 0;
    reset_operation_test(&op);
  }

  /* 12. is_path_param fail in c2openapi_build_operation */
  {
    struct OpBuilderContext ctx;
    struct C2OpenAPI_ParsedSig sig;
    struct C2OpenAPI_ParsedArg arg;
    struct DocMetadata doc;
    struct OpenAPI_Operation op;
    memset(&ctx, 0, sizeof(ctx));
    memset(&sig, 0, sizeof(sig));
    memset(&arg, 0, sizeof(arg));
    memset(&doc, 0, sizeof(doc));
    memset(&op, 0, sizeof(op));
    doc.route = (char *)(size_t) "/items/{x}";
    arg.name = (char *)(size_t) "x";
    arg.type = (char *)(size_t) "int";
    sig.args = &arg;
    sig.n_args = 1;
    ctx.sig = &sig;
    ctx.doc = &doc;
    ctx.func_name = "test_fn";
    g_op_fail_is_path_param = 1;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              c2openapi_build_operation(&ctx, &op));
    g_op_fail_is_path_param = 0;
    reset_operation_test(&op);
  }

  /* 13. is_struct_pointer fail in c2openapi_build_operation */
  {
    struct OpBuilderContext ctx;
    struct C2OpenAPI_ParsedSig sig;
    struct C2OpenAPI_ParsedArg arg;
    struct OpenAPI_Operation op;
    memset(&ctx, 0, sizeof(ctx));
    memset(&sig, 0, sizeof(sig));
    memset(&arg, 0, sizeof(arg));
    memset(&op, 0, sizeof(op));
    arg.name = (char *)(size_t) "x";
    arg.type = (char *)(size_t) "struct Foo *";
    sig.args = &arg;
    sig.n_args = 1;
    ctx.sig = &sig;
    ctx.func_name = "test_fn";
    g_op_fail_is_struct_pointer = 1;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              c2openapi_build_operation(&ctx, &op));
    g_op_fail_is_struct_pointer = 0;
    reset_operation_test(&op);
  }

  /* 14. is_reserved_header_name fail in c2openapi_build_operation */
  {
    struct OpBuilderContext ctx;
    struct C2OpenAPI_ParsedSig sig;
    struct C2OpenAPI_ParsedArg arg;
    struct DocMetadata doc;
    struct DocParam dp;
    struct OpenAPI_Operation op;
    memset(&ctx, 0, sizeof(ctx));
    memset(&sig, 0, sizeof(sig));
    memset(&arg, 0, sizeof(arg));
    memset(&doc, 0, sizeof(doc));
    memset(&dp, 0, sizeof(dp));
    memset(&op, 0, sizeof(op));
    dp.name = (char *)(size_t) "hdr";
    dp.in_loc = (char *)(size_t) "header";
    doc.params = &dp;
    doc.n_params = 1;
    arg.name = (char *)(size_t) "hdr";
    arg.type = (char *)(size_t) "char *";
    sig.args = &arg;
    sig.n_args = 1;
    ctx.sig = &sig;
    ctx.doc = &doc;
    ctx.func_name = "test_fn";
    g_cdd_fail_stricmp = 1;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              c2openapi_build_operation(&ctx, &op));
    g_cdd_fail_stricmp = 0;
    reset_operation_test(&op);
  }

  /* 15. doc_style_to_openapi fail in c2openapi_build_operation parameter */
  {
    struct OpBuilderContext ctx;
    struct C2OpenAPI_ParsedSig sig;
    struct C2OpenAPI_ParsedArg arg;
    struct DocMetadata doc;
    struct DocParam dp;
    struct OpenAPI_Operation op;
    memset(&ctx, 0, sizeof(ctx));
    memset(&sig, 0, sizeof(sig));
    memset(&arg, 0, sizeof(arg));
    memset(&doc, 0, sizeof(doc));
    memset(&dp, 0, sizeof(dp));
    memset(&op, 0, sizeof(op));
    dp.name = (char *)(size_t) "p";
    dp.style_set = 1;
    dp.style = DOC_PARAM_STYLE_FORM;
    doc.params = &dp;
    doc.n_params = 1;
    arg.name = (char *)(size_t) "p";
    arg.type = (char *)(size_t) "int";
    sig.args = &arg;
    sig.n_args = 1;
    ctx.sig = &sig;
    ctx.doc = &doc;
    ctx.func_name = "test_fn";
    g_op_fail_doc_style_to_openapi = 1;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              c2openapi_build_operation(&ctx, &op));
    g_op_fail_doc_style_to_openapi = 0;
    reset_operation_test(&op);
  }

  /* 16. find_media_type_op fail for rb->example */
  {
    struct OpBuilderContext ctx;
    struct C2OpenAPI_ParsedSig sig;
    struct DocMetadata doc;
    struct DocRequestBody rb;
    struct OpenAPI_Operation op;
    memset(&ctx, 0, sizeof(ctx));
    memset(&sig, 0, sizeof(sig));
    memset(&doc, 0, sizeof(doc));
    memset(&rb, 0, sizeof(rb));
    memset(&op, 0, sizeof(op));
    rb.content_type = (char *)(size_t) "application/json";
    rb.example = (char *)(size_t) "{}";
    doc.request_bodies = &rb;
    doc.n_request_bodies = 1;
    ctx.sig = &sig;
    ctx.doc = &doc;
    ctx.func_name = "test_fn";
    g_op_fail_find_media_type_op = 1;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              c2openapi_build_operation(&ctx, &op));
    g_op_fail_find_media_type_op = 0;
    reset_operation_test(&op);
  }

  /* 17. find_media_type_op fail for doc->encodings */
  {
    struct OpBuilderContext ctx;
    struct C2OpenAPI_ParsedSig sig;
    struct DocMetadata doc;
    struct DocRequestBody rb;
    struct DocEncoding enc;
    struct OpenAPI_Operation op;
    memset(&ctx, 0, sizeof(ctx));
    memset(&sig, 0, sizeof(sig));
    memset(&doc, 0, sizeof(doc));
    memset(&rb, 0, sizeof(rb));
    memset(&enc, 0, sizeof(enc));
    memset(&op, 0, sizeof(op));
    rb.content_type = (char *)(size_t) "multipart/form-data";
    doc.request_bodies = &rb;
    doc.n_request_bodies = 1;
    enc.name = (char *)(size_t) "field1";
    doc.encodings = &enc;
    doc.n_encodings = 1;
    ctx.sig = &sig;
    ctx.doc = &doc;
    ctx.func_name = "test_fn";
    g_op_fail_find_media_type_op = 1;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              c2openapi_build_operation(&ctx, &op));
    g_op_fail_find_media_type_op = 0;
    reset_operation_test(&op);
  }

  /* 18. doc_style_to_openapi fail for d_enc->style */
  {
    struct OpBuilderContext ctx;
    struct C2OpenAPI_ParsedSig sig;
    struct DocMetadata doc;
    struct DocRequestBody rb;
    struct DocEncoding enc;
    struct OpenAPI_Operation op;
    memset(&ctx, 0, sizeof(ctx));
    memset(&sig, 0, sizeof(sig));
    memset(&doc, 0, sizeof(doc));
    memset(&rb, 0, sizeof(rb));
    memset(&enc, 0, sizeof(enc));
    memset(&op, 0, sizeof(op));
    rb.content_type = (char *)(size_t) "multipart/form-data";
    doc.request_bodies = &rb;
    doc.n_request_bodies = 1;
    enc.name = (char *)(size_t) "field1";
    enc.style = DOC_PARAM_STYLE_FORM;
    doc.encodings = &enc;
    doc.n_encodings = 1;
    ctx.sig = &sig;
    ctx.doc = &doc;
    ctx.func_name = "test_fn";
    g_op_fail_doc_style_to_openapi = 1;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              c2openapi_build_operation(&ctx, &op));
    g_op_fail_doc_style_to_openapi = 0;
    reset_operation_test(&op);
  }

  /* 19. ensure_response_for_code null for doc->response_headers */
  {
    struct OpBuilderContext ctx;
    struct C2OpenAPI_ParsedSig sig;
    struct DocMetadata doc;
    struct DocResponseHeader rh;
    struct OpenAPI_Operation op;
    memset(&ctx, 0, sizeof(ctx));
    memset(&sig, 0, sizeof(sig));
    memset(&doc, 0, sizeof(doc));
    memset(&rh, 0, sizeof(rh));
    memset(&op, 0, sizeof(op));
    rh.code = (char *)(size_t) "200";
    rh.name = (char *)(size_t) "X-Header";
    doc.response_headers = &rh;
    doc.n_response_headers = 1;
    ctx.sig = &sig;
    ctx.doc = &doc;
    ctx.func_name = "test_fn";
    g_op_fail_ensure_response_null = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, c2openapi_build_operation(&ctx, &op));
    g_op_fail_ensure_response_null = 0;
    reset_operation_test(&op);
  }

  /* 19b. ensure_response_for_code rc error for doc->response_headers */
  {
    struct OpBuilderContext ctx;
    struct C2OpenAPI_ParsedSig sig;
    struct DocMetadata doc;
    struct DocResponseHeader rh;
    struct OpenAPI_Operation op;
    memset(&ctx, 0, sizeof(ctx));
    memset(&sig, 0, sizeof(sig));
    memset(&doc, 0, sizeof(doc));
    memset(&rh, 0, sizeof(rh));
    memset(&op, 0, sizeof(op));
    rh.code = (char *)(size_t) "200";
    rh.name = (char *)(size_t) "X-Header";
    doc.response_headers = &rh;
    doc.n_response_headers = 1;
    ctx.sig = &sig;
    ctx.doc = &doc;
    ctx.func_name = "test_fn";
    g_op_fail_ensure_response_for_code = 1;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              c2openapi_build_operation(&ctx, &op));
    g_op_fail_ensure_response_for_code = 0;
    reset_operation_test(&op);
  }

  /* 20. ensure_response_for_code null for doc->links */
  {
    struct OpBuilderContext ctx;
    struct C2OpenAPI_ParsedSig sig;
    struct DocMetadata doc;
    struct DocLink dl;
    struct OpenAPI_Operation op;
    memset(&ctx, 0, sizeof(ctx));
    memset(&sig, 0, sizeof(sig));
    memset(&doc, 0, sizeof(doc));
    memset(&dl, 0, sizeof(dl));
    memset(&op, 0, sizeof(op));
    dl.code = (char *)(size_t) "200";
    dl.name = (char *)(size_t) "MyLink";
    doc.links = &dl;
    doc.n_links = 1;
    ctx.sig = &sig;
    ctx.doc = &doc;
    ctx.func_name = "test_fn";
    g_op_fail_ensure_response_null = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, c2openapi_build_operation(&ctx, &op));
    g_op_fail_ensure_response_null = 0;
    reset_operation_test(&op);
  }

  /* 20b. ensure_response_for_code rc error for doc->links */
  {
    struct OpBuilderContext ctx;
    struct C2OpenAPI_ParsedSig sig;
    struct DocMetadata doc;
    struct DocLink dl;
    struct OpenAPI_Operation op;
    memset(&ctx, 0, sizeof(ctx));
    memset(&sig, 0, sizeof(sig));
    memset(&doc, 0, sizeof(doc));
    memset(&dl, 0, sizeof(dl));
    memset(&op, 0, sizeof(op));
    dl.code = (char *)(size_t) "200";
    dl.name = (char *)(size_t) "MyLink";
    doc.links = &dl;
    doc.n_links = 1;
    ctx.sig = &sig;
    ctx.doc = &doc;
    ctx.func_name = "test_fn";
    g_op_fail_ensure_response_for_code = 1;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              c2openapi_build_operation(&ctx, &op));
    g_op_fail_ensure_response_for_code = 0;
    reset_operation_test(&op);
  }

  PASS();
}

TEST test_operation_reset_with_link_details(void) {
  struct OpenAPI_Operation op;
  struct OpenAPI_Response *resp;
  struct OpenAPI_Link *lnk;
  struct OpenAPI_Server *srv;
  struct OpenAPI_LinkParam *lp;

  memset(&op, 0, sizeof(op));
  resp = (struct OpenAPI_Response *)calloc(1, sizeof(*resp));
  lnk = (struct OpenAPI_Link *)calloc(1, sizeof(*lnk));
  srv = (struct OpenAPI_Server *)calloc(1, sizeof(*srv));
  lp = (struct OpenAPI_LinkParam *)calloc(1, sizeof(*lp));

  lp->name = strdup("param1");
  lp->value.string = strdup("val1");
  lp->value.json = strdup("{\"a\":1}");

  srv->url = strdup("http://example.com");
  srv->name = strdup("srv");
  srv->description = strdup("desc");

  lnk->n_parameters = 1;
  lnk->parameters = lp;
  lnk->server = srv;

  resp->n_links = 1;
  resp->links = lnk;

  op.n_responses = 1;
  op.responses = resp;

  reset_operation_test(&op);
  PASS();
}

SUITE(operation_suite) {
  RUN_TEST(test_operation_reset_with_link_details);
  RUN_TEST(test_operation_reach_100_percent);
  RUN_TEST(test_operation_100_percent_coverage);
  RUN_TEST(test_operation_100_percent_coverage_part2);
  RUN_TEST(test_operation_all_null_and_boundary_checks);
  RUN_TEST(test_operation_final_gaps);
  RUN_TEST(test_operation_is_reserved_header_name);
  RUN_TEST(test_operation_parse_example_any);
  RUN_TEST(test_operation_parse_example_any_oom);
  RUN_TEST(test_operation_any_from_json_value);
  RUN_TEST(test_operation_any_from_json_value_oom_and_types);
  RUN_TEST(test_operation_parse_link_params_json);
  RUN_TEST(test_operation_parse_link_params_json_oom);
  RUN_TEST(test_operation_parse_link_params_json_cleanup_branches);
  RUN_TEST(test_operation_copy_and_free_any_value_local);
  RUN_TEST(test_operation_copy_any_value_local_nulls);
  RUN_TEST(test_operation_apply_example_to_response_branches);
  RUN_TEST(test_operation_free_openapi_server_variables_op);
  RUN_TEST(test_operation_copy_doc_server_variables_op);
  RUN_TEST(test_operation_copy_doc_server_variables_op_oom);
  RUN_TEST(test_operation_doc_server_variables_enum_calloc_oom);
  RUN_TEST(test_operation_find_doc_param);
  RUN_TEST(test_operation_find_response_by_code);
  RUN_TEST(test_operation_find_media_type_op);
  RUN_TEST(test_operation_ensure_response_for_code);
  RUN_TEST(test_operation_ensure_response_for_code_oom);
  RUN_TEST(test_operation_apply_example);
  RUN_TEST(test_operation_apply_example_to_media_type_oom);
  RUN_TEST(test_operation_add_header_to_response);
  RUN_TEST(test_operation_add_header_to_response_advanced);
  RUN_TEST(test_operation_add_header_to_response_existing_oom);
  RUN_TEST(test_operation_header_example_failure);
  RUN_TEST(test_operation_add_link_to_response);
  RUN_TEST(test_operation_add_link_to_response_advanced);
  RUN_TEST(test_operation_add_link_to_response_oom_sweep);
  RUN_TEST(test_operation_link_cleanup_specific);
  RUN_TEST(test_operation_add_param_to_op);
  RUN_TEST(test_operation_schema_ref_has_data_basic);
  RUN_TEST(test_operation_copy_schema_ref_basic);
  RUN_TEST(test_operation_response_has_media_type);
  RUN_TEST(test_operation_is_struct_pointer);
  RUN_TEST(test_operation_doc_style_to_openapi);
  RUN_TEST(test_operation_oa_type_is_primitive);
  RUN_TEST(test_operation_apply_format_to_schema_ref);
  RUN_TEST(test_operation_set_querystring_schema_from_type_map);
  RUN_TEST(test_operation_init_and_add_response_media_type);
  RUN_TEST(test_operation_request_body_media_types);
  RUN_TEST(test_operation_media_types_item_schema_oom);
  RUN_TEST(test_operation_c2openapi_build_operation_nulls);
  RUN_TEST(test_operation_c2openapi_build_operation_verbs);
  RUN_TEST(test_operation_c2openapi_build_operation_metadata);
  RUN_TEST(test_operation_c2openapi_build_operation_arguments);
  RUN_TEST(test_operation_c2openapi_build_operation_bodies_returns_links);
  RUN_TEST(test_operation_c2openapi_build_operation_coverage_extensions);
  RUN_TEST(test_operation_c2openapi_build_operation_mega_oom);
  RUN_TEST(test_operation_c2openapi_build_operation_oom);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_OPERATION_H */
