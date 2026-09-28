/**
 * @file test_text_patcher.h
 * @brief Unit tests for the text patching engine.
 */

#ifndef TEST_TEXT_PATCHER_H
#define TEST_TEXT_PATCHER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include "cdd_c_error.h"
#include <greatest.h>
#include <stdlib.h>
#include <string.h>

#include "functions/emit/patcher.h"
#include "functions/parse/str.h"
#include "functions/parse/tokenizer.h"
/* clang-format on */

/* Moved extern declarations for C89 compliance */

/* Helper to setup a token list from a string */
static cdd_c_error_t setup_patch_tokens(const char *code,
                                        struct TokenList **_out_val) {
  struct TokenList *tl = NULL;
  int rc;
  if (!_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = tokenize(az_span_create_from_str((char *)(size_t)(size_t)code), &tl);
  if (rc != 0) {
    *_out_val = NULL;
    return rc;
  }
  *_out_val = tl;
  return CDD_C_SUCCESS;
}

TEST test_patch_init_free(void) {
  char *_ast_strdup_0 = NULL;
  struct PatchList pl;
  struct TokenList *tl_dummy = NULL;
  int rc;

  /* Exercise setup_patch_tokens error branches */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, setup_patch_tokens("code", NULL));
  {
    extern C_CDD_EXPORT int g_cdd_alloc_fail;
    g_cdd_alloc_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, setup_patch_tokens("code", &tl_dummy));
    g_cdd_alloc_fail = 0;
  }

  rc = patch_list_init(&pl);
  ASSERT_EQ(0, rc);
  ASSERT(pl.patches != NULL);
  ASSERT_EQ(0, pl.size);

  /* Add one to test free logic */
  patch_list_add(&pl, 0, 1,
                 (c_cdd_strdup("test", &_ast_strdup_0), _ast_strdup_0));

  patch_list_free(&pl);
  ASSERT_EQ(0, pl.size);
  ASSERT(pl.patches == NULL);
  g_fail_io_after = -1;
  PASS();
}

TEST test_patch_basic_replacement(void) {
  struct TokenList *_ast_setup_patch_tokens_0;
  char *_ast_strdup_1 = NULL;
  /* Input: int x = 5; */
  /* Tokens: [int] [ ] [x] [ ] [=] [ ] [5] [;] */
  /* Indices: 0     1   2   3   4   5   6   7 */
  const char *code = (char *)(size_t)(size_t) "int x = 5;";
  struct TokenList *tl = (setup_patch_tokens(code, &_ast_setup_patch_tokens_0),
                          _ast_setup_patch_tokens_0);
  struct PatchList pl;
  char *result = NULL;
  int rc;

  ASSERT(tl);
  patch_list_init(&pl);

  /* Replace '5' (index 6, len 1) with '10' */
  /* "5" is at token index 6 because of whitespace tokens */
  /* Let's verify token indices first to be robust */
  ASSERT(tl->tokens[6].kind == TOKEN_NUMBER_LITERAL);

  rc = patch_list_add(&pl, 6, 7,
                      (c_cdd_strdup("10", &_ast_strdup_1), _ast_strdup_1));
  ASSERT_EQ(0, rc);

  rc = patch_list_apply(&pl, tl, &result);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("int x = 10;", result);

  free(result);
  patch_list_free(&pl);
  free_token_list(tl);
  g_fail_io_after = -1;
  PASS();
}

TEST test_patch_insertion(void) {
  struct TokenList *_ast_setup_patch_tokens_1;
  char *_ast_strdup_2 = NULL;
  /* Input: void f(){} */
  /* Tokens: [void] [ ] [f] [(] [)] [{] [}] */
  /* Indices: 0     1   2   3   4   5   6 */
  const char *code = ""
                     "void f(){}";
  struct TokenList *tl = (setup_patch_tokens(code, &_ast_setup_patch_tokens_1),
                          _ast_setup_patch_tokens_1);
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

  rc = patch_list_add(
      &pl, 5, 7, (c_cdd_strdup("{ int x; }", &_ast_strdup_2), _ast_strdup_2));
  ASSERT_EQ(0, rc);

  rc = patch_list_apply(&pl, tl, &result);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ(""
                "void f(){ int x; }",
                result);

  free(result);
  patch_list_free(&pl);
  free_token_list(tl);
  g_fail_io_after = -1;
  PASS();
}

