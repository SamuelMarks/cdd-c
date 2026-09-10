/**
 * @file test_diff.h
 * @brief Unit tests for diff generation.
 */

#ifndef TEST_DIFF_H
#define TEST_DIFF_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include <greatest.h>
#include <string.h>

#include "c_cdd/memory.h"
#include "functions/emit/diff.h"
#include "functions/parse/tokenizer.h"
/* clang-format on */

#ifdef CDD_BUILD_TESTS
extern C_CDD_EXPORT int g_cdd_alloc_fail;
extern C_CDD_EXPORT int g_cdd_fail_patch_list_sort;
extern C_CDD_EXPORT int g_cdd_fail_find_line_for_token;
extern C_CDD_EXPORT int g_cdd_fail_append_to_diff;

struct DiffLine;
extern C_CDD_EXPORT cdd_c_error_t cdd_test_find_line_for_token(
    const struct Token *tok, const struct DiffLine *old_lines,
    size_t old_line_count, size_t *out_line);
extern C_CDD_EXPORT cdd_c_error_t
cdd_test_split_lines(const char *str, size_t len, struct DiffLine **out_lines,
                     size_t *out_count);
#endif

/**
 * @brief Test basic diff generation.
 */
TEST test_patch_list_to_diff_basic(void) {
  struct PatchList list;
  struct TokenList *tokens = NULL;
  const char *src =
      (char *)(size_t)(size_t) "int main() {\n      return 0;\n}\n";
  int res;
  char *diff_str = NULL;
  size_t tok_idx = 0;
  int found = 0;

  res = tokenize(az_span_create_from_str((char *)(size_t)src), &tokens);
  ASSERT_EQ(0, res);

  res = patch_list_init(&list);
  ASSERT_EQ(0, res);

  /* Find token for 0 */
  for (tok_idx = 0; tok_idx < tokens->size; ++tok_idx) {
    if (tokens->tokens[tok_idx].kind == TOKEN_NUMBER_LITERAL &&
        tokens->tokens[tok_idx].length == 1 &&
        tokens->tokens[tok_idx].start[0] == '0') {
      found = true;
      break;
    }
  }
  ASSERT(found);

  /* Replace 0 with 1 */
  {
    char *text = (char *)(size_t)malloc(2);
#if defined(_MSC_VER)
    strcpy_s(text, 2, "1");
#else
    strcpy(text, "1");
#endif
    res = patch_list_add(&list, tok_idx, tok_idx + 1, text);
  }
  ASSERT_EQ(0, res);

  res = patch_list_to_diff(&list, tokens, "main.c", &diff_str);
  ASSERT_EQ(0, res);
  ASSERT(diff_str != NULL);

  ASSERT(strstr(diff_str, "--- main.c\n") != NULL);
  ASSERT(strstr(diff_str, "+++ main.c\n") != NULL);
  ASSERT(strstr(diff_str, "-      return 0;\n") != NULL);
  ASSERT(strstr(diff_str, "+      return 1;\n") != NULL);

  free(diff_str);
  patch_list_free(&list);
  free_token_list(tokens);
  PASS();
}

/**
 * @brief Test diff generation on empty list and empty tokens.
 */
TEST test_patch_list_to_diff_empty(void) {
  struct PatchList list;
  struct TokenList *tokens = NULL;
  struct TokenList empty_tokens;
  char *diff_str = NULL;
  char *t;
  cdd_c_error_t rc;

  rc = patch_list_init(&list);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = tokenize(az_span_create_from_str((char *)(size_t) ""), &tokens);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  ASSERT_EQ(CDD_C_SUCCESS,
            patch_list_to_diff(&list, tokens, "empty.c", &diff_str));
  ASSERT_STR_EQ("", diff_str);
  free(diff_str);

  /* Test empty list with OOM */
#ifdef CDD_BUILD_TESTS
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            patch_list_to_diff(&list, tokens, "empty.c", &diff_str));
  g_cdd_alloc_fail = 0;

  /* Test empty list with g_cdd_alloc_fail = 2 (branch false on
   * --g_cdd_alloc_fail == 0) */
  g_cdd_alloc_fail = 2;
  rc = patch_list_to_diff(&list, tokens, "empty.c", &diff_str);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  free(diff_str);
  g_cdd_alloc_fail = 0;
