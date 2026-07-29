#ifndef TEST_MACRO_OVERLAY_H
#define TEST_MACRO_OVERLAY_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/* clang-format off */
#include "c_cdd/memory.h"
#include "c_cdd_export.h"
#include "functions/parse/macro_overlay.h"
#include <greatest.h>
/* clang-format on */
TEST test_macro_overlay_basic(void) {
  struct MacroOverlayList list;
  struct CstNodeList cst;
  struct TokenList *tl = NULL;

  ASSERT_EQ(0, tokenize(az_span_create_from_str("int x;"), &tl));
  cst.size = 0;
  cst.capacity = 0;
  cst.nodes = NULL;

  (void)macro_overlay_list_init(&list);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cst_build_macro_overlay(NULL, tl, &list));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cst_build_macro_overlay(&cst, NULL, &list));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cst_build_macro_overlay(&cst, tl, NULL));

  ASSERT_EQ(0, cst_build_macro_overlay(&cst, tl, &list));

  macro_overlay_list_free(&list);
  free_token_list(tl);

  PASS();
}

TEST test_macro_overlay_null_args(void) {
  (void)macro_overlay_list_init(NULL);
  macro_overlay_list_free(NULL);

  PASS();
}

TEST test_macro_overlay_with_nodes(void) {
  struct MacroOverlayList list;
  struct CstNodeList cst;
  struct TokenList *tl = NULL;
  size_t i;

  ASSERT_EQ(0, tokenize(az_span_create_from_str("MACRO(x);"), &tl));
  cst.size = 1;
  cst.capacity = 1;
  cst.nodes = C_CDD_CALLOC(1, sizeof(struct CstNode));
  cst.nodes[0].kind = CST_NODE_MACRO;

  (void)macro_overlay_list_init(&list);
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
  cst.nodes = C_CDD_REALLOC(cst.nodes, 10 * sizeof(struct CstNode));
  for (i = 0; i < 10; i++) {
    cst.nodes[i].kind = CST_NODE_MACRO;
  }

  macro_overlay_list_free(&list);
  (void)macro_overlay_list_init(&list);
  ASSERT_EQ(0, cst_build_macro_overlay(&cst, tl, &list));
  ASSERT_EQ(10, list.size);

  macro_overlay_list_free(&list);

  /* Double free should be safe */
  macro_overlay_list_free(&list);

  C_CDD_FREE(cst.nodes);
  free_token_list(tl);

  PASS();
}

TEST test_macro_overlay_free_with_expanded(void) {
  struct MacroOverlayList list;
  struct CstNodeList *expanded;

  (void)macro_overlay_list_init(&list);

  expanded = C_CDD_CALLOC(1, sizeof(struct CstNodeList));
  expanded->nodes = C_CDD_CALLOC(1, sizeof(struct CstNode));
  expanded->size = 1;
  expanded->capacity = 1;

  /* Manually add a node to hit the free_cst_node_list branch */
  list.capacity = 1;
  list.size = 1;
  list.nodes = C_CDD_CALLOC(1, sizeof(struct MacroOverlayNode));
  list.nodes[0].expanded_ast = expanded;

  macro_overlay_list_free(&list);

  PASS();
}

SUITE(macro_overlay_suite) {
  RUN_TEST(test_macro_overlay_basic);
  RUN_TEST(test_macro_overlay_null_args);
  RUN_TEST(test_macro_overlay_with_nodes);
  RUN_TEST(test_macro_overlay_free_with_expanded);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_MACRO_OVERLAY_H */
