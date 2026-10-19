/**
 * @file test_arazzo_expr.h
 * @brief Exhaustive unit tests for Arazzo v1.1.0 runtime expression parser and
 * formatter.
 *
 * Exercises 100% of functions, lines, and branches in
 * `src/arazzo/parse/arazzo_expr.c`, including all 17 runtime expression roots,
 * embedded templates, error conditions, and OOM fault-injection paths.
 *
 * @author Samuel Marks
 */

#ifndef TEST_ARAZZO_EXPR_H
#define TEST_ARAZZO_EXPR_H

/* clang-format off */
#include <greatest.h>
#include <stdlib.h>
#include <string.h>
#include "arazzo/parse/arazzo_expr.h"
#include "c_cdd/memory.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief Tests `arazzo_runtime_expr_parse` on all 17 Arazzo 1.1.0 runtime
 * expression types and templates.
 */
TEST test_arazzo_runtime_expr_parse_valid(void) {
  struct Arazzo_RuntimeExpr expr;

  /* Literals & templates */
  ASSERT_EQ(CDD_C_SUCCESS, arazzo_runtime_expr_parse("plain_literal", &expr));
  ASSERT_EQ(ARAZZO_EXPR_LITERAL, expr.kind);
  ASSERT_EQ(0, expr.is_embedded_template);
  arazzo_runtime_expr_free(&expr);

  ASSERT_EQ(CDD_C_SUCCESS,
            arazzo_runtime_expr_parse("Bearer {$steps.s1.outputs.tok}", &expr));
  ASSERT_EQ(ARAZZO_EXPR_LITERAL, expr.kind);
  ASSERT_EQ(1, expr.is_embedded_template);
  arazzo_runtime_expr_free(&expr);

  /* Simple roots */
  ASSERT_EQ(CDD_C_SUCCESS, arazzo_runtime_expr_parse("$url", &expr));
  ASSERT_EQ(ARAZZO_EXPR_URL, expr.kind);
  arazzo_runtime_expr_free(&expr);

  ASSERT_EQ(CDD_C_SUCCESS, arazzo_runtime_expr_parse("$method", &expr));
  ASSERT_EQ(ARAZZO_EXPR_METHOD, expr.kind);
  arazzo_runtime_expr_free(&expr);

  ASSERT_EQ(CDD_C_SUCCESS, arazzo_runtime_expr_parse("$statusCode", &expr));
  ASSERT_EQ(ARAZZO_EXPR_STATUS_CODE, expr.kind);
  arazzo_runtime_expr_free(&expr);

  ASSERT_EQ(CDD_C_SUCCESS, arazzo_runtime_expr_parse("$self", &expr));
  ASSERT_EQ(ARAZZO_EXPR_SELF, expr.kind);
  arazzo_runtime_expr_free(&expr);

  /* Request expressions */
  ASSERT_EQ(CDD_C_SUCCESS,
            arazzo_runtime_expr_parse("$request.header.Authorization", &expr));
  ASSERT_EQ(ARAZZO_EXPR_REQUEST_HEADER, expr.kind);
  ASSERT_STR_EQ("Authorization", expr.property_name);
  arazzo_runtime_expr_free(&expr);

  ASSERT_EQ(CDD_C_SUCCESS,
            arazzo_runtime_expr_parse("$request.query.limit", &expr));
  ASSERT_EQ(ARAZZO_EXPR_REQUEST_QUERY, expr.kind);
  ASSERT_STR_EQ("limit", expr.property_name);
  arazzo_runtime_expr_free(&expr);

  ASSERT_EQ(CDD_C_SUCCESS,
            arazzo_runtime_expr_parse("$request.path.petId", &expr));
  ASSERT_EQ(ARAZZO_EXPR_REQUEST_PATH, expr.kind);
  ASSERT_STR_EQ("petId", expr.property_name);
  arazzo_runtime_expr_free(&expr);

  ASSERT_EQ(CDD_C_SUCCESS, arazzo_runtime_expr_parse("$request.body", &expr));
  ASSERT_EQ(ARAZZO_EXPR_REQUEST_BODY, expr.kind);
  ASSERT_EQ(NULL, expr.json_pointer);
  arazzo_runtime_expr_free(&expr);

  ASSERT_EQ(CDD_C_SUCCESS,
            arazzo_runtime_expr_parse("$request.body#/user/id", &expr));
  ASSERT_EQ(ARAZZO_EXPR_REQUEST_BODY, expr.kind);
  ASSERT_STR_EQ("/user/id", expr.json_pointer);
  arazzo_runtime_expr_free(&expr);

  /* Response expressions */
  ASSERT_EQ(CDD_C_SUCCESS,
            arazzo_runtime_expr_parse("$response.header.X-Rate-Limit", &expr));
  ASSERT_EQ(ARAZZO_EXPR_RESPONSE_HEADER, expr.kind);
  ASSERT_STR_EQ("X-Rate-Limit", expr.property_name);
  arazzo_runtime_expr_free(&expr);

  ASSERT_EQ(CDD_C_SUCCESS, arazzo_runtime_expr_parse("$response.body", &expr));
  ASSERT_EQ(ARAZZO_EXPR_RESPONSE_BODY, expr.kind);
  ASSERT_EQ(NULL, expr.json_pointer);
  arazzo_runtime_expr_free(&expr);

  ASSERT_EQ(CDD_C_SUCCESS,
            arazzo_runtime_expr_parse("$response.body#/data/0/token", &expr));
  ASSERT_EQ(ARAZZO_EXPR_RESPONSE_BODY, expr.kind);
  ASSERT_STR_EQ("/data/0/token", expr.json_pointer);
  arazzo_runtime_expr_free(&expr);

  /* Message expressions */
  ASSERT_EQ(CDD_C_SUCCESS, arazzo_runtime_expr_parse("$message.header", &expr));
  ASSERT_EQ(ARAZZO_EXPR_MESSAGE_HEADER, expr.kind);
  arazzo_runtime_expr_free(&expr);

  ASSERT_EQ(CDD_C_SUCCESS,
            arazzo_runtime_expr_parse("$message.header#/corrId", &expr));
  ASSERT_EQ(ARAZZO_EXPR_MESSAGE_HEADER, expr.kind);
  ASSERT_STR_EQ("/corrId", expr.json_pointer);
  arazzo_runtime_expr_free(&expr);

  ASSERT_EQ(CDD_C_SUCCESS,
            arazzo_runtime_expr_parse("$message.header.X-Msg-Id", &expr));
  ASSERT_EQ(ARAZZO_EXPR_MESSAGE_HEADER, expr.kind);
  ASSERT_STR_EQ("X-Msg-Id", expr.property_name);
  arazzo_runtime_expr_free(&expr);

  ASSERT_EQ(CDD_C_SUCCESS,
            arazzo_runtime_expr_parse("$message.payload", &expr));
  ASSERT_EQ(ARAZZO_EXPR_MESSAGE_PAYLOAD, expr.kind);
  arazzo_runtime_expr_free(&expr);

  ASSERT_EQ(CDD_C_SUCCESS,
            arazzo_runtime_expr_parse("$message.payload#/event/id", &expr));
  ASSERT_EQ(ARAZZO_EXPR_MESSAGE_PAYLOAD, expr.kind);
  ASSERT_STR_EQ("/event/id", expr.json_pointer);
  arazzo_runtime_expr_free(&expr);

  /* Inputs & Outputs */
  ASSERT_EQ(CDD_C_SUCCESS,
            arazzo_runtime_expr_parse("$inputs.username#/sub", &expr));
  ASSERT_EQ(ARAZZO_EXPR_INPUTS, expr.kind);
  ASSERT_STR_EQ("username", expr.property_name);
  ASSERT_STR_EQ("/sub", expr.json_pointer);
  arazzo_runtime_expr_free(&expr);

  ASSERT_EQ(CDD_C_SUCCESS,
            arazzo_runtime_expr_parse("$outputs.resultId", &expr));
  ASSERT_EQ(ARAZZO_EXPR_OUTPUTS, expr.kind);
  ASSERT_STR_EQ("resultId", expr.property_name);
  arazzo_runtime_expr_free(&expr);

  /* Steps, Workflows, SourceDescriptions, Components */
  ASSERT_EQ(CDD_C_SUCCESS, arazzo_runtime_expr_parse(
                               "$steps.login.outputs.token#/jwt", &expr));
  ASSERT_EQ(ARAZZO_EXPR_STEPS, expr.kind);
  ASSERT_STR_EQ("login", expr.step_or_workflow_id);
  ASSERT_STR_EQ("token", expr.property_name);
  ASSERT_STR_EQ("/jwt", expr.json_pointer);
  arazzo_runtime_expr_free(&expr);

  ASSERT_EQ(CDD_C_SUCCESS,
            arazzo_runtime_expr_parse("$workflows.auth.inputs.user", &expr));
  ASSERT_EQ(ARAZZO_EXPR_WORKFLOWS, expr.kind);
  ASSERT_STR_EQ("auth", expr.step_or_workflow_id);
  ASSERT_STR_EQ("user", expr.property_name);
  arazzo_runtime_expr_free(&expr);

  ASSERT_EQ(CDD_C_SUCCESS, arazzo_runtime_expr_parse(
                               "$sourceDescriptions.petstore.url", &expr));
  ASSERT_EQ(ARAZZO_EXPR_SOURCE_DESCRIPTIONS, expr.kind);
  ASSERT_STR_EQ("petstore", expr.source_name);
  ASSERT_STR_EQ("url", expr.property_name);
  arazzo_runtime_expr_free(&expr);

  ASSERT_EQ(CDD_C_SUCCESS, arazzo_runtime_expr_parse(
                               "$components.parameters.authHeader", &expr));
  ASSERT_EQ(ARAZZO_EXPR_COMPONENTS, expr.kind);
  ASSERT_STR_EQ("parameters", expr.source_name);
  ASSERT_STR_EQ("authHeader", expr.property_name);
  arazzo_runtime_expr_free(&expr);

  PASS();
}

