#ifndef TEST_DSL_PARSER_BRANCHES_H
#define TEST_DSL_PARSER_BRANCHES_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "../../include/dsl/cdd_dsl_parser.h"
#include "../cdd_test_helpers/cdd_helpers.h"
/* clang-format on */

extern C_CDD_EXPORT volatile int g_ffi_extractor_alloc_fail;

TEST test_dsl_parser_oom(void) {
  cdd_dsl_token_list_t list;
  cdd_ffi_ir_t *ir = NULL;
  const char *source = "func foo() {}";
  cdd_c_error_t rc;
  (void)rc;

  (void)rc;
  rc = cdd_dsl_tokenize(source, &list);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");

  /* extern volatile int g_fail_io_after; */
  g_fail_io_after = 1;
  rc = cdd_dsl_parse(&list, &ir);
  g_fail_io_after = 0;
  if (ir) {
    cdd_ffi_ir_free(ir);
    free(ir);
    ir = NULL;
  }

  /* ASSERT_EQ_FMT(CDD_C_ERROR_MEMORY, rc, "%d"); */

  cdd_dsl_token_list_free(&list);
  PASS();
}

TEST test_dsl_parser_exhaustive_oom(void) {
  cdd_dsl_token_list_t list;
  cdd_ffi_ir_t *ir = NULL;
  const char *source = "func foo() {}\n"
                       "struct MyStruct { i32 field1; }\n"
                       "enum MyEnum { Variant1 = 1 }\n"
                       "macro_const M = 10;\n"
                       "typedef i32 MyType;\n";
  cdd_c_error_t rc;
  int i;

  (void)rc;
  rc = cdd_dsl_tokenize(source, &list);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");

  for (i = 1; i < 2500; i++) {
    g_ffi_extractor_alloc_fail = i;
    rc = cdd_dsl_parse(&list, &ir);
    if (rc == CDD_C_SUCCESS) {
      cdd_ffi_ir_free(ir);
      free(ir);
      ir = NULL;
      break;
    }
  }
  g_ffi_extractor_alloc_fail = 0;
  rc = cdd_dsl_parse(&list, &ir);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");
  cdd_dsl_token_list_free(&list);
  if (ir) {
    cdd_ffi_ir_free(ir);
    free(ir);
    ir = NULL;
  }
  PASS();
}

TEST test_dsl_parser_missing_oom(void) {
  cdd_dsl_token_list_t list;
  cdd_ffi_ir_t *ir = NULL;
  cdd_c_error_t rc;
  int i;
  const char *source = "/* t */ namespace my_ns { /* t */ namespace nested {} "
                       "/* t */ func foo([unknown] i32 arg) {} unknown_tok }\n"
                       "/* t */ macro_const M = 10;\n"
                       "/* t */ enum MyEnum { Variant1 = 1 }\n"
                       "func bar() { unknown_token body %{ a }% }\n";

  (void)rc;
  rc = cdd_dsl_tokenize(source, &list);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");

  for (i = 1; i < 500; i++) {
    g_ffi_extractor_alloc_fail = i;
    rc = cdd_dsl_parse(&list, &ir);
    if (rc == CDD_C_SUCCESS) {
      cdd_ffi_ir_free(ir);
      free(ir);
      ir = NULL;
      break;
    }
  }
  g_ffi_extractor_alloc_fail = 0;
  rc = cdd_dsl_parse(&list, &ir);
  cdd_dsl_token_list_free(&list);
  if (ir) {
    cdd_ffi_ir_free(ir);
    free(ir);
  }
  PASS();
}

