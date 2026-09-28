/**
 * @file test_codegen_url_builder.h
 * @brief Unit tests for routes/emit/url.c internals.
 */

#ifndef TEST_CODEGEN_URL_BUILDER_H
#define TEST_CODEGEN_URL_BUILDER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"
#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cdd_c_error.h"
#include "cdd_test_helpers_export.h"
#include "openapi/parse/openapi.h"
#include "routes/emit/url.h"
/* clang-format on */

extern C_CDD_EXPORT int g_cdd_alloc_fail;
extern C_CDD_EXPORT int g_fail_io_after;
extern C_CDD_EXPORT int g_cdd_fail_url_segment_alloc;
extern C_CDD_EXPORT int g_io_calls;
CDD_TEST_HELPERS_EXPORT FILE *cdd_test_tmpfile_global(void);

TEST test_url_builder_and_query_branches(void) {
  FILE *fp = cdd_test_tmpfile_global();
  struct OpenAPI_Operation op;
  struct OpenAPI_Parameter params[5];
  struct CodegenUrlConfig cfg;
  ASSERT(fp != NULL);

  /* codegen_url_write_builder invalid args */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            codegen_url_write_builder(NULL, NULL, NULL, 0, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            codegen_url_write_builder(fp, NULL, NULL, 0, NULL));

  /* Builder error from parse_segments */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            codegen_url_write_builder(fp, "/users/{unclosed", NULL, 0, NULL));

  /* Builder with config out_variable & custom base_variable */
  memset(&cfg, 0, sizeof(cfg));
  cfg.base_variable = "custom_base";
  cfg.out_variable = "custom_url";

  memset(params, 0, sizeof(params));

  /* Path param: boolean */
  params[0].name = (char *)(size_t) "flag";
  params[0].in = OA_PARAM_IN_PATH;
  params[0].type = (char *)(size_t) "boolean";

  /* Path param: number */
  params[1].name = (char *)(size_t) "score";
  params[1].in = OA_PARAM_IN_PATH;
  params[1].type = (char *)(size_t) "number";

  /* Path param: custom fallback */
  params[2].name = (char *)(size_t) "custom";
  params[2].in = OA_PARAM_IN_PATH;
  params[2].type = (char *)(size_t) "custom_type";

  ASSERT_EQ(CDD_C_SUCCESS, codegen_url_write_builder(
                               fp, "/api/{flag}/{score}/{custom}/{unmatched}",
                               params, 3, &cfg));

  /* codegen_url_write_query_params invalid args */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            codegen_url_write_query_params(NULL, NULL, 0));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            codegen_url_write_query_params(fp, NULL, 0));

  /* Query params operation tests */
  memset(&op, 0, sizeof(op));
  memset(params, 0, sizeof(params));

  /* Querystring param with integer json_item */
  params[0].name = (char *)(size_t) "qs";
  params[0].in = OA_PARAM_IN_QUERYSTRING;
  params[0].content_type = (char *)(size_t) "application/json";
  params[0].is_array = 1;
  params[0].items_type = (char *)(size_t) "integer";
  op.parameters = params;
  op.n_parameters = 1;
  ASSERT_EQ(CDD_C_SUCCESS, codegen_url_write_query_params(fp, &op, 0));

  /* Querystring param with number json_item */
  params[0].items_type = (char *)(size_t) "number";
  ASSERT_EQ(CDD_C_SUCCESS, codegen_url_write_query_params(fp, &op, 0));

  /* Querystring param with boolean json_item */
  params[0].items_type = (char *)(size_t) "boolean";
  ASSERT_EQ(CDD_C_SUCCESS, codegen_url_write_query_params(fp, &op, 0));

  /* Querystring param with other json_item */
  params[0].items_type = (char *)(size_t) "other";
  ASSERT_EQ(CDD_C_SUCCESS, codegen_url_write_query_params(fp, &op, 0));

  /* Querystring primitive: integer */
  params[0].is_array = 0;
  params[0].items_type = NULL;
  params[0].type = (char *)(size_t) "integer";
  ASSERT_EQ(CDD_C_SUCCESS, codegen_url_write_query_params(fp, &op, 0));

  /* Querystring primitive: number */
  params[0].type = (char *)(size_t) "number";
  ASSERT_EQ(CDD_C_SUCCESS, codegen_url_write_query_params(fp, &op, 0));

  /* Querystring primitive: boolean */
  params[0].type = (char *)(size_t) "boolean";
  ASSERT_EQ(CDD_C_SUCCESS, codegen_url_write_query_params(fp, &op, 0));

  /* Querystring raw: number */
  params[0].content_type = (char *)(size_t) "text/plain";
  params[0].type = (char *)(size_t) "number";
  ASSERT_EQ(CDD_C_SUCCESS, codegen_url_write_query_params(fp, &op, 0));

  /* Querystring raw: boolean */
  params[0].type = (char *)(size_t) "boolean";
  ASSERT_EQ(CDD_C_SUCCESS, codegen_url_write_query_params(fp, &op, 0));

  /* Querystring raw: unsupported */
  params[0].type = (char *)(size_t) "unsupported";
  ASSERT_EQ(CDD_C_SUCCESS, codegen_url_write_query_params(fp, &op, 0));

  /* Querystring raw with NULL name (fallback to "querystring") */
  params[0].name = NULL;
  params[0].content_type = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, codegen_url_write_query_params(fp, &op, 0));

  /* Query param array with pipeDelimited and allow_reserved=1 */
  memset(params, 0, sizeof(params));
  params[0].name = (char *)(size_t) "pipe_arr";
  params[0].in = OA_PARAM_IN_QUERY;
  params[0].is_array = 1;
  params[0].items_type = (char *)(size_t) "string";
  params[0].style = OA_STYLE_PIPE_DELIMITED;
  params[0].allow_reserved = 1;
  params[0].allow_reserved_set = 1;
  op.parameters = params;
  op.n_parameters = 1;
  ASSERT_EQ(CDD_C_SUCCESS, codegen_url_write_query_params(fp, &op, 0));

  /* Query param array with fallback explode=true: string with allow_reserved=1
   */
  params[0].style = OA_STYLE_DEEP_OBJECT;
  params[0].explode = 1;
  params[0].explode_set = 1;
  ASSERT_EQ(CDD_C_SUCCESS, codegen_url_write_query_params(fp, &op, 0));

  /* Query param array with fallback explode=true: string with allow_reserved=0
   */
  params[0].allow_reserved = 0;
  params[0].allow_reserved_set = 0;
  ASSERT_EQ(CDD_C_SUCCESS, codegen_url_write_query_params(fp, &op, 0));

  /* Query param array with fallback explode=true: integer */
  params[0].items_type = (char *)(size_t) "integer";
  ASSERT_EQ(CDD_C_SUCCESS, codegen_url_write_query_params(fp, &op, 0));

  /* Query param array with fallback explode=true: number */
  params[0].items_type = (char *)(size_t) "number";
  ASSERT_EQ(CDD_C_SUCCESS, codegen_url_write_query_params(fp, &op, 0));

  /* Query param array with fallback explode=true: boolean */
  params[0].items_type = (char *)(size_t) "boolean";
  ASSERT_EQ(CDD_C_SUCCESS, codegen_url_write_query_params(fp, &op, 0));

  /* Query param array with fallback explode=false (unsupported) */
  params[0].explode = 0;
  ASSERT_EQ(CDD_C_SUCCESS, codegen_url_write_query_params(fp, &op, 0));

  /* Query param scalar boolean */
  params[0].is_array = 0;
  params[0].items_type = NULL;
  params[0].type = (char *)(size_t) "boolean";
  params[0].style = OA_STYLE_FORM;
  ASSERT_EQ(CDD_C_SUCCESS, codegen_url_write_query_params(fp, &op, 0));

  /* Query param scalar string with allow_reserved=1 */
  params[0].type = (char *)(size_t) "string";
  params[0].allow_reserved = 1;
  params[0].allow_reserved_set = 1;
  ASSERT_EQ(CDD_C_SUCCESS, codegen_url_write_query_params(fp, &op, 0));

  /* Path param: array with OA_STYLE_MATRIX and explode=1 */
  memset(params, 0, sizeof(params));
  params[0].name = (char *)(size_t) "arr_mat";
  params[0].in = OA_PARAM_IN_PATH;
  params[0].is_array = 1;
  params[0].items_type = (char *)(size_t) "string";
  params[0].style = OA_STYLE_MATRIX;
  params[0].explode = 1;
  params[0].explode_set = 1;

  /* Path param: array with OA_STYLE_MATRIX and explode=0 */
  params[1].name = (char *)(size_t) "arr_mat_noexp";
  params[1].in = OA_PARAM_IN_PATH;
  params[1].is_array = 1;
  params[1].items_type = (char *)(size_t) "string";
  params[1].style = OA_STYLE_MATRIX;
  params[1].explode = 0;
  params[1].explode_set = 1;

  /* Path param: array with OA_STYLE_SIMPLE */
  params[2].name = (char *)(size_t) "arr_simple";
  params[2].in = OA_PARAM_IN_PATH;
  params[2].is_array = 1;
  params[2].items_type = (char *)(size_t) "string";
  params[2].style = OA_STYLE_SIMPLE;

  /* Path param: scalar string with OA_STYLE_LABEL */
  params[3].name = (char *)(size_t) "scalar_lbl";
  params[3].in = OA_PARAM_IN_PATH;
  params[3].type = (char *)(size_t) "string";
  params[3].style = OA_STYLE_LABEL;
  params[3].allow_reserved = 1;
  params[3].allow_reserved_set = 1;

  /* Path param: array of objects */
  params[4].name = (char *)(size_t) "arr_obj";
  params[4].in = OA_PARAM_IN_PATH;
  params[4].type = (char *)(size_t) "object";
  params[4].is_array = 1;
  params[4].items_type = (char *)(size_t) "object";

  ASSERT_EQ(
      CDD_C_SUCCESS,
      codegen_url_write_builder(
          fp,
          "/api/{arr_mat}/{arr_mat_noexp}/{arr_simple}/{scalar_lbl}/{arr_obj}",
          params, 5, NULL));

  /* Path param: array with OA_STYLE_LABEL and explode=1 */
  {
    struct OpenAPI_Parameter p_lbl;
    memset(&p_lbl, 0, sizeof(p_lbl));
    p_lbl.name = (char *)(size_t) "arr_lbl_exp";
    p_lbl.in = OA_PARAM_IN_PATH;
    p_lbl.is_array = 1;
    p_lbl.items_type = (char *)(size_t) "string";
    p_lbl.style = OA_STYLE_LABEL;
    p_lbl.explode = 1;
    p_lbl.explode_set = 1;
    ASSERT_EQ(CDD_C_SUCCESS, codegen_url_write_builder(fp, "/api/{arr_lbl_exp}",
                                                       &p_lbl, 1, NULL));
  }

  /* Querystring primitive: string */
  memset(&op, 0, sizeof(op));
  memset(params, 0, sizeof(params));
  params[0].name = (char *)(size_t) "qstr";
  params[0].in = OA_PARAM_IN_QUERYSTRING;
  params[0].content_type = (char *)(size_t) "application/json";
  params[0].type = (char *)(size_t) "string";
  op.parameters = params;
  op.n_parameters = 1;
  ASSERT_EQ(CDD_C_SUCCESS, codegen_url_write_query_params(fp, &op, 0));

  /* Query param scalar string with allow_reserved=0 */
  memset(params, 0, sizeof(params));
  params[0].name = (char *)(size_t) "plain_str";
  params[0].in = OA_PARAM_IN_QUERY;
  params[0].type = (char *)(size_t) "string";
  params[0].allow_reserved = 0;
  params[0].allow_reserved_set = 1;
  op.parameters = params;
  op.n_parameters = 1;
  ASSERT_EQ(CDD_C_SUCCESS, codegen_url_write_query_params(fp, &op, 0));

  fclose(fp);
  PASS();
}

