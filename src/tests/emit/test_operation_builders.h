/**
 * @file test_operation_builders.h
 * @brief Builder and OOM tests for operation generator.
 */

#ifndef TEST_OPERATION_BUILDERS_H
#define TEST_OPERATION_BUILDERS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "emit/test_operation_common.h"
/* clang-format on */

encs[1].name = (char *)(size_t) "prefix";
encs[1].kind = 1;

encs[2].name = (char *)(size_t) "items";
encs[2].kind = 2;

doc.request_bodies = rbs;
doc.n_request_bodies = 1;
doc.encodings = encs;
doc.n_encodings = 3;
doc.request_body_description = (char *)(size_t) "Upload payload";
doc.request_body_required_set = 1;
doc.request_body_required = 1;

rets[0].code = (char *)(size_t) "200";
rets[0].summary = (char *)(size_t) "Success summary";
rets[0].description = (char *)(size_t) "Success description";
rets[0].content_type = (char *)(size_t) "application/json";
rets[0].example = (char *)(size_t) "{\"status\": \"ok\"}";

rets[1].code = (char *)(size_t) "400";
rets[1].summary = (char *)(size_t) "Error summary";
rets[1].description = (char *)(size_t) "Error description";
rets[1].content_type = (char *)(size_t) "application/problem+json";
rets[1].example = (char *)(size_t) "{\"error\": \"bad request\"}";

doc.returns = rets;
doc.n_returns = 2;

headers[0].code = (char *)(size_t) "200";
headers[0].name = (char *)(size_t) "X-Upload-Rate";
headers[0].type = (char *)(size_t) "integer";

headers[1].code = (char *)(size_t) "404";
headers[1].name = (char *)(size_t) "X-Missing-Id";
headers[1].type = (char *)(size_t) "string";

doc.response_headers = headers;
doc.n_response_headers = 2;

links[0].code = (char *)(size_t) "200";
links[0].name = (char *)(size_t) "GetFile";
links[0].operation_id = (char *)(size_t) "api_file_get";

links[1].code = (char *)(size_t) "500";
links[1].name = (char *)(size_t) "SupportTicket";
links[1].operation_ref = (char *)(size_t) "#/components/links/Support";

doc.links = links;
doc.n_links = 2;

ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
ASSERT_STR_EQ("Upload payload", op.req_body_description);
ASSERT(op.n_responses >= 4);
reset_operation_test(&op);

doc.n_request_bodies = 0;
doc.request_bodies = NULL;
doc.n_encodings = 0;
doc.request_body_content_type = (char *)(size_t) "text/plain";
ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
ASSERT_STR_EQ("text/plain", op.req_body.content_type);
reset_operation_test(&op);

PASS();
}

