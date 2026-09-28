/**
 * @file test_declarator_parser_helpers.h
 * @brief Unit tests for declarator parser helpers, mocks, and pivot branches.
 *
 * @author Samuel Marks
 */

#ifndef TEST_DECLARATOR_PARSER_HELPERS_H
#define TEST_DECLARATOR_PARSER_HELPERS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "functions/parse/declarator.h"
#include "functions/parse/tokenizer.h"
/* clang-format on */

extern C_CDD_EXPORT int g_cdd_fail_skip_ws;
extern C_CDD_EXPORT int g_cdd_fail_skip_ws_back;
extern C_CDD_EXPORT int g_cdd_fail_skip_group;
extern C_CDD_EXPORT int g_cdd_fail_add_type_node;
extern C_CDD_EXPORT int g_cdd_fail_create_node;
extern C_CDD_EXPORT int g_cdd_fail_find_pivot;
extern C_CDD_EXPORT int g_cdd_fail_is_grouping_paren;
extern C_CDD_EXPORT int g_cdd_alloc_fail;
extern C_CDD_EXPORT int g_cdd_strdup_fail;
extern C_CDD_EXPORT int g_fail_io_after;

static void reset_decl_helpers_mocks(void) {
  g_cdd_fail_skip_ws = 0;
  g_cdd_fail_skip_ws_back = 0;
  g_cdd_fail_skip_group = 0;
  g_cdd_fail_add_type_node = 0;
  g_cdd_fail_create_node = 0;
  g_cdd_fail_find_pivot = 0;
  g_cdd_fail_is_grouping_paren = 0;
  g_cdd_alloc_fail = 0;
  g_cdd_strdup_fail = 0;
  g_fail_io_after = -1;
}

static struct TokenList *setup_tokens_helpers(const char *code) {
  struct TokenList *tl = NULL;
  cdd_c_error_t rc;
  reset_decl_helpers_mocks();
  rc = tokenize(az_span_create_from_str((char *)(size_t)(size_t)code), &tl);
  if (rc != CDD_C_SUCCESS) {
    return NULL;
  }
  return tl;
}

/**
 * @brief Tests unit helper functions for declarator parser.
 */
