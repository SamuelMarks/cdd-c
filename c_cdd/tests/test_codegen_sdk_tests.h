/**
 * @file test_codegen_sdk_tests.h
 * @brief Unit tests for SDK Test Generator.
 * @author Samuel Marks
 */

#ifndef TEST_CODEGEN_SDK_TESTS_H
#define TEST_CODEGEN_SDK_TESTS_H

#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "codegen_sdk_tests.h"
#include "openapi_loader.h"

TEST test_gen_sdk_test_basic(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Path path;
  struct OpenAPI_Operation op;
  struct OpenAPI_Parameter param;
  struct SdkTestsConfig config;
  FILE *tmp = tmpfile();
  long sz;
  char *content;

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
  content = calloc(1, sz + 1);
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

SUITE(codegen_sdk_tests_suite) {
  RUN_TEST(test_gen_sdk_test_basic);
  RUN_TEST(test_gen_sdk_test_nulls);
}

#endif /* TEST_CODEGEN_SDK_TESTS_H */
