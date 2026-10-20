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

/**
 * @brief Unit test test_operation_reach_100_percent.
 * @return Greatest test result.
 */
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
    r.code = cdd_test_mut_str("200");
    op.responses = &r;
    op.n_responses = 1;
    g_cdd_fail_stricmp = 1;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              find_response_by_code(&op, "200", &out));
    g_cdd_fail_stricmp = 0;
  }

  /* 5. apply_example_to_response with content_type not found */
  {
    struct OpenAPI_Response resp;
    struct OpenAPI_MediaType mt;
    memset(&resp, 0, sizeof(resp));
    memset(&mt, 0, sizeof(mt));
    mt.name = cdd_test_mut_str("application/json");
    resp.content_media_types = &mt;
    resp.n_content_media_types = 1;
    ASSERT_EQ(CDD_C_SUCCESS,
              apply_example_to_response(&resp, "{\"a\":1}", "text/plain"));
  }

  /* 6. ensure_response_for_code find_response_by_code fail */
  {
    struct OpenAPI_Operation op;
    struct OpenAPI_Response r;
    struct OpenAPI_Response *out = NULL;
    memset(&op, 0, sizeof(op));
    memset(&r, 0, sizeof(r));
    r.code = cdd_test_mut_str("200");
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
    hdr.name = cdd_test_mut_str("X-Test");
    resp.headers = &hdr;
    resp.n_headers = 1;
    dh.name = cdd_test_mut_str("X-Test");
    g_cdd_fail_stricmp = 1;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, add_header_to_response(&resp, &dh));
    g_cdd_fail_stricmp = 0;
  }

  /* 9. apply_format_to_schema_ref non-primitive type */
  {
    struct OpenAPI_SchemaRef schema;
    struct OpenApiTypeMapping map;
    int applied = 0;
    memset(&schema, 0, sizeof(schema));
    memset(&map, 0, sizeof(map));
    map.oa_format = cdd_test_mut_str("custom");
    map.oa_type = cdd_test_mut_str("object");
    ASSERT_EQ(CDD_C_SUCCESS,
              apply_format_to_schema_ref(&schema, &map, NULL, &applied));
    ASSERT_EQ(0, applied);
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
    arg.name = cdd_test_mut_str("x");
    arg.type = cdd_test_mut_str("int");
    sig.args = &arg;
    sig.n_args = 1;
    ctx.sig = &sig;
    ctx.func_name = "test_fn";
    g_mapping_fail_init = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, c2openapi_build_operation(&ctx, &op));
    g_mapping_fail_init = 0;
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
    dp.name = cdd_test_mut_str("hdr");
    dp.in_loc = cdd_test_mut_str("header");
    doc.params = &dp;
    doc.n_params = 1;
    arg.name = cdd_test_mut_str("hdr");
    arg.type = cdd_test_mut_str("char *");
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

  /* 19. ensure_response_for_code OOM for doc->response_headers */
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
    rh.code = cdd_test_mut_str("200");
    rh.name = cdd_test_mut_str("X-Header");
    doc.response_headers = &rh;
    doc.n_response_headers = 1;
    ctx.sig = &sig;
    ctx.doc = &doc;
    ctx.func_name = "test_fn";
    g_cdd_alloc_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, c2openapi_build_operation(&ctx, &op));
    g_cdd_alloc_fail = 0;
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
    rh.code = cdd_test_mut_str("200");
    rh.name = cdd_test_mut_str("X-Header");
    doc.response_headers = &rh;
    doc.n_response_headers = 1;
    ctx.sig = &sig;
    ctx.doc = &doc;
    ctx.func_name = "test_fn";
    g_cdd_fail_stricmp = 1;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              c2openapi_build_operation(&ctx, &op));
    g_cdd_fail_stricmp = 0;
    reset_operation_test(&op);
  }

  /* 20. ensure_response_for_code OOM for doc->links */
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
    dl.code = cdd_test_mut_str("200");
    dl.name = cdd_test_mut_str("MyLink");
    doc.links = &dl;
    doc.n_links = 1;
    ctx.sig = &sig;
    ctx.doc = &doc;
    ctx.func_name = "test_fn";
    g_cdd_alloc_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, c2openapi_build_operation(&ctx, &op));
    g_cdd_alloc_fail = 0;
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
    dl.code = cdd_test_mut_str("200");
    dl.name = cdd_test_mut_str("MyLink");
    doc.links = &dl;
    doc.n_links = 1;
    ctx.sig = &sig;
    ctx.doc = &doc;
    ctx.func_name = "test_fn";
    g_cdd_fail_stricmp = 1;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              c2openapi_build_operation(&ctx, &op));
    g_cdd_fail_stricmp = 0;
    reset_operation_test(&op);
  }

  PASS();
}

/**
 * @brief Unit test test_operation_reset_with_link_details.
 * @return Greatest test result.
 */
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
