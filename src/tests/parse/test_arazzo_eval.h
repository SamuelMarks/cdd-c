/**
 * @file test_arazzo_eval.h
 * @brief Exhaustive unit tests for Arazzo v1.1.0 runtime expression, JSON
 * Pointer, JSONPath, XPath, criterion, selector, and payload replacement
 * evaluation.
 *
 * Exercises 100% of functions, lines, and branches in
 * `src/arazzo/parse/arazzo_eval.c`.
 *
 * @author Samuel Marks
 */

#ifndef TEST_ARAZZO_EVAL_H
#define TEST_ARAZZO_EVAL_H

/* clang-format off */
#include <greatest.h>
#include <stdlib.h>
#include <string.h>
#include "arazzo/parse/arazzo_eval.h"
#include "c_cdd/memory.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief Tests `arazzo_eval_json_pointer` including RFC 6901 unescaping (`~0`,
 * `~1`), arrays, errors, and OOM.
 */
TEST test_arazzo_eval_json_pointer(void) {
  JSON_Value *root_val;
  JSON_Object *root_obj;
  JSON_Value *ab_val;
  JSON_Object *ab_obj;
  JSON_Value *cd_val;
  JSON_Array *cd_arr;
  const JSON_Value *out = NULL;

  root_val = json_value_init_object();
  root_obj = json_value_get_object(root_val);
  ab_val = json_value_init_object();
  ab_obj = json_value_get_object(ab_val);
  cd_val = json_value_init_array();
  cd_arr = json_value_get_array(cd_val);

  json_array_append_string(cd_arr, "first");
  json_array_append_number(cd_arr, 42);
  json_array_append_boolean(cd_arr, 1);
  json_array_append_null(cd_arr);
  json_object_set_value(ab_obj, "c/d", cd_val);
  json_object_set_value(root_obj, "a~b", ab_val);

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_eval_json_pointer(NULL, "/a", &out));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_eval_json_pointer(root_val, NULL, &out));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_eval_json_pointer(root_val, "/a", NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_eval_json_pointer(root_val, "no_slash", &out));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_eval_json_pointer(root_val, "/bad~2escape", &out));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_eval_json_pointer(root_val, "/bad~", &out));

  /* Empty / `#` returns root */
  ASSERT_EQ(CDD_C_SUCCESS, arazzo_eval_json_pointer(root_val, "", &out));
  ASSERT_EQ(root_val, out);
  ASSERT_EQ(CDD_C_SUCCESS, arazzo_eval_json_pointer(root_val, "#", &out));
  ASSERT_EQ(root_val, out);

  /* Unescaped ~0 and ~1 + array indexing */
  ASSERT_EQ(CDD_C_SUCCESS,
            arazzo_eval_json_pointer(root_val, "#/a~0b/c~1d/0", &out));
  ASSERT_STR_EQ("first", json_value_get_string(out));

  /* Non-numeric or empty array index */
  ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
            arazzo_eval_json_pointer(root_val, "/a~0b/c~1d/abc", &out));
  ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
            arazzo_eval_json_pointer(root_val, "/a~0b/c~1d/", &out));
  ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
            arazzo_eval_json_pointer(root_val, "/a~0b/c~1d/99", &out));

  /* Indexing into primitive scalar */
  ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
            arazzo_eval_json_pointer(root_val, "/a~0b/c~1d/0/sub", &out));

  /* OOM in unescape_pointer_token */
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            arazzo_eval_json_pointer(root_val, "/a~0b", &out));
  g_cdd_alloc_fail = 0;

  json_value_free(root_val);
  PASS();
}

/**
 * @brief Tests `arazzo_eval_jsonpath` and `arazzo_eval_xpath` across valid,
 * invalid, not-found, and OOM paths.
 */
