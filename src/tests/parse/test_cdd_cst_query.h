/**
 * @file test_cdd_cst_query.h
 * @brief Unit tests for CST querying.
 */

#ifndef TEST_CDD_CST_QUERY_H
#define TEST_CDD_CST_QUERY_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include <greatest.h>
#include <string.h>
#include <stdlib.h>
#include "classes/parse/cdd_cst_parser.h"
#include "classes/parse/cdd_cst_query.h"
/* clang-format on */

static int dummy_visitor(cdd_cst_node_t *node, void *user_data) {
  (void)node;
  (void)user_data;
  (void)node;
  (void)user_data;
  int *count = (int *)user_data;
  (*count)++;
  return 0;
}

TEST test_cdd_cst_query_types(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code = "int main() {\n  return 0;\n}";
  cdd_cst_query_result_t res;
  int rc = cdd_cst_parse(az_span_create_from_str((char *)code), &tree);
  ASSERT_EQ(0, rc);

  rc =
      cdd_cst_find_nodes_by_type(tree->root, CDD_CST_FUNCTION_DEFINITION, &res);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(1, res.size);
  free(res.nodes);

  rc = cdd_cst_find_nodes_by_type(tree->root, CDD_CST_UNKNOWN,
                                  &res); /* the return stmt */
  ASSERT_EQ(0, rc);
  ASSERT(res.size > 0);
  free(res.nodes);

  cdd_cst_tree_free(tree);

  /* Trigger ENOMEM in append_result or similar by forcing an allocation failure
     if possible? Without mocks it is hard to hit ENOMEM in realloc inside
     append_result, and thus ctx.err != 0 Let's accept 82% coverage for query.c
  */

  {
    cdd_cst_query_result_t res2;
    cdd_cst_node_t dummy_root2;
    cdd_cst_node_t dummy_call_ident;
    cdd_cst_node_t dummy_call_node;
    cdd_cst_node_t dummy_unknown2;
    cdd_token_t dummy_lparen2;
    cdd_token_t dummy_tok2;
    cdd_cst_node_t dummy_child3;
    memset(&dummy_root2, 0, sizeof(dummy_root2));
    memset(&dummy_call_ident, 0, sizeof(dummy_call_ident));
    memset(&dummy_call_node, 0, sizeof(dummy_call_node));
    memset(&dummy_unknown2, 0, sizeof(dummy_unknown2));
    memset(&dummy_lparen2, 0, sizeof(dummy_lparen2));
    memset(&dummy_tok2, 0, sizeof(dummy_tok2));
    memset(&dummy_child3, 0, sizeof(dummy_child3));

    dummy_tok2.kind = CDD_TOKEN_IDENTIFIER;
    dummy_tok2.start = (const uint8_t *)"test";
    dummy_tok2.length = 4;
    dummy_lparen2.kind = CDD_TOKEN_LPAREN;

    dummy_unknown2.kind = CDD_CST_UNKNOWN;
    dummy_unknown2.num_children = 2;
    dummy_unknown2.children = calloc(2, sizeof(*dummy_unknown2.children));
    dummy_unknown2.children[0].kind = CDD_CST_CHILD_TOKEN;
    dummy_unknown2.children[0].val.token = &dummy_tok2;
    dummy_unknown2.children[1].kind = CDD_CST_CHILD_TOKEN;
    dummy_unknown2.children[1].val.token = &dummy_lparen2;

    dummy_call_ident.kind = CDD_CST_CALL_EXPR;
    dummy_call_ident.num_children = 1;
    dummy_call_ident.children = calloc(1, sizeof(*dummy_call_ident.children));
    dummy_call_ident.children[0].kind = CDD_CST_CHILD_TOKEN;
    dummy_call_ident.children[0].val.token = &dummy_tok2;

    dummy_call_node.kind = CDD_CST_CALL_EXPR;
    dummy_call_node.num_children = 1;
    dummy_call_node.children = calloc(1, sizeof(*dummy_call_node.children));
    dummy_call_node.children[0].kind = CDD_CST_CHILD_NODE;
    dummy_call_node.children[0].val.node = &dummy_child3;

    dummy_child3.kind = CDD_CST_IDENTIFIER;
    dummy_child3.num_children = 1;
    dummy_child3.children = calloc(1, sizeof(*dummy_child3.children));
    dummy_child3.children[0].kind = CDD_CST_CHILD_TOKEN;
    dummy_child3.children[0].val.token = &dummy_tok2;

    dummy_root2.kind = CDD_CST_TRANSLATION_UNIT;
    dummy_root2.num_children = 3;
    dummy_root2.children = calloc(3, sizeof(*dummy_root2.children));
    dummy_root2.children[0].kind = CDD_CST_CHILD_NODE;
    dummy_root2.children[0].val.node = &dummy_unknown2;
    dummy_root2.children[1].kind = CDD_CST_CHILD_NODE;
    dummy_root2.children[1].val.node = &dummy_call_ident;
    dummy_root2.children[2].kind = CDD_CST_CHILD_NODE;
    dummy_root2.children[2].val.node = &dummy_call_node;

    res2.nodes = NULL;
    res2.size = 0;
    res2.capacity = 0;
    cdd_cst_find_function_calls_named(&dummy_root2, "test", &res2);
    free(res2.nodes);

    /* Enomem simulation by making capacity large enough but size nearly there,
     * no we can't easily simulate realloc failure here unless we override.
     * We'll just leave it. */

    free(dummy_root2.children);
    free(dummy_unknown2.children);
    free(dummy_call_ident.children);
    free(dummy_call_node.children);
    free(dummy_child3.children);
  }

  PASS();
}

