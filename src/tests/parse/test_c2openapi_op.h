/**
 * @file test_c2openapi_op.h
 * @brief Unit tests for the Operation Builder.
 */

#ifndef TEST_C2OPENAPI_OP_H
#define TEST_C2OPENAPI_OP_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "parse/test_c2openapi_op_helpers.h"

TEST test_build_simple_get(void) {
  /*
   * Case: int api_user_get(int id);
   * Doc: @route GET /user/{id}
   */
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct C2OpenAPI_ParsedArg args[1];
  struct DocMetadata doc;
  struct OpenAPI_Operation op;
  int rc;

  /* Setup Signature */
  (void)rc;
  memset(args, 0, sizeof(args));
  memset(&op, 0, sizeof(op));
  sig.name = (char *)(size_t)(size_t) "api_user_get";
  sig.n_args = 1;
  sig.args = args;
  args[0].name = (char *)(size_t)(size_t) "id";
  args[0].type = (char *)(size_t)(size_t) "int";

  /* Setup Doc */
  memset(&doc, 0, sizeof(doc));
  doc.route = strdup("/user/{id}");
  doc.verb = strdup("GET");
  doc.summary = (char *)(size_t)(size_t) "Get a user";

  /* Setup Context */
  memset(&ctx, 0, sizeof(ctx));
  ctx.sig = &sig;
  ctx.doc = &doc;
  ctx.func_name = sig.name;

  rc = c2openapi_build_operation(&ctx, &op);
  ASSERT_EQ(0, rc);

  /* Verify Basic */
  ASSERT_EQ(OA_VERB_GET, op.verb);
  ASSERT_STR_EQ("api_user_get", op.operation_id);
  ASSERT_STR_EQ("Get a user", op.summary);

  /* Verify Parameter */
  ASSERT_EQ(1, op.n_parameters);
  ASSERT_STR_EQ("id", op.parameters[0].name);
  ASSERT_EQ(OA_PARAM_IN_PATH, op.parameters[0].in);
  ASSERT(op.parameters[0].required);
  ASSERT_STR_EQ("integer", op.parameters[0].type);

  free(doc.route);
  free(doc.verb);
  reset_op(&op);
  g_fail_io_after = -1;
  PASS();
}

TEST test_build_param_format_from_mapping(void) {
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct C2OpenAPI_ParsedArg args[1];
  struct DocMetadata doc;
  struct OpenAPI_Operation op;
  int rc;

  (void)rc;
  memset(args, 0, sizeof(args));
  memset(&op, 0, sizeof(op));
  sig.name = (char *)(size_t)(size_t) "api_user_get";
  sig.n_args = 1;
  sig.args = args;
  args[0].name = (char *)(size_t)(size_t) "id";
  args[0].type = (char *)(size_t)(size_t) "long";

  memset(&doc, 0, sizeof(doc));
  doc.route = strdup("/user/{id}");
  doc.verb = strdup("GET");

  memset(&ctx, 0, sizeof(ctx));
  ctx.sig = &sig;
  ctx.doc = &doc;
  ctx.func_name = sig.name;

  rc = c2openapi_build_operation(&ctx, &op);
  ASSERT_EQ(0, rc);

  ASSERT_EQ(1, op.n_parameters);
  ASSERT(op.parameters[0].schema_set);
  ASSERT_STR_EQ("integer", op.parameters[0].schema.inline_type);
  ASSERT_STR_EQ("int64", op.parameters[0].schema.format);

  free(doc.route);
  free(doc.verb);
  reset_op(&op);
  g_fail_io_after = -1;
  PASS();
}

TEST test_build_param_format_override(void) {
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct C2OpenAPI_ParsedArg args[1];
  struct DocMetadata doc;
  struct DocParam *params = NULL;
  struct OpenAPI_Operation op;
  int rc;

  (void)rc;
  memset(args, 0, sizeof(args));
  memset(&op, 0, sizeof(op));
  sig.name = (char *)(size_t)(size_t) "api_user_get";
  sig.n_args = 1;
  sig.args = args;
  args[0].name = (char *)(size_t)(size_t) "id";
  args[0].type = (char *)(size_t)(size_t) "int";

  memset(&doc, 0, sizeof(doc));
  doc.route = strdup("/user/{id}");
  doc.verb = strdup("GET");
  params = (struct DocParam *)calloc(1, sizeof(struct DocParam));
  ASSERT(params != NULL);
  params[0].name = strdup("id");
  params[0].format = strdup("int64");
  doc.params = params;
  doc.n_params = 1;

  memset(&ctx, 0, sizeof(ctx));
  ctx.sig = &sig;
  ctx.doc = &doc;
  ctx.func_name = sig.name;

  rc = c2openapi_build_operation(&ctx, &op);
  ASSERT_EQ(0, rc);

  ASSERT_EQ(1, op.n_parameters);
  ASSERT(op.parameters[0].schema_set);
  ASSERT_STR_EQ("int64", op.parameters[0].schema.format);

  reset_op(&op);
  doc_metadata_free(&doc);
  g_fail_io_after = -1;
  PASS();
}

