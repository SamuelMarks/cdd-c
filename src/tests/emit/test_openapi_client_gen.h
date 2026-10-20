/**
 * @file test_openapi_client_gen.h
 * @brief Tests for OpenAPI Client Library Generator (Basic & Options).
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

/** @brief Test definition g_fail_io_after. */
extern C_CDD_EXPORT int g_fail_io_after;
/** @brief Test definition g_io_calls. */
extern C_CDD_EXPORT int g_io_calls;
extern C_CDD_EXPORT int g_client_gen_fail;

/**
 * @brief Helper function setup_minimal_spec.
 * @param[in,out] spec Parameter spec.
 * @param[in,out] op Parameter op.
 * @return Result status or value.
 */
static cdd_c_error_t setup_minimal_spec(struct OpenAPI_Spec *spec,
                                        struct OpenAPI_Operation *op) {
  static struct OpenAPI_Path path;
  static struct OpenAPI_Response resp = {0};
  cdd_c_error_t rc = 0;

  rc = openapi_spec_init(spec);
  if (rc != CDD_C_SUCCESS) {
    return rc;
  }
  memset(op, 0, sizeof(*op));
  op->operation_id = cdd_test_mut_str("test_op");
  op->verb = OA_VERB_GET;

  memset(&resp, 0, sizeof(resp));
  resp.code = cdd_test_mut_str("200");
  op->responses = &resp;
  op->n_responses = 1;

  memset(&path, 0, sizeof(path));
  path.route = cdd_test_mut_str("/test");
  path.operations = op;
  path.n_operations = 1;

  spec->paths = &path;
  spec->n_paths = 1;
  return CDD_C_SUCCESS;
}

/**
 * @brief Unit test test_gen_client_basic.
 * @return Greatest test result.
 */
TEST test_gen_client_basic(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op = {0};
  struct OpenApiClientConfig config;
  const char *base = "build/test_out/gen_client_test";
  char *h_file = cdd_test_mut_str("build/test_out/src/gen_client_test.h");
  char *c_file = cdd_test_mut_str("build/test_out/src/gen_client_test.c");
  char *content = NULL;
  size_t sz;
  int rc = 0;

  /* Exercise setup_minimal_spec error path */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, setup_minimal_spec(NULL, &op));

  ASSERT_EQ(CDD_C_SUCCESS, setup_minimal_spec(&spec, &op));

  memset(&config, 0, sizeof(config));
  config.filename_base = base;
  config.func_prefix = cdd_test_mut_str("api_");
  config.model_header = cdd_test_mut_str("my_models.h");

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

/**
 * @brief Unit test test_gen_client_operation_server_override.
 * @return Greatest test result.
 */
