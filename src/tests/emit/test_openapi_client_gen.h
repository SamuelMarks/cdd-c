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

extern C_CDD_EXPORT int g_fail_io_after;
extern C_CDD_EXPORT int g_io_calls;
extern C_CDD_EXPORT int g_client_gen_fail;

static cdd_c_error_t setup_minimal_spec(struct OpenAPI_Spec *spec,
                                        struct OpenAPI_Operation *op) {
  static struct OpenAPI_Path path;
  static struct OpenAPI_Response resp = {0};
  cdd_c_error_t rc;

  rc = openapi_spec_init(spec);
  if (rc != CDD_C_SUCCESS) {
    return rc;
  }
  memset(op, 0, sizeof(*op));
  op->operation_id = (char *)(size_t)(size_t) "test_op";
  op->verb = OA_VERB_GET;

  memset(&resp, 0, sizeof(resp));
  resp.code = (char *)(size_t)(size_t) "200";
  op->responses = &resp;
  op->n_responses = 1;

  memset(&path, 0, sizeof(path));
  path.route = (char *)(size_t)(size_t) "/test";
  path.operations = op;
  path.n_operations = 1;

  spec->paths = &path;
  spec->n_paths = 1;
  return CDD_C_SUCCESS;
}

TEST test_gen_client_basic(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op = {0};
  struct OpenApiClientConfig config;
  const char *base = "build/test_out/gen_client_test";
  char *h_file =
      (char *)(size_t)(size_t) "build/test_out/src/gen_client_test.h";
  char *c_file =
      (char *)(size_t)(size_t) "build/test_out/src/gen_client_test.c";
  char *content = NULL;
  size_t sz;
  int rc;

  ASSERT_EQ(CDD_C_SUCCESS, setup_minimal_spec(&spec, &op));

  memset(&config, 0, sizeof(config));
  config.filename_base = base;
  config.func_prefix = (char *)(size_t)(size_t) "api_";
  config.model_header = (char *)(size_t)(size_t) "my_models.h";

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
  char *h_file =
      (char *)(size_t)(size_t) "build/test_out/src/gen_client_op_server.h";
  char *c_file =
      (char *)(size_t)(size_t) "build/test_out/src/gen_client_op_server.c";
  char *content = NULL;
  size_t sz;
  int rc;

  ASSERT_EQ(CDD_C_SUCCESS, setup_minimal_spec(&spec, &op));

  memset(&op_server, 0, sizeof(op_server));
  op_server.url = (char *)(size_t)(size_t) "https://op.example.com/api";
  op.servers = &op_server;
  op.n_servers = 1;

  memset(&config, 0, sizeof(config));
  config.filename_base = base;
  config.func_prefix = (char *)(size_t)(size_t) "api_";

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
  char *h_file =
      (char *)(size_t)(size_t) "build/test_out/src/gen_client_text_plain_req.h";
  char *c_file =
      (char *)(size_t)(size_t) "build/test_out/src/gen_client_text_plain_req.c";
  char *content = NULL;
  size_t sz;
  int rc;

  ASSERT_EQ(CDD_C_SUCCESS, setup_minimal_spec(&spec, &op));
  op.verb = OA_VERB_POST;
  op.req_body.content_type = (char *)(size_t)(size_t) "text/plain";
  op.req_body.inline_type = (char *)(size_t)(size_t) "string";

  memset(&config, 0, sizeof(config));
  config.filename_base = base;
  config.func_prefix = (char *)(size_t)(size_t) "api_";

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
  char *h_file =
      (char *)(size_t)(size_t) "build/test_out/src/gen_client_octet_req.h";
  char *c_file =
      (char *)(size_t)(size_t) "build/test_out/src/gen_client_octet_req.c";
  char *content = NULL;
  size_t sz;
  int rc;

  ASSERT_EQ(CDD_C_SUCCESS, setup_minimal_spec(&spec, &op));
  op.verb = OA_VERB_POST;
  op.req_body.content_type =
      (char *)(size_t)(size_t) "application/octet-stream";

  memset(&config, 0, sizeof(config));
  config.filename_base = base;
  config.func_prefix = (char *)(size_t)(size_t) "api_";

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
  char *h_file =
      (char *)(size_t)(size_t) "build/test_out/src/gen_client_octet_resp.h";
  char *c_file =
      (char *)(size_t)(size_t) "build/test_out/src/gen_client_octet_resp.c";
  char *content = NULL;
  size_t sz;
  int rc;

  ASSERT_EQ(CDD_C_SUCCESS, setup_minimal_spec(&spec, &op));
  op.responses[0].content_type =
      (char *)(size_t)(size_t) "application/octet-stream";

  memset(&config, 0, sizeof(config));
  config.filename_base = base;
  config.func_prefix = (char *)(size_t)(size_t) "api_";

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
  char *h_file =
      (char *)(size_t)(size_t) "build/test_out/src/gen_client_default_url.h";
  char *c_file =
      (char *)(size_t)(size_t) "build/test_out/src/gen_client_default_url.c";
  char *content = NULL;
  size_t sz;
  int rc;

  ASSERT_EQ(CDD_C_SUCCESS, setup_minimal_spec(&spec, &op));

  memset(&server, 0, sizeof(server));
  memset(&var, 0, sizeof(var));
  server.url = (char *)(size_t)(size_t) "https://{env}.example.com/v1";
  var.name = (char *)(size_t)(size_t) "env";
  var.default_value = (char *)(size_t)(size_t) "api";
  server.variables = &var;
  server.n_variables = 1;
  spec.servers = &server;
  spec.n_servers = 1;

  memset(&config, 0, sizeof(config));
  config.filename_base = base;
  config.func_prefix = (char *)(size_t)(size_t) "api_";

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
  char *h_file = (char *)(size_t)(size_t) "build/test_out/src/"
                                          "gen_client_default_url_none.h";
  char *c_file = (char *)(size_t)(size_t) "build/test_out/src/"
                                          "gen_client_default_url_none.c";
  char *content = NULL;
  size_t sz;
  int rc;

  ASSERT_EQ(CDD_C_SUCCESS, setup_minimal_spec(&spec, &op));

  memset(&config, 0, sizeof(config));
  config.filename_base = base;
  config.func_prefix = (char *)(size_t)(size_t) "api_";

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
  char *h_file =
      (char *)(size_t)(size_t) "build/test_out/src/gen_additional_op.h";
  char *c_file =
      (char *)(size_t)(size_t) "build/test_out/src/gen_additional_op.c";
  char *content = NULL;
  size_t sz;
  int rc;

  ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));
  memset(&path, 0, sizeof(path));

  op.operation_id = (char *)(size_t)(size_t) "custom_connect";
  op.verb = OA_VERB_UNKNOWN;
  op.is_additional = 1;
  op.method = (char *)(size_t)(size_t) "CONNECT";
  resp.code = (char *)(size_t)(size_t) "200";
  op.responses = &resp;
  op.n_responses = 1;

  path.route = (char *)(size_t)(size_t) "/custom";
  path.additional_operations = &op;
  path.n_additional_operations = 1;

  spec.paths = &path;
  spec.n_paths = 1;

  memset(&config, 0, sizeof(config));
  config.filename_base = base;
  config.func_prefix = (char *)(size_t)(size_t) "api_";

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
  char *h_file = (char *)(size_t)(size_t) "build/test_out/src/gen_op_params.h";
  char *content = NULL;
  size_t sz;
  int rc;

  ASSERT_EQ(CDD_C_SUCCESS, setup_minimal_spec(&spec, &op));

  memset(&op_param, 0, sizeof(op_param));
  op_param.name = (char *)(size_t)(size_t) "limit";
  op_param.in = OA_PARAM_IN_QUERY;
  op_param.type = (char *)(size_t)(size_t) "integer";
  op.parameters = &op_param;
  op.n_parameters = 1;

  memset(&config, 0, sizeof(config));
  config.filename_base = base;
  config.func_prefix = (char *)(size_t)(size_t) "api_";

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
  char *h_file =
      (char *)(size_t)(size_t) "build/test_out/src/gen_querystring_param.h";
  char *content = NULL;
  size_t sz;
  int rc;

  ASSERT_EQ(CDD_C_SUCCESS, setup_minimal_spec(&spec, &op));

  memset(&op_param, 0, sizeof(op_param));
  op_param.name = (char *)(size_t)(size_t) "qs";
  op_param.in = OA_PARAM_IN_QUERYSTRING;
  op_param.type = (char *)(size_t)(size_t) "string";
  op.parameters = &op_param;
  op.n_parameters = 1;

  memset(&config, 0, sizeof(config));
  config.filename_base = base;
  config.func_prefix = (char *)(size_t)(size_t) "api_";

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
  char *h_file =
      (char *)(size_t)(size_t) "build/test_out/src/gen_path_params.h";
  char *content = NULL;
  size_t sz;
  int rc;

  ASSERT_EQ(CDD_C_SUCCESS, setup_minimal_spec(&spec, &op));

  memset(&path_param, 0, sizeof(path_param));
  path_param.name = (char *)(size_t)(size_t) "x_trace";
  path_param.in = OA_PARAM_IN_HEADER;
  path_param.type = (char *)(size_t)(size_t) "string";

  spec.paths[0].parameters = &path_param;
  spec.paths[0].n_parameters = 1;

  memset(&config, 0, sizeof(config));
  config.filename_base = base;
  config.func_prefix = (char *)(size_t)(size_t) "api_";

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
  char *h_file =
      (char *)(size_t)(size_t) "build/test_out/src/gen_path_override.h";
  char *content = NULL;
  size_t sz;
  int rc;

  ASSERT_EQ(CDD_C_SUCCESS, setup_minimal_spec(&spec, &op));

  memset(&path_param, 0, sizeof(path_param));
  path_param.name = (char *)(size_t)(size_t) "id";
  path_param.in = OA_PARAM_IN_PATH;
  path_param.type = (char *)(size_t)(size_t) "integer";

  memset(&op_param, 0, sizeof(op_param));
  op_param.name = (char *)(size_t)(size_t) "id";
  op_param.in = OA_PARAM_IN_PATH;
  op_param.type = (char *)(size_t)(size_t) "string";

  spec.paths[0].parameters = &path_param;
  spec.paths[0].n_parameters = 1;
  op.parameters = &op_param;
  op.n_parameters = 1;

  memset(&config, 0, sizeof(config));
  config.filename_base = base;
  config.func_prefix = (char *)(size_t)(size_t) "api_";

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
  char *h_file =
      (char *)(size_t)(size_t) "build/test_out/src/gen_group_ns_test.h";
  char *content = NULL;
  size_t sz;
  int rc;
  /* Tag string array setup */
  static const char *tags[] = {"pet"};

  ASSERT_EQ(CDD_C_SUCCESS, setup_minimal_spec(&spec, &op));

  /* Inject tag manually */
  op.tags = (char **)(size_t)tags;
  op.n_tags = 1;

  memset(&config, 0, sizeof(config));
  config.filename_base = base;
  config.func_prefix = (char *)(size_t)(size_t) "api_";
  config.namespace_prefix = (char *)(size_t)(size_t) "Foo";

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
  char *h_file =
      (char *)(size_t)(size_t) "build/test_out/src/gen_ns_only_test.h";
  char *content = NULL;
  size_t sz;
  int rc;

  ASSERT_EQ(CDD_C_SUCCESS, setup_minimal_spec(&spec, &op));
  /* No tags */

  memset(&config, 0, sizeof(config));
  config.filename_base = base;
  config.func_prefix = (char *)(size_t)(size_t) "api_";
  config.namespace_prefix = (char *)(size_t)(size_t) "Bar";

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

  ASSERT_EQ(CDD_C_SUCCESS, setup_minimal_spec(&spec, &dummy_op));

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
  int rc;

  ASSERT_EQ(CDD_C_SUCCESS, setup_minimal_spec(&spec, &op));
  config.filename_base =
      (char *)(size_t)(size_t) "/this_dir_does_not_exist/file";
  g_io_calls = 0;
  g_fail_io_after = 1;

  rc = openapi_client_generate(&spec, &config);
  ASSERT(rc == CDD_C_ERROR_IO || rc == CDD_C_ERROR_NOT_FOUND);
  g_fail_io_after = -1;
  PASS();
}

