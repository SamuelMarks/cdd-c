/**
 * @file test_codegen_client_sig.h
 * @brief Unit tests for C Client Signature Generation.
 */

#ifndef TEST_CODEGEN_CLIENT_SIG_H
#define TEST_CODEGEN_CLIENT_SIG_H

#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "codegen_client_sig.h"
#include "openapi_loader.h"

static char *gen_sig(const struct OpenAPI_Operation *op,
                     const struct CodegenSigConfig *cfg) {
  FILE *tmp = tmpfile();
  long sz;
  char *content;

  if (!tmp)
    return NULL;

  if (codegen_client_write_signature(tmp, op, cfg) != 0) {
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

TEST test_sig_simple_get(void) {
  struct OpenAPI_Operation op;
  struct OpenAPI_Parameter param;
  char *code;

  memset(&op, 0, sizeof(op));
  op.operation_id = "get_pet";

  param.name = "id";
  param.type = "integer";
  op.parameters = &param;
  op.n_parameters = 1;

  op.req_body.ref_name = "Pet";

  code = gen_sig(&op, NULL);
  ASSERT(code);

  /* Verify standard signature including ApiError */
  ASSERT(strstr(code,
                "int get_pet(struct HttpClient *ctx, int id, struct Pet **out, "
                "struct ApiError **api_error) {"));

  free(code);
  PASS();
}

TEST test_sig_verify_apierror(void) {
  struct OpenAPI_Operation op;
  char *code;
  memset(&op, 0, sizeof(op));
  op.operation_id = "do";

  code = gen_sig(&op, NULL);
  ASSERT(code);

  ASSERT(strstr(code, ", struct ApiError **api_error)"));

  free(code);
  PASS();
}

TEST test_sig_grouped(void) {
  struct OpenAPI_Operation op;
  struct CodegenSigConfig cfg;
  char *code;

  memset(&op, 0, sizeof(op));
  op.operation_id = "getById";

  memset(&cfg, 0, sizeof(cfg));
  cfg.prefix = "api_";
  cfg.group_name = "Pet";

  code = gen_sig(&op, &cfg);
  ASSERT(code);

  /* Expect: Pet_api_getById */
  ASSERT(strstr(code, "int Pet_api_getById(struct HttpClient *ctx"));

  free(code);
  PASS();
}

TEST test_sig_success_range_response(void) {
  struct OpenAPI_Operation op;
  struct OpenAPI_Response resp;
  char *code;

  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));
  op.operation_id = "listPets";

  resp.code = "2XX";
  resp.schema.ref_name = "Pet";
  op.responses = &resp;
  op.n_responses = 1;

  code = gen_sig(&op, NULL);
  ASSERT(code);
  ASSERT(strstr(code, "struct Pet **out") != NULL);

  free(code);
  PASS();
}

TEST test_sig_default_response_success(void) {
  struct OpenAPI_Operation op;
  struct OpenAPI_Response resp;
  char *code;

  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));
  op.operation_id = "defaultPet";

  resp.code = "default";
  resp.schema.ref_name = "Pet";
  op.responses = &resp;
  op.n_responses = 1;

  code = gen_sig(&op, NULL);
  ASSERT(code);
  ASSERT(strstr(code, "struct Pet **out") != NULL);

  free(code);
  PASS();
}

TEST test_sig_inline_response_string(void) {
  struct OpenAPI_Operation op;
  struct OpenAPI_Response resp;
  char *code;

  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));
  op.operation_id = "getInline";

  resp.code = "200";
  resp.schema.inline_type = "string";
  op.responses = &resp;
  op.n_responses = 1;

  code = gen_sig(&op, NULL);
  ASSERT(code);
  ASSERT(strstr(code, "char **out") != NULL);

  free(code);
  PASS();
}

