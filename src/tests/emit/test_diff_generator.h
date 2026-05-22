/**
 * @file test_diff_generator.h
 * @brief Unit tests for CST diff generation.
 */

#ifndef TEST_DIFF_GENERATOR_H
#define TEST_DIFF_GENERATOR_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "functions/emit/diff_generator.h"
#include "functions/emit/patcher.h"
#include "functions/parse/tokenizer.h"
/* clang-format on */

/**
 * @brief Tests basic diff generation
 * @return TEST
 */
TEST test_diff_generation_basic(void) {
  const char *src = "int a = 1;";
  struct TokenList *tokens = NULL;
  struct PatchList patch_list;
  char *diff = NULL;
  int rc;
  az_span span;

  patch_list_init(&patch_list);
  span = az_span_create((uint8_t *)src, strlen(src));
  rc = tokenize(span, &tokens);
  ASSERT_EQ(0, rc);

  ASSERT_EQ(EINVAL, patch_list_generate_diff(NULL, &patch_list, "a.c", &diff));
  ASSERT_EQ(EINVAL, patch_list_generate_diff(tokens, NULL, "a.c", &diff));
  ASSERT_EQ(EINVAL, patch_list_generate_diff(tokens, &patch_list, NULL, &diff));
  ASSERT_EQ(EINVAL, patch_list_generate_diff(tokens, &patch_list, "a.c", NULL));

  rc = patch_list_generate_diff(tokens, &patch_list, "a.c", &diff);
  ASSERT_EQ(0, rc);
  ASSERT(diff != NULL);
  ASSERT_EQ('\0', diff[0]);
  free(diff);

  {
    char *text = (char *)malloc(5);
    strcpy(text, "void");
    ASSERT_EQ(0, patch_list_add(&patch_list, 0, 1, text));
  }

  rc = patch_list_generate_diff(tokens, &patch_list, "a.c", &diff);
  ASSERT_EQ(0, rc);
  ASSERT(strstr(diff, "--- a/a.c"));
  ASSERT(strstr(diff, "+++ b/a.c"));
  ASSERT(strstr(diff, "@@ -patch 0 @@"));
  ASSERT(strstr(diff, "-int"));
  ASSERT(strstr(diff, "+void"));

  free(diff);
  patch_list_free(&patch_list);
  free_token_list(tokens);
  PASS();
}

/**
 * @brief Suite for diff generator
 */
SUITE(diff_generator_suite) { RUN_TEST(test_diff_generation_basic); }

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_DIFF_GENERATOR_H */
