extern C_CDD_EXPORT int g_fail_io_after;
extern C_CDD_EXPORT int g_io_calls;
/**
 * @file test_codegen_client_body.h
 * @brief Unit tests for Client Body Logic Generator.
 */

#ifndef TEST_CODEGEN_CLIENT_BODY_H
#define TEST_CODEGEN_CLIENT_BODY_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "classes/emit/struct.h"
#include "functions/emit/client_body.h"
#include "openapi/parse/openapi.h"
/* clang-format on */

static int gen_body(const struct OpenAPI_Operation *op,
                    const struct OpenAPI_Spec *spec, const char *tmpl,
                    const char *base_url_expr, char **_out_val) {
  FILE *tmp = tmpfile();
  long sz;
  char *content = NULL;

  if (!tmp) {
    *_out_val = NULL;
    return 0;
  }

  if (codegen_client_write_body(tmp, op, spec, tmpl, base_url_expr) != 0) {
    fclose(tmp);
    {
      *_out_val = NULL;
      return 0;
    }
  }

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);

  content = (char *)calloc(1, sz + 1);
  if (sz > 0)
    fread(content, 1, sz, tmp);

  fclose(tmp);
  {
    *_out_val = content;
    return 0;
  }
}

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
  openapi_spec_init(&spec);

  op.verb = OA_VERB_GET;
  resp.code = "200";
  op.responses = &resp;
  op.n_responses = 1;

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
  openapi_spec_init(&spec);

  op.verb = OA_VERB_GET;
  resp.code = "200";
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
  openapi_spec_init(&spec);

  op.verb = OA_VERB_OPTIONS;
  resp.code = "200";
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
  openapi_spec_init(&spec);

  op.verb = OA_VERB_TRACE;
  resp.code = "200";
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
  openapi_spec_init(&spec);

  op.verb = OA_VERB_QUERY;
  resp.code = "200";
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
  openapi_spec_init(&spec);

  op.verb = OA_VERB_UNKNOWN;
  op.is_additional = 1;
  op.method = "CONNECT";
  resp.code = "200";
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
  openapi_spec_init(&spec);

  op.verb = OA_VERB_GET;
  resp.code = "200";
  op.responses = &resp;
  op.n_responses = 1;

  param.name = "qs";
  param.in = OA_PARAM_IN_QUERYSTRING;
  param.type = "string";
  op.parameters = &param;
  op.n_parameters = 1;

  code = (gen_body(&op, &spec, "/search", NULL, &_ast_gen_body_6),
          _ast_gen_body_6);
  ASSERT(code);
  ASSERT(strstr(code, "Querystring Parameter") != NULL);
  ASSERT(strstr(code, "asprintf(&query_str") != NULL);

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
  openapi_spec_init(&spec);

  op.verb = OA_VERB_GET;
  resp.code = "200";
  resp.schema.inline_type = "string";
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
  openapi_spec_init(&spec);

  op.verb = OA_VERB_GET;
  resp.code = "200";
  resp.schema.is_array = 1;
  resp.schema.inline_type = "number";
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
  openapi_spec_init(&spec);

  op.verb = OA_VERB_POST;
  op.req_body.content_type = "application/json";
  op.req_body.inline_type = "string";
  resp.code = "200";
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
  openapi_spec_init(&spec);

  op.verb = OA_VERB_POST;
  op.req_body.content_type = "Application/JSON; charset=utf-8";
  op.req_body.inline_type = "string";
  resp.code = "200";
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
  openapi_spec_init(&spec);

  op.verb = OA_VERB_POST;
  op.req_body.content_type = "application/json";
  op.req_body.is_array = 1;
  op.req_body.inline_type = "integer";
  resp.code = "200";
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
  openapi_spec_init(&spec);

  op.verb = OA_VERB_POST;
  op.req_body.content_type = "application/xml";
  op.req_body.ref_name = "Pet";

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
  openapi_spec_init(&spec);

  op.verb = OA_VERB_POST;
  op.req_body.content_type = "application/pdf";
  op.req_body.ref_name = "Pet";

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

