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

/* Moved extern declarations for C89 compliance */
/** @brief Test definition g_cdd_fail_alloc_decl_hoist. */
extern C_CDD_EXPORT int g_cdd_fail_alloc_decl_hoist;
/** @brief Test definition g_is_basic_type_keyword_fail. */
extern C_CDD_EXPORT int g_is_basic_type_keyword_fail;

/**
 * @brief Tests basic functionality of scanning for mixed declarations.
 *
 * @return The result of the test.
 */
TEST test_scan_for_mixed_declarations_basic(void) {
  struct TokenList *tokens = NULL;
  struct HoistSiteList list;
  const char *src = cdd_test_mut_str("void func() {\n      int a = 1;\n  a = "
                                     "2;\n  int b = 3;\n}\n");

  ASSERT_EQ(0, tokenize(az_span_create_from_str(src), &tokens));

  ASSERT_EQ(CDD_C_SUCCESS, hoist_site_list_init(&list));
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

#ifdef CDD_BUILD_TESTS
  g_is_basic_type_keyword_fail = 1;
  ASSERT_NEQ(0, scan_for_mixed_declarations(tokens, &list));
  g_is_basic_type_keyword_fail = 0;

  g_is_basic_type_keyword_fail = 2;
  ASSERT_NEQ(0, scan_for_mixed_declarations(tokens, &list));
  g_is_basic_type_keyword_fail = 0;
#endif

  hoist_site_list_free(&list);
  free_token_list(tokens);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Tests error handling of scanning for mixed declarations.
 *
 * @return The result of the test.
 * @return Test or helper result.
 */
TEST test_scan_for_mixed_declarations_errors(void) {
  struct TokenList *tl = NULL;
  struct HoistSiteList list;
  tokenize(az_span_create_from_str("int a = 1;"), &tl);

  ASSERT_EQ(CDD_C_SUCCESS, hoist_site_list_init(&list));
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
 * @brief Helper to tokenize source and scan for mixed declarations.
 * @param[in] src Input C source string.
 * @param[in,out] list Output hoist site list.
 * @return Test or helper result.
 */
static cdd_c_error_t check_hoist(const char *src, struct HoistSiteList *list) {
  struct TokenList *tl = NULL;
  int rc = 0;
  tokenize(az_span_create_from_str(src), &tl);
  rc = scan_for_mixed_declarations(tl, list);
  free_token_list(tl);
  return rc;
}

/**
 * @brief Unit test test_decl_hoist_branches.
 * @return Greatest test result.
 */
TEST test_decl_hoist_branches(void) {
  struct HoistSiteList list;

  /* RBRACE with depth == 0 */
  ASSERT_EQ(CDD_C_SUCCESS, hoist_site_list_init(&list));
  ASSERT_EQ(0, check_hoist("}", &list));

  /* EOF immediately */
  ASSERT_EQ(0, check_hoist("   ", &list));

  /* Test basic type */
  ASSERT_EQ(0, check_hoist("{ f(); int a; }", &list));
  ASSERT_EQ(1, list.count);
  hoist_site_list_free(&list);

  /* Test struct */
  ASSERT_EQ(0, check_hoist("{ f(); struct A a; }", &list));
  ASSERT_EQ(1, list.count);
  hoist_site_list_free(&list);

  /* Test union */
  ASSERT_EQ(0, check_hoist("{ f(); union A a; }", &list));
  ASSERT_EQ(1, list.count);
  hoist_site_list_free(&list);

  /* Test enum */
  ASSERT_EQ(0, check_hoist("{ f(); enum A a; }", &list));
  ASSERT_EQ(1, list.count);
  hoist_site_list_free(&list);

  /* Test typedef (identifier followed by identifier) */
  ASSERT_EQ(0, check_hoist("{ f(); MyType a; }", &list));
  ASSERT_EQ(1, list.count);
  hoist_site_list_free(&list);

  /* Test typedef (identifier followed by star) */
  ASSERT_EQ(0, check_hoist("{ f(); MyType *a; }", &list));
  ASSERT_EQ(1, list.count);
  hoist_site_list_free(&list);

  /* Test typedef with EOF during lookahead */
  ASSERT_EQ(0, check_hoist("{ f(); MyType", &list));
  ASSERT_EQ(0, list.count);

  /* Scan to end of statement hitting LBRACE / RBRACE */
  ASSERT_EQ(0, check_hoist("{ f(); int a {", &list));
  ASSERT_EQ(0, list.count);

  ASSERT_EQ(0, check_hoist("{ f(); int a }", &list));
  ASSERT_EQ(0, list.count);

  /* Statement that does not start with identifier */
  ASSERT_EQ(0, check_hoist("{ *p = 1; return; }", &list));
  ASSERT_EQ(0, list.count);

  hoist_site_list_free(&list);

#ifdef CDD_BUILD_TESTS
  /* OOM test */
  g_cdd_fail_alloc_decl_hoist = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, check_hoist("{ f(); int a; }", &list));
  g_cdd_fail_alloc_decl_hoist = 0;
  hoist_site_list_free(&list);

  /* OOM test > 1 */
  g_cdd_fail_alloc_decl_hoist = 2;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            check_hoist("{ f(); int a; int b; int c; int d; int e; }", &list));
  g_cdd_fail_alloc_decl_hoist = 0;
#endif

  hoist_site_list_free(&list);
  PASS();
}

/**
 * @brief Unit test test_decl_hoist_edges.
 * @return Greatest test result.
 */
TEST test_decl_hoist_edges(void) {
  struct HoistSiteList list = {0};
  struct TokenList *tl = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, hoist_site_list_init(NULL));
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

/**
 * @brief Executes the decl_hoist_suite test suite.
 */
SUITE(decl_hoist_suite) {
  RUN_TEST(test_scan_for_mixed_declarations_basic);
  RUN_TEST(test_scan_for_mixed_declarations_errors);
  RUN_TEST(test_decl_hoist_branches);
  RUN_TEST(test_decl_hoist_edges);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_DECL_HOIST_H */