TEST test_gen_client_operation_server_override(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Server op_server;
  struct OpenApiClientConfig config;
  const char *base = "build/test_out/gen_client_op_server";
  char *h_file = cdd_test_mut_str("build/test_out/src/gen_client_op_server.h");
  char *c_file = cdd_test_mut_str("build/test_out/src/gen_client_op_server.c");
  char *content = NULL;
  size_t sz;
  int rc = 0;

  ASSERT_EQ(CDD_C_SUCCESS, setup_minimal_spec(&spec, &op));

  memset(&op_server, 0, sizeof(op_server));
  op_server.url = cdd_test_mut_str("https://op.example.com/api");
  op.servers = &op_server;
  op.n_servers = 1;

  memset(&config, 0, sizeof(config));
  config.filename_base = base;
  config.func_prefix = cdd_test_mut_str("api_");

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

/**
 * @brief Unit test test_gen_client_text_plain_request_body.
 * @return Greatest test result.
 */
TEST test_gen_client_text_plain_request_body(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op = {0};
  struct OpenApiClientConfig config;
  const char *base = "build/test_out/gen_client_text_plain_req";
  char *h_file =
      cdd_test_mut_str("build/test_out/src/gen_client_text_plain_req.h");
  char *c_file =
      cdd_test_mut_str("build/test_out/src/gen_client_text_plain_req.c");
  char *content = NULL;
  size_t sz;
  int rc = 0;

  ASSERT_EQ(CDD_C_SUCCESS, setup_minimal_spec(&spec, &op));
  op.verb = OA_VERB_POST;
  op.req_body.content_type = cdd_test_mut_str("text/plain");
  op.req_body.inline_type = cdd_test_mut_str("string");

  memset(&config, 0, sizeof(config));
  config.filename_base = base;
  config.func_prefix = cdd_test_mut_str("api_");

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

/**
 * @brief Unit test test_gen_client_octet_stream_request_body.
 * @return Greatest test result.
 */
TEST test_gen_client_octet_stream_request_body(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op = {0};
  struct OpenApiClientConfig config;
  const char *base = "build/test_out/gen_client_octet_req";
  char *h_file = cdd_test_mut_str("build/test_out/src/gen_client_octet_req.h");
  char *c_file = cdd_test_mut_str("build/test_out/src/gen_client_octet_req.c");
  char *content = NULL;
  size_t sz;
  int rc = 0;

  ASSERT_EQ(CDD_C_SUCCESS, setup_minimal_spec(&spec, &op));
  op.verb = OA_VERB_POST;
  op.req_body.content_type = cdd_test_mut_str("application/octet-stream");

  memset(&config, 0, sizeof(config));
  config.filename_base = base;
  config.func_prefix = cdd_test_mut_str("api_");

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

/**
 * @brief Unit test test_gen_client_octet_stream_response_body.
 * @return Greatest test result.
 */
TEST test_gen_client_octet_stream_response_body(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op = {0};
  struct OpenApiClientConfig config;
  const char *base = "build/test_out/gen_client_octet_resp";
  char *h_file = cdd_test_mut_str("build/test_out/src/gen_client_octet_resp.h");
  char *c_file = cdd_test_mut_str("build/test_out/src/gen_client_octet_resp.c");
  char *content = NULL;
  size_t sz;
  int rc = 0;

  ASSERT_EQ(CDD_C_SUCCESS, setup_minimal_spec(&spec, &op));
  op.responses[0].content_type = cdd_test_mut_str("application/octet-stream");

  memset(&config, 0, sizeof(config));
  config.filename_base = base;
  config.func_prefix = cdd_test_mut_str("api_");

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

/**
 * @brief Unit test test_gen_client_default_base_url_from_server.
 * @return Greatest test result.
 */
TEST test_gen_client_default_base_url_from_server(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op = {0};
  struct OpenApiClientConfig config;
  struct OpenAPI_Server server;
  struct OpenAPI_ServerVariable var;
  const char *base = "build/test_out/gen_client_default_url";
  char *h_file =
      cdd_test_mut_str("build/test_out/src/gen_client_default_url.h");
  char *c_file =
      cdd_test_mut_str("build/test_out/src/gen_client_default_url.c");
  char *content = NULL;
  size_t sz;
  int rc = 0;

  ASSERT_EQ(CDD_C_SUCCESS, setup_minimal_spec(&spec, &op));

  memset(&server, 0, sizeof(server));
  memset(&var, 0, sizeof(var));
  server.url = cdd_test_mut_str("https://{env}.example.com/v1");
  var.name = cdd_test_mut_str("env");
  var.default_value = cdd_test_mut_str("api");
  server.variables = &var;
  server.n_variables = 1;
  spec.servers = &server;
  spec.n_servers = 1;

  memset(&config, 0, sizeof(config));
  config.filename_base = base;
  config.func_prefix = cdd_test_mut_str("api_");

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

/**
 * @brief Unit test test_gen_client_default_base_url_no_servers.
 * @return Greatest test result.
 */
TEST test_gen_client_default_base_url_no_servers(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op = {0};
  struct OpenApiClientConfig config;
  const char *base = "build/test_out/gen_client_default_url_none";
  char *h_file = cdd_test_mut_str("build/test_out/src/"
                                  "gen_client_default_url_none.h");
  char *c_file = cdd_test_mut_str("build/test_out/src/"
                                  "gen_client_default_url_none.c");
  char *content = NULL;
  size_t sz;
  int rc = 0;

  ASSERT_EQ(CDD_C_SUCCESS, setup_minimal_spec(&spec, &op));

  memset(&config, 0, sizeof(config));
  config.filename_base = base;
  config.func_prefix = cdd_test_mut_str("api_");

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

/**
 * @brief Unit test test_gen_client_additional_operation.
 * @return Greatest test result.
 */
TEST test_gen_client_additional_operation(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op = {0};
  struct OpenAPI_Response resp = {0};
  struct OpenApiClientConfig config;
  struct OpenAPI_Path path;
  const char *base = "build/test_out/gen_additional_op";
  char *h_file = cdd_test_mut_str("build/test_out/src/gen_additional_op.h");
  char *c_file = cdd_test_mut_str("build/test_out/src/gen_additional_op.c");
  char *content = NULL;
  size_t sz;
  int rc = 0;

  ASSERT_EQ(CDD_C_SUCCESS, openapi_spec_init(&spec));
  memset(&op, 0, sizeof(op));
  memset(&resp, 0, sizeof(resp));
  memset(&path, 0, sizeof(path));

  op.operation_id = cdd_test_mut_str("custom_connect");
  op.verb = OA_VERB_UNKNOWN;
  op.is_additional = 1;
  op.method = cdd_test_mut_str("CONNECT");
  resp.code = cdd_test_mut_str("200");
  op.responses = &resp;
  op.n_responses = 1;

  path.route = cdd_test_mut_str("/custom");
  path.additional_operations = &op;
  path.n_additional_operations = 1;

  spec.paths = &path;
  spec.n_paths = 1;

  memset(&config, 0, sizeof(config));
  config.filename_base = base;
  config.func_prefix = cdd_test_mut_str("api_");

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

/**
 * @brief Unit test test_gen_client_op_params_only.
 * @return Greatest test result.
 */
TEST test_gen_client_op_params_only(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op = {0};
  struct OpenApiClientConfig config;
  struct OpenAPI_Parameter op_param;
  const char *base = "build/test_out/gen_op_params";
  char *h_file = cdd_test_mut_str("build/test_out/src/gen_op_params.h");
  char *content = NULL;
  size_t sz;
  int rc = 0;

  ASSERT_EQ(CDD_C_SUCCESS, setup_minimal_spec(&spec, &op));

  memset(&op_param, 0, sizeof(op_param));
  op_param.name = cdd_test_mut_str("limit");
  op_param.in = OA_PARAM_IN_QUERY;
  op_param.type = cdd_test_mut_str("integer");
  op.parameters = &op_param;
  op.n_parameters = 1;

  memset(&config, 0, sizeof(config));
  config.filename_base = base;
  config.func_prefix = cdd_test_mut_str("api_");

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

/**
 * @brief Unit test test_gen_client_querystring_param.
 * @return Greatest test result.
 */
TEST test_gen_client_querystring_param(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op = {0};
  struct OpenApiClientConfig config;
  struct OpenAPI_Parameter op_param;
  const char *base = "build/test_out/gen_querystring_param";
  char *h_file = cdd_test_mut_str("build/test_out/src/gen_querystring_param.h");
  char *content = NULL;
  size_t sz;
  int rc = 0;

  ASSERT_EQ(CDD_C_SUCCESS, setup_minimal_spec(&spec, &op));

  memset(&op_param, 0, sizeof(op_param));
  op_param.name = cdd_test_mut_str("qs");
  op_param.in = OA_PARAM_IN_QUERYSTRING;
  op_param.type = cdd_test_mut_str("string");
  op.parameters = &op_param;
  op.n_parameters = 1;

  memset(&config, 0, sizeof(config));
  config.filename_base = base;
  config.func_prefix = cdd_test_mut_str("api_");

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

/**
 * @brief Unit test test_gen_client_path_level_params.
 * @return Greatest test result.
 */
TEST test_gen_client_path_level_params(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op = {0};
  struct OpenApiClientConfig config;
  struct OpenAPI_Parameter path_param;
  const char *base = "build/test_out/gen_path_params";
  char *h_file = cdd_test_mut_str("build/test_out/src/gen_path_params.h");
  char *content = NULL;
  size_t sz;
  int rc = 0;

  ASSERT_EQ(CDD_C_SUCCESS, setup_minimal_spec(&spec, &op));

  memset(&path_param, 0, sizeof(path_param));
  path_param.name = cdd_test_mut_str("x_trace");
  path_param.in = OA_PARAM_IN_HEADER;
  path_param.type = cdd_test_mut_str("string");

  spec.paths[0].parameters = &path_param;
  spec.paths[0].n_parameters = 1;

  memset(&config, 0, sizeof(config));
  config.filename_base = base;
  config.func_prefix = cdd_test_mut_str("api_");

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

/**
 * @brief Unit test test_gen_client_path_param_override.
 * @return Greatest test result.
 */
TEST test_gen_client_path_param_override(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op = {0};
  struct OpenApiClientConfig config;
  struct OpenAPI_Parameter path_param;
  struct OpenAPI_Parameter op_param;
  const char *base = "build/test_out/gen_path_override";
  char *h_file = cdd_test_mut_str("build/test_out/src/gen_path_override.h");
  char *content = NULL;
  size_t sz;
  int rc = 0;

  ASSERT_EQ(CDD_C_SUCCESS, setup_minimal_spec(&spec, &op));

  memset(&path_param, 0, sizeof(path_param));
  path_param.name = cdd_test_mut_str("id");
  path_param.in = OA_PARAM_IN_PATH;
  path_param.type = cdd_test_mut_str("integer");

  memset(&op_param, 0, sizeof(op_param));
  op_param.name = cdd_test_mut_str("id");
  op_param.in = OA_PARAM_IN_PATH;
  op_param.type = cdd_test_mut_str("string");

  spec.paths[0].parameters = &path_param;
  spec.paths[0].n_parameters = 1;
  op.parameters = &op_param;
  op.n_parameters = 1;

  memset(&config, 0, sizeof(config));
  config.filename_base = base;
  config.func_prefix = cdd_test_mut_str("api_");

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

/**
 * @brief Unit test test_gen_client_grouped_tags_namespace.
 * @return Greatest test result.
 */
TEST test_gen_client_grouped_tags_namespace(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op = {0};
  struct OpenApiClientConfig config;
  const char *base = "build/test_out/gen_group_ns_test";
  char *h_file = cdd_test_mut_str("build/test_out/src/gen_group_ns_test.h");
  char *content = NULL;
  size_t sz;
  int rc = 0;
  /* Tag string array setup */
  static const char *tags[1];

  tags[0] = cdd_test_mut_str("pet");
  ASSERT_EQ(CDD_C_SUCCESS, setup_minimal_spec(&spec, &op));

  /* Inject tag manually */
  op.tags = (char **)(size_t)tags;
  op.n_tags = 1;

  memset(&config, 0, sizeof(config));
  config.filename_base = base;
  config.func_prefix = cdd_test_mut_str("api_");
  config.namespace_prefix = cdd_test_mut_str("Foo");

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

/**
 * @brief Unit test test_gen_client_namespace_only.
 * @return Greatest test result.
 */
TEST test_gen_client_namespace_only(void) {
  /* Case: Namespace present, but no tags on operation */
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op = {0};
  struct OpenApiClientConfig config;
  const char *base = "build/test_out/gen_ns_only_test";
  char *h_file = cdd_test_mut_str("build/test_out/src/gen_ns_only_test.h");
  char *content = NULL;
  size_t sz;
  int rc = 0;

  ASSERT_EQ(CDD_C_SUCCESS, setup_minimal_spec(&spec, &op));
  /* No tags */

  memset(&config, 0, sizeof(config));
  config.filename_base = base;
  config.func_prefix = cdd_test_mut_str("api_");
  config.namespace_prefix = cdd_test_mut_str("Bar");

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

/**
 * @brief Unit test test_gen_client_error_nulls.
 * @return Greatest test result.
 */
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

/**
 * @brief Unit test test_gen_client_file_error.
 * @return Greatest test result.
 */
TEST test_gen_client_file_error(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op = {0};
  struct OpenApiClientConfig config = {0};
  int rc = 0;

  ASSERT_EQ(CDD_C_SUCCESS, setup_minimal_spec(&spec, &op));
  config.filename_base = cdd_test_mut_str("/this_dir_does_not_exist/file");
  g_io_calls = 0;
  g_fail_io_after = 1;

  rc = openapi_client_generate(&spec, &config);
  ASSERT(rc == CDD_C_ERROR_IO || rc == CDD_C_ERROR_NOT_FOUND);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Unit test test_gen_client_defaults.
 * @return Greatest test result.
 */
TEST test_gen_client_defaults(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op = {0};
  struct OpenApiClientConfig config = {0};
  char *content = NULL;
  size_t sz;

  ASSERT_EQ(CDD_C_SUCCESS, setup_minimal_spec(&spec, &op));
  config.filename_base = cdd_test_mut_str("build/test_out/gen_def");

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

/**
 * @brief Unit test test_gen_transport_selection.
 * @return Greatest test result.
 */
TEST test_gen_transport_selection(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op = {0};
  struct OpenApiClientConfig config = {0};
  char *content = NULL;
  size_t sz;

  ASSERT_EQ(CDD_C_SUCCESS, setup_minimal_spec(&spec, &op));
  config.filename_base = cdd_test_mut_str("build/test_out/gen_transport");

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

/**
 * @brief Unit test test_mcp_raw_schemas_io_fail.
 * @return Greatest test result.
 */
TEST test_mcp_raw_schemas_io_fail(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op = {0};
  struct OpenApiClientConfig config;
  int i;

  memset(&config, 0, sizeof(config));
  config.filename_base = "build/test_out/mcp_raw_io";
  config.func_prefix = "api_";

  ASSERT_EQ(CDD_C_SUCCESS, setup_minimal_spec(&spec, &op));

  spec.n_raw_schemas = 1;
  spec.raw_schema_names = (char **)malloc(1 * sizeof(char *));
  spec.raw_schema_names[0] = cdd_test_mut_str("TestStruct");
  spec.raw_schema_json = (char **)malloc(1 * sizeof(char *));
  spec.raw_schema_json[0] = cdd_test_mut_str("{ \"type\": \"object\" }");

  spec.n_defined_schemas = 1;
  spec.defined_schema_names = (char **)malloc(1 * sizeof(char *));
  spec.defined_schema_names[0] = cdd_test_mut_str("TestStruct");

  for (i = 0; i < 30; ++i) {
    g_io_calls = 0;
    g_fail_io_after = i;
    openapi_client_generate(&spec, &config);
  }
  g_fail_io_after = -1;

  free(spec.raw_schema_names);
  free(spec.raw_schema_json);
  free(spec.defined_schema_names);
  PASS();
}

/**
 * @brief Unit test test_mcp_raw_schemas_escape_fail.
 * @return Greatest test result.
 */
TEST test_mcp_raw_schemas_escape_fail(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op = {0};
  struct OpenApiClientConfig config;
  int rc;

  memset(&config, 0, sizeof(config));
  config.filename_base = "build/test_out/mcp_raw_escape";
  config.func_prefix = "api_";

  ASSERT_EQ(CDD_C_SUCCESS, setup_minimal_spec(&spec, &op));

  spec.n_raw_schemas = 1;
  spec.raw_schema_names = (char **)malloc(1 * sizeof(char *));
  spec.raw_schema_names[0] = cdd_test_mut_str("TestStruct");
  spec.raw_schema_json = (char **)malloc(1 * sizeof(char *));
  spec.raw_schema_json[0] = cdd_test_mut_str("{ \"type\": \"object\" }");

  spec.n_defined_schemas = 1;
  spec.defined_schema_names = (char **)malloc(1 * sizeof(char *));
  spec.defined_schema_names[0] = cdd_test_mut_str("TestStruct");
  config.no_installable_package = 1;

  g_cdd_alloc_fail = 16; /* mock escape_c_string_literal inner failure */
  rc = openapi_client_generate(&spec, &config);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  g_cdd_alloc_fail = 0;

  free(spec.raw_schema_names);
  free(spec.raw_schema_json);
  free(spec.defined_schema_names);
  PASS();
}

/**
 * @brief Unit test test_mcp_raw_schemas.
 * @return Greatest test result.
 */
TEST test_mcp_raw_schemas(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Operation op = {0};
  struct OpenApiClientConfig config;
  cdd_c_error_t rc;

  memset(&config, 0, sizeof(config));

  config.filename_base = "build/test_out/mcp_raw";
  config.func_prefix = "api_";

  ASSERT_EQ(CDD_C_SUCCESS, setup_minimal_spec(&spec, &op));

  spec.n_raw_schemas = 2;
  spec.raw_schema_names = (char **)malloc(2 * sizeof(char *));
  spec.raw_schema_names[0] = cdd_test_mut_str("OtherStruct");
  spec.raw_schema_names[1] = cdd_test_mut_str("TestStruct");
  spec.raw_schema_json = (char **)malloc(2 * sizeof(char *));
  spec.raw_schema_json[0] = cdd_test_mut_str("{ \"type\": \"string\" }");
  spec.raw_schema_json[1] = cdd_test_mut_str("{ \"type\": \"object\" }");

  spec.n_defined_schemas = 1;
  spec.defined_schema_names = (char **)malloc(sizeof(char *));
  spec.defined_schema_names[0] = cdd_test_mut_str("TestStruct");

  rc = openapi_client_generate(&spec, &config);
  if (rc != CDD_C_SUCCESS) {
    printf("RC WAS: %d\n", rc);
  }
  printf("raw_schema_names[0] = %s\n", spec.raw_schema_names[0]);
  printf("g_io_calls = %d\n", g_io_calls);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  free(spec.raw_schema_names);
  free(spec.raw_schema_json);
  free(spec.defined_schema_names);

  PASS();
}

/**
 * @brief Executes the openapi_client_gen_suite test suite.
 */
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
  RUN_TEST(test_mcp_raw_schemas);
  RUN_TEST(test_mcp_raw_schemas_io_fail);
  RUN_TEST(test_mcp_raw_schemas_escape_fail);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_OPENAPI_CLIENT_GEN_H */