TEST test_operation_c2openapi_build_operation_coverage_extensions(void) {
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct C2OpenAPI_ParsedArg args[4];
  struct DocMetadata doc;
  struct DocParam params[4];
  struct DocResponse rets[1];
  struct OpenAPI_Operation op;
  int k;

  memset(&ctx, 0, sizeof(ctx));
  memset(&sig, 0, sizeof(sig));
  memset(args, 0, sizeof(args));
  memset(&doc, 0, sizeof(doc));
  memset(params, 0, sizeof(params));
  memset(rets, 0, sizeof(rets));
  memset(&op, 0, sizeof(op));

  ctx.sig = &sig;
  ctx.doc = &doc;
  ctx.func_name = "api_user_patch";
  doc.verb = (char *)(size_t) "PATCH";

  sig.args = args;
  sig.n_args = 4;
  doc.params = params;
  doc.n_params = 4;

  args[0].name = (char *)(size_t) "in_patch";
  args[0].type = (char *)(size_t) "const struct User *";

  args[1].name = (char *)(size_t) "out";
  args[1].type = (char *)(size_t) "struct Output **";

  args[2].name = (char *)(size_t) "list";
  args[2].type = (char *)(size_t) "int[]";
  params[2].name = (char *)(size_t) "list";
  params[2].in_loc = (char *)(size_t) "query";
  params[2].format = (char *)(size_t) "int64";

  args[3].name = (char *)(size_t) "X-Key";
  args[3].type = (char *)(size_t) "char *";
  params[3].name = (char *)(size_t) "X-Key";
  params[3].in_loc = (char *)(size_t) "header";
  params[3].example = (char *)(size_t) "key123";

  rets[0].code = (char *)(size_t) "200";
  rets[0].description = (char *)(size_t) "User updated successfully";
  doc.returns = rets;
  doc.n_returns = 1;

  ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
  reset_operation_test(&op);

  for (k = 1; k <= 30; ++k) {
    memset(&op, 0, sizeof(op));
    g_cdd_strdup_fail = k;
    if (c2openapi_build_operation(&ctx, &op) != CDD_C_SUCCESS) {
    }
    g_cdd_strdup_fail = 0;
    reset_operation_test(&op);
  }

  sig.n_args = 0;
  doc.n_returns = 0;
  doc.returns = NULL;
  doc.n_params = 0;
  doc.params = NULL;
  memset(&op, 0, sizeof(op));
  ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
  ASSERT_EQ(1, op.n_responses);
  ASSERT_STR_EQ("200", op.responses[0].code);
  reset_operation_test(&op);

  memset(&op, 0, sizeof(op));
  g_cdd_strdup_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, c2openapi_build_operation(&ctx, &op));
  g_cdd_strdup_fail = 0;
  reset_operation_test(&op);

  memset(&op, 0, sizeof(op));
  g_cdd_strdup_fail = 2;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, c2openapi_build_operation(&ctx, &op));
  g_cdd_strdup_fail = 0;
  reset_operation_test(&op);

  PASS();
}