TEST test_body_header_array_param(void) {
  struct OpenAPI_Response resp;
  struct OpenAPI_Parameter param;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  char *code = NULL;
  char *_ast_gen_body_14 = NULL;

  memset(&op, 0, sizeof(op));

  memset(&resp, 0, sizeof(resp));

  memset(&param, 0, sizeof(param));

  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));
  memset(&param, 0, sizeof(param));
  openapi_spec_init(&spec);

  op.verb = OA_VERB_GET;
  resp.code = "200";
  op.responses = &resp;
  op.n_responses = 1;

  param.name = "X-Ids";
  param.in = OA_PARAM_IN_HEADER;
  param.type = "array";
  param.is_array = 1;
  param.items_type = "integer";
  op.parameters = &param;
  op.n_parameters = 1;

  code = (gen_body(&op, &spec, "/", NULL, &_ast_gen_body_14), _ast_gen_body_14);
  ASSERT(code);
  ASSERT(strstr(code, "Header Parameter: X-Ids") != NULL);
  ASSERT(strstr(code, "http_headers_add(&req.headers, \"X-Ids\", joined)") !=
         NULL);
  ASSERT(strstr(code, "joined_len") != NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_body_header_object_param(void) {
  struct OpenAPI_Response resp;
  struct OpenAPI_Parameter param;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  char *code = NULL;
  char *_ast_gen_body_15 = NULL;

  memset(&op, 0, sizeof(op));

  memset(&resp, 0, sizeof(resp));

  memset(&param, 0, sizeof(param));

  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));
  memset(&param, 0, sizeof(param));
  openapi_spec_init(&spec);

  op.verb = OA_VERB_GET;
  resp.code = "200";
  op.responses = &resp;
  op.n_responses = 1;

  param.name = "X-Filter";
  param.in = OA_PARAM_IN_HEADER;
  param.type = "object";
  param.style = OA_STYLE_SIMPLE;
  param.explode = 1;
  param.explode_set = 1;
  op.parameters = &param;
  op.n_parameters = 1;

  code = (gen_body(&op, &spec, "/", NULL, &_ast_gen_body_15), _ast_gen_body_15);
  ASSERT(code);
  ASSERT(strstr(code, "Header Parameter: X-Filter") != NULL);
  ASSERT(strstr(code, "const struct OpenAPI_KV *kv = &X-Filter[i]") != NULL);
  ASSERT(strstr(code, "joined[joined_len++] = '='") != NULL);
  ASSERT(strstr(code, "http_headers_add(&req.headers, \"X-Filter\", joined)") !=
         NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_body_header_json_param_ref(void) {
  struct OpenAPI_Response resp;
  struct OpenAPI_Parameter param;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  char *code = NULL;
  char *_ast_gen_body_16 = NULL;

  memset(&op, 0, sizeof(op));

  memset(&resp, 0, sizeof(resp));

  memset(&param, 0, sizeof(param));

  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));
  memset(&param, 0, sizeof(param));
  openapi_spec_init(&spec);

  op.verb = OA_VERB_GET;
  resp.code = "200";
  op.responses = &resp;
  op.n_responses = 1;

  param.name = "X-Filter";
  param.in = OA_PARAM_IN_HEADER;
  param.content_type = "application/json";
  param.schema.ref_name = "Filter";
  param.type = "Filter";
  op.parameters = &param;
  op.n_parameters = 1;

  code = (gen_body(&op, &spec, "/", NULL, &_ast_gen_body_16), _ast_gen_body_16);
  ASSERT(code);
  ASSERT(strstr(code, "Header Parameter: X-Filter") != NULL);
  ASSERT(strstr(code, "Filter_to_json") != NULL);
  ASSERT(
      strstr(code, "http_headers_add(&req.headers, \"X-Filter\", hdr_json)") !=
      NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_body_header_number_param(void) {
  struct OpenAPI_Response resp;
  struct OpenAPI_Parameter param;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  char *code = NULL;
  char *_ast_gen_body_17 = NULL;

  memset(&op, 0, sizeof(op));

  memset(&resp, 0, sizeof(resp));

  memset(&param, 0, sizeof(param));

  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));
  memset(&param, 0, sizeof(param));
  openapi_spec_init(&spec);

  op.verb = OA_VERB_GET;
  resp.code = "200";
  op.responses = &resp;
  op.n_responses = 1;

  param.name = "X-Rate";
  param.in = OA_PARAM_IN_HEADER;
  param.type = "number";
  op.parameters = &param;
  op.n_parameters = 1;

  code = (gen_body(&op, &spec, "/", NULL, &_ast_gen_body_17), _ast_gen_body_17);
  ASSERT(code);
  ASSERT(strstr(code, "Header Parameter: X-Rate") != NULL);
  ASSERT(strstr(code, "sprintf(num_buf, \"%g\", X-Rate)") != NULL);
  ASSERT(strstr(code, "http_headers_add(&req.headers, \"X-Rate\", num_buf)") !=
         NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_body_cookie_param(void) {
  struct OpenAPI_Response resp;
  struct OpenAPI_Parameter param;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  char *code = NULL;
  char *_ast_gen_body_18 = NULL;

  memset(&op, 0, sizeof(op));

  memset(&resp, 0, sizeof(resp));

  memset(&param, 0, sizeof(param));

  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));
  memset(&param, 0, sizeof(param));
  openapi_spec_init(&spec);

  op.verb = OA_VERB_GET;
  resp.code = "200";
  op.responses = &resp;
  op.n_responses = 1;

  param.name = "session";
  param.in = OA_PARAM_IN_COOKIE;
  param.type = "string";
  op.parameters = &param;
  op.n_parameters = 1;

  code = (gen_body(&op, &spec, "/", NULL, &_ast_gen_body_18), _ast_gen_body_18);
  ASSERT(code);
  ASSERT(strstr(code, "Cookie Parameters") != NULL);
  ASSERT(
      strstr(code, "http_headers_add(&req.headers, \"Cookie\", cookie_str)") !=
      NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_body_cookie_param_number_array(void) {
  struct OpenAPI_Response resp;
  struct OpenAPI_Parameter param;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  char *code = NULL;
  char *_ast_gen_body_19 = NULL;

  memset(&op, 0, sizeof(op));

  memset(&resp, 0, sizeof(resp));

  memset(&param, 0, sizeof(param));

  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));
  memset(&param, 0, sizeof(param));
  openapi_spec_init(&spec);

  op.verb = OA_VERB_GET;
  resp.code = "200";
  op.responses = &resp;
  op.n_responses = 1;

  param.name = "weights";
  param.in = OA_PARAM_IN_COOKIE;
  param.type = "array";
  param.is_array = 1;
  param.items_type = "number";
  param.explode = 1;
  param.explode_set = 1;
  op.parameters = &param;
  op.n_parameters = 1;

  code = (gen_body(&op, &spec, "/", NULL, &_ast_gen_body_19), _ast_gen_body_19);
  ASSERT(code);
  ASSERT(strstr(code, "Cookie Parameters") != NULL);
  ASSERT(strstr(code, "sprintf(num_buf, \"%g\", weights[i])") != NULL);
  ASSERT(
      strstr(code, "http_headers_add(&req.headers, \"Cookie\", cookie_str)") !=
      NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_body_cookie_param_array_explode_false(void) {
  struct OpenAPI_Response resp;
  struct OpenAPI_Parameter param;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  char *code = NULL;
  char *_ast_gen_body_20 = NULL;

  memset(&op, 0, sizeof(op));

  memset(&resp, 0, sizeof(resp));

  memset(&param, 0, sizeof(param));

  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));
  memset(&param, 0, sizeof(param));
  openapi_spec_init(&spec);

  op.verb = OA_VERB_GET;
  resp.code = "200";
  op.responses = &resp;
  op.n_responses = 1;

  param.name = "session";
  param.in = OA_PARAM_IN_COOKIE;
  param.type = "array";
  param.is_array = 1;
  param.items_type = "string";
  param.explode_set = 1;
  param.explode = 0;
  op.parameters = &param;
  op.n_parameters = 1;

  code = (gen_body(&op, &spec, "/", NULL, &_ast_gen_body_20), _ast_gen_body_20);
  ASSERT(code);
  ASSERT(strstr(code, "joined_len") != NULL);
  ASSERT(strstr(code, "joined[joined_len++] = ','") != NULL);
  ASSERT(
      strstr(code, "http_headers_add(&req.headers, \"Cookie\", cookie_str)") !=
      NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_body_cookie_param_object_form(void) {
  struct OpenAPI_Response resp;
  struct OpenAPI_Parameter param;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  char *code = NULL;
  char *_ast_gen_body_21 = NULL;

  memset(&op, 0, sizeof(op));

  memset(&resp, 0, sizeof(resp));

  memset(&param, 0, sizeof(param));

  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));
  memset(&param, 0, sizeof(param));
  openapi_spec_init(&spec);

  op.verb = OA_VERB_GET;
  resp.code = "200";
  op.responses = &resp;
  op.n_responses = 1;

  param.name = "prefs";
  param.in = OA_PARAM_IN_COOKIE;
  param.type = "object";
  param.style = OA_STYLE_FORM;
  op.parameters = &param;
  op.n_parameters = 1;

  code = (gen_body(&op, &spec, "/", NULL, &_ast_gen_body_21), _ast_gen_body_21);
  ASSERT(code);
  ASSERT(strstr(code, "const struct OpenAPI_KV *kv = &prefs[i]") != NULL);
  ASSERT(strstr(code, "url_encode(") != NULL);
  ASSERT(
      strstr(code, "http_headers_add(&req.headers, \"Cookie\", cookie_str)") !=
      NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_body_cookie_param_string_allow_reserved(void) {
  struct OpenAPI_Response resp;
  struct OpenAPI_Parameter param;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  char *code = NULL;
  char *_ast_gen_body_22 = NULL;

  memset(&op, 0, sizeof(op));

  memset(&resp, 0, sizeof(resp));

  memset(&param, 0, sizeof(param));

  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));
  memset(&param, 0, sizeof(param));
  openapi_spec_init(&spec);

  op.verb = OA_VERB_GET;
  resp.code = "200";
  op.responses = &resp;
  op.n_responses = 1;

  param.name = "session";
  param.in = OA_PARAM_IN_COOKIE;
  param.type = "string";
  param.style = OA_STYLE_FORM;
  param.allow_reserved = 1;
  param.allow_reserved_set = 1;
  op.parameters = &param;
  op.n_parameters = 1;

  code = (gen_body(&op, &spec, "/", NULL, &_ast_gen_body_22), _ast_gen_body_22);
  ASSERT(code);
  ASSERT(strstr(code, "url_encode_allow_reserved") != NULL);
  ASSERT(
      strstr(code, "http_headers_add(&req.headers, \"Cookie\", cookie_str)") !=
      NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_body_security_query_api_key(void) {
  struct OpenAPI_SecurityRequirement req;
  struct OpenAPI_SecurityRequirementSet sec_set;
  struct OpenAPI_Response resp;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  struct OpenAPI_SecurityScheme scheme;
  char *code = NULL;
  char *_ast_gen_body_23 = NULL;

  memset(&op, 0, sizeof(op));

  memset(&resp, 0, sizeof(resp));

  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));
  memset(&spec, 0, sizeof(spec));
  memset(&scheme, 0, sizeof(scheme));
  memset(&req, 0, sizeof(req));
  memset(&sec_set, 0, sizeof(sec_set));

  op.verb = OA_VERB_GET;
  resp.code = "200";
  op.responses = &resp;
  op.n_responses = 1;

  scheme.name = "QueryKey";
  scheme.type = OA_SEC_APIKEY;
  scheme.in = OA_SEC_IN_QUERY;
  scheme.key_name = "api_key";
  spec.security_schemes = &scheme;
  spec.n_security_schemes = 1;

  /* Add global security requirement to activate the scheme */
  req.scheme = "QueryKey";

  sec_set.requirements = &req;
  sec_set.n_requirements = 1;

  spec.security = &sec_set;
  spec.n_security = 1;
  spec.security_set = 1;

  code = (gen_body(&op, &spec, "/", NULL, &_ast_gen_body_23), _ast_gen_body_23);
  ASSERT(code);
  printf("\n--- SEC QUERY KEY ---\n%s\n--------------------\n", code);
  ASSERT(strstr(code, "struct UrlQueryParams qp") != NULL);
  ASSERT(strstr(code, "url_query_add(&qp, \"api_key\"") != NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_body_security_cookie_api_key(void) {
  struct OpenAPI_SecurityRequirement req;
  struct OpenAPI_SecurityRequirementSet sec_set;
  struct OpenAPI_Response resp;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  struct OpenAPI_SecurityScheme scheme;
  char *code = NULL;
  char *_ast_gen_body_24 = NULL;

  memset(&op, 0, sizeof(op));

  memset(&resp, 0, sizeof(resp));

  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));
  memset(&spec, 0, sizeof(spec));
  memset(&scheme, 0, sizeof(scheme));

  op.verb = OA_VERB_GET;
  resp.code = "200";
  op.responses = &resp;
  op.n_responses = 1;

  scheme.name = "CookieKey";
  scheme.type = OA_SEC_APIKEY;
  scheme.in = OA_SEC_IN_COOKIE;
  scheme.key_name = "session_id";
  spec.security_schemes = &scheme;
  spec.n_security_schemes = 1;

  memset(&req, 0, sizeof(req));
  req.scheme = "CookieKey";

  memset(&sec_set, 0, sizeof(sec_set));
  sec_set.requirements = &req;
  sec_set.n_requirements = 1;

  spec.security = &sec_set;
  spec.n_security = 1;
  spec.security_set = 1;

  code = (gen_body(&op, &spec, "/", NULL, &_ast_gen_body_24), _ast_gen_body_24);
  ASSERT(code);
  printf("\n--- SEC COOKIE KEY ---\n%s\n--------------------\n", code);
  ASSERT(strstr(code, "cookie_str") != NULL);
  ASSERT(strstr(code, "session_id") != NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_body_form_urlencoded(void) {
  struct OpenAPI_Response resp;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  char *code = NULL;
  char *_ast_gen_body_25 = NULL;
  char *schema_name = "FormData";

  memset(&op, 0, sizeof(op));

  memset(&resp, 0, sizeof(resp));

  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));
  openapi_spec_init(&spec);

  spec.defined_schemas =
      (struct StructFields *)calloc(1, sizeof(struct StructFields));
  spec.defined_schema_names = (char **)calloc(1, sizeof(char *));
  ASSERT(spec.defined_schemas);
  ASSERT(spec.defined_schema_names);
  struct_fields_init(&spec.defined_schemas[0]);
  struct_fields_add(&spec.defined_schemas[0], "name", "string", NULL, NULL,
                    NULL);
  struct_fields_add(&spec.defined_schemas[0], "age", "integer", NULL, NULL,
                    NULL);
  spec.defined_schema_names[0] = schema_name;
  spec.n_defined_schemas = 1;

  op.verb = OA_VERB_POST;
  resp.code = "200";
  op.responses = &resp;
  op.n_responses = 1;

  op.req_body.ref_name = "FormData";
  op.req_body.content_type = "application/x-www-form-urlencoded";

  code = (gen_body(&op, &spec, "/submit", NULL, &_ast_gen_body_25),
          _ast_gen_body_25);
  ASSERT(code);
  ASSERT(strstr(code, "Form URL-Encoded Body Construction") != NULL);
  ASSERT(strstr(code, "url_query_build_form(&form_qp, &form_body)") != NULL);
  ASSERT(strstr(code, "\"application/x-www-form-urlencoded\"") != NULL);
  ASSERT(strstr(code, "url_query_add(&form_qp, \"name\"") != NULL);
  ASSERT(strstr(code, "sprintf(num_buf, \"%d\", req_body->age)") != NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_body_form_urlencoded_with_params(void) {
  struct OpenAPI_Response resp;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  char *code = NULL;
  char *_ast_gen_body_26 = NULL;
  char *schema_name = "FormData";

  memset(&op, 0, sizeof(op));

  memset(&resp, 0, sizeof(resp));

  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));
  openapi_spec_init(&spec);

  spec.defined_schemas =
      (struct StructFields *)calloc(1, sizeof(struct StructFields));
  spec.defined_schema_names = (char **)calloc(1, sizeof(char *));
  ASSERT(spec.defined_schemas);
  ASSERT(spec.defined_schema_names);
  struct_fields_init(&spec.defined_schemas[0]);
  struct_fields_add(&spec.defined_schemas[0], "name", "string", NULL, NULL,
                    NULL);
  spec.defined_schema_names[0] = schema_name;
  spec.n_defined_schemas = 1;

  op.verb = OA_VERB_POST;
  resp.code = "200";
  op.responses = &resp;
  op.n_responses = 1;

  op.req_body.ref_name = "FormData";
  op.req_body.content_type = "application/x-www-form-urlencoded; charset=utf-8";

  code = (gen_body(&op, &spec, "/submit", NULL, &_ast_gen_body_26),
          _ast_gen_body_26);
  ASSERT(code);
  ASSERT(strstr(code, "Form URL-Encoded Body Construction") != NULL);
  ASSERT(strstr(code, "url_query_build_form(&form_qp, &form_body)") != NULL);
  ASSERT(strstr(code, "\"application/x-www-form-urlencoded\"") != NULL);
  ASSERT(strstr(code, "url_query_add(&form_qp, \"name\"") != NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_body_form_urlencoded_object_fields(void) {
  struct OpenAPI_Response resp;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  char *code = NULL;
  char *_ast_gen_body_27 = NULL;
  char *schema_name = "FormData";

  memset(&op, 0, sizeof(op));

  memset(&resp, 0, sizeof(resp));

  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));
  openapi_spec_init(&spec);

  spec.defined_schemas =
      (struct StructFields *)calloc(1, sizeof(struct StructFields));
  spec.defined_schema_names = (char **)calloc(1, sizeof(char *));
  ASSERT(spec.defined_schemas);
  ASSERT(spec.defined_schema_names);
  struct_fields_init(&spec.defined_schemas[0]);
  struct_fields_add(&spec.defined_schemas[0], "pet", "object", "Pet", NULL,
                    NULL);
  struct_fields_add(&spec.defined_schemas[0], "pets", "array", "Pet", NULL,
                    NULL);
  spec.defined_schema_names[0] = schema_name;
  spec.n_defined_schemas = 1;

  op.verb = OA_VERB_POST;
  resp.code = "200";
  op.responses = &resp;
  op.n_responses = 1;

  op.req_body.ref_name = "FormData";
  op.req_body.content_type = "application/x-www-form-urlencoded";

  code = (gen_body(&op, &spec, "/submit", NULL, &_ast_gen_body_27),
          _ast_gen_body_27);
  ASSERT(code);
  ASSERT(strstr(code, "Pet_to_json(req_body->pet") != NULL);
  ASSERT(strstr(code, "Pet_to_json(req_body->pets[i]") != NULL);
  ASSERT(strstr(code, "url_query_add_encoded(&form_qp, \"pet\"") != NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_body_form_urlencoded_object_style_form_explode_true(void) {
  struct OpenAPI_Encoding enc;
  struct OpenAPI_Response resp;
  struct OpenAPI_MediaType mt;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  char *code = NULL;
  char *_ast_gen_body_28 = NULL;

  memset(&op, 0, sizeof(op));

  memset(&resp, 0, sizeof(resp));

  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));
  memset(&mt, 0, sizeof(mt));
  memset(&enc, 0, sizeof(enc));
  openapi_spec_init(&spec);

  spec.defined_schemas =
      (struct StructFields *)calloc(2, sizeof(struct StructFields));
  spec.defined_schema_names = (char **)calloc(2, sizeof(char *));
  ASSERT(spec.defined_schemas);
  ASSERT(spec.defined_schema_names);

  struct_fields_init(&spec.defined_schemas[0]);
  struct_fields_add(&spec.defined_schemas[0], "filter", "object", "Filter",
                    NULL, NULL);
  spec.defined_schema_names[0] = "FormData";

  struct_fields_init(&spec.defined_schemas[1]);
  struct_fields_add(&spec.defined_schemas[1], "color", "string", NULL, NULL,
                    NULL);
  struct_fields_add(&spec.defined_schemas[1], "limit", "integer", NULL, NULL,
                    NULL);
  spec.defined_schema_names[1] = "Filter";

  spec.n_defined_schemas = 2;

  op.verb = OA_VERB_POST;
  resp.code = "200";
  op.responses = &resp;
  op.n_responses = 1;

  op.req_body.ref_name = "FormData";
  op.req_body.content_type = "application/x-www-form-urlencoded";

  enc.name = "filter";
  enc.style = OA_STYLE_FORM;
  enc.style_set = 1;
  enc.explode = 1;
  enc.explode_set = 1;

  mt.name = "application/x-www-form-urlencoded";
  mt.encoding = &enc;
  mt.n_encoding = 1;
  op.req_body_media_types = &mt;
  op.n_req_body_media_types = 1;

  code = (gen_body(&op, &spec, "/submit", NULL, &_ast_gen_body_28),
          _ast_gen_body_28);
  ASSERT(code);
  ASSERT(strstr(code, "url_query_add(&form_qp, \"color\"") != NULL);
  ASSERT(strstr(code, "Filter_to_json") == NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_body_form_urlencoded_object_style_form_explode_false(void) {
  struct OpenAPI_Encoding enc;
  struct OpenAPI_Response resp;
  struct OpenAPI_MediaType mt;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  char *code = NULL;
  char *_ast_gen_body_29 = NULL;

  memset(&op, 0, sizeof(op));

  memset(&resp, 0, sizeof(resp));

  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));
  memset(&mt, 0, sizeof(mt));
  memset(&enc, 0, sizeof(enc));
  openapi_spec_init(&spec);

  spec.defined_schemas =
      (struct StructFields *)calloc(2, sizeof(struct StructFields));
  spec.defined_schema_names = (char **)calloc(2, sizeof(char *));
  ASSERT(spec.defined_schemas);
  ASSERT(spec.defined_schema_names);

  struct_fields_init(&spec.defined_schemas[0]);
  struct_fields_add(&spec.defined_schemas[0], "filter", "object", "Filter",
                    NULL, NULL);
  spec.defined_schema_names[0] = "FormData";

  struct_fields_init(&spec.defined_schemas[1]);
  struct_fields_add(&spec.defined_schemas[1], "color", "string", NULL, NULL,
                    NULL);
  struct_fields_add(&spec.defined_schemas[1], "limit", "integer", NULL, NULL,
                    NULL);
  spec.defined_schema_names[1] = "Filter";

  spec.n_defined_schemas = 2;

  op.verb = OA_VERB_POST;
  resp.code = "200";
  op.responses = &resp;
  op.n_responses = 1;

  op.req_body.ref_name = "FormData";
  op.req_body.content_type = "application/x-www-form-urlencoded";

  enc.name = "filter";
  enc.style = OA_STYLE_FORM;
  enc.style_set = 1;
  enc.explode = 0;
  enc.explode_set = 1;

  mt.name = "application/x-www-form-urlencoded";
  mt.encoding = &enc;
  mt.n_encoding = 1;
  op.req_body_media_types = &mt;
  op.n_req_body_media_types = 1;

  code = (gen_body(&op, &spec, "/submit", NULL, &_ast_gen_body_29),
          _ast_gen_body_29);
  ASSERT(code);
  ASSERT(strstr(code, "openapi_kv_join_form") != NULL);
  ASSERT(strstr(code, "url_query_add_encoded(&form_qp, \"filter\"") != NULL);
  ASSERT(strstr(code, "Filter_to_json") == NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_body_form_urlencoded_object_style_deep_object(void) {
  struct OpenAPI_Encoding enc;
  struct OpenAPI_Response resp;
  struct OpenAPI_MediaType mt;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  char *code = NULL;
  char *_ast_gen_body_30 = NULL;

  memset(&op, 0, sizeof(op));

  memset(&resp, 0, sizeof(resp));

  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));
  memset(&mt, 0, sizeof(mt));
  memset(&enc, 0, sizeof(enc));
  openapi_spec_init(&spec);

  spec.defined_schemas =
      (struct StructFields *)calloc(2, sizeof(struct StructFields));
  spec.defined_schema_names = (char **)calloc(2, sizeof(char *));
  ASSERT(spec.defined_schemas);
  ASSERT(spec.defined_schema_names);

  struct_fields_init(&spec.defined_schemas[0]);
  struct_fields_add(&spec.defined_schemas[0], "filter", "object", "Filter",
                    NULL, NULL);
  spec.defined_schema_names[0] = "FormData";

  struct_fields_init(&spec.defined_schemas[1]);
  struct_fields_add(&spec.defined_schemas[1], "color", "string", NULL, NULL,
                    NULL);
  struct_fields_add(&spec.defined_schemas[1], "limit", "integer", NULL, NULL,
                    NULL);
  spec.defined_schema_names[1] = "Filter";

  spec.n_defined_schemas = 2;

  op.verb = OA_VERB_POST;
  resp.code = "200";
  op.responses = &resp;
  op.n_responses = 1;

  op.req_body.ref_name = "FormData";
  op.req_body.content_type = "application/x-www-form-urlencoded";

  enc.name = "filter";
  enc.style = OA_STYLE_DEEP_OBJECT;
  enc.style_set = 1;
  enc.explode = 1;
  enc.explode_set = 1;

  mt.name = "application/x-www-form-urlencoded";
  mt.encoding = &enc;
  mt.n_encoding = 1;
  op.req_body_media_types = &mt;
  op.n_req_body_media_types = 1;

  code = (gen_body(&op, &spec, "/submit", NULL, &_ast_gen_body_30),
          _ast_gen_body_30);
  ASSERT(code);
  ASSERT(strstr(code, "filter[color]") != NULL);
  ASSERT(strstr(code, "Filter_to_json") == NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_body_multipart_primitives_and_arrays(void) {
  struct OpenAPI_Response resp;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  char *code = NULL;
  char *_ast_gen_body_31 = NULL;
  char *schema_name = "Upload";

  memset(&op, 0, sizeof(op));

  memset(&resp, 0, sizeof(resp));

  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));
  openapi_spec_init(&spec);

  spec.defined_schemas =
      (struct StructFields *)calloc(1, sizeof(struct StructFields));
  spec.defined_schema_names = (char **)calloc(1, sizeof(char *));
  ASSERT(spec.defined_schemas);
  ASSERT(spec.defined_schema_names);
  struct_fields_init(&spec.defined_schemas[0]);
  struct_fields_add(&spec.defined_schemas[0], "title", "string", NULL, NULL,
                    NULL);
  struct_fields_add(&spec.defined_schemas[0], "count", "integer", NULL, NULL,
                    NULL);
  struct_fields_add(&spec.defined_schemas[0], "ratio", "number", NULL, NULL,
                    NULL);
  struct_fields_add(&spec.defined_schemas[0], "flag", "boolean", NULL, NULL,
                    NULL);
  struct_fields_add(&spec.defined_schemas[0], "tags", "array", "string", NULL,
                    NULL);
  struct_fields_add(&spec.defined_schemas[0], "nums", "array", "integer", NULL,
                    NULL);
  spec.defined_schema_names[0] = schema_name;
  spec.n_defined_schemas = 1;

  op.verb = OA_VERB_POST;
  resp.code = "200";
  op.responses = &resp;
  op.n_responses = 1;

  op.req_body.ref_name = "Upload";
  op.req_body.content_type = "multipart/form-data";

  code = (gen_body(&op, &spec, "/upload", NULL, &_ast_gen_body_31),
          _ast_gen_body_31);
  ASSERT(code);
  ASSERT(strstr(code, "Multipart Body Construction") != NULL);
  ASSERT(strstr(code, "http_request_add_part(&req, \"title\"") != NULL);
  ASSERT(strstr(code, "sprintf(num_buf, \"%g\", req_body->ratio)") != NULL);
  ASSERT(strstr(code, "req_body->flag ? \"true\" : \"false\"") != NULL);
  ASSERT(strstr(code, "for (i = 0; i < req_body->n_tags; ++i)") != NULL);
  ASSERT(strstr(code, "http_request_add_part(&req, \"tags\"") != NULL);
  ASSERT(strstr(code, "for (i = 0; i < req_body->n_nums; ++i)") != NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_body_multipart_object_fields(void) {
  struct OpenAPI_Response resp;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  char *code = NULL;
  char *_ast_gen_body_32 = NULL;
  char *schema_name = "FormData";

  memset(&op, 0, sizeof(op));

  memset(&resp, 0, sizeof(resp));

  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));
  openapi_spec_init(&spec);

  spec.defined_schemas =
      (struct StructFields *)calloc(1, sizeof(struct StructFields));
  spec.defined_schema_names = (char **)calloc(1, sizeof(char *));
  ASSERT(spec.defined_schemas);
  ASSERT(spec.defined_schema_names);
  struct_fields_init(&spec.defined_schemas[0]);
  struct_fields_add(&spec.defined_schemas[0], "pet", "object", "Pet", NULL,
                    NULL);
  struct_fields_add(&spec.defined_schemas[0], "pets", "array", "Pet", NULL,
                    NULL);
  spec.defined_schema_names[0] = schema_name;
  spec.n_defined_schemas = 1;

  op.verb = OA_VERB_POST;
  resp.code = "200";
  op.responses = &resp;
  op.n_responses = 1;

  op.req_body.ref_name = "FormData";
  op.req_body.content_type = "multipart/form-data";

  code = (gen_body(&op, &spec, "/submit", NULL, &_ast_gen_body_32),
          _ast_gen_body_32);
  ASSERT(code);
  ASSERT(strstr(code, "Pet_to_json(req_body->pet") != NULL);
  ASSERT(strstr(code, "Pet_to_json(req_body->pets[i]") != NULL);
  ASSERT(strstr(code, "http_request_add_part(&req, \"pet\", NULL, "
                      "\"application/json\"") != NULL);
  ASSERT(strstr(code, "http_request_add_part(&req, \"pets\", NULL, "
                      "\"application/json\"") != NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_body_multipart_encoding_content_type(void) {
  struct OpenAPI_Encoding enc;
  struct OpenAPI_Response resp;
  struct OpenAPI_MediaType mt;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  char *code = NULL;
  char *_ast_gen_body_33 = NULL;
  char *schema_name = "Upload";

  memset(&op, 0, sizeof(op));

  memset(&resp, 0, sizeof(resp));

  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));
  memset(&mt, 0, sizeof(mt));
  memset(&enc, 0, sizeof(enc));
  openapi_spec_init(&spec);

  spec.defined_schemas =
      (struct StructFields *)calloc(1, sizeof(struct StructFields));
  spec.defined_schema_names = (char **)calloc(1, sizeof(char *));
  ASSERT(spec.defined_schemas);
  ASSERT(spec.defined_schema_names);
  struct_fields_init(&spec.defined_schemas[0]);
  struct_fields_add(&spec.defined_schemas[0], "title", "string", NULL, NULL,
                    NULL);
  spec.defined_schema_names[0] = schema_name;
  spec.n_defined_schemas = 1;

  op.verb = OA_VERB_POST;
  resp.code = "200";
  op.responses = &resp;
  op.n_responses = 1;

  op.req_body.ref_name = "Upload";
  op.req_body.content_type = "multipart/form-data";

  mt.name = "multipart/form-data";
  enc.name = "title";
  enc.content_type = "text/plain; charset=utf-8";
  mt.encoding = &enc;
  mt.n_encoding = 1;
  op.req_body_media_types = &mt;
  op.n_req_body_media_types = 1;

  code = (gen_body(&op, &spec, "/upload", NULL, &_ast_gen_body_33),
          _ast_gen_body_33);
  ASSERT(code);
  ASSERT(strstr(code, "Multipart Body Construction") != NULL);
  ASSERT(strstr(code, "http_request_add_part(&req, \"title\", NULL, "
                      "\"text/plain; charset=utf-8\"") != NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_body_multipart_encoding_content_type_list(void) {
  struct OpenAPI_Encoding enc;
  struct OpenAPI_Response resp;
  struct OpenAPI_MediaType mt;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  char *code = NULL;
  char *_ast_gen_body_34 = NULL;
  char *schema_name = "Upload";

  memset(&op, 0, sizeof(op));

  memset(&resp, 0, sizeof(resp));

  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));
  memset(&mt, 0, sizeof(mt));
  memset(&enc, 0, sizeof(enc));
  openapi_spec_init(&spec);

  spec.defined_schemas =
      (struct StructFields *)calloc(1, sizeof(struct StructFields));
  spec.defined_schema_names = (char **)calloc(1, sizeof(char *));
  ASSERT(spec.defined_schemas);
  ASSERT(spec.defined_schema_names);
  struct_fields_init(&spec.defined_schemas[0]);
  struct_fields_add(&spec.defined_schemas[0], "file", "string", NULL, NULL,
                    NULL);
  spec.defined_schema_names[0] = schema_name;
  spec.n_defined_schemas = 1;

  op.verb = OA_VERB_POST;
  resp.code = "200";
  op.responses = &resp;
  op.n_responses = 1;

  op.req_body.ref_name = "Upload";
  op.req_body.content_type = "multipart/form-data";

  mt.name = "multipart/form-data";
  enc.name = "file";
  enc.content_type = "image/png, image/jpeg";
  mt.encoding = &enc;
  mt.n_encoding = 1;
  op.req_body_media_types = &mt;
  op.n_req_body_media_types = 1;

  code = (gen_body(&op, &spec, "/upload", NULL, &_ast_gen_body_34),
          _ast_gen_body_34);
  ASSERT(code);
  ASSERT(strstr(code, "Multipart Body Construction") != NULL);
  ASSERT(strstr(code, "\"image/png\"") != NULL);
  ASSERT(strstr(code, "image/jpeg") == NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_body_multipart_encoding_headers(void) {
  struct OpenAPI_Encoding enc;
  struct OpenAPI_Header headers[3];
  struct OpenAPI_Response resp;
  struct OpenAPI_MediaType mt;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  char *code = NULL;
  char *_ast_gen_body_35 = NULL;
  char *schema_name = "Upload";

  memset(&op, 0, sizeof(op));

  memset(&resp, 0, sizeof(resp));

  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));
  memset(&mt, 0, sizeof(mt));
  memset(&enc, 0, sizeof(enc));
  memset(&headers, 0, sizeof(headers));
  openapi_spec_init(&spec);

  spec.defined_schemas =
      (struct StructFields *)calloc(1, sizeof(struct StructFields));
  spec.defined_schema_names = (char **)calloc(1, sizeof(char *));
  ASSERT(spec.defined_schemas);
  ASSERT(spec.defined_schema_names);
  struct_fields_init(&spec.defined_schemas[0]);
  struct_fields_add(&spec.defined_schemas[0], "title", "string", NULL, NULL,
                    NULL);
  spec.defined_schema_names[0] = schema_name;
  spec.n_defined_schemas = 1;

  op.verb = OA_VERB_POST;
  resp.code = "200";
  op.responses = &resp;
  op.n_responses = 1;

  op.req_body.ref_name = "Upload";
  op.req_body.content_type = "multipart/form-data";

  headers[0].name = "X-Trace";
  headers[0].type = "string";
  headers[1].name = "X-Ids";
  headers[1].type = "array";
  headers[1].is_array = 1;
  headers[1].items_type = "integer";
  headers[2].name = "Content-Type";
  headers[2].type = "string";

  mt.name = "multipart/form-data";
  enc.name = "title";
  enc.headers = headers;
  enc.n_headers = 3;
  mt.encoding = &enc;
  mt.n_encoding = 1;
  op.req_body_media_types = &mt;
  op.n_req_body_media_types = 1;

  code = (gen_body(&op, &spec, "/upload", NULL, &_ast_gen_body_35),
          _ast_gen_body_35);
  ASSERT(code);
  ASSERT(strstr(code, "http_request_add_part_header_last(&req, \"X-Trace\", "
                      "title_hdr_X_Trace") != NULL);
  ASSERT(strstr(code, "title_hdr_X_Ids_len") != NULL);
  ASSERT(strstr(code, "title_hdr_Content_Type") == NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_body_response_range_success(void) {
  struct OpenAPI_Response resp;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  char *code = NULL;
  char *_ast_gen_body_36 = NULL;

  memset(&op, 0, sizeof(op));

  memset(&resp, 0, sizeof(resp));

  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));
  openapi_spec_init(&spec);

  op.verb = OA_VERB_GET;
  resp.code = "2XX";
  resp.schema.ref_name = "Pet";
  op.responses = &resp;
  op.n_responses = 1;

  code = (gen_body(&op, &spec, "/", NULL, &_ast_gen_body_36), _ast_gen_body_36);
  ASSERT(code);
  ASSERT(strstr(code, "status_code >= 200") != NULL);
  ASSERT(strstr(code, "Pet_from_json") != NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_body_default_response_success(void) {
  struct OpenAPI_Response resp;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  char *code = NULL;
  char *_ast_gen_body_37 = NULL;

  memset(&op, 0, sizeof(op));

  memset(&resp, 0, sizeof(resp));

  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));
  openapi_spec_init(&spec);

  op.verb = OA_VERB_GET;
  resp.code = "default";
  resp.schema.ref_name = "Pet";
  op.responses = &resp;
  op.n_responses = 1;

  code = (gen_body(&op, &spec, "/", NULL, &_ast_gen_body_37), _ast_gen_body_37);
  ASSERT(code);
  ASSERT(strstr(code, "default response") != NULL);
  ASSERT(strstr(code, "Pet_from_json") != NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_body_text_plain_response_string(void) {
  struct OpenAPI_Response resp;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  char *code = NULL;
  char *_ast_gen_body_38 = NULL;

  memset(&op, 0, sizeof(op));

  memset(&resp, 0, sizeof(resp));

  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));
  openapi_spec_init(&spec);

  op.verb = OA_VERB_GET;
  resp.code = "200";
  resp.content_type = "text/plain; charset=utf-8";
  resp.schema.inline_type = "string";
  op.responses = &resp;
  op.n_responses = 1;

  code = (gen_body(&op, &spec, "/", NULL, &_ast_gen_body_38), _ast_gen_body_38);
  ASSERT(code);
  ASSERT(strstr(code, "memcpy(tmp, res->body") != NULL);
  ASSERT(strstr(code, "*out = tmp") != NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_body_text_plain_response_range(void) {
  struct OpenAPI_Response resp;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  char *code = NULL;
  char *_ast_gen_body_39 = NULL;

  memset(&op, 0, sizeof(op));

  memset(&resp, 0, sizeof(resp));

  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));
  openapi_spec_init(&spec);

  op.verb = OA_VERB_GET;
  resp.code = "2XX";
  resp.content_type = "text/plain";
  resp.schema.inline_type = "string";
  op.responses = &resp;
  op.n_responses = 1;

  code = (gen_body(&op, &spec, "/", NULL, &_ast_gen_body_39), _ast_gen_body_39);
  ASSERT(code);
  ASSERT(strstr(code, "status_code >= 200") != NULL);
  ASSERT(strstr(code, "memcpy(tmp, res->body") != NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_body_text_plain_response_default(void) {
  struct OpenAPI_Response resp;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  char *code = NULL;
  char *_ast_gen_body_40 = NULL;

  memset(&op, 0, sizeof(op));

  memset(&resp, 0, sizeof(resp));

  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));
  openapi_spec_init(&spec);

  op.verb = OA_VERB_GET;
  resp.code = "default";
  resp.content_type = "text/plain";
  resp.schema.inline_type = "string";
  op.responses = &resp;
  op.n_responses = 1;

  code = (gen_body(&op, &spec, "/", NULL, &_ast_gen_body_40), _ast_gen_body_40);
  ASSERT(code);
  ASSERT(strstr(code, "default response") != NULL);
  ASSERT(strstr(code, "memcpy(tmp, res->body") != NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_body_textual_response_xml(void) {
  struct OpenAPI_Response resp;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  char *code = NULL;
  char *_ast_gen_body_41 = NULL;

  memset(&op, 0, sizeof(op));

  memset(&resp, 0, sizeof(resp));

  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));
  openapi_spec_init(&spec);

  op.verb = OA_VERB_GET;
  resp.code = "200";
  resp.content_type = "application/xml; charset=utf-8";
  resp.schema.inline_type = "string";
  op.responses = &resp;
  op.n_responses = 1;

  code = (gen_body(&op, &spec, "/", NULL, &_ast_gen_body_41), _ast_gen_body_41);
  ASSERT(code);
  ASSERT(strstr(code, "memcpy(tmp, res->body") != NULL);
  ASSERT(strstr(code, "*out = tmp") != NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_body_binary_response_pdf(void) {
  struct OpenAPI_Response resp;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  char *code = NULL;
  char *_ast_gen_body_42 = NULL;

  memset(&op, 0, sizeof(op));

  memset(&resp, 0, sizeof(resp));

  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));
  openapi_spec_init(&spec);

  op.verb = OA_VERB_GET;
  resp.code = "200";
  resp.content_type = "application/pdf";
  op.responses = &resp;
  op.n_responses = 1;

  code = (gen_body(&op, &spec, "/", NULL, &_ast_gen_body_42), _ast_gen_body_42);
  ASSERT(code);
  ASSERT(strstr(code, "unsigned char *tmp") != NULL);
  ASSERT(strstr(code, "*out_len = res->body_len") != NULL);

  free(code);
  g_fail_io_after = -1;
  PASS();
}

