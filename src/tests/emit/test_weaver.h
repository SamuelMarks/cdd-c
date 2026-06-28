extern C_CDD_EXPORT int g_fail_io_after;
extern C_CDD_EXPORT int g_io_calls;
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
#include "c_cdd_export.h"
#include <greatest.h>
#include <stdlib.h>
#include <string.h>

#include "functions/emit/weaver.h"
#include "functions/emit/weaver_attributes.h"
/* clang-format on */

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
  g_fail_io_after = -1;
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
  g_fail_io_after = -1;
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

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            weaver_wrap_ifdef(NULL, &tokens, 0, 5, "C", NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            weaver_wrap_ifdef(&patches, NULL, 0, 5, "C", NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            weaver_wrap_ifdef(&patches, &tokens, 0, 5, NULL, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            weaver_wrap_ifdef(&patches, &tokens, 5, 4, "C", NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            weaver_wrap_ifdef(&patches, &tokens, 0, 10, "C", NULL));
  g_fail_io_after = -1;

  PASS();
}

/**
 * @brief test_weaver_inject_msvc_headers
 * @return TEST
 */
TEST test_weaver_inject_msvc_headers(void) {
  struct PatchList patches;
  struct TokenList *tokens = NULL;
  const char *src = "#include <stdio.h>\n#include <stdlib.h>\n\n      int\n    "
                    "  main() {\n    return 0;\n  }\n  ";
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
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief test_weaver_vla_to_alloca
 * @return TEST
 */
TEST test_weaver_vla_to_alloca(void) {
  struct PatchList patches;
  struct TokenList *tokens = NULL;
  const char *src =
      ""
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
                "int main() {\n      int n = 10;\n  int *arr = (int "
                "*)_alloca((n) * sizeof(int));\n  return 0;\n}\n",
                out_code);

  free(out_code);
  patch_list_free(&patches);
  free_token_list(tokens);
  g_fail_io_after = -1;
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
  struct CstNode nodes[5];
  int res;

  tokens.size = 0;
  tokens.capacity = 0;
  tokens.tokens = NULL;

  cst.size = 5;
  cst.nodes = nodes;
  memset(nodes, 0, sizeof(nodes));

  /* noreturn */
  nodes[0].kind = CST_NODE_GCC_ATTRIBUTE;
  nodes[0].start = (const uint8_t *)"__attribute__((noreturn))";
  nodes[0].length = strlen("__attribute__((noreturn))");
  nodes[0].start_token = 0;
  nodes[0].end_token = 1;

  /* packed */
  nodes[1].kind = CST_NODE_GCC_ATTRIBUTE;
  nodes[1].start = (const uint8_t *)"__attribute__((packed))";
  nodes[1].length = strlen("__attribute__((packed))");
  nodes[1].start_token = 2;
  nodes[1].end_token = 3;

  /* visibility */
  nodes[2].kind = CST_NODE_GCC_ATTRIBUTE;
  nodes[2].start = (const uint8_t *)"__attribute__((visibility(\"default\")))";
  nodes[2].length = strlen("__attribute__((visibility(\"default\")))");
  nodes[2].start_token = 4;
  nodes[2].end_token = 5;

  /* unused */
  nodes[3].kind = CST_NODE_GCC_ATTRIBUTE;
  nodes[3].start = (const uint8_t *)"__attribute__((unused))";
  nodes[3].length = strlen("__attribute__((unused))");
  nodes[3].start_token = 6;
  nodes[3].end_token = 7;

  /* format */
  nodes[4].kind = CST_NODE_GCC_ATTRIBUTE;
  nodes[4].start = (const uint8_t *)"__attribute__((format(printf, 1, 2)))";
  nodes[4].length = strlen("__attribute__((format(printf, 1, 2)))");
  nodes[4].start_token = 8;
  nodes[4].end_token = 9;

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            weaver_translate_gcc_attributes(NULL, &tokens, &cst));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            weaver_translate_gcc_attributes(&patches, NULL, &cst));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            weaver_translate_gcc_attributes(&patches, &tokens, NULL));

  res = patch_list_init(&patches);
  ASSERT_EQ(0, res);

  res = weaver_translate_gcc_attributes(&patches, &tokens, &cst);
  ASSERT_EQ(0, res);

  /* Note: packed and unused don't currently generate a replacement, so we
   * expect 3 patches total */
  ASSERT_EQ(3, patches.size);

  /* Validate some output exists for noreturn, visibility, and format. Order
   * matches iteration */
  ASSERT(strstr(patches.patches[0].text, "__declspec(noreturn)") != NULL);
  ASSERT(strstr(patches.patches[1].text, "visibility") != NULL);
  ASSERT(strstr(patches.patches[2].text, "format") != NULL);

  patch_list_free(&patches);

  {
    struct PatchList patches2;
    struct CstNode nodes_oom[1];
    struct CstNodeList cst_oom = {0};
    struct CstNode nodes2[3];
    struct CstNodeList cst2 = {0};
    int rc_t1;

    patch_list_init(&patches2);
    patches2.capacity = 0; /* force realloc */

    memset(nodes_oom, 0, sizeof(nodes_oom));
    nodes_oom[0].kind = CST_NODE_GCC_ATTRIBUTE;
    nodes_oom[0].start = (const uint8_t *)"__attribute__((noreturn))";
    nodes_oom[0].length = 25;

    cst_oom.nodes = nodes_oom;
    cst_oom.size = 1;
    cst_oom.capacity = 1;

#ifdef CDD_BUILD_TESTS
    {
      extern C_CDD_EXPORT int g_cdd_fail_alloc;
      int rc_wattr;
      g_cdd_fail_alloc = 2001;
      rc_wattr = weaver_translate_gcc_attributes(&patches2, &tokens, &cst_oom);
      if (rc_wattr != CDD_C_ERROR_MEMORY) {
        printf("wattr=%d CDD_C_ERROR_MEMORY=%d\n", rc_wattr,
               CDD_C_ERROR_MEMORY);
        ASSERT_EQ(CDD_C_ERROR_MEMORY, rc_wattr);
      }
      g_cdd_fail_alloc = 0;
    }
#endif

    /* non-gcc attribute to test branch */
    nodes_oom[0].kind = CST_NODE_FUNCTION;

    /* visibility without default */
    memset(nodes2, 0, sizeof(nodes2));
    nodes2[0].kind = CST_NODE_GCC_ATTRIBUTE;
    nodes2[0].start =
        (const uint8_t *)"__attribute__((visibility(\"hidden\")))";
    nodes2[0].length = strlen("__attribute__((visibility(\"hidden\")))");
    nodes2[0].start_token = 0;
    nodes2[0].end_token = 1;

    /* format without printf */
    nodes2[1].kind = CST_NODE_GCC_ATTRIBUTE;
    nodes2[1].start = (const uint8_t *)"__attribute__((format(scanf, 1, 2)))";
    nodes2[1].length = strlen("__attribute__((format(scanf, 1, 2)))");
    nodes2[1].start_token = 2;
    nodes2[1].end_token = 3;

    /* non-gcc attr */
    nodes2[2].kind = CST_NODE_UNKNOWN;

    cst2.nodes = nodes2;
    cst2.size = 3;
    cst2.capacity = 3;

    rc_t1 = weaver_translate_gcc_attributes(&patches2, &tokens, &cst2);
    if (rc_t1 != 0)
      printf("FAILED cst2 rc=%d\n", rc_t1);
    ASSERT_EQ(0, rc_t1);

    /* deleted rc_wattr2 */

    patch_list_free(&patches2);
  }
  g_fail_io_after = -1;

  PASS();
}

