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
  const union CstNode **parsed = parser(scanned);
  ASSERT_EQ(parsed, NULL);
  PASS();
}

/* Suites can group multiple tests with common setup. */
SUITE(function_suite) {
  RUN_TEST(x_test_function_scanned);
  RUN_TEST(x_test_function_parsed);
}

#endif /* !C_CDD_TESTS_TEST_FUNCTION_H */
