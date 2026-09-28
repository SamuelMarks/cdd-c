/**
 * @file test_codegen_client_body_params.h
 * @brief Unit tests for client body parameters and query strings.
 */

#ifndef TEST_CODEGEN_CLIENT_BODY_PARAMS_H
#define TEST_CODEGEN_CLIENT_BODY_PARAMS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "c_cdd_stdbool.h"
#include "cdd_test_helpers/cdd_helpers.h"
#include "classes/emit/struct.h"
#include "functions/emit/client_body.h"
#include "functions/parse/str.h"
#include "greatest.h"
#include "openapi/parse/openapi.h"
/* clang-format on */

#ifndef C_CDD_STR_LIT
#define C_CDD_STR_LIT(s) ((char *)(size_t)(size_t)(s))
#endif

extern C_CDD_EXPORT int g_io_calls;
extern C_CDD_EXPORT int g_fail_io_after;
extern C_CDD_EXPORT int g_cdd_fail_is_primitive_type;

/**
 * @brief Test direct calls for part headers and form edge cases.
 *
 * @return GREATEST_TEST_RES.
 */
TEST test_client_body_direct_part_headers_and_form_edge_cases(void) {
  FILE *fp;
  struct OpenAPI_Encoding enc;
  struct OpenAPI_Header hdrs[12];
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  struct OpenAPI_Parameter param;
  cdd_c_error_t rc;

  memset(&enc, 0, sizeof(enc));
  memset(hdrs, 0, sizeof(hdrs));
  enc.name = C_CDD_STR_LIT("part");
  enc.headers = hdrs;
  enc.n_headers = 11;

  /* 0: null name */
  hdrs[0].name = NULL;

  /* 1: array integer */
  hdrs[1].name = C_CDD_STR_LIT("h_arr_int");
  hdrs[1].type = C_CDD_STR_LIT("array");
  hdrs[1].is_array = 1;
  hdrs[1].items_type = C_CDD_STR_LIT("integer");

  /* 2: array number */
  hdrs[2].name = C_CDD_STR_LIT("h_arr_num");
  hdrs[2].type = C_CDD_STR_LIT("array");
  hdrs[2].is_array = 1;
  hdrs[2].items_type = C_CDD_STR_LIT("number");

  /* 3: array boolean */
  hdrs[3].name = C_CDD_STR_LIT("h_arr_bool");
  hdrs[3].type = C_CDD_STR_LIT("array");
  hdrs[3].is_array = 1;
  hdrs[3].items_type = C_CDD_STR_LIT("boolean");

  /* 4: array string */
  hdrs[4].name = C_CDD_STR_LIT("h_arr_str");
  hdrs[4].type = C_CDD_STR_LIT("array");
  hdrs[4].is_array = 1;
  hdrs[4].items_type = C_CDD_STR_LIT("string");

  /* 5: object unexploded */
  hdrs[5].name = C_CDD_STR_LIT("h_obj_unexp");
  hdrs[5].type = C_CDD_STR_LIT("object");
  hdrs[5].is_array = 0;
  hdrs[5].explode_set = 1;
  hdrs[5].explode = 0;

  /* 6: scalar integer */
  hdrs[6].name = C_CDD_STR_LIT("h_sc_int");
  hdrs[6].type = C_CDD_STR_LIT("integer");
  hdrs[6].is_array = 0;

  /* 7: scalar number */
  hdrs[7].name = C_CDD_STR_LIT("h_sc_num");
  hdrs[7].type = C_CDD_STR_LIT("number");
  hdrs[7].is_array = 0;

  /* 8: scalar boolean */
  hdrs[8].name = C_CDD_STR_LIT("h_sc_bool");
  hdrs[8].type = C_CDD_STR_LIT("boolean");
  hdrs[8].is_array = 0;

  /* 9: object exploded */
  hdrs[9].name = C_CDD_STR_LIT("h_obj_exp");
  hdrs[9].type = C_CDD_STR_LIT("object");
  hdrs[9].is_array = 0;
  hdrs[9].explode_set = 1;
  hdrs[9].explode = 1;

  /* 10: scalar string */
  hdrs[10].name = C_CDD_STR_LIT("h_sc_str");
  hdrs[10].type = C_CDD_STR_LIT("string");
  hdrs[10].is_array = 0;

  fp = cdd_test_tmpfile_global();
  ASSERT(fp != NULL);
  rc = client_body_write_multipart_part_headers(fp, &enc);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  fclose(fp);

  /* Loop I/O failures across write_multipart_part_headers */
  {
    int io_fail;
    for (io_fail = 0; io_fail < 50; ++io_fail) {
      fp = cdd_test_tmpfile_global();
      g_io_calls = 0;
      g_fail_io_after = io_fail;
      client_body_write_multipart_part_headers(fp, &enc);
      g_fail_io_after = -1;
      fclose(fp);
    }
  }

  /* Test cookie array unencoded string */
  memset(&op, 0, sizeof(op));
  memset(&param, 0, sizeof(param));
  param.name = C_CDD_STR_LIT("cookie_str_arr");
  param.in = OA_PARAM_IN_COOKIE;
  param.style = OA_STYLE_COOKIE;
  param.is_array = 1;
  param.items_type = C_CDD_STR_LIT("string");
  param.explode_set = 1;
  param.explode = 0;
  op.parameters = &param;
  op.n_parameters = 1;

  fp = cdd_test_tmpfile_global();
  ASSERT(fp != NULL);
  rc = client_body_write_cookie_param_logic(fp, &op);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  fclose(fp);

  /* Test form urlencoded object field with empty ref, missing schema, and
   * allow_reserved_set only */
  ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
  spec.defined_schemas =
      (struct StructFields *)calloc(1, sizeof(struct StructFields));
  spec.defined_schema_names = (char **)calloc(1, sizeof(char *));
  ASSERT(spec.defined_schemas);
  ASSERT(spec.defined_schema_names);
  struct_fields_init(&spec.defined_schemas[0]);
  struct_fields_add(&spec.defined_schemas[0], "empty_obj", "object", "", NULL,
                    NULL);
  struct_fields_add(&spec.defined_schemas[0], "unknown_obj", "object",
                    "UnknownSchemaRef", NULL, NULL);
  struct_fields_add(&spec.defined_schemas[0], "res_only_obj", "object",
                    "EmptyObjForm", NULL, NULL);
  c_cdd_strdup("EmptyObjForm", &spec.defined_schema_names[0]);
  spec.n_defined_schemas = 1;

  memset(&op, 0, sizeof(op));
  op.req_body.ref_name = C_CDD_STR_LIT("EmptyObjForm");
  op.req_body.content_type = C_CDD_STR_LIT("application/x-www-form-urlencoded");

  {
    struct OpenAPI_Encoding res_enc;
    struct OpenAPI_MediaType res_mt;
    memset(&res_enc, 0, sizeof(res_enc));
    memset(&res_mt, 0, sizeof(res_mt));
    res_enc.name = C_CDD_STR_LIT("res_only_obj");
    res_enc.allow_reserved_set = 1;
    res_enc.allow_reserved = 1;
    res_mt.name = C_CDD_STR_LIT("application/x-www-form-urlencoded");
    res_mt.encoding = &res_enc;
    res_mt.n_encoding = 1;
    op.req_body_media_types = &res_mt;
    op.n_req_body_media_types = 1;

    fp = cdd_test_tmpfile_global();
    ASSERT(fp != NULL);
    rc = client_body_write_form_urlencoded_body(fp, &op, &spec);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    fclose(fp);
  }

  openapi_spec_free(&spec);
  PASS();
}

