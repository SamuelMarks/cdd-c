/**
 * @file test_refactor_api_sync.h
 * @brief Unit tests for refactoring API sync logic.
 */

#ifndef TEST_REFACTOR_API_SYNC_H
#define TEST_REFACTOR_API_SYNC_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include "cdd_c_error.h"
#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "functions/parse/fs.h"
#include "openapi/parse/openapi.h"
#include "routes/parse/sync.h"
/* clang-format on */

#ifndef __EMSCRIPTEN__

/* LCOV_EXCL_START */
static enum cdd_c_error load_spec(const char *json, struct OpenAPI_Spec *spec) {
  JSON_Value *dyn = json_parse_string(json);
  /* LCOV_EXCL_STOP */
  int rc;
  /* LCOV_EXCL_START */
  if (!dyn)
    return -1;
  (void)openapi_spec_init(spec);
  rc = openapi_load_from_json(dyn, spec);
  json_value_free(dyn);
  return rc;
  /* LCOV_EXCL_STOP */
}

/* LCOV_EXCL_START */
TEST test_sync_signature_update(void) {
  /* LCOV_EXCL_STOP */
  /* SKIPm("fix me"); */
  /* LCOV_EXCL_START */
  const char *src_file = "sync_sig.c";
  const char *old_code = "#include \"client.h\"\n"
                         /* LCOV_EXCL_STOP */
                         ""
                         "int get_pet(struct HttpClient *ctx) {\n"
                         "  return 0;\n"
                         "}\n";
  /* LCOV_EXCL_START */
  const char *spec_json =
      /* LCOV_EXCL_STOP */
      "{\"openapi\":\"3.1.0\",\"info\":{\"title\":\"test\",\"version\":\"1\"},"
      "\"paths\":{\"/pets/{id}\":{\"get\":{\"operationId\":\"get_pet\","
      "\"parameters\":[{\"name\":\"id\",\"in\":\"path\",\"required\":true,"
      "\"schema\":{\"type\":\"integer\"}}]"
      "}}}}";
  struct OpenAPI_Spec spec;
  /* LCOV_EXCL_START */
  char *content = NULL;
  /* LCOV_EXCL_STOP */
  size_t sz;
  int rc;

  /* LCOV_EXCL_START */
  write_to_file(src_file, old_code);
  ASSERT_EQ(0, load_spec(spec_json, &spec));
  rc = api_sync_file(src_file, &spec, NULL);
  ASSERT_EQ(0, rc);
  read_to_file(src_file, "r", &content, &sz);
  ASSERT(strstr(content, "int get_pet(struct HttpClient *ctx, int id, struct "
                         /* LCOV_EXCL_STOP */
                         "ApiError **api_error)"));
  /* LCOV_EXCL_START */
  free(content);
  openapi_spec_free(&spec);
  remove(src_file);
  g_fail_io_after = -1;
  PASS();
  /* LCOV_EXCL_STOP */
}

/* LCOV_EXCL_START */
TEST test_sync_url_logic_update(void) {
  /* LCOV_EXCL_STOP */
  /* SKIPm("fix me"); */
  /* LCOV_EXCL_START */
  const char *src_file = "sync_url.c";
  const char *old_code =
      /* LCOV_EXCL_STOP */
      ""
      "int get_pet(struct HttpClient *ctx, int id) {\n"
      "  char *url;\n"
      "  asprintf(&url, \"%s/pets/oldpath\", ctx->base_url);\n"
      "  return 0;\n"
      "}\n";
  /* LCOV_EXCL_START */
  const char *spec_json =
      /* LCOV_EXCL_STOP */
      "{\"openapi\":\"3.1.0\",\"info\":{\"title\":\"test\",\"version\":\"1\"},"
      "\"paths\":{\"/pets/{id}\":{\"get\":{\"operationId\":\"get_pet\","
      "\"parameters\":[{\"name\":\"id\",\"in\":\"path\",\"required\":true,"
      "\"schema\":{\"type\":\"integer\"}}]"
      "}}}}";
  struct OpenAPI_Spec spec;
  /* LCOV_EXCL_START */
  char *content = NULL;
  /* LCOV_EXCL_STOP */
  size_t sz;
  int rc;

  /* LCOV_EXCL_START */
  write_to_file(src_file, old_code);
  ASSERT_EQ(0, load_spec(spec_json, &spec));
  rc = api_sync_file(src_file, &spec, NULL);
  ASSERT_EQ(0, rc);
  read_to_file(src_file, "r", &content, &sz);
  ASSERT(strstr(content,
                /* LCOV_EXCL_STOP */
                "asprintf(&url, \"%s/pets/%s\", ctx->base_url, path_id)") !=
         NULL);
  /* LCOV_EXCL_START */
  ASSERT(strstr(content, "oldpath") == NULL);
  free(content);
  openapi_spec_free(&spec);
  remove(src_file);
  g_fail_io_after = -1;
  PASS();
  /* LCOV_EXCL_STOP */
}

