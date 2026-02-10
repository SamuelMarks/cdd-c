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

SUITE(client_sig_suite) {
  RUN_TEST(test_sig_simple_get);
  RUN_TEST(test_sig_verify_apierror);
  RUN_TEST(test_sig_grouped);
}

#endif /* TEST_CODEGEN_CLIENT_SIG_H */