TEST test_arazzo_eval_jsonpath_and_xpath(void) {
  JSON_Value *root_val;
  JSON_Object *root_obj;
  JSON_Value *store_val;
  JSON_Object *store_obj;
  JSON_Value *book_arr_val;
  JSON_Array *book_arr;
  JSON_Value *b0_val;
  const JSON_Value *out = NULL;
  char *xml_out = NULL;
  char xml_buf[128];
  char unterm_xml[64];

  CDD_SNPRINTF(
      xml_buf, sizeof(xml_buf),
      "<order id=%cord-99%c><status>shipped</status><unclosed></order>", '"',
      '"');
  CDD_SNPRINTF(unterm_xml, sizeof(unterm_xml), "<order id=%cunterminated>",
               '"');

  root_val = json_value_init_object();
  root_obj = json_value_get_object(root_val);
  store_val = json_value_init_object();
  store_obj = json_value_get_object(store_val);
  book_arr_val = json_value_init_array();
  book_arr = json_value_get_array(book_arr_val);
  b0_val = json_value_init_object();
  json_object_set_string(json_value_get_object(b0_val), "title", "C89");
  json_array_append_value(book_arr, b0_val);
  json_object_set_value(store_obj, "book", book_arr_val);
  json_object_set_value(root_obj, "store", store_val);

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_eval_jsonpath(NULL, "$.store", &out));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_eval_jsonpath(root_val, NULL, &out));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_eval_jsonpath(root_val, "$.store", NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_eval_jsonpath(root_val, "store", &out));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_eval_jsonpath(root_val, "$.", &out));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_eval_jsonpath(root_val, "$[abc]", &out));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_eval_jsonpath(root_val, "$[0", &out));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_eval_jsonpath(root_val, "$?bad", &out));

  ASSERT_EQ(CDD_C_SUCCESS,
            arazzo_eval_jsonpath(root_val, "$.store.book[0].title", &out));
  ASSERT_STR_EQ("C89", json_value_get_string(out));

  ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
            arazzo_eval_jsonpath(root_val, "$.store.missing", &out));
  ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
            arazzo_eval_jsonpath(root_val, "$.store[0]", &out));
  ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
            arazzo_eval_jsonpath(root_val, "$.store.book.not_obj", &out));
  ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
            arazzo_eval_jsonpath(root_val, "$.store.book[5]", &out));

  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            arazzo_eval_jsonpath(root_val, "$.store", &out));
  g_cdd_alloc_fail = 0;

  json_value_free(root_val);

  /* XPath tests */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_eval_xpath(NULL, "/order/status", &xml_out));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_eval_xpath(xml_buf, NULL, &xml_out));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_eval_xpath(xml_buf, "/order/status", NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_eval_xpath(xml_buf, "relative", &xml_out));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_eval_xpath(xml_buf, "/order/", &xml_out));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_eval_xpath(xml_buf, "/order/@", &xml_out));

  ASSERT_EQ(CDD_C_SUCCESS, arazzo_eval_xpath(xml_buf, "/order/@id", &xml_out));
  ASSERT_STR_EQ("ord-99", xml_out);
  free(xml_out);

  ASSERT_EQ(CDD_C_SUCCESS,
            arazzo_eval_xpath(xml_buf, "/order/status", &xml_out));
  ASSERT_STR_EQ("shipped", xml_out);
  free(xml_out);

  ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
            arazzo_eval_xpath(xml_buf, "/order/@missing", &xml_out));
  ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
            arazzo_eval_xpath(unterm_xml, "/order/@id", &xml_out));
  ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
            arazzo_eval_xpath(xml_buf, "/order/missing", &xml_out));
  ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
            arazzo_eval_xpath("<broken", "/broken", &xml_out));
  ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
            arazzo_eval_xpath(xml_buf, "/order/unclosed", &xml_out));

  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            arazzo_eval_xpath(xml_buf, "/order/@id", &xml_out));
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            arazzo_eval_xpath(xml_buf, "/order/status", &xml_out));
  g_cdd_alloc_fail = 0;

  PASS();
}

/**
 * @brief Tests `arazzo_eval_runtime_expr_to_str` across all 17 runtime
 * expression roots and templates.
 */
