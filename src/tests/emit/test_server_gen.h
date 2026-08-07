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

#include "cdd_test_helpers/cdd_helpers.h"
#include "routes/emit/client_gen.h"
#include "routes/emit/server_gen.h"
/* clang-format on */

/**
 * @brief test_server_gen_basic
 * @return TEST
 */
TEST test_server_gen_basic(void) {
  struct OpenAPI_Spec spec;
  struct OpenApiClientConfig config;
  int rc;
  FILE *f;

  memset(&spec, 0, sizeof(spec));
  spec.n_paths = 1;
  spec.paths =
      (struct OpenAPI_Path *)C_CDD_CALLOC(1, sizeof(struct OpenAPI_Path));
  spec.paths[0].route = "/test/route";

  spec.paths[0].n_operations = 9;
  spec.paths[0].operations = (struct OpenAPI_Operation *)C_CDD_CALLOC(
      9, sizeof(struct OpenAPI_Operation));

  spec.paths[0].operations[0].verb = OA_VERB_GET;
  spec.paths[0].operations[0].operation_id = "doGet";

  spec.paths[0].operations[1].verb = OA_VERB_POST;
  spec.paths[0].operations[1].operation_id = "doPost";

  spec.paths[0].operations[2].verb = OA_VERB_PUT;
  spec.paths[0].operations[2].operation_id = "doPut";

  spec.paths[0].operations[3].verb = OA_VERB_DELETE;
  spec.paths[0].operations[3].operation_id = "doDelete";

  spec.paths[0].operations[4].verb = OA_VERB_OPTIONS;
  spec.paths[0].operations[4].operation_id = "doOptions";

  spec.paths[0].operations[5].verb = OA_VERB_HEAD;
  spec.paths[0].operations[5].operation_id = "doHead";

  spec.paths[0].operations[6].verb = OA_VERB_PATCH;
  spec.paths[0].operations[6].operation_id = "doPatch";

  spec.paths[0].operations[7].verb = OA_VERB_TRACE;
  spec.paths[0].operations[7].operation_id = "doTrace";

  spec.paths[0].operations[8].verb =
      (enum OpenAPI_Verb)999; /* Unknown verb triggers default branch */
  spec.paths[0].operations[8].operation_id = "doUnknown";
  spec.info.description = "Test Desc";
  spec.info.contact.name = "Test Contact";
  spec.info.license.name = "Test License";
  spec.n_servers = 1;
  spec.servers =
      (struct OpenAPI_Server *)C_CDD_CALLOC(1, sizeof(struct OpenAPI_Server));
  spec.servers[0].url = "http://test";

  spec.paths[0].operations[0].description = "Test Op Desc";
  spec.paths[0].operations[0].n_parameters = 1;
  spec.paths[0].operations[0].parameters =
      (struct OpenAPI_Parameter *)C_CDD_CALLOC(
          1, sizeof(struct OpenAPI_Parameter));
  spec.paths[0].operations[0].parameters[0].name = "param1";
  spec.paths[0].operations[0].parameters[0].in = OA_PARAM_IN_QUERY;
  spec.paths[0].operations[0].parameters[0].description = "Test Param Desc";

  spec.paths[0].operations[0].deprecated = 1;
  spec.paths[0].operations[0].req_body.content_schema =
      (struct OpenAPI_SchemaRef *)C_CDD_CALLOC(
          1, sizeof(struct OpenAPI_SchemaRef));
  spec.paths[0].operations[0].req_body.ref = "TestRef";

  spec.paths[0].operations[1].req_body.ref_name = "TestRefName";

  spec.paths[0].operations[0].n_responses = 1;
  spec.paths[0].operations[0].n_callbacks = 1;
  spec.paths[0].operations[0].security =
      (struct OpenAPI_SecurityRequirementSet *)C_CDD_CALLOC(
          1, sizeof(struct OpenAPI_SecurityRequirementSet));

  spec.paths[0].operations[1].n_req_body_media_types = 2;
  spec.paths[0].operations[1].req_body_media_types =
      (struct OpenAPI_MediaType *)C_CDD_CALLOC(
          2, sizeof(struct OpenAPI_MediaType));
  spec.paths[0].operations[1].req_body_media_types[0].name = "application/json";
  spec.paths[0].operations[1].req_body_media_types[1].name =
      "application/x-www-form-urlencoded";

  memset(&config, 0, sizeof(config));
  config.filename_base = "test_server";

  rc = openapi_server_generate(&spec, &config);
  ASSERT_EQ(0, rc);

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  if (fopen_s(&f, "src/test_server_server.c", "r") != 0)
    f = NULL;
#elif defined(_MSC_VER)
  fopen_s(&f, "src/test_server_server.c", "r");
#else
  f = fopen("src/test_server_server.c", "r");
#endif

  ASSERT(f != NULL);
  if (f) {
    char buf[16384];
    size_t n = fread(buf, 1, sizeof(buf) - 1, f);
    buf[n] = '\0';
    ASSERT(strstr(buf, "handle_mcp_sse") != NULL);
    ASSERT(strstr(buf, "handle_mcp_message") != NULL);
    fclose(f);
  }

  remove("src/test_server_server.c");
  C_CDD_FREE(spec.paths[0].operations[1].req_body_media_types);

  C_CDD_FREE(spec.servers);
  C_CDD_FREE(spec.paths[0].operations[0].parameters);
  C_CDD_FREE(spec.paths[0].operations[0].req_body.content_schema);
  C_CDD_FREE(spec.paths[0].operations[0].security);
  C_CDD_FREE(spec.paths[0].operations);

  C_CDD_FREE(spec.paths);
  g_fail_io_after = -1;

  g_cdd_alloc_fail = 1;
  config.filename_base = "test_server";
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
  int rc;

  memset(&spec, 0, sizeof(spec));
  memset(&config, 0, sizeof(config));
  config.filename_base = "/nonexistent/dir/test_server";

  rc = openapi_server_generate(&spec, &config);
  ASSERT_EQ(0, rc);
  g_fail_io_after = -1;

  g_cdd_alloc_fail = 1;
  config.filename_base = "test_server";
  rc = openapi_server_generate(&spec, &config);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  g_cdd_alloc_fail = 0;

  PASS();
}

/**
 * @brief server_gen_suite
 */
SUITE(server_gen_suite) {
  RUN_TEST(test_server_gen_basic);
  RUN_TEST(test_server_gen_fail_open);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !TEST_SERVER_GEN_H */
