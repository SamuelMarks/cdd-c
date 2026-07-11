#ifndef TEST_CDD_FFI_IR_H
#define TEST_CDD_FFI_IR_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "c_cdd_export.h"
#include <greatest.h>
#include <stdlib.h>
#include <string.h>

#include <ffi/cdd_ffi_ir.h>

#ifdef CDD_BUILD_TESTS
extern C_CDD_EXPORT int g_cdd_ffi_ir_calloc_fail;
extern C_CDD_EXPORT int g_cdd_ffi_ir_malloc_fail;
#endif

TEST test_ffi_ir_toposort_null(void) {
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_ffi_ir_topological_sort(NULL));
  PASS();
}

TEST test_ffi_ir_toposort_empty(void) {
  cdd_ffi_ir_t ir = {0};
  ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_ir_topological_sort(&ir));
  PASS();
}

TEST test_ffi_ir_free_null(void) {
  cdd_ffi_ir_free(NULL);
  PASS();
}

TEST test_ffi_ir_free_full(void) {
  cdd_ffi_ir_t ir = {0};
  cdd_ffi_ir_node_t *n;

  ir.nodes_count = 1;
  ir.nodes = (cdd_ffi_ir_node_t *)calloc(1, sizeof(cdd_ffi_ir_node_t));
  n = &ir.nodes[0];

  n->name = strdup("Node1");
  n->doc = strdup("Doc1");
  n->evaluated_value = strdup("Val1");

  n->return_or_base_type.ref_name = strdup("RetRef");
  n->return_or_base_type.template_args_count = 1;
  n->return_or_base_type.template_args =
      (cdd_ffi_type_t *)calloc(1, sizeof(cdd_ffi_type_t));
  n->return_or_base_type.template_args[0].ref_name = strdup("TArg1");

  n->fields_count = 1;
  n->fields = (cdd_ffi_field_t *)calloc(1, sizeof(cdd_ffi_field_t));
  n->fields[0].name = strdup("F1");
  n->fields[0].doc = strdup("FDoc1");
  n->fields[0].array_length_ref = strdup("FLen1");
  n->fields[0].type.ref_name = strdup("FType1");

  n->base_classes_count = 1;
  n->base_classes =
      (cdd_ffi_base_class_t *)calloc(1, sizeof(cdd_ffi_base_class_t));
  n->base_classes[0].name = strdup("Base1");
  n->base_classes[0].access = strdup("public");

  n->virtual_methods_count = 1;
  n->virtual_methods =
      (cdd_ffi_virtual_method_t *)calloc(1, sizeof(cdd_ffi_virtual_method_t));
  n->virtual_methods[0].name = strdup("VMethod1");
  n->virtual_methods[0].return_type.ref_name = strdup("VRet1");
  n->virtual_methods[0].args_count = 1;
  n->virtual_methods[0].args =
      (cdd_ffi_field_t *)calloc(1, sizeof(cdd_ffi_field_t));
  n->virtual_methods[0].args[0].name = strdup("VArg1");
  n->virtual_methods[0].args[0].doc = strdup("VArgDoc1");
  n->virtual_methods[0].args[0].type.ref_name = strdup("VArgType1");

  n->variants_count = 1;
  n->variants =
      (cdd_ffi_enum_variant_t *)calloc(1, sizeof(cdd_ffi_enum_variant_t));
  n->variants[0].name = strdup("Var1");
  n->variants[0].value = strdup("VarVal1");
  n->variants[0].doc = strdup("VarDoc1");

  cdd_ffi_ir_free(&ir);
  PASS();
}

