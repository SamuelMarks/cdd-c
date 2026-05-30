extern C_CDD_EXPORT int g_fail_io_after;
extern C_CDD_EXPORT int g_io_calls;
/**
 * @file test_cdd_cst_escape.h
 * @brief Unit tests for CST escape analysis.
 */

#ifndef TEST_CDD_CST_ESCAPE_H
#define TEST_CDD_CST_ESCAPE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include <greatest.h>
#include <errno.h>

#include "classes/parse/cdd_cst_escape.h"
/* clang-format on */

/**
 * @brief Tests basic functionality and error handling of CST escape analysis.
 *
 * @return The result of the test.
 */
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
  g_fail_io_after = -1;

  PASS();
}

/**
 * @brief CST escape analysis test suite.
 */
SUITE(cdd_cst_escape_suite) { RUN_TEST(test_cdd_cst_escape_basic); }

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !TEST_CDD_CST_ESCAPE_H */