TEST test_build_response_header_format(void) {
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct DocMetadata doc;
  struct DocResponseHeader *headers = NULL;
  struct OpenAPI_Operation op;
  int rc;

  (void)rc;
  memset(&op, 0, sizeof(op));
  sig.name = (char *)(size_t)(size_t) "api_ping";
  sig.n_args = 0;
  sig.args = NULL;

  memset(&doc, 0, sizeof(doc));
  doc.route = strdup("/ping");
  doc.verb = strdup("GET");
  headers = (struct DocResponseHeader *)calloc(1, sizeof(*headers));
  ASSERT(headers != NULL);
  headers[0].code = strdup("200");
  headers[0].name = strdup("X-Rate");
  headers[0].type = strdup("integer");
  headers[0].format = strdup("int64");
  headers[0].description = strdup("Rate limit");
  doc.response_headers = headers;
  doc.n_response_headers = 1;

  memset(&ctx, 0, sizeof(ctx));
  ctx.sig = &sig;
  ctx.doc = &doc;
  ctx.func_name = sig.name;

  rc = c2openapi_build_operation(&ctx, &op);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(1, op.n_responses);
  ASSERT_EQ(1, op.responses[0].n_headers);
  ASSERT(op.responses[0].headers[0].schema_set);
  ASSERT_STR_EQ("int64", op.responses[0].headers[0].schema.format);

  reset_op(&op);
  doc_metadata_free(&doc);
  g_fail_io_after = -1;
  PASS();
}

TEST test_build_default_response_when_missing(void) {
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct DocMetadata doc;
  struct OpenAPI_Operation op;
  int rc;

  (void)rc;
  memset(&op, 0, sizeof(op));
  sig.name = (char *)(size_t)(size_t) "api_ping";
  sig.n_args = 0;
  sig.args = NULL;

  memset(&doc, 0, sizeof(doc));
  doc.route = strdup("/ping");
  doc.verb = strdup("GET");

  memset(&ctx, 0, sizeof(ctx));
  ctx.sig = &sig;
  ctx.doc = &doc;
  ctx.func_name = sig.name;

  rc = c2openapi_build_operation(&ctx, &op);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(1, op.n_responses);
  ASSERT_STR_EQ("200", op.responses[0].code);
  ASSERT_STR_EQ("Success", op.responses[0].description);

  free(doc.route);
  free(doc.verb);
  reset_op(&op);
  g_fail_io_after = -1;
  PASS();
}

TEST test_build_operation_id_override(void) {
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct C2OpenAPI_ParsedArg args[1];
  struct DocMetadata doc;
  struct OpenAPI_Operation op;
  int rc;

  (void)rc;
  memset(args, 0, sizeof(args));
  memset(&op, 0, sizeof(op));
  sig.name = (char *)(size_t)(size_t) "api_user_get";
  sig.n_args = 1;
  sig.args = args;
  args[0].name = (char *)(size_t)(size_t) "id";
  args[0].type = (char *)(size_t)(size_t) "int";

  memset(&doc, 0, sizeof(doc));
  doc.route = strdup("/user/{id}");
  doc.verb = strdup("GET");
  doc.operation_id = (char *)(size_t)(size_t) "getUserById";

  memset(&ctx, 0, sizeof(ctx));
  ctx.sig = &sig;
  ctx.doc = &doc;
  ctx.func_name = sig.name;

  rc = c2openapi_build_operation(&ctx, &op);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("getUserById", op.operation_id);

  free(doc.route);
  free(doc.verb);
  reset_op(&op);
  g_fail_io_after = -1;
  PASS();
}

