/**
 * @file test_cst_parser_coverage_statements.h
 * @brief Unit tests for CST parser statement coverage.
 */

#ifndef TEST_CST_PARSER_COVERAGE_STATEMENTS_H
#define TEST_CST_PARSER_COVERAGE_STATEMENTS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <greatest.h>

#include "functions/parse/cst.h"
#include "functions/parse/tokenizer.h"
/* clang-format on */

#ifdef CDD_BUILD_TESTS
extern C_CDD_EXPORT int g_cdd_cst_realloc_fail;
extern C_CDD_EXPORT int g_cdd_cst_parser_fast_grow;
extern C_CDD_EXPORT int g_cdd_cst_alloc_token_fail;
extern C_CDD_EXPORT int g_cdd_fail_alloc;
extern C_CDD_EXPORT int g_cdd_fail_skip_ws;
extern C_CDD_EXPORT int g_cdd_fail_skip_ws_back;
extern C_CDD_EXPORT int g_cdd_fail_is_type_start;
extern C_CDD_EXPORT int g_cdd_fail_consume_balanced_parens;
extern C_CDD_EXPORT int g_cdd_fail_consume_attributes;
extern C_CDD_EXPORT int g_cdd_fail_consume_static_assert;
extern C_CDD_EXPORT int g_cdd_fail_consume_generic_selection;
extern C_CDD_EXPORT int g_cdd_fail_is_expression_brace;
extern C_CDD_EXPORT int g_cdd_fail_consume_balanced_braces;
extern C_CDD_EXPORT int g_cdd_fail_match_function_definition;
extern C_CDD_EXPORT int g_cdd_fail_cst_list_add;
extern C_CDD_EXPORT int g_cdd_fail_token_matches_string;
#endif

