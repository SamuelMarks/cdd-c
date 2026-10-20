#include "cdd_test_helpers_export.h"
CDD_TEST_HELPERS_EXPORT FILE *cdd_test_tmpfile_global(void);
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
#include "c_cdd_export.h"
#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "openapi/parse/openapi.h"
#include "c_cdd/memory.h"
#include "tests/emit/sdk_tests.h"
/* clang-format on */

/**
 * @brief Unit test test_gen_sdk_test_basic.
 * @return Greatest test result.
 */
TEST test_gen_sdk_test_basic(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Path path;
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Parameter param = {0};
  struct SdkTestsConfig config;
  FILE *tmp;
#if defined(_MSC_VER)
  if (((tmp = cdd_test_tmpfile_global()) == NULL))
    tmp = NULL;
#else
  tmp = cdd_test_tmpfile_global();
#endif
  {
    long sz;
    char *content = NULL;

    ASSERT(tmp != NULL);

    /* Setup Spec */
    memset(&spec, 0, sizeof(spec));
    memset(&path, 0, sizeof(path));
    memset(&op, 0, sizeof(op));
    memset(&param, 0, sizeof(param));

    spec.paths = &path;
    spec.n_paths = 1;

    path.operations = &op;
    path.n_operations = 1;
    path.route = cdd_test_mut_str("/api/test");

    op.operation_id = cdd_test_mut_str("runOp");
    op.parameters = &param;
    op.n_parameters = 1;

    param.name = cdd_test_mut_str("count");
    param.type = cdd_test_mut_str("integer");
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
    content = (char *)C_CDD_CALLOC(1, (size_t)sz + 1);
    if (fread(content, 1, (size_t)sz, tmp)) {
    }

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

    C_CDD_FREE(content);
    if (tmp)
      fclose(tmp);
    g_fail_io_after = -1;
    PASS();
  }
}

/**
 * @brief Unit test test_gen_sdk_test_nulls.
 * @return Greatest test result.
 */
TEST test_gen_sdk_test_nulls(void) {
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            codegen_sdk_tests_generate(NULL, NULL, NULL));
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_gen_sdk_test_exhaustive.
 * @return Greatest test result.
 */
TEST test_gen_sdk_test_exhaustive(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Path path;
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Parameter param[3];
  struct SdkTestsConfig config;
  struct OpenAPI_Response responses[2];
  FILE *tmp;
#if defined(_MSC_VER)
  if (((tmp = cdd_test_tmpfile_global()) == NULL))
    tmp = NULL;
#else
  tmp = cdd_test_tmpfile_global();
#endif

  memset(&spec, 0, sizeof(spec));
  memset(&path, 0, sizeof(path));
  memset(&op, 0, sizeof(op));
  memset(param, 0, sizeof(param));
  memset(responses, 0, sizeof(responses));

  spec.paths = &path;
  spec.n_paths = 1;

  path.operations = &op;
  path.n_operations = 1;
  path.route = cdd_test_mut_str("/api/test");

  op.operation_id = cdd_test_mut_str("runExhaustive");
  op.parameters = param;
  op.n_parameters = 3;

  param[0].name = cdd_test_mut_str("count");
  param[0].type = cdd_test_mut_str("integer");
  param[1].name = cdd_test_mut_str("flag");
  param[1].type = cdd_test_mut_str("boolean");
  param[1].is_array = 1;
  param[2].name = cdd_test_mut_str("str");
  param[2].type = cdd_test_mut_str("string");

  op.req_body.ref_name = cdd_test_mut_str("MyRequestBody");
  op.req_body.is_array = 0;
  {
    char *tags[1];

    tags[0] = cdd_test_mut_str("auth");
    op.n_tags = 1;
    op.tags = (char **)(size_t)tags;

    op.n_responses = 2;
    op.responses = responses;

    responses[0].code = cdd_test_mut_str("400");
    responses[0].schema.ref_name = cdd_test_mut_str("MyError");
    responses[1].code = cdd_test_mut_str("200");
    responses[1].schema.ref_name = cdd_test_mut_str("MyResponse");

    config.client_header = "client.h";
    config.func_prefix = "api_";
    config.mock_server_url = "http://loopback";

    ASSERT_EQ(0, codegen_sdk_tests_generate(tmp, &spec, &config));

    g_io_calls = 0;
    g_fail_io_after = 2;
    ASSERT_EQ(CDD_C_ERROR_IO, codegen_sdk_tests_generate(tmp, &spec, &config));
    g_fail_io_after = -1;

    /* Also test array request body */
    op.req_body.is_array = 1;
    ASSERT_EQ(0, codegen_sdk_tests_generate(tmp, &spec, &config));

    if (tmp)
      fclose(tmp);
    g_fail_io_after = -1;
    PASS();
  }
}

/**
 * @brief Executes the codegen_sdk_tests_suite test suite.
 */
SUITE(codegen_sdk_tests_suite) {
  RUN_TEST(test_gen_sdk_test_basic);
  RUN_TEST(test_gen_sdk_test_nulls);
  RUN_TEST(test_gen_sdk_test_exhaustive);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CODEGEN_SDK_TESTS_H */
