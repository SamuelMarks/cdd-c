/**
 * @file test_codegen_client_sig_coverage.h
 * @brief Unit tests for C Client Signature Generation branch and IO error
 * coverage.
 */

#ifndef TEST_CODEGEN_CLIENT_SIG_COVERAGE_H
#define TEST_CODEGEN_CLIENT_SIG_COVERAGE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "emit/test_codegen_client_sig_common.h"
/* clang-format on */

/**
 * @brief Unit test test_sig_complete_branches_coverage.
 * @return Greatest test result.
 */
TEST test_sig_complete_branches_coverage(void) {
  int int_val = 0;
  const char *str_val = NULL;
  char buf[64];
  struct OpenAPI_Operation op;
  struct OpenAPI_Parameter param;
  struct OpenAPI_Response resp[3];
  struct OpenAPI_MediaType mt;
  struct OpenAPI_Encoding enc;
  struct OpenAPI_Header hdr[3];
  const struct OpenAPI_Response *out_resp = NULL;
  const struct OpenAPI_SchemaRef *out_schema = NULL;
  char *code = NULL;

  /* 1. media_type_is_textual: text/html and application/xml triggers */
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_media_type_is_textual("text/html", &int_val));
  ASSERT_EQ(1, int_val);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_media_type_is_textual("application/xml", &int_val));
  ASSERT_EQ(1, int_val);
  g_cdd_fail_media_type_ieq = 2;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_sig_media_type_is_textual("application/custom", &int_val));

  /* 2. querystring_param_is_form_object: json error and non-json with ref_name
   */
  memset(&param, 0, sizeof(param));
  param.content_type = cdd_test_mut_str("application/json");
  g_cdd_fail_media_type_is_json = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_sig_querystring_param_is_form_object(&param, &int_val));
  param.content_type = NULL;
  param.schema.ref_name = cdd_test_mut_str("MyFormObj");
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_querystring_param_is_form_object(&param, &int_val));
  ASSERT_EQ(1, int_val);

  /* 2b. p.in != OA_PARAM_IN_QUERYSTRING in json_array_item_type and
   * json_array_item_ref */
  param.in = OA_PARAM_IN_HEADER;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_querystring_param_json_array_item_type(
                               &param, &str_val));
  ASSERT(str_val == NULL);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_querystring_param_json_array_item_ref(
                               &param, &str_val));
  ASSERT(str_val == NULL);

  /* 3. g_cdd_fail_sanitize_ident inside multipart_header_param_name */
  g_cdd_fail_sanitize_ident = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, cdd_test_sig_multipart_header_param_name(
                                     buf, sizeof(buf), "f", "h"));

  /* 3b. g_cdd_fail_c_cdd_str_iequal in header_name_is_content_type */
  g_cdd_fail_c_cdd_str_iequal = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_sig_header_name_is_content_type("Content-Type", &int_val));

  /* 4. get_success_response with NULL code */
  memset(&op, 0, sizeof(op));
  memset(resp, 0, sizeof(resp));
  op.responses = resp;
  op.n_responses = 2;
  resp[0].code = NULL;
  resp[1].code = cdd_test_mut_str("200");
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_get_success_response(&op, &out_resp));
  ASSERT(out_resp == &resp[1]);

  /* 5. get_success_schema: NULL code, 2XX without schema, and default response
   * with schema */
  resp[0].code = NULL;
  resp[1].code = cdd_test_mut_str("2XX"); /* no schema, so continue */
  resp[1].schema.ref_name = NULL;
  resp[1].schema.inline_type = NULL;
  resp[1].schema.is_array = 0;
  op.n_responses = 3;
  resp[2].code = cdd_test_mut_str("default");
  resp[2].schema.ref_name = cdd_test_mut_str("DefaultModel");
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_get_success_schema(&op, &out_schema));
  ASSERT(out_schema == &resp[2].schema);

  /* 5b. get_success_schema: default response fail and default response fallback
   */
  memset(&op, 0, sizeof(op));
  memset(resp, 0, sizeof(resp));
  op.responses = resp;
  op.n_responses = 1;
  resp[0].code = cdd_test_mut_str("default");
  resp[0].schema.ref_name = cdd_test_mut_str("DefaultModel");
  g_cdd_fail_schema_has_inline = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_sig_get_success_schema(&op, &out_schema));

  resp[0].schema.ref_name = NULL;
  resp[0].schema.inline_type = NULL;
  resp[0].schema.is_array = 0;
  op.req_body.ref_name = cdd_test_mut_str("FallbackBody");
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_get_success_schema(&op, &out_schema));
  ASSERT(out_schema == &op.req_body);

  /* 6. codegen_client_write_signature: is_json_ref_val and is_form_obj in
   * querystring param */
  memset(&op, 0, sizeof(op));
  memset(&param, 0, sizeof(param));
  op.operation_id = cdd_test_mut_str("testQsJsonRef");
  op.n_parameters = 1;
  op.parameters = &param;
  param.name = cdd_test_mut_str("filter");
  param.in = OA_PARAM_IN_QUERYSTRING;
  param.content_type = cdd_test_mut_str("application/json");
  param.schema.ref_name = cdd_test_mut_str("FilterRef");
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  ASSERT(code != NULL);
  ASSERT(strstr(code, "const struct FilterRef *filter") != NULL);
  free(code);
  code = NULL;

  param.content_type = NULL;
  param.schema.ref_name = cdd_test_mut_str("FormRef");
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  ASSERT(code != NULL);
  ASSERT(strstr(code, "const struct OpenAPI_KV *filter, size_t filter_len") !=
         NULL);
  free(code);
  code = NULL;

  /* 7. codegen_client_write_signature: header JSON param with ref_name =
   * p->type */
  param.in = OA_PARAM_IN_HEADER;
  param.content_type = cdd_test_mut_str("application/json");
  param.schema.ref_name = NULL;
  param.type = cdd_test_mut_str("CustomType");
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  ASSERT(code != NULL);
  ASSERT(strstr(code, "const struct CustomType *filter") != NULL);
  free(code);
  code = NULL;

  /* 8. codegen_client_write_signature: header JSON param with array of
   * primitives (string) */
  param.content_type = cdd_test_mut_str("application/json");
  param.is_array = 1;
  param.items_type = cdd_test_mut_str("string");
  param.type = cdd_test_mut_str("array");
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  ASSERT(code != NULL);
  ASSERT(strstr(code, "const char **filter, size_t filter_len") != NULL);
  free(code);
  code = NULL;

  /* failure of is_primitive_type on array items */
  param.type = NULL;
  g_cdd_fail_is_primitive_type = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, gen_sig(&op, NULL, &code));

  /* 9. Request body array of strings and array of custom structs */
  memset(&op, 0, sizeof(op));
  op.operation_id = cdd_test_mut_str("testReqBodyArrays");
  op.req_body.content_type = cdd_test_mut_str("application/json");
  op.req_body.is_array = 1;
  op.req_body.ref_name = cdd_test_mut_str("string");
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  ASSERT(code != NULL);
  ASSERT(strstr(code, "const char **body, size_t body_len") != NULL);
  free(code);
  code = NULL;

  op.req_body.ref_name = cdd_test_mut_str("MyCustomItem");
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  ASSERT(code != NULL);
  ASSERT(strstr(code, "struct MyCustomItem **body, size_t body_len") != NULL);
  free(code);
  code = NULL;

  /* 10. Multipart with NULL name, NULL headers, 0 headers, Content-Type skip,
   * and object */
  memset(&op, 0, sizeof(op));
  memset(&mt, 0, sizeof(mt));
  memset(&enc, 0, sizeof(enc));
  memset(hdr, 0, sizeof(hdr));
  op.operation_id = cdd_test_mut_str("testMultipartCases");
  op.req_body.content_type = cdd_test_mut_str("multipart/form-data");
  op.n_req_body_media_types = 1;
  op.req_body_media_types = &mt;
  mt.name = cdd_test_mut_str("multipart/form-data");
  mt.n_encoding = 1;
  mt.encoding = &enc;
  enc.name = cdd_test_mut_str("part1");
  enc.n_headers = 3;
  enc.headers = hdr;
  hdr[0].name = NULL;                             /* skipped */
  hdr[1].name = cdd_test_mut_str("Content-Type"); /* skipped */
  hdr[2].name = cdd_test_mut_str("X-Meta-Obj");
  hdr[2].type = cdd_test_mut_str("object");
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  ASSERT(code != NULL);
  ASSERT(strstr(code, "const struct OpenAPI_KV *part1_hdr_X_Meta_Obj, size_t "
                      "part1_hdr_X_Meta_Obj_len") != NULL);
  free(code);
  code = NULL;

  /* Empty encoding array */
  mt.n_encoding = 0;
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  ASSERT(code != NULL);
  free(code);
  code = NULL;

  /* Encoding with NULL headers */
  mt.n_encoding = 1;
  enc.headers = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  ASSERT(code != NULL);
  free(code);
  code = NULL;
  enc.headers = hdr;

  /* Multipart step 2b failure */
  g_cdd_fail_media_type_is_multipart_form = 2;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, gen_sig(&op, NULL, &code));

  /* 11. Success schema variants: struct array, string/int/bool/custom inline,
   * and binary */
  memset(&op, 0, sizeof(op));
  memset(resp, 0, sizeof(resp));
  op.operation_id = cdd_test_mut_str("testSuccessCases");
  op.responses = resp;
  op.n_responses = 1;
  resp[0].code = cdd_test_mut_str("200");

  /* Failure of schema_has_inline in step 3 */
  resp[0].schema.inline_type = cdd_test_mut_str("string");
  g_cdd_fail_schema_has_inline = 2;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, gen_sig(&op, NULL, &code));

  /* Array of custom structs */
  resp[0].schema.is_array = 1;
  resp[0].schema.ref_name = cdd_test_mut_str("MyRespStruct");
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  ASSERT(code != NULL);
  ASSERT(strstr(code, "struct MyRespStruct ***out, size_t *out_len") != NULL);
  free(code);
  code = NULL;

  /* Array of inline string */
  resp[0].schema.ref_name = NULL;
  resp[0].schema.inline_type = cdd_test_mut_str("string");
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  ASSERT(code != NULL);
  ASSERT(strstr(code, "char ***out, size_t *out_len") != NULL);
  free(code);
  code = NULL;

  /* Array of inline int */
  resp[0].schema.inline_type = cdd_test_mut_str("integer");
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  ASSERT(code != NULL);
  ASSERT(strstr(code, "int **out, size_t *out_len") != NULL);
  free(code);
  code = NULL;

  /* Array of inline bool */
  resp[0].schema.inline_type = cdd_test_mut_str("boolean");
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  ASSERT(code != NULL);
  ASSERT(strstr(code, "int **out, size_t *out_len") != NULL);
  free(code);
  code = NULL;

  /* Array of inline custom */
  resp[0].schema.inline_type = cdd_test_mut_str("custom");
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  ASSERT(code != NULL);
  ASSERT(strstr(code, "void **out, size_t *out_len") != NULL);
  free(code);
  code = NULL;

  /* Non-array custom struct */
  resp[0].schema.is_array = 0;
  resp[0].schema.inline_type = NULL;
  resp[0].schema.ref_name = cdd_test_mut_str("SingleStruct");
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  ASSERT(code != NULL);
  ASSERT(strstr(code, "struct SingleStruct **out") != NULL);
  free(code);
  code = NULL;

  /* Non-array inline string */
  resp[0].schema.ref_name = NULL;
  resp[0].schema.inline_type = cdd_test_mut_str("string");
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  ASSERT(code != NULL);
  ASSERT(strstr(code, "char **out") != NULL);
  free(code);
  code = NULL;

  /* Non-array inline int */
  resp[0].schema.inline_type = cdd_test_mut_str("integer");
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  ASSERT(code != NULL);
  ASSERT(strstr(code, "int *out") != NULL);
  free(code);
  code = NULL;

  /* Non-array inline bool */
  resp[0].schema.inline_type = cdd_test_mut_str("boolean");
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  ASSERT(code != NULL);
  ASSERT(strstr(code, "int *out") != NULL);
  free(code);
  code = NULL;

  /* Non-array inline custom */
  resp[0].schema.inline_type = cdd_test_mut_str("custom");
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  ASSERT(code != NULL);
  ASSERT(strstr(code, "void *out") != NULL);
  free(code);
  code = NULL;

  /* Binary response success */
  resp[0].schema.inline_type = NULL;
  resp[0].content_type = cdd_test_mut_str("application/octet-stream");
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  ASSERT(code != NULL);
  ASSERT(strstr(code, "unsigned char **out, size_t *out_len") != NULL);
  free(code);
  code = NULL;

  /* 12. IO failure loops to hit CHECK_IO error returns */
  {
    int io_i;
    FILE *fp = cdd_test_tmpfile_global();
    if (fp) {
      /* hitting qs json ref */
      memset(&op, 0, sizeof(op));
      memset(&param, 0, sizeof(param));
      op.operation_id = cdd_test_mut_str("testQsJsonRef");
      op.n_parameters = 1;
      op.parameters = &param;
      param.name = cdd_test_mut_str("filter");
      param.in = OA_PARAM_IN_QUERYSTRING;
      param.content_type = cdd_test_mut_str("application/json");
      param.schema.ref_name = cdd_test_mut_str("FilterRef");
      for (io_i = 0; io_i < 8; ++io_i) {
        g_io_calls = 0;
        g_fail_io_after = io_i;
        codegen_client_write_signature(fp, &op, NULL);
      }
      g_fail_io_after = -1;

      /* hitting header json array of objects */
      param.in = OA_PARAM_IN_HEADER;
      param.is_array = 1;
      param.items_type = cdd_test_mut_str("object");
      param.schema.ref_name = NULL;
      for (io_i = 0; io_i < 8; ++io_i) {
        g_io_calls = 0;
        g_fail_io_after = io_i;
        codegen_client_write_signature(fp, &op, NULL);
      }
      g_fail_io_after = -1;

      /* hitting multipart headers */
      memset(&op, 0, sizeof(op));
      memset(&mt, 0, sizeof(mt));
      memset(&enc, 0, sizeof(enc));
      memset(hdr, 0, sizeof(hdr));
      op.operation_id = cdd_test_mut_str("testMultipartCases");
      op.req_body.content_type = cdd_test_mut_str("multipart/form-data");
      op.n_req_body_media_types = 1;
      op.req_body_media_types = &mt;
      mt.name = cdd_test_mut_str("multipart/form-data");
      mt.n_encoding = 1;
      mt.encoding = &enc;
      enc.name = cdd_test_mut_str("part1");
      enc.n_headers = 2;
      enc.headers = hdr;
      hdr[0].name = cdd_test_mut_str("X-Arr");
      hdr[0].type = cdd_test_mut_str("array");
      hdr[0].is_array = 1;
      hdr[0].items_type = cdd_test_mut_str("string");
      hdr[1].name = cdd_test_mut_str("X-Obj");
      hdr[1].type = cdd_test_mut_str("object");
      for (io_i = 0; io_i < 10; ++io_i) {
        g_io_calls = 0;
        g_fail_io_after = io_i;
        codegen_client_write_signature(fp, &op, NULL);
      }
      g_fail_io_after = -1;

      /* hitting struct array response and binary response */
      memset(&op, 0, sizeof(op));
      memset(resp, 0, sizeof(resp));
      op.operation_id = cdd_test_mut_str("testRespCases");
      op.responses = resp;
      op.n_responses = 1;
      resp[0].code = cdd_test_mut_str("200");
      resp[0].schema.is_array = 1;
      resp[0].schema.ref_name = cdd_test_mut_str("MyRespStruct");
      for (io_i = 0; io_i < 8; ++io_i) {
        g_io_calls = 0;
        g_fail_io_after = io_i;
        codegen_client_write_signature(fp, &op, NULL);
      }
      g_fail_io_after = -1;

      resp[0].schema.is_array = 0;
      resp[0].schema.ref_name = NULL;
      resp[0].content_type = cdd_test_mut_str("application/octet-stream");
      for (io_i = 0; io_i < 8; ++io_i) {
        g_io_calls = 0;
        g_fail_io_after = io_i;
        codegen_client_write_signature(fp, &op, NULL);
      }
      g_fail_io_after = -1;
      /* hitting semicolon config IO error */
      {
        struct CodegenSigConfig cfg_semi;
        memset(&cfg_semi, 0, sizeof(cfg_semi));
        cfg_semi.include_semicolon = 1;
        for (io_i = 0; io_i < 6; ++io_i) {
          g_io_calls = 0;
          g_fail_io_after = io_i;
          codegen_client_write_signature(fp, &op, &cfg_semi);
        }
        g_fail_io_after = -1;
      }

      /* hitting req_body string array, integer array, struct array */
      memset(&op, 0, sizeof(op));
      op.operation_id = cdd_test_mut_str("testReqBodyArrays");
      op.req_body.content_type = cdd_test_mut_str("application/json");
      op.req_body.is_array = 1;
      op.req_body.ref_name = cdd_test_mut_str("string");
      for (io_i = 0; io_i < 8; ++io_i) {
        g_io_calls = 0;
        g_fail_io_after = io_i;
        codegen_client_write_signature(fp, &op, NULL);
      }
      g_fail_io_after = -1;

      op.req_body.ref_name = cdd_test_mut_str("integer");
      for (io_i = 0; io_i < 8; ++io_i) {
        g_io_calls = 0;
        g_fail_io_after = io_i;
        codegen_client_write_signature(fp, &op, NULL);
      }
      g_fail_io_after = -1;

      op.req_body.ref_name = cdd_test_mut_str("MyReqItem");
      for (io_i = 0; io_i < 8; ++io_i) {
        g_io_calls = 0;
        g_fail_io_after = io_i;
        codegen_client_write_signature(fp, &op, NULL);
      }
      g_fail_io_after = -1;

      /* hitting params: form_object, json array of items, path param, header
       * primitive */
      memset(&op, 0, sizeof(op));
      memset(&param, 0, sizeof(param));
      op.operation_id = cdd_test_mut_str("testParamIOs");
      op.n_parameters = 1;
      op.parameters = &param;
      param.name = cdd_test_mut_str("kv");
      param.in = OA_PARAM_IN_QUERYSTRING;
      param.is_array = 0;
      param.type = cdd_test_mut_str("object");
      param.content_type =
          cdd_test_mut_str("application/x-www-form-urlencoded");
      param.schema.ref_name = cdd_test_mut_str("KvRef");
      for (io_i = 0; io_i < 8; ++io_i) {
        g_io_calls = 0;
        g_fail_io_after = io_i;
        codegen_client_write_signature(fp, &op, NULL);
      }
      g_fail_io_after = -1;

      param.in = OA_PARAM_IN_HEADER;
      param.content_type = cdd_test_mut_str("application/json");
      param.is_array = 1;
      param.items_type = cdd_test_mut_str("integer");
      param.schema.ref_name = NULL;
      for (io_i = 0; io_i < 8; ++io_i) {
        g_io_calls = 0;
        g_fail_io_after = io_i;
        codegen_client_write_signature(fp, &op, NULL);
      }
      g_fail_io_after = -1;

      param.in = OA_PARAM_IN_HEADER;
      param.content_type = cdd_test_mut_str("application/json");
      param.is_array = 1;
      param.items_type = cdd_test_mut_str("MyCustomItem");
      param.schema.ref_name = NULL;
      for (io_i = 0; io_i < 8; ++io_i) {
        g_io_calls = 0;
        g_fail_io_after = io_i;
        codegen_client_write_signature(fp, &op, NULL);
      }
      g_fail_io_after = -1;

      param.in = OA_PARAM_IN_HEADER;
      param.content_type = cdd_test_mut_str("application/json");
      param.is_array = 0;
      param.type = cdd_test_mut_str("object");
      param.schema.ref_name = NULL;
      for (io_i = 0; io_i < 8; ++io_i) {
        g_io_calls = 0;
        g_fail_io_after = io_i;
        codegen_client_write_signature(fp, &op, NULL);
      }
      g_fail_io_after = -1;

      param.in = OA_PARAM_IN_HEADER;
      param.content_type = cdd_test_mut_str("application/json");
      param.is_array = 0;
      param.type = cdd_test_mut_str("string");
      param.schema.ref_name = NULL;
      for (io_i = 0; io_i < 8; ++io_i) {
        g_io_calls = 0;
        g_fail_io_after = io_i;
        codegen_client_write_signature(fp, &op, NULL);
      }
      g_fail_io_after = -1;

      param.in = OA_PARAM_IN_HEADER;
      param.is_array = 0;
      param.type = cdd_test_mut_str("string");
      for (io_i = 0; io_i < 8; ++io_i) {
        g_io_calls = 0;
        g_fail_io_after = io_i;
        codegen_client_write_signature(fp, &op, NULL);
      }
      g_fail_io_after = -1;

      param.in = OA_PARAM_IN_HEADER;
      param.content_type = NULL;
      param.is_array = 1;
      param.items_type = cdd_test_mut_str("integer");
      for (io_i = 0; io_i < 8; ++io_i) {
        g_io_calls = 0;
        g_fail_io_after = io_i;
        codegen_client_write_signature(fp, &op, NULL);
      }
      g_fail_io_after = -1;

      fclose(fp);
    }
  }

  PASS();
}

/**
 * @brief Executes the client_sig_coverage_suite test suite.
 */
SUITE(client_sig_coverage_suite) {
  RUN_TEST(test_sig_complete_branches_coverage);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CODEGEN_CLIENT_SIG_COVERAGE_H */
