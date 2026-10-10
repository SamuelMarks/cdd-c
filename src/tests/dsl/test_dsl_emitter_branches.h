#if defined(_MSC_VER) || defined(__MINGW32__)
/* clang-format off */
#include <direct.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#endif
#ifndef TEST_DSL_EMITTER_BRANCHES_H
#define TEST_DSL_EMITTER_BRANCHES_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "../../include/dsl/cdd_ffi_emit_dsl.h"
#include "../cdd_test_helpers/cdd_helpers.h"
/* clang-format on */

TEST test_dsl_emitter_invalid(void) {
  cdd_c_error_t rc;
  struct cdd_generate_bindings_config_t {
    char *output_dir;
  } config = {0};

  rc = cdd_ffi_emit_dsl(NULL, (const void *)&config);
  ASSERT_EQ_FMT(CDD_C_ERROR_INVALID_ARGUMENT, rc, "%d");

  PASS();
}

extern C_CDD_EXPORT int g_fail_io_after;

TEST test_dsl_emitter_exhaustive(void) {
  cdd_ffi_ir_t *ir;
  cdd_ffi_ir_node_t *node;
  cdd_ffi_trivia_t *t1, *t2, *t3;
  struct cdd_generate_bindings_config_t {
    char *output_dir;
  } config = {0};
  cdd_c_error_t rc;

  ir = (cdd_ffi_ir_t *)calloc(1, sizeof(cdd_ffi_ir_t));
  config.output_dir = strdup("out");

  rc = ir_add_node(ir, CDD_FFI_NODE_FUNCTION, "exhaustive_func", &node);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");

  t1 = calloc(1, sizeof(cdd_ffi_trivia_t));
  t1->kind = CDD_FFI_TRIVIA_COMMENT_LINE;
  t1->text = strdup(" comment ");
  t2 = calloc(1, sizeof(cdd_ffi_trivia_t));
  t2->kind = CDD_FFI_TRIVIA_WHITESPACE;
  t2->text = strdup("\n");
  t1->next = t2;
  t3 = calloc(1, sizeof(cdd_ffi_trivia_t));
  t3->kind = CDD_FFI_TRIVIA_COMMENT_BLOCK;
  t3->text = strdup(" block ");
  t2->next = t3;

  node->leading_trivia = t1;

  node->fields = (cdd_ffi_field_t *)calloc(10, sizeof(cdd_ffi_field_t));
  node->fields_count = 10;

  node->fields[0].name = strdup("f1");
  node->fields[0].type.kind = CDD_FFI_KIND_VOID;

  node->fields[1].name = strdup("f2");
  node->fields[1].type.kind = CDD_FFI_KIND_UINT8;
  node->fields[1].type.pointer_depth = 1;
  node->fields[1].type.is_const = 1;

  node->fields[2].name = strdup("f3");
  node->fields[2].type.kind = CDD_FFI_KIND_INT16;
  node->fields[2].type.array_size = 10;
  node->fields[2].intent = CDD_FFI_INTENT_OUT;

  node->fields[3].name = strdup("f4");
  node->fields[3].type.kind = CDD_FFI_KIND_UINT16;
  node->fields[3].intent = CDD_FFI_INTENT_INOUT;

  node->fields[4].name = strdup("f5");
  node->fields[4].type.kind = CDD_FFI_KIND_UINT32;
  node->fields[4].array_length_ref = strdup("len5");

  node->fields[5].name = strdup("f6");
  node->fields[5].type.kind = CDD_FFI_KIND_INT64;
  node->fields[5].intent = CDD_FFI_INTENT_IN;
  node->fields[5].array_length_ref = strdup("len6");

  node->fields[6].name = strdup("f7");
  node->fields[6].type.kind = CDD_FFI_KIND_FLOAT32;

  node->fields[7].name = strdup("f8");
  node->fields[7].type.kind = CDD_FFI_KIND_FLOAT64;

  node->fields[8].name = strdup("f9");
  node->fields[8].type.kind = CDD_FFI_KIND_BOOL;

  node->fields[9].name = strdup("f10");
  node->fields[9].type.kind = CDD_FFI_KIND_UINT64;

  rc = cdd_ffi_emit_dsl(ir, (const void *)&config);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");

  {
    cdd_ffi_ir_node_t *node2;
    rc = ir_add_node(ir, CDD_FFI_NODE_FUNCTION, "exhaustive_func2", &node2);
    ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");

    node2->fields = (cdd_ffi_field_t *)calloc(4, sizeof(cdd_ffi_field_t));
    node2->fields_count = 4;

    node2->fields[0].name = strdup("f11");
    node2->fields[0].type.kind = CDD_FFI_KIND_INT8;

    node2->fields[1].name = strdup("f12");
    node2->fields[1].type.kind = CDD_FFI_KIND_STRUCT_REF;
    node2->fields[1].type.ref_name = strdup("MyStruct");

    node2->fields[2].name = strdup("f13");
    node2->fields[2].type.kind = CDD_FFI_KIND_STD_STRING;

    node2->fields[3].type.kind = CDD_FFI_KIND_ENUM_REF;
    node2->fields[3].type.ref_name = strdup("MyEnum");
  }

  rc = cdd_ffi_emit_dsl(ir, (const void *)&config);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");

  {
    cdd_ffi_ir_node_t *node3;
    rc = ir_add_node(ir, CDD_FFI_NODE_FUNCTION, "exhaustive_func_null_ref",
                     &node3);

    node3->fields = (cdd_ffi_field_t *)calloc(1, sizeof(cdd_ffi_field_t));
    node3->fields_count = 1;
    node3->fields[0].name = strdup("f14");
    node3->fields[0].type.kind = CDD_FFI_KIND_STRUCT_REF;
    node3->fields[0].type.ref_name = NULL;

    node3->leading_trivia = calloc(1, sizeof(cdd_ffi_trivia_t));
    node3->leading_trivia->kind = CDD_FFI_TRIVIA_WHITESPACE;
    node3->leading_trivia->text = NULL;

    node3->leading_trivia->next = calloc(1, sizeof(cdd_ffi_trivia_t));
    node3->leading_trivia->next->kind = CDD_FFI_TRIVIA_CONTINUATION;
    node3->leading_trivia->next->text = strdup("\\");
  }

  {
    cdd_ffi_ir_node_t *node4;
    rc = ir_add_node(ir, CDD_FFI_NODE_STRUCT, "some_struct", &node4);
  }
  rc = cdd_ffi_emit_dsl(ir, (const void *)&config);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");

  cdd_ffi_ir_free(ir);
  free(ir);
  free(config.output_dir);
  PASS();
}