TEST test_gen_client_defaults(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op = {0};
  struct OpenApiClientConfig config = {0};
  char *content = NULL;
  size_t sz;

  ASSERT_EQ(CDD_C_SUCCESS, setup_minimal_spec(&spec, &op));
  config.filename_base = (char *)(size_t)(size_t) "build/test_out/gen_def";

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

  ASSERT_EQ(CDD_C_SUCCESS, setup_minimal_spec(&spec, &op));
  config.filename_base =
      (char *)(size_t)(size_t) "build/test_out/gen_transport";

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

TEST test_client_gen_defined_schemas(void) {
  struct OpenAPI_Spec spec;
  struct StructFields sf[3];
  char *names[3];
  struct OpenApiClientConfig config;
  char *content = NULL;
  size_t sz;
  int rc;

  memset(&spec, 0, sizeof(spec));
  memset(sf, 0, sizeof(sf));
  memset(&config, 0, sizeof(config));

  names[0] = (char *)(size_t)(size_t) "MyEnum";
  sf[0].is_enum = 1;

  names[1] = (char *)(size_t)(size_t) "MyUnion";
  sf[1].is_union = 1;

  names[2] = (char *)(size_t)(size_t) "MyStruct";

  spec.defined_schemas = sf;
  spec.defined_schema_names = names;
  spec.n_defined_schemas = 3;

  config.filename_base =
      (char *)(size_t)(size_t) "build/test_out/test_client_gen_schemas";
  config.func_prefix = (char *)(size_t)(size_t) "api_";

  rc = openapi_client_generate(&spec, &config);
  ASSERT_EQ(0, rc);

  read_to_file("build/test_out/src/test_client_gen_schemas_models.h", "r",
               &content, &sz);
  ASSERT(content != NULL);
  free(content);

  remove("build/test_out/src/test_client_gen_schemas_models.h");
  remove("build/test_out/src/test_client_gen_schemas_models.c");
  remove("build/test_out/src/test_client_gen_schemas.h");
  remove("build/test_out/src/test_client_gen_schemas.c");
  remove("build/test_out/src/url_utils.h");
  remove("build/test_out/src/url_utils.c");

  g_fail_io_after = -1;
  PASS();
}

TEST test_client_gen_create_tests_mocks(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation ops[5];
  struct OpenAPI_Response resps[5];
  struct OpenAPI_Path path;
  struct OpenApiClientConfig config;
  struct OpenAPI_Parameter param;
  int rc;

  memset(&spec, 0, sizeof(spec));
  memset(ops, 0, sizeof(ops));
  memset(resps, 0, sizeof(resps));
  memset(&path, 0, sizeof(path));
  memset(&config, 0, sizeof(config));
  memset(&param, 0, sizeof(param));

  path.route = (char *)(size_t)(size_t) "/items/{id}";
  param.name = (char *)(size_t)(size_t) "filter";
  param.in = OA_PARAM_IN_QUERY;
  param.type = (char *)(size_t)(size_t) "string";

  ops[0].operation_id = (char *)(size_t)(size_t) "getItems";
  ops[0].verb = OA_VERB_GET;
  resps[0].code = (char *)(size_t)(size_t) "200";
  ops[0].responses = &resps[0];
  ops[0].n_responses = 1;
  ops[0].parameters = &param;
  ops[0].n_parameters = 1;

  ops[1].operation_id = (char *)(size_t)(size_t) "postItem";
  ops[1].verb = OA_VERB_POST;
  ops[1].req_body.is_array = 0;
  resps[1].code = (char *)(size_t)(size_t) "201";
  ops[1].responses = &resps[1];
  ops[1].n_responses = 1;

  ops[2].operation_id = (char *)(size_t)(size_t) "putItem";
  ops[2].verb = OA_VERB_PUT;
  ops[2].req_body.is_array = 1;
  resps[2].code = (char *)(size_t)(size_t) "200";
  ops[2].responses = &resps[2];
  ops[2].n_responses = 1;

  ops[3].operation_id = (char *)(size_t)(size_t) "deleteItem";
  ops[3].verb = OA_VERB_DELETE;
  resps[3].code = (char *)(size_t)(size_t) "204";
  ops[3].responses = &resps[3];
  ops[3].n_responses = 1;

  ops[4].operation_id = (char *)(size_t)(size_t) "patchItem";
  ops[4].verb = OA_VERB_PATCH;
  resps[4].code = (char *)(size_t)(size_t) "200";
  ops[4].responses = &resps[4];
  ops[4].n_responses = 1;

  path.operations = ops;
  path.n_operations = 5;

  spec.paths = &path;
  spec.n_paths = 1;

  config.filename_base =
      (char *)(size_t)(size_t) "build/test_out/test_client_sdk";
  config.create_tests_and_mocks = 1;
  config.no_installable_package = 1;

  rc = openapi_client_generate(&spec, &config);
  ASSERT_EQ(0, rc);

  remove("build/test_out/src/test/test_sdk.c");
  remove("build/test_out/src/test_client_sdk_models.h");
  remove("build/test_out/src/test_client_sdk_models.c");
  remove("build/test_out/src/test_client_sdk.h");
  remove("build/test_out/src/test_client_sdk.c");

  g_fail_io_after = -1;
  PASS();
}

TEST test_client_gen_mock_errors(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  struct OpenApiClientConfig config;
  struct OpenAPI_Server srv;
  struct OpenAPI_ServerVariable var;
  struct StructFields sf[3];
  char *names[3];
  char *out = NULL;
  int rc;

  memset(&op, 0, sizeof(op));

  /* Setup server with variable for render_server_url_default mocks */
  memset(&srv, 0, sizeof(srv));
  memset(&var, 0, sizeof(var));
  var.name = (char *)(size_t)(size_t) "v";
  var.default_value = (char *)(size_t)(size_t) "val";
  srv.url = (char *)(size_t)(size_t) "http://api.com/{v}";
  srv.variables = &var;
  srv.n_variables = 1;

  /* mock 1: pass 1 malloc fail */
  g_client_gen_fail = 1;
  rc = render_server_url_default(&srv, &out);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(out == NULL);

  /* mock 2: out malloc fail */
  g_client_gen_fail = 2;
  rc = render_server_url_default(&srv, &out);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(out == NULL);

  /* mock 3: pass 2 name malloc fail */
  g_client_gen_fail = 3;
  rc = render_server_url_default(&srv, &out);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(out == NULL);

  /* mock 47: pass 2 end == NULL */
  g_client_gen_fail = 47;
  rc = render_server_url_default(&srv, &out);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(out == NULL);

  /* mock 48: pass 2 name_len == 0 */
  g_client_gen_fail = 48;
  rc = render_server_url_default(&srv, &out);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(out == NULL);

  /* mock 49: pass 2 var == NULL */
  g_client_gen_fail = 49;
  rc = render_server_url_default(&srv, &out);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(out == NULL);

  /* mock 4: escape_c_string_literal malloc fail */
  g_client_gen_fail = 4;
  rc = escape_c_string_literal("test", &out);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(out == NULL);

  /* mock 50: build_base_url_literal escaped == NULL */
  g_client_gen_fail = 50;
  rc = build_base_url_literal("http://test", &out);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(out == NULL);

  /* mock 5: build_base_url_literal malloc fail */
  g_client_gen_fail = 5;
  rc = build_base_url_literal("http://test", &out);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(out == NULL);

  /* mock 6: generate_guard malloc fail */
  g_client_gen_fail = 6;
  rc = generate_guard("myguard", &out);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(out == NULL);

  /* mock 7: derive_model_header malloc fail */
  g_client_gen_fail = 7;
  rc = derive_model_header("mybase", &out);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(out == NULL);

  /* mock 8: sanitize_tag malloc fail */
  g_client_gen_fail = 8;
  rc = sanitize_tag("my-tag", &out);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(out == NULL);

  /* mock 9: build_effective_parameters calloc fail */
  {
    struct OpenAPI_Parameter *p_out = NULL;
    size_t p_count = 0;
    struct OpenAPI_Path p_path;
    struct OpenAPI_Parameter p_param;
    memset(&p_path, 0, sizeof(p_path));
    memset(&p_param, 0, sizeof(p_param));
    p_path.parameters = &p_param;
    p_path.n_parameters = 1;
    g_client_gen_fail = 9;
    rc = build_effective_parameters(&p_path, NULL, &p_out, &p_count);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  }

  /* openapi_client_generate mocks */
  ASSERT_EQ(CDD_C_SUCCESS, setup_minimal_spec(&spec, &op));
  memset(&config, 0, sizeof(config));
  config.filename_base = (char *)(size_t)(size_t) "build/test_out/test_cg_mock";
  config.no_installable_package = 1;

  /* Setup schemas */
  memset(sf, 0, sizeof(sf));
  names[0] = (char *)(size_t)(size_t) "Enum1";
  sf[0].is_enum = 1;
  names[1] = (char *)(size_t)(size_t) "Union1";
  sf[1].is_union = 1;
  names[2] = (char *)(size_t)(size_t) "Struct1";
  spec.defined_schemas = sf;
  spec.defined_schema_names = names;
  spec.n_defined_schemas = 3;

  /* mock 24: get_dirname fail */
  g_client_gen_fail = 24;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            openapi_client_generate(&spec, &config));

  /* mock 25: get_basename fail */
  g_client_gen_fail = 25;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            openapi_client_generate(&spec, &config));

  /* mock 10: src_dir fail */
  g_client_gen_fail = 10;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, openapi_client_generate(&spec, &config));

  /* mock 62: makedirs(src_dir) fail */
  g_client_gen_fail = 62;
  ASSERT_EQ(CDD_C_ERROR_IO, openapi_client_generate(&spec, &config));

  /* mock 11: actual_base fail */
  g_client_gen_fail = 11;
  rc = openapi_client_generate(&spec, &config);
  ASSERT(rc == CDD_C_SUCCESS || rc == CDD_C_ERROR_MEMORY);

  /* mock 61: strdup actual_base fail */
  g_client_gen_fail = 61;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, openapi_client_generate(&spec, &config));

  /* mock 12: filenames malloc fail */
  g_client_gen_fail = 12;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, openapi_client_generate(&spec, &config));

  /* mock 16: file open fail */
  g_client_gen_fail = 16;
  ASSERT_EQ(CDD_C_ERROR_IO, openapi_client_generate(&spec, &config));

  /* mock 17: guard fail */
  g_client_gen_fail = 17;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, openapi_client_generate(&spec, &config));

  /* mock 18: model_guard fail */
  g_client_gen_fail = 18;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, openapi_client_generate(&spec, &config));

  /* schema loop mocks 26..38 */
  g_client_gen_fail = 26;
  ASSERT_EQ(CDD_C_ERROR_IO, openapi_client_generate(&spec, &config));

  g_client_gen_fail = 27;
  ASSERT_EQ(CDD_C_ERROR_IO, openapi_client_generate(&spec, &config));

  g_client_gen_fail = 28;
  ASSERT_EQ(CDD_C_ERROR_IO, openapi_client_generate(&spec, &config));

  g_client_gen_fail = 29;
  ASSERT_EQ(CDD_C_ERROR_IO, openapi_client_generate(&spec, &config));

  g_client_gen_fail = 30;
  ASSERT_EQ(CDD_C_ERROR_IO, openapi_client_generate(&spec, &config));

  g_client_gen_fail = 31;
  ASSERT_EQ(CDD_C_ERROR_IO, openapi_client_generate(&spec, &config));

  g_client_gen_fail = 32;
  ASSERT_EQ(CDD_C_ERROR_IO, openapi_client_generate(&spec, &config));

  g_client_gen_fail = 33;
  ASSERT_EQ(CDD_C_ERROR_IO, openapi_client_generate(&spec, &config));

  g_client_gen_fail = 34;
  ASSERT_EQ(CDD_C_ERROR_IO, openapi_client_generate(&spec, &config));

  g_client_gen_fail = 35;
  ASSERT_EQ(CDD_C_ERROR_IO, openapi_client_generate(&spec, &config));

  g_client_gen_fail = 36;
  ASSERT_EQ(CDD_C_ERROR_IO, openapi_client_generate(&spec, &config));

  g_client_gen_fail = 37;
  ASSERT_EQ(CDD_C_ERROR_IO, openapi_client_generate(&spec, &config));

  g_client_gen_fail = 38;
  ASSERT_EQ(CDD_C_ERROR_IO, openapi_client_generate(&spec, &config));

  g_client_gen_fail = 66;
  ASSERT_EQ(CDD_C_ERROR_IO, openapi_client_generate(&spec, &config));

  g_client_gen_fail = 67;
  ASSERT_EQ(CDD_C_ERROR_IO, openapi_client_generate(&spec, &config));

  g_client_gen_fail = 68;
  ASSERT_EQ(CDD_C_ERROR_IO, openapi_client_generate(&spec, &config));

  g_client_gen_fail = 69;
  ASSERT_EQ(CDD_C_ERROR_IO, openapi_client_generate(&spec, &config));

  /* mock 65: get_basename for mh_name fail */
  g_client_gen_fail = 65;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            openapi_client_generate(&spec, &config));

  /* Test with a NULL schema name to test continue branch */
  names[0] = NULL;
  g_client_gen_fail = 0;
  rc = openapi_client_generate(&spec, &config);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  names[0] = (char *)(size_t)(size_t) "Enum1";

  /* mock 21: write_header_preamble fail */
  g_client_gen_fail = 21;
  ASSERT_EQ(CDD_C_ERROR_IO, openapi_client_generate(&spec, &config));

  /* mock 53: get_basename(h_name) fail */
  g_client_gen_fail = 53;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            openapi_client_generate(&spec, &config));

  /* mock 22: write_source_preamble fail */
  g_client_gen_fail = 22;
  ASSERT_EQ(CDD_C_ERROR_IO, openapi_client_generate(&spec, &config));

  /* mock 23: write_lifecycle_funcs fail */
  g_client_gen_fail = 23;
  ASSERT_EQ(CDD_C_ERROR_IO, openapi_client_generate(&spec, &config));

  /* Test with config->header_guard and config->model_header explicitly set */
  config.header_guard = (char *)(size_t)(size_t) "CUSTOM_GUARD_H";
  config.model_header = (char *)(size_t)(size_t) "custom_models.h";
  g_client_gen_fail = 0;
  ASSERT_EQ(CDD_C_SUCCESS, openapi_client_generate(&spec, &config));
  config.header_guard = NULL;
  config.model_header = NULL;

  /* mock 54: uh fopen fail */
  config.no_installable_package = 0;
  g_client_gen_fail = 54;
  ASSERT_EQ(CDD_C_SUCCESS, openapi_client_generate(&spec, &config));

  /* mock 55: uc fopen fail */
  g_client_gen_fail = 55;
  ASSERT_EQ(CDD_C_SUCCESS, openapi_client_generate(&spec, &config));

  /* mock 64: fputs fail */
  g_client_gen_fail = 64;
  ASSERT_EQ(CDD_C_SUCCESS, openapi_client_generate(&spec, &config));

  /* mock 63: fprintf fail */
  g_client_gen_fail = 63;
  ASSERT_EQ(CDD_C_SUCCESS, openapi_client_generate(&spec, &config));

  /* mock 20: generate_cmake_project fail */
  g_client_gen_fail = 20;
  ASSERT_EQ(CDD_C_ERROR_IO, openapi_client_generate(&spec, &config));
  config.no_installable_package = 1;

  /* mock 51: makedirs(tdir) fail */
  config.create_tests_and_mocks = 1;
  g_client_gen_fail = 51;
  ASSERT_EQ(CDD_C_ERROR_IO, openapi_client_generate(&spec, &config));

  /* mock 52: fopen(tfile) fail */
  g_client_gen_fail = 52;
  ASSERT_EQ(CDD_C_SUCCESS, openapi_client_generate(&spec, &config));
  config.create_tests_and_mocks = 0;

  /* mock 70: newline after forward decls */
  g_client_gen_fail = 70;
  ASSERT_EQ(CDD_C_SUCCESS, openapi_client_generate(&spec, &config));

  /* mock 71: emit_operation fail */
  g_client_gen_fail = 71;
  ASSERT_EQ(CDD_C_ERROR_IO, openapi_client_generate(&spec, &config));

  /* mock 72: emit_operation additional fail */
  spec.paths[0].additional_operations = spec.paths[0].operations;
  spec.paths[0].n_additional_operations = 1;
  g_client_gen_fail = 72;
  ASSERT_EQ(CDD_C_ERROR_IO, openapi_client_generate(&spec, &config));
  spec.paths[0].additional_operations = NULL;
  spec.paths[0].n_additional_operations = 0;

  /* mock 74: endif guard fprintf fail */
  g_client_gen_fail = 74;
  ASSERT_EQ(CDD_C_SUCCESS, openapi_client_generate(&spec, &config));

  /* emit_operation mocks 40..46, 56..60 */
  {
    FILE *hf = cdd_test_tmpfile_global();
    FILE *cf = cdd_test_tmpfile_global();
    char *tag = (char *)(size_t)(size_t) "mytag";
    op.tags = &tag;
    op.n_tags = 1;
    config.namespace_prefix = (char *)(size_t)(size_t) "ns";

    g_client_gen_fail = 40;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              emit_operation(hf, cf, spec.paths, &op, &spec, &config, "api_"));

    g_client_gen_fail = 56;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              emit_operation(hf, cf, spec.paths, &op, &spec, &config, "api_"));

    g_client_gen_fail = 41;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              emit_operation(hf, cf, spec.paths, &op, &spec, &config, "api_"));

    g_client_gen_fail = 42;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              emit_operation(hf, cf, spec.paths, &op, &spec, &config, "api_"));

    /* namespace only */
    op.n_tags = 0;
    g_client_gen_fail = 43;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              emit_operation(hf, cf, spec.paths, &op, &spec, &config, "api_"));

    /* tag only */
    op.n_tags = 1;
    config.namespace_prefix = NULL;
    g_client_gen_fail = 44;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              emit_operation(hf, cf, spec.paths, &op, &spec, &config, "api_"));

    /* base_url_expr fail with server override */
    op.servers = &srv;
    op.n_servers = 1;
    g_client_gen_fail = 45;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              emit_operation(hf, cf, spec.paths, &op, &spec, &config, "api_"));

    g_client_gen_fail = 60;
    ASSERT_EQ(CDD_C_ERROR_IO,
              emit_operation(hf, cf, spec.paths, &op, &spec, &config, "api_"));

    g_client_gen_fail = 57;
    ASSERT_EQ(CDD_C_ERROR_IO,
              emit_operation(hf, cf, spec.paths, &op, &spec, &config, "api_"));

    g_client_gen_fail = 58;
    ASSERT_EQ(CDD_C_ERROR_IO,
              emit_operation(hf, cf, spec.paths, &op, &spec, &config, "api_"));

    g_client_gen_fail = 59;
    ASSERT_EQ(CDD_C_ERROR_IO,
              emit_operation(hf, cf, spec.paths, &op, &spec, &config, "api_"));

    g_client_gen_fail = 46;
    ASSERT_EQ(CDD_C_ERROR_IO,
              emit_operation(hf, cf, spec.paths, &op, &spec, &config, "api_"));

    if (hf)
      fclose(hf);
    if (cf)
      fclose(cf);
  }

  g_client_gen_fail = 0;
  remove("build/test_out/src/test_cg_mock.h");
  remove("build/test_out/src/test_cg_mock.c");
  remove("build/test_out/src/test_cg_mock_models.h");
  remove("build/test_out/src/test_cg_mock_models.c");
  remove("build/test_out/src/test/test_sdk.c");
  PASS();
}

