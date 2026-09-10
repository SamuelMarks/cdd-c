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

static cdd_c_error_t load_spec(const char *json, struct OpenAPI_Spec *spec) {
  JSON_Value *dyn = json_parse_string(json);
  int rc;
  if (!dyn)
    return -1;
  (void)openapi_spec_init(spec);
  rc = openapi_load_from_json(dyn, spec);
  json_value_free(dyn);
  return rc;
}

TEST test_sync_signature_update(void) {
  /* SKIPm("fix me"); */
  const char *src_file = (char *)(size_t)(size_t) "sync_sig.c";
  const char *old_code = "#include \"client.h\"\n"
                         ""
                         "int get_pet(struct HttpClient *ctx) {\n"
                         "  return 0;\n"
                         "}\n";
  const char *spec_json =
      "{\"openapi\":\"3.1.0\",\"info\":{\"title\":\"test\",\"version\":\"1\"},"
      "\"paths\":{\"/pets/{id}\":{\"get\":{\"operationId\":\"get_pet\","
      "\"parameters\":[{\"name\":\"id\",\"in\":\"path\",\"required\":true,"
      "\"schema\":{\"type\":\"integer\"}}]"
      "}}}}";
  struct OpenAPI_Spec spec;
  char *content = NULL;
  size_t sz;
  int rc;

  (void)rc;
  write_to_file(src_file, old_code);
  ASSERT_EQ(0, load_spec(spec_json, &spec));
  rc = api_sync_file(src_file, &spec, NULL);
  ASSERT_EQ(0, rc);
  read_to_file(src_file, "r", &content, &sz);
  ASSERT(strstr(content, "int get_pet(struct HttpClient *ctx, int id, struct "
                         "ApiError **api_error)"));
  free(content);
  openapi_spec_free(&spec);
  remove(src_file);
  g_fail_io_after = -1;
  PASS();
}

TEST test_sync_url_logic_update(void) {
  /* SKIPm("fix me"); */
  const char *src_file = (char *)(size_t)(size_t) "sync_url.c";
  const char *old_code =
      ""
      "int get_pet(struct HttpClient *ctx, int id) {\n"
      "  char *url;\n"
      "  asprintf(&url, \"%s/pets/oldpath\", ctx->base_url);\n"
      "  return 0;\n"
      "}\n";
  const char *spec_json =
      "{\"openapi\":\"3.1.0\",\"info\":{\"title\":\"test\",\"version\":\"1\"},"
      "\"paths\":{\"/pets/{id}\":{\"get\":{\"operationId\":\"get_pet\","
      "\"parameters\":[{\"name\":\"id\",\"in\":\"path\",\"required\":true,"
      "\"schema\":{\"type\":\"integer\"}}]"
      "}}}}";
  struct OpenAPI_Spec spec;
  char *content = NULL;
  size_t sz;
  int rc;

  (void)rc;
  write_to_file(src_file, old_code);
  ASSERT_EQ(0, load_spec(spec_json, &spec));
  rc = api_sync_file(src_file, &spec, NULL);
  ASSERT_EQ(0, rc);
  read_to_file(src_file, "r", &content, &sz);
  ASSERT(strstr(content,
                "asprintf(&url, \"%s/pets/%s\", ctx->base_url, path_id)") !=
         NULL);
  ASSERT(strstr(content, "oldpath") == NULL);
  free(content);
  openapi_spec_free(&spec);
  remove(src_file);
  g_fail_io_after = -1;
  PASS();
}

TEST test_sync_query_update(void) {
  /* SKIPm("fix me"); */
  const char *src_file = (char *)(size_t)(size_t) "sync_query.c";
  const char *old_code = ""
                         "int list_pets(struct HttpClient *ctx) {\n"
                         "  /* Old logic */\n"
                         "  rc = url_query_init(&qp);\n"
                         "  url_query_add(&qp, \"old\", \"val\");\n"
                         "  rc = url_query_build(&qp, &query_str);\n"
                         "  return 0;\n"
                         "}\n";
  const char *spec_json =
      "{\"openapi\":\"3.1.0\",\"info\":{\"title\":\"test\",\"version\":\"1\"},"
      "\"paths\":{\"/pets\":{\"get\":{\"operationId\":\"list_pets\","
      "\"parameters\":[{\"name\":\"tags\",\"in\":\"query\",\"schema\":{"
      "\"type\":\"array\",\"items\":{\"type\":\"string\"}},\"explode\":true}]"
      "}}}}";

  struct OpenAPI_Spec spec;
  char *content = NULL;
  size_t sz;
  int rc;

  (void)rc;
  write_to_file(src_file, old_code);
  ASSERT_EQ(0, load_spec(spec_json, &spec));
  rc = api_sync_file(src_file, &spec, NULL);
  ASSERT_EQ(0, rc);
  read_to_file(src_file, "r", &content, &sz);

  /* Should replace old block with loop logic */
  printf("CONTENT:\n%s\n", content);
  ASSERT(strstr(content, "for(i=0; i < tags_len; ++i)") != NULL);
  ASSERT(strstr(content, "url_query_add(&qp, \"old\", \"val\")") == NULL);

  free(content);
  openapi_spec_free(&spec);
  remove(src_file);
  g_fail_io_after = -1;
  PASS();
}

TEST test_sync_header_update(void) {
  /* SKIPm("fix me"); */
  const char *src_file = (char *)(size_t)(size_t) "sync_header.c";
  const char *old_code = ""
                         "int op(struct HttpClient *ctx, const char *key) {\n"
                         "  /* Header Parameter: key */\n"
                         "  if (key) { old_call(); }\n"
                         "  return 0;\n"
                         "}\n";
  const char *spec_json =
      "{\"openapi\":\"3.1.0\",\"info\":{\"title\":\"test\",\"version\":\"1\"},"
      "\"paths\":{\"/h\":{\"get\":{\"operationId\":\"op\","
      "\"parameters\":[{\"name\":\"key\",\"in\":\"header\",\"schema\":{"
      "\"type\":\"string\"}}]"
      "}}}}";

  struct OpenAPI_Spec spec;
  char *content = NULL;
  size_t sz;
  int rc;

  (void)rc;
  write_to_file(src_file, old_code);
  ASSERT_EQ(0, load_spec(spec_json, &spec));
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
  g_fail_io_after = -1;
  PASS();
}
#endif

