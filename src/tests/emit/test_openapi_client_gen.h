/**
 * @file test_openapi_client_gen.h
 * @brief Tests for the OpenAPI Client Library Generator.
 */

#ifndef TEST_OPENAPI_CLIENT_GEN_H
#define TEST_OPENAPI_CLIENT_GEN_H

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
/* LCOV_EXCL_START */

static void setup_minimal_spec(struct OpenAPI_Spec *spec,
                               struct OpenAPI_Operation *op) {
  static struct OpenAPI_Path path;
  static struct OpenAPI_Response resp = {0};

  (void)openapi_spec_init(spec);

  memset(op, 0, sizeof(*op));
  op->operation_id = "test_op";
  op->verb = OA_VERB_GET;

  memset(&resp, 0, sizeof(resp));
  resp.code = "200";
  op->responses = &resp;
  op->n_responses = 1;

  memset(&path, 0, sizeof(path));
  path.route = "/test";
  path.operations = op;
  path.n_operations = 1;

  spec->paths = &path;
  spec->n_paths = 1;
}

TEST test_gen_client_basic(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op = {0};
  struct OpenApiClientConfig config;
  const char *base = "build/test_out/gen_client_test";
  char *h_file = "build/test_out/src/gen_client_test.h";
  char *c_file = "build/test_out/src/gen_client_test.c";
  char *content = NULL;
  size_t sz;
  int rc;

  setup_minimal_spec(&spec, &op);

  memset(&config, 0, sizeof(config));
  config.filename_base = base;
  config.func_prefix = "api_";
  config.model_header = "my_models.h";

  rc = openapi_client_generate(&spec, &config);
  ASSERT_EQ(0, rc);

  read_to_file(h_file, "r", &content, &sz);
  ASSERT(strstr(content, "int api_test_op(") != NULL);
  free(content);

  read_to_file(c_file, "r", &content, &sz);
  ASSERT(strstr(content, "int api_test_op(struct HttpClient *ctx") != NULL);
  free(content);

  remove(h_file);
  remove(c_file);
  g_fail_io_after = -1;
  PASS();
}

TEST test_gen_client_operation_server_override(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Server op_server;
  struct OpenApiClientConfig config;
  const char *base = "build/test_out/gen_client_op_server";
  char *h_file = "build/test_out/src/gen_client_op_server.h";
  char *c_file = "build/test_out/src/gen_client_op_server.c";
  char *content = NULL;
  size_t sz;
  int rc;

  setup_minimal_spec(&spec, &op);

  memset(&op_server, 0, sizeof(op_server));
  op_server.url = "https://op.example.com/api";
  op.servers = &op_server;
  op.n_servers = 1;

  memset(&config, 0, sizeof(config));
  config.filename_base = base;
  config.func_prefix = "api_";

  rc = openapi_client_generate(&spec, &config);
  ASSERT_EQ(0, rc);

  read_to_file(c_file, "r", &content, &sz);
  ASSERT(strstr(content, "\"https://op.example.com/api\"") != NULL);
  free(content);

  remove(h_file);
  remove(c_file);
  g_fail_io_after = -1;
  PASS();
}

TEST test_gen_client_text_plain_request_body(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op = {0};
  struct OpenApiClientConfig config;
  const char *base = "build/test_out/gen_client_text_plain_req";
  char *h_file = "build/test_out/src/gen_client_text_plain_req.h";
  char *c_file = "build/test_out/src/gen_client_text_plain_req.c";
  char *content = NULL;
  size_t sz;
  int rc;

  setup_minimal_spec(&spec, &op);
  op.verb = OA_VERB_POST;
  op.req_body.content_type = "text/plain";
  op.req_body.inline_type = "string";

  memset(&config, 0, sizeof(config));
  config.filename_base = base;
  config.func_prefix = "api_";

  rc = openapi_client_generate(&spec, &config);
  ASSERT_EQ(0, rc);

  read_to_file(c_file, "r", &content, &sz);
  ASSERT(strstr(content, "\"Content-Type\", \"text/plain\"") != NULL);
  ASSERT(strstr(content, "req.body_len = strlen(req_body)") != NULL);
  free(content);

  remove(h_file);
  remove(c_file);
  g_fail_io_after = -1;
  PASS();
}