TEST test_client_gen_extra_cases(void) {
  struct OpenAPI_Server srv;
  char *out = NULL;
  char *escaped = NULL;
  int rc;

  /* 1. Unclosed brace */
  memset(&srv, 0, sizeof(srv));
  srv.url = (char *)(size_t)(size_t) "http://api.com/{unclosed";
  rc = render_server_url_default(&srv, &out);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(out == NULL);

  /* 2. Empty variable */
  srv.url = (char *)(size_t)(size_t) "http://api.com/{}";
  rc = render_server_url_default(&srv, &out);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(out == NULL);

  /* 3. Missing variable */
  srv.url = (char *)(size_t)(size_t) "http://api.com/{missing}";
  rc = render_server_url_default(&srv, &out);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(out == NULL);

  /* 4. Escaping special chars: \r, \t, \n, \", \\ */
  rc = escape_c_string_literal("a\rb\tc\nd\"e\\f", &escaped);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(escaped != NULL);
  ASSERT(strstr(escaped, "\\r") != NULL);
  ASSERT(strstr(escaped, "\\t") != NULL);
  ASSERT(strstr(escaped, "\\n") != NULL);
  ASSERT(strstr(escaped, "\\\"") != NULL);
  ASSERT(strstr(escaped, "\\\\") != NULL);
  free(escaped);

  /* 5. select_operation_server fallback to path->servers */
  {
    struct OpenAPI_Path p;
    struct OpenAPI_Server p_srv;
    struct OpenAPI_Server *res_srv = NULL;
    memset(&p, 0, sizeof(p));
    memset(&p_srv, 0, sizeof(p_srv));
    p_srv.url = (char *)(size_t)(size_t) "http://path-server.com";
    p.servers = &p_srv;
    p.n_servers = 1;
    rc = select_operation_server(&p, NULL, &res_srv);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT(res_srv == &p_srv);

    /* Both NULL */
    rc = select_operation_server(NULL, NULL, &res_srv);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT(res_srv == NULL);
  }

  PASS();
}

