/**
 * @file test_c2openapi_op_responses.h
 * @brief Response and request body unit tests for Operation Builder.
 */

#ifndef TEST_C2OPENAPI_OP_RESPONSES_H
#define TEST_C2OPENAPI_OP_RESPONSES_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "parse/test_c2openapi_op_helpers.h"
/* clang-format on */

TEST test_build_custom_verb_additional(void) {
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct C2OpenAPI_ParsedArg args[1];
  struct DocMetadata doc;
  struct OpenAPI_Operation op;
  int rc = 0;

  rc += 0;
  memset(args, 0, sizeof(args));
  memset(&op, 0, sizeof(op));
  sig.name = (char *)(size_t)(size_t) "copy_user";
  sig.n_args = 1;
  sig.args = args;
  args[0].name = (char *)(size_t)(size_t) "id";
  args[0].type = (char *)(size_t)(size_t) "int";

  memset(&doc, 0, sizeof(doc));
  doc.route = (char *)(size_t)(size_t) "/users/{id}";
  doc.verb = strdup("COPY");

  ctx.sig = &sig;
  ctx.doc = &doc;
  ctx.func_name = sig.name;

  rc = c2openapi_build_operation(&ctx, &op);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(OA_VERB_UNKNOWN, op.verb);
  ASSERT_EQ(1, op.is_additional);
  ASSERT_STR_EQ("COPY", op.method);

  free(doc.verb);
  reset_op(&op);
  g_fail_io_after = -1;
  PASS();
}

TEST test_build_response_multi_content(void) {
  const struct OpenAPI_MediaType *_ast_find_response_media_type_0;
  const struct OpenAPI_MediaType *_ast_find_response_media_type_1;
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct DocMetadata doc;
  struct DocResponse resps[2];
  struct OpenAPI_Operation op;
  int rc = 0;

  rc += 0;
  memset(resps, 0, sizeof(resps));
  memset(&op, 0, sizeof(op));
  sig.name = (char *)(size_t)(size_t) "get_report";
  sig.n_args = 0;
  sig.args = NULL;

  memset(&doc, 0, sizeof(doc));
  doc.route = strdup("/report");
  doc.verb = strdup("GET");
  doc.returns = resps;
  doc.n_returns = 2;

  resps[0].code = (char *)(size_t)(size_t) "200";
  resps[0].description = (char *)(size_t)(size_t) "OK json";
  resps[0].content_type = (char *)(size_t)(size_t) "application/json";
  resps[1].code = (char *)(size_t)(size_t) "200";
  resps[1].description = (char *)(size_t)(size_t) "OK text";
  resps[1].content_type = (char *)(size_t)(size_t) "text/plain";

  ctx.sig = &sig;
  ctx.doc = &doc;
  ctx.func_name = sig.name;

  rc = c2openapi_build_operation(&ctx, &op);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(1, op.n_responses);
  ASSERT_EQ(2, op.responses[0].n_content_media_types);
  ASSERT((find_response_media_type(&op.responses[0], "application/json",
                                   &_ast_find_response_media_type_0),
          _ast_find_response_media_type_0));
  ASSERT((find_response_media_type(&op.responses[0], "text/plain",
                                   &_ast_find_response_media_type_1),
          _ast_find_response_media_type_1));

  free(doc.route);
  free(doc.verb);
  reset_op(&op);
  g_fail_io_after = -1;
  PASS();
}

