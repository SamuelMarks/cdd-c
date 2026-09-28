/**
 * @file test_openapi_client_gen_helpers.h
 * @brief Tests for OpenAPI Client Generator helper functions.
 */

#ifndef TEST_OPENAPI_CLIENT_GEN_HELPERS_H
#define TEST_OPENAPI_CLIENT_GEN_HELPERS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cdd_test_helpers/cdd_helpers.h"
#include "functions/parse/fs.h"
#include "openapi/parse/openapi.h"
#include "routes/emit/client_gen.h"
/* clang-format on */

extern C_CDD_EXPORT int g_fail_io_after;
extern C_CDD_EXPORT int g_io_calls;
extern C_CDD_EXPORT int g_client_gen_fail;

TEST test_client_gen_find_server_variable(void) {
  struct OpenAPI_Server srv;
  const struct OpenAPI_ServerVariable *out = NULL;

  memset(&srv, 0, sizeof(srv));

  ASSERT_EQ(0, find_server_variable(NULL, "test", &out));
  ASSERT(out == NULL);

  ASSERT_EQ(0, find_server_variable(&srv, "test", &out));
  ASSERT(out == NULL);

  srv.n_variables = 1;
  srv.variables =
      (struct OpenAPI_ServerVariable *)calloc(1, sizeof(*srv.variables));
  srv.variables[0].name = (char *)(size_t)(size_t) "test";

  ASSERT_EQ(0, find_server_variable(&srv, "test", &out));
  ASSERT(out == &srv.variables[0]);

  ASSERT_EQ(0, find_server_variable(&srv, "missing", &out));
  ASSERT(out == NULL);

  /* Test out of memory logic in render_server_url_default */
  /* This is hard to do without custom mocks, we will need to inject
   * CDD_C_ERROR_MEMORY via mock allocations or leave it. */

  /* Test out of memory logic in render_server_url_default */
  /* This is hard to do without custom mocks, we will need to inject
   * CDD_C_ERROR_MEMORY via mock allocations or leave it. */

  /* Test docblock fail on CDD_C_ERROR_MEMORY using mocking if needed but
   * probably skip for now */
  free(srv.variables);
  g_fail_io_after = -1;
  PASS();
}

TEST test_client_gen_render_server_url_default(void) {
  struct OpenAPI_Server srv;
  char *out = NULL;

  memset(&srv, 0, sizeof(srv));

  ASSERT_EQ(0, render_server_url_default(NULL, &out));
  ASSERT(out == NULL);

  srv.url = (char *)(size_t)(size_t) "http://test";
  ASSERT_EQ(0, render_server_url_default(&srv, &out));
  ASSERT_STR_EQ("http://test", out);
  free(out);
  out = NULL;

  /* With variables */
  srv.url = (char *)(size_t)(size_t) "http://{domain}:{port}/v1";
  srv.n_variables = 2;
  srv.variables =
      (struct OpenAPI_ServerVariable *)calloc(2, sizeof(*srv.variables));
  srv.variables[0].name = (char *)(size_t)(size_t) "domain";
  srv.variables[0].default_value = (char *)(size_t)(size_t) "localhost";
  srv.variables[1].name = (char *)(size_t)(size_t) "port";
  srv.variables[1].default_value = (char *)(size_t)(size_t) "8080";

  ASSERT_EQ(0, render_server_url_default(&srv, &out));
  ASSERT_STR_EQ("http://localhost:8080/v1", out);
  free(out);
  out = NULL;

  /* Unmatched variable */
  srv.url = (char *)(size_t)(size_t) "http://{missing}/test";
  ASSERT_EQ(0, render_server_url_default(&srv, &out));
  ASSERT(out == NULL);

  /* Missing closing brace */
  srv.url = (char *)(size_t)(size_t) "http://{missing/test";
  ASSERT_EQ(0, render_server_url_default(&srv, &out));
  ASSERT(out == NULL);

  /* Empty braces */
  srv.url = (char *)(size_t)(size_t) "http://{}/test";
  ASSERT_EQ(0, render_server_url_default(&srv, &out));
  ASSERT(out == NULL);

  /* Valid var but missing default */
  if (srv.variables)
    free(srv.variables);
  srv.url = (char *)(size_t)(size_t) "http://{noval}/test";
  srv.n_variables = 1;
  srv.variables =
      (struct OpenAPI_ServerVariable *)calloc(1, sizeof(*srv.variables));
  srv.variables[0].name = (char *)(size_t)(size_t) "noval";
  ASSERT_EQ(0, render_server_url_default(&srv, &out));
  ASSERT(out == NULL);

  /* Test out of memory logic in render_server_url_default */
  /* This is hard to do without custom mocks, we will need to inject
   * CDD_C_ERROR_MEMORY via mock allocations or leave it. */

  /* Test out of memory logic in render_server_url_default */
  /* This is hard to do without custom mocks, we will need to inject
   * CDD_C_ERROR_MEMORY via mock allocations or leave it. */

  /* Test docblock fail on CDD_C_ERROR_MEMORY using mocking if needed but
   * probably skip for now */
  free(srv.variables);
  g_fail_io_after = -1;
  PASS();
}

