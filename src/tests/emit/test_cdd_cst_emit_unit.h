extern C_CDD_EXPORT int g_fail_io_after;
extern C_CDD_EXPORT int g_io_calls;
#ifndef TEST_CDD_CST_EMIT_UNIT_H
#define TEST_CDD_CST_EMIT_UNIT_H

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */
#include "c_cdd_export.h"
#include "classes/emit/cdd_cst_emit.h"
#include "classes/parse/cdd_cst_factory.h"
#include <greatest.h>
#include <string.h>
/* clang-format on */

TEST test_cdd_cst_emit_invalid(void) {
  cdd_cst_tree_t t = {0};
  char *out = NULL;
  ASSERT_EQ(EINVAL, cdd_cst_emit(NULL, &out));
  ASSERT_EQ(EINVAL, cdd_cst_emit(&t, NULL));
  g_fail_io_after = -1;
  PASS();
}

TEST test_cdd_cst_emit_empty(void) {
  cdd_cst_tree_t tree = {0};
  char *out = NULL;

  /* empty tree -> empty buf -> malloc(1) */
  ASSERT_EQ(0, cdd_cst_emit(&tree, &out));
  ASSERT(out != NULL);
  ASSERT_EQ(0, strlen(out));
  free(out);
  g_fail_io_after = -1;

  PASS();
}

TEST test_cdd_cst_emit_null_children(void) {
  cdd_cst_tree_t tree = {0};
  cdd_cst_node_t root = {0};
  cdd_cst_child_t children[3] = {0};
  char *out = NULL;

  tree.root = &root;
  root.children = children;
  root.num_children = 3;

  /* Child 0 is node, but node is NULL */
  children[0].kind = CDD_CST_CHILD_NODE;
  children[0].val.node = NULL;

  /* Child 1 is token, but token is NULL */
  children[1].kind = CDD_CST_CHILD_TOKEN;
  children[1].val.token = NULL;

  /* Child 2 is unknown kind */
  children[2].kind = (enum cdd_cst_child_kind_t) - 1;

  ASSERT_EQ(0, cdd_cst_emit(&tree, &out));
  ASSERT(out != NULL);
  ASSERT_EQ(0, strlen(out));
  free(out);

  /* Test tree with a node returning error (e.g. from trivia or token) */
  { /* Not easy to simulate ENOMEM without mock, but we can hit EOF logic */
  }
  g_fail_io_after = -1;

  PASS();
}

TEST test_cdd_cst_emit_large_string(void) {
  cdd_cst_tree_t tree = {0};
  cdd_cst_node_t root = {0};
  cdd_token_t tok = {0};
  cdd_trivia_t t1 = {0};
  cdd_trivia_t t2 = {0};
  cdd_trivia_t t3 = {0};
  cdd_token_t eof_tok = {0};
  cdd_cst_child_t eof_child = {0};
  cdd_cst_child_t children[2] = {0};
  char *large_str;
  char *out = NULL;

  large_str = (char *)malloc(5000);
  memset(large_str, 'A', 4999);
  large_str[4999] = '\0';

  t1.start = (const uint8_t *)"/* PRE */";
  t1.length = 9;

  t2.start = (const uint8_t *)"/* POST */";
  t2.length = 10;

  t3.start = (const uint8_t *)"/* EOF_PRE */";
  t3.length = 13;

  tok.kind = CDD_TOKEN_IDENTIFIER;
  tok.start = (const uint8_t *)large_str;
  tok.length = 4999;
  tok.leading_trivia = &t1;
  tok.trailing_trivia = &t2;

  eof_tok.kind = CDD_TOKEN_EOF;
  eof_tok.leading_trivia = &t3;
  eof_child.kind = CDD_CST_CHILD_TOKEN;
  eof_child.val.token = &eof_tok;

  tree.root = &root;
  root.children = children;
  root.num_children = 2;
  children[0].kind = CDD_CST_CHILD_TOKEN;
  children[0].val.token = &tok;
  children[1] = eof_child;

  ASSERT_EQ(0, cdd_cst_emit(&tree, &out));
  ASSERT(out != NULL);
  ASSERT_EQ(4999 + 9 + 10 + 13, strlen(out));

  free(out);
  free(large_str);
  g_fail_io_after = -1;

  PASS();
}

