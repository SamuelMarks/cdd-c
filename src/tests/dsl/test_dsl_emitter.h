#ifndef TEST_DSL_EMITTER_H
#define TEST_DSL_EMITTER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "../../include/dsl/cdd_dsl_parser.h"
#include "../../include/dsl/cdd_ffi_emit_dsl.h"
#include "../cdd_test_helpers/cdd_helpers.h"

#include <string.h>
/* clang-format on */

extern C_CDD_EXPORT cdd_c_error_t ir_add_node(cdd_ffi_ir_t *ir,
                                              cdd_ffi_node_kind_t kind,
                                              const char *name,
                                              cdd_ffi_ir_node_t **out_node);

TEST test_dsl_emitter_basic(void) {
  cdd_ffi_ir_t *ir;
  cdd_ffi_ir_node_t *node;
  struct cdd_generate_bindings_config_t {
    char *output_dir;
  } config = {0};
  cdd_c_error_t rc;

  ir = (cdd_ffi_ir_t *)calloc(1, sizeof(cdd_ffi_ir_t));
  config.output_dir = (char *)(size_t)(size_t) "out";

  rc = ir_add_node(ir, CDD_FFI_NODE_FUNCTION, "test_func", &node);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");

  node->fields = (cdd_ffi_field_t *)calloc(1, sizeof(cdd_ffi_field_t));
  node->fields_count = 1;
  node->fields[0].name = strdup("arg1");
  node->fields[0].type.kind = CDD_FFI_KIND_INT32;
  node->fields[0].type.pointer_depth = 1;
  node->fields[0].intent = CDD_FFI_INTENT_IN;

  node->raw_body = strdup(" return 0; ");

  rc = cdd_ffi_emit_dsl(ir, (const void *)&config);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");

  cdd_ffi_ir_free(ir);
  free(ir);
  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_DSL_EMITTER_H */