TEST test_cst_full_coverage_statements(void) {
  struct CstNodeList list;
  struct TokenList *tl = NULL;
  struct TokenList *tl_comma = NULL;
  struct TokenList *tl_bracket = NULL;
  struct TokenList tl_hash;
  struct Token h_tok;
  cdd_c_error_t rc = 0;

  /* 13. Statement with struct not preceded by paren: typedef struct S S_t; */
  rc = tokenize(
      az_span_create_from_str((char *)(size_t) "typedef struct S S_t;"), &tl);
  ASSERT_EQ(0, rc);
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  free_cst_node_list(&list);

  /* Test skip_ws_back failure inside CST_NODE_OTHER when encountering struct */
  g_cdd_fail_skip_ws_back = 1;
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT(rc != CDD_C_SUCCESS);
  g_cdd_fail_skip_ws_back = 0;
  free_cst_node_list(&list);
  free_token_list(tl);
  tl = NULL;

  /* 14. Fallthrough when _Generic is not followed by parens */
  rc = tokenize(az_span_create_from_str((char *)(size_t) "_Generic;"), &tl);
  ASSERT_EQ(0, rc);
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  free_cst_node_list(&list);
  free_token_list(tl);
  tl = NULL;

  /* 15. token_matches_string failure inside CST_NODE_OTHER loop */
  rc = tokenize(az_span_create_from_str((char *)(size_t) "int x = 1;"), &tl);
  ASSERT_EQ(0, rc);
  g_cdd_fail_token_matches_string = 3;
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT(rc != CDD_C_SUCCESS);
  g_cdd_fail_token_matches_string = 0;
  free_cst_node_list(&list);
  free_token_list(tl);
  tl = NULL;

  /* 16. C23 attribute [[ inside statement */
  rc = tokenize(
      az_span_create_from_str((char *)(size_t) "int a [[nodiscard]] int b;"),
      &tl);
  ASSERT_EQ(0, rc);
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  free_cst_node_list(&list);
  free_token_list(tl);
  tl = NULL;

  /* 14. parse_tokens parameter validation */
  rc = parse_tokens(NULL, &list);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  rc = tokenize(az_span_create_from_str((char *)(size_t) "int a;"), &tl);
  ASSERT_EQ(0, rc);
  rc = parse_tokens(tl, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  free_token_list(tl);
  tl = NULL;

  /* 15. Direct testing of wrapper functions for 100% parameter validation and
   * internal branches */
  rc = tokenize(az_span_create_from_str(
                    (char *)(size_t) "  int a; void f(); ((struct S){ 1 });"),
                &tl);
  ASSERT_EQ(0, rc);
  {
    size_t val = 0;
    int is_match = 0;
    int is_type = 0;
    size_t end_idx = 0;

    /* cdd_test_cst_skip_ws */
    rc = cdd_test_cst_skip_ws(NULL, 0, 1, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = cdd_test_cst_skip_ws(tl, 0, 1, NULL);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_fail_skip_ws = 1;
    rc = cdd_test_cst_skip_ws(tl, 0, 1, &val);
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
    g_cdd_fail_skip_ws = 0;
    rc = cdd_test_cst_skip_ws(tl, 0, tl->size, &val);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    /* cdd_test_cst_skip_ws_back */
    rc = cdd_test_cst_skip_ws_back(NULL, 1, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = cdd_test_cst_skip_ws_back(tl, 1, NULL);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = cdd_test_cst_skip_ws_back(tl, 0, &val);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_EQ(0, val);
    g_cdd_fail_skip_ws_back = 1;
    rc = cdd_test_cst_skip_ws_back(tl, 1, &val);
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
    g_cdd_fail_skip_ws_back = 0;
    rc = cdd_test_cst_skip_ws_back(tl, 2, &val);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    /* cdd_test_cst_is_type_start */
    rc = cdd_test_cst_is_type_start(NULL, &is_type);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = cdd_test_cst_is_type_start(&tl->tokens[0], NULL);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_fail_is_type_start = 2;
    rc = cdd_test_cst_is_type_start(&tl->tokens[0], &is_type);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    rc = cdd_test_cst_is_type_start(&tl->tokens[0], &is_type);
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
    g_cdd_fail_is_type_start = 0;

    /* Test is_type_start with various token kinds */
    {
      struct Token dummy;
      dummy.kind = TOKEN_KEYWORD_VOID;
      rc = cdd_test_cst_is_type_start(&dummy, &is_type);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      ASSERT_EQ(1, is_type);

      dummy.kind = TOKEN_KEYWORD_CHAR;
      rc = cdd_test_cst_is_type_start(&dummy, &is_type);
      ASSERT_EQ(1, is_type);

      dummy.kind = TOKEN_KEYWORD_INT;
      rc = cdd_test_cst_is_type_start(&dummy, &is_type);
      ASSERT_EQ(1, is_type);

      dummy.kind = TOKEN_KEYWORD_FLOAT;
      rc = cdd_test_cst_is_type_start(&dummy, &is_type);
      ASSERT_EQ(1, is_type);

      dummy.kind = TOKEN_KEYWORD_DOUBLE;
      rc = cdd_test_cst_is_type_start(&dummy, &is_type);
      ASSERT_EQ(1, is_type);

      dummy.kind = TOKEN_KEYWORD_LONG;
      rc = cdd_test_cst_is_type_start(&dummy, &is_type);
      ASSERT_EQ(1, is_type);

      dummy.kind = TOKEN_KEYWORD_SHORT;
      rc = cdd_test_cst_is_type_start(&dummy, &is_type);
      ASSERT_EQ(1, is_type);

      dummy.kind = TOKEN_KEYWORD_SIGNED;
      rc = cdd_test_cst_is_type_start(&dummy, &is_type);
      ASSERT_EQ(1, is_type);

      dummy.kind = TOKEN_KEYWORD_UNSIGNED;
      rc = cdd_test_cst_is_type_start(&dummy, &is_type);
      ASSERT_EQ(1, is_type);

      dummy.kind = TOKEN_KEYWORD_STRUCT;
      rc = cdd_test_cst_is_type_start(&dummy, &is_type);
      ASSERT_EQ(1, is_type);

      dummy.kind = TOKEN_KEYWORD_ENUM;
      rc = cdd_test_cst_is_type_start(&dummy, &is_type);
      ASSERT_EQ(1, is_type);

      dummy.kind = TOKEN_KEYWORD_UNION;
      rc = cdd_test_cst_is_type_start(&dummy, &is_type);
      ASSERT_EQ(1, is_type);

      dummy.kind = TOKEN_KEYWORD_STATIC;
      rc = cdd_test_cst_is_type_start(&dummy, &is_type);
      ASSERT_EQ(1, is_type);

      dummy.kind = TOKEN_KEYWORD_INLINE;
      rc = cdd_test_cst_is_type_start(&dummy, &is_type);
      ASSERT_EQ(1, is_type);

      dummy.kind = TOKEN_KEYWORD_EXTERN;
      rc = cdd_test_cst_is_type_start(&dummy, &is_type);
      ASSERT_EQ(1, is_type);

      dummy.kind = TOKEN_KEYWORD_CONST;
      rc = cdd_test_cst_is_type_start(&dummy, &is_type);
      ASSERT_EQ(1, is_type);

      dummy.kind = TOKEN_KEYWORD_VOLATILE;
      rc = cdd_test_cst_is_type_start(&dummy, &is_type);
      ASSERT_EQ(1, is_type);

      dummy.kind = TOKEN_KEYWORD_AUTO;
      rc = cdd_test_cst_is_type_start(&dummy, &is_type);
      ASSERT_EQ(1, is_type);

      dummy.kind = TOKEN_KEYWORD_REGISTER;
      rc = cdd_test_cst_is_type_start(&dummy, &is_type);
      ASSERT_EQ(1, is_type);

      dummy.kind = TOKEN_KEYWORD_BOOL;
      rc = cdd_test_cst_is_type_start(&dummy, &is_type);
      ASSERT_EQ(1, is_type);

      dummy.kind = TOKEN_SEMICOLON;
      rc = cdd_test_cst_is_type_start(&dummy, &is_type);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      ASSERT_EQ(0, is_type);
    }

    /* cdd_test_cst_match_function_definition */
    rc =
        cdd_test_cst_match_function_definition(NULL, 0, 0, &end_idx, &is_match);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = cdd_test_cst_match_function_definition(tl, 0, 0, NULL, &is_match);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = cdd_test_cst_match_function_definition(tl, 0, 0, &end_idx, NULL);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_fail_match_function_definition = 2;
    rc = cdd_test_cst_match_function_definition(tl, 0, tl->size, &end_idx,
                                                &is_match);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    rc = cdd_test_cst_match_function_definition(tl, 0, tl->size, &end_idx,
                                                &is_match);
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
    g_cdd_fail_match_function_definition = 0;

    /* Fail is_type_start inside match_function_definition */
    g_cdd_fail_is_type_start = 1;
    rc = cdd_test_cst_match_function_definition(tl, 0, tl->size, &end_idx,
                                                &is_match);
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
    g_cdd_fail_is_type_start = 0;

    /* cdd_test_cst_consume_balanced_parens */
    rc = cdd_test_cst_consume_balanced_parens(NULL, 0, 0, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = cdd_test_cst_consume_balanced_parens(tl, 0, 0, NULL);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = cdd_test_cst_consume_balanced_parens(tl, tl->size, tl->size, &val);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    g_cdd_fail_consume_balanced_parens = 2;
    rc = cdd_test_cst_consume_balanced_parens(tl, 0, tl->size, &val);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    rc = cdd_test_cst_consume_balanced_parens(tl, 0, tl->size, &val);
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
    g_cdd_fail_consume_balanced_parens = 0;
    {
      struct TokenList *tl_unclosed_paren = NULL;
      rc = tokenize(az_span_create_from_str((char *)(size_t) "(a + b"),
                    &tl_unclosed_paren);
      ASSERT_EQ(0, rc);
      rc = cdd_test_cst_consume_balanced_parens(tl_unclosed_paren, 0,
                                                tl_unclosed_paren->size, &val);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      ASSERT_EQ(0, val);
      /* Test starting at non-LPAREN token (token 1 is 'a') */
      rc = cdd_test_cst_consume_balanced_parens(tl_unclosed_paren, 1,
                                                tl_unclosed_paren->size, &val);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      ASSERT_EQ(1, val);
      free_token_list(tl_unclosed_paren);
    }
    {
      struct TokenList *tl_unclosed_gen = NULL;
      rc = tokenize(
          az_span_create_from_str((char *)(size_t) "_Generic(1, int: 2"),
          &tl_unclosed_gen);
      ASSERT_EQ(0, rc);
      memset(&list, 0, sizeof(list));
      rc = parse_tokens(tl_unclosed_gen, &list);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      free_cst_node_list(&list);
      free_token_list(tl_unclosed_gen);
    }

    /* cdd_test_cst_consume_attributes */
    rc = cdd_test_cst_consume_attributes(NULL, 0, 0, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = cdd_test_cst_consume_attributes(tl, 0, 0, NULL);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_fail_consume_attributes = 2;
    rc = cdd_test_cst_consume_attributes(tl, 0, tl->size, &val);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    rc = cdd_test_cst_consume_attributes(tl, 0, tl->size, &val);
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
    g_cdd_fail_consume_attributes = 0;

    /* cdd_test_cst_consume_static_assert */
    rc = cdd_test_cst_consume_static_assert(NULL, 0, 0, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = cdd_test_cst_consume_static_assert(tl, 0, 0, NULL);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_fail_consume_static_assert = 2;
    rc = cdd_test_cst_consume_static_assert(tl, 0, tl->size, &val);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    rc = cdd_test_cst_consume_static_assert(tl, 0, tl->size, &val);
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
    g_cdd_fail_consume_static_assert = 0;

    /* cdd_test_cst_consume_generic_selection */
    rc = cdd_test_cst_consume_generic_selection(NULL, 0, 0, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = cdd_test_cst_consume_generic_selection(tl, 0, 0, NULL);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = cdd_test_cst_consume_generic_selection(tl, tl->size, tl->size, &val);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    g_cdd_fail_consume_generic_selection = 2;
    rc = cdd_test_cst_consume_generic_selection(tl, 0, tl->size, &val);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    rc = cdd_test_cst_consume_generic_selection(tl, 0, tl->size, &val);
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
    g_cdd_fail_consume_generic_selection = 0;

    /* cdd_test_cst_is_expression_brace */
    rc = cdd_test_cst_is_expression_brace(NULL, 0, &is_match);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = cdd_test_cst_is_expression_brace(tl, 0, NULL);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_fail_is_expression_brace = 2;
    rc = cdd_test_cst_is_expression_brace(tl, 0, &is_match);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    rc = cdd_test_cst_is_expression_brace(tl, 0, &is_match);
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
    g_cdd_fail_is_expression_brace = 0;

    /* cdd_test_cst_consume_balanced_braces */
    rc = cdd_test_cst_consume_balanced_braces(NULL, 0, 0, &val);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    rc = cdd_test_cst_consume_balanced_braces(tl, 0, 0, NULL);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
    g_cdd_fail_consume_balanced_braces = 2;
    rc = cdd_test_cst_consume_balanced_braces(tl, 0, tl->size, &val);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    rc = cdd_test_cst_consume_balanced_braces(tl, 0, tl->size, &val);
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
    g_cdd_fail_consume_balanced_braces = 0;

    /* Call is_expression_brace on token sequence with paren before brace */
    {
      struct TokenList *tl_expr = NULL;
      size_t b_idx = 0;
      rc = tokenize(az_span_create_from_str(
                        (char *)(size_t) "int x = ((struct S){ 1 });"),
                    &tl_expr);
      ASSERT_EQ(0, rc);
      for (val = 0; val < tl_expr->size; val++) {
        if (tl_expr->tokens[val].kind == TOKEN_LBRACE) {
          b_idx = val;
          break;
        }
      }
      /* Test failure of second skip_ws_back inside is_expression_brace */
      g_cdd_fail_skip_ws_back = 2;
      rc = cdd_test_cst_is_expression_brace(tl_expr, b_idx, &is_match);
      ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
      g_cdd_fail_skip_ws_back = 0;

      rc = cdd_test_cst_is_expression_brace(tl_expr, b_idx, &is_match);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      ASSERT_EQ(1, is_match);
      free_token_list(tl_expr);
    }

    /* Test cdd_test_cst_parse_recursive starting at struct keyword immediately
     * preceded by LPAREN */
    {
      struct TokenList *tl_lit = NULL;
      memset(&list, 0, sizeof(list));
      rc =
          tokenize(az_span_create_from_str((char *)(size_t) "(struct S){ 1 };"),
                   &tl_lit);
      ASSERT_EQ(0, rc);
      /* Token 0 is LPAREN, Token 1 is struct */
      rc = cdd_test_cst_parse_recursive(tl_lit, 1, tl_lit->size, &list);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      free_cst_node_list(&list);
      free_token_list(tl_lit);
    }

    /* Test pre-allocated list (capacity != 0) */
    {
      struct CstNodeList pre_list;
      pre_list.capacity = 16;
      pre_list.size = 0;
      pre_list.nodes = (struct CstNode *)malloc(16 * sizeof(struct CstNode));
      ASSERT(pre_list.nodes != NULL);
      rc = parse_tokens(tl, &pre_list);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      free_cst_node_list(&pre_list);
    }

    /* Test isolated grammar constructs */
    {
      struct TokenList *tl_isolated = NULL;

      /* 1. *func_star() {} to test is_type == 0 && tok->kind == TOKEN_STAR */
      rc = tokenize(az_span_create_from_str((char *)(size_t) "*func_star() {}"),
                    &tl_isolated);
      ASSERT_EQ(0, rc);
      memset(&list, 0, sizeof(list));
      rc = parse_tokens(tl_isolated, &list);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      free_cst_node_list(&list);
      free_token_list(tl_isolated);

      /* 2. struct S int a; to test decl_end < end && tokens[decl_end].kind !=
       * TOKEN_SEMICOLON */
      rc = tokenize(az_span_create_from_str((char *)(size_t) "struct S int a;"),
                    &tl_isolated);
      ASSERT_EQ(0, rc);
      memset(&list, 0, sizeof(list));
      rc = parse_tokens(tl_isolated, &list);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      free_cst_node_list(&list);
      free_token_list(tl_isolated);

      /* 3. int } foo() {} to test kind == TOKEN_RBRACE before ( */
      rc = tokenize(az_span_create_from_str((char *)(size_t) "int } foo() {}"),
                    &tl_isolated);
      ASSERT_EQ(0, rc);
      memset(&list, 0, sizeof(list));
      rc = parse_tokens(tl_isolated, &list);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      free_cst_node_list(&list);
      free_token_list(tl_isolated);

      /* 4. Comma expression brace, empty struct, no-semi struct, cast
       * union/enum */
      rc = tokenize(az_span_create_from_str(
                        (char *)(size_t) "int arr[2][2] = { {1, 2}, {3, 4} };"),
                    &tl_isolated);
      ASSERT_EQ(0, rc);
      memset(&list, 0, sizeof(list));
      rc = parse_tokens(tl_isolated, &list);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      free_cst_node_list(&list);
      free_token_list(tl_isolated);

      rc = tokenize(
          az_span_create_from_str((char *)(size_t) "struct Empty { };"),
          &tl_isolated);
      ASSERT_EQ(0, rc);
      memset(&list, 0, sizeof(list));
      rc = parse_tokens(tl_isolated, &list);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      free_cst_node_list(&list);
      free_token_list(tl_isolated);

      rc = tokenize(az_span_create_from_str(
                        (char *)(size_t) "(union U *)p; (enum E *)q;"),
                    &tl_isolated);
      ASSERT_EQ(0, rc);
      memset(&list, 0, sizeof(list));
      rc = parse_tokens(tl_isolated, &list);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      free_cst_node_list(&list);
      free_token_list(tl_isolated);

      rc = tokenize(az_span_create_from_str(
                        (char *)(size_t) "_Static_assert((1), \"\");"),
                    &tl_isolated);
      ASSERT_EQ(0, rc);
      memset(&list, 0, sizeof(list));
      rc = parse_tokens(tl_isolated, &list);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      free_cst_node_list(&list);
      free_token_list(tl_isolated);

      rc = tokenize(
          az_span_create_from_str((char *)(size_t) "_Static_assert(1)"),
          &tl_isolated);
      ASSERT_EQ(0, rc);
      memset(&list, 0, sizeof(list));
      rc = parse_tokens(tl_isolated, &list);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      free_cst_node_list(&list);
      free_token_list(tl_isolated);

      rc = tokenize(
          az_span_create_from_str((char *)(size_t) "struct NoSemi { int x; }"),
          &tl_isolated);
      ASSERT_EQ(0, rc);
      memset(&list, 0, sizeof(list));
      rc = parse_tokens(tl_isolated, &list);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      free_cst_node_list(&list);
      free_token_list(tl_isolated);

      /* pk == TOKEN_COMMA before brace */
      rc = tokenize(az_span_create_from_str((char *)(size_t) ", {"), &tl_comma);
      ASSERT_EQ(0, rc);
      rc = cdd_test_cst_is_expression_brace(tl_comma, 1, &is_match);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      ASSERT_EQ(1, is_match);
      free_token_list(tl_comma);

      /* pk == TOKEN_KEYWORD_RETURN before brace */
      rc = tokenize(az_span_create_from_str((char *)(size_t) "return {"),
                    &tl_comma);
      ASSERT_EQ(0, rc);
      rc = cdd_test_cst_is_expression_brace(tl_comma, 1, &is_match);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      ASSERT_EQ(1, is_match);
      free_token_list(tl_comma);

      /* Non-type top level statement: 42; (void)0; */
      rc = tokenize(az_span_create_from_str((char *)(size_t) "42; (void)0;"),
                    &tl_comma);
      ASSERT_EQ(0, rc);
      memset(&list, 0, sizeof(list));
      rc = parse_tokens(tl_comma, &list);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      free_cst_node_list(&list);
      free_token_list(tl_comma);

      /* Struct forward decl in block without semi: { struct S } */
      rc = tokenize(az_span_create_from_str((char *)(size_t) "{ struct S }"),
                    &tl_comma);
      ASSERT_EQ(0, rc);
      memset(&list, 0, sizeof(list));
      rc = parse_tokens(tl_comma, &list);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      free_cst_node_list(&list);
      free_token_list(tl_comma);

      /* Static assert without semicolon after parens (tokens[i].kind !=
       * TOKEN_SEMICOLON) */
      rc = tokenize(
          az_span_create_from_str((char *)(size_t) "_Static_assert(1) int x;"),
          &tl_comma);
      ASSERT_EQ(0, rc);
      rc =
          cdd_test_cst_consume_static_assert(tl_comma, 0, tl_comma->size, &val);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      ASSERT_EQ(0, val);
      free_token_list(tl_comma);

      /* Single bracket at EOF (i + 1 >= end) */
      rc = tokenize(az_span_create_from_str((char *)(size_t) "["), &tl_bracket);
      ASSERT_EQ(0, rc);
      memset(&list, 0, sizeof(list));
      rc = parse_tokens(tl_bracket, &list);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      free_cst_node_list(&list);
      free_token_list(tl_bracket);

      /* [x]; tests tokens[i + 1].kind != TOKEN_LBRACKET */
      rc = tokenize(az_span_create_from_str((char *)(size_t) "[x];"),
                    &tl_bracket);
      ASSERT_EQ(0, rc);
      memset(&list, 0, sizeof(list));
      rc = parse_tokens(tl_bracket, &list);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      free_cst_node_list(&list);
      free_token_list(tl_bracket);

      /* TOKEN_HASH and TOKEN_MACRO tests */
      memset(&tl_hash, 0, sizeof(tl_hash));
      memset(&h_tok, 0, sizeof(h_tok));
      h_tok.kind = TOKEN_HASH;
      h_tok.start = (const uint8_t *)"#";
      h_tok.length = 1;
      tl_hash.tokens = &h_tok;
      tl_hash.size = 1;
      tl_hash.capacity = 1;
      memset(&list, 0, sizeof(list));
      rc = parse_tokens(&tl_hash, &list);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      free_cst_node_list(&list);

      h_tok.kind = TOKEN_MACRO;
      h_tok.start = (const uint8_t *)"#define X 1\n";
      h_tok.length = 12;
      memset(&list, 0, sizeof(list));
      rc = parse_tokens(&tl_hash, &list);
      ASSERT_EQ(CDD_C_SUCCESS, rc);
      free_cst_node_list(&list);

      /* Statement with TOKEN_COMMENT, TOKEN_MACRO, TOKEN_HASH,
       * TOKEN_KEYWORD_STATIC_ASSERT breaking CST_NODE_OTHER */
      {
        struct Token stmt_pair[2];
        memset(stmt_pair, 0, sizeof(stmt_pair));
        stmt_pair[0].kind = TOKEN_KEYWORD_INT;
        stmt_pair[0].start = (const uint8_t *)"int";
        stmt_pair[0].length = 3;

        tl_hash.tokens = stmt_pair;
        tl_hash.size = 2;
        tl_hash.capacity = 2;

        stmt_pair[1].kind = TOKEN_COMMENT;
        stmt_pair[1].start = (const uint8_t *)"/*c*/";
        stmt_pair[1].length = 5;
        memset(&list, 0, sizeof(list));
        rc = parse_tokens(&tl_hash, &list);
        ASSERT_EQ(CDD_C_SUCCESS, rc);
        free_cst_node_list(&list);

        stmt_pair[1].kind = TOKEN_MACRO;
        stmt_pair[1].start = (const uint8_t *)"#\n";
        stmt_pair[1].length = 2;
        memset(&list, 0, sizeof(list));
        rc = parse_tokens(&tl_hash, &list);
        ASSERT_EQ(CDD_C_SUCCESS, rc);
        free_cst_node_list(&list);

        stmt_pair[1].kind = TOKEN_HASH;
        stmt_pair[1].start = (const uint8_t *)"#\n";
        stmt_pair[1].length = 2;
        memset(&list, 0, sizeof(list));
        rc = parse_tokens(&tl_hash, &list);
        ASSERT_EQ(CDD_C_SUCCESS, rc);
        free_cst_node_list(&list);

        stmt_pair[1].kind = TOKEN_KEYWORD_STATIC_ASSERT;
        stmt_pair[1].start = (const uint8_t *)"_Static_assert";
        stmt_pair[1].length = 14;
        memset(&list, 0, sizeof(list));
        rc = parse_tokens(&tl_hash, &list);
        ASSERT_EQ(CDD_C_SUCCESS, rc);
        free_cst_node_list(&list);
      }
    }
  }
  free_token_list(tl);
  tl = NULL;

  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !TEST_CST_PARSER_COVERAGE_STATEMENTS_H */
