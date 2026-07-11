#ifndef TEST_CDD_API_H
#define TEST_CDD_API_H

/* clang-format off */
#include "cdd_api.h"
#include <greatest.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

TEST test_cdd_generate_from_openapi(void) {
  cdd_from_openapi_config_t config = {0};

  config.input = "nonexistent.json";
  config.output = "out";
  config.subcommand = "to_sdk";
  cdd_generate_from_openapi(&config);

  config.subcommand = NULL;
  config.input = NULL;
  config.input_dir = "nonexistent_dir";
  config.no_github_actions = 1;
  config.no_installable_package = 1;
  config.tests = 1;
  cdd_generate_from_openapi(&config);

  /* Cover branches where flags are false but input_dir is not set, input not
   * set */
  config.input_dir = NULL;
  config.no_github_actions = 0;
  config.no_installable_package = 0;
  config.tests = 0;
  config.output = NULL;
  cdd_generate_from_openapi(&config);

  PASS();
}

TEST test_cdd_generate_to_openapi(void) {
  cdd_to_openapi_config_t config = {0};

  config.input = "nonexistent.c";
  config.output = "out.json";
  cdd_generate_to_openapi(&config);

  config.input = NULL;
  config.output = NULL;
  cdd_generate_to_openapi(&config);

  PASS();
}

TEST test_cdd_generate_docs_json(void) {
  cdd_docs_json_config_t config = {0};

  config.input = "nonexistent.json";
  config.output = "out.json";
  config.no_imports = 1;
  config.no_wrapping = 1;
  cdd_generate_docs_json(&config);

  config.input = NULL;
  config.no_imports = 0;
  config.no_wrapping = 0;
  config.output = NULL;
  cdd_generate_docs_json(&config);

  PASS();
}

TEST test_cdd_serve_json_rpc(void) {
  cdd_serve_json_rpc_config_t config = {0};

  config.port = 1; /* port > 0 */
  config.listen_host = "255";
  cdd_serve_json_rpc(&config);

  config.port = 0;
  config.listen_host = NULL;
  cdd_serve_json_rpc(&config);

  PASS();
}

SUITE(cdd_api_suite) {
  RUN_TEST(test_cdd_generate_from_openapi);
  RUN_TEST(test_cdd_generate_to_openapi);
  RUN_TEST(test_cdd_generate_docs_json);
  RUN_TEST(test_cdd_serve_json_rpc);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
