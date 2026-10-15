#ifndef TEST_CLI_GEN_H
#define TEST_CLI_GEN_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>

#include "cdd_test_helpers/cdd_helpers.h"
#include "routes/emit/cli_gen.h"
#include "routes/emit/client_gen.h"
/* clang-format on */

extern C_CDD_EXPORT int g_fail_io_after;
extern C_CDD_EXPORT int g_io_calls;

/**
 * @brief test_cli_gen_basic
 * @return TEST
 */
TEST test_cli_gen_basic(void) {
  struct OpenAPI_Spec spec;
  struct OpenApiClientConfig config;
  int rc = 0;
  FILE *f;

  struct OpenAPI_Response resp = {0};
  struct OpenAPI_Parameter param = {0};
  struct OpenAPI_Link link = {0};
  struct OpenAPI_Header header = {0};
  struct OpenAPI_SecurityRequirement sec = {0};
  struct OpenAPI_SecurityRequirementSet sec_set = {0};
  struct OpenAPI_SecurityScheme scheme = {0};
  struct OpenAPI_OAuthFlow flow = {0};
  struct OpenAPI_Callback cb = {0};

  memset(&spec, 0, sizeof(spec));
  spec.n_paths = 1;
  spec.paths = (struct OpenAPI_Path *)calloc(1, sizeof(struct OpenAPI_Path));
  spec.paths[0].n_operations = 1;
  spec.paths[0].operations =
      (struct OpenAPI_Operation *)calloc(1, sizeof(struct OpenAPI_Operation));
  spec.paths[0].operations[0].operation_id = cdd_test_mut_str("doSomething");
  spec.paths[0].operations[0].summary = cdd_test_mut_str("Does a thing");

  /* Add parameters with examples */
  param.name = cdd_test_mut_str("query_param");
  param.example_set = 1;
  spec.paths[0].operations[0].parameters = &param;
  spec.paths[0].operations[0].n_parameters = 1;

  /* Add requestBody */
  spec.paths[0].operations[0].req_body_required_set = 1;
  spec.paths[0].operations[0].req_body_required = 1;
  spec.paths[0].operations[0].req_body.content_schema =
      (struct OpenAPI_SchemaRef *)calloc(1, sizeof(struct OpenAPI_SchemaRef));

  spec.paths[0].operations[0].deprecated = 1;

  /* Add responses with links and headers */
  resp.code = cdd_test_mut_str("200");
  link.operation_ref = cdd_test_mut_str("opRef");
  resp.links = &link;
  resp.n_links = 1;
  header.name = cdd_test_mut_str("X-Header");
  resp.headers = &header;
  resp.n_headers = 1;
  spec.paths[0].operations[0].responses = &resp;
  spec.paths[0].operations[0].n_responses = 1;

  /* Add externalDocs */
  spec.paths[0].operations[0].external_docs.url =
      cdd_test_mut_str("http://doc");

  /* Add callbacks */
  cb.name = cdd_test_mut_str("myCb");
  spec.paths[0].operations[0].callbacks = &cb;
  spec.paths[0].operations[0].n_callbacks = 1;

  /* Add security schemes and requirements */
  sec.n_scopes = 0;
  sec_set.requirements = &sec;
  sec_set.n_requirements = 1;
  spec.paths[0].operations[0].security = &sec_set;
  spec.paths[0].operations[0].n_security = 1;
  spec.paths[0].operations[0].security_set = 1;

  flow.type = OA_OAUTH_FLOW_IMPLICIT;
  flow.authorization_url = cdd_test_mut_str("url");
  scheme.flows = &flow;
  scheme.n_flows = 1;
  spec.security_schemes = &scheme;
  spec.n_security_schemes = 1;

  memset(&config, 0, sizeof(config));
  config.filename_base = cdd_test_mut_str("test_cli");

  rc = openapi_cli_generate(&spec, &config);
  ASSERT_EQ(0, rc);

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  if (fopen_s(&f, "src/test_cli_cli.c", "r") != 0)
    f = NULL;
#elif defined(_MSC_VER)
  fopen_s(&f, "src/test_cli_cli.c", "r");
#else
#if defined(_MSC_VER)
  if (fopen_s(&f, "src/test_cli_cli.c", "r") != 0)
    f = NULL;
#else
  f = fopen("src/test_cli_cli.c", "r");
#endif
#endif
  ASSERT(f != NULL);
  if (f)
    if (f)
      fclose(f);

  remove("src/test_cli_cli.c");
  free(spec.paths[0].operations[0].req_body.content_schema);
  free(spec.paths[0].operations);
  free(spec.paths);
  g_fail_io_after = -1;

  PASS();
}