TEST test_operation_c2openapi_build_operation_mega_oom(void) {
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct C2OpenAPI_ParsedArg args[6];
  struct DocMetadata doc;
  struct DocParam params[6];
  struct DocSecurityRequirement sec;
  struct DocServer srv;
  struct DocServerVar svar;
  struct DocRequestBody rbs[1];
  struct DocEncoding encs[3];
  struct DocResponse rets[2];
  struct DocResponseHeader hdrs[2];
  struct DocLink links[2];
  struct OpenAPI_Operation op;
  char *tags[2];
  char *scopes[2];
  char *enums[2];
  int k;

  memset(&ctx, 0, sizeof(ctx));
  memset(&sig, 0, sizeof(sig));
  memset(args, 0, sizeof(args));
  memset(&doc, 0, sizeof(doc));
  memset(params, 0, sizeof(params));
  memset(&sec, 0, sizeof(sec));
  memset(&srv, 0, sizeof(srv));
  memset(&svar, 0, sizeof(svar));
  memset(rbs, 0, sizeof(rbs));
  memset(encs, 0, sizeof(encs));
  memset(rets, 0, sizeof(rets));
  memset(hdrs, 0, sizeof(hdrs));
  memset(links, 0, sizeof(links));
  memset(&op, 0, sizeof(op));

  ctx.sig = &sig;
  ctx.doc = &doc;
  ctx.func_name = "api_mega_post";

  doc.operation_id = (char *)(size_t) "megaPost";
  doc.summary = (char *)(size_t) "Mega summary";
  doc.description = (char *)(size_t) "Mega description";
  doc.external_docs_url = (char *)(size_t) "https://docs.example.com";
  doc.external_docs_description = (char *)(size_t) "Docs";

  tags[0] = (char *)(size_t) "Tag1";
  tags[1] = (char *)(size_t) "Tag2";
  doc.tags = tags;
  doc.n_tags = 2;

  scopes[0] = (char *)(size_t) "read";
  scopes[1] = (char *)(size_t) "write";
  sec.scheme = (char *)(size_t) "Bearer";
  sec.scopes = scopes;
  sec.n_scopes = 2;
  doc.security = &sec;
  doc.n_security = 1;

  enums[0] = (char *)(size_t) "v1";
  enums[1] = (char *)(size_t) "v2";
  svar.name = (char *)(size_t) "env";
  svar.default_value = (char *)(size_t) "v1";
  svar.enum_values = enums;
  svar.n_enum_values = 2;
  srv.url = (char *)(size_t) "https://api.example.com";
  srv.name = (char *)(size_t) "Server1";
  srv.description = (char *)(size_t) "Primary";
  srv.variables = &svar;
  srv.n_variables = 1;
  doc.servers = &srv;
  doc.n_servers = 1;

  sig.args = args;
  sig.n_args = 6;
  doc.params = params;
  doc.n_params = 6;

  args[0].name = (char *)(size_t) "body_in";
  args[0].type = (char *)(size_t) "const struct Input *";

  args[1].name = (char *)(size_t) "data_out";
  args[1].type = (char *)(size_t) "struct Output **";

  args[2].name = (char *)(size_t) "num_list";
  args[2].type = (char *)(size_t) "int[]";
  params[2].name = (char *)(size_t) "num_list";
  params[2].in_loc = (char *)(size_t) "query";
  params[2].format = (char *)(size_t) "int32";

  args[3].name = (char *)(size_t) "query_str";
  args[3].type = (char *)(size_t) "char *";
  params[3].name = (char *)(size_t) "query_str";
  params[3].in_loc = (char *)(size_t) "querystring";

  args[4].name = (char *)(size_t) "sess_id";
  args[4].type = (char *)(size_t) "char *";
  params[4].name = (char *)(size_t) "sess_id";
  params[4].in_loc = (char *)(size_t) "cookie";

  args[5].name = (char *)(size_t) "X-Custom-Hdr";
  args[5].type = (char *)(size_t) "char *";
  params[5].name = (char *)(size_t) "X-Custom-Hdr";
  params[5].in_loc = (char *)(size_t) "header";
  params[5].content_type = (char *)(size_t) "text/plain";
  params[5].example = (char *)(size_t) "hdr_val";

  rbs[0].content_type = (char *)(size_t) "application/json";
  rbs[0].example = (char *)(size_t) "{\"id\": 1}";
  doc.request_bodies = rbs;
  doc.n_request_bodies = 1;

  encs[0].name = (char *)(size_t) "avatar";
  encs[0].content_type = (char *)(size_t) "image/png";
  encs[0].kind = 0;
  doc.encodings = encs;
  doc.n_encodings = 1;

  rets[0].code = (char *)(size_t) "200";
  rets[0].summary = (char *)(size_t) "Ok";
  rets[0].description = (char *)(size_t) "Success desc";
  rets[0].content_type = (char *)(size_t) "application/json";
  rets[0].example = (char *)(size_t) "{\"res\": 1}";

  rets[1].code = (char *)(size_t) "404";
  rets[1].summary = (char *)(size_t) "NotFound";
  rets[1].description = (char *)(size_t) "NotFound desc";
  rets[1].content_type = (char *)(size_t) "application/problem+json";
  rets[1].example = (char *)(size_t) "{\"err\": 1}";
  doc.returns = rets;
  doc.n_returns = 2;

  hdrs[0].code = (char *)(size_t) "200";
  hdrs[0].name = (char *)(size_t) "X-Rate-Limit";
  hdrs[0].type = (char *)(size_t) "integer";

  hdrs[1].code = (char *)(size_t) "400";
  hdrs[1].name = (char *)(size_t) "X-Bad-Request";
  hdrs[1].type = (char *)(size_t) "string";
  doc.response_headers = hdrs;
  doc.n_response_headers = 2;

  links[0].code = (char *)(size_t) "200";
  links[0].name = (char *)(size_t) "GetDetails";
  links[0].operation_id = (char *)(size_t) "getDetailsOp";

  links[1].code = (char *)(size_t) "500";
  links[1].name = (char *)(size_t) "ErrorReport";
  links[1].operation_ref = (char *)(size_t) "#/components/links/Error";
  doc.links = links;
  doc.n_links = 2;

  ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
  reset_operation_test(&op);

  for (k = 1; k <= 70; ++k) {
    memset(&op, 0, sizeof(op));
    g_cdd_strdup_fail = k;
    if (c2openapi_build_operation(&ctx, &op) != CDD_C_SUCCESS) {
    }
    g_cdd_strdup_fail = 0;
    reset_operation_test(&op);
  }

  for (k = 1; k <= 40; ++k) {
    memset(&op, 0, sizeof(op));
    g_cdd_alloc_fail = k;
    if (c2openapi_build_operation(&ctx, &op) != CDD_C_SUCCESS) {
    }
    g_cdd_alloc_fail = 0;
    reset_operation_test(&op);
  }

  PASS();
}

