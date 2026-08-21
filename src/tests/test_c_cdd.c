#if defined(_MSC_VER)
different base types from char[3] * / file scope * /
#endif
#if defined(__GNUC__) || defined(__clang__)
#endif

/**
 * @file test_c_cdd.c
 * @brief Main test runner.
 */

/* clang-format off */
#include "c_cdd_export.h"
#include <errno.h>

#ifndef ENOTSUP
#define ENOTSUP 134
#endif

#include <stdlib.h>
#include <time.h>

#include <greatest.h>
#include "c_cdd/format_specifiers.h"
extern int g_fail_io_after;
extern int g_io_calls;

static char g_cdd_test_tmp_buf[65536][64];

static FILE* cdd_test_tmpfile(void) {
    static int counter = 0;
    FILE *f;
    if (counter >= 65536) counter = 0;
    sprintf(g_cdd_test_tmp_buf[counter], "cdd_test_tmp_%d.txt", counter);
    remove(g_cdd_test_tmp_buf[counter]);
    f = fopen(g_cdd_test_tmp_buf[counter], "w+b");
    counter++;
    return f;
}
#define tmpfile() cdd_test_tmpfile()

#include "c_cdd/test_int128.h"
#include "test_cdd_api.h"
#include "emit/test_cdd_cst_emit_unit.h"
#include "emit/test_cst_printer.h"
#ifdef CDD_BUILD_TESTS
extern int g_cdd_alloc_fail;
#endif
#include "emit/test_codegen_build.h"

#include <stdio.h>
#include <c_cdd_export.h>
extern int g_fail_io_after;
static FILE *mock_tmpfile_fuzzer(void) {
    if (g_fail_io_after >= 0) {
        return fopen("/dev/null", "w+b");
    }
    return tmpfile();
}
#ifdef tmpfile
#undef tmpfile
#endif
#define tmpfile mock_tmpfile_fuzzer

#include "emit/test_codegen_client_body.h"
#include "emit/test_codegen_client_sig.h"
#include "emit/test_codegen_defaults.h"
#include "emit/test_codegen_enum.h"
#include "emit/test_codegen_eq.h"
#include "emit/test_codegen_json.h"
#include "emit/test_standalone_json.h"
#include "emit/test_codegen_form.h"
#include "emit/test_codegen_jwt.h"
#include "emit/test_codegen_oauth2_error.h"
#include "emit/test_codegen_make.h"
#include "emit/test_codegen_root_arrays.h"
#include "emit/test_codegen_sdk_tests.h"
#include "emit/test_codegen_security.h"
#include "emit/test_codegen_struct.h"
#include "emit/test_codegen_types.h"
#include "emit/test_codegen_url.h"
#include "emit/test_codegen_validation.h"
#include "emit/test_generate_build_system.h"
#ifdef C_CDD_USE_LIBCURL
#endif
#include "emit/test_openapi_client_gen.h"
#include "emit/test_rewriter_body.h"
#include "emit/test_rewriter_sig.h"
#include "emit/test_schema2tests.h"
#include "emit/test_schema_codegen.h"
#include "emit/test_sync_code.h"
#include "emit/test_weaver.h"
#include "emit/test_text_patcher.h"
#include "emit/test_url_utils.h"
#include "parse/test_analysis.h"
#include "parse/test_c_cdd_integration.h"
#include "parse/test_c_inspector_types.h"
#include "ffi/test_ffi_extractor.h"
#include "ffi/test_cdd_ffi_ir.h"
#include "ffi/test_ffi_e2e.h"
#include "ffi/test_ffi_variadic.h"
#include "ffi/test_ffi_emitters.h"
#include "parse/test_code2schema.h"
#include "parse/test_crypto.h"
#include "parse/test_cst_parser.h"
#include "parse/test_cdd_lexer.h"
#include "parse/test_cdd_cst.h"
#include "parse/test_cdd_cst_mutate.h"
#include "parse/test_cdd_cst_query.h"
#include "parse/test_cdd_cst_trivia.h"
#include "transformers/extern_c/test_extern_c.h"
#include "transformers/msvc_port/test_msvc_port.h"
#include "transformers/gnu_standardizer/test_gnu_standardizer.h"
#include "transformers/error_percolator/test_error_percolator.h"
#include "transformers/macros/test_macros.h"
#include "emit/test_cdd_cst_emit_unit.h"
#include "emit/test_diff_generator.h"


