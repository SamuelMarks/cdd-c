/**
 * @file test_aggregator.h
 * @brief Unit tests for OpenAPI Aggregator logic.
 *
 * Verifies path grouping, deduplication, and memory management.
 *
 * @author Samuel Marks
 */

#ifndef TEST_AGGREGATOR_H
#define TEST_AGGREGATOR_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "openapi/parse/openapi.h"
#include "routes/emit/aggregator.h"
/* clang-format on */

/* Moved extern declarations for C89 compliance */
extern int g_cdd_aggregator_fail_path_realloc;
extern int g_cdd_aggregator_fail_ops_realloc;
extern int g_cdd_aggregator_fail_route_strdup;

static void dummy_op(struct OpenAPI_Operation *op, const char *id) {
  memset(op, 0, sizeof(*op));
  /* Allocate something to test ownership transfer */
  if (id) {
    op->operation_id = (char *)malloc(strlen(id) + 1);
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
    strcpy_s(op->operation_id, strlen(id) + 1, id);
#else
#if defined(_MSC_VER)
    strcpy_s(op->operation_id, strlen(id) + 1, id);
#else
#if defined(_MSC_VER)
    strcpy_s(op->operation_id, strlen(id) + 1, id);
#else
    strcpy(op->operation_id, id);
#endif
#endif
#endif
  }
}

/**
 * @brief test_aggregator_add_new
 * @return TEST
 */
TEST test_aggregator_add_new(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op = {0};

  (void)openapi_spec_init(&spec);
  dummy_op(&op, "op1");

  ASSERT_EQ(0, openapi_aggregator_add_operation(&spec, "/users", &op));

  ASSERT_EQ(1, spec.n_paths);
  ASSERT_STR_EQ("/users", spec.paths[0].route);
  ASSERT_EQ(1, spec.paths[0].n_operations);
  ASSERT_STR_EQ("op1", spec.paths[0].operations[0].operation_id);

  /* Verify the input op was zeroed (ownership transfer) */
  ASSERT_EQ(NULL, op.operation_id);

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief test_aggregator_merge_paths
 * @return TEST
 */
TEST test_aggregator_merge_paths(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op1, op2;

  (void)openapi_spec_init(&spec);
  dummy_op(&op1, "getUsers");
  op1.verb = OA_VERB_GET;

  dummy_op(&op2, "createUser");
  op2.verb = OA_VERB_POST;

  ASSERT_EQ(0, openapi_aggregator_add_operation(&spec, "/users", &op1));
  ASSERT_EQ(0, openapi_aggregator_add_operation(&spec, "/users", &op2));

  /* Should still have only 1 path item */
  ASSERT_EQ(1, spec.n_paths);
  ASSERT_STR_EQ("/users", spec.paths[0].route);

  /* But 2 operations inside */
  ASSERT_EQ(2, spec.paths[0].n_operations);
  ASSERT_STR_EQ("getUsers", spec.paths[0].operations[0].operation_id);
  ASSERT_STR_EQ("createUser", spec.paths[0].operations[1].operation_id);

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief test_aggregator_distinct_paths
 * @return TEST
 */
TEST test_aggregator_distinct_paths(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op1, op2;

  (void)openapi_spec_init(&spec);
  dummy_op(&op1, "opA");
  dummy_op(&op2, "opB");

  ASSERT_EQ(0, openapi_aggregator_add_operation(&spec, "/a", &op1));
  ASSERT_EQ(0, openapi_aggregator_add_operation(&spec, "/b", &op2));

  ASSERT_EQ(2, spec.n_paths);
  ASSERT_STR_EQ("/a", spec.paths[0].route);
  ASSERT_STR_EQ("/b", spec.paths[1].route);

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief test_aggregator_add_additional_operation
 * @return TEST
 */
TEST test_aggregator_add_additional_operation(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op = {0};

  (void)openapi_spec_init(&spec);
  dummy_op(&op, "copyUser");
  op.is_additional = 1;
  op.method = strdup("COPY");
  op.verb = OA_VERB_UNKNOWN;

  ASSERT_EQ(0, openapi_aggregator_add_operation(&spec, "/users/{id}", &op));

  ASSERT_EQ(1, spec.n_paths);
  ASSERT_STR_EQ("/users/{id}", spec.paths[0].route);
  ASSERT_EQ(0, spec.paths[0].n_operations);
  ASSERT_EQ(1, spec.paths[0].n_additional_operations);
  ASSERT_STR_EQ("copyUser",
                spec.paths[0].additional_operations[0].operation_id);
  ASSERT_STR_EQ("COPY", spec.paths[0].additional_operations[0].method);

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief test_aggregator_add_webhook
 * @return TEST
 */
TEST test_aggregator_add_webhook(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op = {0};

  (void)openapi_spec_init(&spec);
  dummy_op(&op, "webhookOp");
  op.verb = OA_VERB_POST;

  ASSERT_EQ(0, openapi_aggregator_add_webhook_operation(&spec, "/events", &op));

  ASSERT_EQ(0, spec.n_paths);
  ASSERT_EQ(1, spec.n_webhooks);
  ASSERT_STR_EQ("/events", spec.webhooks[0].route);
  ASSERT_EQ(1, spec.webhooks[0].n_operations);
  ASSERT_STR_EQ("webhookOp", spec.webhooks[0].operations[0].operation_id);
  ASSERT_EQ(NULL, op.operation_id);

  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief test_aggregator_bad_args
 * @return TEST
 */
TEST test_aggregator_bad_args(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op = {0};
  (void)openapi_spec_init(&spec);
  dummy_op(&op, "x");

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            openapi_aggregator_add_operation(NULL, "/a", &op));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            openapi_aggregator_add_operation(&spec, NULL, &op));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            openapi_aggregator_add_operation(&spec, "/a", NULL));

  /* Since calls failed, ownership RETAINED by op. We must free it manually
   * here. */
  free(op.operation_id);
  openapi_spec_free(&spec);
  g_fail_io_after = -1;
  PASS();
}

/* OOM tracking */
#ifdef CDD_BUILD_TESTS
TEST test_aggregator_oom(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op = {0};
  int i;

  (void)openapi_spec_init(&spec);
  dummy_op(&op, "op1");

  /* Test path alloc failure */
  /* extern int g_cdd_aggregator_fail_path_realloc; (moved to global) */
  /* extern int g_cdd_aggregator_fail_ops_realloc; (moved to global) */
  /* extern int g_cdd_aggregator_fail_route_strdup; (moved to global) */

  g_cdd_aggregator_fail_path_realloc = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            openapi_aggregator_add_operation(&spec, "/users", &op));
  g_cdd_aggregator_fail_path_realloc = 0;

  /* Test route strdup failure */
  g_cdd_aggregator_fail_route_strdup = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            openapi_aggregator_add_operation(&spec, "/users", &op));
  g_cdd_aggregator_fail_route_strdup = 0;

  /* Test op alloc failure */
  g_cdd_aggregator_fail_ops_realloc = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            openapi_aggregator_add_operation(&spec, "/users", &op));
  g_cdd_aggregator_fail_ops_realloc = 0;

  /* Test webhook add alloc failure */
  g_cdd_aggregator_fail_path_realloc = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            openapi_aggregator_add_webhook_operation(&spec, "/users", &op));
  g_cdd_aggregator_fail_path_realloc = 0;

  /* Test webhook op alloc failure */
  g_cdd_aggregator_fail_ops_realloc = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            openapi_aggregator_add_webhook_operation(&spec, "/users", &op));
  g_cdd_aggregator_fail_ops_realloc = 0;

  /* Clean up the dangling op */
  free(op.operation_id);
  openapi_spec_free(&spec);
  PASS();
}

