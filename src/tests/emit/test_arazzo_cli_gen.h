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
#include "c_cdd/safe_crt.h"
#include "routes/emit/arazzo_cli_gen.h"
#include "cdd_test_helpers/cdd_helpers.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief Tests `arazzo_cli_render_to_string` and `arazzo_cli_generate`.
 * @return Test or helper result.
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

  memset(&spec, 0, sizeof(spec));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_cli_render_to_string(NULL, "b", "", &cli_src));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_cli_render_to_string(&spec, NULL, "", &cli_src));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_cli_render_to_string(&spec, "b", "", NULL));
  memset(&cfg, 0, sizeof(cfg));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_cli_generate(NULL, NULL, &cfg));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_cli_generate(&spec, NULL, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_cli_generate(&spec, NULL, &cfg));

  {
    struct Arazzo_Spec min_spec;
    struct Arazzo_Workflow min_wf;
    memset(&min_spec, 0, sizeof(min_spec));
    memset(&min_wf, 0, sizeof(min_wf));
    min_spec.workflows = &min_wf;
    min_spec.n_workflows = 1;
    ASSERT_EQ(CDD_C_SUCCESS,
              arazzo_cli_render_to_string(&min_spec, "min", NULL, &cli_src));
    C_CDD_FREE(cli_src);
    cli_src = NULL;
  }

  memset(&spec, 0, sizeof(spec));
  ASSERT_EQ(CDD_C_SUCCESS, arazzo_load_from_json(arazzo_json, &spec));

  {
    int f;
    for (f = 1; f <= 15; ++f) {
      g_cdd_alloc_fail = f;
      if (arazzo_cli_render_to_string(&spec, "job", "api_", &cli_src) ==
          CDD_C_SUCCESS) {
        C_CDD_FREE(cli_src);
        cli_src = NULL;
      }
      g_cdd_alloc_fail = 0;
    }
  }

  ASSERT_EQ(CDD_C_SUCCESS,
            arazzo_cli_render_to_string(&spec, "job", "api_", &cli_src));
  ASSERT(cli_src != NULL);
  ASSERT(strstr(cli_src, "#include \"job_workflows.h\"") != NULL);
  ASSERT(strstr(cli_src, "api_workflow_runJob(&clients, &in_s, &out_s)") !=
         NULL);
  C_CDD_FREE(cli_src);

  CDD_STRCPY(base_path, sizeof(base_path), "test_build_dir/arazzo_job/job");
  memset(&cfg, 0, sizeof(cfg));
  cfg.filename_base = base_path;
  cfg.func_prefix = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, arazzo_cli_generate(&spec, NULL, &cfg));
  cfg.func_prefix = "api_";
  {
    int f;
    for (f = 1; f <= 20; ++f) {
      g_cdd_alloc_fail = f;
      arazzo_cli_generate(&spec, NULL, &cfg);
      g_cdd_alloc_fail = 0;
    }
    for (f = 1; f <= 10; ++f) {
      g_cdd_strdup_fail = f;
      arazzo_cli_generate(&spec, NULL, &cfg);
      g_cdd_strdup_fail = 0;
    }
    for (f = 0; f <= 3; ++f) {
      g_fail_io_after = f;
      arazzo_cli_generate(&spec, NULL, &cfg);
      g_fail_io_after = -1;
    }
  }
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