TEST test_arazzo_eval_runtime_expr_all_roots(void) {
  struct Arazzo_ExecutionContext ctx;
  JSON_Value *root_val;
  JSON_Object *root_obj;
  char *out = NULL;

  root_val = json_value_init_object();
  root_obj = json_value_get_object(root_val);
  json_object_dotset_string(root_obj, "req_h.Authorization", "Bearer abc");
  json_object_dotset_number(root_obj, "req_q.page", 2);
  json_object_dotset_number(root_obj, "req_p.id", 99);
  json_object_dotset_boolean(root_obj, "req_b.flag", 1);
  json_object_dotset_null(root_obj, "req_b.empty");
  json_object_dotset_string(root_obj, "resp_h.X-Rate", "100");
  json_object_dotset_string(root_obj, "resp_b.token", "xyz");
  json_object_dotset_boolean(root_obj, "resp_b.false_flag", 0);
  json_object_dotset_string(root_obj, "msg_h.corr", "c1");
  json_object_dotset_string(root_obj, "msg_p.evt", "created");
  json_object_dotset_string(root_obj, "inputs.env", "prod");
  json_object_dotset_string(root_obj, "inputs.nested.k", "v");
  json_object_dotset_string(root_obj, "outputs.final", "done");
  json_object_dotset_string(root_obj, "steps.s1.tok", "t123");
  json_object_dotset_string(root_obj, "steps.s1.obj.sub", "sv");
  json_object_dotset_string(root_obj, "workflows.w1.res", "wres");
  json_object_dotset_string(root_obj, "sources.petstore.url",
                            "https://api.petstore.io");
  json_object_dotset_string(root_obj, "components.parameters.p1", "pval");

  memset(&ctx, 0, sizeof(ctx));
  ctx.url = "https://api.petstore.io/v1/pets";
  ctx.method = "POST";
  ctx.status_code = 201;
  ctx.self_uri = "https://api.petstore.io/arazzo.json";
  ctx.req_headers = json_object_get_object(root_obj, "req_h");
  ctx.req_query = json_object_get_object(root_obj, "req_q");
  ctx.req_path = json_object_get_object(root_obj, "req_p");
  ctx.req_body = json_object_get_value(root_obj, "req_b");
  ctx.resp_headers = json_object_get_object(root_obj, "resp_h");
  ctx.resp_body = json_object_get_value(root_obj, "resp_b");
  ctx.msg_header = json_object_get_value(root_obj, "msg_h");
  ctx.msg_payload = json_object_get_value(root_obj, "msg_p");
  ctx.inputs = json_object_get_object(root_obj, "inputs");
  ctx.outputs = json_object_get_object(root_obj, "outputs");
  ctx.steps = json_object_get_object(root_obj, "steps");
  ctx.workflows = json_object_get_object(root_obj, "workflows");
  ctx.sources = json_object_get_object(root_obj, "sources");
  ctx.components = json_object_get_object(root_obj, "components");

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_eval_runtime_expr_to_str(NULL, &ctx, &out));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_eval_runtime_expr_to_str("$url", NULL, &out));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_eval_runtime_expr_to_str("$url", &ctx, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_eval_runtime_expr_to_str("$invalid", &ctx, &out));

  ASSERT_EQ(CDD_C_SUCCESS,
            arazzo_eval_runtime_expr_to_str("literal_str", &ctx, &out));
  ASSERT_STR_EQ("literal_str", out);
  free(out);

  ASSERT_EQ(CDD_C_SUCCESS, arazzo_eval_runtime_expr_to_str("$url", &ctx, &out));
  ASSERT_STR_EQ("https://api.petstore.io/v1/pets", out);
  free(out);

  ASSERT_EQ(CDD_C_SUCCESS,
            arazzo_eval_runtime_expr_to_str("$method", &ctx, &out));
  ASSERT_STR_EQ("POST", out);
  free(out);

  ASSERT_EQ(CDD_C_SUCCESS,
            arazzo_eval_runtime_expr_to_str("$statusCode", &ctx, &out));
  ASSERT_STR_EQ("201", out);
  free(out);

  ASSERT_EQ(CDD_C_SUCCESS,
            arazzo_eval_runtime_expr_to_str("$self", &ctx, &out));
  ASSERT_STR_EQ("https://api.petstore.io/arazzo.json", out);
  free(out);

  ASSERT_EQ(CDD_C_SUCCESS, arazzo_eval_runtime_expr_to_str(
                               "$request.header.Authorization", &ctx, &out));
  ASSERT_STR_EQ("Bearer abc", out);
  free(out);

  ASSERT_EQ(CDD_C_SUCCESS,
            arazzo_eval_runtime_expr_to_str("$request.query.page", &ctx, &out));
  ASSERT_STR_EQ("2", out);
  free(out);

  ASSERT_EQ(CDD_C_SUCCESS,
            arazzo_eval_runtime_expr_to_str("$request.path.id", &ctx, &out));
  ASSERT_STR_EQ("99", out);
  free(out);

  ASSERT_EQ(CDD_C_SUCCESS,
            arazzo_eval_runtime_expr_to_str("$request.body", &ctx, &out));
  free(out);

  ASSERT_EQ(CDD_C_SUCCESS,
            arazzo_eval_runtime_expr_to_str("$request.body#/flag", &ctx, &out));
  ASSERT_STR_EQ("true", out);
  free(out);

  ASSERT_EQ(CDD_C_SUCCESS, arazzo_eval_runtime_expr_to_str(
                               "$request.body#/empty", &ctx, &out));
  ASSERT_STR_EQ("null", out);
  free(out);

  ASSERT_EQ(CDD_C_ERROR_NOT_FOUND, arazzo_eval_runtime_expr_to_str(
                                       "$request.body#/missing", &ctx, &out));

  ASSERT_EQ(CDD_C_SUCCESS, arazzo_eval_runtime_expr_to_str(
                               "$response.header.X-Rate", &ctx, &out));
  ASSERT_STR_EQ("100", out);
  free(out);

  ASSERT_EQ(CDD_C_SUCCESS,
            arazzo_eval_runtime_expr_to_str("$response.body", &ctx, &out));
  free(out);

  ASSERT_EQ(CDD_C_SUCCESS, arazzo_eval_runtime_expr_to_str(
                               "$response.body#/false_flag", &ctx, &out));
  ASSERT_STR_EQ("false", out);
  free(out);

  ASSERT_EQ(CDD_C_ERROR_NOT_FOUND, arazzo_eval_runtime_expr_to_str(
                                       "$response.body#/missing", &ctx, &out));

  ASSERT_EQ(CDD_C_SUCCESS,
            arazzo_eval_runtime_expr_to_str("$message.header", &ctx, &out));
  free(out);

  ASSERT_EQ(CDD_C_SUCCESS, arazzo_eval_runtime_expr_to_str(
                               "$message.header.corr", &ctx, &out));
  ASSERT_STR_EQ("c1", out);
  free(out);

  ASSERT_EQ(CDD_C_SUCCESS, arazzo_eval_runtime_expr_to_str(
                               "$message.header#/corr", &ctx, &out));
  ASSERT_STR_EQ("c1", out);
  free(out);

  ASSERT_EQ(CDD_C_ERROR_NOT_FOUND, arazzo_eval_runtime_expr_to_str(
                                       "$message.header#/missing", &ctx, &out));

  ASSERT_EQ(CDD_C_SUCCESS,
            arazzo_eval_runtime_expr_to_str("$message.payload", &ctx, &out));
  free(out);

  ASSERT_EQ(CDD_C_SUCCESS, arazzo_eval_runtime_expr_to_str(
                               "$message.payload#/evt", &ctx, &out));
  ASSERT_STR_EQ("created", out);
  free(out);

  ASSERT_EQ(
      CDD_C_ERROR_NOT_FOUND,
      arazzo_eval_runtime_expr_to_str("$message.payload#/missing", &ctx, &out));

  ASSERT_EQ(CDD_C_SUCCESS,
            arazzo_eval_runtime_expr_to_str("$inputs.nested#/k", &ctx, &out));
  ASSERT_STR_EQ("v", out);
  free(out);

  ASSERT_EQ(CDD_C_ERROR_NOT_FOUND, arazzo_eval_runtime_expr_to_str(
                                       "$inputs.nested#/missing", &ctx, &out));

  ASSERT_EQ(CDD_C_SUCCESS,
            arazzo_eval_runtime_expr_to_str("$outputs.final", &ctx, &out));
  ASSERT_STR_EQ("done", out);
  free(out);

  ASSERT_EQ(CDD_C_SUCCESS, arazzo_eval_runtime_expr_to_str(
                               "$steps.s1.outputs.obj#/sub", &ctx, &out));
  ASSERT_STR_EQ("sv", out);
  free(out);

  ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
            arazzo_eval_runtime_expr_to_str("$steps.s1.outputs.obj#/missing",
                                            &ctx, &out));

  ASSERT_EQ(CDD_C_SUCCESS, arazzo_eval_runtime_expr_to_str(
                               "$workflows.w1.outputs.res", &ctx, &out));
  ASSERT_STR_EQ("wres", out);
  free(out);

  ASSERT_EQ(CDD_C_SUCCESS, arazzo_eval_runtime_expr_to_str(
                               "$sourceDescriptions.petstore.url", &ctx, &out));
  ASSERT_STR_EQ("https://api.petstore.io", out);
  free(out);

  ASSERT_EQ(CDD_C_SUCCESS, arazzo_eval_runtime_expr_to_str(
                               "$components.parameters.p1", &ctx, &out));
  ASSERT_STR_EQ("pval", out);
  free(out);

  /* Embedded template */
  ASSERT_EQ(CDD_C_SUCCESS,
            arazzo_eval_runtime_expr_to_str(
                "Bearer {$steps.s1.outputs.tok} ({$inputs.env})!", &ctx, &out));
  ASSERT_STR_EQ("Bearer t123 (prod)!", out);
  free(out);

  ASSERT_EQ(CDD_C_SUCCESS, arazzo_eval_runtime_expr_to_str(
                               "Bearer {$inputs.env}", &ctx, &out));
  ASSERT_STR_EQ("Bearer prod", out);
  free(out);

  /* OOM in embedded template evaluation */
  {
    int fail_step;
    for (fail_step = 1; fail_step <= 8; ++fail_step) {
      g_cdd_alloc_fail = fail_step;
      if (arazzo_eval_runtime_expr_to_str("Bearer {$inputs.env}!", &ctx,
                                          &out) == CDD_C_SUCCESS) {
        free(out);
      }
      g_cdd_alloc_fail = 0;
    }
  }

  /* OOM in eval_single_expr_to_str and json_val_to_str */
  g_cdd_alloc_fail = 2;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            arazzo_eval_runtime_expr_to_str("literal", &ctx, &out));
  g_cdd_alloc_fail = 2;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            arazzo_eval_runtime_expr_to_str("$url", &ctx, &out));
  g_cdd_alloc_fail = 2;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            arazzo_eval_runtime_expr_to_str("$statusCode", &ctx, &out));
  g_cdd_alloc_fail = 2;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            arazzo_eval_runtime_expr_to_str("$request.header.Authorization",
                                            &ctx, &out));
  g_cdd_alloc_fail = 2;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            arazzo_eval_runtime_expr_to_str("$request.query.page", &ctx, &out));
  g_cdd_alloc_fail = 3;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            arazzo_eval_runtime_expr_to_str("$request.body#/flag", &ctx, &out));
  g_cdd_alloc_fail = 3;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, arazzo_eval_runtime_expr_to_str(
                                    "$request.body#/empty", &ctx, &out));
  g_cdd_alloc_fail = 2;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            arazzo_eval_runtime_expr_to_str("$request.body", &ctx, &out));
  g_cdd_alloc_fail = 0;

  ASSERT_EQ(CDD_C_ERROR_NOT_FOUND, arazzo_eval_runtime_expr_to_str(
                                       "Bearer {$inputs.missing}", &ctx, &out));

  /* Empty context NOT_FOUND paths */
  memset(&ctx, 0, sizeof(ctx));
  ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
            arazzo_eval_runtime_expr_to_str("$url", &ctx, &out));
  ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
            arazzo_eval_runtime_expr_to_str("$method", &ctx, &out));
  ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
            arazzo_eval_runtime_expr_to_str("$self", &ctx, &out));
  ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
            arazzo_eval_runtime_expr_to_str("$request.body", &ctx, &out));
  ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
            arazzo_eval_runtime_expr_to_str("$response.body", &ctx, &out));
  ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
            arazzo_eval_runtime_expr_to_str("$message.header", &ctx, &out));
  ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
            arazzo_eval_runtime_expr_to_str("$message.payload", &ctx, &out));
  ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
            arazzo_eval_runtime_expr_to_str("$request.header.H", &ctx, &out));
  ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
            arazzo_eval_runtime_expr_to_str("$request.query.Q", &ctx, &out));
  ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
            arazzo_eval_runtime_expr_to_str("$request.path.P", &ctx, &out));
  ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
            arazzo_eval_runtime_expr_to_str("$response.header.H", &ctx, &out));
  ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
            arazzo_eval_runtime_expr_to_str("$inputs.in", &ctx, &out));
  ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
            arazzo_eval_runtime_expr_to_str("$outputs.out", &ctx, &out));
  ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
            arazzo_eval_runtime_expr_to_str("$steps.s.outputs.o", &ctx, &out));
  ASSERT_EQ(CDD_C_ERROR_NOT_FOUND, arazzo_eval_runtime_expr_to_str(
                                       "$workflows.w.outputs.o", &ctx, &out));
  ASSERT_EQ(CDD_C_ERROR_NOT_FOUND, arazzo_eval_runtime_expr_to_str(
                                       "$sourceDescriptions.s.u", &ctx, &out));
  ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
            arazzo_eval_runtime_expr_to_str("$components.c.p", &ctx, &out));

  json_value_free(root_val);
  PASS();
}

