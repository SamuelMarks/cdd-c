/**
 * @file test_arazzo_client_gen.h
 * @brief Unit tests for Arazzo C Client SDK generator (`arazzo_client_gen.c`)
 * and round-trip extraction via `c2arazzo`.
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_TEST_ARAZZO_CLIENT_GEN_H
/** @brief Header guard for test_arazzo_client_gen.h */
#define C_CDD_TEST_ARAZZO_CLIENT_GEN_H

/* clang-format off */
#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "arazzo/parse/arazzo.h"
#include "arazzo/parse/c2arazzo.h"
#include "c_cdd/memory.h"
#include "c_cdd/safe_crt.h"
#include "cdd_test_helpers/cdd_helpers.h"
#include "routes/emit/arazzo_client_gen.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief Tests `arazzo_client_render_to_strings`, `arazzo_client_generate`, and
 * bidirectional round-trip extraction (`Arazzo -> C SDK -> Arazzo`).
 */
TEST test_arazzo_client_gen_and_roundtrip(void) {
  struct Arazzo_Spec spec;
  struct Arazzo_Spec roundtrip;
  struct OpenApiClientConfig cfg;
  char *hdr = NULL;
  char *src = NULL;
  char base_path[512];
  char arazzo_json[4096];
  const char *chunks[] = {
      "{\"arazzo\":\"1.1.0\",\"$self\":\"https://example.com/arazzo.json\","
      "\"x-root\":true,\"info\":{\"title\":\"Checkout Workflows\","
      "\"version\":\"1.0.0\",\"summary\":\"Checkout summary\","
      "\"description\":\"Checkout desc\"},",
      "\"sourceDescriptions\":["
      "{\"name\":\"petstore\",\"url\":\"./openapi.json\",\"type\":\"openapi\"},"
      "{\"name\":\"untyped\",\"url\":\"./other.json\"}],",
      "\"components\":{\"parameters\":{\"sharedParam\":{\"name\":\"sp\","
      "\"in\":\"query\",\"value\":\"1\"}},\"successActions\":{\"doneAct\":{"
      "\"name\":\"doneAct\",\"type\":\"end\"}},\"failureActions\":{\"errAct\":{"
      "\"name\":\"errAct\",\"type\":\"end\"}}},",
      "\"workflows\":["
      "{\"workflowId\":\"auth\",\"steps\":[{\"stepId\":\"authStep\","
      "\"operationId\":\"login\"}]},"
      "{\"workflowId\":\"nestedWf\",\"steps\":[{\"stepId\":\"nStep\","
      "\"operationId\":\"nestedOp\"}]},"
      "{\"workflowId\":\"otherWf\",\"steps\":[{\"stepId\":\"oStep\","
      "\"operationId\":\"otherOp\"}]},",
      "{\"workflowId\":\"checkout\",\"summary\":\"Checkout flow\","
      "\"description\":\"Full checkout\",\"dependsOn\":[\"auth\"],"
      "\"inputs\":{\"type\":\"object\",\"properties\":{\"userId\":{\"type\":"
      "\"string\"}}},",
      "\"parameters\":["
      "{\"name\":\"apiKey\",\"in\":\"header\",\"value\":\"$inputs.userId\"},"
      "{\"name\":\"plainParam\",\"in\":\"query\",\"value\":\"123\"},"
      "{\"reference\":\"$components.parameters.sharedParam\",\"value\":\"456\"}"
      "],\"outputs\":{\"orderId\":\"$steps.s1.outputs.id\"},\"steps\":[",
      "{\"stepId\":\"s1\",\"description\":\"Create order\","
      "\"operationId\":\"createOrder\",\"timeout\":3000,"
      "\"correlationId\":\"$message.header#/cid\",\"action\":\"send\","
      "\"parameters\":["
      "{\"name\":\"p1\",\"in\":\"query\",\"value\":\"$inputs.userId\"},"
      "{\"name\":\"p2\",\"in\":\"query\",\"value\":\"val\"},"
      "{\"reference\":\"$components.parameters.sharedParam\"}],",
      "\"requestBody\":{\"contentType\":\"application/json\","
      "\"payload\":{\"item\":\"book\"},\"replacements\":["
      "{\"target\":\"/item\",\"value\":\"$inputs.userId\"},"
      "{\"target\":\"/qty\",\"value\":2}]},",
      "\"successCriteria\":["
      "{\"condition\":\"$statusCode == 200\"},"
      "{\"context\":\"$response.body\",\"condition\":\"^ok$\",\"type\":"
      "\"regex\"},"
      "{\"context\":\"$response.body\",\"condition\":\"$.id\",\"type\":"
      "\"jsonpath\"},"
      "{\"context\":\"$response.body\",\"condition\":\"/root\",\"type\":"
      "\"xpath\"}],",
      "\"onSuccess\":["
      "{\"name\":\"nextStep\",\"type\":\"goto\",\"stepId\":\"s2\"},"
      "{\"reference\":\"$components.successActions.doneAct\"}],"
      "\"onFailure\":["
      "{\"name\":\"retryOrder\",\"type\":\"retry\",\"retryAfter\":1.0,"
      "\"retryLimit\":2,\"stepId\":\"s1\"},"
      "{\"name\":\"fallback\",\"type\":\"goto\",\"stepId\":\"s2\"},"
      "{\"reference\":\"$components.failureActions.errAct\"}],"
      "\"outputs\":{\"id\":\"$response.body#/id\"}},",
      "{\"stepId\":\"s2\",\"dependsOn\":[\"s1\"],"
      "\"operationPath\":\"{$sourceDescriptions.petstore.url}#/paths/~1orders/"
      "get\",\"action\":\"receive\","
      "\"requestBody\":{\"payload\":\"$inputs.userId\"}},"
      "{\"stepId\":\"s3\",\"channelPath\":\"/orders/events\"},"
      "{\"stepId\":\"s4\",\"workflowId\":\"nestedWf\"}"
      "]}]}",
      NULL};
  size_t idx = 0;

  arazzo_json[0] = '\0';
  while (chunks[idx]) {
    CDD_STRCAT(arazzo_json, sizeof(arazzo_json), chunks[idx]);
    idx++;
  }

  memset(&spec, 0, sizeof(spec));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_client_render_to_strings(NULL, NULL, "b", "", &hdr, &src));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_client_render_to_strings(&spec, NULL, NULL, "", &hdr, &src));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_client_render_to_strings(&spec, NULL, "b", "", NULL, &src));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_client_render_to_strings(&spec, NULL, "b", "", &hdr, NULL));
  memset(&cfg, 0, sizeof(cfg));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_client_generate(NULL, NULL, &cfg));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_client_generate(&spec, NULL, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_client_generate(&spec, NULL, &cfg));

  {
    struct Arazzo_Spec min_spec;
    struct Arazzo_SourceDescription min_sd;
    struct Arazzo_Extension min_ext;
    struct Arazzo_Workflow min_wf;
    struct Arazzo_Step min_st;
    struct Arazzo_Parameter min_p;
    struct Arazzo_OutputMapEntry min_out;
    struct Arazzo_Criterion min_cr;
    struct Arazzo_SuccessAction min_sa[2];
    struct Arazzo_FailureAction min_fa[3];
    struct Arazzo_PayloadReplacement min_rep;
    memset(&min_spec, 0, sizeof(min_spec));
    memset(&min_sd, 0, sizeof(min_sd));
    memset(&min_ext, 0, sizeof(min_ext));
    memset(&min_wf, 0, sizeof(min_wf));
    memset(&min_st, 0, sizeof(min_st));
    memset(&min_p, 0, sizeof(min_p));
    memset(&min_out, 0, sizeof(min_out));
    memset(&min_cr, 0, sizeof(min_cr));
    memset(min_sa, 0, sizeof(min_sa));
    memset(min_fa, 0, sizeof(min_fa));
    memset(&min_rep, 0, sizeof(min_rep));
    min_ext.key = C_CDD_STR_LIT("x-empty");
    min_p.name = C_CDD_STR_LIT("mp");
    min_p.in = ARAZZO_PARAM_IN_UNKNOWN;
    min_out.key = C_CDD_STR_LIT("mok");
    min_cr.type = ARAZZO_CRITERION_EXPRESSION_TYPE;
    min_cr.expression_type.type = ARAZZO_DIALECT_JSONPATH;
    min_sa[0].type = ARAZZO_ACTION_GOTO;
    min_sa[0].workflow_id = C_CDD_STR_LIT("wTarget");
    min_sa[1].type = ARAZZO_ACTION_UNKNOWN;
    min_fa[0].type = ARAZZO_ACTION_GOTO;
    min_fa[0].workflow_id = C_CDD_STR_LIT("wErr");
    min_fa[0].has_retry_after = 1;
    min_fa[0].retry_after = 1.0;
    min_fa[1].type = ARAZZO_ACTION_UNKNOWN;
    min_fa[2].type = ARAZZO_ACTION_RETRY;
    min_st.parameters = &min_p;
    min_st.n_parameters = 1;
    min_st.outputs = &min_out;
    min_st.n_outputs = 1;
    min_st.has_request_body = 1;
    min_st.request_body.content_type = C_CDD_STR_LIT("text/plain");
    min_st.request_body.replacements = &min_rep;
    min_st.request_body.n_replacements = 1;
    min_st.success_criteria = &min_cr;
    min_st.n_success_criteria = 1;
    min_st.on_success = min_sa;
    min_st.n_on_success = 2;
    min_st.on_failure = min_fa;
    min_st.n_on_failure = 3;
    min_wf.parameters = &min_p;
    min_wf.n_parameters = 1;
    min_wf.outputs = &min_out;
    min_wf.n_outputs = 1;
    min_wf.steps = &min_st;
    min_wf.n_steps = 1;
    min_spec.extensions.items = &min_ext;
    min_spec.extensions.count = 1;
    min_spec.workflows = &min_wf;
    min_spec.n_workflows = 1;
    ASSERT_EQ(CDD_C_SUCCESS, arazzo_client_render_to_strings(
                                 &min_spec, NULL, "min", NULL, &hdr, &src));
    C_CDD_FREE(hdr);
    C_CDD_FREE(src);
    hdr = NULL;
    src = NULL;

    min_st.request_body.content_type = NULL;
    min_spec.source_descriptions = &min_sd;
    min_spec.n_source_descriptions = 1;
    ASSERT_EQ(CDD_C_SUCCESS, arazzo_client_render_to_strings(
                                 &min_spec, NULL, "min", NULL, &hdr, &src));
    C_CDD_FREE(hdr);
    C_CDD_FREE(src);
    hdr = NULL;
    src = NULL;
  }

  memset(&spec, 0, sizeof(spec));
  ASSERT_EQ(CDD_C_SUCCESS, arazzo_load_from_json(arazzo_json, &spec));

  {
    int f;
    for (f = 1; f <= 40; ++f) {
      g_cdd_alloc_fail = f;
      if (arazzo_client_render_to_strings(&spec, NULL, "shop", "api_", &hdr,
                                          &src) == CDD_C_SUCCESS) {
        C_CDD_FREE(hdr);
        C_CDD_FREE(src);
        hdr = NULL;
        src = NULL;
      }
      g_cdd_alloc_fail = 0;
    }
  }

  ASSERT_EQ(CDD_C_SUCCESS, arazzo_client_render_to_strings(&spec, NULL, "shop",
                                                           "api_", &hdr, &src));
  ASSERT(hdr != NULL);
  ASSERT(src != NULL);
  ASSERT(strstr(hdr, "api_workflow_checkout") != NULL);
  ASSERT(strstr(src, "step_s1:") != NULL);

  memset(&roundtrip, 0, sizeof(roundtrip));
  ASSERT_EQ(CDD_C_SUCCESS, c2arazzo_parse_source(hdr, &roundtrip));
  ASSERT_EQ(4, roundtrip.n_workflows);
  ASSERT_STR_EQ("checkout", roundtrip.workflows[3].workflow_id);
  ASSERT_EQ(4, roundtrip.workflows[3].n_steps);
  arazzo_spec_free(&roundtrip);

  CDD_STRCPY(base_path, sizeof(base_path), "test_build_dir/arazzo_shop/shop");
  memset(&cfg, 0, sizeof(cfg));
  cfg.filename_base = base_path;
  cfg.func_prefix = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, arazzo_client_generate(&spec, NULL, &cfg));
  cfg.func_prefix = "api_";
  {
    int f;
    for (f = 1; f <= 40; ++f) {
      g_cdd_alloc_fail = f;
      arazzo_client_generate(&spec, NULL, &cfg);
      g_cdd_alloc_fail = 0;
    }
    for (f = 1; f <= 10; ++f) {
      g_cdd_strdup_fail = f;
      arazzo_client_generate(&spec, NULL, &cfg);
      g_cdd_strdup_fail = 0;
    }
    for (f = 0; f <= 3; ++f) {
      g_fail_io_after = f;
      arazzo_client_generate(&spec, NULL, &cfg);
      g_fail_io_after = -1;
    }
    g_fail_io_after = 88;
    arazzo_client_generate(&spec, NULL, &cfg);
    g_fail_io_after = -1;
    makedirs("test_build_dir/arazzo_shop_hfail/src/shop_workflows.h");
    cfg.filename_base = "test_build_dir/arazzo_shop_hfail/shop";
    ASSERT_NEQ(CDD_C_SUCCESS, arazzo_client_generate(&spec, NULL, &cfg));
    cfg.filename_base = base_path;
  }
  ASSERT_EQ(CDD_C_SUCCESS, arazzo_client_generate(&spec, NULL, &cfg));
  remove("test_build_dir/arazzo_shop/src/shop_workflows.h");
  remove("test_build_dir/arazzo_shop/src/shop_workflows.c");

  C_CDD_FREE(hdr);
  C_CDD_FREE(src);
  arazzo_spec_free(&spec);
  PASS();
}

/**
 * @brief Suite for Arazzo C Client SDK generation tests.
 */
SUITE(arazzo_client_gen_suite) {
  RUN_TEST(test_arazzo_client_gen_and_roundtrip);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_TEST_ARAZZO_CLIENT_GEN_H */
