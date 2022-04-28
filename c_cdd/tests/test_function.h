#ifndef C_CDD_TESTS_TEST_FUNCTION_H
#define C_CDD_TESTS_TEST_FUNCTION_H

#include <greatest.h>

#include "cst.h"

static const char sum_func_src[] = "int sum(int a, int b) { return a + b; }";

TEST x_test_function_scanned(void) {
  const az_span sum_func_span = az_span_create_from_str((char *)sum_func_src);
  const struct az_span_elem *scanned = scanner(sum_func_span);
  struct az_span_elem *iter;
  enum { n = 4 };
  size_t i = 0;
  static const char *scanned_str_l[n] = {"int sum(int a, int b) ", "{",
                                         " return a + b;", " }"};

  for (iter = (struct az_span_elem *)scanned; iter != NULL; iter = iter->next) {
    const size_t n = az_span_size(iter->span);
    char *iter_s = malloc(n + 1);
    az_span_to_str(iter_s, (int32_t)n, iter->span);
    ASSERT_STR_EQ(iter_s, scanned_str_l[i++]);
    free(iter_s);
  }
  PASS();
}

TEST x_test_function_tokenizer(void) {
  const az_span sum_func_span = az_span_create_from_str((char *)sum_func_src);
  const struct az_span_elem *scanned = scanner(sum_func_span);
  const struct az_span_elem *tokens = tokenizer((struct az_span_elem *)scanned);
  PASS();
}

TEST x_test_function_parsed(void) {
  const az_span sum_func_span = az_span_create_from_str((char *)sum_func_src);
  const struct az_span_elem *scanned = scanner(sum_func_span);
  const struct CstNode **parsed = parser((struct az_span_elem *)scanned);
  //  static enum TypeSpecifier int_specifier[] = {INT};
  //  static const struct Declaration a_arg = {/* pos_start */ 0,
  //                                           /* scope */ NULL,
  //                                           /* value */ "int a",
  //                                           /* specifiers */ int_specifier,
  //                                           /* type */ NULL,
  //                                           /* name */ "a"};
  //  static const struct Declaration b_arg = {/* pos_start */ 0,
  //                                           /* scope */ NULL,
  //                                           /* value */ "int b",
  //                                           /* specifiers */ int_specifier,
  //                                           /* type */ NULL,
  //                                           /* name */ "b"};
  //  struct Declaration args[2];
  //  struct Return _return = {/* pos_start */ 0,
  //                           /* scope */ NULL,
  //                           /* value */ "return a + b;",
  //                           /* val */ "a + b"};
  //  struct CstNode return_cst_node = {Return};
  //  struct CstNode sum_func_body[1];
  //  return_cst_node.type._return = _return;
  //  sum_func_body[0] = return_cst_node;
  //
  //  args[0] = a_arg;
  //  args[1] = b_arg;
  //
  //  {
  //    struct Function sum_func = {/* pos_start */ 0,
  //                                /* scope */ NULL,
  //                                /* value */ sum_func_src,
  //                                /* specifiers */ int_specifier,
  //                                /* name */ "sum",
  //                                /* args */ NULL,
  //                                /* body */ NULL};
  //    sum_func.args = args;
  //    sum_func.body = sum_func_body;
  //
  //    ASSERT_EQ(parsed, NULL);
  //    /* TODO: loop through parsed, assert contents contain expected
  //    information
  //     */ }
  PASS();
}

SUITE(function_suite) {
  RUN_TEST(x_test_function_scanned);
  RUN_TEST(x_test_function_tokenizer);
  /*RUN_TEST(x_test_function_parsed);*/
}

#endif /* !C_CDD_TESTS_TEST_FUNCTION_H */
