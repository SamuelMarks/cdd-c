#if defined(_MSC_VER)
#if defined(_MSC_VER)
#pragma warning(disable : 4005)
#endif
#endif
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeclaration-after-statement"
#pragma GCC diagnostic ignored "-Wpedantic"
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
extern C_CDD_EXPORT int g_fail_io_after;
extern C_CDD_EXPORT int g_io_calls;

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
#include "emit/test_cdd_cst_emit_unit.h"
#include "emit/test_codegen_build.h"
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
#include "transformers/safe_crt/test_safe_crt.h"
#include "emit/test_cst_printer.h"
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
#include "parse/test_schema_constraints.h"
#include "parse/test_schema_enum_required.h"
#include "parse/test_simple_json.h"
#include "parse/test_str_utils.h"
#include "parse/test_tokenizer.h"
#include "parse/test_tokenizer_trigraphs.h"

/* New Suites */
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
/* #include "parse/test_c2openapi_schema.h" */
#include "parse/test_c_mapping.h"
#include "parse/test_doc_parser.h"
/* #include "parse/test_integration_c2openapi.h" */
#include "parse/test_macro_overlay.h"
#include "parse/test_main.h"
#include "parse/test_to_docs_json.h"
#include "parse/test_cli_cst.h"
#include "parse/test_cdd_cst_builder.h"
#include "parse/test_cdd_cst_factory.h"

#include "c_cdd/test_int128.h"
/* clang-format on */

GREATEST_MAIN_DEFS();

TEST test_cdd_helpers(void) {
  cdd_precondition_failed();
  ASSERT_EQ(EXIT_FAILURE, write_to_file(NULL, NULL));
  ASSERT_EQ(EXIT_FAILURE,
            write_to_file("/invalid/path/that/cannot/exist/ever.txt", "abc"));
  g_fail_io_after = -1;
  PASS();
}

SUITE(cdd_helpers_suite) { RUN_TEST(test_cdd_helpers); }

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  srand((unsigned int)time(NULL));

  RUN_SUITE(analysis_suite);
  RUN_SUITE(code2schema_suite);
  RUN_SUITE(c_inspector_types_suite);
  RUN_SUITE(codegen_build_suite);
  RUN_SUITE(client_body_suite);
  RUN_SUITE(client_sig_suite);
  RUN_SUITE(codegen_defaults_suite);
  RUN_SUITE(codegen_enum_suite);
  RUN_SUITE(codegen_eq_suite);
  RUN_SUITE(codegen_json_suite);
  RUN_SUITE(standalone_json_suite);
  RUN_SUITE(codegen_form_suite);
  RUN_SUITE(codegen_jwt_suite);
  RUN_SUITE(codegen_oauth2_error_suite);
  RUN_SUITE(codegen_make_suite);
  RUN_SUITE(codegen_security_suite);
  RUN_SUITE(codegen_struct_suite);
  RUN_SUITE(codegen_types_suite);
  RUN_SUITE(codegen_url_suite);
  RUN_SUITE(codegen_validation_suite);
  RUN_SUITE(root_array_suite);
  RUN_SUITE(codegen_sdk_tests_suite);
  RUN_SUITE(cst_parser_suite);
  RUN_SUITE(cdd_lexer_suite);
  RUN_SUITE(cdd_cst_suite);
  RUN_SUITE(cdd_cst_mutate_suite);
  RUN_SUITE(cdd_cst_query_suite);
  RUN_SUITE(cdd_cst_trivia_suite);
  RUN_SUITE(cdd_cst_emit_unit_suite);
  RUN_SUITE(transformer_extern_c_suite);
  RUN_SUITE(transformer_msvc_port_suite);
  RUN_SUITE(transformer_gnu_standardizer_suite);
  RUN_SUITE(transformer_error_percolator_suite);
  RUN_SUITE(transformer_safe_crt_suite);
  RUN_SUITE(crypto_suite);
  RUN_SUITE(dataclasses_suite);
  RUN_SUITE(declarator_parser_suite);
  RUN_SUITE(decl_hoist_suite);
  RUN_SUITE(db_loader_suite);
  RUN_SUITE(desig_init_suite);
  RUN_SUITE(vla_analyzer_suite);
  RUN_SUITE(vcpkg_integration_suite);
  RUN_SUITE(cmake_parser_suite);
  RUN_SUITE(strategy_suite);
  RUN_SUITE(makefile_scraper_suite);

  RUN_SUITE(flexible_array_suite);
  RUN_SUITE(fs_suite);
  RUN_SUITE(generate_build_system_suite);
