#ifndef TEST_DSL_MODIFIERS_H
#define TEST_DSL_MODIFIERS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "../../include/dsl/cdd_dsl_parser.h"
#include "../cdd_test_helpers/cdd_helpers.h"
/* clang-format on */

TEST test_dsl_parser_modifiers(void) {
  cdd_dsl_token_list_t list;
  cdd_ffi_ir_t *ir = NULL;
  cdd_c_error_t rc;
  const char *source = "virtual pure func my_func() = 0;";

  rc = cdd_dsl_tokenize(source, &list);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");

  rc = cdd_dsl_parse(&list, &ir);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");

  ASSERT_EQ_FMT((size_t)1, ir->nodes_count, "%zu");
  ASSERT_EQ_FMT(CDD_FFI_NODE_FUNCTION, ir->nodes[0].kind, "%d");
  ASSERT_STR_EQ("my_func", ir->nodes[0].name);

  cdd_dsl_token_list_free(&list);
  cdd_ffi_ir_free(ir);
  free(ir);
  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_DSL_MODIFIERS_H */