TEST test_patch_deletion(void) {
  struct TokenList *_ast_setup_patch_tokens_2;
  char *_ast_strdup_3 = NULL;
  /* Input: int x; */
  /* Tokens: [int] [ ] [x] [;] */
  const char *code = (char *)(size_t)(size_t) "int x;";
  struct TokenList *tl = (setup_patch_tokens(code, &_ast_setup_patch_tokens_2),
                          _ast_setup_patch_tokens_2);
  struct PatchList pl;
  char *result = NULL;
  int rc;

  ASSERT(tl);
  patch_list_init(&pl);

  /* Delete "int " tokens [0, 2) */
  rc = patch_list_add(&pl, 0, 2,
                      (c_cdd_strdup("", &_ast_strdup_3), _ast_strdup_3));
  ASSERT_EQ(0, rc);

  rc = patch_list_apply(&pl, tl, &result);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("x;", result);

  free(result);
  patch_list_free(&pl);
  free_token_list(tl);
  g_fail_io_after = -1;
  PASS();
}

TEST test_patch_multiple_disjoint(void) {
  struct TokenList *_ast_setup_patch_tokens_3;
  char *_ast_strdup_4 = NULL;
  char *_ast_strdup_5 = NULL;
  /* Input: A B C */
  const char *code = (char *)(size_t)(size_t) "A B C";
  struct TokenList *tl = (setup_patch_tokens(code, &_ast_setup_patch_tokens_3),
                          _ast_setup_patch_tokens_3);
  struct PatchList pl;
  char *result = NULL;
  int rc;

  ASSERT(tl);
  patch_list_init(&pl);

  /* Replace A -> X */
  rc = patch_list_add(
      &pl, 0, 1,
      (c_cdd_strdup("X", &_ast_strdup_4), _ast_strdup_4)); /* A at 0 */
  /* Replace C -> Z */
  /* [A] [ ] [B] [ ] [C] -> 0 1 2 3 4 */
  rc = patch_list_add(
      &pl, 4, 5,
      (c_cdd_strdup("Z", &_ast_strdup_5), _ast_strdup_5)); /* C at 4 */

  rc = patch_list_apply(&pl, tl, &result);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ("X B Z", result);

  free(result);
  patch_list_free(&pl);
  free_token_list(tl);
  g_fail_io_after = -1;
  PASS();
}

TEST test_patch_overlap_behavior(void) {
  struct TokenList *_ast_setup_patch_tokens_4;
  char *_ast_strdup_6 = NULL;
  char *_ast_strdup_7 = NULL;
  /* Input: A */
  const char *code = (char *)(size_t)(size_t) "A";
  struct TokenList *tl = (setup_patch_tokens(code, &_ast_setup_patch_tokens_4),
                          _ast_setup_patch_tokens_4);
  struct PatchList pl;
  char *result = NULL;
  int rc;

  /* Assert undefined behavior matches implementation (sorted, first wins) */
  ASSERT(tl);
  patch_list_init(&pl);

  /* Replace A -> X */
  rc = patch_list_add(&pl, 0, 1,
                      (c_cdd_strdup("X", &_ast_strdup_6), _ast_strdup_6));
  /* Replace A -> Y (Same range) -> Should be skipped or overwrite?
   * Implementation skips overlaps */
  rc = patch_list_add(&pl, 0, 1,
                      (c_cdd_strdup("Y", &_ast_strdup_7), _ast_strdup_7));

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
  g_fail_io_after = -1;
  PASS();
}