TEST test_client_body_verb_mapping(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;

  FILE *fp = tmpfile();

  memset(&spec, 0, sizeof(spec));
  memset(&op, 0, sizeof(op));

  op.operation_id = "testVerb";

  /* Test additional non-standard verbs */
  op.is_additional = 1;

  op.method = NULL;
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  op.method = "get";
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  op.method = "post";
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  op.method = "put";
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  op.method = "delete";
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  op.method = "head";
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  op.method = "patch";
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  op.method = "options";
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  op.method = "trace";
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  op.method = "query";
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  op.method = "unknown";
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  fclose(fp);
  g_fail_io_after = -1;
  PASS();
}

TEST test_client_body_mapped_err_code(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;

  FILE *fp = tmpfile();

  memset(&spec, 0, sizeof(spec));
  memset(&op, 0, sizeof(op));

  op.operation_id = "testErrCode";
  op.n_responses = 5;
  op.responses = calloc(5, sizeof(*op.responses));
  op.responses[0].code = "400";
  op.responses[1].code = "401";
  op.responses[2].code = "403";
  op.responses[3].code = "404";
  op.responses[4].code = "500";

  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  free(op.responses);
  fclose(fp);
  g_fail_io_after = -1;
  PASS();
}

TEST test_client_body_media_type_matching(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;

  FILE *fp = tmpfile();

  memset(&spec, 0, sizeof(spec));
  memset(&op, 0, sizeof(op));

  op.method = "post";
  op.verb = OA_VERB_POST;
  op.operation_id = "testMediaMatch";
  op.summary = "Test summary";

  op.req_body.content_type = "application/vnd.github+JSON";
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  op.req_body.content_type = "APPLICATION/XML";
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  op.req_body.content_type = "multipart/form-data";
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  op.req_body.content_type = "text/plain";
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  fclose(fp);
  g_fail_io_after = -1;
  PASS();
}

