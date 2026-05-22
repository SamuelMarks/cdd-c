/**
 * @file test_cdd_cst_factory.h
 * @brief Unit tests for the CST factory.
 */

#ifndef TEST_CDD_CST_FACTORY_H
#define TEST_CDD_CST_FACTORY_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "classes/parse/cdd_cst_factory.h"
#include "classes/parse/cdd_cst_parser.h"
/* clang-format on */

/**
 * @brief Tests node allocation.
 * @return TEST
 */
TEST test_cst_alloc_node(void) {
  cdd_cst_node_t *node = NULL;

  ASSERT_EQ(EINVAL, cdd_cst_alloc_node(CDD_CST_DECLARATION, NULL));
  ASSERT_EQ(0, cdd_cst_alloc_node(CDD_CST_DECLARATION, &node));
  ASSERT(node != NULL);
  ASSERT_EQ(CDD_CST_DECLARATION, node->kind);

  cdd_cst_free_node_only(node);
  PASS();
}

/**
 * @brief Tests synthetic token creation tracking.
 * @return TEST
 */
TEST test_cst_create_token(void) {
  cdd_cst_tree_t *tree;
  cdd_token_t *tok = NULL;
  int rc;

  rc = cdd_cst_parse(az_span_create_from_str(""), &tree);
  ASSERT_EQ(0, rc);

  ASSERT_EQ(EINVAL,
            cdd_cst_create_token(NULL, CDD_TOKEN_IDENTIFIER, "foo", &tok));
  ASSERT_EQ(EINVAL,
            cdd_cst_create_token(tree, CDD_TOKEN_IDENTIFIER, "foo", NULL));

  rc = cdd_cst_create_token(tree, CDD_TOKEN_IDENTIFIER, "foo", &tok);
  ASSERT_EQ(0, rc);
  ASSERT(tok != NULL);
  ASSERT_STRN_EQ("foo", (const char *)tok->start, 3);
  ASSERT_EQ(3, tok->length);
  ASSERT_EQ(1, tree->num_synthesized);
  ASSERT_EQ(tok, tree->synthesized_tokens[0]);

  cdd_cst_tree_free(tree);
  PASS();
}

/**
 * @brief Tests appending node children.
 * @return TEST
 */
TEST test_cst_append_child_node(void) {
  cdd_cst_node_t *parent = NULL;
  cdd_cst_node_t *child = NULL;

  ASSERT_EQ(0, cdd_cst_alloc_node(CDD_CST_DECLARATION, &parent));
  ASSERT_EQ(0, cdd_cst_alloc_node(CDD_CST_IDENTIFIER, &child));

  ASSERT_EQ(EINVAL, cdd_cst_append_child_node(NULL, child));
  ASSERT_EQ(EINVAL, cdd_cst_append_child_node(parent, NULL));

  ASSERT_EQ(0, cdd_cst_append_child_node(parent, child));
  ASSERT_EQ(1, parent->num_children);
  ASSERT_EQ(CDD_CST_CHILD_NODE, parent->children[0].kind);
  ASSERT_EQ(child, parent->children[0].val.node);
  ASSERT_EQ(parent, child->parent);

  cdd_cst_free_node_only(child);
  cdd_cst_free_node_only(parent);
  PASS();
}

/**
 * @brief Tests appending token children.
 * @return TEST
 */
TEST test_cst_append_child_token(void) {
  cdd_cst_node_t *parent = NULL;
  cdd_cst_tree_t *tree;
  cdd_token_t *tok = NULL;

  ASSERT_EQ(0, cdd_cst_alloc_node(CDD_CST_DECLARATION, &parent));
  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str(""), &tree));
  ASSERT_EQ(0, cdd_cst_create_token(tree, CDD_TOKEN_IDENTIFIER, "x", &tok));

  ASSERT_EQ(EINVAL, cdd_cst_append_child_token(NULL, tok));
  ASSERT_EQ(EINVAL, cdd_cst_append_child_token(parent, NULL));

  ASSERT_EQ(0, cdd_cst_append_child_token(parent, tok));
  ASSERT_EQ(1, parent->num_children);
  ASSERT_EQ(CDD_CST_CHILD_TOKEN, parent->children[0].kind);
  ASSERT_EQ(tok, parent->children[0].val.token);

  cdd_cst_free_node_only(parent);
  cdd_cst_tree_free(tree);
  PASS();
}

/**
 * @brief Suite for CST factory
 */
/* old suite */

/**
 * @brief Tests parse format.
 * @return TEST
 */
TEST test_cst_parse_format(void) {
  cdd_cst_tree_t *tree = NULL;
  cdd_cst_node_t *node = NULL;
  int rc;

  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str(""), &tree));

  ASSERT_EQ(EINVAL, cdd_cst_parse_format(NULL, &node, "int x;"));
  ASSERT_EQ(EINVAL, cdd_cst_parse_format(tree, NULL, "int x;"));
  ASSERT_EQ(EINVAL, cdd_cst_parse_format(tree, &node, NULL));

  rc = cdd_cst_parse_format(tree, &node, "int a = %d;", 5);
  ASSERT_EQ(0, rc);
  ASSERT(node != NULL);

  cdd_cst_free_node_only(node);
  cdd_cst_tree_free(tree);
  PASS();
}

/**
 * @brief Test test_cst_parse_format is now in the suite
 */

SUITE(cdd_cst_factory_suite) {
  RUN_TEST(test_cst_alloc_node);
  RUN_TEST(test_cst_create_token);
  RUN_TEST(test_cst_append_child_node);
  RUN_TEST(test_cst_append_child_token);
  RUN_TEST(test_cst_parse_format);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CDD_CST_FACTORY_H */