TEST test_declarator_unit_helpers(void) {
  struct TokenList *tl;
  char *str = NULL;
  size_t val = 0;
  struct DeclType *node = NULL;
  struct DeclInfo info;
  struct DeclType *tail = NULL;
  int is_group = 0;
  int is_abstract = 0;
  struct TokenList *tl_comment = NULL;

  tl = setup_tokens_helpers("int x;");
  ASSERT(tl != NULL);

  /* join_tokens_range */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_join_tokens_range(NULL, 0, 1, &str));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_join_tokens_range(tl, 0, 1, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_join_tokens_range(tl, 2, 1, &str));
  ASSERT(str != NULL);
  C_CDD_FREE(str);
  str = NULL;

  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_test_join_tokens_range(tl, 2, 1, &str));
  reset_decl_helpers_mocks();

  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_join_tokens_range(tl, 0, 2, &str));
  ASSERT(str != NULL);
  C_CDD_FREE(str);
  str = NULL;

  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_test_join_tokens_range(tl, 0, 2, &str));
  reset_decl_helpers_mocks();

  /* skip_ws */
  g_cdd_fail_skip_ws = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, cdd_test_skip_ws(tl, 0, 1, &val));
  reset_decl_helpers_mocks();
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_test_skip_ws(NULL, 0, 1, &val));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_test_skip_ws(tl, 0, 1, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_skip_ws(tl, 0, tl->size, &val));

  tl_comment = setup_tokens_helpers("int /* c1 */ * /* c2 */ x;");
  ASSERT(tl_comment != NULL);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_skip_ws(tl_comment, 1, tl_comment->size, &val));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_skip_ws_back(tl_comment, 3, 0, &val));
  free_token_list(tl_comment);
  tl_comment = NULL;

  /* skip_ws_back */
  g_cdd_fail_skip_ws_back = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, cdd_test_skip_ws_back(tl, 1, 0, &val));
  reset_decl_helpers_mocks();
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_skip_ws_back(NULL, 1, 0, &val));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_skip_ws_back(tl, 1, 0, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_skip_ws_back(tl, 0, 1, &val));
  ASSERT_EQ(SIZE_MAX, val);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_skip_ws_back(tl, 1, 0, &val));

  /* skip_group */
  g_cdd_fail_skip_group = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_skip_group(tl, 0, 1, TOKEN_LPAREN, TOKEN_RPAREN, &val));
  reset_decl_helpers_mocks();
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_skip_group(NULL, 0, 1, TOKEN_LPAREN, TOKEN_RPAREN, &val));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_skip_group(tl, 0, 1, TOKEN_LPAREN, TOKEN_RPAREN, NULL));

  /* create_node */
  g_cdd_fail_create_node = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_test_create_node(DECL_BASE, &node));
  reset_decl_helpers_mocks();
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_create_node(DECL_BASE, NULL));
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_test_create_node(DECL_BASE, &node));
  reset_decl_helpers_mocks();
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_create_node(DECL_BASE, &node));
  ASSERT(node != NULL);
  cdd_test_free_decl_type(node);
  node = NULL;

  /* add_type_node */
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_create_node(DECL_BASE, &node));
  ASSERT_EQ(CDD_C_SUCCESS, decl_info_init(&info));
  g_cdd_fail_add_type_node = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, add_type_node(&info, &tail, node));
  reset_decl_helpers_mocks();
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, add_type_node(NULL, &tail, node));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, add_type_node(&info, NULL, node));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, add_type_node(&info, &tail, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, add_type_node(&info, &tail, node));
  node = NULL;
  decl_info_free(&info);

  /* is_grouping_paren */
  g_cdd_fail_is_grouping_paren = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, is_grouping_paren(tl, 0, 1, &is_group));
  reset_decl_helpers_mocks();
  g_cdd_fail_skip_ws = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, is_grouping_paren(tl, 0, 1, &is_group));
  reset_decl_helpers_mocks();

  /* find_pivot */
  g_cdd_fail_find_pivot = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_find_pivot(tl, 0, 1, &is_abstract, &val));
  reset_decl_helpers_mocks();
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_find_pivot(NULL, 0, 1, &is_abstract, &val));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_find_pivot(tl, 0, 1, NULL, &val));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_find_pivot(tl, 0, 1, &is_abstract, NULL));

  /* find_abstract_pivot */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_find_abstract_pivot(NULL, 0, 1, &val));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_test_find_abstract_pivot(tl, 0, 1, NULL));

  /* cdd_test_free_decl_type branches */
  cdd_test_free_decl_type(NULL);

  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_create_node(DECL_PTR, &node));
  node->data.ptr.qualifiers = (char *)C_CDD_MALLOC(5);
  memcpy(node->data.ptr.qualifiers, "test", 5);
  cdd_test_free_decl_type(node);
  node = NULL;

  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_create_node(DECL_ARRAY, &node));
  node->data.array.size_expr = (char *)C_CDD_MALLOC(5);
  memcpy(node->data.array.size_expr, "test", 5);
  cdd_test_free_decl_type(node);
  node = NULL;

  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_create_node(DECL_FUNC, &node));
  node->data.func.args_str = (char *)C_CDD_MALLOC(5);
  memcpy(node->data.func.args_str, "test", 5);
  cdd_test_free_decl_type(node);
  node = NULL;

  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_create_node(DECL_BASE, &node));
  node->data.base.name = (char *)C_CDD_MALLOC(5);
  memcpy(node->data.base.name, "test", 5);
  cdd_test_free_decl_type(node);
  node = NULL;

  free_token_list(tl);
  PASS();
}

/**
 * @brief Tests declarator parser mock failures and edge cases.
 */
