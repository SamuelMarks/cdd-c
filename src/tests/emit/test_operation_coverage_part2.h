/**
 * @file test_operation_coverage_part2.h
 * @brief Extended coverage tests (part 2) for operation generator.
 */

#ifndef TEST_OPERATION_COVERAGE_PART2_H
#define TEST_OPERATION_COVERAGE_PART2_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "emit/test_operation_common.h"
/* clang-format on */

/**
 * @brief Unit test test_operation_100_percent_coverage_part2.
 * @return Greatest test result.
 */
TEST test_operation_100_percent_coverage_part2(void) {
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, find_doc_param(NULL, "x", NULL));

  {
    struct OpenAPI_Any a;
    memset(&a, 0, sizeof(a));
    a.type = OA_ANY_STRING;
    a.string = NULL;
    free_any_value_local(&a);
    a.type = OA_ANY_JSON;
    a.json = NULL;
    free_any_value_local(&a);
  }

  {
    struct OpenAPI_Operation op_resp;
    struct OpenAPI_Response resps[2];
    struct OpenAPI_Response *found_r = NULL;
    memset(&op_resp, 0, sizeof(op_resp));
    memset(resps, 0, sizeof(resps));
    resps[0].code = NULL;
    resps[1].code = cdd_test_mut_str("200");
    op_resp.responses = resps;
    op_resp.n_responses = 2;
    ASSERT_EQ(CDD_C_SUCCESS, find_response_by_code(&op_resp, "200", &found_r));
    ASSERT(found_r != NULL);
  }

  {
    struct OpenAPI_MediaType mts[2];
    struct OpenAPI_MediaType *found_mt = NULL;
    memset(mts, 0, sizeof(mts));
    mts[0].name = NULL;
    mts[1].name = cdd_test_mut_str("text/plain");
    ASSERT_EQ(CDD_C_SUCCESS,
              find_media_type_op(mts, 2, "text/plain", &found_mt));
    ASSERT(found_mt != NULL);
  }

  {
    struct OpenAPI_Response resp_hdr;
    struct DocResponseHeader dh;
    memset(&resp_hdr, 0, sizeof(resp_hdr));
    resp_hdr.headers =
        (struct OpenAPI_Header *)calloc(1, sizeof(struct OpenAPI_Header));
    resp_hdr.headers[0].name = NULL;
    resp_hdr.n_headers = 1;
    memset(&dh, 0, sizeof(dh));
    dh.name = cdd_test_mut_str("X-Custom");
    dh.type = cdd_test_mut_str("string");
    dh.description = cdd_test_mut_str("desc");
    dh.content_type = cdd_test_mut_str("text/plain");
    ASSERT_EQ(CDD_C_SUCCESS, add_header_to_response(&resp_hdr, &dh));
    free(resp_hdr.headers[1].name);
    free(resp_hdr.headers[1].type);
    free(resp_hdr.headers[1].description);
    free(resp_hdr.headers[1].content_type);
    free(resp_hdr.headers);
  }

  {
    struct OpenAPI_Response resp_lnk;
    struct DocLink dl;
    memset(&resp_lnk, 0, sizeof(resp_lnk));
    resp_lnk.links =
        (struct OpenAPI_Link *)calloc(1, sizeof(struct OpenAPI_Link));
    resp_lnk.links[0].name = NULL;
    resp_lnk.n_links = 1;
    memset(&dl, 0, sizeof(dl));
    dl.name = cdd_test_mut_str("Link1");
    dl.operation_id = cdd_test_mut_str("op1");
    ASSERT_EQ(CDD_C_SUCCESS, add_link_to_response(&resp_lnk, &dl));
    g_cdd_strdup_fail = 1;
    dl.name = cdd_test_mut_str("Link2");
    ASSERT_EQ(CDD_C_ERROR_MEMORY, add_link_to_response(&resp_lnk, &dl));
    g_cdd_strdup_fail = 0;
    cleanup_link_fields(&resp_lnk.links[1]);
    free(resp_lnk.links);
  }

  {
    int d_ptr = 0, is_st = 0;
    ASSERT_EQ(CDD_C_SUCCESS, is_struct_pointer("struct Foo*", NULL, NULL));
    ASSERT_EQ(CDD_C_SUCCESS, is_struct_pointer("struct Foo*", &d_ptr, &is_st));
    ASSERT_EQ(CDD_C_SUCCESS, is_struct_pointer("struct Foo**", &d_ptr, &is_st));
    ASSERT_EQ(CDD_C_SUCCESS, is_struct_pointer("*struct Foo", &d_ptr, &is_st));
  }

  {
    struct OpBuilderContext ctx;
    struct C2OpenAPI_ParsedSig sig;
    struct OpenAPI_Operation op;
    memset(&ctx, 0, sizeof(ctx));
    memset(&sig, 0, sizeof(sig));
    memset(&op, 0, sizeof(op));
    ctx.sig = &sig;
    ctx.func_name = "do_something";
    ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
    ASSERT_EQ(OA_VERB_GET, op.verb);
    reset_operation_test(&op);
  }

  {
    struct OpBuilderContext ctx;
    struct C2OpenAPI_ParsedSig sig;
    struct C2OpenAPI_ParsedArg arg;
    struct OpenAPI_Operation op;
    memset(&ctx, 0, sizeof(ctx));
    memset(&sig, 0, sizeof(sig));
    memset(&arg, 0, sizeof(arg));
    memset(&op, 0, sizeof(op));
    arg.name = cdd_test_mut_str("items");
    arg.type = cdd_test_mut_str("int[]");
    sig.args = &arg;
    sig.n_args = 1;
    ctx.sig = &sig;
    ctx.func_name = "api_user_get";
    g_cdd_strdup_fail = 3;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, c2openapi_build_operation(&ctx, &op));
    g_cdd_strdup_fail = 0;
    reset_operation_test(&op);

    g_cdd_strdup_fail = 4;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, c2openapi_build_operation(&ctx, &op));
    g_cdd_strdup_fail = 0;
    reset_operation_test(&op);
  }

  {
    struct OpBuilderContext ctx;
    struct C2OpenAPI_ParsedSig sig;
    struct DocMetadata doc;
    struct DocRequestBody rb;
    struct DocEncoding encs[2];
    struct OpenAPI_Operation op;
    memset(&ctx, 0, sizeof(ctx));
    memset(&sig, 0, sizeof(sig));
    memset(&doc, 0, sizeof(doc));
    memset(&rb, 0, sizeof(rb));
    memset(encs, 0, sizeof(encs));
    memset(&op, 0, sizeof(op));

    rb.content_type = cdd_test_mut_str("application/json");
    doc.request_bodies = &rb;
    doc.n_request_bodies = 1;

    encs[0].name = cdd_test_mut_str("field1");
    encs[0].content_type = cdd_test_mut_str("application/xml");
    encs[0].kind = 1;

    encs[1].name = cdd_test_mut_str("field2");
    encs[1].content_type = cdd_test_mut_str("text/plain");
    encs[1].kind = 2;

    doc.encodings = encs;
    doc.n_encodings = 2;

    ctx.sig = &sig;
    ctx.doc = &doc;
    ctx.func_name = "api_user_post";

    ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
    reset_operation_test(&op);
  }

  {
    struct OpBuilderContext ctx;
    struct C2OpenAPI_ParsedSig sig;
    struct OpenAPI_Operation op;
    char *custom_tag = cdd_test_mut_str("Custom");
    memset(&ctx, 0, sizeof(ctx));
    memset(&sig, 0, sizeof(sig));
    memset(&op, 0, sizeof(op));

    ctx.sig = &sig;
    ctx.func_name = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
    reset_operation_test(&op);

    memset(&op, 0, sizeof(op));
    op.tags = &custom_tag;
    op.n_tags = 1;
    ctx.func_name = "api_user_get";
    ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
    reset_operation_test(&op);

    memset(&op, 0, sizeof(op));
    ctx.func_name = "api_user_get";
    g_cdd_alloc_fail = 2;
    ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
    g_cdd_alloc_fail = 0;
    reset_operation_test(&op);
  }

  {
    struct OpBuilderContext ctx;
    struct C2OpenAPI_ParsedSig sig;
    struct DocMetadata doc;
    struct DocServer srv;
    struct OpenAPI_Operation op;
    memset(&ctx, 0, sizeof(ctx));
    memset(&sig, 0, sizeof(sig));
    memset(&doc, 0, sizeof(doc));
    memset(&srv, 0, sizeof(srv));
    memset(&op, 0, sizeof(op));

    doc.external_docs_url = cdd_test_mut_str("https://example.com/docs");
    doc.external_docs_description = NULL;

    srv.url = NULL;
    srv.name = NULL;
    srv.description = NULL;
    srv.n_variables = 0;
    doc.servers = &srv;
    doc.n_servers = 1;

    ctx.sig = &sig;
    ctx.doc = &doc;
    ctx.func_name = "api_user_get";

    ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
    reset_operation_test(&op);
  }

  /* Case 1: copy_any_value_local types */
  {
    struct OpenAPI_Any src, dst;
    memset(&src, 0, sizeof(src));
    src.type = OA_ANY_NUMBER;
    src.number = 42;
    ASSERT_EQ(CDD_C_SUCCESS, copy_any_value_local(&dst, &src));
    ASSERT_EQ(42, (int)dst.number);
    free_any_value_local(&dst);

    memset(&src, 0, sizeof(src));
    src.type = OA_ANY_BOOL;
    src.boolean = 1;
    ASSERT_EQ(CDD_C_SUCCESS, copy_any_value_local(&dst, &src));
    ASSERT_EQ(1, dst.boolean);
    free_any_value_local(&dst);

    memset(&src, 0, sizeof(src));
    src.type = OA_ANY_NULL;
    ASSERT_EQ(CDD_C_SUCCESS, copy_any_value_local(&dst, &src));
    ASSERT_EQ(OA_ANY_NULL, dst.type);
    free_any_value_local(&dst);

    memset(&src, 0, sizeof(src));
    src.type = OA_ANY_UNSET;
    ASSERT_EQ(CDD_C_SUCCESS, copy_any_value_local(&dst, &src));
    ASSERT_EQ(OA_ANY_UNSET, dst.type);
    free_any_value_local(&dst);
  }

  /* Case 2: DocServerVar with default_value == NULL and enum_values == NULL
   * with count > 0 */
  {
    struct OpenAPI_Server srv;
    struct DocServer dserver;
    struct DocServerVar svar;
    memset(&srv, 0, sizeof(srv));
    memset(&dserver, 0, sizeof(dserver));
    memset(&svar, 0, sizeof(svar));
    svar.name = cdd_test_mut_str("env");
    svar.default_value = NULL;
    dserver.variables = &svar;
    dserver.n_variables = 1;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              copy_doc_server_variables_op(&srv, &dserver));

    svar.default_value = cdd_test_mut_str("prod");
    svar.enum_values = NULL;
    svar.n_enum_values = 1;
    ASSERT_EQ(CDD_C_SUCCESS, copy_doc_server_variables_op(&srv, &dserver));
    free_openapi_server_variables_op(&srv);
  }

  /* Case 3: apply_example_to_response when content_type not found in
   * resp->content_media_types */
  {
    struct OpenAPI_Response resp;
    struct OpenAPI_MediaType mt;
    memset(&resp, 0, sizeof(resp));
    memset(&mt, 0, sizeof(mt));
    mt.name = cdd_test_mut_str("application/json");
    resp.content_media_types = &mt;
    resp.n_content_media_types = 1;
    ASSERT_EQ(CDD_C_SUCCESS,
              apply_example_to_response(&resp, "ex", "nonexistent/type"));
  }

  /* Case 4: add_header_to_response when hdr->type == NULL and dh->type == NULL
   */
  {
    struct OpenAPI_Response resp;
    struct DocResponseHeader dh;
    memset(&resp, 0, sizeof(resp));
    memset(&dh, 0, sizeof(dh));
    dh.name = cdd_test_mut_str("X-Test");
    dh.type = NULL;
    dh.format = cdd_test_mut_str("int32");
    ASSERT_EQ(CDD_C_SUCCESS, add_header_to_response(&resp, &dh));
    dh.example = cdd_test_mut_str("123");
    resp.headers[0].example_set = 1;
    ASSERT_EQ(CDD_C_SUCCESS, add_header_to_response(&resp, &dh));
    free(resp.headers[0].name);
    free(resp.headers[0].type);
    free(resp.headers[0].schema.inline_type);
    free(resp.headers[0].schema.format);
    free(resp.headers);
  }

  /* Case 5: cleanup_link_fields with link->parameters[p].name == NULL */
  {
    struct OpenAPI_Link lnk;
    memset(&lnk, 0, sizeof(lnk));
    lnk.parameters =
        (struct OpenAPI_LinkParam *)calloc(1, sizeof(struct OpenAPI_LinkParam));
    lnk.n_parameters = 1;
    cleanup_link_fields(&lnk);
  }

  /* Case 6: schema_ref_has_data_basic with is_array = 1 and ref_name == NULL */
  {
    struct OpenAPI_SchemaRef sref;
    int has_d = 0;
    memset(&sref, 0, sizeof(sref));
    sref.is_array = 1;
    ASSERT_EQ(CDD_C_SUCCESS, schema_ref_has_data_basic(&sref, &has_d));
    ASSERT_EQ(1, has_d);
  }

  /* Case 7: copy_schema_ref_basic with ref_name != NULL */
  {
    struct OpenAPI_SchemaRef src, dst;
    memset(&src, 0, sizeof(src));
    src.ref_name = cdd_test_mut_str("MyRef");
    ASSERT_EQ(CDD_C_SUCCESS, copy_schema_ref_basic(&dst, &src));
    ASSERT(dst.ref_name != NULL);
    free(dst.ref_name);
  }

  /* Case 8: response_has_media_type and request_body_has_media_type name
   * mismatch */
  {
    struct OpenAPI_Response resp;
    struct OpenAPI_Operation op;
    struct OpenAPI_MediaType mt;
    int has_mt = 0;
    memset(&resp, 0, sizeof(resp));
    memset(&op, 0, sizeof(op));
    memset(&mt, 0, sizeof(mt));
    mt.name = cdd_test_mut_str("application/json");
    resp.content_media_types = &mt;
    resp.n_content_media_types = 1;
    ASSERT_EQ(CDD_C_SUCCESS,
              response_has_media_type(&resp, "text/xml", &has_mt));
    ASSERT_EQ(0, has_mt);

    op.req_body_media_types = &mt;
    op.n_req_body_media_types = 1;
    ASSERT_EQ(CDD_C_SUCCESS,
              request_body_has_media_type(&op, "text/xml", &has_mt));
    ASSERT_EQ(0, has_mt);
  }

  /* Case 9: apply_format_to_schema_ref with override_format = "" and
   * out_applied = NULL */
  {
    struct OpenAPI_SchemaRef sref;
    struct OpenApiTypeMapping map;
    memset(&sref, 0, sizeof(sref));
    memset(&map, 0, sizeof(map));
    map.oa_type = cdd_test_mut_str("string");
    map.oa_format = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, apply_format_to_schema_ref(&sref, &map, "", NULL));
    ASSERT_EQ(CDD_C_SUCCESS,
              apply_format_to_schema_ref(&sref, &map, "uuid", NULL));
    free(sref.inline_type);
    free(sref.format);
  }

  /* Case 10: doc_style_to_openapi DOC_PARAM_STYLE_UNSET */
  {
    enum OpenAPI_Style st = OA_STYLE_FORM;
    ASSERT_EQ(CDD_C_SUCCESS, doc_style_to_openapi(DOC_PARAM_STYLE_UNSET, &st));
    ASSERT_EQ(OA_STYLE_UNKNOWN, st);
  }

  /* Case 11: PUT verb with non-const struct pointer */
  {
    struct OpBuilderContext ctx;
    struct C2OpenAPI_ParsedSig sig;
    struct C2OpenAPI_ParsedArg arg;
    struct OpenAPI_Operation op;
    memset(&ctx, 0, sizeof(ctx));
    memset(&sig, 0, sizeof(sig));
    memset(&arg, 0, sizeof(arg));
    memset(&op, 0, sizeof(op));
    arg.name = cdd_test_mut_str("in");
    arg.type = cdd_test_mut_str("struct Foo *");
    sig.args = &arg;
    sig.n_args = 1;
    ctx.sig = &sig;
    ctx.func_name = "api_put_user";
    ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
    reset_operation_test(&op);
  }

  /* Case 12: Header parameter default style and query param style unset */
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

    arg.name = cdd_test_mut_str("X-Header");
    arg.type = cdd_test_mut_str("char *");
    sig.args = &arg;
    sig.n_args = 1;
    dp.name = cdd_test_mut_str("X-Header");
    dp.in_loc = cdd_test_mut_str("header");
    dp.style = DOC_PARAM_STYLE_UNSET;
    dp.style_set = 1;
    doc.params = &dp;
    doc.n_params = 1;
    ctx.sig = &sig;
    ctx.doc = &doc;
    ctx.func_name = "api_user_get";

    ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
    reset_operation_test(&op);
  }

  /* Case 13: Querystring parameter example without content_type */
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

    arg.name = cdd_test_mut_str("qs");
    arg.type = cdd_test_mut_str("char *");
    sig.args = &arg;
    sig.n_args = 1;
    dp.name = cdd_test_mut_str("qs");
    dp.in_loc = cdd_test_mut_str("querystring");
    dp.example = cdd_test_mut_str("a=1");
    doc.params = &dp;
    doc.n_params = 1;
    ctx.sig = &sig;
    ctx.doc = &doc;
    ctx.func_name = "api_user_get";

    ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
    reset_operation_test(&op);
  }

  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_OPERATION_COVERAGE_PART2_H */