TEST test_build_param_content_type(void) {
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct C2OpenAPI_ParsedArg args[1];
  struct DocMetadata doc;
  struct DocParam params[1];
  struct OpenAPI_Operation op;
  int rc;

  (void)rc;
  memset(args, 0, sizeof(args));
  memset(params, 0, sizeof(params));
  memset(&op, 0, sizeof(op));
  sig.name = (char *)(size_t)(size_t) "api_user_search";
  sig.n_args = 1;
  sig.args = args;
  args[0].name = (char *)(size_t)(size_t) "payload";
  args[0].type = (char *)(size_t)(size_t) "const char *";

  memset(&doc, 0, sizeof(doc));
  memset(params, 0, sizeof(params));
  doc.route = strdup("/user/search");
  doc.verb = strdup("GET");
  doc.params = params;
  doc.n_params = 1;
  doc.params[0].name = (char *)(size_t)(size_t) "payload";
  doc.params[0].in_loc = (char *)(size_t)(size_t) "query";
  doc.params[0].content_type = (char *)(size_t)(size_t) "application/json";

  memset(&ctx, 0, sizeof(ctx));
  ctx.sig = &sig;
  ctx.doc = &doc;
  ctx.func_name = sig.name;

  rc = c2openapi_build_operation(&ctx, &op);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(1, op.n_parameters);
  ASSERT_STR_EQ("application/json", op.parameters[0].content_type);

  free(doc.route);
  free(doc.verb);
  reset_op(&op);
  g_fail_io_after = -1;
  PASS();
}

TEST test_build_param_example(void) {
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct C2OpenAPI_ParsedArg args[1];
  struct DocMetadata doc;
  struct DocParam params[1];
  struct OpenAPI_Operation op;
  int rc;

  (void)rc;
  memset(args, 0, sizeof(args));
  memset(params, 0, sizeof(params));
  memset(&op, 0, sizeof(op));
  sig.name = (char *)(size_t)(size_t) "api_user_get";
  sig.n_args = 1;
  sig.args = args;
  args[0].name = (char *)(size_t)(size_t) "id";
  args[0].type = (char *)(size_t)(size_t) "int";

  memset(&doc, 0, sizeof(doc));
  memset(params, 0, sizeof(params));
  doc.route = strdup("/user/{id}");
  doc.verb = strdup("GET");
  doc.params = params;
  doc.n_params = 1;
  doc.params[0].name = (char *)(size_t)(size_t) "id";
  doc.params[0].in_loc = (char *)(size_t)(size_t) "path";
  doc.params[0].example = (char *)(size_t)(size_t) "123";

  memset(&ctx, 0, sizeof(ctx));
  ctx.sig = &sig;
  ctx.doc = &doc;
  ctx.func_name = sig.name;

  rc = c2openapi_build_operation(&ctx, &op);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(1, op.n_parameters);
  ASSERT(op.parameters[0].example_set);
  ASSERT_EQ(OA_ANY_NUMBER, op.parameters[0].example.type);
  ASSERT_EQ(OA_EXAMPLE_LOC_OBJECT, op.parameters[0].example_location);

  free(doc.route);
  free(doc.verb);
  reset_op(&op);
  g_fail_io_after = -1;
  PASS();
}

TEST test_build_return_content_type(void) {
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct DocMetadata doc;
  struct DocResponse returns[1];
  struct OpenAPI_Operation op;
  int rc;

  (void)rc;
  memset(returns, 0, sizeof(returns));
  memset(&op, 0, sizeof(op));
  sig.name = (char *)(size_t)(size_t) "api_status";
  sig.n_args = 0;
  sig.args = NULL;

  memset(&doc, 0, sizeof(doc));
  memset(returns, 0, sizeof(returns));
  doc.route = (char *)(size_t)(size_t) "/status";
  doc.verb = strdup("GET");
  doc.returns = returns;
  doc.n_returns = 1;
  doc.returns[0].code = (char *)(size_t)(size_t) "200";
  doc.returns[0].summary = (char *)(size_t)(size_t) "Status";
  doc.returns[0].description = (char *)(size_t)(size_t) "OK";
  doc.returns[0].content_type = (char *)(size_t)(size_t) "text/plain";

  memset(&ctx, 0, sizeof(ctx));
  ctx.sig = &sig;
  ctx.doc = &doc;
  ctx.func_name = sig.name;

  rc = c2openapi_build_operation(&ctx, &op);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(1, op.n_responses);
  ASSERT_STR_EQ("Status", op.responses[0].summary);
  ASSERT_STR_EQ("text/plain", op.responses[0].content_type);

  free(doc.verb);
  reset_op(&op);
  g_fail_io_after = -1;
  PASS();
}

