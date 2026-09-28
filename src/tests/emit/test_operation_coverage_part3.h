/**
 * @file test_operation_coverage_part3.h
 * @brief Extended coverage tests (part 3) for operation generator.
 */

#ifndef TEST_OPERATION_COVERAGE_PART3_H
#define TEST_OPERATION_COVERAGE_PART3_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "emit/test_operation_common.h"
/* clang-format on */

TEST test_operation_100_percent_coverage_part3(void) {
  /* Case 14: Encoding with name == NULL and multiple item encodings */
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

    rb.content_type = (char *)(size_t) "application/json";
    doc.request_bodies = &rb;
    doc.n_request_bodies = 1;

    encs[0].name = NULL;
    encs[0].kind = 2;
    encs[1].name = NULL;
    encs[1].kind = 2;

    doc.encodings = encs;
    doc.n_encodings = 2;

    ctx.sig = &sig;
    ctx.doc = &doc;
    ctx.func_name = "api_user_post";

    ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
    reset_operation_test(&op);
  }

  /* Case 15: Returns with existing summary, code == NULL, and description ==
   * NULL */
  {
    struct OpBuilderContext ctx;
    struct C2OpenAPI_ParsedSig sig;
    struct DocMetadata doc;
    struct DocResponse rets[2];
    struct OpenAPI_Operation op;
    memset(&ctx, 0, sizeof(ctx));
    memset(&sig, 0, sizeof(sig));
    memset(&doc, 0, sizeof(doc));
    memset(rets, 0, sizeof(rets));
    memset(&op, 0, sizeof(op));

    rets[0].code = (char *)(size_t) "200";
    rets[0].summary = (char *)(size_t) "First summary";
    rets[0].description = NULL;
    rets[1].code = (char *)(size_t) "200";
    rets[1].summary = (char *)(size_t) "Second summary";
    rets[1].description = (char *)(size_t) "New desc";

    doc.returns = rets;
    doc.n_returns = 2;

    ctx.sig = &sig;
    ctx.doc = &doc;
    ctx.func_name = "api_user_get";

    ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
    reset_operation_test(&op);
  }

  /* Case 16: Return with code == NULL */
  {
    struct OpBuilderContext ctx;
    struct C2OpenAPI_ParsedSig sig;
    struct DocMetadata doc;
    struct DocResponse rets[2];
    struct OpenAPI_Operation op;
    memset(&ctx, 0, sizeof(ctx));
    memset(&sig, 0, sizeof(sig));
    memset(&doc, 0, sizeof(doc));
    memset(rets, 0, sizeof(rets));
    memset(&op, 0, sizeof(op));

    rets[0].code = (char *)(size_t) "200";
    rets[1].code = NULL;
    doc.returns = rets;
    doc.n_returns = 2;

    ctx.sig = &sig;
    ctx.doc = &doc;
    ctx.func_name = "api_user_get";

    ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
    reset_operation_test(&op);
  }

  /* Case 17: Tags strdup failure */
  {
    struct OpBuilderContext ctx;
    struct C2OpenAPI_ParsedSig sig;
    struct OpenAPI_Operation op;
    memset(&ctx, 0, sizeof(ctx));
    memset(&sig, 0, sizeof(sig));
    memset(&op, 0, sizeof(op));

    ctx.sig = &sig;
    ctx.func_name = "api_user_get";
    g_cdd_strdup_fail = 5;
    ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
    g_cdd_strdup_fail = 0;
    reset_operation_test(&op);
  }

  /* Case 18: copy_any_value_local and doc_style_to_openapi default cases */
  {
    struct OpenAPI_Any src, dst;
    enum OpenAPI_Style st = OA_STYLE_FORM;
    memset(&src, 0, sizeof(src));
    src.type = (enum OpenAPI_AnyType)99;
    ASSERT_EQ(CDD_C_SUCCESS, copy_any_value_local(&dst, &src));
    ASSERT_EQ(CDD_C_SUCCESS, doc_style_to_openapi((enum DocParamStyle)99, &st));
    ASSERT_EQ(OA_STYLE_UNKNOWN, st);
  }

  /* Case 19: copy_doc_server_variables_op enum_values != NULL && n_enum_values
   * == 0 */
  {
    struct OpenAPI_Server srv;
    struct DocServer dserver;
    struct DocServerVar svar;
    char *enums[1];
    memset(&srv, 0, sizeof(srv));
    memset(&dserver, 0, sizeof(dserver));
    memset(&svar, 0, sizeof(svar));
    enums[0] = (char *)(size_t) "v1";
    svar.name = (char *)(size_t) "env";
    svar.default_value = (char *)(size_t) "v1";
    svar.enum_values = enums;
    svar.n_enum_values = 0;
    dserver.variables = &svar;
    dserver.n_variables = 1;
    ASSERT_EQ(CDD_C_SUCCESS, copy_doc_server_variables_op(&srv, &dserver));
    free_openapi_server_variables_op(&srv);
  }

  /* Case 20: apply_example_to_response with mt->example_set == 1 and
   * n_content_media_types == 0 */
  {
    struct OpenAPI_Response resp;
    struct OpenAPI_MediaType mt;
    memset(&resp, 0, sizeof(resp));
    memset(&mt, 0, sizeof(mt));
    mt.name = (char *)(size_t) "application/json";
    mt.example_set = 1;
    resp.content_media_types = &mt;
    resp.n_content_media_types = 1;
    ASSERT_EQ(CDD_C_SUCCESS,
              apply_example_to_response(&resp, "ex", "application/json"));

    resp.n_content_media_types = 0;
    ASSERT_EQ(CDD_C_SUCCESS, apply_example_to_response(&resp, "ex", NULL));
  }

  /* Case 21: schema_ref_has_data_basic empty strings */
  {
    struct OpenAPI_SchemaRef sref;
    int has_d = 0;
    memset(&sref, 0, sizeof(sref));
    sref.inline_type = (char *)(size_t) "";
    ASSERT_EQ(CDD_C_SUCCESS, schema_ref_has_data_basic(&sref, &has_d));
    ASSERT_EQ(0, has_d);

    sref.inline_type = NULL;
    sref.ref_name = (char *)(size_t) "";
    ASSERT_EQ(CDD_C_SUCCESS, schema_ref_has_data_basic(&sref, &has_d));
    ASSERT_EQ(0, has_d);

    sref.ref_name = NULL;
    sref.ref = (char *)(size_t) "";
    ASSERT_EQ(CDD_C_SUCCESS, schema_ref_has_data_basic(&sref, &has_d));
    ASSERT_EQ(0, has_d);
  }

  /* Case 22: copy_schema_ref_basic strdup failure */
  {
    struct OpenAPI_SchemaRef src, dst;
    memset(&src, 0, sizeof(src));
    src.ref_name = (char *)(size_t) "MyRef";
    g_cdd_strdup_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, copy_schema_ref_basic(&dst, &src));
    g_cdd_strdup_fail = 0;
  }

  /* Case 23: response_has_media_type and request_body_has_media_type with
   * mt.name == NULL */
  {
    struct OpenAPI_Response resp;
    struct OpenAPI_Operation op;
    struct OpenAPI_MediaType mt;
    int has_mt = 0;
    memset(&resp, 0, sizeof(resp));
    memset(&op, 0, sizeof(op));
    memset(&mt, 0, sizeof(mt));
    mt.name = NULL;
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

  /* Case 24: apply_format_to_schema_ref with map->oa_format = "" */
  {
    struct OpenAPI_SchemaRef sref;
    struct OpenApiTypeMapping map;
    int applied = 0;
    memset(&sref, 0, sizeof(sref));
    memset(&map, 0, sizeof(map));
    map.oa_type = (char *)(size_t) "string";
    map.oa_format = (char *)(size_t) "";
    ASSERT_EQ(CDD_C_SUCCESS,
              apply_format_to_schema_ref(&sref, &map, NULL, &applied));
    ASSERT_EQ(0, applied);
  }

  /* Case 25: GET with non-const struct pointer */
  {
    struct OpBuilderContext ctx;
    struct C2OpenAPI_ParsedSig sig;
    struct C2OpenAPI_ParsedArg arg;
    struct OpenAPI_Operation op;
    memset(&ctx, 0, sizeof(ctx));
    memset(&sig, 0, sizeof(sig));
    memset(&arg, 0, sizeof(arg));
    memset(&op, 0, sizeof(op));
    arg.name = (char *)(size_t) "in";
    arg.type = (char *)(size_t) "struct Foo *";
    sig.args = &arg;
    sig.n_args = 1;
    ctx.sig = &sig;
    ctx.func_name = "api_user_get";
    ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
    reset_operation_test(&op);
  }

  /* Case 26: Output param and Body param with primitive oa_type and OOM */
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
    memset(&op, 0, sizeof(op));
    arg.name = (char *)(size_t) "out";
    arg.type = (char *)(size_t) "int **";
    sig.args = &arg;
    sig.n_args = 1;
    ctx.sig = &sig;
    ctx.func_name = "api_user_get";
    ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
    reset_operation_test(&op);

    memset(&ctx, 0, sizeof(ctx));
    memset(&sig, 0, sizeof(sig));
    memset(&arg, 0, sizeof(arg));
    memset(&doc, 0, sizeof(doc));
    memset(&dp, 0, sizeof(dp));
    memset(&op, 0, sizeof(op));
    arg.name = (char *)(size_t) "in";
    arg.type = (char *)(size_t) "int *";
    sig.args = &arg;
    sig.n_args = 1;
    dp.name = (char *)(size_t) "in";
    dp.in_loc = (char *)(size_t) "body";
    doc.params = &dp;
    doc.n_params = 1;
    ctx.sig = &sig;
    ctx.doc = &doc;
    ctx.func_name = "api_post_user";
    ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
    reset_operation_test(&op);
  }

  /* Case 27: Querystring param with example and no style */
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

    arg.name = (char *)(size_t) "qs";
    arg.type = (char *)(size_t) "int";
    sig.args = &arg;
    sig.n_args = 1;
    dp.name = (char *)(size_t) "qs";
    dp.in_loc = (char *)(size_t) "querystring";
    dp.example = (char *)(size_t) "10";
    doc.params = &dp;
    doc.n_params = 1;
    ctx.sig = &sig;
    ctx.doc = &doc;
    ctx.func_name = "api_user_get";
    ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
    reset_operation_test(&op);
  }

  /* Case 28: Doc with request_body_content_type and n_request_bodies > 0 */
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
    doc.request_bodies = &rb;
    doc.n_request_bodies = 1;
    doc.request_body_content_type = (char *)(size_t) "application/xml";

    ctx.sig = &sig;
    ctx.doc = &doc;
    ctx.func_name = "api_user_post";
    ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
    reset_operation_test(&op);
  }

  /* Case 29: Allocation failure on prefix_encoding and item_encoding */
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

    rb.content_type = (char *)(size_t) "application/json";
    doc.request_bodies = &rb;
    doc.n_request_bodies = 1;

    enc.name = (char *)(size_t) "f1";
    enc.content_type = (char *)(size_t) "text/plain";
    enc.kind = 1;
    doc.encodings = &enc;
    doc.n_encodings = 1;

    ctx.sig = &sig;
    ctx.doc = &doc;
    ctx.func_name = "api_user_post";

    g_cdd_alloc_fail = 2;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, c2openapi_build_operation(&ctx, &op));
    g_cdd_alloc_fail = 0;
    reset_operation_test(&op);

    enc.kind = 2;
    g_cdd_alloc_fail = 2;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, c2openapi_build_operation(&ctx, &op));
    g_cdd_alloc_fail = 0;
    reset_operation_test(&op);
  }

  /* Case 30: free_encoding_fields with non-null and null */
  {
    struct OpenAPI_Encoding test_enc;
    memset(&test_enc, 0, sizeof(test_enc));
    test_enc.name = (char *)malloc(10);
    test_enc.content_type = (char *)malloc(10);
    free_encoding_fields(&test_enc);
    free_encoding_fields(&test_enc);
    free_encoding_fields(NULL);
  }

  /* Case 31: add_header_to_response existing header with type == NULL and
   * example == NULL */
  {
    struct OpenAPI_Response resp_ex;
    struct OpenAPI_Header h_ex;
    struct DocResponseHeader dh_ex;
    memset(&resp_ex, 0, sizeof(resp_ex));
    memset(&h_ex, 0, sizeof(h_ex));
    memset(&dh_ex, 0, sizeof(dh_ex));
    h_ex.name = (char *)(size_t) "X-Null-Type";
    h_ex.type = NULL;
    resp_ex.headers = &h_ex;
    resp_ex.n_headers = 1;

    dh_ex.name = (char *)(size_t) "X-Null-Type";
    dh_ex.format = (char *)(size_t) "int32";
    dh_ex.example = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, add_header_to_response(&resp_ex, &dh_ex));
    free(h_ex.schema.inline_type);
    free(h_ex.schema.format);

    h_ex.type = (char *)(size_t) "string";
    h_ex.schema.inline_type = NULL;
    h_ex.schema.format = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, add_header_to_response(&resp_ex, &dh_ex));
    free(h_ex.schema.inline_type);
    free(h_ex.schema.format);
  }

  /* Case 32: copy_schema_ref_basic with ref_name == NULL */
  {
    struct OpenAPI_SchemaRef src_no_ref, dst_no_ref;
    memset(&src_no_ref, 0, sizeof(src_no_ref));
    src_no_ref.ref_name = NULL;
    src_no_ref.ref = (char *)(size_t) "SomeRef";
    ASSERT_EQ(CDD_C_SUCCESS, copy_schema_ref_basic(&dst_no_ref, &src_no_ref));
    free(dst_no_ref.ref);
  }

  /* Case 33: output param strdup failure loop */
  {
    struct OpBuilderContext ctx;
    struct C2OpenAPI_ParsedSig sig;
    struct C2OpenAPI_ParsedArg arg;
    struct OpenAPI_Operation op;
    int k;
    memset(&ctx, 0, sizeof(ctx));
    memset(&sig, 0, sizeof(sig));
    memset(&arg, 0, sizeof(arg));
    arg.name = (char *)(size_t) "out";
    arg.type = (char *)(size_t) "int **";
    sig.args = &arg;
    sig.n_args = 1;
    ctx.sig = &sig;
    ctx.func_name = "api_user_get";

    for (k = 1; k <= 8; ++k) {
      memset(&op, 0, sizeof(op));
      g_cdd_strdup_fail = k;
      if (c2openapi_build_operation(&ctx, &op) != CDD_C_SUCCESS) {
      }
      g_cdd_strdup_fail = 0;
      reset_operation_test(&op);
    }
  }

  /* Case 34: body param strdup failure loop */
  {
    struct OpBuilderContext ctx;
    struct C2OpenAPI_ParsedSig sig;
    struct C2OpenAPI_ParsedArg arg;
    struct DocMetadata doc;
    struct DocParam dp;
    struct OpenAPI_Operation op;
    int k;
    memset(&ctx, 0, sizeof(ctx));
    memset(&sig, 0, sizeof(sig));
    memset(&arg, 0, sizeof(arg));
    memset(&doc, 0, sizeof(doc));
    memset(&dp, 0, sizeof(dp));
    arg.name = (char *)(size_t) "in";
    arg.type = (char *)(size_t) "int *";
    sig.args = &arg;
    sig.n_args = 1;
    dp.name = (char *)(size_t) "in";
    dp.in_loc = (char *)(size_t) "body";
    doc.params = &dp;
    doc.n_params = 1;
    ctx.sig = &sig;
    ctx.doc = &doc;
    ctx.func_name = "api_post_user";

    for (k = 1; k <= 8; ++k) {
      memset(&op, 0, sizeof(op));
      g_cdd_strdup_fail = k;
      if (c2openapi_build_operation(&ctx, &op) != CDD_C_SUCCESS) {
      }
      g_cdd_strdup_fail = 0;
      reset_operation_test(&op);
    }
  }

  /* Case 35: array param strdup failure loop */
  {
    struct OpBuilderContext ctx;
    struct C2OpenAPI_ParsedSig sig;
    struct C2OpenAPI_ParsedArg arg;
    struct OpenAPI_Operation op;
    int k;

    memset(&ctx, 0, sizeof(ctx));
    memset(&sig, 0, sizeof(sig));
    memset(&arg, 0, sizeof(arg));
    arg.name = (char *)(size_t) "items[]";
    arg.type = (char *)(size_t) "int";
    sig.args = &arg;
    sig.n_args = 1;
    ctx.sig = &sig;
    ctx.func_name = "api_user_get";

    for (k = 0; k <= 10; ++k) {
      memset(&op, 0, sizeof(op));
      g_cdd_strdup_fail = k;
      if (c2openapi_build_operation(&ctx, &op) != CDD_C_SUCCESS) {
      }
      g_cdd_strdup_fail = 0;
      reset_operation_test(&op);
    }

    arg.name = (char *)(size_t) "users[]";
    arg.type = (char *)(size_t) "struct User";
    for (k = 0; k <= 10; ++k) {
      memset(&op, 0, sizeof(op));
      g_cdd_strdup_fail = k;
      if (c2openapi_build_operation(&ctx, &op) != CDD_C_SUCCESS) {
      }
      g_cdd_strdup_fail = 0;
      reset_operation_test(&op);
    }
  }

  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_OPERATION_COVERAGE_PART3_H */