/**
 * @brief Tests invalid syntax and OOM branches in `arazzo_runtime_expr_parse`.
 */
TEST test_arazzo_runtime_expr_parse_invalid_and_oom(void) {
  struct Arazzo_RuntimeExpr expr;

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_runtime_expr_parse(NULL, &expr));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_runtime_expr_parse("$url", NULL));

  /* Invalid templates & expressions */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_runtime_expr_parse("Bearer {$unclosed", &expr));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_runtime_expr_parse("Bearer {$}", &expr));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_runtime_expr_parse("$unknown", &expr));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_runtime_expr_parse("$request.header.", &expr));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_runtime_expr_parse("$request.query.", &expr));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_runtime_expr_parse("$request.path.", &expr));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_runtime_expr_parse("$request.body#", &expr));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_runtime_expr_parse("$request.body_invalid", &expr));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_runtime_expr_parse("$response.header.", &expr));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_runtime_expr_parse("$response.body#", &expr));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_runtime_expr_parse("$message.header#", &expr));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_runtime_expr_parse("$message.header.", &expr));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_runtime_expr_parse("$message.payload#", &expr));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_runtime_expr_parse("$inputs.", &expr));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_runtime_expr_parse("$inputs.#", &expr));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_runtime_expr_parse("$outputs.", &expr));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_runtime_expr_parse("$outputs.#", &expr));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_runtime_expr_parse("$steps.#", &expr));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_runtime_expr_parse("$steps.nodot", &expr));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_runtime_expr_parse("$steps..bad", &expr));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_runtime_expr_parse("$steps.s1.", &expr));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_runtime_expr_parse("$steps.s1.outputs.", &expr));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_runtime_expr_parse("$workflows.#", &expr));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_runtime_expr_parse("$workflows.w1.", &expr));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_runtime_expr_parse("$sourceDescriptions.#", &expr));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_runtime_expr_parse("$sourceDescriptions.s1.", &expr));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_runtime_expr_parse("$components.#", &expr));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_runtime_expr_parse("$components.c1.", &expr));

  /* OOM branches */
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, arazzo_runtime_expr_parse("$url", &expr));
  g_cdd_alloc_fail = 0;

  g_cdd_alloc_fail = 2;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            arazzo_runtime_expr_parse("$request.header.H", &expr));
  g_cdd_alloc_fail = 2;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            arazzo_runtime_expr_parse("$request.query.Q", &expr));
  g_cdd_alloc_fail = 2;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            arazzo_runtime_expr_parse("$request.path.P", &expr));
  g_cdd_alloc_fail = 2;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            arazzo_runtime_expr_parse("$request.body#/a", &expr));
  g_cdd_alloc_fail = 2;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            arazzo_runtime_expr_parse("$response.header.H", &expr));
  g_cdd_alloc_fail = 2;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            arazzo_runtime_expr_parse("$response.body#/a", &expr));
  g_cdd_alloc_fail = 2;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            arazzo_runtime_expr_parse("$message.header.H", &expr));
  g_cdd_alloc_fail = 2;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            arazzo_runtime_expr_parse("$message.header#/a", &expr));
  g_cdd_alloc_fail = 2;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            arazzo_runtime_expr_parse("$message.payload#/a", &expr));
  g_cdd_alloc_fail = 2;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            arazzo_runtime_expr_parse("$inputs.in1", &expr));
  g_cdd_alloc_fail = 2;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            arazzo_runtime_expr_parse("$outputs.out1", &expr));
  g_cdd_alloc_fail = 2;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            arazzo_runtime_expr_parse("$steps.s1.outputs.o1", &expr));
  g_cdd_alloc_fail = 3;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            arazzo_runtime_expr_parse("$steps.s1.outputs.o1", &expr));
  g_cdd_alloc_fail = 0;

  PASS();
}