TEST test_ffi_ir_toposort_complex(void) {
  cdd_ffi_ir_t ir = {0};
  cdd_ffi_ir_node_t *n;
  int rc;

  ir.nodes_count = 11;
  ir.nodes = (cdd_ffi_ir_node_t *)calloc(11, sizeof(cdd_ffi_ir_node_t));

  /* Node 0: struct A, depends on B */
  n = &ir.nodes[0];
  n->name = strdup("A");
  n->kind = CDD_FFI_NODE_STRUCT;
  n->fields_count = 1;
  n->fields = (cdd_ffi_field_t *)calloc(1, sizeof(cdd_ffi_field_t));
  n->fields[0].type.ref_name = strdup("B");

  /* Node 1: struct B, depends on C */
  n = &ir.nodes[1];
  n->name = strdup("B");
  n->kind = CDD_FFI_NODE_STRUCT;
  n->fields_count = 1;
  n->fields = (cdd_ffi_field_t *)calloc(1, sizeof(cdd_ffi_field_t));
  n->fields[0].type.ref_name = strdup("C");

  /* Node 2: struct C, no deps but let's make it have a cycle with D */
  n = &ir.nodes[2];
  n->name = strdup("C");
  n->kind = CDD_FFI_NODE_STRUCT;
  n->fields_count = 2;
  n->fields = (cdd_ffi_field_t *)calloc(2, sizeof(cdd_ffi_field_t));
  n->fields[0].type.ref_name = strdup("D");
  n->fields[1].type.ref_name = strdup("NON_EXISTENT"); /* Missing dep */

  /* Node 3: struct D, depends on C (cycle) */
  n = &ir.nodes[3];
  n->name = strdup("D");
  n->kind = CDD_FFI_NODE_STRUCT;
  n->fields_count = 1;
  n->fields = (cdd_ffi_field_t *)calloc(1, sizeof(cdd_ffi_field_t));
  n->fields[0].type.ref_name = strdup("C");

  /* Node 4: typedef myA A */
  n = &ir.nodes[4];
  n->name = strdup("myA");
  n->kind = CDD_FFI_NODE_TYPEDEF;
  n->return_or_base_type.ref_name = strdup("A");

  /* Node 5: function func1, returns A, takes B */
  n = &ir.nodes[5];
  n->name = strdup("func1");
  n->kind = CDD_FFI_NODE_FUNCTION;
  n->return_or_base_type.ref_name = strdup("A");
  n->fields_count = 1;
  n->fields = (cdd_ffi_field_t *)calloc(1, sizeof(cdd_ffi_field_t));
  n->fields[0].type.ref_name = strdup("B");

  /* Node 6: union U */
  n = &ir.nodes[6];
  n->name = strdup("U");
  n->kind = CDD_FFI_NODE_UNION;
  n->fields_count = 1;
  n->fields = (cdd_ffi_field_t *)calloc(1, sizeof(cdd_ffi_field_t));
  n->fields[0].type.ref_name = strdup("A");

  /* Node 7: Unnamed node to cover NULL name check in find_node_index */
  n = &ir.nodes[7];
  n->name = NULL;
  n->kind = CDD_FFI_NODE_STRUCT;

  /* Node 8: struct with primitive field to cover type->ref_name == NULL */
  n = &ir.nodes[8];
  n->name = strdup("PrimitiveStruct");
  n->kind = CDD_FFI_NODE_STRUCT;
  n->fields_count = 1;
  n->fields = (cdd_ffi_field_t *)calloc(1, sizeof(cdd_ffi_field_t));
  n->fields[0].type.ref_name = NULL;

  /* Node 9: typedef with primitive base type to cover
   * return_or_base_type.ref_name == NULL */
  n = &ir.nodes[9];
  n->name = strdup("PrimitiveTypedef");
  n->kind = CDD_FFI_NODE_TYPEDEF;
  n->return_or_base_type.ref_name = NULL;

  /* Node 10: typedef with missing base type to cover dep_idx == -1 */
  n = &ir.nodes[10];
  n->name = strdup("MissingTypedef");
  n->kind = CDD_FFI_NODE_TYPEDEF;
  n->return_or_base_type.ref_name = strdup("MISSING_TYPE_99");

  rc = cdd_ffi_ir_topological_sort(&ir);
  ASSERT_EQ(0, rc);

  /* C should be before B, B before A, A before myA */
  size_t idxA = 0, idxB = 0, idxC = 0, idxD = 0, idxM = 0;
  size_t i;
  for (i = 0; i < ir.nodes_count; i++) {
    if (!ir.nodes[i].name)
      continue;
    if (strcmp(ir.nodes[i].name, "A") == 0)
      idxA = i;
    else if (strcmp(ir.nodes[i].name, "B") == 0)
      idxB = i;
    else if (strcmp(ir.nodes[i].name, "C") == 0)
      idxC = i;
    else if (strcmp(ir.nodes[i].name, "D") == 0)
      idxD = i;
    else if (strcmp(ir.nodes[i].name, "myA") == 0)
      idxM = i;
  }

  ASSERT(idxC < idxB);
  ASSERT(idxB < idxA);
  ASSERT(idxA < idxM);
  (void)idxD; /* Just to avoid unused warning */

  cdd_ffi_ir_free(&ir);
  PASS();
}

