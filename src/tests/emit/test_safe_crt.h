/**
 * @file test_safe_crt.h
 * @brief Unit tests for Safe CRT transformations.
 */

#ifndef TEST_SAFE_CRT_H
#define TEST_SAFE_CRT_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd/memory.h"
#include "c_cdd_export.h"
#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "functions/emit/safe_crt.h"
#include "functions/parse/tokenizer.h"
/* clang-format on */

extern C_CDD_EXPORT int g_fail_io_after;
extern C_CDD_EXPORT int g_cdd_alloc_fail;
extern C_CDD_EXPORT int g_cdd_strdup_fail;

static cdd_c_error_t helper_generate_patches(const char *src,
                                             struct SafeCrtPatchList *patches,
                                             struct CstNodeList **out_nodes,
                                             struct TokenList **out_tokens) {
  az_span span;
  cdd_c_error_t rc;

  if (patches)
    memset(patches, 0, sizeof(*patches));
  *out_nodes = (struct CstNodeList *)calloc(1, sizeof(struct CstNodeList));
  *out_tokens = NULL;
  span = az_span_create((uint8_t *)(size_t)src, strlen(src));
  rc = tokenize(span, out_tokens);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = parse_tokens(*out_tokens, *out_nodes);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = safe_crt_patch_list_init(patches);
  if (rc != CDD_C_SUCCESS)
    return rc;
  return cst_generate_safe_crt_patches(*out_nodes, *out_tokens, patches);
}

static void helper_cleanup(struct SafeCrtPatchList *patches,
                           struct CstNodeList *nodes,
                           struct TokenList *tokens) {
  if (patches)
    safe_crt_patch_list_free(patches);
  if (nodes) {
    free_cst_node_list(nodes);
    free(nodes);
  }
  if (tokens)
    free_token_list(tokens);
}

TEST test_safe_crt_init_free(void) {
  struct SafeCrtPatchList patches;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, safe_crt_patch_list_init(NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, safe_crt_patch_list_free(NULL));

  ASSERT_EQ(CDD_C_SUCCESS, safe_crt_patch_list_init(&patches));
  ASSERT_EQ(0, patches.size);
  ASSERT_EQ(0, patches.capacity);
  ASSERT(patches.patches == NULL);

  ASSERT_EQ(CDD_C_SUCCESS, safe_crt_patch_list_free(&patches));
  ASSERT(patches.patches == NULL);

  memset(&patches, 0, sizeof(patches));
  patches.patches =
      (struct SafeCrtPatch *)calloc(1, sizeof(struct SafeCrtPatch));
  patches.size = 1;
  patches.capacity = 1;
  patches.patches[0].replacement_text = NULL;
  ASSERT_EQ(CDD_C_SUCCESS, safe_crt_patch_list_free(&patches));
  PASS();
}

TEST test_safe_crt_add_patch_unit(void) {
  struct SafeCrtPatchList list;
  size_t i;
  char buf[32];
  cdd_c_error_t rc;

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, safe_crt_add_patch(NULL, 0, 1, "a"));
  memset(&list, 0, sizeof(list));
  safe_crt_patch_list_init(&list);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            safe_crt_add_patch(&list, 0, 1, NULL));

  for (i = 0; i < 10; ++i) {
#if defined(_MSC_VER)
    sprintf_s(buf, sizeof(buf), "patch_%lu", (unsigned long)i);
#else
    sprintf(buf, "patch_%lu", (unsigned long)i);
#endif
    rc = safe_crt_add_patch(&list, i, i + 1, buf);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
  }
  ASSERT_EQ(10, list.size);
  ASSERT(list.capacity >= 16);

  safe_crt_patch_list_free(&list);
  PASS();
}

TEST test_safe_crt_extract_token_text_unit(void) {
  const char *src = "int x;";
  struct TokenList *tokens = NULL;
  az_span span = az_span_create((uint8_t *)(size_t)src, strlen(src));
  char *out = NULL;

  ASSERT_EQ(CDD_C_SUCCESS, tokenize(span, &tokens));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            safe_crt_extract_token_text(NULL, 0, 1, &out));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            safe_crt_extract_token_text(tokens, 0, 1, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            safe_crt_extract_token_text(tokens, 2, 1, &out));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            safe_crt_extract_token_text(tokens, 0, 9999, &out));

  ASSERT_EQ(CDD_C_SUCCESS, safe_crt_extract_token_text(tokens, 0, 1, &out));
  ASSERT_STR_EQ("int", out);
  C_CDD_FREE(out);

  free_token_list(tokens);
  PASS();
}