TEST test_patch_append_end(void) {
  struct TokenList *_ast_setup_patch_tokens_5;
  char *_ast_strdup_8 = NULL;
  const char *code = (char *)(size_t)(size_t) "End";
  struct TokenList *tl = (setup_patch_tokens(code, &_ast_setup_patch_tokens_5),
                          _ast_setup_patch_tokens_5);
  struct PatchList pl;
  char *result = NULL;
  int rc;
  char huge_str[3000];

  ASSERT(tl);
  patch_list_init(&pl);

  memset(huge_str, 'A', 2999);
  huge_str[2999] = '\0';

  /* Append after end. Token list size is 1. Insert at index 1 (end) */
  rc = patch_list_add(&pl, 1, 1,
                      (c_cdd_strdup(huge_str, &_ast_strdup_8), _ast_strdup_8));
  ASSERT_EQ(0, rc);

  rc = patch_list_apply(&pl, tl, &result);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ(huge_str, result + 3); /* End is 3 chars */

  free(result);
  patch_list_free(&pl);
  free_token_list(tl);
  g_fail_io_after = -1;
  PASS();
}

TEST test_patch_bounds(void) {
  struct PatchList pl;
  struct TokenList tl;
  char *res = NULL;
  memset(&tl, 0, sizeof(tl));
  patch_list_init(&pl);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, patch_list_init(NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            patch_list_add(NULL, 0, 1, strdup("X")));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, patch_list_apply(NULL, &tl, &res));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, patch_list_apply(&pl, NULL, &res));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, patch_list_apply(&pl, &tl, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, patch_list_apply(&pl, NULL, NULL));
  patch_list_free(&pl);
  patch_list_free(NULL); /* Should not crash */

#ifdef CDD_BUILD_TESTS
  {
    extern C_CDD_EXPORT int g_patcher_test_cap_1;
    extern C_CDD_EXPORT int g_cdd_fail_patch_list_sort;
    struct PatchList pl_test;
    struct TokenList tl_test;
    char *out_test = NULL;
    memset(&tl_test, 0, sizeof(tl_test));

    g_patcher_test_cap_1 = 1;
    ASSERT_EQ(CDD_C_SUCCESS, patch_list_init(&pl_test));
    ASSERT_EQ(CDD_C_SUCCESS, patch_list_add(&pl_test, 0, 1, strdup("X")));
    ASSERT_EQ(CDD_C_SUCCESS, patch_list_add(&pl_test, 1, 2, strdup("Y")));
    g_patcher_test_cap_1 = 0;

    g_cdd_fail_patch_list_sort = 1;
    ASSERT_NEQ(0, patch_list_apply(&pl_test, &tl_test, &out_test));
    g_cdd_fail_patch_list_sort = 0;

    g_cdd_fail_patch_list_sort = 2;
    ASSERT_EQ(0, patch_list_apply(&pl_test, &tl_test, &out_test));
    if (out_test) {
      free(out_test);
      out_test = NULL;
    }
    ASSERT_NEQ(0, patch_list_apply(&pl_test, &tl_test, &out_test));
    g_cdd_fail_patch_list_sort = 0;

    patch_list_free(&pl_test);
  }
#endif

  g_fail_io_after = -1;
  PASS();
}

