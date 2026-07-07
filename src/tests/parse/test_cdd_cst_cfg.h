/**
 * @file test_cdd_cst_cfg.h
 * @brief Unit tests for CST Control Flow Graph generator.
 */

#ifndef TEST_CDD_CST_CFG_H
#define TEST_CDD_CST_CFG_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include "classes/parse/cdd_cst_cfg.h"
#include "classes/parse/cdd_cst_parser.h"
#include <greatest.h>
/* clang-format on */
/* LCOV_EXCL_START */

/**
 * @brief Tests basic functionality of the CFG generator.
 *
 * @return The result of the test.
 */
TEST test_cdd_cst_cfg_basic(void) {
  cdd_cst_tree_t *tree = NULL;
  size_t i;
  cdd_cst_node_t *func = NULL;
  cdd_cst_cfg_t *cfg = NULL;
  int rc;
  const char *src = "int main() { return 0; }";

  rc = cdd_cst_parse(az_span_create_from_str((char *)src), &tree);
  ASSERT_EQ(0, rc);

  for (i = 0; i < tree->root->num_children; i++) {
    if (tree->root->children[i].kind == CDD_CST_CHILD_NODE) {
      if (tree->root->children[i].val.node->kind ==
          CDD_CST_FUNCTION_DEFINITION) {
        func = tree->root->children[i].val.node;
        break;
      }
    }
  }
  ASSERT(func != NULL);

  rc = cdd_cst_cfg_build(func, &cfg);

  ASSERT_EQ(0, rc);
  ASSERT(cfg != NULL);

  cdd_cst_cfg_free(cfg);
  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;

  PASS();
}

/**
 * @brief Tests error handling of the CFG APIs.
 *
 * @return The result of the test.
 */

#ifdef CDD_BUILD_TESTS
extern C_CDD_EXPORT int g_cdd_cfg_alloc_fail;
#endif

TEST test_cdd_cst_cfg_oom(void) {
  cdd_cst_tree_t *tree = NULL;
  cdd_cst_node_t *func = NULL;
  cdd_cst_cfg_t *cfg = NULL;
  int rc;
  const char *src = "int main() { return 0; }";

  rc = cdd_cst_parse(az_span_create_from_str((char *)src), &tree);
  ASSERT_EQ(0, rc);
  func = tree->root;

  /* Missing CDD_C_ERROR_INVALID_ARGUMENT tests */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_cst_cfg_build(func, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_cst_cfg_build(NULL, &cfg));

#ifdef CDD_BUILD_TESTS
  g_cdd_cfg_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_cst_cfg_build(func, &cfg));
  g_cdd_cfg_alloc_fail = 0;
#endif

  {
    const char *long_src = "int main() { if(1){} if(1){} if(1){} if(1){} "
                           "if(1){} if(1){} if(1){} if(1){} if(1){} if(1){} "
                           "if(1){} if(1){} if(1){} if(1){} if(1){} if(1){} }";
    cdd_cst_tree_t *t2 = NULL;
    cdd_cst_cfg_t *cfg2 = NULL;
    rc = cdd_cst_parse(az_span_create_from_str((char *)long_src), &t2);
    ASSERT_EQ(0, rc);
    rc = cdd_cst_cfg_build(t2->root->children[0].val.node, &cfg2);
    ASSERT_EQ(0, rc);
    cdd_cst_cfg_free(cfg2);
    cdd_cst_tree_free(t2);
  }
  {
    extern C_CDD_EXPORT int g_cdd_cfg_alloc_fail;
    int i;
    const char *ret_src = "int f() { return 0; }";
    cdd_cst_tree_t *ret_t = NULL;
    cdd_cst_cfg_t *ret_cfg = NULL;
    cdd_cst_parse(az_span_create_from_str((char *)ret_src), &ret_t);
    for (i = 1; i < 100; ++i) {
      g_cdd_cfg_alloc_fail = i;
      rc = cdd_cst_cfg_build(ret_t->root->children[0].val.node, &ret_cfg);
      if (rc == 0) {
        cdd_cst_cfg_free(ret_cfg);
        break;
      }
    }
    g_cdd_cfg_alloc_fail = 0;
    cdd_cst_tree_free(ret_t);
  }

  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}
TEST test_cdd_cst_cfg_errors(void) {
  cdd_cst_cfg_t *cfg = NULL;

  /* NULL pointers */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_cst_cfg_build(NULL, &cfg));

  /* Freeing NULL */
  cdd_cst_cfg_free(NULL);
  g_fail_io_after = -1;

  PASS();
}

/**
 * @brief Tests CFG generator on an empty function.
 *
 * @return The result of the test.
 */
TEST test_cdd_cst_cfg_empty(void) {
  cdd_cst_tree_t *tree = NULL;
  cdd_cst_cfg_t *cfg = NULL;
  size_t i;
  cdd_cst_node_t *func = NULL;
  int rc;
  const char *src = "void main() { }";

  rc = cdd_cst_parse(az_span_create_from_str((char *)src), &tree);
  ASSERT_EQ(0, rc);

  for (i = 0; i < tree->root->num_children; i++) {
    if (tree->root->children[i].kind == CDD_CST_CHILD_NODE) {
      if (tree->root->children[i].val.node->kind ==
          CDD_CST_FUNCTION_DEFINITION) {
        func = tree->root->children[i].val.node;
        break;
      }
    }
  }
  ASSERT(func != NULL);

  rc = cdd_cst_cfg_build(func, &cfg);
  ASSERT_EQ(0, rc);
  ASSERT(cfg != NULL);

  cdd_cst_cfg_free(cfg);
  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;

  PASS();
}

/**
 * @brief Tests CFG generator on a function without a return statement.
 *
 * @return The result of the test.
 */
TEST test_cdd_cst_cfg_no_return(void) {
  cdd_cst_tree_t *tree = NULL;
  cdd_cst_cfg_t *cfg = NULL;
  size_t i;
  cdd_cst_node_t *func = NULL;
  int rc;
  const char *src = "void main() { int a = 5; }";

  rc = cdd_cst_parse(az_span_create_from_str((char *)src), &tree);
  ASSERT_EQ(0, rc);

  for (i = 0; i < tree->root->num_children; i++) {
    if (tree->root->children[i].kind == CDD_CST_CHILD_NODE) {
      if (tree->root->children[i].val.node->kind ==
          CDD_CST_FUNCTION_DEFINITION) {
        func = tree->root->children[i].val.node;
        break;
      }
    }
  }
  ASSERT(func != NULL);

  rc = cdd_cst_cfg_build(func, &cfg);
  ASSERT_EQ(0, rc);
  ASSERT(cfg != NULL);

  cdd_cst_cfg_free(cfg);
  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;

  PASS();
}

TEST test_cdd_cst_cfg_extra(void) {
  /* Internal functions alloc_block and add_edge are static.
   * To achieve 100% coverage, we trigger failures via public API using OOM
   * mocks.
   */
  cdd_cst_tree_t *tree = NULL;
  cdd_cst_node_t *func = NULL;
  cdd_cst_cfg_t *cfg = NULL;
  int rc;
  const char *src = "int main() { if (1) { return 0; } else { return 1; } }";

  rc = cdd_cst_parse(az_span_create_from_str((char *)src), &tree);
  ASSERT_EQ(0, rc);
  func = tree->root->children[0].val.node; /* get function node */

#ifdef CDD_BUILD_TESTS
  {
    extern C_CDD_EXPORT int g_cdd_cfg_alloc_fail;
    int i;
    for (i = 1; i < 100; ++i) {
      g_cdd_cfg_alloc_fail = i;
      rc = cdd_cst_cfg_build(func, &cfg);
      if (rc == 0) {
        cdd_cst_cfg_free(cfg);
        break;
      }
      ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
    }
    g_cdd_cfg_alloc_fail = 0;
  }
#endif

  {
    const char *long_src = "int main() { if(1){} if(1){} if(1){} if(1){} "
                           "if(1){} if(1){} if(1){} if(1){} if(1){} if(1){} "
                           "if(1){} if(1){} if(1){} if(1){} if(1){} if(1){} }";
    cdd_cst_tree_t *t2 = NULL;
    cdd_cst_cfg_t *cfg2 = NULL;
    rc = cdd_cst_parse(az_span_create_from_str((char *)long_src), &t2);
    ASSERT_EQ(0, rc);
    rc = cdd_cst_cfg_build(t2->root->children[0].val.node, &cfg2);
    ASSERT_EQ(0, rc);
    cdd_cst_cfg_free(cfg2);
    cdd_cst_tree_free(t2);
  }
  {
    extern C_CDD_EXPORT int g_cdd_cfg_alloc_fail;
    int i;
    const char *ret_src = "int f() { return 0; }";
    cdd_cst_tree_t *ret_t = NULL;
    cdd_cst_cfg_t *ret_cfg = NULL;
    cdd_cst_parse(az_span_create_from_str((char *)ret_src), &ret_t);
    for (i = 1; i < 100; ++i) {
      g_cdd_cfg_alloc_fail = i;
      rc = cdd_cst_cfg_build(ret_t->root->children[0].val.node, &ret_cfg);
      if (rc == 0) {
        cdd_cst_cfg_free(ret_cfg);
        break;
      }
    }
    g_cdd_cfg_alloc_fail = 0;
    cdd_cst_tree_free(ret_t);
  }

  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief CFG test suite.
 */
SUITE(cdd_cst_cfg_suite) {
  RUN_TEST(test_cdd_cst_cfg_extra);
  RUN_TEST(test_cdd_cst_cfg_no_return);
  RUN_TEST(test_cdd_cst_cfg_empty);
  RUN_TEST(test_cdd_cst_cfg_basic);
  RUN_TEST(test_cdd_cst_cfg_errors);
  RUN_TEST(test_cdd_cst_cfg_oom);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CDD_CST_CFG_H */

/* LCOV_EXCL_STOP */
