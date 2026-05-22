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
#include "classes/parse/cdd_cst_cfg.h"
#include "classes/parse/cdd_cst_parser.h"
#include <greatest.h>
/* clang-format on */

/**
 * @brief Tests basic functionality of the CFG generator.
 *
 * @return The result of the test.
 */
TEST test_cdd_cst_cfg_basic(void) {
  cdd_cst_tree_t *tree = NULL;
  cdd_cst_cfg_t *cfg = NULL;
  int rc;
  const char *src = "int main() { return 0; }";

  rc = cdd_cst_parse(az_span_create_from_str((char *)src), &tree);
  ASSERT_EQ(0, rc);

  cdd_cst_node_t *func = NULL;
  size_t i;
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

  PASS();
}

/**
 * @brief Tests error handling of the CFG APIs.
 *
 * @return The result of the test.
 */
TEST test_cdd_cst_cfg_errors(void) {
  cdd_cst_cfg_t *cfg = NULL;

  /* NULL pointers */
  ASSERT_EQ(EINVAL, cdd_cst_cfg_build(NULL, &cfg));

  /* Freeing NULL */
  cdd_cst_cfg_free(NULL);

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
  int rc;
  const char *src = "void main() { }";

  rc = cdd_cst_parse(az_span_create_from_str((char *)src), &tree);
  ASSERT_EQ(0, rc);

  cdd_cst_node_t *func = NULL;
  size_t i;
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
  int rc;
  const char *src = "void main() { int a = 5; }";

  rc = cdd_cst_parse(az_span_create_from_str((char *)src), &tree);
  ASSERT_EQ(0, rc);

  cdd_cst_node_t *func = NULL;
  size_t i;
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

  PASS();
}

/**
 * @brief CFG test suite.
 */
SUITE(cdd_cst_cfg_suite) {
  RUN_TEST(test_cdd_cst_cfg_no_return);
  RUN_TEST(test_cdd_cst_cfg_empty);
  RUN_TEST(test_cdd_cst_cfg_basic);
  RUN_TEST(test_cdd_cst_cfg_errors);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CDD_CST_CFG_H */
