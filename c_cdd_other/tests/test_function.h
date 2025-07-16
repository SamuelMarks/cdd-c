#ifndef C_CDD_TESTS_TEST_FUNCTION_H
#define C_CDD_TESTS_TEST_FUNCTION_H

#include <cdd_helpers.h>
#include <str_includes.h>

#include <greatest.h>

#include <c_str_precondition_internal.h>

#include <cst.h>

#include "cst_parser_types.h"

static const char sum_func_src[] = "int sum(int a, int b) { return a + b; }"
                                   "int sum(int a, int b) { return a + b; }";

/*
int const f(void) ??< return 42; %>

int const f(void) { return 42; }


int f(void) { return 123; } int (*g(void))(void) { return  f; }

// call with
printf("%d", g()());


int (*f(void))[3] { static int arr[3] = {123, 456, 789}; return &arr; }
printf("%d", 1[*f()]);
 */

TEST x_test_function_tokenized(void) {
  const az_span sum_func_span = az_span_create_from_str((char *)sum_func_src);
  struct tokenizer_az_span_arr tokenized_stack = {NULL, 0};
  struct tokenizer_az_span_arr *tokenized = &tokenized_stack;
  enum { n = 26 };
  size_t i;
  static const char *const tokenized_str_l[] = {"int sum(int a, int b) ",
                                                "{ return a + b", "; ", "}"};

  struct tokenizer_az_span_elem *iter;

  struct StrTokenizerKind tokenized_l[n] = {{"int", intKeyword},
                                            {" ", WHITESPACE},
                                            {"sum", WORD},
                                            {"(", LPAREN},
                                            {"int", intKeyword},
                                            {" ", WHITESPACE},
                                            {"a", WORD},
                                            {",", COMMA},
                                            {" ", WHITESPACE},
                                            {"int", intKeyword},
                                            {" ", WHITESPACE},
                                            {"b", WORD},
                                            {")", RPAREN},
                                            {" ", WHITESPACE},
                                            {"{", LBRACE},
                                            {" ", WHITESPACE},
                                            {"return", returnKeyword},
                                            {" ", WHITESPACE},
                                            {"a", WORD},
                                            {" ", WHITESPACE},
                                            {"+", PLUS},
                                            {" ", WHITESPACE},
                                            {"b", WORD},
                                            {";", TERMINATOR},
                                            {" ", WHITESPACE},
                                            {"}", RBRACE}};

  tokenizer(sum_func_span, &tokenized);

  ASSERT_EQ(tokenized->size, n);

  for (iter = tokenized->elem, i = 0;
       iter != NULL && az_span_ptr(iter->span) != NULL; iter++, i++) {
    const size_t n = az_span_size(iter->span) + 1;
    char *iter_s = malloc(n);
    if (iter_s == NULL)
      exit(ENOMEM);
    az_span_to_str(iter_s, n, iter->span);
    ASSERT_STR_EQ(tokenized_l[i].s, iter_s);
    ASSERT_STR_EQ(TokenizerKind_to_str(tokenized_l[i].kind),
                  TokenizerKind_to_str(iter->kind));
    ASSERT_EQ(tokenized_l[i].kind, iter->kind);
    free(iter_s);
  }
  ASSERT_EQ(tokenized->size, i);
  tokenizer_az_span_elem_arr_cleanup(tokenized);
  ASSERT_EQ(tokenized->size, 0);
  ASSERT_EQ(tokenized->elem, NULL);

  PASS();
}

TEST x_test_function_parsed(void) {
  const az_span sum_func_span = az_span_create_from_str((char *)sum_func_src);
  struct tokenizer_az_span_arr tokenized_stack = {NULL, 0};
  struct tokenizer_az_span_arr *tokenized = &tokenized_stack;
  tokenizer(sum_func_span, &tokenized);
  {
    struct cst_node_arr *cst_arr = calloc(1, sizeof *cst_arr);
    size_t i;
    cst_parser(tokenized, &cst_arr);
    {
      struct CstNode *cst_node;
      ASSERT_GT(cst_arr->size, 0);
      for (cst_node = cst_arr->elem, i = 0; cst_node->kind != Sentinel;
           cst_node++, i++) {
        printf("parse_cst_list[%" NUM_LONG_FMT "u]:%s\n", i,
               CstNodeKind_to_str(cst_node->kind));
      }
      printf("i = %" NUM_LONG_FMT "u\n", i);
    }
    ASSERT_EQ(cst_arr->size, i);
    ASSERT_EQ(cst_arr->elem[0].kind, Function);
    ASSERT_EQ(cst_arr->elem[cst_arr->size].kind, Sentinel);
    tokenizer_az_span_elem_arr_cleanup(tokenized);
    cst_node_arr_cleanup(cst_arr);
  }
  PASS();
}

TEST x_test_function_parsed1(void) {
  const az_span sum_func_span = az_span_create_from_str((char *)sum_func_src);
  struct tokenizer_az_span_arr tokenized_stack = {NULL, 0};
  struct tokenizer_az_span_arr *tokenized = &tokenized_stack;
  tokenizer(sum_func_span, &tokenized);
  /* const struct CstNode **parsed = parser((struct az_span_elem *)tokenized);
   */
  /*
    static enum TypeSpecifier int_specifier[] = {INT};
    static const struct Declaration a_arg = {*/
  /* pos_start */  /* 0,*/
  /* scope */      /* NULL,*/
  /* value */      /* "int a",*/
  /* specifiers */ /* int_specifier,*/
  /* type */       /* NULL,*/
  /* name */       /* "a"};*/
  /*static const struct Declaration b_arg = {*/ /* pos_start */ /* 0,*/
  /* scope */                                                   /* NULL,*/
  /* value */                                                   /* "int b",*/
  /* specifiers */ /* int_specifier,*/
  /* type */       /* NULL,*/
  /* name */       /* "b"};*/
  /*struct Declaration args[2];
  struct Return _return = {*/
  /* pos_start */  /* 0,*/
  /* scope */      /* NULL,*/
  /* value */      /* "return a + b;",*/
  /* val */        /* "a + b"};
struct CstNode return_cst_node = {Return};
struct CstNode sum_func_body[1];
return_cst_node.type._return = _return;
sum_func_body[0] = return_cst_node;
       
args[0] = a_arg;
args[1] = b_arg;
       
{
struct Function sum_func = {*/
  /* pos_start */  /* 0,*/
  /* scope */      /* NULL,*/
  /* value */      /* sum_func_src,*/
  /* specifiers */ /* int_specifier,*/
  /* name */       /* "sum",*/
  /* args */       /* NULL,*/
  /* body */       /* NULL};
sum_func.args = args;
sum_func.body = sum_func_body;
      
ASSERT_EQ(parsed, NULL);
// TODO: loop through parsed, assert contents contain expected information
}
*/
  PASS();
}

SUITE(function_suite) {
  az_precondition_failed_set_callback(cdd_precondition_failed);
  /*RUN_TEST(x_test_function_tokenized);*/
  RUN_TEST(x_test_function_parsed);
  /*RUN_TEST(x_test_function_parsed1);*/
}

#endif /* !C_CDD_TESTS_TEST_FUNCTION_H */
