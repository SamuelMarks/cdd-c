/**
 * @file test_arazzo_server_gen.h
 * @brief Unit tests for Arazzo Backend Server Orchestrator & E2E Test Suite
 * generator (`arazzo_server_gen.c`) and public API/CLI
 * (`cdd_generate_*_arazzo`).
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_TEST_ARAZZO_SERVER_GEN_H
/** @brief Header guard for test_arazzo_server_gen.h */
#define C_CDD_TEST_ARAZZO_SERVER_GEN_H

/* clang-format off */
#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arazzo/parse/arazzo.h"
#include "c_cdd/memory.h"
#include "cdd_api.h"
#include "cdd_test_helpers/cdd_helpers.h"
#include "functions/parse/fs.h"
#include "functions/parse/main.h"
#include "routes/emit/arazzo_server_gen.h"
#include "routes/emit/serve_json_rpc.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief Tests `arazzo_server_render_to_strings`, `arazzo_server_generate`,
 * `cdd_generate_from_arazzo`, `cdd_generate_to_arazzo`, and `cdd_main`.
 */
TEST test_arazzo_server_gen_and_api(void) {
  struct Arazzo_Spec spec;
  struct OpenApiClientConfig cfg;
  char *srv_src = NULL;
  char *tst_hdr = NULL;
  char base_path[512];
  char spec_path[512];
  char oa_path[512];
  char out_spec_path[512];
  const char *arazzo_json =
      "{"
      "\"arazzo\":\"1.1.0\","
      "\"info\":{\"title\":\"Server Flow\",\"version\":\"1.0.0\"},"
      "\"sourceDescriptions\":[{\"name\":\"api\",\"url\":\"./openapi.json\"}],"
      "\"workflows\":[{"
      "\"workflowId\":\"syncOrder\","
      "\"description\":\"Sync order workflow\","
      "\"steps\":[{\"stepId\":\"s1\",\"operationId\":\"postOrder\"}]"
      "}]"
      "}";
  const char *oa_json =
      "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"T\",\"version\":\"1.0\"},"
      "\"paths\":{\"/orders\":{\"post\":{\"operationId\":\"postOrder\","
      "\"responses\":{\"200\":{\"description\":\"ok\"}}}}}}";

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_server_render_to_strings(NULL, "b", "", &srv_src, &tst_hdr));
  memset(&cfg, 0, sizeof(cfg));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_server_generate(NULL, NULL, &cfg));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_generate_from_arazzo(NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_generate_to_arazzo(NULL));

  memset(&spec, 0, sizeof(spec));
  ASSERT_EQ(CDD_C_SUCCESS, arazzo_load_from_json(arazzo_json, &spec));

  ASSERT_EQ(CDD_C_SUCCESS, arazzo_server_render_to_strings(&spec, "ord", "api_",
                                                           &srv_src, &tst_hdr));
  ASSERT(srv_src != NULL);
  ASSERT(tst_hdr != NULL);
  ASSERT(strstr(srv_src, "handle_workflow_syncOrder") != NULL);
  ASSERT(strstr(tst_hdr, "TEST test_workflow_syncOrder(void)") != NULL);
  C_CDD_FREE(srv_src);
  C_CDD_FREE(tst_hdr);

  ASSERT_EQ(CDD_C_SUCCESS, makedirs("test_build_dir/arazzo_ord/src"));
  strcpy(base_path, "test_build_dir/arazzo_ord/ord");
  strcpy(spec_path, "test_build_dir/arazzo_ord/arazzo_in.json");
  strcpy(oa_path, "test_build_dir/arazzo_ord/arazzo_oa.json");
  strcpy(out_spec_path, "test_build_dir/arazzo_ord/arazzo_extracted.json");

  ASSERT_EQ(CDD_C_SUCCESS, fs_write_to_file(spec_path, arazzo_json));
  ASSERT_EQ(CDD_C_SUCCESS, fs_write_to_file(oa_path, oa_json));

  memset(&cfg, 0, sizeof(cfg));
  cfg.filename_base = base_path;
  cfg.func_prefix = "api_";
  ASSERT_EQ(CDD_C_SUCCESS, arazzo_server_generate(&spec, NULL, &cfg));

  {
    cdd_from_arazzo_config_t fcfg;
    memset(&fcfg, 0, sizeof(fcfg));
    fcfg.input = "nonexistent_arazzo_file.json";
    ASSERT_EQ(CDD_C_ERROR_IO, cdd_generate_from_arazzo(&fcfg));

    fcfg.input = spec_path;
    fcfg.openapi_input = oa_path;
    fcfg.output = base_path;
    fcfg.subcommand = "to_sdk";
    ASSERT_EQ(CDD_C_SUCCESS, cdd_generate_from_arazzo(&fcfg));

    fcfg.openapi_input = NULL;
    fcfg.subcommand = "to_sdk_cli";
    ASSERT_EQ(CDD_C_SUCCESS, cdd_generate_from_arazzo(&fcfg));

    fcfg.subcommand = "to_server";
    ASSERT_EQ(CDD_C_SUCCESS, cdd_generate_from_arazzo(&fcfg));

    fcfg.subcommand = "invalid_subcmd";
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_generate_from_arazzo(&fcfg));
  }

  {
    cdd_to_arazzo_config_t tcfg;
    char *argv_help1[4];
    char *argv_help2[4];
    char *argv_from[10];

    argv_help1[0] = C_CDD_STR_LIT("cdd-c");
    argv_help1[1] = C_CDD_STR_LIT("from_arazzo");
    argv_help1[2] = C_CDD_STR_LIT("--help");
    argv_help1[3] = NULL;

    argv_help2[0] = C_CDD_STR_LIT("cdd-c");
    argv_help2[1] = C_CDD_STR_LIT("to_arazzo");
    argv_help2[2] = C_CDD_STR_LIT("--help");
    argv_help2[3] = NULL;

    argv_from[0] = C_CDD_STR_LIT("cdd-c");
    argv_from[1] = C_CDD_STR_LIT("from_arazzo");
    argv_from[2] = C_CDD_STR_LIT("to_sdk");
    argv_from[3] = C_CDD_STR_LIT("-i");
    argv_from[4] = spec_path;
    argv_from[5] = C_CDD_STR_LIT("--openapi");
    argv_from[6] = oa_path;
    argv_from[7] = C_CDD_STR_LIT("-o");
    argv_from[8] = base_path;
    argv_from[9] = NULL;

    ASSERT_EQ(CDD_C_SUCCESS, cdd_main(3, argv_help1));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_main(3, argv_help2));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_main(9, argv_from));

    memset(&tcfg, 0, sizeof(tcfg));
    tcfg.input = spec_path;
    tcfg.output = out_spec_path;
    tcfg.self_uri = "https://example.com/self.json";
    ASSERT_EQ(CDD_C_SUCCESS, cdd_generate_to_arazzo(&tcfg));
  }

  remove("test_build_dir/arazzo_ord/src/ord_workflows.h");
  remove("test_build_dir/arazzo_ord/src/ord_workflows.c");
  remove("test_build_dir/arazzo_ord/src/ord_workflow_cli.c");
  remove("test_build_dir/arazzo_ord/src/ord_workflow_server.c");
  remove("test_build_dir/arazzo_ord/src/ord_workflow_tests.h");
  remove("test_build_dir/arazzo_ord/src/ord.h");
  remove("test_build_dir/arazzo_ord/src/ord.c");
  remove("test_build_dir/arazzo_ord/src/ord_models.h");
  remove("test_build_dir/arazzo_ord/src/ord_models.c");
  remove("test_build_dir/arazzo_ord/src/url_utils.h");
  remove("test_build_dir/arazzo_ord/src/url_utils.c");
  remove("test_build_dir/arazzo_ord/src/CMakeLists.txt");
  remove("test_build_dir/arazzo_ord/CMakeLists.txt");
  remove(spec_path);
  remove(oa_path);
  remove(out_spec_path);
  arazzo_spec_free(&spec);
  PASS();
}

/**
 * @brief Suite for Arazzo Backend Server & API generation tests.
 */
SUITE(arazzo_server_gen_suite) { RUN_TEST(test_arazzo_server_gen_and_api); }

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_TEST_ARAZZO_SERVER_GEN_H */
