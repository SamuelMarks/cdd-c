/**
 * @file test_codegen_client_body.h
 * @brief Unit tests for client body generator basic verbs and requests.
 * @author Samuel Marks
 */

#ifndef TEST_CODEGEN_CLIENT_BODY_H
#define TEST_CODEGEN_CLIENT_BODY_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "emit/test_codegen_client_body_common.h"
/* clang-format on */

TEST test_body_basic_get(void) {
  struct OpenAPI_Response resp;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  char *code = NULL;
  char *_ast_gen_body_0 = NULL;

  memset(&op, 0, sizeof(op));

  memset(&resp, 0, sizeof(resp));

  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));

  ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
  op.verb = OA_VERB_GET;
  resp.code = (char *)(size_t)(size_t) "200";
  op.responses = &resp;
  op.n_responses = 1;

  g_client_body_fail_tmpfile = 1;
  ASSERT(gen_body(&op, &spec, "/", NULL, &code) ==
         CDD_C_ERROR_INVALID_ARGUMENT);
  g_client_body_fail_tmpfile = 0;

  code = (gen_body(&op, &spec, "/", NULL, &_ast_gen_body_0), _ast_gen_body_0);
  ASSERT(code);

  /* Check error init */
  ASSERT(strstr(code, "if (api_error) *api_error = NULL;"));

  /* Check default failure parsing */
  ASSERT(strstr(code, "if (res->body && api_error)"));
  ASSERT(strstr(code, "ApiError_from_json"));

  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_body_base_url_override(void) {
  struct OpenAPI_Response resp;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  char *code = NULL;
  char *_ast_gen_body_1 = NULL;

  memset(&op, 0, sizeof(op));

  memset(&resp, 0, sizeof(resp));

  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));

  ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
  op.verb = OA_VERB_GET;
  resp.code = (char *)(size_t)(size_t) "200";
  op.responses = &resp;
  op.n_responses = 1;

  code = (gen_body(&op, &spec, "/pets", "\"https://override.example.com\"",
                   &_ast_gen_body_1),
          _ast_gen_body_1);
  ASSERT(code);
  ASSERT(strstr(code, "\"https://override.example.com\"") != NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_body_options_verb(void) {
  struct OpenAPI_Response resp;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  char *code = NULL;
  char *_ast_gen_body_2 = NULL;

  memset(&op, 0, sizeof(op));

  memset(&resp, 0, sizeof(resp));

  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));

  ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
  op.verb = OA_VERB_OPTIONS;
  resp.code = (char *)(size_t)(size_t) "200";
  op.responses = &resp;
  op.n_responses = 1;

  code = (gen_body(&op, &spec, "/", NULL, &_ast_gen_body_2), _ast_gen_body_2);
  ASSERT(code);
  ASSERT(strstr(code, "req.method = HTTP_OPTIONS;") != NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_body_trace_verb(void) {
  struct OpenAPI_Response resp;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  char *code = NULL;
  char *_ast_gen_body_3 = NULL;

  memset(&op, 0, sizeof(op));

  memset(&resp, 0, sizeof(resp));

  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));

  ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
  op.verb = OA_VERB_TRACE;
  resp.code = (char *)(size_t)(size_t) "200";
  op.responses = &resp;
  op.n_responses = 1;

  code = (gen_body(&op, &spec, "/", NULL, &_ast_gen_body_3), _ast_gen_body_3);
  ASSERT(code);
  ASSERT(strstr(code, "req.method = HTTP_TRACE;") != NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_body_query_verb(void) {
  struct OpenAPI_Response resp;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  char *code = NULL;
  char *_ast_gen_body_4 = NULL;

  memset(&op, 0, sizeof(op));

  memset(&resp, 0, sizeof(resp));

  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));

  ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
  op.verb = OA_VERB_QUERY;
  resp.code = (char *)(size_t)(size_t) "200";
  op.responses = &resp;
  op.n_responses = 1;

  code = (gen_body(&op, &spec, "/", NULL, &_ast_gen_body_4), _ast_gen_body_4);
  ASSERT(code);
  ASSERT(strstr(code, "req.method = HTTP_QUERY;") != NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_body_additional_connect_method(void) {
  struct OpenAPI_Response resp;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  char *code = NULL;
  char *_ast_gen_body_5 = NULL;

  memset(&op, 0, sizeof(op));

  memset(&resp, 0, sizeof(resp));

  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));

  ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
  op.verb = OA_VERB_UNKNOWN;
  op.is_additional = 1;
  op.method = (char *)(size_t)(size_t) "CONNECT";
  resp.code = (char *)(size_t)(size_t) "200";
  op.responses = &resp;
  op.n_responses = 1;

  code = (gen_body(&op, &spec, "/", NULL, &_ast_gen_body_5), _ast_gen_body_5);
  ASSERT(code);
  ASSERT(strstr(code, "req.method = HTTP_CONNECT;") != NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_body_querystring_param(void) {
  struct OpenAPI_Response resp;
  struct OpenAPI_Parameter param;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  char *code = NULL;
  char *_ast_gen_body_6 = NULL;

  memset(&op, 0, sizeof(op));

  memset(&resp, 0, sizeof(resp));

  memset(&param, 0, sizeof(param));

  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));
  memset(&param, 0, sizeof(param));

  ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
  op.verb = OA_VERB_GET;
  resp.code = (char *)(size_t)(size_t) "200";
  op.responses = &resp;
  op.n_responses = 1;

  param.name = (char *)(size_t)(size_t) "qs";
  param.in = OA_PARAM_IN_QUERYSTRING;
  param.type = (char *)(size_t)(size_t) "string";
  op.parameters = &param;
  op.n_parameters = 1;

  code = (gen_body(&op, &spec, "/search", NULL, &_ast_gen_body_6),
          _ast_gen_body_6);
  ASSERT(code);
  ASSERT(strstr(code, "Querystring Parameter") != NULL);
  ASSERT(strstr(code, "asprintf(&query_str") != NULL);
  free(code);

  /* Test primitive arrays */
  param.is_array = 1;
  param.items_type = (char *)(size_t)(size_t) "string";
  code = (gen_body(&op, &spec, "/search", NULL, &_ast_gen_body_6),
          _ast_gen_body_6);
  ASSERT(code);
  free(code);

  param.items_type = (char *)(size_t)(size_t) "integer";
  code = (gen_body(&op, &spec, "/search", NULL, &_ast_gen_body_6),
          _ast_gen_body_6);
  ASSERT(code);
  free(code);

  param.items_type = (char *)(size_t)(size_t) "number";
  code = (gen_body(&op, &spec, "/search", NULL, &_ast_gen_body_6),
          _ast_gen_body_6);
  ASSERT(code);
  free(code);

  param.items_type = (char *)(size_t)(size_t) "boolean";
  code = (gen_body(&op, &spec, "/search", NULL, &_ast_gen_body_6),
          _ast_gen_body_6);
  ASSERT(code);
  free(code);

  param.items_type = (char *)(size_t)(size_t) "unsupported";
  code = (gen_body(&op, &spec, "/search", NULL, &_ast_gen_body_6),
          _ast_gen_body_6);
  ASSERT(code);
  free(code);

  g_fail_io_after = -1;
  PASS();
}

