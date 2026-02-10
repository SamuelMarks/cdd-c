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

SUITE(client_body_suite) {
  RUN_TEST(test_body_basic_get);
  RUN_TEST(test_body_options_verb);
  RUN_TEST(test_body_trace_verb);
  RUN_TEST(test_body_query_verb);
  RUN_TEST(test_body_querystring_param);
}

#endif /* TEST_CODEGEN_CLIENT_BODY_H */
