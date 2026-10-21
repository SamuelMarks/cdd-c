/**
 * @file test_ffi_ir_json.h
 * @brief Unit tests and mocks in test_ffi_ir_json.h.
 */
#ifndef TEST_FFI_IR_JSON_H
#define TEST_FFI_IR_JSON_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "../../../include/ffi/cdd_ffi_ir.h"
#include "../../functions/ffi/cdd_ffi_emit_ir_json.h"
#include "../../src/cdd_api.h"
#include "routes/parse/cli.h"
#include "greatest.h"
#include <string.h>
/* clang-format on */

/**
 * @brief Helper function set_both_parson_allocs_ffi.
 * @param[in,out] malloc_fun Parameter malloc_fun.
 * @param[in,out] free_fun Parameter free_fun.
 */
static void set_both_parson_allocs_ffi(void *(*malloc_fun)(size_t),
                                       void (*free_fun)(void *)) {
  (json_set_allocation_functions)(malloc_fun, free_fun);
  c2openapi_set_json_allocators(malloc_fun, free_fun);
}
#ifdef json_set_allocation_functions
#undef json_set_allocation_functions
#endif
/**
 * @brief Macro json_set_allocation_functions.
 */
#define json_set_allocation_functions set_both_parson_allocs_ffi

/**
 * @brief Unit test test_cdd_ffi_ir_json_basic.
 * @return Greatest test result.
 */
TEST test_cdd_ffi_ir_json_basic(void) {
  cdd_ffi_ir_t ir = {0};
  char *json_str = NULL;
  cdd_ffi_ir_t *out_ir = NULL;
  cdd_c_error_t rc;

  ir.nodes_count = 1;
  ir.nodes = calloc(1, sizeof(cdd_ffi_ir_node_t));
  ir.nodes[0].kind = CDD_FFI_NODE_FUNCTION;
  ir.nodes[0].name = cdd_test_mut_str("test_func");
  ir.nodes[0].doc = cdd_test_mut_str("test doc");
  ir.nodes[0].return_or_base_type.kind = CDD_FFI_KIND_INT32;

  ir.nodes[0].return_or_base_type.template_args_count = 1;
  ir.nodes[0].return_or_base_type.template_args =
      calloc(1, sizeof(cdd_ffi_type_t));
  ir.nodes[0].return_or_base_type.template_args[0].kind = CDD_FFI_KIND_INT32;
  ir.nodes[0].return_or_base_type.leading_trivia =
      calloc(1, sizeof(cdd_ffi_trivia_t));
  ir.nodes[0].return_or_base_type.leading_trivia->text =
      cdd_test_mut_str("/* t */");
  ir.nodes[0].return_or_base_type.trailing_trivia =
      calloc(1, sizeof(cdd_ffi_trivia_t));
  ir.nodes[0].return_or_base_type.trailing_trivia->text =
      cdd_test_mut_str("/* t */");
  ir.nodes[0].return_or_base_type.raw_spelling = cdd_test_mut_str("i32");

  ir.nodes[0].fields_count = 1;
  ir.nodes[0].fields = calloc(1, sizeof(cdd_ffi_field_t));
  ir.nodes[0].fields[0].name = cdd_test_mut_str("f");
  ir.nodes[0].fields[0].leading_trivia = calloc(1, sizeof(cdd_ffi_trivia_t));
  ir.nodes[0].fields[0].trailing_trivia = calloc(1, sizeof(cdd_ffi_trivia_t));

  ir.nodes[0].base_classes_count = 1;
  ir.nodes[0].base_classes = calloc(1, sizeof(cdd_ffi_base_class_t));
  ir.nodes[0].base_classes[0].name = cdd_test_mut_str("Base");
  ir.nodes[0].base_classes[0].access = cdd_test_mut_str("public");

  ir.nodes[0].virtual_methods_count = 1;
  ir.nodes[0].virtual_methods = calloc(1, sizeof(cdd_ffi_virtual_method_t));
  ir.nodes[0].virtual_methods[0].name = cdd_test_mut_str("vfunc");

  ir.nodes[0].variants_count = 1;
  ir.nodes[0].variants = calloc(1, sizeof(cdd_ffi_enum_variant_t));
  ir.nodes[0].variants[0].name = cdd_test_mut_str("V1");
  ir.nodes[0].variants[0].number_details =
      calloc(1, sizeof(struct cdd_ffi_number_t));
  ir.nodes[0].variants[0].leading_trivia = calloc(1, sizeof(cdd_ffi_trivia_t));
  ir.nodes[0].variants[0].trailing_trivia = calloc(1, sizeof(cdd_ffi_trivia_t));
  ir.nodes[0].variants[0].raw_spelling = cdd_test_mut_str("V1");

  ir.nodes[0].macro_expansions_count = 1;
  ir.nodes[0].macro_expansions = calloc(1, sizeof(cdd_ffi_macro_expansion_t));
  ir.nodes[0].macro_expansions[0].macro_name = cdd_test_mut_str("MY_MACRO");
  ir.nodes[0].macro_expansions[0].raw_args = cdd_test_mut_str("a, b");
  ir.nodes[0].macro_expansions[0].expanded_text = cdd_test_mut_str("a + b");

  rc = cdd_ffi_ir_to_json(&ir, &json_str);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(json_str != NULL);

  rc = cdd_ffi_ir_from_json(json_str, &out_ir);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(out_ir != NULL);
  ASSERT_EQ(1, out_ir->nodes_count);
  ASSERT_STR_EQ("test_func", out_ir->nodes[0].name);

  cdd_ffi_ir_free(out_ir);
  free(out_ir);
  free(json_str);

  free(ir.nodes[0].macro_expansions);
  free(ir.nodes[0].variants[0].number_details);
  free(ir.nodes[0].variants[0].leading_trivia);
  free(ir.nodes[0].variants[0].trailing_trivia);
  free(ir.nodes[0].variants);
  free(ir.nodes[0].virtual_methods);
  free(ir.nodes[0].base_classes);
  free(ir.nodes[0].fields[0].leading_trivia);
  free(ir.nodes[0].fields[0].trailing_trivia);
  free(ir.nodes[0].fields);
  free(ir.nodes[0].return_or_base_type.template_args);
  free(ir.nodes[0].return_or_base_type.leading_trivia);
  free(ir.nodes[0].return_or_base_type.trailing_trivia);
  free(ir.nodes);
  PASS();
}

