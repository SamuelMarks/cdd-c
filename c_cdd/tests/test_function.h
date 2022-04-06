#ifndef C_CDD_TESTS_TEST_FUNCTION_H
#define C_CDD_TESTS_TEST_FUNCTION_H

#include <greatest.h>

#include "cst.h"

static const char sum_func_src[] = "int sum(int a, int b) { return a + b; }";

TEST x_test_function_scanned(void) {
  const char **scanned = scanner(sum_func_src);
  ASSERT_EQ(scanned, NULL);
  /* TODO: loop through scanned, assert contents contain expected information */
  PASS();
}

TEST x_test_function_parsed(void) {
  const char **scanned = scanner(sum_func_src);
  const struct CstNode **parsed = parser(scanned);
  static enum Keywords int_specifier[] = {INT};
  static const struct Arg a_arg = {/* pos_start */ 0,
                                   /* scope */ NULL,
                                   /* value */ "int a",
                                   /* specifiers */ int_specifier,
                                   /* name */ "a"};
  static const struct Arg b_arg = {/* pos_start */ 0,
                                   /* scope */ NULL,
                                   /* value */ "int b",
                                   /* specifiers */ int_specifier,
                                   /* name */ "b"};
  struct Arg args[2] = {a_arg, b_arg};
  struct Return _return = {/* pos_start */ 0,
                           /* scope */ NULL,
                           /* value */ "return a + b;",
                           /* val */ "a + b"};
  struct CstNode return_cst_node = {Return};
  struct CstNode sum_func_body[1];
  return_cst_node._return = _return;
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

/* Suites can group multiple tests with common setup. */
SUITE(function_suite) {
  RUN_TEST(x_test_function_scanned);
  RUN_TEST(x_test_function_parsed);
}

#endif /* !C_CDD_TESTS_TEST_FUNCTION_H */
