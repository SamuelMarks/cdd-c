extern C_CDD_EXPORT int g_fail_io_after;
extern C_CDD_EXPORT int g_io_calls;
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
#include "c_cdd_export.h"
#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "classes/parse/cdd_cst_factory.h"
#include "classes/parse/cdd_cst_parser.h"
/* clang-format on */
/* LCOV_EXCL_START */

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
  g_fail_io_after = -1;
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

  /* Force realloc in track_synthesized (default cap is something big, 128?
   * Let's check: 128) */
  {
    size_t i;
    for (i = 0; i < 130; i++) {
      cdd_token_t *extra_tok = NULL;
      ASSERT_EQ(0, cdd_cst_create_token(tree, CDD_TOKEN_IDENTIFIER, "foo",
                                        &extra_tok));
    }
    ASSERT_EQ(131, tree->num_synthesized);
  }

  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
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

  /* Force realloc by exceeding initial capacity (8) */
  {
    size_t i;
    for (i = 0; i < 15; i++) {
      cdd_cst_node_t *extra_child = NULL;
      ASSERT_EQ(0, cdd_cst_alloc_node(CDD_CST_IDENTIFIER, &extra_child));
      ASSERT_EQ(0, cdd_cst_append_child_node(parent, extra_child));
    }
    ASSERT_EQ(16, parent->num_children);
  }

  /* free_node_only won't recursively free the extra children, we must free them
   * to avoid leaks */
  {
    size_t i;
    for (i = 1; i < parent->num_children; i++) {
      cdd_cst_free_node_only(parent->children[i].val.node);
    }
  }

  cdd_cst_free_node_only(child);
  cdd_cst_free_node_only(parent);
  g_fail_io_after = -1;
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

  /* Force realloc by exceeding initial capacity (8) */
  {
    size_t i;
    for (i = 0; i < 15; i++) {
      ASSERT_EQ(0, cdd_cst_append_child_token(parent, tok));
    }
    ASSERT_EQ(16, parent->num_children);
  }

  cdd_cst_free_node_only(parent);
  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
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

  /* Try parse with invalid tokens to trigger token skip logic */
  rc = cdd_cst_parse_format(tree, &node, "int a = @;");
  ASSERT_EQ(0, rc);
  ASSERT(node != NULL);
  cdd_cst_free_node_only(node);

  /* Clean up */
  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Test test_cst_parse_format is now in the suite
 */

#ifdef CDD_BUILD_TESTS
extern C_CDD_EXPORT int g_cdd_cst_alloc_token_fail;
extern C_CDD_EXPORT int g_cdd_cst_alloc_node_fail;

TEST test_cdd_cst_parse_format_oom(void) {
  cdd_cst_tree_t *tree = NULL;
  cdd_cst_node_t *node = NULL;
  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str(""), &tree));

  g_cdd_cst_alloc_token_fail = 1;
  {
    int rc_tmp = cdd_cst_parse_format(tree, &node, "int x;");
    if (rc_tmp != ENOMEM) {
      printf("rc_tmp = %d, expected ENOMEM\n", rc_tmp);
    }
    ASSERT(rc_tmp != 0);
  }
  g_cdd_cst_alloc_token_fail = 0;

  g_cdd_cst_alloc_node_fail = 1;
  {
    int rc_tmp = cdd_cst_parse_format(tree, &node, "int x;");
    if (rc_tmp != ENOMEM) {
      printf("rc_tmp = %d, expected ENOMEM\n", rc_tmp);
    }
    ASSERT(rc_tmp != 0);
  }
  g_cdd_cst_alloc_node_fail = 0;

  /* Create an invalid format string that triggers the parsing failure to return
     ENOMEM Or just an empty parsing result */
  ASSERT_EQ(0, cdd_cst_parse_format(tree, &node, ""));

  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}
#endif
TEST test_cst_parse_format_extra(void) {
  cdd_cst_tree_t *tree = NULL;
  cdd_cst_node_t *node = NULL;
  int rc;

  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str(""), &tree));

  /* Provide a large formatted string to test `vasprintf` failure path or large
   * buffer fallback */
  rc = cdd_cst_parse_format(tree, &node, "int a = %*d;", 1000, 5);
  ASSERT_EQ(0, rc);
  ASSERT(node != NULL);
  cdd_cst_free_node_only(node);

#ifdef CDD_BUILD_TESTS
  {
    extern int g_cdd_cst_realloc_fail;
    /* fail node alloc inside format parser (use a high value to avoid parser
     * failing first) */
    g_cdd_cst_alloc_node_fail = 1000;
    rc = cdd_cst_parse_format(tree, &node, "int x;");
    ASSERT_EQ(ENOENT, rc);
    g_cdd_cst_alloc_node_fail = 0;
  }
#endif

  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

SUITE(cdd_cst_factory_suite) {
  RUN_TEST(test_cst_alloc_node);
  RUN_TEST(test_cst_create_token);
  RUN_TEST(test_cst_append_child_node);
  RUN_TEST(test_cst_append_child_token);
  RUN_TEST(test_cst_parse_format);
  RUN_TEST(test_cst_parse_format_extra);
#ifdef CDD_BUILD_TESTS
  RUN_TEST(test_cdd_cst_parse_format_oom);
#endif
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CDD_CST_FACTORY_H */

/* LCOV_EXCL_STOP */