TEST test_client_gen_escape_c_string_literal(void) {
  char *out = NULL;

  /* NULL */
  ASSERT_EQ(0, escape_c_string_literal(NULL, &out));
  ASSERT(out == NULL);

  ASSERT_EQ(0, escape_c_string_literal("hello", &out));
  ASSERT_STR_EQ("hello", out);
  free(out);
  out = NULL;

  ASSERT_EQ(0, escape_c_string_literal("hello \"world\"\n\r\t", &out));
  ASSERT_STR_EQ("hello \\\"world\\\"\\n\\r\\t", out);
  free(out);
  out = NULL;
  g_fail_io_after = -1;

  PASS();
}

TEST test_client_gen_select_operation_server(void) {
  struct OpenAPI_Path path;
  struct OpenAPI_Operation op;
  struct OpenAPI_Server *out = NULL;

  memset(&path, 0, sizeof(path));
  memset(&op, 0, sizeof(op));

  ASSERT_EQ(0, select_operation_server(NULL, NULL, &out));
  ASSERT(out == NULL);

  op.n_servers = 1;
  op.servers = (struct OpenAPI_Server *)calloc(1, sizeof(*op.servers));
  ASSERT_EQ(0, select_operation_server(&path, &op, &out));
  ASSERT(out == &op.servers[0]);

  op.n_servers = 0;
  path.n_servers = 1;
  path.servers = (struct OpenAPI_Server *)calloc(1, sizeof(*path.servers));
  ASSERT_EQ(0, select_operation_server(&path, &op, &out));
  ASSERT(out == &path.servers[0]);

  free(op.servers[0].variables);
  free(op.servers);
  free(path.servers);
  g_fail_io_after = -1;
  PASS();
}

TEST test_client_gen_build_base_url_literal(void) {
  char *out = NULL;

  ASSERT_EQ(0, build_base_url_literal(NULL, &out));
  ASSERT(out == NULL);

  ASSERT_EQ(0, build_base_url_literal("http://test.com", &out));
  ASSERT_STR_EQ("\"http://test.com\"", out);
  free(out);
  g_fail_io_after = -1;

  PASS();
}

TEST test_client_gen_generate_guard(void) {
  char *out = NULL;

  ASSERT_EQ(0, generate_guard("my-test.h", &out));
  ASSERT_STR_EQ("MY_TEST_H_H", out);
  free(out);
  g_fail_io_after = -1;

  PASS();
}

TEST test_client_gen_derive_model_header(void) {
  char *out = NULL;

  ASSERT_EQ(0, derive_model_header("test", &out));
  ASSERT_STR_EQ("test_models.h", out);
  free(out);
  g_fail_io_after = -1;

  PASS();
}

