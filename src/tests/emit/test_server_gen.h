/**
 * @file test_server_gen.h
 * @brief Unit tests and mocks in test_server_gen.h.
 */
#ifndef TEST_SERVER_GEN_H
#define TEST_SERVER_GEN_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#if defined(_WIN32)
#include <direct.h>
#endif

#include "cdd_test_helpers/cdd_helpers.h"
#include "routes/emit/client_gen.h"
#include "routes/emit/server_gen.h"
/* clang-format on */

/** @brief Test definition g_fail_io_after. */
extern C_CDD_EXPORT int g_fail_io_after;
/** @brief Test definition g_io_calls. */
extern C_CDD_EXPORT int g_io_calls;

/**
 * @brief test_server_gen_basic
 * @return TEST
 */
TEST test_server_gen_basic(void) {
  struct OpenAPI_Spec spec;
  struct OpenApiClientConfig config;
  int rc = 0;
  FILE *f;

  memset(&spec, 0, sizeof(spec));
  spec.n_paths = 1;
  spec.paths =
      (struct OpenAPI_Path *)C_CDD_CALLOC(1, sizeof(struct OpenAPI_Path));
  spec.paths[0].route = cdd_test_mut_str("/test/route");

  spec.paths[0].n_operations = 9;
  spec.paths[0].operations = (struct OpenAPI_Operation *)C_CDD_CALLOC(
      9, sizeof(struct OpenAPI_Operation));

  spec.paths[0].operations[0].verb = OA_VERB_GET;
  spec.paths[0].operations[0].operation_id = cdd_test_mut_str("doGet");

  spec.paths[0].operations[1].verb = OA_VERB_POST;
  spec.paths[0].operations[1].operation_id = cdd_test_mut_str("doPost");

  spec.paths[0].operations[2].verb = OA_VERB_PUT;
  spec.paths[0].operations[2].operation_id = cdd_test_mut_str("doPut");

  spec.paths[0].operations[3].verb = OA_VERB_DELETE;
  spec.paths[0].operations[3].operation_id = cdd_test_mut_str("doDelete");

  spec.paths[0].operations[4].verb = OA_VERB_OPTIONS;
  spec.paths[0].operations[4].operation_id = cdd_test_mut_str("doOptions");

  spec.paths[0].operations[5].verb = OA_VERB_HEAD;
  spec.paths[0].operations[5].operation_id = cdd_test_mut_str("doHead");

  spec.paths[0].operations[6].verb = OA_VERB_PATCH;
  spec.paths[0].operations[6].operation_id = cdd_test_mut_str("doPatch");

  spec.paths[0].operations[7].verb = OA_VERB_TRACE;
  spec.paths[0].operations[7].operation_id = cdd_test_mut_str("doTrace");

  spec.paths[0].operations[8].verb =
      (enum OpenAPI_Verb)999; /* Unknown verb triggers default branch */
  spec.paths[0].operations[8].operation_id = cdd_test_mut_str("doUnknown");
  spec.info.description = cdd_test_mut_str("Test Desc");
  spec.info.contact.name = cdd_test_mut_str("Test Contact");
  spec.info.license.name = cdd_test_mut_str("Test License");
  spec.n_servers = 1;
  spec.servers =
      (struct OpenAPI_Server *)C_CDD_CALLOC(1, sizeof(struct OpenAPI_Server));
  spec.servers[0].url = cdd_test_mut_str("http://test");

  spec.paths[0].operations[0].description = cdd_test_mut_str("Test Op Desc");
  spec.paths[0].operations[0].n_parameters = 1;
  spec.paths[0].operations[0].parameters =
      (struct OpenAPI_Parameter *)C_CDD_CALLOC(
          1, sizeof(struct OpenAPI_Parameter));
  spec.paths[0].operations[0].parameters[0].name = cdd_test_mut_str("param1");
  spec.paths[0].operations[0].parameters[0].in = OA_PARAM_IN_QUERY;
  spec.paths[0].operations[0].parameters[0].description =
      cdd_test_mut_str("Test Param Desc");

  spec.paths[0].operations[0].deprecated = 1;
  spec.paths[0].operations[0].req_body.content_schema =
      (struct OpenAPI_SchemaRef *)C_CDD_CALLOC(
          1, sizeof(struct OpenAPI_SchemaRef));
  spec.paths[0].operations[0].req_body.ref = cdd_test_mut_str("TestRef");

  spec.paths[0].operations[1].req_body.ref_name =
      cdd_test_mut_str("TestRefName");

  spec.paths[0].operations[0].n_responses = 1;
  spec.paths[0].operations[0].n_callbacks = 1;
  spec.paths[0].operations[0].security =
      (struct OpenAPI_SecurityRequirementSet *)C_CDD_CALLOC(
          1, sizeof(struct OpenAPI_SecurityRequirementSet));

  spec.paths[0].operations[1].n_req_body_media_types = 2;
  spec.paths[0].operations[1].req_body_media_types =
      (struct OpenAPI_MediaType *)C_CDD_CALLOC(
          2, sizeof(struct OpenAPI_MediaType));
  spec.paths[0].operations[1].req_body_media_types[0].name =
      cdd_test_mut_str("application/json");
  spec.paths[0].operations[1].req_body_media_types[1].name =
      cdd_test_mut_str("application/x-www-form-urlencoded");

  memset(&config, 0, sizeof(config));
  config.filename_base = cdd_test_mut_str("test_server");

  rc = openapi_server_generate(&spec, &config);
  ASSERT_EQ(0, rc);

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  if (fopen_s(&f, "src/test_server_server.c", "r") != 0)
    f = NULL;
#elif defined(_MSC_VER)
  fopen_s(&f, "src/test_server_server.c", "r");
#else
#if defined(_MSC_VER)
  if (fopen_s(&f, "src/test_server_server.c", "r") != 0)
    f = NULL;
#else
  f = fopen("src/test_server_server.c", "r");
#endif
#endif

  ASSERT(f != NULL);
  if (f) {
    char buf[16384];
    size_t n = fread(buf, 1, sizeof(buf) - 1, f);
    buf[n] = '\0';
    ASSERT(strstr(buf, "handle_mcp_sse") != NULL);
    ASSERT(strstr(buf, "handle_mcp_message") != NULL);
    ASSERT(strstr(buf, "resolve_daos") != NULL);
    ASSERT(strstr(buf, "seed_database") != NULL);
    ASSERT(strstr(buf, "cors_middleware") != NULL);
    ASSERT(strstr(buf, "strict_validation_middleware") != NULL);
    ASSERT(strstr(buf, "auth_middleware") != NULL);
    ASSERT(strstr(buf, "handle_auth_idp") != NULL);
    ASSERT(strstr(buf, "handle_mock_trigger_webhook") != NULL);
    ASSERT(strstr(buf, "--ephemeral") != NULL);
    ASSERT(strstr(buf, "--seed") != NULL);
    if (f)
      fclose(f);
  }

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  if (fopen_s(&f, "test_test_server_server.c", "r") != 0)
    f = NULL;
#elif defined(_MSC_VER)
  fopen_s(&f, "test_test_server_server.c", "r");
#else
#if defined(_MSC_VER)
  if (fopen_s(&f, "test_test_server_server.c", "r") != 0)
    f = NULL;
#else
  f = fopen("test_test_server_server.c", "r");
#endif
#endif
  ASSERT(f != NULL);
  if (f) {
    char buf[16384];
    size_t n = fread(buf, 1, sizeof(buf) - 1, f);
    buf[n] = '\0';
    ASSERT(strstr(buf, "ASSERT_EQ(CDD_C_SUCCESS, status);") != NULL);
    ASSERT(strstr(buf, "SUITE(server_endpoints_suite) {") != NULL);
    if (f)
      fclose(f);
  }

  remove("src/test_server_server.c");
  remove("test_test_server_server.c");
  C_CDD_FREE(spec.paths[0].operations[1].req_body_media_types);

  C_CDD_FREE(spec.servers);
  C_CDD_FREE(spec.paths[0].operations[0].parameters);
  C_CDD_FREE(spec.paths[0].operations[0].req_body.content_schema);
  C_CDD_FREE(spec.paths[0].operations[0].security);
  C_CDD_FREE(spec.paths[0].operations);

  C_CDD_FREE(spec.paths);
  g_fail_io_after = -1;

  g_cdd_alloc_fail = 1;
  config.filename_base = cdd_test_mut_str("test_server");
  rc = openapi_server_generate(&spec, &config);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  g_cdd_alloc_fail = 0;

  PASS();
}

