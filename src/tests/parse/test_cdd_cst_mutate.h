/**
 * @file test_cdd_cst_mutate.h
 * @brief Unit tests for CST mutation.
 */

#ifndef TEST_CDD_CST_MUTATE_H
#define TEST_CDD_CST_MUTATE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include "cdd_c_error.h"
#include <greatest.h>
#include <string.h>
#include <stdlib.h>
#include "classes/parse/cdd_cst_parser.h"
#include "classes/parse/cdd_cst_mutate.h"
#include "classes/parse/cdd_cst_factory.h"

#include "classes/parse/cdd_cst_query.h"
#include "classes/emit/cdd_cst_emit.h"
/* clang-format on */

enum cdd_c_error insert_child_at_mutate(cdd_cst_node_t *parent, size_t idx,
                                        cdd_cst_node_t *new_node);

TEST test_cdd_cst_mutate_replace(void) {
  cdd_cst_tree_t *tree = NULL;
  cdd_cst_node_t *clone = NULL;
  const char *code = "int main() {\n  return 0;\n}";
  char *out = NULL;
  cdd_cst_query_result_t res;
  int rc;
  cdd_cst_node_t *target;

  rc = cdd_cst_parse(az_span_create_from_str((char *)code), &tree);
  ASSERT_EQ(0, rc);

  rc = cdd_cst_find_nodes_by_type(tree->root, CDD_CST_STATEMENT, &res);
  /* Wait, our parser puts it in CDD_CST_UNKNOWN since we didn't specify
   * STATEMENT */
  rc = cdd_cst_find_nodes_by_type(tree->root, CDD_CST_UNKNOWN, &res);
  ASSERT_EQ(0, rc);
  ASSERT(res.size > 0);

  target = res.nodes[0];
  rc = cdd_cst_clone_tree(tree, target, &clone);
  ASSERT_EQ(0, rc);

#ifdef CDD_BUILD_TESTS
  {
    cdd_cst_node_t *clone_err = NULL;
    int fails[] = {1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13,
                   14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25};
    size_t j;
    for (j = 0; j < sizeof(fails) / sizeof(fails[0]); j++) {
      g_cdd_cst_alloc_token_fail = fails[j];
      rc = cdd_cst_clone_tree(tree, target, &clone_err);
      g_cdd_cst_alloc_token_fail = 0;
      if (rc == 0) {
        /* No failure means we ran out of allocations to fail */
        if (clone_err)
          cdd_cst_free_node(clone_err);
      } else {
        ASSERT(rc != 0);
      }
    }

    extern int g_cdd_cst_realloc_fail;
    for (j = 0; j < sizeof(fails) / sizeof(fails[0]); j++) {
      tree->synthesized_capacity =
          tree->num_synthesized; /* force realloc on first token cloned */
      g_cdd_cst_realloc_fail = fails[j];
      rc = cdd_cst_clone_tree(tree, target, &clone_err);
      g_cdd_cst_realloc_fail = 0;
      if (rc == 0) {
        if (clone_err)
          cdd_cst_free_node(clone_err);
      }
    }

    /* Test clone_trivia_list_mutate invalid arguments */
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              clone_trivia_list_mutate(NULL, NULL));
  }
