#ifndef TEST_FFI_EXTRACTOR_EMIT_H
#define TEST_FFI_EXTRACTOR_EMIT_H

/* clang-format off */
#include "../cdd_test_helpers/cdd_helpers.h"
#include "../../functions/ffi/cdd_ffi_ir_extractor.h"
#include "../../functions/ffi/cdd_ffi_emit_python.h"
#include "../../functions/ffi/cdd_ffi_emit_rust.h"
#include "../../functions/ffi/cdd_ffi_emit_csharp.h"
#include "../../functions/ffi/cdd_ffi_emit_typescript.h"
#include "../../functions/ffi/cdd_ffi_emit_napi.h"
#include "../../functions/ffi/cdd_ffi_emit_java.h"
#include "../../functions/ffi/cdd_ffi_emit_cpp.h"
#include "../../functions/ffi/cdd_ffi_emit_go.h"
#include "../../functions/ffi/cdd_ffi_emit_swift.h"
#include "../../functions/ffi/cdd_ffi_emit_dart.h"
#include "c_cdd/format_specifiers.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

TEST test_ffi_ir_emit_python(void) {
  cdd_ffi_ir_t *ir = (cdd_ffi_ir_t *)calloc(1, sizeof(cdd_ffi_ir_t));
  cdd_ffi_ir_node_t *nodes;
  cdd_generate_bindings_config_t config = {0};
  char *output_dir = (char *)(size_t)(size_t) "test_python_out";
  FILE *f;

  config.target_langs = "python";
  config.output_dir = output_dir;
  config.library_name = "test_lib";
  config.generate_tests = 1;

  /* create dummy dir */
  makedir(output_dir);

  ASSERT_EQ(1, ir != NULL);
  ir->nodes_capacity = 2;
  ir->nodes_count = 2;
  nodes = (cdd_ffi_ir_node_t *)calloc(2, sizeof(cdd_ffi_ir_node_t));
  ir->nodes = nodes;

  /* Node 0: struct A */
  nodes[0].kind = CDD_FFI_NODE_STRUCT;
  nodes[0].name = strdup("A");
  nodes[0].fields_count = 1;
  nodes[0].fields = (cdd_ffi_field_t *)calloc(1, sizeof(cdd_ffi_field_t));
  nodes[0].fields[0].name = strdup("b");
  nodes[0].fields[0].type.kind = CDD_FFI_KIND_INT32;

  /* Node 1: my_func */
  nodes[1].kind = CDD_FFI_NODE_FUNCTION;
  nodes[1].name = strdup("my_func");
  nodes[1].return_or_base_type.kind = CDD_FFI_KIND_VOID;

  ASSERT_EQ(0, cdd_ffi_emit_python(ir, &config));

  /* Assert file exists */
#if defined(_MSC_VER)
  fopen_s(&f, "test_python_out\\cdd_bindings.py", "r");
#else
#if defined(_MSC_VER)
  if (fopen_s(&f, "test_python_out/cdd_bindings.py", "r") != 0)
    f = NULL;
#else
  f = fopen("test_python_out/cdd_bindings.py", "r");
#endif
#endif
  ASSERT_EQ(1, f != NULL);
  if (f)
    if (f)
      fclose(f);

  cdd_ffi_ir_free(ir);
  free(ir);
  PASS();
}

TEST test_ffi_ir_emit_rust(void) {
  cdd_ffi_ir_t *ir = (cdd_ffi_ir_t *)calloc(1, sizeof(cdd_ffi_ir_t));
  cdd_ffi_ir_node_t *nodes;
  cdd_generate_bindings_config_t config = {0};
  char *output_dir = (char *)(size_t)(size_t) "test_rust_out";
  FILE *f;

  config.target_langs = "rust";
  config.output_dir = output_dir;
  config.library_name = "test_lib";
  config.generate_tests = 1;

  /* create dummy dir */
  makedir(output_dir);

  ASSERT_EQ(1, ir != NULL);
  ir->nodes_capacity = 2;
  ir->nodes_count = 2;
  nodes = (cdd_ffi_ir_node_t *)calloc(2, sizeof(cdd_ffi_ir_node_t));
  ir->nodes = nodes;

  /* Node 0: struct A */
  nodes[0].kind = CDD_FFI_NODE_STRUCT;
  nodes[0].name = strdup("A");
  nodes[0].fields_count = 1;
  nodes[0].fields = (cdd_ffi_field_t *)calloc(1, sizeof(cdd_ffi_field_t));
  nodes[0].fields[0].name = strdup("b");
  nodes[0].fields[0].type.kind = CDD_FFI_KIND_INT32;

  /* Node 1: my_func */
  nodes[1].kind = CDD_FFI_NODE_FUNCTION;
  nodes[1].name = strdup("my_func");
  nodes[1].return_or_base_type.kind = CDD_FFI_KIND_VOID;

  ASSERT_EQ(0, cdd_ffi_emit_rust(ir, &config));

  /* Assert files exist */
#if defined(_MSC_VER)
  fopen_s(&f, "test_rust_out\\Cargo.toml", "r");
#else
#if defined(_MSC_VER)
  if (fopen_s(&f, "test_rust_out/Cargo.toml", "r") != 0)
    f = NULL;
#else
  f = fopen("test_rust_out/Cargo.toml", "r");
#endif
#endif
  ASSERT_EQ(1, f != NULL);
  if (f)
    if (f)
      fclose(f);

  cdd_ffi_ir_free(ir);
  free(ir);
  PASS();
}