/**
 * @brief test_server_gen_fail_open
 * @return TEST
 */
TEST test_server_gen_fail_open(void) {
  struct OpenAPI_Spec spec;
  struct OpenApiClientConfig config;
  int rc = 0;

  memset(&spec, 0, sizeof(spec));
  memset(&config, 0, sizeof(config));
  config.filename_base = cdd_test_mut_str("/nonexistent/dir/test_server");
  g_io_calls = 0;
  g_fail_io_after = 1;

  rc = openapi_server_generate(&spec, &config);
  ASSERT(rc == CDD_C_ERROR_IO || rc == CDD_C_ERROR_NOT_FOUND);
  g_fail_io_after = -1;

  /* Test fopen failures */
  makedirs("test_build_dir/bad_server/src/bad_server_server.c");
  config.filename_base =
      cdd_test_mut_str("test_build_dir/bad_server/bad_server");
  rc = openapi_server_generate(&spec, &config);
  ASSERT_EQ(CDD_C_ERROR_IO, rc);
  remove("test_build_dir/bad_server/src/bad_server_server.c");

  config.filename_base = cdd_test_mut_str("test_build_dir/test_server");
  {
    int i;
    for (i = 1; i <= 5; ++i) {
      g_cdd_alloc_fail = i;
      rc = openapi_server_generate(&spec, &config);
      g_cdd_alloc_fail = 0;
      if (rc == CDD_C_SUCCESS)
        break;
      ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    }
  }

  PASS();
}