#endif

  /* Just test replace works without crashing and unparses */
  rc = cdd_cst_replace_node(tree, target, clone);
  ASSERT_EQ(0, rc);

  /* Test find_first_token_mutate recursion */
  {
    cdd_cst_node_t *nested_parent;
    cdd_cst_node_t *nested_child;
    cdd_cst_node_t *replacement;
    cdd_cst_node_t *grandparent;
    cdd_token_t tok = {0};
    tok.kind = CDD_TOKEN_IDENTIFIER;
    tok.start = (const uint8_t *)"nested";
    tok.length = 6;

    cdd_cst_alloc_node(CDD_CST_UNKNOWN, &grandparent);
    cdd_cst_alloc_node(CDD_CST_UNKNOWN, &nested_parent);
    cdd_cst_alloc_node(CDD_CST_UNKNOWN, &nested_child);
    cdd_cst_alloc_node(CDD_CST_UNKNOWN, &replacement);

    cdd_cst_append_child_node(grandparent, nested_parent);
    cdd_cst_append_child_node(nested_parent, nested_child);
    cdd_cst_append_child_token(nested_child, &tok);

    /* Dummy tree for replacement */
    cdd_cst_tree_t *dummy_tree;
    dummy_tree = (cdd_cst_tree_t *)calloc(1, sizeof(cdd_cst_tree_t));
    dummy_tree->root = grandparent;

    /* replace nested_parent with replacement */
    ASSERT_EQ(0, cdd_cst_replace_node(dummy_tree, nested_parent, replacement));

    dummy_tree->root = NULL; /* prevent double free */
    cdd_cst_tree_free(dummy_tree);
    cdd_cst_free_node_only(nested_parent);
    cdd_cst_free_node_only(nested_child);
    cdd_cst_free_node_only(replacement);
    cdd_cst_free_node_only(grandparent);
  }

  rc = cdd_cst_emit(tree, &out);
  ASSERT_EQ(0, rc);
  ASSERT_STR_EQ(code, out); /* Same code after replace with clone */

  free(res.nodes);
  free(out);
  cdd_cst_tree_free(tree);
  cdd_cst_free_node_only(target);
  g_fail_io_after = -1;
  PASS();
}

