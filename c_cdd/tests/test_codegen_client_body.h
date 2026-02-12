/**
 * @file test_codegen_client_body.h
 * @brief Unit tests for Client Body Logic Generator.
 */

#ifndef TEST_CODEGEN_CLIENT_BODY_H
#define TEST_CODEGEN_CLIENT_BODY_H

#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "codegen_client_body.h"
#include "codegen_struct.h"
#include "openapi_loader.h"

static char *gen_body(const struct OpenAPI_Operation *op,
                      const struct OpenAPI_Spec *spec, const char *tmpl) {
  FILE *tmp = tmpfile();
  long sz;
  char *content;

  if (!tmp)
    return NULL;

  if (codegen_client_write_body(tmp, op, spec, tmpl) != 0) {
    fclose(tmp);
    return NULL;
  }

  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);

  content = (char *)calloc(1, sz + 1);
  if (sz > 0)
    fread(content, 1, sz, tmp);

  fclose(tmp);
  return content;
}

TEST test_body_basic_get(void) {
  struct OpenAPI_Operation op;
  struct OpenAPI_Response resp;
  struct OpenAPI_Spec spec;
  char *code;

  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));
  openapi_spec_init(&spec);

  op.verb = OA_VERB_GET;
  resp.code = "200";
  op.responses = &resp;
  op.n_responses = 1;

  code = gen_body(&op, &spec, "/");
  ASSERT(code);

  /* Check error init */
  ASSERT(strstr(code, "if (api_error) *api_error = NULL;"));

  /* Check default failure parsing */
  ASSERT(strstr(code, "if (res->body && api_error)"));
  ASSERT(strstr(code, "ApiError_from_json"));

  free(code);
  openapi_spec_free(&spec);
  PASS();
}

TEST test_body_options_verb(void) {
  struct OpenAPI_Operation op;
  struct OpenAPI_Response resp;
  struct OpenAPI_Spec spec;
  char *code;

  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));
  openapi_spec_init(&spec);

  op.verb = OA_VERB_OPTIONS;
  resp.code = "200";
  op.responses = &resp;
  op.n_responses = 1;

  code = gen_body(&op, &spec, "/");
  ASSERT(code);
  ASSERT(strstr(code, "req.method = HTTP_OPTIONS;") != NULL);

  free(code);
  openapi_spec_free(&spec);
  PASS();
}

TEST test_body_trace_verb(void) {
  struct OpenAPI_Operation op;
  struct OpenAPI_Response resp;
  struct OpenAPI_Spec spec;
  char *code;

  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));
  openapi_spec_init(&spec);

  op.verb = OA_VERB_TRACE;
  resp.code = "200";
  op.responses = &resp;
  op.n_responses = 1;

  code = gen_body(&op, &spec, "/");
  ASSERT(code);
  ASSERT(strstr(code, "req.method = HTTP_TRACE;") != NULL);

  free(code);
  openapi_spec_free(&spec);
  PASS();
}

TEST test_body_query_verb(void) {
  struct OpenAPI_Operation op;
  struct OpenAPI_Response resp;
  struct OpenAPI_Spec spec;
  char *code;

  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));
  openapi_spec_init(&spec);

  op.verb = OA_VERB_QUERY;
  resp.code = "200";
  op.responses = &resp;
  op.n_responses = 1;

  code = gen_body(&op, &spec, "/");
  ASSERT(code);
  ASSERT(strstr(code, "req.method = HTTP_QUERY;") != NULL);

  free(code);
  openapi_spec_free(&spec);
  PASS();
}

TEST test_body_additional_connect_method(void) {
  struct OpenAPI_Operation op;
  struct OpenAPI_Response resp;
  struct OpenAPI_Spec spec;
  char *code;

  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));
  openapi_spec_init(&spec);

  op.verb = OA_VERB_UNKNOWN;
  op.is_additional = 1;
  op.method = "CONNECT";
  resp.code = "200";
  op.responses = &resp;
  op.n_responses = 1;

  code = gen_body(&op, &spec, "/");
  ASSERT(code);
  ASSERT(strstr(code, "req.method = HTTP_CONNECT;") != NULL);

  free(code);
  openapi_spec_free(&spec);
  PASS();
}