/**
 * @brief server_gen_suite
 * @return Test or helper result.
 */

TEST test_server_gen_null_args(void) {
  struct OpenAPI_Spec spec;
  struct OpenApiClientConfig config;
  memset(&spec, 0, sizeof(spec));
  memset(&config, 0, sizeof(config));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            openapi_server_generate(NULL, &config));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, openapi_server_generate(&spec, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            openapi_server_generate(&spec, &config));
  PASS();
}

/**
 * @brief Unit test test_server_gen_test_fopen_fail.
 * @return Greatest test result.
 */
TEST test_server_gen_test_fopen_fail(void) {
  struct OpenAPI_Spec spec;
  struct OpenApiClientConfig config;
  int rc = 0;

  memset(&spec, 0, sizeof(spec));
  memset(&config, 0, sizeof(config));
  config.filename_base = cdd_test_mut_str("test_server");

/* create directory so test file fopen fails */
#if defined(_WIN32)
  _mkdir("test_test_server_server.c");
#else
  mkdir("test_test_server_server.c", 0777);
#endif

  rc = openapi_server_generate(&spec, &config);
  ASSERT_EQ(0, rc);

#if defined(_WIN32)
  _rmdir("test_test_server_server.c");
#else
  rmdir("test_test_server_server.c");
#endif

  remove("src/test_server_server.c");
  remove("test_test_server_server.c");

  PASS();
}

/**
 * @brief Unit test test_server_gen_branches.
 * @return Greatest test result.
 */