TEST test_sig_inline_response_array(void) {
  struct OpenAPI_Operation op;
  struct OpenAPI_Response resp;
  char *code;

  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));
  op.operation_id = "getInlineArr";

  resp.code = "200";
  resp.schema.is_array = 1;
  resp.schema.inline_type = "integer";
  op.responses = &resp;
  op.n_responses = 1;

  code = gen_sig(&op, NULL);
  ASSERT(code);
  ASSERT(strstr(code, "int **out, size_t *out_len") != NULL);

  free(code);
  PASS();
}

TEST test_sig_inline_request_body_string(void) {
  struct OpenAPI_Operation op;
  char *code;

  memset(&op, 0, sizeof(op));
  op.operation_id = "postInline";
  op.req_body.content_type = "application/json";
  op.req_body.inline_type = "string";

  code = gen_sig(&op, NULL);
  ASSERT(code);
  ASSERT(strstr(code, "const char *req_body") != NULL);

  free(code);
  PASS();
}

TEST test_sig_inline_request_body_array(void) {
  struct OpenAPI_Operation op;
  char *code;

  memset(&op, 0, sizeof(op));
  op.operation_id = "postInlineArr";
  op.req_body.content_type = "application/json";
  op.req_body.is_array = 1;
  op.req_body.inline_type = "number";

  code = gen_sig(&op, NULL);
  ASSERT(code);
  ASSERT(strstr(code, "const double *body, size_t body_len") != NULL);

  free(code);
  PASS();
}

TEST test_sig_query_object_param_kv(void) {
  struct OpenAPI_Operation op;
  struct OpenAPI_Parameter param;
  char *code;

  memset(&op, 0, sizeof(op));
  memset(&param, 0, sizeof(param));
  op.operation_id = "list";

  param.name = "filter";
  param.type = "object";
  param.in = OA_PARAM_IN_QUERY;

  op.parameters = &param;
  op.n_parameters = 1;

  code = gen_sig(&op, NULL);
  ASSERT(code);
  ASSERT(strstr(code, "const struct OpenAPI_KV *filter, size_t filter_len") !=
         NULL);

  free(code);
  PASS();
}

TEST test_sig_path_object_param_kv(void) {
  struct OpenAPI_Operation op;
  struct OpenAPI_Parameter param;
  char *code;

  memset(&op, 0, sizeof(op));
  memset(&param, 0, sizeof(param));
  op.operation_id = "byPath";

  param.name = "filter";
  param.type = "object";
  param.in = OA_PARAM_IN_PATH;

  op.parameters = &param;
  op.n_parameters = 1;

  code = gen_sig(&op, NULL);
  ASSERT(code);
  ASSERT(strstr(code, "const struct OpenAPI_KV *filter, size_t filter_len") !=
         NULL);

  free(code);
  PASS();
}

TEST test_sig_header_object_param_kv(void) {
  struct OpenAPI_Operation op;
  struct OpenAPI_Parameter param;
  char *code;

  memset(&op, 0, sizeof(op));
  memset(&param, 0, sizeof(param));
  op.operation_id = "byHeader";

  param.name = "filter";
  param.type = "object";
  param.in = OA_PARAM_IN_HEADER;

  op.parameters = &param;
  op.n_parameters = 1;

  code = gen_sig(&op, NULL);
  ASSERT(code);
  ASSERT(strstr(code, "const struct OpenAPI_KV *filter, size_t filter_len") !=
         NULL);

  free(code);
  PASS();
}

SUITE(client_sig_suite) {
  RUN_TEST(test_sig_simple_get);
  RUN_TEST(test_sig_verify_apierror);
  RUN_TEST(test_sig_grouped);
  RUN_TEST(test_sig_success_range_response);
  RUN_TEST(test_sig_default_response_success);
  RUN_TEST(test_sig_inline_response_string);
  RUN_TEST(test_sig_inline_response_array);
  RUN_TEST(test_sig_inline_request_body_string);
  RUN_TEST(test_sig_inline_request_body_array);
  RUN_TEST(test_sig_query_object_param_kv);
  RUN_TEST(test_sig_path_object_param_kv);
  RUN_TEST(test_sig_header_object_param_kv);
}

#endif /* TEST_CODEGEN_CLIENT_SIG_H */