/**
 * @brief test_cli_gen_fail_open
 * @return TEST
 */
TEST test_cli_gen_fail_open(void) {
  struct OpenAPI_Spec spec;
  struct OpenApiClientConfig config;
  int rc = 0;

  memset(&spec, 0, sizeof(spec));
  memset(&config, 0, sizeof(config));

  /* Case 1: makedirs fails because directory path cannot be created */
  config.filename_base = cdd_test_mut_str("/nonexistent/dir/test_cli");
  g_io_calls = 0;
  g_fail_io_after = 1;

  rc = openapi_cli_generate(&spec, &config);
  ASSERT(rc == CDD_C_ERROR_IO || rc == CDD_C_ERROR_NOT_FOUND);
  g_fail_io_after = -1;

  /* Case 2: fopen fails because destination path is an existing directory */
  makedirs("test_build_dir/bad_cli/src/bad_base_cli.c");
  config.filename_base = cdd_test_mut_str("test_build_dir/bad_cli/bad_base");
  rc = openapi_cli_generate(&spec, &config);
  ASSERT_EQ(CDD_C_ERROR_IO, rc);
  remove("test_build_dir/bad_cli/src/bad_base_cli.c");

  PASS();
}

/**
 * @brief test_cli_gen_full
 * @return TEST
 */
TEST test_cli_gen_full(void) {
  struct OpenAPI_Spec spec;
  struct OpenApiClientConfig config;
  int rc = 0;

  memset(&spec, 0, sizeof(spec));
  spec.openapi_version = cdd_test_mut_str("3.1.0");
  spec.info.title = cdd_test_mut_str("Full API");
  spec.info.version = cdd_test_mut_str("1.0.0");
  spec.info.description = cdd_test_mut_str("Full Description");
  spec.info.terms_of_service = cdd_test_mut_str("https://terms.example.com");
  spec.info.contact.name = cdd_test_mut_str("Support");
  spec.info.contact.email = cdd_test_mut_str("support@example.com");
  spec.info.contact.url = cdd_test_mut_str("https://support.example.com");
  spec.info.license.name = cdd_test_mut_str("MIT");
  spec.info.license.identifier = cdd_test_mut_str("MIT-id");
  spec.info.license.url = cdd_test_mut_str("https://license.example.com");

  spec.n_servers = 1;
  spec.servers = calloc(1, sizeof(*spec.servers));
  spec.servers[0].url = cdd_test_mut_str("https://api.example.com");
  spec.servers[0].description = cdd_test_mut_str("Prod server");
  {
    static struct OpenAPI_ServerVariable dummy_var;
    spec.servers[0].variables = &dummy_var;
  }

  spec.n_webhooks = 1; /* dummy trigger */
  spec.external_docs.url = cdd_test_mut_str("https://docs.example.com");

  spec.n_paths = 1;
  spec.paths = calloc(1, sizeof(*spec.paths));
  spec.paths[0].route = cdd_test_mut_str("/full");
  spec.paths[0].n_operations = 1;
  spec.paths[0].operations = calloc(1, sizeof(*spec.paths[0].operations));
  spec.paths[0].operations[0].method = cdd_test_mut_str("get");
  spec.paths[0].operations[0].operation_id = cdd_test_mut_str("doFull");
  spec.paths[0].operations[0].summary = cdd_test_mut_str("Does full thing");

  spec.json_schema_dialect =
      cdd_test_mut_str("https://json-schema.org/draft/2020-12/schema");

  spec.paths[0].operations[0].n_parameters = 1;
  spec.paths[0].operations[0].parameters =
      calloc(1, sizeof(*spec.paths[0].operations[0].parameters));
  spec.paths[0].operations[0].parameters[0].name = cdd_test_mut_str("param1");
  spec.paths[0].operations[0].parameters[0].in = 1;
  spec.paths[0].operations[0].parameters[0].required = 1;
  spec.paths[0].operations[0].parameters[0].explode = 1;
  spec.paths[0].operations[0].parameters[0].description =
      cdd_test_mut_str("desc");
  spec.paths[0].operations[0].parameters[0].allow_empty_value = 1;
  spec.paths[0].operations[0].parameters[0].allow_reserved = 1;
  spec.paths[0].operations[0].parameters[0].style = 1;
  spec.paths[0].operations[0].parameters[0].example.type =
      1; /* OA_ANY_INT or something */

  memset(&config, 0, sizeof(config));
  config.filename_base = cdd_test_mut_str("test_cli_full");

  rc = openapi_cli_generate(&spec, &config);
  ASSERT_EQ(0, rc);

  remove("src/test_cli_full_cli.c");
  free(spec.servers);
  free(spec.paths[0].operations[0].parameters);
  free(spec.paths[0].operations);
  free(spec.paths);
  g_fail_io_after = -1;

  PASS();
}

