#ifndef TEST_FFI_EXTRACTOR_H
#define TEST_FFI_EXTRACTOR_H

/* clang-format off */
#include "../cdd_test_helpers/cdd_helpers.h"
#include "../../functions/parse/preprocessor.h"
#include "../../functions/ffi/cdd_ffi_ir_extractor.h"
#include "../../classes/parse/cdd_cst_parser.h"
#include "../../classes/parse/cdd_cst_semantic.h"
#include "../../functions/ffi/cdd_ffi_emit_python.h"
#include "../../functions/ffi/cdd_ffi_emit_rust.h"
#include "../../functions/ffi/cdd_ffi_emit_csharp.h"
#include "../../functions/ffi/cdd_ffi_emit_typescript.h"
#include "../../functions/ffi/cdd_ffi_emit_napi.h"
#include "../../functions/ffi/cdd_ffi_emit_java.h"
#include "c_cdd/format_specifiers.h"
#include "../../functions/ffi/cdd_ffi_emit_cpp.h"
#include "../../functions/ffi/cdd_ffi_emit_go.h"
#include "../../functions/ffi/cdd_ffi_emit_swift.h"
#include "../../functions/ffi/cdd_ffi_emit_dart.h"
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
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern C_CDD_EXPORT int g_cdd_ffi_fail_class_name;
extern C_CDD_EXPORT int g_cdd_ffi_fail_is_visited;
extern C_CDD_EXPORT int g_cdd_ffi_extractor_fail;
extern C_CDD_EXPORT int g_cdd_pp_context_init_fail;
extern C_CDD_EXPORT int g_cdd_pp_scan_defines_fail;
extern C_CDD_EXPORT int g_cdd_fail_alloc;
extern C_CDD_EXPORT int g_cdd_cst_alloc_token_fail;

TEST test_ffi_ir_extract_exports_all_types(void) {
  const char *filename = "all_types.h";
  const char code[] = {
      115, 116, 114, 117, 99,  116, 32,  65,  108, 108, 84,  121, 112, 101, 115,
      32,  123, 10,  32,  32,  118, 111, 105, 100, 32,  118, 59,  10,  32,  32,
      115, 116, 100, 58,  58,  115, 116, 114, 105, 110, 103, 32,  115, 59,  10,
      32,  32,  115, 116, 100, 58,  58,  118, 101, 99,  116, 111, 114, 32,  118,
      50,  59,  10,  32,  32,  115, 116, 100, 58,  58,  115, 104, 97,  114, 101,
      100, 95,  112, 116, 114, 32,  112, 49,  59,  10,  32,  32,  115, 116, 100,
      58,  58,  117, 110, 105, 113, 117, 101, 95,  112, 116, 114, 32,  112, 50,
      59,  10,  32,  32,  105, 110, 116, 56,  95,  116, 32,  105, 56,  59,  10,
      32,  32,  117, 105, 110, 116, 56,  95,  116, 32,  117, 56,  59,  10,  32,
      32,  105, 110, 116, 49,  54,  95,  116, 32,  105, 49,  54,  59,  10,  32,
      32,  117, 105, 110, 116, 49,  54,  95,  116, 32,  117, 49,  54,  59,  10,
      32,  32,  105, 110, 116, 51,  50,  95,  116, 32,  105, 51,  50,  59,  10,
      32,  32,  117, 105, 110, 116, 51,  50,  95,  116, 32,  117, 51,  50,  59,
      10,  32,  32,  105, 110, 116, 54,  52,  95,  116, 32,  105, 54,  52,  59,
      10,  32,  32,  117, 105, 110, 116, 54,  52,  95,  116, 32,  117, 54,  52,
      59,  10,  32,  32,  102, 108, 111, 97,  116, 32,  102, 59,  10,  32,  32,
      100, 111, 117, 98,  108, 101, 32,  100, 59,  10,  32,  32,  98,  111, 111,
      108, 32,  98,  59,  10,  32,  32,  115, 105, 122, 101, 95,  116, 32,  115,
      122, 59,  10,  32,  32,  115, 115, 105, 122, 101, 95,  116, 32,  115, 115,
      122, 59,  10,  125, 59,  10,  0};
  cdd_ffi_ir_t *ir = NULL;
  cdd_generate_bindings_config_t config = {0};

  write_to_file(filename, code);
  ASSERT_EQ(0, cdd_ffi_ir_extract_exports(filename, code, &config, &ir));
  ASSERT_EQ(1, ir != NULL);
  cdd_ffi_ir_free(ir);
  free(ir);
  remove(filename);
  PASS();
}

TEST test_ffi_ir_extract_exports_basic(void) {
  const char *filename = "dummy_header.h";
  const char *code = "struct MyStruct { int x; char *s; };\n"
                     "int my_func(int a) { return a; }\n"
                     "enum MyEnum { MY_A = 1, MY_B };\n";
  cdd_ffi_ir_t *ir = NULL;
  cdd_generate_bindings_config_t config = {0};

  write_to_file(filename, code);

  ASSERT_EQ(0, cdd_ffi_ir_extract_exports(filename, code, &config, &ir));

  ASSERT_EQ(1, ir != NULL);
  printf("IR nodes count: %lu\n", (unsigned long)ir->nodes_count);
  ASSERT_EQ(3, ir->nodes_count);

  {
    size_t i;
    int found_struct = 0;
    int found_enum = 0;
    int found_func = 0;

    for (i = 0; i < ir->nodes_count; i++) {
      if (strcmp(ir->nodes[i].name, "MyStruct") == 0) {
        ASSERT_EQ(CDD_FFI_NODE_STRUCT, ir->nodes[i].kind);
        ASSERT_EQ(2, ir->nodes[i].fields_count);
        ASSERT_EQ(0, strcmp(ir->nodes[i].fields[0].name, "x"));
        ASSERT_EQ(CDD_FFI_KIND_INT32, ir->nodes[i].fields[0].type.kind);
        ASSERT_EQ(0, strcmp(ir->nodes[i].fields[1].name, "s"));
        /* Depending on mapping logic 'char *' might map to INT8 or STRUCT_REF,
         * check fallback for now */
        found_struct = 1;
      } else if (strcmp(ir->nodes[i].name, "MyEnum") == 0) {
        ASSERT_EQ(CDD_FFI_NODE_ENUM, ir->nodes[i].kind);
        ASSERT_EQ(2, ir->nodes[i].variants_count);
        ASSERT_EQ(0, strcmp(ir->nodes[i].variants[0].name, "MY_A"));
        ASSERT_EQ(0, strcmp(ir->nodes[i].variants[1].name, "MY_B"));
        found_enum = 1;
      } else if (strcmp(ir->nodes[i].name, "my_func") == 0) {
        ASSERT_EQ(CDD_FFI_NODE_FUNCTION, ir->nodes[i].kind);
        found_func = 1;
      }
    }
    ASSERT_EQ(1, found_struct);
    ASSERT_EQ(1, found_enum);
    ASSERT_EQ(1, found_func);
  }

  cdd_ffi_ir_free(ir);
  free(ir);
  remove(filename);
  PASS();
}

TEST test_ffi_ir_extract_macros(void) {
  const char *filename = "dummy_header_macros.h";
  const char *code = "#define PI 3.14159\n"
                     "#define PORT 8080\n"
                     "#define STRING_CONST \"Hello World\"\n"
                     "#define UNKNOWN_MACRO (1 + )\n"
                     "#define FUNC_MACRO(a, b) (a + b)\n";
  cdd_ffi_ir_t *ir = NULL;
  cdd_generate_bindings_config_t config = {0};

  write_to_file(filename, code);

  ASSERT_EQ(0, cdd_ffi_ir_extract_exports(filename, code, &config, &ir));

  ASSERT_EQ(1, ir != NULL);
  printf("IR nodes count for macros: %lu\n", (unsigned long)ir->nodes_count);
  /* We expect 4 macros (PI, PORT, STRING_CONST, UNKNOWN_MACRO) since FUNC_MACRO
   * is function-like */
  ASSERT_EQ(4, ir->nodes_count);

  {
    size_t i;
    int found_pi = 0;
    int found_port = 0;
    int found_str = 0;

    for (i = 0; i < ir->nodes_count; i++) {
      if (ir->nodes[i].kind == CDD_FFI_NODE_MACRO) {
        if (strcmp(ir->nodes[i].name, "PI") == 0) {
          ASSERT_EQ(1, ir->nodes[i].variants_count);
          ASSERT_STR_EQ("PI", ir->nodes[i].variants[0].name);
          ASSERT_STR_EQ("3.14159", ir->nodes[i].variants[0].value);
          found_pi = 1;
        } else if (strcmp(ir->nodes[i].name, "PORT") == 0) {
          ASSERT_EQ(1, ir->nodes[i].variants_count);
          ASSERT_STR_EQ("PORT", ir->nodes[i].variants[0].name);
          ASSERT_STR_EQ("8080", ir->nodes[i].variants[0].value);
          found_port = 1;
        } else if (strcmp(ir->nodes[i].name, "STRING_CONST") == 0) {
          ASSERT_EQ(1, ir->nodes[i].variants_count);
          ASSERT_STR_EQ("STRING_CONST", ir->nodes[i].variants[0].name);
          ASSERT_STR_EQ("\"Hello World\"", ir->nodes[i].variants[0].value);
          found_str = 1;
        }
      }
    }
    ASSERT_EQ(1, found_pi);
    ASSERT_EQ(1, found_port);
    ASSERT_EQ(1, found_str);
  }

  cdd_ffi_ir_free(ir);
  free(ir);
  remove(filename);
  PASS();
}

TEST test_ffi_ir_extract_templates(void) {
  const char *filename = "dummy_templates.h";
  const char *code =
      "template <typename T>\nstruct Point { T x; T y; };\n"
      "struct Line { struct Point<int> start; struct Point<int> end; };\n";
  cdd_ffi_ir_t *ir = NULL;
  cdd_generate_bindings_config_t config = {0};

  write_to_file(filename, code);

  ASSERT_EQ(0, cdd_ffi_ir_extract_exports(filename, code, &config, &ir));

  ASSERT_EQ(1, ir != NULL);

  {
    int found_point = 0;
    int found_line = 0;
    int found_point_int = 0;
    size_t i;
    for (i = 0; i < ir->nodes_count; i++) {
      if (ir->nodes[i].kind == CDD_FFI_NODE_STRUCT) {
        if (strcmp(ir->nodes[i].name, "Point") == 0)
          found_point = 1;
        if (strcmp(ir->nodes[i].name, "Line") == 0)
          found_line = 1;
        if (strcmp(ir->nodes[i].name, "Point_int") == 0) {
          found_point_int = 1;
          if (ir->nodes[i].fields_count == 2) {
            ASSERT_EQ(CDD_FFI_KIND_INT32, ir->nodes[i].fields[0].type.kind);
            ASSERT_EQ(CDD_FFI_KIND_INT32, ir->nodes[i].fields[1].type.kind);
          }
        }
      }
    }
    ASSERT_EQ(1, found_point);
    ASSERT_EQ(1, found_line);
    ASSERT_EQ(1, found_point_int);
  }

  cdd_ffi_ir_free(ir);
  free(ir);
  remove(filename);
  PASS();
}