#endif

  /* Test non-empty list with empty tokens (tokens->size == 0) */
  t = (char *)malloc(2);
#if defined(_MSC_VER)
  strcpy_s(t, 2, "a");
#else
  strcpy(t, "a");
#endif
  rc = patch_list_add(&list, 0, 1, t);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  memset(&empty_tokens, 0, sizeof(empty_tokens));
  ASSERT_EQ(CDD_C_SUCCESS,
            patch_list_to_diff(&list, &empty_tokens, "empty.c", &diff_str));
  ASSERT_STR_EQ("", diff_str);
  free(diff_str);

  patch_list_free(&list);
  free_token_list(tokens);
  PASS();
}

/**
 * @brief Test argument validation and NULL filename.
 */
TEST test_diff_invalid_args(void) {
  struct PatchList list;
  struct TokenList *tokens = NULL;
  char *diff_str = NULL;
  char *t1;
  cdd_c_error_t rc;

  rc = patch_list_init(&list);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc =
      tokenize(az_span_create_from_str(((char *)(size_t) "int a;\n")), &tokens);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            patch_list_to_diff(NULL, tokens, "a.c", &diff_str));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            patch_list_to_diff(&list, NULL, "a.c", &diff_str));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            patch_list_to_diff(&list, tokens, "a.c", NULL));

  /* Test with filename == NULL to hit filename ? filename : "" branch */
  t1 = (char *)malloc(2);
#if defined(_MSC_VER)
  strcpy_s(t1, 2, "b");
#else
  strcpy(t1, "b");
#endif
  rc = patch_list_add(&list, 2, 3, t1);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = patch_list_to_diff(&list, tokens, NULL, &diff_str);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(diff_str != NULL);
  free(diff_str);

  patch_list_free(&list);
  free_token_list(tokens);
  PASS();
}

/**
 * @brief Test sorting failure.
 */
TEST test_diff_sort_fail(void) {
  struct PatchList list;
  struct TokenList *tokens = NULL;
  char *diff_str = NULL;
  char *text;
  cdd_c_error_t rc;

  rc = patch_list_init(&list);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc =
      tokenize(az_span_create_from_str(((char *)(size_t) "int a;\n")), &tokens);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  text = (char *)malloc(6);
#if defined(_MSC_VER)
  strcpy_s(text, 6, "float");
#else
  strcpy(text, "float");
#endif
  rc = patch_list_add(&list, 0, 1, text);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

#ifdef CDD_BUILD_TESTS
  g_cdd_fail_patch_list_sort = 1;
  rc = patch_list_to_diff(&list, tokens, "a.c", &diff_str);
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
  g_cdd_fail_patch_list_sort = 0;
#endif

  patch_list_free(&list);
  free_token_list(tokens);
  PASS();
}

/**
 * @brief Test pure insertion, pure deletion, and adjacent patches.
 */