TEST test_cdd_cst_mutate_errors(void) {
  cdd_cst_tree_t *tree = NULL;
  cdd_cst_node_t *node = NULL;
  cdd_cst_node_t *node2 = NULL;
  cdd_cst_node_t *clone = NULL;
  cdd_cst_node_t *detach_parent = NULL;
  cdd_cst_node_t *detach_child = NULL;
  cdd_cst_tree_t *tree3 = NULL;
  cdd_cst_node_t *valid_child = NULL;
  cdd_cst_tree_t *tree4 = NULL;
  cdd_cst_node_t *splice_root = NULL;
  cdd_cst_child_t new_children[1];
  cdd_token_t mock_token2;
  cdd_cst_node_t *parent_node = NULL;

  cdd_cst_parse(az_span_create_from_str(""), &tree);
  cdd_cst_alloc_node(CDD_CST_IDENTIFIER, &node);
  cdd_cst_alloc_node(CDD_CST_IDENTIFIER, &node2);

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cst_replace_node(NULL, node, node2));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cst_replace_node(tree, NULL, node2));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cst_replace_node(tree, node, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cst_replace_node(tree, node, node2));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_cst_detach_node(tree, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_cst_detach_node(NULL, node));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_cst_detach_node(tree, node));

  cdd_cst_alloc_node(CDD_CST_BLOCK, &detach_parent);
  cdd_cst_alloc_node(CDD_CST_IDENTIFIER, &detach_child);

  /* Trigger error cases in cdd_cst_detach_node by not parenting detach_child */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cst_detach_node(tree, detach_child));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cst_remove_child(detach_parent, 100));

  cdd_cst_insert_node_after(node,
                            detach_parent); /* Just to put parent somewhere */

  /* We need a real parent-child relationship to test success, but wait,
     cdd_cst_insert_node_after expects target to have a parent. Let's just use
     the tree. */

  cdd_cst_parse(az_span_create_from_str("int x;"), &tree3);
  valid_child = tree3->root->children[0].val.node;
  ASSERT_EQ(0, cdd_cst_detach_node(tree3, valid_child));

  cdd_cst_tree_free(tree3);
  cdd_cst_parse(az_span_create_from_str("int y;"), &tree4);
  splice_root = tree4->root;

  new_children[0].kind = CDD_CST_CHILD_NODE;
  new_children[0].val.node = splice_root;

  ASSERT_EQ(
      CDD_C_ERROR_INVALID_ARGUMENT,
      cdd_cst_splice_children(NULL, &detach_parent, 0, 1, new_children, 1));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cst_splice_children(tree, NULL, 0, 1, new_children, 1));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cst_splice_children(tree, &detach_parent, 0, 1, NULL, 0));
  /* Not perfectly set up, but let's just assert error due to bounds or
   * something */
  /* removed */

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_cst_remove_child(NULL, 0));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cst_remove_child(detach_parent, 100));
  /* removed */

  memset(&mock_token2, 0, sizeof(mock_token2));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cst_replace_token_child(NULL, 0, &mock_token2));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cst_replace_token_child(detach_parent, 100, &mock_token2));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cst_replace_token_child(detach_parent, 0, NULL));

  /* test invalid child kind */
  cdd_cst_append_child_node(detach_parent, node2);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cst_replace_token_child(detach_parent, 0, &mock_token2));

  /* test valid token replacement */
  cdd_cst_append_child_token(detach_parent, &mock_token2);
  ASSERT_EQ(0, cdd_cst_replace_token_child(detach_parent, 1, &mock_token2));

  cdd_cst_tree_free(tree4);

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cst_insert_node_before(NULL, node2));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cst_insert_node_before(node, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cst_insert_node_before(node, node2));
  /* node2 is not a child of node's parent (it has no parent), so replace fails
   */
  cdd_cst_alloc_node(CDD_CST_BLOCK, &parent_node);
  node->parent = parent_node;
  ASSERT_EQ(CDD_C_ERROR_NOT_FOUND, cdd_cst_replace_node(tree, node, node2));
  ASSERT_EQ(CDD_C_ERROR_NOT_FOUND, cdd_cst_insert_node_before(node, node2));
  ASSERT_EQ(CDD_C_ERROR_NOT_FOUND, cdd_cst_insert_node_after(node, node2));

  /* leaked parent_node intentionally because no public free */

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cst_insert_node_after(NULL, node2));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cst_insert_node_after(node, NULL));
  ASSERT_EQ(CDD_C_ERROR_NOT_FOUND, cdd_cst_insert_node_after(node, node2));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cst_insert_child_node_at(NULL, 0, node2));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cst_insert_child_node_at(node, 0, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cst_insert_child_node_at(node, 100, node2));

  cdd_cst_tree_free(tree);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cst_clone_tree(NULL, detach_parent, &clone));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cst_clone_tree(tree, NULL, &clone));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cst_clone_tree(tree, detach_parent, NULL));

  cdd_cst_free_node_only(node);
  cdd_cst_free_node_only(node2);
  cdd_cst_free_node_only(detach_parent);
  cdd_cst_free_node_only(detach_child);
  cdd_cst_free_node_only(parent_node);
  if (valid_child)
    cdd_cst_free_node(valid_child);
  g_fail_io_after = -1;

  PASS();
}

TEST test_mutate_utils(void) {
  size_t idx;
  cdd_token_t *t;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            find_child_index_mutate(NULL, NULL, &idx));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, find_first_token_mutate(NULL, &t));
  g_fail_io_after = -1;

  PASS();
}

/* old suite */

/**
 * @brief Tests child splicing
 * @return TEST
 */