TEST test_ffi_ir_emit_csharp(void) {
  cdd_ffi_ir_t *ir = (cdd_ffi_ir_t *)calloc(1, sizeof(cdd_ffi_ir_t));
  cdd_ffi_ir_node_t *nodes;
  cdd_generate_bindings_config_t config = {0};
  char *output_dir = (char *)(size_t)(size_t) "test_csharp_out";
  FILE *f;

  config.target_langs = "csharp";
  config.output_dir = output_dir;
  config.library_name = "test_lib";
  config.generate_tests = 1;

  /* create dummy dir */
  makedir(output_dir);

  ASSERT_EQ(1, ir != NULL);
  ir->nodes_capacity = 2;
  ir->nodes_count = 2;
  nodes = (cdd_ffi_ir_node_t *)calloc(2, sizeof(cdd_ffi_ir_node_t));
  ir->nodes = nodes;

  /* Node 0: struct A */
  nodes[0].kind = CDD_FFI_NODE_STRUCT;
  nodes[0].name = strdup("A");
  nodes[0].fields_count = 1;
  nodes[0].fields = (cdd_ffi_field_t *)calloc(1, sizeof(cdd_ffi_field_t));
  nodes[0].fields[0].name = strdup("b");
  nodes[0].fields[0].type.kind = CDD_FFI_KIND_INT32;

  /* Node 1: my_func */
  nodes[1].kind = CDD_FFI_NODE_FUNCTION;
  nodes[1].name = strdup("my_func");
  nodes[1].return_or_base_type.kind = CDD_FFI_KIND_VOID;

  ASSERT_EQ(0, cdd_ffi_emit_csharp(ir, &config));

  /* Assert files exist */
#if defined(_MSC_VER)
  fopen_s(&f, "test_csharp_out\\Bindings.cs", "r");
#else
#if defined(_MSC_VER)
  if (fopen_s(&f, "test_csharp_out/Bindings.cs", "r") != 0)
    f = NULL;
#else
  f = fopen("test_csharp_out/Bindings.cs", "r");
#endif
#endif
  ASSERT_EQ(1, f != NULL);
  if (f)
    if (f)
      fclose(f);

  cdd_ffi_ir_free(ir);
  free(ir);
  PASS();
}

TEST test_ffi_ir_emit_typescript(void) {
  cdd_ffi_ir_t *ir = (cdd_ffi_ir_t *)calloc(1, sizeof(cdd_ffi_ir_t));
  cdd_ffi_ir_node_t *nodes;
  cdd_generate_bindings_config_t config = {0};
  char *output_dir = (char *)(size_t)(size_t) "test_typescript_out";
  FILE *f;

  config.target_langs = "typescript";
  config.output_dir = output_dir;
  config.library_name = "test_lib";
  config.generate_tests = 1;

  makedir(output_dir);

  ASSERT_EQ(1, ir != NULL);
  ir->nodes_capacity = 2;
  ir->nodes_count = 2;
  nodes = (cdd_ffi_ir_node_t *)calloc(2, sizeof(cdd_ffi_ir_node_t));
  ir->nodes = nodes;

  nodes[0].kind = CDD_FFI_NODE_STRUCT;
  nodes[0].name = strdup("A");
  nodes[0].fields_count = 1;
  nodes[0].fields = (cdd_ffi_field_t *)calloc(1, sizeof(cdd_ffi_field_t));
  nodes[0].fields[0].name = strdup("b");
  nodes[0].fields[0].type.kind = CDD_FFI_KIND_INT32;

  nodes[1].kind = CDD_FFI_NODE_FUNCTION;
  nodes[1].name = strdup("my_func");
  nodes[1].return_or_base_type.kind = CDD_FFI_KIND_VOID;

  ASSERT_EQ(0, cdd_ffi_emit_typescript(ir, &config));

#if defined(_MSC_VER)
  fopen_s(&f, "test_typescript_out\\test_lib.ts", "r");
#else
#if defined(_MSC_VER)
  if (fopen_s(&f, "test_typescript_out/test_lib.ts", "r") != 0)
    f = NULL;
#else
  f = fopen("test_typescript_out/test_lib.ts", "r");
#endif
#endif
  ASSERT_EQ(1, f != NULL);
  if (f)
    if (f)
      fclose(f);

  cdd_ffi_ir_free(ir);
  free(ir);
  PASS();
}