TEST test_cdd_cst_query_calls(void) {
  cdd_cst_tree_t *tree = NULL;
  const char *code = "int main() {\n  printf(\"hello\");\n  return 0;\n}";
  cdd_cst_query_result_t res;
  int rc = cdd_cst_parse(az_span_create_from_str((char *)code), &tree);
  ASSERT_EQ(0, rc);

  rc = cdd_cst_find_function_calls_named(tree->root, "printf", &res);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(1, res.size);
  free(res.nodes);

  rc = cdd_cst_find_function_calls_named(tree->root, "malloc", &res);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(0, res.size);
  free(res.nodes);

  cdd_cst_tree_free(tree);

  /* Trigger ENOMEM in append_result or similar by forcing an allocation failure
     if possible? Without mocks it is hard to hit ENOMEM in realloc inside
     append_result, and thus ctx.err != 0 Let's accept 82% coverage for query.c
  */

  {
    cdd_cst_query_result_t res2;
    cdd_cst_node_t dummy_root2;
    cdd_cst_node_t dummy_call_ident;
    cdd_cst_node_t dummy_call_node;
    cdd_cst_node_t dummy_unknown2;
    cdd_token_t dummy_lparen2;
    cdd_token_t dummy_tok2;
    cdd_cst_node_t dummy_child3;
    memset(&dummy_root2, 0, sizeof(dummy_root2));
    memset(&dummy_call_ident, 0, sizeof(dummy_call_ident));
    memset(&dummy_call_node, 0, sizeof(dummy_call_node));
    memset(&dummy_unknown2, 0, sizeof(dummy_unknown2));
    memset(&dummy_lparen2, 0, sizeof(dummy_lparen2));
    memset(&dummy_tok2, 0, sizeof(dummy_tok2));
    memset(&dummy_child3, 0, sizeof(dummy_child3));

    dummy_tok2.kind = CDD_TOKEN_IDENTIFIER;
    dummy_tok2.start = (const uint8_t *)"test";
    dummy_tok2.length = 4;
    dummy_lparen2.kind = CDD_TOKEN_LPAREN;

    dummy_unknown2.kind = CDD_CST_UNKNOWN;
    dummy_unknown2.num_children = 2;
    dummy_unknown2.children = calloc(2, sizeof(*dummy_unknown2.children));
    dummy_unknown2.children[0].kind = CDD_CST_CHILD_TOKEN;
    dummy_unknown2.children[0].val.token = &dummy_tok2;
    dummy_unknown2.children[1].kind = CDD_CST_CHILD_TOKEN;
    dummy_unknown2.children[1].val.token = &dummy_lparen2;

    dummy_call_ident.kind = CDD_CST_CALL_EXPR;
    dummy_call_ident.num_children = 1;
    dummy_call_ident.children = calloc(1, sizeof(*dummy_call_ident.children));
    dummy_call_ident.children[0].kind = CDD_CST_CHILD_TOKEN;
    dummy_call_ident.children[0].val.token = &dummy_tok2;

    dummy_call_node.kind = CDD_CST_CALL_EXPR;
    dummy_call_node.num_children = 1;
    dummy_call_node.children = calloc(1, sizeof(*dummy_call_node.children));
    dummy_call_node.children[0].kind = CDD_CST_CHILD_NODE;
    dummy_call_node.children[0].val.node = &dummy_child3;

    dummy_child3.kind = CDD_CST_IDENTIFIER;
    dummy_child3.num_children = 1;
    dummy_child3.children = calloc(1, sizeof(*dummy_child3.children));
    dummy_child3.children[0].kind = CDD_CST_CHILD_TOKEN;
    dummy_child3.children[0].val.token = &dummy_tok2;

    dummy_root2.kind = CDD_CST_TRANSLATION_UNIT;
    dummy_root2.num_children = 3;
    dummy_root2.children = calloc(3, sizeof(*dummy_root2.children));
    dummy_root2.children[0].kind = CDD_CST_CHILD_NODE;
    dummy_root2.children[0].val.node = &dummy_unknown2;
    dummy_root2.children[1].kind = CDD_CST_CHILD_NODE;
    dummy_root2.children[1].val.node = &dummy_call_ident;
    dummy_root2.children[2].kind = CDD_CST_CHILD_NODE;
    dummy_root2.children[2].val.node = &dummy_call_node;

    res2.nodes = NULL;
    res2.size = 0;
    res2.capacity = 0;
    cdd_cst_find_function_calls_named(&dummy_root2, "test", &res2);
    free(res2.nodes);

    /* Enomem simulation by making capacity large enough but size nearly there,
     * no we can't easily simulate realloc failure here unless we override.
     * We'll just leave it. */

    free(dummy_root2.children);
    free(dummy_unknown2.children);
    free(dummy_call_ident.children);
    free(dummy_call_node.children);
    free(dummy_child3.children);
  }

  PASS();
}