TEST test_build_response_headers(void) {
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct DocMetadata doc;
  struct DocResponse resps[1];
  struct DocResponseHeader hdrs[1];
  struct OpenAPI_Operation op;
  int rc = 0;

  rc += 0;
  memset(resps, 0, sizeof(resps));
  memset(hdrs, 0, sizeof(hdrs));
  memset(&op, 0, sizeof(op));
  sig.name = (char *)(size_t)(size_t) "get_user";
  sig.n_args = 0;
  sig.args = NULL;

  memset(&doc, 0, sizeof(doc));
  doc.returns = resps;
  doc.n_returns = 1;
  resps[0].code = (char *)(size_t)(size_t) "200";
  resps[0].description = (char *)(size_t)(size_t) "OK";

  doc.response_headers = hdrs;
  doc.n_response_headers = 1;
  hdrs[0].code = (char *)(size_t)(size_t) "200";
  hdrs[0].name = (char *)(size_t)(size_t) "X-Request-Id";
  hdrs[0].type = (char *)(size_t)(size_t) "string";
  hdrs[0].content_type = (char *)(size_t)(size_t) "application/xml";
  hdrs[0].description = (char *)(size_t)(size_t) "Request identifier";
  hdrs[0].example = (char *)(size_t)(size_t) "42";
  hdrs[0].required_set = 1;
  hdrs[0].required = 1;

  ctx.sig = &sig;
  ctx.doc = &doc;
  ctx.func_name = sig.name;

  rc = c2openapi_build_operation(&ctx, &op);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(1, op.n_responses);
  ASSERT_STR_EQ("200", op.responses[0].code);
  ASSERT_STR_EQ("OK", op.responses[0].description);
  ASSERT_EQ(1, op.responses[0].n_headers);
  ASSERT_STR_EQ("X-Request-Id", op.responses[0].headers[0].name);
  ASSERT_STR_EQ("string", op.responses[0].headers[0].type);
  ASSERT_STR_EQ("application/xml", op.responses[0].headers[0].content_type);
  ASSERT_STR_EQ("Request identifier", op.responses[0].headers[0].description);
  ASSERT_EQ(1, op.responses[0].headers[0].required);
  ASSERT(op.responses[0].headers[0].example_set);
  ASSERT_EQ(OA_ANY_NUMBER, op.responses[0].headers[0].example.type);
  ASSERT_EQ(42, (int)op.responses[0].headers[0].example.number);

  reset_op(&op);
  g_fail_io_after = -1;
  PASS();
}

TEST test_build_response_links(void) {
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct DocMetadata doc;
  struct DocLink links[1];
  struct OpenAPI_Operation op;
  int rc = 0;

  rc += 0;
  memset(&op, 0, sizeof(op));
  sig.name = (char *)(size_t)(size_t) "get_page";
  sig.n_args = 0;
  sig.args = NULL;

  memset(&doc, 0, sizeof(doc));
  doc.route = (char *)(size_t)(size_t) "/pages";
  doc.verb = strdup("GET");
  doc.links = links;
  doc.n_links = 1;

  memset(links, 0, sizeof(links));
  links[0].code = (char *)(size_t)(size_t) "200";
  links[0].name = (char *)(size_t)(size_t) "next";
  links[0].operation_id = (char *)(size_t)(size_t) "getNextPage";
  links[0].summary = (char *)(size_t)(size_t) "Next page";
  links[0].description = (char *)(size_t)(size_t) "Fetch next page";
  links[0].parameters_json =
      (char *)(size_t)(size_t) "{\"cursor\":\"$response.body#/next\"}";
  links[0].request_body_json = (char *)(size_t)(size_t) "{\"foo\":1}";
  links[0].server_url = (char *)(size_t)(size_t) "https://example.com";
  links[0].server_name = (char *)(size_t)(size_t) "prod";
  links[0].server_description = (char *)(size_t)(size_t) "Primary server";

  ctx.sig = &sig;
  ctx.doc = &doc;
  ctx.func_name = sig.name;

  rc = c2openapi_build_operation(&ctx, &op);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(1, op.n_responses);
  ASSERT_EQ(1, op.responses[0].n_links);
  ASSERT_STR_EQ("next", op.responses[0].links[0].name);
  ASSERT_STR_EQ("getNextPage", op.responses[0].links[0].operation_id);
  ASSERT_STR_EQ("Next page", op.responses[0].links[0].summary);
  ASSERT_STR_EQ("Fetch next page", op.responses[0].links[0].description);
  ASSERT_EQ(1, op.responses[0].links[0].n_parameters);
  ASSERT_STR_EQ("cursor", op.responses[0].links[0].parameters[0].name);
  ASSERT_EQ(OA_ANY_STRING, op.responses[0].links[0].parameters[0].value.type);
  ASSERT_STR_EQ("$response.body#/next",
                op.responses[0].links[0].parameters[0].value.string);
  ASSERT(op.responses[0].links[0].request_body_set);
  ASSERT_EQ(OA_ANY_JSON, op.responses[0].links[0].request_body.type);
  ASSERT(op.responses[0].links[0].server_set);
  ASSERT(op.responses[0].links[0].server != NULL);
  ASSERT_STR_EQ("https://example.com", op.responses[0].links[0].server->url);
  ASSERT_STR_EQ("prod", op.responses[0].links[0].server->name);
  ASSERT_STR_EQ("Primary server", op.responses[0].links[0].server->description);

  free(doc.verb);
  reset_op(&op);
  g_fail_io_after = -1;
  PASS();
}