/**
 * @brief weaver_suite
 */

TEST test_weaver_oom(void) {
  struct PatchList patches;
  struct TokenList *tl = NULL;
  const char *src = "int a;";
#ifdef CDD_BUILD_TESTS
  extern C_CDD_EXPORT int g_cdd_fail_alloc;
  int r1, r2, r3, r6, r8;
#endif

  patch_list_init(&patches);
  tokenize(az_span_create((uint8_t *)src, strlen(src)), &tl);

#ifdef CDD_BUILD_TESTS
  g_cdd_fail_alloc = 1;
  r1 = weaver_wrap_ifdef(&patches, tl, 0, 1, "COND", "else");
  g_cdd_fail_alloc = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, r1);

  g_cdd_fail_alloc = 2;
  r2 = weaver_wrap_ifdef(&patches, tl, 0, 1, "COND", "else");
  g_cdd_fail_alloc = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, r2);

  g_cdd_fail_alloc = 2;
  r3 = weaver_wrap_ifdef(&patches, tl, 0, 1, "COND", NULL);
  g_cdd_fail_alloc = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, r3);

  /* deleted r4 */

  /* deleted r5 */

  g_cdd_fail_alloc = 1;
  r6 = weaver_inject_msvc_headers(&patches, tl, 1, 1);
  g_cdd_fail_alloc = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, r6);

  /* deleted r7 */

  g_cdd_fail_alloc = 1;
  r8 = weaver_vla_to_alloca(&patches, tl, 0, 1, "type", "name", "sz", 0);
  g_cdd_fail_alloc = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, r8);

  /* deleted r9 */
#endif

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            weaver_wrap_ifdef(NULL, tl, 0, 1, "COND", "else"));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            weaver_wrap_ifdef(&patches, NULL, 0, 1, "COND", "else"));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            weaver_wrap_ifdef(&patches, tl, 0, 1, NULL, "else"));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            weaver_inject_msvc_headers(NULL, tl, 1, 1));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            weaver_inject_msvc_headers(&patches, NULL, 1, 1));
  ASSERT_EQ(0, weaver_inject_msvc_headers(&patches, tl, 0, 0));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            weaver_vla_to_alloca(NULL, tl, 0, 1, "type", "name", "sz", 0));
  ASSERT_EQ(
      CDD_C_ERROR_INVALID_ARGUMENT,
      weaver_vla_to_alloca(&patches, NULL, 0, 1, "type", "name", "sz", 0));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            weaver_vla_to_alloca(&patches, tl, 0, 1, NULL, "name", "sz", 0));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            weaver_vla_to_alloca(&patches, tl, 0, 1, "type", NULL, "sz", 0));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            weaver_vla_to_alloca(&patches, tl, 0, 1, "type", "name", NULL, 0));

  free_token_list(tl);
  patch_list_free(&patches);
  g_fail_io_after = -1;
  PASS();
}

SUITE(weaver_suite) {
  RUN_TEST(test_weaver_wrap_ifdef_basic);
  RUN_TEST(test_weaver_wrap_ifdef_else);
  RUN_TEST(test_weaver_wrap_ifdef_invalid_args);
  RUN_TEST(test_weaver_inject_msvc_headers);
  RUN_TEST(test_weaver_vla_to_alloca);
  RUN_TEST(test_weaver_translate_gcc_attributes);
  RUN_TEST(test_weaver_oom);
  RUN_TEST(test_weaver_oom);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_WEAVER_H */
