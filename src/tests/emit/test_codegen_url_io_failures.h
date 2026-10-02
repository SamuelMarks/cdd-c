/**
 * @file test_codegen_url_io_failures.h
 * @brief Unit tests for routes/emit/url.c internals.
 */

#ifndef TEST_CODEGEN_URL_IO_FAILURES_H
#define TEST_CODEGEN_URL_IO_FAILURES_H

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

TEST test_url_io_failure_branches(void) {
  FILE *fp = cdd_test_tmpfile_global();
  struct OpenAPI_Operation op;
  struct OpenAPI_Parameter p;
  struct OpenAPI_Parameter builder_params[12];
  struct CodegenUrlConfig cfg;
  cdd_c_error_t rc = 0;
  ASSERT(fp != NULL);

  memset(&op, 0, sizeof(op));
  memset(&p, 0, sizeof(p));

  /* 1. write_query_json_param exhaustive IO */
  p.name = (char *)(size_t) "json_arr_str";
  p.in = OA_PARAM_IN_QUERY;
  p.content_type = (char *)(size_t) "application/json";
  p.is_array = 1;
  p.items_type = (char *)(size_t) "string";
  rc = test_run_io_exhaustive_json(fp, &p);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.items_type = (char *)(size_t) "integer";
  rc = test_run_io_exhaustive_json(fp, &p);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.items_type = (char *)(size_t) "number";
  rc = test_run_io_exhaustive_json(fp, &p);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.items_type = (char *)(size_t) "boolean";
  rc = test_run_io_exhaustive_json(fp, &p);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.items_type = (char *)(size_t) "object";
  rc = test_run_io_exhaustive_json(fp, &p);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* Array with custom model Tag */
  p.items_type = (char *)(size_t) "Tag";
  rc = test_run_io_exhaustive_json(fp, &p);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.items_type = NULL;
  p.schema.inline_type = NULL;
  rc = test_run_io_exhaustive_json(fp, &p);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* Scalar JSON primitives */
  p.is_array = 0;
  p.type = (char *)(size_t) "string";
  rc = test_run_io_exhaustive_json(fp, &p);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.type = (char *)(size_t) "integer";
  rc = test_run_io_exhaustive_json(fp, &p);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.type = (char *)(size_t) "number";
  rc = test_run_io_exhaustive_json(fp, &p);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.type = (char *)(size_t) "boolean";
  rc = test_run_io_exhaustive_json(fp, &p);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.type = (char *)(size_t) "object";
  rc = test_run_io_exhaustive_json(fp, &p);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.type = (char *)(size_t) "CustomModel";
  p.schema.ref_name = (char *)(size_t) "CustomModel";
  rc = test_run_io_exhaustive_json(fp, &p);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.schema.ref_name = NULL;
  p.type = (char *)(size_t) "unknown";
  rc = test_run_io_exhaustive_json(fp, &p);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* JSON param with name == NULL */
  p.name = NULL;
  p.type = (char *)(size_t) "string";
  rc = test_run_io_exhaustive_json(fp, &p);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* JSON param with type == NULL */
  p.name = (char *)(size_t) "null_type";
  p.type = NULL;
  rc = test_run_io_exhaustive_json(fp, &p);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* 2. write_query_object_param exhaustive IO */
  memset(&p, 0, sizeof(p));
  p.name = (char *)(size_t) "obj_p";
  p.type = (char *)(size_t) "object";
  p.style = OA_STYLE_DEEP_OBJECT;
  rc = test_run_io_exhaustive_obj(fp, &p);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.style = OA_STYLE_FORM;
  p.explode = 1;
  p.explode_set = 1;
  p.allow_reserved = 0;
  rc = test_run_io_exhaustive_obj(fp, &p);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.allow_reserved = 1;
  p.allow_reserved_set = 1;
  rc = test_run_io_exhaustive_obj(fp, &p);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.style = OA_STYLE_FORM;
  p.explode = 0;
  p.explode_set = 1;
  rc = test_run_io_exhaustive_obj(fp, &p);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.style = OA_STYLE_SPACE_DELIMITED;
  p.allow_reserved = 0;
  p.allow_reserved_set = 0;
  rc = test_run_io_exhaustive_obj(fp, &p);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.allow_reserved = 1;
  p.allow_reserved_set = 1;
  rc = test_run_io_exhaustive_obj(fp, &p);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.style = OA_STYLE_PIPE_DELIMITED;
  p.allow_reserved = 0;
  p.allow_reserved_set = 0;
  rc = test_run_io_exhaustive_obj(fp, &p);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.allow_reserved = 1;
  p.allow_reserved_set = 1;
  rc = test_run_io_exhaustive_obj(fp, &p);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.style = OA_STYLE_LABEL;
  rc = test_run_io_exhaustive_obj(fp, &p);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* Object with name == NULL and style == OA_STYLE_UNKNOWN */
  p.name = NULL;
  p.style = OA_STYLE_UNKNOWN;
  rc = test_run_io_exhaustive_obj(fp, &p);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* 3. write_path_object_serialization exhaustive IO */
  memset(&p, 0, sizeof(p));
  p.name = (char *)(size_t) "path_obj";
  p.type = (char *)(size_t) "object";
  p.style = OA_STYLE_SIMPLE;
  p.explode_set = 1;
  p.explode = 1;
  rc = test_run_io_exhaustive_path_obj(fp, &p);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.explode = 0;
  rc = test_run_io_exhaustive_path_obj(fp, &p);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.style = OA_STYLE_MATRIX;
  p.explode = 1;
  rc = test_run_io_exhaustive_path_obj(fp, &p);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.explode = 0;
  rc = test_run_io_exhaustive_path_obj(fp, &p);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.style = OA_STYLE_LABEL;
  p.explode = 1;
  rc = test_run_io_exhaustive_path_obj(fp, &p);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.explode = 0;
  rc = test_run_io_exhaustive_path_obj(fp, &p);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.style = OA_STYLE_COOKIE;
  rc = test_run_io_exhaustive_path_obj(fp, &p);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.style = OA_STYLE_FORM;
  rc = test_run_io_exhaustive_path_obj(fp, &p);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* Path object with name == NULL and style == OA_STYLE_UNKNOWN */
  p.name = NULL;
  p.style = OA_STYLE_UNKNOWN;
  rc = test_run_io_exhaustive_path_obj(fp, &p);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* 4. write_path_array_serialization exhaustive IO */
  memset(&p, 0, sizeof(p));
  p.name = (char *)(size_t) "path_arr";
  p.is_array = 1;
  p.items_type = (char *)(size_t) "string";
  p.allow_reserved = 0;
  p.allow_reserved_set = 0;
  rc = test_run_io_exhaustive_path_arr(fp, &p, ";arr=", ";arr=");
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.allow_reserved = 1;
  p.allow_reserved_set = 1;
  rc = test_run_io_exhaustive_path_arr(fp, &p, ";arr=", ",");
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.items_type = (char *)(size_t) "integer";
  rc = test_run_io_exhaustive_path_arr(fp, &p, ".", ".");
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.items_type = (char *)(size_t) "number";
  rc = test_run_io_exhaustive_path_arr(fp, &p, ".", ",");
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.items_type = (char *)(size_t) "boolean";
  rc = test_run_io_exhaustive_path_arr(fp, &p, "", ",");
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.items_type = (char *)(size_t) "string";
  rc = test_run_io_exhaustive_path_arr(fp, &p, ".", ".");
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = test_run_io_exhaustive_path_arr(fp, &p, ".", ",");
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = test_run_io_exhaustive_path_arr(fp, &p, "", ",");
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = test_run_io_exhaustive_path_arr(fp, &p, "", "");
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* Path array with name == NULL, items_type == NULL, allow_reserved_set == 1,
   * allow_reserved == 0 */
  p.name = NULL;
  p.items_type = NULL;
  p.allow_reserved_set = 1;
  p.allow_reserved = 0;
  rc = test_run_io_exhaustive_path_arr(fp, &p, "", "");
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* 5. write_joined_query_array & encoded_delim exhaustive IO */
  memset(&p, 0, sizeof(p));
  p.name = (char *)(size_t) "joined_arr";
  p.is_array = 1;
  p.items_type = (char *)(size_t) "string";
  p.allow_reserved = 0;
  p.allow_reserved_set = 0;
  rc = test_run_io_exhaustive_joined(fp, &p, ',', "url_encode", 1);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.allow_reserved = 1;
  p.allow_reserved_set = 1;
  rc = test_run_io_exhaustive_joined(fp, &p, ',', "url_encode_allow_reserved",
                                     1);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.items_type = (char *)(size_t) "integer";
  rc = test_run_io_exhaustive_joined(fp, &p, ',', "url_encode", 1);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.items_type = (char *)(size_t) "number";
  rc = test_run_io_exhaustive_joined(fp, &p, ',', "url_encode", 1);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.items_type = (char *)(size_t) "boolean";
  rc = test_run_io_exhaustive_joined(fp, &p, ',', "url_encode", 1);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.name = NULL;
  p.items_type = NULL;
  rc = test_run_io_exhaustive_joined(fp, &p, ',', "url_encode", 1);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.name = (char *)(size_t) "joined_enc";
  p.items_type = (char *)(size_t) "string";
  rc = test_run_io_exhaustive_joined_enc(fp, &p, "%7C", "url_encode");
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.items_type = (char *)(size_t) "integer";
  rc = test_run_io_exhaustive_joined_enc(fp, &p, "%7C", "url_encode");
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.items_type = (char *)(size_t) "number";
  rc = test_run_io_exhaustive_joined_enc(fp, &p, "%7C", "url_encode");
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.items_type = (char *)(size_t) "boolean";
  rc = test_run_io_exhaustive_joined_enc(fp, &p, "%7C", "url_encode");
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.name = NULL;
  p.items_type = NULL;
  rc = test_run_io_exhaustive_joined_enc(fp, &p, "%7C", "url_encode");
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* 6. codegen_url_write_builder exhaustive IO */
  memset(builder_params, 0, sizeof(builder_params));
  memset(&cfg, 0, sizeof(cfg));
  cfg.out_variable = "custom_url";
  cfg.base_variable = "base_url";

  builder_params[0].name = (char *)(size_t) "p_str";
  builder_params[0].in = OA_PARAM_IN_PATH;
  builder_params[0].type = (char *)(size_t) "string";
  builder_params[0].allow_reserved = 1;
  builder_params[0].allow_reserved_set = 1;

  builder_params[1].name = (char *)(size_t) "p_int";
  builder_params[1].in = OA_PARAM_IN_PATH;
  builder_params[1].type = (char *)(size_t) "integer";

  builder_params[2].name = (char *)(size_t) "p_num";
  builder_params[2].in = OA_PARAM_IN_PATH;
  builder_params[2].type = (char *)(size_t) "number";

  builder_params[3].name = (char *)(size_t) "p_bool";
  builder_params[3].in = OA_PARAM_IN_PATH;
  builder_params[3].type = (char *)(size_t) "boolean";

  builder_params[4].name = (char *)(size_t) "p_mat";
  builder_params[4].in = OA_PARAM_IN_PATH;
  builder_params[4].type = (char *)(size_t) "string";
  builder_params[4].style = OA_STYLE_MATRIX;

  builder_params[5].name = (char *)(size_t) "p_lbl";
  builder_params[5].in = OA_PARAM_IN_PATH;
  builder_params[5].type = (char *)(size_t) "string";
  builder_params[5].style = OA_STYLE_LABEL;

  builder_params[6].name = (char *)(size_t) "p_obj";
  builder_params[6].in = OA_PARAM_IN_PATH;
  builder_params[6].type = (char *)(size_t) "object";
  builder_params[6].is_array = 0;
  builder_params[6].style = OA_STYLE_SIMPLE;

  builder_params[7].name = (char *)(size_t) "p_arr";
  builder_params[7].in = OA_PARAM_IN_PATH;
  builder_params[7].is_array = 1;
  builder_params[7].items_type = (char *)(size_t) "string";
  builder_params[7].style = OA_STYLE_MATRIX;
  builder_params[7].explode = 1;

  builder_params[8].name = (char *)(size_t) "p_arr_lbl";
  builder_params[8].in = OA_PARAM_IN_PATH;
  builder_params[8].is_array = 1;
  builder_params[8].items_type = (char *)(size_t) "string";
  builder_params[8].style = OA_STYLE_LABEL;
  builder_params[8].explode = 0;

  builder_params[9].name = (char *)(size_t) "p_obj_arr";
  builder_params[9].in = OA_PARAM_IN_PATH;
  builder_params[9].type = (char *)(size_t) "object";
  builder_params[9].is_array = 1;

  builder_params[10].name = (char *)(size_t) "p_unknown";
  builder_params[10].in = OA_PARAM_IN_PATH;
  builder_params[10].type = (char *)(size_t) "string";
  builder_params[10].style = OA_STYLE_UNKNOWN;
  builder_params[10].allow_reserved_set = 1;
  builder_params[10].allow_reserved = 0;

  builder_params[11].name = (char *)(size_t) "p_custom";
  builder_params[11].in = OA_PARAM_IN_PATH;
  builder_params[11].type = (char *)(size_t) "custom_type";

  rc = test_run_io_exhaustive_builder(
      fp,
      "/v1/{p_str}/{p_int}/{p_num}/{p_bool}/{p_mat}/{p_lbl}/{p_obj}/{p_arr}/{"
      "p_arr_lbl}/{p_obj_arr}/{p_unknown}/{p_custom}/{missing}",
      builder_params, 12, &cfg);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  cfg.out_variable = NULL;
  rc = test_run_io_exhaustive_builder(fp, "/v1/{p_str}/{p_obj}/{p_arr}",
                                      builder_params, 12, &cfg);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* 7. codegen_url_write_query_params exhaustive IO */
  /* Query operation with raw querystring */
  memset(&op, 0, sizeof(op));
  memset(&p, 0, sizeof(p));
  p.name = (char *)(size_t) "raw_qs";
  p.in = OA_PARAM_IN_QUERYSTRING;
  op.parameters = &p;
  op.n_parameters = 1;
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* Query operation with form-object querystring */
  p.type = (char *)(size_t) "object";
  p.content_type = (char *)(size_t) "application/x-www-form-urlencoded";
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* Query operation with JSON array querystring (all primitive types) */
  p.content_type = (char *)(size_t) "application/json";
  p.is_array = 1;
  p.items_type = (char *)(size_t) "string";
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.items_type = (char *)(size_t) "integer";
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.items_type = (char *)(size_t) "number";
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.items_type = (char *)(size_t) "boolean";
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.items_type = (char *)(size_t) "MyObj";
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* Query operation with scalar JSON querystring (all primitive types) */
  p.is_array = 0;
  p.items_type = NULL;
  p.type = (char *)(size_t) "string";
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.type = (char *)(size_t) "integer";
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.type = (char *)(size_t) "number";
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.type = (char *)(size_t) "boolean";
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.type = (char *)(size_t) "MyModel";
  p.schema.ref_name = (char *)(size_t) "MyModel";
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* Query operation with raw primitive querystring */
  p.schema.ref_name = NULL;
  p.content_type = (char *)(size_t) "text/plain";
  p.type = (char *)(size_t) "string";
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.type = (char *)(size_t) "integer";
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.type = (char *)(size_t) "number";
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.type = (char *)(size_t) "boolean";
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* Query operation with 0 params and qp_tracking = 1 */
  op.n_parameters = 0;
  rc = test_run_io_exhaustive_query(fp, &op, 1);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* Query operation with OA_PARAM_IN_QUERY: non-json content_type */
  memset(&p, 0, sizeof(p));
  p.name = (char *)(size_t) "plain_param";
  p.in = OA_PARAM_IN_QUERY;
  p.content_type = (char *)(size_t) "text/plain";
  p.type = (char *)(size_t) "string";
  op.parameters = &p;
  op.n_parameters = 1;
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* Query operation with OA_PARAM_IN_QUERY: form explode=1 */
  memset(&p, 0, sizeof(p));
  p.name = (char *)(size_t) "q_arr";
  p.in = OA_PARAM_IN_QUERY;
  p.is_array = 1;
  p.items_type = (char *)(size_t) "string";
  p.style = OA_STYLE_FORM;
  p.explode = 1;
  p.explode_set = 1;
  p.allow_reserved = 0;
  p.allow_reserved_set = 0;
  op.parameters = &p;
  op.n_parameters = 1;
  rc = test_run_io_exhaustive_query(fp, &op, 1);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.allow_reserved = 1;
  p.allow_reserved_set = 1;
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.allow_reserved = 0;
  p.allow_reserved_set = 1;
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.allow_reserved_set = 0;
  p.items_type = (char *)(size_t) "integer";
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.items_type = (char *)(size_t) "number";
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.items_type = (char *)(size_t) "boolean";
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.allow_reserved = 0;
  p.allow_reserved_set = 1;
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.allow_reserved_set = 0;
  p.items_type = (char *)(size_t) "custom";
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.items_type = NULL;
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* Form explode=0 with allow_reserved=1 and allow_reserved=0 (set=1) */
  p.style = OA_STYLE_FORM;
  p.explode = 0;
  p.explode_set = 1;
  p.items_type = (char *)(size_t) "string";
  p.allow_reserved = 1;
  p.allow_reserved_set = 1;
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.allow_reserved = 0;
  p.allow_reserved_set = 1;
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* PipeDelimited array with allow_reserved=1, allow_reserved=0 (set=1), and
   * allow_reserved=0 (set=0) */
  p.items_type = (char *)(size_t) "string";
  p.style = OA_STYLE_PIPE_DELIMITED;
  p.allow_reserved = 1;
  p.allow_reserved_set = 1;
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.allow_reserved = 0;
  p.allow_reserved_set = 1;
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.allow_reserved = 0;
  p.allow_reserved_set = 0;
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* SpaceDelimited array with allow_reserved=1, allow_reserved=0 (set=1), and
   * allow_reserved=0 (set=0) */
  p.style = OA_STYLE_SPACE_DELIMITED;
  p.allow_reserved = 1;
  p.allow_reserved_set = 1;
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.allow_reserved = 0;
  p.allow_reserved_set = 1;
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.allow_reserved = 0;
  p.allow_reserved_set = 0;
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* Query operation with OA_PARAM_IN_QUERY: fallback explode=1 */
  p.style = OA_STYLE_DEEP_OBJECT;
  p.explode = 1;
  p.explode_set = 1;
  p.items_type = (char *)(size_t) "string";
  p.allow_reserved = 0;
  p.allow_reserved_set = 0;
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.allow_reserved = 1;
  p.allow_reserved_set = 1;
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.allow_reserved = 0;
  p.allow_reserved_set = 1;
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.items_type = (char *)(size_t) "integer";
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.items_type = (char *)(size_t) "number";
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.items_type = (char *)(size_t) "boolean";
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.items_type = (char *)(size_t) "custom";
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.items_type = NULL;
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* Fallback explode=0 */
  p.explode = 0;
  p.explode_set = 1;
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* Query operation with OA_PARAM_IN_QUERY: scalars */
  p.is_array = 0;
  p.items_type = NULL;
  p.type = (char *)(size_t) "string";
  p.allow_reserved = 0;
  p.allow_reserved_set = 0;
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.allow_reserved = 1;
  p.allow_reserved_set = 1;
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.type = (char *)(size_t) "integer";
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.type = (char *)(size_t) "number";
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.type = (char *)(size_t) "boolean";
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.type = (char *)(size_t) "unknown";
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.style = OA_STYLE_UNKNOWN;
  p.type = (char *)(size_t) "custom";
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  p.name = NULL;
  p.type = (char *)(size_t) "string";
  rc = test_run_io_exhaustive_query(fp, &op, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  g_fail_io_after = -1;
  fclose(fp);
  PASS();
}

SUITE(codegen_url_io_failures_suite) { RUN_TEST(test_url_io_failure_branches); }

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CODEGEN_URL_IO_FAILURES_H */