TEST test_build_response_example(void) {
  /* */
  /* */

  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct C2OpenAPI_ParsedArg args[1];
  struct DocMetadata doc;
  struct DocResponse returns[1];
  struct OpenAPI_Operation op;
  int rc;

  (void)rc;
  memset(args, 0, sizeof(args));
  memset(returns, 0, sizeof(returns));
  memset(&op, 0, sizeof(op));
  sig.name = (char *)(size_t)(size_t) "api_user_get";
  sig.n_args = 1;
  sig.args = args;
  args[0].name = (char *)(size_t)(size_t) "id";
  args[0].type = (char *)(size_t)(size_t) "int";

  memset(&doc, 0, sizeof(doc));
  memset(returns, 0, sizeof(returns));
  doc.route = strdup("/user/{id}");
  doc.verb = strdup("GET");
  doc.returns = returns;
  doc.n_returns = 1;
  doc.returns[0].code = (char *)(size_t)(size_t) "200";
  doc.returns[0].description = (char *)(size_t)(size_t) "OK";
  doc.returns[0].content_type = (char *)(size_t)(size_t) "application/json";
  doc.returns[0].example = (char *)(size_t)(size_t) "{\"ok\":true}";

  memset(&ctx, 0, sizeof(ctx));
  ctx.sig = &sig;
  ctx.doc = &doc;
  ctx.func_name = sig.name;

  rc = c2openapi_build_operation(&ctx, &op);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(1, op.n_responses);
  ASSERT_EQ(1, op.responses[0].n_content_media_types);
  ASSERT(op.responses[0].content_media_types[0].example_set);
  ASSERT_EQ(OA_ANY_JSON, op.responses[0].content_media_types[0].example.type);

  free(doc.route);
  free(doc.verb);
  reset_op(&op);
  g_fail_io_after = -1;
  PASS();
}

TEST test_build_post_with_body(void) {
  /*
   * Case: int api_pet_create(const struct Pet *p);
   * Implicit POST from name.
   */
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct C2OpenAPI_ParsedArg args[1];
  struct OpenAPI_Operation op;
  int rc;

  /* Sig */
  (void)rc;
  memset(args, 0, sizeof(args));
  memset(&op, 0, sizeof(op));
  sig.name = (char *)(size_t)(size_t) "api_pet_create";
  sig.n_args = 1;
  sig.args = args;
  args[0].name = (char *)(size_t)(size_t) "p";
  args[0].type = (char *)(size_t)(size_t) "const struct Pet *";

  /* Doc (minimal) */
  ctx.sig = &sig;
  ctx.doc = NULL; /* No explicit doc to test implicit logic */
  ctx.func_name = sig.name;

  rc = c2openapi_build_operation(&ctx, &op);
  ASSERT_EQ(0, rc);

  /* Implicit Verb */
  ASSERT_EQ(OA_VERB_POST, op.verb);

  /* Parameter becomes Body */
  ASSERT_EQ(0, op.n_parameters); /* Should NOT be a parameter */
  ASSERT_STR_EQ("Pet", op.req_body.ref_name);
  ASSERT_STR_EQ("application/json", op.req_body.content_type);
  ASSERT_EQ(1, op.req_body_required_set);
  ASSERT_EQ(1, op.req_body_required);

  reset_op(&op);
  g_fail_io_after = -1;
  PASS();
}

