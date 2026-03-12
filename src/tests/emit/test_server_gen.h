#ifndef TEST_SERVER_GEN_H
#define TEST_SERVER_GEN_H

/* clang-format off */
#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>

#include "cdd_test_helpers/cdd_helpers.h"
#include "routes/emit/client_gen.h"
#include "routes/emit/server_gen.h"
/* clang-format on */

TEST test_server_gen_basic(void) {
  struct OpenAPI_Spec spec;
  struct OpenApiClientConfig config;
  int rc;
  FILE *f;

  memset(&spec, 0, sizeof(spec));
  spec.n_paths = 1;
  spec.paths = (struct OpenAPI_Path *)calloc(1, sizeof(struct OpenAPI_Path));
  spec.paths[0].route = "/test/route";

  spec.paths[0].n_operations = 9;
  spec.paths[0].operations =
      (struct OpenAPI_Operation *)calloc(9, sizeof(struct OpenAPI_Operation));

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

  memset(&config, 0, sizeof(config));
  config.filename_base = "test_server";

  rc = openapi_server_generate(&spec, &config);
  ASSERT_EQ(0, rc);

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  if (fopen_s(&f, "test_server_server.c", "r") != 0)
    f = NULL;
#elif defined(_MSC_VER)
  fopen_s(&f, "test_server_server.c", "r");
#else
  f = fopen("test_server_server.c", "r");
#endif
  ASSERT(f != NULL);
  if (f)
    fclose(f);

  remove("test_server_server.c");
  free(spec.paths[0].operations);
  free(spec.paths);

  PASS();
}

TEST test_server_gen_fail_open(void) {
  struct OpenAPI_Spec spec;
  struct OpenApiClientConfig config;
  int rc;

  memset(&spec, 0, sizeof(spec));
  memset(&config, 0, sizeof(config));
  config.filename_base = "/nonexistent/dir/test_server";

  rc = openapi_server_generate(&spec, &config);
  ASSERT_EQ(-1, rc);

  PASS();
}

SUITE(server_gen_suite) {
  RUN_TEST(test_server_gen_basic);
  RUN_TEST(test_server_gen_fail_open);
}

#endif /* !TEST_SERVER_GEN_H */
