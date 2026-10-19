/**
 * @file test_arazzo_types.h
 * @brief Unit tests for Arazzo v1.1.0 Core IR types, lifecycle, and enum
 * conversions.
 *
 * Tests 100% of functions, lines, and branches in
 * `src/arazzo/parse/arazzo_free.c` and `src/arazzo/parse/arazzo_types.h`,
 * including OOM fault-injection paths.
 *
 * @author Samuel Marks
 */

#ifndef TEST_ARAZZO_TYPES_H
#define TEST_ARAZZO_TYPES_H

/* clang-format off */
#include <greatest.h>
#include <stdlib.h>
#include <string.h>
#include "arazzo/parse/arazzo_types.h"
#include "c_cdd/memory.h"
#include "openapi/parse/openapi_internal.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief Tests `arazzo_spec_init` and `arazzo_spec_free` including NULL and OOM
 * branches.
 */
TEST test_arazzo_spec_init_and_free(void) {
  struct Arazzo_Spec spec;
  cdd_c_error_t rc;

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, arazzo_spec_init(NULL));
  arazzo_spec_free(NULL);
  arazzo_runtime_expr_free(NULL);
  arazzo_extension_list_free(NULL);

  g_cdd_alloc_fail = 1;
  rc = arazzo_spec_init(&spec);
  g_cdd_alloc_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);

  rc = arazzo_spec_init(&spec);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_STR_EQ("1.1.0", spec.arazzo);
  ASSERT_EQ(ARAZZO_VER_1_1_0, spec.version);

  arazzo_spec_free(&spec);
  ASSERT_EQ(NULL, spec.arazzo);
  PASS();
}

/**
 * @brief Tests deep cleanup of a fully populated `Arazzo_Spec` tree.
 */