/**
 * @brief cli_gen_suite
 */

/**
 * @brief test_cli_gen_malloc_fail
 * @return TEST
 */
TEST test_cli_gen_malloc_fail(void) {
  struct OpenAPI_Spec spec;
  struct OpenApiClientConfig config;
  int rc = 0;
  int i;

  memset(&spec, 0, sizeof(spec));
  memset(&config, 0, sizeof(config));
  config.filename_base = cdd_test_mut_str("test_build_dir/cli");

  for (i = 1; i < 5; i++) {
    g_cdd_alloc_fail = i;
    rc = openapi_cli_generate(&spec, &config);
    g_cdd_alloc_fail = 0;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  }

  /* Test NULL arguments */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, openapi_cli_generate(NULL, &config));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, openapi_cli_generate(&spec, NULL));
  config.filename_base = NULL;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, openapi_cli_generate(&spec, &config));

  PASS();
}

TEST test_cli_gen_partial(void) {
  struct OpenAPI_Spec spec;
  struct OpenApiClientConfig config;
  int rc = 0;

  memset(&spec, 0, sizeof(spec));
  spec.openapi_version = cdd_test_mut_str("3.1.0");
  spec.info.title = cdd_test_mut_str("Partial API");
  spec.info.version = cdd_test_mut_str("1.0.0");

  spec.info.contact.url = cdd_test_mut_str("https://support.example.com");
  spec.info.license.name = cdd_test_mut_str("MIT");

  spec.n_servers = 2;
  spec.servers = (struct OpenAPI_Server *)calloc(2, sizeof(*spec.servers));
  spec.servers[0].variables = NULL;
  {
    static struct OpenAPI_ServerVariable dummy_var2;
    spec.servers[1].variables = &dummy_var2;
  }

  spec.n_paths = 1;
  spec.paths = (struct OpenAPI_Path *)calloc(1, sizeof(*spec.paths));
  spec.paths[0].route = cdd_test_mut_str("/partial");
  spec.paths[0].n_operations = 3;
  spec.paths[0].operations =
      (struct OpenAPI_Operation *)calloc(3, sizeof(*spec.paths[0].operations));
  spec.paths[0].operations[0].method = cdd_test_mut_str("get");
  spec.paths[0].operations[0].operation_id = cdd_test_mut_str("partial_op");
  spec.paths[0].operations[0].n_parameters = 1;
  spec.paths[0].operations[0].parameters =
      (struct OpenAPI_Parameter *)calloc(1, sizeof(struct OpenAPI_Parameter));
  spec.paths[0].operations[0].n_responses = 1;
  spec.paths[0].operations[0].responses =
      (struct OpenAPI_Response *)calloc(1, sizeof(struct OpenAPI_Response));
  spec.paths[0].operations[0].responses[0].code = cdd_test_mut_str("200");
  spec.paths[0].operations[0].security_set = 1;
  spec.paths[0].operations[0].n_security = 1;
  spec.paths[0].operations[0].security =
      (struct OpenAPI_SecurityRequirementSet *)calloc(
          1, sizeof(struct OpenAPI_SecurityRequirementSet));

  spec.paths[0].operations[1].method = cdd_test_mut_str("post");
  spec.paths[0].operations[1].operation_id = cdd_test_mut_str("empty_op");
  spec.paths[0].operations[1].n_parameters = 0;
  spec.paths[0].operations[1].n_responses = 1;
  spec.paths[0].operations[1].responses =
      (struct OpenAPI_Response *)calloc(1, sizeof(struct OpenAPI_Response));
  spec.paths[0].operations[1].responses[0].code = cdd_test_mut_str("200");

  spec.paths[0].operations[2].method = cdd_test_mut_str("put");
  spec.paths[0].operations[2].operation_id = NULL;
  spec.paths[0].operations[2].summary = cdd_test_mut_str("should_not_print");
  spec.paths[0].operations[2].n_parameters = 0;
  spec.paths[0].operations[2].n_responses = 1;
  spec.paths[0].operations[2].responses =
      (struct OpenAPI_Response *)calloc(1, sizeof(struct OpenAPI_Response));
  spec.paths[0].operations[2].responses[0].code = cdd_test_mut_str("200");

  spec.security_schemes = NULL;

  memset(&config, 0, sizeof(config));
  config.filename_base = cdd_test_mut_str("test_cli_partial");

  rc = openapi_cli_generate(&spec, &config);
  ASSERT_EQ(0, rc);

  remove("src/test_cli_partial_cli.c");
  free(spec.paths[0].operations[0].security);
  free(spec.paths[0].operations[0].responses);
  free(spec.paths[0].operations[0].parameters);
  free(spec.paths[0].operations[1].responses);
  free(spec.paths[0].operations[2].responses);
  free(spec.servers);
  free(spec.paths[0].operations);
  free(spec.paths);
  g_fail_io_after = -1;

  PASS();
}

