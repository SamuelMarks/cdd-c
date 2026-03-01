/**
 * @file test_refactor_api_sync.h
 * @brief Integration tests for API Synchronization.
 *
 * Verifies updates to signature, URL logic, and query/header blocks.
 *
 * @author Samuel Marks
 */

#ifndef TEST_REFACTOR_API_SYNC_H
#define TEST_REFACTOR_API_SYNC_H

#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "functions/parse/fs.h"
#include "openapi/parse/openapi.h"
#include "routes/parse/sync.h"

static int load_spec(const char *json, struct OpenAPI_Spec *spec) {
  JSON_Value *dyn = json_parse_string(json);
  int rc;
  if (!dyn)
    return -1;
  openapi_spec_init(spec);
  rc = openapi_load_from_json(dyn, spec);
  json_value_free(dyn);
  return rc;
}

TEST test_sync_signature_update(void) {
  SKIPm("fix me");
  const char *src_file = "sync_sig.c";
  const char *old_code = "#include \"client.h\"\n"
                         "int get_pet(struct HttpClient *ctx) {\n"
                         "  return 0;\n"
                         "}\n";
  const char *spec_json =
      "{\"paths\":{\"/pets/{id}\":{\"get\":{\"operationId\":\"get_pet\","
      "\"parameters\":[{\"name\":\"id\",\"in\":\"path\",\"required\":true,"
      "\"schema\":{\"type\":\"integer\"}}]"
      "}}}}";
  struct OpenAPI_Spec spec;
  char *content;
  size_t sz;
  int rc;

  write_to_file(src_file, old_code);
  load_spec(spec_json, &spec);
  rc = api_sync_file(src_file, &spec, NULL);
  ASSERT_EQ(0, rc);
  read_to_file(src_file, "r", &content, &sz);
  ASSERT(strstr(content, "int get_pet(struct HttpClient *ctx, int id, struct "
                         "Pet **out, struct ApiError **api_error)"));
  free(content);
  openapi_spec_free(&spec);
  remove(src_file);
  PASS();
}

TEST test_sync_url_logic_update(void) {
  SKIPm("fix me");
  const char *src_file = "sync_url.c";
  const char *old_code =
      "int get_pet(struct HttpClient *ctx, int id) {\n"
      "  char *url;\n"
      "  asprintf(&url, \"%s/pets/oldpath\", ctx->base_url);\n"
      "  return 0;\n"
      "}\n";
  const char *spec_json =
      "{\"paths\":{\"/pets/{id}\":{\"get\":{\"operationId\":\"get_pet\","
      "\"parameters\":[{\"name\":\"id\",\"in\":\"path\",\"required\":true,"
      "\"schema\":{\"type\":\"integer\"}}]"
      "}}}}";
  struct OpenAPI_Spec spec;
  char *content;
  size_t sz;
  int rc;

  write_to_file(src_file, old_code);
  load_spec(spec_json, &spec);
  rc = api_sync_file(src_file, &spec, NULL);
  ASSERT_EQ(0, rc);
  read_to_file(src_file, "r", &content, &sz);
  ASSERT(strstr(content, "asprintf(&url, \"%s/pets/%d\", ctx->base_url, id)") !=
         NULL);
  ASSERT(strstr(content, "oldpath") == NULL);
  free(content);
  openapi_spec_free(&spec);
  remove(src_file);
  PASS();
}

TEST test_sync_query_update(void) {
  SKIPm("fix me");
  const char *src_file = "sync_query.c";
  const char *old_code = "int list_pets(struct HttpClient *ctx) {\n"
                         "  /* Old logic */\n"
                         "  rc = url_query_init(&qp);\n"
                         "  url_query_add(&qp, \"old\", \"val\");\n"
                         "  rc = url_query_build(&qp, &query_str);\n"
                         "  return 0;\n"
                         "}\n";
  const char *spec_json =
      "{\"paths\":{\"/pets\":{\"get\":{\"operationId\":\"list_pets\","
      "\"parameters\":[{\"name\":\"tags\",\"in\":\"query\",\"schema\":{"
      "\"type\":\"array\",\"items\":{\"type\":\"string\"}},\"explode\":true}]"
      "}}}}";

  struct OpenAPI_Spec spec;
  char *content;
  size_t sz;
  int rc;

  write_to_file(src_file, old_code);
  load_spec(spec_json, &spec);
  rc = api_sync_file(src_file, &spec, NULL);
  ASSERT_EQ(0, rc);
  read_to_file(src_file, "r", &content, &sz);

  /* Should replace old block with loop logic */
  ASSERT(strstr(content, "for(i=0; i < tags_len; ++i)") != NULL);
  ASSERT(strstr(content, "url_query_add(&qp, \"old\", \"val\")") == NULL);

  free(content);
  openapi_spec_free(&spec);
  remove(src_file);
  PASS();
}

TEST test_sync_header_update(void) {
  SKIPm("fix me");
  const char *src_file = "sync_header.c";
  const char *old_code = "int op(struct HttpClient *ctx, const char *key) {\n"
                         "  /* Header Parameter: key */\n"
                         "  if (key) { old_call(); }\n"
                         "  return 0;\n"
                         "}\n";
  const char *spec_json =
      "{\"paths\":{\"/h\":{\"get\":{\"operationId\":\"op\","
      "\"parameters\":[{\"name\":\"key\",\"in\":\"header\",\"schema\":{"
      "\"type\":\"string\"}}]"
      "}}}}";

  struct OpenAPI_Spec spec;
  char *content;
  size_t sz;
  int rc;

  write_to_file(src_file, old_code);
  load_spec(spec_json, &spec);
  rc = api_sync_file(src_file, &spec, NULL);
  ASSERT_EQ(0, rc);
  read_to_file(src_file, "r", &content, &sz);

  /* Should replace old_call with http_headers_add */
  ASSERT(strstr(content, "http_headers_add(&req.headers, \"key\", key)") !=
         NULL);
  ASSERT(strstr(content, "old_call") == NULL);

  free(content);
  openapi_spec_free(&spec);
  remove(src_file);
  PASS();
}

SUITE(api_sync_suite) {
  RUN_TEST(test_sync_signature_update);
  RUN_TEST(test_sync_url_logic_update);
  RUN_TEST(test_sync_query_update);
  RUN_TEST(test_sync_header_update);
}

#endif /* TEST_REFACTOR_API_SYNC_H */