TEST test_ffi_ir_emit_napi(void) {
  cdd_ffi_ir_t *ir = (cdd_ffi_ir_t *)calloc(1, sizeof(cdd_ffi_ir_t));
  cdd_ffi_ir_node_t *nodes;
  cdd_generate_bindings_config_t config = {0};
  char *output_dir = (char *)(size_t)(size_t) "test_napi_out";
  FILE *f;

  config.target_langs = "napi";
  config.output_dir = output_dir;
  config.library_name = "test_lib";
  config.generate_tests = 1;
  config.input = "dummy_header.h";

  makedir(output_dir);

  ASSERT_EQ(1, ir != NULL);
  ir->nodes_capacity = 2;
  ir->nodes_count = 2;
  nodes = (cdd_ffi_ir_node_t *)calloc(2, sizeof(cdd_ffi_ir_node_t));
  ir->nodes = nodes;

  nodes[0].kind = CDD_FFI_NODE_STRUCT;
  nodes[0].name = strdup("A");
  nodes[0].fields_count = 1;
  nodes[0].fields = (cdd_ffi_field_t *)calloc(1, sizeof(cdd_ffi_field_t));
  nodes[0].fields[0].name = strdup("b");
  nodes[0].fields[0].type.kind = CDD_FFI_KIND_INT32;

  nodes[1].kind = CDD_FFI_NODE_FUNCTION;
  nodes[1].name = strdup("my_func");
  nodes[1].return_or_base_type.kind = CDD_FFI_KIND_VOID;

  ASSERT_EQ(0, cdd_ffi_emit_napi(ir, &config));

#if defined(_MSC_VER)
  fopen_s(&f, "test_napi_out\\test_lib_napi.c", "r");
#else
#if defined(_MSC_VER)
  if (fopen_s(&f, "test_napi_out/test_lib_napi.c", "r") != 0)
    f = NULL;
#else
  f = fopen("test_napi_out/test_lib_napi.c", "r");
#endif
#endif
  ASSERT_EQ(1, f != NULL);
  if (f)
    if (f)
      fclose(f);

  cdd_ffi_ir_free(ir);
  free(ir);
  PASS();
}

TEST test_ffi_ir_emit_java(void) {
  cdd_ffi_ir_t *ir = (cdd_ffi_ir_t *)calloc(1, sizeof(cdd_ffi_ir_t));
  cdd_ffi_ir_node_t *nodes;
  cdd_generate_bindings_config_t config = {0};
  char *output_dir = (char *)(size_t)(size_t) "test_java_out";
  FILE *f;

  config.target_langs = "java";
  config.output_dir = output_dir;
  config.library_name = "test_lib";
  config.generate_tests = 1;

  makedir(output_dir);

  ASSERT_EQ(1, ir != NULL);
  ir->nodes_capacity = 2;
  ir->nodes_count = 2;
  nodes = (cdd_ffi_ir_node_t *)calloc(2, sizeof(cdd_ffi_ir_node_t));
  ir->nodes = nodes;

  nodes[0].kind = CDD_FFI_NODE_STRUCT;
  nodes[0].name = strdup("A");
  nodes[0].fields_count = 1;
  nodes[0].fields = (cdd_ffi_field_t *)calloc(1, sizeof(cdd_ffi_field_t));
  nodes[0].fields[0].name = strdup("b");
  nodes[0].fields[0].type.kind = CDD_FFI_KIND_INT32;
  nodes[0].base_classes_count = 2;
  nodes[0].base_classes =
      (cdd_ffi_base_class_t *)calloc(2, sizeof(cdd_ffi_base_class_t));
  nodes[0].base_classes[0].name = strdup("Base1");
  nodes[0].base_classes[1].name = strdup("Base2");

  nodes[1].kind = CDD_FFI_NODE_FUNCTION;
  nodes[1].name = strdup("my_func");
  nodes[1].return_or_base_type.kind = CDD_FFI_KIND_VOID;

  ASSERT_EQ(0, cdd_ffi_emit_java(ir, &config));

#if defined(_MSC_VER)
  fopen_s(&f, "test_java_out\\TestLib.java", "r");
#else
#if defined(_MSC_VER)
  if (fopen_s(&f, "test_java_out/TestLib.java", "r") != 0)
    f = NULL;
#else
  f = fopen("test_java_out/TestLib.java", "r");
#endif
#endif
  ASSERT_EQ(1, f != NULL);
  if (f)
    if (f)
      fclose(f);

  cdd_ffi_ir_free(ir);
  free(ir);
  PASS();
}

