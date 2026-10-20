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
/** @brief Test definition g_fail_io_after. */
extern C_CDD_EXPORT int g_fail_io_after;
extern C_CDD_EXPORT int g_cdd_fail_url_segment_alloc;
/** @brief Test definition g_io_calls. */
extern C_CDD_EXPORT int g_io_calls;
CDD_TEST_HELPERS_EXPORT FILE *cdd_test_tmpfile_global(void);

/**
 * @brief Unit test test_url_builder_and_query_branches.
 * @return Greatest test result.
 */
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
  params[0].name = cdd_test_mut_str("flag");
  params[0].in = OA_PARAM_IN_PATH;
  params[0].type = cdd_test_mut_str("boolean");

  /* Path param: number */
  params[1].name = cdd_test_mut_str("score");
  params[1].in = OA_PARAM_IN_PATH;
  params[1].type = cdd_test_mut_str("number");

  /* Path param: custom fallback */
  params[2].name = cdd_test_mut_str("custom");
  params[2].in = OA_PARAM_IN_PATH;
  params[2].type = cdd_test_mut_str("custom_type");

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
  params[0].name = cdd_test_mut_str("qs");
  params[0].in = OA_PARAM_IN_QUERYSTRING;
  params[0].content_type = cdd_test_mut_str("application/json");
  params[0].is_array = 1;
  params[0].items_type = cdd_test_mut_str("integer");
  op.parameters = params;
  op.n_parameters = 1;
  ASSERT_EQ(CDD_C_SUCCESS, codegen_url_write_query_params(fp, &op, 0));

  /* Querystring param with number json_item */
  params[0].items_type = cdd_test_mut_str("number");
  ASSERT_EQ(CDD_C_SUCCESS, codegen_url_write_query_params(fp, &op, 0));

  /* Querystring param with boolean json_item */
  params[0].items_type = cdd_test_mut_str("boolean");
  ASSERT_EQ(CDD_C_SUCCESS, codegen_url_write_query_params(fp, &op, 0));

  /* Querystring param with other json_item */
  params[0].items_type = cdd_test_mut_str("other");
  ASSERT_EQ(CDD_C_SUCCESS, codegen_url_write_query_params(fp, &op, 0));

  /* Querystring primitive: integer */
  params[0].is_array = 0;
  params[0].items_type = NULL;
  params[0].type = cdd_test_mut_str("integer");
  ASSERT_EQ(CDD_C_SUCCESS, codegen_url_write_query_params(fp, &op, 0));

  /* Querystring primitive: number */
  params[0].type = cdd_test_mut_str("number");
  ASSERT_EQ(CDD_C_SUCCESS, codegen_url_write_query_params(fp, &op, 0));

  /* Querystring primitive: boolean */
  params[0].type = cdd_test_mut_str("boolean");
  ASSERT_EQ(CDD_C_SUCCESS, codegen_url_write_query_params(fp, &op, 0));

  /* Querystring raw: number */
  params[0].content_type = cdd_test_mut_str("text/plain");
  params[0].type = cdd_test_mut_str("number");
  ASSERT_EQ(CDD_C_SUCCESS, codegen_url_write_query_params(fp, &op, 0));

  /* Querystring raw: boolean */
  params[0].type = cdd_test_mut_str("boolean");
  ASSERT_EQ(CDD_C_SUCCESS, codegen_url_write_query_params(fp, &op, 0));

  /* Querystring raw: unsupported */
  params[0].type = cdd_test_mut_str("unsupported");
  ASSERT_EQ(CDD_C_SUCCESS, codegen_url_write_query_params(fp, &op, 0));

  /* Querystring raw with NULL name (fallback to "querystring") */
  params[0].name = NULL;
  params[0].content_type = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, codegen_url_write_query_params(fp, &op, 0));

  /* Query param array with pipeDelimited and allow_reserved=1 */
  memset(params, 0, sizeof(params));
  params[0].name = cdd_test_mut_str("pipe_arr");
  params[0].in = OA_PARAM_IN_QUERY;
  params[0].is_array = 1;
  params[0].items_type = cdd_test_mut_str("string");
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
  params[0].items_type = cdd_test_mut_str("integer");
  ASSERT_EQ(CDD_C_SUCCESS, codegen_url_write_query_params(fp, &op, 0));

  /* Query param array with fallback explode=true: number */
  params[0].items_type = cdd_test_mut_str("number");
  ASSERT_EQ(CDD_C_SUCCESS, codegen_url_write_query_params(fp, &op, 0));

  /* Query param array with fallback explode=true: boolean */
  params[0].items_type = cdd_test_mut_str("boolean");
  ASSERT_EQ(CDD_C_SUCCESS, codegen_url_write_query_params(fp, &op, 0));

  /* Query param array with fallback explode=false (unsupported) */
  params[0].explode = 0;
  ASSERT_EQ(CDD_C_SUCCESS, codegen_url_write_query_params(fp, &op, 0));

  /* Query param scalar boolean */
  params[0].is_array = 0;
  params[0].items_type = NULL;
  params[0].type = cdd_test_mut_str("boolean");
  params[0].style = OA_STYLE_FORM;
  ASSERT_EQ(CDD_C_SUCCESS, codegen_url_write_query_params(fp, &op, 0));

  /* Query param scalar string with allow_reserved=1 */
  params[0].type = cdd_test_mut_str("string");
  params[0].allow_reserved = 1;
  params[0].allow_reserved_set = 1;
  ASSERT_EQ(CDD_C_SUCCESS, codegen_url_write_query_params(fp, &op, 0));

  /* Path param: array with OA_STYLE_MATRIX and explode=1 */
  memset(params, 0, sizeof(params));
  params[0].name = cdd_test_mut_str("arr_mat");
  params[0].in = OA_PARAM_IN_PATH;
  params[0].is_array = 1;
  params[0].items_type = cdd_test_mut_str("string");
  params[0].style = OA_STYLE_MATRIX;
  params[0].explode = 1;
  params[0].explode_set = 1;

  /* Path param: array with OA_STYLE_MATRIX and explode=0 */
  params[1].name = cdd_test_mut_str("arr_mat_noexp");
  params[1].in = OA_PARAM_IN_PATH;
  params[1].is_array = 1;
  params[1].items_type = cdd_test_mut_str("string");
  params[1].style = OA_STYLE_MATRIX;
  params[1].explode = 0;
  params[1].explode_set = 1;

  /* Path param: array with OA_STYLE_SIMPLE */
  params[2].name = cdd_test_mut_str("arr_simple");
  params[2].in = OA_PARAM_IN_PATH;
  params[2].is_array = 1;
  params[2].items_type = cdd_test_mut_str("string");
  params[2].style = OA_STYLE_SIMPLE;

  /* Path param: scalar string with OA_STYLE_LABEL */
  params[3].name = cdd_test_mut_str("scalar_lbl");
  params[3].in = OA_PARAM_IN_PATH;
  params[3].type = cdd_test_mut_str("string");
  params[3].style = OA_STYLE_LABEL;
  params[3].allow_reserved = 1;
  params[3].allow_reserved_set = 1;

  /* Path param: array of objects */
  params[4].name = cdd_test_mut_str("arr_obj");
  params[4].in = OA_PARAM_IN_PATH;
  params[4].type = cdd_test_mut_str("object");
  params[4].is_array = 1;
  params[4].items_type = cdd_test_mut_str("object");

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
    p_lbl.name = cdd_test_mut_str("arr_lbl_exp");
    p_lbl.in = OA_PARAM_IN_PATH;
    p_lbl.is_array = 1;
    p_lbl.items_type = cdd_test_mut_str("string");
    p_lbl.style = OA_STYLE_LABEL;
    p_lbl.explode = 1;
    p_lbl.explode_set = 1;
    ASSERT_EQ(CDD_C_SUCCESS, codegen_url_write_builder(fp, "/api/{arr_lbl_exp}",
                                                       &p_lbl, 1, NULL));
  }

  /* Querystring primitive: string */
  memset(&op, 0, sizeof(op));
  memset(params, 0, sizeof(params));
  params[0].name = cdd_test_mut_str("qstr");
  params[0].in = OA_PARAM_IN_QUERYSTRING;
  params[0].content_type = cdd_test_mut_str("application/json");
  params[0].type = cdd_test_mut_str("string");
  op.parameters = params;
  op.n_parameters = 1;
  ASSERT_EQ(CDD_C_SUCCESS, codegen_url_write_query_params(fp, &op, 0));

  /* Query param scalar string with allow_reserved=0 */
  memset(params, 0, sizeof(params));
  params[0].name = cdd_test_mut_str("plain_str");
  params[0].in = OA_PARAM_IN_QUERY;
  params[0].type = cdd_test_mut_str("string");
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
 * @param[in,out] fp Parameter fp.
 * @param[in,out] op Parameter op.
 * @param[in] qp_tracking Parameter qp_tracking.
 * @return Test or helper result.
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
 * @param[in,out] fp Parameter fp.
 * @param[in] tmpl Parameter tmpl.
 * @param[in,out] params Parameter params.
 * @param[in] n_params Parameter n_params.
 * @param[in] cfg Parameter cfg.
 * @return Test or helper result.
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
 * @param[in,out] fp Parameter fp.
 * @param[in] p Parameter p.
 * @return Test or helper result.
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
 * @param[in,out] fp Parameter fp.
 * @param[in] p Parameter p.
 * @return Test or helper result.
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
 * @param[in,out] fp Parameter fp.
 * @param[in] p Parameter p.
 * @return Test or helper result.
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
 * @param[in,out] fp Parameter fp.
 * @param[in] p Parameter p.
 * @param[in] prefix Parameter prefix.
 * @param[in] delim Parameter delim.
 * @return Test or helper result.
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
 * @param[in,out] fp Parameter fp.
 * @param[in] p Parameter p.
 * @param[in] delim Parameter delim.
 * @param[in] encode_fn Parameter encode_fn.
 * @param[in] encode_delim Parameter encode_delim.
 * @return Test or helper result.
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
 * @param[in,out] fp Parameter fp.
 * @param[in] p Parameter p.
 * @param[in] delim_enc Parameter delim_enc.
 * @param[in] encode_fn Parameter encode_fn.
 * @return Test or helper result.
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

/**
 * @brief Executes the codegen_url_builder_suite test suite.
 */
SUITE(codegen_url_builder_suite) {
  RUN_TEST(test_url_builder_and_query_branches);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CODEGEN_URL_BUILDER_H */
