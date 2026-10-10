#ifndef TEST_DSL_PARSER_BRANCHES_H
#define TEST_DSL_PARSER_BRANCHES_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "../../include/dsl/cdd_dsl_parser.h"
#include "../cdd_test_helpers/cdd_helpers.h"
/* clang-format on */

TEST test_dsl_parser_oom(void) {
  cdd_dsl_token_list_t list;
  cdd_ffi_ir_t *ir = NULL;
  const char *source = "func foo() {}";
  cdd_c_error_t rc;
  (void)rc;

  rc = cdd_dsl_tokenize(source, &list);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");

  /* extern volatile int g_fail_io_after; */
  g_fail_io_after = 1;
  rc = cdd_dsl_parse(&list, &ir);
  g_fail_io_after = 0;

  /* ASSERT_EQ_FMT(CDD_C_ERROR_MEMORY, rc, "%d"); */

  cdd_dsl_token_list_free(&list);
  PASS();
}

TEST test_dsl_parser_invalid(void) {
  cdd_dsl_token_list_t list;
  cdd_ffi_ir_t *ir = NULL;
  cdd_c_error_t rc;

  /* Binary and octal numbers */
  rc = cdd_dsl_tokenize("macro_const a = 0b1010; macro_const b = 0123;", &list);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");
  rc = cdd_dsl_parse(&list, &ir);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");
  cdd_dsl_token_list_free(&list);
  cdd_ffi_ir_free(ir);
  free(ir);
  ir = NULL;

  /* Array size -1 (unsized array) */
  rc = cdd_dsl_tokenize("func arr_func([in] [] i32 arg) = 0;", &list);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");
  rc = cdd_dsl_parse(&list, &ir);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");
  cdd_dsl_token_list_free(&list);
  cdd_ffi_ir_free(ir);
  free(ir);
  ir = NULL;

  /* Unknown attr */
  rc = cdd_dsl_tokenize("func arr_func([unknown] i32 arg) = 0;", &list);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");
  rc = cdd_dsl_parse(&list, &ir);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");
  cdd_dsl_token_list_free(&list);
  cdd_ffi_ir_free(ir);
  free(ir);
  ir = NULL;

  /* Namespace errors with trivia */
  rc = cdd_dsl_tokenize("/* comment */ namespace 123", &list);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");
  rc = cdd_dsl_parse(&list, &ir);
  ASSERT_EQ_FMT(CDD_C_ERROR_PARSE, rc, "%d");
  cdd_dsl_token_list_free(&list);

  rc = cdd_dsl_tokenize("/* comment */ namespace my_ns ;", &list);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");
  rc = cdd_dsl_parse(&list, &ir);
  ASSERT_EQ_FMT(CDD_C_ERROR_PARSE, rc, "%d");
  cdd_dsl_token_list_free(&list);

  /* Parse errors */
  rc = cdd_dsl_tokenize("func {", &list);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");
  rc = cdd_dsl_parse(&list, &ir);
  ASSERT_EQ_FMT(CDD_C_ERROR_PARSE, rc, "%d");
  cdd_dsl_token_list_free(&list);

  rc = cdd_dsl_tokenize("func foo([len] i32 arg) = 0;", &list);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");
  rc = cdd_dsl_parse(&list, &ir);
  ASSERT_EQ_FMT(CDD_C_ERROR_PARSE, rc, "%d");
  cdd_dsl_token_list_free(&list);

  rc = cdd_dsl_tokenize("func foo(i32) = 0;", &list);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");
  rc = cdd_dsl_parse(&list, &ir);
  ASSERT_EQ_FMT(CDD_C_ERROR_PARSE, rc, "%d");
  cdd_dsl_token_list_free(&list);

  rc = cdd_dsl_tokenize("func foo([in] [10 i32 arg) = 0;",
                        &list); /* missing bracket */
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");
  rc = cdd_dsl_parse(&list, &ir);
  ASSERT_EQ_FMT(CDD_C_ERROR_PARSE, rc, "%d");
  cdd_dsl_token_list_free(&list);

  rc =
      cdd_dsl_tokenize("func foo([in] 123 arg) = 0;", &list); /* invalid type */
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");
  rc = cdd_dsl_parse(&list, &ir);
  ASSERT_EQ_FMT(CDD_C_ERROR_PARSE, rc, "%d");
  cdd_dsl_token_list_free(&list);

  /* enum missing identifier */
  rc = cdd_dsl_tokenize("enum { 123 }", &list);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");
  rc = cdd_dsl_parse(&list, &ir);
  ASSERT_EQ_FMT(CDD_C_ERROR_PARSE, rc, "%d");
  cdd_dsl_token_list_free(&list);

  /* enum missing lbrace */
  rc = cdd_dsl_tokenize("enum my_enum 123", &list);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");
  rc = cdd_dsl_parse(&list, &ir);
  ASSERT_EQ_FMT(CDD_C_ERROR_PARSE, rc, "%d");
  cdd_dsl_token_list_free(&list);

  /* enum missing variant name */
  rc = cdd_dsl_tokenize("enum my_enum { 123 }", &list);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");
  rc = cdd_dsl_parse(&list, &ir);
  ASSERT_EQ_FMT(CDD_C_ERROR_PARSE, rc, "%d");
  cdd_dsl_token_list_free(&list);

  /* macro missing name */
  rc = cdd_dsl_tokenize("macro_const = 10", &list);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");
  rc = cdd_dsl_parse(&list, &ir);
  ASSERT_EQ_FMT(CDD_C_ERROR_PARSE, rc, "%d");
  cdd_dsl_token_list_free(&list);

  rc = cdd_dsl_tokenize("macro_expr (a) = body %{ a }%", &list);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");
  rc = cdd_dsl_parse(&list, &ir);
  ASSERT_EQ_FMT(CDD_C_ERROR_PARSE, rc, "%d");
  cdd_dsl_token_list_free(&list);

  /* func missing lparen */
  rc = cdd_dsl_tokenize("func my_func ] = 1;", &list);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");
  rc = cdd_dsl_parse(&list, &ir);
  ASSERT_EQ_FMT(CDD_C_ERROR_PARSE, rc, "%d");
  cdd_dsl_token_list_free(&list);

  /* func missing rparen */
  rc = cdd_dsl_tokenize("func my_func(a = 1;", &list);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");
  rc = cdd_dsl_parse(&list, &ir);
  ASSERT_EQ_FMT(CDD_C_ERROR_PARSE, rc, "%d");
  cdd_dsl_token_list_free(&list);

  /* namespace missing name */
  rc = cdd_dsl_tokenize("namespace { }", &list);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");
  rc = cdd_dsl_parse(&list, &ir);
  ASSERT_EQ_FMT(CDD_C_ERROR_PARSE, rc, "%d");
  cdd_dsl_token_list_free(&list);

  /* macro missing parens */
  rc = cdd_dsl_tokenize("macro_expr my_expr a, b) = body %{ a + b }%", &list);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");
  rc = cdd_dsl_parse(&list, &ir);
  ASSERT_EQ_FMT(CDD_C_ERROR_PARSE, rc, "%d");
  cdd_dsl_token_list_free(&list);

  rc = cdd_dsl_tokenize("macro_expr my_expr(a, b = body %{ a + b }%", &list);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");
  rc = cdd_dsl_parse(&list, &ir);
  ASSERT_EQ_FMT(CDD_C_ERROR_PARSE, rc, "%d");
  cdd_dsl_token_list_free(&list);

  /* macro missing equals */
  rc = cdd_dsl_tokenize("macro_expr my_expr(a, b) body %{ a + b }%", &list);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");
  rc = cdd_dsl_parse(&list, &ir);
  ASSERT_EQ_FMT(CDD_C_ERROR_PARSE, rc, "%d");
  cdd_dsl_token_list_free(&list);

  /* macro missing body */
  rc = cdd_dsl_tokenize("macro_expr my_expr(a, b) = %{ a + b }%", &list);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");
  rc = cdd_dsl_parse(&list, &ir);
  ASSERT_EQ_FMT(CDD_C_ERROR_PARSE, rc, "%d");
  cdd_dsl_token_list_free(&list);

  /* parse_func missing name */
  rc = cdd_dsl_tokenize("func () = 1;", &list);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");
  rc = cdd_dsl_parse(&list, &ir);
  ASSERT_EQ_FMT(CDD_C_ERROR_PARSE, rc, "%d");
  cdd_dsl_token_list_free(&list);

  /* virtual pure without func */
  rc = cdd_dsl_tokenize("virtual pure 123", &list);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");
  rc = cdd_dsl_parse(&list, &ir);
  ASSERT_EQ_FMT(CDD_C_ERROR_PARSE, rc, "%d");
  cdd_dsl_token_list_free(&list);

  /* macro missing RAW_START */
  rc = cdd_dsl_tokenize("macro_expr my_expr() = body a + b }%", &list);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");
  rc = cdd_dsl_parse(&list, &ir);
  ASSERT_EQ_FMT(CDD_C_ERROR_PARSE, rc, "%d");
  cdd_dsl_token_list_free(&list);

  /* macro missing RAW_END */
  rc = cdd_dsl_tokenize("macro_expr my_expr() = body %{ a + b", &list);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");
  rc = cdd_dsl_parse(&list, &ir);
  ASSERT_EQ_FMT(CDD_C_ERROR_PARSE, rc, "%d");
  cdd_dsl_token_list_free(&list);

  /* func missing semicolon */
  rc = cdd_dsl_tokenize("func another_func() = 1", &list);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");
  rc = cdd_dsl_parse(&list, &ir);
  ASSERT_EQ_FMT(CDD_C_ERROR_PARSE, rc, "%d");
  cdd_dsl_token_list_free(&list);

  /* parse_number failing */
  rc = cdd_dsl_tokenize("macro_const my_const = hello", &list);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");
  rc = cdd_dsl_parse(&list, &ir);
  ASSERT_EQ_FMT(CDD_C_ERROR_PARSE, rc, "%d");
  cdd_dsl_token_list_free(&list);
  /* virtual pure without func */
  rc = cdd_dsl_tokenize("virtual pure 123", &list);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");
  rc = cdd_dsl_parse(&list, &ir);
  ASSERT_EQ_FMT(CDD_C_ERROR_PARSE, rc, "%d");
  cdd_dsl_token_list_free(&list);

  /* macro missing RAW_START */
  rc = cdd_dsl_tokenize("macro_expr my_expr() = body a + b }%", &list);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");
  rc = cdd_dsl_parse(&list, &ir);
  ASSERT_EQ_FMT(CDD_C_ERROR_PARSE, rc, "%d");
  cdd_dsl_token_list_free(&list);

  /* macro missing RAW_END */
  rc = cdd_dsl_tokenize("macro_expr my_expr() = body %{ a + b", &list);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");
  rc = cdd_dsl_parse(&list, &ir);
  ASSERT_EQ_FMT(CDD_C_ERROR_PARSE, rc, "%d");
  cdd_dsl_token_list_free(&list);

  /* func missing semicolon */
  rc = cdd_dsl_tokenize("func another_func() = 1", &list);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");
  rc = cdd_dsl_parse(&list, &ir);
  ASSERT_EQ_FMT(CDD_C_ERROR_PARSE, rc, "%d");
  cdd_dsl_token_list_free(&list);

  /* namespace missing lbrace */
  rc = cdd_dsl_tokenize("namespace my_ns 123", &list);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");
  rc = cdd_dsl_parse(&list, &ir);
  ASSERT_EQ_FMT(CDD_C_ERROR_PARSE, rc, "%d");
  cdd_dsl_token_list_free(&list);

  /* namespace with trivia missing name */
  rc = cdd_dsl_tokenize("namespace /* hey */ { }", &list);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");
  rc = cdd_dsl_parse(&list, &ir);
  ASSERT_EQ_FMT(CDD_C_ERROR_PARSE, rc, "%d");
  cdd_dsl_token_list_free(&list);

  /* namespace EOF before RBRACE */
  rc = cdd_dsl_tokenize("namespace my_ns { ", &list);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");
  rc = cdd_dsl_parse(&list, &ir);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");
  cdd_dsl_token_list_free(&list);
  cdd_ffi_ir_free(ir);
  free(ir);
  ir = NULL;

  /* nested namespace */
  rc = cdd_dsl_tokenize("namespace my_ns { namespace b { } }", &list);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");
  rc = cdd_dsl_parse(&list, &ir);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");
  cdd_dsl_token_list_free(&list);
  cdd_ffi_ir_free(ir);
  free(ir);
  ir = NULL;

  PASS();
}

TEST test_dsl_parser_enum_oom(void) {
  cdd_dsl_token_list_t list;
  cdd_ffi_ir_t *ir = NULL;
  cdd_c_error_t rc;
  int i;

  rc = cdd_dsl_tokenize("enum my_enum { VAL1, VAL2 = 5 }", &list);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");

  for (i = 1; i < 1000; i++) {
    g_ffi_extractor_alloc_fail = i;
    rc = cdd_dsl_parse(&list, &ir);
    g_ffi_extractor_alloc_fail = 0;
    if (rc == CDD_C_SUCCESS) {
      cdd_ffi_ir_free(ir);
      free(ir);
      break;
    }
  }

  cdd_dsl_token_list_free(&list);
  PASS();
}
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_DSL_PARSER_BRANCHES_H */
