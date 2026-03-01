/**
 * @file test_openapi_client_gen.h
 * @brief Tests for the OpenAPI Client Library Generator.
 */

#ifndef TEST_OPENAPI_CLIENT_GEN_H
#define TEST_OPENAPI_CLIENT_GEN_H

#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cdd_test_helpers/cdd_helpers.h"
#include "functions/parse/fs.h"
#include "openapi/parse/openapi.h"
#include "routes/emit/client_gen.h"

static void setup_minimal_spec(struct OpenAPI_Spec *spec,
                               struct OpenAPI_Operation *op) {
  static struct OpenAPI_Path path;
  static struct OpenAPI_Response resp;

  openapi_spec_init(spec);

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
  struct OpenAPI_Operation op;
  struct OpenApiClientConfig config;
  const char *base = "gen_client_test";
  char *h_file = "gen_client_test.h";
  char *c_file = "gen_client_test.c";
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
  PASS();
}

TEST test_gen_client_operation_server_override(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  struct OpenAPI_Server op_server;
  struct OpenApiClientConfig config;
  const char *base = "gen_client_op_server";
  char *h_file = "gen_client_op_server.h";
  char *c_file = "gen_client_op_server.c";
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
  PASS();
}

TEST test_gen_client_text_plain_request_body(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  struct OpenApiClientConfig config;
  const char *base = "gen_client_text_plain_req";
  char *h_file = "gen_client_text_plain_req.h";
  char *c_file = "gen_client_text_plain_req.c";
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
  PASS();
}

TEST test_gen_client_octet_stream_request_body(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  struct OpenApiClientConfig config;
  const char *base = "gen_client_octet_req";
  char *h_file = "gen_client_octet_req.h";
  char *c_file = "gen_client_octet_req.c";
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
  PASS();
}

TEST test_gen_client_octet_stream_response_body(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  struct OpenApiClientConfig config;
  const char *base = "gen_client_octet_resp";
  char *h_file = "gen_client_octet_resp.h";
  char *c_file = "gen_client_octet_resp.c";
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
  PASS();
}

TEST test_gen_client_default_base_url_from_server(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  struct OpenApiClientConfig config;
  struct OpenAPI_Server server;
  struct OpenAPI_ServerVariable var;
  const char *base = "gen_client_default_url";
  char *h_file = "gen_client_default_url.h";
  char *c_file = "gen_client_default_url.c";
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
  PASS();
}

TEST test_gen_client_default_base_url_no_servers(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  struct OpenApiClientConfig config;
  const char *base = "gen_client_default_url_none";
  char *h_file = "gen_client_default_url_none.h";
  char *c_file = "gen_client_default_url_none.c";
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
  PASS();
}

TEST test_gen_client_additional_operation(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  struct OpenAPI_Response resp;
  struct OpenApiClientConfig config;
  struct OpenAPI_Path path;
  const char *base = "gen_additional_op";
  char *h_file = "gen_additional_op.h";
  char *c_file = "gen_additional_op.c";
  char *content = NULL;
  size_t sz;
  int rc;

  openapi_spec_init(&spec);
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
  PASS();
}

TEST test_gen_client_op_params_only(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  struct OpenApiClientConfig config;
  struct OpenAPI_Parameter op_param;
  const char *base = "gen_op_params";
  char *h_file = "gen_op_params.h";
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
  remove("gen_op_params.c");
  PASS();
}

TEST test_gen_client_querystring_param(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  struct OpenApiClientConfig config;
  struct OpenAPI_Parameter op_param;
  const char *base = "gen_querystring_param";
  char *h_file = "gen_querystring_param.h";
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
  remove("gen_querystring_param.c");
  PASS();
}

TEST test_gen_client_path_level_params(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  struct OpenApiClientConfig config;
  struct OpenAPI_Parameter path_param;
  const char *base = "gen_path_params";
  char *h_file = "gen_path_params.h";
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
  remove("gen_path_params.c");
  PASS();
}

TEST test_gen_client_path_param_override(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  struct OpenApiClientConfig config;
  struct OpenAPI_Parameter path_param;
  struct OpenAPI_Parameter op_param;
  const char *base = "gen_path_override";
  char *h_file = "gen_path_override.h";
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
  remove("gen_path_override.c");
  PASS();
}

