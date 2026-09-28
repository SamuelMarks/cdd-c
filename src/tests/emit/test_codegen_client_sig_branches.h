/**
 * @file test_codegen_client_sig_branches.h
 * @brief Unit tests for C Client Signature Generation error percolations and
 * branch coverage.
 */

#ifndef TEST_CODEGEN_CLIENT_SIG_BRANCHES_H
#define TEST_CODEGEN_CLIENT_SIG_BRANCHES_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "emit/test_codegen_client_sig_common.h"
/* clang-format on */

TEST test_sig_all_error_percolations_and_branches(void) {
  int int_val = 0;
  size_t sz_val = 0;
  const char *str_val = NULL;
  char buf[64];
  struct OpenAPI_Operation op;
  struct OpenAPI_Parameter param;
  struct OpenAPI_MediaType mt;
  struct OpenAPI_Encoding enc;
  struct OpenAPI_Header hdr;
  char *code = NULL;

  memset(&op, 0, sizeof(op));
  memset(&param, 0, sizeof(param));
  memset(&mt, 0, sizeof(mt));
  memset(&enc, 0, sizeof(enc));
  memset(&hdr, 0, sizeof(hdr));

  /* Helper error injections */
  g_cdd_fail_map_type_to_c_arg = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_sig_map_type_to_c_arg("int", &str_val));
  g_cdd_fail_is_primitive_type = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_sig_is_primitive_type("int", &int_val));
  g_cdd_fail_param_is_object_kv = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_sig_param_is_object_kv(&param, &int_val));
  g_cdd_fail_media_type_base_len = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_sig_media_type_base_len("test", &sz_val));
  g_cdd_fail_media_type_has_prefix = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_sig_media_type_has_prefix("a", "b", &int_val));
  g_cdd_fail_media_type_has_suffix = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_sig_media_type_has_suffix("a", "b", &int_val));
  g_cdd_fail_media_type_ieq = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_sig_media_type_ieq("a", "b", &int_val));
  g_cdd_fail_media_type_is_json = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_sig_media_type_is_json("a", &int_val));
  g_cdd_fail_media_type_is_form = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_sig_media_type_is_form("a", &int_val));
  g_cdd_fail_media_type_is_text_plain = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_sig_media_type_is_text_plain("a", &int_val));
  g_cdd_fail_media_type_is_multipart = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_sig_media_type_is_multipart("a", &int_val));
  g_cdd_fail_media_type_is_multipart_form = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_sig_media_type_is_multipart_form("a", &int_val));
  g_cdd_fail_find_media_type = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_sig_find_media_type(
                &mt, 1, "a", (const struct OpenAPI_MediaType **)&str_val));
  g_cdd_fail_media_type_is_textual = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_sig_media_type_is_textual("a", &int_val));
  g_cdd_fail_media_type_is_binary = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_sig_media_type_is_binary("a", &int_val));
  g_cdd_fail_qs_form_obj = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_sig_querystring_param_is_form_object(&param, &int_val));
  g_cdd_fail_qs_json_ref = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_sig_querystring_param_is_json_ref(&param, &int_val));
  g_cdd_fail_qs_json_prim = 1;
  ASSERT_EQ(
      CDD_C_ERROR_UNKNOWN,
      cdd_test_sig_querystring_param_json_primitive_type(&param, &str_val));
  g_cdd_fail_qs_json_array_item_type = 1;
  ASSERT_EQ(
      CDD_C_ERROR_UNKNOWN,
      cdd_test_sig_querystring_param_json_array_item_type(&param, &str_val));
  g_cdd_fail_qs_json_array_item_ref = 1;
  ASSERT_EQ(
      CDD_C_ERROR_UNKNOWN,
      cdd_test_sig_querystring_param_json_array_item_ref(&param, &str_val));
  g_cdd_fail_qs_raw = 1;
  ASSERT_EQ(
      CDD_C_ERROR_UNKNOWN,
      cdd_test_sig_querystring_param_raw_primitive_type(&param, &str_val));
  g_cdd_fail_map_array_item_type = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_sig_map_array_item_type("int", &str_val));
  g_cdd_fail_header_name_is_content_type = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_sig_header_name_is_content_type("Content-Type", &int_val));
  g_cdd_fail_multipart_header_param_name = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, cdd_test_sig_multipart_header_param_name(
                                     buf, sizeof(buf), "f", "h"));
  g_cdd_fail_map_type_to_c_out = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_sig_map_type_to_c_out("int", &str_val));
  g_cdd_fail_map_array_item_type_out = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_sig_map_array_item_type_out("int", &str_val));
  g_cdd_fail_schema_has_inline = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_sig_schema_has_inline(
                (const struct OpenAPI_SchemaRef *)&param, &int_val));
  g_cdd_fail_get_success_response = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_sig_get_success_response(
                &op, (const struct OpenAPI_Response **)&str_val));
  g_cdd_fail_response_is_binary_success = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_sig_response_is_binary_success(&op, &int_val));
  g_cdd_fail_get_success_schema = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_sig_get_success_schema(
                &op, (const struct OpenAPI_SchemaRef **)&str_val));

  /* Now test internal sub-call failures in helpers */
  g_cdd_fail_media_type_base_len = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_sig_media_type_has_suffix("a+json", "+json", &int_val));
  g_cdd_fail_media_type_base_len = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_sig_media_type_ieq("application/json", "application/json",
                                        &int_val));
  g_cdd_fail_media_type_ieq = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_sig_media_type_is_json("application/json", &int_val));
  g_cdd_fail_media_type_has_suffix = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, cdd_test_sig_media_type_is_json(
                                     "application/vnd.api+json", &int_val));

  g_cdd_fail_media_type_is_text_plain = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_sig_media_type_is_textual("text/plain", &int_val));
  g_cdd_fail_media_type_has_prefix = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_sig_media_type_is_textual("text/html", &int_val));
  g_cdd_fail_media_type_ieq = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_sig_media_type_is_textual("other", &int_val));
  g_cdd_fail_media_type_has_suffix = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_sig_media_type_is_textual("other", &int_val));

  g_cdd_fail_media_type_is_json = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_sig_media_type_is_binary("bin", &int_val));
  g_cdd_fail_media_type_is_form = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_sig_media_type_is_binary("bin", &int_val));
  g_cdd_fail_media_type_is_multipart = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_sig_media_type_is_binary("bin", &int_val));
  g_cdd_fail_media_type_is_textual = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_sig_media_type_is_binary("bin", &int_val));

  memset(&param, 0, sizeof(param));
  param.in = OA_PARAM_IN_QUERYSTRING;
  param.content_type = (char *)(size_t)(size_t) "application/json";
  g_cdd_fail_media_type_is_json = 1;
  ASSERT_EQ(
      CDD_C_ERROR_UNKNOWN,
      cdd_test_sig_querystring_param_json_primitive_type(&param, &str_val));
  g_cdd_fail_media_type_is_json = 1;
  ASSERT_EQ(
      CDD_C_ERROR_UNKNOWN,
      cdd_test_sig_querystring_param_json_array_item_type(&param, &str_val));
  g_cdd_fail_media_type_is_json = 1;
  ASSERT_EQ(
      CDD_C_ERROR_UNKNOWN,
      cdd_test_sig_querystring_param_json_array_item_ref(&param, &str_val));
  g_cdd_fail_media_type_is_json = 1;
  ASSERT_EQ(
      CDD_C_ERROR_UNKNOWN,
      cdd_test_sig_querystring_param_raw_primitive_type(&param, &str_val));
  param.content_type = (char *)(size_t)(size_t) "text/plain";
  g_cdd_fail_media_type_is_form = 1;
  ASSERT_EQ(
      CDD_C_ERROR_UNKNOWN,
      cdd_test_sig_querystring_param_raw_primitive_type(&param, &str_val));

  memset(&op, 0, sizeof(op));
  g_cdd_fail_get_success_response = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_sig_response_is_binary_success(&op, &int_val));
  {
    struct OpenAPI_Response r;
    memset(&r, 0, sizeof(r));
    r.code = (char *)(size_t)(size_t) "200";
    op.responses = &r;
    op.n_responses = 1;
    g_cdd_fail_schema_has_inline = 1;
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
              cdd_test_sig_get_success_schema(
                  &op, (const struct OpenAPI_SchemaRef **)&str_val));
  }

  /* Now test each failure branch in codegen_client_write_signature */
  memset(&op, 0, sizeof(op));
  memset(&param, 0, sizeof(param));
  op.operation_id = (char *)(size_t)(size_t) "failOp";
  op.parameters = &param;
  op.n_parameters = 1;
  param.name = (char *)(size_t)(size_t) "p";
  param.in = OA_PARAM_IN_QUERYSTRING;

  g_cdd_fail_qs_json_array_item_type = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, gen_sig(&op, NULL, &code));
  g_cdd_fail_qs_json_array_item_ref = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, gen_sig(&op, NULL, &code));
  g_cdd_fail_qs_json_prim = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, gen_sig(&op, NULL, &code));
  g_cdd_fail_qs_raw = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, gen_sig(&op, NULL, &code));
  g_cdd_fail_qs_form_obj = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, gen_sig(&op, NULL, &code));
  g_cdd_fail_qs_json_ref = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, gen_sig(&op, NULL, &code));

  /* qs_json_item array type map error */
  param.content_type = (char *)(size_t)(size_t) "application/json";
  param.is_array = 1;
  param.items_type = (char *)(size_t)(size_t) "integer";
  g_cdd_fail_map_array_item_type = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, gen_sig(&op, NULL, &code));

  /* qs_json_prim map type error */
  param.is_array = 0;
  param.items_type = NULL;
  param.type = (char *)(size_t)(size_t) "integer";
  g_cdd_fail_map_type_to_c_arg = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, gen_sig(&op, NULL, &code));

  /* qs_raw map type error */
  param.content_type = (char *)(size_t)(size_t) "text/plain";
  param.type = (char *)(size_t)(size_t) "integer";
  g_cdd_fail_map_type_to_c_arg = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, gen_sig(&op, NULL, &code));

  /* non-querystring param failures */
  param.in = OA_PARAM_IN_HEADER;
  param.content_type = (char *)(size_t)(size_t) "application/json";
  g_cdd_fail_media_type_is_json = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, gen_sig(&op, NULL, &code));

  g_cdd_fail_is_primitive_type = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, gen_sig(&op, NULL, &code));

  param.is_array = 1;
  param.items_type = (char *)(size_t)(size_t) "integer";
  g_cdd_fail_is_primitive_type = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, gen_sig(&op, NULL, &code));

  g_cdd_fail_map_array_item_type = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, gen_sig(&op, NULL, &code));

  param.is_array = 0;
  param.items_type = NULL;
  param.type = (char *)(size_t)(size_t) "string";
  g_cdd_fail_map_type_to_c_arg = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, gen_sig(&op, NULL, &code));

  param.content_type = NULL;
  g_cdd_fail_param_is_object_kv = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, gen_sig(&op, NULL, &code));

  param.is_array = 1;
  param.items_type = (char *)(size_t)(size_t) "integer";
  g_cdd_fail_map_array_item_type = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, gen_sig(&op, NULL, &code));

  param.is_array = 0;
  param.items_type = NULL;
  param.type = (char *)(size_t)(size_t) "integer";
  g_cdd_fail_map_type_to_c_arg = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, gen_sig(&op, NULL, &code));

  /* Request body failures */
  op.n_parameters = 0;
  op.req_body.content_type = (char *)(size_t)(size_t) "application/json";
  g_cdd_fail_media_type_is_binary = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, gen_sig(&op, NULL, &code));

  g_cdd_fail_media_type_is_multipart = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, gen_sig(&op, NULL, &code));

  g_cdd_fail_media_type_is_multipart_form = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, gen_sig(&op, NULL, &code));

  g_cdd_fail_media_type_is_textual = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, gen_sig(&op, NULL, &code));

  op.req_body.inline_type = (char *)(size_t)(size_t) "string";
  op.req_body.is_array = 1;
  g_cdd_fail_map_array_item_type = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, gen_sig(&op, NULL, &code));

  op.req_body.is_array = 0;
  g_cdd_fail_map_type_to_c_arg = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, gen_sig(&op, NULL, &code));

  /* Multipart per-part failures */
  op.req_body.content_type = (char *)(size_t)(size_t) "multipart/form-data";
  op.req_body.inline_type = NULL;
  memset(&mt, 0, sizeof(mt));
  memset(&enc, 0, sizeof(enc));
  memset(&hdr, 0, sizeof(hdr));
  op.n_req_body_media_types = 1;
  op.req_body_media_types = &mt;
  mt.name = (char *)(size_t)(size_t) "multipart/form-data";
  mt.n_encoding = 1;
  mt.encoding = &enc;
  enc.name = (char *)(size_t)(size_t) "field";
  enc.n_headers = 1;
  enc.headers = &hdr;
  hdr.name = (char *)(size_t)(size_t) "X-Header";

  g_cdd_fail_media_type_is_multipart_form = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, gen_sig(&op, NULL, &code));

  g_cdd_fail_find_media_type = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, gen_sig(&op, NULL, &code));

  g_cdd_fail_header_name_is_content_type = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, gen_sig(&op, NULL, &code));

  g_cdd_fail_multipart_header_param_name = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, gen_sig(&op, NULL, &code));

  hdr.is_array = 1;
  hdr.items_type = (char *)(size_t)(size_t) "string";
  g_cdd_fail_map_array_item_type = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, gen_sig(&op, NULL, &code));

  hdr.is_array = 0;
  hdr.items_type = NULL;
  hdr.type = (char *)(size_t)(size_t) "integer";
  g_cdd_fail_map_type_to_c_arg = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, gen_sig(&op, NULL, &code));

  /* Success output failures */
  op.req_body.content_type = NULL;
  op.n_req_body_media_types = 0;
  g_cdd_fail_response_is_binary_success = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, gen_sig(&op, NULL, &code));

  g_cdd_fail_get_success_schema = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, gen_sig(&op, NULL, &code));

  {
    struct OpenAPI_Response r;
    memset(&r, 0, sizeof(r));
    r.code = (char *)(size_t)(size_t) "200";
    r.schema.inline_type = (char *)(size_t)(size_t) "string";
    r.schema.is_array = 1;
    op.responses = &r;
    op.n_responses = 1;

    g_cdd_fail_schema_has_inline = 1;
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, gen_sig(&op, NULL, &code));

    g_cdd_fail_map_array_item_type_out = 1;
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, gen_sig(&op, NULL, &code));

    r.schema.is_array = 0;
    g_cdd_fail_map_type_to_c_out = 1;
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, gen_sig(&op, NULL, &code));
  }

  PASS();
}

SUITE(client_sig_branches_suite) {
  RUN_TEST(test_sig_all_error_percolations_and_branches);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CODEGEN_CLIENT_SIG_BRANCHES_H */
