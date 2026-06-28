extern C_CDD_EXPORT int g_fail_io_after;
extern C_CDD_EXPORT int g_io_calls;
/**
 * @file test_decl_hoist.h
 * @brief Unit tests for declaration hoisting analysis.
 */

#ifndef TEST_DECL_HOIST_H
#define TEST_DECL_HOIST_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include <greatest.h>
#include <string.h>

#include "functions/parse/decl_hoist.h"
#include "functions/parse/tokenizer.h"
/* clang-format on */

/**
 * @brief Tests basic functionality of scanning for mixed declarations.
 *
 * @return The result of the test.
 */
TEST test_scan_for_mixed_declarations_basic(void) {
  struct TokenList *tokens = NULL;
  struct HoistSiteList list;
  const char *src =
      "void func() {\n      int a = 1;\n  a = 2;\n  int b = 3;\n}\n";

  ASSERT_EQ(0, tokenize(az_span_create_from_str((char *)src), &tokens));
  hoist_site_list_init(&list);

  ASSERT_EQ(0, scan_for_mixed_declarations(tokens, &list));

  /* Should find 1 mixed declaration: `int b = 3;` */
  ASSERT_EQ(1, list.count);

  /* It should point to `int` and end at `;` */
  ASSERT_EQ(TOKEN_KEYWORD_INT,
            tokens->tokens[list.sites[0].start_token_idx].kind);
  ASSERT_EQ(TOKEN_SEMICOLON,
            tokens->tokens[list.sites[0].end_token_idx - 1].kind);
  /* Target block should be the first `{` */
  ASSERT_EQ(TOKEN_LBRACE, tokens->tokens[list.sites[0].target_block_idx].kind);

  hoist_site_list_free(&list);
  free_token_list(tokens);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Tests error handling of scanning for mixed declarations.
 *
 * @return The result of the test.
 */
TEST test_scan_for_mixed_declarations_errors(void) {
  struct TokenList *tl = setup_tokens("int a = 1;");
  struct HoistSiteList list;
  hoist_site_list_init(&list);

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            scan_for_mixed_declarations(NULL, &list));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            scan_for_mixed_declarations(tl, NULL));

  hoist_site_list_free(&list);
  free_token_list(tl);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Declaration hoist test suite.
 */

TEST test_decl_hoist_edges(void) {
  struct HoistSiteList list = {0};
  struct TokenList *tl = NULL;
  hoist_site_list_init(NULL);
  hoist_site_list_free(NULL);

  hoist_site_list_free(&list);

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            scan_for_mixed_declarations(NULL, &list));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            scan_for_mixed_declarations(tl, NULL));

  tokenize(az_span_create_from_str("int main() { int a; return 0; }"), &tl);
  list.capacity = 1;
  list.count = 1;
  list.sites = calloc(1, sizeof(struct HoistSite));

  hoist_site_list_free(&list);

  free_token_list(tl);
  g_fail_io_after = -1;
  PASS();
}

SUITE(decl_hoist_suite) {
  RUN_TEST(test_scan_for_mixed_declarations_basic);
  RUN_TEST(test_scan_for_mixed_declarations_errors);
  RUN_TEST(test_decl_hoist_edges);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_DECL_HOIST_H */
