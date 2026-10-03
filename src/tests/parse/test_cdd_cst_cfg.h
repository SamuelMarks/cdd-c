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

/* Moved extern declarations for C89 compliance */
extern C_CDD_EXPORT int g_cdd_cfg_alloc_fail;

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
  int rc = 0;
  const char *src = (char *)(size_t)(size_t) "int main() { return 0; }";

  rc = cdd_cst_parse(az_span_create_from_str((char *)(size_t)src), &tree);
  ASSERT_EQ(0, rc);

  for (i = 0; i < tree->root->num_children; i++) {
    if (tree->root->children[i].kind == CDD_CST_CHILD_NODE &&
        tree->root->children[i].val.node->kind == CDD_CST_FUNCTION_DEFINITION &&
        func == NULL) {
      func = tree->root->children[i].val.node;
    }
  }
  ASSERT(func != NULL);

  {
    cdd_cst_node_t *dummy_node = NULL;
    cdd_cst_alloc_node(CDD_CST_UNKNOWN, &dummy_node);
    cdd_cst_append_child_node(func, dummy_node);
    rc = cdd_cst_cfg_build(func, &cfg);
    func->num_children--;
    cdd_cst_free_node_only(dummy_node);

    ASSERT_EQ(0, rc);
    ASSERT(cfg != NULL);

    cdd_cst_cfg_free(cfg);
    cfg = NULL;
    cdd_cst_tree_free(tree);
    g_fail_io_after = -1;

    PASS();
  }
}

/**
 * @brief Tests error handling of the CFG APIs.
 *
 * @return The result of the test.
 */

#ifdef CDD_BUILD_TESTS
/* extern C_CDD_EXPORT int g_cdd_cfg_alloc_fail; (moved to global) */
#endif

