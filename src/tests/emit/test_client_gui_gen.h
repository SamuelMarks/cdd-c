#ifndef TEST_CLIENT_GUI_GEN_H
#define TEST_CLIENT_GUI_GEN_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "cdd_test_helpers/cdd_helpers.h"
#include "routes/emit/client_gui_gen.h"
#include "routes/emit/client_gen.h"
/* clang-format on */

TEST test_client_gui_gen_basic(void) {
  struct OpenAPI_Spec spec;
  struct OpenApiClientConfig config;
  int rc;
  FILE *f;

  memset(&spec, 0, sizeof(spec));
  memset(&config, 0, sizeof(config));
  config.filename_base = "test_gui";

  rc = openapi_client_gui_generate(&spec, &config);
  ASSERT_EQ(0, rc);

  f = fopen("src/test_gui_gui.c", "r");
  ASSERT(f != NULL);
  if (f)
    fclose(f);

  f = fopen("src/test_gui_gui.h", "r");
  ASSERT(f != NULL);
  if (f)
    fclose(f);

  remove("src/test_gui_gui.c");
  remove("src/test_gui_gui.h");

  PASS();
}

TEST test_client_gui_gen_with_server(void) {
  struct OpenAPI_Spec spec;
  struct OpenApiClientConfig config;
  int rc;

  memset(&spec, 0, sizeof(spec));
  spec.n_servers = 1;
  spec.servers =
      (struct OpenAPI_Server *)calloc(1, sizeof(struct OpenAPI_Server));
  spec.servers[0].url = "https://api.example.com";

  memset(&config, 0, sizeof(config));
  config.filename_base = "test_gui2";

  rc = openapi_client_gui_generate(&spec, &config);
  ASSERT_EQ(0, rc);

  remove("src/test_gui2_gui.c");
  remove("src/test_gui2_gui.h");
  free(spec.servers);

  PASS();
}

TEST test_client_gui_gen_errors(void) {
  struct OpenAPI_Spec spec;
  struct OpenApiClientConfig config;
  int rc;

  memset(&spec, 0, sizeof(spec));
  memset(&config, 0, sizeof(config));
  config.filename_base = "/nonexistent/dir/test_gui";

  rc = openapi_client_gui_generate(&spec, &config);
  ASSERT_EQ(EIO, rc);

  rc = openapi_client_gui_generate(NULL, &config);
  ASSERT_EQ(EINVAL, rc);

  rc = openapi_client_gui_generate(&spec, NULL);
  ASSERT_EQ(EINVAL, rc);

  config.filename_base = NULL;
  rc = openapi_client_gui_generate(&spec, &config);
  ASSERT_EQ(EINVAL, rc);

  PASS();
}

SUITE(client_gui_gen_suite) {
  RUN_TEST(test_client_gui_gen_basic);
  RUN_TEST(test_client_gui_gen_with_server);
  RUN_TEST(test_client_gui_gen_errors);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !TEST_CLIENT_GUI_GEN_H */
