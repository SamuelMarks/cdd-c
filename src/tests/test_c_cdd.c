#if defined(__clang__)

#endif
#if defined(__GNUC__) || defined(__clang__)
#endif

/**
 * @file test_c_cdd.c
 * @brief Main test runner.
 */

/* clang-format off */
#include <wchar.h>
#include "c_cdd/safe_crt_msvc.h"

#include "c_cdd_export.h"
#include <errno.h>






#include "c_cdd/memory.h"



extern C_CDD_EXPORT int g_fail_io_after;
extern C_CDD_EXPORT int g_io_calls;
extern C_CDD_EXPORT int g_cdd_cst_emit_realloc_fail;
extern C_CDD_EXPORT int g_schema_strdup_fail;
extern C_CDD_EXPORT int g_schema_realloc_fail;
extern C_CDD_EXPORT int g_schema_fail_io_after;

int g_force_gnu_alloc_fail = 0;
extern C_CDD_EXPORT int g_force_parse_tokens_fail;
extern C_CDD_EXPORT int g_force_find_allocations_fail;
int g_force_strdup_fail = 0;
extern C_CDD_EXPORT int g_force_tokenize_fail;

int g_cdd_wine_skip = 0;

extern C_CDD_EXPORT int g_schema_io_calls;
extern C_CDD_EXPORT int g_schema_codegen_force_fail;
extern C_CDD_EXPORT int g_cdd_cst_realloc_fail;
extern C_CDD_EXPORT int g_cdd_cst_parser_fast_grow;
extern C_CDD_EXPORT int g_cdd_cst_alloc_token_fail;
extern C_CDD_EXPORT int g_cdd_cst_alloc_node_fail;
extern C_CDD_EXPORT int g_cdd_scope_alloc_fail;


extern C_CDD_EXPORT int g_fail_io_after;
extern C_CDD_EXPORT int g_io_calls;
extern C_CDD_EXPORT int g_cdd_cst_emit_realloc_fail;
extern C_CDD_EXPORT int g_schema_strdup_fail;
extern C_CDD_EXPORT int g_schema_realloc_fail;
extern C_CDD_EXPORT int g_schema_fail_io_after;
extern C_CDD_EXPORT int g_schema_io_calls;
extern C_CDD_EXPORT int g_schema_codegen_force_fail;
extern C_CDD_EXPORT int g_cdd_cst_realloc_fail;
extern C_CDD_EXPORT int g_cdd_cst_parser_fast_grow;




extern C_CDD_EXPORT int g_fail_io_after;
extern C_CDD_EXPORT int g_io_calls;
extern C_CDD_EXPORT int g_cdd_cst_emit_realloc_fail;
extern C_CDD_EXPORT int g_schema_strdup_fail;
extern C_CDD_EXPORT int g_schema_realloc_fail;
extern C_CDD_EXPORT int g_schema_fail_io_after;
extern C_CDD_EXPORT int g_schema_io_calls;
extern C_CDD_EXPORT int g_schema_codegen_force_fail;
extern C_CDD_EXPORT int g_cdd_cst_realloc_fail;
extern C_CDD_EXPORT int g_cdd_cst_parser_fast_grow;


extern C_CDD_EXPORT int g_schema_strdup_fail;
extern C_CDD_EXPORT int g_schema_realloc_fail;
extern C_CDD_EXPORT int g_schema_fail_io_after;
extern C_CDD_EXPORT int g_schema_io_calls;
extern C_CDD_EXPORT int g_schema_codegen_force_fail;
extern C_CDD_EXPORT int g_cdd_cst_realloc_fail;
extern C_CDD_EXPORT int g_cdd_cst_parser_fast_grow;




extern C_CDD_EXPORT int g_fail_io_after;
extern C_CDD_EXPORT int g_io_calls;
extern C_CDD_EXPORT int g_cdd_cst_emit_realloc_fail;
extern C_CDD_EXPORT int g_schema_strdup_fail;
extern C_CDD_EXPORT int g_schema_realloc_fail;
extern C_CDD_EXPORT int g_schema_fail_io_after;
extern C_CDD_EXPORT int g_schema_io_calls;
extern C_CDD_EXPORT int g_schema_codegen_force_fail;
extern C_CDD_EXPORT int g_cdd_cst_realloc_fail;
extern C_CDD_EXPORT int g_cdd_cst_parser_fast_grow;


extern C_CDD_EXPORT int g_schema_strdup_fail;
extern C_CDD_EXPORT int g_schema_realloc_fail;
extern C_CDD_EXPORT int g_schema_fail_io_after;
extern C_CDD_EXPORT int g_schema_io_calls;
extern C_CDD_EXPORT int g_schema_codegen_force_fail;
extern C_CDD_EXPORT int g_cdd_cst_realloc_fail;
extern C_CDD_EXPORT int g_cdd_cst_parser_fast_grow;




#ifndef ENOTSUP
#define ENOTSUP 134
#endif

#include <stdlib.h>
#include <time.h>

#include "c_cdd/format_specifiers.h"
#include <greatest.h>
  /* extern C_CDD_EXPORT int g_fail_io_after; (moved to global) */
  /* extern C_CDD_EXPORT int g_io_calls; (moved to global) */

#include "cdd_test_helpers_export.h"
CDD_TEST_HELPERS_EXPORT FILE* cdd_test_tmpfile_global(void);


#include "c_cdd/test_int128.h"
#include "emit/test_cdd_cst_emit_unit.h"
#include "emit/test_cst_printer.h"
#include "test_cdd_api.h"
#ifdef CDD_BUILD_TESTS
  /*  (moved to global) */
#endif
#include "emit/test_codegen_build.h"

#include <c_cdd_export.h>
#include <stdio.h>
  /* extern C_CDD_EXPORT int g_fail_io_after; (moved to global) */
static FILE *mock_tmpfile_fuzzer(void) {
    if (g_fail_io_after >= 0) {
        return fopen("/dev/null", "w+b");
    }
    return cdd_test_tmpfile_global();
}
#ifdef tmpfile
#undef tmpfile
#endif
#define tmpfile mock_tmpfile_fuzzer

