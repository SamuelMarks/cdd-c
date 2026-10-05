#ifndef TEST_FFI_EXTRACTOR_HELPERS_H
#define TEST_FFI_EXTRACTOR_HELPERS_H

/* clang-format off */
#include "../cdd_test_helpers/cdd_helpers.h"
#include "../../functions/parse/preprocessor.h"
#include "../../functions/ffi/cdd_ffi_ir_extractor.h"
#include "../../classes/parse/cdd_cst_parser.h"
#include "../../classes/parse/cdd_cst_semantic.h"
#include "../../functions/ffi/cdd_ffi_emit_java.h"
#include "c_cdd/format_specifiers.h"
#include <string.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern C_CDD_EXPORT int g_cdd_ffi_extractor_fail;

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
  CDD_SNPRINTF(code_buf, sizeof(code_buf), "void test_long_fn(int %s) {}\n",
               long_param_buf);
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

  {
    char small_buf[3];
    ASSERT_EQ(CDD_C_SUCCESS, cdd_ffi_int64_to_str_test(987654321, small_buf,
                                                       sizeof(small_buf)));
  }

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
  int rc = 0;
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
#endif /* TEST_FFI_EXTRACTOR_HELPERS_H */
