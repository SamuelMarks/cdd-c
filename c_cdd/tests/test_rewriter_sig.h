#ifndef TEST_REWRITER_SIG_H
#define TEST_REWRITER_SIG_H

#include <greatest.h>
#include <stdlib.h>
#include <string.h>

#include "rewriter_sig.h"
#include "tokenizer.h"

/**
 * @brief Helper to run rewrite_signature on a string input and verify against
 * expectation.
 */
static int test_rewrite(const char *input, const char *expected) {
  struct TokenList *tl = NULL;
  char *output = NULL;
  int rc;
  const az_span source = az_span_create_from_str((char *)input);

  if (tokenize(source, &tl) != 0)
    return -1;

  rc = rewrite_signature(tl, &output);
  if (rc != 0) {
    free_token_list(tl);
    return rc;
  }

  if (output == NULL || strcmp(output, expected) != 0) {
    fprintf(stderr, "\nExpected: '%s'\nGot:      '%s'\n", expected,
            output ? output : "(null)");
    free(output);
    free_token_list(tl);
    return 1; /* Fail */
  }

  free(output);
  free_token_list(tl);
  return 0; /* Success */
}

TEST test_rewrite_void_ret(void) {
  /* void f() -> int f() */
  ASSERT_EQ(0, test_rewrite("void f()", "int f()"));
  ASSERT_EQ(0, test_rewrite("void f(void)", "int f(void)"));
  ASSERT_EQ(0, test_rewrite("void bar(int a)", "int bar(int a)"));
  PASS();
}

TEST test_rewrite_int_ret(void) {
  /* int f() -> unchanged */
  ASSERT_EQ(0, test_rewrite("int f()", "int f()"));
  /* signed int -> unchanged */
  ASSERT_EQ(0, test_rewrite("signed int f()", "signed int f()"));
  ASSERT_EQ(0, test_rewrite("int main(int c, char **v)",
                            "int main(int c, char **v)"));
  PASS();
}

TEST test_rewrite_ptr_ret(void) {
  /* char *f() -> int f(char * *out) */
  ASSERT_EQ(0, test_rewrite("char *f()", "int f(char * *out)"));
  /* preserving internal spaces */
  ASSERT_EQ(0, test_rewrite("char * f()", "int f(char * *out)"));
  PASS();
}

TEST test_rewrite_complex_ptr_ret(void) {
  /* char ***f() -> int f(char *** *out) */
  /* Note: tokenizer splits pointers usually as '*' and '*' or others depending
     on logic. Join handles them nicely. */
  ASSERT_EQ(0, test_rewrite("char ***f()", "int f(char *** *out)"));
  PASS();
}

TEST test_rewrite_struct_ret(void) {
  /* struct S f() -> int f(struct S *out) */
  ASSERT_EQ(0, test_rewrite("struct S f()", "int f(struct S *out)"));
  PASS();
}

TEST test_rewrite_args_append(void) {
  /* double f(int x) -> int f(int x, double *out) */
  ASSERT_EQ(0, test_rewrite("double f(int x)", "int f(int x, double *out)"));
  PASS();
}

TEST test_rewrite_args_void(void) {
  /* double f(void) -> int f(double *out) */
  ASSERT_EQ(0, test_rewrite("double f(void)", "int f(double *out)"));
  /* spaces */
  ASSERT_EQ(0, test_rewrite("double f( void )", "int f(double *out)"));
  PASS();
}

TEST test_rewrite_storage_class(void) {
  /* static void f() -> static int f() */
  ASSERT_EQ(0, test_rewrite("static void f()", "static int f()"));

  /* extern char *g(void) -> extern int g(char * *out) */
  ASSERT_EQ(0,
            test_rewrite("extern char *g(void)", "extern int g(char * *out)"));

  /* static inline void h() -> static inline int h() */
  ASSERT_EQ(0, test_rewrite("static inline void h()", "static inline int h()"));
  PASS();
}

TEST test_rewrite_complex_type(void) {
  /* unsigned long long f() -> int f(unsigned long long *out) */
  ASSERT_EQ(0, test_rewrite("unsigned long long f()",
                            "int f(unsigned long long *out)"));
  PASS();
}

TEST test_rewrite_with_const(void) {
  /* const char *f() -> int f(const char * *out) */
  ASSERT_EQ(0, test_rewrite("const char *f()", "int f(const char * *out)"));
  PASS();
}

/* Error cases */
TEST test_rewrite_invalid_input(void) {
  struct TokenList tmpl = {0};
  char *out = NULL;
  ASSERT_EQ(EINVAL, rewrite_signature(NULL, &out));
  ASSERT_EQ(EINVAL, rewrite_signature(&tmpl, NULL));
  PASS();
}

TEST test_rewrite_no_parens(void) {
  /* "int x;" is not a function */
  ASSERT_NEQ(0, test_rewrite("int x;", ""));
  PASS();
}

SUITE(rewriter_sig_suite) {
  RUN_TEST(test_rewrite_void_ret);
  RUN_TEST(test_rewrite_int_ret);
  RUN_TEST(test_rewrite_ptr_ret);
  RUN_TEST(test_rewrite_complex_ptr_ret);
  RUN_TEST(test_rewrite_struct_ret);
  RUN_TEST(test_rewrite_args_append);
  RUN_TEST(test_rewrite_args_void);
  RUN_TEST(test_rewrite_storage_class);
  RUN_TEST(test_rewrite_complex_type);
  RUN_TEST(test_rewrite_with_const);
  RUN_TEST(test_rewrite_invalid_input);
  RUN_TEST(test_rewrite_no_parens);
}

#endif /* TEST_REWRITER_SIG_H */