TEST test_operation_c2openapi_build_operation_oom(void) {
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct C2OpenAPI_ParsedArg args[2];
  struct DocMetadata doc;
  struct OpenAPI_Operation op;
  int i;

  memset(&ctx, 0, sizeof(ctx));
  memset(&sig, 0, sizeof(sig));
  memset(args, 0, sizeof(args));
  memset(&doc, 0, sizeof(doc));
  memset(&op, 0, sizeof(op));

  ctx.sig = &sig;
  ctx.doc = &doc;
  ctx.func_name = "api_user_get";
  doc.operation_id = (char *)(size_t) "getUser";
  doc.summary = (char *)(size_t) "Get user summary";
  doc.description = (char *)(size_t) "Get user desc";
  doc.external_docs_url = (char *)(size_t) "https://docs.example.com";
  doc.external_docs_description = (char *)(size_t) "Docs";

  sig.args = args;
  sig.n_args = 2;
  args[0].name = (char *)(size_t) "id";
  args[0].type = (char *)(size_t) "int";
  args[1].name = (char *)(size_t) "out";
  args[1].type = (char *)(size_t) "struct User **";

  for (i = 1; i <= 25; ++i) {
    memset(&op, 0, sizeof(op));
    g_cdd_strdup_fail = i;
    if (c2openapi_build_operation(&ctx, &op) != CDD_C_SUCCESS) {
    }
    g_cdd_strdup_fail = 0;
    reset_operation_test(&op);
  }

  PASS();
}