/**
 * @brief Test all inline JSON request body types (scalars and arrays).
 *
 * @return GREATEST_TEST_RES.
 */
TEST test_client_body_inline_req_body_json_types(void) {
  FILE *fp;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  struct OpenAPI_Response resp;
  cdd_c_error_t rc;
  const char *types[5];
  size_t t_idx;

  types[0] = "string";
  types[1] = "integer";
  types[2] = "number";
  types[3] = "boolean";
  types[4] = "custom_unknown";

  ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));

  op.operation_id = C_CDD_STR_LIT("testInlineJson");
  op.verb = OA_VERB_POST;
  op.method = C_CDD_STR_LIT("post");
  op.req_body.content_type = C_CDD_STR_LIT("application/json");

  resp.code = C_CDD_STR_LIT("200");
  op.responses = &resp;
  op.n_responses = 1;

  for (t_idx = 0; t_idx < 5; ++t_idx) {
    /* scalar */
    op.req_body.is_array = 0;
    op.req_body.inline_type = C_CDD_STR_LIT(types[t_idx]);
    fp = cdd_test_tmpfile_global();
    ASSERT(fp != NULL);
    rc = codegen_client_write_body(fp, &op, &spec, "/items", NULL);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    fclose(fp);

    /* array */
    op.req_body.is_array = 1;
    fp = cdd_test_tmpfile_global();
    ASSERT(fp != NULL);
    rc = codegen_client_write_body(fp, &op, &spec, "/items", NULL);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    fclose(fp);
  }

  /* Response with NULL code branch */
  resp.code = NULL;
  fp = cdd_test_tmpfile_global();
  ASSERT(fp != NULL);
  rc = codegen_client_write_body(fp, &op, &spec, "/items", NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  fclose(fp);

  openapi_spec_free(&spec);
  PASS();
}