/**
 * @brief Tests `arazzo_eval_criterion`, `arazzo_eval_selector`, and
 * `arazzo_apply_payload_replacements`.
 */
TEST test_arazzo_eval_criteria_selectors_and_replacements(void) {
  struct Arazzo_ExecutionContext ctx;
  struct Arazzo_Criterion crit;
  struct Arazzo_Selector sel;
  struct Arazzo_PayloadReplacement reps[4];
  JSON_Value *resp_val;
  JSON_Value *payload_val;
  JSON_Value *inputs_val;
  char xml_doc[64];
  char cond_buf[128];
  char ctx_buf[128];
  char sel_buf[128];
  char t0[32];
  char t1[32];
  char t2[32];
  char t3[32];
  char v0[32];
  int passed = 0;
  char *out_str = NULL;

  CDD_SNPRINTF(xml_doc, sizeof(xml_doc), "<order id=%c7%c><ok>1</ok></order>",
               '"', '"');

  resp_val = json_value_init_object();
  json_object_set_string(json_value_get_object(resp_val), "status", "active");
  json_object_set_string(json_value_get_object(resp_val), "xml", xml_doc);

  inputs_val = json_value_init_object();
  json_object_set_string(json_value_get_object(inputs_val), "newName", "Rex");

  memset(&ctx, 0, sizeof(ctx));
  ctx.status_code = 200;
  ctx.url = "https://api.example.com/v1/pets";
  ctx.resp_body = resp_val;
  ctx.inputs = json_value_get_object(inputs_val);

  /* Criterion validation */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_eval_criterion(NULL, &ctx, &passed));
  memset(&crit, 0, sizeof(crit));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_eval_criterion(&crit, &ctx, &passed));
  crit.condition = cond_buf;
  CDD_SNPRINTF(cond_buf, sizeof(cond_buf), "%s", "$statusCode == 200");
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_eval_criterion(&crit, NULL, &passed));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_eval_criterion(&crit, &ctx, NULL));

  /* Simple criteria: ==, !=, >=, <=, >, <, &&, boolean truthiness */
  crit.type = ARAZZO_CRITERION_SIMPLE;
  CDD_SNPRINTF(cond_buf, sizeof(cond_buf), "%s",
               "$statusCode == 200 && $response.body#/status == 'active'");
  ASSERT_EQ(CDD_C_SUCCESS, arazzo_eval_criterion(&crit, &ctx, &passed));
  ASSERT_EQ(1, passed);

  /* OOM in simple criteria */
  {
    int f;
    for (f = 1; f <= 6; ++f) {
      g_cdd_alloc_fail = f;
      arazzo_eval_criterion(&crit, &ctx, &passed);
      g_cdd_alloc_fail = 0;
    }
    CDD_SNPRINTF(cond_buf, sizeof(cond_buf), "%s", "100 == 100");
    for (f = 1; f <= 3; ++f) {
      g_cdd_alloc_fail = f;
      arazzo_eval_criterion(&crit, &ctx, &passed);
      g_cdd_alloc_fail = 0;
    }
  }

  CDD_SNPRINTF(cond_buf, sizeof(cond_buf), "%s", "\"quoted\" == \"other\"");
  ASSERT_EQ(CDD_C_SUCCESS, arazzo_eval_criterion(&crit, &ctx, &passed));
  ASSERT_EQ(0, passed);

  CDD_SNPRINTF(cond_buf, sizeof(cond_buf), "%s", "200 == 200");
  ASSERT_EQ(CDD_C_SUCCESS, arazzo_eval_criterion(&crit, &ctx, &passed));
  ASSERT_EQ(1, passed);

  CDD_SNPRINTF(cond_buf, sizeof(cond_buf), "%s", "300 < 200");
  ASSERT_EQ(CDD_C_SUCCESS, arazzo_eval_criterion(&crit, &ctx, &passed));
  ASSERT_EQ(0, passed);

  CDD_SNPRINTF(cond_buf, sizeof(cond_buf), "%s",
               "$statusCode == 404 && $response.body#/status == 'active'");
  ASSERT_EQ(CDD_C_SUCCESS, arazzo_eval_criterion(&crit, &ctx, &passed));
  ASSERT_EQ(0, passed);

  CDD_SNPRINTF(cond_buf, sizeof(cond_buf), "%s", "$statusCode != 500");
  ASSERT_EQ(CDD_C_SUCCESS, arazzo_eval_criterion(&crit, &ctx, &passed));
  ASSERT_EQ(1, passed);

  CDD_SNPRINTF(cond_buf, sizeof(cond_buf), "%s", "$statusCode >= 200");
  ASSERT_EQ(CDD_C_SUCCESS, arazzo_eval_criterion(&crit, &ctx, &passed));
  ASSERT_EQ(1, passed);

  CDD_SNPRINTF(cond_buf, sizeof(cond_buf), "%s", "$statusCode <= 299");
  ASSERT_EQ(CDD_C_SUCCESS, arazzo_eval_criterion(&crit, &ctx, &passed));
  ASSERT_EQ(1, passed);

  CDD_SNPRINTF(cond_buf, sizeof(cond_buf), "%s", "$statusCode > 100");
  ASSERT_EQ(CDD_C_SUCCESS, arazzo_eval_criterion(&crit, &ctx, &passed));
  ASSERT_EQ(1, passed);

  CDD_SNPRINTF(cond_buf, sizeof(cond_buf), "%s", "$statusCode < 300");
  ASSERT_EQ(CDD_C_SUCCESS, arazzo_eval_criterion(&crit, &ctx, &passed));
  ASSERT_EQ(1, passed);

  CDD_SNPRINTF(cond_buf, sizeof(cond_buf), "%s", "100 < $statusCode");
  ASSERT_EQ(CDD_C_SUCCESS, arazzo_eval_criterion(&crit, &ctx, &passed));
  ASSERT_EQ(1, passed);

  CDD_SNPRINTF(cond_buf, sizeof(cond_buf), "%s", "$response.body#/status");
  ASSERT_EQ(CDD_C_SUCCESS, arazzo_eval_criterion(&crit, &ctx, &passed));
  ASSERT_EQ(1, passed);

  CDD_SNPRINTF(cond_buf, sizeof(cond_buf), "%s", "$response.body#/missing");
  ASSERT_EQ(CDD_C_SUCCESS, arazzo_eval_criterion(&crit, &ctx, &passed));
  ASSERT_EQ(0, passed);

  CDD_SNPRINTF(cond_buf, sizeof(cond_buf), "%s",
               "$response.body#/missing == 1");
  ASSERT_EQ(CDD_C_SUCCESS, arazzo_eval_criterion(&crit, &ctx, &passed));
  ASSERT_EQ(0, passed);

  CDD_SNPRINTF(cond_buf, sizeof(cond_buf), "%s",
               "1 == $response.body#/missing");
  ASSERT_EQ(CDD_C_SUCCESS, arazzo_eval_criterion(&crit, &ctx, &passed));
  ASSERT_EQ(0, passed);

  CDD_SNPRINTF(cond_buf, sizeof(cond_buf), "%s", "   ");
  ASSERT_EQ(CDD_C_SUCCESS, arazzo_eval_criterion(&crit, &ctx, &passed));
  ASSERT_EQ(0, passed);

  CDD_SNPRINTF(cond_buf, sizeof(cond_buf), "%s", "  'true'  ");
  ASSERT_EQ(CDD_C_SUCCESS, arazzo_eval_criterion(&crit, &ctx, &passed));
  ASSERT_EQ(1, passed);

  CDD_SNPRINTF(cond_buf, sizeof(cond_buf), "%s", "  'mismatched\"  ");
  ASSERT_EQ(CDD_C_SUCCESS, arazzo_eval_criterion(&crit, &ctx, &passed));
  ASSERT_EQ(0, passed);

  CDD_SNPRINTF(cond_buf, sizeof(cond_buf), "%s", "'' == 'abc'");
  ASSERT_EQ(CDD_C_SUCCESS, arazzo_eval_criterion(&crit, &ctx, &passed));
  ASSERT_EQ(0, passed);

  CDD_SNPRINTF(cond_buf, sizeof(cond_buf), "%s", "100 == $inputs.env");
  ASSERT_EQ(CDD_C_SUCCESS, arazzo_eval_criterion(&crit, &ctx, &passed));
  ASSERT_EQ(0, passed);

  CDD_SNPRINTF(cond_buf, sizeof(cond_buf), "%s", "100 == 'abc'");
  ASSERT_EQ(CDD_C_SUCCESS, arazzo_eval_criterion(&crit, &ctx, &passed));
  ASSERT_EQ(0, passed);

  json_object_set_boolean(json_value_get_object(resp_val), "f_bool", 0);
  json_object_set_null(json_value_get_object(resp_val), "n_val");
  CDD_SNPRINTF(cond_buf, sizeof(cond_buf), "%s", "$response.body#/f_bool");
  ASSERT_EQ(CDD_C_SUCCESS, arazzo_eval_criterion(&crit, &ctx, &passed));
  ASSERT_EQ(0, passed);
  CDD_SNPRINTF(cond_buf, sizeof(cond_buf), "%s", "$response.body#/n_val");
  ASSERT_EQ(CDD_C_SUCCESS, arazzo_eval_criterion(&crit, &ctx, &passed));
  ASSERT_EQ(0, passed);

  json_object_set_string(json_value_get_object(resp_val), "empty_str", "");
  CDD_SNPRINTF(cond_buf, sizeof(cond_buf), "%s", "$response.body#/empty_str");
  ASSERT_EQ(CDD_C_SUCCESS, arazzo_eval_criterion(&crit, &ctx, &passed));
  ASSERT_EQ(0, passed);

  /* Regex criteria */
  crit.type = ARAZZO_CRITERION_REGEX;
  crit.context = NULL;
  CDD_SNPRINTF(cond_buf, sizeof(cond_buf), "%s", "^active$");
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_eval_criterion(&crit, &ctx, &passed));

  crit.context = ctx_buf;
  CDD_SNPRINTF(ctx_buf, sizeof(ctx_buf), "%s", "$response.body#/status");
  CDD_SNPRINTF(cond_buf, sizeof(cond_buf), "%s", "^active$");
  ASSERT_EQ(CDD_C_SUCCESS, arazzo_eval_criterion(&crit, &ctx, &passed));
  ASSERT_EQ(1, passed);

  CDD_SNPRINTF(cond_buf, sizeof(cond_buf), "%s", "^other$");
  ASSERT_EQ(CDD_C_SUCCESS, arazzo_eval_criterion(&crit, &ctx, &passed));
  ASSERT_EQ(0, passed);

  CDD_SNPRINTF(cond_buf, sizeof(cond_buf), "%s", "^act");
  ASSERT_EQ(CDD_C_SUCCESS, arazzo_eval_criterion(&crit, &ctx, &passed));
  ASSERT_EQ(1, passed);

  CDD_SNPRINTF(cond_buf, sizeof(cond_buf), "%s", "^verylongprefix");
  ASSERT_EQ(CDD_C_SUCCESS, arazzo_eval_criterion(&crit, &ctx, &passed));
  ASSERT_EQ(0, passed);

  CDD_SNPRINTF(cond_buf, sizeof(cond_buf), "%s", "tive$");
  ASSERT_EQ(CDD_C_SUCCESS, arazzo_eval_criterion(&crit, &ctx, &passed));
  ASSERT_EQ(1, passed);

  CDD_SNPRINTF(cond_buf, sizeof(cond_buf), "%s", "verylongsuffix$");
  ASSERT_EQ(CDD_C_SUCCESS, arazzo_eval_criterion(&crit, &ctx, &passed));
  ASSERT_EQ(0, passed);

  CDD_SNPRINTF(cond_buf, sizeof(cond_buf), "%s", "cti");
  ASSERT_EQ(CDD_C_SUCCESS, arazzo_eval_criterion(&crit, &ctx, &passed));
  ASSERT_EQ(1, passed);

  CDD_SNPRINTF(cond_buf, sizeof(cond_buf), "%s", "");
  ASSERT_EQ(CDD_C_SUCCESS, arazzo_eval_criterion(&crit, &ctx, &passed));
  ASSERT_EQ(1, passed);

  CDD_SNPRINTF(ctx_buf, sizeof(ctx_buf), "%s", "$response.body#/missing");
  ASSERT_EQ(CDD_C_SUCCESS, arazzo_eval_criterion(&crit, &ctx, &passed));
  ASSERT_EQ(0, passed);

  /* JSONPath & ExpressionType criteria */
  crit.type = ARAZZO_CRITERION_EXPRESSION_TYPE;
  crit.expression_type.type = ARAZZO_DIALECT_JSONPATH;
  CDD_SNPRINTF(cond_buf, sizeof(cond_buf), "%s", "$.status");
  ASSERT_EQ(CDD_C_SUCCESS, arazzo_eval_criterion(&crit, &ctx, &passed));
  ASSERT_EQ(1, passed);

  CDD_SNPRINTF(cond_buf, sizeof(cond_buf), "%s", "$.missing");
  ASSERT_EQ(CDD_C_SUCCESS, arazzo_eval_criterion(&crit, &ctx, &passed));
  ASSERT_EQ(0, passed);

  ctx.resp_body = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, arazzo_eval_criterion(&crit, &ctx, &passed));
  ASSERT_EQ(0, passed);
  ctx.resp_body = resp_val;

  crit.type = (enum Arazzo_CriterionType)99;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_eval_criterion(&crit, &ctx, &passed));
  crit.type = ARAZZO_CRITERION_EXPRESSION_TYPE;

  crit.expression_type.type = ARAZZO_DIALECT_XPATH;
  crit.context = ctx_buf;
  CDD_SNPRINTF(ctx_buf, sizeof(ctx_buf), "%s", "$response.body#/xml");
  CDD_SNPRINTF(cond_buf, sizeof(cond_buf), "%s", "/order/ok");
  ASSERT_EQ(CDD_C_SUCCESS, arazzo_eval_criterion(&crit, &ctx, &passed));
  ASSERT_EQ(1, passed);

  CDD_SNPRINTF(cond_buf, sizeof(cond_buf), "%s", "/order/missing");
  ASSERT_EQ(CDD_C_SUCCESS, arazzo_eval_criterion(&crit, &ctx, &passed));
  ASSERT_EQ(0, passed);

  crit.context = NULL;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_eval_criterion(&crit, &ctx, &passed));

  crit.context = ctx_buf;
  CDD_SNPRINTF(ctx_buf, sizeof(ctx_buf), "%s", "$response.body#/missing");
  ASSERT_EQ(CDD_C_SUCCESS, arazzo_eval_criterion(&crit, &ctx, &passed));
  ASSERT_EQ(0, passed);

  crit.expression_type.type = ARAZZO_DIALECT_UNKNOWN;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_eval_criterion(&crit, &ctx, &passed));

  /* Selector evaluation */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_eval_selector(NULL, &ctx, &out_str));
  memset(&sel, 0, sizeof(sel));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_eval_selector(&sel, &ctx, &out_str));
  sel.context = ctx_buf;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_eval_selector(&sel, &ctx, &out_str));
  sel.selector = sel_buf;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_eval_selector(&sel, NULL, &out_str));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_eval_selector(&sel, &ctx, NULL));

  CDD_SNPRINTF(ctx_buf, sizeof(ctx_buf), "%s", "$response.body");
  CDD_SNPRINTF(sel_buf, sizeof(sel_buf), "%s", "$.status");
  sel.type = ARAZZO_CRITERION_EXPRESSION_TYPE;
  sel.expression_type.type = ARAZZO_DIALECT_JSONPATH;
  ASSERT_EQ(CDD_C_SUCCESS, arazzo_eval_selector(&sel, &ctx, &out_str));
  ASSERT_STR_EQ("active", out_str);
  free(out_str);

  CDD_SNPRINTF(sel_buf, sizeof(sel_buf), "%s", "$.missing");
  ASSERT_EQ(CDD_C_ERROR_NOT_FOUND, arazzo_eval_selector(&sel, &ctx, &out_str));

  CDD_SNPRINTF(ctx_buf, sizeof(ctx_buf), "%s", "$request.body");
  ASSERT_EQ(CDD_C_ERROR_NOT_FOUND, arazzo_eval_selector(&sel, &ctx, &out_str));

  CDD_SNPRINTF(ctx_buf, sizeof(ctx_buf), "%s", "$response.body#/xml");
  CDD_SNPRINTF(sel_buf, sizeof(sel_buf), "%s", "/order/@id");
  sel.expression_type.type = ARAZZO_DIALECT_XPATH;
  ASSERT_EQ(CDD_C_SUCCESS, arazzo_eval_selector(&sel, &ctx, &out_str));
  ASSERT_STR_EQ("7", out_str);
  free(out_str);

  CDD_SNPRINTF(ctx_buf, sizeof(ctx_buf), "%s", "$response.body#/missing");
  ASSERT_EQ(CDD_C_ERROR_NOT_FOUND, arazzo_eval_selector(&sel, &ctx, &out_str));

  sel.expression_type.type = ARAZZO_DIALECT_UNKNOWN;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_eval_selector(&sel, &ctx, &out_str));

  sel.type = ARAZZO_CRITERION_SIMPLE;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_eval_selector(&sel, &ctx, &out_str));

  /* Payload replacements */
  payload_val = json_value_init_object();
  json_object_dotset_string(json_value_get_object(payload_val), "pet.name",
                            "old");
  json_object_set_number(json_value_get_object(payload_val), "count", 1);
  json_object_set_boolean(json_value_get_object(payload_val), "active", 0);
  json_object_set_string(json_value_get_object(payload_val), "extra", "x");

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_apply_payload_replacements(NULL, reps, 1, &ctx));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_apply_payload_replacements(payload_val, NULL, 1, &ctx));

  memset(reps, 0, sizeof(reps));
  CDD_SNPRINTF(t0, sizeof(t0), "%s", "/pet/name");
  CDD_SNPRINTF(v0, sizeof(v0), "%s", "$inputs.newName");
  reps[0].target = t0;
  reps[0].value.type = OA_ANY_STRING;
  reps[0].value.string = v0;

  CDD_SNPRINTF(t1, sizeof(t1), "%s", "/count");
  reps[1].target = t1;
  reps[1].value.type = OA_ANY_NUMBER;
  reps[1].value.number = 5;

  CDD_SNPRINTF(t2, sizeof(t2), "%s", "/active");
  reps[2].target = t2;
  reps[2].value.type = OA_ANY_BOOL;
  reps[2].value.boolean = 1;

  CDD_SNPRINTF(t3, sizeof(t3), "%s", "/extra");
  reps[3].target = t3;
  reps[3].value.type = OA_ANY_NULL;

  ASSERT_EQ(CDD_C_SUCCESS,
            arazzo_apply_payload_replacements(payload_val, reps, 4, &ctx));
  ASSERT_STR_EQ("Rex", json_object_dotget_string(
                           json_value_get_object(payload_val), "pet.name"));
  ASSERT_EQ(5, (int)json_object_get_number(json_value_get_object(payload_val),
                                           "count"));
  ASSERT_EQ(
      1, json_object_get_boolean(json_value_get_object(payload_val), "active"));

  /* Literal string replacement */
  CDD_SNPRINTF(v0, sizeof(v0), "%s", "StaticName");
  ASSERT_EQ(CDD_C_SUCCESS,
            arazzo_apply_payload_replacements(payload_val, reps, 1, &ctx));
  ASSERT_STR_EQ("StaticName",
                json_object_dotget_string(json_value_get_object(payload_val),
                                          "pet.name"));

  reps[0].value.string = NULL;
  ASSERT_EQ(CDD_C_SUCCESS,
            arazzo_apply_payload_replacements(payload_val, reps, 1, &ctx));
  reps[0].value.string = v0;

  /* Invalid target replacement */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_apply_payload_replacements(payload_val, reps, 1, NULL));
  ASSERT_EQ(CDD_C_SUCCESS,
            arazzo_apply_payload_replacements(payload_val, reps, 0, &ctx));
  reps[0].target = NULL;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_apply_payload_replacements(payload_val, reps, 1, &ctx));
  reps[0].target = t0;
  CDD_SNPRINTF(t0, sizeof(t0), "%s", "bad_target");
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_apply_payload_replacements(payload_val, reps, 1, &ctx));
  CDD_SNPRINTF(t0, sizeof(t0), "%s", "/missing/child");
  ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
            arazzo_apply_payload_replacements(payload_val, reps, 1, &ctx));
  CDD_SNPRINTF(t0, sizeof(t0), "%s", "/count/not_obj");
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            arazzo_apply_payload_replacements(payload_val, reps, 1, &ctx));
  CDD_SNPRINTF(t0, sizeof(t0), "%s", "/pet/name");
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            arazzo_apply_payload_replacements(payload_val, reps, 1, &ctx));
  g_cdd_alloc_fail = 0;
  CDD_SNPRINTF(v0, sizeof(v0), "%s", "$inputs.missing");
  ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
            arazzo_apply_payload_replacements(payload_val, reps, 1, &ctx));

  json_value_free(payload_val);
  json_value_free(resp_val);
  json_value_free(inputs_val);
  PASS();
}

SUITE(arazzo_eval_suite) {
  RUN_TEST(test_arazzo_eval_json_pointer);
  RUN_TEST(test_arazzo_eval_jsonpath_and_xpath);
  RUN_TEST(test_arazzo_eval_runtime_expr_all_roots);
  RUN_TEST(test_arazzo_eval_criteria_selectors_and_replacements);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_ARAZZO_EVAL_H */