#include "emit/test_codegen_client_body.h"
#include "emit/test_codegen_client_body_headers.h"
#include "emit/test_codegen_client_body_forms.h"
#include "emit/test_codegen_client_body_responses.h"
#include "emit/test_codegen_client_body_media.h"
#include "emit/test_codegen_client_body_joined.h"
#include "emit/test_codegen_client_body_types.h"
#include "emit/test_codegen_client_body_mega.h"
#include "emit/test_codegen_client_body_internals.h"
#include "emit/test_codegen_client_sig.h"
#include "emit/test_codegen_client_sig_branches.h"
#include "emit/test_codegen_client_sig_coverage.h"
#include "emit/test_codegen_client_sig_helpers.h"
#include "emit/test_codegen_client_sig_media.h"
#include "emit/test_codegen_client_sig_params.h"
#include "emit/test_codegen_client_sig_ultra.h"
#include "emit/test_codegen_defaults.h"
#include "emit/test_codegen_enum.h"
#include "emit/test_codegen_eq.h"
#include "emit/test_codegen_form.h"
#include "emit/test_codegen_json.h"
#include "emit/test_codegen_json_comprehensive.h"
#include "emit/test_codegen_jwt.h"
#include "emit/test_codegen_make.h"
#include "emit/test_codegen_oauth2_error.h"
#include "emit/test_codegen_root_arrays.h"
#include "emit/test_codegen_sdk_tests.h"
#include "emit/test_codegen_security.h"
#include "emit/test_codegen_security_nulls.h"
#include "emit/test_codegen_struct.h"
#include "emit/test_codegen_struct_io.h"
#include "emit/test_codegen_types.h"
#include "emit/test_codegen_types_exhaustive.h"
#include "emit/test_codegen_types_uncovered.h"
#include "emit/test_codegen_url.h"
#include "emit/test_codegen_url_objects.h"
#include "emit/test_codegen_url_internals.h"
#include "emit/test_codegen_url_builder.h"
#include "emit/test_codegen_url_io_failures.h"
#include "emit/test_codegen_validation.h"
#include "emit/test_generate_build_system.h"
#include "emit/test_standalone_json.h"
#ifdef C_CDD_USE_LIBCURL
#endif
#include "emit/test_cdd_cst_emit_unit.h"
#include "emit/test_diff_generator.h"
#include "emit/test_openapi_client_gen.h"
#include "emit/test_openapi_client_gen_helpers.h"
#include "emit/test_openapi_client_gen_mocks.h"
#include "emit/test_openapi_client_gen_expanded.h"
#include "emit/test_rewriter_body.h"
#include "emit/test_rewriter_sig.h"
#include "emit/test_schema2tests.h"
#include "emit/test_schema_codegen.h"
#include "emit/test_schema_codegen_cli.h"
#include "emit/test_sync_code.h"
#include "emit/test_text_patcher.h"
#include "emit/test_text_patcher_oom.h"
#include "emit/test_url_utils.h"
#include "emit/test_weaver.h"
#include "ffi/test_cdd_ffi_ir.h"
#include "ffi/test_ffi_e2e.h"
#include "ffi/test_ffi_emitters.h"
#include "ffi/test_ffi_emitters_edge.h"
#include "ffi/test_ffi_extractor.h"
#include "ffi/test_ffi_extractor_emit.h"
#include "ffi/test_ffi_extractor_emit_extra.h"
#include "ffi/test_ffi_extractor_error.h"
#include "ffi/test_ffi_extractor_helpers.h"
#include "ffi/test_ffi_variadic.h"
#include "parse/test_analysis.h"
#include "parse/test_c_cdd_integration.h"
#include "parse/test_c_inspector_types.h"
#include "parse/test_cdd_cst.h"
#include "parse/test_cdd_cst_mutate.h"
#include "parse/test_cdd_cst_query.h"
#include "parse/test_cdd_cst_trivia.h"
#include "parse/test_cdd_lexer.h"
#include "parse/test_code2schema.h"
#include "parse/test_code2schema_helpers.h"
#include "parse/test_crypto.h"
#include "parse/test_cst_parser.h"
#include "transformers/error_percolator/test_error_percolator.h"
#include "transformers/error_percolator/test_error_percolator_call_sites.h"
#include "transformers/extern_c/test_extern_c.h"
#include "transformers/extern_c/test_extern_c_coverage.h"
#include "transformers/gnu_standardizer/test_gnu_standardizer.h"
#include "transformers/gnu_standardizer/test_gnu_standardizer_branches.h"
#include "transformers/macros/test_macros.h"
#include "transformers/msvc_port/test_msvc_port.h"


#include "parse/test_cmake_parser.h"
#include "parse/test_dataclasses.h"
#include "parse/test_db_loader.h"
#include "parse/test_decl_hoist.h"
#include "parse/test_declarator_parser.h"
#include "parse/test_declarator_parser_helpers.h"
#include "parse/test_desig_init.h"
#include "parse/test_makefile_scraper.h"
#include "parse/test_strategy.h"
#include "parse/test_vcpkg_integration.h"
#include "parse/test_vla_analyzer.h"

#include "parse/test_flexible_array.h"
#include "parse/test_fs.h"
#include "parse/test_initializer_parser.h"
#include "parse/test_json_from_and_to.h"
#include "parse/test_numeric_parser.h"
#include "parse/test_openapi_loader.h"
#include "parse/test_orchestrator_internals.h"
#include "parse/test_parsing.h"
#include "parse/test_pragma.h"
#include "parse/test_preprocessor.h"
#include "parse/test_preprocessor_macros.h"
#include "parse/test_project_audit.h"
#include "parse/test_refactor.h"
#include "parse/test_refactor_api_sync.h"
#include "parse/test_refactor_api_sync_branches.h"
#include "parse/test_refactor_orchestrator.h"
#include "parse/test_schema_constraints.h"
#include "parse/test_schema_enum_required.h"
#include "parse/test_simple_json.h"
#include "parse/test_str_utils.h"
#include "parse/test_tokenizer.h"
#include "parse/test_tokenizer_trigraphs.h"

