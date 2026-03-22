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
#include <greatest.h>
#include <string.h>

#include "functions/parse/decl_hoist.h"
#include "functions/parse/tokenizer.h"
/* clang-format on */

TEST test_scan_for_mixed_declarations_basic(void) {
  struct TokenList *tokens = NULL;
  struct HoistSiteList list;
  const char *src = ""
                    "void func() {
      int a = 1;
  a = 2;
  int b = 3;
}
";

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
PASS();
}

SUITE(decl_hoist_suite) { RUN_TEST(test_scan_for_mixed_declarations_basic); }

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_DECL_HOIST_H */