TEST test_safe_crt_direct_patch_generators_invalid(void) {
  struct SafeCrtPatchList patches;
  struct TokenList tokens;
  memset(&patches, 0, sizeof(patches));
  memset(&tokens, 0, sizeof(tokens));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            safe_crt_generate_strcpy_patch(NULL, 0, 1, &patches));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            safe_crt_generate_strcpy_patch(&tokens, 0, 1, NULL));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            safe_crt_generate_fopen_patch(NULL, 0, 1, &patches));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            safe_crt_generate_fopen_patch(&tokens, 0, 1, NULL));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            safe_crt_generate_strncpy_patch(NULL, 0, 1, &patches));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            safe_crt_generate_strncpy_patch(&tokens, 0, 1, NULL));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            safe_crt_generate_sprintf_patch(NULL, 0, 1, &patches));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            safe_crt_generate_sprintf_patch(&tokens, 0, 1, NULL));

  {
    const char *empty_src = "void f() {}";
    struct TokenList *tl = NULL;
    az_span sp =
        az_span_create((uint8_t *)(size_t)empty_src, strlen(empty_src));
    safe_crt_patch_list_init(&patches);
    (void)tokenize(sp, &tl);
    ASSERT_EQ(CDD_C_SUCCESS,
              safe_crt_generate_fopen_patch(tl, 0, tl->size, &patches));
    safe_crt_patch_list_free(&patches);
    free_token_list(tl);
  }
  PASS();
}

TEST test_safe_crt_cst_generate_args(void) {
  struct CstNodeList nodes;
  struct TokenList tokens;
  struct SafeCrtPatchList patches;

  memset(&nodes, 0, sizeof(nodes));
  memset(&tokens, 0, sizeof(tokens));
  memset(&patches, 0, sizeof(patches));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cst_generate_safe_crt_patches(NULL, &tokens, &patches));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cst_generate_safe_crt_patches(&nodes, NULL, &patches));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cst_generate_safe_crt_patches(&nodes, &tokens, NULL));
  PASS();
}

TEST test_safe_crt_strcpy(void) {
  const char *src = "void foo() {\n"
                    "  char dest[100];\n"
                    "  const char *src = \"hello\";\n"
                    "  str"
                    "cpy(dest, src);\n"
                    "}\n";
  struct TokenList *tokens = NULL;
  struct CstNodeList *nodes = NULL;
  struct SafeCrtPatchList patches;
  cdd_c_error_t rc;

  rc = helper_generate_patches(src, &patches, &nodes, &tokens);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, patches.size);
  ASSERT(strstr(patches.patches[0].replacement_text,
                "strcpy_s(dest, sizeof(dest),  src)") != NULL);

  helper_cleanup(&patches, nodes, tokens);
  PASS();
}

TEST test_safe_crt_strncpy(void) {
  const char *src = "void foo() {\n"
                    "  char dest[100];\n"
                    "  const char *src = \"hello\";\n"
                    "  str"
                    "ncpy(dest, src, 5);\n"
                    "}\n";
  struct TokenList *tokens = NULL;
  struct CstNodeList *nodes = NULL;
  struct SafeCrtPatchList patches;
  cdd_c_error_t rc;

  rc = helper_generate_patches(src, &patches, &nodes, &tokens);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, patches.size);
  ASSERT(strstr(patches.patches[0].replacement_text,
                "strncpy_s(dest, sizeof(dest),  src,  5)") != NULL);

  helper_cleanup(&patches, nodes, tokens);
  PASS();
}

TEST test_safe_crt_sprintf(void) {
  const char *src = "void foo() {\n"
                    "  char dest[100];\n"
                    "  spr"
                    "intf(dest, \"%d\", 42);\n"
                    "}\n";
  struct TokenList *tokens = NULL;
  struct CstNodeList *nodes = NULL;
  struct SafeCrtPatchList patches;
  cdd_c_error_t rc;

  rc = helper_generate_patches(src, &patches, &nodes, &tokens);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, patches.size);
  ASSERT(strstr(patches.patches[0].replacement_text,
                "sprintf_s(dest, sizeof(dest),  \"%d\", 42)") != NULL);

  helper_cleanup(&patches, nodes, tokens);
  PASS();
}