TEST test_patcher_invalid(void) {
  struct PatchList pl2;
  char huge_str[2000];
  struct TokenList *tl_huge = NULL;
  char *res_huge = NULL;
  struct PatchList pl3;
  struct TokenList tl_empty;
  int i;
  int rc;
  /*  (moved to global) */

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            patch_list_add(NULL, 0, 1, strdup("a")));

  /* Test patch_list_init allocation failure */
  g_cdd_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, patch_list_init(&pl2));
  patch_list_free(&pl2); /* This covers list->patches == NULL */
  g_cdd_alloc_fail = 0;

  /* Test patch_list_add with text == NULL */
  patch_list_init(&pl2);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, patch_list_add(&pl2, 0, 1, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, patch_list_add(NULL, 0, 1, NULL));

  /* Trigger capacity == 0 branch false in patch_list_add */
  patch_list_add(&pl2, 0, 1, strdup("1"));
  patch_list_add(&pl2, 0, 1, strdup("2"));
  patch_list_add(&pl2, 0, 1, strdup("3"));
  patch_list_add(&pl2, 0, 1, strdup("4"));
  patch_list_add(&pl2, 0, 1, strdup("5"));
  patch_list_add(&pl2, 0, 1, strdup("6"));
  patch_list_add(&pl2, 0, 1, strdup("7"));
  patch_list_add(&pl2, 0, 1, strdup("8"));

  /* Trigger C_CDD_REALLOC failure when capacity > 0 */
  g_cdd_alloc_fail = 1;
  rc = patch_list_add(&pl2, 0, 1, strdup("9_fail"));
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  g_cdd_alloc_fail = 0;

  /* Test patch_list_sort edge cases */
  ASSERT_EQ(CDD_C_SUCCESS, patch_list_sort(NULL));
  ASSERT_EQ(CDD_C_SUCCESS, patch_list_sort(&pl2)); /* Size is 8 > 1 */

  patch_list_free(&pl2);
  patch_list_init(&pl2);

  memset(huge_str, 'x', 1999);
  huge_str[1999] = '\0';

  for (i = 0; i < 8; i++) {
    patch_list_add(&pl2, 0, 1, strdup("test"));
  }

  g_cdd_alloc_fail = 1;
  {
    char *dummy = malloc(5);
#if defined(_MSC_VER)
    strcpy_s(dummy, 5, "test");
#else
    strcpy(dummy, "test");
#endif
    rc = patch_list_add(&pl2, 0, 1, dummy);
    printf("DEBUG: rc=%d\n", rc);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    g_cdd_alloc_fail = 0;

    patch_list_add(&pl2, 0, 1, strdup(huge_str));
    patch_list_add(&pl2, 2, 3, strdup(huge_str));

    setup_patch_tokens(huge_str, &tl_huge);

    for (i = 1; i <= 20; i++) {
      res_huge = NULL;
      g_cdd_alloc_fail = i;
      rc = patch_list_apply(&pl2, tl_huge, &res_huge);
      if (res_huge) {
        free(res_huge);
      }
      if (rc == 0) {
        break; /* We have exhausted all allocation failure paths */
      }
      printf("DEBUG: rc=%d\n", rc);
      ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    }
    g_cdd_alloc_fail = 0;

    patch_list_init(&pl3);
    res_huge = NULL;
    ASSERT_EQ(0, patch_list_apply(&pl3, tl_huge, &res_huge));
    if (res_huge) {
      free(res_huge);
      res_huge = NULL;
    }

    memset(&tl_empty, 0, sizeof(tl_empty));

    patch_list_free(&pl2);
    patch_list_free(&pl3);
    if (tl_huge) {
      free_token_list(tl_huge);
    }
    PASS();
  }
}

TEST test_patcher_cov(void) {
  struct PatchList pl;
  char *out = NULL;
  struct TokenList *tl = NULL;

  /* test patch_list_sort with NULL patches */
  pl.patches = NULL;
  pl.size = 2;
  ASSERT_EQ(CDD_C_SUCCESS, patch_list_sort(&pl));

  /* test patch_list_sort with size <= 1 */
  pl.patches = (struct Patch *)1;
  pl.size = 1;
  ASSERT_EQ(CDD_C_SUCCESS, patch_list_sort(&pl));

  /* test patch_list_apply OOM in copy original token content loop */
  patch_list_init(&pl);
  tokenize(az_span_create_from_str((char *)(size_t)(size_t) "int a;"), &tl);

#ifdef CDD_BUILD_TESTS
  {
    /*  (moved to global) */
    g_cdd_alloc_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, patch_list_apply(&pl, tl, &out));
    g_cdd_alloc_fail = 0;
  }
