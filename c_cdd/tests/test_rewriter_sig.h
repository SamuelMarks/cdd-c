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
  /* Space padding checks */
  ASSERT_EQ(0, test_rewrite("void  f ( void )", "int f ( void )"));
  PASS();
}

TEST test_rewrite_ptr_ret(void) {
  /* char *f() -> int f(char * *out) */
  ASSERT_EQ(0, test_rewrite("char *f()", "int f(char * *out)"));
  /* preserving internal spaces */
  ASSERT_EQ(0, test_rewrite("char * f()", "int f(char * *out)"));
  PASS();
}

TEST test_rewrite_struct_ret(void) {
  /* struct S f() -> int f(struct S *out) */
  ASSERT_EQ(0, test_rewrite("struct S f()", "int f(struct S *out)"));
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

TEST test_rewrite_c23_attributes(void) {
  /* [[nodiscard]] void f() -> [[nodiscard]] int f() */
  ASSERT_EQ(0, test_rewrite("[[nodiscard]] void f()", "[[nodiscard]] int f()"));

  /* [[maybe_unused]] int * f() -> [[maybe_unused]] int f(int * *out) */
  ASSERT_EQ(0, test_rewrite("[[maybe_unused]] int * f()",
                            "[[maybe_unused]] int f(int * *out)"));
  PASS();
}

TEST test_rewrite_array_args(void) {
  /* void process(int a[]) -> int process(int a[]) */
  ASSERT_EQ(0, test_rewrite("void process(int a[])", "int process(int a[])"));

  /* int * sort(int a[10]) -> int sort(int a[10], int * *out) */
  ASSERT_EQ(0, test_rewrite("int * sort(int a[10])",
                            "int sort(int a[10], int * *out)"));
  PASS();
}

TEST test_rewrite_function_pointer_args(void) {
  /* void register(void (*cb)(int)) -> int register(void (*cb)(int)) */
  ASSERT_EQ(0, test_rewrite("void register_cb(void (*cb)(int))",
                            "int register_cb(void (*cb)(int))"));

  /* Complex nested parens in args */
  ASSERT_EQ(
      0, test_rewrite("void f(int (*g)(char *))", "int f(int (*g)(char *))"));
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

/* --- K&R Support Tests --- */

TEST test_rewrite_kr_void_ret(void) {
  /* void f(a) int a; -> int f(a) int a; */
  ASSERT_EQ(0, test_rewrite("void f(a) int a;", "int f(a) int a;"));
  PASS();
}

TEST test_rewrite_kr_ptr_ret(void) {
  /* char *f(a) int a; -> int f(a, out) int a; char * *out; */
  /* Note: whitespace in generated K&R suffix depends on join_tokens format.
     Our implementation preserves existing tokens (with internal spacing logic
     if join_tokens used spaces, but join_tokens here is a raw memcpy
     concatenation). We must ensure the test string matches the raw
     concatenation + injected parts.
  */
  /* The join_tokens implementation just concatenates. So " int a;" becomes "
   * int a;" */

  const char *expected = "int f(a, out) int a; char * *out;";
  /* input has a space before int a; */
  ASSERT_EQ(0, test_rewrite("char *f(a) int a;", expected));
  PASS();
}

TEST test_rewrite_kr_complex(void) {
  /* struct S *f(x, y) int x; double y; */
  /* -> int f(x, y, out) int x; double y; struct S * *out; */
  const char *input = "struct S *f(x, y) int x; double y;";
  const char *expected = "int f(x, y, out) int x; double y; struct S * *out;";

  ASSERT_EQ(0, test_rewrite(input, expected));
  PASS();
}

TEST test_rewrite_kr_empty_args(void) {
  /* char *f() int x; -> int f(out) int x; char * *out; */
  /* This is technically invalid C (declaring x but not in param list), but
   * parser should preserve logic */
  const char *input = "char *f() int x;";
  const char *expected = "int f(out) int x; char * *out;";

  ASSERT_EQ(0, test_rewrite(input, expected));
  PASS();
}

SUITE(rewriter_sig_suite) {
  RUN_TEST(test_rewrite_void_ret);
  RUN_TEST(test_rewrite_ptr_ret);
  RUN_TEST(test_rewrite_struct_ret);
  RUN_TEST(test_rewrite_storage_class);
  RUN_TEST(test_rewrite_c23_attributes);
  RUN_TEST(test_rewrite_array_args);
  RUN_TEST(test_rewrite_function_pointer_args);
  RUN_TEST(test_rewrite_complex_type);
  RUN_TEST(test_rewrite_with_const);
  RUN_TEST(test_rewrite_invalid_input);
  RUN_TEST(test_rewrite_no_parens);

  /* C89/K&R Tests */
  RUN_TEST(test_rewrite_kr_void_ret);
  RUN_TEST(test_rewrite_kr_ptr_ret);
  RUN_TEST(test_rewrite_kr_complex);
  RUN_TEST(test_rewrite_kr_empty_args);
}

#endif /* TEST_REWRITER_SIG_H */