TEST test_safe_crt_fopen(void) {
  const char *src = "void foo() {\n"
                    "  FILE *f;\n"
                    "  f = fopen(\"test.txt\", \"r\");\n"
                    "}\n";
  struct TokenList *tokens = NULL;
  struct CstNodeList *nodes = NULL;
  struct SafeCrtPatchList patches;
  cdd_c_error_t rc;

  rc = helper_generate_patches(src, &patches, &nodes, &tokens);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, patches.size);
  ASSERT(strstr(patches.patches[0].replacement_text,
                "fopen_s(&f, \"test.txt\",  \"r\")") != NULL);

  helper_cleanup(&patches, nodes, tokens);
  PASS();
}

TEST test_safe_crt_vla(void) {
  const char *src = "void foo(int n) {\n"
                    "  char buf_c[n];\n"
                    "  int buf_i[n];\n"
                    "  double buf_d[n];\n"
                    "}\n";
  struct TokenList *tokens = NULL;
  struct CstNodeList *nodes = NULL;
  struct SafeCrtPatchList patches;
  cdd_c_error_t rc;

  rc = helper_generate_patches(src, &patches, &nodes, &tokens);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(3, patches.size);
  ASSERT(strstr(patches.patches[0].replacement_text,
                "char *buf_c = (char*)_alloca((n) * sizeof(char))") != NULL);
  ASSERT(strstr(patches.patches[1].replacement_text,
                "int *buf_i = (int*)_alloca((n) * sizeof(int))") != NULL);
  ASSERT(strstr(patches.patches[2].replacement_text,
                "double *buf_d = (double*)_alloca((n) * sizeof(double))") !=
         NULL);

  helper_cleanup(&patches, nodes, tokens);
  PASS();
}

TEST test_safe_crt_edge_cases_1(void) {
  const char *src = "void foo() {\n"
                    "  char dest[100];\n"
                    "  int nums[50];\n"
                    "  int no_lbrack = 1;\n"
                    "  int *p;\n"
                    "  char buf_space [n];\n"
                    "  char foobar[10];\n"
                    "  char foobar7[10];\n"
                    "  char foob5[10];\n"
                    "  str"
                    "cpy;\n"
                    "  str"
                    "cpy();\n"
                    "  str"
                    "cpy((dest), src);\n"
                    "  str"
                    "cpy(dest, src, extra);\n"
                    "  str"
                    "cpy(dest, (src));\n"
                    "  str"
                    "cpy(dest, src);\n"
                    "}\n";
  struct TokenList *tokens = NULL;
  struct CstNodeList *nodes = NULL;
  struct SafeCrtPatchList patches;
  cdd_c_error_t rc;

  rc = helper_generate_patches(src, &patches, &nodes, &tokens);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  helper_cleanup(&patches, nodes, tokens);
  PASS();
}

TEST test_safe_crt_edge_cases_2(void) {
  const char *src = "void foo() {\n"
                    "  str"
                    "ncpy;\n"
                    "  str"
                    "ncpy(dest);\n"
                    "  str"
                    "ncpy(dest, src);\n"
                    "  str"
                    "ncpy((dest), src, 1);\n"
                    "  str"
                    "ncpy(dest, (src), 1);\n"
                    "  str"
                    "ncpy(dest, src, (1));\n"
                    "  str"
                    "ncpy(dest, src, 1, 2);\n"
                    "  str"
                    "ncpy(dest, src, 1);\n"
                    "  spr"
                    "intf;\n"
                    "  spr"
                    "intf(dest);\n"
                    "  spr"
                    "intf((dest), \"%s\", 1);\n"
                    "  spr"
                    "intf(dest, \"%s\", (1));\n"
                    "  spr"
                    "intf(dest, \"%s\", 1);\n"
                    "}\n";
  struct TokenList *tokens = NULL;
  struct CstNodeList *nodes = NULL;
  struct SafeCrtPatchList patches;
  cdd_c_error_t rc;

  rc = helper_generate_patches(src, &patches, &nodes, &tokens);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  helper_cleanup(&patches, nodes, tokens);
  PASS();
}