TEST test_build_params_explicit(void) {
  /*
   * Case: int list(int limit);
   * Doc: @param limit [in:query]
   */
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct C2OpenAPI_ParsedArg args[1];
  struct DocMetadata doc;
  struct DocParam dparams[1];
  struct OpenAPI_Operation op;

  /* Sig */
  memset(args, 0, sizeof(args));
  memset(dparams, 0, sizeof(dparams));
  memset(&op, 0, sizeof(op));
  sig.name = (char *)(size_t)(size_t) "list";
  sig.n_args = 1;
  sig.args = args;
  args[0].name = (char *)(size_t)(size_t) "limit";
  args[0].type = (char *)(size_t)(size_t) "int";

  /* Doc */
  memset(&doc, 0, sizeof(doc));
  doc.params = dparams;
  doc.n_params = 1;
  dparams[0].name = (char *)(size_t)(size_t) "limit";
  dparams[0].in_loc = (char *)(size_t)(size_t) "query";
  dparams[0].description = (char *)(size_t)(size_t) "Max items";
  dparams[0].required = 0;

  ctx.sig = &sig;
  ctx.doc = &doc;
  ctx.func_name = sig.name;

  c2openapi_build_operation(&ctx, &op);

  ASSERT_EQ(1, op.n_parameters);
  ASSERT_STR_EQ("limit", op.parameters[0].name);
  ASSERT_EQ(OA_PARAM_IN_QUERY, op.parameters[0].in);
  /* Default required for query is 0 unless specified */
  ASSERT_EQ(0, op.parameters[0].required);

  reset_op(&op);
  g_fail_io_after = -1;
  PASS();
}

TEST test_build_param_style_flags(void) {
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct C2OpenAPI_ParsedArg args[1];
  struct DocMetadata doc;
  struct DocParam dparams[1];
  struct OpenAPI_Operation op;
  int rc;

  (void)rc;
  memset(args, 0, sizeof(args));
  memset(dparams, 0, sizeof(dparams));
  memset(&op, 0, sizeof(op));
  sig.name = (char *)(size_t)(size_t) "search";
  sig.n_args = 1;
  sig.args = args;
  args[0].name = (char *)(size_t)(size_t) "tags";
  args[0].type = (char *)(size_t)(size_t) "char **";

  memset(&doc, 0, sizeof(doc));
  memset(dparams, 0, sizeof(dparams));
  doc.params = dparams;
  doc.n_params = 1;
  dparams[0].name = (char *)(size_t)(size_t) "tags";
  dparams[0].in_loc = (char *)(size_t)(size_t) "query";
  dparams[0].style = DOC_PARAM_STYLE_SPACE_DELIMITED;
  dparams[0].style_set = 1;
  dparams[0].explode = 0;
  dparams[0].explode_set = 1;
  dparams[0].allow_reserved = 1;
  dparams[0].allow_reserved_set = 1;
  dparams[0].allow_empty_value = 1;
  dparams[0].allow_empty_value_set = 1;

  memset(&ctx, 0, sizeof(ctx));
  ctx.sig = &sig;
  ctx.doc = &doc;
  ctx.func_name = sig.name;

  rc = c2openapi_build_operation(&ctx, &op);
  ASSERT_EQ(0, rc);

  ASSERT_EQ(1, op.n_parameters);
  ASSERT_EQ(OA_STYLE_SPACE_DELIMITED, op.parameters[0].style);
  ASSERT_EQ(1, op.parameters[0].explode_set);
  ASSERT_EQ(0, op.parameters[0].explode);
  ASSERT_EQ(1, op.parameters[0].allow_reserved_set);
  ASSERT_EQ(1, op.parameters[0].allow_reserved);
  ASSERT_EQ(1, op.parameters[0].allow_empty_value_set);
  ASSERT_EQ(1, op.parameters[0].allow_empty_value);

  reset_op(&op);
  g_fail_io_after = -1;
  PASS();
}

TEST test_build_param_default_styles(void) {
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct C2OpenAPI_ParsedArg args[2];
  struct DocMetadata doc;
  struct DocParam dparams[1];
  struct OpenAPI_Operation op;
  int rc;

  (void)rc;
  memset(args, 0, sizeof(args));
  memset(dparams, 0, sizeof(dparams));
  memset(&op, 0, sizeof(op));
  sig.name = (char *)(size_t)(size_t) "get_item";
  sig.n_args = 2;
  sig.args = args;
  args[0].name = (char *)(size_t)(size_t) "id";
  args[0].type = (char *)(size_t)(size_t) "int";
  args[1].name = (char *)(size_t)(size_t) "token";
  args[1].type = (char *)(size_t)(size_t) "char *";

  memset(&doc, 0, sizeof(doc));
  doc.route = (char *)(size_t)(size_t) "/items/{id}";
  doc.params = dparams;
  doc.n_params = 1;
  memset(dparams, 0, sizeof(dparams));
  dparams[0].name = (char *)(size_t)(size_t) "token";
  dparams[0].in_loc = (char *)(size_t)(size_t) "header";

  memset(&ctx, 0, sizeof(ctx));
  ctx.sig = &sig;
  ctx.doc = &doc;
  ctx.func_name = sig.name;

  rc = c2openapi_build_operation(&ctx, &op);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(2, op.n_parameters);
  ASSERT_EQ(OA_PARAM_IN_PATH, op.parameters[0].in);
  ASSERT_EQ(OA_STYLE_SIMPLE, op.parameters[0].style);
  ASSERT_EQ(OA_PARAM_IN_HEADER, op.parameters[1].in);
  ASSERT_EQ(OA_STYLE_SIMPLE, op.parameters[1].style);

  reset_op(&op);
  g_fail_io_after = -1;
  PASS();
}