TEST test_client_gen_sanitize_tag(void) {
  char *out = NULL;

  ASSERT_EQ(0, sanitize_tag(NULL, &out));
  ASSERT(out == NULL);

  ASSERT_EQ(0, sanitize_tag("my-tag! test", &out));
  ASSERT_STR_EQ("My_tag__test", out);
  free(out);
  g_fail_io_after = -1;

  PASS();
}

TEST test_client_gen_param_keys_match(void) {
  struct OpenAPI_Parameter a, b;
  memset(&a, 0, sizeof(a));
  memset(&b, 0, sizeof(b));

  ASSERT_EQ(0, param_keys_match(NULL, NULL));
  ASSERT_EQ(0, param_keys_match(&a, &b));

  a.name = (char *)(size_t)(size_t) "test";
  ASSERT_EQ(0, param_keys_match(&a, &b));

  b.name = (char *)(size_t)(size_t) "test2";
  ASSERT_EQ(0, param_keys_match(&a, &b));

  b.name = (char *)(size_t)(size_t) "test";
  a.in = OA_PARAM_IN_HEADER;
  b.in = OA_PARAM_IN_PATH;
  ASSERT_EQ(0, param_keys_match(&a, &b));

  b.in = OA_PARAM_IN_HEADER;
  ASSERT_EQ(1, param_keys_match(&a, &b));
  g_fail_io_after = -1;

  PASS();
}

TEST test_client_gen_build_effective_parameters(void) {
  struct OpenAPI_Path path;
  struct OpenAPI_Operation op;
  struct OpenAPI_Parameter *out = NULL;
  size_t count = 0;

  memset(&path, 0, sizeof(path));
  memset(&op, 0, sizeof(op));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            build_effective_parameters(NULL, NULL, NULL, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            build_effective_parameters(NULL, NULL, &out, NULL));

  ASSERT_EQ(0, build_effective_parameters(NULL, NULL, &out, &count));
  ASSERT_EQ(0, count);

  /* Path params */
  path.n_parameters = 1;
  path.parameters =
      (struct OpenAPI_Parameter *)calloc(1, sizeof(*path.parameters));
  path.parameters[0].name = (char *)(size_t)(size_t) "p1";
  path.parameters[0].in = OA_PARAM_IN_PATH;

  ASSERT_EQ(0, build_effective_parameters(&path, NULL, &out, &count));
  ASSERT_EQ(1, count);
  ASSERT_STR_EQ("p1", out[0].name);
  free(out);
  out = NULL;

  /* Op params overrides */
  op.n_parameters = 2;
  op.parameters = (struct OpenAPI_Parameter *)calloc(2, sizeof(*op.parameters));
  op.parameters[0].name = (char *)(size_t)(size_t) "p1";
  op.parameters[0].in = OA_PARAM_IN_PATH;
  op.parameters[0].description = (char *)(size_t)(size_t) "overridden";
  op.parameters[1].name = (char *)(size_t)(size_t) "p2";
  op.parameters[1].in = OA_PARAM_IN_QUERY;

  ASSERT_EQ(0, build_effective_parameters(&path, &op, &out, &count));
  ASSERT_EQ(2, count);
  ASSERT_STR_EQ("p1", out[0].name);
  ASSERT_STR_EQ("p2", out[1].name);
  free(out);
  out = NULL;

  free(path.parameters);
  free(op.parameters);
  g_fail_io_after = -1;
  PASS();
}