TEST test_dsl_emitter_null_config(void) {
  cdd_ffi_ir_t ir = {0};
  cdd_c_error_t rc;

  rc = cdd_ffi_emit_dsl(&ir, NULL);
  ASSERT_EQ_FMT(CDD_C_ERROR_INVALID_ARGUMENT, rc, "%d");

  PASS();
}

TEST test_dsl_emitter_io_fail(void) {
  cdd_ffi_ir_t ir = {0};
  struct cdd_generate_bindings_config_t {
    char *output_dir;
  } config = {0};
  cdd_c_error_t rc;
  cdd_c_error_t rc2 = CDD_C_SUCCESS;
  ir.nodes_count = 0;
  remove("test_out_dsl.dsl");
#if defined(_MSC_VER) || defined(__MINGW32__)
  rc2 = _mkdir("test_out_dsl.dsl") == 0 ? CDD_C_SUCCESS : CDD_C_ERROR_UNKNOWN;
#else
  rc2 = mkdir("test_out_dsl.dsl", 0755) == 0 ? CDD_C_SUCCESS
                                             : CDD_C_ERROR_UNKNOWN;
#endif

  rc = cdd_ffi_emit_dsl(&ir, (const void *)&config);

#if defined(_MSC_VER) || defined(__MINGW32__)
  if (rc2 == CDD_C_SUCCESS) {
    _rmdir("test_out_dsl.dsl");
  }
#else
  if (rc2 == CDD_C_SUCCESS) {
    rmdir("test_out_dsl.dsl");
  }
#endif

  ASSERT_EQ_FMT(CDD_C_ERROR_IO, rc, "%d");

  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_DSL_EMITTER_BRANCHES_H */