TEST test_build_reserved_header_param_ignored(void) {
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct C2OpenAPI_ParsedArg args[2];
  struct DocMetadata doc;
  struct DocParam dparams[1];
  struct OpenAPI_Operation op;
  int rc;

  (void)rc;
  memset(args, 0, sizeof(args));
  memset(dparams, 0, sizeof(dparams));
  memset(&op, 0, sizeof(op));
  sig.name = (char *)(size_t)(size_t) "get_item";
  sig.n_args = 2;
  sig.args = args;
  args[0].name = (char *)(size_t)(size_t) "id";
  args[0].type = (char *)(size_t)(size_t) "int";
  args[1].name = (char *)(size_t)(size_t) "Accept";
  args[1].type = (char *)(size_t)(size_t) "char *";

  memset(&doc, 0, sizeof(doc));
  doc.route = (char *)(size_t)(size_t) "/items/{id}";
  doc.params = dparams;
  doc.n_params = 1;
  memset(dparams, 0, sizeof(dparams));
  dparams[0].name = (char *)(size_t)(size_t) "Accept";
  dparams[0].in_loc = (char *)(size_t)(size_t) "header";

  memset(&ctx, 0, sizeof(ctx));
  ctx.sig = &sig;
  ctx.doc = &doc;
  ctx.func_name = sig.name;

  rc = c2openapi_build_operation(&ctx, &op);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(1, op.n_parameters);
  ASSERT_STR_EQ("id", op.parameters[0].name);
  ASSERT_EQ(OA_PARAM_IN_PATH, op.parameters[0].in);

  reset_op(&op);
  g_fail_io_after = -1;
  PASS();
}

TEST test_build_with_tags_description_and_deprecated(void) {
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct C2OpenAPI_ParsedArg args[1];
  struct DocMetadata doc;
  struct OpenAPI_Operation op;
  int rc;

  (void)rc;
  memset(args, 0, sizeof(args));
  memset(&op, 0, sizeof(op));
  sig.name = (char *)(size_t)(size_t) "api_user_list";
  sig.n_args = 1;
  sig.args = args;
  args[0].name = (char *)(size_t)(size_t) "limit";
  args[0].type = (char *)(size_t)(size_t) "int";

  memset(&doc, 0, sizeof(doc));
  doc.summary = (char *)(size_t)(size_t) "List users";
  doc.description = (char *)(size_t)(size_t) "Longer description text";
  doc.deprecated_set = 1;
  doc.deprecated = 1;
  doc.external_docs_url = (char *)(size_t)(size_t) "https://example.com/docs";
  doc.external_docs_description = (char *)(size_t)(size_t) "External docs";
  {
    static char *tags[] = {(char *)(size_t)(size_t) "users",
                           (char *)(size_t)(size_t) "admin"};
    doc.tags = tags;
    doc.n_tags = 2;
  }

  ctx.sig = &sig;
  ctx.doc = &doc;
  ctx.func_name = sig.name;

  rc = c2openapi_build_operation(&ctx, &op);
  ASSERT_EQ(0, rc);

  ASSERT_STR_EQ("List users", op.summary);
  ASSERT_STR_EQ("Longer description text", op.description);
  ASSERT_EQ(1, op.deprecated);
  ASSERT_EQ(2, op.n_tags);
  ASSERT_STR_EQ("users", op.tags[0]);
  ASSERT_STR_EQ("admin", op.tags[1]);
  ASSERT_STR_EQ("https://example.com/docs", op.external_docs.url);
  ASSERT_STR_EQ("External docs", op.external_docs.description);

  reset_op(&op);
  g_fail_io_after = -1;
  PASS();
}