TEST test_client_gen_verb_to_string(void) {
  char *out = NULL;

  ASSERT_EQ(0, verb_to_string(OA_VERB_GET, &out));
  ASSERT_STR_EQ("GET", out);

  ASSERT_EQ(0, verb_to_string(OA_VERB_POST, &out));
  ASSERT_STR_EQ("POST", out);

  ASSERT_EQ(0, verb_to_string(OA_VERB_PUT, &out));
  ASSERT_STR_EQ("PUT", out);

  ASSERT_EQ(0, verb_to_string(OA_VERB_DELETE, &out));
  ASSERT_STR_EQ("DELETE", out);

  ASSERT_EQ(0, verb_to_string(OA_VERB_PATCH, &out));
  ASSERT_STR_EQ("PATCH", out);

  ASSERT_EQ(0, verb_to_string(OA_VERB_HEAD, &out));
  ASSERT_STR_EQ("HEAD", out);

  ASSERT_EQ(0, verb_to_string(OA_VERB_OPTIONS, &out));
  ASSERT_STR_EQ("OPTIONS", out);

  ASSERT_EQ(0, verb_to_string(OA_VERB_TRACE, &out));
  ASSERT_STR_EQ("TRACE", out);

  ASSERT_EQ(0, verb_to_string(OA_VERB_QUERY, &out));
  ASSERT_STR_EQ("QUERY", out);

  ASSERT_EQ(0, verb_to_string(OA_VERB_UNKNOWN, &out));
  ASSERT_STR_EQ("UNKNOWN", out);
  g_fail_io_after = -1;

  PASS();
}

TEST test_client_gen_write_docblock(void) {
  FILE *fp;
  struct OpenAPI_Path path;
  struct OpenAPI_Operation op;

  memset(&path, 0, sizeof(path));
  memset(&op, 0, sizeof(op));

#if defined(_MSC_VER)
  if (fopen_s(&fp, "test_docblock.txt", "w") != 0)
    fp = NULL;
#else
  fp = fopen("test_docblock.txt", "w");
#endif
  ASSERT(fp != NULL);

  /* Fallback */
  ASSERT_EQ(0, write_docblock(fp, NULL, &op));

  /* Various branch hits */
  op.summary = (char *)(size_t)(size_t) "sum";
  op.operation_id = (char *)(size_t)(size_t) "opId";
  op.description = (char *)(size_t)(size_t) "desc";
  path.route = (char *)(size_t)(size_t) "/test";
  op.verb = OA_VERB_POST;
  op.external_docs.url = (char *)(size_t)(size_t) "http://doc";
  op.callbacks =
      (struct OpenAPI_Callback *)calloc(1, sizeof(struct OpenAPI_Callback));
  op.n_responses = 1;
  op.responses = (struct OpenAPI_Response *)calloc(1, sizeof(*op.responses));
  op.responses[0].links =
      (struct OpenAPI_Link *)calloc(1, sizeof(struct OpenAPI_Link));
  op.security = (struct OpenAPI_SecurityRequirementSet *)calloc(
      1, sizeof(struct OpenAPI_SecurityRequirementSet));
  op.n_servers = 1;
  op.servers = (struct OpenAPI_Server *)calloc(1, sizeof(*op.servers));
  op.servers[0].variables = (struct OpenAPI_ServerVariable *)calloc(
      1, sizeof(struct OpenAPI_ServerVariable));

  op.n_parameters = 1;
  op.parameters = (struct OpenAPI_Parameter *)calloc(1, sizeof(*op.parameters));
  op.parameters[0].name = (char *)(size_t)(size_t) "p";
  op.parameters[0].description = (char *)(size_t)(size_t) "desc p";
  op.parameters[0].allow_empty_value = 1;
  op.parameters[0].allow_reserved = 1;
  op.deprecated = 1;

  op.n_parameters = 6;
  op.parameters = (struct OpenAPI_Parameter *)realloc(
      op.parameters, 6 * sizeof(*op.parameters));
  memset(&op.parameters[1], 0, sizeof(*op.parameters));
  op.parameters[1].name = (char *)(size_t)(size_t) "cookiep";
  op.parameters[1].in = OA_PARAM_IN_COOKIE;
  memset(&op.parameters[2], 0, sizeof(*op.parameters));
  op.parameters[2].name = (char *)(size_t)(size_t) "unkp";
  op.parameters[2].in = OA_PARAM_IN_UNKNOWN;
  memset(&op.parameters[3], 0, sizeof(*op.parameters));
  op.parameters[3].name = (char *)(size_t)(size_t) "queryp";
  op.parameters[3].in = OA_PARAM_IN_QUERY;
  memset(&op.parameters[4], 0, sizeof(*op.parameters));
  op.parameters[4].name = (char *)(size_t)(size_t) "qsp";
  op.parameters[4].in = OA_PARAM_IN_QUERYSTRING;
  memset(&op.parameters[5], 0, sizeof(*op.parameters));
  op.parameters[5].name = (char *)(size_t)(size_t) "hdrp";
  op.parameters[5].in = OA_PARAM_IN_HEADER;

  /* we will just execute all branches */
  ASSERT_EQ(0, write_docblock(fp, &path, &op));

  free(op.callbacks);
  free(op.responses[0].links);
  free(op.security);
  free(op.responses);
  free(op.servers[0].variables);
  free(op.servers);
  free(op.parameters);
  if (fp)
    fclose(fp);
  remove("test_docblock.txt");
  g_fail_io_after = -1;

  PASS();
}