TEST test_client_body_find_media_type_not_found(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;

  FILE *fp = tmpfile();

  memset(&spec, 0, sizeof(spec));
  memset(&op, 0, sizeof(op));

  op.operation_id = "testMediaTypeFindNotFound";
  op.method = "post";
  op.verb = OA_VERB_POST;

  op.req_body.content_type = "application/x-www-form-urlencoded";
  op.n_req_body_media_types = 1;
  op.req_body_media_types = calloc(1, sizeof(*op.req_body_media_types));
  op.req_body_media_types[0].name = "application/json"; /* Doesn't match */

  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  op.req_body.content_type = "multipart/form-data";
  op.req_body_media_types[0].name = "application/xml"; /* Doesn't match */

  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  free(op.req_body_media_types);
  fclose(fp);
  g_fail_io_after = -1;
  PASS();
}

TEST test_client_body_find_encoding_not_found(void) {
  struct OpenAPI_Encoding enc;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;

  FILE *fp = tmpfile();

  memset(&spec, 0, sizeof(spec));
  memset(&op, 0, sizeof(op));

  op.operation_id = "testEncodingFindNotFound";
  op.method = "post";
  op.verb = OA_VERB_POST;

  op.req_body.content_type = "multipart/form-data";
  op.req_body.ref_name = "MockSchema";

  /* Provide req_body_media_types */
  op.n_req_body_media_types = 1;
  op.req_body_media_types = calloc(1, sizeof(*op.req_body_media_types));
  op.req_body_media_types[0].name = "multipart/form-data";

  memset(&enc, 0, sizeof(enc));
  enc.name = "other_prop"; /* Different from test_prop */
  enc.content_type = "text/plain";

  op.req_body_media_types[0].n_encoding = 1;
  op.req_body_media_types[0].encoding = &enc;

  /* Setup the global Spec schema definitions */
  spec.n_defined_schemas = 1;
  spec.defined_schema_names = calloc(1, sizeof(char *));
  spec.defined_schema_names[0] = "MockSchema";

  spec.defined_schemas = calloc(1, sizeof(struct StructFields));
  spec.defined_schemas[0].size = 1;
  spec.defined_schemas[0].fields = calloc(1, sizeof(struct StructField));
  strcpy(spec.defined_schemas[0].fields[0].name, "test_prop");
  strcpy(spec.defined_schemas[0].fields[0].type, "string");

  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  free(spec.defined_schemas[0].fields);
  free(spec.defined_schemas);
  free(spec.defined_schema_names);
  free(op.req_body_media_types);
  fclose(fp);
  g_fail_io_after = -1;
  PASS();
}