#include "parse/test_dataclasses.h"
#include "parse/test_declarator_parser.h"
#include "parse/test_decl_hoist.h"
#include "parse/test_db_loader.h"
#include "parse/test_desig_init.h"
#include "parse/test_vla_analyzer.h"
#include "parse/test_vcpkg_integration.h"
#include "parse/test_cmake_parser.h"
#include "parse/test_strategy.h"
#include "parse/test_makefile_scraper.h"

#include "parse/test_flexible_array.h"
#include "parse/test_fs.h"
#include "parse/test_initializer_parser.h"
#include "parse/test_json_from_and_to.h"
#include "parse/test_numeric_parser.h"
#include "parse/test_openapi_loader.h"
#include "parse/test_parsing.h"
#include "parse/test_pragma.h"
#include "parse/test_preprocessor.h"
#include "parse/test_preprocessor_macros.h"
#include "parse/test_project_audit.h"
#include "parse/test_refactor.h"
#include "parse/test_refactor_api_sync.h"
#include "parse/test_refactor_orchestrator.h"
#include "parse/test_orchestrator_internals.h"
#include "parse/test_schema_constraints.h"
#include "parse/test_schema_enum_required.h"
#include "parse/test_simple_json.h"
#include "parse/test_str_utils.h"
#include "parse/test_tokenizer.h"
#include "parse/test_tokenizer_trigraphs.h"

/* New Suites */
#include "parse/test_arrays_primitive.h"
#include "parse/test_arrays_object.h"
#include "parse/test_anonymous.h"
#include "parse/test_preprocessor_internals.h"
#include "parse/test_code2schema_coverage.h"
#include "parse/test_orchestrator_coverage.h"
#include "parse/test_c2openapi_op.h"
#include "emit/test_rewriter_body.h"
#include "parse/test_main_coverage.h"
#include "parse/test_c2openapi_schema.h"
#include "../transformers/gnu_standardizer/test_gnu_standardizer_internals.h"
#include "parse/test_integration_c2openapi.h"
#include "parse/test_query_projection.h"
#include "parse/test_cli_parser.h"
#include "parse/test_code2schema_coverage.h"

#include "cdd_test_helpers/test_mock_server.h"
#include "parse/test_fs_coverage.h"
#include "parse/test_cli_c2openapi.h"
#include "emit/test_safe_crt.h"
#include "emit/test_diff.h"

#include "cdd_test_helpers/test_mock_server.h"
#include "parse/test_cdd_cst_escape.h"
#include "parse/test_cdd_cst_scope.h"
#include "parse/test_cdd_cst_semantic.h"
#include "parse/test_cdd_cst_cfg.h"
#include "parse/test_cdd_cst_type_eval.h"
#include "parse/test_cdd_cst_cfg.h"
#include "parse/test_cdd_cst_type_eval.h"

#include "emit/test_aggregator.h"
#include "emit/test_cli_gen.h"
#include "emit/test_client_gui_gen.h"
#include "emit/test_openapi_writer.h"
#include "emit/test_operation.h"
#include "emit/test_server_gen.h"
#include "emit/test_serve_json_rpc.h"
/* #include "parse/test_c2openapi_op.h" */
#include "parse/test_c_mapping.h"
#include "emit/test_codegen_sdk_tests.h"
#include "parse/test_doc_parser.h"

#include "cdd_test_helpers/test_mock_server.h"
/* #include "parse/test_c2openapi_schema.h"
#include "../transformers/gnu_standardizer/test_gnu_standardizer_internals.h"
#include "parse/test_integration_c2openapi.h"
#include "parse/test_query_projection.h"
#include "parse/test_cli_parser.h" */
#include "parse/test_macro_overlay.h"
#include "parse/test_main.h"
#include "parse/test_to_docs_json.h"
#include "parse/test_cli_c2openapi.h"
#include "emit/test_safe_crt.h"
#include "emit/test_diff.h"

#include "cdd_test_helpers/test_mock_server.h"
#include "parse/test_cli_cst.h"
#include "parse/test_cdd_cst_builder.h"
#include "parse/test_cdd_cst_factory.h"