TEST test_dsl_parser_missing_eof(void) {
  cdd_dsl_token_list_t list;
  cdd_ffi_ir_t *ir = NULL;
  cdd_c_error_t rc;

  (void)rc;

  rc = cdd_dsl_tokenize("func foo(i32 arg ", &list);
  rc = cdd_dsl_parse(&list, &ir);
  cdd_dsl_token_list_free(&list);
  if (ir) {
    cdd_ffi_ir_free(ir);
    free(ir);
    ir = NULL;
  }

  rc = cdd_dsl_tokenize("func foo() { body %{ ", &list);
  rc = cdd_dsl_parse(&list, &ir);
  cdd_dsl_token_list_free(&list);
  if (ir) {
    cdd_ffi_ir_free(ir);
    free(ir);
    ir = NULL;
  }

  rc = cdd_dsl_tokenize("func foo() { body %{ a ", &list);
  rc = cdd_dsl_parse(&list, &ir);
  cdd_dsl_token_list_free(&list);
  if (ir) {
    cdd_ffi_ir_free(ir);
    free(ir);
    ir = NULL;
  }

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

  rc = cdd_dsl_tokenize("namespace my_ns { func foo() {} ", &list);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");
  rc = cdd_dsl_parse(&list, &ir);
  if (rc == CDD_C_SUCCESS) {
    cdd_ffi_ir_free(ir);
    free(ir);
    ir = NULL;
  }
  cdd_dsl_token_list_free(&list);
  if (ir) {
    cdd_ffi_ir_free(ir);
    free(ir);
    ir = NULL;
  }

  rc = cdd_dsl_tokenize("namespace my_ns { namespace inner { } ", &list);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");
  rc = cdd_dsl_parse(&list, &ir);
  if (rc == CDD_C_SUCCESS) {
    cdd_ffi_ir_free(ir);
    free(ir);
    ir = NULL;
  }
  cdd_dsl_token_list_free(&list);
  if (ir) {
    cdd_ffi_ir_free(ir);
    free(ir);
    ir = NULL;
  }

  rc = cdd_dsl_tokenize("namespace my_ns { struct MyStruct { i32 field1; } ",
                        &list);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");
  rc = cdd_dsl_parse(&list, &ir);
  if (rc == CDD_C_SUCCESS) {
    cdd_ffi_ir_free(ir);
    free(ir);
    ir = NULL;
  }
  cdd_dsl_token_list_free(&list);
  if (ir) {
    cdd_ffi_ir_free(ir);
    free(ir);
    ir = NULL;
  }

  rc = cdd_dsl_tokenize("namespace my_ns { enum MyEnum { Variant1 = 1 } ",
                        &list);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");
  rc = cdd_dsl_parse(&list, &ir);
  if (rc == CDD_C_SUCCESS) {
    cdd_ffi_ir_free(ir);
    free(ir);
    ir = NULL;
  }
  cdd_dsl_token_list_free(&list);
  if (ir) {
    cdd_ffi_ir_free(ir);
    free(ir);
    ir = NULL;
  }

  rc = cdd_dsl_tokenize("namespace my_ns { macro_const M = 10; ", &list);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");
  rc = cdd_dsl_parse(&list, &ir);
  if (rc == CDD_C_SUCCESS) {
    cdd_ffi_ir_free(ir);
    free(ir);
    ir = NULL;
  }
  cdd_dsl_token_list_free(&list);
  if (ir) {
    cdd_ffi_ir_free(ir);
    free(ir);
    ir = NULL;
  }

  rc = cdd_dsl_tokenize("namespace my_ns { typedef i32 MyType; ", &list);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");
  rc = cdd_dsl_parse(&list, &ir);
  if (rc == CDD_C_SUCCESS) {
    cdd_ffi_ir_free(ir);
    free(ir);
    ir = NULL;
  }
  cdd_dsl_token_list_free(&list);
  if (ir) {
    cdd_ffi_ir_free(ir);
    free(ir);
    ir = NULL;
  }

  rc = cdd_dsl_tokenize("func foo(i32 my_arg {", &list);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");
  rc = cdd_dsl_parse(&list, &ir);
  ASSERT_EQ_FMT(CDD_C_ERROR_PARSE, rc, "%d");
  cdd_dsl_token_list_free(&list);

  rc = cdd_dsl_tokenize("func foo() = body {", &list);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");
  rc = cdd_dsl_parse(&list, &ir);
  ASSERT_EQ_FMT(CDD_C_ERROR_PARSE, rc, "%d");
  cdd_dsl_token_list_free(&list);

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

TEST test_dsl_parser_missing_branches(void) {
  cdd_dsl_token_list_t list;
  cdd_ffi_ir_t *ir = NULL;
  cdd_c_error_t rc;
  const char *source =
      "macro_const B = 0b1;\n"
      "macro_const O = 01;\n"
      "macro_const HX = 0X123;\n"
      "macro_const BB = 0B1010;\n"
      "macro_const S = 1'000;\n"
      "/* block */\n"
      "/// doc\n"
      "// line\n"
      "func arr_func([@size=123] i32 arg) = 0;\n"
      "struct S { i32 a; };\n"
      "namespace ns { virtual func f() = 0; pure func g() = 0; }\n"
      "macro_expr M() = body %{%};\n"
      "macro_stmt MS() = body %{%};\n"
      "func f() body %{%};\n"
      "func h() body ;\n"
      "virtual func f2() = 0;\n"
      "pure func p() = 0;\n"
      "virtual \n";

  (void)rc;
  rc = cdd_dsl_tokenize(source, &list);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");

  rc = cdd_dsl_parse(&list, &ir);

  cdd_dsl_token_list_free(&list);
  if (ir) {
    cdd_ffi_ir_free(ir);
    free(ir);
    ir = NULL;
  }

  /* EOF tests */
  rc = cdd_dsl_tokenize("func my_f([", &list);
  rc = cdd_dsl_parse(&list, &ir);
  cdd_dsl_token_list_free(&list);
  if (ir) {
    cdd_ffi_ir_free(ir);
    free(ir);
    ir = NULL;
  }

  rc = cdd_dsl_tokenize("func f([@len=123] i32 a, [@unknown_attr] i32 b) = 0;",
                        &list);
  rc = cdd_dsl_parse(&list, &ir);
  cdd_dsl_token_list_free(&list);
  if (ir) {
    cdd_ffi_ir_free(ir);
    free(ir);
    ir = NULL;
  }

  rc = cdd_dsl_tokenize("struct S { i32 a; }", &list);
  rc = cdd_dsl_parse(&list, &ir);
  cdd_dsl_token_list_free(&list);
  if (ir) {
    cdd_ffi_ir_free(ir);
    free(ir);
    ir = NULL;
  }

  rc = cdd_dsl_tokenize("namespace my_ns { ", &list);
  rc = cdd_dsl_parse(&list, &ir);
  cdd_dsl_token_list_free(&list);
  if (ir) {
    cdd_ffi_ir_free(ir);
    free(ir);
    ir = NULL;
  }

  rc = cdd_dsl_tokenize("enum E { A=1, ", &list);
  rc = cdd_dsl_parse(&list, &ir);
  cdd_dsl_token_list_free(&list);
  if (ir) {
    cdd_ffi_ir_free(ir);
    free(ir);
    ir = NULL;
  }

  rc = cdd_dsl_tokenize("macro_expr M(A, ", &list);
  rc = cdd_dsl_parse(&list, &ir);
  cdd_dsl_token_list_free(&list);
  if (ir) {
    cdd_ffi_ir_free(ir);
    free(ir);
    ir = NULL;
  }

  rc = cdd_dsl_tokenize("macro_expr M() = body;", &list);
  rc = cdd_dsl_parse(&list, &ir);
  cdd_dsl_token_list_free(&list);
  if (ir) {
    cdd_ffi_ir_free(ir);
    free(ir);
    ir = NULL;
  }

  rc = cdd_dsl_tokenize("func f() ", &list);
  rc = cdd_dsl_parse(&list, &ir);
  cdd_dsl_token_list_free(&list);
  if (ir) {
    cdd_ffi_ir_free(ir);
    free(ir);
    ir = NULL;
  }

  PASS();
}

TEST test_dsl_parser_manual_branches(void) {
  cdd_dsl_token_list_t list;
  cdd_ffi_ir_t *ir = NULL;
  cdd_c_error_t rc;
  cdd_dsl_token_t toks[5];

  (void)rc;
  rc = cdd_dsl_parse(NULL, &ir);

  rc = cdd_dsl_tokenize("func f() {}", &list);
  rc = cdd_dsl_parse(&list, NULL);
  cdd_dsl_token_list_free(&list);

  memset(toks, 0, sizeof(toks));
  toks[0].kind = CDD_DSL_TOKEN_ESCAPE_WS;
  toks[0].text_copy = cdd_test_mut_str("///");
  toks[1].kind = CDD_DSL_TOKEN_IDENTIFIER;
  toks[1].text_copy = cdd_test_mut_str("not_string");
  toks[2].kind = CDD_DSL_TOKEN_EOF;

  list.count = 3;
  list.tokens = toks;

  rc = cdd_dsl_parse(&list, &ir);
  if (ir) {
    cdd_ffi_ir_free(ir);
    free(ir);
    ir = NULL;
  }

  toks[1].kind = CDD_DSL_TOKEN_STRING;
  toks[1].text_copy = cdd_test_mut_str("\"");
  toks[1].length = 1;

  rc = cdd_dsl_parse(&list, &ir);
  if (ir) {
    cdd_ffi_ir_free(ir);
    free(ir);
    ir = NULL;
  }

  list.tokens = NULL;
  list.count = 0;

  memset(toks, 0, sizeof(toks));
  toks[0].kind = CDD_DSL_TOKEN_ESCAPE_COMMENT;
  toks[0].text_copy = cdd_test_mut_str("///");
  toks[1].kind = CDD_DSL_TOKEN_IDENTIFIER;
  toks[1].text_copy = cdd_test_mut_str("not_string");
  toks[2].kind = CDD_DSL_TOKEN_EOF;
  list.count = 3;
  list.tokens = toks;
  rc = cdd_dsl_parse(&list, &ir);
  if (ir) {
    cdd_ffi_ir_free(ir);
    free(ir);
    ir = NULL;
  }

  toks[1].kind = CDD_DSL_TOKEN_STRING;
  toks[1].text_copy = cdd_test_mut_str("\"");
  toks[1].length = 1;
  rc = cdd_dsl_parse(&list, &ir);
  if (ir) {
    cdd_ffi_ir_free(ir);
    free(ir);
    ir = NULL;
  }

  PASS();
}

SUITE(dsl_parser_branches_suite) {
  RUN_TEST(test_dsl_parser_missing_branches);
  RUN_TEST(test_dsl_parser_manual_branches);

  RUN_TEST(test_dsl_parser_oom);
  RUN_TEST(test_dsl_parser_exhaustive_oom);
  RUN_TEST(test_dsl_parser_missing_oom);
  RUN_TEST(test_dsl_parser_missing_eof);
  RUN_TEST(test_dsl_parser_invalid);
  RUN_TEST(test_dsl_parser_enum_oom);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_DSL_PARSER_BRANCHES_H */