/**
 * @brief Tests `arazzo_runtime_expr_format` across all expression kinds,
 * invalid states, and OOM.
 */
TEST test_arazzo_runtime_expr_format_all(void) {
  struct Arazzo_RuntimeExpr expr;
  char *out = NULL;
  int k;

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_runtime_expr_format(NULL, &out));
  memset(&expr, 0, sizeof(expr));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_runtime_expr_format(&expr, NULL));

  /* With raw set */
  expr.raw = C_CDD_STRDUP("$url");
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, arazzo_runtime_expr_format(&expr, &out));
  g_cdd_alloc_fail = 0;
  ASSERT_EQ(CDD_C_SUCCESS, arazzo_runtime_expr_format(&expr, &out));
  ASSERT_STR_EQ("$url", out);
  free(out);
  arazzo_runtime_expr_free(&expr);

  /* Without raw set - test all kinds */
  for (k = (int)ARAZZO_EXPR_URL; k <= (int)ARAZZO_EXPR_SELF; ++k) {
    memset(&expr, 0, sizeof(expr));
    expr.kind = (enum Arazzo_RuntimeExprKind)k;

    /* Test missing required fields first where applicable */
    if (k == ARAZZO_EXPR_REQUEST_HEADER || k == ARAZZO_EXPR_REQUEST_QUERY ||
        k == ARAZZO_EXPR_REQUEST_PATH || k == ARAZZO_EXPR_RESPONSE_HEADER ||
        k == ARAZZO_EXPR_INPUTS || k == ARAZZO_EXPR_OUTPUTS ||
        k == ARAZZO_EXPR_STEPS || k == ARAZZO_EXPR_WORKFLOWS ||
        k == ARAZZO_EXPR_SOURCE_DESCRIPTIONS || k == ARAZZO_EXPR_COMPONENTS) {
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                arazzo_runtime_expr_format(&expr, &out));
    }

    expr.source_name = C_CDD_STRDUP("src");
    expr.step_or_workflow_id = C_CDD_STRDUP("id1");
    if (k == ARAZZO_EXPR_STEPS || k == ARAZZO_EXPR_WORKFLOWS ||
        k == ARAZZO_EXPR_SOURCE_DESCRIPTIONS || k == ARAZZO_EXPR_COMPONENTS) {
      ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
                arazzo_runtime_expr_format(&expr, &out));
    }
    expr.property_name = C_CDD_STRDUP("prop");

    /* Test OOM on strdup */
    g_cdd_alloc_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, arazzo_runtime_expr_format(&expr, &out));
    g_cdd_alloc_fail = 0;

    ASSERT_EQ(CDD_C_SUCCESS, arazzo_runtime_expr_format(&expr, &out));
    free(out);
    out = NULL;

    /* Also test with json_pointer for body/message kinds */
    if (k == ARAZZO_EXPR_REQUEST_BODY || k == ARAZZO_EXPR_RESPONSE_BODY ||
        k == ARAZZO_EXPR_MESSAGE_HEADER || k == ARAZZO_EXPR_MESSAGE_PAYLOAD) {
      free(expr.property_name);
      expr.property_name = NULL;
      ASSERT_EQ(CDD_C_SUCCESS, arazzo_runtime_expr_format(&expr, &out));
      free(out);
      out = NULL;
      expr.json_pointer = C_CDD_STRDUP("/ptr");
      ASSERT_EQ(CDD_C_SUCCESS, arazzo_runtime_expr_format(&expr, &out));
      free(out);
      out = NULL;
    }

    arazzo_runtime_expr_free(&expr);
  }

  /* Default/literal without raw */
  memset(&expr, 0, sizeof(expr));
  expr.kind = ARAZZO_EXPR_LITERAL;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_runtime_expr_format(&expr, &out));

  PASS();
}

SUITE(arazzo_expr_suite) {
  RUN_TEST(test_arazzo_runtime_expr_parse_valid);
  RUN_TEST(test_arazzo_runtime_expr_parse_invalid_and_oom);
  RUN_TEST(test_arazzo_runtime_expr_format_all);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_ARAZZO_EXPR_H */