TEST test_operation_final_gaps(void) {
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct C2OpenAPI_ParsedArg args[4];
  struct DocMetadata doc;
  struct DocResponse rets[4];
  struct DocResponseHeader hdrs[1];
  struct DocLink links[1];
  struct OpenAPI_Operation op;

  memset(&ctx, 0, sizeof(ctx));
  memset(&sig, 0, sizeof(sig));
  memset(args, 0, sizeof(args));
  memset(&doc, 0, sizeof(doc));
  memset(rets, 0, sizeof(rets));
  memset(hdrs, 0, sizeof(hdrs));
  memset(links, 0, sizeof(links));
  memset(&op, 0, sizeof(op));

  ctx.sig = &sig;
  ctx.doc = &doc;
  ctx.func_name = "api_test_final";

  sig.args = args;
  sig.n_args = 4;

  args[0].name = (char *)(size_t) "list[]";
  args[0].type = (char *)(size_t) "int";

  args[1].name = (char *)(size_t) "users[]";
  args[1].type = (char *)(size_t) "struct User";

  args[2].name = (char *)(size_t) "single_user";
  args[2].type = (char *)(size_t) "struct User";

  args[3].name = (char *)(size_t) "out_val";
  args[3].type = (char *)(size_t) "int **";

  rets[0].code = (char *)(size_t) "200";
  rets[0].description = NULL;

  rets[1].code = (char *)(size_t) "200";
  rets[1].description = (char *)(size_t) "Merged desc";

  rets[2].code = (char *)(size_t) "404";
  rets[2].description = NULL;

  rets[3].code = (char *)(size_t) "500";
  rets[3].description = NULL;

  doc.returns = rets;
  doc.n_returns = 4;

  hdrs[0].code = (char *)(size_t) "404";
  hdrs[0].name = (char *)(size_t) "X-Missing";
  hdrs[0].type = (char *)(size_t) "string";
  doc.response_headers = hdrs;
  doc.n_response_headers = 1;

  links[0].code = (char *)(size_t) "500";
  links[0].name = (char *)(size_t) "L500";
  links[0].operation_id = (char *)(size_t) "op500";
  doc.links = links;
  doc.n_links = 1;

  ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx, &op));
  reset_operation_test(&op);

  /* Test path param doc override & request body content type override without
   * request bodies */
  {
    struct OpBuilderContext ctx2;
    struct C2OpenAPI_ParsedSig sig2;
    struct C2OpenAPI_ParsedArg args2[1];
    struct DocMetadata doc2;
    struct DocParam dp2;
    struct OpenAPI_Operation op2;

    memset(&ctx2, 0, sizeof(ctx2));
    memset(&sig2, 0, sizeof(sig2));
    memset(&doc2, 0, sizeof(doc2));
    memset(&dp2, 0, sizeof(dp2));
    memset(&op2, 0, sizeof(op2));

    ctx2.sig = &sig2;
    ctx2.doc = &doc2;
    ctx2.func_name = "api_path_test";

    sig2.args = args2;
    sig2.n_args = 1;
    args2[0].name = (char *)(size_t) "user_id";
    args2[0].type = (char *)(size_t) "int";

    dp2.name = (char *)(size_t) "user_id";
    dp2.in_loc = (char *)(size_t) "path";
    dp2.content_type = (char *)(size_t) "text/plain";
    doc2.params = &dp2;
    doc2.n_params = 1;

    doc2.request_body_content_type = (char *)(size_t) "application/xml";
    doc2.request_body_required_set = 1;
    doc2.request_body_required = 1;

    ASSERT_EQ(CDD_C_SUCCESS, c2openapi_build_operation(&ctx2, &op2));
    reset_operation_test(&op2);
  }

  PASS();
}