TEST test_cli_gen_partial2(void) {
  struct OpenAPI_Spec spec;
  struct OpenApiClientConfig config;
  int rc = 0;

  memset(&spec, 0, sizeof(spec));
  spec.openapi_version = cdd_test_mut_str("3.1.0");
  spec.info.title = cdd_test_mut_str("Partial API 2");
  spec.info.version = cdd_test_mut_str("1.0.0");

  spec.info.contact.email = cdd_test_mut_str("support@example.com");
  spec.info.license.identifier = cdd_test_mut_str("MIT-id");

  memset(&config, 0, sizeof(config));
  config.filename_base = cdd_test_mut_str("test_cli_partial2");

  rc = openapi_cli_generate(&spec, &config);
  ASSERT_EQ(0, rc);

  remove("src/test_cli_partial2_cli.c");
  g_fail_io_after = -1;

  PASS();
}

SUITE(cli_gen_suite) {
  RUN_TEST(test_cli_gen_basic);
  RUN_TEST(test_cli_gen_fail_open);
  RUN_TEST(test_cli_gen_full);
  RUN_TEST(test_cli_gen_partial);
  RUN_TEST(test_cli_gen_partial2);
  RUN_TEST(test_cli_gen_malloc_fail);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !TEST_CLI_GEN_H */
