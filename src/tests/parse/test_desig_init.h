/**
 * @file test_desig_init.h
 * @brief Unit tests for designated initializer analysis.
 */

#ifndef TEST_DESIG_INIT_H
#define TEST_DESIG_INIT_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include <greatest.h>
#include <string.h>

#include "functions/parse/desig_init.h"
#include "functions/parse/tokenizer.h"

extern C_CDD_EXPORT int g_cdd_fail_alloc;
/* clang-format on */

/**
 * @brief Tests basic functionality of designated initializer scanning.
 *
 * @return The result of the test.
 */

TEST test_scan_for_designated_initializers_basic(void) {
  struct TokenList *tokens = NULL;
  struct DesigInitList list;
  const char *src =
      "struct S my_s = { . x = 10, .y = \"hello\", .z = {.inner = 5} };";
  const char *long_src =
      "struct S my_s = { .a=1, .b=2, .c=3, .d=4, .e=5, .f=6, "
      "{ { { { { { { { { { { { { { { { { { { { .x = 10 } } } } } } } } } } } } "
      "} } } } } } } } };";

  ASSERT_EQ(0, tokenize(az_span_create_from_str((char *)src), &tokens));
  (void)desig_init_list_init(&list);
  ASSERT_EQ(0, scan_for_designated_initializers(tokens, &list));
  ASSERT_EQ(3, list.count);
  desig_init_list_free(&list);
  free_token_list(tokens);

  tokens = NULL;
  ASSERT_EQ(0, tokenize(az_span_create_from_str((char *)long_src), &tokens));
  (void)desig_init_list_init(&list);
  ASSERT_EQ(0, scan_for_designated_initializers(tokens, &list));
  desig_init_list_free(&list);
  free_token_list(tokens);

  g_fail_io_after = -1;
  PASS();
}

TEST test_scan_for_designated_initializers_errors(void) {
  struct TokenList *tl = NULL;
  struct DesigInitList list;
  (void)desig_init_list_init(&list);
  ASSERT_EQ(0, tokenize(az_span_create_from_str("int x = 1;"), &tl));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            scan_for_designated_initializers(NULL, &list));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            scan_for_designated_initializers(tl, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, desig_init_list_init(NULL));

  desig_init_list_free(&list);
  free_token_list(tl);
  g_fail_io_after = -1;
  PASS();
}

TEST test_scan_for_designated_initializers_oom(void) {
  struct TokenList *tokens = NULL;
  struct DesigInitList list;
  const char *src =
      "struct S my_s = { .x = 10, .y = \"hello\", .z = {.inner = 5} };";
  int i;
  int res;

  ASSERT_EQ(0, tokenize(az_span_create_from_str((char *)src), &tokens));

  for (i = 1; i < 20; ++i) {
    (void)desig_init_list_init(&list);
    g_cdd_fail_alloc = i;
    res = scan_for_designated_initializers(tokens, &list);
    g_cdd_fail_alloc = 0;

    if (res != 0) {
      ASSERT_EQ(ENOMEM, res);
    }
    desig_init_list_free(&list);
  }

  free_token_list(tokens);
  PASS();
}

TEST test_scan_for_designated_initializers_oom_long(void) {
  struct TokenList *tokens = NULL;
  struct DesigInitList list;
  const char *long_src =
      "struct S my_s = { .a=1, .b=2, .c=3, .d=4, .e=5, .f=6, "
      "{ { { { { { { { { { { { { { { { { { { { .x = 10 } } } } } } } } } } } } "
      "} } } } } } } } };";
  int i;
  int res;

  ASSERT_EQ(0, tokenize(az_span_create_from_str((char *)long_src), &tokens));

  for (i = 1; i < 30; ++i) {
    (void)desig_init_list_init(&list);
    g_cdd_fail_alloc = i;
    res = scan_for_designated_initializers(tokens, &list);
    g_cdd_fail_alloc = 0;

    if (res != 0) {
      ASSERT_EQ(ENOMEM, res);
    }
    desig_init_list_free(&list);
  }

  free_token_list(tokens);
  PASS();
}

