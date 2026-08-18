#if defined(_MSC_VER)
#pragma warning(disable : 4189)
#pragma warning(disable : 4057)
#pragma warning(disable : 4101)
#pragma warning(disable : 4267)
#pragma warning(disable : 4456)
#pragma warning(disable : 5286)
#pragma warning(disable : 4210)
#pragma warning(disable : 4703)
#pragma warning(disable : 4244)
#endif
#if defined(_MSC_VER)
#pragma warning(                                                               \
    disable : 4189) /* local variable is initialized but not referenced */
#pragma warning(                                                               \
    disable : 4057) /* const uint8_t * differs in indirection to slightly      \
                       different base types from char [3] */
#pragma warning(disable : 4101) /* unreferenced local variable */
#pragma warning(                                                               \
    disable : 4267) /* conversion from size_t to int, possible loss of data */
#pragma warning(                                                               \
    disable : 4456) /* declaration of ... hides previous local declaration */
#pragma warning(disable : 5286) /* implicit conversion from enum type */
#pragma warning(disable : 4210) /* nonstandard extension used: function given  \
                                   file scope */
#endif
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
#include "c_cdd/format_specifiers.h"
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
#include "test_cdd_api.h"
#include "emit/test_cdd_cst_emit_unit.h"
#include "emit/test_cst_printer.h"
#ifdef CDD_BUILD_TESTS
extern C_CDD_EXPORT int g_cdd_alloc_fail;
#endif
#include "emit/test_codegen_build.h"

#include <stdio.h>
#include <c_cdd_export.h>
extern C_CDD_EXPORT int g_fail_io_after;
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
#include "transformers/safe_crt/test_safe_crt.h"
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
#include "cdd_test_helpers/test_mock_server.h"
/* #include "parse/test_integration_c2openapi.h" */
#include "parse/test_macro_overlay.h"
#include "parse/test_main.h"
#include "parse/test_to_docs_json.h"
#include "parse/test_cli_c2openapi.h"
#include "parse/test_cli_cst.h"
#include "parse/test_cdd_cst_builder.h"
#include "parse/test_cdd_cst_factory.h"

#include "c_cdd/test_int128.h"
#include "test_cdd_api.h"
/* clang-format on */

GREATEST_MAIN_DEFS();

TEST test_cdd_helpers(void) {
  cdd_precondition_failed();
  printf("write_to_file(NULL, NULL) = %d\\n", write_to_file(NULL, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, write_to_file(NULL, NULL));
  ASSERT_NEQ(CDD_C_SUCCESS,
             write_to_file("/invalid/path/that/cannot/exist/ever.txt", "abc"));

#include "cdd_test_helpers_export.h"
  extern CDD_TEST_HELPERS_EXPORT int g_cdd_helpers_fopen_err;
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

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(analysis_suite);

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

  /*  */

  /*  */
  /*  */

  /*   */

  GREATEST_MAIN_END();
}

#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif
