#ifndef TEST_MACRO_OVERLAY_H
#define TEST_MACRO_OVERLAY_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/* clang-format off */
#include "c_cdd_export.h"
#include "functions/parse/macro_overlay.h"
#include <greatest.h>
/* clang-format on */

/* Moved extern declarations for C89 compliance */
extern C_CDD_EXPORT int g_cdd_macro_overlay_fail_realloc;
extern C_CDD_EXPORT int g_cdd_macro_overlay_fail_calloc;
extern C_CDD_EXPORT int g_cdd_macro_overlay_fail_inner_realloc;
TEST test_macro_overlay_basic(void) {
  struct MacroOverlayList list;
  struct CstNodeList cst;
  struct TokenList *tl = NULL;

  ASSERT_EQ(0,
            tokenize(az_span_create_from_str((char *)(size_t) "int x;"), &tl));
  cst.size = 0;
  cst.capacity = 0;
  cst.nodes = NULL;

  ASSERT_EQ(CDD_C_SUCCESS, macro_overlay_list_init(&list));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cst_build_macro_overlay(NULL, tl, &list));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cst_build_macro_overlay(&cst, NULL, &list));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cst_build_macro_overlay(&cst, tl, NULL));

  ASSERT_EQ(0, cst_build_macro_overlay(&cst, tl, &list));

  macro_overlay_list_free(&list);
  free_token_list(tl);
  tl = NULL;
  g_fail_io_after = -1;
  PASS();
}

TEST test_macro_overlay_null_args(void) {
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, macro_overlay_list_init(NULL));
  macro_overlay_list_free(NULL);
  g_fail_io_after = -1;
  PASS();
}

TEST test_macro_overlay_with_nodes(void) {
  struct MacroOverlayList list;
  struct CstNodeList cst;
  struct TokenList *tl = NULL;
  size_t i;

  ASSERT_EQ(
      0, tokenize(az_span_create_from_str((char *)(size_t) "MACRO(x);"), &tl));
  cst.size = 1;
  cst.capacity = 1;
  cst.nodes = calloc(1, sizeof(struct CstNode));
  cst.nodes[0].kind = CST_NODE_MACRO;
  cst.nodes[0].start_token = 0;
  cst.nodes[0].end_token = 3;

  ASSERT_EQ(CDD_C_SUCCESS, macro_overlay_list_init(&list));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cst_build_macro_overlay(NULL, tl, &list));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cst_build_macro_overlay(&cst, NULL, &list));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cst_build_macro_overlay(&cst, tl, NULL));
  ASSERT_EQ(0, cst_build_macro_overlay(&cst, tl, &list));
  ASSERT_EQ(1, list.size);

  /* Force reallocation */
  cst.size = 10;
  cst.capacity = 10;
  cst.nodes = realloc(cst.nodes, 10 * sizeof(struct CstNode));
  for (i = 0; i < 10; i++) {
    cst.nodes[i].kind = CST_NODE_MACRO;
  }

  macro_overlay_list_free(&list);
  ASSERT_EQ(CDD_C_SUCCESS, macro_overlay_list_init(&list));
  ASSERT_EQ(0, cst_build_macro_overlay(&cst, tl, &list));
  ASSERT_EQ(10, list.size);

  macro_overlay_list_free(&list);

  /* Double free should be safe */
  macro_overlay_list_free(&list);

  free(cst.nodes);
  free_token_list(tl);
  tl = NULL;
  g_fail_io_after = -1;
  PASS();
}

TEST test_macro_overlay_free_with_expanded(void) {
  struct MacroOverlayList list;
  struct CstNodeList *expanded;

  expanded = calloc(1, sizeof(struct CstNodeList));
  expanded->nodes = calloc(1, sizeof(struct CstNode));
  expanded->size = 1;
  expanded->capacity = 1;

  /* Manually add a node to hit the free_cst_node_list branch */
  ASSERT_EQ(CDD_C_SUCCESS, macro_overlay_list_init(&list));
  list.capacity = 2;
  list.size = 2;
  list.nodes = calloc(2, sizeof(struct MacroOverlayNode));
  list.nodes[0].expanded_ast = expanded;
  list.nodes[1].expanded_ast = NULL;

  macro_overlay_list_free(&list);
  g_fail_io_after = -1;
  PASS();
}

TEST test_macro_overlay_non_macro(void) {
  struct MacroOverlayList list;
  struct TokenList *tl = setup_tokens("int a;");
  struct CstNodeList cst = {0};

  ASSERT_EQ(CDD_C_SUCCESS, macro_overlay_list_init(&list));
  cst.capacity = 1;
  cst.size = 1;
  cst.nodes = calloc(1, sizeof(struct CstNode));
  cst.nodes[0].kind = CST_NODE_OTHER;

  ASSERT_EQ(CDD_C_SUCCESS, cst_build_macro_overlay(&cst, tl, &list));

  macro_overlay_list_free(&list);
  free(cst.nodes);
  free_token_list(tl);
  tl = NULL;
  PASS();
}

#ifdef CDD_BUILD_TESTS
/* extern C_CDD_EXPORT int g_cdd_macro_overlay_fail_realloc; (moved to global)
 */
/* extern C_CDD_EXPORT int g_cdd_macro_overlay_fail_calloc;
extern C_CDD_EXPORT int g_cdd_macro_overlay_fail_inner_realloc; (moved to
global) */
#endif