TEST test_build_params_querystring(void) {
  /*
   * Case: int search(const char *qs);
   * Doc: @param qs [in:querystring]
   */
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct C2OpenAPI_ParsedArg args[1];
  struct DocMetadata doc;
  struct DocParam dparams[1];
  struct OpenAPI_Operation op;

  memset(args, 0, sizeof(args));
  memset(dparams, 0, sizeof(dparams));
  memset(&op, 0, sizeof(op));
  sig.name = (char *)(size_t)(size_t) "search";
  sig.n_args = 1;
  sig.args = args;
  args[0].name = (char *)(size_t)(size_t) "qs";
  args[0].type = (char *)(size_t)(size_t) "const char *";

  memset(&doc, 0, sizeof(doc));
  doc.params = dparams;
  doc.n_params = 1;
  dparams[0].name = (char *)(size_t)(size_t) "qs";
  dparams[0].in_loc = (char *)(size_t)(size_t) "querystring";
  dparams[0].description = (char *)(size_t)(size_t) "Serialized query string";

  ctx.sig = &sig;
  ctx.doc = &doc;
  ctx.func_name = sig.name;

  c2openapi_build_operation(&ctx, &op);

  ASSERT_EQ(1, op.n_parameters);
  ASSERT_STR_EQ("qs", op.parameters[0].name);
  ASSERT_EQ(OA_PARAM_IN_QUERYSTRING, op.parameters[0].in);
  ASSERT_STR_EQ("string", op.parameters[0].type);
  ASSERT_STR_EQ("application/x-www-form-urlencoded",
                op.parameters[0].content_type);

  reset_op(&op);
  g_fail_io_after = -1;
  PASS();
}

TEST test_build_params_querystring_json_struct(void) {
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct C2OpenAPI_ParsedArg args[1];
  struct DocMetadata doc;
  struct DocParam dparams[1];
  struct OpenAPI_Operation op;

  memset(args, 0, sizeof(args));
  memset(dparams, 0, sizeof(dparams));
  memset(&op, 0, sizeof(op));
  sig.name = (char *)(size_t)(size_t) "search_query";
  sig.n_args = 1;
  sig.args = args;
  args[0].name = (char *)(size_t)(size_t) "qs";
  args[0].type = (char *)(size_t)(size_t) "struct Query *";

  memset(&doc, 0, sizeof(doc));
  doc.params = dparams;
  doc.n_params = 1;
  dparams[0].name = (char *)(size_t)(size_t) "qs";
  dparams[0].in_loc = (char *)(size_t)(size_t) "querystring";
  dparams[0].content_type = (char *)(size_t)(size_t) "application/json";

  ctx.sig = &sig;
  ctx.doc = &doc;
  ctx.func_name = sig.name;

  c2openapi_build_operation(&ctx, &op);

  ASSERT_EQ(1, op.n_parameters);
  ASSERT_EQ(OA_PARAM_IN_QUERYSTRING, op.parameters[0].in);
  ASSERT_STR_EQ("application/json", op.parameters[0].content_type);
  ASSERT(op.parameters[0].schema_set);
  ASSERT_STR_EQ("Query", op.parameters[0].schema.ref_name);

  reset_op(&op);
  g_fail_io_after = -1;
  PASS();
}

SUITE(c2openapi_op_suite) {
  RUN_TEST(test_build_simple_get);
  RUN_TEST(test_build_param_format_from_mapping);
  RUN_TEST(test_build_param_format_override);
  RUN_TEST(test_build_response_header_format);
  RUN_TEST(test_build_default_response_when_missing);
  RUN_TEST(test_build_operation_id_override);
  RUN_TEST(test_build_param_content_type);
  RUN_TEST(test_build_param_example);
  RUN_TEST(test_build_return_content_type);
  RUN_TEST(test_build_response_example);
  RUN_TEST(test_build_post_with_body);
  RUN_TEST(test_build_params_explicit);
  RUN_TEST(test_build_param_style_flags);
  RUN_TEST(test_build_param_default_styles);
  RUN_TEST(test_build_reserved_header_param_ignored);
  RUN_TEST(test_build_with_tags_description_and_deprecated);
  RUN_TEST(test_build_params_querystring);
  RUN_TEST(test_build_params_querystring_json_struct);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_C2OPENAPI_OP_H */