TEST test_cdd_cst_cfg_oom(void) {
  cdd_cst_tree_t *tree = NULL;
  cdd_cst_node_t *func = NULL;
  cdd_cst_cfg_t *cfg = NULL;
  int rc = 0;
  const char *src = (char *)(size_t)(size_t) "int main() { return 0; }";

  rc = cdd_cst_parse(az_span_create_from_str((char *)(size_t)src), &tree);
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
    const char *long_src =
        "int main() { if(1){} if(1){} if(1){} if(1){} if(1){} if(1){} if(1){} "
        "if(1){} if(1){} if(1){} if(1){} if(1){} if(1){} if(1){} if(1){} "
        "if(1){} if(1){} if(1){} if(1){} if(1){} }";
    cdd_cst_tree_t *t2 = NULL;
    cdd_cst_cfg_t *cfg2 = NULL;
    rc = cdd_cst_parse(az_span_create_from_str((char *)(size_t)long_src), &t2);
    ASSERT_EQ(0, rc);
    rc = cdd_cst_cfg_build(t2->root->children[0].val.node, &cfg2);
    ASSERT_EQ(0, rc);
    cdd_cst_cfg_free(cfg2);
    cdd_cst_tree_free(t2);
  }
  {
    /* extern C_CDD_EXPORT int g_cdd_cfg_alloc_fail; (moved to global) */
    int i;
    const char *ret_src = (char *)(size_t)(size_t) "int f() { return 0; }";
    cdd_cst_tree_t *ret_t = NULL;
    cdd_cst_cfg_t *ret_cfg = NULL;
    cdd_cst_parse(az_span_create_from_str((char *)(size_t)ret_src), &ret_t);
    for (i = 1; i < 50; ++i) {
      g_cdd_cfg_alloc_fail = i;
      rc = cdd_cst_cfg_build(ret_t->root->children[0].val.node, &ret_cfg);
      if (rc == 0) {
        cdd_cst_cfg_free(ret_cfg);
        break;
      }
    }
    g_cdd_cfg_alloc_fail = 0;
    cdd_cst_tree_free(ret_t);

    {
      const char *if_src = "int f() { if(1){} else{} }";
      cdd_cst_tree_t *if_t = NULL;
      cdd_cst_cfg_t *if_cfg = NULL;
      cdd_cst_parse(az_span_create_from_str((char *)(size_t)if_src), &if_t);
      for (i = 1; i < 150; ++i) {
        g_cdd_cfg_alloc_fail = i;
        rc = cdd_cst_cfg_build(if_t->root->children[0].val.node, &if_cfg);
        if (if_cfg) {
          cdd_cst_cfg_free(if_cfg);
          if_cfg = NULL;
        }
        if (rc == CDD_C_SUCCESS)
          break;
      }
      g_cdd_cfg_alloc_fail = 0;
      cdd_cst_tree_free(if_t);
    }
    {
      const char *if2_src = "int f() { if(1){} }";
      cdd_cst_tree_t *if2_t = NULL;
      cdd_cst_cfg_t *if2_cfg = NULL;
      cdd_cst_parse(az_span_create_from_str((char *)(size_t)if2_src), &if2_t);
      for (i = 1; i < 150; ++i) {
        g_cdd_cfg_alloc_fail = i;
        rc = cdd_cst_cfg_build(if2_t->root->children[0].val.node, &if2_cfg);
        if (if2_cfg) {
          cdd_cst_cfg_free(if2_cfg);
          if2_cfg = NULL;
        }
        if (rc == CDD_C_SUCCESS)
          break;
      }
      g_cdd_cfg_alloc_fail = 0;
      cdd_cst_tree_free(if2_t);
    }
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
  int rc = 0;
  const char *src = (char *)(size_t)(size_t) "void main() { }";

  rc = cdd_cst_parse(az_span_create_from_str((char *)(size_t)src), &tree);
  ASSERT_EQ(0, rc);

  for (i = 0; i < tree->root->num_children; i++) {
    if (tree->root->children[i].kind == CDD_CST_CHILD_NODE &&
        tree->root->children[i].val.node->kind == CDD_CST_FUNCTION_DEFINITION &&
        func == NULL) {
      func = tree->root->children[i].val.node;
    }
  }
  ASSERT(func != NULL);

  {
    cdd_cst_node_t *dummy_node = NULL;
    cdd_cst_alloc_node(CDD_CST_UNKNOWN, &dummy_node);
    cdd_cst_append_child_node(func, dummy_node);
    rc = cdd_cst_cfg_build(func, &cfg);
    func->num_children--;
    cdd_cst_free_node_only(dummy_node);
    ASSERT_EQ(0, rc);
    ASSERT(cfg != NULL);

    cdd_cst_cfg_free(cfg);
    cfg = NULL;
    cdd_cst_tree_free(tree);
    g_fail_io_after = -1;

    PASS();
  }
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
  int rc = 0;
  const char *src = (char *)(size_t)(size_t) "void main() { int a = 5; }";

  rc = cdd_cst_parse(az_span_create_from_str((char *)(size_t)src), &tree);
  ASSERT_EQ(0, rc);

  for (i = 0; i < tree->root->num_children; i++) {
    if (tree->root->children[i].kind == CDD_CST_CHILD_NODE &&
        tree->root->children[i].val.node->kind == CDD_CST_FUNCTION_DEFINITION &&
        func == NULL) {
      func = tree->root->children[i].val.node;
    }
  }
  ASSERT(func != NULL);

  {
    cdd_cst_node_t *dummy_node = NULL;
    cdd_cst_alloc_node(CDD_CST_UNKNOWN, &dummy_node);
    cdd_cst_append_child_node(func, dummy_node);
    rc = cdd_cst_cfg_build(func, &cfg);
    func->num_children--;
    cdd_cst_free_node_only(dummy_node);
    ASSERT_EQ(0, rc);
    ASSERT(cfg != NULL);

    cdd_cst_cfg_free(cfg);
    cfg = NULL;
    cdd_cst_tree_free(tree);
    g_fail_io_after = -1;

    PASS();
  }
}