#endif

  patch_list_free(&pl);
  free_token_list(tl);
  PASS();
}

TEST test_patcher_cov_extra(void) {
  struct PatchList pl;
  char *out = NULL;
  struct TokenList *tl = NULL;

  patch_list_init(&pl);
  tokenize(az_span_create_from_str((char *)(size_t)(size_t) "int"),
           &tl); /* 1 token */

  /* Patch 0: replaces token 0 up to 3 (which exceeds size 1), making
   * current_token = 3 */
  patch_list_add(&pl, 0, 3, strdup("replacement"));

  /* Patch 1: starts at 0, overlaps, but since outer loop terminates it's
   * evaluated in the trailing loop */
  patch_list_add(&pl, 0, 1, strdup("overlapped"));

  ASSERT_EQ(CDD_C_SUCCESS, patch_list_apply(&pl, tl, &out));

  if (out)
    free(out);
  patch_list_free(&pl);
  free_token_list(tl);
  PASS();
}

TEST test_patcher_oom_original_token_copy(void) {
  struct PatchList list;
  struct TokenList *tl = NULL;
  const char *src = (char *)(size_t)(size_t) "int a = 5; int b = 6; int c = 7;";
  int res;
  char *out_code = NULL;
  int i;

  res = patch_list_init(&list);
  ASSERT_EQ(CDD_C_SUCCESS, res);

  res = tokenize(az_span_create((uint8_t *)(size_t)src, strlen(src)), &tl);
  ASSERT_EQ(CDD_C_SUCCESS, res);

  /* Add a patch so patch loop logic applies */
  patch_list_add(&list, 2, 3, C_CDD_STRDUP("patched_value"));

#ifdef CDD_BUILD_TESTS
  {
    /*  (moved to global) */
    /* Try failing allocations for the token copying block. We try a range. */
    for (i = 1; i < 20; i++) {
      g_cdd_alloc_fail = i;
      res = patch_list_apply(&list, tl, &out_code);
      g_cdd_alloc_fail = 0;
      if (res == CDD_C_SUCCESS) {
        free(out_code);
        break;
      }
    }
  }
#endif

  patch_list_free(&list);
  free_token_list(tl);
  PASS();
}

TEST test_patcher_oom_no_patches(void) {
  struct PatchList list;
  struct TokenList *tl = NULL;
  const char src[] = {
      105, 110, 116, 32, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65, 65,
      65,  65,  65,  65, 65, 65, 32, 61, 32, 53, 59, 0};
  int res;
  char *out_code = NULL;
  int i;

  res = patch_list_init(&list);
  res = tokenize(az_span_create((uint8_t *)(size_t)src, strlen(src)), &tl);

#ifdef CDD_BUILD_TESTS
  {
    /*  (moved to global) */
    for (i = 1; i < 5; i++) {
      g_cdd_alloc_fail = i;
      res = patch_list_apply(&list, tl, &out_code);
      g_cdd_alloc_fail = 0;
      if (res == CDD_C_SUCCESS) {
        free(out_code);
        break;
      }
    }
  }
#endif

  patch_list_free(&list);
  free_token_list(tl);
  PASS();
}

SUITE(text_patcher_suite) {
  RUN_TEST(test_patcher_oom_no_patches);
  RUN_TEST(test_patcher_oom_original_token_copy);
  RUN_TEST(test_patch_bounds);
  RUN_TEST(test_patch_init_free);
  RUN_TEST(test_patch_basic_replacement);
  RUN_TEST(test_patch_insertion);
  RUN_TEST(test_patch_deletion);
  RUN_TEST(test_patch_multiple_disjoint);
  RUN_TEST(test_patch_overlap_behavior);
  RUN_TEST(test_patch_append_end);
  RUN_TEST(test_patcher_invalid);
  RUN_TEST(test_patcher_cov);
  RUN_TEST(test_patcher_cov_extra);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_TEXT_PATCHER_H */
