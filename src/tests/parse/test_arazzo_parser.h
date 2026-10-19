/**
 * @file test_arazzo_parser.h
 * @brief Exhaustive unit and round-trip tests for Arazzo v1.1.0 parser,
 * validator, and emitter.
 *
 * Exercises `src/arazzo/parse/arazzo.c` and `src/arazzo/emit/arazzo.c` across
 * valid/invalid inputs, round-trip JSON emission, semantic validation rules,
 * and OOM fault-injection paths.
 *
 * @author Samuel Marks
 */

#ifndef TEST_ARAZZO_PARSER_H
#define TEST_ARAZZO_PARSER_H

/* clang-format off */
#include <greatest.h>
#include <stdlib.h>
#include <string.h>
#include "arazzo/emit/arazzo.h"
#include "arazzo/parse/arazzo.h"
#include "c_cdd/memory.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief Builds a comprehensive valid Arazzo 1.1.0 `JSON_Value *` exercising
 * all spec objects.
 *
 * @param[out] out_root Output `JSON_Value *` object.
 * @return `CDD_C_SUCCESS` on success.
 */
static cdd_c_error_t build_full_arazzo_json(JSON_Value **out_root) {
  JSON_Value *root_val = json_value_init_object();
  JSON_Object *root = json_value_get_object(root_val);
  JSON_Value *sd_arr_val = json_value_init_array();
  JSON_Array *sd_arr = json_value_get_array(sd_arr_val);
  JSON_Value *sd0 = json_value_init_object();
  JSON_Value *sd1 = json_value_init_object();
  JSON_Value *sd2 = json_value_init_object();
  JSON_Value *wf_arr_val = json_value_init_array();
  JSON_Array *wf_arr = json_value_get_array(wf_arr_val);
  JSON_Value *wf0 = json_value_init_object();
  JSON_Value *wf1 = json_value_init_object();
  JSON_Object *wf0_obj = json_value_get_object(wf0);
  JSON_Object *wf1_obj = json_value_get_object(wf1);
  JSON_Value *steps0_val = json_value_init_array();
  JSON_Array *steps0 = json_value_get_array(steps0_val);
  JSON_Value *st0 = json_value_init_object();
  JSON_Value *st1 = json_value_init_object();
  JSON_Value *st2 = json_value_init_object();
  JSON_Value *st3 = json_value_init_object();
  JSON_Value *steps1_val = json_value_init_array();
  JSON_Array *steps1 = json_value_get_array(steps1_val);
  JSON_Value *st_w1 = json_value_init_object();

  json_object_set_string(root, "arazzo", "1.1.0");
  json_object_set_string(root, "$self", "https://example.com/arazzo.json");
  json_object_set_string(root, "x-root-ext", "root_val");

  json_object_dotset_string(root, "info.title", "Petstore Order Workflow");
  json_object_dotset_string(root, "info.summary", "End-to-end order flow");
  json_object_dotset_string(root, "info.description", "Full description");
  json_object_dotset_string(root, "info.version", "1.1.0");
  json_object_dotset_string(root, "info.x-info-ext", "info_val");

  /* sourceDescriptions */
  json_object_set_string(json_value_get_object(sd0), "name", "petstore");
  json_object_set_string(json_value_get_object(sd0), "url", "./openapi.json");
  json_object_set_string(json_value_get_object(sd0), "type", "openapi");
  json_object_set_string(json_value_get_object(sd0), "x-sd", "1");

  json_object_set_string(json_value_get_object(sd1), "name", "events");
  json_object_set_string(json_value_get_object(sd1), "url", "./asyncapi.json");
  json_object_set_string(json_value_get_object(sd1), "type", "asyncapi");

  json_object_set_string(json_value_get_object(sd2), "name", "subflows");
  json_object_set_string(json_value_get_object(sd2), "url",
                         "./sub.arazzo.json");
  /* omit type on sd2 to test default openapi fallback */
  json_array_append_value(sd_arr, sd0);
  json_array_append_value(sd_arr, sd1);
  json_array_append_value(sd_arr, sd2);
  json_object_set_value(root, "sourceDescriptions", sd_arr_val);

  /* components */
  json_object_dotset_string(root, "components.inputs.UserInput.type", "object");
  json_object_dotset_string(root, "components.parameters.AuthParam.name",
                            "Authorization");
  json_object_dotset_string(root, "components.parameters.AuthParam.in",
                            "header");
  json_object_dotset_string(root, "components.parameters.AuthParam.value",
                            "$inputs.token");
  json_object_dotset_string(root, "components.successActions.DoneAct.name",
                            "done");
  json_object_dotset_string(root, "components.successActions.DoneAct.type",
                            "end");
  json_object_dotset_string(root, "components.failureActions.RetryAct.name",
                            "retryIt");
  json_object_dotset_string(root, "components.failureActions.RetryAct.type",
                            "retry");
  json_object_dotset_number(
      root, "components.failureActions.RetryAct.retryAfter", 1.5);
  json_object_dotset_number(root,
                            "components.failureActions.RetryAct.retryLimit", 3);
  json_object_dotset_string(root, "components.x-comp", "c");

  /* Workflow 0 */
  json_object_set_string(wf0_obj, "workflowId", "prereqWf");
  json_object_set_string(json_value_get_object(st_w1), "stepId", "initStep");
  json_object_set_string(json_value_get_object(st_w1), "operationId", "initOp");
  json_array_append_value(steps1, st_w1);
  json_object_set_value(wf0_obj, "steps", steps1_val);

  /* Workflow 1 (comprehensive) */
  json_object_set_string(wf1_obj, "workflowId", "mainWf");
  json_object_set_string(wf1_obj, "summary", "Main summary");
  json_object_set_string(wf1_obj, "description", "Main desc");
  json_object_dotset_string(wf1_obj, "inputs.type", "object");
  {
    JSON_Value *dep_val = json_value_init_array();
    json_array_append_string(json_value_get_array(dep_val), "prereqWf");
    json_object_set_value(wf1_obj, "dependsOn", dep_val);
  }

  /* Step 0: operationId + requestBody + replacements + criteria + actions +
   * outputs */
  {
    JSON_Object *s0 = json_value_get_object(st0);
    JSON_Value *p_arr_val = json_value_init_array();
    JSON_Array *p_arr = json_value_get_array(p_arr_val);
    JSON_Value *p0 = json_value_init_object();
    JSON_Value *p1 = json_value_init_object();
    JSON_Value *p2 = json_value_init_object();
    JSON_Value *rep_arr_val = json_value_init_array();
    JSON_Array *rep_arr = json_value_get_array(rep_arr_val);
    JSON_Value *rep0 = json_value_init_object();
    JSON_Value *crit_arr_val = json_value_init_array();
    JSON_Array *crit_arr = json_value_get_array(crit_arr_val);
    JSON_Value *c0 = json_value_init_object();
    JSON_Value *c1 = json_value_init_object();
    JSON_Value *c2 = json_value_init_object();
    JSON_Value *c3 = json_value_init_object();
    JSON_Value *c4 = json_value_init_object();
    JSON_Value *ons_arr_val = json_value_init_array();
    JSON_Array *ons_arr = json_value_get_array(ons_arr_val);
    JSON_Value *sa0 = json_value_init_object();
    JSON_Value *sa1 = json_value_init_object();
    JSON_Value *onf_arr_val = json_value_init_array();
    JSON_Array *onf_arr = json_value_get_array(onf_arr_val);
    JSON_Value *fa0 = json_value_init_object();
    JSON_Value *fa1 = json_value_init_object();

    json_object_set_string(s0, "stepId", "step1");
    json_object_set_string(s0, "description", "First step");
    json_object_set_string(s0, "operationId", "createOrder");
    json_object_set_number(s0, "timeout", 5000);
    json_object_set_string(s0, "correlationId", "$inputs.corr");
    json_object_set_string(s0, "action", "send");
    json_object_set_string(s0, "x-step", "s");

    /* Reusable param with override, literal param, and Selector param */
    json_object_set_string(json_value_get_object(p0), "reference",
                           "$components.parameters.AuthParam");
    json_object_set_string(json_value_get_object(p0), "value", "Bearer custom");

    json_object_set_string(json_value_get_object(p1), "name", "filter");
    json_object_set_string(json_value_get_object(p1), "in", "querystring");
    json_object_set_string(json_value_get_object(p1), "value", "$inputs.q");

    json_object_set_string(json_value_get_object(p2), "name", "dynId");
    json_object_set_string(json_value_get_object(p2), "in", "query");
    json_object_dotset_string(json_value_get_object(p2), "value.context",
                              "$response.body");
    json_object_dotset_string(json_value_get_object(p2), "value.selector",
                              "$.id");
    json_object_dotset_string(json_value_get_object(p2), "value.type",
                              "jsonpath");

    json_array_append_value(p_arr, p0);
    json_array_append_value(p_arr, p1);
    json_array_append_value(p_arr, p2);
    {
      JSON_Value *p3 = json_value_init_object();
      JSON_Value *p4 = json_value_init_object();
      json_object_set_string(json_value_get_object(p3), "name", "numParam");
      json_object_set_number(json_value_get_object(p3), "value", 42);
      json_object_set_string(json_value_get_object(p4), "reference",
                             "$components.parameters.AuthParam");
      json_array_append_value(p_arr, p3);
      json_array_append_value(p_arr, p4);
    }
    json_object_set_value(s0, "parameters", p_arr_val);

    /* requestBody */
    json_object_dotset_string(s0, "requestBody.contentType",
                              "application/json");
    json_object_dotset_string(s0, "requestBody.payload.item", "book");
    json_object_set_string(json_value_get_object(rep0), "target", "/item");
    json_object_dotset_string(json_value_get_object(rep0),
                              "targetSelectorType.type", "jsonpointer");
    json_object_dotset_string(json_value_get_object(rep0),
                              "targetSelectorType.version", "rfc6901");
    json_object_set_string(json_value_get_object(rep0), "value",
                           "$inputs.bookName");
    json_array_append_value(rep_arr, rep0);
    json_object_dotset_value(s0, "requestBody.replacements", rep_arr_val);

    /* successCriteria: simple, regex, jsonpath, xpath, expressionType */
    json_object_set_string(json_value_get_object(c0), "condition",
                           "$statusCode == 200");
    json_object_set_string(json_value_get_object(c0), "type", "simple");
    json_object_set_string(json_value_get_object(c1), "context",
                           "$response.body#/status");
    json_object_set_string(json_value_get_object(c1), "condition", "^ok$");
    json_object_set_string(json_value_get_object(c1), "type", "regex");

    json_object_set_string(json_value_get_object(c2), "condition", "$.id");
    json_object_set_string(json_value_get_object(c2), "type", "jsonpath");

    json_object_set_string(json_value_get_object(c3), "context",
                           "$response.body");
    json_object_set_string(json_value_get_object(c3), "condition", "/order/id");
    json_object_set_string(json_value_get_object(c3), "type", "xpath");

    json_object_set_string(json_value_get_object(c4), "condition", "$.items");
    json_object_dotset_string(json_value_get_object(c4), "type.type",
                              "jsonpath");
    json_object_dotset_string(json_value_get_object(c4), "type.version",
                              "draft-goessner-dispatch-jsonpath-00");

    json_array_append_value(crit_arr, c0);
    json_array_append_value(crit_arr, c1);
    json_array_append_value(crit_arr, c2);
    json_array_append_value(crit_arr, c3);
    json_array_append_value(crit_arr, c4);
    json_object_set_value(s0, "successCriteria", crit_arr_val);

    /* onSuccess & onFailure */
    json_object_set_string(json_value_get_object(sa0), "reference",
                           "$components.successActions.DoneAct");
    json_object_set_string(json_value_get_object(sa1), "name", "gotoStep2");
    json_object_set_string(json_value_get_object(sa1), "type", "goto");
    json_object_set_string(json_value_get_object(sa1), "stepId", "step2");
    json_object_set_string(json_value_get_object(sa1), "workflowId",
                           "prereqWf");
    {
      JSON_Value *sap_arr = json_value_init_array();
      JSON_Value *sap0 = json_value_init_object();
      JSON_Value *sac_arr = json_value_init_array();
      JSON_Value *sac0 = json_value_init_object();
      json_object_set_string(json_value_get_object(sap0), "name", "actP");
      json_object_set_string(json_value_get_object(sap0), "value", "1");
      json_array_append_value(json_value_get_array(sap_arr), sap0);
      json_object_set_value(json_value_get_object(sa1), "parameters", sap_arr);
      json_object_set_string(json_value_get_object(sac0), "condition",
                             "$statusCode == 200");
      json_array_append_value(json_value_get_array(sac_arr), sac0);
      json_object_set_value(json_value_get_object(sa1), "criteria", sac_arr);
    }
    json_array_append_value(ons_arr, sa0);
    json_array_append_value(ons_arr, sa1);
    json_object_set_value(s0, "onSuccess", ons_arr_val);

    json_object_set_string(json_value_get_object(fa0), "reference",
                           "$components.failureActions.RetryAct");
    json_object_set_string(json_value_get_object(fa1), "name", "failGoto");
    json_object_set_string(json_value_get_object(fa1), "type", "goto");
    json_object_set_string(json_value_get_object(fa1), "stepId", "step2");
    json_object_set_string(json_value_get_object(fa1), "workflowId",
                           "prereqWf");
    json_object_set_number(json_value_get_object(fa1), "retryAfter", 2.0);
    json_object_set_number(json_value_get_object(fa1), "retryLimit", 1);
    {
      JSON_Value *fap_arr = json_value_init_array();
      JSON_Value *fap0 = json_value_init_object();
      JSON_Value *fac_arr = json_value_init_array();
      JSON_Value *fac0 = json_value_init_object();
      json_object_set_string(json_value_get_object(fap0), "name", "fActP");
      json_object_set_string(json_value_get_object(fap0), "value", "2");
      json_array_append_value(json_value_get_array(fap_arr), fap0);
      json_object_set_value(json_value_get_object(fa1), "parameters", fap_arr);
      json_object_set_string(json_value_get_object(fac0), "condition",
                             "$statusCode == 500");
      json_array_append_value(json_value_get_array(fac_arr), fac0);
      json_object_set_value(json_value_get_object(fa1), "criteria", fac_arr);
    }
    json_array_append_value(onf_arr, fa0);
    json_array_append_value(onf_arr, fa1);
    json_object_set_value(s0, "onFailure", onf_arr_val);

    /* outputs */
    json_object_dotset_string(s0, "outputs.orderId", "$response.body#/id");
    json_object_dotset_string(s0, "outputs.extracted.context",
                              "$response.body");
    json_object_dotset_string(s0, "outputs.extracted.selector", "$.meta");
    json_object_dotset_string(s0, "outputs.extracted.type.type", "xpath");
  }

  /* Step 1: operationPath + dependsOn */
  json_object_set_string(json_value_get_object(st1), "stepId", "step2");
  json_object_set_string(json_value_get_object(st1), "operationPath",
                         "$sourceDescriptions.petstore.url#/paths/~1pets/get");
  {
    JSON_Value *sdep = json_value_init_array();
    json_array_append_string(json_value_get_array(sdep), "step1");
    json_object_set_value(json_value_get_object(st1), "dependsOn", sdep);
  }

  /* Step 2: channelPath + receive */
  json_object_set_string(json_value_get_object(st2), "stepId", "step3");
  json_object_set_string(json_value_get_object(st2), "channelPath",
                         "/orders/completed");
  json_object_set_string(json_value_get_object(st2), "action", "receive");

  /* Step 3: workflowId */
  json_object_set_string(json_value_get_object(st3), "stepId", "step4");
  json_object_set_string(json_value_get_object(st3), "workflowId", "prereqWf");

  json_array_append_value(steps0, st0);
  json_array_append_value(steps0, st1);
  json_array_append_value(steps0, st2);
  json_array_append_value(steps0, st3);
  json_object_set_value(wf1_obj, "steps", steps0_val);

  /* workflow-level successActions, failureActions, outputs, parameters */
  {
    JSON_Value *wsa_arr = json_value_init_array();
    JSON_Value *wsa0 = json_value_init_object();
    JSON_Value *wfa_arr = json_value_init_array();
    JSON_Value *wfa0 = json_value_init_object();
    JSON_Value *wp_arr = json_value_init_array();
    JSON_Value *wp0 = json_value_init_object();

    json_object_set_string(json_value_get_object(wsa0), "name", "wfEnd");
    json_object_set_string(json_value_get_object(wsa0), "type", "end");
    json_array_append_value(json_value_get_array(wsa_arr), wsa0);
    json_object_set_value(wf1_obj, "successActions", wsa_arr);

    json_object_set_string(json_value_get_object(wfa0), "name", "wfFailEnd");
    json_object_set_string(json_value_get_object(wfa0), "type", "end");
    json_array_append_value(json_value_get_array(wfa_arr), wfa0);
    json_object_set_value(wf1_obj, "failureActions", wfa_arr);

    json_object_set_string(json_value_get_object(wp0), "reference",
                           "$components.parameters.AuthParam");
    json_array_append_value(json_value_get_array(wp_arr), wp0);
    json_object_set_value(wf1_obj, "parameters", wp_arr);

    json_object_dotset_string(wf1_obj, "outputs.resultOrder",
                              "$steps.step1.outputs.orderId");
  }

  json_array_append_value(wf_arr, wf0);
  json_array_append_value(wf_arr, wf1);
  json_object_set_value(root, "workflows", wf_arr_val);

  *out_root = root_val;
  return CDD_C_SUCCESS;
}

