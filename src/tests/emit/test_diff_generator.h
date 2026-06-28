extern C_CDD_EXPORT int g_fail_io_after;
extern C_CDD_EXPORT int g_io_calls;
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
#include "c_cdd_export.h"
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
  char *diff2 = NULL;
  char huge_str[5000];
  struct PatchList patch_list2;
  char *diff3 = NULL;
  struct TokenList *tokens2 = NULL;
  struct PatchList patch_list3;
#ifdef CDD_BUILD_TESTS
#endif
  az_span span;

  patch_list_init(&patch_list);
  span = az_span_create((uint8_t *)src, strlen(src));
  rc = tokenize(span, &tokens);
  ASSERT_EQ(0, rc);

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            patch_list_generate_diff(NULL, &patch_list, "a.c", &diff));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            patch_list_generate_diff(tokens, NULL, "a.c", &diff));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            patch_list_generate_diff(tokens, &patch_list, NULL, &diff));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            patch_list_generate_diff(tokens, &patch_list, "a.c", NULL));

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

#ifdef CDD_BUILD_TESTS
  {
    extern C_CDD_EXPORT int g_cdd_fail_alloc;
    g_cdd_fail_alloc = 5555;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              patch_list_generate_diff(tokens, &patch_list, "file.c", &diff2));
    g_cdd_fail_alloc = 0;
  }
#endif

  memset(huge_str, 'x', 4999);
  huge_str[4999] = '\0';

  patch_list_init(&patch_list2);
  patch_list_add(&patch_list2, 0, 1, strdup(huge_str));

  /* Trigger realloc success */
  ASSERT_EQ(0, patch_list_generate_diff(tokens, &patch_list2, "a.c", &diff3));
  ASSERT(diff3 != NULL);
  free(diff3);

#ifdef CDD_BUILD_TESTS
  {
    extern C_CDD_EXPORT int g_cdd_fail_alloc;

    /* Trigger realloc failure */
    diff3 = NULL;
    g_cdd_fail_alloc = 7777;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              patch_list_generate_diff(tokens, &patch_list2, "a.c", &diff3));
    g_cdd_fail_alloc = 0;

    /* Trigger realloc failure in the other branch */
    /* For the other branch, we need a huge token! */
    tokenize(az_span_create_from_str(huge_str), &tokens2);

    patch_list_init(&patch_list3);
    patch_list_add(&patch_list3, 0, 1, strdup("small"));

    diff3 = NULL;
    g_cdd_fail_alloc = 6666;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              patch_list_generate_diff(tokens2, &patch_list3, "a.c", &diff3));
    g_cdd_fail_alloc = 0;

    /* Trigger realloc success in the first branch */
    diff3 = NULL;
    ASSERT_EQ(0,
              patch_list_generate_diff(tokens2, &patch_list3, "a.c", &diff3));
    ASSERT(diff3 != NULL);
    free(diff3);

    patch_list_free(&patch_list3);
    free_token_list(tokens2);
  }
#endif

  patch_list_free(&patch_list2);
  free_token_list(tokens);
  g_fail_io_after = -1;
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
