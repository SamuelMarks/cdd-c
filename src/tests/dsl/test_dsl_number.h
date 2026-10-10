#ifndef TEST_DSL_NUMBER_H
#define TEST_DSL_NUMBER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "../../include/dsl/cdd_dsl_parser.h"
#include "../cdd_test_helpers/cdd_helpers.h"
/* clang-format on */

TEST test_dsl_parser_number(void) {
  cdd_dsl_token_list_t list;
  cdd_ffi_ir_t *ir = NULL;
  cdd_c_error_t rc;
  const char *source =
      "enum Status {\n  OK = 0,\n  ERR = 0xFA,\n  TIMEOUT = 1'000\n}";

  rc = cdd_dsl_tokenize(source, &list);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");

  rc = cdd_dsl_parse(&list, &ir);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");

  ASSERT_EQ_FMT((size_t)1, ir->nodes_count, "%zu");
  ASSERT_EQ_FMT(CDD_FFI_NODE_ENUM, ir->nodes[0].kind, "%d");
  ASSERT_STR_EQ("Status", ir->nodes[0].name);
  ASSERT_EQ_FMT((size_t)3, ir->nodes[0].variants_count, "%zu");

  ASSERT_STR_EQ("OK", ir->nodes[0].variants[0].name);
  ASSERT_STR_EQ("0", ir->nodes[0].variants[0].number_details->raw_spelling);
  ASSERT_EQ_FMT(CDD_FFI_NUM_PREFIX_NONE,
                ir->nodes[0].variants[0].number_details->prefix, "%d");

  ASSERT_STR_EQ("ERR", ir->nodes[0].variants[1].name);
  ASSERT_STR_EQ("0xFA", ir->nodes[0].variants[1].number_details->raw_spelling);
  ASSERT_EQ_FMT(CDD_FFI_NUM_PREFIX_HEX,
                ir->nodes[0].variants[1].number_details->prefix, "%d");

  ASSERT_STR_EQ("TIMEOUT", ir->nodes[0].variants[2].name);
  ASSERT_STR_EQ("1'000", ir->nodes[0].variants[2].number_details->raw_spelling);
  ASSERT_EQ_FMT(1, ir->nodes[0].variants[2].number_details->has_separators,
                "%d");

  cdd_dsl_token_list_free(&list);
  cdd_ffi_ir_free(ir);
  free(ir);
  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_DSL_NUMBER_H */