#include "c_cdd/test_int128.h"
#include "test_cdd_api.h"
/* clang-format on */

#include "transformers/safe_crt/test_safe_crt.h"
GREATEST_MAIN_DEFS();

TEST test_cdd_helpers(void) {
  cdd_precondition_failed();
  printf("write_to_file(NULL, NULL) = %d\\n", write_to_file(NULL, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, write_to_file(NULL, NULL));
  ASSERT_NEQ(CDD_C_SUCCESS,
             write_to_file("/invalid/path/that/cannot/exist/ever.txt", "abc"));

#include "cdd_test_helpers_export.h"
  extern int g_cdd_helpers_fopen_err;
  g_io_calls = 0;
  g_fail_io_after = 1;
  g_cdd_helpers_fopen_err = ENOMEM;
  {
    cdd_c_error_t rc = write_to_file("test_helpers.txt", "abc");
    printf("write_to_file ENOMEM test got: %d, g_io_calls: %d, errno: %d\n", rc,
           g_io_calls, errno);
    ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  }

  g_io_calls = 0;
  g_fail_io_after = 1;
  g_cdd_helpers_fopen_err = EINVAL;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            write_to_file("test_helpers.txt", "abc"));

  g_io_calls = 0;
  g_fail_io_after = 1;
  g_cdd_helpers_fopen_err = EIO;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, write_to_file("test_helpers.txt", "abc"));

  g_io_calls = 0;
  g_fail_io_after = 2; /* FPUTS fails */
  ASSERT_EQ(CDD_C_ERROR_IO, write_to_file("test_helpers.txt", "abc"));

  g_io_calls = 0;
  g_fail_io_after = 3; /* FCLOSE fails */
  ASSERT_EQ(CDD_C_ERROR_IO, write_to_file("test_helpers.txt", "abc"));

  g_fail_io_after = -1;
  PASS();
}

SUITE(cdd_helpers_suite) { RUN_TEST(test_cdd_helpers); }

SUITE(ffi_extractor_suite) {
  RUN_TEST(test_ffi_e2e_complex_codebase);
  RUN_TEST(test_ffi_ir_extract_exports_all_types);
  RUN_TEST(test_ffi_ir_extract_exports_basic);
  RUN_TEST(test_ffi_ir_extract_macros);
  RUN_TEST(test_ffi_ir_extract_templates);
  RUN_TEST(test_ffi_ir_extract_includes);
  RUN_TEST(test_ffi_ir_extract_stl_types);
  RUN_TEST(test_ffi_ir_extract_error_paths);
  RUN_TEST(test_ffi_ir_extract_array_out);
  RUN_TEST(test_ffi_ir_extract_inheritance_casting);
  RUN_TEST(test_ffi_ir_extract_trampoline);
  RUN_TEST(test_ffi_ir_extract_exports_oom);
  RUN_TEST(test_ffi_ir_free_robustness);
  RUN_TEST(test_ffi_ir_toposort_basic);
  RUN_TEST(test_ffi_ir_toposort_oom);
  RUN_TEST(test_ffi_ir_emit_python);
  RUN_TEST(test_cdd_ffi_mangle_cpp_name);
  RUN_TEST(test_ffi_ir_emit_rust);
  RUN_TEST(test_ffi_ir_emit_csharp);
  RUN_TEST(test_ffi_ir_emit_typescript);
  RUN_TEST(test_ffi_ir_emit_napi);
  RUN_TEST(test_ffi_ir_emit_java);
  RUN_TEST(test_ffi_emit_java_fopen_fail);
  RUN_TEST(test_ffi_ir_emit_cpp);
  RUN_TEST(test_ffi_ir_emit_go);
  RUN_TEST(test_ffi_ir_emit_swift);
  RUN_TEST(test_ffi_ir_emit_dart);
  RUN_TEST(test_ffi_ir_emit_ruby);
  RUN_TEST(test_ffi_ir_emit_kotlin);
  RUN_TEST(test_ffi_ir_emit_php);
  RUN_TEST(test_ffi_ir_emit_lua);
  RUN_TEST(test_ffi_ir_emit_zig);
  RUN_TEST(test_ffi_ir_emit_odin);
  RUN_TEST(test_ffi_ir_emit_julia);
  RUN_TEST(test_ffi_ir_emit_r);
  RUN_TEST(test_ffi_ir_emit_matlab);
  RUN_TEST(test_ffi_ir_emit_haskell);
  RUN_TEST(test_ffi_ir_emit_ocaml);
}