TEST test_cdd_cst_cfg_extra(void) {
  /* Internal functions alloc_block and add_edge are static.
   * To achieve 100% coverage, we trigger failures via public API using OOM
   * mocks.
   */
  cdd_cst_tree_t *tree = NULL;
  cdd_cst_node_t *func = NULL;
  cdd_cst_cfg_t *cfg = NULL;
  int rc = 0;
  const char *src =
      (char *)(size_t)(size_t) "int main() { if (1) { return 0; } "
                               "else { return 1; } }";

  rc = cdd_cst_parse(az_span_create_from_str((char *)(size_t)src), &tree);
  ASSERT_EQ(0, rc);
  func = tree->root->children[0].val.node; /* get function node */

#ifdef CDD_BUILD_TESTS
  {
    /* extern C_CDD_EXPORT int g_cdd_cfg_alloc_fail; (moved to global) */
    int i;
    for (i = 1; i < 50; ++i) {
      g_cdd_cfg_alloc_fail = i;
      {
        cdd_cst_node_t *dummy_node = NULL;
        cdd_cst_alloc_node(CDD_CST_UNKNOWN, &dummy_node);
        cdd_cst_append_child_node(func, dummy_node);
        rc = cdd_cst_cfg_build(func, &cfg);
        func->num_children--;
        cdd_cst_free_node_only(dummy_node);
        if (rc == 0) {
          cdd_cst_cfg_free(cfg);
          cfg = NULL;
          break;
        }
        ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
      }
    }
    g_cdd_cfg_alloc_fail = 0;
  }
#endif

  {
    const char *long_src =
        "int main() { if(1){} if(1){} if(1){} if(1){} if(1){} if(1){} if(1){} "
        "if(1){} if(1){} if(1){} if(1){} if(1){} if(1){} if(1){} if(1){} "
        "if(1){} if(1){} if(1){} if(1){} if(1){} }";
    cdd_cst_tree_t *t2 = NULL;
    cdd_cst_cfg_t *cfg2 = NULL;
    rc = cdd_cst_parse(az_span_create_from_str((char *)(size_t)long_src), &t2);
    ASSERT_EQ(0, rc);
    rc = cdd_cst_cfg_build(t2->root->children[0].val.node, &cfg2);
    ASSERT_EQ(0, rc);
    cdd_cst_cfg_free(cfg2);
    cdd_cst_tree_free(t2);
  }
  {
    /* extern C_CDD_EXPORT int g_cdd_cfg_alloc_fail; (moved to global) */
    int i;
    const char *ret_src = (char *)(size_t)(size_t) "int f() { return 0; }";
    cdd_cst_tree_t *ret_t = NULL;
    cdd_cst_cfg_t *ret_cfg = NULL;
    cdd_cst_parse(az_span_create_from_str((char *)(size_t)ret_src), &ret_t);
    for (i = 1; i < 50; ++i) {
      g_cdd_cfg_alloc_fail = i;
      rc = cdd_cst_cfg_build(ret_t->root->children[0].val.node, &ret_cfg);
      if (rc == 0) {
        cdd_cst_cfg_free(ret_cfg);
        break;
      }
    }
    g_cdd_cfg_alloc_fail = 0;
    cdd_cst_tree_free(ret_t);

    {
      const char *if_src = "int f() { if(1){} else{} }";
      cdd_cst_tree_t *if_t = NULL;
      cdd_cst_cfg_t *if_cfg = NULL;
      cdd_cst_parse(az_span_create_from_str((char *)(size_t)if_src), &if_t);
      for (i = 1; i < 150; ++i) {
        g_cdd_cfg_alloc_fail = i;
        rc = cdd_cst_cfg_build(if_t->root->children[0].val.node, &if_cfg);
        if (if_cfg) {
          cdd_cst_cfg_free(if_cfg);
          if_cfg = NULL;
        }
        if (rc == CDD_C_SUCCESS)
          break;
      }
      g_cdd_cfg_alloc_fail = 0;
      cdd_cst_tree_free(if_t);
    }
    {
      const char *if2_src = "int f() { if(1){} }";
      cdd_cst_tree_t *if2_t = NULL;
      cdd_cst_cfg_t *if2_cfg = NULL;
      cdd_cst_parse(az_span_create_from_str((char *)(size_t)if2_src), &if2_t);
      for (i = 1; i < 150; ++i) {
        g_cdd_cfg_alloc_fail = i;
        rc = cdd_cst_cfg_build(if2_t->root->children[0].val.node, &if2_cfg);
        if (if2_cfg) {
          cdd_cst_cfg_free(if2_cfg);
          if2_cfg = NULL;
        }
        if (rc == CDD_C_SUCCESS)
          break;
      }
      g_cdd_cfg_alloc_fail = 0;
      cdd_cst_tree_free(if2_t);
    }
  }

  cdd_cst_tree_free(tree);
  g_fail_io_after = -1;
  PASS();
}