TEST test_client_gen_emit_operation(void) {
  struct OpenAPI_Path path;
  struct OpenAPI_Operation op;
  struct OpenAPI_Spec spec;
  struct OpenApiClientConfig config;
  struct OpenAPI_Server srv;
  char *tags[1];
  FILE *hfile;
  FILE *cfile;

  /* Missing params */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            emit_operation(NULL, NULL, NULL, NULL, NULL, NULL, NULL));

  memset(&path, 0, sizeof(path));
  memset(&op, 0, sizeof(op));
  memset(&spec, 0, sizeof(spec));
  memset(&config, 0, sizeof(config));
  memset(&srv, 0, sizeof(srv));

  path.route = (char *)(size_t)(size_t) "/users/{id}";
  op.operation_id = (char *)(size_t)(size_t) "getUser";
  tags[0] = (char *)(size_t)(size_t) "users_tag";
  op.tags = tags;
  op.n_tags = 1;

  srv.url = (char *)(size_t)(size_t) "https://server.example.com";
  op.servers = &srv;
  op.n_servers = 1;

  config.namespace_prefix = (char *)(size_t)(size_t) "NS";

  hfile = tmpfile();
  cfile = tmpfile();
  ASSERT(hfile != NULL);
  ASSERT(cfile != NULL);

  ASSERT_EQ(0,
            emit_operation(hfile, cfile, &path, &op, &spec, &config, "api_"));

  /* Also test with only sanitized_group (no namespace) */
  config.namespace_prefix = NULL;
  ASSERT_EQ(0,
            emit_operation(hfile, cfile, &path, &op, &spec, &config, "api_"));

  /* And test with namespace but no tag */
  op.n_tags = 0;
  config.namespace_prefix = (char *)(size_t)(size_t) "OnlyNS";
  ASSERT_EQ(0,
            emit_operation(hfile, cfile, &path, &op, &spec, &config, "api_"));

  fclose(hfile);
  fclose(cfile);

  g_fail_io_after = -1;
  PASS();
}

SUITE(openapi_client_gen_helpers_suite) {
  RUN_TEST(test_client_gen_find_server_variable);
  RUN_TEST(test_client_gen_render_server_url_default);
  RUN_TEST(test_client_gen_escape_c_string_literal);
  RUN_TEST(test_client_gen_select_operation_server);
  RUN_TEST(test_client_gen_build_base_url_literal);
  RUN_TEST(test_client_gen_generate_guard);
  RUN_TEST(test_client_gen_derive_model_header);
  RUN_TEST(test_client_gen_sanitize_tag);
  RUN_TEST(test_client_gen_param_keys_match);
  RUN_TEST(test_client_gen_build_effective_parameters);
  RUN_TEST(test_client_gen_verb_to_string);
  RUN_TEST(test_client_gen_write_docblock);
  RUN_TEST(test_client_gen_emit_operation);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_OPENAPI_CLIENT_GEN_HELPERS_H */