TEST test_body_querystring_param(void) {
  struct OpenAPI_Operation op;
  struct OpenAPI_Response resp;
  struct OpenAPI_Parameter param;
  struct OpenAPI_Spec spec;
  char *code;

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

  code = gen_body(&op, &spec, "/search");
  ASSERT(code);
  ASSERT(strstr(code, "Querystring Parameter") != NULL);
  ASSERT(strstr(code, "asprintf(&query_str") != NULL);

  free(code);
  openapi_spec_free(&spec);
  PASS();
}

TEST test_body_inline_response_string(void) {
  struct OpenAPI_Operation op;
  struct OpenAPI_Response resp;
  struct OpenAPI_Spec spec;
  char *code;

  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));
  openapi_spec_init(&spec);

  op.verb = OA_VERB_GET;
  resp.code = "200";
  resp.schema.inline_type = "string";
  op.responses = &resp;
  op.n_responses = 1;

  code = gen_body(&op, &spec, "/");
  ASSERT(code);
  ASSERT(strstr(code, "json_value_get_string") != NULL);
  ASSERT(strstr(code, "strdup(") != NULL);

  free(code);
  openapi_spec_free(&spec);
  PASS();
}

TEST test_body_inline_response_array_number(void) {
  struct OpenAPI_Operation op;
  struct OpenAPI_Response resp;
  struct OpenAPI_Spec spec;
  char *code;

  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));
  openapi_spec_init(&spec);

  op.verb = OA_VERB_GET;
  resp.code = "200";
  resp.schema.is_array = 1;
  resp.schema.inline_type = "number";
  op.responses = &resp;
  op.n_responses = 1;

  code = gen_body(&op, &spec, "/");
  ASSERT(code);
  ASSERT(strstr(code, "json_array_get_count") != NULL);
  ASSERT(strstr(code, "json_array_get_number") != NULL);

  free(code);
  openapi_spec_free(&spec);
  PASS();
}

TEST test_body_inline_request_body_string(void) {
  struct OpenAPI_Operation op;
  struct OpenAPI_Response resp;
  struct OpenAPI_Spec spec;
  char *code;

  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));
  openapi_spec_init(&spec);

  op.verb = OA_VERB_POST;
  op.req_body.content_type = "application/json";
  op.req_body.inline_type = "string";
  resp.code = "200";
  op.responses = &resp;
  op.n_responses = 1;

  code = gen_body(&op, &spec, "/");
  ASSERT(code);
  ASSERT(strstr(code, "json_value_init_string") != NULL);
  ASSERT(strstr(code, "json_serialize_to_string") != NULL);
  ASSERT(strstr(code, "Content-Type\", \"application/json\"") != NULL);

  free(code);
  openapi_spec_free(&spec);
  PASS();
}

TEST test_body_inline_request_body_array(void) {
  struct OpenAPI_Operation op;
  struct OpenAPI_Response resp;
  struct OpenAPI_Spec spec;
  char *code;

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

  code = gen_body(&op, &spec, "/");
  ASSERT(code);
  ASSERT(strstr(code, "json_value_init_array") != NULL);
  ASSERT(strstr(code, "json_array_append_number") != NULL);

  free(code);
  openapi_spec_free(&spec);
  PASS();
}

TEST test_body_header_array_param(void) {
  struct OpenAPI_Operation op;
  struct OpenAPI_Response resp;
  struct OpenAPI_Parameter param;
  struct OpenAPI_Spec spec;
  char *code;

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

  code = gen_body(&op, &spec, "/");
  ASSERT(code);
  ASSERT(strstr(code, "Header Parameter: X-Ids") != NULL);
  ASSERT(strstr(code, "http_headers_add(&req.headers, \"X-Ids\", joined)") !=
         NULL);
  ASSERT(strstr(code, "joined_len") != NULL);

  free(code);
  openapi_spec_free(&spec);
  PASS();
}

TEST test_body_header_object_param(void) {
  struct OpenAPI_Operation op;
  struct OpenAPI_Response resp;
  struct OpenAPI_Parameter param;
  struct OpenAPI_Spec spec;
  char *code;

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

  code = gen_body(&op, &spec, "/");
  ASSERT(code);
  ASSERT(strstr(code, "Header Parameter: X-Filter") != NULL);
  ASSERT(strstr(code, "const struct OpenAPI_KV *kv = &X-Filter[i]") != NULL);
  ASSERT(strstr(code, "joined[joined_len++] = '='") != NULL);
  ASSERT(strstr(code, "http_headers_add(&req.headers, \"X-Filter\", joined)") !=
         NULL);

  free(code);
  openapi_spec_free(&spec);
  PASS();
}