TEST test_scan_for_designated_initializers_edge_cases(void) {
  struct TokenList tokens;
  struct Token t[10];
  memset(t, 0, sizeof(t));
  t[2].start = (const uint8_t *)"a";
  t[2].length = 1;
  t[3].start = (const uint8_t *)"=";
  t[3].length = 1;
  struct DesigInitList list;

  (void)desig_init_list_init(&list);
  tokens.tokens = t;
  tokens.size = 0;
  tokens.capacity = 10;

  /* Case: RBRACE when brace_depth == 0 */
  t[0].kind = TOKEN_RBRACE;
  tokens.size = 1;
  ASSERT_EQ(0, scan_for_designated_initializers(&tokens, &list));

  /* Case: DOT followed by EOF */
  t[0].kind = TOKEN_LBRACE;
  t[1].kind = TOKEN_DOT;
  tokens.size = 2;
  ASSERT_EQ(0, scan_for_designated_initializers(&tokens, &list));

  /* Case: DOT followed by non-identifier */
  t[2].kind = TOKEN_PLUS;
  tokens.size = 3;
  ASSERT_EQ(0, scan_for_designated_initializers(&tokens, &list));

  /* Case: DOT followed by identifier then EOF */
  t[2].kind = TOKEN_IDENTIFIER;
  tokens.size = 3;
  ASSERT_EQ(0, scan_for_designated_initializers(&tokens, &list));

  /* Case: DOT followed by identifier then non-assign */
  t[3].kind = TOKEN_PLUS;
  tokens.size = 4;
  ASSERT_EQ(0, scan_for_designated_initializers(&tokens, &list));

  /* Case: DOT followed by identifier then assign then EOF */
  t[3].kind = TOKEN_ASSIGN;
  tokens.size = 4;
  ASSERT_EQ(0, scan_for_designated_initializers(&tokens, &list));

  /* Case: Comma inside nested braces */
  memset(t, 0, sizeof(t));
  t[0].kind = TOKEN_LBRACE;
  t[1].kind = TOKEN_DOT;
  t[2].kind = TOKEN_IDENTIFIER;
  t[2].start = (const uint8_t *)"x";
  t[2].length = 1;
  t[3].kind = TOKEN_ASSIGN;
  t[4].kind = TOKEN_LBRACE;
  t[5].kind = TOKEN_NUMBER_LITERAL;
  t[6].kind = TOKEN_COMMA;
  t[7].kind = TOKEN_NUMBER_LITERAL;
  t[8].kind = TOKEN_RBRACE;
  t[9].kind = TOKEN_RBRACE;
  tokens.size = 10;
  ASSERT_EQ(0, scan_for_designated_initializers(&tokens, &list));

  desig_init_list_free(&list);
  PASS();
}

TEST test_scan_for_designated_initializers_oom_empty(void) {
  struct TokenList tokens;
  struct Token t[10];
  struct DesigInitList list;
  int i;
  int res;

  memset(t, 0, sizeof(t));
  t[0].kind = TOKEN_LBRACE;
  t[1].kind = TOKEN_DOT;
  t[2].kind = TOKEN_IDENTIFIER;
  t[2].start = (const uint8_t *)"a";
  t[2].length = 1;
  t[3].kind = TOKEN_ASSIGN;
  t[3].start = (const uint8_t *)"=";
  t[3].length = 1;

  tokens.tokens = t;
  tokens.size = 4;
  tokens.capacity = 10;

  for (i = 1; i < 10; ++i) {
    (void)desig_init_list_init(&list);
    g_cdd_fail_alloc = i;
    res = scan_for_designated_initializers(&tokens, &list);
    g_cdd_fail_alloc = 0;

    if (res != 0) {
      ASSERT_EQ(ENOMEM, res);
    }
    desig_init_list_free(&list);
  }

  PASS();
}

TEST test_desig_init_list_free_nulls(void) {
  struct DesigInitList list;
  (void)desig_init_list_init(&list);
  list.count = 2;
  list.capacity = 2;
  list.sites = (struct DesigInitSite *)calloc(2, sizeof(struct DesigInitSite));

  /* leave sites[0] fields NULL */
  list.sites[1].field_name = (char *)malloc(2);
  strcpy(list.sites[1].field_name, "a");
  list.sites[1].value_expr = (char *)malloc(2);
  strcpy(list.sites[1].value_expr, "1");

  desig_init_list_free(&list);
  desig_init_list_free(NULL);
  PASS();
}

SUITE(desig_init_suite) {
  RUN_TEST(test_scan_for_designated_initializers_basic);
  RUN_TEST(test_scan_for_designated_initializers_errors);
  RUN_TEST(test_scan_for_designated_initializers_oom);
  RUN_TEST(test_scan_for_designated_initializers_oom_long);
  RUN_TEST(test_scan_for_designated_initializers_edge_cases);
  RUN_TEST(test_scan_for_designated_initializers_oom_empty);
  RUN_TEST(test_desig_init_list_free_nulls);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_DESIG_INIT_H */