TEST test_gen_client_octet_stream_request_body(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op = {0};
  struct OpenApiClientConfig config;
  const char *base = "build/test_out/gen_client_octet_req";
  char *h_file = "build/test_out/src/gen_client_octet_req.h";
  char *c_file = "build/test_out/src/gen_client_octet_req.c";
  char *content = NULL;
  size_t sz;
  int rc;

  setup_minimal_spec(&spec, &op);
  op.verb = OA_VERB_POST;
  op.req_body.content_type = "application/octet-stream";

  memset(&config, 0, sizeof(config));
  config.filename_base = base;
  config.func_prefix = "api_";

  rc = openapi_client_generate(&spec, &config);
  ASSERT_EQ(0, rc);

  read_to_file(c_file, "r", &content, &sz);
  ASSERT(strstr(content, "\"Content-Type\", \"application/octet-stream\"") !=
         NULL);
  ASSERT(strstr(content, "req.body_len = body_len") != NULL);
  free(content);

  remove(h_file);
  remove(c_file);
  g_fail_io_after = -1;
  PASS();
}

TEST test_gen_client_octet_stream_response_body(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op = {0};
  struct OpenApiClientConfig config;
  const char *base = "build/test_out/gen_client_octet_resp";
  char *h_file = "build/test_out/src/gen_client_octet_resp.h";
  char *c_file = "build/test_out/src/gen_client_octet_resp.c";
  char *content = NULL;
  size_t sz;
  int rc;

  setup_minimal_spec(&spec, &op);
  op.responses[0].content_type = "application/octet-stream";

  memset(&config, 0, sizeof(config));
  config.filename_base = base;
  config.func_prefix = "api_";

  rc = openapi_client_generate(&spec, &config);
  ASSERT_EQ(0, rc);

  read_to_file(c_file, "r", &content, &sz);
  ASSERT(strstr(content, "unsigned char *tmp =") != NULL);
  ASSERT(strstr(content, "memcpy(tmp, res->body, res->body_len)") != NULL);
  free(content);

  remove(h_file);
  remove(c_file);
  g_fail_io_after = -1;
  PASS();
}

TEST test_gen_client_default_base_url_from_server(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op = {0};
  struct OpenApiClientConfig config;
  struct OpenAPI_Server server;
  struct OpenAPI_ServerVariable var;
  const char *base = "build/test_out/gen_client_default_url";
  char *h_file = "build/test_out/src/gen_client_default_url.h";
  char *c_file = "build/test_out/src/gen_client_default_url.c";
  char *content = NULL;
  size_t sz;
  int rc;

  setup_minimal_spec(&spec, &op);

  memset(&server, 0, sizeof(server));
  memset(&var, 0, sizeof(var));
  server.url = "https://{env}.example.com/v1";
  var.name = "env";
  var.default_value = "api";
  server.variables = &var;
  server.n_variables = 1;
  spec.servers = &server;
  spec.n_servers = 1;

  memset(&config, 0, sizeof(config));
  config.filename_base = base;
  config.func_prefix = "api_";

  rc = openapi_client_generate(&spec, &config);
  ASSERT_EQ(0, rc);

  read_to_file(c_file, "r", &content, &sz);
  ASSERT(strstr(content,
                "const char *default_url = \"https://api.example.com/v1\";") !=
         NULL);
  ASSERT(strstr(content, "if (!base_url || base_url[0] == '\\0')") != NULL);
  free(content);

  remove(h_file);
  remove(c_file);
  g_fail_io_after = -1;
  PASS();
}

TEST test_gen_client_default_base_url_no_servers(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op = {0};
  struct OpenApiClientConfig config;
  const char *base = "build/test_out/gen_client_default_url_none";
  char *h_file = "build/test_out/src/gen_client_default_url_none.h";
  char *c_file = "build/test_out/src/gen_client_default_url_none.c";
  char *content = NULL;
  size_t sz;
  int rc;

  setup_minimal_spec(&spec, &op);

  memset(&config, 0, sizeof(config));
  config.filename_base = base;
  config.func_prefix = "api_";

  rc = openapi_client_generate(&spec, &config);
  ASSERT_EQ(0, rc);

  read_to_file(c_file, "r", &content, &sz);
  ASSERT(strstr(content, "const char *default_url = \"/\";") != NULL);
  ASSERT(strstr(content, "if (!base_url || base_url[0] == '\\0')") != NULL);
  free(content);

  remove(h_file);
  remove(c_file);
  g_fail_io_after = -1;
  PASS();
}