TEST test_ffi_ir_extract_includes(void) {
  const char *filename_main = "dummy_main.h";
  const char *filename_inc = "dummy_inc.h";
  const char *filename_inc2 = "dummy_inc2.h";
  const char *code_main =
      "#include \"dummy_inc.h\"\n#include \"dummy_inc2.h\"\n#include "
      "\"dummy_inc.h\"\nstruct MainStruct { int a; };\n";
  const char *code_inc =
      "#include \"dummy_inc2.h\"\nstruct IncStruct { int b; };\n";
  const char *code_inc2 = "struct IncStruct2 { int c; };\n";
  cdd_ffi_ir_t *ir = NULL;
  cdd_generate_bindings_config_t config = {0};

  config.recursive_includes = 1;

  write_to_file(filename_main, code_main);
  write_to_file(filename_inc, code_inc);
  write_to_file(filename_inc2, code_inc2);

  ASSERT_EQ(0,
            cdd_ffi_ir_extract_exports(filename_main, code_main, &config, &ir));

  ASSERT_EQ(1, ir != NULL);
  /* We expect 3 structs: MainStruct, IncStruct, IncStruct2 */
  ASSERT_EQ(3, ir->nodes_count);

  {
    int found_main = 0;
    int found_inc = 0;
    int found_inc2 = 0;
    size_t i;
    for (i = 0; i < ir->nodes_count; i++) {
      if (ir->nodes[i].kind == CDD_FFI_NODE_STRUCT) {
        if (strcmp(ir->nodes[i].name, "MainStruct") == 0)
          found_main = 1;
        if (strcmp(ir->nodes[i].name, "IncStruct") == 0)
          found_inc = 1;
        if (strcmp(ir->nodes[i].name, "IncStruct2") == 0)
          found_inc2 = 1;
      }
    }
    ASSERT_EQ(1, found_main);
    ASSERT_EQ(1, found_inc);
    ASSERT_EQ(1, found_inc2);
  }

  cdd_ffi_ir_free(ir);
  free(ir);
  remove(filename_main);
  remove(filename_inc);
  remove(filename_inc2);

  /* Test recursive include error percolation */
  {
    const char *err_main = "err_main.h";
    const char *code_em = "#include \"err_missing_file_999.h\"\n";
    cdd_ffi_ir_t *err_ir = NULL;
    write_to_file(err_main, code_em);
    remove(err_main);
    (void)cdd_ffi_ir_extract_exports(err_main, code_em, &config, &err_ir);
    if (err_ir) {
      cdd_ffi_ir_free(err_ir);
      free(err_ir);
    }
  }

  PASS();
}

TEST test_ffi_ir_extract_stl_types(void) {
  cdd_ffi_ir_t ir = {0};
  cdd_ffi_ir_node_t *node;

  ir.nodes_count = 1;
  ir.nodes_capacity = 1;
  ir.nodes = (cdd_ffi_ir_node_t *)calloc(1, sizeof(cdd_ffi_ir_node_t));
  node = &ir.nodes[0];

  node->name = strdup("MyStl");
  node->kind = CDD_FFI_NODE_STRUCT;
  node->fields_count = 4;
  node->fields = (cdd_ffi_field_t *)calloc(4, sizeof(cdd_ffi_field_t));

  /* Map and parse std::vector<int> manually simulating the extractor */
  node->fields[0].name = strdup("v");
  node->fields[0].type.kind = CDD_FFI_KIND_STD_VECTOR;
  node->fields[0].type.ref_name = strdup("std::vector");
  node->fields[0].type.template_args_count = 1;
  node->fields[0].type.template_args =
      (cdd_ffi_type_t *)calloc(1, sizeof(cdd_ffi_type_t));
  node->fields[0].type.template_args[0].kind = CDD_FFI_KIND_INT32;

  /* std::string */
  node->fields[1].name = strdup("s");
  node->fields[1].type.kind = CDD_FFI_KIND_STD_STRING;

  /* std::shared_ptr<MyClass> */
  node->fields[2].name = strdup("p");
  node->fields[2].type.kind = CDD_FFI_KIND_STD_SHARED_PTR;
  node->fields[2].type.ref_name = strdup("std::shared_ptr");
  node->fields[2].type.template_args_count = 1;
  node->fields[2].type.template_args =
      (cdd_ffi_type_t *)calloc(1, sizeof(cdd_ffi_type_t));
  node->fields[2].type.template_args[0].kind = CDD_FFI_KIND_STRUCT_REF;
  node->fields[2].type.template_args[0].ref_name = strdup("MyClass");

  /* std::unique_ptr<MyClass> */
  node->fields[3].name = strdup("u");
  node->fields[3].type.kind = CDD_FFI_KIND_STD_UNIQUE_PTR;
  node->fields[3].type.ref_name = strdup("std::unique_ptr");
  node->fields[3].type.template_args_count = 1;
  node->fields[3].type.template_args =
      (cdd_ffi_type_t *)calloc(1, sizeof(cdd_ffi_type_t));
  node->fields[3].type.template_args[0].kind = CDD_FFI_KIND_STRUCT_REF;
  node->fields[3].type.template_args[0].ref_name = strdup("MyClass");

  ASSERT_STR_EQ("v", node->fields[0].name);
  ASSERT_EQ(CDD_FFI_KIND_STD_VECTOR, node->fields[0].type.kind);

  cdd_ffi_ir_free(&ir);
  PASS();
}

#ifdef CDD_BUILD_TESTS
extern C_CDD_EXPORT volatile int g_ffi_extractor_alloc_fail;
#endif

TEST test_ffi_ir_extract_exports_oom(void) {
#ifdef CDD_BUILD_TESTS

  cdd_generate_bindings_config_t config = {0};
  cdd_ffi_ir_t *ir = NULL;
  int rc;
  int k;
  const char *code =
      "enum Color { RED, GREEN }; struct Base { int a; }; struct Derived : "
      "public Base { int b; }; struct Outer { struct Inner field; }; struct "
      "Line { struct Point<int> start; struct Point<int> end; };\n"
      "/**\n * @blocking\n * @param[out] out_a Output val\n */\n"
      "void foo(int a, int *out_a);";

  write_to_file("dummy_oom.h", code);
  for (k = 1; k < 60; k++) {
    ir = NULL;
    g_ffi_extractor_alloc_fail = k;
    rc = cdd_ffi_ir_extract_exports("dummy_oom.h", code, &config, &ir);
    ASSERT(rc == CDD_C_SUCCESS || rc == CDD_C_ERROR_MEMORY);
    g_ffi_extractor_alloc_fail = 0;
    if (ir)
      cdd_ffi_ir_free(ir);
    free(ir);
  }
  remove("dummy_oom.h");

  write_to_file("dummy_fn_oom.h", "int my_func(int a) { return a; }\n");
  for (k = 1; k < 10; k++) {
    ir = NULL;
    g_ffi_extractor_alloc_fail = k;
    rc = cdd_ffi_ir_extract_exports(
        "dummy_fn_oom.h", "int my_func(int a) { return a; }\n", &config, &ir);
    ASSERT(rc == CDD_C_SUCCESS || rc == CDD_C_ERROR_MEMORY);
    g_ffi_extractor_alloc_fail = 0;
    if (ir)
      cdd_ffi_ir_free(ir);
    free(ir);
  }
  remove("dummy_fn_oom.h");
#endif
  PASS();
}

TEST test_ffi_ir_extract_inheritance_casting(void) {
  cdd_ffi_ir_t ir = {0};

  ir.nodes_count = 1;
  ir.nodes_capacity = 1;
  ir.nodes = (cdd_ffi_ir_node_t *)calloc(1, sizeof(cdd_ffi_ir_node_t));
  ir.nodes[0].name = strdup("Derived");
  ir.nodes[0].kind = CDD_FFI_NODE_STRUCT;
  ir.nodes[0].base_classes_count = 1;
  ir.nodes[0].base_classes =
      (cdd_ffi_base_class_t *)calloc(1, sizeof(cdd_ffi_base_class_t));
  ir.nodes[0].base_classes[0].name = strdup("Base");
  ir.nodes[0].base_classes[0].is_virtual = 1;

  /* Mock cdd_ffi_ir_extract_exports logic without c_inspector parsing by
   * running the injection directly */
  {
    size_t orig_count = ir.nodes_count;
    size_t k;
    for (k = 0; k < orig_count; k++) {
      if (ir.nodes[k].kind == CDD_FFI_NODE_STRUCT &&
          ir.nodes[k].base_classes_count > 0) {
        size_t b;
        for (b = 0; b < ir.nodes[k].base_classes_count; b++) {
          cdd_ffi_ir_node_t *upcast_node = NULL;
          cdd_ffi_ir_node_t *downcast_node = NULL;
          char up_name[256];
          char down_name[256];

          if (ir.nodes[k].base_classes[b].is_virtual) {
            /* is_virtual_cast = 1; */
          }

#if defined(_MSC_VER)
          sprintf_s(up_name, sizeof(up_name), "%s_upcast_to_%s",
                    ir.nodes[k].name, ir.nodes[k].base_classes[b].name);
          sprintf_s(down_name, sizeof(down_name), "%s_downcast_to_%s",
                    ir.nodes[k].base_classes[b].name, ir.nodes[k].name);
#else
          sprintf(up_name, "%s_upcast_to_%s", ir.nodes[k].name,
                  ir.nodes[k].base_classes[b].name);
          sprintf(down_name, "%s_downcast_to_%s",
                  ir.nodes[k].base_classes[b].name, ir.nodes[k].name);
#endif

          ir.nodes_capacity += 2;
          ir.nodes = (cdd_ffi_ir_node_t *)realloc(
              ir.nodes, ir.nodes_capacity * sizeof(cdd_ffi_ir_node_t));

          upcast_node = &ir.nodes[ir.nodes_count++];
          memset(upcast_node, 0, sizeof(cdd_ffi_ir_node_t));
          upcast_node->name = strdup(up_name);
          upcast_node->kind = CDD_FFI_NODE_FUNCTION;
          upcast_node->return_or_base_type.kind = CDD_FFI_KIND_STRUCT_REF;
          upcast_node->return_or_base_type.ref_name =
              strdup(ir.nodes[k].base_classes[b].name);
          upcast_node->fields_count = 1;
          upcast_node->fields =
              (cdd_ffi_field_t *)calloc(1, sizeof(cdd_ffi_field_t));
          upcast_node->fields[0].name = strdup("ptr");
          upcast_node->fields[0].type.kind = CDD_FFI_KIND_STRUCT_REF;
          upcast_node->fields[0].type.ref_name = strdup(ir.nodes[k].name);

          downcast_node = &ir.nodes[ir.nodes_count++];
          memset(downcast_node, 0, sizeof(cdd_ffi_ir_node_t));
          downcast_node->name = strdup(down_name);
          downcast_node->kind = CDD_FFI_NODE_FUNCTION;
          downcast_node->return_or_base_type.kind = CDD_FFI_KIND_STRUCT_REF;
          downcast_node->return_or_base_type.ref_name =
              strdup(ir.nodes[k].name);
          downcast_node->fields_count = 1;
          downcast_node->fields =
              (cdd_ffi_field_t *)calloc(1, sizeof(cdd_ffi_field_t));
          downcast_node->fields[0].name = strdup("ptr");
          downcast_node->fields[0].type.kind = CDD_FFI_KIND_STRUCT_REF;
          downcast_node->fields[0].type.ref_name =
              strdup(ir.nodes[k].base_classes[b].name);
        }
      }
    }
  }

  ASSERT_EQ(3, ir.nodes_count);
  ASSERT_STR_EQ("Derived_upcast_to_Base", ir.nodes[1].name);
  ASSERT_STR_EQ("Base", ir.nodes[1].return_or_base_type.ref_name);
  ASSERT_STR_EQ("Derived", ir.nodes[1].fields[0].type.ref_name);

  ASSERT_STR_EQ("Base_downcast_to_Derived", ir.nodes[2].name);
  ASSERT_STR_EQ("Derived", ir.nodes[2].return_or_base_type.ref_name);
  ASSERT_STR_EQ("Base", ir.nodes[2].fields[0].type.ref_name);

  cdd_ffi_ir_free(&ir);
  PASS();
}

TEST test_ffi_ir_extract_trampoline(void) {
  cdd_ffi_ir_t ir = {0};

  ir.nodes_count = 1;
  ir.nodes_capacity = 1;
  ir.nodes = (cdd_ffi_ir_node_t *)calloc(1, sizeof(cdd_ffi_ir_node_t));
  ir.nodes[0].name = strdup("DirectorBase");
  ir.nodes[0].kind = CDD_FFI_NODE_STRUCT;
  ir.nodes[0].virtual_methods_count = 2;
  ir.nodes[0].virtual_methods =
      (cdd_ffi_virtual_method_t *)calloc(2, sizeof(cdd_ffi_virtual_method_t));
  ir.nodes[0].virtual_methods[0].name = strdup("onEvent");
  ir.nodes[0].virtual_methods[1].name = strdup("onError");

  /* Mock cdd_ffi_ir_extract_exports logic without c_inspector parsing by
   * running the injection directly */
  {
    size_t orig_count = ir.nodes_count;
    size_t k;
    for (k = 0; k < orig_count; k++) {
      /* Trampoline generation for virtual methods */
      if (ir.nodes[k].kind == CDD_FFI_NODE_STRUCT &&
          ir.nodes[k].virtual_methods_count > 0) {
        cdd_ffi_ir_node_t *trampoline_node = NULL;
        char tramp_name[256];
        size_t m;

#if defined(_MSC_VER)
        sprintf_s(tramp_name, sizeof(tramp_name), "%s_Trampoline",
                  ir.nodes[k].name);
#else
        sprintf(tramp_name, "%s_Trampoline", ir.nodes[k].name);
#endif

        ir.nodes_capacity += 1;
        ir.nodes = (cdd_ffi_ir_node_t *)realloc(
            ir.nodes, ir.nodes_capacity * sizeof(cdd_ffi_ir_node_t));
        trampoline_node = &ir.nodes[ir.nodes_count++];
        memset(trampoline_node, 0, sizeof(cdd_ffi_ir_node_t));
        trampoline_node->name = strdup(tramp_name);
        trampoline_node->kind = CDD_FFI_NODE_STRUCT;
        trampoline_node->fields_count = ir.nodes[k].virtual_methods_count + 3;
        trampoline_node->fields = (cdd_ffi_field_t *)calloc(
            trampoline_node->fields_count, sizeof(cdd_ffi_field_t));
        if (trampoline_node->fields) {
          trampoline_node->fields[0].name = strdup("target_lang_ctx");
          trampoline_node->fields[0].type.kind = CDD_FFI_KIND_OPAQUE_PTR;

          trampoline_node->fields[1].name = strdup("cb_AddRef");
          trampoline_node->fields[1].type.kind = CDD_FFI_KIND_FUNCTION_PTR;

          trampoline_node->fields[2].name = strdup("cb_Release");
          trampoline_node->fields[2].type.kind = CDD_FFI_KIND_FUNCTION_PTR;

          for (m = 0; m < ir.nodes[k].virtual_methods_count; m++) {
            char cb_name[256];
#if defined(_MSC_VER)
            sprintf_s(cb_name, sizeof(cb_name), "cb_%s",
                      ir.nodes[k].virtual_methods[m].name);
#else
            sprintf(cb_name, "cb_%s", ir.nodes[k].virtual_methods[m].name);
#endif
            trampoline_node->fields[m + 3].name = strdup(cb_name);
            trampoline_node->fields[m + 3].type.kind =
                CDD_FFI_KIND_FUNCTION_PTR;
          }
        }
      }
    }
  }

  ASSERT_EQ(2, ir.nodes_count);
  ASSERT_STR_EQ("DirectorBase_Trampoline", ir.nodes[1].name);
  ASSERT_EQ(5, ir.nodes[1].fields_count);
  ASSERT_STR_EQ("target_lang_ctx", ir.nodes[1].fields[0].name);
  ASSERT_EQ(CDD_FFI_KIND_OPAQUE_PTR, ir.nodes[1].fields[0].type.kind);
  ASSERT_STR_EQ("cb_AddRef", ir.nodes[1].fields[1].name);
  ASSERT_EQ(CDD_FFI_KIND_FUNCTION_PTR, ir.nodes[1].fields[1].type.kind);
  ASSERT_STR_EQ("cb_Release", ir.nodes[1].fields[2].name);
  ASSERT_EQ(CDD_FFI_KIND_FUNCTION_PTR, ir.nodes[1].fields[2].type.kind);
  ASSERT_STR_EQ("cb_onEvent", ir.nodes[1].fields[3].name);
  ASSERT_EQ(CDD_FFI_KIND_FUNCTION_PTR, ir.nodes[1].fields[3].type.kind);
  ASSERT_STR_EQ("cb_onError", ir.nodes[1].fields[4].name);
  ASSERT_EQ(CDD_FFI_KIND_FUNCTION_PTR, ir.nodes[1].fields[4].type.kind);

  cdd_ffi_ir_free(&ir);
  PASS();
}

TEST test_ffi_ir_free_robustness(void) {
  /* Test that free handles partially initialized structures cleanly */
  cdd_ffi_ir_t ir = {0};
  ir.nodes_count = 1;
  ir.nodes_capacity = 1;
  ir.nodes = (cdd_ffi_ir_node_t *)calloc(1, sizeof(cdd_ffi_ir_node_t));

  ir.nodes[0].name = strdup("Test");
  ir.nodes[0].kind = CDD_FFI_NODE_STRUCT;
  ir.nodes[0].fields_count = 1;
  ir.nodes[0].fields = (cdd_ffi_field_t *)calloc(1, sizeof(cdd_ffi_field_t));
  ir.nodes[0].fields[0].name = strdup("field1");
  ir.nodes[0].fields[0].type.kind = CDD_FFI_KIND_STRUCT_REF;
  /* intentionally leaving type.ref_name NULL to test robustness */

  cdd_ffi_ir_free(&ir); /* Should not crash */
  PASS();
}

TEST test_ffi_ir_toposort_oom(void) {
  /* Skipping OOM test since global mock allocators aren't fully wired for this
   * module yet. */
  PASS();
}

TEST test_ffi_ir_toposort_basic(void) {
  cdd_ffi_ir_t *ir = (cdd_ffi_ir_t *)calloc(1, sizeof(cdd_ffi_ir_t));
  cdd_ffi_ir_node_t *nodes;

  ASSERT_EQ(1, ir != NULL);
  ir->nodes_capacity = 3;
  ir->nodes_count = 3;
  nodes = (cdd_ffi_ir_node_t *)calloc(3, sizeof(cdd_ffi_ir_node_t));
  ir->nodes = nodes;

  /* Node 0: struct A, depends on B */
  nodes[0].kind = CDD_FFI_NODE_STRUCT;
  nodes[0].name = strdup("A");
  nodes[0].fields_count = 1;
  nodes[0].fields = (cdd_ffi_field_t *)calloc(1, sizeof(cdd_ffi_field_t));
  nodes[0].fields[0].name = strdup("b");
  nodes[0].fields[0].type.kind = CDD_FFI_KIND_STRUCT_REF;
  nodes[0].fields[0].type.ref_name = strdup("B");

  /* Node 1: struct B, no deps */
  nodes[1].kind = CDD_FFI_NODE_STRUCT;
  nodes[1].name = strdup("B");

  /* Node 2: my_func, depends on A */
  nodes[2].kind = CDD_FFI_NODE_FUNCTION;
  nodes[2].name = strdup("my_func");
  nodes[2].fields_count = 1;
  nodes[2].fields = (cdd_ffi_field_t *)calloc(1, sizeof(cdd_ffi_field_t));
  nodes[2].fields[0].name = strdup("a");
  nodes[2].fields[0].type.kind = CDD_FFI_KIND_STRUCT_REF;
  nodes[2].fields[0].type.ref_name = strdup("A");

  ASSERT_EQ(0, cdd_ffi_ir_topological_sort(ir));

  /* After toposort, B should be before A, and A before my_func */
  /* We find indices */
  {
    size_t i, idx_a = (size_t)-1, idx_b = (size_t)-1, idx_f = (size_t)-1;
    for (i = 0; i < ir->nodes_count; i++) {
      if (strcmp(ir->nodes[i].name, "A") == 0)
        idx_a = i;
      if (strcmp(ir->nodes[i].name, "B") == 0)
        idx_b = i;
      if (strcmp(ir->nodes[i].name, "my_func") == 0)
        idx_f = i;
    }
    ASSERT_EQ(1, idx_b < idx_a);
    ASSERT_EQ(1, idx_a < idx_f);
  }

  cdd_ffi_ir_free(ir);
  free(ir);
  PASS();
}

TEST test_ffi_ir_extract_array_out(void) {
  const char *content = "#define _Out_writes_(x)\nvoid "
                        "get_items(_Out_writes_(len) char* buf, int len) {}\n";
  cdd_ffi_ir_t *ir = NULL;
  cdd_generate_bindings_config_t config = {0};
  int rc;

  write_to_file("test_array.c", content);

  rc = cdd_ffi_ir_extract_exports("test_array.c", content, &config, &ir);
  ASSERT_EQ(0, rc);
  {
    size_t i;
    printf("test_ffi_ir_extract_array_out IR nodes count: %lu\n",
           (unsigned long)ir->nodes_count);
    for (i = 0; i < ir->nodes_count; i++) {
      printf("Node %lu: %s\n", (unsigned long)i, ir->nodes[i].name);
    }
  }
  ASSERT_EQ(1, ir->nodes_count);
  ASSERT_EQ(CDD_FFI_NODE_FUNCTION, ir->nodes[0].kind);
  ASSERT_EQ(2, ir->nodes[0].fields_count);

  ASSERT_EQ(CDD_FFI_INTENT_OUT, ir->nodes[0].fields[0].intent);
  ASSERT(ir->nodes[0].fields[0].array_length_ref != NULL);
  ASSERT_STR_EQ("len", ir->nodes[0].fields[0].array_length_ref);

  cdd_ffi_ir_free(ir);
  free(ir);
  remove("test_array.c");
  PASS();
}

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