TEST test_client_body_array_items_statics(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;

  FILE *fp = tmpfile();

  memset(&spec, 0, sizeof(spec));
  memset(&op, 0, sizeof(op));

  op.operation_id = "testArrayItemsStatics";
  op.method = "post";
  op.verb = OA_VERB_POST;

  op.req_body.content_type = "multipart/form-data";
  op.req_body.ref_name = "MockSchema2";

  op.n_req_body_media_types = 1;
  op.req_body_media_types = calloc(1, sizeof(*op.req_body_media_types));
  op.req_body_media_types[0].name = "multipart/form-data";

  spec.n_defined_schemas = 1;
  spec.defined_schema_names = calloc(1, sizeof(char *));
  spec.defined_schema_names[0] = "MockSchema2";

  spec.defined_schemas = calloc(1, sizeof(struct StructFields));
  spec.defined_schemas[0].size = 5;
  spec.defined_schemas[0].fields = calloc(5, sizeof(struct StructField));

  /* Field 0: array of object */
  strcpy(spec.defined_schemas[0].fields[0].name, "arr_obj");
  strcpy(spec.defined_schemas[0].fields[0].type, "array");
  strcpy(spec.defined_schemas[0].fields[0].ref, "object");

  /* Field 1: array of array */
  strcpy(spec.defined_schemas[0].fields[1].name, "arr_arr");
  strcpy(spec.defined_schemas[0].fields[1].type, "array");
  strcpy(spec.defined_schemas[0].fields[1].ref, "array");

  /* Field 2: array of enum */
  strcpy(spec.defined_schemas[0].fields[2].name, "arr_enum");
  strcpy(spec.defined_schemas[0].fields[2].type, "array");
  strcpy(spec.defined_schemas[0].fields[2].ref, "enum");

  /* Field 3: object */
  strcpy(spec.defined_schemas[0].fields[3].name, "obj_field");
  strcpy(spec.defined_schemas[0].fields[3].type, "object");

  /* Field 4: array with empty ref */
  strcpy(spec.defined_schemas[0].fields[4].name, "arr_str");
  strcpy(spec.defined_schemas[0].fields[4].type, "array");
  spec.defined_schemas[0].fields[4].ref[0] = '\0';

  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  op.req_body.content_type = "application/x-www-form-urlencoded";
  op.req_body_media_types[0].name = "application/x-www-form-urlencoded";
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  free(spec.defined_schemas[0].fields);
  free(spec.defined_schemas);
  free(spec.defined_schema_names);
  free(op.req_body_media_types);
  fclose(fp);
  g_fail_io_after = -1;
  PASS();
}

TEST test_client_body_verb_enum_indirect(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;

  FILE *fp = tmpfile();

  memset(&spec, 0, sizeof(spec));
  memset(&op, 0, sizeof(op));

  op.operation_id = "testVerb";

  op.verb = OA_VERB_PUT;
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  op.verb = OA_VERB_DELETE;
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  op.verb = OA_VERB_HEAD;
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  op.verb = OA_VERB_PATCH;
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  fclose(fp);
  g_fail_io_after = -1;
  PASS();
}

TEST test_client_body_header_formatting_indirect(void) {
  struct OpenAPI_Encoding enc;
  struct OpenAPI_Header hdr;
  struct OpenAPI_MultipartField mf;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;

  FILE *fp = tmpfile();

  memset(&spec, 0, sizeof(spec));
  memset(&op, 0, sizeof(op));

  op.method = "post";
  op.verb = OA_VERB_POST;
  op.operation_id = "testHdrFormat";

  /* Set up global schemas to bypass properties missing error */
  spec.n_defined_schemas = 1;
  spec.defined_schema_names = calloc(1, sizeof(char *));
  spec.defined_schema_names[0] = "MockSchemaHdr";

  spec.defined_schemas = calloc(1, sizeof(struct StructFields));
  spec.defined_schemas[0].size = 1;
  spec.defined_schemas[0].fields = calloc(1, sizeof(struct StructField));
  strcpy(spec.defined_schemas[0].fields[0].name, "1test_prop");
  strcpy(spec.defined_schemas[0].fields[0].type, "string");

  op.req_body.ref_name = "MockSchemaHdr";

  /* Multipart data with specific encodings to hit header name formatting */
  op.req_body.content_type = "multipart/mixed";
  op.n_req_body_media_types = 1;
  op.req_body_media_types = calloc(1, sizeof(*op.req_body_media_types));
  op.req_body_media_types[0].name = "multipart/mixed";

  memset(&enc, 0, sizeof(enc));
  enc.name = "1test_prop";

  memset(&hdr, 0, sizeof(hdr));
  hdr.name =
      "1Content-Type"; /* hit sanitize starting with number, and it isn't */
                       /* Content-Type exact */

  enc.n_headers = 1;
  enc.headers = &hdr;

  op.req_body_media_types[0].n_encoding = 1;
  op.req_body_media_types[0].encoding = &enc;

  memset(&mf, 0, sizeof(mf));
  mf.name = "1test_prop";
  mf.type = "string";
  mf.is_binary = 0;
  op.req_body.n_multipart_fields = 1;
  op.req_body.multipart_fields = &mf;

  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  free(spec.defined_schemas[0].fields);
  free(spec.defined_schemas);
  free(spec.defined_schema_names);
  free(op.req_body_media_types);
  fclose(fp);
  g_fail_io_after = -1;
  PASS();
}

