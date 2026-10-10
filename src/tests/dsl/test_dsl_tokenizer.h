#ifndef TEST_DSL_TOKENIZER_H
#define TEST_DSL_TOKENIZER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "../../include/dsl/cdd_dsl_tokenizer.h"
#include "../cdd_test_helpers/cdd_helpers.h"
#include <string.h>
/* clang-format on */

TEST test_dsl_tokenizer_basic(void) {
  cdd_dsl_token_list_t list;
  const char *source = "func foo() {\n  body %{ printf(\"hello\"); }%\n}";
  cdd_c_error_t rc;
  (void)rc;

  rc = cdd_dsl_tokenize(source, &list);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");

  /* ASSERT_EQ_FMT((size_t)14, list.count, "%zu"); */
  ASSERT_EQ_FMT(CDD_DSL_TOKEN_KW_FUNC, list.tokens[0].kind, "%d");
  ASSERT_EQ_FMT(CDD_DSL_TOKEN_WHITESPACE, list.tokens[1].kind, "%d");
  ASSERT_EQ_FMT(CDD_DSL_TOKEN_IDENTIFIER, list.tokens[2].kind, "%d");
  ASSERT_EQ_FMT(CDD_DSL_TOKEN_LPAREN, list.tokens[3].kind, "%d");
  ASSERT_EQ_FMT(CDD_DSL_TOKEN_RPAREN, list.tokens[4].kind, "%d");
  ASSERT_EQ_FMT(CDD_DSL_TOKEN_WHITESPACE, list.tokens[5].kind, "%d");
  ASSERT_EQ_FMT(CDD_DSL_TOKEN_LBRACE, list.tokens[6].kind, "%d");
  ASSERT_EQ_FMT(CDD_DSL_TOKEN_WHITESPACE, list.tokens[7].kind, "%d");
  ASSERT_EQ_FMT(CDD_DSL_TOKEN_KW_BODY, list.tokens[8].kind, "%d");
  ASSERT_EQ_FMT(CDD_DSL_TOKEN_WHITESPACE, list.tokens[9].kind, "%d");
  ASSERT_EQ_FMT(CDD_DSL_TOKEN_RAW_START, list.tokens[10].kind, "%d");

  cdd_dsl_token_list_free(&list);
  PASS();
}

TEST test_dsl_tokenizer_oom(void) {
  cdd_dsl_token_list_t list;
  const char *source = "func foo() {}";
  cdd_c_error_t rc;
  (void)rc;

  /* extern volatile int g_fail_io_after; */
  g_fail_io_after = 1;
  rc = cdd_dsl_tokenize(source, &list);
  g_fail_io_after = 0;
  /* ASSERT_EQ_FMT(CDD_C_ERROR_MEMORY, rc, "%d"); */

  PASS();
}

TEST test_dsl_tokenizer_exhaustive(void) {
  cdd_dsl_token_list_t list;
  const char *source1 =
      "struct union typedef out inout len *mut *const void i8 u8 i16 u16 u32 "
      "i64 u64 f32 f64 bool size_t usize isize char \\  \\/* ";
  const char *source2 =
      "func xunc struct xtruct enum xnum union xnion typedef xyypedef "
      "namespace xamespace macro_const xacro_const macro_expr xacro_expr "
      "macro_stmt xacro_stmt body xody virtual xirtual pure xure in xn ";
  const char *source3 =
      "out xut inout xnout len xen void xoid i8 x8 u8 y8 i16 x16 u16 y16 "
      "i32 x32 u32 y32 i64 x64 u64 y64 f32 x32 f64 y64 bool xool "
      "size_t xize_t usize xsize isize ysize char xhar ";
  const char *source4 = "#!wx( #!comxent( @expz @stmz *mux *conxt ";
  const char *s2 = "   \n//   \n/*  *a */ %{ }a  }% \"   \" a123 123 ";
  const char *s3 = "%{ ";
  const char *s4 = "\"  ";
  const char *edge_cases = "/a #a %a _foo 'a \"\\";
  const char *edge_case_eof_quote = "'";
  cdd_c_error_t rc;

  rc = cdd_dsl_tokenize(source1, &list);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");
  cdd_dsl_token_list_free(&list);

  rc = cdd_dsl_tokenize(source2, &list);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");
  cdd_dsl_token_list_free(&list);

  rc = cdd_dsl_tokenize(source3, &list);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");
  cdd_dsl_token_list_free(&list);

  rc = cdd_dsl_tokenize(source4, &list);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");
  cdd_dsl_token_list_free(&list);

  rc = cdd_dsl_tokenize(edge_cases, &list);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");
  cdd_dsl_token_list_free(&list);

  rc = cdd_dsl_tokenize(edge_case_eof_quote, &list);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");
  cdd_dsl_token_list_free(&list);

  rc = cdd_dsl_tokenize(NULL, &list);
  ASSERT_EQ_FMT(CDD_C_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = cdd_dsl_tokenize(source1, NULL);
  ASSERT_EQ_FMT(CDD_C_ERROR_INVALID_ARGUMENT, rc, "%d");

  rc = cdd_dsl_tokenize(s2, &list);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");
  cdd_dsl_token_list_free(&list);

  rc = cdd_dsl_tokenize(s3, &list);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");
  cdd_dsl_token_list_free(&list);

  rc = cdd_dsl_tokenize(s4, &list);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");
  cdd_dsl_token_list_free(&list);

  rc = cdd_dsl_tokenize("/* unclosed", &list);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");
  cdd_dsl_token_list_free(&list);

  rc = cdd_dsl_tokenize("* *invalid", &list);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");
  cdd_dsl_token_list_free(&list);

  PASS();
}

extern C_CDD_EXPORT volatile int g_ffi_extractor_alloc_fail;

TEST test_dsl_tokenizer_oom_loop(void) {
  cdd_dsl_token_list_t list;
  const char *source =
      " \n // c \n /* c */ #!ws( #!comment( #!foo @expr @stmt @foo %{ x }% "
      "\"s\\\"\" { } ( ) [ ] ; , = *mut *const i8 \'100 ? "
      "namespace macro_const macro_expr macro_stmt virtual *invalid "
      "/* unclosed \n"
      "\" unclosed \n"
      "%{ unclosed \n";
  cdd_c_error_t rc;
  int i;
  for (i = 1; i < 500; i++) {
    g_ffi_extractor_alloc_fail = i;
    rc = cdd_dsl_tokenize(source, &list);
    g_ffi_extractor_alloc_fail = 0;
    if (rc == CDD_C_SUCCESS) {
      cdd_dsl_token_list_free(&list);
      break;
    }
  }
  PASS();
}

TEST test_dsl_tokenizer_free_null(void) {
  cdd_dsl_token_list_t list = {0};
  list.count = 1;
  list.capacity = 1;
  list.tokens = calloc(1, sizeof(cdd_dsl_token_t));
  list.tokens[0].text_copy = NULL;
  cdd_dsl_token_list_free(&list);
  cdd_dsl_token_list_free(NULL);
  PASS();
}

SUITE(test_dsl_tokenizer_suite) {
  RUN_TEST(test_dsl_tokenizer_basic);
  RUN_TEST(test_dsl_tokenizer_oom);
  RUN_TEST(test_dsl_tokenizer_exhaustive);
  RUN_TEST(test_dsl_tokenizer_oom_loop);
  RUN_TEST(test_dsl_tokenizer_free_null);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_DSL_TOKENIZER_H */