TEST test_diff_insert_delete_adjacent(void) {
  struct PatchList list;
  struct TokenList *tokens = NULL;
  char *diff_str = NULL;
  char *t1;
  char *t2;
  char *tdel;
  char *large_text;
  cdd_c_error_t rc;

  rc = tokenize(
      az_span_create_from_str(((char *)(size_t) "int a = 1;\nint b = 2;\n")),
      &tokens);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = patch_list_init(&list);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* 1. Pure insertion at token 0 */
  t1 = (char *)malloc(16);
#if defined(_MSC_VER)
  strcpy_s(t1, 16, "/* comment */\n");
#else
  strcpy(t1, "/* comment */\n");
#endif
  rc = patch_list_add(&list, 0, 0, t1);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* 2. Pure deletion of token 1: add then set text to NULL */
  tdel = (char *)malloc(1);
  tdel[0] = '\0';
  rc = patch_list_add(&list, 1, 2, tdel);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  free(list.patches[list.size - 1].text);
  list.patches[list.size - 1].text = NULL;

  /* 3. Adjacent replacement at token 2 */
  t2 = (char *)malloc(4);
#if defined(_MSC_VER)
  strcpy_s(t2, 4, "+=");
#else
  strcpy(t2, "+=");
#endif
  rc = patch_list_add(&list, 2, 3, t2);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* 4. Large patch to trigger buffer resizing in generate_block_new_text */
  large_text = (char *)malloc(200);
  memset(large_text, 'x', 198);
  large_text[198] = '\n';
  large_text[199] = '\0';
  rc = patch_list_add(&list, 3, 4, large_text);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = patch_list_to_diff(&list, tokens, "test.c", &diff_str);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(diff_str != NULL);

  free(diff_str);
  patch_list_free(&list);
  free_token_list(tokens);
  PASS();
}

/**
 * @brief Test single patch at index 0 and max_mod_line < min_mod_line.
 */
TEST test_diff_end_token_zero_and_max_mod(void) {
  struct PatchList list;
  struct TokenList *tokens = NULL;
  char *diff_str = NULL;
  char *t1;
  cdd_c_error_t rc;

  /* Test 1: Single pure insertion at token 0 (end_token_idx == 0) */
  rc = tokenize(az_span_create_from_str(((char *)(size_t) "int a;\nint b;\n")),
                &tokens);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = patch_list_init(&list);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  t1 = (char *)malloc(16);
#if defined(_MSC_VER)
  strcpy_s(t1, 16, "// header\n");
#else
  strcpy(t1, "// header\n");
#endif
  rc = patch_list_add(&list, 0, 0, t1);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = patch_list_to_diff(&list, tokens, "zero.c", &diff_str);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(diff_str != NULL);
  free(diff_str);
  patch_list_free(&list);
  free_token_list(tokens);

  /* Test 2: Pure insertion on line 2 (start = end = token 5: 'int' on line 2)
   */
  rc = tokenize(az_span_create_from_str(((char *)(size_t) "int a;\nint b;\n")),
                &tokens);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = patch_list_init(&list);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  t1 = (char *)malloc(16);
#if defined(_MSC_VER)
  strcpy_s(t1, 16, "/* ins */\n");
#else
  strcpy(t1, "/* ins */\n");
#endif
  rc = patch_list_add(&list, 5, 5, t1);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  diff_str = NULL;
  rc = patch_list_to_diff(&list, tokens, "maxmod.c", &diff_str);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(diff_str != NULL);
  free(diff_str);
  patch_list_free(&list);
  free_token_list(tokens);
  PASS();
}

/**
 * @brief Test deleting entire 1-line file so new_text is empty and cursor ==
 * block_end_ptr.
 */
TEST test_diff_empty_block_and_cursor_end(void) {
  struct PatchList list;
  struct TokenList *tokens = NULL;
  char *diff_str = NULL;
  char *empty_text;
  cdd_c_error_t rc;

  /* Source has only 1 line, replacing all tokens 0..tokens->size with "" */
  rc = tokenize(az_span_create_from_str(((char *)(size_t) "int a;")), &tokens);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = patch_list_init(&list);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  empty_text = (char *)malloc(1);
  empty_text[0] = '\0';
  rc = patch_list_add(&list, 0, tokens->size, empty_text);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = patch_list_to_diff(&list, tokens, "delall.c", &diff_str);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(diff_str != NULL);
  free(diff_str);
  patch_list_free(&list);
  free_token_list(tokens);
  PASS();
}

/**
 * @brief Test fallback when token is outside all lines.
 */