TEST test_declarator_mock_failures(void) {
  struct TokenList *tl;
  struct DeclInfo info;

  /* find_pivot fail */
  tl = setup_tokens_helpers("int x");
  g_cdd_fail_find_pivot = 1;
  ASSERT_NEQ(CDD_C_SUCCESS, parse_declaration(tl, 0, tl->size, &info));
  reset_decl_helpers_mocks();

  /* join_tokens_range fail for identifier */
  g_cdd_alloc_fail = 1;
  ASSERT_NEQ(CDD_C_SUCCESS, parse_declaration(tl, 0, tl->size, &info));
  reset_decl_helpers_mocks();

  /* skip_ws_back fail on left */
  g_cdd_fail_skip_ws_back = 1;
  ASSERT_NEQ(CDD_C_SUCCESS, parse_declaration(tl, 0, tl->size, &info));
  reset_decl_helpers_mocks();

  /* skip_ws fail on right */
  g_cdd_fail_skip_ws = 1;
  ASSERT_NEQ(CDD_C_SUCCESS, parse_declaration(tl, 0, tl->size, &info));
  reset_decl_helpers_mocks();
  free_token_list(tl);

  /* abstract declarator skip_ws_back on left */
  tl = setup_tokens_helpers("int *");
  g_cdd_fail_skip_ws_back = 1;
  ASSERT_NEQ(CDD_C_SUCCESS, parse_declaration(tl, 0, tl->size, &info));
  reset_decl_helpers_mocks();

  /* abstract declarator skip_ws on right */
  g_cdd_fail_skip_ws = 1;
  ASSERT_NEQ(CDD_C_SUCCESS, parse_declaration(tl, 0, tl->size, &info));
  reset_decl_helpers_mocks();
  free_token_list(tl);

  /* array failures */
  tl = setup_tokens_helpers("int a[10]");
  g_cdd_fail_create_node = 1;
  ASSERT_NEQ(CDD_C_SUCCESS, parse_declaration(tl, 0, tl->size, &info));
  reset_decl_helpers_mocks();

  g_cdd_fail_skip_group = 1;
  ASSERT_NEQ(CDD_C_SUCCESS, parse_declaration(tl, 0, tl->size, &info));
  reset_decl_helpers_mocks();

  g_cdd_alloc_fail = 3;
  ASSERT_NEQ(CDD_C_SUCCESS, parse_declaration(tl, 0, tl->size, &info));
  reset_decl_helpers_mocks();

  g_cdd_fail_add_type_node = 1;
  ASSERT_NEQ(CDD_C_SUCCESS, parse_declaration(tl, 0, tl->size, &info));
  reset_decl_helpers_mocks();

  g_cdd_fail_skip_ws = 2;
  ASSERT_NEQ(CDD_C_SUCCESS, parse_declaration(tl, 0, tl->size, &info));
  reset_decl_helpers_mocks();
  free_token_list(tl);

  /* function failures */
  tl = setup_tokens_helpers("int f(void)");
  g_cdd_fail_create_node = 1;
  ASSERT_NEQ(CDD_C_SUCCESS, parse_declaration(tl, 0, tl->size, &info));
  reset_decl_helpers_mocks();

  g_cdd_fail_skip_group = 1;
  ASSERT_NEQ(CDD_C_SUCCESS, parse_declaration(tl, 0, tl->size, &info));
  reset_decl_helpers_mocks();

  g_cdd_alloc_fail = 3;
  ASSERT_NEQ(CDD_C_SUCCESS, parse_declaration(tl, 0, tl->size, &info));
  reset_decl_helpers_mocks();

  g_cdd_fail_add_type_node = 1;
  ASSERT_NEQ(CDD_C_SUCCESS, parse_declaration(tl, 0, tl->size, &info));
  reset_decl_helpers_mocks();

  g_cdd_fail_skip_ws = 2;
  ASSERT_NEQ(CDD_C_SUCCESS, parse_declaration(tl, 0, tl->size, &info));
  reset_decl_helpers_mocks();
  free_token_list(tl);

  /* pointer and qualifier failures */
  tl = setup_tokens_helpers("int * const p");
  g_cdd_fail_create_node = 1;
  ASSERT_NEQ(CDD_C_SUCCESS, parse_declaration(tl, 0, tl->size, &info));
  reset_decl_helpers_mocks();

  g_cdd_alloc_fail = 3;
  ASSERT_NEQ(CDD_C_SUCCESS, parse_declaration(tl, 0, tl->size, &info));
  reset_decl_helpers_mocks();

  g_cdd_fail_add_type_node = 1;
  ASSERT_NEQ(CDD_C_SUCCESS, parse_declaration(tl, 0, tl->size, &info));
  reset_decl_helpers_mocks();

  g_cdd_fail_skip_ws_back = 2;
  ASSERT_NEQ(CDD_C_SUCCESS, parse_declaration(tl, 0, tl->size, &info));
  reset_decl_helpers_mocks();

  g_cdd_fail_skip_ws_back = 3;
  ASSERT_NEQ(CDD_C_SUCCESS, parse_declaration(tl, 0, tl->size, &info));
  reset_decl_helpers_mocks();
  free_token_list(tl);

  /* grouping paren unnesting */
  tl = setup_tokens_helpers("int (*p)");
  g_cdd_fail_skip_ws_back = 3;
  ASSERT_NEQ(CDD_C_SUCCESS, parse_declaration(tl, 0, tl->size, &info));
  reset_decl_helpers_mocks();

  g_cdd_fail_skip_ws = 2;
  ASSERT_NEQ(CDD_C_SUCCESS, parse_declaration(tl, 0, tl->size, &info));
  reset_decl_helpers_mocks();
  free_token_list(tl);

  /* base type failures */
  tl = setup_tokens_helpers("int x");
  g_cdd_fail_create_node = 1;
  ASSERT_NEQ(CDD_C_SUCCESS, parse_declaration(tl, 0, tl->size, &info));
  reset_decl_helpers_mocks();

  g_cdd_alloc_fail = 3;
  ASSERT_NEQ(CDD_C_SUCCESS, parse_declaration(tl, 0, tl->size, &info));
  reset_decl_helpers_mocks();

  g_cdd_fail_add_type_node = 1;
  ASSERT_NEQ(CDD_C_SUCCESS, parse_declaration(tl, 0, tl->size, &info));
  reset_decl_helpers_mocks();
  free_token_list(tl);

  tl = setup_tokens_helpers("x");
  g_cdd_strdup_fail = 1;
  ASSERT_NEQ(CDD_C_SUCCESS, parse_declaration(tl, 0, tl->size, &info));
  reset_decl_helpers_mocks();
  free_token_list(tl);

  PASS();
}