TEST test_arazzo_spec_free_populated(void) {
  struct Arazzo_Spec spec;
  cdd_c_error_t rc;

  rc = arazzo_spec_init(&spec);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  spec.self_uri = C_CDD_STRDUP("https://example.com/arazzo.yaml");
  spec.info.title = C_CDD_STRDUP("Petstore Workflow");
  spec.info.summary = C_CDD_STRDUP("Summary");
  spec.info.description = C_CDD_STRDUP("Description");
  spec.info.version = C_CDD_STRDUP("1.0.0");

  spec.n_source_descriptions = 2;
  spec.source_descriptions = (struct Arazzo_SourceDescription *)C_CDD_CALLOC(
      2, sizeof(struct Arazzo_SourceDescription));
  spec.source_descriptions[0].name = C_CDD_STRDUP("petstore");
  spec.source_descriptions[0].url = C_CDD_STRDUP("./openapi.json");
  spec.source_descriptions[0].type = ARAZZO_SOURCE_OPENAPI;
  spec.source_descriptions[0].resolved_openapi =
      (struct OpenAPI_Spec *)C_CDD_CALLOC(1, sizeof(struct OpenAPI_Spec));
  spec.source_descriptions[1].name = C_CDD_STRDUP("events");
  spec.source_descriptions[1].url = C_CDD_STRDUP("./asyncapi.json");
  spec.source_descriptions[1].type = ARAZZO_SOURCE_ASYNCAPI;

  spec.n_workflows = 1;
  spec.workflows =
      (struct Arazzo_Workflow *)C_CDD_CALLOC(1, sizeof(struct Arazzo_Workflow));
  spec.workflows[0].workflow_id = C_CDD_STRDUP("loginAndBuy");
  spec.workflows[0].summary = C_CDD_STRDUP("wf summary");
  spec.workflows[0].description = C_CDD_STRDUP("wf desc");
  spec.workflows[0].n_depends_on = 1;
  spec.workflows[0].depends_on = (char **)C_CDD_CALLOC(1, sizeof(char *));
  spec.workflows[0].depends_on[0] = C_CDD_STRDUP("prereqWf");

  spec.workflows[0].n_parameters = 1;
  spec.workflows[0].parameters = (struct Arazzo_Parameter *)C_CDD_CALLOC(
      1, sizeof(struct Arazzo_Parameter));
  spec.workflows[0].parameters[0].name = C_CDD_STRDUP("X-Trace");

  spec.workflows[0].n_success_actions = 1;
  spec.workflows[0].success_actions =
      (struct Arazzo_SuccessAction *)C_CDD_CALLOC(
          1, sizeof(struct Arazzo_SuccessAction));
  spec.workflows[0].success_actions[0].name = C_CDD_STRDUP("wfDone");
  spec.workflows[0].success_actions[0].n_parameters = 1;
  spec.workflows[0].success_actions[0].parameters =
      (struct Arazzo_Parameter *)C_CDD_CALLOC(1,
                                              sizeof(struct Arazzo_Parameter));
  spec.workflows[0].success_actions[0].n_criteria = 1;
  spec.workflows[0].success_actions[0].criteria =
      (struct Arazzo_Criterion *)C_CDD_CALLOC(1,
                                              sizeof(struct Arazzo_Criterion));

  spec.workflows[0].n_failure_actions = 1;
  spec.workflows[0].failure_actions =
      (struct Arazzo_FailureAction *)C_CDD_CALLOC(
          1, sizeof(struct Arazzo_FailureAction));
  spec.workflows[0].failure_actions[0].name = C_CDD_STRDUP("wfFail");

  spec.workflows[0].n_outputs = 1;
  spec.workflows[0].outputs = (struct Arazzo_OutputMapEntry *)C_CDD_CALLOC(
      1, sizeof(struct Arazzo_OutputMapEntry));
  spec.workflows[0].outputs[0].key = C_CDD_STRDUP("orderId");
  spec.workflows[0].outputs[0].expression_or_value =
      C_CDD_STRDUP("$steps.step1.outputs.id");

  spec.workflows[0].n_steps = 1;
  spec.workflows[0].steps =
      (struct Arazzo_Step *)C_CDD_CALLOC(1, sizeof(struct Arazzo_Step));
  spec.workflows[0].steps[0].step_id = C_CDD_STRDUP("step1");
  spec.workflows[0].steps[0].description = C_CDD_STRDUP("First step");
  spec.workflows[0].steps[0].operation_id = C_CDD_STRDUP("placeOrder");
  spec.workflows[0].steps[0].correlation_id = C_CDD_STRDUP("$inputs.corrId");
  spec.workflows[0].steps[0].n_parameters = 1;
  spec.workflows[0].steps[0].parameters =
      (struct Arazzo_Parameter *)C_CDD_CALLOC(1,
                                              sizeof(struct Arazzo_Parameter));
  spec.workflows[0].steps[0].request_body.content_type =
      C_CDD_STRDUP("application/json");
  spec.workflows[0].steps[0].request_body.n_replacements = 1;
  spec.workflows[0].steps[0].request_body.replacements =
      (struct Arazzo_PayloadReplacement *)C_CDD_CALLOC(
          1, sizeof(struct Arazzo_PayloadReplacement));
  spec.workflows[0].steps[0].request_body.replacements[0].target =
      C_CDD_STRDUP("/petId");
  spec.workflows[0].steps[0].n_success_criteria = 1;
  spec.workflows[0].steps[0].success_criteria =
      (struct Arazzo_Criterion *)C_CDD_CALLOC(1,
                                              sizeof(struct Arazzo_Criterion));
  spec.workflows[0].steps[0].n_on_success = 1;
  spec.workflows[0].steps[0].on_success =
      (struct Arazzo_SuccessAction *)C_CDD_CALLOC(
          1, sizeof(struct Arazzo_SuccessAction));
  spec.workflows[0].steps[0].n_on_failure = 1;
  spec.workflows[0].steps[0].on_failure =
      (struct Arazzo_FailureAction *)C_CDD_CALLOC(
          1, sizeof(struct Arazzo_FailureAction));
  spec.workflows[0].steps[0].n_outputs = 1;
  spec.workflows[0].steps[0].outputs =
      (struct Arazzo_OutputMapEntry *)C_CDD_CALLOC(
          1, sizeof(struct Arazzo_OutputMapEntry));
  spec.workflows[0].steps[0].n_depends_on = 1;
  spec.workflows[0].steps[0].depends_on =
      (char **)C_CDD_CALLOC(1, sizeof(char *));
  spec.workflows[0].steps[0].depends_on[0] = C_CDD_STRDUP("step0");

  spec.components.n_inputs = 1;
  spec.components.inputs = (struct OpenAPI_SchemaRef *)C_CDD_CALLOC(
      1, sizeof(struct OpenAPI_SchemaRef));
  spec.components.input_keys = (char **)C_CDD_CALLOC(1, sizeof(char *));
  spec.components.input_keys[0] = C_CDD_STRDUP("UserInput");

  spec.components.n_parameters = 1;
  spec.components.parameters = (struct Arazzo_Parameter *)C_CDD_CALLOC(
      1, sizeof(struct Arazzo_Parameter));
  spec.components.parameter_keys = (char **)C_CDD_CALLOC(1, sizeof(char *));
  spec.components.parameter_keys[0] = C_CDD_STRDUP("TokenParam");

  spec.components.n_success_actions = 1;
  spec.components.success_actions = (struct Arazzo_SuccessAction *)C_CDD_CALLOC(
      1, sizeof(struct Arazzo_SuccessAction));
  spec.components.success_action_keys =
      (char **)C_CDD_CALLOC(1, sizeof(char *));
  spec.components.success_action_keys[0] = C_CDD_STRDUP("EndAction");

  spec.components.n_failure_actions = 1;
  spec.components.failure_actions = (struct Arazzo_FailureAction *)C_CDD_CALLOC(
      1, sizeof(struct Arazzo_FailureAction));
  spec.components.failure_action_keys =
      (char **)C_CDD_CALLOC(1, sizeof(char *));
  spec.components.failure_action_keys[0] = C_CDD_STRDUP("RetryAction");

  arazzo_spec_free(&spec);
  ASSERT_EQ(0, spec.n_workflows);
  PASS();
}