TEST test_diff_token_outside_lines(void) {
#ifdef CDD_BUILD_TESTS
  struct Token tok;
  size_t out_line = 0;
  cdd_c_error_t rc;
  struct DiffLine *lines = NULL;
  size_t count = 0;
  struct DiffLine dline;
  const char *txt = "hello\n";

  memset(&tok, 0, sizeof(tok));
  tok.start = (const uint8_t *)"abc";

  rc = cdd_test_find_line_for_token(&tok, NULL, 0, &out_line);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, out_line);

  /* Test branch where tok->start < line_start */
  dline.text = txt + 3;
  dline.len = 3;
  tok.start = (const uint8_t *)txt;
  rc = cdd_test_find_line_for_token(&tok, &dline, 1, &out_line);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, out_line);

  rc = cdd_test_split_lines("", 0, &lines, &count);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(lines == NULL);
  ASSERT_EQ(0, count);
#endif
  PASS();
}

/**
 * @brief Test find_line_for_token error paths.
 */
TEST test_diff_find_line_errors(void) {
#ifdef CDD_BUILD_TESTS
  int i;
  for (i = 1; i <= 4; ++i) {
    struct PatchList list;
    struct TokenList *tokens = NULL;
    char *diff_str = NULL;
    char *t1;
    cdd_c_error_t rc;

    rc =
        tokenize(az_span_create_from_str(((char *)(size_t) "int a;\nint b;\n")),
                 &tokens);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    rc = patch_list_init(&list);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    t1 = (char *)malloc(4);
#if defined(_MSC_VER)
    strcpy_s(t1, 4, "x");
#else
    strcpy(t1, "x");
#endif
    rc = patch_list_add(&list, 1, 2, t1);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    g_cdd_fail_find_line_for_token = i;
    rc = patch_list_to_diff(&list, tokens, "err.c", &diff_str);
    g_cdd_fail_find_line_for_token = 0;
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);

    patch_list_free(&list);
    free_token_list(tokens);
  }

  /* Test failure at tokens[0] fallback when end_token_idx == 0 */
  {
    struct PatchList list;
    struct TokenList *tokens = NULL;
    char *diff_str = NULL;
    char *t1;
    cdd_c_error_t rc;

    rc =
        tokenize(az_span_create_from_str(((char *)(size_t) "int a;\nint b;\n")),
                 &tokens);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    rc = patch_list_init(&list);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    t1 = (char *)malloc(4);
#if defined(_MSC_VER)
    strcpy_s(t1, 4, "x");
#else
    strcpy(t1, "x");
#endif
    rc = patch_list_add(&list, 0, 0, t1);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    g_cdd_fail_find_line_for_token = 3;
    rc = patch_list_to_diff(&list, tokens, "err.c", &diff_str);
    g_cdd_fail_find_line_for_token = 0;
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);

    patch_list_free(&list);
    free_token_list(tokens);
  }
#endif
  PASS();
}

/**
 * @brief Test append_to_diff error paths across header, hunk, keep, del, and
 * nonl.
 */
TEST test_diff_append_errors(void) {
#ifdef CDD_BUILD_TESTS
  int i;
  const char *src = "first line\nmiddle line\nlast line";
  for (i = 1; i <= 12; ++i) {
    struct PatchList list;
    struct TokenList *tokens = NULL;
    char *diff_str = NULL;
    char *t1;
    cdd_c_error_t rc;

    rc = tokenize(az_span_create_from_str(((char *)(size_t)src)), &tokens);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    rc = patch_list_init(&list);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    /* Modify middle line */
    t1 = (char *)malloc(8);
#if defined(_MSC_VER)
    strcpy_s(t1, 8, "NEW_MID");
#else
    strcpy(t1, "NEW_MID");
#endif
    rc = patch_list_add(&list, 2, 4, t1);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    g_cdd_fail_append_to_diff = i;
    rc = patch_list_to_diff(&list, tokens, "err.c", &diff_str);
    g_cdd_fail_append_to_diff = 0;

    if (rc != CDD_C_SUCCESS) {
      ASSERT(diff_str == NULL);
    } else {
      free(diff_str);
    }

    patch_list_free(&list);
    free_token_list(tokens);
  }

  /* Del and ins with no trailing newline */
  for (i = 1; i <= 8; ++i) {
    struct PatchList list;
    struct TokenList *tokens = NULL;
    char *diff_str = NULL;
    char *t1;
    cdd_c_error_t rc;

    rc = tokenize(az_span_create_from_str(((char *)(size_t)src)), &tokens);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    rc = patch_list_init(&list);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    /* Replace last line with END lacking trailing newline */
    t1 = (char *)malloc(4);
#if defined(_MSC_VER)
    strcpy_s(t1, 4, "END");
#else
    strcpy(t1, "END");
#endif
    rc = patch_list_add(&list, tokens->size - 1, tokens->size, t1);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

    g_cdd_fail_append_to_diff = i;
    rc = patch_list_to_diff(&list, tokens, "err2.c", &diff_str);
    g_cdd_fail_append_to_diff = 0;

    if (rc != CDD_C_SUCCESS) {
      ASSERT(diff_str == NULL);
    } else {
      free(diff_str);
    }

    patch_list_free(&list);
    free_token_list(tokens);
  }
#endif
  PASS();
}