/**
 * @brief Tests abstract pivot error branches.
 */
TEST test_declarator_abstract_pivot_branches(void) {
  struct TokenList *tl;
  size_t val = 0;

  tl = setup_tokens_helpers("struct S { int a; } *");
  g_cdd_fail_skip_ws = 1;
  ASSERT_NEQ(CDD_C_SUCCESS,
             cdd_test_find_abstract_pivot(tl, 0, tl->size, &val));
  reset_decl_helpers_mocks();

  g_cdd_fail_skip_ws = 2;
  ASSERT_NEQ(CDD_C_SUCCESS,
             cdd_test_find_abstract_pivot(tl, 0, tl->size, &val));
  reset_decl_helpers_mocks();

  g_cdd_fail_skip_group = 1;
  ASSERT_NEQ(CDD_C_SUCCESS,
             cdd_test_find_abstract_pivot(tl, 0, tl->size, &val));
  reset_decl_helpers_mocks();
  free_token_list(tl);

  tl = setup_tokens_helpers("typeof(int) *");
  g_cdd_fail_skip_ws = 1;
  ASSERT_NEQ(CDD_C_SUCCESS,
             cdd_test_find_abstract_pivot(tl, 0, tl->size, &val));
  reset_decl_helpers_mocks();

  g_cdd_fail_skip_group = 1;
  ASSERT_NEQ(CDD_C_SUCCESS,
             cdd_test_find_abstract_pivot(tl, 0, tl->size, &val));
  reset_decl_helpers_mocks();
  free_token_list(tl);

  tl = setup_tokens_helpers("int (*)(int)");
  g_cdd_fail_is_grouping_paren = 1;
  ASSERT_NEQ(CDD_C_SUCCESS,
             cdd_test_find_abstract_pivot(tl, 0, tl->size, &val));
  reset_decl_helpers_mocks();

  g_cdd_fail_skip_group = 1;
  ASSERT_NEQ(CDD_C_SUCCESS,
             cdd_test_find_abstract_pivot(tl, 0, tl->size, &val));
  reset_decl_helpers_mocks();
  free_token_list(tl);

  tl = setup_tokens_helpers("struct S *");
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_find_abstract_pivot(tl, 0, tl->size, &val));
  free_token_list(tl);

  tl = setup_tokens_helpers("typeof *");
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_find_abstract_pivot(tl, 0, tl->size, &val));
  free_token_list(tl);

  tl = setup_tokens_helpers("struct");
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_find_abstract_pivot(tl, 0, tl->size, &val));
  free_token_list(tl);

  tl = setup_tokens_helpers("typeof");
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_find_abstract_pivot(tl, 0, tl->size, &val));
  free_token_list(tl);

  tl = setup_tokens_helpers("int [5]");
  g_cdd_fail_skip_group = 1;
  ASSERT_NEQ(CDD_C_SUCCESS,
             cdd_test_find_abstract_pivot(tl, 0, tl->size, &val));
  reset_decl_helpers_mocks();
  free_token_list(tl);

  PASS();
}