TEST test_client_gen_create_tests_mocks_expanded(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  struct OpenAPI_Response resps[2];
  struct OpenAPI_Path path;
  struct OpenApiClientConfig config;
  struct OpenAPI_Parameter params[2];
  struct OpenAPI_Server servers[2];
  struct OpenAPI_MediaType req_media;
  int rc;

  memset(&op, 0, sizeof(op));
  memset(&spec, 0, sizeof(spec));
  memset(resps, 0, sizeof(resps));
  memset(&path, 0, sizeof(path));
  memset(&config, 0, sizeof(config));
  memset(params, 0, sizeof(params));
  memset(servers, 0, sizeof(servers));
  memset(&req_media, 0, sizeof(req_media));

  path.route = (char *)(size_t)(size_t) "/items/{id}";
  params[0].name = (char *)(size_t)(size_t) "p1";
  params[0].in = OA_PARAM_IN_QUERY;
  params[1].name = (char *)(size_t)(size_t) "p2";
  params[1].in = OA_PARAM_IN_QUERYSTRING;

  op.operation_id = (char *)(size_t)(size_t) "doExpanded";
  op.verb = OA_VERB_POST;
  op.parameters = params;
  op.n_parameters = 2;

  /* First response 200 with schema ref, second default */
  resps[0].code = (char *)(size_t)(size_t) "200";
  resps[0].schema.ref_name = (char *)(size_t)(size_t) "ItemResponse";
  resps[0].schema.is_array = 0;
  resps[1].code = (char *)(size_t)(size_t) "default";
  op.responses = resps;
  op.n_responses = 2;

  /* Custom req body media type */
  req_media.name = (char *)(size_t)(size_t) "application/xml";
  op.req_body_media_types = &req_media;
  op.n_req_body_media_types = 1;

  path.operations = &op;
  path.n_operations = 1;
  spec.paths = &path;
  spec.n_paths = 1;

  /* Server 1: http://api.com/v1 (has :// with path) */
  servers[0].url = (char *)(size_t)(size_t) "http://api.com/v1";
  spec.servers = servers;
  spec.n_servers = 1;

  config.filename_base =
      (char *)(size_t)(size_t) "build/test_out/test_client_sdk_exp1";
  config.create_tests_and_mocks = 1;
  config.no_installable_package = 0; /* test generate_cmake_project */

  rc = openapi_client_generate(&spec, &config);
  ASSERT_EQ(0, rc);

  remove("build/test_out/src/test/test_sdk.c");
  remove("build/test_out/src/test_client_sdk_exp1_models.h");
  remove("build/test_out/src/test_client_sdk_exp1_models.c");
  remove("build/test_out/src/test_client_sdk_exp1.h");
  remove("build/test_out/src/test_client_sdk_exp1.c");
  remove("build/test_out/CMakeLists.txt");

  /* Server 2: http://api.com (no path after hostname) */
  servers[0].url = (char *)(size_t)(size_t) "http://api.com";
  config.filename_base =
      (char *)(size_t)(size_t) "build/test_out/test_client_sdk_exp2";
  config.no_installable_package = 1;
  rc = openapi_client_generate(&spec, &config);
  ASSERT_EQ(0, rc);

  remove("build/test_out/src/test/test_sdk.c");
  remove("build/test_out/src/test_client_sdk_exp2_models.h");
  remove("build/test_out/src/test_client_sdk_exp2_models.c");
  remove("build/test_out/src/test_client_sdk_exp2.h");
  remove("build/test_out/src/test_client_sdk_exp2.c");

  /* Server 3: /v1 (no ://) */
  servers[0].url = (char *)(size_t)(size_t) "/v1";
  config.filename_base =
      (char *)(size_t)(size_t) "build/test_out/test_client_sdk_exp3";
  rc = openapi_client_generate(&spec, &config);
  ASSERT_EQ(0, rc);

  remove("build/test_out/src/test/test_sdk.c");
  remove("build/test_out/src/test_client_sdk_exp3_models.h");
  remove("build/test_out/src/test_client_sdk_exp3_models.c");
  remove("build/test_out/src/test_client_sdk_exp3.h");
  remove("build/test_out/src/test_client_sdk_exp3.c");

  /* Swagger 2.0 basePath: "/" */
  spec.servers = NULL;
  spec.n_servers = 0;
  spec.basePath = (char *)(size_t)(size_t) "/";
  config.filename_base =
      (char *)(size_t)(size_t) "build/test_out/test_client_sdk_exp4";
  rc = openapi_client_generate(&spec, &config);
  ASSERT_EQ(0, rc);

  remove("build/test_out/src/test/test_sdk.c");
  remove("build/test_out/src/test_client_sdk_exp4_models.h");
  remove("build/test_out/src/test_client_sdk_exp4_models.c");
  remove("build/test_out/src/test_client_sdk_exp4.h");
  remove("build/test_out/src/test_client_sdk_exp4.c");

  g_fail_io_after = -1;
  PASS();
}