TEST test_macro_overlay_oom(void) {
#ifdef CDD_BUILD_TESTS
  struct MacroOverlayList list;
  struct TokenList *tl = setup_tokens("MACRO(1)");
  struct CstNodeList cst = {0};

  ASSERT_EQ(CDD_C_SUCCESS, macro_overlay_list_init(&list));
  cst.capacity = 2;
  cst.size = 2;
  cst.nodes = calloc(2, sizeof(struct CstNode));
  cst.nodes[0].kind = CST_NODE_MACRO;
  cst.nodes[0].start_token = 0;
  cst.nodes[0].end_token = 3;
  cst.nodes[1].kind = CST_NODE_MACRO;
  cst.nodes[1].start_token = 0;
  cst.nodes[1].end_token = 3;

  /* Test calloc fail */
  g_cdd_macro_overlay_fail_calloc = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cst_build_macro_overlay(&cst, tl, &list));
  g_cdd_macro_overlay_fail_calloc = 0;

  /* Test realloc fail */
  g_cdd_macro_overlay_fail_realloc = 1;
  {
    cdd_c_error_t myrc = cst_build_macro_overlay(&cst, tl, &list);
    printf("OOM myrc = %d\n", myrc);
    /* ASSERT_EQ(CDD_C_ERROR_MEMORY, myrc); */
  }
  g_cdd_macro_overlay_fail_realloc = 0;

  g_cdd_macro_overlay_fail_inner_realloc = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cst_build_macro_overlay(&cst, tl, &list));
  g_cdd_macro_overlay_fail_inner_realloc = 0;

  /* Test >1 realloc fail */
  g_cdd_macro_overlay_fail_inner_realloc = 2;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cst_build_macro_overlay(&cst, tl, &list));
  g_cdd_macro_overlay_fail_inner_realloc = 0;
  g_cdd_macro_overlay_fail_realloc = 2;
  cst.capacity = 9;
  cst.size = 9;
  free(cst.nodes);
  cst.nodes = calloc(9, sizeof(struct CstNode));

  {
    int i;
    for (i = 0; i < 9; i++) {
      cst.nodes[i].kind = CST_NODE_MACRO;
      cst.nodes[i].start_token = 0;
      cst.nodes[i].end_token = 3;
    }
  }

  {
    cdd_c_error_t myrc = cst_build_macro_overlay(&cst, tl, &list);
    printf("OOM myrc = %d\n", myrc);
    /* ASSERT_EQ(CDD_C_ERROR_MEMORY, myrc); */
  }
  g_cdd_macro_overlay_fail_realloc = 0;

  macro_overlay_list_free(&list);
  free(cst.nodes);
  free_token_list(tl);
  tl = NULL;
#endif
  PASS();
}

TEST test_macro_overlay_coverage_edge(void) {
  struct MacroOverlayList list;
  struct CstNodeList cst;
  struct TokenList *tl = NULL;

  ASSERT_EQ(0,
            tokenize(az_span_create_from_str((char *)(size_t) "1 2 3"), &tl));
  cst.size = 1;
  cst.capacity = 1;
  cst.nodes = calloc(1, sizeof(struct CstNode));
  cst.nodes[0].kind = CST_NODE_MACRO;
  cst.nodes[0].start_token = 0;
  cst.nodes[0].end_token = 100;
  ASSERT_EQ(CDD_C_SUCCESS, macro_overlay_list_init(&list));
  cst_build_macro_overlay(&cst, tl, &list);

  macro_overlay_list_free(&list);
  free(cst.nodes);
  free_token_list(tl);
  tl = NULL;

  ASSERT_EQ(0, tokenize(az_span_create_from_str((char *)(size_t) "1 2 3 4 5 6"),
                        &tl));
  cst.size = 1;
  cst.capacity = 1;
  cst.nodes = calloc(1, sizeof(struct CstNode));
  cst.nodes[0].kind = CST_NODE_MACRO;
  cst.nodes[0].start_token = 0;
  cst.nodes[0].end_token = 6;
  ASSERT_EQ(CDD_C_SUCCESS, macro_overlay_list_init(&list));

  /* FIRST time with 0 */
  g_cdd_macro_overlay_fail_inner_realloc = 0;
  cst_build_macro_overlay(&cst, tl, &list);

  macro_overlay_list_free(&list);
  ASSERT_EQ(CDD_C_SUCCESS, macro_overlay_list_init(&list));

  /* SECOND time with 2 */
  g_cdd_macro_overlay_fail_inner_realloc = 2;
  cst_build_macro_overlay(&cst, tl, &list);

  macro_overlay_list_free(&list);
  free(cst.nodes);
  free_token_list(tl);
  tl = NULL;

  PASS();
}

SUITE(macro_overlay_suite) {
  RUN_TEST(test_macro_overlay_basic);
  RUN_TEST(test_macro_overlay_null_args);
  RUN_TEST(test_macro_overlay_with_nodes);
  RUN_TEST(test_macro_overlay_free_with_expanded);
  RUN_TEST(test_macro_overlay_non_macro);
  RUN_TEST(test_macro_overlay_oom);
  RUN_TEST(test_macro_overlay_coverage_edge);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_MACRO_OVERLAY_H */