/* New Suites */
#include "../transformers/gnu_standardizer/test_gnu_standardizer_internals.h"
#include "../transformers/gnu_standardizer/test_gnu_standardizer_internals_coverage.h"
#include "emit/test_rewriter_body.h"
#include "parse/test_anonymous.h"
#include "parse/test_arrays_object.h"
#include "parse/test_arrays_primitive.h"
#include "parse/test_c2openapi_op.h"
#include "parse/test_c2openapi_op_responses.h"
#include "parse/test_c2openapi_op_coverage.h"
#include "parse/test_c2openapi_schema.h"
#include "parse/test_cli_parser.h"
#include "parse/test_code2schema_coverage.h"
#include "parse/test_code2schema_internals.h"
#include "parse/test_code2schema_internals_writers.h"
#include "parse/test_code2schema_internals_branches.h"
#include "parse/test_code2schema_internals_exhaustive_p1.h"
#include "parse/test_code2schema_internals_exhaustive_p2.h"
#include "parse/test_code2schema_internals_exhaustive_p3.h"
#include "parse/test_code2schema_internals_exhaustive_p4.h"
#include "parse/test_code2schema_internals_reach.h"
#include "parse/test_integration_c2openapi.h"
#include "parse/test_integration_c2openapi_cli.h"
#include "parse/test_main_coverage.h"
#include "parse/test_orchestrator_coverage.h"
#include "parse/test_preprocessor_internals.h"
#include "parse/test_query_projection.h"

#include "cdd_test_helpers/test_mock_server.h"
#include "emit/test_diff.h"
#include "emit/test_safe_crt.h"
#include "parse/test_cli_c2openapi.h"
#include "parse/test_fs_coverage.h"

#include "parse/test_cdd_cst_cfg.h"
#include "parse/test_cdd_cst_escape.h"
#include "parse/test_cdd_cst_scope.h"
#include "parse/test_cdd_cst_semantic.h"
#include "parse/test_cdd_cst_semantic_branches.h"
#include "parse/test_cdd_cst_type_eval.h"

#include "emit/test_aggregator.h"
#include "emit/test_cli_gen.h"
#include "emit/test_client_gui_gen.h"
#include "emit/test_openapi_writer.h"
#include "emit/test_operation.h"
#include "emit/test_serve_json_rpc.h"
#include "emit/test_server_gen.h"
/* #include "parse/test_c2openapi_op.h" */
#include "emit/test_codegen_sdk_tests.h"
#include "parse/test_c_mapping.h"
#include "parse/test_doc_parser.h"
#include "parse/test_doc_parser_coverage.h"
#include "parse/test_doc_parser_boost.h"
#include "parse/test_macro_overlay.h"
#include "parse/test_main.h"
#include "parse/test_query_projection.h"
#include "parse/test_to_docs_json.h"

#include "parse/test_cdd_cst_builder.h"
#include "parse/test_cdd_cst_builder_exhaustive.h"
#include "parse/test_cdd_cst_builder_branches.h"
#include "parse/test_cdd_cst_factory.h"
#include "parse/test_cli_cst.h"

#include "c_cdd/test_int128.h"
#include "test_cdd_api.h"
#include "test_log.h"
#include "test_cdd_c_error.h"
#include "transformers/safe_crt/test_safe_crt.h"
#include "transformers/safe_crt/test_safe_crt_branches.h"
#include "transformers/safe_crt/test_safe_crt_internals.h"
#include "transformers/safe_crt/test_safe_crt_internals_coverage.h"
/* clang-format on */

GREATEST_MAIN_DEFS();

extern cdd_c_error_t dummy_client(void);