/**
 * @brief Test multiple disjoint blocks and block capacity expansion.
 */
TEST test_diff_multiple_blocks(void) {
  struct PatchList list;
  struct TokenList *tokens = NULL;
  char *diff_str = NULL;
  const char *src = "line 01\nline 02\nline 03\nline 04\nline 05\n"
                    "line 06\nline 07\nline 08\nline 09\nline 10\n"
                    "line 11\nline 12\nline 13\nline 14\nline 15\n"
                    "line 16\nline 17\nline 18\nline 19\nline 20\n"
                    "line 21\nline 22\nline 23\nline 24\nline 25\n"
                    "line 26\nline 27\nline 28\nline 29\nline 30\n"
                    "line 31\nline 32\nline 33\nline 34\nline 35\n";
  cdd_c_error_t rc;
  char *rep1;
  char *rep2;
  char *rep3;
  char *rep4;

  rc = tokenize(az_span_create_from_str(((char *)(size_t)src)), &tokens);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = patch_list_init(&list);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* Block 1: patch on line 2 (tokens 4..7) */
  rep1 = (char *)malloc(8);
#if defined(_MSC_VER)
  strcpy_s(rep1, 8, "MOD02\n");
#else
  strcpy(rep1, "MOD02\n");
#endif
  rc = patch_list_add(&list, 4, 7, rep1);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* Overlapping patch: on line 3 (tokens 8..11, merges into Block 1) */
  rep2 = (char *)malloc(8);
#if defined(_MSC_VER)
  strcpy_s(rep2, 8, "MOD03\n");
#else
  strcpy(rep2, "MOD03\n");
#endif
  rc = patch_list_add(&list, 8, 11, rep2);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* Block 2: patch on line 15 (tokens 56..59, ctx_start=12 > block 1 end line
   * 6) */
  rep3 = (char *)malloc(8);
#if defined(_MSC_VER)
  strcpy_s(rep3, 8, "MOD15\n");
#else
  strcpy(rep3, "MOD15\n");
#endif
  rc = patch_list_add(&list, 56, 59, rep3);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* Block 3: patch on line 30 (tokens 116..119, ctx_start=27 > block 2 end line
   * 18) */
  /* This triggers block_count >= block_cap (2 >= 2) and block_cap * 2 */
  rep4 = (char *)malloc(8);
#if defined(_MSC_VER)
  strcpy_s(rep4, 8, "MOD30\n");
#else
  strcpy(rep4, "MOD30\n");
#endif
  rc = patch_list_add(&list, 116, 119, rep4);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = patch_list_to_diff(&list, tokens, "multi.c", &diff_str);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(diff_str != NULL);

  free(diff_str);
  patch_list_free(&list);
  free_token_list(tokens);
  PASS();
}

/**
 * @brief Test No newline at end of file handling.
 */
