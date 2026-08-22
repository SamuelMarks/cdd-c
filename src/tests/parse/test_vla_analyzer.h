/**
 * @file test_vla_analyzer.h
 * @brief Unit tests for VLA analyzer.
 */

#ifndef TEST_VLA_ANALYZER_H
#define TEST_VLA_ANALYZER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include <greatest.h>
#include <string.h>

#include "functions/parse/tokenizer.h"
#include "functions/parse/vla_analyzer.h"

  /* extern int g_io_calls; (moved to global) */
  /* extern int g_fail_io_after; (moved to global) */
extern cdd_c_error_t is_basic_type_keyword_test(enum TokenKind k, int *out_is_basic);

/* clang-format on */

/* Moved extern declarations for C89 compliance */
extern int g_io_calls;
extern int g_fail_io_after;

/**
 * @brief Tests basic functionality of VLA scanning.
 *
 * @return The result of the test.
 */
TEST test_scan_for_vlas_basic(void) {
  struct TokenList *tokens = NULL;
  struct VLASiteList list;
  const char *src = "void func(int n) {\n"
                    "  int normal[10];\n"
                    "  char vla[n];\n"
                    "  int multi[n], b;\n"
                    "  struct S s_vla[n * 2];\n"
                    "  union U u_vla[n];\n"
                    "  enum E e_vla[n];\n"
                    "  int const c_vla[n];\n"
                    "}\n";

  ASSERT_EQ(0, tokenize(az_span_create_from_str((char *)src), &tokens));
  (void)vla_site_list_init(&list);

  ASSERT_EQ(0, scan_for_vlas(tokens, &list));

  ASSERT_EQ(5, list.count);

  ASSERT_STR_EQ("char", list.sites[0].type_str);
  ASSERT_STR_EQ("vla", list.sites[0].var_name);
  ASSERT_STR_EQ("n", list.sites[0].size_expr);

  ASSERT_STR_EQ("struct S", list.sites[1].type_str);
  ASSERT_STR_EQ("s_vla", list.sites[1].var_name);
  ASSERT_STR_EQ("n * 2", list.sites[1].size_expr);

  ASSERT_STR_EQ("union U", list.sites[2].type_str);
  ASSERT_STR_EQ("u_vla", list.sites[2].var_name);
  ASSERT_STR_EQ("n", list.sites[2].size_expr);

  ASSERT_STR_EQ("enum E", list.sites[3].type_str);
  ASSERT_STR_EQ("e_vla", list.sites[3].var_name);
  ASSERT_STR_EQ("n", list.sites[3].size_expr);

  ASSERT_STR_EQ("int const", list.sites[4].type_str);
  ASSERT_STR_EQ("c_vla", list.sites[4].var_name);
  ASSERT_STR_EQ("n", list.sites[4].size_expr);

  vla_site_list_free(&list);
  free_token_list(tokens);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Tests error handling of VLA scanning.
 *
 * @return The result of the test.
 */
TEST test_scan_for_vlas_errors(void) {
  struct TokenList *tl = NULL;
  struct VLASiteList list;
  (void)vla_site_list_init(&list);
  ASSERT_EQ(0, tokenize(az_span_create_from_str("int x[n];"), &tl));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, scan_for_vlas(NULL, &list));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, scan_for_vlas(tl, NULL));

  free_token_list(tl);
  g_fail_io_after = -1;
  PASS();
}

TEST test_vla_site_list_nulls(void) {
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, vla_site_list_init(NULL));
  vla_site_list_free(NULL); /* Should not crash */
  PASS();
}

TEST test_is_basic_type_keyword(void) {
  int is_basic;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            is_basic_type_keyword_test(TOKEN_KEYWORD_INT, NULL));
  ASSERT_EQ(CDD_C_SUCCESS,
            is_basic_type_keyword_test(TOKEN_KEYWORD_INT, &is_basic));
  ASSERT_EQ(1, is_basic);
  ASSERT_EQ(CDD_C_SUCCESS,
            is_basic_type_keyword_test(TOKEN_IDENTIFIER, &is_basic));
  ASSERT_EQ(0, is_basic);
  PASS();
}

TEST test_scan_for_vlas_oom(void) {
  struct TokenList *tokens = NULL;
  struct VLASiteList list;
  const char *src = "void func(int n) { int normal[10]; char vla[n]; struct S "
                    "s_vla[n * 2]; }";
  int i;
  int res;

  ASSERT_EQ(0, tokenize(az_span_create_from_str((char *)src), &tokens));

  for (i = 0; i < 25; ++i) {
    (void)vla_site_list_init(&list);
    g_io_calls = 0;
    g_fail_io_after = i;
    res = scan_for_vlas(tokens, &list);
    if (res != 0) {
      ASSERT_EQ(CDD_C_ERROR_MEMORY, res);
    }
    g_fail_io_after = -1;
    vla_site_list_free(&list);
  }

  free_token_list(tokens);
  PASS();
}