/**
 * @brief Tests `arazzo_extension_list_append` and `arazzo_extension_list_free`
 * including error and OOM paths.
 */
TEST test_arazzo_extension_list(void) {
  struct Arazzo_ExtensionList list;
  struct OpenAPI_Any val;
  cdd_c_error_t rc;

  memset(&list, 0, sizeof(list));
  memset(&val, 0, sizeof(val));
  val.type = OA_ANY_STRING;
  val.string = C_CDD_STRDUP("custom_value");

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_extension_list_append(NULL, "x-ext", &val));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_extension_list_append(&list, NULL, &val));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_extension_list_append(&list, "x-ext", NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_extension_list_append(&list, "invalid", &val));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_extension_list_append(&list, "x-", &val));

  /* OOM on strdup(key) */
  g_cdd_alloc_fail = 1;
  rc = arazzo_extension_list_append(&list, "x-test", &val);
  g_cdd_alloc_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);

  /* OOM on realloc(list->items) */
  g_cdd_alloc_fail = 2;
  rc = arazzo_extension_list_append(&list, "x-test", &val);
  g_cdd_alloc_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);

  /* OOM inside copy_any_value */
  g_cdd_strdup_fail = 1;
  rc = arazzo_extension_list_append(&list, "x-test", &val);
  g_cdd_strdup_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);

  rc = arazzo_extension_list_append(&list, "x-test", &val);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, list.count);
  ASSERT_STR_EQ("x-test", list.items[0].key);
  ASSERT_STR_EQ("custom_value", list.items[0].val.string);

  arazzo_extension_list_free(&list);
  free_any_value(&val);
  ASSERT_EQ(0, list.count);
  PASS();
}

/**
 * @brief Tests enum conversion helpers for SourceType, ParamIn, and ActionType.
 */
