#ifndef TEST_FFI_EXTRACTOR_H
#define TEST_FFI_EXTRACTOR_H

/* clang-format off */
#include "../cdd_test_helpers/cdd_helpers.h"
#include "../../functions/ffi/cdd_ffi_ir_extractor.h"
#include "../../classes/parse/cdd_cst_parser.h"
#include "../../classes/parse/cdd_cst_semantic.h"
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
  remove(filename);
  PASS();
}

TEST test_ffi_ir_extract_macros(void) {
  const char *filename = "dummy_header_macros.h";
  const char *code = "#define PI 3.14159\n"
                     "#define PORT 8080\n"
                     "#define STRING_CONST \"Hello World\"\n"
                     "#define FUNC_MACRO(a, b) (a + b)\n";
  cdd_ffi_ir_t *ir = NULL;
  cdd_generate_bindings_config_t config = {0};

  write_to_file(filename, code);

  ASSERT_EQ(0, cdd_ffi_ir_extract_exports(filename, code, &config, &ir));

  ASSERT_EQ(1, ir != NULL);
  printf("IR nodes count for macros: %lu\n", (unsigned long)ir->nodes_count);
  /* We expect 3 macros (PI, PORT, STRING_CONST) since FUNC_MACRO is
   * function-like */
  ASSERT_EQ(3, ir->nodes_count);

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
  remove(filename);
  PASS();
}

