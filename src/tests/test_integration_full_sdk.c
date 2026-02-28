/**
 * @file test_integration_full_sdk.c
 * @brief End-to-End Integration Test for OpenAPI Client SDK.
 * Updated to support ApiError signature.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <greatest.h>
#include <parson.h>

#include <cdd_test_helpers/cdd_helpers.h>
#include <cdd_test_helpers/mock_server.h>

#include "functions/emit_client_body.h"
#include "functions/emit_client_sig.h"
#include "functions/parse_fs.h"
#include "functions/parse_http_curl.h"
#include "functions/parse_http_types.h"
#include "functions/parse_http_winhttp.h"
#include "functions/parse_str.h"
#include "openapi/parse_openapi.h"
#include "routes/emit_client_gen.h"

/* Platform compatibility for strdup/sprintfs */
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#define strdup _strdup
#define sprintf_s_compat(buf, sz, fmt, ...) sprintf_s(buf, sz, fmt, __VA_ARGS__)
#else
#define sprintf_s_compat snprintf
#endif

/* Mock Pet and ApiError struct (normally generated) */
struct Pet {
  int id;
  char *name;
  char *tag;
};

/* Minimal ApiError for reference impl */
struct ApiError {
  char *type;
  char *title;
  int status;
  char *detail;
  char *instance;
  char *raw_body;
};

static int Pet_from_json(const char *json, struct Pet **out) {
  JSON_Value *root = json_parse_string(json);
  if (!root)
    return 1;
  *out = calloc(1, sizeof(struct Pet));
  json_value_free(root);
  return 0;
}

/* Reference Implementation with new signature */
int api_get_pet_by_id(struct HttpClient *ctx, int petId, struct Pet **out,
                      struct ApiError **api_error) {
  struct HttpRequest req;
  struct HttpResponse *res = NULL;
  int rc = 0;
  char *url = NULL;

  if (api_error)
    *api_error = NULL;

  if (!ctx || !ctx->send)
    return 22;
  rc = http_request_init(&req);
  if (rc != 0)
    return rc;

  {
    char tmp_path[256];
#if defined(_MSC_VER)
    sprintf_s(tmp_path, sizeof(tmp_path), "%s/pets/%d", ctx->base_url, petId);
#else
    sprintf(tmp_path, "%s/pets/%d", ctx->base_url, petId);
#endif
    url = strdup(tmp_path);
  }
  req.url = url;
  req.method = HTTP_GET;

  rc = ctx->send(ctx->transport, &req, &res);
  if (rc != 0)
    goto cleanup;
  if (!res) {
    rc = 5;
    goto cleanup;
  }

  switch (res->status_code) {
  case 200:
    if (res->body && out) {
      rc = Pet_from_json((const char *)res->body, out);
    }
    break;
  default:
    rc = 5;
    break;
  }

cleanup:
  http_request_free(&req);
  if (res) {
    http_response_free(res);
    free(res);
  }
  return rc;
}

/* --- Tests --- */

TEST test_generator_output_correctness(void) {
  const char *spec_fname = "petstore_gen_test.json";
  const char *out_c = "petstore_gen_test.c";
  char *c_content = NULL;
  size_t sz;
  int rc;

  const char *json_spec =
      "{\"openapi\":\"3.1.0\",\"info\":{\"title\":\"t\",\"version\":\"1\"},\"paths\":{\"/pets/"
      "{petId}\":{\"get\":{\"operationId\":\"getPetById\",\"parameters\":[{"
      "\"name\":\"petId\",\"in\":\"path\",\"required\":true,\"schema\":{"
      "\"type\":\"integer\"}}],\"responses\":{\"200\":{\"description\":\"ok\",\"content\":{"
      "\"application/json\":{\"schema\":{\"$ref\":\"#/components/schemas/"
      "Pet\"}}}}}}}},\"components\":{\"schemas\":{\"Pet\":{\"type\":\"object\"}}}}";

  write_to_file(spec_fname, json_spec);

  /* Run Generator */
  {
    struct OpenAPI_Spec spec;
    struct OpenApiClientConfig config;
    JSON_Value *root = json_parse_file(spec_fname);
    ASSERT(root != NULL);
    openapi_spec_init(&spec);
    ASSERT_EQ(0, openapi_load_from_json(root, &spec));

    memset(&config, 0, sizeof(config));
    config.filename_base = "petstore_gen_test";
    config.func_prefix = "api_";
    config.model_header = "generated_models.h";

    rc = openapi_client_generate(&spec, &config);
    ASSERT_EQ(0, rc);

    json_value_free(root);
    openapi_spec_free(&spec);
  }

  /* Verify signature contains ApiError */
  read_to_file(out_c, "r", &c_content, &sz);
  ASSERT(strstr(c_content, "struct ApiError **api_error") != NULL);
  ASSERT(strstr(c_content, "ApiError_from_json") != NULL);
  /* Verify generated ApiError helper function exists */
  ASSERT(strstr(c_content, "static int ApiError_from_json") != NULL);

  free(c_content);
  remove(spec_fname);
  remove("petstore_gen_test.h");
  remove("petstore_gen_test.c");
  PASS();
}

TEST test_runtime_execution_with_mock_server(void) {
  MockServerPtr server;
  struct HttpClient client;
  struct Pet *pet = NULL;
  struct ApiError *err = NULL;
  int port;
  char base_url[64];
  int rc;

  server = mock_server_init();
  if (mock_server_start(server) != 0) {
    mock_server_destroy(server);
    SKIPm("Mock server start failed (sockets unavailable?)");
  }
  port = mock_server_get_port(server);

  http_client_init(&client);
#if defined(_MSC_VER)
  sprintf_s(base_url, sizeof(base_url), "http://127.0.0.1:%d", port);
#else
  sprintf(base_url, "http://127.0.0.1:%d", port);
#endif
  client.base_url = strdup(base_url);

#if defined(_WIN32)
  http_winhttp_global_init();
  http_winhttp_context_init(&client.transport);
  client.send = http_winhttp_send;
#else
  http_curl_global_init();
  http_curl_context_init(&client.transport);
  client.send = http_curl_send;
#endif

  rc = api_get_pet_by_id(&client, 123, &pet, &err);

  ASSERT_EQ(1, rc);
  ASSERT(pet == NULL);
  ASSERT(err == NULL); /* Mock server returns simple text text 200, so error
                          parsing skipped */

  http_client_free(&client);
#if defined(_WIN32)
  http_winhttp_context_free(client.transport);
#else
  http_curl_context_free(client.transport);
  http_curl_global_cleanup();
#endif
  mock_server_destroy(server);
  PASS();
}

GREATEST_MAIN_DEFS();

SUITE(full_sdk_integration_suite) {
  RUN_TEST(test_generator_output_correctness);
  RUN_TEST(test_runtime_execution_with_mock_server);
}

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(full_sdk_integration_suite);
  GREATEST_MAIN_END();
}