TEST test_aggregator_webhook_additional(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op = {0};
  (void)openapi_spec_init(&spec);
  dummy_op(&op, "op1");
  op.is_additional = 1;
  ASSERT_EQ(0, openapi_aggregator_add_webhook_operation(&spec, "/hooks", &op));
  openapi_spec_free(&spec);
  PASS();
}

TEST test_aggregator_webhook_bad_args(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op = {0};
  (void)openapi_spec_init(&spec);
  dummy_op(&op, "op1");
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            openapi_aggregator_add_webhook_operation(NULL, "/hooks", &op));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            openapi_aggregator_add_webhook_operation(&spec, NULL, &op));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            openapi_aggregator_add_webhook_operation(&spec, "/hooks", NULL));
  free(op.operation_id);
  openapi_spec_free(&spec);
  PASS();
}
TEST test_aggregator_webhook_existing(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op1 = {0};
  struct OpenAPI_Operation op2 = {0};
  (void)openapi_spec_init(&spec);
  dummy_op(&op1, "hook1");
  dummy_op(&op2, "hook2");

  /* Add first hook to create path */
  ASSERT_EQ(CDD_C_SUCCESS,
            openapi_aggregator_add_webhook_operation(&spec, "/hooks", &op1));
  /* Add second hook to trigger find branch */
  ASSERT_EQ(CDD_C_SUCCESS,
            openapi_aggregator_add_webhook_operation(&spec, "/hooks", &op2));

  ASSERT_EQ(1, spec.n_webhooks);
  ASSERT_EQ(2, spec.webhooks[0].n_operations);

  openapi_spec_free(&spec);
  PASS();
}
#endif

/**
 * @brief aggregator_suite
 */
SUITE(aggregator_suite) {
  RUN_TEST(test_aggregator_add_new);
  RUN_TEST(test_aggregator_merge_paths);
  RUN_TEST(test_aggregator_distinct_paths);
  RUN_TEST(test_aggregator_add_additional_operation);
  RUN_TEST(test_aggregator_add_webhook);
  RUN_TEST(test_aggregator_bad_args);
#ifdef CDD_BUILD_TESTS
  RUN_TEST(test_aggregator_oom);
  RUN_TEST(test_aggregator_webhook_additional);
  RUN_TEST(test_aggregator_webhook_bad_args);
  RUN_TEST(test_aggregator_webhook_existing);
#endif
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_AGGREGATOR_H */