TEST test_body_inline_response_string(void) {
  struct OpenAPI_Response resp;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  char *code = NULL;
  char *_ast_gen_body_7 = NULL;

  memset(&op, 0, sizeof(op));

  memset(&resp, 0, sizeof(resp));

  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));

  ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
  op.verb = OA_VERB_GET;
  resp.code = (char *)(size_t)(size_t) "200";
  resp.schema.inline_type = (char *)(size_t)(size_t) "string";
  op.responses = &resp;
  op.n_responses = 1;

  code = (gen_body(&op, &spec, "/", NULL, &_ast_gen_body_7), _ast_gen_body_7);
  ASSERT(code);
  ASSERT(strstr(code, "json_value_get_string") != NULL);
  ASSERT(strstr(code, "strdup(") != NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_body_inline_response_array_number(void) {
  struct OpenAPI_Response resp;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  char *code = NULL;
  char *_ast_gen_body_8 = NULL;

  memset(&op, 0, sizeof(op));

  memset(&resp, 0, sizeof(resp));

  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));

  ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
  op.verb = OA_VERB_GET;
  resp.code = (char *)(size_t)(size_t) "200";
  resp.schema.is_array = 1;
  resp.schema.inline_type = (char *)(size_t)(size_t) "number";
  op.responses = &resp;
  op.n_responses = 1;

  code = (gen_body(&op, &spec, "/", NULL, &_ast_gen_body_8), _ast_gen_body_8);
  ASSERT(code);
  ASSERT(strstr(code, "json_array_get_count") != NULL);
  ASSERT(strstr(code, "json_array_get_number") != NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_body_inline_request_body_string(void) {
  struct OpenAPI_Response resp;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  char *code = NULL;
  char *_ast_gen_body_9 = NULL;

  memset(&op, 0, sizeof(op));

  memset(&resp, 0, sizeof(resp));

  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));

  ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
  op.verb = OA_VERB_POST;
  op.req_body.content_type = (char *)(size_t)(size_t) "application/json";
  op.req_body.inline_type = (char *)(size_t)(size_t) "string";
  resp.code = (char *)(size_t)(size_t) "200";
  op.responses = &resp;
  op.n_responses = 1;

  code = (gen_body(&op, &spec, "/", NULL, &_ast_gen_body_9), _ast_gen_body_9);
  ASSERT(code);
  ASSERT(strstr(code, "json_value_init_string") != NULL);
  ASSERT(strstr(code, "json_serialize_to_string") != NULL);
  ASSERT(strstr(code, "Content-Type\", \"application/json\"") != NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_body_inline_request_body_string_json_params(void) {
  struct OpenAPI_Response resp;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  char *code = NULL;
  char *_ast_gen_body_10 = NULL;

  memset(&op, 0, sizeof(op));

  memset(&resp, 0, sizeof(resp));

  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));

  ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
  op.verb = OA_VERB_POST;
  op.req_body.content_type =
      (char *)(size_t)(size_t) "Application/JSON; charset=utf-8";
  op.req_body.inline_type = (char *)(size_t)(size_t) "string";
  resp.code = (char *)(size_t)(size_t) "200";
  op.responses = &resp;
  op.n_responses = 1;

  code = (gen_body(&op, &spec, "/", NULL, &_ast_gen_body_10), _ast_gen_body_10);
  ASSERT(code);
  ASSERT(strstr(code, "json_value_init_string") != NULL);
  ASSERT(strstr(code, "Content-Type\", \"application/json\"") != NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_body_inline_request_body_array(void) {
  struct OpenAPI_Response resp;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  char *code = NULL;
  char *_ast_gen_body_11 = NULL;

  memset(&op, 0, sizeof(op));

  memset(&resp, 0, sizeof(resp));

  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));

  ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
  op.verb = OA_VERB_POST;
  op.req_body.content_type = (char *)(size_t)(size_t) "application/json";
  op.req_body.is_array = 1;
  op.req_body.inline_type = (char *)(size_t)(size_t) "integer";
  resp.code = (char *)(size_t)(size_t) "200";
  op.responses = &resp;
  op.n_responses = 1;

  code = (gen_body(&op, &spec, "/", NULL, &_ast_gen_body_11), _ast_gen_body_11);
  ASSERT(code);
  ASSERT(strstr(code, "json_value_init_array") != NULL);
  ASSERT(strstr(code, "json_array_append_number") != NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_body_textual_request_body_xml(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  char *code = NULL;
  char *_ast_gen_body_12 = NULL;

  memset(&op, 0, sizeof(op));

  memset(&op, 0, sizeof(op));

  ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
  op.verb = OA_VERB_POST;
  op.req_body.content_type = (char *)(size_t)(size_t) "application/xml";
  op.req_body.ref_name = (char *)(size_t)(size_t) "Pet";

  code = (gen_body(&op, &spec, "/pets", NULL, &_ast_gen_body_12),
          _ast_gen_body_12);
  ASSERT(code);
  ASSERT(strstr(code, "req.body = (void *)req_body") != NULL);
  ASSERT(strstr(code, "\"Content-Type\", \"application/xml\"") != NULL);
  ASSERT(strstr(code, "Pet_to_json") == NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_body_binary_request_body_pdf(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  char *code = NULL;
  char *_ast_gen_body_13 = NULL;

  memset(&op, 0, sizeof(op));

  memset(&op, 0, sizeof(op));

  ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
  op.verb = OA_VERB_POST;
  op.req_body.content_type = (char *)(size_t)(size_t) "application/pdf";
  op.req_body.ref_name = (char *)(size_t)(size_t) "Pet";

  code =
      (gen_body(&op, &spec, "/pdf", NULL, &_ast_gen_body_13), _ast_gen_body_13);
  ASSERT(code);
  ASSERT(strstr(code, "req.body = (void *)body") != NULL);
  ASSERT(strstr(code, "\"Content-Type\", \"application/pdf\"") != NULL);
  ASSERT(strstr(code, "Pet_to_json") == NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_gen_body_null_and_error(void) {
  char *code = NULL;
  struct OpenAPI_Operation op = {0};
  ASSERT(gen_body(NULL, NULL, NULL, NULL, &code) != CDD_C_SUCCESS);
  /* Force failure with invalid op */
  op.verb = (enum OpenAPI_Verb)999;
  ASSERT(gen_body(&op, NULL, NULL, NULL, &code) != CDD_C_SUCCESS);
  PASS();
}

SUITE(client_body_suite) {
  RUN_TEST(test_body_basic_get);
  RUN_TEST(test_body_base_url_override);
  RUN_TEST(test_body_options_verb);
  RUN_TEST(test_body_trace_verb);
  RUN_TEST(test_body_query_verb);
  RUN_TEST(test_body_additional_connect_method);
  RUN_TEST(test_body_querystring_param);
  RUN_TEST(test_body_inline_response_string);
  RUN_TEST(test_body_inline_response_array_number);
  RUN_TEST(test_body_inline_request_body_string);
  RUN_TEST(test_body_inline_request_body_string_json_params);
  RUN_TEST(test_body_inline_request_body_array);
  RUN_TEST(test_body_textual_request_body_xml);
  RUN_TEST(test_body_binary_request_body_pdf);
  RUN_TEST(test_gen_body_null_and_error);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CODEGEN_CLIENT_BODY_H */