TEST test_safe_crt_edge_cases_3(void) {
  const char *src = "void foo() {\n"
                    "  fopen;\n"
                    "  fopen(\"x\");\n"
                    "  f = fopen;\n"
                    "  f = fopen(\"x\");\n"
                    "  f = fopen((\"x\"), \"r\");\n"
                    "  f = fopen(\"x\", \"r\", \"extra\");\n"
                    "  f = fopen(\"x\", (\"r\"));\n"
                    "  f = fopen(\"x\", \"r\");\n"
                    "  { f = fopen(\"x\", \"r\"); }\n"
                    "  } f = fopen(\"x\", \"r\");\n"
                    "  = fopen(\"x\", \"r\");\n"
                    "  char incomplete[n\n"
                    "}\n"
                    "void bad1() { str"
                    "cpy }\n"
                    "void bad2() { str"
                    "ncpy }\n"
                    "void bad3() { spr"
                    "intf }\n"
                    "void bad4() { fopen }\n"
                    "void bad5() { int }\n"
                    "void bad6() { int x }\n"
                    "void bad7(int n) { int vla_no_semi[n] }\n"
                    "void bad8(int n) { int vla_no_rbracket[n }\n";
  struct TokenList *tokens = NULL;
  struct CstNodeList *nodes = NULL;
  struct SafeCrtPatchList patches;
  cdd_c_error_t rc;

  rc = helper_generate_patches(src, &patches, &nodes, &tokens);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  helper_cleanup(&patches, nodes, tokens);
  PASS();
}

TEST test_safe_crt_short_nodes(void) {
  struct TokenList *tokens = NULL;
  struct CstNodeList nodes;
  struct SafeCrtPatchList patches;
  struct CstNode n;
  const char *src = "int x;";
  az_span span = az_span_create((uint8_t *)(size_t)src, strlen(src));

  (void)tokenize(span, &tokens);
  memset(&nodes, 0, sizeof(nodes));
  memset(&patches, 0, sizeof(patches));
  safe_crt_patch_list_init(&patches);

  nodes.size = 1;
  nodes.capacity = 1;
  nodes.nodes = &n;

  /* node with only keyword: end_token == 1 */
  n.start_token = 0;
  n.end_token = 1;
  ASSERT_EQ(CDD_C_SUCCESS,
            cst_generate_safe_crt_patches(&nodes, tokens, &patches));

  /* node with keyword and identifier: end_token == 3 */
  n.start_token = 0;
  n.end_token = 3;
  ASSERT_EQ(CDD_C_SUCCESS,
            cst_generate_safe_crt_patches(&nodes, tokens, &patches));

  safe_crt_patch_list_free(&patches);
  free_token_list(tokens);
  PASS();
}

TEST test_safe_crt_direct_branches(void) {
  struct TokenList *tl = NULL;
  struct SafeCrtPatchList patches;
  az_span sp;

  safe_crt_patch_list_init(&patches);

  /* strcpy direct */
  sp = az_span_create((uint8_t *)(size_t) "strcpy, (a, b);", 15);
  (void)tokenize(sp, &tl);
  ASSERT_EQ(CDD_C_SUCCESS,
            safe_crt_generate_strcpy_patch(tl, 0, tl->size, &patches));
  free_token_list(tl);

  sp = az_span_create((uint8_t *)(size_t) "strcpy(a, b;", 12);
  (void)tokenize(sp, &tl);
  ASSERT_EQ(CDD_C_SUCCESS,
            safe_crt_generate_strcpy_patch(tl, 0, tl->size, &patches));
  free_token_list(tl);

  /* fopen: comma before lparen */
  sp = az_span_create((uint8_t *)(size_t) "fopen, (a, b);", 14);
  (void)tokenize(sp, &tl);
  ASSERT_EQ(CDD_C_SUCCESS,
            safe_crt_generate_fopen_patch(tl, 0, tl->size, &patches));
  free_token_list(tl);

  /* fopen: missing rparen */
  sp = az_span_create((uint8_t *)(size_t) "f = fopen(a, b;", 15);
  (void)tokenize(sp, &tl);
  ASSERT_EQ(CDD_C_SUCCESS,
            safe_crt_generate_fopen_patch(tl, 4, tl->size, &patches));
  free_token_list(tl);

  /* fopen: backward search stops at rbrace before finding = */
  sp = az_span_create((uint8_t *)(size_t) "; } fopen(a, b);", 16);
  (void)tokenize(sp, &tl);
  ASSERT_EQ(CDD_C_SUCCESS,
            safe_crt_generate_fopen_patch(tl, 4, tl->size, &patches));
  free_token_list(tl);

  /* fopen: no id before assign, stops at lbrace */
  sp = az_span_create((uint8_t *)(size_t) "{ = fopen(a, b);", 16);
  (void)tokenize(sp, &tl);
  ASSERT_EQ(CDD_C_SUCCESS,
            safe_crt_generate_fopen_patch(tl, 4, tl->size, &patches));
  free_token_list(tl);

  /* fopen: no id before assign, stops at rbrace */
  sp = az_span_create((uint8_t *)(size_t) "} = fopen(a, b);", 16);
  (void)tokenize(sp, &tl);
  ASSERT_EQ(CDD_C_SUCCESS,
            safe_crt_generate_fopen_patch(tl, 4, tl->size, &patches));
  free_token_list(tl);

  /* fopen: id_idx loop terminates on 0 with literal int */
  sp = az_span_create((uint8_t *)(size_t) "1 = fopen(a, b);", 16);
  (void)tokenize(sp, &tl);
  ASSERT_EQ(CDD_C_SUCCESS,
            safe_crt_generate_fopen_patch(tl, 4, tl->size, &patches));
  free_token_list(tl);

  /* strncpy direct */
  sp = az_span_create((uint8_t *)(size_t) "strncpy, (a, b, c);", 19);
  (void)tokenize(sp, &tl);
  ASSERT_EQ(CDD_C_SUCCESS,
            safe_crt_generate_strncpy_patch(tl, 0, tl->size, &patches));
  free_token_list(tl);

  sp = az_span_create((uint8_t *)(size_t) "strncpy(a, b, c;", 16);
  (void)tokenize(sp, &tl);
  ASSERT_EQ(CDD_C_SUCCESS,
            safe_crt_generate_strncpy_patch(tl, 0, tl->size, &patches));
  free_token_list(tl);

  /* sprintf direct */
  sp = az_span_create((uint8_t *)(size_t) "sprintf, (a, b);", 16);
  (void)tokenize(sp, &tl);
  ASSERT_EQ(CDD_C_SUCCESS,
            safe_crt_generate_sprintf_patch(tl, 0, tl->size, &patches));
  free_token_list(tl);

  sp = az_span_create((uint8_t *)(size_t) "sprintf(a, b;", 13);
  (void)tokenize(sp, &tl);
  ASSERT_EQ(CDD_C_SUCCESS,
            safe_crt_generate_sprintf_patch(tl, 0, tl->size, &patches));
  free_token_list(tl);

  safe_crt_patch_list_free(&patches);
  PASS();
}

