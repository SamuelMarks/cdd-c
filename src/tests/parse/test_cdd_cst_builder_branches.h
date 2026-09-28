/**
 * @file test_cdd_cst_builder_branches.h
 * @brief Unit tests for CST builder.
 */

#ifndef TEST_CDD_CST_BUILDER_BRANCHES_H
#define TEST_CDD_CST_BUILDER_BRANCHES_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include <greatest.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "classes/parse/cdd_cst_parser.h"
#include "classes/parse/cdd_cst_builder.h"
#include "classes/emit/cdd_cst_emit.h"
#include "classes/parse/cdd_cst_factory.h"
/* clang-format on */

TEST test_cdd_cst_builder_long_token(void) {
  cdd_cst_tree_t *tree = NULL;
  cdd_cst_builder_t b;
  cdd_cst_node_t *node = NULL;
  char long_tok[2055];
  memset(long_tok, 'a', sizeof(long_tok) - 1);
  long_tok[sizeof(long_tok) - 1] = '\0';
  cdd_cst_parse(az_span_create_from_str((char *)(size_t) ""), &tree);
  node = tree->root;
  cdd_cst_builder_init(&b, tree, node);
  ASSERT_EQ(0, cdd_cst_bld_snippet(&b, long_tok));
  cdd_cst_tree_free(tree);
  PASS();
}

/**
 * @brief Test branch coverage for cdd_cst_builder
 * @return TEST
 */