/**
 * @brief CFG test suite.
 */

TEST test_cdd_cst_cfg_if_else(void) {
  cdd_c_error_t rc;
  cdd_cst_cfg_t *cfg = NULL;
  cdd_cst_node_t *func, *blk, *stmt, *then_stmt, *else_stmt, *extra_stmt;
  cdd_token_t ret_tok;
  cdd_cst_child_t child_tok;

  memset(&ret_tok, 0, sizeof(ret_tok));
  ret_tok.kind = CDD_TOKEN_KEYWORD_RETURN;

  child_tok.kind = CDD_CST_CHILD_TOKEN;
  child_tok.val.token = &ret_tok;

  cdd_cst_alloc_node(CDD_CST_FUNCTION_DEFINITION, &func);
  cdd_cst_alloc_node(CDD_CST_BLOCK, &blk);
  cdd_cst_alloc_node(CDD_CST_STATEMENT, &stmt);
  cdd_cst_alloc_node(CDD_CST_EXPRESSION, &then_stmt);

  cdd_cst_alloc_node(CDD_CST_EXPRESSION, &else_stmt);
  cdd_cst_alloc_node(CDD_CST_STATEMENT, &extra_stmt);

  then_stmt->num_children = 1;
  then_stmt->children = &child_tok;
  else_stmt->num_children = 1;
  else_stmt->children = &child_tok;

  cdd_cst_append_child_node(func, blk);
  cdd_cst_append_child_node(blk, stmt);
  cdd_cst_append_child_node(stmt, then_stmt);
  cdd_cst_append_child_node(stmt, else_stmt);
  cdd_cst_append_child_node(stmt, extra_stmt);

  rc = cdd_cst_cfg_build(func, &cfg);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  cdd_cst_cfg_free(cfg);
  cfg = NULL;

#ifdef CDD_BUILD_TESTS
  {
    int j;
    for (j = 1; j < 60; j++) {
      g_cdd_cfg_alloc_fail = j;
      rc = cdd_cst_cfg_build(func, &cfg);
      if (cfg) {
        cdd_cst_cfg_free(cfg);
        cfg = NULL;
        cfg = NULL;
      }
      if (rc == CDD_C_SUCCESS)
        break;
    }
    g_cdd_cfg_alloc_fail = 0;
  }
#endif
#ifdef CDD_BUILD_TESTS
  {
    int i;
    for (i = 1; i < 60; i++) {
      g_cdd_cfg_alloc_fail = i;
      cdd_cst_cfg_build(func, &cfg);
    }
  }
  g_cdd_cfg_alloc_fail = 0;
#endif
  then_stmt->num_children = 0;
  then_stmt->children = NULL;
  else_stmt->num_children = 0;
  else_stmt->children = NULL;
  cdd_cst_free_node(func);

  PASS();
}