TEST test_operation_all_null_and_boundary_checks(void) {
  struct OpenAPI_Any any_val;
  struct OpenAPI_LinkParam *lp = NULL;
  size_t count = 0;
  int bool_out = 0;
  struct OpenAPI_Server srv;
  struct DocServer doc_srv;
  struct OpenAPI_Operation op;
  struct OpenAPI_Response resp;
  struct OpenAPI_Response *resp_ptr = NULL;
  struct OpenAPI_MediaType mt;
  struct OpenAPI_MediaType *mt_ptr = NULL;
  struct OpenAPI_Parameter param;
  struct OpenApiTypeMapping tm;
  struct OpenAPI_SchemaRef schema;
  JSON_Value *jv = NULL;

  memset(&any_val, 0, sizeof(any_val));
  memset(&srv, 0, sizeof(srv));
  memset(&doc_srv, 0, sizeof(doc_srv));
  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));
  memset(&mt, 0, sizeof(mt));
  memset(&param, 0, sizeof(param));
  memset(&tm, 0, sizeof(tm));
  memset(&schema, 0, sizeof(schema));

  /* parse_example_any */
  ASSERT_EQ(CDD_C_SUCCESS, parse_example_any(NULL, &any_val));
  ASSERT_EQ(CDD_C_SUCCESS, parse_example_any("ex", NULL));
  ASSERT_EQ(CDD_C_SUCCESS, parse_example_any(NULL, NULL));

  /* any_from_json_value */
  jv = json_parse_string("123");
  ASSERT_EQ(CDD_C_SUCCESS, any_from_json_value(NULL, &any_val));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, any_from_json_value(jv, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, any_from_json_value(NULL, NULL));
  json_value_free(jv);

  /* parse_link_params_json */
  ASSERT_EQ(CDD_C_SUCCESS, parse_link_params_json(NULL, &lp, &count));
  ASSERT_EQ(CDD_C_SUCCESS, parse_link_params_json("{}", NULL, &count));
  ASSERT_EQ(CDD_C_SUCCESS, parse_link_params_json("{}", &lp, NULL));

  /* is_path_param */
  ASSERT_EQ(CDD_C_SUCCESS, is_path_param(NULL, "id", &bool_out));
  ASSERT_EQ(CDD_C_SUCCESS, is_path_param("/user/{id}", NULL, &bool_out));
  ASSERT_EQ(CDD_C_SUCCESS, is_path_param("/user/{id}", "id", NULL));

  /* copy_doc_server_variables_op */
  ASSERT_EQ(CDD_C_SUCCESS, copy_doc_server_variables_op(NULL, &doc_srv));
  ASSERT_EQ(CDD_C_SUCCESS, copy_doc_server_variables_op(&srv, NULL));
  doc_srv.n_variables = 0;
  ASSERT_EQ(CDD_C_SUCCESS, copy_doc_server_variables_op(&srv, &doc_srv));

  /* find_response_by_code */
  ASSERT_EQ(CDD_C_SUCCESS, find_response_by_code(NULL, "200", &resp_ptr));
  ASSERT_EQ(CDD_C_SUCCESS, find_response_by_code(&op, NULL, &resp_ptr));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            find_response_by_code(&op, "200", NULL));

  /* find_media_type_op */
  ASSERT_EQ(CDD_C_SUCCESS, find_media_type_op(NULL, 0, "app/json", &mt_ptr));
  ASSERT_EQ(CDD_C_SUCCESS, find_media_type_op(&mt, 0, NULL, &mt_ptr));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            find_media_type_op(&mt, 0, "app/json", NULL));

  /* apply_example_to_media_type */
  ASSERT_EQ(CDD_C_SUCCESS, apply_example_to_media_type(NULL, "ex"));
  ASSERT_EQ(CDD_C_SUCCESS, apply_example_to_media_type(&mt, NULL));

  /* apply_example_to_response */
  ASSERT_EQ(CDD_C_SUCCESS, apply_example_to_response(NULL, "ex", NULL));
  ASSERT_EQ(CDD_C_SUCCESS, apply_example_to_response(&resp, NULL, NULL));

  /* ensure_response_for_code */
  ASSERT_EQ(CDD_C_SUCCESS, ensure_response_for_code(NULL, "200", &resp_ptr));
  ASSERT_EQ(CDD_C_SUCCESS, ensure_response_for_code(&op, NULL, &resp_ptr));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            ensure_response_for_code(&op, "200", NULL));

  /* add_param_to_op */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, add_param_to_op(NULL, &param));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, add_param_to_op(&op, NULL));

  /* copy_schema_ref_basic */
  ASSERT_EQ(CDD_C_SUCCESS, copy_schema_ref_basic(NULL, &schema));
  ASSERT_EQ(CDD_C_SUCCESS, copy_schema_ref_basic(&schema, NULL));

  /* set_querystring_schema_from_type_map */
  ASSERT_EQ(CDD_C_SUCCESS, set_querystring_schema_from_type_map(NULL, &tm));
  ASSERT_EQ(CDD_C_SUCCESS, set_querystring_schema_from_type_map(&param, NULL));

  /* apply_format_to_schema_ref */
  ASSERT_EQ(CDD_C_SUCCESS,
            apply_format_to_schema_ref(NULL, &tm, "fmt", &bool_out));
  ASSERT_EQ(CDD_C_SUCCESS,
            apply_format_to_schema_ref(&schema, NULL, "fmt", &bool_out));
  ASSERT_EQ(CDD_C_SUCCESS,
            apply_format_to_schema_ref(&schema, &tm, "fmt", NULL));

  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_OPERATION_BUILDERS_H */
