#ifndef C_CDD_TESTS_TEST_FUNCTION_H
#define C_CDD_TESTS_TEST_FUNCTION_H

#include <greatest.h>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#include <c89stringutils_string_extras.h>
#endif /* defined(WIN32) || defined(_WIN32) || defined(__WIN32__) ||           \
          defined(__NT__) */

#include <c_str_precondition_internal.h>

#include <c_cdd_utils.h>
#include <cst.h>

#include <cdd_helpers.h>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#define NUM_LONG_FMT "z"
#else
#define NUM_LONG_FMT "l"
#endif

static const char sum_func_src[] = "int sum(int a, int b) { return a + b; }";

TEST x_test_function_scanned(void) {
  const az_span sum_func_span = az_span_create_from_str((char *)sum_func_src);
  struct scan_az_span_list *const scanned = scanner(sum_func_span);
  enum { n = 26 };
  size_t i;
  static const char *scanned_str_l[] = {"int sum(int a, int b) ",
                                        "{ return a + b", "; ", "}"};

  struct scan_az_span_elem *iter;

  struct StrScannerKind scanned_l[n] = {
      {"int", WORD},     {" ", WHITESPACE}, {"sum", WORD},
      {"(", LPAREN},     {"int", WORD},     {" ", WHITESPACE},
      {"a", WORD},       {",", COMMA},      {" ", WHITESPACE},
      {"int", WORD},     {" ", WHITESPACE}, {"b", WORD},
      {")", RPAREN},     {" ", WHITESPACE}, {"{", LBRACE},
      {" ", WHITESPACE}, {"return", WORD},  {" ", WHITESPACE},
      {"a", WORD},       {" ", WHITESPACE}, {"+", PLUS},
      {" ", WHITESPACE}, {"b", WORD},       {";", TERMINATOR},
      {" ", WHITESPACE}, {"}", RBRACE}};

  ASSERT_EQ(scanned->size, n);

  for (iter = (struct scan_az_span_elem *)scanned->list, i = 0; iter != NULL;
       iter = iter->next, i++) {
    const size_t n = az_span_size(iter->span) + 1;
    char *iter_s = malloc(n);
    az_span_to_str(iter_s, n, iter->span);
    ASSERT_STR_EQ(scanned_l[i].s, iter_s);
    ASSERT_EQ(scanned_l[i].kind, iter->kind);
    free(iter_s);
  }
  ASSERT_EQ(scanned->size, i);
  scan_az_span_list_cleanup(scanned);
  ASSERT_EQ(scanned->size, 0);
  ASSERT_EQ(scanned->list, NULL);

  PASS();
}

TEST x_test_function_tokenizer(void) {
  const az_span sum_func_span = az_span_create_from_str((char *)sum_func_src);
  struct scan_az_span_list *const scanned = scanner(sum_func_span);
  const struct az_span_elem *tokens =
      tokenizer((struct scan_az_span_list *)scanned->list);
  scan_az_span_list_cleanup(scanned);
  PASS();
}

TEST x_test_function_parsed(void) {
  const az_span sum_func_span = az_span_create_from_str((char *)sum_func_src);
  struct scan_az_span_list *const scanned = scanner(sum_func_span);
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
  az_precondition_failed_set_callback(cdd_precondition_failed);
  RUN_TEST(x_test_function_scanned);
  /*RUN_TEST(x_test_function_tokenizer);*/
  /*RUN_TEST(x_test_function_parsed);*/
}

#endif /* !C_CDD_TESTS_TEST_FUNCTION_H */