/* LCOV_EXCL_START */
TEST test_sync_query_update(void) {
  /* LCOV_EXCL_STOP */
  /* SKIPm("fix me"); */
  /* LCOV_EXCL_START */
  const char *src_file = "sync_query.c";
  const char *old_code = ""
                         /* LCOV_EXCL_STOP */
                         "int list_pets(struct HttpClient *ctx) {\n"
                         "  /* Old logic */\n"
                         "  rc = url_query_init(&qp);\n"
                         "  url_query_add(&qp, \"old\", \"val\");\n"
                         "  rc = url_query_build(&qp, &query_str);\n"
                         "  return 0;\n"
                         "}\n";
  /* LCOV_EXCL_START */
  const char *spec_json =
      /* LCOV_EXCL_STOP */
      "{\"openapi\":\"3.1.0\",\"info\":{\"title\":\"test\",\"version\":\"1\"},"
      "\"paths\":{\"/pets\":{\"get\":{\"operationId\":\"list_pets\","
      "\"parameters\":[{\"name\":\"tags\",\"in\":\"query\",\"schema\":{"
      "\"type\":\"array\",\"items\":{\"type\":\"string\"}},\"explode\":true}]"
      "}}}}";

  struct OpenAPI_Spec spec;
  /* LCOV_EXCL_START */
  char *content = NULL;
  /* LCOV_EXCL_STOP */
  size_t sz;
  int rc;

  /* LCOV_EXCL_START */
  write_to_file(src_file, old_code);
  ASSERT_EQ(0, load_spec(spec_json, &spec));
  rc = api_sync_file(src_file, &spec, NULL);
  ASSERT_EQ(0, rc);
  read_to_file(src_file, "r", &content, &sz);
  /* LCOV_EXCL_STOP */

  /* Should replace old block with loop logic */
  /* LCOV_EXCL_START */
  ASSERT(strstr(content, "for(i=0; i < tags_len; ++i)") != NULL);
  ASSERT(strstr(content, "url_query_add(&qp, \"old\", \"val\")") == NULL);
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  free(content);
  openapi_spec_free(&spec);
  remove(src_file);
  g_fail_io_after = -1;
  PASS();
  /* LCOV_EXCL_STOP */
}

/* LCOV_EXCL_START */
TEST test_sync_header_update(void) {
  /* LCOV_EXCL_STOP */
  /* SKIPm("fix me"); */
  /* LCOV_EXCL_START */
  const char *src_file = "sync_header.c";
  const char *old_code = ""
                         /* LCOV_EXCL_STOP */
                         "int op(struct HttpClient *ctx, const char *key) {\n"
                         "  /* Header Parameter: key */\n"
                         "  if (key) { old_call(); }\n"
                         "  return 0;\n"
                         "}\n";
  /* LCOV_EXCL_START */
  const char *spec_json =
      /* LCOV_EXCL_STOP */
      "{\"openapi\":\"3.1.0\",\"info\":{\"title\":\"test\",\"version\":\"1\"},"
      "\"paths\":{\"/h\":{\"get\":{\"operationId\":\"op\","
      "\"parameters\":[{\"name\":\"key\",\"in\":\"header\",\"schema\":{"
      "\"type\":\"string\"}}]"
      "}}}}";

  struct OpenAPI_Spec spec;
  /* LCOV_EXCL_START */
  char *content = NULL;
  /* LCOV_EXCL_STOP */
  size_t sz;
  int rc;

  /* LCOV_EXCL_START */
  write_to_file(src_file, old_code);
  ASSERT_EQ(0, load_spec(spec_json, &spec));
  rc = api_sync_file(src_file, &spec, NULL);
  ASSERT_EQ(0, rc);
  read_to_file(src_file, "r", &content, &sz);
  /* LCOV_EXCL_STOP */

  /* Should replace old_call with http_headers_add */
  /* LCOV_EXCL_START */
  ASSERT(strstr(content, "http_headers_add(&req.headers, \"key\", key)") !=
         /* LCOV_EXCL_STOP */
         NULL);
  /* LCOV_EXCL_START */
  ASSERT(strstr(content, "old_call") == NULL);
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  free(content);
  openapi_spec_free(&spec);
  remove(src_file);
  g_fail_io_after = -1;
  PASS();
  /* LCOV_EXCL_STOP */
}
#endif

/* LCOV_EXCL_START */
SUITE(api_sync_suite) {
/* LCOV_EXCL_STOP */
#ifndef __EMSCRIPTEN__
  /* LCOV_EXCL_START */
  RUN_TEST(test_sync_signature_update);
  RUN_TEST(test_sync_url_logic_update);
  RUN_TEST(test_sync_query_update);
  RUN_TEST(test_sync_header_update);
/* LCOV_EXCL_STOP */
#endif
  /* LCOV_EXCL_START */
  RUN_TEST(test_sync_oom);
}
/* LCOV_EXCL_STOP */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_REFACTOR_API_SYNC_H */