TEST test_gen_client_grouped_tags_namespace(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  struct OpenApiClientConfig config;
  const char *base = "gen_group_ns_test";
  char *h_file = "gen_group_ns_test.h";
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
  remove("gen_group_ns_test.c");
  PASS();
}

TEST test_gen_client_namespace_only(void) {
  /* Case: Namespace present, but no tags on operation */
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  struct OpenApiClientConfig config;
  const char *base = "gen_ns_only_test";
  char *h_file = "gen_ns_only_test.h";
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
  remove("gen_ns_only_test.c");
  PASS();
}

TEST test_gen_client_error_nulls(void) {
  struct OpenAPI_Spec spec;
  struct OpenApiClientConfig config = {0};
  /* Use explicit struct instead of compound literal */
  struct OpenAPI_Operation dummy_op;
  memset(&dummy_op, 0, sizeof(dummy_op));

  setup_minimal_spec(&spec, &dummy_op);

  ASSERT_EQ(EINVAL, openapi_client_generate(NULL, &config));
  ASSERT_EQ(EINVAL, openapi_client_generate(&spec, NULL));

  config.filename_base = NULL;
  ASSERT_EQ(EINVAL, openapi_client_generate(&spec, &config));

  PASS();
}

TEST test_gen_client_file_error(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  struct OpenApiClientConfig config = {0};

  setup_minimal_spec(&spec, &op);
  config.filename_base = "/";

  ASSERT_NEQ(0, openapi_client_generate(&spec, &config));
  PASS();
}

TEST test_gen_client_defaults(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  struct OpenApiClientConfig config = {0};
  char *content;
  size_t sz;

  setup_minimal_spec(&spec, &op);
  config.filename_base = "gen_def";

  ASSERT_EQ(0, openapi_client_generate(&spec, &config));

  /* Check header for default guard and model include logic */
  read_to_file("gen_def.h", "r", &content, &sz);
  ASSERT(strstr(content, "GEN_DEF_H") != NULL);
  /* Derived model header name should be present in the header file */
  ASSERT(strstr(content, "#include \"gen_def_models.h\"") != NULL);
  free(content);

  /* Check source for header inclusion */
  read_to_file("gen_def.c", "r", &content, &sz);
  ASSERT(strstr(content, "#include \"gen_def.h\"") != NULL);
  free(content);

  remove("gen_def.h");
  remove("gen_def.c");
  PASS();
}

TEST test_gen_transport_selection(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  struct OpenApiClientConfig config = {0};
  char *content;
  size_t sz;

  setup_minimal_spec(&spec, &op);
  config.filename_base = "gen_transport";

  ASSERT_EQ(0, openapi_client_generate(&spec, &config));

  read_to_file("gen_transport.c", "r", &content, &sz);

  /* Verify macros are present in preamble */
  ASSERT(strstr(content, "#ifdef USE_WININET") != NULL);
  ASSERT(strstr(content, "#include \"http_wininet.h\"") != NULL);
  ASSERT(strstr(content, "#elif defined(USE_WINHTTP)") != NULL);
  ASSERT(strstr(content, "#include \"http_winhttp.h\"") != NULL);
  ASSERT(strstr(content, "#else") != NULL);
  ASSERT(strstr(content, "#include \"http_curl.h\"") != NULL);

  /* Verify macros are present in _init function */
  ASSERT(strstr(content, "rc = http_wininet_context_init") != NULL);
  ASSERT(strstr(content, "client->send = http_wininet_send") != NULL);
  ASSERT(strstr(content, "rc = http_curl_context_init") != NULL);

  /* Verify macros are present in _cleanup function */
  ASSERT(strstr(content, "http_wininet_context_free") != NULL);
  ASSERT(strstr(content, "http_curl_context_free") != NULL);

  free(content);
  remove("gen_transport.h");
  remove("gen_transport.c");
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
  RUN_TEST(test_gen_client_path_param_override);
  RUN_TEST(test_gen_client_grouped_tags_namespace);
  RUN_TEST(test_gen_client_namespace_only);
  RUN_TEST(test_gen_client_error_nulls);
  RUN_TEST(test_gen_client_file_error);
  RUN_TEST(test_gen_client_defaults);
  RUN_TEST(test_gen_transport_selection);
}

#endif /* TEST_OPENAPI_CLIENT_GEN_H */