TEST test_body_header_number_param(void) {
  struct OpenAPI_Operation op;
  struct OpenAPI_Response resp;
  struct OpenAPI_Parameter param;
  struct OpenAPI_Spec spec;
  char *code;

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

  code = gen_body(&op, &spec, "/");
  ASSERT(code);
  ASSERT(strstr(code, "Header Parameter: X-Rate") != NULL);
  ASSERT(strstr(code, "sprintf(num_buf, \"%g\", X-Rate)") != NULL);
  ASSERT(strstr(code, "http_headers_add(&req.headers, \"X-Rate\", num_buf)") !=
         NULL);

  free(code);
  openapi_spec_free(&spec);
  PASS();
}

TEST test_body_cookie_param(void) {
  struct OpenAPI_Operation op;
  struct OpenAPI_Response resp;
  struct OpenAPI_Parameter param;
  struct OpenAPI_Spec spec;
  char *code;

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

  code = gen_body(&op, &spec, "/");
  ASSERT(code);
  ASSERT(strstr(code, "Cookie Parameters") != NULL);
  ASSERT(
      strstr(code, "http_headers_add(&req.headers, \"Cookie\", cookie_str)") !=
      NULL);

  free(code);
  openapi_spec_free(&spec);
  PASS();
}

TEST test_body_cookie_param_number_array(void) {
  struct OpenAPI_Operation op;
  struct OpenAPI_Response resp;
  struct OpenAPI_Parameter param;
  struct OpenAPI_Spec spec;
  char *code;

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

  code = gen_body(&op, &spec, "/");
  ASSERT(code);
  ASSERT(strstr(code, "Cookie Parameters") != NULL);
  ASSERT(strstr(code, "sprintf(num_buf, \"%g\", weights[i])") != NULL);
  ASSERT(
      strstr(code, "http_headers_add(&req.headers, \"Cookie\", cookie_str)") !=
      NULL);

  free(code);
  openapi_spec_free(&spec);
  PASS();
}

TEST test_body_cookie_param_array_explode_false(void) {
  struct OpenAPI_Operation op;
  struct OpenAPI_Response resp;
  struct OpenAPI_Parameter param;
  struct OpenAPI_Spec spec;
  char *code;

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

  code = gen_body(&op, &spec, "/");
  ASSERT(code);
  ASSERT(strstr(code, "joined_len") != NULL);
  ASSERT(strstr(code, "joined[joined_len++] = ','") != NULL);
  ASSERT(
      strstr(code, "http_headers_add(&req.headers, \"Cookie\", cookie_str)") !=
      NULL);

  free(code);
  openapi_spec_free(&spec);
  PASS();
}

TEST test_body_security_query_api_key(void) {
  struct OpenAPI_Operation op;
  struct OpenAPI_Response resp;
  struct OpenAPI_Spec spec;
  struct OpenAPI_SecurityScheme scheme;
  char *code;

  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));
  memset(&spec, 0, sizeof(spec));
  memset(&scheme, 0, sizeof(scheme));

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

  code = gen_body(&op, &spec, "/");
  ASSERT(code);
  ASSERT(strstr(code, "struct UrlQueryParams qp") != NULL);
  ASSERT(strstr(code, "url_query_add(&qp, \"api_key\"") != NULL);

  free(code);
  openapi_spec_free(&spec);
  PASS();
}

TEST test_body_security_cookie_api_key(void) {
  struct OpenAPI_Operation op;
  struct OpenAPI_Response resp;
  struct OpenAPI_Spec spec;
  struct OpenAPI_SecurityScheme scheme;
  char *code;

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

  code = gen_body(&op, &spec, "/");
  ASSERT(code);
  ASSERT(strstr(code, "cookie_str") != NULL);
  ASSERT(strstr(code, "session_id") != NULL);

  free(code);
  openapi_spec_free(&spec);
  PASS();
}