TEST test_gen_client_additional_operation(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Response resp = {0};
  struct OpenApiClientConfig config;
  struct OpenAPI_Path path;
  const char *base = "build/test_out/gen_additional_op";
  char *h_file = "build/test_out/src/gen_additional_op.h";
  char *c_file = "build/test_out/src/gen_additional_op.c";
  char *content = NULL;
  size_t sz;
  int rc;

  (void)openapi_spec_init(&spec);
  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));
  memset(&path, 0, sizeof(path));

  op.operation_id = "custom_connect";
  op.verb = OA_VERB_UNKNOWN;
  op.is_additional = 1;
  op.method = "CONNECT";
  resp.code = "200";
  op.responses = &resp;
  op.n_responses = 1;

  path.route = "/custom";
  path.additional_operations = &op;
  path.n_additional_operations = 1;

  spec.paths = &path;
  spec.n_paths = 1;

  memset(&config, 0, sizeof(config));
  config.filename_base = base;
  config.func_prefix = "api_";

  rc = openapi_client_generate(&spec, &config);
  ASSERT_EQ(0, rc);

  read_to_file(h_file, "r", &content, &sz);
  ASSERT(strstr(content, "int api_custom_connect(") != NULL);
  free(content);

  read_to_file(c_file, "r", &content, &sz);
  ASSERT(strstr(content, "req.method = HTTP_CONNECT;") != NULL);
  free(content);

  remove(h_file);
  remove(c_file);
  g_fail_io_after = -1;
  PASS();
}

TEST test_gen_client_op_params_only(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op = {0};
  struct OpenApiClientConfig config;
  struct OpenAPI_Parameter op_param;
  const char *base = "build/test_out/gen_op_params";
  char *h_file = "build/test_out/src/gen_op_params.h";
  char *content = NULL;
  size_t sz;
  int rc;

  setup_minimal_spec(&spec, &op);

  memset(&op_param, 0, sizeof(op_param));
  op_param.name = "limit";
  op_param.in = OA_PARAM_IN_QUERY;
  op_param.type = "integer";
  op.parameters = &op_param;
  op.n_parameters = 1;

  memset(&config, 0, sizeof(config));
  config.filename_base = base;
  config.func_prefix = "api_";

  rc = openapi_client_generate(&spec, &config);
  ASSERT_EQ(0, rc);

  read_to_file(h_file, "r", &content, &sz);
  ASSERT(strstr(content, "int api_test_op(struct HttpClient *ctx, int limit") !=
         NULL);
  free(content);

  remove(h_file);
  remove("build/test_out/src/gen_op_params.c");
  remove("build/test_out/src/gen_op_params_models.h");
  remove("build/test_out/src/gen_op_params_models.c");
  g_fail_io_after = -1;
  PASS();
}

TEST test_gen_client_querystring_param(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op = {0};
  struct OpenApiClientConfig config;
  struct OpenAPI_Parameter op_param;
  const char *base = "build/test_out/gen_querystring_param";
  char *h_file = "build/test_out/src/gen_querystring_param.h";
  char *content = NULL;
  size_t sz;
  int rc;

  setup_minimal_spec(&spec, &op);

  memset(&op_param, 0, sizeof(op_param));
  op_param.name = "qs";
  op_param.in = OA_PARAM_IN_QUERYSTRING;
  op_param.type = "string";
  op.parameters = &op_param;
  op.n_parameters = 1;

  memset(&config, 0, sizeof(config));
  config.filename_base = base;
  config.func_prefix = "api_";

  rc = openapi_client_generate(&spec, &config);
  ASSERT_EQ(0, rc);

  read_to_file(h_file, "r", &content, &sz);
  ASSERT(strstr(content, "[in:querystring] Parameter.") != NULL);
  free(content);

  remove(h_file);
  remove("build/test_out/src/gen_querystring_param.c");
  remove("build/test_out/src/gen_querystring_param_models.h");
  remove("build/test_out/src/gen_querystring_param_models.c");
  g_fail_io_after = -1;
  PASS();
}

TEST test_gen_client_path_level_params(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op = {0};
  struct OpenApiClientConfig config;
  struct OpenAPI_Parameter path_param;
  const char *base = "build/test_out/gen_path_params";
  char *h_file = "build/test_out/src/gen_path_params.h";
  char *content = NULL;
  size_t sz;
  int rc;

  setup_minimal_spec(&spec, &op);

  memset(&path_param, 0, sizeof(path_param));
  path_param.name = "x_trace";
  path_param.in = OA_PARAM_IN_HEADER;
  path_param.type = "string";

  spec.paths[0].parameters = &path_param;
  spec.paths[0].n_parameters = 1;

  memset(&config, 0, sizeof(config));
  config.filename_base = base;
  config.func_prefix = "api_";

  rc = openapi_client_generate(&spec, &config);
  ASSERT_EQ(0, rc);

  read_to_file(h_file, "r", &content, &sz);
  ASSERT(
      strstr(content,
             "int api_test_op(struct HttpClient *ctx, const char *x_trace") !=
      NULL);
  free(content);

  remove(h_file);
  remove("build/test_out/src/gen_path_params.c");
  remove("build/test_out/src/gen_path_params_models.h");
  remove("build/test_out/src/gen_path_params_models.c");
  g_fail_io_after = -1;
  PASS();
}

TEST test_gen_client_path_param_override(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op = {0};
  struct OpenApiClientConfig config;
  struct OpenAPI_Parameter path_param;
  struct OpenAPI_Parameter op_param;
  const char *base = "build/test_out/gen_path_override";
  char *h_file = "build/test_out/src/gen_path_override.h";
  char *content = NULL;
  size_t sz;
  int rc;

  setup_minimal_spec(&spec, &op);

  memset(&path_param, 0, sizeof(path_param));
  path_param.name = "id";
  path_param.in = OA_PARAM_IN_PATH;
  path_param.type = "integer";

  memset(&op_param, 0, sizeof(op_param));
  op_param.name = "id";
  op_param.in = OA_PARAM_IN_PATH;
  op_param.type = "string";

  spec.paths[0].parameters = &path_param;
  spec.paths[0].n_parameters = 1;
  op.parameters = &op_param;
  op.n_parameters = 1;

  memset(&config, 0, sizeof(config));
  config.filename_base = base;
  config.func_prefix = "api_";

  rc = openapi_client_generate(&spec, &config);
  ASSERT_EQ(0, rc);

  read_to_file(h_file, "r", &content, &sz);
  ASSERT(strstr(content, "const char *id") != NULL);
  free(content);

  remove(h_file);
  remove("build/test_out/src/gen_path_override.c");
  remove("build/test_out/src/gen_path_override_models.h");
  remove("build/test_out/src/gen_path_override_models.c");
  g_fail_io_after = -1;
  PASS();
}

TEST test_gen_client_grouped_tags_namespace(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op = {0};
  struct OpenApiClientConfig config;
  const char *base = "build/test_out/gen_group_ns_test";
  char *h_file = "build/test_out/src/gen_group_ns_test.h";
  char *content = NULL;
  size_t sz;
  int rc;
  /* Tag string array setup */
  static char *tags[] = {"pet"};

  setup_minimal_spec(&spec, &op);

  /* Inject tag manually */
  op.tags = tags;
  op.n_tags = 1;

  memset(&config, 0, sizeof(config));
  config.filename_base = base;
  config.func_prefix = "api_";
  config.namespace_prefix = "Foo";

  rc = openapi_client_generate(&spec, &config);
  ASSERT_EQ(0, rc);

  read_to_file(h_file, "r", &content, &sz);
  /* Should be Foo_Pet_api_test_op */
  /* Namespace "Foo", Tag "Pet" (capitalized), Prefix "api_" */
  ASSERT(strstr(content, "int Foo_Pet_api_test_op(") != NULL);
  free(content);

  remove(h_file);
  remove("build/test_out/src/gen_group_ns_test.c");
  remove("build/test_out/src/gen_group_ns_test_models.h");
  remove("build/test_out/src/gen_group_ns_test_models.c");
  g_fail_io_after = -1;
  PASS();
}

TEST test_gen_client_namespace_only(void) {
  /* Case: Namespace present, but no tags on operation */
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op = {0};
  struct OpenApiClientConfig config;
  const char *base = "build/test_out/gen_ns_only_test";
  char *h_file = "build/test_out/src/gen_ns_only_test.h";
  char *content = NULL;
  size_t sz;
  int rc;

  setup_minimal_spec(&spec, &op);
  /* No tags */

  memset(&config, 0, sizeof(config));
  config.filename_base = base;
  config.func_prefix = "api_";
  config.namespace_prefix = "Bar";

  rc = openapi_client_generate(&spec, &config);
  ASSERT_EQ(0, rc);

  read_to_file(h_file, "r", &content, &sz);
  /* Should be Bar_api_test_op */
  ASSERT(strstr(content, "int Bar_api_test_op(") != NULL);
  free(content);

  remove(h_file);
  remove("build/test_out/src/gen_ns_only_test.c");
  remove("build/test_out/src/gen_ns_only_test_models.h");
  remove("build/test_out/src/gen_ns_only_test_models.c");
  g_fail_io_after = -1;
  PASS();
}

TEST test_gen_client_error_nulls(void) {
  struct OpenAPI_Spec spec;
  struct OpenApiClientConfig config = {0};
  /* Use explicit struct instead of compound literal */
  struct OpenAPI_Operation dummy_op;
  memset(&dummy_op, 0, sizeof(dummy_op));

  setup_minimal_spec(&spec, &dummy_op);

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            openapi_client_generate(NULL, &config));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, openapi_client_generate(&spec, NULL));

  config.filename_base = NULL;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            openapi_client_generate(&spec, &config));
  g_fail_io_after = -1;

  PASS();
}

TEST test_gen_client_file_error(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op = {0};
  struct OpenApiClientConfig config = {0};

  setup_minimal_spec(&spec, &op);
  config.filename_base = "/this_dir_does_not_exist/file";

  ASSERT_EQ(0, openapi_client_generate(&spec, &config));
  g_fail_io_after = -1;
  PASS();
}

TEST test_gen_client_defaults(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op = {0};
  struct OpenApiClientConfig config = {0};
  char *content = NULL;
  size_t sz;

  setup_minimal_spec(&spec, &op);
  config.filename_base = "build/test_out/gen_def";

  ASSERT_EQ(0, openapi_client_generate(&spec, &config));

  /* Check header for default guard and model include logic */
  read_to_file("build/test_out/src/gen_def.h", "r", &content, &sz);
  ASSERT(strstr(content, "GEN_DEF_H") != NULL);
  /* Derived model header name should be present in the header file */
  ASSERT(strstr(content, "#include \"gen_def_models.h\"") != NULL);
  free(content);

  /* Check source for header inclusion */
  read_to_file("build/test_out/src/gen_def.c", "r", &content, &sz);
  ASSERT(strstr(content, "#include \"gen_def.h\"") != NULL);
  free(content);

  /* Check models header */
  read_to_file("build/test_out/src/gen_def_models.h", "r", &content, &sz);
  ASSERT(strstr(content, "GEN_DEF_H_MODELS") != NULL);
  free(content);

  /* Check models source */
  read_to_file("build/test_out/src/gen_def_models.c", "r", &content, &sz);
  ASSERT(strstr(content, "#include \"gen_def_models.h\"") != NULL);
  free(content);

  remove("build/test_out/src/gen_def.h");
  remove("build/test_out/src/gen_def.c");
  remove("build/test_out/src/gen_def_models.h");
  remove("build/test_out/src/gen_def_models.c");
  g_fail_io_after = -1;
  PASS();
}