#ifdef C_CDD_USE_LIBCURL
#endif
  RUN_SUITE(initializer_parser_suite);
  RUN_SUITE(integration_suite);
  RUN_SUITE(json_from_and_to_suite);
  RUN_SUITE(numeric_parser_suite);
  RUN_SUITE(openapi_client_gen_suite);
  RUN_SUITE(openapi_loader_suite);
  RUN_SUITE(parsing_suite);
  RUN_SUITE(pragma_suite);
  RUN_SUITE(preprocessor_suite);
  RUN_SUITE(preprocessor_macros_suite);
  RUN_SUITE(project_audit_suite);
  RUN_SUITE(api_sync_suite);
  RUN_SUITE(refactor_orchestrator_suite);
  RUN_SUITE(refactor_suite);
  RUN_SUITE(rewriter_body_suite);
  RUN_SUITE(rewriter_sig_suite);
  RUN_SUITE(schema2tests_suite);
  RUN_SUITE(schema_codegen_suite);
  RUN_SUITE(schema_constraints_suite);
  RUN_SUITE(schema_enum_required_suite);
  RUN_SUITE(simple_mocks_suite);
  RUN_SUITE(str_utils_suite);
  RUN_SUITE(sync_code_suite);
  RUN_SUITE(weaver_suite);
  RUN_SUITE(text_patcher_suite);
  RUN_SUITE(cst_printer_suite);
  RUN_SUITE(diff_generator_suite);
  RUN_SUITE(tokenizer_suite);
  RUN_SUITE(tokenizer_trigraphs_suite);
  RUN_SUITE(url_utils_suite);

  /* New Runners */
  RUN_SUITE(cdd_cst_escape_suite);
  RUN_SUITE(cdd_cst_scope_suite);
  RUN_SUITE(cdd_cst_semantic_suite);
  RUN_SUITE(cdd_cst_cfg_suite);
  RUN_SUITE(cdd_cst_type_eval_suite);
  RUN_SUITE(cdd_cst_cfg_suite);
  RUN_SUITE(cdd_cst_type_eval_suite);

  RUN_SUITE(openapi_writer_suite);
  RUN_SUITE(operation_suite);
  RUN_SUITE(doc_parser_suite);
  RUN_SUITE(c_mapping_suite);
  /* RUN_SUITE(c2openapi_op_suite); */
  RUN_SUITE(aggregator_suite);
  /* RUN_SUITE(c2openapi_schema_suite); */
  /* RUN_SUITE(integration_c2openapi_suite); */
  RUN_SUITE(to_docs_json_suite);
  RUN_SUITE(main_suite);
  RUN_SUITE(cli_gen_suite);
  RUN_SUITE(cdd_helpers_suite);
  RUN_SUITE(client_gui_gen_suite);
  RUN_SUITE(macro_overlay_suite);
  RUN_SUITE(server_gen_suite);
  RUN_SUITE(serve_json_rpc_suite);
  RUN_SUITE(cli_cst_suite);
  RUN_SUITE(cdd_cst_factory_suite);

  RUN_SUITE(cdd_cst_builder_suite);
  RUN_SUITE(c_cdd_int128_suite);

  GREATEST_MAIN_END();
}

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif
