#ifndef TEST_FFI_EXTRACTOR_EMIT_EXTRA_H
#define TEST_FFI_EXTRACTOR_EMIT_EXTRA_H

/* clang-format off */
#include "../cdd_test_helpers/cdd_helpers.h"
#include "../../functions/ffi/cdd_ffi_ir_extractor.h"
#include "../../functions/ffi/cdd_ffi_emit_ruby.h"
#include "../../functions/ffi/cdd_ffi_emit_kotlin.h"
#include "../../functions/ffi/cdd_ffi_emit_php.h"
#include "../../functions/ffi/cdd_ffi_emit_lua.h"
#include "../../functions/ffi/cdd_ffi_emit_zig.h"
#include "../../functions/ffi/cdd_ffi_emit_odin.h"
#include "../../functions/ffi/cdd_ffi_emit_julia.h"
#include "../../functions/ffi/cdd_ffi_emit_r.h"
#include "../../functions/ffi/cdd_ffi_emit_matlab.h"
#include "../../functions/ffi/cdd_ffi_emit_haskell.h"
#include "../../functions/ffi/cdd_ffi_emit_ocaml.h"
#include "c_cdd/format_specifiers.h"
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

TEST test_ffi_ir_emit_ruby(void) {
  cdd_ffi_ir_t *ir = (cdd_ffi_ir_t *)calloc(1, sizeof(cdd_ffi_ir_t));
  cdd_ffi_ir_node_t *nodes;
  cdd_generate_bindings_config_t config = {0};
  char *output_dir = (char *)(size_t)(size_t) "test_ruby_out";
  FILE *f;

  config.target_langs = "ruby";
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

  ASSERT_EQ(0, cdd_ffi_emit_ruby(ir, &config));

#if defined(_MSC_VER)
  fopen_s(&f, "test_ruby_out\\test_lib.rb", "r");
#else
#if defined(_MSC_VER)
  if (fopen_s(&f, "test_ruby_out/test_lib.rb", "r") != 0)
    f = NULL;
#else
  f = fopen("test_ruby_out/test_lib.rb", "r");
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

TEST test_ffi_ir_emit_kotlin(void) {
  cdd_ffi_ir_t *ir = (cdd_ffi_ir_t *)calloc(1, sizeof(cdd_ffi_ir_t));
  cdd_ffi_ir_node_t *nodes;
  cdd_generate_bindings_config_t config = {0};
  char *output_dir = (char *)(size_t)(size_t) "test_kotlin_out";
  FILE *f;

  config.target_langs = "kotlin";
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

  ASSERT_EQ(0, cdd_ffi_emit_kotlin(ir, &config));

#if defined(_MSC_VER)
  fopen_s(&f, "test_kotlin_out\\test_lib.def", "r");
#else
#if defined(_MSC_VER)
  if (fopen_s(&f, "test_kotlin_out/test_lib.def", "r") != 0)
    f = NULL;
#else
  f = fopen("test_kotlin_out/test_lib.def", "r");
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

TEST test_ffi_ir_emit_php(void) {
  cdd_ffi_ir_t *ir = (cdd_ffi_ir_t *)calloc(1, sizeof(cdd_ffi_ir_t));
  cdd_ffi_ir_node_t *nodes;
  cdd_generate_bindings_config_t config = {0};
  char *output_dir = (char *)(size_t)(size_t) "test_php_out";
  FILE *f;

  config.target_langs = "php";
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

  ASSERT_EQ(0, cdd_ffi_emit_php(ir, &config));

#if defined(_MSC_VER)
  fopen_s(&f, "test_php_out\\TestLib.php", "r");
#else
#if defined(_MSC_VER)
  if (fopen_s(&f, "test_php_out/TestLib.php", "r") != 0)
    f = NULL;
#else
  f = fopen("test_php_out/TestLib.php", "r");
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

TEST test_ffi_ir_emit_lua(void) {
  cdd_ffi_ir_t *ir = (cdd_ffi_ir_t *)calloc(1, sizeof(cdd_ffi_ir_t));
  cdd_ffi_ir_node_t *nodes;
  cdd_generate_bindings_config_t config = {0};
  char *output_dir = (char *)(size_t)(size_t) "test_lua_out";
  FILE *f;

  config.target_langs = "lua";
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

  ASSERT_EQ(0, cdd_ffi_emit_lua(ir, &config));

#if defined(_MSC_VER)
  fopen_s(&f, "test_lua_out\\test_lib.lua", "r");
#else
#if defined(_MSC_VER)
  if (fopen_s(&f, "test_lua_out/test_lib.lua", "r") != 0)
    f = NULL;
#else
  f = fopen("test_lua_out/test_lib.lua", "r");
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

TEST test_ffi_ir_emit_zig(void) {
  cdd_ffi_ir_t *ir = (cdd_ffi_ir_t *)calloc(1, sizeof(cdd_ffi_ir_t));
  cdd_ffi_ir_node_t *nodes;
  cdd_generate_bindings_config_t config = {0};
  char *output_dir = (char *)(size_t)(size_t) "test_zig_out";
  FILE *f;

  config.target_langs = "zig";
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

  ASSERT_EQ(0, cdd_ffi_emit_zig(ir, &config));

#if defined(_MSC_VER)
  fopen_s(&f, "test_zig_out\\test_lib.zig", "r");
#else
#if defined(_MSC_VER)
  if (fopen_s(&f, "test_zig_out/test_lib.zig", "r") != 0)
    f = NULL;
#else
  f = fopen("test_zig_out/test_lib.zig", "r");
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

TEST test_ffi_ir_emit_odin(void) {
  cdd_ffi_ir_t *ir = (cdd_ffi_ir_t *)calloc(1, sizeof(cdd_ffi_ir_t));
  cdd_ffi_ir_node_t *nodes;
  cdd_generate_bindings_config_t config = {0};
  char *output_dir = (char *)(size_t)(size_t) "test_odin_out";
  FILE *f;

  config.target_langs = "odin";
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
  nodes[1].fields_count = 1;
  nodes[1].fields = (cdd_ffi_field_t *)calloc(1, sizeof(cdd_ffi_field_t));
  nodes[1].fields[0].name = strdup("context");
  nodes[1].fields[0].type.kind = CDD_FFI_KIND_INT32;

  ASSERT_EQ(0, cdd_ffi_emit_odin(ir, &config));

#if defined(_MSC_VER)
  fopen_s(&f, "test_odin_out\\test_lib.odin", "r");
#else
#if defined(_MSC_VER)
  if (fopen_s(&f, "test_odin_out/test_lib.odin", "r") != 0)
    f = NULL;
#else
  f = fopen("test_odin_out/test_lib.odin", "r");
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

TEST test_ffi_ir_emit_julia(void) {
  cdd_ffi_ir_t *ir = (cdd_ffi_ir_t *)calloc(1, sizeof(cdd_ffi_ir_t));
  cdd_ffi_ir_node_t *nodes;
  cdd_generate_bindings_config_t config = {0};
  char *output_dir = (char *)(size_t)(size_t) "test_julia_out";
  FILE *f;

  config.target_langs = "julia";
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

  ASSERT_EQ(0, cdd_ffi_emit_julia(ir, &config));

#if defined(_MSC_VER)
  fopen_s(&f, "test_julia_out\\TestLib.jl", "r");
#else
#if defined(_MSC_VER)
  if (fopen_s(&f, "test_julia_out/TestLib.jl", "r") != 0)
    f = NULL;
#else
  f = fopen("test_julia_out/TestLib.jl", "r");
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

TEST test_ffi_ir_emit_r(void) {
  cdd_ffi_ir_t *ir = (cdd_ffi_ir_t *)calloc(1, sizeof(cdd_ffi_ir_t));
  cdd_ffi_ir_node_t *nodes;
  cdd_generate_bindings_config_t config = {0};
  char *output_dir = (char *)(size_t)(size_t) "test_r_out";
  FILE *f;

  config.target_langs = "r";
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

  ASSERT_EQ(0, cdd_ffi_emit_r(ir, &config));

#if defined(_MSC_VER)
  fopen_s(&f, "test_r_out\\test_lib.R", "r");
#else
#if defined(_MSC_VER)
  if (fopen_s(&f, "test_r_out/test_lib.R", "r") != 0)
    f = NULL;
#else
  f = fopen("test_r_out/test_lib.R", "r");
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

TEST test_ffi_ir_emit_matlab(void) {
  cdd_ffi_ir_t *ir = (cdd_ffi_ir_t *)calloc(1, sizeof(cdd_ffi_ir_t));
  cdd_ffi_ir_node_t *nodes;
  cdd_generate_bindings_config_t config = {0};
  char *output_dir = (char *)(size_t)(size_t) "test_matlab_out";
  FILE *f;

  config.target_langs = "matlab";
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

  ASSERT_EQ(0, cdd_ffi_emit_matlab(ir, &config));

#if defined(_MSC_VER)
  fopen_s(&f, "test_matlab_out\\test_lib.m", "r");
#else
#if defined(_MSC_VER)
  if (fopen_s(&f, "test_matlab_out/test_lib.m", "r") != 0)
    f = NULL;
#else
  f = fopen("test_matlab_out/test_lib.m", "r");
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

TEST test_ffi_ir_emit_haskell(void) {
  cdd_ffi_ir_t *ir = (cdd_ffi_ir_t *)calloc(1, sizeof(cdd_ffi_ir_t));
  cdd_ffi_ir_node_t *nodes;
  cdd_generate_bindings_config_t config = {0};
  char *output_dir = (char *)(size_t)(size_t) "test_haskell_out";
  FILE *f;

  config.target_langs = "haskell";
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

  ASSERT_EQ(0, cdd_ffi_emit_haskell(ir, &config));

#if defined(_MSC_VER)
  fopen_s(&f, "test_haskell_out\\TestLib.hs", "r");
#else
#if defined(_MSC_VER)
  if (fopen_s(&f, "test_haskell_out/TestLib.hs", "r") != 0)
    f = NULL;
#else
  f = fopen("test_haskell_out/TestLib.hs", "r");
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

TEST test_ffi_ir_emit_ocaml(void) {
  cdd_ffi_ir_t *ir = (cdd_ffi_ir_t *)calloc(1, sizeof(cdd_ffi_ir_t));
  cdd_ffi_ir_node_t *nodes;
  cdd_generate_bindings_config_t config = {0};
  char *output_dir = (char *)(size_t)(size_t) "test_ocaml_out";
  FILE *f;

  config.target_langs = "ocaml";
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

  ASSERT_EQ(0, cdd_ffi_emit_ocaml(ir, &config));

#if defined(_MSC_VER)
  fopen_s(&f, "test_ocaml_out\\test_lib.ml", "r");
#else
#if defined(_MSC_VER)
  if (fopen_s(&f, "test_ocaml_out/test_lib.ml", "r") != 0)
    f = NULL;
#else
  f = fopen("test_ocaml_out/test_lib.ml", "r");
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
#endif /* TEST_FFI_EXTRACTOR_EMIT_EXTRA_H */