TEST test_diff_no_newline_eof(void) {
  struct PatchList list;
  struct TokenList *tokens = NULL;
  char *diff_str = NULL;
  const char *src = "alpha\nbeta\ngamma";
  cdd_c_error_t rc;
  char *rep;

  rc = tokenize(az_span_create_from_str(((char *)(size_t)src)), &tokens);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = patch_list_init(&list);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* Modify the last token gamma with a replacement that also has no newline */
  rep = (char *)malloc(6);
#if defined(_MSC_VER)
  strcpy_s(rep, 6, "delta");
#else
  strcpy(rep, "delta");
#endif
  rc = patch_list_add(&list, 4, 5, rep);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = patch_list_to_diff(&list, tokens, "nonl.c", &diff_str);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(diff_str != NULL);
  ASSERT(strstr(diff_str, "\\ No newline at end of file") != NULL);

  free(diff_str);
  patch_list_free(&list);
  free_token_list(tokens);

  /* Test where the last line is in keep_end */
  rc = tokenize(az_span_create_from_str(((char *)(size_t) "one\ntwo\nthree")),
                &tokens);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = patch_list_init(&list);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rep = (char *)malloc(4);
#if defined(_MSC_VER)
  strcpy_s(rep, 4, "1\n");
#else
  strcpy(rep, "1\n");
#endif
  rc = patch_list_add(&list, 0, 1, rep);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  diff_str = NULL;
  rc = patch_list_to_diff(&list, tokens, "nonl2.c", &diff_str);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(diff_str != NULL);
  ASSERT(strstr(diff_str, "\\ No newline at end of file") != NULL);

  free(diff_str);
  patch_list_free(&list);
  free_token_list(tokens);
  PASS();
}

/**
 * @brief Test OOM allocation failures.
 */
TEST test_diff_alloc_failures(void) {
#ifdef CDD_BUILD_TESTS
  int fail_count;
  for (fail_count = 1; fail_count <= 40; ++fail_count) {
    struct PatchList list;
    struct TokenList *tokens = NULL;
    char *diff_str = NULL;
    char *rep;
    cdd_c_error_t rc;

    rc = tokenize(az_span_create_from_str(
                      ((char *)(size_t) "int x = 10;\nint y = 20;\n")),
                  &tokens);
    if (rc != CDD_C_SUCCESS)
      continue;

    rc = patch_list_init(&list);
    if (rc != CDD_C_SUCCESS) {
      free_token_list(tokens);
      continue;
    }

    rep = (char *)malloc(80);
    memset(rep, 'z', 78);
    rep[78] = '\n';
    rep[79] = '\0';
    rc = patch_list_add(&list, 3, 4, rep);
    if (rc != CDD_C_SUCCESS) {
      patch_list_free(&list);
      free_token_list(tokens);
      continue;
    }

    g_cdd_alloc_fail = fail_count;
    rc = patch_list_to_diff(&list, tokens, "x.c", &diff_str);
    g_cdd_alloc_fail = 0;

    if (rc == CDD_C_SUCCESS && diff_str) {
      free(diff_str);
    }
    patch_list_free(&list);
    free_token_list(tokens);
  }
#endif
  PASS();
}

SUITE(diff_suite) {
  RUN_TEST(test_patch_list_to_diff_basic);
  RUN_TEST(test_patch_list_to_diff_empty);
  RUN_TEST(test_diff_invalid_args);
  RUN_TEST(test_diff_sort_fail);
  RUN_TEST(test_diff_insert_delete_adjacent);
  RUN_TEST(test_diff_end_token_zero_and_max_mod);
  RUN_TEST(test_diff_empty_block_and_cursor_end);
  RUN_TEST(test_diff_token_outside_lines);
  RUN_TEST(test_diff_find_line_errors);
  RUN_TEST(test_diff_append_errors);
  RUN_TEST(test_diff_multiple_blocks);
  RUN_TEST(test_diff_no_newline_eof);
  RUN_TEST(test_diff_alloc_failures);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_DIFF_H */