TEST test_cdd_cst_cfg_if_only(void) {
  cdd_c_error_t rc;
  cdd_cst_cfg_t *cfg = NULL;
  cdd_cst_node_t *func, *blk, *stmt, *then_stmt;
  cdd_cst_alloc_node(CDD_CST_FUNCTION_DEFINITION, &func);
  cdd_cst_alloc_node(CDD_CST_BLOCK, &blk);
  cdd_cst_alloc_node(CDD_CST_STATEMENT, &stmt);
  cdd_cst_alloc_node(CDD_CST_EXPRESSION, &then_stmt);
  cdd_cst_append_child_node(func, blk);
  cdd_cst_append_child_node(blk, stmt);
  cdd_cst_append_child_node(stmt, then_stmt);
  rc = cdd_cst_cfg_build(func, &cfg);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  cdd_cst_cfg_free(cfg);
  cfg = NULL;

#ifdef CDD_BUILD_TESTS
  {
    int j;
    for (j = 1; j < 60; j++) {
      g_cdd_cfg_alloc_fail = j;
      rc = cdd_cst_cfg_build(func, &cfg);
      if (cfg) {
        cdd_cst_cfg_free(cfg);
        cfg = NULL;
        cfg = NULL;
      }
      if (rc == CDD_C_SUCCESS)
        break;
    }
    g_cdd_cfg_alloc_fail = 0;
  }
#endif
  cdd_cst_free_node(func);

  PASS();
}

TEST test_cdd_cst_cfg_dead_code(void) {
  cdd_cst_tree_t *tree = NULL;
  cdd_cst_node_t *func = NULL;
  cdd_cst_cfg_t *cfg = NULL;
  int rc = 0;
  const char *src =
      (char *)(size_t)(size_t) "int main() { return 0; int dead = 1; }";

  rc = cdd_cst_parse(az_span_create_from_str((char *)(size_t)src), &tree);
  ASSERT_EQ(0, rc);
  func = tree->root->children[0].val.node;

  rc = cdd_cst_cfg_build(func, &cfg);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  cdd_cst_cfg_free(cfg);
  cfg = NULL;
  cdd_cst_tree_free(tree);

  PASS();
}

TEST test_cdd_cst_cfg_if_returns(void) {
  cdd_cst_tree_t *tree = NULL;
  cdd_cst_node_t *func = NULL;
  cdd_cst_cfg_t *cfg = NULL;
  int rc = 0;
  size_t i;
  const char *src = "int f() { if (1) { return 1; } else { return 0; } }";

  rc = cdd_cst_parse(az_span_create_from_str((char *)(size_t)src), &tree);
  ASSERT_EQ(0, rc);

  for (i = 0; i < tree->root->num_children; i++) {
    if (tree->root->children[i].kind == CDD_CST_CHILD_NODE &&
        tree->root->children[i].val.node->kind == CDD_CST_FUNCTION_DEFINITION &&
        func == NULL) {
      func = tree->root->children[i].val.node;
    }
  }
  ASSERT(func != NULL);

  rc = cdd_cst_cfg_build(func, &cfg);
  ASSERT_EQ(0, rc);
  ASSERT(cfg != NULL);

  cdd_cst_cfg_free(cfg);
  cfg = NULL;

#ifdef CDD_BUILD_TESTS
  {
    int j;
    for (j = 1; j < 60; j++) {
      g_cdd_cfg_alloc_fail = j;
      rc = cdd_cst_cfg_build(func, &cfg);
      if (cfg) {
        cdd_cst_cfg_free(cfg);
        cfg = NULL;
        cfg = NULL;
      }
      if (rc == CDD_C_SUCCESS)
        break;
    }
    g_cdd_cfg_alloc_fail = 0;
  }
#endif
  cdd_cst_tree_free(tree);
  PASS();
}