/**
 * @brief Helper for exhaustive IO failure testing on query parameters.
 */
static cdd_c_error_t test_run_io_exhaustive_query(FILE *fp,
                                                  struct OpenAPI_Operation *op,
                                                  int qp_tracking) {
  int total_calls;
  int i;
  g_fail_io_after = 100000;
  g_io_calls = 0;
  codegen_url_write_query_params(fp, op, qp_tracking);
  total_calls = g_io_calls;
  for (i = 0; i <= total_calls; ++i) {
    g_io_calls = 0;
    g_fail_io_after = i;
    codegen_url_write_query_params(fp, op, qp_tracking);
  }
  g_fail_io_after = -1;
  return CDD_C_SUCCESS;
}

/**
 * @brief Helper for exhaustive IO failure testing on URL builder.
 */
static cdd_c_error_t test_run_io_exhaustive_builder(
    FILE *fp, const char *tmpl, struct OpenAPI_Parameter *params,
    size_t n_params, const struct CodegenUrlConfig *cfg) {
  int total_calls;
  int i;
  g_fail_io_after = 100000;
  g_io_calls = 0;
  codegen_url_write_builder(fp, tmpl, params, n_params, cfg);
  total_calls = g_io_calls;
  for (i = 0; i <= total_calls; ++i) {
    g_io_calls = 0;
    g_fail_io_after = i;
    codegen_url_write_builder(fp, tmpl, params, n_params, cfg);
  }
  g_fail_io_after = -1;
  return CDD_C_SUCCESS;
}

