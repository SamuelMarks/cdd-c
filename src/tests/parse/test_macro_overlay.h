#ifndef TEST_MACRO_OVERLAY_H
#define TEST_MACRO_OVERLAY_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "functions/parse/macro_overlay.h"
#include <greatest.h>

TEST test_macro_overlay_basic(void) {
  struct MacroOverlayList list;
  struct CstNodeList cst;
  struct TokenList *tl = NULL;

  ASSERT_EQ(0, tokenize(az_span_create_from_str("int x;"), &tl));
  cst.size = 0;
  cst.capacity = 0;
  cst.nodes = NULL;

  macro_overlay_list_init(&list);
  ASSERT_EQ(EINVAL, cst_build_macro_overlay(NULL, tl, &list));
  ASSERT_EQ(EINVAL, cst_build_macro_overlay(&cst, NULL, &list));
  ASSERT_EQ(EINVAL, cst_build_macro_overlay(&cst, tl, NULL));

  ASSERT_EQ(0, cst_build_macro_overlay(&cst, tl, &list));

  macro_overlay_list_free(&list);
  free_token_list(tl);
  PASS();
}

SUITE(macro_overlay_suite) { RUN_TEST(test_macro_overlay_basic); }

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_MACRO_OVERLAY_H */