/**
 * @brief Tests find_pivot error branches.
 */
TEST test_declarator_find_pivot_branches(void) {
  struct TokenList *tl;
  int is_abstract = 0;
  size_t val = 0;

  tl = setup_tokens_helpers("struct S { int a; } x;");
  g_cdd_fail_skip_ws = 1;
  ASSERT_NEQ(CDD_C_SUCCESS,
             cdd_test_find_pivot(tl, 0, tl->size, &is_abstract, &val));
  reset_decl_helpers_mocks();

  g_cdd_fail_skip_ws = 2;
  ASSERT_NEQ(CDD_C_SUCCESS,
             cdd_test_find_pivot(tl, 0, tl->size, &is_abstract, &val));
  reset_decl_helpers_mocks();

  g_cdd_fail_skip_group = 1;
  ASSERT_NEQ(CDD_C_SUCCESS,
             cdd_test_find_pivot(tl, 0, tl->size, &is_abstract, &val));
  reset_decl_helpers_mocks();
  free_token_list(tl);

  tl = setup_tokens_helpers("typeof(int) x;");
  g_cdd_fail_skip_ws = 1;
  ASSERT_NEQ(CDD_C_SUCCESS,
             cdd_test_find_pivot(tl, 0, tl->size, &is_abstract, &val));
  reset_decl_helpers_mocks();

  g_cdd_fail_skip_group = 1;
  ASSERT_NEQ(CDD_C_SUCCESS,
             cdd_test_find_pivot(tl, 0, tl->size, &is_abstract, &val));
  reset_decl_helpers_mocks();
  free_token_list(tl);

  tl = setup_tokens_helpers("struct S x;");
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_find_pivot(tl, 0, tl->size, &is_abstract, &val));
  free_token_list(tl);

  tl = setup_tokens_helpers("typeof x;");
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_find_pivot(tl, 0, tl->size, &is_abstract, &val));
  free_token_list(tl);

  tl = setup_tokens_helpers("struct");
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_find_pivot(tl, 0, tl->size, &is_abstract, &val));
  free_token_list(tl);

  tl = setup_tokens_helpers("typeof");
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_find_pivot(tl, 0, tl->size, &is_abstract, &val));
  free_token_list(tl);

  PASS();
}

/**
 * @brief Tests mock cycling for 100% branch coverage of mock conditions.
 */
