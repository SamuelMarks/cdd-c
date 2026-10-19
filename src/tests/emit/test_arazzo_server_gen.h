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
#include "c_cdd/safe_crt.h"
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

  memset(&spec, 0, sizeof(spec));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_server_render_to_strings(NULL, "b", "", &srv_src, &tst_hdr));
  ASSERT_EQ(
      CDD_C_ERROR_INVALID_ARGUMENT,
      arazzo_server_render_to_strings(&spec, NULL, "", &srv_src, &tst_hdr));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_server_render_to_strings(&spec, "b", "", NULL, &tst_hdr));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_server_render_to_strings(&spec, "b", "", &srv_src, NULL));
  memset(&cfg, 0, sizeof(cfg));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_server_generate(NULL, NULL, &cfg));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_server_generate(&spec, NULL, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_server_generate(&spec, NULL, &cfg));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_generate_from_arazzo(NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_generate_to_arazzo(NULL));

  {
    struct Arazzo_Spec min_spec;
    struct Arazzo_Workflow min_wf;
    struct Arazzo_Step min_st;
    memset(&min_spec, 0, sizeof(min_spec));
    memset(&min_wf, 0, sizeof(min_wf));
    memset(&min_st, 0, sizeof(min_st));
    min_wf.steps = &min_st;
    min_wf.n_steps = 1;
    min_spec.workflows = &min_wf;
    min_spec.n_workflows = 1;
    ASSERT_EQ(CDD_C_SUCCESS, arazzo_server_render_to_strings(
                                 &min_spec, "min", NULL, &srv_src, &tst_hdr));
    C_CDD_FREE(srv_src);
    C_CDD_FREE(tst_hdr);
    srv_src = NULL;
    tst_hdr = NULL;
  }

  memset(&spec, 0, sizeof(spec));
  ASSERT_EQ(CDD_C_SUCCESS, arazzo_load_from_json(arazzo_json, &spec));

  {
    int f;
    for (f = 1; f <= 20; ++f) {
      g_cdd_alloc_fail = f;
      if (arazzo_server_render_to_strings(&spec, "ord", "api_", &srv_src,
                                          &tst_hdr) == CDD_C_SUCCESS) {
        C_CDD_FREE(srv_src);
        C_CDD_FREE(tst_hdr);
        srv_src = NULL;
        tst_hdr = NULL;
      }
      g_cdd_alloc_fail = 0;
    }
  }

  ASSERT_EQ(CDD_C_SUCCESS, arazzo_server_render_to_strings(&spec, "ord", "api_",
                                                           &srv_src, &tst_hdr));
  ASSERT(srv_src != NULL);
  ASSERT(tst_hdr != NULL);
  ASSERT(strstr(srv_src, "handle_workflow_syncOrder") != NULL);
  ASSERT(strstr(tst_hdr, "TEST test_workflow_syncOrder(void)") != NULL);
  C_CDD_FREE(srv_src);
  C_CDD_FREE(tst_hdr);

  ASSERT_EQ(CDD_C_SUCCESS, makedirs("test_build_dir/arazzo_ord/src"));
  CDD_STRCPY(base_path, sizeof(base_path), "test_build_dir/arazzo_ord/ord");
  CDD_STRCPY(spec_path, sizeof(spec_path),
             "test_build_dir/arazzo_ord/arazzo_in.json");
  CDD_STRCPY(oa_path, sizeof(oa_path),
             "test_build_dir/arazzo_ord/arazzo_oa.json");
  CDD_STRCPY(out_spec_path, sizeof(out_spec_path),
             "test_build_dir/arazzo_ord/arazzo_extracted.json");

  ASSERT_EQ(CDD_C_SUCCESS, fs_write_to_file(spec_path, arazzo_json));
  ASSERT_EQ(CDD_C_SUCCESS, fs_write_to_file(oa_path, oa_json));

  memset(&cfg, 0, sizeof(cfg));
  cfg.filename_base = base_path;
  cfg.func_prefix = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, arazzo_server_generate(&spec, NULL, &cfg));
  cfg.func_prefix = "api_";
  {
    int f;
    for (f = 1; f <= 25; ++f) {
      g_cdd_alloc_fail = f;
      arazzo_server_generate(&spec, NULL, &cfg);
      g_cdd_alloc_fail = 0;
    }
    for (f = 1; f <= 10; ++f) {
      g_cdd_strdup_fail = f;
      arazzo_server_generate(&spec, NULL, &cfg);
      g_cdd_strdup_fail = 0;
    }
    for (f = 0; f <= 4; ++f) {
      g_fail_io_after = f;
      arazzo_server_generate(&spec, NULL, &cfg);
      g_fail_io_after = -1;
    }
  }
  ASSERT_EQ(CDD_C_SUCCESS, arazzo_server_generate(&spec, NULL, &cfg));

  {
    cdd_from_arazzo_config_t fcfg;
    memset(&fcfg, 0, sizeof(fcfg));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_generate_from_arazzo(&fcfg));
    fcfg.input = "nonexistent_arazzo_file.json";
    ASSERT_EQ(CDD_C_ERROR_IO, cdd_generate_from_arazzo(&fcfg));

    fcfg.input = oa_path; /* Valid JSON, invalid Arazzo */
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_generate_from_arazzo(&fcfg));

    fcfg.input = spec_path;
    fcfg.openapi_input = "nonexistent_oa.json";
    fcfg.output = NULL;
    fcfg.func_prefix = "my_";
    fcfg.subcommand = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_generate_from_arazzo(&fcfg));
    remove("src/generated_arazzo_workflows.h");
    remove("src/generated_arazzo_workflows.c");

    fcfg.openapi_input = spec_path; /* Valid JSON, invalid OpenAPI */
    fcfg.output = base_path;
    fcfg.func_prefix = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_generate_from_arazzo(&fcfg));

    fcfg.openapi_input = oa_path;
    fcfg.subcommand = "to_sdk";
    ASSERT_EQ(CDD_C_SUCCESS, cdd_generate_from_arazzo(&fcfg));
    g_fail_io_after = 0;
    ASSERT_NEQ(CDD_C_SUCCESS, cdd_generate_from_arazzo(&fcfg));
    g_fail_io_after = -1;
    fcfg.subcommand = "to_sdk_cli";
    ASSERT_EQ(CDD_C_SUCCESS, cdd_generate_from_arazzo(&fcfg));
    fcfg.subcommand = "to_server";
    ASSERT_EQ(CDD_C_SUCCESS, cdd_generate_from_arazzo(&fcfg));
    g_fail_io_after = 0;
    ASSERT_NEQ(CDD_C_SUCCESS, cdd_generate_from_arazzo(&fcfg));
    g_fail_io_after = -1;

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
    char *argv_help1_short[4];
    char *argv_help2[4];
    char *argv_help2_short[4];
    char *argv_from[10];
    char *argv_from_long[11];
    char *argv_from_srv[8];
    char *argv_from_no_sub[7];
    char *argv_from_bad_sub[5];
    char *argv_from_empty[3];
    char *argv_to[7];
    char *argv_c2a[7];
    char *argv_c2a_err[3];
    char *argv_to_err[3];
    char *argv_foa_h[4];
    int f;

    argv_help1[0] = C_CDD_STR_LIT("cdd-c");
    argv_help1[1] = C_CDD_STR_LIT("from_arazzo");
    argv_help1[2] = C_CDD_STR_LIT("--help");
    argv_help1[3] = NULL;

    argv_help1_short[0] = C_CDD_STR_LIT("cdd-c");
    argv_help1_short[1] = C_CDD_STR_LIT("from_arazzo");
    argv_help1_short[2] = C_CDD_STR_LIT("-h");
    argv_help1_short[3] = NULL;

    argv_help2[0] = C_CDD_STR_LIT("cdd-c");
    argv_help2[1] = C_CDD_STR_LIT("to_arazzo");
    argv_help2[2] = C_CDD_STR_LIT("--help");
    argv_help2[3] = NULL;

    argv_help2_short[0] = C_CDD_STR_LIT("cdd-c");
    argv_help2_short[1] = C_CDD_STR_LIT("to_arazzo");
    argv_help2_short[2] = C_CDD_STR_LIT("-h");
    argv_help2_short[3] = NULL;

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

    argv_from_long[0] = C_CDD_STR_LIT("cdd-c");
    argv_from_long[1] = C_CDD_STR_LIT("from_arazzo");
    argv_from_long[2] = C_CDD_STR_LIT("to_sdk_cli");
    argv_from_long[3] = C_CDD_STR_LIT("--input");
    argv_from_long[4] = spec_path;
    argv_from_long[5] = C_CDD_STR_LIT("--output");
    argv_from_long[6] = base_path;
    argv_from_long[7] = C_CDD_STR_LIT("--openapi");
    argv_from_long[8] = oa_path;
    argv_from_long[9] = C_CDD_STR_LIT("--unknown");
    argv_from_long[10] = NULL;

    argv_from_srv[0] = C_CDD_STR_LIT("cdd-c");
    argv_from_srv[1] = C_CDD_STR_LIT("from_arazzo");
    argv_from_srv[2] = C_CDD_STR_LIT("to_server");
    argv_from_srv[3] = C_CDD_STR_LIT("-i");
    argv_from_srv[4] = spec_path;
    argv_from_srv[5] = C_CDD_STR_LIT("-o");
    argv_from_srv[6] = base_path;
    argv_from_srv[7] = NULL;

    argv_from_no_sub[0] = C_CDD_STR_LIT("cdd-c");
    argv_from_no_sub[1] = C_CDD_STR_LIT("from_arazzo");
    argv_from_no_sub[2] = C_CDD_STR_LIT("-i");
    argv_from_no_sub[3] = spec_path;
    argv_from_no_sub[4] = C_CDD_STR_LIT("-o");
    argv_from_no_sub[5] = base_path;
    argv_from_no_sub[6] = NULL;

    argv_from_bad_sub[0] = C_CDD_STR_LIT("cdd-c");
    argv_from_bad_sub[1] = C_CDD_STR_LIT("from_arazzo");
    argv_from_bad_sub[2] = C_CDD_STR_LIT("bad_sub");
    argv_from_bad_sub[3] = C_CDD_STR_LIT("--openapi");
    argv_from_bad_sub[4] = NULL;

    argv_from_empty[0] = C_CDD_STR_LIT("cdd-c");
    argv_from_empty[1] = C_CDD_STR_LIT("from_arazzo");
    argv_from_empty[2] = NULL;

    argv_to[0] = C_CDD_STR_LIT("cdd-c");
    argv_to[1] = C_CDD_STR_LIT("to_arazzo");
    argv_to[2] = C_CDD_STR_LIT("-i");
    argv_to[3] = C_CDD_STR_LIT("test_build_dir/arazzo_ord/src");
    argv_to[4] = C_CDD_STR_LIT("-o");
    argv_to[5] = out_spec_path;
    argv_to[6] = NULL;

    argv_c2a[0] = C_CDD_STR_LIT("cdd-c");
    argv_c2a[1] = C_CDD_STR_LIT("c2arazzo");
    argv_c2a[2] = C_CDD_STR_LIT("-i");
    argv_c2a[3] = C_CDD_STR_LIT("test_build_dir/arazzo_ord/src");
    argv_c2a[4] = C_CDD_STR_LIT("-o");
    argv_c2a[5] = out_spec_path;
    argv_c2a[6] = NULL;

    argv_c2a_err[0] = C_CDD_STR_LIT("cdd-c");
    argv_c2a_err[1] = C_CDD_STR_LIT("c2arazzo");
    argv_c2a_err[2] = NULL;

    argv_to_err[0] = C_CDD_STR_LIT("cdd-c");
    argv_to_err[1] = C_CDD_STR_LIT("to_arazzo");
    argv_to_err[2] = NULL;

    argv_foa_h[0] = C_CDD_STR_LIT("cdd-c");
    argv_foa_h[1] = C_CDD_STR_LIT("from_openapi");
    argv_foa_h[2] = C_CDD_STR_LIT("-h");
    argv_foa_h[3] = NULL;

    ASSERT_EQ(CDD_C_SUCCESS, cdd_main(3, argv_help1));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_main(3, argv_help1_short));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_main(3, argv_help2));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_main(3, argv_help2_short));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_main(9, argv_from));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_main(10, argv_from_long));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_main(7, argv_from_srv));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_main(6, argv_from_no_sub));
    ASSERT_NEQ(CDD_C_SUCCESS, cdd_main(4, argv_from_bad_sub));
    ASSERT_NEQ(CDD_C_SUCCESS, cdd_main(2, argv_from_empty));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_main(6, argv_to));
    remove(out_spec_path);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_main(6, argv_c2a));
    remove(out_spec_path);
    ASSERT_NEQ(CDD_C_SUCCESS, cdd_main(2, argv_c2a_err));
    ASSERT_NEQ(CDD_C_SUCCESS, cdd_main(2, argv_to_err));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_main(3, argv_foa_h));

    memset(&tcfg, 0, sizeof(tcfg));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_generate_to_arazzo(&tcfg));
    tcfg.input = "nonexistent_arazzo_src.c";
    tcfg.output = out_spec_path;
    ASSERT_NEQ(CDD_C_SUCCESS, cdd_generate_to_arazzo(&tcfg));

    tcfg.input = "test_build_dir/arazzo_ord/src/ord_workflows.c";
    tcfg.output = NULL;
    tcfg.self_uri = "";
    ASSERT_EQ(CDD_C_SUCCESS, cdd_generate_to_arazzo(&tcfg));
    remove("arazzo.json");

    ASSERT_EQ(
        CDD_C_SUCCESS,
        fs_write_to_file("test_build_dir/arazzo_ord/tiny.c", "int x = 1;\n"));
    tcfg.input = "test_build_dir/arazzo_ord/tiny.c";
    tcfg.output = out_spec_path;
    tcfg.self_uri = "https://example.com/self.json";
    for (f = 1; f <= 25; ++f) {
      g_cdd_alloc_fail = f;
      cdd_generate_to_arazzo(&tcfg);
      g_cdd_alloc_fail = 0;
    }
    for (f = 1; f <= 25; ++f) {
      g_cdd_strdup_fail = f;
      cdd_generate_to_arazzo(&tcfg);
      g_cdd_strdup_fail = 0;
    }
    ASSERT_EQ(CDD_C_SUCCESS, cdd_generate_to_arazzo(&tcfg));
    remove("test_build_dir/arazzo_ord/tiny.c");
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
