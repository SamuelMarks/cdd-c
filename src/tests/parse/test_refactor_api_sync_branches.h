/**
 * @file test_refactor_api_sync_branches.h
 * @brief Branch and error coverage tests for API sync.
 *
 * @author Samuel Marks
 */

#ifndef TEST_REFACTOR_API_SYNC_BRANCHES_H
#define TEST_REFACTOR_API_SYNC_BRANCHES_H

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

extern C_CDD_EXPORT int g_cdd_alloc_fail;
extern C_CDD_EXPORT int g_cdd_fail_cst_list_add;
extern C_CDD_EXPORT int g_cdd_fail_make_tmpfile;
extern C_CDD_EXPORT int g_cdd_fail_patch_list_add;
extern C_CDD_EXPORT int g_cdd_fail_token_matches_string;
extern C_CDD_EXPORT int g_cdd_strdup_fail;
extern C_CDD_EXPORT int g_cdd_sync_sig_no_brace;

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

SUITE(api_sync_branches_suite) { RUN_TEST(test_sync_remaining_branches); }

#endif /* !__EMSCRIPTEN__ */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !TEST_REFACTOR_API_SYNC_BRANCHES_H */
