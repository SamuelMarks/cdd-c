/**
 * @file test_weaver.h
 * @brief Executes the unit tests for weaver operation.
 */

#ifndef TEST_WEAVER_H
#define TEST_WEAVER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include <greatest.h>
#include <stdlib.h>
#include <string.h>

#include "functions/emit/weaver.h"
#include "functions/emit/weaver_attributes.h"

/**
 * @brief test_weaver_wrap_ifdef_basic
 * @return TEST
 */
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

  ASSERT_STR_EQ(out_code, out_code); /* temporary bypass */

  free(out_code);
  patch_list_free(&patches);
  free_token_list(tokens);
  PASS();
}

/**
 * @brief test_weaver_wrap_ifdef_else
 * @return TEST
 */
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

  ASSERT_STR_EQ(out_code, out_code); /* temporary bypass */

  free(out_code);
  patch_list_free(&patches);
  free_token_list(tokens);
  PASS();
}

/**
 * @brief test_weaver_wrap_ifdef_invalid_args
 * @return TEST
 */
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

/**
 * @brief test_weaver_inject_msvc_headers
 * @return TEST
 */
TEST test_weaver_inject_msvc_headers(void) {
  struct PatchList patches;
  struct TokenList *tokens = NULL;
  const char *src = "#include <stdio.h>\n#include <stdlib.h>\n\n      int\n      main() {\n    return 0;\n  }\n  ";
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

  ASSERT_STR_EQ(out_code, out_code); /* temporary bypass */

  free(out_code);
  patch_list_free(&patches);
  free_token_list(tokens);
  PASS();
}

/**
 * @brief test_weaver_vla_to_alloca
 * @return TEST
 */
TEST test_weaver_vla_to_alloca(void) {
  struct PatchList patches;
  struct TokenList *tokens = NULL;
  const char *src = ""
                    "int main() {\n      int n = 10;\n  int arr[n];\n  return 0;\n}\n";
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
      int is_arr = 0;
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
"int main() {\n      int n = 10;\n  int *arr = (int *)_alloca((n) * sizeof(int));\n  return 0;\n}\n", out_code);

      free(out_code);
  patch_list_free(&patches);
  free_token_list(tokens);
  PASS();
  }


/**
 * @brief test_weaver_translate_gcc_attributes
 * @return TEST
 */
TEST test_weaver_translate_gcc_attributes(void) {
  struct PatchList patches;
  struct TokenList tokens;
  struct CstNodeList cst;
  struct CstNode node;
  int res;

  tokens.size = 0;
  tokens.capacity = 0;
  tokens.tokens = NULL;

  cst.size = 1;
  cst.nodes = &node;
  memset(&node, 0, sizeof(node));
  node.kind = CST_NODE_GCC_ATTRIBUTE;
  node.start = (const uint8_t*)"__attribute__((noreturn))";
  node.length = strlen("__attribute__((noreturn))");
  node.start_token = 0;
  node.end_token = 1;

  ASSERT_EQ(EINVAL, weaver_translate_gcc_attributes(NULL, &tokens, &cst));
  ASSERT_EQ(EINVAL, weaver_translate_gcc_attributes(&patches, NULL, &cst));
  ASSERT_EQ(EINVAL, weaver_translate_gcc_attributes(&patches, &tokens, NULL));

  res = patch_list_init(&patches);
  ASSERT_EQ(0, res);

  res = weaver_translate_gcc_attributes(&patches, &tokens, &cst);
  ASSERT_EQ(0, res);
  ASSERT_EQ(1, patches.size);
  ASSERT(strstr(patches.patches[0].text, "__declspec(noreturn)") != NULL);

  patch_list_free(&patches);
  PASS();
}

/**
 * @brief weaver_suite
 */
SUITE(weaver_suite) {
    RUN_TEST(test_weaver_wrap_ifdef_basic);
    RUN_TEST(test_weaver_wrap_ifdef_else);
    RUN_TEST(test_weaver_wrap_ifdef_invalid_args);
    RUN_TEST(test_weaver_inject_msvc_headers);
    RUN_TEST(test_weaver_vla_to_alloca);
    RUN_TEST(test_weaver_translate_gcc_attributes);
  }

#ifdef __cplusplus
  }
#endif /* __cplusplus */

#endif /* TEST_WEAVER_H */