TEST test_declarator_mock_cycles(void) {
  struct TokenList *tl;
  size_t val = 0;
  int is_abstract = 0;
  int is_group = 0;
  struct DeclType *node = NULL;
  struct DeclType *tail = NULL;
  struct DeclInfo info;
  char *str = NULL;

  tl = setup_tokens_helpers("int x;");

  /* skip_ws cycle */
  g_cdd_fail_skip_ws = 2;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_skip_ws(tl, 0, 1, &val));
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, cdd_test_skip_ws(tl, 0, 1, &val));
  reset_decl_helpers_mocks();

  /* skip_ws_back cycle */
  g_cdd_fail_skip_ws_back = 2;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_skip_ws_back(tl, 1, 0, &val));
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, cdd_test_skip_ws_back(tl, 1, 0, &val));
  reset_decl_helpers_mocks();

  /* skip_group cycle */
  g_cdd_fail_skip_group = 2;
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_test_skip_group(tl, 0, 1, TOKEN_LPAREN, TOKEN_RPAREN, &val));
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_skip_group(tl, 0, 1, TOKEN_LPAREN, TOKEN_RPAREN, &val));
  reset_decl_helpers_mocks();

  /* add_type_node cycle */
  ASSERT_EQ(CDD_C_SUCCESS, decl_info_init(&info));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_create_node(DECL_BASE, &node));
  g_cdd_fail_add_type_node = 2;
  ASSERT_EQ(CDD_C_SUCCESS, add_type_node(&info, &tail, node));
  node = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_create_node(DECL_BASE, &node));
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, add_type_node(&info, &tail, node));
  cdd_test_free_decl_type(node);
  node = NULL;
  decl_info_free(&info);
  reset_decl_helpers_mocks();

  /* create_node cycle */
  g_cdd_fail_create_node = 2;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_create_node(DECL_BASE, &node));
  cdd_test_free_decl_type(node);
  node = NULL;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_test_create_node(DECL_BASE, &node));
  reset_decl_helpers_mocks();

  /* create_node alloc fail cycle */
  g_cdd_fail_create_node = 0;
  g_cdd_alloc_fail = 2;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_create_node(DECL_BASE, &node));
  cdd_test_free_decl_type(node);
  node = NULL;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_test_create_node(DECL_BASE, &node));
  reset_decl_helpers_mocks();

  /* create_node with NULL out_val under mock */
  g_cdd_fail_create_node = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_test_create_node(DECL_BASE, NULL));
  reset_decl_helpers_mocks();

  /* is_grouping_paren cycle */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            is_grouping_paren(NULL, 0, 1, &is_group));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, is_grouping_paren(tl, 0, 1, NULL));
  g_cdd_fail_is_grouping_paren = 2;
  ASSERT_EQ(CDD_C_SUCCESS, is_grouping_paren(tl, 0, 1, &is_group));
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, is_grouping_paren(tl, 0, 1, &is_group));
  reset_decl_helpers_mocks();

  /* find_pivot cycle */
  g_cdd_fail_find_pivot = 2;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_find_pivot(tl, 0, 1, &is_abstract, &val));
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
            cdd_test_find_pivot(tl, 0, 1, &is_abstract, &val));
  reset_decl_helpers_mocks();

  /* C_CDD_MALLOC cycle for start >= end */
  g_cdd_alloc_fail = 2;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_join_tokens_range(tl, 2, 1, &str));
  C_CDD_FREE(str);
  str = NULL;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_test_join_tokens_range(tl, 2, 1, &str));
  reset_decl_helpers_mocks();

  /* C_CDD_MALLOC cycle for start < end */
  g_cdd_alloc_fail = 2;
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_join_tokens_range(tl, 0, 2, &str));
  C_CDD_FREE(str);
  str = NULL;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_test_join_tokens_range(tl, 0, 2, &str));
  reset_decl_helpers_mocks();

  /* default switch branch in free_decl_type */
  ASSERT_EQ(CDD_C_SUCCESS, cdd_test_create_node((enum DeclTypeKind)99, &node));
  cdd_test_free_decl_type(node);
  node = NULL;

  free_token_list(tl);
  PASS();
}

SUITE(declarator_parser_helpers_suite) {
#if defined(_MSC_VER) && _MSC_VER <= 1400
  /* skipped on old msvc */
#else
  RUN_TEST(test_declarator_unit_helpers);
  RUN_TEST(test_declarator_mock_failures);
  RUN_TEST(test_declarator_abstract_pivot_branches);
  RUN_TEST(test_declarator_find_pivot_branches);
  RUN_TEST(test_declarator_mock_cycles);
#endif
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !TEST_DECLARATOR_PARSER_HELPERS_H */