TEST test_scan_for_vlas_edge_cases(void) {
  struct TokenList *tokens = NULL;
  struct VLASiteList list;

  /* Empty array size */
  const char *src1 = "int arr[]; int arr2[ ];";
  ASSERT_EQ(0, tokenize(az_span_create_from_str((char *)src1), &tokens));
  (void)vla_site_list_init(&list);
  ASSERT_EQ(0, scan_for_vlas(tokens, &list));
  ASSERT_EQ(0, list.count); /* Not a VLA */
  vla_site_list_free(&list);
  free_token_list(tokens);
  tokens = NULL;

  /* Type alias / Typedef name as type: e.g. MyType arr[n]; or MyType *arr[n];
   */
  const char *src2 = "MyType arr[n]; MyType *arr2[n]; MyType   arr3[n];";
  ASSERT_EQ(0, tokenize(az_span_create_from_str((char *)src2), &tokens));
  (void)vla_site_list_init(&list);
  ASSERT_EQ(0, scan_for_vlas(tokens, &list));
  ASSERT_EQ(3, list.count);
  vla_site_list_free(&list);
  free_token_list(tokens);
  tokens = NULL;

  /* Missing variable name (syntax error but we should not crash) */
  const char *src3 = "int [n];";
  ASSERT_EQ(0, tokenize(az_span_create_from_str((char *)src3), &tokens));
  (void)vla_site_list_init(&list);
  ASSERT_EQ(0, scan_for_vlas(tokens, &list));
  vla_site_list_free(&list);
  free_token_list(tokens);
  tokens = NULL;

  /* Missing semicolon */
  const char *src4 = "int arr[n]";
  ASSERT_EQ(0, tokenize(az_span_create_from_str((char *)src4), &tokens));
  (void)vla_site_list_init(&list);
  ASSERT_EQ(0, scan_for_vlas(tokens, &list));
  vla_site_list_free(&list);
  free_token_list(tokens);
  tokens = NULL;

  /* Missing array bracket closing */
  const char *src5 = "int arr[n";
  ASSERT_EQ(0, tokenize(az_span_create_from_str((char *)src5), &tokens));
  (void)vla_site_list_init(&list);
  ASSERT_EQ(0, scan_for_vlas(tokens, &list));
  vla_site_list_free(&list);
  free_token_list(tokens);
  tokens = NULL;

  /* Type modifiers with whitespace */
  const char *src6 = "const  int  arr [ n ] ;";
  ASSERT_EQ(0, tokenize(az_span_create_from_str((char *)src6), &tokens));
  (void)vla_site_list_init(&list);
  ASSERT_EQ(0, scan_for_vlas(tokens, &list));
  ASSERT_EQ(1, list.count);
  vla_site_list_free(&list);
  free_token_list(tokens);
  tokens = NULL;

  /* Trigger capacity growth (> 4 items) */
  const char *src7 = "void f(int n) { int a[n]; int b[n]; int c[n]; int d[n]; "
                     "int e[n]; int f[n]; }";
  ASSERT_EQ(0, tokenize(az_span_create_from_str((char *)src7), &tokens));
  (void)vla_site_list_init(&list);
  ASSERT_EQ(0, scan_for_vlas(tokens, &list));
  ASSERT_EQ(6, list.count);
  vla_site_list_free(&list);
  free_token_list(tokens);
  tokens = NULL;

  /* Abrupt endings to hit i < tokens->size branches */
  const char *abrupt_cases[] = {"struct", "union",    "enum",     "struct S",
                                "const",  "int",      "int a",    "int a   ",
                                "int *",  "int a[",   "int a[n",  "int a[10",
                                "MyType", "MyType *", "int a[n]", "int a[n]  "};
  size_t i;
  for (i = 0; i < sizeof(abrupt_cases) / sizeof(abrupt_cases[0]); i++) {
    ASSERT_EQ(
        0, tokenize(az_span_create_from_str((char *)abrupt_cases[i]), &tokens));
    (void)vla_site_list_init(&list);
    ASSERT_EQ(0, scan_for_vlas(tokens, &list));
    vla_site_list_free(&list);
    free_token_list(tokens);
    tokens = NULL;
  }

  /* Mock a token list WITHOUT an EOF token to hit out-of-bounds branches */
  struct TokenList mock_tokens = {0};
  struct Token mock_toks[4];

  /* Test 1: Ends with WHITESPACE */
  mock_toks[0].kind = TOKEN_KEYWORD_INT;
  mock_toks[1].kind = TOKEN_IDENTIFIER;
  mock_toks[1].start = (const uint8_t *)"a";
  mock_toks[1].length = 1;
  mock_toks[2].kind = TOKEN_WHITESPACE;
  mock_tokens.tokens = mock_toks;
  mock_tokens.size = 3;
  mock_tokens.capacity = 4;
  (void)vla_site_list_init(&list);
  scan_for_vlas(&mock_tokens, &list);
  vla_site_list_free(&list);

  /* Test 2: Ends exactly at identifier */
  mock_tokens.size = 2; /* INT, IDENTIFIER */
  (void)vla_site_list_init(&list);
  scan_for_vlas(&mock_tokens, &list);
  vla_site_list_free(&list);

  /* Test 3: Ends with LBRACKET */
  mock_toks[2].kind = TOKEN_LBRACKET;
  mock_tokens.size = 3;
  (void)vla_site_list_init(&list);
  scan_for_vlas(&mock_tokens, &list);
  vla_site_list_free(&list);

  /* Test 4: Semicolon, LBRACE, RBRACE to hit line 209-211 */
  ASSERT_EQ(
      0, tokenize(az_span_create_from_str("int a[; int a[{ int a[}"), &tokens));
  (void)vla_site_list_init(&list);
  scan_for_vlas(tokens, &list);
  vla_site_list_free(&list);
  free_token_list(tokens);
  tokens = NULL;

  PASS();
}

/**
 * @brief VLA analyzer test suite.
 */
SUITE(vla_analyzer_suite) {

  RUN_TEST(test_scan_for_vlas_basic);
  RUN_TEST(test_scan_for_vlas_errors);
  RUN_TEST(test_vla_site_list_nulls);
  RUN_TEST(test_is_basic_type_keyword);
  RUN_TEST(test_scan_for_vlas_oom);
  RUN_TEST(test_scan_for_vlas_edge_cases);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_VLA_ANALYZER_H */