TEST test_ffi_ir_emit_cpp(void) {
  cdd_ffi_ir_t *ir = (cdd_ffi_ir_t *)calloc(1, sizeof(cdd_ffi_ir_t));
  cdd_ffi_ir_node_t *nodes;
  cdd_generate_bindings_config_t config = {0};
  char *output_dir = (char *)(size_t)(size_t) "test_cpp_out";
  FILE *f;

  config.target_langs = "cpp";
  config.output_dir = output_dir;
  config.library_name = "test_lib";
  config.generate_tests = 1;

  makedir(output_dir);

  ASSERT_EQ(1, ir != NULL);
  ir->nodes_capacity = 2;
  ir->nodes_count = 2;
  nodes = (cdd_ffi_ir_node_t *)calloc(2, sizeof(cdd_ffi_ir_node_t));
  ir->nodes = nodes;

  nodes[0].kind = CDD_FFI_NODE_STRUCT;
  nodes[0].name = strdup("A");
  nodes[0].fields_count = 1;
  nodes[0].fields = (cdd_ffi_field_t *)calloc(1, sizeof(cdd_ffi_field_t));
  nodes[0].fields[0].name = strdup("b");
  nodes[0].fields[0].type.kind = CDD_FFI_KIND_INT32;

  nodes[1].kind = CDD_FFI_NODE_FUNCTION;
  nodes[1].name = strdup("my_func");
  nodes[1].return_or_base_type.kind = CDD_FFI_KIND_VOID;

  ASSERT_EQ(0, cdd_ffi_emit_cpp(ir, &config));

#if defined(_MSC_VER)
  fopen_s(&f, "test_cpp_out\\test_lib.hpp", "r");
#else
#if defined(_MSC_VER)
  if (fopen_s(&f, "test_cpp_out/test_lib.hpp", "r") != 0)
    f = NULL;
#else
  f = fopen("test_cpp_out/test_lib.hpp", "r");
#endif
#endif
  ASSERT_EQ(1, f != NULL);
  if (f)
    if (f)
      fclose(f);

  cdd_ffi_ir_free(ir);
  free(ir);
  PASS();
}

TEST test_ffi_ir_emit_go(void) {
  cdd_ffi_ir_t *ir = (cdd_ffi_ir_t *)calloc(1, sizeof(cdd_ffi_ir_t));
  cdd_ffi_ir_node_t *nodes;
  cdd_generate_bindings_config_t config = {0};
  char *output_dir = (char *)(size_t)(size_t) "test_go_out";
  FILE *f;

  config.target_langs = "go";
  config.output_dir = output_dir;
  config.library_name = "test_lib";
  config.generate_tests = 1;

  makedir(output_dir);

  ASSERT_EQ(1, ir != NULL);
  ir->nodes_capacity = 2;
  ir->nodes_count = 2;
  nodes = (cdd_ffi_ir_node_t *)calloc(2, sizeof(cdd_ffi_ir_node_t));
  ir->nodes = nodes;

  nodes[0].kind = CDD_FFI_NODE_STRUCT;
  nodes[0].name = strdup("A");
  nodes[0].fields_count = 1;
  nodes[0].fields = (cdd_ffi_field_t *)calloc(1, sizeof(cdd_ffi_field_t));
  nodes[0].fields[0].name = strdup("b");
  nodes[0].fields[0].type.kind = CDD_FFI_KIND_INT32;

  nodes[1].kind = CDD_FFI_NODE_FUNCTION;
  nodes[1].name = strdup("my_func");
  nodes[1].return_or_base_type.kind = CDD_FFI_KIND_VOID;

  ASSERT_EQ(0, cdd_ffi_emit_go(ir, &config));

#if defined(_MSC_VER)
  fopen_s(&f, "test_go_out\\test_lib.go", "r");
#else
#if defined(_MSC_VER)
  if (fopen_s(&f, "test_go_out/test_lib.go", "r") != 0)
    f = NULL;
#else
  f = fopen("test_go_out/test_lib.go", "r");
#endif
#endif
  ASSERT_EQ(1, f != NULL);
  if (f)
    if (f)
      fclose(f);

  cdd_ffi_ir_free(ir);
  free(ir);
  PASS();
}