/**
 * @brief Test querystring combined with security query.
 *
 * @return GREATEST_TEST_RES.
 */
TEST test_client_body_querystring_and_security_query(void) {
  FILE *fp;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  struct OpenAPI_Parameter param;
  struct OpenAPI_Response resp;
  struct OpenAPI_SecurityScheme sch;
  cdd_c_error_t rc;

  ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
  memset(&op, 0, sizeof(op));
  memset(&param, 0, sizeof(param));
  memset(&resp, 0, sizeof(resp));
  memset(&sch, 0, sizeof(sch));

  sch.type = OA_SEC_APIKEY;
  sch.in = OA_SEC_IN_QUERY;
  c_cdd_strdup("query_key", &sch.name);
  spec.security_schemes =
      (struct OpenAPI_SecurityScheme *)calloc(1, sizeof(sch));
  ASSERT(spec.security_schemes);
  spec.security_schemes[0] = sch;
  spec.n_security_schemes = 1;
  op.security_set = 0;
  spec.security_set = 0;

  op.operation_id = C_CDD_STR_LIT("testSecQuery");
  op.verb = OA_VERB_GET;
  op.method = C_CDD_STR_LIT("get");

  param.name = C_CDD_STR_LIT("qs");
  param.in = OA_PARAM_IN_QUERYSTRING;
  param.type = C_CDD_STR_LIT("string");
  op.parameters = &param;
  op.n_parameters = 1;

  resp.code = C_CDD_STR_LIT("200");
  op.responses = &resp;
  op.n_responses = 1;

  fp = cdd_test_tmpfile_global();
  ASSERT(fp != NULL);
  rc = codegen_client_write_body(fp, &op, &spec, "/items", NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  fclose(fp);

  openapi_spec_free(&spec);
  PASS();
}

/**
 * @brief Systematic I/O failure loops to exercise all CHECK_IO branches.
 *
 * @return GREATEST_TEST_RES.
 */
TEST test_client_body_systematic_io_failures(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  struct OpenAPI_Parameter params[4];
  struct OpenAPI_Response responses[4];
  int io_fail;
  FILE *fp;

  ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
  spec.defined_schemas =
      (struct StructFields *)calloc(1, sizeof(struct StructFields));
  spec.defined_schema_names = (char **)calloc(1, sizeof(char *));
  ASSERT(spec.defined_schemas);
  ASSERT(spec.defined_schema_names);

  struct_fields_init(&spec.defined_schemas[0]);
  struct_fields_add(&spec.defined_schemas[0], "title", "string", NULL, NULL,
                    NULL);
  struct_fields_add(&spec.defined_schemas[0], "count", "integer", NULL, NULL,
                    NULL);
  struct_fields_add(&spec.defined_schemas[0], "flag", "boolean", NULL, NULL,
                    NULL);
  c_cdd_strdup("Item", &spec.defined_schema_names[0]);
  spec.n_defined_schemas = 1;

  memset(&op, 0, sizeof(op));
  memset(params, 0, sizeof(params));
  memset(responses, 0, sizeof(responses));

  op.operation_id = C_CDD_STR_LIT("testIoSim");
  op.verb = OA_VERB_POST;
  op.method = C_CDD_STR_LIT("post");

  params[0].name = C_CDD_STR_LIT("id");
  params[0].in = OA_PARAM_IN_PATH;
  params[0].type = C_CDD_STR_LIT("string");

  params[1].name = C_CDD_STR_LIT("q");
  params[1].in = OA_PARAM_IN_QUERY;
  params[1].type = C_CDD_STR_LIT("string");

  params[2].name = C_CDD_STR_LIT("h");
  params[2].in = OA_PARAM_IN_HEADER;
  params[2].type = C_CDD_STR_LIT("string");

  params[3].name = C_CDD_STR_LIT("c");
  params[3].in = OA_PARAM_IN_COOKIE;
  params[3].type = C_CDD_STR_LIT("string");

  op.parameters = params;
  op.n_parameters = 4;

  op.req_body.ref_name = C_CDD_STR_LIT("Item");
  op.req_body.content_type = C_CDD_STR_LIT("application/json");

  responses[0].code = C_CDD_STR_LIT("200");
  responses[0].content_type = C_CDD_STR_LIT("application/json");
  responses[0].schema.ref_name = C_CDD_STR_LIT("Item");

  responses[1].code = C_CDD_STR_LIT("2XX");
  responses[1].content_type = C_CDD_STR_LIT("text/plain");
  responses[1].schema.inline_type = C_CDD_STR_LIT("string");

  responses[2].code = C_CDD_STR_LIT("400");
  responses[3].code = C_CDD_STR_LIT("default");
  responses[3].content_type = C_CDD_STR_LIT("application/octet-stream");

  op.responses = responses;
  op.n_responses = 4;

  /* Run I/O failure loop until success */
  for (io_fail = 0; io_fail < 300; ++io_fail) {
    cdd_c_error_t rc;
    fp = cdd_test_tmpfile_global();
    g_io_calls = 0;
    g_fail_io_after = io_fail;
    rc = codegen_client_write_body(fp, &op, &spec, "/items/{id}", NULL);
    g_fail_io_after = -1;
    fclose(fp);
    if (rc == CDD_C_SUCCESS)
      break;
  }

  /* Now repeat with form urlencoded body to trip form CHECK_IO branches */
  op.req_body.content_type = C_CDD_STR_LIT("application/x-www-form-urlencoded");
  for (io_fail = 0; io_fail < 300; ++io_fail) {
    cdd_c_error_t rc;
    fp = cdd_test_tmpfile_global();
    g_io_calls = 0;
    g_fail_io_after = io_fail;
    rc = codegen_client_write_body(fp, &op, &spec, "/items/{id}", NULL);
    g_fail_io_after = -1;
    fclose(fp);
    if (rc == CDD_C_SUCCESS)
      break;
  }

  /* Now repeat with multipart body to trip multipart CHECK_IO branches */
  op.req_body.content_type = C_CDD_STR_LIT("multipart/form-data");
  for (io_fail = 0; io_fail < 300; ++io_fail) {
    cdd_c_error_t rc;
    fp = cdd_test_tmpfile_global();
    g_io_calls = 0;
    g_fail_io_after = io_fail;
    rc = codegen_client_write_body(fp, &op, &spec, "/items/{id}", NULL);
    g_fail_io_after = -1;
    fclose(fp);
    if (rc == CDD_C_SUCCESS)
      break;
  }

  /* Loop with 200 binary response */
  op.req_body.content_type = C_CDD_STR_LIT("application/json");
  responses[0].code = C_CDD_STR_LIT("200");
  responses[0].content_type = C_CDD_STR_LIT("application/octet-stream");
  responses[0].schema.ref_name = NULL;
  responses[0].schema.inline_type = NULL;
  for (io_fail = 0; io_fail < 300; ++io_fail) {
    cdd_c_error_t rc;
    fp = cdd_test_tmpfile_global();
    g_io_calls = 0;
    g_fail_io_after = io_fail;
    rc = codegen_client_write_body(fp, &op, &spec, "/items/{id}", NULL);
    g_fail_io_after = -1;
    fclose(fp);
    if (rc == CDD_C_SUCCESS)
      break;
  }

  /* Loop with 200 text/plain response */
  responses[0].content_type = C_CDD_STR_LIT("text/plain");
  responses[0].schema.inline_type = C_CDD_STR_LIT("string");
  for (io_fail = 0; io_fail < 300; ++io_fail) {
    cdd_c_error_t rc;
    fp = cdd_test_tmpfile_global();
    g_io_calls = 0;
    g_fail_io_after = io_fail;
    rc = codegen_client_write_body(fp, &op, &spec, "/items/{id}", NULL);
    g_fail_io_after = -1;
    fclose(fp);
    if (rc == CDD_C_SUCCESS)
      break;
  }

  /* Loop with 2XX binary and default text/plain */
  responses[0].code = C_CDD_STR_LIT("404");
  responses[1].code = C_CDD_STR_LIT("2XX");
  responses[1].content_type = C_CDD_STR_LIT("application/octet-stream");
  responses[1].schema.inline_type = NULL;
  responses[3].code = C_CDD_STR_LIT("default");
  responses[3].content_type = C_CDD_STR_LIT("text/plain");
  responses[3].schema.inline_type = C_CDD_STR_LIT("string");
  for (io_fail = 0; io_fail < 300; ++io_fail) {
    cdd_c_error_t rc;
    fp = cdd_test_tmpfile_global();
    g_io_calls = 0;
    g_fail_io_after = io_fail;
    rc = codegen_client_write_body(fp, &op, &spec, "/items/{id}", NULL);
    g_fail_io_after = -1;
    fclose(fp);
    if (rc == CDD_C_SUCCESS)
      break;
  }

  /* Loop with default response array schema */
  responses[0].code = C_CDD_STR_LIT("200");
  responses[0].schema.ref_name = C_CDD_STR_LIT("Item");
  responses[0].schema.is_array = 1;
  responses[3].code = C_CDD_STR_LIT("default");
  responses[3].content_type = C_CDD_STR_LIT("application/json");
  responses[3].schema.ref_name = C_CDD_STR_LIT("Item");
  responses[3].schema.is_array = 1;
  for (io_fail = 0; io_fail < 100; ++io_fail) {
    fp = cdd_test_tmpfile_global();
    g_io_calls = 0;
    g_fail_io_after = io_fail;
    codegen_client_write_body(fp, &op, &spec, "/items/{id}", NULL);
    g_fail_io_after = -1;
    fclose(fp);
  }

  /* Loop form urlencoded with arr_pipe */
  {
    struct OpenAPI_Encoding pipe_enc;
    struct OpenAPI_MediaType pipe_mt;
    struct OpenAPI_Spec pipe_spec;
    struct OpenAPI_Operation pipe_op;

    ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&pipe_spec));
    pipe_spec.defined_schemas =
        (struct StructFields *)calloc(1, sizeof(struct StructFields));
    pipe_spec.defined_schema_names = (char **)calloc(1, sizeof(char *));
    ASSERT(pipe_spec.defined_schemas);
    ASSERT(pipe_spec.defined_schema_names);
    struct_fields_init(&pipe_spec.defined_schemas[0]);
    struct_fields_add(&pipe_spec.defined_schemas[0], "arr_p", "array", "string",
                      NULL, NULL);
    c_cdd_strdup("PipeSchema", &pipe_spec.defined_schema_names[0]);
    pipe_spec.n_defined_schemas = 1;

    memset(&pipe_op, 0, sizeof(pipe_op));
    memset(&pipe_enc, 0, sizeof(pipe_enc));
    memset(&pipe_mt, 0, sizeof(pipe_mt));

    pipe_op.req_body.ref_name = C_CDD_STR_LIT("PipeSchema");
    pipe_op.req_body.content_type =
        C_CDD_STR_LIT("application/x-www-form-urlencoded");

    pipe_enc.name = C_CDD_STR_LIT("arr_p");
    pipe_enc.style_set = 1;
    pipe_enc.style = OA_STYLE_PIPE_DELIMITED;

    pipe_mt.name = C_CDD_STR_LIT("application/x-www-form-urlencoded");
    pipe_mt.encoding = &pipe_enc;
    pipe_mt.n_encoding = 1;
    pipe_op.req_body_media_types = &pipe_mt;
    pipe_op.n_req_body_media_types = 1;

    for (io_fail = 0; io_fail < 40; ++io_fail) {
      fp = cdd_test_tmpfile_global();
      g_io_calls = 0;
      g_fail_io_after = io_fail;
      client_body_write_form_urlencoded_body(fp, &pipe_op, &pipe_spec);
      g_fail_io_after = -1;
      fclose(fp);
    }
    openapi_spec_free(&pipe_spec);
  }

  /* Loop with security header scheme */
  {
    struct OpenAPI_SecurityScheme s_sch;
    memset(&s_sch, 0, sizeof(s_sch));
    s_sch.type = OA_SEC_APIKEY;
    s_sch.in = OA_SEC_IN_HEADER;
    c_cdd_strdup("s_key", &s_sch.name);
    spec.security_schemes =
        (struct OpenAPI_SecurityScheme *)calloc(1, sizeof(s_sch));
    ASSERT(spec.security_schemes);
    spec.security_schemes[0] = s_sch;
    spec.n_security_schemes = 1;
    spec.security_set = 0;
    op.security_set = 0;
    for (io_fail = 0; io_fail < 40; ++io_fail) {
      fp = cdd_test_tmpfile_global();
      g_io_calls = 0;
      g_fail_io_after = io_fail;
      codegen_client_write_body(fp, &op, &spec, "/items/{id}", NULL);
      g_fail_io_after = -1;
      fclose(fp);
    }
  }

  g_fail_io_after = -1;
  openapi_spec_free(&spec);
  PASS();
}