TEST test_cdd_cst_builder_branches(void) {
  extern C_CDD_EXPORT int g_cdd_cst_alloc_token_fail;
  cdd_cst_tree_t *tree = NULL;
  cdd_cst_builder_t b;
  cdd_cst_node_t *node = NULL;

  int fail_i;
  cdd_cst_parse(az_span_create_from_str((char *)(size_t) ""), &tree);
  node = tree->root;
  cdd_cst_builder_init(&b, tree, node);

  /* 1. cdd_cst_bld_int token fail */
  b.target_node = NULL;
  ASSERT(cdd_cst_bld_int(&b, 42) != CDD_C_SUCCESS);
  b.target_node = node;
  b.error_state = 0;

  /* 2. cdd_cst_bld_include branches */
  for (fail_i = 1; fail_i <= 6; fail_i++) {
    g_cdd_cst_alloc_token_fail = fail_i;
    cdd_cst_bld_include(&b, "test.h", 1);
    g_cdd_cst_alloc_token_fail = 0;
    b.error_state = 0;
  }
  for (fail_i = 1; fail_i <= 6; fail_i++) {
    g_cdd_cst_alloc_token_fail = fail_i;
    cdd_cst_bld_include(&b, "test.h", 0);
    g_cdd_cst_alloc_token_fail = 0;
    b.error_state = 0;
  }

  /* 3. cdd_cst_bld_ifndef branches */
  g_cdd_cst_alloc_token_fail = 1;
  ASSERT(cdd_cst_bld_ifndef(&b, "TEST") != CDD_C_SUCCESS);
  g_cdd_cst_alloc_token_fail = 0;
  b.error_state = 0;

  g_cdd_cst_alloc_token_fail = 2;
  ASSERT(cdd_cst_bld_ifndef(&b, "TEST") != CDD_C_SUCCESS);
  g_cdd_cst_alloc_token_fail = 0;
  b.error_state = 0;

  g_cdd_cst_alloc_token_fail = 3;
  ASSERT(cdd_cst_bld_ifndef(&b, "TEST") != CDD_C_SUCCESS);
  g_cdd_cst_alloc_token_fail = 0;
  b.error_state = 0;

  g_cdd_cst_alloc_token_fail = 4;
  ASSERT(cdd_cst_bld_ifndef(&b, "TEST") != CDD_C_SUCCESS);
  g_cdd_cst_alloc_token_fail = 0;
  b.error_state = 0;

  /* 4. cdd_cst_bld_else branches */
  g_cdd_cst_alloc_token_fail = 1;
  ASSERT(cdd_cst_bld_else(&b) != CDD_C_SUCCESS);
  g_cdd_cst_alloc_token_fail = 0;
  b.error_state = 0;

  g_cdd_cst_alloc_token_fail = 2;
  ASSERT(cdd_cst_bld_else(&b) != CDD_C_SUCCESS);
  g_cdd_cst_alloc_token_fail = 0;
  b.error_state = 0;

  /* 5. cdd_cst_quote with %d and error */
  ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_quote(&b, "%d", 42));
  b.target_node = NULL;
  ASSERT(cdd_cst_quote(&b, "%d", 42) != CDD_C_SUCCESS);
  b.target_node = node;
  b.error_state = 0;

  /* 6. cdd_cst_splice_nodes memory fail */
  {
    cdd_cst_node_t *s1 = NULL;
    cdd_cst_node_t *s2 = NULL;
    cdd_cst_alloc_node(CDD_CST_TRANSLATION_UNIT, &s1);
    g_cdd_cst_alloc_token_fail = 2;
    cdd_cst_splice_nodes(&b, node, 0, &s1, 1);
    g_cdd_cst_alloc_token_fail = 0;
    b.error_state = 0;

    cdd_cst_alloc_node(CDD_CST_TRANSLATION_UNIT, &s2);
    g_cdd_cst_alloc_token_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_cst_splice_nodes(&b, node, 0, &s2, 1));
    g_cdd_cst_alloc_token_fail = 0;
    b.error_state = 0;
    cdd_cst_free_node(s2);
  }

  /* 7. cdd_cst_splice_nodes parameter & branch checks */
  {
    cdd_cst_node_t *test_node = NULL;
    cdd_cst_alloc_node(CDD_CST_STATEMENT, &test_node);
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_cst_splice_nodes(NULL, test_node, 0, NULL, 0));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_cst_splice_nodes(&b, NULL, 0, NULL, 0));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_cst_splice_nodes(&b, test_node, 0, NULL, 1));
    b.error_state = CDD_C_ERROR_INVALID_ARGUMENT;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_cst_splice_nodes(&b, test_node, 0, NULL, 0));
    b.error_state = 0;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_splice_nodes(&b, test_node, 0, NULL, 0));
    {
      cdd_cst_node_t *s = NULL;
      cdd_cst_alloc_node(CDD_CST_STATEMENT, &s);
      ASSERT(cdd_cst_splice_nodes(&b, test_node, 9999, &s, 1) != CDD_C_SUCCESS);
      b.error_state = 0;
      cdd_cst_free_node(s);
    }
    cdd_cst_free_node(test_node);
  }

  /* 8. cdd_cst_replace_node_preserve_trivia parameter & branch checks */
  {
    cdd_cst_node_t *target = NULL;
    cdd_cst_node_t *repl = NULL;
    cdd_cst_alloc_node(CDD_CST_STATEMENT, &target);
    cdd_cst_alloc_node(CDD_CST_STATEMENT, &repl);

    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_cst_replace_node_preserve_trivia(NULL, target, repl));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_cst_replace_node_preserve_trivia(&b, NULL, repl));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_cst_replace_node_preserve_trivia(&b, target, NULL));

    b.error_state = CDD_C_ERROR_INVALID_ARGUMENT;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_cst_replace_node_preserve_trivia(&b, target, repl));
    b.error_state = 0;

    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_cst_replace_node_preserve_trivia(&b, target, repl));
    b.error_state = 0;

    target->parent = tree->root;
    ASSERT_EQ(CDD_C_ERROR_NOT_FOUND,
              cdd_cst_replace_node_preserve_trivia(&b, target, repl));
    target->parent = NULL;
    b.error_state = 0;

    {
      cdd_cst_child_t bad_child;
      bad_child.kind = CDD_CST_CHILD_NODE;
      bad_child.val.node = NULL;
      target->children = &bad_child;
      target->num_children = 1;
      ASSERT(cdd_cst_replace_node_preserve_trivia(&b, target, repl) !=
             CDD_C_SUCCESS);
      target->children = NULL;
      target->num_children = 0;
    }

    cdd_cst_free_node(target);
    cdd_cst_free_node(repl);
  }

  /* 9. cdd_cst_transfer_trivia checks */
  {
    cdd_cst_node_t *n_src = NULL;
    cdd_cst_node_t *n_dst = NULL;
    cdd_token_t tok_src;
    cdd_token_t tok_dst;
    cdd_cst_child_t ch_src;
    cdd_cst_child_t ch_dst;
    cdd_trivia_t *l1 = (cdd_trivia_t *)C_CDD_CALLOC(1, sizeof(cdd_trivia_t));
    cdd_trivia_t *l2 = (cdd_trivia_t *)C_CDD_CALLOC(1, sizeof(cdd_trivia_t));
    cdd_trivia_t *t1 = (cdd_trivia_t *)C_CDD_CALLOC(1, sizeof(cdd_trivia_t));
    cdd_trivia_t *t2 = (cdd_trivia_t *)C_CDD_CALLOC(1, sizeof(cdd_trivia_t));
    cdd_trivia_t *cur;

    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_cst_transfer_trivia(NULL, node));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_cst_transfer_trivia(node, NULL));

    l1->next = l2;
    t1->next = t2;
    memset(&tok_src, 0, sizeof(tok_src));
    tok_src.leading_trivia = l1;
    tok_src.trailing_trivia = t1;

    cdd_cst_alloc_node(CDD_CST_STATEMENT, &n_src);
    cdd_cst_alloc_node(CDD_CST_STATEMENT, &n_dst);

    ch_src.kind = CDD_CST_CHILD_TOKEN;
    ch_src.val.token = &tok_src;
    n_src->children = &ch_src;
    n_src->num_children = 1;

    /* n_dst has no tokens -> hits else if (lead) and else if (trail) freeing
     * loops */
    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_transfer_trivia(n_src, n_dst));

    /* now source with no trivia to dst with tokens */
    memset(&tok_dst, 0, sizeof(tok_dst));
    ch_dst.kind = CDD_CST_CHILD_TOKEN;
    ch_dst.val.token = &tok_dst;
    n_dst->children = &ch_dst;
    n_dst->num_children = 1;

    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_transfer_trivia(n_src, n_dst));

    /* source with >1 lead and >1 trail to dst with tokens having
     * trailing_trivia == NULL */
    l1 = (cdd_trivia_t *)C_CDD_CALLOC(1, sizeof(cdd_trivia_t));
    l2 = (cdd_trivia_t *)C_CDD_CALLOC(1, sizeof(cdd_trivia_t));
    t1 = (cdd_trivia_t *)C_CDD_CALLOC(1, sizeof(cdd_trivia_t));
    t2 = (cdd_trivia_t *)C_CDD_CALLOC(1, sizeof(cdd_trivia_t));
    l1->next = l2;
    t1->next = t2;
    tok_src.leading_trivia = l1;
    tok_src.trailing_trivia = t1;

    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_transfer_trivia(n_src, n_dst));

    cur = tok_dst.leading_trivia;
    while (cur) {
      cdd_trivia_t *next = cur->next;
      C_CDD_FREE(cur);
      cur = next;
    }
    cur = tok_dst.trailing_trivia;
    while (cur) {
      cdd_trivia_t *next = cur->next;
      C_CDD_FREE(cur);
      cur = next;
    }
    tok_dst.leading_trivia = NULL;
    tok_dst.trailing_trivia = NULL;

    /* dst ALREADY having >1 trailing trivia */
    l1 = (cdd_trivia_t *)C_CDD_CALLOC(1, sizeof(cdd_trivia_t));
    l2 = (cdd_trivia_t *)C_CDD_CALLOC(1, sizeof(cdd_trivia_t));
    t1 = (cdd_trivia_t *)C_CDD_CALLOC(1, sizeof(cdd_trivia_t));
    t2 = (cdd_trivia_t *)C_CDD_CALLOC(1, sizeof(cdd_trivia_t));
    l1->next = l2;
    t1->next = t2;
    tok_src.leading_trivia = l1;
    tok_src.trailing_trivia = t1;

    {
      cdd_trivia_t *dt1 = (cdd_trivia_t *)C_CDD_CALLOC(1, sizeof(cdd_trivia_t));
      cdd_trivia_t *dt2 = (cdd_trivia_t *)C_CDD_CALLOC(1, sizeof(cdd_trivia_t));
      dt1->next = dt2;
      tok_dst.trailing_trivia = dt1;
    }

    ASSERT_EQ(CDD_C_SUCCESS, cdd_cst_transfer_trivia(n_src, n_dst));

    cur = tok_dst.leading_trivia;
    while (cur) {
      cdd_trivia_t *next = cur->next;
      C_CDD_FREE(cur);
      cur = next;
    }
    cur = tok_dst.trailing_trivia;
    while (cur) {
      cdd_trivia_t *next = cur->next;
      C_CDD_FREE(cur);
      cur = next;
    }

    /* Error branches in cdd_cst_transfer_trivia */
    {
      cdd_token_t tok_err;
      cdd_cst_child_t ch_err[2];
      cdd_cst_child_t ch_dst_err[2];
      memset(&tok_err, 0, sizeof(tok_err));

      /* source extract_leading_trivia error */
      ch_err[0].kind = CDD_CST_CHILD_NODE;
      ch_err[0].val.node = NULL;
      n_src->children = ch_err;
      n_src->num_children = 1;
      ASSERT(cdd_cst_transfer_trivia(n_src, n_dst) != CDD_C_SUCCESS);

      /* source extract_trailing_trivia error with multi-item leading trivia */
      {
        cdd_trivia_t *el1 =
            (cdd_trivia_t *)C_CDD_CALLOC(1, sizeof(cdd_trivia_t));
        cdd_trivia_t *el2 =
            (cdd_trivia_t *)C_CDD_CALLOC(1, sizeof(cdd_trivia_t));
        el1->next = el2;
        tok_err.leading_trivia = el1;
      }
      ch_err[0].kind = CDD_CST_CHILD_TOKEN;
      ch_err[0].val.token = &tok_err;
      ch_err[1].kind = CDD_CST_CHILD_NODE;
      ch_err[1].val.node = NULL;
      n_src->children = ch_err;
      n_src->num_children = 2;
      ASSERT(cdd_cst_transfer_trivia(n_src, n_dst) != CDD_C_SUCCESS);

      /* source extract_trailing_trivia error without leading trivia */
      memset(&tok_err, 0, sizeof(tok_err));
      ch_err[0].kind = CDD_CST_CHILD_TOKEN;
      ch_err[0].val.token = &tok_err;
      ch_err[1].kind = CDD_CST_CHILD_NODE;
      ch_err[1].val.node = NULL;
      n_src->children = ch_err;
      n_src->num_children = 2;
      ASSERT(cdd_cst_transfer_trivia(n_src, n_dst) != CDD_C_SUCCESS);

      /* target get_first_token error with multi-item lead and trail */
      {
        cdd_trivia_t *el1 =
            (cdd_trivia_t *)C_CDD_CALLOC(1, sizeof(cdd_trivia_t));
        cdd_trivia_t *el2 =
            (cdd_trivia_t *)C_CDD_CALLOC(1, sizeof(cdd_trivia_t));
        cdd_trivia_t *et1 =
            (cdd_trivia_t *)C_CDD_CALLOC(1, sizeof(cdd_trivia_t));
        cdd_trivia_t *et2 =
            (cdd_trivia_t *)C_CDD_CALLOC(1, sizeof(cdd_trivia_t));
        el1->next = el2;
        et1->next = et2;
        memset(&tok_err, 0, sizeof(tok_err));
        tok_err.leading_trivia = el1;
        tok_err.trailing_trivia = et1;
        ch_err[0].kind = CDD_CST_CHILD_TOKEN;
        ch_err[0].val.token = &tok_err;
        n_src->children = ch_err;
        n_src->num_children = 1;

        ch_dst_err[0].kind = CDD_CST_CHILD_NODE;
        ch_dst_err[0].val.node = NULL;
        n_dst->children = ch_dst_err;
        n_dst->num_children = 1;
        ASSERT(cdd_cst_transfer_trivia(n_src, n_dst) != CDD_C_SUCCESS);
      }

      /* target get_last_token error with multi-item lead and trail */
      {
        cdd_token_t tok_dst_item;
        cdd_trivia_t *el1 =
            (cdd_trivia_t *)C_CDD_CALLOC(1, sizeof(cdd_trivia_t));
        cdd_trivia_t *el2 =
            (cdd_trivia_t *)C_CDD_CALLOC(1, sizeof(cdd_trivia_t));
        cdd_trivia_t *et1 =
            (cdd_trivia_t *)C_CDD_CALLOC(1, sizeof(cdd_trivia_t));
        cdd_trivia_t *et2 =
            (cdd_trivia_t *)C_CDD_CALLOC(1, sizeof(cdd_trivia_t));
        el1->next = el2;
        et1->next = et2;
        memset(&tok_err, 0, sizeof(tok_err));
        tok_err.leading_trivia = el1;
        tok_err.trailing_trivia = et1;
        ch_err[0].kind = CDD_CST_CHILD_TOKEN;
        ch_err[0].val.token = &tok_err;
        n_src->children = ch_err;
        n_src->num_children = 1;

        memset(&tok_dst_item, 0, sizeof(tok_dst_item));
        ch_dst_err[0].kind = CDD_CST_CHILD_TOKEN;
        ch_dst_err[0].val.token = &tok_dst_item;
        ch_dst_err[1].kind = CDD_CST_CHILD_NODE;
        ch_dst_err[1].val.node = NULL;
        n_dst->children = ch_dst_err;
        n_dst->num_children = 2;
        ASSERT(cdd_cst_transfer_trivia(n_src, n_dst) != CDD_C_SUCCESS);
      }

      /* target get_first_token error without source trivia */
      {
        memset(&tok_err, 0, sizeof(tok_err));
        ch_err[0].kind = CDD_CST_CHILD_TOKEN;
        ch_err[0].val.token = &tok_err;
        n_src->children = ch_err;
        n_src->num_children = 1;

        ch_dst_err[0].kind = CDD_CST_CHILD_NODE;
        ch_dst_err[0].val.node = NULL;
        n_dst->children = ch_dst_err;
        n_dst->num_children = 1;
        ASSERT(cdd_cst_transfer_trivia(n_src, n_dst) != CDD_C_SUCCESS);
      }

      /* target get_last_token error without source trivia */
      {
        cdd_token_t tok_dst_item;
        memset(&tok_err, 0, sizeof(tok_err));
        ch_err[0].kind = CDD_CST_CHILD_TOKEN;
        ch_err[0].val.token = &tok_err;
        n_src->children = ch_err;
        n_src->num_children = 1;

        memset(&tok_dst_item, 0, sizeof(tok_dst_item));
        ch_dst_err[0].kind = CDD_CST_CHILD_TOKEN;
        ch_dst_err[0].val.token = &tok_dst_item;
        ch_dst_err[1].kind = CDD_CST_CHILD_NODE;
        ch_dst_err[1].val.node = NULL;
        n_dst->children = ch_dst_err;
        n_dst->num_children = 2;
        ASSERT(cdd_cst_transfer_trivia(n_src, n_dst) != CDD_C_SUCCESS);
      }
    }

    n_src->children = NULL;
    n_src->num_children = 0;
    n_dst->children = NULL;
    n_dst->num_children = 0;
    cdd_cst_free_node(n_src);
    cdd_cst_free_node(n_dst);
  }

  cdd_cst_tree_free(tree);
  PASS();
}

SUITE(cdd_cst_builder_branches_suite) {
  RUN_TEST(test_cdd_cst_builder_long_token);
  RUN_TEST(test_cdd_cst_builder_branches);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CDD_CST_BUILDER_BRANCHES_H */