TEST test_ffi_ir_emit_swift(void) {
  cdd_ffi_ir_t *ir = (cdd_ffi_ir_t *)calloc(1, sizeof(cdd_ffi_ir_t));
  cdd_ffi_ir_node_t *nodes;
  cdd_generate_bindings_config_t config = {0};
  char *output_dir = (char *)(size_t)(size_t) "test_swift_out";
  FILE *f;

  config.target_langs = "swift";
  config.output_dir = output_dir;
  config.library_name = "test_lib";
  config.generate_tests = 1;

  makedir(output_dir);

  ASSERT_EQ(1, ir != NULL);
  ir->nodes_capacity = 2;
  ir->nodes_count = 2;
  nodes = (cdd_ffi_ir_node_t *)calloc(2, sizeof(cdd_ffi_ir_node_t));
  ir->nodes = nodes;

  nodes[0].kind = CDD_FFI_NODE_STRUCT;
  nodes[0].name = strdup("A");
  nodes[0].fields_count = 1;
  nodes[0].fields = (cdd_ffi_field_t *)calloc(1, sizeof(cdd_ffi_field_t));
  nodes[0].fields[0].name = strdup("b");
  nodes[0].fields[0].type.kind = CDD_FFI_KIND_INT32;

  nodes[1].kind = CDD_FFI_NODE_FUNCTION;
  nodes[1].name = strdup("my_func");
  nodes[1].return_or_base_type.kind = CDD_FFI_KIND_VOID;

  ASSERT_EQ(0, cdd_ffi_emit_swift(ir, &config));

#if defined(_MSC_VER)
  fopen_s(&f, "test_swift_out\\test_lib.swift", "r");
#else
#if defined(_MSC_VER)
  if (fopen_s(&f, "test_swift_out/test_lib.swift", "r") != 0)
    f = NULL;
#else
  f = fopen("test_swift_out/test_lib.swift", "r");
#endif
#endif
  ASSERT_EQ(1, f != NULL);
  if (f)
    if (f)
      fclose(f);

  cdd_ffi_ir_free(ir);
  free(ir);
  PASS();
}

TEST test_ffi_ir_emit_dart(void) {
  cdd_ffi_ir_t *ir = (cdd_ffi_ir_t *)calloc(1, sizeof(cdd_ffi_ir_t));
  cdd_ffi_ir_node_t *nodes;
  cdd_generate_bindings_config_t config = {0};
  char *output_dir = (char *)(size_t)(size_t) "test_dart_out";
  FILE *f;

  config.target_langs = "dart";
  config.output_dir = output_dir;
  config.library_name = "test_lib";
  config.generate_tests = 1;

  makedir(output_dir);

  ASSERT_EQ(1, ir != NULL);
  ir->nodes_capacity = 2;
  ir->nodes_count = 2;
  nodes = (cdd_ffi_ir_node_t *)calloc(2, sizeof(cdd_ffi_ir_node_t));
  ir->nodes = nodes;

  nodes[0].kind = CDD_FFI_NODE_STRUCT;
  nodes[0].name = strdup("A");
  nodes[0].fields_count = 1;
  nodes[0].fields = (cdd_ffi_field_t *)calloc(1, sizeof(cdd_ffi_field_t));
  nodes[0].fields[0].name = strdup("b");
  nodes[0].fields[0].type.kind = CDD_FFI_KIND_INT32;

  nodes[1].kind = CDD_FFI_NODE_FUNCTION;
  nodes[1].name = strdup("my_func");
  nodes[1].return_or_base_type.kind = CDD_FFI_KIND_VOID;

  ASSERT_EQ(0, cdd_ffi_emit_dart(ir, &config));

#if defined(_MSC_VER)
  fopen_s(&f, "test_dart_out\\test_lib.dart", "r");
#else
#if defined(_MSC_VER)
  if (fopen_s(&f, "test_dart_out/test_lib.dart", "r") != 0)
    f = NULL;
#else
  f = fopen("test_dart_out/test_lib.dart", "r");
#endif
#endif
  ASSERT_EQ(1, f != NULL);
  if (f)
    if (f)
      fclose(f);

  cdd_ffi_ir_free(ir);
  free(ir);
  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* TEST_FFI_EXTRACTOR_EMIT_H */
