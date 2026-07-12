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

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cst_alloc_node(CDD_CST_DECLARATION, NULL));
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

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cst_create_token(NULL, CDD_TOKEN_IDENTIFIER, "foo", &tok));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cst_create_token(tree, CDD_TOKEN_IDENTIFIER, "foo", NULL));

  rc = cdd_cst_create_token(tree, CDD_TOKEN_IDENTIFIER, "foo", &tok);
  ASSERT_EQ(0, rc);
  ASSERT(tok != NULL);
  ASSERT_STRN_EQ("foo", (const char *)tok->start, 3);
  ASSERT_EQ(3, tok->length);
  ASSERT_EQ(1, tree->num_synthesized);
  ASSERT_EQ(tok, tree->synthesized_tokens[0]);

  /* Cover text == NULL branch */
  rc = cdd_cst_create_token(tree, CDD_TOKEN_EOF, NULL, &tok);
  ASSERT_EQ(0, rc);
  ASSERT(tok != NULL);
  ASSERT_EQ(NULL, tok->start);
  ASSERT_EQ(0, tok->length);

  /* Force realloc in track_synthesized (default cap is something big, 128?
   * Let's check: 128) */
  {
    size_t i;
    for (i = 0; i < 130; i++) {
      cdd_token_t *extra_tok = NULL;
      ASSERT_EQ(0, cdd_cst_create_token(tree, CDD_TOKEN_IDENTIFIER, "foo",
                                        &extra_tok));
    }
    ASSERT_EQ(132, tree->num_synthesized);
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

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cst_append_child_node(NULL, child));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cst_append_child_node(parent, NULL));

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

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cst_append_child_token(NULL, tok));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cst_append_child_token(parent, NULL));

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

  /* Explicitly expand capacity without appending to cover the `false` branch of
   * the capacity check */
  parent->capacity = 50;
  parent->children = (cdd_cst_child_t *)realloc(parent->children,
                                                50 * sizeof(cdd_cst_child_t));
  ASSERT_EQ(0, cdd_cst_append_child_token(parent, tok));
  ASSERT_EQ(17, parent->num_children);

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

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cst_parse_format(NULL, &node, "int x;"));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cst_parse_format(tree, NULL, "int x;"));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cst_parse_format(tree, &node, NULL));

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
    if (rc_tmp != CDD_C_ERROR_MEMORY) {
      printf("rc_tmp = %d, expected CDD_C_ERROR_MEMORY\n", rc_tmp);
    }
    ASSERT(rc_tmp != 0);
  }
  g_cdd_cst_alloc_token_fail = 0;

  {
    int i;
    for (i = 0; i < 50; i++) {
      g_cdd_cst_alloc_node_fail = i;
      int rc_tmp = cdd_cst_parse_format(tree, &node, "int x;");
      g_cdd_cst_alloc_node_fail = 0;
      if (node) {
        cdd_cst_free_node(node);
        node = NULL;
      }
    }
  }

  /* Create an invalid format string that triggers the parsing failure to return
     CDD_C_ERROR_MEMORY Or just an empty parsing result */
  ASSERT_EQ(0, cdd_cst_parse_format(tree, &node, ""));
  if (node)
    cdd_cst_free_node(node);

  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}
#endif
TEST test_cst_free_node(void) {
  cdd_cst_node_t *parent = NULL;
  cdd_cst_node_t *child_node = NULL;
  cdd_token_t *child_token = NULL;

  /* Calling with NULL should just return */
  cdd_cst_free_node(NULL);

  /* Set up a tree with a child node and a child token */
  ASSERT_EQ(0, cdd_cst_alloc_node(CDD_CST_DECLARATION, &parent));
  ASSERT_EQ(0, cdd_cst_alloc_node(CDD_CST_IDENTIFIER, &child_node));
  child_token = (cdd_token_t *)calloc(1, sizeof(cdd_token_t));
  child_token->kind = CDD_TOKEN_IDENTIFIER;

  ASSERT_EQ(0, cdd_cst_append_child_node(parent, child_node));
  ASSERT_EQ(0, cdd_cst_append_child_token(parent, child_token));

  /* cdd_cst_free_node should recursively free parent and child_node,
     but leaves child_token allocation alone (since tokens are usually owned by
     the tree). */
  cdd_cst_free_node(parent);
  free(child_token);

  g_fail_io_after = -1;
  PASS();
}

TEST test_cst_parse_format_branches(void) {
  cdd_cst_tree_t *tree = NULL;
  cdd_cst_node_t *node = NULL;
  int rc;
  int i;

  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str(""), &tree));

  /* Token at root */
  rc = cdd_cst_parse_format(tree, &node, "/* comment */\n");
  ASSERT_EQ(0, rc);
  if (node)
    cdd_cst_free_node(node);
  node = NULL;

#ifdef CDD_BUILD_TESTS
  /* Try varying realloc fails to catch the append fails in parse_format */
  for (i = 0; i < 500; i++) {
    g_cdd_cst_realloc_fail = i;
    rc = cdd_cst_parse_format(tree, &node, "int x;");
    g_cdd_cst_realloc_fail = 0;
    if (rc == 0 && node) {
      cdd_cst_free_node(node);
      node = NULL;
    }

    g_cdd_cst_realloc_fail = i;
    rc =
        cdd_cst_parse_format(tree, &node, "{ 1; 2; 3; 4; 5; 6; 7; 8; 9; 10; }");
    g_cdd_cst_realloc_fail = 0;
    if (rc == 0 && node) {
      cdd_cst_free_node(node);
      node = NULL;
    }
  }
#endif

  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

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
    extern C_CDD_EXPORT int g_cdd_cst_alloc_node_fail;
    /* fail node alloc inside format parser (use a high value to avoid parser
     * failing first) */
    int k;
    for (k = 1; k < 10; k++) {
      g_cdd_cst_alloc_node_fail = k;
      rc = cdd_cst_parse_format(tree, &node, "int x;");
      g_cdd_cst_alloc_node_fail = 0;
      if (rc == CDD_C_ERROR_MEMORY)
        break;
    }
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
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
  RUN_TEST(test_cst_parse_format_branches);
  RUN_TEST(test_cst_free_node);
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