TEST test_body_form_urlencoded(void) {
  struct OpenAPI_Operation op;
  struct OpenAPI_Response resp;
  struct OpenAPI_Spec spec;
  char *schema_name = "FormData";
  char *code;

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

  code = gen_body(&op, &spec, "/submit");
  ASSERT(code);
  ASSERT(strstr(code, "Form URL-Encoded Body Construction") != NULL);
  ASSERT(strstr(code, "url_query_build_form(&form_qp, &form_body)") != NULL);
  ASSERT(strstr(code, "\"application/x-www-form-urlencoded\"") != NULL);
  ASSERT(strstr(code, "url_query_add(&form_qp, \"name\"") != NULL);
  ASSERT(strstr(code, "sprintf(num_buf, \"%d\", req_body->age)") != NULL);

  free(code);
  openapi_spec_free(&spec);
  PASS();
}

TEST test_body_multipart_primitives_and_arrays(void) {
  struct OpenAPI_Operation op;
  struct OpenAPI_Response resp;
  struct OpenAPI_Spec spec;
  char *schema_name = "Upload";
  char *code;

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

  code = gen_body(&op, &spec, "/upload");
  ASSERT(code);
  ASSERT(strstr(code, "Multipart Body Construction") != NULL);
  ASSERT(strstr(code, "http_request_add_part(&req, \"title\"") != NULL);
  ASSERT(strstr(code, "sprintf(num_buf, \"%g\", req_body->ratio)") != NULL);
  ASSERT(strstr(code, "req_body->flag ? \"true\" : \"false\"") != NULL);
  ASSERT(strstr(code, "for (i = 0; i < req_body->n_tags; ++i)") != NULL);
  ASSERT(strstr(code, "http_request_add_part(&req, \"tags\"") != NULL);
  ASSERT(strstr(code, "for (i = 0; i < req_body->n_nums; ++i)") != NULL);

  free(code);
  openapi_spec_free(&spec);
  PASS();
}

TEST test_body_response_range_success(void) {
  struct OpenAPI_Operation op;
  struct OpenAPI_Response resp;
  struct OpenAPI_Spec spec;
  char *code;

  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));
  openapi_spec_init(&spec);

  op.verb = OA_VERB_GET;
  resp.code = "2XX";
  resp.schema.ref_name = "Pet";
  op.responses = &resp;
  op.n_responses = 1;

  code = gen_body(&op, &spec, "/");
  ASSERT(code);
  ASSERT(strstr(code, "status_code >= 200") != NULL);
  ASSERT(strstr(code, "Pet_from_json") != NULL);

  free(code);
  openapi_spec_free(&spec);
  PASS();
}

TEST test_body_default_response_success(void) {
  struct OpenAPI_Operation op;
  struct OpenAPI_Response resp;
  struct OpenAPI_Spec spec;
  char *code;

  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));
  openapi_spec_init(&spec);

  op.verb = OA_VERB_GET;
  resp.code = "default";
  resp.schema.ref_name = "Pet";
  op.responses = &resp;
  op.n_responses = 1;

  code = gen_body(&op, &spec, "/");
  ASSERT(code);
  ASSERT(strstr(code, "default response") != NULL);
  ASSERT(strstr(code, "Pet_from_json") != NULL);

  free(code);
  openapi_spec_free(&spec);
  PASS();
}

SUITE(client_body_suite) {
  RUN_TEST(test_body_basic_get);
  RUN_TEST(test_body_options_verb);
  RUN_TEST(test_body_trace_verb);
  RUN_TEST(test_body_query_verb);
  RUN_TEST(test_body_additional_connect_method);
  RUN_TEST(test_body_querystring_param);
  RUN_TEST(test_body_inline_response_string);
  RUN_TEST(test_body_inline_response_array_number);
  RUN_TEST(test_body_inline_request_body_string);
  RUN_TEST(test_body_inline_request_body_array);
  RUN_TEST(test_body_header_array_param);
  RUN_TEST(test_body_header_object_param);
  RUN_TEST(test_body_header_number_param);
  RUN_TEST(test_body_cookie_param);
  RUN_TEST(test_body_cookie_param_number_array);
  RUN_TEST(test_body_cookie_param_array_explode_false);
  RUN_TEST(test_body_security_query_api_key);
  RUN_TEST(test_body_security_cookie_api_key);
  RUN_TEST(test_body_form_urlencoded);
  RUN_TEST(test_body_multipart_primitives_and_arrays);
  RUN_TEST(test_body_response_range_success);
  RUN_TEST(test_body_default_response_success);
}

#endif /* TEST_CODEGEN_CLIENT_BODY_H */