/**
 * @brief Helper for exhaustive IO failure testing on write_query_json_param.
 */
static cdd_c_error_t
test_run_io_exhaustive_json(FILE *fp, const struct OpenAPI_Parameter *p) {
  int total_calls;
  int i;
  g_fail_io_after = 100000;
  g_io_calls = 0;
  write_query_json_param(fp, p);
  total_calls = g_io_calls;
  for (i = 0; i <= total_calls; ++i) {
    g_io_calls = 0;
    g_fail_io_after = i;
    write_query_json_param(fp, p);
  }
  g_fail_io_after = -1;
  return CDD_C_SUCCESS;
}

/**
 * @brief Helper for exhaustive IO failure testing on write_query_object_param.
 */
static cdd_c_error_t
test_run_io_exhaustive_obj(FILE *fp, const struct OpenAPI_Parameter *p) {
  int total_calls;
  int i;
  g_fail_io_after = 100000;
  g_io_calls = 0;
  write_query_object_param(fp, p);
  total_calls = g_io_calls;
  for (i = 0; i <= total_calls; ++i) {
    g_io_calls = 0;
    g_fail_io_after = i;
    write_query_object_param(fp, p);
  }
  g_fail_io_after = -1;
  return CDD_C_SUCCESS;
}

/**
 * @brief Helper for exhaustive IO failure testing on path object serialization.
 */
