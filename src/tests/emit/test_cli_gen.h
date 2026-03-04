#ifndef TEST_CLI_GEN_H
#define TEST_CLI_GEN_H

#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>

#include "cdd_test_helpers/cdd_helpers.h"
#include "routes/emit/cli_gen.h"
#include "routes/emit/client_gen.h"

TEST test_cli_gen_basic(void) {
  struct OpenAPI_Spec spec;
  struct OpenApiClientConfig config;
  int rc;
  FILE *f;

  memset(&spec, 0, sizeof(spec));
  spec.n_paths = 1;
  spec.paths = calloc(1, sizeof(struct OpenAPI_Path));
  spec.paths[0].n_operations = 1;
  spec.paths[0].operations = calloc(1, sizeof(struct OpenAPI_Operation));
  spec.paths[0].operations[0].operation_id = "doSomething";
  spec.paths[0].operations[0].summary = "Does a thing";

  memset(&config, 0, sizeof(config));
  config.filename_base = "test_cli";

  rc = openapi_cli_generate(&spec, &config);
  ASSERT_EQ(0, rc);

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  if (fopen_s(&f, "test_cli_cli.c", "r") != 0)
    f = NULL;
#else
  f = fopen("test_cli_cli.c", "r");
#endif
  ASSERT(f != NULL);
  if (f)
    fclose(f);

  remove("test_cli_cli.c");
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

SUITE(cli_gen_suite) {
  RUN_TEST(test_cli_gen_basic);
  RUN_TEST(test_cli_gen_fail_open);
}

#endif /* !TEST_CLI_GEN_H */