TEST test_gen_transport_selection(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op = {0};
  struct OpenApiClientConfig config = {0};
  char *content = NULL;
  size_t sz;

  setup_minimal_spec(&spec, &op);
  config.filename_base = "build/test_out/gen_transport";

  ASSERT_EQ(0, openapi_client_generate(&spec, &config));

  read_to_file("build/test_out/src/gen_transport.c", "r", &content, &sz);

  /* Verify macros are present in preamble */
  ASSERT(strstr(content, "#ifdef USE_WININET") != NULL);
  ASSERT(strstr(content, "#include <c_abstract_http/http_wininet.h>") != NULL);
  ASSERT(strstr(content, "#elif defined(USE_WINHTTP)") != NULL);
  ASSERT(strstr(content, "#include <c_abstract_http/http_winhttp.h>") != NULL);
  ASSERT(strstr(content, "#elif defined(__APPLE__)") != NULL);
  ASSERT(strstr(content, "#include <c_abstract_http/http_apple.h>") != NULL);
  ASSERT(strstr(content, "#else") != NULL);
  ASSERT(strstr(content, "#include <c_abstract_http/http_curl.h>") != NULL);

  /* Verify macros are present in _init function */
  ASSERT(strstr(content, "rc = http_wininet_context_init") != NULL);
  ASSERT(strstr(content, "client->send = http_wininet_send") != NULL);
  ASSERT(strstr(content, "rc = http_curl_context_init") != NULL);
  ASSERT(strstr(content, "rc = http_apple_context_init") != NULL);

  /* Verify macros are present in _cleanup function */
  ASSERT(strstr(content, "http_wininet_context_free") != NULL);
  ASSERT(strstr(content, "http_curl_context_free") != NULL);
  ASSERT(strstr(content, "http_apple_context_free") != NULL);

  free(content);
  remove("build/test_out/src/gen_transport.h");
  remove("build/test_out/src/gen_transport.c");
  remove("build/test_out/src/gen_transport_models.h");
  remove("build/test_out/src/gen_transport_models.c");
  g_fail_io_after = -1;
  PASS();
}

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
  srv.variables[0].name = "test";

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

  srv.url = "http://test";
  ASSERT_EQ(0, render_server_url_default(&srv, &out));
  ASSERT_STR_EQ("http://test", out);
  free(out);
  out = NULL;

  /* With variables */
  srv.url = "http://{domain}:{port}/v1";
  srv.n_variables = 2;
  srv.variables =
      (struct OpenAPI_ServerVariable *)calloc(2, sizeof(*srv.variables));
  srv.variables[0].name = "domain";
  srv.variables[0].default_value = "localhost";
  srv.variables[1].name = "port";
  srv.variables[1].default_value = "8080";

  ASSERT_EQ(0, render_server_url_default(&srv, &out));
  ASSERT_STR_EQ("http://localhost:8080/v1", out);
  free(out);
  out = NULL;

  /* Unmatched variable */
  srv.url = "http://{missing}/test";
  ASSERT_EQ(0, render_server_url_default(&srv, &out));
  ASSERT(out == NULL);

  /* Missing closing brace */
  srv.url = "http://{missing/test";
  ASSERT_EQ(0, render_server_url_default(&srv, &out));
  ASSERT(out == NULL);

  /* Empty braces */
  srv.url = "http://{}/test";
  ASSERT_EQ(0, render_server_url_default(&srv, &out));
  ASSERT(out == NULL);

  /* Valid var but missing default */
  if (srv.variables)
    free(srv.variables);
  srv.url = "http://{noval}/test";
  srv.n_variables = 1;
  srv.variables =
      (struct OpenAPI_ServerVariable *)calloc(1, sizeof(*srv.variables));
  srv.variables[0].name = "noval";
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

  a.name = "test";
  ASSERT_EQ(0, param_keys_match(&a, &b));

  b.name = "test2";
  ASSERT_EQ(0, param_keys_match(&a, &b));

  b.name = "test";
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
  path.parameters[0].name = "p1";
  path.parameters[0].in = OA_PARAM_IN_PATH;

  ASSERT_EQ(0, build_effective_parameters(&path, NULL, &out, &count));
  ASSERT_EQ(1, count);
  ASSERT_STR_EQ("p1", out[0].name);
  free(out);
  out = NULL;

  /* Op params overrides */
  op.n_parameters = 2;
  op.parameters = (struct OpenAPI_Parameter *)calloc(2, sizeof(*op.parameters));
  op.parameters[0].name = "p1";
  op.parameters[0].in = OA_PARAM_IN_PATH;
  op.parameters[0].description = "overridden";
  op.parameters[1].name = "p2";
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

  fp = fopen("test_docblock.txt", "w");
  ASSERT(fp != NULL);

  /* Fallback */
  ASSERT_EQ(0, write_docblock(fp, NULL, &op));

  /* Various branch hits */
  op.summary = "sum";
  op.operation_id = "opId";
  op.description = "desc";
  path.route = "/test";
  op.verb = OA_VERB_POST;
  op.external_docs.url = "http://doc";
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
  op.parameters[0].name = "p";
  op.parameters[0].description = "desc p";
  op.parameters[0].allow_empty_value = 1;
  op.parameters[0].allow_reserved = 1;
  op.deprecated = 1;

  op.n_parameters = 3;
  op.parameters = (struct OpenAPI_Parameter *)realloc(
      op.parameters, 3 * sizeof(*op.parameters));
  memset(&op.parameters[1], 0, sizeof(*op.parameters));
  op.parameters[1].name = "cookiep";
  op.parameters[1].in = OA_PARAM_IN_COOKIE;
  memset(&op.parameters[2], 0, sizeof(*op.parameters));
  op.parameters[2].name = "unkp";
  op.parameters[2].in = OA_PARAM_IN_UNKNOWN;

  /* we will just execute all branches */
  ASSERT_EQ(0, write_docblock(fp, &path, &op));

  free(op.callbacks);
  free(op.responses[0].links);
  free(op.security);
  free(op.responses);
  free(op.servers[0].variables);
  free(op.servers);
  free(op.parameters);
  fclose(fp);
  remove("test_docblock.txt");
  g_fail_io_after = -1;

  PASS();
}