static cdd_c_error_t
test_run_io_exhaustive_path_obj(FILE *fp, const struct OpenAPI_Parameter *p) {
  int total_calls;
  int i;
  g_fail_io_after = 100000;
  g_io_calls = 0;
  write_path_object_serialization(fp, p);
  total_calls = g_io_calls;
  for (i = 0; i <= total_calls; ++i) {
    g_io_calls = 0;
    g_fail_io_after = i;
    write_path_object_serialization(fp, p);
  }
  g_fail_io_after = -1;
  return CDD_C_SUCCESS;
}

/**
 * @brief Helper for exhaustive IO failure testing on path array serialization.
 */
static cdd_c_error_t
test_run_io_exhaustive_path_arr(FILE *fp, const struct OpenAPI_Parameter *p,
                                const char *prefix, const char *delim) {
  int total_calls;
  int i;
  g_fail_io_after = 100000;
  g_io_calls = 0;
  write_path_array_serialization(fp, p, prefix, delim);
  total_calls = g_io_calls;
  for (i = 0; i <= total_calls; ++i) {
    g_io_calls = 0;
    g_fail_io_after = i;
    write_path_array_serialization(fp, p, prefix, delim);
  }
  g_fail_io_after = -1;
  return CDD_C_SUCCESS;
}

/**
 * @brief Helper for exhaustive IO failure testing on write_joined_query_array.
 */
static cdd_c_error_t
test_run_io_exhaustive_joined(FILE *fp, const struct OpenAPI_Parameter *p,
                              char delim, const char *encode_fn,
                              int encode_delim) {
  int total_calls;
  int i;
  g_fail_io_after = 100000;
  g_io_calls = 0;
  write_joined_query_array(fp, p, delim, encode_fn, encode_delim);
  total_calls = g_io_calls;
  for (i = 0; i <= total_calls; ++i) {
    g_io_calls = 0;
    g_fail_io_after = i;
    write_joined_query_array(fp, p, delim, encode_fn, encode_delim);
  }
  g_fail_io_after = -1;
  return CDD_C_SUCCESS;
}

/**
 * @brief Helper for exhaustive IO failure testing on
 * write_joined_query_array_encoded_delim.
 */
static cdd_c_error_t
test_run_io_exhaustive_joined_enc(FILE *fp, const struct OpenAPI_Parameter *p,
                                  const char *delim_enc,
                                  const char *encode_fn) {
  int total_calls;
  int i;
  g_fail_io_after = 100000;
  g_io_calls = 0;
  write_joined_query_array_encoded_delim(fp, p, delim_enc, encode_fn);
  total_calls = g_io_calls;
  for (i = 0; i <= total_calls; ++i) {
    g_io_calls = 0;
    g_fail_io_after = i;
    write_joined_query_array_encoded_delim(fp, p, delim_enc, encode_fn);
  }
  g_fail_io_after = -1;
  return CDD_C_SUCCESS;
}

SUITE(codegen_url_builder_suite) {
  RUN_TEST(test_url_builder_and_query_branches);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CODEGEN_URL_BUILDER_H */
