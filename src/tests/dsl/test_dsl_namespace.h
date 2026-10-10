#ifndef TEST_DSL_NAMESPACE_H
#define TEST_DSL_NAMESPACE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "../../include/dsl/cdd_dsl_parser.h"
#include "../cdd_test_helpers/cdd_helpers.h"
/* clang-format on */

TEST test_dsl_parser_namespace(void) {
  cdd_dsl_token_list_t list;
  cdd_ffi_ir_t *ir = NULL;
  cdd_c_error_t rc;
  const char *source = "namespace MySpace {\n  func test() {}\n}";

  rc = cdd_dsl_tokenize(source, &list);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");

  rc = cdd_dsl_parse(&list, &ir);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");

  ASSERT_STR_EQ("MySpace", ir->module_name);
  ASSERT_EQ_FMT((size_t)1, ir->nodes_count, "%zu");
  ASSERT_STR_EQ("test", ir->nodes[0].name);

  cdd_dsl_token_list_free(&list);
  cdd_ffi_ir_free(ir);
  free(ir);
  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_DSL_NAMESPACE_H */