/**
 * @brief Tests full round-trip `JSON -> Arazzo_Spec -> JSON -> Arazzo_Spec`.
 */
TEST test_arazzo_parse_and_emit_roundtrip(void) {
  JSON_Value *root_val = NULL;
  struct Arazzo_Spec spec1;
  struct Arazzo_Spec spec2;
  char *emitted_json = NULL;
  cdd_c_error_t rc;

  rc = build_full_arazzo_json(&root_val);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = arazzo_load_from_json_value(root_val, &spec1);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("1.1.0", spec1.arazzo);
  ASSERT_EQ(ARAZZO_VER_1_1_0, spec1.version);
  ASSERT_STR_EQ("Petstore Order Workflow", spec1.info.title);
  ASSERT_EQ(3, spec1.n_source_descriptions);
  ASSERT_EQ(2, spec1.n_workflows);
  ASSERT_EQ(4, spec1.workflows[1].n_steps);

  rc = arazzo_write_spec_to_json(&spec1, &emitted_json);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(emitted_json != NULL);

  rc = arazzo_load_from_json(emitted_json, &spec2);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ(spec1.info.title, spec2.info.title);
  ASSERT_EQ(spec1.n_workflows, spec2.n_workflows);
  ASSERT_EQ(spec1.workflows[1].n_steps, spec2.workflows[1].n_steps);

  free(emitted_json);
  arazzo_spec_free(&spec1);
  arazzo_spec_free(&spec2);
  json_value_free(root_val);
  PASS();
}