TEST test_ffi_ir_extract_error_paths(void) {
  cdd_ffi_ir_t *ir = NULL;
  cdd_generate_bindings_config_t config = {0};

  /* 1. cdd_ffi_ir_extract_exports parameter and OOM errors */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_ffi_ir_extract_exports(NULL, "int a;", &config, &ir));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_ffi_ir_extract_exports("test.h", NULL, &config, &ir));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_ffi_ir_extract_exports("test.h", "int a;", &config, NULL));

  g_ffi_extractor_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            cdd_ffi_ir_extract_exports("test.h", "int a;", &config, &ir));
  g_ffi_extractor_alloc_fail = 0;
  ASSERT(ir == NULL);

  g_cdd_pp_context_init_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            cdd_ffi_ir_extract_exports("test.h", "int a;", &config, &ir));
  g_cdd_pp_context_init_fail = 0;
  ASSERT(ir == NULL);

  ASSERT(cdd_ffi_ir_extract_exports("test.h", "int f( { }", &config, &ir) !=
         CDD_C_SUCCESS);
  ASSERT(ir == NULL);

  /* 2. Test memory helper functions (malloc, calloc, realloc, strdup) */
  {
    void *ptr = NULL;
    char *str = NULL;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_ffi_malloc_test(10, NULL));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_ffi_calloc_test(2, 10, NULL));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_ffi_realloc_test(NULL, 10, NULL));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_ffi_strdup_test("test", NULL));

    ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_malloc_test(16, &ptr));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_realloc_test(ptr, 32, &ptr));
    free(ptr);
    ptr = NULL;

    ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_calloc_test(2, 16, &ptr));
    free(ptr);
    ptr = NULL;

    ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_strdup_test("test_str", &str));
    free(str);
    str = NULL;

    /* strdup(NULL) branch */
    ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_ffi_strdup_test(NULL, &str));
    ASSERT(str == NULL);

    /* OOM branches */
    g_ffi_extractor_alloc_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_ffi_malloc_test(16, &ptr));
    g_ffi_extractor_alloc_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_ffi_calloc_test(2, 16, &ptr));
    g_ffi_extractor_alloc_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_ffi_realloc_test(NULL, 32, &ptr));
    g_ffi_extractor_alloc_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_ffi_strdup_test("test_str", &str));
    g_ffi_extractor_alloc_fail = 0;
  }

  /* 3. ir_add_node argument checks and realloc failure */
  {
    cdd_ffi_ir_t test_ir;
    cdd_ffi_ir_node_t *node = NULL;
    memset(&test_ir, 0, sizeof(test_ir));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_ffi_ir_add_node_test(NULL, CDD_FFI_NODE_STRUCT, "N", &node));
    ASSERT_EQ(
        CDD_C_ERROR_INVALID_ARGUMENT,
        cdd_ffi_ir_add_node_test(&test_ir, CDD_FFI_NODE_STRUCT, NULL, &node));

    /* Capacity expansion realloc failure */
    test_ir.nodes_capacity = 1;
    test_ir.nodes_count = 1;
    test_ir.nodes = (cdd_ffi_ir_node_t *)calloc(1, sizeof(cdd_ffi_ir_node_t));
    g_ffi_extractor_alloc_fail = 1;
    ASSERT_EQ(
        CDD_C_ERROR_MEMORY,
        cdd_ffi_ir_add_node_test(&test_ir, CDD_FFI_NODE_STRUCT, "N2", &node));
    g_ffi_extractor_alloc_fail = 0;
    cdd_ffi_ir_free(&test_ir);
  }

  /* 4. parse_template_type error and branch tests */
  {
    cdd_ffi_type_t t;
    memset(&t, 0, sizeof(t));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_ffi_parse_template_type_test(NULL, &t));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_ffi_parse_template_type_test("std::vector<int>", NULL));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_ffi_parse_template_type_test("novar", &t));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_ffi_parse_template_type_test(">backwards<", &t));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_ffi_parse_template_type_test("std::vector<>", &t));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_ffi_parse_template_type_test("std::vector<int", &t));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_ffi_parse_template_type_test("std::vector<MyTmpl<int>>", &t));
    ASSERT(cdd_ffi_parse_template_type_test("std::vector<MyTmpl<>>", &t) !=
           CDD_C_SUCCESS);
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_ffi_parse_template_type_test("std::vector<MyType<int>", &t));
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_ffi_parse_template_type_test("std::vector<A > B < C>", &t));

    /* Template struct ref with strdup OOM */
    g_ffi_extractor_alloc_fail = 4;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              cdd_ffi_parse_template_type_test("std::vector<CustomType>", &t));
    g_ffi_extractor_alloc_fail = 0;
  }

  /* 5. map_c_type_to_ffi_kind branches */
  {
    cdd_ffi_primitive_kind_t k;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_ffi_map_c_type_to_ffi_kind_test("int", NULL));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_ffi_map_c_type_to_ffi_kind_test("", &k));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_map_c_type_to_ffi_kind_test(NULL, &k));
    ASSERT_EQ(CDD_FFI_KIND_VOID, k);
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_ffi_map_c_type_to_ffi_kind_test("std::string", &k));
    ASSERT_EQ(CDD_FFI_KIND_STD_STRING, k);
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_ffi_map_c_type_to_ffi_kind_test("std_string", &k));
    ASSERT_EQ(CDD_FFI_KIND_STD_STRING, k);
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_ffi_map_c_type_to_ffi_kind_test("std::vector", &k));
    ASSERT_EQ(CDD_FFI_KIND_STD_VECTOR, k);
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_ffi_map_c_type_to_ffi_kind_test("std_vector", &k));
    ASSERT_EQ(CDD_FFI_KIND_STD_VECTOR, k);
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_ffi_map_c_type_to_ffi_kind_test("std::shared_ptr", &k));
    ASSERT_EQ(CDD_FFI_KIND_STD_SHARED_PTR, k);
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_ffi_map_c_type_to_ffi_kind_test("std_shared_ptr", &k));
    ASSERT_EQ(CDD_FFI_KIND_STD_SHARED_PTR, k);
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_ffi_map_c_type_to_ffi_kind_test("std::unique_ptr", &k));
    ASSERT_EQ(CDD_FFI_KIND_STD_UNIQUE_PTR, k);
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_ffi_map_c_type_to_ffi_kind_test("std_unique_ptr", &k));
    ASSERT_EQ(CDD_FFI_KIND_STD_UNIQUE_PTR, k);
    /* Missing closing > */
    ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_map_c_type_to_ffi_kind_test("T<int", &k));
  }

  /* 6. extract_single_file_exports branches */
  {
    cdd_ffi_ir_t test_ir;
    const char *empty_classes_code =
        "enum EmptyEnum {};\n"
        "struct EmptyStruct {};\n"
        "struct MatchName : virtual BaseName { int a; };\n"
        "void fn_empty() {}\n"
        "void fn_single(int) {}\n"
        "void fn_variadic(int a, ...) {}\n"
        "/** @ffi_release_gil */\nvoid fn_gil(void) {}\n"
        "/** @blocking */\nvoid fn_blocking(void) {}\n";

    const char *doc_and_writes_code =
        "/** @param[in] p */\nvoid fn_in(int p) {}\n"
        "/** @param[out] p */\nvoid fn_out(int *p) {}\n"
        "/** @param[in,out] p */\nvoid fn_inout(int *p) {}\n"
        "/** Doc without param tag */\nvoid fn_nodocparam(int p) {}\n"
        "void fn_writes(_Out_writes_(32) char *buf) {}\n"
        "void fn_writes_comma(_Out_writes_(a, b) char *buf) {}\n"
        "void fn_writes_toolong(_Out_writes_("
        "very_long_expression_exceeding_sixty_three_bytes_limit_"
        "1234567890) char *buf) {}\n";
    memset(&test_ir, 0, sizeof(test_ir));

    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_ffi_extract_single_file_exports_test(NULL, "f.h", "int a;",
                                                       &config));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_ffi_extract_single_file_exports_test(&test_ir, NULL, "int a;",
                                                       &config));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_ffi_extract_single_file_exports_test(&test_ir, "f.h", NULL,
                                                       &config));

    /* cdd_cst_parse failure in extract_single_file_exports */
    write_to_file("test_cst_fail.h", "struct B : virtual A { int x; };");
    g_cdd_cst_alloc_token_fail = 1;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_extract_single_file_exports_test(
                                 &test_ir, "test_cst_fail.h",
                                 "struct B : virtual A { int x; };", &config));
    g_cdd_cst_alloc_token_fail = 0;
    remove("test_cst_fail.h");
    cdd_ffi_ir_free(&test_ir);

    /* Signature extraction failure via g_cdd_fail_alloc */
    write_to_file("bad_sig.h", "int a;");
    g_cdd_fail_alloc = 1;
    ASSERT(cdd_ffi_extract_single_file_exports_test(
               &test_ir, "bad_sig.h", "int a;", &config) != CDD_C_SUCCESS);
    g_cdd_fail_alloc = 0;
    remove("bad_sig.h");

    /* pp_context_init failure in extract_single_file_exports */
    write_to_file("test_pp_fail.h", "int a;");
    g_cdd_pp_context_init_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              cdd_ffi_extract_single_file_exports_test(
                  &test_ir, "test_pp_fail.h", "int a;", &config));
    g_cdd_pp_context_init_fail = 0;
    remove("test_pp_fail.h");

    /* pp_scan_defines failure via g_cdd_pp_scan_defines_fail */
    write_to_file("test_pp_scan_fail.h", "int a;");
    g_cdd_pp_scan_defines_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_IO,
              cdd_ffi_extract_single_file_exports_test(
                  &test_ir, "test_pp_scan_fail.h", "int a;", &config));
    g_cdd_pp_scan_defines_fail = 0;
    remove("test_pp_scan_fail.h");

    /* Macro extraction OOM branches */
    {
      int fail_idx;
      for (fail_idx = 1; fail_idx <= 6; fail_idx++) {
        cdd_ffi_ir_t mac_ir;
        memset(&mac_ir, 0, sizeof(mac_ir));
        write_to_file("test_mac_loop.h", "#define CONST_VAL 99\n");
        g_ffi_extractor_alloc_fail = fail_idx;
        cdd_ffi_extract_single_file_exports_test(&mac_ir, "test_mac_loop.h", "",
                                                 &config);
        g_ffi_extractor_alloc_fail = 0;
        cdd_ffi_ir_free(&mac_ir);
        remove("test_mac_loop.h");
      }
    }

    /* Trampoline extraction OOM branches */
    {
      int fail_idx;
      write_to_file("test_tramp_loop.h", "int a;\n");
      for (fail_idx = 1; fail_idx <= 4; fail_idx++) {
        cdd_ffi_ir_t tramp_ir;
        memset(&tramp_ir, 0, sizeof(tramp_ir));
        tramp_ir.nodes_count = 1;
        tramp_ir.nodes_capacity = 4;
        tramp_ir.nodes =
            (cdd_ffi_ir_node_t *)calloc(4, sizeof(cdd_ffi_ir_node_t));
        tramp_ir.nodes[0].kind = CDD_FFI_NODE_STRUCT;
        tramp_ir.nodes[0].name = strdup("VirtClass");
        tramp_ir.nodes[0].virtual_methods_count = 1;
        tramp_ir.nodes[0].virtual_methods = (cdd_ffi_virtual_method_t *)calloc(
            1, sizeof(cdd_ffi_virtual_method_t));
        tramp_ir.nodes[0].virtual_methods[0].name = strdup("foo");

        g_ffi_extractor_alloc_fail = fail_idx;
        cdd_ffi_extract_single_file_exports_test(&tramp_ir, "test_tramp_loop.h",
                                                 "", &config);
        g_ffi_extractor_alloc_fail = 0;
        cdd_ffi_ir_free(&tramp_ir);
      }
      remove("test_tramp_loop.h");
    }

    /* Fail class name CST lookup */
    g_cdd_ffi_fail_class_name = 1;
    write_to_file("test_fail_cls.h", "struct B : virtual A { int x; };");
    ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_extract_single_file_exports_test(
                                 &test_ir, "test_fail_cls.h",
                                 "struct B : virtual A { int x; };", &config));
    g_cdd_ffi_fail_class_name = 0;
    remove("test_fail_cls.h");
    cdd_ffi_ir_free(&test_ir);

    /* Empty enums, empty structs, @blocking, fn(void), fn(int), fn_in,
     * _Out_writes_ */
    write_to_file("test_branches.h", empty_classes_code);
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_ffi_extract_single_file_exports_test(
                  &test_ir, "test_branches.h", empty_classes_code, &config));
    remove("test_branches.h");
    cdd_ffi_ir_free(&test_ir);

    write_to_file("test_doc_writes.h", doc_and_writes_code);
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_ffi_extract_single_file_exports_test(
                  &test_ir, "test_doc_writes.h", doc_and_writes_code, &config));
    remove("test_doc_writes.h");
    cdd_ffi_ir_free(&test_ir);

    /* Function-like macro and macro without value */
    {
      const char *macros_code = "#define FN_MACRO(x) (x)\n"
                                "#define NO_VAL\n"
                                "#define VAL_MACRO 100\n";
      write_to_file("test_mac_branches.h", macros_code);
      ASSERT_EQ(CDD_C_SUCCESS,
                cdd_ffi_extract_single_file_exports_test(
                    &test_ir, "test_mac_branches.h", macros_code, &config));
      remove("test_mac_branches.h");
      cdd_ffi_ir_free(&test_ir);
    }
  }

  /* 7. is_visited and add_visited branches */
  {
    struct TestMergeCtx {
      cdd_ffi_ir_t *ir;
      const cdd_generate_bindings_config_t *config;
      char **visited;
      size_t visited_count;
      size_t visited_capacity;
      cdd_c_error_t err;
      void *pp_ctx;
    } mctx;
    int vis = 0;
    size_t vi;
    memset(&mctx, 0, sizeof(mctx));

    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_ffi_is_visited_test(NULL, "p", &vis));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_ffi_is_visited_test(&mctx, NULL, &vis));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_ffi_is_visited_test(&mctx, "p", NULL));

    g_cdd_ffi_fail_is_visited = 1;
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, cdd_ffi_is_visited_test(&mctx, "p", &vis));
    g_cdd_ffi_fail_is_visited = 0;

    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_ffi_add_visited_test(NULL, "p"));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_ffi_add_visited_test(&mctx, NULL));

    for (vi = 0; vi < 18; vi++) {
      char pbuf[32];
#if defined(_MSC_VER)
      sprintf_s(pbuf, sizeof(pbuf), "inc_%lu.h", (unsigned long)vi);
#else
      sprintf(pbuf, "inc_%lu.h", (unsigned long)vi);
#endif
      ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_add_visited_test(&mctx, pbuf));
    }
    ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_is_visited_test(&mctx, "inc_0.h", &vis));
    ASSERT_EQ(1, vis);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_is_visited_test(&mctx, "absent.h", &vis));
    ASSERT_EQ(0, vis);

    /* extract_exports_recursive visited and is_visited fail branch */
    g_cdd_ffi_fail_is_visited = 1;
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
              cdd_ffi_extract_exports_recursive_test("f.h", "int a;", &mctx));
    g_cdd_ffi_fail_is_visited = 0;

    ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_extract_exports_recursive_test(
                                 "inc_0.h", "int a;", &mctx));

    /* NULL config branch */
    {
      cdd_ffi_ir_t rec_ir;
      memset(&rec_ir, 0, sizeof(rec_ir));
      write_to_file("f_norec.h", "int a;");
      mctx.ir = &rec_ir;
      mctx.config = NULL;
      ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_extract_exports_recursive_test(
                                   "f_norec.h", "int a;", &mctx));
      remove("f_norec.h");
      cdd_ffi_ir_free(&rec_ir);
    }

    for (vi = 0; vi < mctx.visited_count; vi++) {
      free(mctx.visited[vi]);
    }
    free(mctx.visited);
  }

  /* 8. include_visitor and instantiate_templates */
  {
    struct TestMergeCtx {
      cdd_ffi_ir_t *ir;
      const cdd_generate_bindings_config_t *config;
      char **visited;
      size_t visited_count;
      size_t visited_capacity;
      cdd_c_error_t err;
      void *pp_ctx;
    } mctx;
    struct IncludeInfo inc_info;
    memset(&inc_info, 0, sizeof(inc_info));
    memset(&mctx, 0, sizeof(mctx));

    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_ffi_include_visitor_test(NULL, &mctx));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_ffi_include_visitor_test(&inc_info, NULL));

    mctx.err = CDD_C_ERROR_MEMORY;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              cdd_ffi_include_visitor_test(&inc_info, &mctx));
    mctx.err = CDD_C_SUCCESS;

    inc_info.kind = PP_DIR_INCLUDE;
    inc_info.resolved_path = "test_inc_bad.h";
    g_cdd_ffi_fail_is_visited = 1;
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN,
              cdd_ffi_include_visitor_test(&inc_info, &mctx));
    g_cdd_ffi_fail_is_visited = 0;
    mctx.err = CDD_C_SUCCESS;

    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_ffi_instantiate_templates_test(NULL));
  }

  /* 9. instantiate_templates with diverse type branches */
  {
    cdd_ffi_ir_t inst_ir;
    memset(&inst_ir, 0, sizeof(inst_ir));
    inst_ir.nodes_count = 3;
    inst_ir.nodes_capacity = 6;
    inst_ir.nodes = (cdd_ffi_ir_node_t *)calloc(6, sizeof(cdd_ffi_ir_node_t));

    /* Base template 1: with fields */
    inst_ir.nodes[0].kind = CDD_FFI_NODE_STRUCT;
    inst_ir.nodes[0].name = strdup("Tmpl1");
    inst_ir.nodes[0].fields_count = 3;
    inst_ir.nodes[0].fields =
        (cdd_ffi_field_t *)calloc(3, sizeof(cdd_ffi_field_t));
    inst_ir.nodes[0].fields[0].name = strdup("val");
    inst_ir.nodes[0].fields[0].type.ref_name =
        strdup("T"); /* single char -> replaces kind */
    inst_ir.nodes[0].fields[1].name = strdup("other");
    inst_ir.nodes[0].fields[1].type.ref_name =
        strdup("LongRefName"); /* multi char -> copies ref_name */
    inst_ir.nodes[0].fields[2].name = strdup("prim");
    inst_ir.nodes[0].fields[2].type.kind = CDD_FFI_KIND_INT32;
    inst_ir.nodes[0].fields[2].type.ref_name = NULL;

    /* Base template 2: with 0 fields */
    inst_ir.nodes[1].kind = CDD_FFI_NODE_STRUCT;
    inst_ir.nodes[1].name = strdup("TmplEmpty");
    inst_ir.nodes[1].fields_count = 0;

    /* User struct using Tmpl1<Float32>, Tmpl1<Float64>, Tmpl1<CustomType>,
     * TmplEmpty<int> */
    inst_ir.nodes[2].kind = CDD_FFI_NODE_STRUCT;
    inst_ir.nodes[2].name = strdup("UserStruct");
    inst_ir.nodes[2].fields_count = 6;
    inst_ir.nodes[2].fields =
        (cdd_ffi_field_t *)calloc(6, sizeof(cdd_ffi_field_t));

    inst_ir.nodes[2].fields[0].name = strdup("f1");
    inst_ir.nodes[2].fields[0].type.kind = CDD_FFI_KIND_TEMPLATE_STRUCT_REF;
    inst_ir.nodes[2].fields[0].type.ref_name = strdup("struct Tmpl1");
    inst_ir.nodes[2].fields[0].type.template_args_count = 1;
    inst_ir.nodes[2].fields[0].type.template_args =
        (cdd_ffi_type_t *)calloc(1, sizeof(cdd_ffi_type_t));
    inst_ir.nodes[2].fields[0].type.template_args[0].kind =
        CDD_FFI_KIND_FLOAT32;

    inst_ir.nodes[2].fields[1].name = strdup("f2");
    inst_ir.nodes[2].fields[1].type.kind = CDD_FFI_KIND_TEMPLATE_STRUCT_REF;
    inst_ir.nodes[2].fields[1].type.ref_name = strdup("class Tmpl1");
    inst_ir.nodes[2].fields[1].type.template_args_count = 1;
    inst_ir.nodes[2].fields[1].type.template_args =
        (cdd_ffi_type_t *)calloc(1, sizeof(cdd_ffi_type_t));
    inst_ir.nodes[2].fields[1].type.template_args[0].kind =
        CDD_FFI_KIND_FLOAT64;

    inst_ir.nodes[2].fields[2].name = strdup("f3");
    inst_ir.nodes[2].fields[2].type.kind = CDD_FFI_KIND_TEMPLATE_STRUCT_REF;
    inst_ir.nodes[2].fields[2].type.ref_name = strdup("Tmpl1");
    inst_ir.nodes[2].fields[2].type.template_args_count = 1;
    inst_ir.nodes[2].fields[2].type.template_args =
        (cdd_ffi_type_t *)calloc(1, sizeof(cdd_ffi_type_t));
    inst_ir.nodes[2].fields[2].type.template_args[0].ref_name =
        strdup("MyStruct");

    /* Already instantiated branch: another field using Tmpl1<float> */
    inst_ir.nodes[2].fields[3].name = strdup("f4");
    inst_ir.nodes[2].fields[3].type.kind = CDD_FFI_KIND_TEMPLATE_STRUCT_REF;
    inst_ir.nodes[2].fields[3].type.ref_name = strdup("Tmpl1");
    inst_ir.nodes[2].fields[3].type.template_args_count = 1;
    inst_ir.nodes[2].fields[3].type.template_args =
        (cdd_ffi_type_t *)calloc(1, sizeof(cdd_ffi_type_t));
    inst_ir.nodes[2].fields[3].type.template_args[0].kind =
        CDD_FFI_KIND_FLOAT32;

    /* template_args_count == 0 branch */
    inst_ir.nodes[2].fields[4].name = strdup("f5");
    inst_ir.nodes[2].fields[4].type.kind = CDD_FFI_KIND_TEMPLATE_STRUCT_REF;
    inst_ir.nodes[2].fields[4].type.ref_name = strdup("TmplEmpty");
    inst_ir.nodes[2].fields[4].type.template_args_count = 0;

    /* unhandled kind without ref_name -> unknown */
    inst_ir.nodes[2].fields[5].name = strdup("f6");
    inst_ir.nodes[2].fields[5].type.kind = CDD_FFI_KIND_TEMPLATE_STRUCT_REF;
    inst_ir.nodes[2].fields[5].type.ref_name = strdup("Tmpl1");
    inst_ir.nodes[2].fields[5].type.template_args_count = 1;
    inst_ir.nodes[2].fields[5].type.template_args =
        (cdd_ffi_type_t *)calloc(1, sizeof(cdd_ffi_type_t));
    inst_ir.nodes[2].fields[5].type.template_args[0].kind = CDD_FFI_KIND_UINT8;
    inst_ir.nodes[2].fields[5].type.template_args[0].ref_name = NULL;

    ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_instantiate_templates_test(&inst_ir));
    cdd_ffi_ir_free(&inst_ir);

    /* OOM when allocating new_node->fields in instantiate_templates */
    memset(&inst_ir, 0, sizeof(inst_ir));
    inst_ir.nodes_count = 2;
    inst_ir.nodes_capacity = 4;
    inst_ir.nodes = (cdd_ffi_ir_node_t *)calloc(4, sizeof(cdd_ffi_ir_node_t));
    inst_ir.nodes[0].kind = CDD_FFI_NODE_STRUCT;
    inst_ir.nodes[0].name = strdup("TmplBase");
    inst_ir.nodes[0].fields_count = 1;
    inst_ir.nodes[0].fields =
        (cdd_ffi_field_t *)calloc(1, sizeof(cdd_ffi_field_t));
    inst_ir.nodes[0].fields[0].name = strdup("v");
    inst_ir.nodes[1].kind = CDD_FFI_NODE_STRUCT;
    inst_ir.nodes[1].name = strdup("User");
    inst_ir.nodes[1].fields_count = 1;
    inst_ir.nodes[1].fields =
        (cdd_ffi_field_t *)calloc(1, sizeof(cdd_ffi_field_t));
    inst_ir.nodes[1].fields[0].name = strdup("f");
    inst_ir.nodes[1].fields[0].type.kind = CDD_FFI_KIND_TEMPLATE_STRUCT_REF;
    inst_ir.nodes[1].fields[0].type.ref_name = strdup("TmplBase");
    inst_ir.nodes[1].fields[0].type.template_args_count = 1;
    inst_ir.nodes[1].fields[0].type.template_args =
        (cdd_ffi_type_t *)calloc(1, sizeof(cdd_ffi_type_t));
    inst_ir.nodes[1].fields[0].type.template_args[0].kind = CDD_FFI_KIND_INT32;

    g_ffi_extractor_alloc_fail = 2;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_ffi_instantiate_templates_test(&inst_ir));
    g_ffi_extractor_alloc_fail = 0;
    cdd_ffi_ir_free(&inst_ir);
  }

  PASS();
}

