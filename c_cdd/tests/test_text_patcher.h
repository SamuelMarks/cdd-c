/**
 * @file test_text_patcher.h
 * @brief Unit tests for the text patching engine.
 * @author Samuel Marks
 */

#ifndef TEST_TEXT_PATCHER_H
#define TEST_TEXT_PATCHER_H

#include <greatest.h>
#include <stdlib.h>
#include <string.h>

#include "str_utils.h"
#include "text_patcher.h"
#include "tokenizer.h"

/* Helper to setup a token list from a string */
static struct TokenList *setup_patch_tokens(const char *code) {
  struct TokenList *tl = NULL;
  int rc = tokenize(az_span_create_from_str((char *)code), &tl);
  if (rc != 0)
    return NULL;
  return tl;
}

TEST test_patch_init_free(void) {
  struct PatchList pl;
  int rc = patch_list_init(&pl);
  ASSERT_EQ(0, rc);
  ASSERT(pl.patches != NULL);
  ASSERT_EQ(0, pl.size);

  /* Add one to test free logic */
  patch_list_add(&pl, 0, 1, c_cdd_strdup("test"));

  patch_list_free(&pl);
  ASSERT_EQ(0, pl.size);
  ASSERT(pl.patches == NULL);
  PASS();
}

TEST test_patch_basic_replacement(void) {
  /* Input: int x = 5; */
  /* Tokens: [int] [ ] [x] [ ] [=] [ ] [5] [;] */
  /* Indices: 0     1   2   3   4   5   6   7 */
  const char *code = "int x = 5;";
  struct TokenList *tl = setup_patch_tokens(code);
  struct PatchList pl;
  char *result = NULL;
  int rc;

  ASSERT(tl);
  patch_list_init(&pl);

  /* Replace '5' (index 6, len 1) with '10' */
  /* "5" is at token index 6 because of whitespace tokens */
  /* Let's verify token indices first to be robust */
  ASSERT(tl->tokens[6].kind == TOKEN_NUMBER_LITERAL);

  rc = patch_list_add(&pl, 6, 7, c_cdd_strdup("10"));
  ASSERT_EQ(0, rc);

  rc = patch_list_apply(&pl, tl, &result);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("int x = 10;", result);

  free(result);
  patch_list_free(&pl);
  free_token_list(tl);
  PASS();
}

TEST test_patch_insertion(void) {
  /* Input: void f(){} */
  /* Tokens: [void] [ ] [f] [(] [)] [{] [}] */
  /* Indices: 0     1   2   3   4   5   6 */
  const char *code = "void f(){}";
  struct TokenList *tl = setup_patch_tokens(code);
  struct PatchList pl;
  char *result = NULL;
  int rc;

  ASSERT(tl);
  patch_list_init(&pl);

  /* Insert "int x;" at the start of body (index 6 is '{', insert at 6? No,
   * insert inside '{' means after 6) */
  /* Let's replace the empty body "{}" tokens [5, 7) with "{ int x; }" */
  ASSERT(tl->tokens[5].kind == TOKEN_LBRACE);
  ASSERT(tl->tokens[6].kind == TOKEN_RBRACE);

  rc = patch_list_add(&pl, 5, 7, c_cdd_strdup("{ int x; }"));
  ASSERT_EQ(0, rc);

  rc = patch_list_apply(&pl, tl, &result);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("void f(){ int x; }", result);

  free(result);
  patch_list_free(&pl);
  free_token_list(tl);
  PASS();
}

TEST test_patch_deletion(void) {
  /* Input: int x; */
  /* Tokens: [int] [ ] [x] [;] */
  const char *code = "int x;";
  struct TokenList *tl = setup_patch_tokens(code);
  struct PatchList pl;
  char *result = NULL;
  int rc;

  ASSERT(tl);
  patch_list_init(&pl);

  /* Delete "int " tokens [0, 2) */
  rc = patch_list_add(&pl, 0, 2, c_cdd_strdup(""));
  ASSERT_EQ(0, rc);

  rc = patch_list_apply(&pl, tl, &result);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("x;", result);

  free(result);
  patch_list_free(&pl);
  free_token_list(tl);
  PASS();
}

TEST test_patch_multiple_disjoint(void) {
  /* Input: A B C */
  const char *code = "A B C";
  struct TokenList *tl = setup_patch_tokens(code);
  struct PatchList pl;
  char *result = NULL;
  int rc;

  ASSERT(tl);
  patch_list_init(&pl);

  /* Replace A -> X */
  rc = patch_list_add(&pl, 0, 1, c_cdd_strdup("X")); /* A at 0 */
  /* Replace C -> Z */
  /* [A] [ ] [B] [ ] [C] -> 0 1 2 3 4 */
  rc = patch_list_add(&pl, 4, 5, c_cdd_strdup("Z")); /* C at 4 */

  rc = patch_list_apply(&pl, tl, &result);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("X B Z", result);

  free(result);
  patch_list_free(&pl);
  free_token_list(tl);
  PASS();
}

TEST test_patch_overlap_behavior(void) {
  /* Input: A */
  const char *code = "A";
  struct TokenList *tl = setup_patch_tokens(code);
  struct PatchList pl;
  char *result = NULL;
  int rc;

  /* Assert undefined behavior matches implementation (sorted, first wins) */
  ASSERT(tl);
  patch_list_init(&pl);

  /* Replace A -> X */
  rc = patch_list_add(&pl, 0, 1, c_cdd_strdup("X"));
  /* Replace A -> Y (Same range) -> Should be skipped or overwrite?
   * Implementation skips overlaps */
  rc = patch_list_add(&pl, 0, 1, c_cdd_strdup("Y"));

  rc = patch_list_apply(&pl, tl, &result);
  ASSERT_EQ(0, rc);
  /* The list is sorted. Add logic doesn't sort, Apply does.
     Sort is stable-ish or index based. X added first. */
  /* Actually implementation sorts by start index. Equal start? qsort order
     technically undefined for equal keys unless stable. Assuming simple qsort,
     let's just ensure it's one of them, likely X or Y. The important part is
     the overlap skip logic in apply: while (patch_idx < list->size &&
     list->patches[patch_idx].start_token_idx < current_token) After processing
     X, current_token becomes 1. The next patch has starts at 0. 0 < 1, so it is
     skipped.
  */
  ASSERT(strcmp(result, "X") == 0 || strcmp(result, "Y") == 0);

  free(result);
  patch_list_free(&pl);
  free_token_list(tl);
  PASS();
}

TEST test_patch_append_end(void) {
  const char *code = "End";
  struct TokenList *tl = setup_patch_tokens(code);
  struct PatchList pl;
  char *result = NULL;
  int rc;

  ASSERT(tl);
  patch_list_init(&pl);

  /* Append after end. Token list size is 1. Insert at index 1 (end) */
  rc = patch_list_add(&pl, 1, 1, c_cdd_strdup(" appended"));
  ASSERT_EQ(0, rc);

  rc = patch_list_apply(&pl, tl, &result);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("End appended", result);

  free(result);
  patch_list_free(&pl);
  free_token_list(tl);
  PASS();
}

SUITE(text_patcher_suite) {
  RUN_TEST(test_patch_init_free);
  RUN_TEST(test_patch_basic_replacement);
  RUN_TEST(test_patch_insertion);
  RUN_TEST(test_patch_deletion);
  RUN_TEST(test_patch_multiple_disjoint);
  RUN_TEST(test_patch_overlap_behavior);
  RUN_TEST(test_patch_append_end);
}

#endif /* TEST_TEXT_PATCHER_H */