TEST test_client_body_media_types_textual_binary_indirect(void) {
  struct OpenAPI_Encoding enc;
  struct OpenAPI_MultipartField mf;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;

  FILE *fp = tmpfile();

  memset(&spec, 0, sizeof(spec));
  memset(&op, 0, sizeof(op));

  op.method = "post";
  op.verb = OA_VERB_POST;
  op.operation_id = "testMediaTypeClassifiers";

  spec.n_defined_schemas = 1;
  spec.defined_schema_names = calloc(1, sizeof(char *));
  spec.defined_schema_names[0] = "MockSchemaTxtBin";

  spec.defined_schemas = calloc(1, sizeof(struct StructFields));
  spec.defined_schemas[0].size = 1;
  spec.defined_schemas[0].fields = calloc(1, sizeof(struct StructField));
  strcpy(spec.defined_schemas[0].fields[0].name, "test_prop");
  strcpy(spec.defined_schemas[0].fields[0].type, "string");

  op.req_body.ref_name = "MockSchemaTxtBin";

  /* Hit textual formatters indirectly via multipart field generation */
  op.req_body.content_type = "multipart/mixed";
  op.n_req_body_media_types = 1;
  op.req_body_media_types = calloc(1, sizeof(*op.req_body_media_types));
  op.req_body_media_types[0].name = "multipart/mixed";

  memset(&enc, 0, sizeof(enc));
  enc.name = "test_prop";

  op.req_body_media_types[0].n_encoding = 1;
  op.req_body_media_types[0].encoding = &enc;

  memset(&mf, 0, sizeof(mf));
  mf.name = "test_prop";
  mf.type = "string";
  mf.is_binary = 0;
  op.req_body.n_multipart_fields = 1;
  op.req_body.multipart_fields = &mf;

  /* text/html */
  enc.content_type = "text/html";
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  /* application/xml */
  enc.content_type = "application/xml";
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  /* application/rss+xml */
  enc.content_type = "application/rss+xml";
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  /* image/png (binary) */
  enc.content_type = "image/png";
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  /* audio/mp3 (binary) */
  enc.content_type = "audio/mp3";
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  /* video/mp4 (binary) */
  enc.content_type = "video/mp4";
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  /* application/pdf (binary) */
  enc.content_type = "application/pdf";
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  free(spec.defined_schemas[0].fields);
  free(spec.defined_schemas);
  free(spec.defined_schema_names);
  free(op.req_body_media_types);
  fclose(fp);
  g_fail_io_after = -1;
  PASS();
}

TEST test_client_body_media_types_textual_binary_missing_branches_indirect(
    void) {
  struct OpenAPI_Encoding enc;
  struct OpenAPI_MultipartField mf;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;

  FILE *fp = tmpfile();

  memset(&spec, 0, sizeof(spec));
  memset(&op, 0, sizeof(op));

  op.method = "post";
  op.verb = OA_VERB_POST;
  op.operation_id = "testMediaTypeClassifiersMissing";

  spec.n_defined_schemas = 1;
  spec.defined_schema_names = calloc(1, sizeof(char *));
  spec.defined_schema_names[0] = "MockSchemaMissing";

  spec.defined_schemas = calloc(1, sizeof(struct StructFields));
  spec.defined_schemas[0].size = 1;
  spec.defined_schemas[0].fields = calloc(1, sizeof(struct StructField));
  strcpy(spec.defined_schemas[0].fields[0].name, "test_prop");
  strcpy(spec.defined_schemas[0].fields[0].type, "string");

  op.req_body.ref_name = "MockSchemaMissing";

  /* Hit textual formatters indirectly via multipart field generation */
  op.req_body.content_type = "multipart/mixed";
  op.n_req_body_media_types = 1;
  op.req_body_media_types = calloc(1, sizeof(*op.req_body_media_types));
  op.req_body_media_types[0].name = "multipart/mixed";

  memset(&enc, 0, sizeof(enc));
  enc.name = "test_prop";

  op.req_body_media_types[0].n_encoding = 1;
  op.req_body_media_types[0].encoding = &enc;

  memset(&mf, 0, sizeof(mf));
  mf.name = "test_prop";
  mf.type = "string";
  mf.is_binary = 0;
  op.req_body.n_multipart_fields = 1;
  op.req_body.multipart_fields = &mf;

  /* text/css (textual prefix) */
  enc.content_type = "text/css";
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  /* application/atom+xml (textual suffix) */
  enc.content_type = "application/atom+xml";
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  free(spec.defined_schemas[0].fields);
  free(spec.defined_schemas);
  free(spec.defined_schema_names);
  free(op.req_body_media_types);
  fclose(fp);
  g_fail_io_after = -1;
  PASS();
}

TEST test_client_body_media_type_caps_indirect(void) {
  struct OpenAPI_Encoding enc;
  struct OpenAPI_MultipartField mf;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;

  FILE *fp = tmpfile();

  memset(&spec, 0, sizeof(spec));
  memset(&op, 0, sizeof(op));

  op.method = "post";
  op.verb = OA_VERB_POST;
  op.operation_id = "testMediaCapsIndirect";

  spec.n_defined_schemas = 1;
  spec.defined_schema_names = calloc(1, sizeof(char *));
  spec.defined_schema_names[0] = "MockSchemaCaps";

  spec.defined_schemas = calloc(1, sizeof(struct StructFields));
  spec.defined_schemas[0].size = 1;
  spec.defined_schemas[0].fields = calloc(1, sizeof(struct StructField));
  strcpy(spec.defined_schemas[0].fields[0].name, "test_prop");
  strcpy(spec.defined_schemas[0].fields[0].type, "string");

  op.req_body.ref_name = "MockSchemaCaps";

  /* Mixed upper and lower caps */
  op.req_body.content_type = "MULTIPART/MIXED";
  op.n_req_body_media_types = 1;
  op.req_body_media_types = calloc(1, sizeof(*op.req_body_media_types));
  op.req_body_media_types[0].name = "multipart/mixed";

  memset(&enc, 0, sizeof(enc));
  enc.name = "test_prop";

  op.req_body_media_types[0].n_encoding = 1;
  op.req_body_media_types[0].encoding = &enc;

  memset(&mf, 0, sizeof(mf));
  mf.name = "test_prop";
  mf.type = "string";
  mf.is_binary = 0;
  op.req_body.n_multipart_fields = 1;
  op.req_body.multipart_fields = &mf;

  /* text/plain in caps */
  enc.content_type = "TEXT/PLAIN";
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  free(spec.defined_schemas[0].fields);
  free(spec.defined_schemas);
  free(spec.defined_schema_names);
  free(op.req_body_media_types);
  fclose(fp);
  g_fail_io_after = -1;
  PASS();
}

TEST test_client_body_media_type_prefix_caps_indirect(void) {
  struct OpenAPI_Encoding enc;
  struct OpenAPI_MultipartField mf;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;

  FILE *fp = tmpfile();

  memset(&spec, 0, sizeof(spec));
  memset(&op, 0, sizeof(op));

  op.method = "post";
  op.verb = OA_VERB_POST;
  op.operation_id = "testMediaPrefixCapsIndirect";

  spec.n_defined_schemas = 1;
  spec.defined_schema_names = calloc(1, sizeof(char *));
  spec.defined_schema_names[0] = "MockSchemaPrefixCaps";

  spec.defined_schemas = calloc(1, sizeof(struct StructFields));
  spec.defined_schemas[0].size = 1;
  spec.defined_schemas[0].fields = calloc(1, sizeof(struct StructField));
  strcpy(spec.defined_schemas[0].fields[0].name, "test_prop");
  strcpy(spec.defined_schemas[0].fields[0].type, "string");

  op.req_body.ref_name = "MockSchemaPrefixCaps";

  /* Hit prefix upper caps */
  op.req_body.content_type = "MULTIPART/MIXED";
  op.n_req_body_media_types = 1;
  op.req_body_media_types = calloc(1, sizeof(*op.req_body_media_types));
  op.req_body_media_types[0].name = "multipart/mixed";

  memset(&enc, 0, sizeof(enc));
  enc.name = "test_prop";

  op.req_body_media_types[0].n_encoding = 1;
  op.req_body_media_types[0].encoding = &enc;

  memset(&mf, 0, sizeof(mf));
  mf.name = "test_prop";
  mf.type = "string";
  mf.is_binary = 0;
  op.req_body.n_multipart_fields = 1;
  op.req_body.multipart_fields = &mf;

  /* text/plain in mixed caps to hit prefix */
  enc.content_type = "TeXt/PlaIN";
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  free(spec.defined_schemas[0].fields);
  free(spec.defined_schemas);
  free(spec.defined_schema_names);
  free(op.req_body_media_types);
  fclose(fp);
  g_fail_io_after = -1;
  PASS();
}

TEST test_client_body_media_type_prefix_suffix_short(void) {
  struct OpenAPI_Encoding enc;
  struct OpenAPI_MultipartField mf;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;

  FILE *fp = tmpfile();

  memset(&spec, 0, sizeof(spec));
  memset(&op, 0, sizeof(op));

  op.method = "post";
  op.verb = OA_VERB_POST;
  op.operation_id = "testMediaShort";

  spec.n_defined_schemas = 1;
  spec.defined_schema_names = calloc(1, sizeof(char *));
  spec.defined_schema_names[0] = "MockSchemaShort";

  spec.defined_schemas = calloc(1, sizeof(struct StructFields));
  spec.defined_schemas[0].size = 1;
  spec.defined_schemas[0].fields = calloc(1, sizeof(struct StructField));
  strcpy(spec.defined_schemas[0].fields[0].name, "test_prop");
  strcpy(spec.defined_schemas[0].fields[0].type, "string");

  op.req_body.ref_name = "MockSchemaShort";

  op.req_body.content_type = "multipart/mixed";
  op.n_req_body_media_types = 1;
  op.req_body_media_types = calloc(1, sizeof(*op.req_body_media_types));
  op.req_body_media_types[0].name = "multipart/mixed";

  memset(&enc, 0, sizeof(enc));
  enc.name = "test_prop";

  op.req_body_media_types[0].n_encoding = 1;
  op.req_body_media_types[0].encoding = &enc;

  memset(&mf, 0, sizeof(mf));
  mf.name = "test_prop";
  mf.type = "string";
  mf.is_binary = 0;
  op.req_body.n_multipart_fields = 1;
  op.req_body.multipart_fields = &mf;

  /* text/ (len < pre_len = 5) -> e.g. "tex" */
  enc.content_type = "tex";
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  /* +xml (len < suf_len = 4) -> e.g. "xm" */
  enc.content_type = "xm";
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  free(spec.defined_schemas[0].fields);
  free(spec.defined_schemas);
  free(spec.defined_schema_names);
  free(op.req_body_media_types);
  fclose(fp);
  g_fail_io_after = -1;
  PASS();
}

