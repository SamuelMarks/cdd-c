/**
 * @file test_codegen_struct.h
 * @brief Unit tests for Struct Lifecycle generation logic and reflection suite.
 *
 * Verifies that C utility string generation includes necessary null checks,
 * memory allocations, recursion, and runtime reflection descriptors.
 *
 * @author Samuel Marks
 */

#ifndef TEST_CODEGEN_STRUCT_H
#define TEST_CODEGEN_STRUCT_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "emit/test_codegen_struct_common.h"
#include "emit/test_codegen_struct_lifecycle.h"
#include "emit/test_codegen_struct_fields.h"
#include "emit/test_codegen_struct_reflect.h"
#include "emit/test_codegen_struct_reflect_adv.h"
#include "emit/test_codegen_struct_reflect_edges.h"
/* clang-format on */

/**
 * @brief Executes the codegen_struct_suite test suite.
 */
SUITE(codegen_struct_suite) {
  RUN_TEST(test_struct_fields_init_oom);
  RUN_TEST(test_struct_reflection_and_codegen);
  RUN_TEST(test_cleanup_generation);
  RUN_TEST(test_default_generation);
  RUN_TEST(test_deepcopy_generation);
  RUN_TEST(test_eq_generation);
  RUN_TEST(test_guards_injection);
  RUN_TEST(test_null_args);
  RUN_TEST(test_struct_fields_add_bitwidth);
  RUN_TEST(test_struct_debug_func);
  RUN_TEST(test_struct_invalid_args);
  RUN_TEST(test_reflection_advanced_features);
  RUN_TEST(test_reflection_edge_cases_and_oom);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CODEGEN_STRUCT_H */