TEST test_build_response_output_arg(void) {
  /*
   * Case: int get_obj(struct Obj **out);
   * Heuristic: Double pointer -> Output parameter -> 200 Response
   */
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct C2OpenAPI_ParsedArg args[1];
  struct OpenAPI_Operation op;

  memset(args, 0, sizeof(args));
  memset(&op, 0, sizeof(op));
  sig.name = (char *)(size_t)(size_t) "get_obj";
  sig.n_args = 1;
  sig.args = args;
  args[0].name = (char *)(size_t)(size_t) "out";
  args[0].type = (char *)(size_t)(size_t) "struct Obj **";

  ctx.sig = &sig;
  ctx.doc = NULL;
  ctx.func_name = sig.name;

  c2openapi_build_operation(&ctx, &op);

  /* Should skip parameters */
  ASSERT_EQ(0, op.n_parameters);

  /* Check Responses */
  ASSERT_EQ(1, op.n_responses);
  ASSERT_STR_EQ("200", op.responses[0].code);
  ASSERT_STR_EQ("Obj", op.responses[0].schema.ref_name);
  ASSERT_STR_EQ("Success", op.responses[0].description);

  reset_op(&op);
  g_fail_io_after = -1;
  PASS();
}

TEST test_build_op_security_servers_request_body(void) {
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct C2OpenAPI_ParsedArg args[1];
  struct DocMetadata doc;
  struct DocSecurityRequirement sec[2];
  char *scopes1[] = {(char *)(size_t)(size_t) "write:pets",
                     (char *)(size_t)(size_t) "read:pets"};
  struct DocServer servers[1];
  struct DocServerVar server_vars[1];
  char *server_enum[] = {(char *)(size_t)(size_t) "prod",
                         (char *)(size_t)(size_t) "staging"};
  struct OpenAPI_Operation op;
  int rc = 0;

  rc += 0;
  memset(args, 0, sizeof(args));
  memset(&op, 0, sizeof(op));
  sig.name = (char *)(size_t)(size_t) "api_upload";
  sig.n_args = 1;
  sig.args = args;
  args[0].name = (char *)(size_t)(size_t) "payload";
  args[0].type = (char *)(size_t)(size_t) "const struct Payload *";

  memset(&doc, 0, sizeof(doc));
  doc.verb = strdup("POST");
  doc.route = (char *)(size_t)(size_t) "/upload";
  doc.request_body_description = (char *)(size_t)(size_t) "Upload payload";
  doc.request_body_required_set = 1;
  doc.request_body_required = 0;
  doc.request_body_content_type = (char *)(size_t)(size_t) "application/xml";

  memset(sec, 0, sizeof(sec));
  sec[0].scheme = (char *)(size_t)(size_t) "api_key";
  sec[1].scheme = (char *)(size_t)(size_t) "petstore_auth";
  sec[1].scopes = scopes1;
  sec[1].n_scopes = 2;
  doc.security = sec;
  doc.n_security = 2;

  memset(servers, 0, sizeof(servers));
  servers[0].url = (char *)(size_t)(size_t) "https://api.example.com";
  servers[0].name = (char *)(size_t)(size_t) "prod";
  servers[0].description = (char *)(size_t)(size_t) "Production API";
  memset(server_vars, 0, sizeof(server_vars));
  server_vars[0].name = (char *)(size_t)(size_t) "env";
  server_vars[0].default_value = (char *)(size_t)(size_t) "prod";
  server_vars[0].enum_values = server_enum;
  server_vars[0].n_enum_values = 2;
  servers[0].variables = server_vars;
  servers[0].n_variables = 1;
  doc.servers = servers;
  doc.n_servers = 1;

  memset(&ctx, 0, sizeof(ctx));
  ctx.sig = &sig;
  ctx.doc = &doc;
  ctx.func_name = sig.name;

  rc = c2openapi_build_operation(&ctx, &op);
  ASSERT_EQ(0, rc);

  ASSERT_STR_EQ("Upload payload", op.req_body_description);
  ASSERT_EQ(1, op.req_body_required_set);
  ASSERT_EQ(0, op.req_body_required);
  ASSERT_STR_EQ("application/xml", op.req_body.content_type);

  ASSERT_EQ(1, op.security_set);
  ASSERT_EQ(2, op.n_security);
  ASSERT_STR_EQ("api_key", op.security[0].requirements[0].scheme);
  ASSERT_EQ(0, op.security[0].requirements[0].n_scopes);
  ASSERT_STR_EQ("petstore_auth", op.security[1].requirements[0].scheme);
  ASSERT_EQ(2, op.security[1].requirements[0].n_scopes);
  ASSERT_STR_EQ("write:pets", op.security[1].requirements[0].scopes[0]);
  ASSERT_STR_EQ("read:pets", op.security[1].requirements[0].scopes[1]);

  ASSERT_EQ(1, op.n_servers);
  ASSERT_STR_EQ("https://api.example.com", op.servers[0].url);
  ASSERT_STR_EQ("prod", op.servers[0].name);
  ASSERT_STR_EQ("Production API", op.servers[0].description);
  ASSERT_EQ(1, op.servers[0].n_variables);
  ASSERT_STR_EQ("env", op.servers[0].variables[0].name);
  ASSERT_STR_EQ("prod", op.servers[0].variables[0].default_value);
  ASSERT_EQ(2, op.servers[0].variables[0].n_enum_values);
  ASSERT_STR_EQ("prod", op.servers[0].variables[0].enum_values[0]);
  ASSERT_STR_EQ("staging", op.servers[0].variables[0].enum_values[1]);

  free(doc.verb);
  reset_op(&op);
  g_fail_io_after = -1;
  PASS();
}