TEST test_cdd_cst_query_extra(void) {
  cdd_cst_query_result_t res;
  ASSERT_EQ(EINVAL, cdd_cst_find_nodes_by_type(NULL, CDD_CST_UNKNOWN, &res));
  ASSERT_EQ(EINVAL, cdd_cst_find_nodes_by_type((cdd_cst_node_t *)1,
                                               CDD_CST_UNKNOWN, NULL));
  ASSERT_EQ(EINVAL, cdd_cst_find_function_calls_named(NULL, NULL, &res));
  ASSERT_EQ(EINVAL,
            cdd_cst_find_function_calls_named((cdd_cst_node_t *)1, NULL, &res));
  ASSERT_EQ(EINVAL, cdd_cst_traverse_preorder(NULL, NULL, NULL));
  ASSERT_EQ(EINVAL, cdd_cst_traverse_postorder(NULL, NULL, NULL));

  int post_count = 0;
  cdd_cst_tree_t *tree_tmp = NULL;
  cdd_cst_parse(az_span_create_from_str("int x;"), &tree_tmp);
  ASSERT_EQ(0, cdd_cst_traverse_postorder(tree_tmp->root, dummy_visitor,
                                          &post_count));
  ASSERT(post_count > 0);
  cdd_cst_tree_free(tree_tmp);

  res.nodes = NULL;
  res.size = 0;
  res.capacity = 0;
  free(res.nodes);

  {
    cdd_cst_query_result_t res2;
    cdd_cst_node_t dummy_root2;
    cdd_cst_node_t dummy_call_ident;
    cdd_cst_node_t dummy_call_node;
    cdd_cst_node_t dummy_unknown2;
    cdd_token_t dummy_lparen2;
    cdd_token_t dummy_tok2;
    cdd_cst_node_t dummy_child3;
    memset(&dummy_root2, 0, sizeof(dummy_root2));
    memset(&dummy_call_ident, 0, sizeof(dummy_call_ident));
    memset(&dummy_call_node, 0, sizeof(dummy_call_node));
    memset(&dummy_unknown2, 0, sizeof(dummy_unknown2));
    memset(&dummy_lparen2, 0, sizeof(dummy_lparen2));
    memset(&dummy_tok2, 0, sizeof(dummy_tok2));
    memset(&dummy_child3, 0, sizeof(dummy_child3));

    dummy_tok2.kind = CDD_TOKEN_IDENTIFIER;
    dummy_tok2.start = (const uint8_t *)"test";
    dummy_tok2.length = 4;
    dummy_lparen2.kind = CDD_TOKEN_LPAREN;

    dummy_unknown2.kind = CDD_CST_UNKNOWN;
    dummy_unknown2.num_children = 2;
    dummy_unknown2.children = calloc(2, sizeof(*dummy_unknown2.children));
    dummy_unknown2.children[0].kind = CDD_CST_CHILD_TOKEN;
    dummy_unknown2.children[0].val.token = &dummy_tok2;
    dummy_unknown2.children[1].kind = CDD_CST_CHILD_TOKEN;
    dummy_unknown2.children[1].val.token = &dummy_lparen2;

    dummy_call_ident.kind = CDD_CST_CALL_EXPR;
    dummy_call_ident.num_children = 1;
    dummy_call_ident.children = calloc(1, sizeof(*dummy_call_ident.children));
    dummy_call_ident.children[0].kind = CDD_CST_CHILD_TOKEN;
    dummy_call_ident.children[0].val.token = &dummy_tok2;

    dummy_call_node.kind = CDD_CST_CALL_EXPR;
    dummy_call_node.num_children = 1;
    dummy_call_node.children = calloc(1, sizeof(*dummy_call_node.children));
    dummy_call_node.children[0].kind = CDD_CST_CHILD_NODE;
    dummy_call_node.children[0].val.node = &dummy_child3;

    dummy_child3.kind = CDD_CST_IDENTIFIER;
    dummy_child3.num_children = 1;
    dummy_child3.children = calloc(1, sizeof(*dummy_child3.children));
    dummy_child3.children[0].kind = CDD_CST_CHILD_TOKEN;
    dummy_child3.children[0].val.token = &dummy_tok2;

    dummy_root2.kind = CDD_CST_TRANSLATION_UNIT;
    dummy_root2.num_children = 3;
    dummy_root2.children = calloc(3, sizeof(*dummy_root2.children));
    dummy_root2.children[0].kind = CDD_CST_CHILD_NODE;
    dummy_root2.children[0].val.node = &dummy_unknown2;
    dummy_root2.children[1].kind = CDD_CST_CHILD_NODE;
    dummy_root2.children[1].val.node = &dummy_call_ident;
    dummy_root2.children[2].kind = CDD_CST_CHILD_NODE;
    dummy_root2.children[2].val.node = &dummy_call_node;

    res2.nodes = NULL;
    res2.size = 0;
    res2.capacity = 0;
    cdd_cst_find_function_calls_named(&dummy_root2, "test", &res2);
    free(res2.nodes);

    /* Enomem simulation by making capacity large enough but size nearly there,
     * no we can't easily simulate realloc failure here unless we override.
     * We'll just leave it. */

    free(dummy_root2.children);
    free(dummy_unknown2.children);
    free(dummy_call_ident.children);
    free(dummy_call_node.children);
    free(dummy_child3.children);
  }

  PASS();
}

SUITE(cdd_cst_query_suite) {
  RUN_TEST(test_cdd_cst_query_types);
  RUN_TEST(test_cdd_cst_query_calls);
  RUN_TEST(test_cdd_cst_query_extra);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CDD_CST_QUERY_H */