TEST test_cdd_cst_emit_oom(void) {
  cdd_cst_tree_t tree = {0};
  cdd_cst_node_t root = {0};
  cdd_token_t tok = {0};
  cdd_cst_child_t child = {0};
  char *out = NULL;

  /* Test overflow in append_str via tok->length */
  tok.kind = CDD_TOKEN_IDENTIFIER;
  tok.start = (const uint8_t *)"";
  tok.length = (size_t)-1; /* SIZE_MAX */

  tree.root = &root;
  root.children = &child;
  root.num_children = 1;
  child.kind = CDD_CST_CHILD_TOKEN;
  child.val.token = &tok;

  ASSERT_EQ(ENOMEM, cdd_cst_emit(&tree, &out));
  g_fail_io_after = -1;

  PASS();
}

TEST test_cdd_cst_emit_oom_trivia(void) {
  cdd_cst_tree_t tree = {0};
  cdd_cst_node_t root = {0};
  cdd_token_t tok = {0};
  cdd_trivia_t triv = {0};
  cdd_cst_child_t child = {0};
  char *out = NULL;

  triv.start = (const uint8_t *)"";
  triv.length = (size_t)-1; /* SIZE_MAX */

  tok.kind = CDD_TOKEN_IDENTIFIER;
  tok.start = (const uint8_t *)"A";
  tok.length = 1;
  tok.leading_trivia = &triv;

  tree.root = &root;
  root.children = &child;
  root.num_children = 1;
  child.kind = CDD_CST_CHILD_TOKEN;
  child.val.token = &tok;

  ASSERT_EQ(ENOMEM, cdd_cst_emit(&tree, &out));
  g_fail_io_after = -1;

  PASS();
}

TEST test_cdd_cst_emit_oom_realloc(void) {
  if (getenv("RUNNING_UNDER_VALGRIND"))
    SKIPm("Valgrind crash");
  cdd_cst_tree_t tree = {0};
  cdd_cst_node_t root = {0};
  cdd_token_t tok = {0};
  cdd_cst_child_t child = {0};
  char *out = NULL;

  /* Test legitimate realloc failure by requesting an impossibly large block */
  tok.kind = CDD_TOKEN_IDENTIFIER;
  tok.start = (const uint8_t *)"A";
  tok.length =
      ((size_t)-1) - 10; /* Impossibly large, should safely fail realloc */

  tree.root = &root;
  root.children = &child;
  root.num_children = 1;
  child.kind = CDD_CST_CHILD_TOKEN;
  child.val.token = &tok;

  ASSERT_EQ(ENOMEM, cdd_cst_emit(&tree, &out));
  g_fail_io_after = -1;

  PASS();
}

TEST test_cdd_cst_emit_oom_multi(void) {
  cdd_cst_tree_t tree = {0};
  cdd_cst_node_t root = {0};
  cdd_token_t tok1 = {0};
  cdd_token_t tok2 = {0};
  cdd_cst_child_t children[2] = {0};
  char *out = NULL;

  /* First token succeeds and allocates buf */
  tok1.kind = CDD_TOKEN_IDENTIFIER;
  tok1.start = (const uint8_t *)"A";
  tok1.length = 1;

  /* Second token fails via overflow */
  tok2.kind = CDD_TOKEN_IDENTIFIER;
  tok2.start = (const uint8_t *)"";
  tok2.length = (size_t)-1; /* SIZE_MAX */

  tree.root = &root;
  root.children = children;
  root.num_children = 2;

  children[0].kind = CDD_CST_CHILD_TOKEN;
  children[0].val.token = &tok1;
  children[1].kind = CDD_CST_CHILD_TOKEN;
  children[1].val.token = &tok2;

  ASSERT_EQ(ENOMEM, cdd_cst_emit(&tree, &out));
  g_fail_io_after = -1;

  PASS();
}

TEST test_cdd_cst_emit_empty_oom(void) {
  cdd_cst_tree_t tree = {0};
  char *out = NULL;
  extern C_CDD_EXPORT int g_cdd_fail_alloc;

  g_cdd_fail_alloc = 1;
  ASSERT_EQ(ENOMEM, cdd_cst_emit(&tree, &out));
  g_cdd_fail_alloc = 0;
  g_fail_io_after = -1;

  PASS();
}

SUITE(cdd_cst_emit_unit_suite) {
  RUN_TEST(test_cdd_cst_emit_invalid);
  RUN_TEST(test_cdd_cst_emit_empty);
  RUN_TEST(test_cdd_cst_emit_null_children);
  RUN_TEST(test_cdd_cst_emit_large_string);
  RUN_TEST(test_cdd_cst_emit_oom);
  RUN_TEST(test_cdd_cst_emit_oom_trivia);
  /* RUN_TEST(test_cdd_cst_emit_oom_realloc); */ /* Causes internal Valgrind
                                                    crash */
  RUN_TEST(test_cdd_cst_emit_oom_multi);
  RUN_TEST(test_cdd_cst_emit_empty_oom);
}

#ifdef __cplusplus
}
#endif

#endif