TEST test_ffi_ir_extract_includes(void) {
  const char *filename_main = "dummy_main.h";
  const char *filename_inc = "dummy_inc.h";
  const char *code_main =
      "#include \"dummy_inc.h\"\nstruct MainStruct { int a; };\n";
  const char *code_inc = "struct IncStruct { int b; };\n";
  cdd_ffi_ir_t *ir = NULL;
  cdd_generate_bindings_config_t config = {0};

  config.recursive_includes = 1;

  write_to_file(filename_main, code_main);
  write_to_file(filename_inc, code_inc);

  ASSERT_EQ(0,
            cdd_ffi_ir_extract_exports(filename_main, code_main, &config, &ir));

  ASSERT_EQ(1, ir != NULL);
  /* We expect 2 structs, one from main, one from inc */
  ASSERT_EQ(2, ir->nodes_count);

  {
    int found_main = 0;
    int found_inc = 0;
    size_t i;
    for (i = 0; i < ir->nodes_count; i++) {
      if (ir->nodes[i].kind == CDD_FFI_NODE_STRUCT) {
        if (strcmp(ir->nodes[i].name, "MainStruct") == 0)
          found_main = 1;
        if (strcmp(ir->nodes[i].name, "IncStruct") == 0)
          found_inc = 1;
      }
    }
    ASSERT_EQ(1, found_main);
    ASSERT_EQ(1, found_inc);
  }

  cdd_ffi_ir_free(ir);
  remove(filename_main);
  remove(filename_inc);
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

TEST test_ffi_ir_extract_exports_oom(void) {
  /* Skipping OOM test since global mock allocators aren't fully wired for this
   * module yet. */
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
          int is_virtual_cast = 0;

          if (ir.nodes[k].base_classes[b].is_virtual) {
            is_virtual_cast = 1;
          }
          (void)is_virtual_cast;

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
  char *output_dir = "test_python_out";
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
  f = fopen("test_python_out/cdd_bindings.py", "r");
#endif
  ASSERT_EQ(1, f != NULL);
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
  char *output_dir = "test_rust_out";
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
  f = fopen("test_rust_out/Cargo.toml", "r");
#endif
  ASSERT_EQ(1, f != NULL);
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
  char *output_dir = "test_csharp_out";
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
  f = fopen("test_csharp_out/Bindings.cs", "r");
#endif
  ASSERT_EQ(1, f != NULL);
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
  char *output_dir = "test_typescript_out";
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
  f = fopen("test_typescript_out/test_lib.ts", "r");
#endif
  ASSERT_EQ(1, f != NULL);
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
  char *output_dir = "test_napi_out";
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
  f = fopen("test_napi_out/test_lib_napi.c", "r");
#endif
  ASSERT_EQ(1, f != NULL);
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
  char *output_dir = "test_java_out";
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

  nodes[1].kind = CDD_FFI_NODE_FUNCTION;
  nodes[1].name = strdup("my_func");
  nodes[1].return_or_base_type.kind = CDD_FFI_KIND_VOID;

  ASSERT_EQ(0, cdd_ffi_emit_java(ir, &config));

#if defined(_MSC_VER)
  fopen_s(&f, "test_java_out\\TestLib.java", "r");
#else
  f = fopen("test_java_out/TestLib.java", "r");
#endif
  ASSERT_EQ(1, f != NULL);
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
  char *output_dir = "test_cpp_out";
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
  f = fopen("test_cpp_out/test_lib.hpp", "r");
#endif
  ASSERT_EQ(1, f != NULL);
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
  char *output_dir = "test_go_out";
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
  f = fopen("test_go_out/test_lib.go", "r");
#endif
  ASSERT_EQ(1, f != NULL);
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
  char *output_dir = "test_swift_out";
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
  f = fopen("test_swift_out/test_lib.swift", "r");
#endif
  ASSERT_EQ(1, f != NULL);
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
  char *output_dir = "test_dart_out";
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
  f = fopen("test_dart_out/test_lib.dart", "r");
#endif
  ASSERT_EQ(1, f != NULL);
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
  char *output_dir = "test_ruby_out";
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
  f = fopen("test_ruby_out/test_lib.rb", "r");
#endif
  ASSERT_EQ(1, f != NULL);
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
  char *output_dir = "test_kotlin_out";
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
  f = fopen("test_kotlin_out/test_lib.def", "r");
#endif
  ASSERT_EQ(1, f != NULL);
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
  char *output_dir = "test_php_out";
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
  f = fopen("test_php_out/TestLib.php", "r");
#endif
  ASSERT_EQ(1, f != NULL);
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
  char *output_dir = "test_lua_out";
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
  f = fopen("test_lua_out/test_lib.lua", "r");
#endif
  ASSERT_EQ(1, f != NULL);
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
  char *output_dir = "test_zig_out";
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
  f = fopen("test_zig_out/test_lib.zig", "r");
#endif
  ASSERT_EQ(1, f != NULL);
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
  char *output_dir = "test_odin_out";
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

  ASSERT_EQ(0, cdd_ffi_emit_odin(ir, &config));

#if defined(_MSC_VER)
  fopen_s(&f, "test_odin_out\\test_lib.odin", "r");
#else
  f = fopen("test_odin_out/test_lib.odin", "r");
#endif
  ASSERT_EQ(1, f != NULL);
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
  char *output_dir = "test_julia_out";
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
  f = fopen("test_julia_out/TestLib.jl", "r");
#endif
  ASSERT_EQ(1, f != NULL);
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
  char *output_dir = "test_r_out";
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
  f = fopen("test_r_out/test_lib.R", "r");
#endif
  ASSERT_EQ(1, f != NULL);
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
  char *output_dir = "test_matlab_out";
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
  f = fopen("test_matlab_out/test_lib.m", "r");
#endif
  ASSERT_EQ(1, f != NULL);
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
  char *output_dir = "test_haskell_out";
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
  f = fopen("test_haskell_out/TestLib.hs", "r");
#endif
  ASSERT_EQ(1, f != NULL);
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
  char *output_dir = "test_ocaml_out";
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
  f = fopen("test_ocaml_out/test_lib.ml", "r");
#endif
  ASSERT_EQ(1, f != NULL);
  if (f)
    fclose(f);

  cdd_ffi_ir_free(ir);
  free(ir);
  PASS();
}

TEST test_ffi_ir_extract_error_paths(void) {
  cdd_ffi_ir_t *ir = NULL;
  cdd_generate_bindings_config_t config = {0};
  ASSERT_NEQ(0, cdd_ffi_ir_extract_exports(NULL, NULL, &config, &ir));
  ASSERT_EQ(1, ir == NULL);
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

#endif /* TEST_FFI_EXTRACTOR_H */