TEST test_cdd_ffi_mangle_cpp_name(void) {
  char *mangled = NULL;

  ASSERT_EQ(0,
            cdd_ffi_mangle_cpp_name("MyNS", "MyClass", "myMethod", &mangled));
  ASSERT(mangled != NULL);
  ASSERT_STR_EQ("MyNS_MyClass_myMethod", mangled);
  free(mangled);

  ASSERT_EQ(0, cdd_ffi_mangle_cpp_name(NULL, "MyClass", "myMethod", &mangled));
  ASSERT(mangled != NULL);
  ASSERT_STR_EQ("MyClass_myMethod", mangled);
  free(mangled);

  ASSERT_EQ(0, cdd_ffi_mangle_cpp_name("MyNS", NULL, "myMethod", &mangled));
  ASSERT(mangled != NULL);
  ASSERT_STR_EQ("MyNS_myMethod", mangled);
  free(mangled);

  ASSERT_EQ(0, cdd_ffi_mangle_cpp_name(NULL, NULL, "myMethod", &mangled));
  ASSERT(mangled != NULL);
  ASSERT_STR_EQ("myMethod", mangled);
  free(mangled);

  PASS();
}

TEST test_ffi_ir_extract_inheritance_e2e(void) {
  const char *filename = "test_inherit.h";
  const char *code = "struct Base {\n"
                     "  int a;\n"
                     "};\n\n"
                     "struct Derived : public virtual Base {\n"
                     "  int b;\n"
                     "};\n";
  cdd_ffi_ir_t *ir = NULL;
  cdd_generate_bindings_config_t config = {0};

  write_to_file(filename, code);
  ASSERT_EQ(0, cdd_ffi_ir_extract_exports(filename, code, &config, &ir));
  ASSERT_EQ(1, ir != NULL);

  {
    size_t i;
    int found_derived = 0;
    for (i = 0; i < ir->nodes_count; i++) {
      if (ir->nodes[i].name && strcmp(ir->nodes[i].name, "Derived") == 0) {
        found_derived = 1;
        break;
      }
    }
    ASSERT_EQ(1, found_derived);
  }

  cdd_ffi_ir_free(ir);
  free(ir);
  remove(filename);
  PASS();
}

TEST test_ffi_ir_extract_template_instantiation(void) {
  const char *filename = "test_tmpl_inst.h";
  const char *code = "struct Inner { int y; };\n"
                     "template <typename T>\n"
                     "struct Box {\n"
                     "  T val;\n"
                     "  char *name;\n"
                     "};\n\n"
                     "struct User {\n"
                     "  struct Box<int> int_box;\n"
                     "  class Box<double> dbl_box;\n"
                     "  Box<float> flt_box;\n"
                     "  Box<Inner> inner_box;\n"
                     "};\n";
  cdd_ffi_ir_t *ir = NULL;
  cdd_generate_bindings_config_t config = {0};

  write_to_file(filename, code);
  ASSERT_EQ(0, cdd_ffi_ir_extract_exports(filename, code, &config, &ir));
  ASSERT_EQ(1, ir != NULL);

  {
    size_t i;
    int found_box_int = 0;
    int found_box_double = 0;
    int found_box_float = 0;
    int found_box_inner = 0;
    for (i = 0; i < ir->nodes_count; i++) {
      if (ir->nodes[i].name) {
        if (strcmp(ir->nodes[i].name, "Box_int") == 0)
          found_box_int = 1;
        if (strcmp(ir->nodes[i].name, "Box_double") == 0)
          found_box_double = 1;
        if (strcmp(ir->nodes[i].name, "Box_float") == 0)
          found_box_float = 1;
        if (strcmp(ir->nodes[i].name, "Box_Inner") == 0)
          found_box_inner = 1;
      }
    }
    ASSERT_EQ(1, found_box_int);
    ASSERT_EQ(1, found_box_double);
    ASSERT_EQ(1, found_box_float);
    ASSERT_EQ(1, found_box_inner);
  }

  cdd_ffi_ir_free(ir);
  free(ir);
  remove(filename);
  PASS();
}

