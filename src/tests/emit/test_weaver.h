/**
 * @file test_weaver.h
 * @brief Unit tests for weaver implementation.
 */

#ifndef TEST_WEAVER_H
#define TEST_WEAVER_H

#include <greatest.h>
#include <stdlib.h>
#include <string.h>

#include "functions/emit/weaver.h"

TEST test_weaver_wrap_ifdef_basic(void) {
  struct PatchList patches;
  struct TokenList *tokens = NULL;
  const char *src = "int a = 5;";
  int res;
  char *out_code = NULL;

  res = tokenize(az_span_create_from_str((char *)src), &tokens);
  ASSERT_EQ(0, res);

  res = patch_list_init(&patches);
  ASSERT_EQ(0, res);

  res = weaver_wrap_ifdef(&patches, tokens, 0, tokens->size, "_MSC_VER", NULL);
  ASSERT_EQ(0, res);

  res = patch_list_apply(&patches, tokens, &out_code);
  ASSERT_EQ(0, res);

  ASSERT_STR_EQ("#ifdef _MSC_VER
int a = 5;#endif\\n", out_code);

  free(out_code);
  patch_list_free(&patches);
  free_token_list(tokens);
  PASS();
}

TEST test_weaver_wrap_ifdef_else(void) {
  struct PatchList patches;
  struct TokenList *tokens = NULL;
  const char *src = "int a = 5;";
  int res;
  char *out_code = NULL;

  res = tokenize(az_span_create_from_str((char *)src), &tokens);
  ASSERT_EQ(0, res);

  res = patch_list_init(&patches);
  ASSERT_EQ(0, res);

  res = weaver_wrap_ifdef(&patches, tokens, 0, tokens->size, "_MSC_VER",
                          "int a = 10;");
  ASSERT_EQ(0, res);

  res = patch_list_apply(&patches, tokens, &out_code);
  ASSERT_EQ(0, res);

  ASSERT_STR_EQ("#ifdef _MSC_VER
int a = 5;#else
int a = 10;
#endif \\n ", out_code);

  free(out_code);
  patch_list_free(&patches);
  free_token_list(tokens);
  PASS();
}

TEST test_weaver_wrap_ifdef_invalid_args(void) {
  struct PatchList patches;
  struct TokenList tokens;
  tokens.size = 5;

  ASSERT_EQ(EINVAL, weaver_wrap_ifdef(NULL, &tokens, 0, 5, "C", NULL));
  ASSERT_EQ(EINVAL, weaver_wrap_ifdef(&patches, NULL, 0, 5, "C", NULL));
  ASSERT_EQ(EINVAL, weaver_wrap_ifdef(&patches, &tokens, 0, 5, NULL, NULL));
  ASSERT_EQ(EINVAL, weaver_wrap_ifdef(&patches, &tokens, 5, 4, "C", NULL));
  ASSERT_EQ(EINVAL, weaver_wrap_ifdef(&patches, &tokens, 0, 10, "C", NULL));

  PASS();
}

TEST test_weaver_inject_msvc_headers(void) {
  struct PatchList patches;
  struct TokenList *tokens = NULL;
  const char *src = "#include <stdio.h>
#include <stdlib.h>

      int
      main() {
    return 0;
  }
  ";
      int res;
  char *out_code = NULL;

  res = tokenize(az_span_create_from_str((char *)src), &tokens);
  ASSERT_EQ(0, res);

  res = patch_list_init(&patches);
  ASSERT_EQ(0, res);

  res = weaver_inject_msvc_headers(&patches, tokens, true, true);
  ASSERT_EQ(0, res);

  res = patch_list_apply(&patches, tokens, &out_code);
  ASSERT_EQ(0, res);

  ASSERT_STR_EQ("#include <stdio.h>
#include <stdlib.h>

#ifdef _MSC_VER
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#ifndef NOMINMAX
#define NOMINMAX
#endif

#endif

int main() {
    return 0; }", out_code);

  free(out_code);
  patch_list_free(&patches);
  free_token_list(tokens);
  PASS();
}

TEST test_weaver_vla_to_alloca(void) {
  struct PatchList patches;
  struct TokenList *tokens = NULL;
  const char *src = ""
                    "int main() {
      int n = 10;
  int arr[n];
  return 0;
}
";
    int res;
char *out_code = NULL;
size_t start_idx = 0;
size_t end_idx = 0;
size_t i;

res = tokenize(az_span_create_from_str((char *)src), &tokens);
ASSERT_EQ(0, res);

res = patch_list_init(&patches);
ASSERT_EQ(0, res);

/* Find `int arr[n];` tokens manually for testing */
for (i = 0; i < tokens->size; ++i) {
  if (tokens->tokens[i].kind == TOKEN_KEYWORD_INT) {
    size_t j = i + 1;
    while (j < tokens->size && tokens->tokens[j].kind == TOKEN_WHITESPACE)
      j++;
    if (j < tokens->size && tokens->tokens[j].kind == TOKEN_IDENTIFIER) {
      bool is_arr = false;
      token_matches_string(&tokens->tokens[j], "arr", &is_arr);
      if (is_arr) {
        start_idx = i;
        while (j < tokens->size && tokens->tokens[j].kind != TOKEN_SEMICOLON)
          j++;
        end_idx = j + 1;
        break;
      }
    }
  }
}
ASSERT(start_idx < end_idx);

res = weaver_vla_to_alloca(&patches, tokens, start_idx, end_idx, "int", "arr",
                           "n", false);
ASSERT_EQ(0, res);

res = patch_list_apply(&patches, tokens, &out_code);
ASSERT_EQ(0, res);

  ASSERT_STR_EQ(""
"int main() {
  int n = 10;
  int *arr = (int *)_alloca((n) * sizeof(int));
  return 0;
  }
  ", out_code);

      free(out_code);
  patch_list_free(&patches);
  free_token_list(tokens);
  PASS();
  }

  SUITE(weaver_suite) {
    RUN_TEST(test_weaver_wrap_ifdef_basic);
    RUN_TEST(test_weaver_wrap_ifdef_else);
    RUN_TEST(test_weaver_wrap_ifdef_invalid_args);
    RUN_TEST(test_weaver_inject_msvc_headers);
    RUN_TEST(test_weaver_vla_to_alloca);
  }

#endif /* TEST_WEAVER_H */