/**
 * @brief Unit test test_cdd_ffi_ir_json_nulls.
 * @return Greatest test result.
 */
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
extern C_CDD_EXPORT volatile int g_ffi_extractor_alloc_fail;

static void *mocked_json_malloc(size_t size) {
  if (g_cdd_alloc_fail == 1)
    return NULL;
  if (g_ffi_extractor_alloc_fail == 1) {
    g_ffi_extractor_alloc_fail = 0; /* Only fail once for the loop */
    return NULL;
  }
  if (g_ffi_extractor_alloc_fail > 1) {
    g_ffi_extractor_alloc_fail--;
  }
  return malloc(size);
}

static void mocked_json_free(void *ptr) { free(ptr); }

TEST test_cdd_ffi_ir_json_oom(void) {
  cdd_ffi_ir_t ir = {0};
  char *json_str = NULL;
  cdd_ffi_ir_t *out_ir = NULL;
  cdd_c_error_t rc;

  ir.nodes_count = 1;
  ir.nodes = calloc(1, sizeof(cdd_ffi_ir_node_t));
  ir.nodes[0].kind = CDD_FFI_NODE_FUNCTION;
  ir.nodes[0].name = cdd_test_mut_str("test_func");
  ir.nodes[0].doc = cdd_test_mut_str("test doc");
  ir.nodes[0].fields_count = 1;
  ir.nodes[0].fields = calloc(1, sizeof(cdd_ffi_field_t));
  ir.nodes[0].fields[0].name = cdd_test_mut_str("param1");

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

TEST test_cdd_ffi_ir_json_exhaustive_oom(void) {
  cdd_ffi_ir_t ir = {0};
  char *json_str = NULL;
  cdd_ffi_ir_t *out_ir = NULL;
  cdd_c_error_t rc;
  int i;

  json_set_allocation_functions(mocked_json_malloc, mocked_json_free);

  ir.nodes_count = 5;
  ir.nodes = calloc(5, sizeof(cdd_ffi_ir_node_t));

  ir.nodes[0].kind = CDD_FFI_NODE_FUNCTION;
  ir.nodes[0].name = C_CDD_STRDUP("func");
  ir.nodes[0].doc = C_CDD_STRDUP("fdoc");
  ir.nodes[0].raw_body = C_CDD_STRDUP("{ return 0; }");
  ir.nodes[0].leading_trivia = calloc(1, sizeof(cdd_ffi_trivia_t));
  ir.nodes[0].leading_trivia->text = C_CDD_STRDUP("/* nlead */");
  ir.nodes[0].leading_trivia->next = calloc(1, sizeof(cdd_ffi_trivia_t));
  ir.nodes[0].leading_trivia->next->text = C_CDD_STRDUP("/* nlead2 */");
  ir.nodes[0].trailing_trivia = calloc(1, sizeof(cdd_ffi_trivia_t));
  ir.nodes[0].trailing_trivia->text = C_CDD_STRDUP("/* ntrail */");
  ir.nodes[0].return_or_base_type.kind = CDD_FFI_KIND_INT32;
  ir.nodes[0].return_or_base_type.ref_name = C_CDD_STRDUP("MyRet");
  ir.nodes[0].return_or_base_type.array_size = 4;
  ir.nodes[0].return_or_base_type.template_args_count = 1;
  ir.nodes[0].return_or_base_type.template_args =
      calloc(1, sizeof(cdd_ffi_type_t));
  ir.nodes[0].return_or_base_type.template_args[0].kind = CDD_FFI_KIND_INT32;
  ir.nodes[0].return_or_base_type.template_args[0].ref_name =
      C_CDD_STRDUP("TArg");
  ir.nodes[0].return_or_base_type.leading_trivia =
      calloc(1, sizeof(cdd_ffi_trivia_t));
  ir.nodes[0].return_or_base_type.leading_trivia->text =
      C_CDD_STRDUP("/* t */");
  ir.nodes[0].return_or_base_type.trailing_trivia =
      calloc(1, sizeof(cdd_ffi_trivia_t));
  ir.nodes[0].return_or_base_type.trailing_trivia->text =
      C_CDD_STRDUP("/* t */");
  ir.nodes[0].return_or_base_type.raw_spelling = C_CDD_STRDUP("i32");

  ir.nodes[0].fields_count = 1;
  ir.nodes[0].fields = calloc(1, sizeof(cdd_ffi_field_t));
  ir.nodes[0].fields[0].name = C_CDD_STRDUP("param1");
  ir.nodes[0].fields[0].doc = C_CDD_STRDUP("pdoc");
  ir.nodes[0].fields[0].type.kind = CDD_FFI_KIND_INT8;
  ir.nodes[0].fields[0].type.pointer_depth = 1;
  ir.nodes[0].fields[0].leading_trivia = calloc(1, sizeof(cdd_ffi_trivia_t));
  ir.nodes[0].fields[0].leading_trivia->text = C_CDD_STRDUP("/* t */");
  ir.nodes[0].fields[0].trailing_trivia = calloc(1, sizeof(cdd_ffi_trivia_t));
  ir.nodes[0].fields[0].trailing_trivia->text = C_CDD_STRDUP("/* t */");
  ir.nodes[0].fields[0].array_length_ref = C_CDD_STRDUP("arr_len");

  ir.nodes[1].kind = CDD_FFI_NODE_STRUCT;
  ir.nodes[1].name = C_CDD_STRDUP("strct");
  ir.nodes[1].fields_count = 1;
  ir.nodes[1].fields = calloc(1, sizeof(cdd_ffi_field_t));
  ir.nodes[1].fields[0].name = C_CDD_STRDUP("f1");
  ir.nodes[1].fields[0].type.kind = CDD_FFI_KIND_STRUCT_REF;
  ir.nodes[1].fields[0].type.ref_name = C_CDD_STRDUP("inner");
  ir.nodes[1].base_classes_count = 1;
  ir.nodes[1].base_classes = calloc(1, sizeof(cdd_ffi_base_class_t));
  ir.nodes[1].base_classes[0].name = C_CDD_STRDUP("Base");
  ir.nodes[1].base_classes[0].access = C_CDD_STRDUP("public");
  ir.nodes[1].virtual_methods_count = 1;
  ir.nodes[1].virtual_methods = calloc(1, sizeof(cdd_ffi_virtual_method_t));
  ir.nodes[1].virtual_methods[0].name = C_CDD_STRDUP("vfunc");

  ir.nodes[2].kind = CDD_FFI_NODE_ENUM;
  ir.nodes[2].name = C_CDD_STRDUP("enm");
  ir.nodes[2].variants_count = 2;
  ir.nodes[2].variants = calloc(2, sizeof(cdd_ffi_enum_variant_t));
  ir.nodes[2].variants[0].name = C_CDD_STRDUP("E1");
  ir.nodes[2].variants[0].value = C_CDD_STRDUP("0");
  ir.nodes[2].variants[0].doc = C_CDD_STRDUP("edoc");
  ir.nodes[2].variants[0].number_details =
      calloc(1, sizeof(struct cdd_ffi_number_t));
  ir.nodes[2].variants[0].number_details->raw_spelling = C_CDD_STRDUP("0x0");
  ir.nodes[2].variants[0].number_details->has_separators = 1;
  ir.nodes[2].variants[0].leading_trivia = calloc(1, sizeof(cdd_ffi_trivia_t));
  ir.nodes[2].variants[0].leading_trivia->text = C_CDD_STRDUP("/* t */");
  ir.nodes[2].variants[0].trailing_trivia = calloc(1, sizeof(cdd_ffi_trivia_t));
  ir.nodes[2].variants[0].trailing_trivia->text = C_CDD_STRDUP("/* t */");
  ir.nodes[2].variants[0].raw_spelling = C_CDD_STRDUP("E1");
  ir.nodes[2].variants[1].name = C_CDD_STRDUP("E2");
  ir.nodes[2].variants[1].value = C_CDD_STRDUP("1");

  ir.nodes[3].kind = CDD_FFI_NODE_TYPEDEF;
  ir.nodes[3].name = C_CDD_STRDUP("tdef");
  ir.nodes[3].return_or_base_type.kind = CDD_FFI_KIND_INT16;

  ir.nodes[4].kind = CDD_FFI_NODE_MACRO;
  ir.nodes[4].name = C_CDD_STRDUP("MACRO_VAL");
  ir.nodes[4].evaluated_value = C_CDD_STRDUP("123");
  ir.nodes[4].macro_expansions_count = 1;
  ir.nodes[4].macro_expansions = calloc(1, sizeof(cdd_ffi_macro_expansion_t));
  ir.nodes[4].macro_expansions[0].macro_name = C_CDD_STRDUP("MY_MACRO");
  ir.nodes[4].macro_expansions[0].raw_args = C_CDD_STRDUP("a, b");
  ir.nodes[4].macro_expansions[0].expanded_text = C_CDD_STRDUP("a + b");

  for (i = 1; i < 600; i++) {
    g_ffi_extractor_alloc_fail = i;
    json_str = NULL;
    rc = cdd_ffi_ir_to_json(&ir, &json_str);
    if (rc == CDD_C_SUCCESS) {
      free(json_str);
      json_str = NULL;
    }
  }

  g_ffi_extractor_alloc_fail = 0;
  rc = cdd_ffi_ir_to_json(&ir, &json_str);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  for (i = 1; i < 300; i++) {
    g_ffi_extractor_alloc_fail = i;
    out_ir = NULL;
    rc = cdd_ffi_ir_from_json(json_str, &out_ir);
    if (out_ir) {
      cdd_ffi_ir_free(out_ir);
      free(out_ir);
      out_ir = NULL;
    }
  }
  g_ffi_extractor_alloc_fail = 0;
  json_set_allocation_functions(malloc, free);

  /* Also test C_CDD_CALLOC and C_CDD_STRDUP OOM in cdd_ffi_ir_from_json */
  for (i = 1; i < 200; i++) {
    g_cdd_alloc_fail = i;
    out_ir = NULL;
    rc = cdd_ffi_ir_from_json(json_str, &out_ir);
    g_cdd_alloc_fail = 0;
    if (out_ir) {
      cdd_ffi_ir_free(out_ir);
      free(out_ir);
      out_ir = NULL;
    }
  }
  /* Also test non-object element inside trivia array and null template_arg */
  {
    cdd_ffi_ir_t *triv_ir = NULL;
    rc = cdd_ffi_ir_from_json(
        "{\"nodes\":[{\"kind\":0,\"name\":\"f\","
        "\"return_or_base_type\":{\"kind\":0,\"template_args\":[null]},"
        "\"leading_trivia\":[null,{\"kind\":0,\"text\":\"x\"}]}]}",
        &triv_ir);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    if (triv_ir) {
      cdd_ffi_ir_free(triv_ir);
      free(triv_ir);
    }
  }
  /* Also test non-object root JSON for cdd_ffi_ir_from_json */
  out_ir = NULL;
  ASSERT_EQ(CDD_C_ERROR_PARSE, cdd_ffi_ir_from_json("[]", &out_ir));

  free(json_str);
  cdd_ffi_ir_free(&ir);

  json_set_allocation_functions(malloc, free);
  PASS();
}
#endif

/**
 * @brief Unit test test_cdd_ffi_emit_ir_json.
 * @return Greatest test result.
 */
TEST test_cdd_ffi_emit_ir_json(void) {
  cdd_ffi_ir_t ir = {0};
  cdd_generate_bindings_config_t config = {0};
  cdd_c_error_t rc;

  ir.nodes_count = 1;
  ir.nodes = calloc(1, sizeof(cdd_ffi_ir_node_t));
  ir.nodes[0].kind = CDD_FFI_NODE_FUNCTION;
  ir.nodes[0].name = cdd_test_mut_str("test_func");

  config.output_dir = ".";

  rc = cdd_ffi_emit_ir_json(&ir, &config);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  remove("ir_dump.json");

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

/**
 * @brief Executes the test_ffi_ir_json_suite test suite.
 */
SUITE(test_ffi_ir_json_suite) {
  RUN_TEST(test_cdd_ffi_ir_json_basic);
  RUN_TEST(test_cdd_ffi_ir_json_nulls);
  RUN_TEST(test_cdd_ffi_emit_ir_json);
#ifdef CDD_BUILD_TESTS
  RUN_TEST(test_cdd_ffi_ir_json_oom);
  RUN_TEST(test_cdd_ffi_ir_json_exhaustive_oom);
#endif
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_FFI_IR_JSON_H */