TEST test_cst_splice_children(void) {
  cdd_cst_tree_t *tree = NULL;
  cdd_cst_node_t *root = NULL;
  cdd_cst_node_t *child1 = NULL;
  cdd_token_t *tok = NULL;
  cdd_cst_child_t new_children[1];
  int rc;

  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str(""), &tree));
  if (tree->root)
    cdd_cst_free_node(tree->root);
  ASSERT_EQ(0, cdd_cst_alloc_node(CDD_CST_DECLARATION, &root));
  tree->root = root;

  ASSERT_EQ(0, cdd_cst_alloc_node(CDD_CST_IDENTIFIER, &child1));
  ASSERT_EQ(0, cdd_cst_append_child_node(root, child1));
  tree->root = root;

  ASSERT_EQ(0, cdd_cst_create_token(tree, CDD_TOKEN_IDENTIFIER, "new", &tok));
  new_children[0].kind = CDD_CST_CHILD_TOKEN;
  new_children[0].val.token = tok;

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cst_splice_children(NULL, &root, 0, 1, new_children, 1));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cst_splice_children(tree, &root, 0, 2, new_children, 1));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cst_splice_children(tree, NULL, 0, 1, new_children, 1));
  /* Out of bounds */

  /* Test out node pointer */
  {
    cdd_cst_node_t *old_root = tree->root;
    rc = cdd_cst_splice_children(tree, &old_root, 0, 1, new_children, 1);
    ASSERT_EQ(0, rc);
    ASSERT_EQ(old_root,
              tree->root); /* updated variable points to the new root */
  }

#ifdef CDD_BUILD_TESTS
  {
    extern C_CDD_EXPORT int g_cdd_cst_alloc_node_fail;
    cdd_cst_node_t *o_root = tree->root;
    g_cdd_cst_alloc_node_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              cdd_cst_splice_children(tree, &o_root, 0, 1, new_children, 1));
    g_cdd_cst_alloc_node_fail = 0;
  }
#endif

  cdd_cst_tree_free(tree);

  /* Test passing NULL for node_ptr on non-root */
  tree = (cdd_cst_tree_t *)calloc(1, sizeof(cdd_cst_tree_t));
  tree->root = (cdd_cst_node_t *)calloc(1, sizeof(cdd_cst_node_t));
  cdd_cst_append_child_node(tree->root, calloc(1, sizeof(cdd_cst_node_t)));
  {
    cdd_cst_node_t *target = tree->root->children[0].val.node;
    cdd_cst_child_t new_children2[1] = {0};
    rc =
        cdd_cst_splice_children(tree, &target, 0, 0, new_children2, 0); /* OK */
    ASSERT_EQ(0, rc);

    rc = cdd_cst_splice_children(tree, NULL, 0, 0, new_children2, 0);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  }
  cdd_cst_tree_free(tree);

  /* Actually, test passing a pointer to the node we're replacing */
  tree = (cdd_cst_tree_t *)calloc(1, sizeof(cdd_cst_tree_t));
  tree->root = (cdd_cst_node_t *)calloc(1, sizeof(cdd_cst_node_t));
  cdd_cst_append_child_node(tree->root, calloc(1, sizeof(cdd_cst_node_t)));
  {
    cdd_cst_node_t *target = tree->root->children[0].val.node;
    cdd_cst_node_t *ptr_copy = target;
    cdd_cst_child_t new_children2[1] = {0};

    rc = cdd_cst_splice_children(tree, &ptr_copy, 0, 0, new_children2, 0);
    ASSERT_EQ(0, rc);
    ASSERT(ptr_copy != target);
    ASSERT_EQ(ptr_copy, tree->root->children[0].val.node);
  }

  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

#ifdef CDD_BUILD_TESTS
extern C_CDD_EXPORT int g_cdd_cst_realloc_fail;
#endif