TEST test_arazzo_enum_conversions(void) {
  const char *str = NULL;
  enum Arazzo_SourceType st = ARAZZO_SOURCE_UNKNOWN;
  enum Arazzo_ParamIn pi = ARAZZO_PARAM_IN_UNKNOWN;
  enum Arazzo_ActionType at = ARAZZO_ACTION_UNKNOWN;

  /* SourceType */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_source_type_to_str(ARAZZO_SOURCE_OPENAPI, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_source_type_to_str(ARAZZO_SOURCE_UNKNOWN, &str));
  ASSERT_EQ(CDD_C_SUCCESS,
            arazzo_source_type_to_str(ARAZZO_SOURCE_OPENAPI, &str));
  ASSERT_STR_EQ("openapi", str);
  ASSERT_EQ(CDD_C_SUCCESS,
            arazzo_source_type_to_str(ARAZZO_SOURCE_ASYNCAPI, &str));
  ASSERT_STR_EQ("asyncapi", str);
  ASSERT_EQ(CDD_C_SUCCESS,
            arazzo_source_type_to_str(ARAZZO_SOURCE_ARAZZO, &str));
  ASSERT_STR_EQ("arazzo", str);

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_source_type_from_str(NULL, &st));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_source_type_from_str("openapi", NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_source_type_from_str("bogus", &st));
  ASSERT_EQ(ARAZZO_SOURCE_UNKNOWN, st);
  ASSERT_EQ(CDD_C_SUCCESS, arazzo_source_type_from_str("openapi", &st));
  ASSERT_EQ(ARAZZO_SOURCE_OPENAPI, st);
  ASSERT_EQ(CDD_C_SUCCESS, arazzo_source_type_from_str("asyncapi", &st));
  ASSERT_EQ(ARAZZO_SOURCE_ASYNCAPI, st);
  ASSERT_EQ(CDD_C_SUCCESS, arazzo_source_type_from_str("arazzo", &st));
  ASSERT_EQ(ARAZZO_SOURCE_ARAZZO, st);

  /* ParamIn */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_param_in_to_str(ARAZZO_PARAM_IN_PATH, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_param_in_to_str(ARAZZO_PARAM_IN_UNKNOWN, &str));
  ASSERT_EQ(CDD_C_SUCCESS, arazzo_param_in_to_str(ARAZZO_PARAM_IN_PATH, &str));
  ASSERT_STR_EQ("path", str);
  ASSERT_EQ(CDD_C_SUCCESS, arazzo_param_in_to_str(ARAZZO_PARAM_IN_QUERY, &str));
  ASSERT_STR_EQ("query", str);
  ASSERT_EQ(CDD_C_SUCCESS,
            arazzo_param_in_to_str(ARAZZO_PARAM_IN_QUERYSTRING, &str));
  ASSERT_STR_EQ("querystring", str);
  ASSERT_EQ(CDD_C_SUCCESS,
            arazzo_param_in_to_str(ARAZZO_PARAM_IN_HEADER, &str));
  ASSERT_STR_EQ("header", str);
  ASSERT_EQ(CDD_C_SUCCESS,
            arazzo_param_in_to_str(ARAZZO_PARAM_IN_COOKIE, &str));
  ASSERT_STR_EQ("cookie", str);

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, arazzo_param_in_from_str(NULL, &pi));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_param_in_from_str("path", NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_param_in_from_str("bogus", &pi));
  ASSERT_EQ(ARAZZO_PARAM_IN_UNKNOWN, pi);
  ASSERT_EQ(CDD_C_SUCCESS, arazzo_param_in_from_str("path", &pi));
  ASSERT_EQ(ARAZZO_PARAM_IN_PATH, pi);
  ASSERT_EQ(CDD_C_SUCCESS, arazzo_param_in_from_str("query", &pi));
  ASSERT_EQ(ARAZZO_PARAM_IN_QUERY, pi);
  ASSERT_EQ(CDD_C_SUCCESS, arazzo_param_in_from_str("querystring", &pi));
  ASSERT_EQ(ARAZZO_PARAM_IN_QUERYSTRING, pi);
  ASSERT_EQ(CDD_C_SUCCESS, arazzo_param_in_from_str("header", &pi));
  ASSERT_EQ(ARAZZO_PARAM_IN_HEADER, pi);
  ASSERT_EQ(CDD_C_SUCCESS, arazzo_param_in_from_str("cookie", &pi));
  ASSERT_EQ(ARAZZO_PARAM_IN_COOKIE, pi);

  /* ActionType */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_action_type_to_str(ARAZZO_ACTION_END, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_action_type_to_str(ARAZZO_ACTION_UNKNOWN, &str));
  ASSERT_EQ(CDD_C_SUCCESS, arazzo_action_type_to_str(ARAZZO_ACTION_END, &str));
  ASSERT_STR_EQ("end", str);
  ASSERT_EQ(CDD_C_SUCCESS, arazzo_action_type_to_str(ARAZZO_ACTION_GOTO, &str));
  ASSERT_STR_EQ("goto", str);
  ASSERT_EQ(CDD_C_SUCCESS,
            arazzo_action_type_to_str(ARAZZO_ACTION_RETRY, &str));
  ASSERT_STR_EQ("retry", str);

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_action_type_from_str(NULL, &at));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_action_type_from_str("end", NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_action_type_from_str("bogus", &at));
  ASSERT_EQ(ARAZZO_ACTION_UNKNOWN, at);
  ASSERT_EQ(CDD_C_SUCCESS, arazzo_action_type_from_str("end", &at));
  ASSERT_EQ(ARAZZO_ACTION_END, at);
  ASSERT_EQ(CDD_C_SUCCESS, arazzo_action_type_from_str("goto", &at));
  ASSERT_EQ(ARAZZO_ACTION_GOTO, at);
  ASSERT_EQ(CDD_C_SUCCESS, arazzo_action_type_from_str("retry", &at));
  ASSERT_EQ(ARAZZO_ACTION_RETRY, at);

  PASS();
}