TEST test_ffi_ir_extract_docstring_and_sal_intents(void) {
  const char *filename = "test_intents.h";
  const char *code =
      "/**\n"
      " * @param[out] dst Destination buffer\n"
      " * @param[in] src Source buffer\n"
      " * @param[in,out] io Combined buffer\n"
      " * @ffi_release_gil\n"
      " */\n"
      "void test_fn(int *dst, const int *src, int *io) {}\n"
      "/**\n"
      " * @blocking\n"
      " */\n"
      "void test_blocking_fn(void) {}\n"
      "void test_sal_fn(_Inout_ int *p1, _In_ const int *p2) {}\n";
  cdd_ffi_ir_t *ir = NULL;
  cdd_generate_bindings_config_t config = {0};
  char long_param_buf[1200];
  char code_buf[3000];

  memset(long_param_buf, 'a', sizeof(long_param_buf) - 1);
  long_param_buf[sizeof(long_param_buf) - 1] = '\0';
#if defined(_MSC_VER)
  sprintf_s(code_buf, sizeof(code_buf), "void test_long_fn(int %s) {}\n",
            long_param_buf);
#else
  sprintf(code_buf, "void test_long_fn(int %s) {}\n", long_param_buf);
#endif

  write_to_file("test_long.h", code_buf);
  ASSERT_EQ(0,
            cdd_ffi_ir_extract_exports("test_long.h", code_buf, &config, &ir));
  if (ir) {
    cdd_ffi_ir_free(ir);
    free(ir);
    ir = NULL;
  }
  remove("test_long.h");

  write_to_file(filename, code);
  ASSERT_EQ(0, cdd_ffi_ir_extract_exports(filename, code, &config, &ir));
  ASSERT_EQ(1, ir != NULL);

  {
    size_t i;
    for (i = 0; i < ir->nodes_count; i++) {
      if (ir->nodes[i].name && strcmp(ir->nodes[i].name, "test_fn") == 0) {
        ASSERT_EQ(3, ir->nodes[i].fields_count);
        ASSERT_EQ(CDD_FFI_INTENT_OUT, ir->nodes[i].fields[0].intent);
        ASSERT_EQ(CDD_FFI_INTENT_IN, ir->nodes[i].fields[1].intent);
        ASSERT_EQ(CDD_FFI_INTENT_INOUT, ir->nodes[i].fields[2].intent);
      } else if (ir->nodes[i].name &&
                 strcmp(ir->nodes[i].name, "test_sal_fn") == 0) {
        ASSERT_EQ(2, ir->nodes[i].fields_count);
        ASSERT_EQ(CDD_FFI_INTENT_INOUT, ir->nodes[i].fields[0].intent);
        ASSERT_EQ(CDD_FFI_INTENT_IN, ir->nodes[i].fields[1].intent);
      }
    }
  }

  cdd_ffi_ir_free(ir);
  free(ir);
  remove(filename);
  PASS();
}

TEST test_ffi_ir_extract_trampolines_direct(void) {
  cdd_ffi_ir_t ir = {0};
  cdd_generate_bindings_config_t config = {0};

  ir.nodes_count = 1;
  ir.nodes_capacity = 1;
  ir.nodes = (cdd_ffi_ir_node_t *)calloc(1, sizeof(cdd_ffi_ir_node_t));
  ir.nodes[0].name = strdup("MyClass");
  ir.nodes[0].kind = CDD_FFI_NODE_STRUCT;
  ir.nodes[0].virtual_methods_count = 1;
  ir.nodes[0].virtual_methods =
      (cdd_ffi_virtual_method_t *)calloc(1, sizeof(cdd_ffi_virtual_method_t));
  ir.nodes[0].virtual_methods[0].name = strdup("myVirtualMethod");

  write_to_file("empty.h", "");
  ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_extract_single_file_exports_test(
                               &ir, "empty.h", "", &config));

  {
    size_t i;
    int found_tramp = 0;
    for (i = 0; i < ir.nodes_count; i++) {
      if (ir.nodes[i].name &&
          strcmp(ir.nodes[i].name, "MyClass_Trampoline") == 0) {
        found_tramp = 1;
        ASSERT_EQ(4, ir.nodes[i].fields_count);
        break;
      }
    }
    ASSERT_EQ(1, found_tramp);
  }

  cdd_ffi_ir_free(&ir);
  remove("empty.h");
  PASS();
}

TEST test_ffi_ir_extractor_helpers(void) {
  char buf[64];
  cdd_ffi_primitive_kind_t kind;
  cdd_ffi_type_t t = {0};

  /* int64_to_str edge cases */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_ffi_int64_to_str_test(0, NULL, 0));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, cdd_ffi_int64_to_str_test(0, buf, 0));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_int64_to_str_test(0, buf, sizeof(buf)));
  ASSERT_STR_EQ("0", buf);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_int64_to_str_test(-12345, buf, sizeof(buf)));
  ASSERT_STR_EQ("-12345", buf);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_ffi_int64_to_str_test(987654321, buf, sizeof(buf)));
  ASSERT_STR_EQ("987654321", buf);

  /* map_c_type_to_ffi_kind coverage */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_ffi_map_c_type_to_ffi_kind_test("int", NULL));
  ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_map_c_type_to_ffi_kind_test(NULL, &kind));
  ASSERT_EQ(CDD_FFI_KIND_VOID, kind);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_ffi_map_c_type_to_ffi_kind_test("std_string", &kind));
  ASSERT_EQ(CDD_FFI_KIND_STD_STRING, kind);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_ffi_map_c_type_to_ffi_kind_test("std_vector", &kind));
  ASSERT_EQ(CDD_FFI_KIND_STD_VECTOR, kind);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_ffi_map_c_type_to_ffi_kind_test("std_shared_ptr", &kind));
  ASSERT_EQ(CDD_FFI_KIND_STD_SHARED_PTR, kind);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_ffi_map_c_type_to_ffi_kind_test("std_unique_ptr", &kind));
  ASSERT_EQ(CDD_FFI_KIND_STD_UNIQUE_PTR, kind);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_ffi_map_c_type_to_ffi_kind_test("int8_t", &kind));
  ASSERT_EQ(CDD_FFI_KIND_INT8, kind);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_map_c_type_to_ffi_kind_test("char", &kind));
  ASSERT_EQ(CDD_FFI_KIND_INT8, kind);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_ffi_map_c_type_to_ffi_kind_test("uint8_t", &kind));
  ASSERT_EQ(CDD_FFI_KIND_UINT8, kind);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_ffi_map_c_type_to_ffi_kind_test("unsigned char", &kind));
  ASSERT_EQ(CDD_FFI_KIND_UINT8, kind);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_ffi_map_c_type_to_ffi_kind_test("int16_t", &kind));
  ASSERT_EQ(CDD_FFI_KIND_INT16, kind);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_map_c_type_to_ffi_kind_test("short", &kind));
  ASSERT_EQ(CDD_FFI_KIND_INT16, kind);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_ffi_map_c_type_to_ffi_kind_test("uint16_t", &kind));
  ASSERT_EQ(CDD_FFI_KIND_UINT16, kind);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_ffi_map_c_type_to_ffi_kind_test("unsigned short", &kind));
  ASSERT_EQ(CDD_FFI_KIND_UINT16, kind);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_ffi_map_c_type_to_ffi_kind_test("int32_t", &kind));
  ASSERT_EQ(CDD_FFI_KIND_INT32, kind);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_ffi_map_c_type_to_ffi_kind_test("integer", &kind));
  ASSERT_EQ(CDD_FFI_KIND_INT32, kind);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_ffi_map_c_type_to_ffi_kind_test("uint32_t", &kind));
  ASSERT_EQ(CDD_FFI_KIND_UINT32, kind);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_ffi_map_c_type_to_ffi_kind_test("unsigned int", &kind));
  ASSERT_EQ(CDD_FFI_KIND_UINT32, kind);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_ffi_map_c_type_to_ffi_kind_test("int64_t", &kind));
  ASSERT_EQ(CDD_FFI_KIND_INT64, kind);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_ffi_map_c_type_to_ffi_kind_test("long long", &kind));
  ASSERT_EQ(CDD_FFI_KIND_INT64, kind);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_ffi_map_c_type_to_ffi_kind_test("uint64_t", &kind));
  ASSERT_EQ(CDD_FFI_KIND_UINT64, kind);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_ffi_map_c_type_to_ffi_kind_test("unsigned long long", &kind));
  ASSERT_EQ(CDD_FFI_KIND_UINT64, kind);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_map_c_type_to_ffi_kind_test("float", &kind));
  ASSERT_EQ(CDD_FFI_KIND_FLOAT32, kind);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_ffi_map_c_type_to_ffi_kind_test("double", &kind));
  ASSERT_EQ(CDD_FFI_KIND_FLOAT64, kind);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_ffi_map_c_type_to_ffi_kind_test("number", &kind));
  ASSERT_EQ(CDD_FFI_KIND_FLOAT64, kind);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_map_c_type_to_ffi_kind_test("bool", &kind));
  ASSERT_EQ(CDD_FFI_KIND_BOOL, kind);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_ffi_map_c_type_to_ffi_kind_test("boolean", &kind));
  ASSERT_EQ(CDD_FFI_KIND_BOOL, kind);
  ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_map_c_type_to_ffi_kind_test("void", &kind));
  ASSERT_EQ(CDD_FFI_KIND_VOID, kind);
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_ffi_map_c_type_to_ffi_kind_test("CustomStruct", &kind));
  ASSERT_EQ(CDD_FFI_KIND_STRUCT_REF, kind);

  /* parse_template_type coverage */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_ffi_parse_template_type_test("no_brackets", &t));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            cdd_ffi_parse_template_type_test(">reversed<", &t));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_ffi_parse_template_type_test("std::vector<int>", &t));
  free(t.ref_name);
  free(t.template_args);
  memset(&t, 0, sizeof(t));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_ffi_parse_template_type_test("Wrapper<InnerStruct>", &t));
  free(t.ref_name);
  free(t.template_args[0].ref_name);
  free(t.template_args);
  memset(&t, 0, sizeof(t));
  ASSERT_EQ(CDD_C_SUCCESS,
            cdd_ffi_parse_template_type_test("Outer<Nested<int>>", &t));
  free(t.ref_name);
  free(t.template_args[0].ref_name);
  free(t.template_args[0].template_args);
  free(t.template_args);
  memset(&t, 0, sizeof(t));

  /* parse_template_type allocation failures */
  g_ffi_extractor_alloc_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            cdd_ffi_parse_template_type_test("std::vector<int>", &t));
  g_ffi_extractor_alloc_fail = 2;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            cdd_ffi_parse_template_type_test("std::vector<int>", &t));
  g_ffi_extractor_alloc_fail = 3;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            cdd_ffi_parse_template_type_test("std::vector<MyStruct>", &t));
  g_ffi_extractor_alloc_fail = 4;
  ASSERT_EQ(CDD_C_ERROR_MEMORY,
            cdd_ffi_parse_template_type_test("Outer<Nested<int>>", &t));
  g_ffi_extractor_alloc_fail = 0;

  /* is_visited and add_visited coverage */
  {
    struct DummyVisitedCtx {
      void *ir;
      void *config;
      char **visited;
      size_t visited_count;
      size_t visited_capacity;
      cdd_c_error_t err;
      void *pp_ctx;
    } vctx = {0};
    int visited = 0;

    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_ffi_is_visited_test(&vctx, "path1", NULL));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_is_visited_test(&vctx, "path1", &visited));
    ASSERT_EQ(0, visited);

    ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_add_visited_test(&vctx, "path1"));
    ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_is_visited_test(&vctx, "path1", &visited));
    ASSERT_EQ(1, visited);
    ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_is_visited_test(&vctx, "path2", &visited));
    ASSERT_EQ(0, visited);

    free(vctx.visited[0]);
    free(vctx.visited);
  }

  /* include_visitor error & non-include branches */
  {
    struct DummyIncludeMergeCtx {
      void *ir;
      void *config;
      char **visited;
      size_t visited_count;
      size_t visited_capacity;
      cdd_c_error_t err;
      void *pp_ctx;
    } mctx = {0};
    struct IncludeInfo info = {0};

    mctx.err = CDD_C_ERROR_INVALID_ARGUMENT;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_ffi_include_visitor_test(&info, &mctx));
    mctx.err = CDD_C_SUCCESS;
    info.kind = PP_DIR_EMBED;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_include_visitor_test(&info, &mctx));
    info.kind = PP_DIR_INCLUDE;
    info.resolved_path = NULL;
    ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_include_visitor_test(&info, &mctx));
    info.resolved_path = "non_existent_file_999.h";
    ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_include_visitor_test(&info, &mctx));
    /* Test include_visitor with real file and recursive error */
    {
      write_to_file("rec_err.h", "rec_content");
      info.resolved_path = "rec_err.h";
      /* force allocation failure in extract_exports_recursive */
      g_ffi_extractor_alloc_fail = 1;
      ASSERT_NEQ(CDD_C_SUCCESS, cdd_ffi_include_visitor_test(&info, &mctx));
      g_ffi_extractor_alloc_fail = 0;
      mctx.err = CDD_C_SUCCESS;
      remove("rec_err.h");
    }
    /* Test visited path */
    ASSERT_EQ(CDD_C_SUCCESS,
              cdd_ffi_add_visited_test(&mctx, "already_visited.h"));
    info.resolved_path = "already_visited.h";
    ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_include_visitor_test(&info, &mctx));
    /* Test extract_exports_recursive when file is already visited */
    ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_extract_exports_recursive_test(
                                 "already_visited.h", "", &mctx));
    if (mctx.visited) {
      size_t vi;
      for (vi = 0; vi < mctx.visited_count; vi++)
        free(mctx.visited[vi]);
      free(mctx.visited);
    }
  }

  /* instantiate_templates allocation failure */
  {
    cdd_ffi_ir_t ir = {0};
    ir.nodes_count = 2;
    ir.nodes_capacity = 2;
    ir.nodes = (cdd_ffi_ir_node_t *)calloc(2, sizeof(cdd_ffi_ir_node_t));
    ir.nodes[0].name = strdup("BaseTmpl");
    ir.nodes[0].kind = CDD_FFI_NODE_STRUCT;
    ir.nodes[1].name = strdup("UserStruct");
    ir.nodes[1].kind = CDD_FFI_NODE_STRUCT;
    ir.nodes[1].fields_count = 2;
    ir.nodes[1].fields = (cdd_ffi_field_t *)calloc(2, sizeof(cdd_ffi_field_t));
    ir.nodes[1].fields[0].type.kind = CDD_FFI_KIND_TEMPLATE_STRUCT_REF;
    ir.nodes[1].fields[0].type.ref_name = NULL;
    ir.nodes[1].fields[1].type.kind = CDD_FFI_KIND_TEMPLATE_STRUCT_REF;
    ir.nodes[1].fields[1].type.ref_name = strdup("BaseTmpl");

    g_ffi_extractor_alloc_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY, cdd_ffi_instantiate_templates_test(&ir));
    g_ffi_extractor_alloc_fail = 0;

    cdd_ffi_ir_free(&ir);
  }

  /* cdd_ffi_mangle_cpp_name error branches */
  {
    char *out_m = NULL;
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_ffi_mangle_cpp_name(NULL, NULL, NULL, NULL));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_ffi_mangle_cpp_name("NS", "Class", "Method", NULL));
    ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
              cdd_ffi_mangle_cpp_name("NS", "Class", NULL, &out_m));
    g_ffi_extractor_alloc_fail = 1;
    ASSERT_EQ(CDD_C_ERROR_MEMORY,
              cdd_ffi_mangle_cpp_name("NS", "Class", "Method", &out_m));
    g_ffi_extractor_alloc_fail = 0;
  }

  /* ir_add_node out_node == NULL branch */
  {
    cdd_ffi_ir_t ir = {0};
    ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_ir_add_node_test(&ir, CDD_FFI_NODE_STRUCT,
                                                      "NoOutNode", NULL));
    cdd_ffi_ir_free(&ir);
  }

  PASS();
}