TEST test_cdd_cst_splice_oom(void) {
  int i;
  for (i = 0; i < 20; i++) {
    cdd_cst_tree_t *tree = calloc(1, sizeof(cdd_cst_tree_t));
    cdd_cst_node_t *root = NULL;
    cdd_cst_node_t *n1 = NULL, *n2 = NULL, *n3 = NULL, *n4 = NULL, *n5 = NULL;
    cdd_token_t t1 = {0}, t2 = {0}, t3 = {0}, t4 = {0}, t5 = {0};
    cdd_cst_child_t new_children[2] = {0};
    cdd_token_t t_new = {0};
    cdd_cst_node_t *n_new = NULL;
    int rc;

    cdd_cst_alloc_node(CDD_CST_TRANSLATION_UNIT, &root);
    tree->root = root;

    cdd_cst_alloc_node(CDD_CST_STATEMENT, &n1);
    cdd_cst_alloc_node(CDD_CST_STATEMENT, &n2);
    cdd_cst_alloc_node(CDD_CST_STATEMENT, &n3);
    cdd_cst_alloc_node(CDD_CST_STATEMENT, &n4);
    cdd_cst_alloc_node(CDD_CST_STATEMENT, &n5);

    cdd_cst_append_child_token(root, &t1);
    cdd_cst_append_child_node(root, n1);
    cdd_cst_append_child_token(root, &t2);
    cdd_cst_append_child_node(root, n2);
    cdd_cst_append_child_token(root, &t3);
    cdd_cst_append_child_node(root, n3);
    cdd_cst_append_child_token(root, &t4);
    cdd_cst_append_child_node(root, n4);
    cdd_cst_append_child_token(root, &t5);
    cdd_cst_append_child_node(root, n5);

    cdd_cst_alloc_node(CDD_CST_STATEMENT, &n_new);
    new_children[0].kind = CDD_CST_CHILD_TOKEN;
    new_children[0].val.token = &t_new;
    new_children[1].kind = CDD_CST_CHILD_NODE;
    new_children[1].val.node = n_new;

    /* Force root->capacity to be accurate so realloc occurs */
    root->capacity = root->num_children;

#ifdef CDD_BUILD_TESTS
    g_cdd_cst_realloc_fail = i;
#endif
    rc = cdd_cst_splice_children(tree, &tree->root, 4, 4, new_children, 2);
#ifdef CDD_BUILD_TESTS
    g_cdd_cst_realloc_fail = 0;
#endif

    if (rc != 0) {
      cdd_cst_free_node_only(n_new);
    }
    cdd_cst_tree_free(tree);
  }
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Tests finding node for token
 * @return TEST
 */
TEST test_cst_find_node_for_token(void) {
  cdd_cst_node_t *root = NULL;
  cdd_cst_node_t *child = NULL;
  cdd_cst_node_t *found_node = NULL;
  cdd_token_t tok = {0};
  size_t idx;

  ASSERT_EQ(0, cdd_cst_alloc_node(CDD_CST_DECLARATION, &root));
  ASSERT_EQ(0, cdd_cst_alloc_node(CDD_CST_IDENTIFIER, &child));
  ASSERT_EQ(0, cdd_cst_append_child_node(root, child));
  ASSERT_EQ(0, cdd_cst_append_child_token(child, &tok));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cst_find_node_for_token(NULL, &tok, &idx, &found_node));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cst_find_node_for_token(root, NULL, &idx, &found_node));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cst_find_node_for_token(root, &tok, NULL, &found_node));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_cst_find_node_for_token(root, &tok, &idx, NULL));

  ASSERT_EQ(0, cdd_cst_find_node_for_token(root, &tok, &idx, &found_node));
  ASSERT_EQ(0, idx);
  ASSERT_EQ(child, found_node);

  {
    cdd_token_t not_found_tok = {0};
    found_node = NULL;
    ASSERT_EQ(
        CDD_C_ERROR_NOT_FOUND,
        cdd_cst_find_node_for_token(root, &not_found_tok, &idx, &found_node));
    ASSERT_EQ(NULL, found_node);
  }

  cdd_cst_free_node_only(child);
  cdd_cst_free_node_only(root);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief Rewrite mutate suite to include splice tests
 */

