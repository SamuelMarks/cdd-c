#ifndef C_CDD_TESTS_TEST_FUNCTION_H
#define C_CDD_TESTS_TEST_FUNCTION_H

#include <greatest.h>

#include "cst.h"

static const char sum_func_src[] = "int sum(int a, int b) { return a + b; }";

TEST x_test_function_scanned(void) {
  const char **scanned = scanner(sum_func_src);
  ASSERT_EQ(scanned, NULL);
  PASS();
}

TEST x_test_function_parsed(void) {
  const char **scanned = scanner(sum_func_src);
  const struct CstNode **parsed = parser(scanned);
  enum Keywords int_specifier[] = {INT};
  struct Arg a_arg = {/* pos_start */ 0,
                      /* scope */ NULL,
                      /* value */ "int a",
                      /* specifiers */ int_specifier,
                      /* name */ "a"};
  struct Arg b_arg = {/* pos_start */ 0,
                      /* scope */ NULL,
                      /* value */ "int b",
                      /* specifiers */ int_specifier,
                      /* name */ "b"};
  struct Arg args[] = {&a_arg, &b_arg};
  struct Return _return = {/* pos_start */ 0,
                           /* scope */ NULL,
                           /* value */ "return a + b;",
                           /* val */ "a + b"};

  const struct Function sum_func = {/* pos_start */ 0,
                                    /* scope */ NULL,
                                    /* value */ sum_func_src,
                                    /* specifiers */ int_specifier,
                                    /* name */ "sum",
                                    /* args */
                                    {
                                        a_arg,
                                        b_arg,
                                    },
                                    /* body */ {_return}};
  ASSERT_EQ(parsed, NULL);
  /* TODO: loop through scanned, assert contents contain expected information */
  /* TODO: loop through parsed, assert contents contain expected information */
  PASS();
}

/* Suites can group multiple tests with common setup. */
SUITE(function_suite) {
  RUN_TEST(x_test_function_scanned);
  RUN_TEST(x_test_function_parsed);
}

#endif /* !C_CDD_TESTS_TEST_FUNCTION_H */