TEST test_client_body_write_inline_json_parse_indirect(void) {
  struct OpenAPI_Response resp;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;

  FILE *fp = tmpfile();

  memset(&spec, 0, sizeof(spec));
  memset(&op, 0, sizeof(op));

  op.method = "get";
  op.verb = OA_VERB_GET;
  op.operation_id = "testInlineParseIndirect";

  /* We need to hit write_response_read -> write_inline_json_parse */
  op.n_responses = 1;

  memset(&resp, 0, sizeof(resp));
  resp.code = "200";

  /* Setup media type with inline schema */
  resp.n_content_media_types = 1;
  resp.content_media_types = calloc(1, sizeof(*resp.content_media_types));
  resp.content_media_types[0].name = "application/json";

  /* Test array of string */
  resp.content_media_types[0].schema.inline_type = "string";
  resp.content_media_types[0].schema.is_array = 1;
  op.responses = &resp;

  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  /* Test array of integer */
  resp.content_media_types[0].schema.inline_type = "integer";
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  /* Test array of number */
  resp.content_media_types[0].schema.inline_type = "number";
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  /* Test array of boolean */
  resp.content_media_types[0].schema.inline_type = "boolean";
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  /* Test raw boolean */
  resp.content_media_types[0].schema.is_array = 0;
  resp.content_media_types[0].schema.inline_type = "boolean";
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  /* Test unhandled / unknown (e.g., fallback) */
  resp.content_media_types[0].schema.inline_type = "unknown_type_test";
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  /* Add missing branch from write_response_read (raw array of non-json) */
  resp.content_media_types[0].name = "text/plain";
  resp.content_media_types[0].schema.is_array = 1;
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  free(resp.content_media_types);
  fclose(fp);
  g_fail_io_after = -1;
  PASS();
}

TEST test_client_body_write_inline_json_parse_types(void) {
  struct OpenAPI_Response resp;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;

  FILE *fp = tmpfile();

  memset(&spec, 0, sizeof(spec));
  memset(&op, 0, sizeof(op));

  op.method = "get";
  op.verb = OA_VERB_GET;
  op.operation_id = "testInlineParseTypes";

  op.n_responses = 1;

  memset(&resp, 0, sizeof(resp));
  resp.code = "200";

  resp.n_content_media_types = 1;
  resp.content_media_types = calloc(1, sizeof(*resp.content_media_types));
  resp.content_media_types[0].name = "application/json";
  op.responses = &resp;

  /* array of string */
  resp.content_media_types[0].schema.inline_type = "string";
  resp.content_media_types[0].schema.is_array = 1;
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  /* array of int */
  resp.content_media_types[0].schema.inline_type = "integer";
  resp.content_media_types[0].schema.is_array = 1;
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  /* array of number */
  resp.content_media_types[0].schema.inline_type = "number";
  resp.content_media_types[0].schema.is_array = 1;
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  /* array of boolean */
  resp.content_media_types[0].schema.inline_type = "boolean";
  resp.content_media_types[0].schema.is_array = 1;
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  /* array of fallback/unknown */
  resp.content_media_types[0].schema.inline_type = "unknown_test";
  resp.content_media_types[0].schema.is_array = 1;
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  /* raw boolean */
  resp.content_media_types[0].schema.inline_type = "boolean";
  resp.content_media_types[0].schema.is_array = 0;
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  /* array of string (non-json) */
  resp.content_media_types[0].name = "text/plain";
  resp.content_media_types[0].schema.inline_type = "string";
  resp.content_media_types[0].schema.is_array = 1;
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  free(resp.content_media_types);
  fclose(fp);
  g_fail_io_after = -1;
  PASS();
}

TEST test_client_body_write_inline_json_parse_types_indirect(void) {
  struct OpenAPI_Response resp;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;

  FILE *fp = tmpfile();

  memset(&spec, 0, sizeof(spec));
  memset(&op, 0, sizeof(op));

  op.method = "get";
  op.verb = OA_VERB_GET;
  op.operation_id = "testInlineParseTypes";

  op.n_responses = 1;

  memset(&resp, 0, sizeof(resp));
  resp.code = "200";

  resp.n_content_media_types = 1;
  resp.content_media_types = calloc(1, sizeof(*resp.content_media_types));
  resp.content_media_types[0].name = "application/json";
  op.responses = &resp;

  /* Need to provide a valid body payload via dummy schema */
  /* so the codegen logic is fully hit and NOT skipped! */

  /* array of string */
  resp.content_media_types[0].schema.inline_type = "string";
  resp.content_media_types[0].schema.is_array = 1;
  /* Make it think the response is an object with an inline schema so it hits */
  /* the array codegen! No, write_inline_json_parse writes code! We just need it
   */
  /* to emit the C code blocks! */
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  /* array of int */
  resp.content_media_types[0].schema.inline_type = "integer";
  resp.content_media_types[0].schema.is_array = 1;
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  /* array of number */
  resp.content_media_types[0].schema.inline_type = "number";
  resp.content_media_types[0].schema.is_array = 1;
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  /* array of boolean */
  resp.content_media_types[0].schema.inline_type = "boolean";
  resp.content_media_types[0].schema.is_array = 1;
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  /* array of fallback/unknown */
  resp.content_media_types[0].schema.inline_type = "unknown_test";
  resp.content_media_types[0].schema.is_array = 1;
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  /* raw boolean */
  resp.content_media_types[0].schema.inline_type = "boolean";
  resp.content_media_types[0].schema.is_array = 0;
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  free(resp.content_media_types);
  fclose(fp);
  g_fail_io_after = -1;
  PASS();
}

TEST test_client_body_form_object_style_form_explode(void) {
  struct OpenAPI_Encoding enc;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;

  FILE *fp = tmpfile();

  memset(&spec, 0, sizeof(spec));
  memset(&op, 0, sizeof(op));

  op.method = "post";
  op.verb = OA_VERB_POST;
  op.operation_id = "testFormObjStyle";

  spec.n_defined_schemas = 1;
  spec.defined_schema_names = calloc(1, sizeof(char *));
  spec.defined_schema_names[0] = "MockSchemaFormObj";

  spec.defined_schemas = calloc(1, sizeof(struct StructFields));
  spec.defined_schemas[0].size = 1;
  spec.defined_schemas[0].fields = calloc(1, sizeof(struct StructField));
  strcpy(spec.defined_schemas[0].fields[0].name, "obj_prop");
  strcpy(spec.defined_schemas[0].fields[0].type, "object");
  strcpy(spec.defined_schemas[0].fields[0].ref,
         "MockSchemaFormObj"); /* self-ref for test */

  op.req_body.ref_name = "MockSchemaFormObj";

  op.req_body.content_type = "application/x-www-form-urlencoded";
  op.n_req_body_media_types = 1;
  op.req_body_media_types = calloc(1, sizeof(*op.req_body_media_types));
  op.req_body_media_types[0].name = "application/x-www-form-urlencoded";

  memset(&enc, 0, sizeof(enc));
  enc.name = "obj_prop";
  enc.style_set = 1;
  enc.style = OA_STYLE_FORM;
  enc.explode_set = 1;
  enc.explode = 1;

  op.req_body_media_types[0].n_encoding = 1;
  op.req_body_media_types[0].encoding = &enc;

  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  free(spec.defined_schemas[0].fields);
  free(spec.defined_schemas);
  free(spec.defined_schema_names);
  free(op.req_body_media_types);
  fclose(fp);
  g_fail_io_after = -1;
  PASS();
}

TEST test_client_body_cookie_object_style_form_explode(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;

  FILE *fp = tmpfile();

  memset(&spec, 0, sizeof(spec));
  memset(&op, 0, sizeof(op));

  op.method = "get";
  op.verb = OA_VERB_GET;
  op.operation_id = "testCookieObjStyle";

  op.n_parameters = 1;
  op.parameters = calloc(1, sizeof(*op.parameters));
  op.parameters[0].name = "cookie_obj";
  op.parameters[0].in = OA_PARAM_IN_COOKIE;
  op.parameters[0].type = "object";
  op.parameters[0].style = OA_STYLE_FORM;
  op.parameters[0].explode_set = 1;
  op.parameters[0].explode = 1;

  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  free(op.parameters);
  fclose(fp);
  g_fail_io_after = -1;
  PASS();
}

TEST test_client_body_response_is_textual_string_indirect(void) {
  struct OpenAPI_Response resp;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;

  FILE *fp = tmpfile();

  memset(&spec, 0, sizeof(spec));
  memset(&op, 0, sizeof(op));

  op.method = "get";
  op.verb = OA_VERB_GET;
  op.operation_id = "testResponseIsTextualStringIndirect";

  op.n_responses = 1;

  memset(&resp, 0, sizeof(resp));
  resp.code = "200";

  /* Set up content type directly on the response to trigger */
  /* `response_is_textual_string` */
  resp.content_type = "text/plain";
  resp.schema.inline_type = "string";
  resp.schema.is_array = 0;

  /* Also we need to make sure the media_types don't override it in the new */
  /* parser logic, or maybe it's not even used? Let's just set it. */
  op.responses = &resp;

  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  /* Also hit schema_has_inline missing branches */
  resp.content_type = "text/plain";
  resp.schema.inline_type = "integer"; /* not string */
  resp.schema.is_array = 0;
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  fclose(fp);
  g_fail_io_after = -1;
  PASS();
}

TEST test_client_body_response_is_textual_string_success(void) {
  struct OpenAPI_Response resp;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;

  FILE *fp = tmpfile();

  memset(&spec, 0, sizeof(spec));
  memset(&op, 0, sizeof(op));

  op.method = "get";
  op.verb = OA_VERB_GET;
  op.operation_id = "testResponseTextualSuccess";

  op.n_responses = 1;

  memset(&resp, 0, sizeof(resp));
  resp.code = "200";
  resp.content_type = "text/plain";
  resp.schema.inline_type = "string";
  resp.schema.is_array = 0;

  op.responses = &resp;

  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  fclose(fp);
  g_fail_io_after = -1;
  PASS();
}