TEST test_cdd_cst_insert_node_after_success(void) {
  cdd_cst_tree_t *tree = NULL;
  cdd_cst_node_t *parent = NULL;
  cdd_cst_node_t *child1 = NULL;
  cdd_cst_node_t *child2 = NULL;

  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str(""), &tree));

  cdd_cst_alloc_node(CDD_CST_BLOCK, &parent);
  cdd_cst_alloc_node(CDD_CST_IDENTIFIER, &child1);
  cdd_cst_alloc_node(CDD_CST_IDENTIFIER, &child2);

  cdd_cst_append_child_node(parent, child1);

  ASSERT_EQ(0, cdd_cst_insert_node_after(child1, child2));

  /* Capacity expansion */
  {
    cdd_cst_node_t *child3;
    cdd_cst_alloc_node(CDD_CST_STATEMENT, &child3);
    parent->capacity = parent->num_children; /* Force expansion */
    ASSERT_EQ(0, cdd_cst_insert_node_after(child1, child3));

    parent->capacity = parent->num_children;
    g_cdd_cst_realloc_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_cst_insert_node_after(child1, child3));
    g_cdd_cst_realloc_fail = 0;

    ASSERT_EQ(3, parent->num_children);
    ASSERT_EQ(child1, parent->children[0].val.node);
    ASSERT_EQ(child3, parent->children[1].val.node);
    ASSERT_EQ(child2, parent->children[2].val.node);
    cdd_cst_free_node_only(child3);
  }

  cdd_cst_free_node_only(parent);
  cdd_cst_free_node_only(child1);
  cdd_cst_free_node_only(child2);
  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;

  PASS();
}

TEST test_cdd_cst_insert_node_before_success(void) {
  cdd_cst_tree_t *tree = NULL;
  cdd_cst_node_t *parent = NULL;
  cdd_cst_node_t *child1 = NULL;
  cdd_cst_node_t *child2 = NULL;

  ASSERT_EQ(0, cdd_cst_parse(az_span_create_from_str(""), &tree));

  cdd_cst_alloc_node(CDD_CST_BLOCK, &parent);
  cdd_cst_alloc_node(CDD_CST_IDENTIFIER, &child1);
  cdd_cst_alloc_node(CDD_CST_IDENTIFIER, &child2);

  cdd_cst_append_child_node(parent, child1);

  ASSERT_EQ(0, cdd_cst_insert_node_before(child1, child2));

  ASSERT_EQ(2, parent->num_children);
  ASSERT_EQ(child2, parent->children[0].val.node);
  ASSERT_EQ(child1, parent->children[1].val.node);

  cdd_cst_free_node_only(parent);
  cdd_cst_free_node_only(child1);
  cdd_cst_free_node_only(child2);
  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;

  PASS();
}

