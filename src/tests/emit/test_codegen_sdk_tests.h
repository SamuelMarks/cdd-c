/**
 * @file test_codegen_sdk_tests.h
 * @brief Unit tests for SDK Test Generator.
 */

#ifndef TEST_CODEGEN_SDK_TESTS_H
#define TEST_CODEGEN_SDK_TESTS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "openapi/parse/openapi.h"
#include "tests/emit/sdk_tests.h"
/* clang-format on */

TEST test_gen_sdk_test_basic(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Path path;
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Parameter param = {0};
  struct SdkTestsConfig config;
  FILE *tmp = tmpfile();
  long sz;
  char *content = NULL;

  if (!tmp)
    return -1;

  /* Setup Spec */
  memset(&spec, 0, sizeof(spec));
  memset(&path, 0, sizeof(path));
  memset(&op, 0, sizeof(op));
  memset(&param, 0, sizeof(param));

  spec.paths = &path;
  spec.n_paths = 1;

  path.operations = &op;
  path.n_operations = 1;
  path.route = "/api/test";

  op.operation_id = "runOp";
  op.parameters = &param;
  op.n_parameters = 1;

  param.name = "count";
  param.type = "integer";
  param.in = OA_PARAM_IN_QUERY;

  /* Config */
  config.client_header = "client.h";
  config.func_prefix = "api_";
  config.mock_server_url = "http://loopback";

  /* Generate */
  ASSERT_EQ(0, codegen_sdk_tests_generate(tmp, &spec, &config));

  /* Verify */
  fseek(tmp, 0, SEEK_END);
  sz = ftell(tmp);
  rewind(tmp);
  content = (char *)calloc(1, sz + 1);
  fread(content, 1, sz, tmp);

  /* Preamble check */
  ASSERT(strstr(content, "#include \"client.h\""));
  ASSERT(strstr(content, "GREATEST_MAIN_DEFS"));

  /* Operation check */
  ASSERT(strstr(content, "TEST test_runOp(void)"));
  ASSERT(strstr(content, "api_init(&client, \"http://loopback\")"));
  ASSERT(strstr(content, "api_runOp(&client"));
  /* Parameter check */
  ASSERT(strstr(content, "const int count = 1;"));
  /* Cleanup */
  ASSERT(strstr(content, "api_cleanup(&client)"));
  /* Runner check */
  ASSERT(strstr(content, "RUN_TEST(test_runOp)"));

  free(content);
  fclose(tmp);
  PASS();
}

TEST test_gen_sdk_test_nulls(void) {
  ASSERT_EQ(EINVAL, codegen_sdk_tests_generate(NULL, NULL, NULL));
  PASS();
}

TEST test_gen_sdk_test_exhaustive(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Path path;
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Parameter param[3];
  struct SdkTestsConfig config;
  struct OpenAPI_Response responses[2];
  FILE *tmp = tmpfile();

  memset(&spec, 0, sizeof(spec));
  memset(&path, 0, sizeof(path));
  memset(&op, 0, sizeof(op));
  memset(param, 0, sizeof(param));
  memset(responses, 0, sizeof(responses));

  spec.paths = &path;
  spec.n_paths = 1;

  path.operations = &op;
  path.n_operations = 1;
  path.route = "/api/test";

  op.operation_id = "runExhaustive";
  op.parameters = param;
  op.n_parameters = 3;

  param[0].name = "count";
  param[0].type = "integer";
  param[1].name = "flag";
  param[1].type = "boolean";
  param[1].is_array = 1;
  param[2].name = "str";
  param[2].type = "string";

  op.req_body.ref_name = "MyRequestBody";
  op.req_body.is_array = 0;

  op.n_responses = 2;
  op.responses = responses;
  responses[0].code = "200";
  responses[0].schema.ref_name = "MyResponse";
  responses[1].code = "400";
  responses[1].schema.ref_name = "MyError";

  config.client_header = "client.h";
  config.func_prefix = "api_";
  config.mock_server_url = "http://loopback";

  ASSERT_EQ(0, codegen_sdk_tests_generate(tmp, &spec, &config));

  /* Also test array request body */
  op.req_body.is_array = 1;
  ASSERT_EQ(0, codegen_sdk_tests_generate(tmp, &spec, &config));

  fclose(tmp);
  PASS();
}

SUITE(codegen_sdk_tests_suite) {
  RUN_TEST(test_gen_sdk_test_basic);
  RUN_TEST(test_gen_sdk_test_nulls);
  RUN_TEST(test_gen_sdk_test_exhaustive);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CODEGEN_SDK_TESTS_H */
