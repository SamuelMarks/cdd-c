#ifndef TEST_CLI_GEN_H
#define TEST_CLI_GEN_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>

#include "cdd_test_helpers/cdd_helpers.h"
#include "routes/emit/cli_gen.h"
#include "routes/emit/client_gen.h"
/* clang-format on */

TEST test_cli_gen_basic(void) {
  struct OpenAPI_Spec spec;
  struct OpenApiClientConfig config;
  int rc;
  FILE *f;

  memset(&spec, 0, sizeof(spec));
  spec.n_paths = 1;
  spec.paths = (struct OpenAPI_Path *)calloc(1, sizeof(struct OpenAPI_Path));
  spec.paths[0].n_operations = 1;
  spec.paths[0].operations =
      (struct OpenAPI_Operation *)calloc(1, sizeof(struct OpenAPI_Operation));
  spec.paths[0].operations[0].operation_id = "doSomething";
  spec.paths[0].operations[0].summary = "Does a thing";

  memset(&config, 0, sizeof(config));
  config.filename_base = "test_cli";

  rc = openapi_cli_generate(&spec, &config);
  ASSERT_EQ(0, rc);

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  if (fopen_s(&f, "src/test_cli_cli.c", "r") != 0)
    f = NULL;
#elif defined(_MSC_VER)
  fopen_s(&f, "src/test_cli_cli.c", "r");
#else
  f = fopen("src/test_cli_cli.c", "r");
#endif
  ASSERT(f != NULL);
  if (f)
    fclose(f);

  remove("src/test_cli_cli.c");
  free(spec.paths[0].operations[0].parameters);
  free(spec.paths[0].operations);
  free(spec.paths);

  PASS();
}

TEST test_cli_gen_fail_open(void) {
  struct OpenAPI_Spec spec;
  struct OpenApiClientConfig config;
  int rc;

  memset(&spec, 0, sizeof(spec));
  memset(&config, 0, sizeof(config));
  config.filename_base = "/nonexistent/dir/test_cli";

  rc = openapi_cli_generate(&spec, &config);
  ASSERT_EQ(-1, rc);

  PASS();
}

TEST test_cli_gen_full(void) {
  struct OpenAPI_Spec spec;
  struct OpenApiClientConfig config;
  int rc;

  memset(&spec, 0, sizeof(spec));
  spec.openapi_version = "3.1.0";
  spec.info.title = "Full API";
  spec.info.version = "1.0.0";
  spec.info.description = "Full Description";
  spec.info.terms_of_service = "https://terms.example.com";
  spec.info.contact.name = "Support";
  spec.info.contact.email = "support@example.com";
  spec.info.contact.url = "https://support.example.com";
  spec.info.license.name = "MIT";
  spec.info.license.identifier = "MIT-id";
  spec.info.license.url = "https://license.example.com";

  spec.n_servers = 1;
  spec.servers = calloc(1, sizeof(*spec.servers));
  spec.servers[0].url = "https://api.example.com";
  spec.servers[0].description = "Prod server";
  spec.servers[0].variables = (void *)1; // dummy pointer

  spec.n_webhooks = 1; // dummy trigger
  spec.external_docs.url = "https://docs.example.com";

  spec.n_paths = 1;
  spec.paths = calloc(1, sizeof(*spec.paths));
  spec.paths[0].route = "/full";
  spec.paths[0].n_operations = 1;
  spec.paths[0].operations = calloc(1, sizeof(*spec.paths[0].operations));
  spec.paths[0].operations[0].method = "get";
  spec.paths[0].operations[0].operation_id = "doFull";
  spec.paths[0].operations[0].summary = "Does full thing";

  spec.json_schema_dialect = "https://json-schema.org/draft/2020-12/schema";

  spec.paths[0].operations[0].n_parameters = 1;
  spec.paths[0].operations[0].parameters =
      calloc(1, sizeof(*spec.paths[0].operations[0].parameters));
  spec.paths[0].operations[0].parameters[0].name = "param1";
  spec.paths[0].operations[0].parameters[0].in = 1;
  spec.paths[0].operations[0].parameters[0].required = 1;
  spec.paths[0].operations[0].parameters[0].explode = 1;
  spec.paths[0].operations[0].parameters[0].description = "desc";
  spec.paths[0].operations[0].parameters[0].allow_empty_value = 1;
  spec.paths[0].operations[0].parameters[0].allow_reserved = 1;
  spec.paths[0].operations[0].parameters[0].style = 1;
  spec.paths[0].operations[0].parameters[0].example.type =
      1; // OA_ANY_INT or something

  memset(&config, 0, sizeof(config));
  config.filename_base = "test_cli_full";

  rc = openapi_cli_generate(&spec, &config);
  ASSERT_EQ(0, rc);

  remove("src/test_cli_full_cli.c");
  free(spec.servers);
  free(spec.paths[0].operations[0].parameters);
  free(spec.paths[0].operations);
  free(spec.paths);

  PASS();
}

SUITE(cli_gen_suite) {
  RUN_TEST(test_cli_gen_basic);
  RUN_TEST(test_cli_gen_fail_open);
  RUN_TEST(test_cli_gen_full);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !TEST_CLI_GEN_H */
