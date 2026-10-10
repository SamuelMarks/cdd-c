#ifndef TEST_FFI_IR_JSON_H
#define TEST_FFI_IR_JSON_H

/* clang-format off */
#include "../../../include/ffi/cdd_ffi_ir.h"
#include "../../functions/ffi/cdd_ffi_emit_ir_json.h"
#include "../../src/cdd_api.h"
#include "greatest.h"
#include <string.h>
/* clang-format on */

TEST test_cdd_ffi_ir_json_basic(void) {
  cdd_ffi_ir_t ir = {0};
  char *json_str = NULL;
  cdd_ffi_ir_t *out_ir = NULL;
  cdd_c_error_t rc;

  ir.nodes_count = 1;
  ir.nodes = calloc(1, sizeof(cdd_ffi_ir_node_t));
  ir.nodes[0].kind = CDD_FFI_NODE_FUNCTION;
  ir.nodes[0].name = (char *)(size_t)(size_t) "test_func";
  ir.nodes[0].doc = (char *)(size_t)(size_t) "test doc";
  ir.nodes[0].return_or_base_type.kind = CDD_FFI_KIND_INT32;

  rc = cdd_ffi_ir_to_json(&ir, &json_str);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(json_str != NULL);

  rc = cdd_ffi_ir_from_json(json_str, &out_ir);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(out_ir != NULL);
  ASSERT_EQ(1, out_ir->nodes_count);
  ASSERT_STR_EQ("test_func", out_ir->nodes[0].name);
  ASSERT_STR_EQ("test doc", out_ir->nodes[0].doc);
  ASSERT_EQ(CDD_FFI_NODE_FUNCTION, out_ir->nodes[0].kind);
  ASSERT_EQ(CDD_FFI_KIND_INT32, out_ir->nodes[0].return_or_base_type.kind);

  cdd_ffi_ir_free(out_ir);
  free(json_str);
  free(ir.nodes);
  PASS();
}

TEST test_cdd_ffi_ir_json_nulls(void) {
  char *json_str = NULL;
  cdd_ffi_ir_t *out_ir = NULL;

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_ffi_ir_to_json(NULL, &json_str));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_ffi_ir_from_json(NULL, &out_ir));

  /* invalid json */
  ASSERT_EQ(CDD_C_ERROR_PARSE, cdd_ffi_ir_from_json("{invalid json}", &out_ir));

  PASS();
}

#ifdef CDD_BUILD_TESTS
TEST test_cdd_ffi_ir_json_oom(void) {
  cdd_ffi_ir_t ir = {0};
  char *json_str = NULL;
  cdd_ffi_ir_t *out_ir = NULL;
  cdd_c_error_t rc;

  ir.nodes_count = 1;
  ir.nodes = calloc(1, sizeof(cdd_ffi_ir_node_t));
  ir.nodes[0].kind = CDD_FFI_NODE_FUNCTION;
  ir.nodes[0].name = (char *)(size_t)(size_t) "test_func";
  ir.nodes[0].doc = (char *)(size_t)(size_t) "test doc";
  ir.nodes[0].fields_count = 1;
  ir.nodes[0].fields = calloc(1, sizeof(cdd_ffi_field_t));
  ir.nodes[0].fields[0].name = (char *)(size_t)(size_t) "param1";

  rc = cdd_ffi_ir_to_json(&ir, &json_str);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* Simulate allocation failure during parse */
  g_cdd_alloc_fail = 1;
  rc = cdd_ffi_ir_from_json(json_str, &out_ir);
  g_cdd_alloc_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);

  free(json_str);
  free(ir.nodes[0].fields);
  free(ir.nodes);
  PASS();
}
#endif

TEST test_cdd_ffi_emit_ir_json(void) {
  cdd_ffi_ir_t ir = {0};
  cdd_generate_bindings_config_t config = {0};
  cdd_c_error_t rc;

  ir.nodes_count = 1;
  ir.nodes = calloc(1, sizeof(cdd_ffi_ir_node_t));
  ir.nodes[0].kind = CDD_FFI_NODE_FUNCTION;
  ir.nodes[0].name = (char *)(size_t) "test_func";

  config.output_dir = ".";

  rc = cdd_ffi_emit_ir_json(&ir, &config);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  /* Missing args */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_ffi_emit_ir_json(NULL, &config));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_ffi_emit_ir_json(&ir, NULL));
  config.output_dir = NULL;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_ffi_emit_ir_json(&ir, &config));

  /* IO error */
#ifndef _WIN32
  config.output_dir = "/dev/null/impossible";
  ASSERT_EQ(CDD_C_ERROR_IO, cdd_ffi_emit_ir_json(&ir, &config));
#endif

  free(ir.nodes);
  PASS();
}

SUITE(test_ffi_ir_json_suite) {
  RUN_TEST(test_cdd_ffi_ir_json_basic);
  RUN_TEST(test_cdd_ffi_ir_json_nulls);
  RUN_TEST(test_cdd_ffi_emit_ir_json);
#ifdef CDD_BUILD_TESTS
  RUN_TEST(test_cdd_ffi_ir_json_oom);
#endif
}

#endif /* TEST_FFI_IR_JSON_H */
