/**
 * @file test_client_gui_gen.h
 * @brief Unit tests for client GUI generation.
 */

#ifndef TEST_CLIENT_GUI_GEN_H
#define TEST_CLIENT_GUI_GEN_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "cdd_test_helpers/cdd_helpers.h"
#include "routes/emit/client_gui_gen.h"
#include "routes/emit/client_gen.h"
/* clang-format on */

extern C_CDD_EXPORT int g_fail_io_after;
extern C_CDD_EXPORT int g_io_calls;

/**
 * @brief Tests basic functionality of client GUI generation.
 *
 * @return The result of the test.
 */
TEST test_client_gui_gen_basic(void) {
  struct OpenAPI_Spec spec;
  struct OpenApiClientConfig config;
  int rc;
  FILE *f;

  (void)rc;
  memset(&spec, 0, sizeof(spec));
  memset(&config, 0, sizeof(config));
  config.filename_base = (char *)(size_t)(size_t) "test_gui";

  rc = openapi_client_gui_generate(&spec, &config);
  ASSERT_EQ(0, rc);

#if defined(_MSC_VER)
  if (fopen_s(&f, "src/test_gui_gui.c", "r") != 0)
    f = NULL;
#else
  f = fopen("src/test_gui_gui.c", "r");
#endif
  ASSERT(f != NULL);
  if (f)
    if (f)
      fclose(f);

#if defined(_MSC_VER)
  if (fopen_s(&f, "src/test_gui_gui.h", "r") != 0)
    f = NULL;
#else
  f = fopen("src/test_gui_gui.h", "r");
#endif
  ASSERT(f != NULL);
  if (f)
    if (f)
      fclose(f);

  remove("src/test_gui_gui.c");
  remove("src/test_gui_gui.h");
  {
    FILE *dummy;
#if defined(_MSC_VER)
    if (fopen_s(&dummy, "src/test_gui_gui.c", "w") == 0 && dummy)
      fclose(dummy);
    if (fopen_s(&dummy, "src/test_gui_gui.h", "w") == 0 && dummy)
      fclose(dummy);
#else
    dummy = fopen("src/test_gui_gui.c", "w");
    if (dummy)
      fclose(dummy);
    dummy = fopen("src/test_gui_gui.h", "w");
    if (dummy)
      fclose(dummy);
#endif
  }
  g_fail_io_after = -1;

  PASS();
}

/**
 * @brief Tests client GUI generation with server URLs.
 *
 * @return The result of the test.
 */
TEST test_client_gui_gen_with_server(void) {
  struct OpenAPI_Spec spec;
  struct OpenApiClientConfig config;
  int rc;

  (void)rc;
  memset(&spec, 0, sizeof(spec));
  spec.n_servers = 1;
  spec.servers =
      (struct OpenAPI_Server *)C_CDD_CALLOC(1, sizeof(struct OpenAPI_Server));
  spec.servers[0].url = (char *)(size_t)(size_t) "https://api.example.com";

  memset(&config, 0, sizeof(config));
  config.filename_base = (char *)(size_t)(size_t) "test_gui2";

  rc = openapi_client_gui_generate(&spec, &config);
  ASSERT_EQ(0, rc);

  remove("src/test_gui2_gui.c");
  remove("src/test_gui2_gui.h");
  C_CDD_FREE(spec.servers);
  g_fail_io_after = -1;

  PASS();
}

/**
 * @brief Tests error handling of client GUI generation APIs.
 *
 * @return The result of the test.
 */
TEST test_client_gui_gen_errors(void) {
  struct OpenAPI_Spec spec;
  struct OpenApiClientConfig config;
  int rc;

  (void)rc;
  memset(&spec, 0, sizeof(spec));
  memset(&config, 0, sizeof(config));
  config.filename_base = (char *)(size_t)(size_t) "/nonexistent/dir/test_gui";
  g_io_calls = 0;
  g_fail_io_after = 1;
  rc = openapi_client_gui_generate(&spec, &config);
  ASSERT(rc == CDD_C_ERROR_IO || rc == CDD_C_ERROR_NOT_FOUND);
  g_fail_io_after = -1;

  rc = openapi_client_gui_generate(NULL, &config);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  rc = openapi_client_gui_generate(&spec, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  config.filename_base = NULL;
  rc = openapi_client_gui_generate(&spec, &config);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  g_fail_io_after = -1;

  config.filename_base = (char *)(size_t)(size_t) "test_build_dir/test_gui";
  {
    int i;
    for (i = 1; i <= 5; ++i) {
      g_cdd_alloc_fail = i;
      rc = openapi_client_gui_generate(&spec, &config);
      g_cdd_alloc_fail = 0;
      if (rc == CDD_C_SUCCESS)
        break;
      ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    }
  }

  /* Test server with url == NULL (spec->n_servers > 0 && spec->servers[0].url
   * == NULL) */
  {
    struct OpenAPI_Server srv;
    memset(&srv, 0, sizeof(srv));
    srv.url = NULL;
    spec.servers = &srv;
    spec.n_servers = 1;
    config.filename_base = (char *)(size_t)(size_t) "test_gui_null_url";
    rc = openapi_client_gui_generate(&spec, &config);
    ASSERT_EQ(0, rc);
    remove("src/test_gui_null_url_gui.c");
    remove("src/test_gui_null_url_gui.h");
    spec.servers = NULL;
    spec.n_servers = 0;
  }

  /* Test fopen failures */
  /* Case 1: fp_h fails to open because it is a directory */
  makedirs("test_build_dir/bad_gui1/src/bad_gui_gui.h");
  config.filename_base =
      (char *)(size_t)(size_t) "test_build_dir/bad_gui1/bad_gui";
  rc = openapi_client_gui_generate(&spec, &config);
  ASSERT_EQ(CDD_C_ERROR_IO, rc);
  remove("test_build_dir/bad_gui1/src/bad_gui_gui.h");

  /* Case 2: fp_c fails to open because it is a directory (while fp_h succeeds
   * and must be closed) */
  makedirs("test_build_dir/bad_gui2/src/bad_gui_gui.c");
  config.filename_base =
      (char *)(size_t)(size_t) "test_build_dir/bad_gui2/bad_gui";
  rc = openapi_client_gui_generate(&spec, &config);
  ASSERT_EQ(CDD_C_ERROR_IO, rc);
  remove("test_build_dir/bad_gui2/src/bad_gui_gui.c");
  remove("test_build_dir/bad_gui2/src/bad_gui_gui.h");

  PASS();
}

/**
 * @brief Client GUI generation test suite.
 */
SUITE(client_gui_gen_suite) {
  RUN_TEST(test_client_gui_gen_basic);
  RUN_TEST(test_client_gui_gen_with_server);
  RUN_TEST(test_client_gui_gen_errors);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !TEST_CLIENT_GUI_GEN_H */
