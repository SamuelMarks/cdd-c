#ifndef TEST_FFI_EXTRACTOR_ERROR_H
#define TEST_FFI_EXTRACTOR_ERROR_H

/* clang-format off */
#include "../cdd_test_helpers/cdd_helpers.h"
#include "../../functions/parse/preprocessor.h"
#include "../../functions/ffi/cdd_ffi_ir_extractor.h"
#include "../../classes/parse/cdd_cst_parser.h"
#include "../../classes/parse/cdd_cst_semantic.h"
#include "c_cdd/format_specifiers.h"
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
extern C_CDD_EXPORT volatile int g_ffi_extractor_alloc_fail;

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

    /* Trivia OOM branches for KIND_STRUCT */
    {
      int fail_idx;
      write_to_file(
          "test_trivia_struct.h",
          "/* leading */\nstruct MyStruct { int a; };\n// trailing\n");
      for (fail_idx = 1; fail_idx <= 25; fail_idx++) {
        cdd_ffi_ir_t triv_ir;
        memset(&triv_ir, 0, sizeof(triv_ir));
        g_ffi_extractor_alloc_fail = fail_idx;
        cdd_ffi_extract_single_file_exports_test(
            &triv_ir, "test_trivia_struct.h",
            "/* leading */\nstruct MyStruct { int a; };\n// trailing\n",
            &config);
        g_ffi_extractor_alloc_fail = 0;
        cdd_ffi_ir_free(&triv_ir);
      }
      remove("test_trivia_struct.h");
    }

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
      CDD_SNPRINTF(pbuf, sizeof(pbuf), "inc_%lu.h", (unsigned long)vi);
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

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* TEST_FFI_EXTRACTOR_ERROR_H */
