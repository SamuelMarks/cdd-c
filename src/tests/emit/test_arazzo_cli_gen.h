/**
 * @file test_arazzo_cli_gen.h
 * @brief Unit tests for Arazzo C SDK CLI Workflow Runner generator
 * (`arazzo_cli_gen.c`).
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_TEST_ARAZZO_CLI_GEN_H
/** @brief Header guard for test_arazzo_cli_gen.h */
#define C_CDD_TEST_ARAZZO_CLI_GEN_H

/* clang-format off */
#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arazzo/parse/arazzo.h"
#include "c_cdd/memory.h"
#include "routes/emit/arazzo_cli_gen.h"
#include "cdd_test_helpers/cdd_helpers.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief Tests `arazzo_cli_render_to_string` and `arazzo_cli_generate`.
 */
TEST test_arazzo_cli_gen_basic(void) {
  struct Arazzo_Spec spec;
  struct OpenApiClientConfig cfg;
  char *cli_src = NULL;
  char base_path[512];
  const char *arazzo_json =
      "{"
      "\"arazzo\":\"1.1.0\","
      "\"info\":{\"title\":\"CLI Flow\",\"version\":\"1.0.0\"},"
      "\"sourceDescriptions\":[{\"name\":\"api\",\"url\":\"./openapi.json\"}],"
      "\"workflows\":[{"
      "\"workflowId\":\"runJob\","
      "\"summary\":\"Run job\","
      "\"steps\":[{\"stepId\":\"s1\",\"operationId\":\"startJob\"}]"
      "}]"
      "}";

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_cli_render_to_string(NULL, "b", "", &cli_src));
  memset(&cfg, 0, sizeof(cfg));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_cli_generate(NULL, NULL, &cfg));

  memset(&spec, 0, sizeof(spec));
  ASSERT_EQ(CDD_C_SUCCESS, arazzo_load_from_json(arazzo_json, &spec));

  ASSERT_EQ(CDD_C_SUCCESS,
            arazzo_cli_render_to_string(&spec, "job", "api_", &cli_src));
  ASSERT(cli_src != NULL);
  ASSERT(strstr(cli_src, "#include \"job_workflows.h\"") != NULL);
  ASSERT(strstr(cli_src, "api_workflow_runJob(&clients, &in_s, &out_s)") !=
         NULL);
  C_CDD_FREE(cli_src);

  strcpy(base_path, "test_build_dir/arazzo_job/job");
  memset(&cfg, 0, sizeof(cfg));
  cfg.filename_base = base_path;
  cfg.func_prefix = "api_";
  ASSERT_EQ(CDD_C_SUCCESS, arazzo_cli_generate(&spec, NULL, &cfg));
  remove("test_build_dir/arazzo_job/src/job_workflows.h");
  remove("test_build_dir/arazzo_job/src/job_workflows.c");
  remove("test_build_dir/arazzo_job/src/job_workflow_cli.c");

  arazzo_spec_free(&spec);
  PASS();
}

/**
 * @brief Suite for Arazzo C SDK CLI generation tests.
 */
SUITE(arazzo_cli_gen_suite) { RUN_TEST(test_arazzo_cli_gen_basic); }

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_TEST_ARAZZO_CLI_GEN_H */
