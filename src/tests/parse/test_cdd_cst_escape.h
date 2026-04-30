/* clang-format off */
#ifndef TEST_CDD_CST_ESCAPE_H
#define TEST_CDD_CST_ESCAPE_H

#include <greatest.h>
#include "../../classes/parse/cdd_cst_escape.h"
/* clang-format on */

TEST test_cdd_cst_escape_basic(void) {
  int escapes = -1;
  cdd_cst_tree_t tree = {0};
  cdd_cst_scope_env_t env = {0};
  cdd_cst_symbol_t sym = {0};

  ASSERT_EQ(EINVAL, cdd_cst_analyze_escapes(NULL, &env));
  ASSERT_EQ(EINVAL, cdd_cst_analyze_escapes(&tree, NULL));
  ASSERT_EQ(0, cdd_cst_analyze_escapes(&tree, &env));

  ASSERT_EQ(EINVAL, cdd_cst_symbol_escapes(NULL, &escapes));
  ASSERT_EQ(EINVAL, cdd_cst_symbol_escapes(&sym, NULL));
  ASSERT_EQ(0, cdd_cst_symbol_escapes(&sym, &escapes));
  ASSERT_EQ(0, escapes);

  PASS();
}

SUITE(cdd_cst_escape_suite) { RUN_TEST(test_cdd_cst_escape_basic); }

#endif /* !TEST_CDD_CST_ESCAPE_H */
