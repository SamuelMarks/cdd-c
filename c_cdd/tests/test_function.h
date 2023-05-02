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

TEST x_test_function_tokenized(void) {
  const az_span sum_func_span = az_span_create_from_str((char *)sum_func_src);
  struct tokenizer_az_span_list *tokenized;
  enum { n = 26 };
  size_t i;
  static const char *const tokenized_str_l[] = {"int sum(int a, int b) ",
                                                "{ return a + b", "; ", "}"};

  struct tokenizer_az_span_elem *iter;

  struct StrTokenizerKind tokenized_l[n] = {
      {"int", WORD},     {" ", WHITESPACE}, {"sum", WORD},
      {"(", LPAREN},     {"int", WORD},     {" ", WHITESPACE},
      {"a", WORD},       {",", COMMA},      {" ", WHITESPACE},
      {"int", WORD},     {" ", WHITESPACE}, {"b", WORD},
      {")", RPAREN},     {" ", WHITESPACE}, {"{", LBRACE},
      {" ", WHITESPACE}, {"return", WORD},  {" ", WHITESPACE},
      {"a", WORD},       {" ", WHITESPACE}, {"+", PLUS},
      {" ", WHITESPACE}, {"b", WORD},       {";", TERMINATOR},
      {" ", WHITESPACE}, {"}", RBRACE}};

  tokenizer(sum_func_span, &tokenized);

  ASSERT_EQ(tokenized->size, n);

  for (iter = (struct tokenizer_az_span_elem *)tokenized->list, i = 0;
       iter != NULL; iter = iter->next, i++) {
    const size_t n = az_span_size(iter->span) + 1;
    char *iter_s = malloc(n);
    az_span_to_str(iter_s, n, iter->span);
    ASSERT_STR_EQ(tokenized_l[i].s, iter_s);
    ASSERT_EQ(tokenized_l[i].kind, iter->kind);
    free(iter_s);
  }
  ASSERT_EQ(tokenized->size, i);
  tokenizer_az_span_list_cleanup(tokenized);
  ASSERT_EQ(tokenized->size, 0);
  ASSERT_EQ(tokenized->list, NULL);

  PASS();
}

TEST x_test_function_parsed(void) {
  const az_span sum_func_span = az_span_create_from_str((char *)sum_func_src);
  struct tokenizer_az_span_list *tokenized;
  tokenizer(sum_func_span, &tokenized);
  {
    struct parse_cst_list *tokens;
    size_t i;
    cst_parser(tokenized, &tokens);
    {
      struct parse_cst_elem *elem;
      for (elem = (struct parse_cst_elem *)tokens, i = 0; elem != NULL;
           elem = elem->next, i++) {
        printf("parse_cst_list[%" NUM_LONG_FMT "u]:%s\n", i,
               CstNodeKind_to_str(elem->kind));
      }
      printf("i = %" NUM_LONG_FMT "u\n", i);
    }
    ASSERT_EQ(tokens->size, 26);
    ASSERT_EQ(tokens->size, i);
    ASSERT_EQ(tokens->list, NULL);
    tokenizer_az_span_list_cleanup(tokenized);
    parse_cst_list_cleanup(tokens);
  }
  PASS();
}

TEST x_test_function_parsed1(void) {
  const az_span sum_func_span = az_span_create_from_str((char *)sum_func_src);
  struct tokenizer_az_span_list *tokenized;
  tokenizer(sum_func_span, &tokenized);
  /* const struct CstNode **parsed = parser((struct az_span_elem *)tokenized);
   */
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
  /*RUN_TEST(x_test_function_tokenized);*/
  RUN_TEST(x_test_function_parsed);
  /*RUN_TEST(x_test_function_parsed1);*/
}

#endif /* !C_CDD_TESTS_TEST_FUNCTION_H */
