#ifndef C_CDD_TESTS_TEST_FUNCTION_H
#define C_CDD_TESTS_TEST_FUNCTION_H

#include <greatest.h>

#include "cst.h"

static const char sum_func_src[] = "int sum(int a, int b) { return a + b; }";

TEST x_test_function_scanned(void) {
  struct str_elem *scanned = (struct str_elem *)scanner(sum_func_src);
  struct str_elem *iter;
  enum { n = 4 };
  size_t i = 0;
  static const char *scanned_str_l[n] = {"int sum(int a, int b) ", "{",
                                         " return a + b;", " }"};

  for (iter = scanned; iter != NULL; iter = iter->next)
    ASSERT_STR_EQ(iter->s, scanned_str_l[i++]);
  PASS();
}

TEST x_test_function_parsed(void) {
  const struct str_elem *scanned = scanner(sum_func_src);
  const struct CstNode **parsed = parser((struct str_elem *)scanned);
  static enum Keywords int_specifier[] = {INT};
  static const struct Declaration a_arg = {/* pos_start */ 0,
                                           /* scope */ NULL,
                                           /* value */ "int a",
                                           /* specifiers */ int_specifier,
                                           /* type */ NULL,
                                           /* name */ "a"};
  static const struct Declaration b_arg = {/* pos_start */ 0,
                                           /* scope */ NULL,
                                           /* value */ "int b",
                                           /* specifiers */ int_specifier,
                                           /* type */ NULL,
                                           /* name */ "b"};
  struct Declaration args[2];
  struct Return _return = {/* pos_start */ 0,
                           /* scope */ NULL,
                           /* value */ "return a + b;",
                           /* val */ "a + b"};
  struct CstNode return_cst_node = {Return};
  struct CstNode sum_func_body[1];
  return_cst_node.type._return = _return;
  sum_func_body[0] = return_cst_node;

  args[0] = a_arg;
  args[1] = b_arg;

  {
    struct Function sum_func = {/* pos_start */ 0,
                                /* scope */ NULL,
                                /* value */ sum_func_src,
                                /* specifiers */ int_specifier,
                                /* name */ "sum",
                                /* args */ NULL,
                                /* body */ NULL};
    sum_func.args = args;
    sum_func.body = sum_func_body;

    ASSERT_EQ(parsed, NULL);
    /* TODO: loop through parsed, assert contents contain expected information
     */
    PASS();
  }
}

SUITE(function_suite) {
  RUN_TEST(x_test_function_scanned);
  RUN_TEST(x_test_function_parsed);
}

#endif /* !C_CDD_TESTS_TEST_FUNCTION_H */