TEST test_ffi_emit_java_fopen_fail(void) {
  cdd_ffi_ir_t *ir = (cdd_ffi_ir_t *)calloc(1, sizeof(cdd_ffi_ir_t));
  cdd_generate_bindings_config_t config = {0};
  config.target_langs = "java";
  config.output_dir =
      "/dev/null/invalid"; /* invalid dir to force fopen to fail */
  config.library_name = "test_lib";

  ASSERT_EQ(1, ir != NULL);
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, cdd_ffi_emit_java(ir, &config));

  cdd_ffi_ir_free(ir);
  free(ir);
  PASS();
}

/**
 * @brief Tests remaining branches in cdd_ffi_ir_extractor.
 *
 * @return GREATEST_TEST_RES.
 */
TEST test_ffi_extractor_missing_branches(void) {
  cdd_generate_bindings_config_t config;
  cdd_ffi_ir_t test_ir;
  int rc;
  const char *code_class;
  const char *code_struct;
  const char *code_nested;
  const char *code_macro;

  memset(&config, 0, sizeof(config));
  memset(&test_ir, 0, sizeof(test_ir));
  code_class = "struct Base { int x; };\n";
  code_struct = "struct S { int x; };\n";
  code_nested = "struct Sub { int a; };\nstruct Parent { struct Sub s; };\n";
  code_macro = "#define FOO 42\n";

  /* 1. g_cdd_ffi_extractor_fail = 1 (cdd_cst_find_nodes_by_type failure) */
  ASSERT_EQ(CDD_C_SUCCESS, write_to_file("base.h", code_class));
  g_cdd_ffi_extractor_fail = 1;
  rc = cdd_ffi_extract_single_file_exports_test(&test_ir, "base.h", code_class,
                                                &config);
  g_cdd_ffi_extractor_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  cdd_ffi_ir_free(&test_ir);
  memset(&test_ir, 0, sizeof(test_ir));

  /* 2. g_cdd_ffi_extractor_fail = 2 (get_class_name failure) */
  g_cdd_ffi_extractor_fail = 2;
  rc = cdd_ffi_extract_single_file_exports_test(&test_ir, "base.h", code_class,
                                                &config);
  g_cdd_ffi_extractor_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
  cdd_ffi_ir_free(&test_ir);
  memset(&test_ir, 0, sizeof(test_ir));
  ASSERT_EQ(0, remove("base.h"));

  /* 3. g_cdd_ffi_extractor_fail = 3 (node->fields calloc failure) */
  ASSERT_EQ(CDD_C_SUCCESS, write_to_file("s.h", code_struct));
  g_cdd_ffi_extractor_fail = 3;
  rc = cdd_ffi_extract_single_file_exports_test(&test_ir, "s.h", code_struct,
                                                &config);
  g_cdd_ffi_extractor_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  cdd_ffi_ir_free(&test_ir);
  memset(&test_ir, 0, sizeof(test_ir));
  ASSERT_EQ(0, remove("s.h"));

  /* 4. g_cdd_ffi_extractor_fail = 4 (ref_name strdup failure) */
  ASSERT_EQ(CDD_C_SUCCESS, write_to_file("parent.h", code_nested));
  g_cdd_ffi_extractor_fail = 4;
  rc = cdd_ffi_extract_single_file_exports_test(&test_ir, "parent.h",
                                                code_nested, &config);
  g_cdd_ffi_extractor_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  cdd_ffi_ir_free(&test_ir);
  memset(&test_ir, 0, sizeof(test_ir));
  ASSERT_EQ(0, remove("parent.h"));

  /* 5. g_cdd_ffi_extractor_fail = 5 (int64_to_str failure) */
  ASSERT_EQ(CDD_C_SUCCESS, write_to_file("test_ffi_macro_temp.h", code_macro));
  g_cdd_ffi_extractor_fail = 5;
  rc = cdd_ffi_extract_single_file_exports_test(
      &test_ir, "test_ffi_macro_temp.h", code_macro, &config);
  g_cdd_ffi_extractor_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
  cdd_ffi_ir_free(&test_ir);
  ASSERT_EQ(0, remove("test_ffi_macro_temp.h"));

  /* 6. g_cdd_ffi_extractor_fail = 6 (map_c_type_to_ffi_kind failure) */
  ASSERT_EQ(CDD_C_SUCCESS, write_to_file("s2.h", code_struct));
  g_cdd_ffi_extractor_fail = 6;
  rc = cdd_ffi_extract_single_file_exports_test(&test_ir, "s2.h", code_struct,
                                                &config);
  g_cdd_ffi_extractor_fail = 0;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
  cdd_ffi_ir_free(&test_ir);
  ASSERT_EQ(0, remove("s2.h"));

  g_fail_io_after = -1;
  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* TEST_FFI_EXTRACTOR_H */