/**
 * @brief Tests validation failures and malformed JSON inputs on
 * `arazzo_load_from_json` and `arazzo_validate_spec`.
 */
TEST test_arazzo_parse_and_validate_errors(void) {
  struct Arazzo_Spec spec;
  JSON_Value *root_val = NULL;
  char *json_out = NULL;

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, arazzo_load_from_json(NULL, &spec));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, arazzo_load_from_json("{}", NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_load_from_json("{bad json", &spec));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, arazzo_load_from_json("[]", &spec));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, arazzo_load_from_json("{}", &spec));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, arazzo_validate_spec(NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_write_spec_to_json(NULL, &json_out));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_write_spec_to_json(&spec, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_write_spec_to_json_value(NULL, &root_val));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_write_spec_to_json_value(&spec, NULL));

  /* Unsupported version */
  build_full_arazzo_json(&root_val);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_load_from_json_value(root_val, NULL));
  json_object_set_string(json_value_get_object(root_val), "arazzo", "2.0.0");
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_load_from_json_value(root_val, &spec));

  /* Version 1.0.0 and 1.0.1 accepted */
  json_object_set_string(json_value_get_object(root_val), "arazzo", "1.0.0");
  ASSERT_EQ(CDD_C_SUCCESS, arazzo_load_from_json_value(root_val, &spec));
  ASSERT_EQ(ARAZZO_VER_1_0_0, spec.version);
  arazzo_spec_free(&spec);

  json_object_set_string(json_value_get_object(root_val), "arazzo", "1.0.1");
  ASSERT_EQ(CDD_C_SUCCESS, arazzo_load_from_json_value(root_val, &spec));
  ASSERT_EQ(ARAZZO_VER_1_0_1, spec.version);

  /* Duplicate workflowId validation failure */
  free(spec.workflows[1].workflow_id);
  spec.workflows[1].workflow_id = C_CDD_STRDUP("prereqWf");
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, arazzo_validate_spec(&spec));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_write_spec_to_json(&spec, &json_out));
  arazzo_spec_free(&spec);

  /* Unknown workflow dependsOn */
  ASSERT_EQ(CDD_C_SUCCESS, arazzo_load_from_json_value(root_val, &spec));
  free(spec.workflows[1].depends_on[0]);
  spec.workflows[1].depends_on[0] = C_CDD_STRDUP("nonexistentWf");
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, arazzo_validate_spec(&spec));
  arazzo_spec_free(&spec);

  /* Duplicate stepId */
  ASSERT_EQ(CDD_C_SUCCESS, arazzo_load_from_json_value(root_val, &spec));
  free(spec.workflows[1].steps[1].step_id);
  spec.workflows[1].steps[1].step_id = C_CDD_STRDUP("step1");
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, arazzo_validate_spec(&spec));
  arazzo_spec_free(&spec);

  /* Unknown step dependsOn */
  ASSERT_EQ(CDD_C_SUCCESS, arazzo_load_from_json_value(root_val, &spec));
  free(spec.workflows[1].steps[1].depends_on[0]);
  spec.workflows[1].steps[1].depends_on[0] = C_CDD_STRDUP("missingStep");
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, arazzo_validate_spec(&spec));
  arazzo_spec_free(&spec);

  /* Unknown component parameter reference */
  ASSERT_EQ(CDD_C_SUCCESS, arazzo_load_from_json_value(root_val, &spec));
  free(spec.workflows[1].steps[0].parameters[0].reusable.reference);
  spec.workflows[1].steps[0].parameters[0].reusable.reference =
      C_CDD_STRDUP("$components.parameters.Missing");
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, arazzo_validate_spec(&spec));
  free(spec.workflows[1].steps[0].parameters[0].reusable.reference);
  spec.workflows[1].steps[0].parameters[0].reusable.reference =
      C_CDD_STRDUP("bad_prefix");
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, arazzo_validate_spec(&spec));

  /* Self-dependency on step & workflow */
  free(spec.workflows[1].steps[0].parameters[0].reusable.reference);
  spec.workflows[1].steps[0].parameters[0].reusable.reference =
      C_CDD_STRDUP("$components.parameters.AuthParam");
  free(spec.workflows[1].steps[1].depends_on[0]);
  spec.workflows[1].steps[1].depends_on[0] = C_CDD_STRDUP("step2");
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, arazzo_validate_spec(&spec));
  free(spec.workflows[1].steps[1].depends_on[0]);
  spec.workflows[1].steps[1].depends_on[0] = C_CDD_STRDUP("step1");

  free(spec.workflows[1].depends_on[0]);
  spec.workflows[1].depends_on[0] = C_CDD_STRDUP("mainWf");
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, arazzo_validate_spec(&spec));

  /* Missing step_id or target_kind */
  spec.workflows[0].steps[0].target_kind = ARAZZO_STEP_TARGET_NONE;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, arazzo_validate_spec(&spec));
  free(spec.workflows[0].steps[0].step_id);
  spec.workflows[0].steps[0].step_id = NULL;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, arazzo_validate_spec(&spec));
  spec.workflows[0].n_steps = 0;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, arazzo_validate_spec(&spec));
  spec.workflows[0].n_steps = 1;
  free(spec.workflows[0].workflow_id);
  spec.workflows[0].workflow_id = NULL;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, arazzo_validate_spec(&spec));
  free(spec.arazzo);
  spec.arazzo = C_CDD_STRDUP("9.9.9");
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, arazzo_validate_spec(&spec));
  spec.n_workflows = 0;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, arazzo_validate_spec(&spec));
  spec.n_source_descriptions = 0;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, arazzo_validate_spec(&spec));
  free(spec.info.version);
  spec.info.version = NULL;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, arazzo_validate_spec(&spec));
  free(spec.info.title);
  spec.info.title = NULL;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, arazzo_validate_spec(&spec));
  free(spec.arazzo);
  spec.arazzo = NULL;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, arazzo_validate_spec(&spec));
  spec.n_workflows = 2;
  spec.n_source_descriptions = 3;
  arazzo_spec_free(&spec);

  /* OOM sweep across full Arazzo load and emit */
  json_object_set_string(json_value_get_object(root_val), "arazzo", "1.1.0");
  {
    int f;
    for (f = 1; f <= 160; ++f) {
      g_cdd_alloc_fail = f;
      if (arazzo_load_from_json_value(root_val, &spec) == CDD_C_SUCCESS) {
        arazzo_spec_free(&spec);
      }
      g_cdd_alloc_fail = 0;
    }
    for (f = 1; f <= 40; ++f) {
      g_cdd_strdup_fail = f;
      if (arazzo_load_from_json_value(root_val, &spec) == CDD_C_SUCCESS) {
        arazzo_spec_free(&spec);
      }
      g_cdd_strdup_fail = 0;
    }
    ASSERT_EQ(CDD_C_SUCCESS, arazzo_load_from_json_value(root_val, &spec));
    g_cdd_alloc_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, arazzo_write_spec_to_json(&spec, &json_out));
    g_cdd_alloc_fail = 0;

    /* Emit with empty components sub-objects */
    {
      size_t orig_n_inputs = spec.components.n_inputs;
      size_t orig_n_sa = spec.components.n_success_actions;
      size_t orig_n_fa = spec.components.n_failure_actions;
      size_t orig_n_cp = spec.components.n_parameters;
      size_t orig_n_sp = spec.workflows[1].steps[0].n_parameters;

      spec.components.n_inputs = 0;
      spec.components.n_success_actions = 0;
      spec.components.n_failure_actions = 0;
      ASSERT_EQ(CDD_C_SUCCESS, arazzo_write_spec_to_json(&spec, &json_out));
      free(json_out);
      spec.components.n_parameters = 0;
      spec.workflows[1].steps[0].n_parameters = 0;
      ASSERT_EQ(CDD_C_SUCCESS, arazzo_write_spec_to_json(&spec, &json_out));
      free(json_out);

      spec.components.n_inputs = orig_n_inputs;
      spec.components.n_success_actions = orig_n_sa;
      spec.components.n_failure_actions = orig_n_fa;
      spec.components.n_parameters = orig_n_cp;
      spec.workflows[1].steps[0].n_parameters = orig_n_sp;
      arazzo_spec_free(&spec);
    }

    ASSERT_EQ(CDD_C_SUCCESS, arazzo_load_from_json_value(root_val, &spec));

    /* Emit without optional fields (no $self, no summary, no desc, no
     * components) */
    free(spec.self_uri);
    spec.self_uri = NULL;
    free(spec.info.summary);
    spec.info.summary = NULL;
    free(spec.info.description);
    spec.info.description = NULL;
    spec.has_components = 0;
    spec.source_descriptions[0].type = ARAZZO_SOURCE_UNKNOWN;
    spec.workflows[1].steps[0].parameters[0].reusable.has_value_override = 0;
    free(spec.workflows[1].steps[0].parameters[2].selector.context);
    spec.workflows[1].steps[0].parameters[2].selector.context = NULL;
    free(spec.workflows[1].steps[0].outputs[1].selector.context);
    spec.workflows[1].steps[0].outputs[1].selector.context = NULL;
    free(spec.workflows[1].steps[0].request_body.content_type);
    spec.workflows[1].steps[0].request_body.content_type = NULL;
    {
      size_t orig_n_rep =
          spec.workflows[1].steps[0].request_body.n_replacements;
      spec.workflows[1].steps[0].request_body.n_replacements = 0;
      free(spec.workflows[1]
               .steps[0]
               .success_criteria[4]
               .expression_type.version);
      spec.workflows[1].steps[0].success_criteria[4].expression_type.version =
          NULL;
      spec.workflows[1].steps[0].on_success[1].type = ARAZZO_ACTION_UNKNOWN;
      spec.workflows[1].steps[0].on_failure[1].type = ARAZZO_ACTION_UNKNOWN;
      spec.workflows[1].steps[0].parameters[1].in = ARAZZO_PARAM_IN_UNKNOWN;
      ASSERT_EQ(CDD_C_SUCCESS, arazzo_write_spec_to_json(&spec, &json_out));
      free(json_out);
      spec.workflows[1].steps[0].request_body.n_replacements = orig_n_rep;
      arazzo_spec_free(&spec);
    }
  }

  /* Malformed sub-object JSON inputs */
  {
    const char *const bad_jsons[] = {
        /* Invalid sourceDescription */
        "{\"arazzo\":\"1.1.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
        "\"sourceDescriptions\":[{}],\"workflows\":[]}",
        "{\"arazzo\":\"1.1.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
        "\"sourceDescriptions\":[{\"name\":\"s\",\"url\":\"u\",\"type\":"
        "\"bad\"}],\"workflows\":[{\"workflowId\":\"w\",\"steps\":[{\"stepId\":"
        "\"s1\",\"operationId\":\"o1\"}]}]}",
        /* Empty workflows or steps */
        "{\"arazzo\":\"1.1.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
        "\"sourceDescriptions\":[{\"name\":\"s\",\"url\":\"u\"}],\"workflows\":"
        "[{}]}",
        "{\"arazzo\":\"1.1.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
        "\"sourceDescriptions\":[{\"name\":\"s\",\"url\":\"u\"}],\"workflows\":"
        "[{\"workflowId\":\"w\",\"steps\":[]}]}",
        /* Step missing stepId or multiple targets or 0 targets or bad action */
        "{\"arazzo\":\"1.1.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
        "\"sourceDescriptions\":[{\"name\":\"s\",\"url\":\"u\"}],\"workflows\":"
        "[{\"workflowId\":\"w\",\"steps\":[{}]}]}",
        "{\"arazzo\":\"1.1.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
        "\"sourceDescriptions\":[{\"name\":\"s\",\"url\":\"u\"}],\"workflows\":"
        "[{\"workflowId\":\"w\",\"steps\":[{\"stepId\":\"s1\"}]}]}",
        "{\"arazzo\":\"1.1.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
        "\"sourceDescriptions\":[{\"name\":\"s\",\"url\":\"u\"}],\"workflows\":"
        "[{\"workflowId\":\"w\",\"steps\":[{\"stepId\":\"s1\",\"operationId\":"
        "\"o1\",\"workflowId\":\"w2\"}]}]}",
        "{\"arazzo\":\"1.1.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
        "\"sourceDescriptions\":[{\"name\":\"s\",\"url\":\"u\"}],\"workflows\":"
        "[{\"workflowId\":\"w\",\"steps\":[{\"stepId\":\"s1\",\"operationId\":"
        "\"o1\",\"action\":\"bad\"}]}]}",
        /* Bad parameter */
        "{\"arazzo\":\"1.1.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
        "\"sourceDescriptions\":[{\"name\":\"s\",\"url\":\"u\"}],\"workflows\":"
        "[{\"workflowId\":\"w\",\"steps\":[{\"stepId\":\"s1\",\"operationId\":"
        "\"o1\",\"parameters\":[123]}]}]}",
        "{\"arazzo\":\"1.1.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
        "\"sourceDescriptions\":[{\"name\":\"s\",\"url\":\"u\"}],\"workflows\":"
        "[{\"workflowId\":\"w\",\"steps\":[{\"stepId\":\"s1\",\"operationId\":"
        "\"o1\",\"parameters\":[{\"name\":\"p\"}]}]}]}",
        "{\"arazzo\":\"1.1.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
        "\"sourceDescriptions\":[{\"name\":\"s\",\"url\":\"u\"}],\"workflows\":"
        "[{\"workflowId\":\"w\",\"steps\":[{\"stepId\":\"s1\",\"operationId\":"
        "\"o1\",\"parameters\":[{\"name\":\"p\",\"in\":\"bad\",\"value\":\"v\"}"
        "]}]}]}",
        "{\"arazzo\":\"1.1.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
        "\"sourceDescriptions\":[{\"name\":\"s\",\"url\":\"u\"}],\"workflows\":"
        "[{\"workflowId\":\"w\",\"steps\":[{\"stepId\":\"s1\",\"operationId\":"
        "\"o1\",\"parameters\":[{\"name\":\"p\",\"value\":\"$bad_expr\"}]}]}]}",
        /* Bad replacement & criterion */
        "{\"arazzo\":\"1.1.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
        "\"sourceDescriptions\":[{\"name\":\"s\",\"url\":\"u\"}],\"workflows\":"
        "[{\"workflowId\":\"w\",\"steps\":[{\"stepId\":\"s1\",\"operationId\":"
        "\"o1\",\"requestBody\":{\"replacements\":[{}]}}]}]}",
        "{\"arazzo\":\"1.1.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
        "\"sourceDescriptions\":[{\"name\":\"s\",\"url\":\"u\"}],\"workflows\":"
        "[{\"workflowId\":\"w\",\"steps\":[{\"stepId\":\"s1\",\"operationId\":"
        "\"o1\",\"successCriteria\":[{}]}]}]}",
        "{\"arazzo\":\"1.1.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
        "\"sourceDescriptions\":[{\"name\":\"s\",\"url\":\"u\"}],\"workflows\":"
        "[{\"workflowId\":\"w\",\"steps\":[{\"stepId\":\"s1\",\"operationId\":"
        "\"o1\",\"successCriteria\":[{\"condition\":\"c\",\"type\":\"bad\"}]}]}"
        "]}",
        "{\"arazzo\":\"1.1.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
        "\"sourceDescriptions\":[{\"name\":\"s\",\"url\":\"u\"}],\"workflows\":"
        "[{\"workflowId\":\"w\",\"steps\":[{\"stepId\":\"s1\",\"operationId\":"
        "\"o1\",\"successCriteria\":[{\"condition\":\"c\",\"type\":{}}]}]}]}",
        "{\"arazzo\":\"1.1.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
        "\"sourceDescriptions\":[{\"name\":\"s\",\"url\":\"u\"}],\"workflows\":"
        "[{\"workflowId\":\"w\",\"steps\":[{\"stepId\":\"s1\",\"operationId\":"
        "\"o1\",\"successCriteria\":[{\"condition\":\"c\",\"type\":{\"type\":"
        "\"bad\"}}]}]}]}",
        "{\"arazzo\":\"1.1.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
        "\"sourceDescriptions\":[{\"name\":\"s\",\"url\":\"u\"}],\"workflows\":"
        "[{\"workflowId\":\"w\",\"steps\":[{\"stepId\":\"s1\",\"operationId\":"
        "\"o1\",\"successCriteria\":[{\"condition\":\"c\",\"type\":123}]}]}]}",
        /* Bad successAction & failureAction & outputs */
        "{\"arazzo\":\"1.1.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
        "\"sourceDescriptions\":[{\"name\":\"s\",\"url\":\"u\"}],\"workflows\":"
        "[{\"workflowId\":\"w\",\"steps\":[{\"stepId\":\"s1\",\"operationId\":"
        "\"o1\",\"onSuccess\":[123]}]}]}",
        "{\"arazzo\":\"1.1.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
        "\"sourceDescriptions\":[{\"name\":\"s\",\"url\":\"u\"}],\"workflows\":"
        "[{\"workflowId\":\"w\",\"steps\":[{\"stepId\":\"s1\",\"operationId\":"
        "\"o1\",\"onSuccess\":[{\"name\":\"a\"}]}]}]}",
        "{\"arazzo\":\"1.1.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
        "\"sourceDescriptions\":[{\"name\":\"s\",\"url\":\"u\"}],\"workflows\":"
        "[{\"workflowId\":\"w\",\"steps\":[{\"stepId\":\"s1\",\"operationId\":"
        "\"o1\",\"onSuccess\":[{\"type\":\"end\"}]}]}]}",
        "{\"arazzo\":\"1.1.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
        "\"sourceDescriptions\":[{\"name\":\"s\",\"url\":\"u\"}],\"workflows\":"
        "[{\"workflowId\":\"w\",\"steps\":[{\"stepId\":\"s1\",\"operationId\":"
        "\"o1\",\"onSuccess\":[{\"name\":\"a\",\"type\":\"bad\"}]}]}]}",
        "{\"arazzo\":\"1.1.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
        "\"sourceDescriptions\":[{\"name\":\"s\",\"url\":\"u\"}],\"workflows\":"
        "[{\"workflowId\":\"w\",\"steps\":[{\"stepId\":\"s1\",\"operationId\":"
        "\"o1\",\"onSuccess\":[{\"name\":\"a\",\"type\":\"retry\"}]}]}]}",
        "{\"arazzo\":\"1.1.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
        "\"sourceDescriptions\":[{\"name\":\"s\",\"url\":\"u\"}],\"workflows\":"
        "[{\"workflowId\":\"w\",\"steps\":[{\"stepId\":\"s1\",\"operationId\":"
        "\"o1\",\"onFailure\":[123]}]}]}",
        "{\"arazzo\":\"1.1.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
        "\"sourceDescriptions\":[{\"name\":\"s\",\"url\":\"u\"}],\"workflows\":"
        "[{\"workflowId\":\"w\",\"steps\":[{\"stepId\":\"s1\",\"operationId\":"
        "\"o1\",\"onFailure\":[{\"name\":\"a\"}]}]}]}",
        "{\"arazzo\":\"1.1.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
        "\"sourceDescriptions\":[{\"name\":\"s\",\"url\":\"u\"}],\"workflows\":"
        "[{\"workflowId\":\"w\",\"steps\":[{\"stepId\":\"s1\",\"operationId\":"
        "\"o1\",\"onFailure\":[{\"type\":\"end\"}]}]}]}",
        "{\"arazzo\":\"1.1.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
        "\"sourceDescriptions\":[{\"name\":\"s\",\"url\":\"u\"}],\"workflows\":"
        "[{\"workflowId\":\"w\",\"steps\":[{\"stepId\":\"s1\",\"operationId\":"
        "\"o1\",\"onFailure\":[{\"name\":\"a\",\"type\":\"bad\"}]}]}]}",
        "{\"arazzo\":\"1.1.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
        "\"sourceDescriptions\":[{\"name\":\"s\",\"url\":\"u\"}],\"workflows\":"
        "[{\"workflowId\":\"w\",\"steps\":[{\"stepId\":\"s1\",\"operationId\":"
        "\"o1\",\"outputs\":{\"k\":123}}]}]}",
        "{\"arazzo\":\"1.1.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
        "\"sourceDescriptions\":[{\"name\":\"s\",\"url\":\"u\"}],\"workflows\":"
        "[{\"workflowId\":\"w\",\"steps\":[{\"stepId\":\"s1\",\"operationId\":"
        "\"o1\",\"outputs\":{\"k\":\"$bad\"}}]}]}",
        "{\"arazzo\":\"1.1.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
        "\"sourceDescriptions\":[{\"name\":\"s\",\"url\":\"u\"}],\"workflows\":"
        "[{\"workflowId\":\"w\",\"steps\":[{\"stepId\":\"s1\",\"operationId\":"
        "\"o1\"}]}],\"x-\":\"bad_ext\"}",
        "{\"arazzo\":\"1.1.0\"}",
        "{\"arazzo\":\"1.1.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"}}",
        "{\"arazzo\":\"1.1.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
        "\"sourceDescriptions\":[{\"name\":\"s\",\"url\":\"u\"}]}",
        "{\"arazzo\":\"1.1.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
        "\"sourceDescriptions\":[{\"url\":\"u\"}],\"workflows\":[{"
        "\"workflowId\":\"w\",\"steps\":[{\"stepId\":\"s1\",\"operationId\":"
        "\"o1\"}]}]}",
        "{\"arazzo\":\"1.1.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
        "\"sourceDescriptions\":[{\"name\":\"s\"}],\"workflows\":[{"
        "\"workflowId\":\"w\",\"steps\":[{\"stepId\":\"s1\",\"operationId\":"
        "\"o1\"}]}]}",
        "{\"arazzo\":\"1.1.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
        "\"sourceDescriptions\":[{\"name\":\"s\",\"url\":\"u\"}],\"workflows\":"
        "[{\"workflowId\":\"w\",\"steps\":[{\"stepId\":\"s1\",\"operationId\":"
        "\"o1\",\"parameters\":[{\"value\":\"v\"}]}]}]}",
        "{\"arazzo\":\"1.1.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
        "\"sourceDescriptions\":[{\"name\":\"s\",\"url\":\"u\",\"x-\":1}],"
        "\"workflows\":[{\"workflowId\":\"w\",\"steps\":[{\"stepId\":\"s1\","
        "\"operationId\":\"o1\"}]}]}",
        "{\"arazzo\":\"1.1.0\",\"info\":{\"title\":\"t\",\"version\":\"1\",\"x-"
        "\":1},\"sourceDescriptions\":[{\"name\":\"s\",\"url\":\"u\"}],"
        "\"workflows\":[{\"workflowId\":\"w\",\"steps\":[{\"stepId\":\"s1\","
        "\"operationId\":\"o1\"}]}]}",
        "{\"arazzo\":\"1.1.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
        "\"sourceDescriptions\":[{\"name\":\"s\",\"url\":\"u\"}],\"workflows\":"
        "[{\"workflowId\":\"w\",\"steps\":[{\"stepId\":\"s1\",\"operationId\":"
        "\"o1\",\"parameters\":[{\"reference\":\"$components.parameters.A\","
        "\"value\":{\"x-\":1}}]}]}]}",
        "{\"arazzo\":\"1.1.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
        "\"sourceDescriptions\":[{\"name\":\"s\",\"url\":\"u\"}],\"workflows\":"
        "[{\"workflowId\":\"w\",\"steps\":[{\"stepId\":\"s1\",\"operationId\":"
        "\"o1\",\"parameters\":[{\"name\":\"p\",\"value\":{\"selector\":\"$."
        "a\",\"type\":\"bad\"}}]}]}]}",
        "{\"arazzo\":\"1.1.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
        "\"sourceDescriptions\":[{\"name\":\"s\",\"url\":\"u\"}],\"workflows\":"
        "[{\"workflowId\":\"w\",\"steps\":[{\"stepId\":\"s1\",\"operationId\":"
        "\"o1\",\"parameters\":[{\"name\":\"p\",\"value\":{\"selector\":\"$."
        "a\",\"x-\":1}}]}]}]}",
        "{\"arazzo\":\"1.1.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
        "\"sourceDescriptions\":[{\"name\":\"s\",\"url\":\"u\"}],\"workflows\":"
        "[{\"workflowId\":\"w\",\"steps\":[{\"stepId\":\"s1\",\"operationId\":"
        "\"o1\",\"successCriteria\":[{\"condition\":\"c\",\"x-\":1}]}]}]}",
        "{\"arazzo\":\"1.1.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
        "\"sourceDescriptions\":[{\"name\":\"s\",\"url\":\"u\"}],\"workflows\":"
        "[{\"workflowId\":\"w\",\"steps\":[{\"stepId\":\"s1\",\"operationId\":"
        "\"o1\",\"onSuccess\":[{\"name\":\"a\",\"type\":\"end\",\"parameters\":"
        "[123]}]}]}]}",
        "{\"arazzo\":\"1.1.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
        "\"sourceDescriptions\":[{\"name\":\"s\",\"url\":\"u\"}],\"workflows\":"
        "[{\"workflowId\":\"w\",\"steps\":[{\"stepId\":\"s1\",\"operationId\":"
        "\"o1\",\"onSuccess\":[{\"name\":\"a\",\"type\":\"end\",\"criteria\":["
        "123]}]}]}]}",
        "{\"arazzo\":\"1.1.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
        "\"sourceDescriptions\":[{\"name\":\"s\",\"url\":\"u\"}],\"workflows\":"
        "[{\"workflowId\":\"w\",\"steps\":[{\"stepId\":\"s1\",\"operationId\":"
        "\"o1\",\"onFailure\":[{\"name\":\"a\",\"type\":\"end\",\"parameters\":"
        "[123]}]}]}]}",
        "{\"arazzo\":\"1.1.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
        "\"sourceDescriptions\":[{\"name\":\"s\",\"url\":\"u\"}],\"workflows\":"
        "[{\"workflowId\":\"w\",\"steps\":[{\"stepId\":\"s1\",\"operationId\":"
        "\"o1\",\"onFailure\":[{\"name\":\"a\",\"type\":\"end\",\"criteria\":["
        "123]}]}]}]}",
        "{\"arazzo\":\"1.1.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
        "\"sourceDescriptions\":[{\"name\":\"s\",\"url\":\"u\"}],\"workflows\":"
        "[{\"workflowId\":\"w\",\"steps\":[{\"stepId\":\"s1\",\"operationId\":"
        "\"o1\",\"outputs\":{\"k\":{\"selector\":\"$.a\",\"type\":\"bad\"}}}]}]"
        "}",
        "{\"arazzo\":\"1.1.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
        "\"sourceDescriptions\":[{\"name\":\"s\",\"url\":\"u\"}],\"workflows\":"
        "[{\"workflowId\":\"w\",\"steps\":[{\"stepId\":\"s1\",\"operationId\":"
        "\"o1\",\"requestBody\":{\"replacements\":[{\"target\":\"/a\"}]}}]}]}",
        "{\"arazzo\":\"1.1.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
        "\"sourceDescriptions\":[{\"name\":\"s\",\"url\":\"u\"}],\"workflows\":"
        "[{\"workflowId\":\"w\",\"steps\":[{\"stepId\":\"s1\",\"operationId\":"
        "\"o1\",\"requestBody\":{\"replacements\":[{\"target\":\"/"
        "a\",\"value\":1,\"targetSelectorType\":\"bad\"}]}}]}]}",
        "{\"arazzo\":\"1.1.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
        "\"sourceDescriptions\":[{\"name\":\"s\",\"url\":\"u\"}],\"workflows\":"
        "[{\"workflowId\":\"w\",\"steps\":[{\"stepId\":\"s1\",\"operationId\":"
        "\"o1\",\"requestBody\":{\"replacements\":[{\"target\":\"/"
        "a\",\"value\":1,\"x-\":1}]}}]}]}",
        "{\"arazzo\":\"1.1.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
        "\"sourceDescriptions\":[{\"name\":\"s\",\"url\":\"u\"}],\"workflows\":"
        "[{\"workflowId\":\"w\",\"steps\":[{\"stepId\":\"s1\",\"operationId\":"
        "\"o1\",\"requestBody\":{\"x-\":1}}]}]}",
        "{\"arazzo\":\"1.1.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
        "\"sourceDescriptions\":[{\"name\":\"s\",\"url\":\"u\"}],\"workflows\":"
        "[{\"workflowId\":\"w\",\"steps\":[{\"stepId\":\"s1\",\"operationId\":"
        "\"o1\"}],\"successActions\":[123]}]}",
        "{\"arazzo\":\"1.1.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
        "\"sourceDescriptions\":[{\"name\":\"s\",\"url\":\"u\"}],\"workflows\":"
        "[{\"workflowId\":\"w\",\"steps\":[{\"stepId\":\"s1\",\"operationId\":"
        "\"o1\"}],\"failureActions\":[123]}]}",
        "{\"arazzo\":\"1.1.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
        "\"sourceDescriptions\":[{\"name\":\"s\",\"url\":\"u\"}],\"workflows\":"
        "[{\"workflowId\":\"w\",\"steps\":[{\"stepId\":\"s1\",\"operationId\":"
        "\"o1\"}],\"outputs\":{\"k\":123}}]}",
        "{\"arazzo\":\"1.1.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
        "\"sourceDescriptions\":[{\"name\":\"s\",\"url\":\"u\"}],\"workflows\":"
        "[{\"workflowId\":\"w\",\"steps\":[{\"stepId\":\"s1\",\"operationId\":"
        "\"o1\"}],\"parameters\":[123]}]}",
        "{\"arazzo\":\"1.1.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
        "\"sourceDescriptions\":[{\"name\":\"s\",\"url\":\"u\"}],"
        "\"components\":{\"inputs\":{\"I\":123}},\"workflows\":[{"
        "\"workflowId\":\"w\",\"steps\":[{\"stepId\":\"s1\",\"operationId\":"
        "\"o1\"}]}]}",
        "{\"arazzo\":\"1.1.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
        "\"sourceDescriptions\":[{\"name\":\"s\",\"url\":\"u\"}],"
        "\"components\":{\"parameters\":{\"P\":123}},\"workflows\":[{"
        "\"workflowId\":\"w\",\"steps\":[{\"stepId\":\"s1\",\"operationId\":"
        "\"o1\"}]}]}",
        "{\"arazzo\":\"1.1.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
        "\"sourceDescriptions\":[{\"name\":\"s\",\"url\":\"u\"}],"
        "\"components\":{\"successActions\":{\"S\":123}},\"workflows\":[{"
        "\"workflowId\":\"w\",\"steps\":[{\"stepId\":\"s1\",\"operationId\":"
        "\"o1\"}]}]}",
        "{\"arazzo\":\"1.1.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},"
        "\"sourceDescriptions\":[{\"name\":\"s\",\"url\":\"u\"}],"
        "\"components\":{\"failureActions\":{\"F\":123}},\"workflows\":[{"
        "\"workflowId\":\"w\",\"steps\":[{\"stepId\":\"s1\",\"operationId\":"
        "\"o1\"}]}]}"};
    size_t idx;
    for (idx = 0; idx < sizeof(bad_jsons) / sizeof(bad_jsons[0]); ++idx) {
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                arazzo_load_from_json(bad_jsons[idx], &spec));
    }
    /* Valid minimal spec with empty components and requestBody without payload
     */
    ASSERT_EQ(CDD_C_SUCCESS,
              arazzo_load_from_json(
                  "{\"arazzo\":\"1.1.0\",\"info\":{\"title\":\"t\",\"version\":"
                  "\"1\"},\"sourceDescriptions\":[{\"name\":\"s\",\"url\":"
                  "\"u\"}],\"components\":{},\"workflows\":[{\"workflowId\":"
                  "\"w\",\"steps\":[{\"stepId\":\"s1\",\"operationId\":\"o1\","
                  "\"requestBody\":{\"contentType\":\"text/"
                  "plain\"},\"parameters\":[{\"reference\":\"$components."
                  "parameters.Missing\"}]}]}]}",
                  &spec) == CDD_C_ERROR_INVALID_ARGUMENT
                  ? CDD_C_SUCCESS
                  : CDD_C_ERROR_INVALID_ARGUMENT);
  }

  json_value_free(root_val);
  PASS();
}

SUITE(arazzo_parser_suite) {
  RUN_TEST(test_arazzo_parse_and_emit_roundtrip);
  RUN_TEST(test_arazzo_parse_and_validate_errors);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_ARAZZO_PARSER_H */