/**
 * @brief Tests `arazzo_runtime_expr_copy` including NULL fields and all OOM
 * branches.
 */
TEST test_arazzo_runtime_expr_copy(void) {
  struct Arazzo_RuntimeExpr src;
  struct Arazzo_RuntimeExpr dst;
  cdd_c_error_t rc;
  int i;

  memset(&src, 0, sizeof(src));
  memset(&dst, 0, sizeof(dst));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, arazzo_runtime_expr_copy(NULL, &src));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, arazzo_runtime_expr_copy(&dst, NULL));

  /* Copy empty struct (all pointers NULL) */
  rc = arazzo_runtime_expr_copy(&dst, &src);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  arazzo_runtime_expr_free(&dst);

  src.kind = ARAZZO_EXPR_STEPS;
  src.is_embedded_template = 1;
  src.raw = C_CDD_STRDUP("$steps.s1.outputs.val#/ptr");
  src.source_name = C_CDD_STRDUP("petstore");
  src.step_or_workflow_id = C_CDD_STRDUP("s1");
  src.property_name = C_CDD_STRDUP("val");
  src.json_pointer = C_CDD_STRDUP("/ptr");

  for (i = 1; i <= 5; ++i) {
    g_cdd_alloc_fail = i;
    rc = arazzo_runtime_expr_copy(&dst, &src);
    g_cdd_alloc_fail = 0;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  }

  rc = arazzo_runtime_expr_copy(&dst, &src);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(ARAZZO_EXPR_STEPS, dst.kind);
  ASSERT_EQ(1, dst.is_embedded_template);
  ASSERT_STR_EQ("$steps.s1.outputs.val#/ptr", dst.raw);
  ASSERT_STR_EQ("petstore", dst.source_name);
  ASSERT_STR_EQ("s1", dst.step_or_workflow_id);
  ASSERT_STR_EQ("val", dst.property_name);
  ASSERT_STR_EQ("/ptr", dst.json_pointer);

  arazzo_runtime_expr_free(&src);
  arazzo_runtime_expr_free(&dst);
  PASS();
}

SUITE(arazzo_types_suite) {
  RUN_TEST(test_arazzo_spec_init_and_free);
  RUN_TEST(test_arazzo_spec_free_populated);
  RUN_TEST(test_arazzo_extension_list);
  RUN_TEST(test_arazzo_enum_conversions);
  RUN_TEST(test_arazzo_runtime_expr_copy);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_ARAZZO_TYPES_H */
