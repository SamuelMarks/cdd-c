/**
 * @file test_codegen_client_sig_ultra.h
 * @brief Unit tests for C Client Signature Generation ultra edge coverage.
 */

#ifndef TEST_CODEGEN_CLIENT_SIG_ULTRA_H
#define TEST_CODEGEN_CLIENT_SIG_ULTRA_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "emit/test_codegen_client_sig_common.h"
/* clang-format on */

TEST test_sig_ultra_coverage(void) {
  struct OpenAPI_Operation op;
  struct OpenAPI_Parameter param;
  struct OpenAPI_Response resp[3];
  const struct OpenAPI_SchemaRef *out_schema = NULL;
  const char *out_schema_str = NULL;
  int int_val = 0;
  char buf[64];
  char *code = NULL;

  /* sanitize_ident branch tests */
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_sanitize_ident(buf, sizeof(buf), ""));
  ASSERT_STR_EQ("", buf);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_sanitize_ident(buf, sizeof(buf), "[abc]"));
  ASSERT_STR_EQ("_abc_", buf);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_sanitize_ident(buf, sizeof(buf), "abc"));
  ASSERT_STR_EQ("abc", buf);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_sig_sanitize_ident(buf, sizeof(buf), "_abc"));
  ASSERT_STR_EQ("_abc", buf);

  /* find_media_type with NULL entry name */
  {
    struct OpenAPI_MediaType mts[2];
    const struct OpenAPI_MediaType *found = NULL;
    memset(mts, 0, sizeof(mts));
    mts[0].name = NULL;
    mts[1].name = (char *)(size_t)(size_t) "text/plain";
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_sig_find_media_type(mts, 2, "text/plain", &found));
    ASSERT(found == &mts[1]);
  }

  /* querystring_param_raw_primitive_type with NULL content_type */
  memset(&param, 0, sizeof(param));
  param.in = OA_PARAM_IN_QUERYSTRING;
  param.content_type = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_querystring_param_raw_primitive_type(
                               &param, &out_schema_str));
  ASSERT(out_schema_str == NULL);

  /* param_is_object_kv with !is_array and type == NULL */
  memset(&param, 0, sizeof(param));
  param.is_array = 0;
  param.type = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_param_is_object_kv(&param, &int_val));
  ASSERT_EQ(0, int_val);

  /* querystring_param_json_array_item_ref with "number" and "boolean" */
  memset(&param, 0, sizeof(param));
  param.in = OA_PARAM_IN_QUERYSTRING;
  param.content_type = (char *)(size_t)(size_t) "application/json";
  param.is_array = 1;
  param.items_type = (char *)(size_t)(size_t) "number";
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_querystring_param_json_array_item_ref(
                               &param, &out_schema_str));
  ASSERT(out_schema_str == NULL);
  param.items_type = (char *)(size_t)(size_t) "boolean";
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_querystring_param_json_array_item_ref(
                               &param, &out_schema_str));
  ASSERT(out_schema_str == NULL);

  /* response codes "20X" and "2XY" to test branch in strlen == 3 && c[0]=='2'
   * && c[1]=='X' && c[2]=='X' */
  memset(&op, 0, sizeof(op));
  memset(resp, 0, sizeof(resp));
  op.responses = resp;
  op.n_responses = 2;
  resp[0].code = (char *)(size_t)(size_t) "20X";
  resp[1].code = (char *)(size_t)(size_t) "2XY";
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_get_success_schema(&op, &out_schema));
  {
    const struct OpenAPI_Response *r_out = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_get_success_response(&op, &r_out));
    ASSERT(r_out == &resp[0]);
  }

  /* schema_has_inline and is_array branches in get_success_schema */
  resp[0].code = (char *)(size_t)(size_t) "200";
  resp[0].schema.inline_type = (char *)(size_t)(size_t) "string";
  resp[0].schema.is_array = 0;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_get_success_schema(&op, &out_schema));
  ASSERT(out_schema == &resp[0].schema);

  resp[0].schema.inline_type = NULL;
  resp[0].schema.is_array = 1;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_get_success_schema(&op, &out_schema));
  ASSERT(out_schema == &resp[0].schema);

  /* default_resp with inline_type, and with is_array */
  resp[0].code = (char *)(size_t)(size_t) "default";
  resp[0].schema.inline_type = (char *)(size_t)(size_t) "string";
  resp[0].schema.is_array = 0;
  op.n_responses = 1;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_get_success_schema(&op, &out_schema));
  ASSERT(out_schema == &resp[0].schema);

  resp[0].schema.inline_type = NULL;
  resp[0].schema.is_array = 1;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_get_success_schema(&op, &out_schema));
  ASSERT(out_schema == &resp[0].schema);

  /* header JSON param variations */
  memset(&op, 0, sizeof(op));
  memset(&param, 0, sizeof(param));
  op.operation_id = (char *)(size_t)(size_t) "testVariations";
  op.n_parameters = 1;
  op.parameters = &param;
  param.name = (char *)(size_t)(size_t) "p";
  param.in = OA_PARAM_IN_HEADER;
  param.content_type = (char *)(size_t)(size_t) "application/json";

  /* items_type NULL, inline_type set */
  param.is_array = 1;
  param.items_type = NULL;
  param.schema.inline_type = (char *)(size_t)(size_t) "integer";
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  ASSERT(code != NULL);
  free(code);
  code = NULL;

  /* items_type NULL, inline_type NULL (void* array) */
  param.schema.inline_type = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  ASSERT(code != NULL);
  free(code);
  code = NULL;

  /* is_array = 0, p.type NULL, schema.inline_type set */
  param.is_array = 0;
  param.type = NULL;
  param.schema.inline_type = (char *)(size_t)(size_t) "integer";
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  ASSERT(code != NULL);
  free(code);
  code = NULL;

  /* is_array = 0, p.type NULL, schema.inline_type NULL ("string" fallback) */
  param.schema.inline_type = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, NULL, &code));
  ASSERT(code != NULL);
  free(code);
  code = NULL;

  /* config with prefix == NULL, ctx_type == NULL, group_name == NULL */
  {
    struct CodegenSigConfig cfg_empty;
    memset(&cfg_empty, 0, sizeof(cfg_empty));
    cfg_empty.include_semicolon = 1;
    ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op, &cfg_empty, &code));
    ASSERT(code != NULL);
    free(code);
    code = NULL;
  }

  /* op with no responses and req_body with is_array = 1 but no ref_name/inline
   * (hits line 1600 success_schema->is_array true when ref_name and inline are
   * false) */
  {
    struct OpenAPI_Operation op_no_resp;
    memset(&op_no_resp, 0, sizeof(op_no_resp));
    op_no_resp.operation_id = (char *)(size_t)(size_t) "noResp";
    op_no_resp.req_body.is_array = 1;
    ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op_no_resp, NULL, &code));
    ASSERT(code != NULL);
    free(code);
    code = NULL;
  }

  /* Multipart with enc.name == NULL and with enc.n_headers == 0 and
   * hdr.items_type == NULL */
  {
    struct OpenAPI_Operation op_mp;
    struct OpenAPI_MediaType mt_mp;
    struct OpenAPI_Encoding enc_mp[2];
    struct OpenAPI_Header hdr_mp;
    memset(&op_mp, 0, sizeof(op_mp));
    memset(&mt_mp, 0, sizeof(mt_mp));
    memset(enc_mp, 0, sizeof(enc_mp));
    memset(&hdr_mp, 0, sizeof(hdr_mp));
    op_mp.operation_id = (char *)(size_t)(size_t) "mpEdge";
    op_mp.req_body.content_type =
        (char *)(size_t)(size_t) "multipart/form-data";
    op_mp.n_req_body_media_types = 1;
    op_mp.req_body_media_types = &mt_mp;
    mt_mp.name = (char *)(size_t)(size_t) "multipart/form-data";
    mt_mp.n_encoding = 2;
    mt_mp.encoding = enc_mp;
    enc_mp[0].name = NULL; /* enc->name == NULL branch */
    enc_mp[1].name = (char *)(size_t)(size_t) "part";
    enc_mp[1].headers = &hdr_mp;
    enc_mp[1].n_headers = 0; /* enc->n_headers == 0 branch */
    ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op_mp, NULL, &code));
    ASSERT(code != NULL);
    free(code);
    code = NULL;

    /* hdr.items_type == NULL in array */
    enc_mp[1].n_headers = 1;
    hdr_mp.name = (char *)(size_t)(size_t) "X-Arr-Def";
    hdr_mp.is_array = 1;
    hdr_mp.items_type = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op_mp, NULL, &code));
    ASSERT(code != NULL);
    free(code);
    code = NULL;
  }

  /* sanitize_ident small buffer where j + 1 >= outsz and leading char is digit
   */
  {
    char tiny[2];
    ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_sanitize_ident(tiny, 2, "9"));
    ASSERT_STR_EQ("_", tiny);
    /* buffer of size 3 where j+1 == outsz (j=2, outsz=3) and first char is
     * digit */
    {
      char med[3];
      ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_sanitize_ident(med, 3, "9a"));
      ASSERT_STR_EQ("_a", med);
    }
    {
      char small_non_digit[3];
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_test_sig_sanitize_ident(small_non_digit, 3, "abcdef"));
      ASSERT_STR_EQ("ab", small_non_digit);
    }
    {
      char tilde_buf[16];
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_test_sig_sanitize_ident(tilde_buf, sizeof(tilde_buf),
                                            "foo~bar{baz}"));
      ASSERT_STR_EQ("foo_bar_baz_", tilde_buf);
    }
  }

  /* get_success_response and get_success_schema with 3-char code "201" (first
   * char '2', len 3) */
  {
    struct OpenAPI_Operation op_201;
    struct OpenAPI_Response resp_201;
    const struct OpenAPI_Response *r_out = NULL;
    const struct OpenAPI_SchemaRef *s_out = NULL;
    memset(&op_201, 0, sizeof(op_201));
    memset(&resp_201, 0, sizeof(resp_201));
    op_201.n_responses = 1;
    op_201.responses = &resp_201;
    resp_201.code = (char *)(size_t)(size_t) "201";
    resp_201.schema.ref_name = (char *)(size_t)(size_t) "Item";
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_sig_get_success_response(&op_201, &r_out));
    ASSERT(r_out == &resp_201);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_get_success_schema(&op_201, &s_out));
    ASSERT(s_out == &resp_201.schema);

    /* 2XX with no ref_name, no inline, no is_array (continue branch of 2XX) */
    resp_201.code = (char *)(size_t)(size_t) "2XX";
    resp_201.schema.ref_name = NULL;
    s_out = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_get_success_schema(&op_201, &s_out));
    ASSERT(s_out == &op_201.req_body);

    /* 201 with no ref_name, no inline, no is_array (false branch of if(ref_name
     * || inline || is_array) for c[0]=='2') */
    resp_201.code = (char *)(size_t)(size_t) "201";
    s_out = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_get_success_schema(&op_201, &s_out));
    ASSERT(s_out == &op_201.req_body);
  }

  /* get_success_schema with code starting with '2' but length != 3, e.g. "2" */
  {
    struct OpenAPI_Operation op_2;
    struct OpenAPI_Response resp_2;
    const struct OpenAPI_SchemaRef *s_out = NULL;
    memset(&op_2, 0, sizeof(op_2));
    memset(&resp_2, 0, sizeof(resp_2));
    op_2.n_responses = 1;
    op_2.responses = &resp_2;
    resp_2.code = (char *)(size_t)(size_t) "2";
    resp_2.schema.ref_name = (char *)(size_t)(size_t) "Item";
    ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_get_success_schema(&op_2, &s_out));
    ASSERT(s_out == &resp_2.schema);
  }

  /* querystring_param_raw_primitive_type with p->in != OA_PARAM_IN_QUERYSTRING
   */
  {
    struct OpenAPI_Parameter param_hdr;
    const char *raw_type = NULL;
    memset(&param_hdr, 0, sizeof(param_hdr));
    param_hdr.in = OA_PARAM_IN_HEADER;
    param_hdr.content_type = (char *)(size_t)(size_t) "application/json";
    ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_querystring_param_raw_primitive_type(
                                 &param_hdr, &raw_type));
    ASSERT(raw_type == NULL);
  }

  /* codegen_client_write_signature with config != NULL, but config->ctx_type ==
   * NULL */
  {
    struct OpenAPI_Operation op_ctx;
    struct CodegenSigConfig cfg_ctx;
    memset(&op_ctx, 0, sizeof(op_ctx));
    memset(&cfg_ctx, 0, sizeof(cfg_ctx));
    op_ctx.operation_id = (char *)(size_t)(size_t) "testDefaultCtx";
    cfg_ctx.ctx_type = NULL;
    cfg_ctx.prefix = "api_";
    ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op_ctx, &cfg_ctx, &code));
    ASSERT(code != NULL);
    free(code);
    code = NULL;
  }

  /* multipart req_body with is_mp && !is_mp_form (e.g. multipart/mixed) */
  {
    struct OpenAPI_Operation op_mixed;
    memset(&op_mixed, 0, sizeof(op_mixed));
    op_mixed.operation_id = (char *)(size_t)(size_t) "testMixed";
    op_mixed.req_body.content_type = (char *)(size_t)(size_t) "multipart/mixed";
    ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op_mixed, NULL, &code));
    ASSERT(code != NULL);
    free(code);
    code = NULL;
  }

  /* req_body with is_mp_form and encoding != NULL, n_encoding > 0, hdr_type ==
   * "array" */
  {
    struct OpenAPI_Operation op_mp_hdr;
    struct OpenAPI_MediaType mt_hdr;
    struct OpenAPI_Encoding enc_hdr;
    struct OpenAPI_Header h_hdr;
    memset(&op_mp_hdr, 0, sizeof(op_mp_hdr));
    memset(&mt_hdr, 0, sizeof(mt_hdr));
    memset(&enc_hdr, 0, sizeof(enc_hdr));
    memset(&h_hdr, 0, sizeof(h_hdr));
    op_mp_hdr.operation_id = (char *)(size_t)(size_t) "testMpHdrTypeArray";
    op_mp_hdr.req_body.content_type =
        (char *)(size_t)(size_t) "multipart/form-data";
    op_mp_hdr.n_req_body_media_types = 1;
    op_mp_hdr.req_body_media_types = &mt_hdr;
    mt_hdr.name = (char *)(size_t)(size_t) "multipart/form-data";
    mt_hdr.n_encoding = 1;
    mt_hdr.encoding = &enc_hdr;
    enc_hdr.name = (char *)(size_t)(size_t) "part";
    enc_hdr.n_headers = 1;
    enc_hdr.headers = &h_hdr;
    h_hdr.name = (char *)(size_t)(size_t) "X-Arr-Hdr";
    h_hdr.type = (char *)(size_t)(size_t) "array";
    h_hdr.is_array = 0; /* is_array == 0 but type == "array" */
    h_hdr.items_type = (char *)(size_t)(size_t) "string";
    ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op_mp_hdr, NULL, &code));
    ASSERT(code != NULL);
    free(code);
    code = NULL;

    /* mt->encoding == NULL branch */
    mt_hdr.encoding = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op_mp_hdr, NULL, &code));
    ASSERT(code != NULL);
    free(code);
    code = NULL;

    /* mt == NULL branch (media type not found) */
    op_mp_hdr.n_req_body_media_types = 0;
    op_mp_hdr.req_body_media_types = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op_mp_hdr, NULL, &code));
    ASSERT(code != NULL);
    free(code);
    code = NULL;

    /* mt->n_encoding == 0 branch */
    mt_hdr.encoding = &enc_hdr;
    mt_hdr.n_encoding = 0;
    ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op_mp_hdr, NULL, &code));
    ASSERT(code != NULL);
    free(code);
    code = NULL;
  }

  /* success_schema with inline_type (primitive) and is_array == 1, and is_array
   * == 0 */
  {
    struct OpenAPI_Operation op_inl;
    struct OpenAPI_Response resp_inl;
    memset(&op_inl, 0, sizeof(op_inl));
    memset(&resp_inl, 0, sizeof(resp_inl));
    op_inl.operation_id = (char *)(size_t)(size_t) "testInlineResp";
    op_inl.n_responses = 1;
    op_inl.responses = &resp_inl;
    resp_inl.code = (char *)(size_t)(size_t) "200";
    resp_inl.schema.inline_type = (char *)(size_t)(size_t) "integer";
    resp_inl.schema.is_array = 1;
    resp_inl.schema.ref_name = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op_inl, NULL, &code));
    ASSERT(code != NULL);
    free(code);
    code = NULL;

    resp_inl.schema.is_array = 0;
    ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op_inl, NULL, &code));
    ASSERT(code != NULL);
    free(code);
    code = NULL;
  }

  /* success_schema == NULL but success_is_binary == 1 */
  {
    struct OpenAPI_Operation op_bin_resp;
    struct OpenAPI_Response resp_bin;
    memset(&op_bin_resp, 0, sizeof(op_bin_resp));
    memset(&resp_bin, 0, sizeof(resp_bin));
    op_bin_resp.operation_id =
        (char *)(size_t)(size_t) "testBinaryRespNoSchema";
    op_bin_resp.n_responses = 1;
    op_bin_resp.responses = &resp_bin;
    resp_bin.code = (char *)(size_t)(size_t) "200";
    resp_bin.content_type = (char *)(size_t)(size_t) "application/octet-stream";
    /* Clear req_body schema so fallback schema has no ref_name, no inline, no
     * is_array */
    memset(&op_bin_resp.req_body, 0, sizeof(op_bin_resp.req_body));
    ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op_bin_resp, NULL, &code));
    ASSERT(code != NULL);
    ASSERT(strstr(code, "unsigned char **out, size_t *out_len") != NULL);
    free(code);
    code = NULL;
  }

  /* 2XX response with schema.inline_type != NULL (hits has_inline in 2XX check)
   */
  {
    struct OpenAPI_Operation op_2xx_inl;
    struct OpenAPI_Response resp_2xx_inl;
    const struct OpenAPI_SchemaRef *s_out = NULL;
    memset(&op_2xx_inl, 0, sizeof(op_2xx_inl));
    memset(&resp_2xx_inl, 0, sizeof(resp_2xx_inl));
    op_2xx_inl.n_responses = 1;
    op_2xx_inl.responses = &resp_2xx_inl;
    resp_2xx_inl.code = (char *)(size_t)(size_t) "2XX";
    resp_2xx_inl.schema.inline_type = (char *)(size_t)(size_t) "integer";
    resp_2xx_inl.schema.ref_name = NULL;
    resp_2xx_inl.schema.is_array = 0;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_sig_get_success_schema(&op_2xx_inl, &s_out));
    ASSERT(s_out == &resp_2xx_inl.schema);

    /* 2XX response with schema.is_array = 1, ref_name = NULL, inline_type =
     * NULL */
    resp_2xx_inl.schema.inline_type = NULL;
    resp_2xx_inl.schema.is_array = 1;
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_sig_get_success_schema(&op_2xx_inl, &s_out));
    ASSERT(s_out == &resp_2xx_inl.schema);
  }

  /* get_success_response with 3-char code having c[0] == '2' but c[1] == 'X'
   * and c[2] != 'X' (e.g. "2X0") */
  {
    struct OpenAPI_Operation op_2x0;
    struct OpenAPI_Response resp_2x0;
    const struct OpenAPI_Response *r_out = NULL;
    memset(&op_2x0, 0, sizeof(op_2x0));
    memset(&resp_2x0, 0, sizeof(resp_2x0));
    op_2x0.n_responses = 1;
    op_2x0.responses = &resp_2x0;
    resp_2x0.code = (char *)(size_t)(size_t) "2X0";
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_sig_get_success_response(&op_2x0, &r_out));
    ASSERT(r_out == &resp_2x0);
  }

  /* get_success_response with code of length != 3, e.g. "2" */
  {
    struct OpenAPI_Operation op_len2;
    struct OpenAPI_Response resp_len2;
    const struct OpenAPI_Response *r_out = NULL;
    memset(&op_len2, 0, sizeof(op_len2));
    memset(&resp_len2, 0, sizeof(resp_len2));
    op_len2.n_responses = 1;
    op_len2.responses = &resp_len2;
    resp_len2.code = (char *)(size_t)(size_t) "2";
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_test_sig_get_success_response(&op_len2, &r_out));
    ASSERT(r_out == &resp_len2);
  }

  /* get_success_schema with code of length 3 having c[0] == '2', c[1] == '0',
   * c[2] == '0' */
  {
    struct OpenAPI_Operation op_200;
    struct OpenAPI_Response resp_200;
    const struct OpenAPI_SchemaRef *s_out = NULL;
    memset(&op_200, 0, sizeof(op_200));
    memset(&resp_200, 0, sizeof(resp_200));
    op_200.n_responses = 1;
    op_200.responses = &resp_200;
    resp_200.code = (char *)(size_t)(size_t) "200";
    resp_200.schema.ref_name = (char *)(size_t)(size_t) "Item";
    ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_get_success_schema(&op_200, &s_out));
    ASSERT(s_out == &resp_200.schema);
  }

  /* get_success_schema and get_success_response with 3-char code "2X0" */
  {
    struct OpenAPI_Operation op_2x0s;
    struct OpenAPI_Response resp_2x0s;
    const struct OpenAPI_SchemaRef *s_out = NULL;
    memset(&op_2x0s, 0, sizeof(op_2x0s));
    memset(&resp_2x0s, 0, sizeof(resp_2x0s));
    op_2x0s.n_responses = 1;
    op_2x0s.responses = &resp_2x0s;
    resp_2x0s.code = (char *)(size_t)(size_t) "2X0";
    resp_2x0s.schema.ref_name = (char *)(size_t)(size_t) "Item";
    ASSERT_EQ(CDD_C_SUCCESS, cdd_test_sig_get_success_schema(&op_2x0s, &s_out));
    ASSERT(s_out == &resp_2x0s.schema);
  }

  /* config with config->ctx_type set */
  {
    struct OpenAPI_Operation op_ctx2;
    struct CodegenSigConfig cfg_ctx2;
    memset(&op_ctx2, 0, sizeof(op_ctx2));
    memset(&cfg_ctx2, 0, sizeof(cfg_ctx2));
    op_ctx2.operation_id = (char *)(size_t)(size_t) "testCustomCtx";
    cfg_ctx2.ctx_type = "void *";
    ASSERT_EQ(CDD_C_SUCCESS, gen_sig(&op_ctx2, &cfg_ctx2, &code));
    ASSERT(code != NULL);
    ASSERT(strstr(code, "void *ctx") != NULL);
    free(code);
    code = NULL;
  }

  PASS();
}

SUITE(client_sig_ultra_suite) { RUN_TEST(test_sig_ultra_coverage); }

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CODEGEN_CLIENT_SIG_ULTRA_H */