TEST test_build_op_param_deprecated(void) {
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct C2OpenAPI_ParsedArg args[1];
  struct DocMetadata doc;
  struct DocParam params[1];
  struct OpenAPI_Operation op;
  int rc = 0;

  rc += 0;
  memset(args, 0, sizeof(args));
  memset(params, 0, sizeof(params));
  memset(&op, 0, sizeof(op));
  sig.name = (char *)(size_t)(size_t) "api_get_legacy";
  sig.n_args = 1;
  sig.args = args;
  args[0].name = (char *)(size_t)(size_t) "legacyId";
  args[0].type = (char *)(size_t)(size_t) "int";

  memset(&doc, 0, sizeof(doc));
  doc.verb = strdup("GET");
  doc.route = (char *)(size_t)(size_t) "/legacy/{legacyId}";
  memset(params, 0, sizeof(params));
  params[0].name = (char *)(size_t)(size_t) "legacyId";
  params[0].in_loc = (char *)(size_t)(size_t) "path";
  params[0].deprecated_set = 1;
  params[0].deprecated = 1;
  doc.params = params;
  doc.n_params = 1;

  memset(&ctx, 0, sizeof(ctx));
  ctx.sig = &sig;
  ctx.doc = &doc;
  ctx.func_name = sig.name;

  rc = c2openapi_build_operation(&ctx, &op);
  ASSERT_EQ(0, rc);

  ASSERT_EQ(1, op.n_parameters);
  ASSERT_STR_EQ("legacyId", op.parameters[0].name);
  ASSERT_EQ(1, op.parameters[0].deprecated_set);
  ASSERT_EQ(1, op.parameters[0].deprecated);

  free(doc.verb);
  reset_op(&op);
  g_fail_io_after = -1;
  PASS();
}

TEST test_build_request_body_example(void) {
  /* */
  /* */

  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct C2OpenAPI_ParsedArg args[1];
  struct DocMetadata doc;
  struct DocRequestBody bodies[1];
  struct OpenAPI_Operation op;
  int rc = 0;

  rc += 0;
  memset(args, 0, sizeof(args));
  memset(&op, 0, sizeof(op));
  sig.name = (char *)(size_t)(size_t) "api_user_post";
  sig.n_args = 1;
  sig.args = args;
  args[0].name = (char *)(size_t)(size_t) "user";
  args[0].type = (char *)(size_t)(size_t) "struct User *";

  memset(&doc, 0, sizeof(doc));
  doc.verb = strdup("POST");
  doc.route = (char *)(size_t)(size_t) "/user";
  memset(bodies, 0, sizeof(bodies));
  bodies[0].content_type = (char *)(size_t)(size_t) "application/json";
  bodies[0].description = (char *)(size_t)(size_t) "User";
  bodies[0].example = (char *)(size_t)(size_t) "{\"name\":\"x\"}";
  doc.request_bodies = bodies;
  doc.n_request_bodies = 1;

  memset(&ctx, 0, sizeof(ctx));
  ctx.sig = &sig;
  ctx.doc = &doc;
  ctx.func_name = sig.name;

  rc = c2openapi_build_operation(&ctx, &op);
  ASSERT_EQ(0, rc);

  ASSERT_EQ(1, op.n_req_body_media_types);
  ASSERT(op.req_body_media_types[0].example_set);
  ASSERT_EQ(OA_ANY_JSON, op.req_body_media_types[0].example.type);

  free(doc.verb);
  reset_op(&op);
  g_fail_io_after = -1;
  PASS();
}

