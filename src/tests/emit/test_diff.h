/**
 * @file test_diff.h
 * @brief Unit tests for diff generation.
 */

#ifndef TEST_DIFF_H
#define TEST_DIFF_H

#include <greatest.h>
#include <string.h>

#include "functions/emit/diff.h"
#include "functions/parse/tokenizer.h"

TEST test_patch_list_to_diff_basic(void) {
  struct PatchList list;
  struct TokenList *tokens = NULL;
  const char *src = "int main() {
  return 0;
}
";
  int res;
  char *diff_str = NULL;
  size_t tok_idx = 0;
  bool found = false;

  res = tokenize(az_span_create_from_str((char *)src), &tokens);
  ASSERT_EQ(0, res);

  res = patch_list_init(&list);
  ASSERT_EQ(0, res);

  /* Find token for '0' */
  for (tok_idx = 0; tok_idx < tokens->size; ++tok_idx) {
    if (tokens->tokens[tok_idx].kind == TOKEN_NUMBER_LITERAL &&
        tokens->tokens[tok_idx].length == 1 &&
        tokens->tokens[tok_idx].start[0] == '0') {
      found = true;
      break;
    }
  }
  ASSERT(found);

  /* Replace '0' with '1' */
  {
    char *text = (char *)malloc(2);
#if defined(_MSC_VER)
    strcpy_s(text, sizeof(text), "1");
#else
#if defined(_MSC_VER)
    strcpy_s(text, sizeof(text), "1");
#else
#if defined(_MSC_VER)
    strcpy_s(text, sizeof(text), "1");
#else
#if defined(_MSC_VER)
    strcpy_s(text, sizeof(text), "1");
#else
#if defined(_MSC_VER)
    strcpy_s(text, sizeof(text), "1");
#else
    strcpy(text, "1");
#endif
#endif
#endif
#endif
#endif
    res = patch_list_add(&list, tok_idx, tok_idx + 1, text);
  }
  ASSERT_EQ(0, res);

  res = patch_list_to_diff(&list, tokens, "main.c", &diff_str);
  ASSERT_EQ(0, res);
  ASSERT(diff_str != NULL);

  /* Verify diff string structure */
  ASSERT(strstr(diff_str, "--- main.c
") != NULL);
  ASSERT(strstr(diff_str, "+++ main.c
") != NULL);
  ASSERT(strstr(diff_str, "-  return 0;
") != NULL);
  ASSERT(strstr(diff_str, "+  return 1;
") != NULL);

  free(diff_str);
  patch_list_free(&list);
  free_token_list(tokens);
  PASS();
}

TEST test_patch_list_to_diff_empty(void) {
  struct PatchList list;
  struct TokenList *tokens = NULL;
  char *diff_str = NULL;

  patch_list_init(&list);
  tokenize(az_span_create_from_str(""), &tokens);

  ASSERT_EQ(0, patch_list_to_diff(&list, tokens, "empty.c", &diff_str));
  ASSERT_STR_EQ("", diff_str);

  free(diff_str);
  patch_list_free(&list);
  free_token_list(tokens);
  PASS();
}

SUITE(diff_suite) {
  RUN_TEST(test_patch_list_to_diff_basic);
  RUN_TEST(test_patch_list_to_diff_empty);
}

#endif /* TEST_DIFF_H */