TEST test_cdd_cst_cfg_if_returns_manual(void) {
  cdd_c_error_t rc;
  cdd_cst_cfg_t *cfg = NULL;
  cdd_cst_node_t *func, *blk, *stmt, *then_stmt, *else_stmt;
  cdd_token_t ret_tok;

  memset(&ret_tok, 0, sizeof(ret_tok));
  ret_tok.kind = CDD_TOKEN_KEYWORD_RETURN;

  cdd_cst_alloc_node(CDD_CST_FUNCTION_DEFINITION, &func);
  cdd_cst_alloc_node(CDD_CST_BLOCK, &blk);
  cdd_cst_alloc_node(CDD_CST_STATEMENT, &stmt);
  cdd_cst_alloc_node(CDD_CST_STATEMENT, &then_stmt);
  cdd_cst_alloc_node(CDD_CST_STATEMENT, &else_stmt);

  cdd_cst_append_child_token(then_stmt, &ret_tok);
  cdd_cst_append_child_token(else_stmt, &ret_tok);

  cdd_cst_append_child_node(stmt, then_stmt);
  cdd_cst_append_child_node(stmt, else_stmt);

  cdd_cst_append_child_node(func, blk);
  cdd_cst_append_child_node(blk, stmt);

  rc = cdd_cst_cfg_build(func, &cfg);
  ASSERT_EQ(0, rc);
  ASSERT(cfg != NULL);

  cdd_cst_cfg_free(cfg);
  cfg = NULL;

#ifdef CDD_BUILD_TESTS
  {
    int j;
    for (j = 1; j < 60; j++) {
      g_cdd_cfg_alloc_fail = j;
      rc = cdd_cst_cfg_build(func, &cfg);
      if (cfg) {
        cdd_cst_cfg_free(cfg);
        cfg = NULL;
        cfg = NULL;
      }
      if (rc == CDD_C_SUCCESS)
        break;
    }
    g_cdd_cfg_alloc_fail = 0;
  }
#endif
  cdd_cst_free_node_only(stmt);
  cdd_cst_free_node_only(then_stmt);
  cdd_cst_free_node_only(else_stmt);
  cdd_cst_free_node_only(blk);
  cdd_cst_free_node_only(func);
  PASS();
}

TEST test_cdd_cst_cfg_loop(void) {
  cdd_c_error_t rc;
  cdd_cst_cfg_t *cfg = NULL;
  cdd_cst_node_t *func, *blk, *stmt, *cond, *body;
  cdd_token_t for_tok;
  cdd_cst_child_t child_tok;

  memset(&for_tok, 0, sizeof(for_tok));
  for_tok.kind = CDD_TOKEN_KEYWORD_GOTO;

  child_tok.kind = CDD_CST_CHILD_TOKEN;
  child_tok.val.token = &for_tok;

  cdd_cst_alloc_node(CDD_CST_FUNCTION_DEFINITION, &func);
  cdd_cst_alloc_node(CDD_CST_BLOCK, &blk);
  cdd_cst_alloc_node(CDD_CST_STATEMENT, &stmt);
  cdd_cst_alloc_node(CDD_CST_EXPRESSION, &cond);
  cdd_cst_alloc_node(CDD_CST_BLOCK, &body);

  cdd_cst_append_child_node(func, blk);
  cdd_cst_append_child_node(blk, stmt);
  cdd_cst_append_child_token(stmt, &for_tok);
  cdd_cst_append_child_node(stmt, cond);
  cdd_cst_append_child_node(stmt, body);

  rc = cdd_cst_cfg_build(func, &cfg);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  cdd_cst_cfg_free(cfg);
  cfg = NULL;

  cdd_cst_free_node(func);
  PASS();
}

SUITE(cdd_cst_cfg_suite) {
  RUN_TEST(test_cdd_cst_cfg_if_returns);
  RUN_TEST(test_cdd_cst_cfg_if_returns_manual);
  RUN_TEST(test_cdd_cst_cfg_loop);
  RUN_TEST(test_cdd_cst_cfg_if_else);
  RUN_TEST(test_cdd_cst_cfg_if_only);
  RUN_TEST(test_cdd_cst_cfg_dead_code);
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