TEST test_client_body_write_text_plain_success_indirect(void) {
  struct OpenAPI_Response resp;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;

  FILE *fp = tmpfile();

  memset(&spec, 0, sizeof(spec));
  memset(&op, 0, sizeof(op));

  op.method = "get";
  op.verb = OA_VERB_GET;
  op.operation_id = "testResponseTextualSuccessIndirect";

  op.n_responses = 1;

  memset(&resp, 0, sizeof(resp));
  resp.code = "200";
  resp.content_type = "text/plain";
  resp.schema.inline_type = "string";
  resp.schema.is_array = 0;

  resp.n_content_media_types = 1;
  resp.content_media_types = calloc(1, sizeof(*resp.content_media_types));
  resp.content_media_types[0].name = "text/plain";
  resp.content_media_types[0].schema.inline_type = "string";
  resp.content_media_types[0].schema.is_array = 0;

  op.responses = &resp;

  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  free(resp.content_media_types);
  fclose(fp);
  g_fail_io_after = -1;
  PASS();
}

TEST test_client_body_write_binary_success_indirect_real(void) {
  struct OpenAPI_Response resp;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;

  FILE *fp = tmpfile();

  memset(&spec, 0, sizeof(spec));
  memset(&op, 0, sizeof(op));

  op.method = "get";
  op.verb = OA_VERB_GET;
  op.operation_id = "testResponseBinarySuccessIndirectReal";

  op.n_responses = 1;

  memset(&resp, 0, sizeof(resp));
  resp.code = "200";
  resp.content_type = "image/png"; /* Binary! */

  op.responses = &resp;

  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  fclose(fp);
  g_fail_io_after = -1;
  PASS();
}

TEST test_client_body_write_text_plain_success_indirect_real_fixed(void) {
  struct OpenAPI_Response resp;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;

  FILE *fp = tmpfile();

  memset(&spec, 0, sizeof(spec));
  memset(&op, 0, sizeof(op));

  op.method = "get";
  op.verb = OA_VERB_GET;
  op.operation_id = "testResponseTextualSuccessIndirectRealFixed";

  op.n_responses = 1;

  memset(&resp, 0, sizeof(resp));
  resp.code = "200";
  resp.content_type = "text/plain";
  resp.schema.inline_type = "string";
  resp.schema.is_array = 0;

  /* Make sure it DOES NOT have a ref_name */
  resp.schema.ref_name = NULL;

  op.responses = &resp;

  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  fclose(fp);
  g_fail_io_after = -1;
  PASS();
}

TEST test_client_body_write_text_plain_success_indirect_real_fixed4(void) {
  struct OpenAPI_Response resp;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;

  FILE *fp = tmpfile();

  memset(&spec, 0, sizeof(spec));
  memset(&op, 0, sizeof(op));

  op.method = "get";
  op.verb = OA_VERB_GET;
  op.operation_id = "testResponseTextualSuccessIndirectRealFixed4";

  op.n_responses = 1;

  memset(&resp, 0, sizeof(resp));
  resp.code = "200";

  /* Set up content type in the media_types map instead! */
  resp.n_content_media_types = 1;
  resp.content_media_types = calloc(1, sizeof(*resp.content_media_types));
  resp.content_media_types[0].name = "text/plain";
  resp.content_media_types[0].schema.inline_type = "string";
  resp.content_media_types[0].schema.is_array = 0;

  /* Make sure to populate the root content_type so `response_is_textual_string`
   */
  /* works! */
  resp.content_type = "text/plain";

  op.responses = &resp;

  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  free(resp.content_media_types);
  fclose(fp);
  g_fail_io_after = -1;
  PASS();
}

TEST test_client_body_write_inline_json_parse_types_indirect_string(void) {
  struct OpenAPI_Response resp;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;

  FILE *fp = tmpfile();

  memset(&spec, 0, sizeof(spec));
  memset(&op, 0, sizeof(op));

  op.method = "get";
  op.verb = OA_VERB_GET;
  op.operation_id = "testInlineParseTypesString";

  op.n_responses = 1;

  memset(&resp, 0, sizeof(resp));
  resp.code = "200";

  resp.n_content_media_types = 1;
  resp.content_media_types = calloc(1, sizeof(*resp.content_media_types));
  resp.content_media_types[0].name = "application/json";
  op.responses = &resp;

  /* Make sure it doesn't get blocked by success_schema_name! */
  resp.schema.ref_name = NULL;

  /* array of string */
  resp.schema.inline_type = "string";
  resp.schema.is_array = 1;
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  /* array of integer */
  resp.schema.inline_type = "integer";
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  /* array of number */
  resp.schema.inline_type = "number";
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  /* array of boolean */
  resp.schema.inline_type = "boolean";
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  /* array of fallback */
  resp.schema.inline_type = "unknown_type";
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  /* not an array boolean */
  resp.schema.inline_type = "boolean";
  resp.schema.is_array = 0;
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  free(resp.content_media_types);
  fclose(fp);
  g_fail_io_after = -1;
  PASS();
}

TEST test_client_body_write_joined_form_array(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;

  FILE *fp = tmpfile();

  memset(&spec, 0, sizeof(spec));
  memset(&op, 0, sizeof(op));

  op.method = "get";
  op.verb = OA_VERB_GET;
  op.operation_id = "testJoinedFormArray";

  op.n_parameters = 1;
  op.parameters = calloc(1, sizeof(*op.parameters));
  op.parameters[0].name = "query_arr";
  op.parameters[0].in = OA_PARAM_IN_QUERY;
  op.parameters[0].type = "array";
  op.parameters[0].items_type = "object";
  op.parameters[0].style = OA_STYLE_FORM;
  op.parameters[0].explode_set = 1;
  op.parameters[0].explode = 0;

  /* Set array of strings basically via mock */
  op.parameters[0].items_type = "string";

  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  /* Also try different delim via pipedd */
  op.parameters[0].style = OA_STYLE_PIPE_DELIMITED;
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  /* space delimited */
  op.parameters[0].style = OA_STYLE_SPACE_DELIMITED;
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  free(op.parameters);
  fclose(fp);
  g_fail_io_after = -1;
  PASS();
}

TEST test_client_body_write_text_plain_success_indirect_real_fixed3(void) {
  struct OpenAPI_Response resp;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;

  FILE *fp = tmpfile();

  memset(&spec, 0, sizeof(spec));
  memset(&op, 0, sizeof(op));

  op.method = "get";
  op.verb = OA_VERB_GET;
  op.operation_id = "testResponseTextualSuccessIndirectRealFixed3";

  op.n_responses = 1;

  memset(&resp, 0, sizeof(resp));
  resp.code = "200";
  resp.content_type = "text/plain";
  resp.schema.inline_type = "string";
  resp.schema.is_array = 0;

  op.responses = &resp;

  /* Let's actually test response_is_textual_string AND schema_inline_is_string
   */
  /* indirectly The reason it wasn't hit previously is because */
  /* codegen_client_write_body skips generating the response code if we don't */
  /* have any parameters! No wait. Let's ensure the full operation is populated
   */
  /* properly. */
  codegen_client_write_body(fp, &op, &spec, "/path", NULL);

  fclose(fp);
  g_fail_io_after = -1;
  PASS();
}

SUITE(client_body_suite) {
  RUN_TEST(test_client_body_verb_mapping);
  RUN_TEST(test_client_body_mapped_err_code);
  RUN_TEST(test_client_body_media_type_matching);
  RUN_TEST(test_client_body_find_media_type_not_found);
  RUN_TEST(test_client_body_find_encoding_not_found);
  RUN_TEST(test_client_body_array_items_statics);
  RUN_TEST(test_client_body_verb_enum_indirect);
  RUN_TEST(test_client_body_header_formatting_indirect);
  RUN_TEST(test_client_body_media_types_textual_binary_indirect);
  RUN_TEST(
      test_client_body_media_types_textual_binary_missing_branches_indirect);
  RUN_TEST(test_client_body_media_type_caps_indirect);
  RUN_TEST(test_client_body_media_type_prefix_caps_indirect);
  RUN_TEST(test_client_body_media_type_prefix_suffix_short);
  RUN_TEST(test_client_body_write_inline_json_parse_indirect);
  RUN_TEST(test_client_body_write_inline_json_parse_types);
  RUN_TEST(test_client_body_write_inline_json_parse_types_indirect);
  RUN_TEST(test_client_body_form_object_style_form_explode);
  RUN_TEST(test_client_body_cookie_object_style_form_explode);
  RUN_TEST(test_client_body_response_is_textual_string_indirect);
  RUN_TEST(test_client_body_response_is_textual_string_success);
  RUN_TEST(test_client_body_write_text_plain_success_indirect);
  RUN_TEST(test_client_body_write_binary_success_indirect_real);
  RUN_TEST(test_client_body_write_text_plain_success_indirect_real_fixed);
  RUN_TEST(test_client_body_write_text_plain_success_indirect_real_fixed4);
  RUN_TEST(test_client_body_write_inline_json_parse_types_indirect_string);
  RUN_TEST(test_client_body_write_joined_form_array);
  RUN_TEST(test_client_body_write_joined_form_array);
  RUN_TEST(test_client_body_write_text_plain_success_indirect_real_fixed3);
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
  RUN_TEST(test_body_header_array_param);
  RUN_TEST(test_body_header_object_param);
  RUN_TEST(test_body_header_json_param_ref);
  RUN_TEST(test_body_header_number_param);
  RUN_TEST(test_body_cookie_param);
  RUN_TEST(test_body_cookie_param_number_array);
  RUN_TEST(test_body_cookie_param_array_explode_false);
  RUN_TEST(test_body_cookie_param_object_form);
  RUN_TEST(test_body_cookie_param_string_allow_reserved);
  RUN_TEST(test_body_security_query_api_key);
  RUN_TEST(test_body_security_cookie_api_key);
  RUN_TEST(test_body_form_urlencoded);
  RUN_TEST(test_body_form_urlencoded_with_params);
  RUN_TEST(test_body_form_urlencoded_object_fields);
  RUN_TEST(test_body_form_urlencoded_object_style_form_explode_true);
  RUN_TEST(test_body_form_urlencoded_object_style_form_explode_false);
  RUN_TEST(test_body_form_urlencoded_object_style_deep_object);
  RUN_TEST(test_body_multipart_primitives_and_arrays);
  RUN_TEST(test_body_multipart_object_fields);
  RUN_TEST(test_body_multipart_encoding_content_type);
  RUN_TEST(test_body_multipart_encoding_content_type_list);
  RUN_TEST(test_body_multipart_encoding_headers);
  RUN_TEST(test_body_response_range_success);
  RUN_TEST(test_body_default_response_success);
  RUN_TEST(test_body_text_plain_response_string);
  RUN_TEST(test_body_text_plain_response_range);
  RUN_TEST(test_body_text_plain_response_default);
  RUN_TEST(test_body_textual_response_xml);
  RUN_TEST(test_body_binary_response_pdf);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CODEGEN_CLIENT_BODY_H */