/**
 * @brief Test targeted remaining branches to reach 100% coverage.
 *
 * @return GREATEST_TEST_RES.
 */
TEST test_client_body_targeted_remaining_branches(void) {
  FILE *fp;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  struct OpenAPI_Encoding encs[15];
  struct OpenAPI_Header hdrs[4];
  struct OpenAPI_MediaType mt;
  int io_fail;

  ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
  spec.defined_schemas =
      (struct StructFields *)calloc(2, sizeof(struct StructFields));
  spec.defined_schema_names = (char **)calloc(4, sizeof(char *));
  ASSERT(spec.defined_schemas);
  ASSERT(spec.defined_schema_names);

  /* Schema with all multipart types */
  struct_fields_init(&spec.defined_schemas[0]);
  struct_fields_add(&spec.defined_schemas[0], "f_str", "string", NULL, NULL,
                    NULL);
  struct_fields_add(&spec.defined_schemas[0], "f_int", "integer", NULL, NULL,
                    NULL);
  struct_fields_add(&spec.defined_schemas[0], "f_num", "number", NULL, NULL,
                    NULL);
  struct_fields_add(&spec.defined_schemas[0], "f_bool", "boolean", NULL, NULL,
                    NULL);
  struct_fields_add(&spec.defined_schemas[0], "f_obj", "object", "SubM", NULL,
                    NULL);
  struct_fields_add(&spec.defined_schemas[0], "f_arr_str", "array", "string",
                    NULL, NULL);
  struct_fields_add(&spec.defined_schemas[0], "f_arr_int", "array", "integer",
                    NULL, NULL);
  struct_fields_add(&spec.defined_schemas[0], "f_arr_num", "array", "number",
                    NULL, NULL);
  struct_fields_add(&spec.defined_schemas[0], "f_arr_bool", "array", "boolean",
                    NULL, NULL);
  struct_fields_add(&spec.defined_schemas[0], "f_arr_obj", "array", "SubM",
                    NULL, NULL);
  c_cdd_strdup("MultiAll", &spec.defined_schema_names[0]);

  struct_fields_init(&spec.defined_schemas[1]);
  struct_fields_add(&spec.defined_schemas[1], "id", "integer", NULL, NULL,
                    NULL);
  c_cdd_strdup("SubM", &spec.defined_schema_names[1]);
  spec.n_defined_schemas = 2;

  memset(&op, 0, sizeof(op));
  memset(encs, 0, sizeof(encs));
  memset(hdrs, 0, sizeof(hdrs));
  memset(&mt, 0, sizeof(mt));

  hdrs[0].name = C_CDD_STR_LIT("X-Custom-Hdr");
  hdrs[0].type = C_CDD_STR_LIT("string");

  encs[0].name = C_CDD_STR_LIT("f_str");
  encs[0].headers = hdrs;
  encs[0].n_headers = 1;

  encs[1].name = C_CDD_STR_LIT("f_int");
  encs[1].headers = hdrs;
  encs[1].n_headers = 1;

  encs[2].name = C_CDD_STR_LIT("f_num");
  encs[2].headers = hdrs;
  encs[2].n_headers = 1;

  encs[3].name = C_CDD_STR_LIT("f_bool");
  encs[3].headers = hdrs;
  encs[3].n_headers = 1;

  encs[4].name = C_CDD_STR_LIT("f_obj");
  encs[4].headers = hdrs;
  encs[4].n_headers = 1;

  encs[5].name = C_CDD_STR_LIT("f_arr_str");
  encs[5].headers = hdrs;
  encs[5].n_headers = 1;

  encs[6].name = C_CDD_STR_LIT("f_arr_int");
  encs[6].headers = hdrs;
  encs[6].n_headers = 1;

  encs[7].name = C_CDD_STR_LIT("f_arr_num");
  encs[7].headers = hdrs;
  encs[7].n_headers = 1;

  encs[8].name = C_CDD_STR_LIT("f_arr_bool");
  encs[8].headers = hdrs;
  encs[8].n_headers = 1;

  encs[9].name = C_CDD_STR_LIT("f_arr_obj");
  encs[9].headers = hdrs;
  encs[9].n_headers = 1;

  mt.name = C_CDD_STR_LIT("multipart/form-data");
  mt.encoding = encs;
  mt.n_encoding = 10;

  op.operation_id = C_CDD_STR_LIT("testMultiAll");
  op.verb = OA_VERB_POST;
  op.method = C_CDD_STR_LIT("post");
  op.req_body.ref_name = C_CDD_STR_LIT("MultiAll");
  op.req_body.content_type = C_CDD_STR_LIT("multipart/form-data");
  op.req_body_media_types = &mt;
  op.n_req_body_media_types = 1;

  /* Run I/O loop across client_body_write_multipart_body to trip every single
   * write_multipart_part_headers return error */
  for (io_fail = 0; io_fail < 350; ++io_fail) {
    fp = cdd_test_tmpfile_global();
    g_io_calls = 0;
    g_fail_io_after = io_fail;
    client_body_write_multipart_body(fp, &op, &spec);
    g_fail_io_after = -1;
    fclose(fp);
  }

  /* Run I/O loop on operation with req_json, cookie_str, 200 binary, 2XX text,
   * default */
  {
    struct OpenAPI_Parameter t_params[3];
    struct OpenAPI_Response t_responses[4];
    struct OpenAPI_Operation t_op;

    memset(&t_op, 0, sizeof(t_op));
    memset(t_params, 0, sizeof(t_params));
    memset(t_responses, 0, sizeof(t_responses));

    t_op.operation_id = C_CDD_STR_LIT("testCleanupIo");
    t_op.verb = OA_VERB_POST;
    t_op.method = C_CDD_STR_LIT("post");
    t_op.req_body.ref_name = C_CDD_STR_LIT("MultiAll");
    t_op.req_body.content_type = C_CDD_STR_LIT("application/json");

    t_params[0].name = C_CDD_STR_LIT("cookie_c");
    t_params[0].in = OA_PARAM_IN_COOKIE;
    t_params[0].type = C_CDD_STR_LIT("string");

    t_params[1].name = C_CDD_STR_LIT("query_q");
    t_params[1].in = OA_PARAM_IN_QUERY;
    t_params[1].type = C_CDD_STR_LIT("string");

    t_op.parameters = t_params;
    t_op.n_parameters = 2;

    t_responses[0].code = C_CDD_STR_LIT("200");
    t_responses[0].content_type = C_CDD_STR_LIT("application/octet-stream");

    t_responses[1].code = C_CDD_STR_LIT("2XX");
    t_responses[1].content_type = C_CDD_STR_LIT("text/plain");
    t_responses[1].schema.inline_type = C_CDD_STR_LIT("string");

    t_responses[2].code = C_CDD_STR_LIT("default");
    t_responses[2].content_type = C_CDD_STR_LIT("application/octet-stream");

    t_op.responses = t_responses;
    t_op.n_responses = 3;

    for (io_fail = 0; io_fail < 150; ++io_fail) {
      fp = cdd_test_tmpfile_global();
      g_io_calls = 0;
      g_fail_io_after = io_fail;
      codegen_client_write_body(fp, &t_op, &spec, "/test", NULL);
      g_fail_io_after = -1;
      fclose(fp);
    }

    /* Form body cleanup loop */
    t_op.req_body.content_type =
        C_CDD_STR_LIT("application/x-www-form-urlencoded");
    for (io_fail = 0; io_fail < 150; ++io_fail) {
      fp = cdd_test_tmpfile_global();
      g_io_calls = 0;
      g_fail_io_after = io_fail;
      codegen_client_write_body(fp, &t_op, &spec, "/test", NULL);
      g_fail_io_after = -1;
      fclose(fp);
    }
  }

  g_fail_io_after = -1;
  openapi_spec_free(&spec);
  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CODEGEN_CLIENT_BODY_PARAMS_H */
