#ifndef TEST_DSL_MACROS_H
#define TEST_DSL_MACROS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "../../include/dsl/cdd_dsl_parser.h"
#include "../cdd_test_helpers/cdd_helpers.h"
/* clang-format on */

TEST test_dsl_parser_macros(void) {
  cdd_dsl_token_list_t list;
  cdd_ffi_ir_t *ir = NULL;
  cdd_c_error_t rc;
  const char *source =
      "macro_const MAX_BUF = 1024;\nmacro_expr MULT(x, y) = body %{ (x * y) "
      "}%;\nmacro_stmt LOG() = body %{ printf(\"hi\"); }%;";

  rc = cdd_dsl_tokenize(source, &list);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");

  rc = cdd_dsl_parse(&list, &ir);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");

  ASSERT_EQ_FMT((size_t)3, ir->nodes_count, "%zu");

  ASSERT_EQ_FMT(CDD_FFI_NODE_MACRO, ir->nodes[0].kind, "%d");
  ASSERT_STR_EQ("MAX_BUF", ir->nodes[0].name);
  ASSERT_STR_EQ("1024", ir->nodes[0].evaluated_value);

  ASSERT_EQ_FMT(CDD_FFI_NODE_MACRO, ir->nodes[1].kind, "%d");
  ASSERT_STR_EQ("MULT", ir->nodes[1].name);
  ASSERT_STR_EQ(" (x * y) ", ir->nodes[1].raw_body);

  ASSERT_EQ_FMT(CDD_FFI_NODE_MACRO, ir->nodes[2].kind, "%d");
  ASSERT_STR_EQ("LOG", ir->nodes[2].name);
  ASSERT_STR_EQ(" printf(\"hi\"); ", ir->nodes[2].raw_body);

  cdd_dsl_token_list_free(&list);
  cdd_ffi_ir_free(ir);
  free(ir);
  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_DSL_MACROS_H */