TEST test_cdd_helpers(void) {
  cdd_precondition_failed();
  printf("write_to_file(NULL, NULL) = %d\\n", write_to_file(NULL, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, write_to_file(NULL, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, write_to_file("foo.txt", NULL));
  ASSERT_NEQ(CDD_C_SUCCESS,
             write_to_file("/invalid/path/that/cannot/exist/ever.txt", "abc"));

#include "cdd_test_helpers_export.h"

  /* Moved extern declarations for C89 compliance */
  {
    extern C_CDD_EXPORT int g_struct_fields_add_fail;

    extern C_CDD_EXPORT int g_cdd_cst_parser_fast_grow;
    extern C_CDD_EXPORT int g_cdd_query_err_fail;
    extern C_CDD_EXPORT int g_schema_realloc_fail;

    extern C_CDD_EXPORT int g_json_object_to_struct_fields_fail;
    extern C_CDD_EXPORT int g_safe_crt_malloc_fail;
    extern C_CDD_EXPORT int g_msvc_port_bld_fail;
    extern C_CDD_EXPORT int g_cdd_type_eval_ptr_fail;
    extern C_CDD_EXPORT int g_cdd_fprintf_fail;
    extern C_CDD_EXPORT int g_cdd_ffi_ir_calloc_fail;
    extern C_CDD_EXPORT int g_cdd_cst_emit_realloc_fail;

    extern C_CDD_EXPORT int g_enum_members_init_fail;

    extern C_CDD_EXPORT int g_cdd_lexer_id_fail;
    extern C_CDD_EXPORT int g_cdd_cfg_alloc_fail;

    extern C_CDD_EXPORT int g_err_perc_fail;
    extern C_CDD_EXPORT int g_cdd_cst_realloc_fail;
    extern C_CDD_EXPORT int g_fail_io_after;
    extern C_CDD_EXPORT int g_struct_fields_init_fail;
    extern C_CDD_EXPORT int g_enum_members_add_strdup_fail;
    extern C_CDD_EXPORT int g_cdd_ffi_ir_toposort_fail;

    extern C_CDD_EXPORT int g_cdd_semantic_leave_fail;
    extern C_CDD_EXPORT int g_schema_codegen_force_fail;
    extern C_CDD_EXPORT int g_cdd_ffi_ir_malloc_fail;
    extern C_CDD_EXPORT int g_io_calls;

    extern C_CDD_EXPORT int g_cdd_lexer_trivia_fail;
    extern C_CDD_EXPORT int g_schema_strdup_fail;

    extern C_CDD_EXPORT int g_cdd_helpers_fopen_err;

    extern C_CDD_EXPORT int g_enum_members_add_fail;

    extern C_CDD_EXPORT int g_cdd_lexer_id2_fail;

    /* extern C_CDD_EXPORT int g_cdd_helpers_fopen_err; (moved to global) */
    g_io_calls = 0;
    g_fail_io_after = 1;
    g_cdd_helpers_fopen_err = ENOENT;
    ASSERT_EQ(CDD_C_ERROR_NOT_FOUND, write_to_file("test_helpers.txt", "abc"));

    g_io_calls = 0;
    g_fail_io_after = 1;
    g_cdd_helpers_fopen_err = ENOMEM;
    {
      cdd_c_error_t rc = write_to_file("test_helpers.txt", "abc");
      printf("write_to_file ENOMEM test got: %d, g_io_calls: %d, errno: %d\n",
             rc, g_io_calls, errno);
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

    g_io_calls = 0;
    g_fail_io_after = -1; /* Success case where both FPUTS and FCLOSE succeed */
    ASSERT_EQ(CDD_C_SUCCESS, write_to_file("test_helpers.txt", "abc"));
    remove("test_helpers.txt");

    {
      cdd_c_error_t dummy_rc = dummy_client();
      ASSERT_EQ(CDD_C_SUCCESS, dummy_rc);
    }

    g_fail_io_after = -1;
    PASS();
  }
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
  RUN_TEST(test_ffi_extractor_missing_branches);
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
  RUN_TEST(test_ffi_ir_extract_inheritance_e2e);
  RUN_TEST(test_ffi_ir_extract_template_instantiation);
  RUN_TEST(test_ffi_ir_extract_docstring_and_sal_intents);
  RUN_TEST(test_ffi_ir_extract_trampolines_direct);
  RUN_TEST(test_ffi_ir_extractor_helpers);
}

#ifdef CDD_BUILD_TESTS
#endif

extern CDD_TEST_HELPERS_EXPORT int g_accept_fail;
extern CDD_TEST_HELPERS_EXPORT int g_bind_fail;
extern CDD_TEST_HELPERS_EXPORT int g_listen_fail;
extern CDD_TEST_HELPERS_EXPORT int g_getsockname_fail;
extern CDD_TEST_HELPERS_EXPORT int g_pthread_create_fail;
extern CDD_TEST_HELPERS_EXPORT int g_socket_fail;

static void reset_mocks(void) {
  /*  (moved to global) */
  g_accept_fail = 0;
  /*  (moved to global) */
  g_bind_fail = 0;
  g_listen_fail = 0;
  g_getsockname_fail = 0;
  g_pthread_create_fail = 0;
  g_socket_fail = 0;
  /*  (moved to global) */
  g_fail_io_after = -1;
  g_io_calls = 0;
  g_cdd_alloc_fail = 0;
  g_cdd_pp_file_exists_fail = 0;
  g_cdd_pp_match_fail = 0;
  g_cdd_pp_skip_ws_fail = 0;
  g_cdd_pp_resolve_path_fail = 0;
  g_cdd_pp_eval_expr_fail = 0;
  g_cdd_pp_token_to_string_fail = 0;
  g_cdd_pp_is_defined_macro_fail = 0;
  g_cdd_pp_peek_fail = 0;
  g_cdd_pp_primary_fail = 0;
  g_cdd_pp_unary_fail = 0;
  g_cdd_pp_multiplicative_fail = 0;
  g_cdd_pp_additive_fail = 0;
  g_cdd_pp_shift_fail = 0;
  g_cdd_pp_relational_fail = 0;
  g_cdd_pp_equality_fail = 0;
  g_cdd_pp_logic_and_fail = 0;
  g_cdd_pp_context_init_fail = 0;
  g_cdd_pp_scan_defines_fail = 0;
  g_cdd_fail_token_matches_string = 0;
  g_cdd_fail_identify_keyword_or_id = 0;
  {
    extern C_CDD_EXPORT int g_cdd_audit_fail_tokenize;
    extern C_CDD_EXPORT int g_cdd_audit_fail_find;
    extern C_CDD_EXPORT int g_cdd_fail_alloc_audit;
    g_cdd_audit_fail_tokenize = 0;
    g_cdd_audit_fail_find = 0;
    g_cdd_fail_alloc_audit = 0;
  }
  g_schema_codegen_force_fail = 0;
  g_schema_io_calls = 0;
  g_schema_fail_io_after = -1;
  g_cdd_fail_asprintf = 0;
  /* extern C_CDD_EXPORT int g_cdd_cfg_alloc_fail; (moved to global) */
  g_cdd_cfg_alloc_fail = 0;
  /*  (moved to global) */
  g_cdd_cst_alloc_node_fail = 0;
  /* extern  (moved to global) */
  g_cdd_cst_alloc_token_fail = 0;
  /* extern C_CDD_EXPORT int g_cdd_cst_emit_realloc_fail; (moved to global) */
  g_cdd_cst_emit_realloc_fail = 0;
  /* extern C_CDD_EXPORT int g_cdd_cst_parser_fast_grow; (moved to global) */
  g_cdd_cst_parser_fast_grow = 0;
  /* extern C_CDD_EXPORT int g_cdd_cst_realloc_fail; (moved to global) */
  g_cdd_cst_realloc_fail = 0;
  /* extern C_CDD_EXPORT int g_cdd_ffi_ir_calloc_fail; (moved to global) */
  g_cdd_ffi_ir_calloc_fail = 0;
  /* extern C_CDD_EXPORT int g_cdd_ffi_ir_malloc_fail; (moved to global) */
  g_cdd_ffi_ir_malloc_fail = 0;
  /* extern C_CDD_EXPORT int g_cdd_ffi_ir_toposort_fail; (moved to global) */
  g_cdd_ffi_ir_toposort_fail = 0;
  /* extern C_CDD_EXPORT int g_cdd_fprintf_fail; (moved to global) */
  g_cdd_fprintf_fail = 0;
  /* extern C_CDD_EXPORT int g_cdd_lexer_id2_fail; (moved to global) */
  g_cdd_lexer_id2_fail = 0;
  /* extern C_CDD_EXPORT int g_cdd_lexer_id_fail; (moved to global) */
  g_cdd_lexer_id_fail = 0;
  /* extern C_CDD_EXPORT int g_cdd_lexer_trivia_fail; (moved to global) */
  g_cdd_lexer_trivia_fail = 0;
  /* extern C_CDD_EXPORT int g_cdd_query_err_fail; (moved to global) */
  g_cdd_query_err_fail = 0;
  /*  (moved to global) */
  g_cdd_scope_alloc_fail = 0;
  /* extern C_CDD_EXPORT int g_cdd_semantic_leave_fail; (moved to global) */
  g_cdd_semantic_leave_fail = 0;
  /* extern C_CDD_EXPORT int g_cdd_strdup_fail; (moved to global) */
  g_cdd_strdup_fail = 0;
  /* extern C_CDD_EXPORT int g_cdd_type_eval_ptr_fail; (moved to global) */
  g_cdd_type_eval_ptr_fail = 0;
  /* extern C_CDD_EXPORT int g_enum_members_add_fail; (moved to global) */
  g_enum_members_add_fail = 0;
  /* extern C_CDD_EXPORT int g_enum_members_add_strdup_fail; (moved to global)
   */
  g_enum_members_add_strdup_fail = 0;
  /* extern C_CDD_EXPORT int g_enum_members_init_fail; (moved to global) */
  g_enum_members_init_fail = 0;
  /* extern C_CDD_EXPORT int g_err_perc_fail; (moved to global) */
  g_err_perc_fail = 0;
  {
    extern C_CDD_EXPORT volatile int g_extern_c_bot_node_fail;
    g_extern_c_bot_node_fail = 0;
    {
      extern C_CDD_EXPORT volatile int g_extern_c_helper_fail;
      g_extern_c_helper_fail = 0;
      {
        extern C_CDD_EXPORT volatile int g_extern_c_top_node_fail;
        g_extern_c_top_node_fail = 0;
        {
          extern C_CDD_EXPORT volatile int g_ffi_extractor_alloc_fail;
          g_ffi_extractor_alloc_fail = 0;
          /* extern C_CDD_EXPORT int g_force_find_allocations_fail; (moved to
           * global) */
          g_force_find_allocations_fail = 0;
          /* extern C_CDD_EXPORT int g_force_gnu_alloc_fail; (moved to global)
           */
          g_force_gnu_alloc_fail = 0;
          /* extern C_CDD_EXPORT int g_force_parse_tokens_fail; (moved to
           * global) */
          g_force_parse_tokens_fail = 0;
          /* extern C_CDD_EXPORT int g_force_strdup_fail; (moved to global) */
          g_force_strdup_fail = 0;
          /* extern C_CDD_EXPORT int g_force_tokenize_fail; (moved to global) */
          g_force_tokenize_fail = 0;
          /*  (moved to global) */
          g_getsockname_fail = 0;
          /* extern C_CDD_EXPORT int g_json_object_to_struct_fields_fail; (moved
           * to global)
           */
          g_json_object_to_struct_fields_fail = 0;
          /*  (moved to global) */
          g_listen_fail = 0;
          /* extern C_CDD_EXPORT int g_msvc_port_bld_fail; (moved to global) */
          g_msvc_port_bld_fail = 0;
          /*  (moved to global) */
          g_pthread_create_fail = 0;
          /* extern C_CDD_EXPORT int g_safe_crt_malloc_fail; (moved to global)
           */
          g_safe_crt_malloc_fail = 0;
          /* extern C_CDD_EXPORT int g_schema_codegen_force_fail; (moved to
           * global) */
          g_schema_codegen_force_fail = 0;
          /* extern C_CDD_EXPORT int g_schema_realloc_fail; (moved to global) */
          g_schema_realloc_fail = 0;
          /* extern C_CDD_EXPORT int g_schema_strdup_fail; (moved to global) */
          g_schema_strdup_fail = 0;
          /*  (moved to global) */
          g_socket_fail = 0;
          /* extern C_CDD_EXPORT int g_str_unquote_malloc_fail; (moved to
           * global) */
          g_str_unquote_malloc_fail = 0;
          /* extern C_CDD_EXPORT int g_struct_fields_add_fail; (moved to global)
           */
          g_struct_fields_add_fail = 0;
          /* extern C_CDD_EXPORT int g_struct_fields_init_fail; (moved to
           * global) */
          g_struct_fields_init_fail = 0;
          /* extern C_CDD_EXPORT int g_io_calls; (moved to global) */
          g_io_calls = 0;
        }
      }
    }
  }
}

static void test_teardown_cb(void *udata) {
  (void)udata;
  reset_mocks();
}

#define RUN_SUITE_RESET(suite)                                                 \
  do {                                                                         \
    reset_mocks();                                                             \
    RUN_SUITE(suite);                                                          \
  } while (0)

int main(int argc, char **argv) {
  setvbuf(stdout, NULL, _IONBF, 0);
  GREATEST_MAIN_BEGIN();

  test_teardown_cb(NULL);
  {
    FILE *f_cov;
    g_fail_io_after = 0;
    f_cov = mock_tmpfile_fuzzer();
    if (f_cov)
      fclose(f_cov);
    g_fail_io_after = -1;
    f_cov = mock_tmpfile_fuzzer();
    if (f_cov)
      fclose(f_cov);
  }

  SET_TEARDOWN(test_teardown_cb, NULL);

  srand((unsigned int)time(NULL));

  {
    cdd_cst_tree_t *tree = NULL;
    const char snippet[] = {
        '#',  'i',  'f', 'd',  'e',  'f',  ' ',  'A',  '\n', '#',  'e',  'l',
        'i',  'f',  ' ', 'B',  '\n', '#',  'e',  'l',  's',  'e',  '\n', '{',
        ' ',  'i',  'n', 't',  ' ',  'z',  '1',  ';',  ' ',  '}',  '\n', '{',
        ' ',  'i',  'n', 't',  ' ',  'z',  '2',  ';',  ' ',  '}',  '\n', '{',
        ' ',  'i',  'n', 't',  ' ',  'z',  '3',  ';',  ' ',  '}',  '\n', '{',
        ' ',  'i',  'n', 't',  ' ',  'z',  '4',  ';',  ' ',  '}',  '\n', '{',
        ' ',  'i',  'n', 't',  ' ',  'z',  '5',  ';',  ' ',  '}',  '\n', '{',
        ' ',  'i',  'n', 't',  ' ',  'z',  '6',  ';',  ' ',  '}',  '\n', '{',
        ' ',  'i',  'n', 't',  ' ',  'z',  '7',  ';',  ' ',  '}',  '\n', '{',
        ' ',  'i',  'n', 't',  ' ',  'z',  '8',  ';',  ' ',  '}',  '\n', '{',
        ' ',  'i',  'n', 't',  ' ',  'z',  '9',  ';',  ' ',  '}',  '\n', '{',
        ' ',  'i',  'n', 't',  ' ',  'z',  '1',  '0',  ';',  ' ',  '}',  '\n',
        '#',  'e',  'n', 'd',  'i',  'f',  '\n', '#',  'i',  'f',  'n',  'd',
        'e',  'f',  ' ', 'C',  '\n', '{',  ' ',  'i',  'n',  't',  ' ',  'w',
        ';',  ' ',  '}', '\n', '#',  'e',  'n',  'd',  'i',  'f',  '\n', '#',
        'd',  'e',  'f', 'i',  'n',  'e',  ' ',  'D',  ' ',  '1',  '\n', '#',
        'i',  'n',  'c', 'l',  'u',  'd',  'e',  ' ',  '<',  's',  't',  'd',
        'i',  'o',  '.', 'h',  '>',  '\n', '#',  'p',  'r',  'a',  'g',  'm',
        'a',  ' ',  'o', 'n',  'c',  'e',  '\n', 't',  'e',  'm',  'p',  'l',
        'a',  't',  'e', ' ',  '<',  't',  'y',  'p',  'e',  'n',  'a',  'm',
        'e',  ' ',  'T', '1',  ',',  ' ',  't',  'y',  'p',  'e',  'n',  'a',
        'm',  'e',  ' ', 'T',  '2',  ',',  ' ',  't',  'y',  'p',  'e',  'n',
        'a',  'm',  'e', ' ',  'T',  '3',  ',',  ' ',  't',  'y',  'p',  'e',
        'n',  'a',  'm', 'e',  ' ',  'T',  '4',  ',',  ' ',  't',  'y',  'p',
        'e',  'n',  'a', 'm',  'e',  ' ',  'T',  '5',  ',',  ' ',  't',  'y',
        'p',  'e',  'n', 'a',  'm',  'e',  ' ',  'T',  '6',  ',',  ' ',  't',
        'y',  'p',  'e', 'n',  'a',  'm',  'e',  ' ',  'T',  '7',  ',',  ' ',
        't',  'y',  'p', 'e',  'n',  'a',  'm',  'e',  ' ',  'T',  '8',  ',',
        ' ',  't',  'y', 'p',  'e',  'n',  'a',  'm',  'e',  ' ',  'T',  '9',
        ',',  ' ',  't', 'y',  'p',  'e',  'n',  'a',  'm',  'e',  ' ',  'T',
        '1',  '0',  '>', '\n', 'c',  'l',  'a',  's',  's',  ' ',  'F',  'o',
        'o',  ' ',  ':', ' ',  'p',  'u',  'b',  'l',  'i',  'c',  ' ',  'B',
        'a',  'r',  ',', ' ',  'p',  'r',  'i',  'v',  'a',  't',  'e',  ' ',
        'B',  'a',  'z', ' ',  '{',  '\n', 'p',  'u',  'b',  'l',  'i',  'c',
        ':',  '\n', ' ', ' ',  'v',  'o',  'i',  'd',  ' ',  'b',  'a',  'z',
        '(',  ')',  ' ', 'n',  'o',  'e',  'x',  'c',  'e',  'p',  't',  '(',
        't',  'r',  'u', 'e',  ')',  ' ',  '{',  '}',  '\n', ' ',  ' ',  '~',
        'F',  'o',  'o', '(',  ')',  ';',  '\n', ' ',  ' ',  'i',  'n',  't',
        ' ',  'o',  'p', 'e',  'r',  'a',  't',  'o',  'r',  '+',  '(',  'i',
        'n',  't',  ')', ';',  '\n', 'p',  'r',  'o',  't',  'e',  'c',  't',
        'e',  'd',  ':', '\n', ' ',  ' ',  'i',  'n',  't',  ' ',  'x',  ';',
        '\n', 'p',  'r', 'i',  'v',  'a',  't',  'e',  ':',  '\n', ' ',  ' ',
        'i',  'n',  't', ' ',  'y',  ';',  '\n', '}',  ';',  '\n', 'n',  'a',
        'm',  'e',  's', 'p',  'a',  'c',  'e',  ' ',  'N',  ' ',  '{',  '\n',
        ' ',  ' ',  'u', 's',  'i',  'n',  'g',  ' ',  'n',  'a',  'm',  'e',
        's',  'p',  'a', 'c',  'e',  ' ',  's',  't',  'd',  ';',  '\n', ' ',
        ' ',  'v',  'o', 'i',  'd',  ' ',  'f',  '(',  ')',  ' ',  '{',  '\n',
        ' ',  ' ',  ' ', ' ',  't',  'r',  'y',  ' ',  '{',  '\n', ' ',  ' ',
        ' ',  ' ',  ' ', ' ',  't',  'h',  'r',  'o',  'w',  ' ',  '1',  ';',
        '\n', ' ',  ' ', ' ',  ' ',  '}',  ' ',  'c',  'a',  't',  'c',  'h',
        ' ',  '(',  'i', 'n',  't',  ' ',  'e',  ')',  ' ',  '{',  '\n', ' ',
        ' ',  ' ',  ' ', '}',  ' ',  'c',  'a',  't',  'c',  'h',  ' ',  '(',
        '.',  '.',  '.', ')',  ' ',  '{',  '\n', ' ',  ' ',  ' ',  ' ',  '}',
        '\n', ' ',  ' ', '}',  '\n', '}',  '\n', 'i',  'n',  't',  ' ',  'm',
        'a',  'i',  'n', '(',  ')',  ' ',  '{',  ' ',  'a',  's',  'm',  '(',
        '"',  'n',  'o', 'p',  '"',  ')',  ';',  ' ',  'r',  'e',  't',  'u',
        'r',  'n',  ' ', '0',  ';',  ' ',  '}',  '\n', '\0'};
    cdd_c_error_t rc =
        cdd_cst_parse(az_span_create_from_str((char *)(size_t)snippet), &tree);
    printf("PARSE RC = %d, num_children = %lu, capacity = %lu\n", rc,
           (unsigned long)tree->root->num_children,
           (unsigned long)tree->root->capacity);
    if (tree)
      cdd_cst_tree_free(tree);
  }

#if defined(_MSC_VER) && _MSC_VER <= 1400
  /* skipped */
#else

#ifdef C_CDD_USE_LIBCURL
#endif

  /* New Runners */
  RUN_SUITE_RESET(cdd_cst_trivia_suite);
  RUN_SUITE_RESET(cst_parser_suite);
  RUN_SUITE_RESET(preprocessor_macros_suite);
  RUN_SUITE_RESET(orchestrator_internals_suite);
  RUN_SUITE_RESET(analysis_suite);
  RUN_SUITE_RESET(main_suite);
  RUN_SUITE_RESET(operation_suite);
  RUN_SUITE_RESET(code2schema_suite);
  RUN_SUITE_RESET(code2schema_helpers_suite);
  RUN_SUITE_RESET(fs_suite);
  RUN_SUITE_RESET(cdd_api_suite);
  RUN_SUITE_RESET(decl_hoist_suite);
  RUN_SUITE_RESET(cdd_cst_suite);
  RUN_SUITE_RESET(cdd_cst_cfg_suite);
  RUN_SUITE_RESET(tokenizer_suite);
  RUN_SUITE_RESET(vcpkg_integration_suite);
  RUN_SUITE_RESET(cdd_cst_semantic_suite);
  RUN_SUITE_RESET(cdd_cst_semantic_branches_suite);
  RUN_SUITE_RESET(initializer_parser_suite);
  RUN_SUITE_RESET(cdd_cst_type_eval_suite);
  RUN_SUITE_RESET(cdd_cst_escape_suite);
  RUN_SUITE_RESET(desig_init_suite);
  RUN_SUITE_RESET(pragma_suite);
  RUN_SUITE_RESET(makefile_scraper_suite);
  RUN_SUITE_RESET(api_sync_suite);
  RUN_SUITE_RESET(api_sync_branches_suite);
  RUN_SUITE_RESET(strategy_suite);
  RUN_SUITE_RESET(tokenizer_trigraphs_suite);
  RUN_SUITE_RESET(refactor_suite);
  RUN_SUITE_RESET(to_docs_json_suite);
  RUN_SUITE_RESET(cdd_cst_factory_suite);
  RUN_SUITE_RESET(cdd_lexer_suite);
  RUN_SUITE_RESET(cdd_cst_scope_suite);
  RUN_SUITE_RESET(project_audit_suite);
  RUN_SUITE_RESET(json_from_and_to_suite);
  RUN_SUITE_RESET(declarator_parser_suite);
  RUN_SUITE_RESET(declarator_parser_helpers_suite);
  RUN_SUITE_RESET(macro_overlay_suite);
  RUN_SUITE_RESET(refactor_orchestrator_suite);
  RUN_SUITE_RESET(c_mapping_suite);
  RUN_SUITE_RESET(db_loader_suite);
  RUN_SUITE_RESET(schema_enum_required_suite);
  RUN_SUITE_RESET(cmake_parser_suite);
  RUN_SUITE_RESET(cdd_cst_mutate_suite);
  RUN_SUITE_RESET(flexible_array_suite);
  RUN_SUITE_RESET(cdd_cst_query_suite);
  RUN_SUITE_RESET(preprocessor_suite);
  RUN_SUITE_RESET(schema_constraints_suite);
  RUN_SUITE_RESET(cli_cst_suite);
  RUN_SUITE_RESET(openapi_loader_suite);
  RUN_SUITE_RESET(numeric_parser_suite);
  RUN_SUITE_RESET(str_utils_suite);
  RUN_SUITE_RESET(c_inspector_types_suite);
  RUN_SUITE_RESET(doc_parser_suite);
  RUN_SUITE_RESET(doc_parser_coverage_suite);
  RUN_SUITE_RESET(doc_parser_boost_suite);
  RUN_SUITE_RESET(dataclasses_suite);
  RUN_SUITE_RESET(vla_analyzer_suite);
  RUN_SUITE_RESET(integration_suite);
  RUN_SUITE_RESET(crypto_suite);
  RUN_SUITE_RESET(c_cdd_int128_suite);
  RUN_SUITE_RESET(transformer_msvc_port_suite);
  RUN_SUITE_RESET(transformer_extern_c_suite);
  RUN_SUITE_RESET(transformer_extern_c_coverage_suite);
  RUN_SUITE_RESET(transformer_macros_suite);
  RUN_SUITE_RESET(transformer_error_percolator_suite);
  RUN_SUITE_RESET(transformer_error_percolator_call_sites_suite);
  RUN_SUITE_RESET(cdd_ffi_ir_suite);
  RUN_SUITE_RESET(ffi_variadic_suite);
  RUN_SUITE_RESET(ffi_emitters_suite);
  RUN_SUITE_RESET(ffi_emitters_edge_suite);
  RUN_SUITE_RESET(cli_gen_suite);
  RUN_SUITE_RESET(codegen_json_suite);
  RUN_SUITE_RESET(codegen_json_comprehensive_suite);
  RUN_SUITE_RESET(text_patcher_suite);
  RUN_SUITE_RESET(text_patcher_oom_suite);
  RUN_SUITE_RESET(client_sig_suite);
  RUN_SUITE_RESET(client_sig_media_suite);
  RUN_SUITE_RESET(client_sig_params_suite);
  RUN_SUITE_RESET(client_sig_helpers_suite);
  RUN_SUITE_RESET(client_sig_branches_suite);
  RUN_SUITE_RESET(client_sig_coverage_suite);
  RUN_SUITE_RESET(client_sig_ultra_suite);
  RUN_SUITE_RESET(sync_code_suite);
  RUN_SUITE_RESET(codegen_enum_suite);
  RUN_SUITE_RESET(codegen_eq_suite);
  RUN_SUITE_RESET(rewriter_sig_suite);
  RUN_SUITE_RESET(client_gui_gen_suite);
  RUN_SUITE_RESET(codegen_jwt_suite);
  RUN_SUITE_RESET(cdd_cst_emit_unit_suite);
  RUN_SUITE_RESET(codegen_struct_suite);
  RUN_SUITE_RESET(codegen_struct_io_suite);
  RUN_SUITE_RESET(weaver_suite);
  RUN_SUITE_RESET(standalone_json_suite);
  RUN_SUITE_RESET(aggregator_suite);
  RUN_SUITE_RESET(codegen_defaults_suite);
  RUN_SUITE_RESET(codegen_sdk_tests_suite);
  RUN_SUITE_RESET(safe_crt_suite);
  RUN_SUITE_RESET(diff_suite);
  RUN_SUITE_RESET(c_cdd_mock_server_suite);
  RUN_SUITE_RESET(cli_c2openapi_suite);
  RUN_SUITE_RESET(rewriter_body_suite);
  RUN_SUITE_RESET(cdd_cst_builder_suite);
  RUN_SUITE_RESET(cdd_cst_builder_exhaustive_suite);
  RUN_SUITE_RESET(cdd_cst_builder_branches_suite);
  RUN_SUITE_RESET(cdd_helpers_suite);
  RUN_SUITE_RESET(diff_generator_suite);
  RUN_SUITE_RESET(ffi_extractor_suite);
  RUN_SUITE_RESET(parsing_suite);
  RUN_SUITE_RESET(simple_mocks_suite);
  RUN_SUITE_RESET(transformer_gnu_standardizer_suite);
  RUN_SUITE_RESET(transformer_gnu_standardizer_branches_suite);
  RUN_SUITE_RESET(transformer_safe_crt_suite);
  RUN_SUITE_RESET(transformer_safe_crt_branches_suite);
  RUN_SUITE_RESET(transformer_safe_crt_internals_suite);
  RUN_SUITE_RESET(transformer_safe_crt_internals_coverage_suite);
  RUN_SUITE_RESET(root_array_suite);
  RUN_SUITE_RESET(generate_build_system_suite);
  RUN_SUITE_RESET(codegen_make_suite);
  RUN_SUITE_RESET(server_gen_suite);
  RUN_SUITE_RESET(cst_printer_suite);
  RUN_SUITE_RESET(schema2tests_suite);
  RUN_SUITE_RESET(codegen_url_suite);
  RUN_SUITE_RESET(codegen_url_objects_suite);
  RUN_SUITE_RESET(codegen_url_internals_suite);
  RUN_SUITE_RESET(codegen_url_builder_suite);
  RUN_SUITE_RESET(codegen_url_io_failures_suite);
  RUN_SUITE_RESET(codegen_validation_suite);
  RUN_SUITE_RESET(serve_json_rpc_suite);
  RUN_SUITE_RESET(codegen_form_suite);
  RUN_SUITE_RESET(codegen_build_suite);
  RUN_SUITE_RESET(codegen_types_suite);
  RUN_SUITE_RESET(codegen_types_exhaustive_suite);
  RUN_SUITE_RESET(codegen_types_uncovered_suite);
  RUN_SUITE_RESET(openapi_client_gen_suite);
  RUN_SUITE_RESET(openapi_client_gen_helpers_suite);
  RUN_SUITE_RESET(openapi_client_gen_mocks_suite);
  RUN_SUITE_RESET(openapi_client_gen_expanded_suite);
  RUN_SUITE_RESET(codegen_oauth2_error_suite);
  RUN_SUITE_RESET(client_body_suite);
  RUN_SUITE_RESET(client_body_headers_suite);
  RUN_SUITE_RESET(client_body_forms_suite);
  RUN_SUITE_RESET(client_body_responses_suite);
  RUN_SUITE_RESET(client_body_media_suite);
  RUN_SUITE_RESET(client_body_joined_suite);
  RUN_SUITE_RESET(client_body_types_suite);
  RUN_SUITE_RESET(client_body_mega_suite);
  RUN_SUITE_RESET(client_body_internals_suite);
  RUN_SUITE_RESET(openapi_writer_suite);
  RUN_SUITE_RESET(url_utils_suite);
  RUN_SUITE_RESET(schema_codegen_suite);
  RUN_SUITE_RESET(schema_codegen_cli_suite);
  RUN_SUITE_RESET(codegen_security_suite);
  RUN_SUITE_RESET(codegen_security_nulls_suite);
  RUN_SUITE_RESET(arrays_primitive_suite);
  RUN_SUITE_RESET(arrays_object_suite);
  RUN_SUITE_RESET(anonymous_suite);
  RUN_SUITE_RESET(preprocessor_internals_suite);
  RUN_SUITE_RESET(code2schema_coverage_suite);
  RUN_SUITE_RESET(code2schema_internals_suite);
  RUN_SUITE_RESET(code2schema_internals_writers_suite);
  RUN_SUITE_RESET(code2schema_internals_branches_suite);
  RUN_SUITE_RESET(code2schema_internals_exhaustive_p1_suite);
  RUN_SUITE_RESET(code2schema_internals_exhaustive_p2_suite);
  RUN_SUITE_RESET(code2schema_internals_exhaustive_p3_suite);
  RUN_SUITE_RESET(code2schema_internals_exhaustive_p4_suite);
  RUN_SUITE_RESET(code2schema_internals_reach_suite);
  RUN_SUITE_RESET(main_coverage_suite);
  RUN_SUITE_RESET(log_suite);
  RUN_SUITE_RESET(integration_c2openapi_suite);
  RUN_SUITE_RESET(integration_c2openapi_cli_suite);
  RUN_SUITE_RESET(query_projection_suite);
  RUN_SUITE_RESET(cli_parser_suite);
  RUN_SUITE_RESET(orchestrator_coverage_suite);
  RUN_SUITE_RESET(fs_coverage_suite);
  RUN_SUITE_RESET(c2openapi_op_suite);
  RUN_SUITE_RESET(c2openapi_op_responses_suite);
  RUN_SUITE_RESET(c2openapi_op_coverage_suite);
  RUN_SUITE_RESET(c2openapi_schema_suite);
  RUN_SUITE_RESET(transformer_gnu_standardizer_internals_suite);
  RUN_SUITE_RESET(transformer_gnu_standardizer_internals_coverage_suite);
  RUN_SUITE_RESET(cdd_c_error_suite);
  reset_mocks();

#endif
  GREATEST_MAIN_END();
}

#if defined(__GNUC__) || defined(__clang__)
#endif