TEST test_server_gen_branches(void) {
  extern C_CDD_EXPORT int g_schema_fail_io_after;
  extern C_CDD_EXPORT int g_schema_io_calls;
  struct OpenAPI_Spec spec;
  struct OpenApiClientConfig config;
  int rc = 0;

  memset(&spec, 0, sizeof(spec));
  memset(&config, 0, sizeof(config));
  config.filename_base = cdd_test_mut_str("test_server_branches");

  spec.n_paths = 1;
  spec.paths =
      (struct OpenAPI_Path *)C_CDD_CALLOC(1, sizeof(struct OpenAPI_Path));
  spec.paths[0].route = cdd_test_mut_str("/test/route");
  spec.paths[0].n_operations = 1;
  spec.paths[0].operations = (struct OpenAPI_Operation *)C_CDD_CALLOC(
      1, sizeof(struct OpenAPI_Operation));

  spec.paths[0].operations[0].verb = OA_VERB_GET;
  spec.paths[0].operations[0].operation_id = cdd_test_mut_str("doGetBranches");
  spec.paths[0].operations[0].summary = cdd_test_mut_str("MySummary");
  spec.paths[0].operations[0].n_parameters = 2;
  spec.paths[0].operations[0].parameters =
      (struct OpenAPI_Parameter *)C_CDD_CALLOC(
          2, sizeof(struct OpenAPI_Parameter));
  spec.paths[0].operations[0].parameters[0].name = cdd_test_mut_str("p1");
  spec.paths[0].operations[0].parameters[0].in = OA_PARAM_IN_HEADER;
  spec.paths[0].operations[0].parameters[1].name = cdd_test_mut_str("p2");
  spec.paths[0].operations[0].parameters[1].in = OA_PARAM_IN_QUERY;
  spec.paths[0].operations[0].parameters[1].description = NULL;

  spec.paths[0].operations[0].req_body.content_schema =
      (struct OpenAPI_SchemaRef *)C_CDD_CALLOC(
          1, sizeof(struct OpenAPI_SchemaRef));
  spec.paths[0].operations[0].req_body.ref = NULL; /* trigger inline */

  spec.n_servers = 1;
  spec.servers =
      (struct OpenAPI_Server *)C_CDD_CALLOC(1, sizeof(struct OpenAPI_Server));
  spec.servers[0].url = NULL;

  spec.info.title = cdd_test_mut_str("A");
  spec.info.version = cdd_test_mut_str("1");

  rc = openapi_server_generate(&spec, &config);
  ASSERT_EQ(0, rc);

  /* Test operation without parameters and with callbacks but no responses */
  spec.paths[0].operations[0].n_parameters = 0;
  spec.paths[0].operations[0].n_callbacks = 1;
  spec.paths[0].operations[0].n_responses = 0;

  /* Test form-urlencoded without ref_name */
  spec.paths[0].operations[0].n_req_body_media_types = 1;
  spec.paths[0].operations[0].req_body_media_types =
      (struct OpenAPI_MediaType *)C_CDD_CALLOC(
          1, sizeof(struct OpenAPI_MediaType));
  spec.paths[0].operations[0].req_body_media_types[0].name =
      cdd_test_mut_str("application/x-www-form-urlencoded");
  spec.paths[0].operations[0].req_body.ref_name = NULL;

  rc = openapi_server_generate(&spec, &config);
  ASSERT_EQ(0, rc);

  /* Test operation with NULL operation_id */
  spec.paths[0].operations[0].operation_id = NULL;
  rc = openapi_server_generate(&spec, &config);
  ASSERT_EQ(0, rc);
  spec.paths[0].operations[0].operation_id = cdd_test_mut_str("doGetBranches");

  /* Test operation with media type whose name is NULL or other (loop natural
   * exit) */
  C_CDD_FREE(spec.paths[0].operations[0].req_body_media_types);
  spec.paths[0].operations[0].n_req_body_media_types = 2;
  spec.paths[0].operations[0].req_body_media_types =
      (struct OpenAPI_MediaType *)C_CDD_CALLOC(
          2, sizeof(struct OpenAPI_MediaType));
  spec.paths[0].operations[0].req_body_media_types[0].name = NULL;
  spec.paths[0].operations[0].req_body_media_types[1].name =
      cdd_test_mut_str("application/json");
  rc = openapi_server_generate(&spec, &config);
  ASSERT_EQ(0, rc);
  C_CDD_FREE(spec.paths[0].operations[0].req_body_media_types);
  spec.paths[0].operations[0].req_body_media_types = NULL;
  spec.paths[0].operations[0].n_req_body_media_types = 0;

  /* Test test stub generation with an operation where operation_id is NULL
   * alongside one where it is not */
  spec.paths[0].n_operations = 2;
  spec.paths[0].operations = (struct OpenAPI_Operation *)C_CDD_REALLOC(
      spec.paths[0].operations, 2 * sizeof(struct OpenAPI_Operation));
  memset(&spec.paths[0].operations[1], 0, sizeof(struct OpenAPI_Operation));
  spec.paths[0].operations[0].operation_id = cdd_test_mut_str("doGetBranches");
  spec.paths[0].operations[1].operation_id = NULL;
  rc = openapi_server_generate(&spec, &config);
  ASSERT_EQ(0, rc);

  /* Test security failure percolation via stream I/O failure */
  {
    struct OpenAPI_SecurityScheme sec_sch;
    memset(&sec_sch, 0, sizeof(sec_sch));
    sec_sch.name = cdd_test_mut_str("bearerAuth");
    sec_sch.type = OA_SEC_HTTP;
    sec_sch.scheme = cdd_test_mut_str("bearer");
    spec.security_schemes = &sec_sch;
    spec.n_security_schemes = 1;
    g_schema_fail_io_after = 0;
    g_schema_io_calls = 0;
    rc = openapi_server_generate(&spec, &config);
    g_schema_fail_io_after = -1;
    spec.security_schemes = NULL;
    spec.n_security_schemes = 0;
    ASSERT_EQ(CDD_C_ERROR_IO, rc);
  }

  C_CDD_FREE(spec.servers);
  C_CDD_FREE(spec.paths[0].operations[0].parameters);
  C_CDD_FREE(spec.paths[0].operations[0].req_body.content_schema);
  C_CDD_FREE(spec.paths[0].operations);
  C_CDD_FREE(spec.paths);

  remove("src/test_server_branches_server.c");
  remove("test_test_server_branches_server.c");

  PASS();
}
/**
 * @brief Executes the server_gen_suite test suite.
 */
SUITE(server_gen_suite) {
  RUN_TEST(test_server_gen_null_args);
  RUN_TEST(test_server_gen_test_fopen_fail);
  RUN_TEST(test_server_gen_branches);
  RUN_TEST(test_server_gen_basic);
  RUN_TEST(test_server_gen_fail_open);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !TEST_SERVER_GEN_H */