TEST test_ffi_ir_free_empty(void) {
  cdd_ffi_ir_t ir = {0};
  cdd_ffi_ir_free(&ir);
  PASS();
}

TEST test_ffi_ir_free_partial(void) {
  cdd_ffi_ir_t ir = {0};
  cdd_ffi_ir_node_t *n;

  ir.nodes_count = 1;
  ir.nodes = (cdd_ffi_ir_node_t *)calloc(1, sizeof(cdd_ffi_ir_node_t));
  n = &ir.nodes[0];

  /* Allocate arrays but leave elements NULL to cover false branches */
  n->fields_count = 1;
  n->fields = (cdd_ffi_field_t *)calloc(1, sizeof(cdd_ffi_field_t));

  n->base_classes_count = 1;
  n->base_classes =
      (cdd_ffi_base_class_t *)calloc(1, sizeof(cdd_ffi_base_class_t));

  n->virtual_methods_count = 2;
  n->virtual_methods =
      (cdd_ffi_virtual_method_t *)calloc(2, sizeof(cdd_ffi_virtual_method_t));
  n->virtual_methods[0].args_count = 1;
  n->virtual_methods[0].args =
      (cdd_ffi_field_t *)calloc(1, sizeof(cdd_ffi_field_t));

  n->variants_count = 1;
  n->variants =
      (cdd_ffi_enum_variant_t *)calloc(1, sizeof(cdd_ffi_enum_variant_t));

  cdd_ffi_ir_free(&ir);
  PASS();
}

#ifdef CDD_BUILD_TESTS
extern C_CDD_EXPORT int g_cdd_ffi_ir_calloc_fail;
extern C_CDD_EXPORT int g_cdd_ffi_ir_malloc_fail;
#endif

TEST test_ffi_ir_c_toposort_oom(void) {
  cdd_ffi_ir_t ir = {0};
  cdd_ffi_ir_node_t *n;
  int rc;

  ir.nodes_count = 2;
  ir.nodes = (cdd_ffi_ir_node_t *)calloc(2, sizeof(cdd_ffi_ir_node_t));

  n = &ir.nodes[0];
  n->name = strdup("A");
  n->kind = CDD_FFI_NODE_STRUCT;
  n->fields_count = 1;
  n->fields = (cdd_ffi_field_t *)calloc(1, sizeof(cdd_ffi_field_t));
  n->fields[0].type.ref_name = strdup("B");

  n = &ir.nodes[1];
  n->name = strdup("B");
  n->kind = CDD_FFI_NODE_STRUCT;

#ifdef CDD_BUILD_TESTS
  g_cdd_ffi_ir_calloc_fail = 1;
  rc = cdd_ffi_ir_topological_sort(&ir);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  g_cdd_ffi_ir_calloc_fail = 0;

  g_cdd_ffi_ir_malloc_fail = 1;
  rc = cdd_ffi_ir_topological_sort(&ir);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  g_cdd_ffi_ir_malloc_fail = 0;
#endif

  cdd_ffi_ir_free(&ir);
  PASS();
}

SUITE(cdd_ffi_ir_suite) {
  RUN_TEST(test_ffi_ir_toposort_null);
  RUN_TEST(test_ffi_ir_toposort_empty);
  RUN_TEST(test_ffi_ir_free_null);
  RUN_TEST(test_ffi_ir_free_empty);
  RUN_TEST(test_ffi_ir_free_full);
  RUN_TEST(test_ffi_ir_free_partial);
  RUN_TEST(test_ffi_ir_toposort_complex);
  RUN_TEST(test_ffi_ir_c_toposort_oom);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CDD_FFI_IR_H */