#ifdef CDD_BUILD_TESTS
#endif

static void reset_mocks(void) {
  extern int g_accept_fail;
  g_accept_fail = 0;
  extern int g_bind_fail;
  g_bind_fail = 0;
  extern int g_cdd_alloc_fail;
  g_cdd_alloc_fail = 0;
  extern int g_cdd_cfg_alloc_fail;
  g_cdd_cfg_alloc_fail = 0;
  extern int g_cdd_cst_alloc_node_fail;
  g_cdd_cst_alloc_node_fail = 0;
  extern int g_cdd_cst_alloc_token_fail;
  g_cdd_cst_alloc_token_fail = 0;
  extern int g_cdd_cst_emit_realloc_fail;
  g_cdd_cst_emit_realloc_fail = 0;
  extern int g_cdd_cst_parser_fast_grow;
  g_cdd_cst_parser_fast_grow = 0;
  extern int g_cdd_cst_realloc_fail;
  g_cdd_cst_realloc_fail = 0;
  extern int g_cdd_ffi_ir_calloc_fail;
  g_cdd_ffi_ir_calloc_fail = 0;
  extern int g_cdd_ffi_ir_malloc_fail;
  g_cdd_ffi_ir_malloc_fail = 0;
  extern int g_cdd_ffi_ir_toposort_fail;
  g_cdd_ffi_ir_toposort_fail = 0;
  extern int g_cdd_fprintf_fail;
  g_cdd_fprintf_fail = 0;
  extern int g_cdd_lexer_id2_fail;
  g_cdd_lexer_id2_fail = 0;
  extern int g_cdd_lexer_id_fail;
  g_cdd_lexer_id_fail = 0;
  extern int g_cdd_lexer_trivia_fail;
  g_cdd_lexer_trivia_fail = 0;
  extern int g_cdd_query_err_fail;
  g_cdd_query_err_fail = 0;
  extern int g_cdd_scope_alloc_fail;
  g_cdd_scope_alloc_fail = 0;
  extern int g_cdd_semantic_leave_fail;
  g_cdd_semantic_leave_fail = 0;
  extern int g_cdd_strdup_fail;
  g_cdd_strdup_fail = 0;
  extern int g_cdd_type_eval_ptr_fail;
  g_cdd_type_eval_ptr_fail = 0;
  extern int g_enum_members_add_fail;
  g_enum_members_add_fail = 0;
  extern int g_enum_members_add_strdup_fail;
  g_enum_members_add_strdup_fail = 0;
  extern int g_enum_members_init_fail;
  g_enum_members_init_fail = 0;
  extern int g_err_perc_fail;
  g_err_perc_fail = 0;
  extern volatile int g_extern_c_bot_node_fail;
  g_extern_c_bot_node_fail = 0;
  extern volatile int g_extern_c_helper_fail;
  g_extern_c_helper_fail = 0;
  extern volatile int g_extern_c_top_node_fail;
  g_extern_c_top_node_fail = 0;
  extern volatile int g_ffi_extractor_alloc_fail;
  g_ffi_extractor_alloc_fail = 0;
  extern int g_force_find_allocations_fail;
  g_force_find_allocations_fail = 0;
  extern int g_force_gnu_alloc_fail;
  g_force_gnu_alloc_fail = 0;
  extern int g_force_parse_tokens_fail;
  g_force_parse_tokens_fail = 0;
  extern int g_force_strdup_fail;
  g_force_strdup_fail = 0;
  extern int g_force_tokenize_fail;
  g_force_tokenize_fail = 0;
  extern int g_getsockname_fail;
  g_getsockname_fail = 0;
  extern int g_json_object_to_struct_fields_fail;
  g_json_object_to_struct_fields_fail = 0;
  extern int g_listen_fail;
  g_listen_fail = 0;
  extern int g_msvc_port_bld_fail;
  g_msvc_port_bld_fail = 0;
  extern int g_pthread_create_fail;
  g_pthread_create_fail = 0;
  extern int g_safe_crt_malloc_fail;
  g_safe_crt_malloc_fail = 0;
  extern int g_schema_codegen_force_fail;
  g_schema_codegen_force_fail = 0;
  extern int g_schema_realloc_fail;
  g_schema_realloc_fail = 0;
  extern int g_schema_strdup_fail;
  g_schema_strdup_fail = 0;
  extern int g_socket_fail;
  g_socket_fail = 0;
  extern int g_str_unquote_malloc_fail;
  g_str_unquote_malloc_fail = 0;
  extern int g_struct_fields_add_fail;
  g_struct_fields_add_fail = 0;
  extern int g_struct_fields_init_fail;
  g_struct_fields_init_fail = 0;
  extern int g_io_calls;
  g_io_calls = 0;
}

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();

  srand((unsigned int)time(NULL));

  {
    cdd_cst_tree_t *tree = NULL;
    const char *snippet =
        "#ifdef A\n#elif B\n#else\n{ int z1; }\n{ int z2; }\n{ int z3; }\n{ "
        "int z4; }\n{ int z5; }\n{ int z6; }\n{ int z7; }\n{ int z8; }\n{ int "
        "z9; }\n{ int z10; }\n#endif\n#ifndef C\n{ int w; }\n#endif\n#define D "
        "1\n#include <stdio.h>\n#pragma once\ntemplate <typename T1, typename "
        "T2, typename T3, typename T4, typename T5, typename T6, typename T7, "
        "typename T8, typename T9, typename T10>\nclass Foo : public Bar, "
        "private Baz {\npublic:\n  void baz() noexcept(true) {}\n  ~Foo();\n  "
        "int operator+(int);\nprotected:\n  int x;\nprivate:\n  int "
        "y;\n};\nnamespace N {\n  using namespace std;\n  void f() {\n    try "
        "{\n      throw 1;\n    } catch (int e) {\n    } catch (...) {\n    "
        "}\n  }\n}\nint main() { asm(\"nop\"); return 0; }\n";
    cdd_c_error_t rc =
        cdd_cst_parse(az_span_create_from_str((char *)snippet), &tree);
    printf("PARSE RC = %d, num_children = %" CDD_PRIz ", capacity = %" CDD_PRIz
           "\n",
           rc, tree->root->num_children, tree->root->capacity);
    if (tree)
      cdd_cst_tree_free(tree);
  }

