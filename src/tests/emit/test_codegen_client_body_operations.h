/**
 * @file test_codegen_client_body_operations.h
 * @brief Unit tests for client body operations and sub emitters.
 */

#ifndef TEST_CODEGEN_CLIENT_BODY_OPERATIONS_H
#define TEST_CODEGEN_CLIENT_BODY_OPERATIONS_H

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
 * @brief Test direct calls to body writer sub-emitters and validation.
 *
 * @return GREATEST_TEST_RES.
 */
TEST test_client_body_writer_sub_emitters(void) {
  FILE *fp;
  struct OpenAPI_SchemaRef schema;
  struct OpenAPI_Operation op;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Encoding enc;
  cdd_c_error_t rc;

  /* Null argument assertions */
  rc = client_body_write_text_plain_success(NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  rc = client_body_write_binary_success(NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  rc = client_body_write_inline_json_parse(NULL, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  rc = client_body_write_joined_form_array(NULL, NULL, NULL, NULL, '&', NULL, 0,
                                           0);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  rc = client_body_write_header_param_logic(NULL, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  rc = client_body_write_form_urlencoded_body(NULL, NULL, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  rc = client_body_write_cookie_param_logic(NULL, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  rc = client_body_write_multipart_part_headers(NULL, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  rc = client_body_write_multipart_body(NULL, NULL, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  rc = codegen_client_write_body(NULL, NULL, NULL, NULL, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  /* Direct writer calls with temp file */
  fp = cdd_test_tmpfile_global();
  ASSERT(fp != NULL);

  rc = client_body_write_text_plain_success(fp);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = client_body_write_binary_success(fp);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  memset(&schema, 0, sizeof(schema));
  schema.inline_type = C_CDD_STR_LIT("string");
  rc = client_body_write_inline_json_parse(fp, &schema);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  schema.inline_type = C_CDD_STR_LIT("integer");
  rc = client_body_write_inline_json_parse(fp, &schema);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  schema.inline_type = C_CDD_STR_LIT("number");
  rc = client_body_write_inline_json_parse(fp, &schema);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  schema.inline_type = C_CDD_STR_LIT("boolean");
  rc = client_body_write_inline_json_parse(fp, &schema);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  schema.is_array = 1;
  schema.inline_type = C_CDD_STR_LIT("string");
  rc = client_body_write_inline_json_parse(fp, &schema);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  schema.inline_type = C_CDD_STR_LIT("integer");
  rc = client_body_write_inline_json_parse(fp, &schema);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  schema.inline_type = C_CDD_STR_LIT("number");
  rc = client_body_write_inline_json_parse(fp, &schema);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  schema.inline_type = C_CDD_STR_LIT("boolean");
  rc = client_body_write_inline_json_parse(fp, &schema);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  schema.inline_type = C_CDD_STR_LIT("CustomUnknown");
  rc = client_body_write_inline_json_parse(fp, &schema);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  schema.is_array = 0;
  rc = client_body_write_inline_json_parse(fp, &schema);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  g_io_calls = 0;
  g_fail_io_after = 4;
  rc = client_body_write_inline_json_parse(fp, &schema);
  ASSERT_EQ(CDD_C_ERROR_IO, rc);
  g_fail_io_after = -1;

  /* joined form array variants */
  rc = client_body_write_joined_form_array(fp, "tags", "n_tags", "string", '|',
                                           NULL, 0, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = client_body_write_joined_form_array(fp, "tags", "n_tags", "string", ' ',
                                           "url_encode", 1, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = client_body_write_joined_form_array(fp, "items", "n_items", "Item", '&',
                                           NULL, 0, 1);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = client_body_write_joined_form_array(fp, "nums", "n_nums", "integer", ',',
                                           NULL, 0, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = client_body_write_joined_form_array(fp, "rates", "n_rates", "number",
                                           ',', NULL, 0, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = client_body_write_joined_form_array(fp, "flags", "n_flags", "boolean",
                                           ',', NULL, 0, 0);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* multipart part headers */
  memset(&enc, 0, sizeof(enc));
  rc = client_body_write_multipart_part_headers(fp, &enc);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  memset(&op, 0, sizeof(op));
  rc = client_body_write_cookie_param_logic(fp, &op);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = client_body_write_header_param_logic(fp, &op);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  memset(&spec, 0, sizeof(spec));
  rc = client_body_write_form_urlencoded_body(fp, &op, &spec);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = client_body_write_multipart_body(fp, &op, &spec);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  fclose(fp);
  PASS();
}

/**
 * @brief Comprehensive operation test covering form urlencoded, cookie, and
 * multipart body edge cases.
 *
 * @return GREATEST_TEST_RES.
 */
TEST test_client_body_all_operation_patterns(void) {
  FILE *fp;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  struct OpenAPI_Parameter params[25];
  struct OpenAPI_Response responses[8];
  struct OpenAPI_Encoding encs[8];
  struct OpenAPI_Header hdrs[6];
  struct OpenAPI_MediaType mt_form;
  struct OpenAPI_MediaType mt_mp;
  cdd_c_error_t rc;

  ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
  spec.defined_schemas =
      (struct StructFields *)calloc(2, sizeof(struct StructFields));
  spec.defined_schema_names = (char **)calloc(4, sizeof(char *));
  ASSERT(spec.defined_schemas);
  ASSERT(spec.defined_schema_names);

  /* Schema 0: Complex form schema */
  struct_fields_init(&spec.defined_schemas[0]);
  struct_fields_add(&spec.defined_schemas[0], "str_val", "string", NULL, NULL,
                    NULL);
  struct_fields_add(&spec.defined_schemas[0], "res_val", "string", NULL, NULL,
                    NULL);
  struct_fields_add(&spec.defined_schemas[0], "int_val", "integer", NULL, NULL,
                    NULL);
  struct_fields_add(&spec.defined_schemas[0], "num_val", "number", NULL, NULL,
                    NULL);
  struct_fields_add(&spec.defined_schemas[0], "bool_val", "boolean", NULL, NULL,
                    NULL);
  struct_fields_add(&spec.defined_schemas[0], "arr_matrix", "array", "string",
                    NULL, NULL);
  struct_fields_add(&spec.defined_schemas[0], "arr_pipe", "array", "string",
                    NULL, NULL);
  struct_fields_add(&spec.defined_schemas[0], "arr_space", "array", "string",
                    NULL, NULL);
  struct_fields_add(&spec.defined_schemas[0], "arr_num", "array", "number",
                    NULL, NULL);
  struct_fields_add(&spec.defined_schemas[0], "arr_bool", "array", "boolean",
                    NULL, NULL);
  struct_fields_add(&spec.defined_schemas[0], "arr_obj", "array", "SubModel",
                    NULL, NULL);
  struct_fields_add(&spec.defined_schemas[0], "arr_unsup", "array",
                    "custom_unknown", NULL, NULL);
  struct_fields_add(&spec.defined_schemas[0], "obj_deep", "object", "SubModel",
                    NULL, NULL);
  struct_fields_add(&spec.defined_schemas[0], "obj_space", "object", "SubModel",
                    NULL, NULL);
  struct_fields_add(&spec.defined_schemas[0], "obj_pipe", "object", "SubModel",
                    NULL, NULL);
  struct_fields_add(&spec.defined_schemas[0], "obj_matrix", "object",
                    "SubModel", NULL, NULL);
  struct_fields_add(&spec.defined_schemas[0], "obj_val", "object", "SubModel",
                    NULL, NULL);
  struct_fields_add(&spec.defined_schemas[0], "obj_noref", "object", "", NULL,
                    NULL);
  struct_fields_add(&spec.defined_schemas[0], "unsupported", "custom_blob",
                    NULL, NULL, NULL);
  c_cdd_strdup("ComplexForm", &spec.defined_schema_names[0]);

  /* Schema 1: SubModel */
  struct_fields_init(&spec.defined_schemas[1]);
  struct_fields_add(&spec.defined_schemas[1], "p_str", "string", NULL, NULL,
                    NULL);
  struct_fields_add(&spec.defined_schemas[1], "p_int", "integer", NULL, NULL,
                    NULL);
  struct_fields_add(&spec.defined_schemas[1], "p_num", "number", NULL, NULL,
                    NULL);
  struct_fields_add(&spec.defined_schemas[1], "p_bool", "boolean", NULL, NULL,
                    NULL);
  c_cdd_strdup("SubModel", &spec.defined_schema_names[1]);
  spec.n_defined_schemas = 2;

  /* Setup operation */
  memset(&op, 0, sizeof(op));
  memset(params, 0, sizeof(params));
  memset(responses, 0, sizeof(responses));
  memset(encs, 0, sizeof(encs));
  memset(hdrs, 0, sizeof(hdrs));

  op.operation_id = C_CDD_STR_LIT("testComplexAll");
  op.verb = OA_VERB_POST;
  op.method = C_CDD_STR_LIT("post");

  /* Parameters: path, query, header, cookie */
  params[0].name = C_CDD_STR_LIT("path_id");
  params[0].in = OA_PARAM_IN_PATH;
  params[0].type = C_CDD_STR_LIT("string");

  params[1].name = C_CDD_STR_LIT("filter");
  params[1].in = OA_PARAM_IN_QUERY;
  params[1].type = C_CDD_STR_LIT("string");

  params[2].name = C_CDD_STR_LIT("X-Api-Version");
  params[2].in = OA_PARAM_IN_HEADER;
  params[2].type = C_CDD_STR_LIT("string");

  params[3].name = C_CDD_STR_LIT("X-Json-Primitive");
  params[3].in = OA_PARAM_IN_HEADER;
  params[3].content_type = C_CDD_STR_LIT("application/json");
  params[3].type = C_CDD_STR_LIT("integer");

  params[4].name = C_CDD_STR_LIT("X-Json-Array-Prim");
  params[4].in = OA_PARAM_IN_HEADER;
  params[4].content_type = C_CDD_STR_LIT("application/json");
  params[4].is_array = 1;
  params[4].items_type = C_CDD_STR_LIT("string");

  params[5].name = C_CDD_STR_LIT("cookie_str");
  params[5].in = OA_PARAM_IN_COOKIE;
  params[5].type = C_CDD_STR_LIT("string");

  params[6].name = C_CDD_STR_LIT("cookie_int");
  params[6].in = OA_PARAM_IN_COOKIE;
  params[6].type = C_CDD_STR_LIT("integer");

  params[7].name = C_CDD_STR_LIT("cookie_arr");
  params[7].in = OA_PARAM_IN_COOKIE;
  params[7].is_array = 1;
  params[7].items_type = C_CDD_STR_LIT("integer");
  params[7].explode_set = 1;
  params[7].explode = 0;

  params[8].name = C_CDD_STR_LIT("cookie_no_enc_str");
  params[8].in = OA_PARAM_IN_COOKIE;
  params[8].style = OA_STYLE_COOKIE;
  params[8].type = C_CDD_STR_LIT("string");

  params[9].name = C_CDD_STR_LIT("cookie_no_enc_num");
  params[9].in = OA_PARAM_IN_COOKIE;
  params[9].style = OA_STYLE_COOKIE;
  params[9].type = C_CDD_STR_LIT("number");

  params[10].name = C_CDD_STR_LIT("cookie_no_enc_bool");
  params[10].in = OA_PARAM_IN_COOKIE;
  params[10].style = OA_STYLE_COOKIE;
  params[10].type = C_CDD_STR_LIT("boolean");

  params[11].name = C_CDD_STR_LIT("cookie_no_enc_arr");
  params[11].in = OA_PARAM_IN_COOKIE;
  params[11].style = OA_STYLE_COOKIE;
  params[11].is_array = 1;
  params[11].items_type = C_CDD_STR_LIT("string");

  op.parameters = params;
  op.n_parameters = 12;

  /* Request body: form urlencoded */
  op.req_body.ref_name = C_CDD_STR_LIT("ComplexForm");
  op.req_body.content_type = C_CDD_STR_LIT("application/x-www-form-urlencoded");

  memset(&mt_form, 0, sizeof(mt_form));
  mt_form.name = C_CDD_STR_LIT("application/x-www-form-urlencoded");
  mt_form.encoding = encs;
  mt_form.n_encoding = 6;

  encs[0].name = C_CDD_STR_LIT("res_val");
  encs[0].allow_reserved_set = 1;
  encs[0].allow_reserved = 1;

  encs[1].name = C_CDD_STR_LIT("arr_matrix");
  encs[1].style_set = 1;
  encs[1].style = OA_STYLE_MATRIX;

  encs[2].name = C_CDD_STR_LIT("obj_deep");
  encs[2].style_set = 1;
  encs[2].style = OA_STYLE_DEEP_OBJECT;
  encs[2].explode_set = 1;
  encs[2].explode = 1;
  encs[2].allow_reserved_set = 1;
  encs[2].allow_reserved = 1;

  encs[3].name = C_CDD_STR_LIT("obj_space");
  encs[3].style_set = 1;
  encs[3].style = OA_STYLE_SPACE_DELIMITED;

  encs[4].name = C_CDD_STR_LIT("obj_pipe");
  encs[4].style_set = 1;
  encs[4].style = OA_STYLE_PIPE_DELIMITED;

  encs[5].name = C_CDD_STR_LIT("obj_matrix");
  encs[5].style_set = 1;
  encs[5].style = OA_STYLE_MATRIX;

  encs[6].name = C_CDD_STR_LIT("arr_pipe");
  encs[6].style_set = 1;
  encs[6].style = OA_STYLE_PIPE_DELIMITED;

  encs[7].name = C_CDD_STR_LIT("arr_space");
  encs[7].style_set = 1;
  encs[7].style = OA_STYLE_SPACE_DELIMITED;

  op.req_body_media_types = &mt_form;
  op.n_req_body_media_types = 1;

  /* Responses: 200, 204, 1XX, 2XX, 3XX, 4XX, 5XX, default */
  responses[0].code = C_CDD_STR_LIT("200");
  responses[0].content_type = C_CDD_STR_LIT("application/json");
  responses[0].schema.ref_name = C_CDD_STR_LIT("SubModel");

  responses[1].code = C_CDD_STR_LIT("204");

  responses[2].code = C_CDD_STR_LIT("1XX");

  responses[3].code = C_CDD_STR_LIT("2XX");
  responses[3].content_type = C_CDD_STR_LIT("text/plain");
  responses[3].schema.inline_type = C_CDD_STR_LIT("string");

  responses[4].code = C_CDD_STR_LIT("3XX");

  responses[5].code = C_CDD_STR_LIT("4XX");

  responses[6].code = C_CDD_STR_LIT("5XX");

  responses[7].code = C_CDD_STR_LIT("default");
  responses[7].content_type = C_CDD_STR_LIT("application/octet-stream");

  op.responses = responses;
  op.n_responses = 8;

  fp = cdd_test_tmpfile_global();
  ASSERT(fp != NULL);
  rc = codegen_client_write_body(fp, &op, &spec, "/items/{path_id}",
                                 "\"https://override.api.com\"");
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  fclose(fp);

  /* Now test multipart body with part headers */
  op.req_body.ref_name = C_CDD_STR_LIT("ComplexForm");
  op.req_body.content_type = C_CDD_STR_LIT("multipart/form-data");

  memset(&mt_mp, 0, sizeof(mt_mp));
  mt_mp.name = C_CDD_STR_LIT("multipart/form-data");
  mt_mp.encoding = encs;
  mt_mp.n_encoding = 7;

  hdrs[0].name = C_CDD_STR_LIT("X-Part-Hdr");
  hdrs[0].type = C_CDD_STR_LIT("string");

  hdrs[1].name = C_CDD_STR_LIT("Content-Type");
  hdrs[1].type = C_CDD_STR_LIT("string");

  hdrs[2].name = C_CDD_STR_LIT("X-Array-Num");
  hdrs[2].type = C_CDD_STR_LIT("number");
  hdrs[2].is_array = 1;

  hdrs[3].name = C_CDD_STR_LIT("X-Array-Bool");
  hdrs[3].type = C_CDD_STR_LIT("boolean");
  hdrs[3].is_array = 1;

  hdrs[4].name = C_CDD_STR_LIT("X-Scalar-Bool");
  hdrs[4].type = C_CDD_STR_LIT("boolean");

  hdrs[5].name = C_CDD_STR_LIT("X-Obj-Hdr-Unexp");
  hdrs[5].type = C_CDD_STR_LIT("object");
  hdrs[5].explode_set = 1;
  hdrs[5].explode = 0;

  encs[0].name = C_CDD_STR_LIT("str_val");
  encs[0].headers = hdrs;
  encs[0].n_headers = 6;
  encs[0].content_type = C_CDD_STR_LIT("text/plain");

  encs[1].name = C_CDD_STR_LIT("obj_deep");
  encs[1].content_type = C_CDD_STR_LIT("application/json");

  encs[2].name = C_CDD_STR_LIT("int_val");
  encs[2].content_type = C_CDD_STR_LIT("text/plain");

  encs[3].name = C_CDD_STR_LIT("num_val");
  encs[3].content_type = C_CDD_STR_LIT("text/plain");

  encs[4].name = C_CDD_STR_LIT("bool_val");
  encs[4].content_type = C_CDD_STR_LIT("text/plain");

  encs[5].name = C_CDD_STR_LIT("arr_num");
  encs[5].content_type = C_CDD_STR_LIT("text/plain");

  encs[6].name = C_CDD_STR_LIT("arr_bool");
  encs[6].content_type = C_CDD_STR_LIT("text/plain");

  op.req_body_media_types = &mt_mp;
  op.n_req_body_media_types = 1;

  fp = cdd_test_tmpfile_global();
  ASSERT(fp != NULL);
  rc = codegen_client_write_body(fp, &op, &spec, "/items/{path_id}", NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  fclose(fp);

  /* Test cookie object with explode=0 and explode=1, with/without encoding */
  params[6].name = C_CDD_STR_LIT("cookie_obj_no_enc");
  params[6].in = OA_PARAM_IN_COOKIE;
  params[6].style = OA_STYLE_COOKIE;
  params[6].type = C_CDD_STR_LIT("object");
  params[6].is_array = 0;
  params[6].schema.ref_name = C_CDD_STR_LIT("SubModel");
  params[6].explode_set = 1;
  params[6].explode = 0;

  params[7].name = C_CDD_STR_LIT("cookie_obj_exp_no_enc");
  params[7].in = OA_PARAM_IN_COOKIE;
  params[7].style = OA_STYLE_COOKIE;
  params[7].type = C_CDD_STR_LIT("object");
  params[7].is_array = 0;
  params[7].schema.ref_name = C_CDD_STR_LIT("SubModel");
  params[7].explode_set = 1;
  params[7].explode = 1;

  params[8].name = C_CDD_STR_LIT("cookie_arr_num_unexp");
  params[8].in = OA_PARAM_IN_COOKIE;
  params[8].style = OA_STYLE_COOKIE;
  params[8].is_array = 1;
  params[8].items_type = C_CDD_STR_LIT("number");
  params[8].explode_set = 1;
  params[8].explode = 0;

  params[9].name = C_CDD_STR_LIT("cookie_arr_bool_unexp");
  params[9].in = OA_PARAM_IN_COOKIE;
  params[9].style = OA_STYLE_COOKIE;
  params[9].is_array = 1;
  params[9].items_type = C_CDD_STR_LIT("boolean");
  params[9].explode_set = 1;
  params[9].explode = 0;

  params[10].name = C_CDD_STR_LIT("cookie_arr_str_enc_unexp");
  params[10].in = OA_PARAM_IN_COOKIE;
  params[10].style = OA_STYLE_FORM;
  params[10].allow_reserved_set = 1;
  params[10].allow_reserved = 1;
  params[10].is_array = 1;
  params[10].items_type = C_CDD_STR_LIT("string");
  params[10].explode_set = 1;
  params[10].explode = 0;

  params[11].name = C_CDD_STR_LIT("cookie_str_enc");
  params[11].in = OA_PARAM_IN_COOKIE;
  params[11].style = OA_STYLE_FORM;
  params[11].type = C_CDD_STR_LIT("string");
  params[11].is_array = 0;
  params[11].allow_reserved_set = 1;
  params[11].allow_reserved = 1;

  fp = cdd_test_tmpfile_global();
  ASSERT(fp != NULL);
  rc = codegen_client_write_body(fp, &op, &spec, "/items/{path_id}", NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  fclose(fp);

  /* Systematic IO failure loop on form urlencoded */
  op.req_body.content_type = C_CDD_STR_LIT("application/x-www-form-urlencoded");
  op.req_body_media_types = &mt_form;
  op.n_req_body_media_types = 1;
  {
    int io_f;
    for (io_f = 0; io_f < 600; ++io_f) {
      fp = cdd_test_tmpfile_global();
      g_io_calls = 0;
      g_fail_io_after = io_f;
      rc = codegen_client_write_body(fp, &op, &spec, "/items/{path_id}", NULL);
      g_fail_io_after = -1;
      fclose(fp);
      if (rc == CDD_C_SUCCESS)
        break;
    }
  }

  /* Systematic IO failure loop on multipart */
  op.req_body.content_type = C_CDD_STR_LIT("multipart/form-data");
  op.req_body_media_types = &mt_mp;
  op.n_req_body_media_types = 1;
  {
    int io_f;
    for (io_f = 0; io_f < 1200; ++io_f) {
      fp = cdd_test_tmpfile_global();
      g_io_calls = 0;
      g_fail_io_after = io_f;
      rc = codegen_client_write_body(fp, &op, &spec, "/items/{path_id}", NULL);
      g_fail_io_after = -1;
      fclose(fp);
      if (rc == CDD_C_SUCCESS)
        break;
    }
  }

  openapi_spec_free(&spec);
  PASS();
}

/**
 * @brief Test additional coverage branches: default responses, inline types.
 *
 * @return GREATEST_TEST_RES.
 */
TEST test_client_body_default_responses_and_inlines(void) {
  FILE *fp;
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  struct OpenAPI_Parameter param;
  struct OpenAPI_Response responses[3];
  cdd_c_error_t rc;

  ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
  memset(&op, 0, sizeof(op));
  memset(&param, 0, sizeof(param));
  memset(responses, 0, sizeof(responses));

  op.operation_id = C_CDD_STR_LIT("testDefaults");
  op.verb = OA_VERB_GET;
  op.method = C_CDD_STR_LIT("get");

  param.name = C_CDD_STR_LIT("id");
  param.in = OA_PARAM_IN_PATH;
  param.type = C_CDD_STR_LIT("string");
  op.parameters = &param;
  op.n_parameters = 1;

  /* default response with text/plain */
  responses[0].code = C_CDD_STR_LIT("default");
  responses[0].content_type = C_CDD_STR_LIT("text/plain");
  responses[0].schema.inline_type = C_CDD_STR_LIT("string");
  op.responses = responses;
  op.n_responses = 1;

  fp = cdd_test_tmpfile_global();
  ASSERT(fp != NULL);
  rc = codegen_client_write_body(fp, &op, &spec, "/items/{id}", NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  fclose(fp);

  /* default response with inline non-string json */
  responses[0].content_type = C_CDD_STR_LIT("application/json");
  responses[0].schema.inline_type = C_CDD_STR_LIT("integer");
  fp = cdd_test_tmpfile_global();
  ASSERT(fp != NULL);
  rc = codegen_client_write_body(fp, &op, &spec, "/items/{id}", NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  fclose(fp);

  /* default response error (generic) */
  responses[0].content_type = NULL;
  responses[0].schema.inline_type = NULL;
  fp = cdd_test_tmpfile_global();
  ASSERT(fp != NULL);
  rc = codegen_client_write_body(fp, &op, &spec, "/items/{id}", NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  fclose(fp);

  /* 200 with inline non-string json */
  responses[0].code = C_CDD_STR_LIT("200");
  responses[0].content_type = C_CDD_STR_LIT("application/json");
  responses[0].schema.inline_type = C_CDD_STR_LIT("integer");
  fp = cdd_test_tmpfile_global();
  ASSERT(fp != NULL);
  rc = codegen_client_write_body(fp, &op, &spec, "/items/{id}", NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  fclose(fp);

  /* 2XX range response with inline non-string json */
  responses[0].code = C_CDD_STR_LIT("2XX");
  responses[0].content_type = C_CDD_STR_LIT("application/json");
  responses[0].schema.inline_type = C_CDD_STR_LIT("integer");
  fp = cdd_test_tmpfile_global();
  ASSERT(fp != NULL);
  rc = codegen_client_write_body(fp, &op, &spec, "/items/{id}", NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  fclose(fp);

  /* 2XX range response with binary */
  responses[0].code = C_CDD_STR_LIT("2XX");
  responses[0].content_type = C_CDD_STR_LIT("application/octet-stream");
  responses[0].schema.inline_type = NULL;
  fp = cdd_test_tmpfile_global();
  ASSERT(fp != NULL);
  rc = codegen_client_write_body(fp, &op, &spec, "/items/{id}", NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  fclose(fp);

  /* 200 response with array schema ref */
  responses[0].code = C_CDD_STR_LIT("200");
  responses[0].content_type = C_CDD_STR_LIT("application/json");
  responses[0].schema.ref_name = C_CDD_STR_LIT("Item");
  responses[0].schema.is_array = 1;
  responses[0].schema.inline_type = NULL;
  fp = cdd_test_tmpfile_global();
  ASSERT(fp != NULL);
  rc = codegen_client_write_body(fp, &op, &spec, "/items/{id}", NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  fclose(fp);

  /* default response matching 200 array schema */
  responses[0].code = C_CDD_STR_LIT("200");
  responses[0].schema.ref_name = C_CDD_STR_LIT("Item");
  responses[0].schema.is_array = 1;
  responses[1].code = C_CDD_STR_LIT("default");
  responses[1].content_type = C_CDD_STR_LIT("application/json");
  responses[1].schema.ref_name = C_CDD_STR_LIT("Item");
  responses[1].schema.is_array = 1;
  op.n_responses = 2;
  fp = cdd_test_tmpfile_global();
  ASSERT(fp != NULL);
  rc = codegen_client_write_body(fp, &op, &spec, "/items/{id}", NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  fclose(fp);

  /* only default response binary (no 2xx) */
  responses[0].code = C_CDD_STR_LIT("default");
  responses[0].content_type = C_CDD_STR_LIT("application/octet-stream");
  responses[0].schema.ref_name = NULL;
  responses[0].schema.is_array = 0;
  op.n_responses = 1;
  fp = cdd_test_tmpfile_global();
  ASSERT(fp != NULL);
  rc = codegen_client_write_body(fp, &op, &spec, "/items/{id}", NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  fclose(fp);

  /* default response matching 200 scalar schema */
  responses[0].code = C_CDD_STR_LIT("200");
  responses[0].content_type = C_CDD_STR_LIT("application/json");
  responses[0].schema.ref_name = C_CDD_STR_LIT("Item");
  responses[0].schema.is_array = 0;
  responses[1].code = C_CDD_STR_LIT("default");
  responses[1].content_type = C_CDD_STR_LIT("application/json");
  responses[1].schema.ref_name = C_CDD_STR_LIT("Item");
  responses[1].schema.is_array = 0;
  op.n_responses = 2;
  fp = cdd_test_tmpfile_global();
  ASSERT(fp != NULL);
  rc = codegen_client_write_body(fp, &op, &spec, "/items/{id}", NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  fclose(fp);

  /* default response matching 200 inline type */
  responses[0].code = C_CDD_STR_LIT("200");
  responses[0].schema.ref_name = NULL;
  responses[0].schema.inline_type = C_CDD_STR_LIT("integer");
  responses[0].schema.is_array = 0;
  responses[1].code = C_CDD_STR_LIT("default");
  responses[1].schema.ref_name = NULL;
  responses[1].schema.inline_type = C_CDD_STR_LIT("integer");
  responses[1].schema.is_array = 0;
  op.n_responses = 2;
  fp = cdd_test_tmpfile_global();
  ASSERT(fp != NULL);
  rc = codegen_client_write_body(fp, &op, &spec, "/items/{id}", NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  fclose(fp);

  /* 200 success with default error */
  responses[0].code = C_CDD_STR_LIT("200");
  responses[0].schema.ref_name = C_CDD_STR_LIT("Item");
  responses[0].schema.is_array = 0;
  responses[1].code = C_CDD_STR_LIT("default");
  responses[1].schema.ref_name = NULL;
  responses[1].schema.inline_type = NULL;
  responses[1].content_type = NULL;
  op.n_responses = 2;
  fp = cdd_test_tmpfile_global();
  ASSERT(fp != NULL);
  rc = codegen_client_write_body(fp, &op, &spec, "/items/{id}", NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  fclose(fp);

  /* 2XX with schema ref */
  responses[0].code = C_CDD_STR_LIT("2XX");
  responses[0].content_type = C_CDD_STR_LIT("application/json");
  responses[0].schema.ref_name = C_CDD_STR_LIT("Item");
  responses[0].schema.inline_type = NULL;
  responses[0].schema.is_array = 0;
  op.n_responses = 1;
  fp = cdd_test_tmpfile_global();
  ASSERT(fp != NULL);
  rc = codegen_client_write_body(fp, &op, &spec, "/items/{id}", NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  fclose(fp);

  /* Non-literal, non-range response code */
  responses[0].code = C_CDD_STR_LIT("INVALID_CODE");
  op.n_responses = 1;
  fp = cdd_test_tmpfile_global();
  ASSERT(fp != NULL);
  rc = codegen_client_write_body(fp, &op, &spec, "/items/{id}", NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  fclose(fp);

  openapi_spec_free(&spec);
  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CODEGEN_CLIENT_BODY_OPERATIONS_H */
