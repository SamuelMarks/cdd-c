/**
 * @file test_cst_parser_coverage.h
 * @brief Unit tests for CST parser node coverage.
 */

#ifndef TEST_CST_PARSER_COVERAGE_H
#define TEST_CST_PARSER_COVERAGE_H

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

TEST test_cst_full_coverage(void) {
  struct CstNodeList list;
  struct CstNode *found_node = NULL;
  struct TokenList *tl = NULL;
  cdd_c_error_t rc;

  /* 1. Direct cst_list_add tests */
  memset(&list, 0, sizeof(list));
  rc = cst_list_add(NULL, CST_NODE_OTHER, NULL, 0, 0, 0);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  g_cdd_fail_cst_list_add = 2;
  rc = cst_list_add(&list, CST_NODE_OTHER, (const uint8_t *)"x", 1, 0, 1);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = cst_list_add(&list, CST_NODE_OTHER, (const uint8_t *)"x", 1, 0, 1);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  g_cdd_fail_cst_list_add = 0;
  free_cst_node_list(&list);

  memset(&list, 0, sizeof(list));
  g_cdd_fail_alloc = 2;
  rc = cst_list_add(&list, CST_NODE_OTHER, (const uint8_t *)"x", 1, 0, 1);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = cst_list_add(&list, CST_NODE_OTHER, (const uint8_t *)"x", 1, 0, 1);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  g_cdd_fail_alloc = 0;
  free_cst_node_list(&list);

  memset(&list, 0, sizeof(list));

  /* First allocation (capacity was 0) */
  rc = cst_list_add(&list, CST_NODE_OTHER, (const uint8_t *)"x", 1, 0, 1);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, list.size);
  ASSERT_EQ(64, list.capacity);

  /* Force realloc growth (capacity > 0) */
  list.size = list.capacity;
  rc = cst_list_add(&list, CST_NODE_FUNCTION, (const uint8_t *)"f", 1, 1, 2);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(128, list.capacity);

  /* 2. cst_find_first tests */
  rc = cst_find_first(&list, CST_NODE_FUNCTION, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  rc = cst_find_first(NULL, CST_NODE_FUNCTION, &found_node);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(found_node == NULL);

  rc = cst_find_first(&list, CST_NODE_UNKNOWN, &found_node);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(found_node == NULL);

  rc = cst_find_first(&list, CST_NODE_FUNCTION, &found_node);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(found_node != NULL);
  ASSERT_EQ(CST_NODE_FUNCTION, found_node->kind);

  /* 3. free_cst_node_list tests */
  free_cst_node_list(NULL);
  free_cst_node_list(&list);
  ASSERT(list.nodes == NULL);
  ASSERT_EQ(0, list.size);
  ASSERT_EQ(0, list.capacity);
  free_cst_node_list(&list); /* Second free on empty list */

  /* 4. Function definition edge branches */
  /* EOF right after closing paren (k >= limit) */
  rc = tokenize(az_span_create_from_str((char *)(size_t) "int foo()"), &tl);
  ASSERT_EQ(0, rc);
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  free_cst_node_list(&list);
  free_token_list(tl);
  tl = NULL;

  /* Unclosed brace at EOF (brace_depth > 0) */
  rc = tokenize(az_span_create_from_str((char *)(size_t) "int foo() {"), &tl);
  ASSERT_EQ(0, rc);
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  free_cst_node_list(&list);
  free_token_list(tl);
  tl = NULL;

  /* Unclosed paren at EOF */
  rc = tokenize(az_span_create_from_str((char *)(size_t) "int foo(int x"), &tl);
  ASSERT_EQ(0, rc);
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  free_cst_node_list(&list);
  free_token_list(tl);
  tl = NULL;

  /* Non-brace after closing paren */
  rc =
      tokenize(az_span_create_from_str((char *)(size_t) "int foo() = 0;"), &tl);
  ASSERT_EQ(0, rc);
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  free_cst_node_list(&list);
  free_token_list(tl);
  tl = NULL;

  /* Assignment / Number / Semicolon before paren */
  rc = tokenize(
      az_span_create_from_str((char *)(size_t) "int a = 1; int 42; int c;"),
      &tl);
  ASSERT_EQ(0, rc);
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  free_cst_node_list(&list);
  free_token_list(tl);
  tl = NULL;

  /* Failure injections in match_function_definition */
  rc = tokenize(
      az_span_create_from_str((char *)(size_t) "int foo() { return 0; }"), &tl);
  ASSERT_EQ(0, rc);
  g_cdd_fail_is_type_start = 1;
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT(rc != CDD_C_SUCCESS);
  g_cdd_fail_is_type_start = 0;
  free_cst_node_list(&list);

  g_cdd_fail_skip_ws = 1;
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT(rc != CDD_C_SUCCESS);
  g_cdd_fail_skip_ws = 0;
  free_cst_node_list(&list);

  g_cdd_fail_match_function_definition = 1;
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT(rc != CDD_C_SUCCESS);
  g_cdd_fail_match_function_definition = 0;
  free_cst_node_list(&list);

  g_cdd_fail_cst_list_add = 1;
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT(rc != CDD_C_SUCCESS);
  g_cdd_fail_cst_list_add = 0;
  free_cst_node_list(&list);
  free_token_list(tl);
  tl = NULL;

  /* 5. GCC Attributes */
  rc = tokenize(az_span_create_from_str(
                    (char *)(size_t) "__attribute__((unused)) int x;"),
                &tl);
  ASSERT_EQ(0, rc);
  g_cdd_fail_cst_list_add = 1;
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT(rc != CDD_C_SUCCESS);
  g_cdd_fail_cst_list_add = 0;
  free_cst_node_list(&list);
  free_token_list(tl);
  tl = NULL;

  rc = tokenize(
      az_span_create_from_str((char *)(size_t) "__attribute__((unused"), &tl);
  ASSERT_EQ(0, rc);
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  free_cst_node_list(&list);
  free_token_list(tl);
  tl = NULL;

  /* 6. MSVC Declspec */
  rc = tokenize(
      az_span_create_from_str((char *)(size_t) "__declspec(dllexport) int x;"),
      &tl);
  ASSERT_EQ(0, rc);
  g_cdd_fail_cst_list_add = 1;
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT(rc != CDD_C_SUCCESS);
  g_cdd_fail_cst_list_add = 0;
  free_cst_node_list(&list);
  free_token_list(tl);
  tl = NULL;

  rc = tokenize(
      az_span_create_from_str((char *)(size_t) "__declspec(dllexport"), &tl);
  ASSERT_EQ(0, rc);
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  free_cst_node_list(&list);
  free_token_list(tl);
  tl = NULL;

  /* 7. C23 Attributes */
  rc = tokenize(
      az_span_create_from_str((char *)(size_t) "[[nodiscard]] int x;"), &tl);
  ASSERT_EQ(0, rc);
  g_cdd_fail_consume_attributes = 1;
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT(rc != CDD_C_SUCCESS);
  g_cdd_fail_consume_attributes = 0;
  free_cst_node_list(&list);

  g_cdd_fail_cst_list_add = 1;
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT(rc != CDD_C_SUCCESS);
  g_cdd_fail_cst_list_add = 0;
  free_cst_node_list(&list);
  free_token_list(tl);
  tl = NULL;

  rc = tokenize(az_span_create_from_str((char *)(size_t) "[[nodiscard"), &tl);
  ASSERT_EQ(0, rc);
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  free_cst_node_list(&list);
  free_token_list(tl);
  tl = NULL;

  /* 8. Static Assert */
  rc = tokenize(
      az_span_create_from_str((char *)(size_t) "_Static_assert(1, \"msg\");"),
      &tl);
  ASSERT_EQ(0, rc);
  g_cdd_fail_skip_ws = 1;
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT(rc != CDD_C_SUCCESS);
  g_cdd_fail_skip_ws = 0;
  free_cst_node_list(&list);

  g_cdd_fail_skip_ws = 2;
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT(rc != CDD_C_SUCCESS);
  g_cdd_fail_skip_ws = 0;
  free_cst_node_list(&list);

  g_cdd_fail_consume_static_assert = 1;
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT(rc != CDD_C_SUCCESS);
  g_cdd_fail_consume_static_assert = 0;
  free_cst_node_list(&list);

  g_cdd_fail_cst_list_add = 1;
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT(rc != CDD_C_SUCCESS);
  g_cdd_fail_cst_list_add = 0;
  free_cst_node_list(&list);
  free_token_list(tl);
  tl = NULL;

  /* 9. C11 _Generic */
  rc = tokenize(
      az_span_create_from_str((char *)(size_t) "_Generic(1, int: 2);"), &tl);
  ASSERT_EQ(0, rc);
  g_cdd_fail_token_matches_string = 1;
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT(rc != CDD_C_SUCCESS);
  g_cdd_fail_token_matches_string = 0;
  free_cst_node_list(&list);

  g_cdd_fail_token_matches_string = 2;
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT(rc != CDD_C_SUCCESS);
  g_cdd_fail_token_matches_string = 0;
  free_cst_node_list(&list);

  g_cdd_fail_skip_ws = 1;
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT(rc != CDD_C_SUCCESS);
  g_cdd_fail_skip_ws = 0;
  free_cst_node_list(&list);

  g_cdd_fail_consume_balanced_parens = 1;
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT(rc != CDD_C_SUCCESS);
  g_cdd_fail_consume_balanced_parens = 0;
  free_cst_node_list(&list);

  g_cdd_fail_consume_generic_selection = 1;
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT(rc != CDD_C_SUCCESS);
  g_cdd_fail_consume_generic_selection = 0;
  free_cst_node_list(&list);

  g_cdd_fail_cst_list_add = 1;
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT(rc != CDD_C_SUCCESS);
  g_cdd_fail_cst_list_add = 0;
  free_cst_node_list(&list);
  free_token_list(tl);
  tl = NULL;

  rc = tokenize(
      az_span_create_from_str((char *)(size_t) "generic_selection(1, int: 2);"),
      &tl);
  ASSERT_EQ(0, rc);
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  free_cst_node_list(&list);
  free_token_list(tl);
  tl = NULL;

  /* 10. Struct / Enum / Union */
  rc = tokenize(
      az_span_create_from_str((char *)(size_t) "struct S { int x; }; enum E { "
                                               "A }; union U { int y; };"),
      &tl);
  ASSERT_EQ(0, rc);
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  free_cst_node_list(&list);
  free_token_list(tl);
  tl = NULL;

  /* Struct with preceding tokens not LPAREN */
  rc = tokenize(
      az_span_create_from_str((char *)(size_t) "int z; struct S { int x; };"),
      &tl);
  ASSERT_EQ(0, rc);
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  free_cst_node_list(&list);
  free_token_list(tl);
  tl = NULL;

  /* Failures on struct parsing */
  rc = tokenize(
      az_span_create_from_str((char *)(size_t) "struct S { int x; };"), &tl);
  ASSERT_EQ(0, rc);
  g_cdd_fail_skip_ws_back = 1;
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT(rc != CDD_C_SUCCESS);
  g_cdd_fail_skip_ws_back = 0;
  free_cst_node_list(&list);

  g_cdd_fail_consume_balanced_braces = 1;
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT(rc != CDD_C_SUCCESS);
  g_cdd_fail_consume_balanced_braces = 0;
  free_cst_node_list(&list);

  g_cdd_fail_skip_ws = 1;
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT(rc != CDD_C_SUCCESS);
  g_cdd_fail_skip_ws = 0;
  free_cst_node_list(&list);

  g_cdd_fail_cst_list_add = 1;
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT(rc != CDD_C_SUCCESS);
  g_cdd_fail_cst_list_add = 0;
  free_cst_node_list(&list);

  g_cdd_fail_cst_list_add = 2; /* Fails in inner recursive parse */
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT(rc != CDD_C_SUCCESS);
  g_cdd_fail_cst_list_add = 0;
  free_cst_node_list(&list);
  free_token_list(tl);
  tl = NULL;

  /* Forward declaration with and without semicolon */
  rc = tokenize(az_span_create_from_str((char *)(size_t) "struct ForwardDecl;"),
                &tl);
  ASSERT_EQ(0, rc);
  g_cdd_fail_cst_list_add = 1;
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT(rc != CDD_C_SUCCESS);
  g_cdd_fail_cst_list_add = 0;
  free_cst_node_list(&list);
  free_token_list(tl);
  tl = NULL;

  rc = tokenize(
      az_span_create_from_str((char *)(size_t) "struct ForwardNoSemi"), &tl);
  ASSERT_EQ(0, rc);
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  free_cst_node_list(&list);
  free_token_list(tl);
  tl = NULL;

  rc = tokenize(az_span_create_from_str((char *)(size_t) "struct"), &tl);
  ASSERT_EQ(0, rc);
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  free_cst_node_list(&list);
  free_token_list(tl);
  tl = NULL;

  rc = tokenize(az_span_create_from_str((char *)(size_t) "enum ForwardEnum;"),
                &tl);
  ASSERT_EQ(0, rc);
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  free_cst_node_list(&list);
  free_token_list(tl);
  tl = NULL;

  rc = tokenize(az_span_create_from_str((char *)(size_t) "union ForwardUnion;"),
                &tl);
  ASSERT_EQ(0, rc);
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  free_cst_node_list(&list);
  free_token_list(tl);
  tl = NULL;

  /* 11. Comments and Macros */
  rc = tokenize(az_span_create_from_str((char *)(size_t) "/* comment */"), &tl);
  ASSERT_EQ(0, rc);
  g_cdd_fail_cst_list_add = 1;
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT(rc != CDD_C_SUCCESS);
  g_cdd_fail_cst_list_add = 0;
  free_cst_node_list(&list);
  free_token_list(tl);
  tl = NULL;

  rc = tokenize(
      az_span_create_from_str((char *)(size_t) "#define FOO 1\nint x;\n"), &tl);
  ASSERT_EQ(0, rc);
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  free_cst_node_list(&list);
  free_token_list(tl);
  tl = NULL;

  rc = tokenize(az_span_create_from_str((char *)(size_t) "#define FOO 1\n"),
                &tl);
  ASSERT_EQ(0, rc);
  g_cdd_fail_cst_list_add = 1;
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT(rc != CDD_C_SUCCESS);
  g_cdd_fail_cst_list_add = 0;
  free_cst_node_list(&list);
  free_token_list(tl);
  tl = NULL;

  /* 12. Expression braces and statements */
  rc =
      tokenize(az_span_create_from_str((char *)(size_t) "int x = { 1 };"), &tl);
  ASSERT_EQ(0, rc);
  g_cdd_fail_is_expression_brace = 1;
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT(rc != CDD_C_SUCCESS);
  g_cdd_fail_is_expression_brace = 0;
  free_cst_node_list(&list);

  g_cdd_fail_skip_ws_back = 1;
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT(rc != CDD_C_SUCCESS);
  g_cdd_fail_skip_ws_back = 0;
  free_cst_node_list(&list);

  g_cdd_fail_consume_balanced_braces = 1;
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT(rc != CDD_C_SUCCESS);
  g_cdd_fail_consume_balanced_braces = 0;
  free_cst_node_list(&list);

  g_cdd_fail_cst_list_add = 1;
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT(rc != CDD_C_SUCCESS);
  g_cdd_fail_cst_list_add = 0;
  free_cst_node_list(&list);
  free_token_list(tl);
  tl = NULL;

  /* Expression brace preceded by paren: bpk == TOKEN_KEYWORD_IF */
  rc = tokenize(az_span_create_from_str((char *)(size_t) "if (x) { y = 1; }"),
                &tl);
  ASSERT_EQ(0, rc);
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  free_cst_node_list(&list);
  free_token_list(tl);
  tl = NULL;

  /* Expression brace preceded by paren: not if */
  rc = tokenize(
      az_span_create_from_str((char *)(size_t) "int x = ((struct S){ 1 });"),
      &tl);
  ASSERT_EQ(0, rc);
  g_cdd_fail_skip_ws_back =
      2; /* Fails second skip_ws_back inside is_expression_brace */
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT(rc != CDD_C_SUCCESS);
  g_cdd_fail_skip_ws_back = 0;
  free_cst_node_list(&list);
  free_token_list(tl);
  tl = NULL;

  /* Statement with cast: (struct S *)p; */
  rc = tokenize(
      az_span_create_from_str((char *)(size_t) "int y; (struct S *)p;"), &tl);
  ASSERT_EQ(0, rc);
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  free_cst_node_list(&list);
  free_token_list(tl);
  tl = NULL;

  rc = tokenize(az_span_create_from_str((char *)(size_t) "int y; struct S *p;"),
                &tl);
  ASSERT_EQ(0, rc);
  g_cdd_fail_skip_ws_back = 1;
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT(rc != CDD_C_SUCCESS);
  g_cdd_fail_skip_ws_back = 0;
  free_cst_node_list(&list);
  free_token_list(tl);
  tl = NULL;

  rc = tokenize(
      az_span_create_from_str((char *)(size_t) "int y; _Generic(1, int: 1);"),
      &tl);
  ASSERT_EQ(0, rc);
  g_cdd_fail_token_matches_string = 1;
  memset(&list, 0, sizeof(list));
  rc = parse_tokens(tl, &list);
  ASSERT(rc != CDD_C_SUCCESS);
  g_cdd_fail_token_matches_string = 0;
  free_cst_node_list(&list);
  free_token_list(tl);
  tl = NULL;

  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !TEST_CST_PARSER_COVERAGE_H */
