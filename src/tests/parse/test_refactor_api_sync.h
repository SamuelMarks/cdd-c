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
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = openapi_spec_init(spec);
  if (rc != CDD_C_SUCCESS) {
    json_value_free(dyn);
    return rc;
  }
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
  struct OpenAPI_Spec spec_err;
  struct OpenAPI_Spec spec;
  char *content = NULL;
  size_t sz;
  int rc;

  /* Exercise load_spec error branches */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            load_spec("{invalid_json", &spec_err));
  {
    extern C_CDD_EXPORT int g_openapi_spec_init_fail;
    g_openapi_spec_init_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, load_spec("{}", &spec_err));
    g_openapi_spec_init_fail = 0;
  }

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

SUITE(api_sync_suite) {
#ifndef __EMSCRIPTEN__
  RUN_TEST(test_sync_signature_update);
  RUN_TEST(test_sync_url_logic_update);
  RUN_TEST(test_sync_query_update);
  RUN_TEST(test_sync_header_update);
  RUN_TEST(test_sync_full_coverage);
#endif
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_REFACTOR_API_SYNC_H */