TEST test_safe_crt_oom(void) {
  const char *src = "void foo(int n) {\n"
                    "  char dest[100];\n"
                    "  FILE *f;\n"
                    "  str"
                    "cpy(dest, \"a\");\n"
                    "  str"
                    "ncpy(dest, \"a\", 1);\n"
                    "  spr"
                    "intf(dest, \"%s\", \"a\");\n"
                    "  f = fopen(\"a\", \"r\");\n"
                    "  int vla[n];\n"
                    "}\n";
  struct TokenList *tokens = NULL;
  struct CstNodeList *nodes = NULL;
  az_span span;
  int fail_count;
  cdd_c_error_t rc;

  nodes = (struct CstNodeList *)calloc(1, sizeof(struct CstNodeList));
  span = az_span_create((uint8_t *)(size_t)src, strlen(src));
  rc = tokenize(span, &tokens);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = parse_tokens(tokens, nodes);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  for (fail_count = 1; fail_count <= 60; ++fail_count) {
    struct SafeCrtPatchList patches;
    memset(&patches, 0, sizeof(patches));
    safe_crt_patch_list_init(&patches);

    g_cdd_alloc_fail = fail_count;
    g_cdd_strdup_fail = fail_count;
    rc = cst_generate_safe_crt_patches(nodes, tokens, &patches);
    (void)rc;

    g_cdd_alloc_fail = 0;
    g_cdd_strdup_fail = 0;
    safe_crt_patch_list_free(&patches);
  }

  free_cst_node_list(nodes);
  free(nodes);
  free_token_list(tokens);
  PASS();
}

SUITE(safe_crt_suite) {
  RUN_TEST(test_safe_crt_init_free);
  RUN_TEST(test_safe_crt_add_patch_unit);
  RUN_TEST(test_safe_crt_extract_token_text_unit);
  RUN_TEST(test_safe_crt_direct_patch_generators_invalid);
  RUN_TEST(test_safe_crt_cst_generate_args);
  RUN_TEST(test_safe_crt_strcpy);
  RUN_TEST(test_safe_crt_strncpy);
  RUN_TEST(test_safe_crt_sprintf);
  RUN_TEST(test_safe_crt_fopen);
  RUN_TEST(test_safe_crt_vla);
  RUN_TEST(test_safe_crt_edge_cases_1);
  RUN_TEST(test_safe_crt_edge_cases_2);
  RUN_TEST(test_safe_crt_edge_cases_3);
  RUN_TEST(test_safe_crt_short_nodes);
  RUN_TEST(test_safe_crt_direct_branches);
  RUN_TEST(test_safe_crt_oom);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_SAFE_CRT_H */
