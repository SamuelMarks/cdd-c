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
    strcat(arazzo_json, chunks[idx]);
    idx++;
  }

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_client_render_to_strings(NULL, NULL, "b", "", &hdr, &src));
  memset(&cfg, 0, sizeof(cfg));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_client_generate(NULL, NULL, &cfg));

  memset(&spec, 0, sizeof(spec));
  ASSERT_EQ(CDD_C_SUCCESS, arazzo_load_from_json(arazzo_json, &spec));

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

  strcpy(base_path, "test_build_dir/arazzo_shop/shop");
  memset(&cfg, 0, sizeof(cfg));
  cfg.filename_base = base_path;
  cfg.func_prefix = "api_";
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