#ifdef CDD_BUILD_TESTS
extern C_CDD_EXPORT int g_cdd_fail_make_tmpfile;
extern C_CDD_EXPORT int g_cdd_fail_find_function_node;
extern C_CDD_EXPORT int g_cdd_fail_generate_expected_sig;
extern C_CDD_EXPORT int g_cdd_fail_extract_current_sig;
extern C_CDD_EXPORT int g_cdd_fail_apply_query_sync;
extern C_CDD_EXPORT int g_cdd_fail_apply_header_sync;
extern C_CDD_EXPORT int g_cdd_fail_generate_expected_url;
extern C_CDD_EXPORT int g_cdd_fail_codegen_write;
extern C_CDD_EXPORT int g_cdd_fail_patch_list_add;
extern C_CDD_EXPORT int g_cdd_fail_token_matches_string;
extern C_CDD_EXPORT int g_cdd_alloc_fail;
extern C_CDD_EXPORT int g_cdd_fail_cst_list_add;
extern C_CDD_EXPORT int g_cdd_sync_sig_no_brace;
extern C_CDD_EXPORT int g_cdd_strdup_fail;
#endif

/**
 * @brief Comprehensive test covering all functions, branches, and error cases
 * in sync.c.
 */
TEST test_sync_full_coverage(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Path path;
  struct OpenAPI_Operation op;
  struct OpenAPI_Parameter param;
  struct ApiSyncConfig cfg;
  struct PatchList patches;
  struct TokenList *tl = NULL;
  struct CstNodeList cst;
  struct CstNode *node = NULL;
  FILE *tmp = NULL;
  char *str_out = NULL;
  size_t end_idx = 0;
  cdd_c_error_t rc;

  memset(&spec, 0, sizeof(spec));
  memset(&path, 0, sizeof(path));
  memset(&op, 0, sizeof(op));
  memset(&param, 0, sizeof(param));
  memset(&cfg, 0, sizeof(cfg));
  memset(&cst, 0, sizeof(cst));
  memset(&patches, 0, sizeof(patches));

  /* Setup mock parameter and operation */
  param.name = (char *)(size_t) "my_param";
  param.in = OA_PARAM_IN_HEADER;
  param.type = (char *)(size_t) "string";

  op.operation_id = (char *)(size_t) "test_op";
  op.parameters = &param;
  op.n_parameters = 1;

  path.route = (char *)(size_t) "/test/{id}";
  path.operations = &op;
  path.n_operations = 1;

  spec.paths = &path;
  spec.n_paths = 1;

  cfg.func_prefix = "api_";
  cfg.url_var_name = "url_buf";

  /* 1. make_cdd_tmpfile */
  rc = cdd_test_sync_make_cdd_tmpfile(NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  g_cdd_fail_make_tmpfile = 1;
  rc = cdd_test_sync_make_cdd_tmpfile(&tmp);
  ASSERT_EQ(CDD_C_ERROR_IO, rc);
  g_cdd_fail_make_tmpfile = 0;

  g_cdd_fail_make_tmpfile = -1;
  rc = cdd_test_sync_make_cdd_tmpfile(&tmp);
  ASSERT_EQ(CDD_C_ERROR_IO, rc);
  g_cdd_fail_make_tmpfile = 0;

  rc = cdd_test_sync_make_cdd_tmpfile(&tmp);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(tmp != NULL);
  fclose(tmp);
  tmp = NULL;

  /* 2. generate_expected_sig */
  rc = cdd_test_sync_generate_expected_sig(NULL, &cfg, &str_out);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = cdd_test_sync_generate_expected_sig(&op, NULL, &str_out);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = cdd_test_sync_generate_expected_sig(&op, &cfg, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  g_cdd_fail_make_tmpfile = 1;
  rc = cdd_test_sync_generate_expected_sig(&op, &cfg, &str_out);
  ASSERT_EQ(CDD_C_ERROR_IO, rc);
  g_cdd_fail_make_tmpfile = 0;

  g_cdd_fail_codegen_write = 1;
  rc = cdd_test_sync_generate_expected_sig(&op, &cfg, &str_out);
  ASSERT_EQ(CDD_C_ERROR_IO, rc);
  g_cdd_fail_codegen_write = 0;

  g_cdd_alloc_fail = 1;
  rc = cdd_test_sync_generate_expected_sig(&op, &cfg, &str_out);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  g_cdd_alloc_fail = 0;

  rc = cdd_test_sync_generate_expected_sig(&op, &cfg, &str_out);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(str_out != NULL);
  free(str_out);
  str_out = NULL;

  /* 3. generate_expected_query */
  rc = cdd_test_sync_generate_expected_query(NULL, &str_out);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = cdd_test_sync_generate_expected_query(&op, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  g_cdd_fail_make_tmpfile = 1;
  rc = cdd_test_sync_generate_expected_query(&op, &str_out);
  ASSERT_EQ(CDD_C_ERROR_IO, rc);
  g_cdd_fail_make_tmpfile = 0;

  g_cdd_fail_codegen_write = 1;
  rc = cdd_test_sync_generate_expected_query(&op, &str_out);
  ASSERT_EQ(CDD_C_ERROR_IO, rc);
  g_cdd_fail_codegen_write = 0;

  g_cdd_alloc_fail = 1;
  rc = cdd_test_sync_generate_expected_query(&op, &str_out);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  g_cdd_alloc_fail = 0;

  rc = cdd_test_sync_generate_expected_query(&op, &str_out);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  if (str_out) {
    free(str_out);
    str_out = NULL;
  }

  /* 4. generate_expected_header_line */
  rc = cdd_test_sync_generate_expected_header_line(NULL, &str_out);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = cdd_test_sync_generate_expected_header_line(&param, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  g_cdd_alloc_fail = 1;
  rc = cdd_test_sync_generate_expected_header_line(&param, &str_out);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  g_cdd_alloc_fail = 0;

  /* String type */
  param.type = (char *)(size_t) "string";
  rc = cdd_test_sync_generate_expected_header_line(&param, &str_out);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(str_out != NULL);
  free(str_out);
  str_out = NULL;

  /* Integer type */
  param.type = (char *)(size_t) "integer";
  rc = cdd_test_sync_generate_expected_header_line(&param, &str_out);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(str_out != NULL);
  free(str_out);
  str_out = NULL;

  /* Unhandled type */
  param.type = (char *)(size_t) "boolean";
  rc = cdd_test_sync_generate_expected_header_line(&param, &str_out);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(str_out != NULL);
  free(str_out);
  str_out = NULL;

  /* 5. generate_expected_url */
  rc = cdd_test_sync_generate_expected_url(NULL, &op, &cfg, &str_out);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = cdd_test_sync_generate_expected_url("/path", NULL, &cfg, &str_out);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = cdd_test_sync_generate_expected_url("/path", &op, NULL, &str_out);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = cdd_test_sync_generate_expected_url("/path", &op, &cfg, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  g_cdd_fail_make_tmpfile = 1;
  rc = cdd_test_sync_generate_expected_url("/path", &op, &cfg, &str_out);
  ASSERT_EQ(CDD_C_ERROR_IO, rc);
  g_cdd_fail_make_tmpfile = 0;

  g_cdd_fail_codegen_write = 1;
  rc = cdd_test_sync_generate_expected_url("/path", &op, &cfg, &str_out);
  ASSERT_EQ(CDD_C_ERROR_IO, rc);
  g_cdd_fail_codegen_write = 0;

  g_cdd_alloc_fail = 1;
  rc = cdd_test_sync_generate_expected_url("/path", &op, &cfg, &str_out);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  g_cdd_alloc_fail = 0;

  rc = cdd_test_sync_generate_expected_url("/path", &op, &cfg, &str_out);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(str_out != NULL);
  free(str_out);
  str_out = NULL;

  /* 6. find_function_node & extract_current_sig on tokenized code */
  rc = tokenize(az_span_create_from_str(
                    (char *)(size_t) "int api_test_op(int a, int b) {\n"
                                     "  int body = 1;\n"
                                     "  return 0;\n"
                                     "}\n"),
                &tl);
  ASSERT_EQ(0, rc);
  rc = parse_tokens(tl, &cst);
  ASSERT_EQ(0, rc);

  g_cdd_fail_token_matches_string = 1;
  rc = cdd_test_sync_find_function_node(&cst, tl, "api_test_op", &node);
  ASSERT(rc != CDD_C_SUCCESS);
  g_cdd_fail_token_matches_string = 0;

  rc = cdd_test_sync_find_function_node(NULL, tl, "api_test_op", &node);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = cdd_test_sync_find_function_node(&cst, NULL, "api_test_op", &node);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = cdd_test_sync_find_function_node(&cst, tl, NULL, &node);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = cdd_test_sync_find_function_node(&cst, tl, "api_test_op", NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  rc = cdd_test_sync_find_function_node(&cst, tl, "nonexistent", &node);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(node == NULL);

  rc = cdd_test_sync_find_function_node(&cst, tl, "api_test_op", &node);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(node != NULL);

  /* extract_current_sig */
  rc = cdd_test_sync_extract_current_sig(NULL, node, &end_idx, &str_out);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = cdd_test_sync_extract_current_sig(tl, NULL, &end_idx, &str_out);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = cdd_test_sync_extract_current_sig(tl, node, &end_idx, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  g_cdd_alloc_fail = 1;
  rc = cdd_test_sync_extract_current_sig(tl, node, &end_idx, &str_out);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  g_cdd_alloc_fail = 0;

  rc = cdd_test_sync_extract_current_sig(tl, node, &end_idx, &str_out);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(str_out != NULL);
  free(str_out);
  str_out = NULL;

  /* 7. apply_query_sync tests */
  rc = patch_list_init(&patches);
  ASSERT_EQ(0, rc);

  rc = cdd_test_sync_apply_query_sync(NULL, tl, node, &patches);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = cdd_test_sync_apply_query_sync(&op, NULL, node, &patches);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = cdd_test_sync_apply_query_sync(&op, tl, NULL, &patches);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = cdd_test_sync_apply_query_sync(&op, tl, node, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  /* Node without body_start (e.g. forward declaration) */
  {
    struct CstNode fwd_node;
    memset(&fwd_node, 0, sizeof(fwd_node));
    fwd_node.kind = CST_NODE_FUNCTION;
    fwd_node.start_token = 0;
    fwd_node.end_token = 3; /* No LBRACE */
    rc = cdd_test_sync_apply_query_sync(&op, tl, &fwd_node, &patches);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  }

  /* Node without query block */
  rc = cdd_test_sync_apply_query_sync(&op, tl, node, &patches);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* 8. apply_header_sync tests */
  rc = cdd_test_sync_apply_header_sync(NULL, tl, node, &patches);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = cdd_test_sync_apply_header_sync(&op, NULL, node, &patches);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = cdd_test_sync_apply_header_sync(&op, tl, NULL, &patches);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = cdd_test_sync_apply_header_sync(&op, tl, node, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  /* Op with header param but comment not in source */
  rc = cdd_test_sync_apply_header_sync(&op, tl, node, &patches);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  patch_list_free(&patches);
  free_cst_node_list(&cst);
  free_token_list(tl);
  tl = NULL;

  /* 9. Header parameter comment found but entered == 0 (no LBRACE in scope) */
  rc = tokenize(az_span_create_from_str(
                    (char *)(size_t) "int api_test_op(void) {\n"
                                     "  /* Header Parameter: my_param */\n"
                                     "  return 0;\n"
                                     "}\n"),
                &tl);
  ASSERT_EQ(0, rc);
  rc = parse_tokens(tl, &cst);
  ASSERT_EQ(0, rc);
  rc = cdd_test_sync_find_function_node(&cst, tl, "api_test_op", &node);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(node != NULL);

  rc = patch_list_init(&patches);
  ASSERT_EQ(0, rc);
  rc = cdd_test_sync_apply_header_sync(&op, tl, node, &patches);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  patch_list_free(&patches);
  free_cst_node_list(&cst);
  free_token_list(tl);
  tl = NULL;

  /* 10. Node with LBRACE before closing RPAREN (unclosed paren signature) */
  rc = tokenize(az_span_create_from_str(
                    (char *)(size_t) "int bad_func(int a { return 0; }\n"),
                &tl);
  ASSERT_EQ(0, rc);
  {
    struct CstNode bad_node;
    memset(&bad_node, 0, sizeof(bad_node));
    bad_node.kind = CST_NODE_FUNCTION;
    bad_node.start_token = 0;
    bad_node.end_token = tl->size;
    rc = cdd_test_sync_extract_current_sig(tl, &bad_node, &end_idx, &str_out);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT(str_out == NULL);
  }
  free_token_list(tl);
  tl = NULL;

  /* 11. apply_updates tests and failure injections */
  {
    const char *full_code = "int api_test_op(int a) {\n"
                            "  /* Header Parameter: my_param */\n"
                            "  if (1) { old_hdr(); }\n"
                            "  url_query_init(&qp);\n"
                            "  url_query_build(&qp, &query);\n"
                            "  asprintf(&url_buf, \"/old\");\n"
                            "  return 0;\n"
                            "}\n";

    rc = tokenize(az_span_create_from_str((char *)(size_t)full_code), &tl);
    ASSERT_EQ(0, rc);
    rc = parse_tokens(tl, &cst);
    ASSERT_EQ(0, rc);

    /* Test invalid arguments */
    rc = cdd_test_sync_apply_updates(NULL, tl, &cst, &spec, &cfg);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = cdd_test_sync_apply_updates("file.c", NULL, &cst, &spec, &cfg);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = cdd_test_sync_apply_updates("file.c", tl, NULL, &spec, &cfg);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = cdd_test_sync_apply_updates("file.c", tl, &cst, NULL, &cfg);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = cdd_test_sync_apply_updates("file.c", tl, &cst, &spec, NULL);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

    /* patch_list_init OOM */
    g_cdd_alloc_fail = 1;
    rc = cdd_test_sync_apply_updates("file.c", tl, &cst, &spec, &cfg);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    g_cdd_alloc_fail = 0;

    /* Op without operation_id */
    op.operation_id = NULL;
    rc = cdd_test_sync_apply_updates("nonexistent_test_sync_file.c", tl, &cst,
                                     &spec, &cfg);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    op.operation_id = (char *)(size_t) "test_op";

    /* Failure injections in apply_updates */
    g_cdd_fail_find_function_node = 1;
    rc = cdd_test_sync_apply_updates("file.c", tl, &cst, &spec, &cfg);
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
    g_cdd_fail_find_function_node = 0;

    g_cdd_fail_generate_expected_sig = 1;
    rc = cdd_test_sync_apply_updates("file.c", tl, &cst, &spec, &cfg);
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
    g_cdd_fail_generate_expected_sig = 0;

    g_cdd_fail_extract_current_sig = 1;
    rc = cdd_test_sync_apply_updates("file.c", tl, &cst, &spec, &cfg);
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
    g_cdd_fail_extract_current_sig = 0;

    g_cdd_fail_apply_query_sync = 1;
    rc = cdd_test_sync_apply_updates("file.c", tl, &cst, &spec, &cfg);
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
    g_cdd_fail_apply_query_sync = 0;

    g_cdd_fail_apply_header_sync = 1;
    rc = cdd_test_sync_apply_updates("file.c", tl, &cst, &spec, &cfg);
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
    g_cdd_fail_apply_header_sync = 0;

    g_cdd_fail_generate_expected_url = 1;
    rc = cdd_test_sync_apply_updates("file.c", tl, &cst, &spec, &cfg);
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
    g_cdd_fail_generate_expected_url = 0;

    g_cdd_fail_patch_list_add = 1;
    rc = cdd_test_sync_apply_updates("file.c", tl, &cst, &spec, &cfg);
    ASSERT(rc != CDD_C_SUCCESS);
    g_cdd_fail_patch_list_add = 0;

    /* Write to unwritable file */
    rc = cdd_test_sync_apply_updates("/nonexistent_dir_sync/out.c", tl, &cst,
                                     &spec, &cfg);
    ASSERT_EQ(CDD_C_ERROR_IO, rc);

    /* Whitespace before paren in function name */
    {
      struct TokenList *tl_ws = NULL;
      struct CstNodeList cst_ws;
      memset(&cst_ws, 0, sizeof(cst_ws));
      rc = tokenize(
          az_span_create_from_str(
              (char *)(size_t) "int api_test_op  (int a) { return 0; }"),
          &tl_ws);
      ASSERT_EQ(0, rc);
      rc = parse_tokens(tl_ws, &cst_ws);
      ASSERT_EQ(0, rc);
      rc = cdd_test_sync_find_function_node(&cst_ws, tl_ws, "api_test_op",
                                            &node);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      ASSERT(node != NULL);
      free_cst_node_list(&cst_ws);
      free_token_list(tl_ws);
    }

    /* Nested parens in signature */
    {
      struct TokenList *tl_nest = NULL;
      struct CstNodeList cst_nest;
      memset(&cst_nest, 0, sizeof(cst_nest));
      rc = tokenize(
          az_span_create_from_str((
              char *)(size_t) "int api_test_op(int (*cb)(void)) { return 0; }"),
          &tl_nest);
      ASSERT_EQ(0, rc);
      rc = parse_tokens(tl_nest, &cst_nest);
      ASSERT_EQ(0, rc);
      if (cst_nest.size > 0) {
        rc = cdd_test_sync_extract_current_sig(tl_nest, &cst_nest.nodes[0],
                                               &end_idx, &str_out);
        ASSERT_EQ(CDD_C_SUCCESS, rc);
        if (str_out)
          free(str_out);
      }
      free_cst_node_list(&cst_nest);
      free_token_list(tl_nest);
    }

    /* Query init without query build */
    {
      struct TokenList *tl_q = NULL;
      struct CstNodeList cst_q;
      memset(&cst_q, 0, sizeof(cst_q));
      rc = tokenize(az_span_create_from_str(
                        (char *)(size_t) "int api_test_op(int a) { "
                                         "url_query_init(&qp); return 0; }"),
                    &tl_q);
      ASSERT_EQ(0, rc);
      rc = parse_tokens(tl_q, &cst_q);
      ASSERT_EQ(0, rc);
      rc = cdd_test_sync_find_function_node(&cst_q, tl_q, "api_test_op", &node);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      rc = patch_list_init(&patches);
      ASSERT_EQ(0, rc);
      rc = cdd_test_sync_apply_query_sync(&op, tl_q, node, &patches);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      patch_list_free(&patches);
      free_cst_node_list(&cst_q);
      free_token_list(tl_q);
    }

    free_cst_node_list(&cst);
    free_token_list(tl);
    tl = NULL;
  }

  /* 12. api_sync_file validation & file errors */
  rc = api_sync_file(NULL, &spec, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = api_sync_file("test.c", NULL, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  /* Non-existent file */
  rc = api_sync_file("nonexistent_sync_file_12345.c", &spec, NULL);
  ASSERT(rc != CDD_C_SUCCESS);

  /* Temporary file for tokenize/parse errors */
  write_to_file("temp_sync_test.c", "int main() { return 0; }\n");

  /* Tokenize error simulation */
  g_cdd_alloc_fail = 1;
  rc = api_sync_file("temp_sync_test.c", &spec, NULL);
  ASSERT(rc != CDD_C_SUCCESS);
  g_cdd_alloc_fail = 0;

  /* Parse tokens error simulation */
  g_cdd_alloc_fail = 2;
  rc = api_sync_file("temp_sync_test.c", &spec, NULL);
  ASSERT(rc != CDD_C_SUCCESS);
  g_cdd_alloc_fail = 0;

  remove("temp_sync_test.c");
  remove("file.c");
  remove("nonexistent_test_sync_file.c");

  PASS();
}

/**
 * @brief Tests all edge-case branches in sync.c to reach 100% branch coverage.
 */
TEST test_sync_remaining_branches(void) {
  struct OpenAPI_Spec spec;
  struct OpenAPI_Path path;
  struct OpenAPI_Operation op;
  struct OpenAPI_Parameter param;
  struct ApiSyncConfig cfg;
  struct PatchList patches;
  struct TokenList *tl = NULL;
  struct CstNodeList cst;
  struct CstNode *node = NULL;
  char *str_out = NULL;
  cdd_c_error_t rc;

  memset(&spec, 0, sizeof(spec));
  memset(&path, 0, sizeof(path));
  memset(&op, 0, sizeof(op));
  memset(&param, 0, sizeof(param));
  memset(&cfg, 0, sizeof(cfg));
  memset(&cst, 0, sizeof(cst));
  memset(&patches, 0, sizeof(patches));

  param.name = (char *)(size_t) "my_param";
  param.in = OA_PARAM_IN_HEADER;
  param.type = (char *)(size_t) "string";

  op.operation_id = (char *)(size_t) "api_test_op";
  op.parameters = &param;
  op.n_parameters = 1;

  path.route = (char *)(size_t) "/test";
  path.operations = &op;
  path.n_operations = 1;

  spec.paths = &path;
  spec.n_paths = 1;

  /* 1. make_cdd_tmpfile invalid arg */
  rc = cdd_test_sync_make_cdd_tmpfile(NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  /* 2. generate_expected_sig trailing brace false */
  g_cdd_sync_sig_no_brace = 1;
  rc = cdd_test_sync_generate_expected_sig(&op, &cfg, &str_out);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(str_out != NULL);
  free(str_out);
  str_out = NULL;
  g_cdd_sync_sig_no_brace = 0;

  /* 3. generate_expected_header_line param.name == NULL and param.type == NULL
   */
  param.name = NULL;
  rc = cdd_test_sync_generate_expected_header_line(&param, &str_out);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  param.name = (char *)(size_t) "hdr";
  param.type = NULL;
  rc = cdd_test_sync_generate_expected_header_line(&param, &str_out);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(str_out != NULL);
  free(str_out);
  str_out = NULL;

  /* 4. extract_current_sig with out_end_idx == NULL */
  rc = tokenize(az_span_create_from_str(
                    (char *)(size_t) "int api_test_op(int a) { return 0; }\n"),
                &tl);
  ASSERT_EQ(0, rc);
  rc = parse_tokens(tl, &cst);
  ASSERT_EQ(0, rc);
  rc = cdd_test_sync_find_function_node(&cst, tl, "api_test_op", &node);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(node != NULL);

  rc = cdd_test_sync_extract_current_sig(tl, node, NULL, &str_out);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(str_out != NULL);
  free(str_out);
  str_out = NULL;

  free_cst_node_list(&cst);
  free_token_list(tl);
  tl = NULL;

  /* 5. find_function_node with non-function node and non-identifier before
   * paren */
  rc = tokenize(az_span_create_from_str(
                    (char *)(size_t) "struct S { int x; }; int (*fp)(void);\n"),
                &tl);
  ASSERT_EQ(0, rc);
  rc = parse_tokens(tl, &cst);
  ASSERT_EQ(0, rc);
  rc = cdd_test_sync_find_function_node(&cst, tl, "api_test_op", &node);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(node == NULL);

  free_cst_node_list(&cst);
  free_token_list(tl);
  tl = NULL;

  /* 6. apply_query_sync failure paths */
  {
    const char *q_code = "int api_test_op(int a) {\n"
                         "  url_query_init(&qp);\n"
                         "  url_query_build(&qp, &query);\n"
                         "  return 0;\n"
                         "}\n";
    rc = tokenize(az_span_create_from_str((char *)(size_t)q_code), &tl);
    ASSERT_EQ(0, rc);
    rc = parse_tokens(tl, &cst);
    ASSERT_EQ(0, rc);
    rc = cdd_test_sync_find_function_node(&cst, tl, "api_test_op", &node);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    rc = patch_list_init(&patches);
    ASSERT_EQ(0, rc);

    /* token_matches_string error on url_query_init */
    g_cdd_fail_token_matches_string = 1;
    rc = cdd_test_sync_apply_query_sync(&op, tl, node, &patches);
    ASSERT(rc != CDD_C_SUCCESS);
    g_cdd_fail_token_matches_string = 0;

    /* token_matches_string error on url_query_build */
    g_cdd_fail_token_matches_string = 2;
    rc = cdd_test_sync_apply_query_sync(&op, tl, node, &patches);
    ASSERT(rc != CDD_C_SUCCESS);
    g_cdd_fail_token_matches_string = 0;

    /* make_cdd_tmpfile error inside generate_expected_query */
    g_cdd_fail_make_tmpfile = 1;
    rc = cdd_test_sync_apply_query_sync(&op, tl, node, &patches);
    ASSERT(rc != CDD_C_SUCCESS);
    g_cdd_fail_make_tmpfile = 0;

    /* patch_list_add error */
    g_cdd_fail_patch_list_add = 1;
    rc = cdd_test_sync_apply_query_sync(&op, tl, node, &patches);
    ASSERT(rc != CDD_C_SUCCESS);
    g_cdd_fail_patch_list_add = 0;

    patch_list_free(&patches);
    free_cst_node_list(&cst);
    free_token_list(tl);
    tl = NULL;
  }

  /* 7. apply_header_sync failure paths and nested braces */
  {
    struct OpenAPI_Parameter hdr_params[2];
    const char *h_code = "int api_test_op(int a) {\n"
                         "  /* Unrelated comment */\n"
                         "  /* Header Parameter: my_param */\n"
                         "  if (1) { { old_nested(); } }\n"
                         "  return 0;\n"
                         "}\n";
    memset(hdr_params, 0, sizeof(hdr_params));
    hdr_params[0].name = NULL;
    hdr_params[0].in = OA_PARAM_IN_HEADER;
    hdr_params[1].name = (char *)(size_t) "my_param";
    hdr_params[1].in = OA_PARAM_IN_HEADER;
    hdr_params[1].type = (char *)(size_t) "string";
    op.parameters = hdr_params;
    op.n_parameters = 2;

    rc = tokenize(az_span_create_from_str((char *)(size_t)h_code), &tl);
    ASSERT_EQ(0, rc);
    rc = parse_tokens(tl, &cst);
    ASSERT_EQ(0, rc);
    rc = cdd_test_sync_find_function_node(&cst, tl, "api_test_op", &node);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    rc = patch_list_init(&patches);
    ASSERT_EQ(0, rc);

    /* token_matches_string error */
    g_cdd_fail_token_matches_string = 1;
    rc = cdd_test_sync_apply_header_sync(&op, tl, node, &patches);
    ASSERT(rc != CDD_C_SUCCESS);
    g_cdd_fail_token_matches_string = 0;

    /* generate_expected_header_line OOM */
    g_cdd_alloc_fail = 1;
    rc = cdd_test_sync_apply_header_sync(&op, tl, node, &patches);
    ASSERT(rc != CDD_C_SUCCESS);
    g_cdd_alloc_fail = 0;

    /* patch_list_add error */
    g_cdd_fail_patch_list_add = 1;
    rc = cdd_test_sync_apply_header_sync(&op, tl, node, &patches);
    ASSERT(rc != CDD_C_SUCCESS);
    g_cdd_fail_patch_list_add = 0;

    /* Success covering nested braces and non-matching comment and unnamed param
     */
    rc = cdd_test_sync_apply_header_sync(&op, tl, node, &patches);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    param.name = (char *)(size_t) "my_param";
    param.type = (char *)(size_t) "string";
    op.parameters = &param;
    op.n_parameters = 1;

    patch_list_free(&patches);
    free_cst_node_list(&cst);
    free_token_list(tl);
    tl = NULL;
  }

  /* 8. apply_updates branch coverage: snprintf, non-matching var, no-asprintf,
   * forward declaration */
  {
    /* snprintf branch and matching var */
    const char *snp_code = "int api_test_op(int a) {\n"
                           "  snprintf(url, 64, \"/test\");\n"
                           "  return 0;\n"
                           "}\n";
    rc = tokenize(az_span_create_from_str((char *)(size_t)snp_code), &tl);
    ASSERT_EQ(0, rc);
    rc = parse_tokens(tl, &cst);
    ASSERT_EQ(0, rc);

    rc = cdd_test_sync_apply_updates("file.c", tl, &cst, &spec, &cfg);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    /* c_cdd_strdup OOM during apply_updates */
    g_cdd_strdup_fail = 1;
    rc = cdd_test_sync_apply_updates("file.c", tl, &cst, &spec, &cfg);
    ASSERT(rc != CDD_C_SUCCESS);
    g_cdd_strdup_fail = 0;

    /* token_matches_string error on asprintf */
    g_cdd_fail_token_matches_string = 6;
    rc = cdd_test_sync_apply_updates("file.c", tl, &cst, &spec, &cfg);
    ASSERT(rc != CDD_C_SUCCESS);
    g_cdd_fail_token_matches_string = 0;

    /* token_matches_string error on snprintf */
    g_cdd_fail_token_matches_string = 7;
    rc = cdd_test_sync_apply_updates("file.c", tl, &cst, &spec, &cfg);
    ASSERT(rc != CDD_C_SUCCESS);
    g_cdd_fail_token_matches_string = 0;

    /* token_matches_string error on var */
    g_cdd_fail_token_matches_string = 8;
    rc = cdd_test_sync_apply_updates("file.c", tl, &cst, &spec, &cfg);
    ASSERT(rc != CDD_C_SUCCESS);
    g_cdd_fail_token_matches_string = 0;

    /* patch_list_add error on URL new_block */
    g_cdd_fail_patch_list_add = 2;
    rc = cdd_test_sync_apply_updates("file.c", tl, &cst, &spec, &cfg);
    ASSERT(rc != CDD_C_SUCCESS);
    g_cdd_fail_patch_list_add = 0;

    free_cst_node_list(&cst);
    free_token_list(tl);
    tl = NULL;
  }

  /* asprintf with non-matching var */
  {
    const char *non_match_code = "int api_test_op(int a) {\n"
                                 "  asprintf(&other_var, \"/test\");\n"
                                 "  return 0;\n"
                                 "}\n";
    rc = tokenize(az_span_create_from_str((char *)(size_t)non_match_code), &tl);
    ASSERT_EQ(0, rc);
    rc = parse_tokens(tl, &cst);
    ASSERT_EQ(0, rc);

    rc = cdd_test_sync_apply_updates("file.c", tl, &cst, &spec, &cfg);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    free_cst_node_list(&cst);
    free_token_list(tl);
    tl = NULL;
  }

  /* Forward declaration in apply_updates (body_start == 0) */
  {
    const char *fwd_code = "int api_test_op(int a);\n";
    rc = tokenize(az_span_create_from_str((char *)(size_t)fwd_code), &tl);
    ASSERT_EQ(0, rc);
    rc = parse_tokens(tl, &cst);
    ASSERT_EQ(0, rc);

    rc = cdd_test_sync_apply_updates("file.c", tl, &cst, &spec, &cfg);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    free_cst_node_list(&cst);
    free_token_list(tl);
    tl = NULL;
  }

  /* Matching signature in apply_updates (strcmp == 0) */
  {
    struct TokenList *tl_match = NULL;
    struct CstNodeList cst_match;
    const char *match_code =
        "int api_test_op(struct HttpClient *ctx, const char *my_param, "
        "struct ApiError **api_error) {\n  return 0;\n}\n";
    memset(&cst_match, 0, sizeof(cst_match));
    rc = tokenize(az_span_create_from_str((char *)(size_t)match_code),
                  &tl_match);
    ASSERT_EQ(0, rc);
    rc = parse_tokens(tl_match, &cst_match);
    ASSERT_EQ(0, rc);
    rc = cdd_test_sync_apply_updates("file.c", tl_match, &cst_match, &spec,
                                     &cfg);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    free_cst_node_list(&cst_match);
    free_token_list(tl_match);
  }

  /* Node without brace in apply_updates (body_start == 0) */
  {
    struct TokenList *tl_nob = NULL;
    struct CstNodeList cst_nob;
    struct CstNode fake_fn;
    memset(&cst_nob, 0, sizeof(cst_nob));
    rc = tokenize(
        az_span_create_from_str((char *)(size_t) "int api_test_op(int a);"),
        &tl_nob);
    ASSERT_EQ(0, rc);
    memset(&fake_fn, 0, sizeof(fake_fn));
    fake_fn.kind = CST_NODE_FUNCTION;
    fake_fn.start_token = 0;
    fake_fn.end_token = tl_nob->size;
    cst_nob.nodes = &fake_fn;
    cst_nob.size = 1;
    cst_nob.capacity = 1;
    rc = cdd_test_sync_apply_updates("file.c", tl_nob, &cst_nob, &spec, &cfg);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    free_token_list(tl_nob);
  }

  /* Node without semicolon after snprintf in apply_updates */
  {
    struct TokenList *tl_nosemi = NULL;
    struct CstNodeList cst_nosemi;
    struct CstNode fake_fn2;
    memset(&cst_nosemi, 0, sizeof(cst_nosemi));
    rc = tokenize(
        az_span_create_from_str((char *)(size_t) "int api_test_op(int a) { "
                                                 "snprintf(url, 64, \"/\") }"),
        &tl_nosemi);
    ASSERT_EQ(0, rc);
    memset(&fake_fn2, 0, sizeof(fake_fn2));
    fake_fn2.kind = CST_NODE_FUNCTION;
    fake_fn2.start_token = 0;
    fake_fn2.end_token = tl_nosemi->size;
    cst_nosemi.nodes = &fake_fn2;
    cst_nosemi.size = 1;
    cst_nosemi.capacity = 1;
    rc = cdd_test_sync_apply_updates("file.c", tl_nosemi, &cst_nosemi, &spec,
                                     &cfg);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    free_token_list(tl_nosemi);
  }

  /* Node with start_token == end_token in extract_current_sig and
   * find_function_node */
  {
    struct CstNode empty_fn;
    struct TokenList *tl_empty = NULL;
    struct CstNodeList cst_empty;
    memset(&empty_fn, 0, sizeof(empty_fn));
    memset(&cst_empty, 0, sizeof(cst_empty));
    rc = tokenize(az_span_create_from_str((char *)(size_t) "int a = 1;\n"),
                  &tl_empty);
    ASSERT_EQ(0, rc);
    empty_fn.kind = CST_NODE_FUNCTION;
    empty_fn.start_token = 0;
    empty_fn.end_token = tl_empty->size;
    cst_empty.nodes = &empty_fn;
    cst_empty.size = 1;
    cst_empty.capacity = 1;
    rc = cdd_test_sync_find_function_node(&cst_empty, tl_empty, "api_test_op",
                                          &node);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT(node == NULL);
    rc = cdd_test_sync_extract_current_sig(tl_empty, &empty_fn, NULL, &str_out);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT(str_out == NULL);
    free_token_list(tl_empty);
  }

  /* RPAREN before LPAREN */
  {
    struct TokenList *tl_paren = NULL;
    struct CstNode bad_paren_node;
    memset(&bad_paren_node, 0, sizeof(bad_paren_node));
    rc = tokenize(
        az_span_create_from_str((char *)(size_t) ") ( int a ) { return 0; }\n"),
        &tl_paren);
    ASSERT_EQ(0, rc);
    bad_paren_node.kind = CST_NODE_FUNCTION;
    bad_paren_node.start_token = 0;
    bad_paren_node.end_token = tl_paren->size;
    rc = cdd_test_sync_extract_current_sig(tl_paren, &bad_paren_node, NULL,
                                           &str_out);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT(str_out != NULL);
    free(str_out);
    str_out = NULL;
    free_token_list(tl_paren);
  }

  /* Immediate query statements and missing semicolon in query */
  {
    struct TokenList *tl_imm = NULL;
    struct CstNodeList cst_imm;
    const char *imm_code = "int api_test_op(int a) {url_query_init(&qp); "
                           "url_query_build(&qp, &query)}";
    memset(&cst_imm, 0, sizeof(cst_imm));
    rc = tokenize(az_span_create_from_str((char *)(size_t)imm_code), &tl_imm);
    ASSERT_EQ(0, rc);
    rc = parse_tokens(tl_imm, &cst_imm);
    ASSERT_EQ(0, rc);
    rc = cdd_test_sync_find_function_node(&cst_imm, tl_imm, "api_test_op",
                                          &node);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    if (node) {
      rc = patch_list_init(&patches);
      ASSERT_EQ(0, rc);
      rc = cdd_test_sync_apply_query_sync(&op, tl_imm, node, &patches);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      patch_list_free(&patches);
    }
    free_cst_node_list(&cst_imm);
    free_token_list(tl_imm);
  }

  /* Semicolon before query statements in query sync */
  {
    struct TokenList *tl_semi = NULL;
    struct CstNodeList cst_semi;
    const char *semi_code =
        "int api_test_op(int a) { int x = 0; url_query_init(&qp); "
        "url_query_build(&qp, &query); }";
    memset(&cst_semi, 0, sizeof(cst_semi));
    rc = tokenize(az_span_create_from_str((char *)(size_t)semi_code), &tl_semi);
    ASSERT_EQ(0, rc);
    rc = parse_tokens(tl_semi, &cst_semi);
    ASSERT_EQ(0, rc);
    rc = cdd_test_sync_find_function_node(&cst_semi, tl_semi, "api_test_op",
                                          &node);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    if (node) {
      rc = patch_list_init(&patches);
      ASSERT_EQ(0, rc);
      rc = cdd_test_sync_apply_query_sync(&op, tl_semi, node, &patches);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      patch_list_free(&patches);
    }
    free_cst_node_list(&cst_semi);
    free_token_list(tl_semi);
  }

  /* Non-identifier before LPAREN in find_function_node */
  {
    struct TokenList *tl_star = NULL;
    struct CstNode fake_star;
    rc = tokenize(az_span_create_from_str(
                      (char *)(size_t) "int * (void) { return 0; }\n"),
                  &tl_star);
    ASSERT_EQ(0, rc);
    memset(&fake_star, 0, sizeof(fake_star));
    fake_star.kind = CST_NODE_FUNCTION;
    fake_star.start_token = 0;
    fake_star.end_token = tl_star->size;
    {
      struct CstNodeList cst_star;
      cst_star.nodes = &fake_star;
      cst_star.size = 1;
      cst_star.capacity = 1;
      rc = cdd_test_sync_find_function_node(&cst_star, tl_star, "api_test_op",
                                            &node);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      ASSERT(node == NULL);
    }
    free_token_list(tl_star);
  }

  /* Function without parens in apply_updates */
  {
    struct TokenList *tl_noparen = NULL;
    struct CstNode fake_noparen;
    struct CstNodeList cst_noparen;
    rc = tokenize(az_span_create_from_str(
                      (char *)(size_t) "int api_test_op { return 0; }\n"),
                  &tl_noparen);
    ASSERT_EQ(0, rc);
    memset(&fake_noparen, 0, sizeof(fake_noparen));
    fake_noparen.kind = CST_NODE_FUNCTION;
    fake_noparen.start_token = 0;
    fake_noparen.end_token = tl_noparen->size;
    cst_noparen.nodes = &fake_noparen;
    cst_noparen.size = 1;
    cst_noparen.capacity = 1;
    rc = cdd_test_sync_apply_updates("file.c", tl_noparen, &cst_noparen, &spec,
                                     &cfg);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    free_token_list(tl_noparen);
  }

  /* Node starting with LPAREN in find_function_node (id_idx <=
   * node->start_token) */
  {
    struct TokenList *tl_lpar = NULL;
    struct CstNode fake_lpar;
    struct CstNodeList cst_lpar;
    rc = tokenize(
        az_span_create_from_str((char *)(size_t) "(void) { return 0; }\n"),
        &tl_lpar);
    ASSERT_EQ(0, rc);
    memset(&fake_lpar, 0, sizeof(fake_lpar));
    fake_lpar.kind = CST_NODE_FUNCTION;
    fake_lpar.start_token = 0;
    fake_lpar.end_token = tl_lpar->size;
    cst_lpar.nodes = &fake_lpar;
    cst_lpar.size = 1;
    cst_lpar.capacity = 1;
    rc = cdd_test_sync_find_function_node(&cst_lpar, tl_lpar, "api_test_op",
                                          &node);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT(node == NULL);
    free_token_list(tl_lpar);

    /* Test k > node->start_token but id_idx == node->start_token */
    rc = tokenize(
        az_span_create_from_str((char *)(size_t) "int (void) { return 0; }\n"),
        &tl_lpar);
    ASSERT_EQ(0, rc);
    fake_lpar.end_token = tl_lpar->size;
    cst_lpar.nodes = &fake_lpar;
    rc = cdd_test_sync_find_function_node(&cst_lpar, tl_lpar, "api_test_op",
                                          &node);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT(node == NULL);
    free_token_list(tl_lpar);
  }

  /* Function with unclosed paren in apply_updates (actual_sig == NULL) */
  {
    struct TokenList *tl_unclosed = NULL;
    struct CstNode fake_unclosed;
    struct CstNodeList cst_unclosed;
    rc = tokenize(az_span_create_from_str(
                      (char *)(size_t) "int api_test_op(int a { return 0; }\n"),
                  &tl_unclosed);
    ASSERT_EQ(0, rc);
    memset(&fake_unclosed, 0, sizeof(fake_unclosed));
    fake_unclosed.kind = CST_NODE_FUNCTION;
    fake_unclosed.start_token = 0;
    fake_unclosed.end_token = tl_unclosed->size;
    cst_unclosed.nodes = &fake_unclosed;
    cst_unclosed.size = 1;
    cst_unclosed.capacity = 1;
    rc = cdd_test_sync_apply_updates("file.c", tl_unclosed, &cst_unclosed,
                                     &spec, &cfg);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    free_token_list(tl_unclosed);
  }

  /* patch_list_apply OOM in apply_updates */
  {
    const char *snp_code = "int api_test_op(int a) {\n"
                           "  snprintf(url, 64, \"/test\");\n"
                           "  return 0;\n"
                           "}\n";
    rc = tokenize(az_span_create_from_str((char *)(size_t)snp_code), &tl);
    ASSERT_EQ(0, rc);
    rc = parse_tokens(tl, &cst);
    ASSERT_EQ(0, rc);

    g_cdd_alloc_fail = 5;
    rc = cdd_test_sync_apply_updates("file.c", tl, &cst, &spec, &cfg);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    g_cdd_alloc_fail = 0;

    free_cst_node_list(&cst);
    free_token_list(tl);
    tl = NULL;
  }

  /* 9. api_sync_file with non-NULL config and parse_tokens error */
  write_to_file("temp_sync_test2.c", "int main() { return 0; }\n");
  rc = api_sync_file("temp_sync_test2.c", &spec, &cfg);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  g_cdd_fail_cst_list_add = 1;
  rc = api_sync_file("temp_sync_test2.c", &spec, &cfg);
  ASSERT(rc != CDD_C_SUCCESS);
  g_cdd_fail_cst_list_add = 0;

  remove("file.c");
  remove("temp_sync_test2.c");

  PASS();
}

SUITE(api_sync_suite) {
#ifndef __EMSCRIPTEN__
  RUN_TEST(test_sync_signature_update);
  RUN_TEST(test_sync_url_logic_update);
  RUN_TEST(test_sync_query_update);
  RUN_TEST(test_sync_header_update);
  RUN_TEST(test_sync_full_coverage);
  RUN_TEST(test_sync_remaining_branches);
#endif
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_REFACTOR_API_SYNC_H */
