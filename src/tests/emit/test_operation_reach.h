/**
 * @file test_operation_reach.h
 * @brief Reachability tests for operation generator.
 */

#ifndef TEST_OPERATION_REACH_H
#define TEST_OPERATION_REACH_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "emit/test_operation_common.h"
/* clang-format on */

TEST test_operation_reach_100_percent(void) {
  /* 1-3. is_reserved_header_name stricmp fails */
  {
    int is_res = 0;
    g_cdd_fail_stricmp = 1;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              is_reserved_header_name("foo", &is_res));
    g_cdd_fail_stricmp = 2;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              is_reserved_header_name("foo", &is_res));
    g_cdd_fail_stricmp = 3;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              is_reserved_header_name("foo", &is_res));
    g_cdd_fail_stricmp = 0;
  }

  /* 4. find_response stricmp fail */
  {
    struct OpenAPI_Operation op;
    struct OpenAPI_Response r;
    struct OpenAPI_Response *out = NULL;
    memset(&op, 0, sizeof(op));
    memset(&r, 0, sizeof(r));
    r.code = (char *)(size_t) "200";
    op.responses = &r;
    op.n_responses = 1;
    g_cdd_fail_stricmp = 1;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              find_response_by_code(&op, "200", &out));
    g_cdd_fail_stricmp = 0;
  }

  /* 5. apply_example_to_response find_media_type_op fail */
  {
    struct OpenAPI_Response resp;
    struct OpenAPI_MediaType mt;
    memset(&resp, 0, sizeof(resp));
    memset(&mt, 0, sizeof(mt));
    mt.name = (char *)(size_t) "application/json";
    resp.content_media_types = &mt;
    resp.n_content_media_types = 1;
    g_op_fail_find_media_type_op = 1;
    ASSERT_EQ(
        CDD_C_ERROR_INVALID_ARGUMENT,
        apply_example_to_response(&resp, "{\"a\":1}", "application/json"));
    g_op_fail_find_media_type_op = 0;
  }

  /* 6. ensure_response_for_code find_response_by_code fail */
  {
    struct OpenAPI_Operation op;
    struct OpenAPI_Response r;
    struct OpenAPI_Response *out = NULL;
    memset(&op, 0, sizeof(op));
    memset(&r, 0, sizeof(r));
    r.code = (char *)(size_t) "200";
    op.responses = &r;
    op.n_responses = 1;
    g_cdd_fail_stricmp = 1;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              ensure_response_for_code(&op, "200", &out));
    g_cdd_fail_stricmp = 0;
  }

  /* 7. ensure_response_for_code stricmp 200 fail */
  {
    struct OpenAPI_Operation op;
    struct OpenAPI_Response *out = NULL;
    memset(&op, 0, sizeof(op));
    g_cdd_fail_stricmp = 1;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              ensure_response_for_code(&op, "200", &out));
    g_cdd_fail_stricmp = 0;
    if (op.responses)
      free(op.responses);
  }

  /* 8. add_header_to_response stricmp fail */
  {
    struct OpenAPI_Response resp;
    struct OpenAPI_Header hdr;
    struct DocResponseHeader dh;
    memset(&resp, 0, sizeof(resp));
    memset(&hdr, 0, sizeof(hdr));
    memset(&dh, 0, sizeof(dh));
    hdr.name = (char *)(size_t) "X-Test";
    resp.headers = &hdr;
    resp.n_headers = 1;
    dh.name = (char *)(size_t) "X-Test";
    g_cdd_fail_stricmp = 1;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, add_header_to_response(&resp, &dh));
    g_cdd_fail_stricmp = 0;
  }

  /* 9. apply_format_to_schema_ref oa_type_is_primitive fail */
  {
    struct OpenAPI_SchemaRef schema;
    struct OpenApiTypeMapping map;
    int applied = 0;
    memset(&schema, 0, sizeof(schema));
    memset(&map, 0, sizeof(map));
    map.oa_format = (char *)(size_t) "int32";
    map.oa_type = (char *)(size_t) "integer";
    g_op_fail_oa_type_is_primitive = 1;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              apply_format_to_schema_ref(&schema, &map, NULL, &applied));
    g_op_fail_oa_type_is_primitive = 0;
  }

  /* 10. c_mapping_init fail in c2openapi_build_operation */
  {
    struct OpBuilderContext ctx;
    struct C2OpenAPI_ParsedSig sig;
    struct C2OpenAPI_ParsedArg arg;
    struct OpenAPI_Operation op;
    memset(&ctx, 0, sizeof(ctx));
    memset(&sig, 0, sizeof(sig));
    memset(&arg, 0, sizeof(arg));
    memset(&op, 0, sizeof(op));
    arg.name = (char *)(size_t) "x";
    arg.type = (char *)(size_t) "int";
    sig.args = &arg;
    sig.n_args = 1;
    ctx.sig = &sig;
    ctx.func_name = "test_fn";
    g_mapping_fail_init = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, c2openapi_build_operation(&ctx, &op));
    g_mapping_fail_init = 0;
    reset_operation_test(&op);
  }

  /* 11. find_doc_param fail in c2openapi_build_operation */
  {
    struct OpBuilderContext ctx;
    struct C2OpenAPI_ParsedSig sig;
    struct C2OpenAPI_ParsedArg arg;
    struct OpenAPI_Operation op;
    memset(&ctx, 0, sizeof(ctx));
    memset(&sig, 0, sizeof(sig));
    memset(&arg, 0, sizeof(arg));
    memset(&op, 0, sizeof(op));
    arg.name = (char *)(size_t) "x";
    arg.type = (char *)(size_t) "int";
    sig.args = &arg;
    sig.n_args = 1;
    ctx.sig = &sig;
    ctx.func_name = "test_fn";
    g_op_fail_find_doc_param = 1;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              c2openapi_build_operation(&ctx, &op));
    g_op_fail_find_doc_param = 0;
    reset_operation_test(&op);
  }

  /* 12. is_path_param fail in c2openapi_build_operation */
  {
    struct OpBuilderContext ctx;
    struct C2OpenAPI_ParsedSig sig;
    struct C2OpenAPI_ParsedArg arg;
    struct DocMetadata doc;
    struct OpenAPI_Operation op;
    memset(&ctx, 0, sizeof(ctx));
    memset(&sig, 0, sizeof(sig));
    memset(&arg, 0, sizeof(arg));
    memset(&doc, 0, sizeof(doc));
    memset(&op, 0, sizeof(op));
    doc.route = (char *)(size_t) "/items/{x}";
    arg.name = (char *)(size_t) "x";
    arg.type = (char *)(size_t) "int";
    sig.args = &arg;
    sig.n_args = 1;
    ctx.sig = &sig;
    ctx.doc = &doc;
    ctx.func_name = "test_fn";
    g_op_fail_is_path_param = 1;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              c2openapi_build_operation(&ctx, &op));
    g_op_fail_is_path_param = 0;
    reset_operation_test(&op);
  }

  /* 13. is_struct_pointer fail in c2openapi_build_operation */
  {
    struct OpBuilderContext ctx;
    struct C2OpenAPI_ParsedSig sig;
    struct C2OpenAPI_ParsedArg arg;
    struct OpenAPI_Operation op;
    memset(&ctx, 0, sizeof(ctx));
    memset(&sig, 0, sizeof(sig));
    memset(&arg, 0, sizeof(arg));
    memset(&op, 0, sizeof(op));
    arg.name = (char *)(size_t) "x";
    arg.type = (char *)(size_t) "struct Foo *";
    sig.args = &arg;
    sig.n_args = 1;
    ctx.sig = &sig;
    ctx.func_name = "test_fn";
    g_op_fail_is_struct_pointer = 1;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              c2openapi_build_operation(&ctx, &op));
    g_op_fail_is_struct_pointer = 0;
    reset_operation_test(&op);
  }

  /* 14. is_reserved_header_name fail in c2openapi_build_operation */
  {
    struct OpBuilderContext ctx;
    struct C2OpenAPI_ParsedSig sig;
    struct C2OpenAPI_ParsedArg arg;
    struct DocMetadata doc;
    struct DocParam dp;
    struct OpenAPI_Operation op;
    memset(&ctx, 0, sizeof(ctx));
    memset(&sig, 0, sizeof(sig));
    memset(&arg, 0, sizeof(arg));
    memset(&doc, 0, sizeof(doc));
    memset(&dp, 0, sizeof(dp));
    memset(&op, 0, sizeof(op));
    dp.name = (char *)(size_t) "hdr";
    dp.in_loc = (char *)(size_t) "header";
    doc.params = &dp;
    doc.n_params = 1;
    arg.name = (char *)(size_t) "hdr";
    arg.type = (char *)(size_t) "char *";
    sig.args = &arg;
    sig.n_args = 1;
    ctx.sig = &sig;
    ctx.doc = &doc;
    ctx.func_name = "test_fn";
    g_cdd_fail_stricmp = 1;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              c2openapi_build_operation(&ctx, &op));
    g_cdd_fail_stricmp = 0;
    reset_operation_test(&op);
  }

  /* 15. doc_style_to_openapi fail in c2openapi_build_operation parameter */
  {
    struct OpBuilderContext ctx;
    struct C2OpenAPI_ParsedSig sig;
    struct C2OpenAPI_ParsedArg arg;
    struct DocMetadata doc;
    struct DocParam dp;
    struct OpenAPI_Operation op;
    memset(&ctx, 0, sizeof(ctx));
    memset(&sig, 0, sizeof(sig));
    memset(&arg, 0, sizeof(arg));
    memset(&doc, 0, sizeof(doc));
    memset(&dp, 0, sizeof(dp));
    memset(&op, 0, sizeof(op));
    dp.name = (char *)(size_t) "p";
    dp.style_set = 1;
    dp.style = DOC_PARAM_STYLE_FORM;
    doc.params = &dp;
    doc.n_params = 1;
    arg.name = (char *)(size_t) "p";
    arg.type = (char *)(size_t) "int";
    sig.args = &arg;
    sig.n_args = 1;
    ctx.sig = &sig;
    ctx.doc = &doc;
    ctx.func_name = "test_fn";
    g_op_fail_doc_style_to_openapi = 1;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              c2openapi_build_operation(&ctx, &op));
    g_op_fail_doc_style_to_openapi = 0;
    reset_operation_test(&op);
  }

  /* 16. find_media_type_op fail for rb->example */
  {
    struct OpBuilderContext ctx;
    struct C2OpenAPI_ParsedSig sig;
    struct DocMetadata doc;
    struct DocRequestBody rb;
    struct OpenAPI_Operation op;
    memset(&ctx, 0, sizeof(ctx));
    memset(&sig, 0, sizeof(sig));
    memset(&doc, 0, sizeof(doc));
    memset(&rb, 0, sizeof(rb));
    memset(&op, 0, sizeof(op));
    rb.content_type = (char *)(size_t) "application/json";
    rb.example = (char *)(size_t) "{}";
    doc.request_bodies = &rb;
    doc.n_request_bodies = 1;
    ctx.sig = &sig;
    ctx.doc = &doc;
    ctx.func_name = "test_fn";
    g_op_fail_find_media_type_op = 1;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              c2openapi_build_operation(&ctx, &op));
    g_op_fail_find_media_type_op = 0;
    reset_operation_test(&op);
  }

  /* 17. find_media_type_op fail for doc->encodings */
  {
    struct OpBuilderContext ctx;
    struct C2OpenAPI_ParsedSig sig;
    struct DocMetadata doc;
    struct DocRequestBody rb;
    struct DocEncoding enc;
    struct OpenAPI_Operation op;
    memset(&ctx, 0, sizeof(ctx));
    memset(&sig, 0, sizeof(sig));
    memset(&doc, 0, sizeof(doc));
    memset(&rb, 0, sizeof(rb));
    memset(&enc, 0, sizeof(enc));
    memset(&op, 0, sizeof(op));
    rb.content_type = (char *)(size_t) "multipart/form-data";
    doc.request_bodies = &rb;
    doc.n_request_bodies = 1;
    enc.name = (char *)(size_t) "field1";
    doc.encodings = &enc;
    doc.n_encodings = 1;
    ctx.sig = &sig;
    ctx.doc = &doc;
    ctx.func_name = "test_fn";
    g_op_fail_find_media_type_op = 1;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              c2openapi_build_operation(&ctx, &op));
    g_op_fail_find_media_type_op = 0;
    reset_operation_test(&op);
  }

  /* 18. doc_style_to_openapi fail for d_enc->style */
  {
    struct OpBuilderContext ctx;
    struct C2OpenAPI_ParsedSig sig;
    struct DocMetadata doc;
    struct DocRequestBody rb;
    struct DocEncoding enc;
    struct OpenAPI_Operation op;
    memset(&ctx, 0, sizeof(ctx));
    memset(&sig, 0, sizeof(sig));
    memset(&doc, 0, sizeof(doc));
    memset(&rb, 0, sizeof(rb));
    memset(&enc, 0, sizeof(enc));
    memset(&op, 0, sizeof(op));
    rb.content_type = (char *)(size_t) "multipart/form-data";
    doc.request_bodies = &rb;
    doc.n_request_bodies = 1;
    enc.name = (char *)(size_t) "field1";
    enc.style = DOC_PARAM_STYLE_FORM;
    doc.encodings = &enc;
    doc.n_encodings = 1;
    ctx.sig = &sig;
    ctx.doc = &doc;
    ctx.func_name = "test_fn";
    g_op_fail_doc_style_to_openapi = 1;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              c2openapi_build_operation(&ctx, &op));
    g_op_fail_doc_style_to_openapi = 0;
    reset_operation_test(&op);
  }

  /* 19. ensure_response_for_code null for doc->response_headers */
  {
    struct OpBuilderContext ctx;
    struct C2OpenAPI_ParsedSig sig;
    struct DocMetadata doc;
    struct DocResponseHeader rh;
    struct OpenAPI_Operation op;
    memset(&ctx, 0, sizeof(ctx));
    memset(&sig, 0, sizeof(sig));
    memset(&doc, 0, sizeof(doc));
    memset(&rh, 0, sizeof(rh));
    memset(&op, 0, sizeof(op));
    rh.code = (char *)(size_t) "200";
    rh.name = (char *)(size_t) "X-Header";
    doc.response_headers = &rh;
    doc.n_response_headers = 1;
    ctx.sig = &sig;
    ctx.doc = &doc;
    ctx.func_name = "test_fn";
    g_op_fail_ensure_response_null = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, c2openapi_build_operation(&ctx, &op));
    g_op_fail_ensure_response_null = 0;
    reset_operation_test(&op);
  }

  /* 19b. ensure_response_for_code rc error for doc->response_headers */
  {
    struct OpBuilderContext ctx;
    struct C2OpenAPI_ParsedSig sig;
    struct DocMetadata doc;
    struct DocResponseHeader rh;
    struct OpenAPI_Operation op;
    memset(&ctx, 0, sizeof(ctx));
    memset(&sig, 0, sizeof(sig));
    memset(&doc, 0, sizeof(doc));
    memset(&rh, 0, sizeof(rh));
    memset(&op, 0, sizeof(op));
    rh.code = (char *)(size_t) "200";
    rh.name = (char *)(size_t) "X-Header";
    doc.response_headers = &rh;
    doc.n_response_headers = 1;
    ctx.sig = &sig;
    ctx.doc = &doc;
    ctx.func_name = "test_fn";
    g_op_fail_ensure_response_for_code = 1;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              c2openapi_build_operation(&ctx, &op));
    g_op_fail_ensure_response_for_code = 0;
    reset_operation_test(&op);
  }

  /* 20. ensure_response_for_code null for doc->links */
  {
    struct OpBuilderContext ctx;
    struct C2OpenAPI_ParsedSig sig;
    struct DocMetadata doc;
    struct DocLink dl;
    struct OpenAPI_Operation op;
    memset(&ctx, 0, sizeof(ctx));
    memset(&sig, 0, sizeof(sig));
    memset(&doc, 0, sizeof(doc));
    memset(&dl, 0, sizeof(dl));
    memset(&op, 0, sizeof(op));
    dl.code = (char *)(size_t) "200";
    dl.name = (char *)(size_t) "MyLink";
    doc.links = &dl;
    doc.n_links = 1;
    ctx.sig = &sig;
    ctx.doc = &doc;
    ctx.func_name = "test_fn";
    g_op_fail_ensure_response_null = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, c2openapi_build_operation(&ctx, &op));
    g_op_fail_ensure_response_null = 0;
    reset_operation_test(&op);
  }

  /* 20b. ensure_response_for_code rc error for doc->links */
  {
    struct OpBuilderContext ctx;
    struct C2OpenAPI_ParsedSig sig;
    struct DocMetadata doc;
    struct DocLink dl;
    struct OpenAPI_Operation op;
    memset(&ctx, 0, sizeof(ctx));
    memset(&sig, 0, sizeof(sig));
    memset(&doc, 0, sizeof(doc));
    memset(&dl, 0, sizeof(dl));
    memset(&op, 0, sizeof(op));
    dl.code = (char *)(size_t) "200";
    dl.name = (char *)(size_t) "MyLink";
    doc.links = &dl;
    doc.n_links = 1;
    ctx.sig = &sig;
    ctx.doc = &doc;
    ctx.func_name = "test_fn";
    g_op_fail_ensure_response_for_code = 1;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              c2openapi_build_operation(&ctx, &op));
    g_op_fail_ensure_response_for_code = 0;
    reset_operation_test(&op);
  }

  PASS();
}

TEST test_operation_reset_with_link_details(void) {
  struct OpenAPI_Operation op;
  struct OpenAPI_Response *resp;
  struct OpenAPI_Link *lnk;
  struct OpenAPI_Server *srv;
  struct OpenAPI_LinkParam *lp;

  memset(&op, 0, sizeof(op));
  resp = (struct OpenAPI_Response *)calloc(1, sizeof(*resp));
  lnk = (struct OpenAPI_Link *)calloc(1, sizeof(*lnk));
  srv = (struct OpenAPI_Server *)calloc(1, sizeof(*srv));
  lp = (struct OpenAPI_LinkParam *)calloc(1, sizeof(*lp));

  lp->name = strdup("param1");
  lp->value.string = strdup("val1");
  lp->value.json = strdup("{\"a\":1}");

  srv->url = strdup("http://example.com");
  srv->name = strdup("srv");
  srv->description = strdup("desc");

  lnk->n_parameters = 1;
  lnk->parameters = lp;
  lnk->server = srv;

  resp->n_links = 1;
  resp->links = lnk;

  op.n_responses = 1;
  op.responses = resp;

  reset_operation_test(&op);
  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_OPERATION_REACH_H */