TEST test_build_request_body_default_content_type(void) {
  /* */
  /* */

  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct C2OpenAPI_ParsedArg args[1];
  struct DocMetadata doc;
  struct DocRequestBody bodies[1];
  struct OpenAPI_Operation op;
  int rc = 0;

  rc += 0;
  memset(args, 0, sizeof(args));
  memset(&op, 0, sizeof(op));
  sig.name = (char *)(size_t)(size_t) "api_user_post";
  sig.n_args = 1;
  sig.args = args;
  args[0].name = (char *)(size_t)(size_t) "user";
  args[0].type = (char *)(size_t)(size_t) "struct User *";

  memset(&doc, 0, sizeof(doc));
  doc.verb = strdup("POST");
  doc.route = (char *)(size_t)(size_t) "/user";
  memset(bodies, 0, sizeof(bodies));
  bodies[0].description = (char *)(size_t)(size_t) "User";
  doc.request_bodies = bodies;
  doc.n_request_bodies = 1;

  memset(&ctx, 0, sizeof(ctx));
  ctx.sig = &sig;
  ctx.doc = &doc;
  ctx.func_name = sig.name;

  rc = c2openapi_build_operation(&ctx, &op);
  ASSERT_EQ(0, rc);

  ASSERT_STR_EQ("application/json", op.req_body.content_type);
  ASSERT_EQ(1, op.n_req_body_media_types);
  ASSERT_STR_EQ("application/json", op.req_body_media_types[0].name);

  free(doc.verb);
  reset_op(&op);
  g_fail_io_after = -1;
  PASS();
}

TEST test_build_op_request_body_multi_content(void) {
  struct OpBuilderContext ctx;
  struct C2OpenAPI_ParsedSig sig;
  struct C2OpenAPI_ParsedArg args[1];
  struct DocMetadata doc;
  struct DocRequestBody bodies[2];
  struct OpenAPI_Operation op;
  int rc = 0;

  rc += 0;
  memset(args, 0, sizeof(args));
  memset(&op, 0, sizeof(op));
  sig.name = (char *)(size_t)(size_t) "api_upload_multi";
  sig.n_args = 1;
  sig.args = args;
  args[0].name = (char *)(size_t)(size_t) "payload";
  args[0].type = (char *)(size_t)(size_t) "const struct Payload *";

  memset(&doc, 0, sizeof(doc));
  doc.verb = strdup("POST");
  doc.route = (char *)(size_t)(size_t) "/upload";
  memset(bodies, 0, sizeof(bodies));
  bodies[0].content_type = (char *)(size_t)(size_t) "application/json";
  bodies[0].description = (char *)(size_t)(size_t) "JSON body";
  bodies[1].content_type = (char *)(size_t)(size_t) "application/xml";
  bodies[1].description = (char *)(size_t)(size_t) "XML body";
  doc.request_bodies = bodies;
  doc.n_request_bodies = 2;

  memset(&ctx, 0, sizeof(ctx));
  ctx.sig = &sig;
  ctx.doc = &doc;
  ctx.func_name = sig.name;

  rc = c2openapi_build_operation(&ctx, &op);
  ASSERT_EQ(0, rc);

  ASSERT_STR_EQ("application/json", op.req_body.content_type);
  ASSERT_EQ(2, op.n_req_body_media_types);
  ASSERT_STR_EQ("application/json", op.req_body_media_types[0].name);
  ASSERT_STR_EQ("application/xml", op.req_body_media_types[1].name);

  free(doc.verb);
  reset_op(&op);
  g_fail_io_after = -1;
  PASS();
}

SUITE(c2openapi_op_responses_suite) {
  RUN_TEST(test_build_custom_verb_additional);
  RUN_TEST(test_build_response_multi_content);
  RUN_TEST(test_build_response_headers);
  RUN_TEST(test_build_response_links);
  RUN_TEST(test_build_response_output_arg);
  RUN_TEST(test_build_op_security_servers_request_body);
  RUN_TEST(test_build_op_param_deprecated);
  RUN_TEST(test_build_request_body_example);
  RUN_TEST(test_build_request_body_default_content_type);
  RUN_TEST(test_build_op_request_body_multi_content);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_C2OPENAPI_OP_RESPONSES_H */