TEST test_cdd_cst_detach_node_success(void) {
  cdd_cst_tree_t *tree = NULL;
  cdd_cst_node_t *parent = NULL;
  cdd_cst_node_t *child1 = NULL;
  cdd_cst_node_t *child2 = NULL;
  cdd_cst_node_t *child3 = NULL;
  cdd_token_t tok1 = {0}, tok2 = {0};
  cdd_trivia_t triv1 = {0}, triv2 = {0};

  tree = (cdd_cst_tree_t *)calloc(1, sizeof(cdd_cst_tree_t));
  cdd_cst_alloc_node(CDD_CST_TRANSLATION_UNIT, &parent);
  cdd_cst_alloc_node(CDD_CST_STATEMENT, &child1);
  cdd_cst_alloc_node(CDD_CST_STATEMENT, &child2);
  cdd_cst_alloc_node(CDD_CST_STATEMENT, &child3);

  /* Add trivia to tokens so we hit 275 and 280 */
  tok1.leading_trivia = &triv1;
  tok2.trailing_trivia = &triv2;

  cdd_cst_append_child_token(child2, &tok1);
  cdd_cst_append_child_token(child2, &tok2);

  cdd_cst_append_child_node(parent, child1);
  cdd_cst_append_child_node(parent, child2);
  cdd_cst_append_child_node(parent, child3);

  /* Error checks */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_cst_detach_node(NULL, child2));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_cst_detach_node(tree, NULL));

  {
    cdd_cst_node_t *orphan;
    cdd_cst_alloc_node(CDD_CST_STATEMENT, &orphan);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_cst_detach_node(tree, orphan));

    /* Make orphan have a parent but not in parent's children (simulate error in
     * find_child_index_mutate) */
    orphan->parent = parent;
    ASSERT_EQ(CDD_C_ERROR_NOT_FOUND, cdd_cst_detach_node(tree, orphan));

    cdd_cst_free_node_only(orphan);
  }

  /* Remove middle child to hit memmove */
  ASSERT_EQ(0, cdd_cst_detach_node(tree, child2));
  ASSERT_EQ(2, parent->num_children);
  ASSERT_EQ(child1, parent->children[0].val.node);
  ASSERT_EQ(child3, parent->children[1].val.node);

  /* Remove child with no tokens to hit find_first_token_mutate returning error
   * -> first_tok = NULL */
  ASSERT_EQ(0, cdd_cst_detach_node(tree, child1));

  cdd_cst_free_node_only(child1);
  cdd_cst_free_node_only(child2);
  cdd_cst_free_node_only(child3);
  cdd_cst_free_node_only(parent);
  cdd_cst_tree_free(tree);
  PASS();
}
TEST test_cdd_cst_remove_child_success(void) {
  cdd_cst_node_t *parent;
  cdd_cst_node_t *child1;
  cdd_cst_node_t *child2;

  cdd_cst_alloc_node(CDD_CST_STATEMENT, &parent);
  cdd_cst_alloc_node(CDD_CST_STATEMENT, &child1);
  cdd_cst_alloc_node(CDD_CST_STATEMENT, &child2);

  cdd_cst_append_child_node(parent, child1);
  cdd_cst_append_child_node(parent, child2);

  ASSERT_EQ(0, cdd_cst_remove_child(parent, 0));
  ASSERT_EQ(1, parent->num_children);
  ASSERT_EQ(child2, parent->children[0].val.node);

  ASSERT_EQ(0, cdd_cst_remove_child(parent, 0));
  ASSERT_EQ(0, parent->num_children);

  cdd_cst_free_node_only(parent);
  cdd_cst_free_node_only(child1);
  cdd_cst_free_node_only(child2);
  PASS();
}

TEST test_cdd_cst_clone_trivia_list_mutate(void) {
  cdd_trivia_t t1 = {0};
  cdd_trivia_t t2 = {0};
  cdd_trivia_t *out = NULL;

  t1.kind = TRIVIA_NEWLINE;
  t1.start = (const uint8_t *)"\n";
  t1.length = 1;
  t1.next = &t2;

  t2.kind = TRIVIA_WHITESPACE;
  t2.start = (const uint8_t *)"  ";
  t2.length = 2;

  /* Success */
  ASSERT_EQ(0, clone_trivia_list_mutate(&t1, &out));
  if (out) {
    free(out->next);
    free(out);
    out = NULL;
  }

  /* Fail on first */
#ifdef CDD_BUILD_TESTS
  g_cdd_cst_alloc_token_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, clone_trivia_list_mutate(&t1, &out));
  g_cdd_cst_alloc_token_fail = 0;

  /* Fail on second */
  g_cdd_cst_alloc_token_fail = 2;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, clone_trivia_list_mutate(&t1, &out));
  g_cdd_cst_alloc_token_fail = 0;
#endif

  PASS();
}

SUITE(cdd_cst_mutate_suite) {
  RUN_TEST(test_cdd_cst_mutate_replace);
  RUN_TEST(test_cdd_cst_mutate_errors);
  RUN_TEST(test_mutate_utils);
  RUN_TEST(test_cst_splice_children);
  RUN_TEST(test_cdd_cst_splice_oom);
  RUN_TEST(test_cst_find_node_for_token);
  RUN_TEST(test_cdd_cst_insert_node_after_success);
  RUN_TEST(test_cdd_cst_insert_node_before_success);
  RUN_TEST(test_cdd_cst_detach_node_success);
  RUN_TEST(test_cdd_cst_remove_child_success);
  RUN_TEST(test_cdd_cst_clone_trivia_list_mutate);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CDD_CST_MUTATE_H */