#if defined(_MSC_VER) && _MSC_VER <= 1400
  GREATEST_MAIN_END();
#endif

#ifdef C_CDD_USE_LIBCURL
#endif

  /* New Runners */
  reset_mocks();
  RUN_SUITE(cdd_cst_trivia_suite);
  reset_mocks();
  RUN_SUITE(cst_parser_suite);
  reset_mocks();
  RUN_SUITE(preprocessor_macros_suite);
  reset_mocks();
  RUN_SUITE(orchestrator_internals_suite);
  reset_mocks();
  RUN_SUITE(analysis_suite);
  reset_mocks();
  RUN_SUITE(main_suite);
  reset_mocks();
  RUN_SUITE(operation_suite);
  reset_mocks();
  RUN_SUITE(code2schema_suite);
  reset_mocks();
  RUN_SUITE(fs_suite);
  reset_mocks();
  RUN_SUITE(cdd_api_suite);
  reset_mocks();
  RUN_SUITE(decl_hoist_suite);
  reset_mocks();
  RUN_SUITE(cdd_cst_suite);
  reset_mocks();
  RUN_SUITE(cdd_cst_cfg_suite);
  reset_mocks();
  RUN_SUITE(tokenizer_suite);
  reset_mocks();
  RUN_SUITE(vcpkg_integration_suite);
  reset_mocks();
  RUN_SUITE(cdd_cst_semantic_suite);
  reset_mocks();
  RUN_SUITE(initializer_parser_suite);
  reset_mocks();
  RUN_SUITE(cdd_cst_type_eval_suite);
  reset_mocks();
  RUN_SUITE(cdd_cst_escape_suite);
  reset_mocks();
  RUN_SUITE(desig_init_suite);
  reset_mocks();
  RUN_SUITE(pragma_suite);
  reset_mocks();
  RUN_SUITE(makefile_scraper_suite);
  reset_mocks();
  RUN_SUITE(api_sync_suite);
  reset_mocks();
  RUN_SUITE(strategy_suite);
  reset_mocks();
  reset_mocks();
  RUN_SUITE(tokenizer_trigraphs_suite);
  reset_mocks();
  RUN_SUITE(refactor_suite);
  reset_mocks();
  reset_mocks();
  RUN_SUITE(to_docs_json_suite);
  reset_mocks();
  RUN_SUITE(cdd_cst_factory_suite);
  reset_mocks();
  RUN_SUITE(cdd_lexer_suite);
  reset_mocks();
  reset_mocks();
  RUN_SUITE(cdd_cst_scope_suite);
  reset_mocks();
  RUN_SUITE(project_audit_suite);
  reset_mocks();
  reset_mocks();
  RUN_SUITE(json_from_and_to_suite);
  reset_mocks();
  RUN_SUITE(declarator_parser_suite);
  reset_mocks();
  RUN_SUITE(macro_overlay_suite);
  reset_mocks();
  RUN_SUITE(refactor_orchestrator_suite);
  reset_mocks();
  RUN_SUITE(c_mapping_suite);
  reset_mocks();
  RUN_SUITE(db_loader_suite);
  reset_mocks();
  RUN_SUITE(schema_enum_required_suite);
  reset_mocks();
  RUN_SUITE(cmake_parser_suite);
  reset_mocks();
  RUN_SUITE(cdd_cst_mutate_suite);
  reset_mocks();
  RUN_SUITE(flexible_array_suite);
  reset_mocks();
  RUN_SUITE(cdd_cst_query_suite);
  reset_mocks();
  RUN_SUITE(preprocessor_suite);
  reset_mocks();
  RUN_SUITE(schema_constraints_suite);
  reset_mocks();
  RUN_SUITE(cli_cst_suite);
  reset_mocks();
  RUN_SUITE(openapi_loader_suite);
  reset_mocks();
  RUN_SUITE(numeric_parser_suite);
  reset_mocks();
  RUN_SUITE(str_utils_suite);
  reset_mocks();
  RUN_SUITE(c_inspector_types_suite);
  reset_mocks();
  RUN_SUITE(doc_parser_suite);
  reset_mocks();
  reset_mocks();
  reset_mocks();
  RUN_SUITE(dataclasses_suite);
  reset_mocks();
  RUN_SUITE(vla_analyzer_suite);
  reset_mocks();
  RUN_SUITE(integration_suite);
  reset_mocks();
  RUN_SUITE(crypto_suite);
  reset_mocks();
  RUN_SUITE(c_cdd_int128_suite);
  reset_mocks();
  RUN_SUITE(transformer_msvc_port_suite);
  reset_mocks();
  RUN_SUITE(transformer_extern_c_suite);
  reset_mocks();
  RUN_SUITE(transformer_macros_suite);
  reset_mocks();
  RUN_SUITE(transformer_error_percolator_suite);
  reset_mocks();
  RUN_SUITE(cdd_ffi_ir_suite);
  reset_mocks();
  RUN_SUITE(ffi_variadic_suite);
  reset_mocks();
  RUN_SUITE(ffi_emitters_suite);
  reset_mocks();
  RUN_SUITE(cli_gen_suite);
  reset_mocks();
  RUN_SUITE(codegen_json_suite);
  reset_mocks();
  RUN_SUITE(text_patcher_suite);
  reset_mocks();
  RUN_SUITE(client_sig_suite);
  reset_mocks();
  RUN_SUITE(sync_code_suite);
  reset_mocks();
  RUN_SUITE(codegen_enum_suite);
  reset_mocks();
  RUN_SUITE(codegen_eq_suite);
  reset_mocks();
  RUN_SUITE(rewriter_sig_suite);
  reset_mocks();
  RUN_SUITE(client_gui_gen_suite);
  reset_mocks();
  RUN_SUITE(codegen_jwt_suite);
  reset_mocks();
  RUN_SUITE(cdd_cst_emit_unit_suite);
  reset_mocks();
  RUN_SUITE(codegen_struct_suite);
  reset_mocks();
  RUN_SUITE(weaver_suite);
  reset_mocks();
  RUN_SUITE(standalone_json_suite);
  reset_mocks();
  RUN_SUITE(aggregator_suite);
  reset_mocks();
  RUN_SUITE(codegen_defaults_suite);
  reset_mocks();
  RUN_SUITE(codegen_sdk_tests_suite);
  reset_mocks();
  RUN_SUITE(safe_crt_suite);
  reset_mocks();
  RUN_SUITE(diff_suite);
  reset_mocks();
  RUN_SUITE(c_cdd_mock_server_suite);
  reset_mocks();
  RUN_SUITE(cli_c2openapi_suite);
  reset_mocks();
  RUN_SUITE(rewriter_body_suite);
  reset_mocks();
  RUN_SUITE(cdd_cst_builder_suite);
  reset_mocks();
  RUN_SUITE(cdd_helpers_suite);
  reset_mocks();
  RUN_SUITE(diff_generator_suite);
  reset_mocks();
  RUN_SUITE(ffi_extractor_suite);
  reset_mocks();
  RUN_SUITE(parsing_suite);
  reset_mocks();
  RUN_SUITE(transformer_gnu_standardizer_suite);
  reset_mocks();
  RUN_SUITE(transformer_safe_crt_suite);
  reset_mocks();
  RUN_SUITE(root_array_suite);
  reset_mocks();
  RUN_SUITE(generate_build_system_suite);
  reset_mocks();
  RUN_SUITE(codegen_make_suite);
  reset_mocks();
  RUN_SUITE(server_gen_suite);
  reset_mocks();
  RUN_SUITE(cst_printer_suite);
  reset_mocks();
  RUN_SUITE(schema2tests_suite);
  reset_mocks();
  RUN_SUITE(codegen_url_suite);
  reset_mocks();
  reset_mocks();
  RUN_SUITE(codegen_validation_suite);
  reset_mocks();
  RUN_SUITE(serve_json_rpc_suite);
  reset_mocks();
  RUN_SUITE(codegen_form_suite);
  reset_mocks();
  RUN_SUITE(codegen_build_suite);
  reset_mocks();
  RUN_SUITE(codegen_types_suite);
  reset_mocks();
  RUN_SUITE(openapi_client_gen_suite);
  reset_mocks();
  RUN_SUITE(codegen_oauth2_error_suite);
  reset_mocks();
  RUN_SUITE(client_body_suite);
  reset_mocks();
  RUN_SUITE(openapi_writer_suite);
  reset_mocks();
  RUN_SUITE(url_utils_suite);
  reset_mocks();
  RUN_SUITE(schema_codegen_suite);
  reset_mocks();
  RUN_SUITE(codegen_security_suite);
  reset_mocks();
  RUN_SUITE(arrays_primitive_suite);
  reset_mocks();
  RUN_SUITE(arrays_object_suite);
  reset_mocks();
  RUN_SUITE(anonymous_suite);
  reset_mocks();
  RUN_SUITE(preprocessor_internals_suite);
  reset_mocks();
  RUN_SUITE(code2schema_coverage_suite);
  reset_mocks();
  RUN_SUITE(main_coverage_suite);
  reset_mocks();
  RUN_SUITE(integration_c2openapi_suite);
  reset_mocks();
  RUN_SUITE(query_projection_suite);
  reset_mocks();
  RUN_SUITE(cli_parser_suite);
  reset_mocks();
  RUN_SUITE(orchestrator_coverage_suite);
  reset_mocks();
  RUN_SUITE(fs_coverage_suite);
  reset_mocks();
  RUN_SUITE(c2openapi_op_suite);
  reset_mocks();
  RUN_SUITE(c2openapi_schema_suite);
  reset_mocks();
  RUN_SUITE(transformer_gnu_standardizer_internals_suite);
  reset_mocks();
  reset_mocks();
  reset_mocks();
  reset_mocks();
  reset_mocks();
  reset_mocks();
  reset_mocks();
  reset_mocks();
  reset_mocks();
  reset_mocks();
  reset_mocks();
  reset_mocks();
  reset_mocks();

  /*  */

  /*  */
  /*  */

  /*   */

  GREATEST_MAIN_END();
}

#if defined(__GNUC__) || defined(__clang__)
#endif