TEST test_client_gen_io_failures(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op;
  struct OpenApiClientConfig config;
  struct StructFields sf[1];
  char *snames[1];
  int i;
  int rc;

  memset(&op, 0, sizeof(op));
  ASSERT_EQ(CDD_C_SUCCESS, setup_minimal_spec(&spec, &op));
  op.description = (char *)(size_t)(size_t) "Operation description";
  op.summary = (char *)(size_t)(size_t) "Operation summary";

  memset(sf, 0, sizeof(sf));
  snames[0] = (char *)(size_t)(size_t) "SampleSchema";
  spec.defined_schemas = sf;
  spec.defined_schema_names = snames;
  spec.n_defined_schemas = 1;

  memset(&config, 0, sizeof(config));
  config.filename_base = (char *)(size_t)(size_t) "build/test_out/test_cg_io";
  config.no_installable_package = 1;

  for (i = 0; i <= 350; ++i) {
    g_io_calls = 0;
    g_fail_io_after = i;
    rc = openapi_client_generate(&spec, &config);
    (void)rc;
  }

  config.no_installable_package = 0;
  for (i = 0; i <= 60; ++i) {
    g_io_calls = 0;
    g_fail_io_after = i;
    rc = openapi_client_generate(&spec, &config);
    (void)rc;
  }

  g_fail_io_after = -1;
  remove("build/test_out/src/test_cg_io.h");
  remove("build/test_out/src/test_cg_io.c");
  remove("build/test_out/src/test_cg_io_models.h");
  remove("build/test_out/src/test_cg_io_models.c");
  remove("build/test_out/src/url_utils.h");
  remove("build/test_out/src/url_utils.c");
  remove("build/test_out/CMakeLists.txt");
  PASS();
}

SUITE(openapi_client_gen_suite) {
  RUN_TEST(test_client_gen_mock_errors);
  RUN_TEST(test_client_gen_extra_cases);
  RUN_TEST(test_client_gen_create_tests_mocks_expanded);
  RUN_TEST(test_client_gen_io_failures);
  RUN_TEST(test_client_gen_defined_schemas);
  RUN_TEST(test_client_gen_create_tests_mocks);
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