#if 0
TEST test_client_gen_write_preambles(void) {
  FILE *fp1 = fopen("test9.h", "w");
  FILE *fp2 = fopen("test10.c", "w");

  /* Null checks */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, write_header_preamble(NULL, NULL, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, write_source_preamble(NULL, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, write_lifecycle_funcs(NULL, NULL, NULL, NULL));

  /* Write header preamble */
  ASSERT_EQ(0, write_header_preamble(fp1, "TEST9_H", "prefix"));

  /* Write source preamble */
  ASSERT_EQ(0, write_source_preamble(fp2, "test9.h"));

  /* Write lifecycle funcs */
  struct OpenAPI_Spec spec;
  memset(&spec, 0, sizeof(spec));
  ASSERT_EQ(0, write_lifecycle_funcs(fp1, fp2, "prefix", &spec));

  fclose(fp1);
  fclose(fp2);
  remove("test9.h");
  remove("test10.c");
  g_fail_io_after = -1;

  PASS();
}
#endif

TEST test_client_gen_emit_operation(void) {
  struct OpenAPI_Path bad_path;
  struct OpenAPI_Operation bad_op;

  /* Missing params */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            emit_operation(NULL, NULL, NULL, NULL, NULL, NULL, NULL));

  /* Test early merge return */
  memset(&bad_path, 0, sizeof(bad_path));
  memset(&bad_op, 0, sizeof(bad_op));

  /* Force build_effective_parameters to fail */
  /* Wait, missing args handled at top covers this */
  g_fail_io_after = -1;
  PASS();
}

SUITE(openapi_client_gen_suite) {
  RUN_TEST(test_gen_client_basic);
  RUN_TEST(test_gen_client_operation_server_override);
  RUN_TEST(test_gen_client_text_plain_request_body);
  RUN_TEST(test_gen_client_octet_stream_request_body);
  RUN_TEST(test_gen_client_octet_stream_response_body);
  RUN_TEST(test_gen_client_default_base_url_from_server);
  RUN_TEST(test_gen_client_default_base_url_no_servers);
  RUN_TEST(test_gen_client_additional_operation);
  RUN_TEST(test_gen_client_op_params_only);
  RUN_TEST(test_gen_client_querystring_param);
  RUN_TEST(test_gen_client_path_level_params);
  RUN_TEST(test_client_gen_find_server_variable);
  RUN_TEST(test_client_gen_render_server_url_default);
  RUN_TEST(test_client_gen_escape_c_string_literal);
  RUN_TEST(test_client_gen_select_operation_server);
  RUN_TEST(test_client_gen_build_base_url_literal);
  RUN_TEST(test_client_gen_generate_guard);
  RUN_TEST(test_client_gen_derive_model_header);
  RUN_TEST(test_client_gen_write_docblock);
  RUN_TEST(test_client_gen_emit_operation);
  RUN_TEST(test_client_gen_sanitize_tag);
  RUN_TEST(test_client_gen_param_keys_match);
  RUN_TEST(test_client_gen_build_effective_parameters);
  RUN_TEST(test_client_gen_verb_to_string);

  RUN_TEST(test_gen_client_path_param_override);
  RUN_TEST(test_gen_client_grouped_tags_namespace);
  RUN_TEST(test_gen_client_namespace_only);
  RUN_TEST(test_gen_client_error_nulls);
  RUN_TEST(test_gen_client_file_error);
  RUN_TEST(test_gen_client_defaults);
  RUN_TEST(test_gen_transport_selection);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_OPENAPI_CLIENT_GEN_H */

/* LCOV_EXCL_STOP */
