#ifndef TEST_DSL_PARSER_H
#define TEST_DSL_PARSER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "../../include/dsl/cdd_dsl_parser.h"
#include "../cdd_test_helpers/cdd_helpers.h"
/* clang-format on */

TEST test_dsl_parser_basic(void) {
  cdd_dsl_token_list_t list;
  cdd_ffi_ir_t *ir = NULL;
  const char *source = "/* comment */\nfunc my_func([in] *mut i32 arg1) {\n  "
                       "body %{ return 0; }%\n}";
  cdd_c_error_t rc;
  (void)rc;

  rc = cdd_dsl_tokenize(source, &list);

  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");

  rc = cdd_dsl_parse(&list, &ir);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");

  ASSERT_EQ_FMT((size_t)1, ir->nodes_count, "%zu");
  ASSERT_EQ_FMT(CDD_FFI_NODE_FUNCTION, ir->nodes[0].kind, "%d");
  ASSERT_STR_EQ("my_func", ir->nodes[0].name);
  ASSERT_EQ_FMT((size_t)1, ir->nodes[0].fields_count, "%zu");
  ASSERT_STR_EQ("arg1", ir->nodes[0].fields[0].name);
  ASSERT_EQ_FMT(CDD_FFI_KIND_INT32, ir->nodes[0].fields[0].type.kind, "%d");
  ASSERT_EQ_FMT(1, ir->nodes[0].fields[0].type.pointer_depth, "%d");
  ASSERT_EQ_FMT(CDD_FFI_INTENT_IN, ir->nodes[0].fields[0].intent, "%d");
  ASSERT_STR_EQ(" return 0; ", ir->nodes[0].raw_body);

  ASSERT(ir->nodes[0].leading_trivia != NULL);
  ASSERT_EQ_FMT(CDD_FFI_TRIVIA_COMMENT_BLOCK, ir->nodes[0].leading_trivia->kind,
                "%d");
  ASSERT_STR_EQ("/* comment */", ir->nodes[0].leading_trivia->text);

  cdd_dsl_token_list_free(&list);
  cdd_ffi_ir_free(ir);
  free(ir);
  PASS();
}

TEST test_dsl_parser_exhaustive(void) {
  cdd_dsl_token_list_t list;
  cdd_ffi_ir_t *ir = NULL;
  const char *source =
      "namespace my_ns {\n"
      "  enum my_enum {\n"
      "    VAL1,\n"
      "    VAL2 = 5\n"
      "  }\n"
      "  macro_const my_const = 10\n"
      "  macro_expr my_expr(a, b) = body %{ a + b }%\n"
      "  macro_stmt my_stmt(a, b) = body %{ printf(\"hello\"); }%\n"
      "  virtual pure func my_func([in] i32 arg1, [out] *mut u8 arg2, [inout, "
      "len=arg1] *const void arg3) {\n"
      "    body %{ return 0; }%\n"
      "  }\n"
      "}\n"
      "func another_func() = 1;\n"
      "func arr_func([in] [10] i32 arg) = 0;\n"
      "func str_func([in] string arg) = 0;\n"
      "/* block \n comment */\n"
      "#!ws(\"     \")\n"
      "#!comment(\"1234567890\")\n"
      "// line comment\n";
  cdd_c_error_t rc;

  rc = cdd_dsl_tokenize(source, &list);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");

  rc = cdd_dsl_parse(&list, &ir);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");

  cdd_dsl_token_list_free(&list);
  cdd_ffi_ir_free(ir);
  free(ir);
  PASS();
}

extern C_CDD_EXPORT volatile int g_ffi_extractor_alloc_fail;

TEST test_dsl_parser_oom_loop(void) {
  cdd_dsl_token_list_t list;
  cdd_ffi_ir_t *ir = NULL;
  const char *source =
      "namespace my_ns {\n"
      "  enum my_enum {\n"
      "    VAL1,\n"
      "    VAL2 = 5\n"
      "  }\n"
      "  macro_const my_const = 10\n"
      "  macro_expr my_expr(a, b) = body %{ a + b }%\n"
      "  macro_stmt my_stmt(a, b) = body %{ printf(\"hello\"); }%\n"
      "  virtual pure func my_func([in] i32 arg1, [out] *mut u8 arg2, [inout, "
      "len=arg1] *const void arg3) {\n"
      "    body %{ return 0; }%\n"
      "  }\n"
      "}\n"
      "func another_func() = 1;\n"
      "func arr_func([in] [10] i32 arg) = 0;\n"
      "func str_func([in] string arg) = 0;\n"
      "/* block \n comment */\n"
      "#!ws(\"     \")\n"
      "#!comment(\"1234567890\")\n"
      "// line comment\n";
  cdd_c_error_t rc;
  int i;

  rc = cdd_dsl_tokenize(source, &list);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");

  for (i = 1; i < 10000; i++) {
    g_ffi_extractor_alloc_fail = i;
    rc = cdd_dsl_parse(&list, &ir);
    g_ffi_extractor_alloc_fail = 0;
    if (rc == CDD_C_SUCCESS) {
      printf("OOM loop succeeded at i=%d\n", i);
      cdd_ffi_ir_free(ir);
      free(ir);
      ir = NULL;
      break;
    }
  }

  cdd_dsl_token_list_free(&list);
  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_DSL_PARSER_H */
