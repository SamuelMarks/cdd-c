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
#include "fs.h"
#include "openapi_client_gen.h"
#include "openapi_loader.h"

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
  RUN_TEST(test_gen_client_grouped_tags_namespace);
  RUN_TEST(test_gen_client_namespace_only);
  RUN_TEST(test_gen_client_error_nulls);
  RUN_TEST(test_gen_client_file_error);
  RUN_TEST(test_gen_client_defaults);
  RUN_TEST(test_gen_transport_selection);
}

#endif /* TEST_OPENAPI_CLIENT_GEN_H */
