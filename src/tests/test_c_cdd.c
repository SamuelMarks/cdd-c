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



/** @brief Test definition g_fail_io_after. */
extern C_CDD_EXPORT int g_fail_io_after;
/** @brief Test definition g_io_calls. */
extern C_CDD_EXPORT int g_io_calls;
/** @brief External test variable `g_cdd_cst_emit_realloc_fail`. */
extern C_CDD_EXPORT int g_cdd_cst_emit_realloc_fail;
/** @brief External test variable `g_schema_strdup_fail`. */
extern C_CDD_EXPORT int g_schema_strdup_fail;
/** @brief External test variable `g_schema_realloc_fail`. */
extern C_CDD_EXPORT int g_schema_realloc_fail;
/** @brief External test variable `g_schema_fail_io_after`. */
extern C_CDD_EXPORT int g_schema_fail_io_after;

/** @brief Test definition g_force_gnu_alloc_fail. */
int g_force_gnu_alloc_fail = 0;
/** @brief Test definition g_force_parse_tokens_fail. */
extern C_CDD_EXPORT int g_force_parse_tokens_fail;
/** @brief Test definition g_force_find_allocations_fail. */
extern C_CDD_EXPORT int g_force_find_allocations_fail;
/** @brief Test definition g_force_strdup_fail. */
int g_force_strdup_fail = 0;
/** @brief Test definition g_force_tokenize_fail. */
extern C_CDD_EXPORT int g_force_tokenize_fail;

/** @brief Test definition g_cdd_wine_skip. */
int g_cdd_wine_skip = 0;

/** @brief External test variable `g_schema_io_calls`. */
extern C_CDD_EXPORT int g_schema_io_calls;
/** @brief External test variable `g_schema_codegen_force_fail`. */
extern C_CDD_EXPORT int g_schema_codegen_force_fail;
/** @brief External test variable `g_cdd_cst_realloc_fail`. */
extern C_CDD_EXPORT int g_cdd_cst_realloc_fail;
/** @brief External test variable `g_cdd_cst_alloc_token_fail`. */
extern C_CDD_EXPORT int g_cdd_cst_alloc_token_fail;
/** @brief External test variable `g_cdd_cst_alloc_node_fail`. */
extern C_CDD_EXPORT int g_cdd_cst_alloc_node_fail;
/** @brief External test variable `g_fail_io_after`. */
extern C_CDD_EXPORT int g_fail_io_after;
/** @brief External test variable `g_io_calls`. */
extern C_CDD_EXPORT int g_io_calls;
/** @brief External test variable `g_cdd_cst_emit_realloc_fail`. */
extern C_CDD_EXPORT int g_cdd_cst_emit_realloc_fail;
/** @brief External test variable `g_schema_strdup_fail`. */
extern C_CDD_EXPORT int g_schema_strdup_fail;
/** @brief External test variable `g_schema_realloc_fail`. */
extern C_CDD_EXPORT int g_schema_realloc_fail;
/** @brief External test variable `g_schema_fail_io_after`. */
extern C_CDD_EXPORT int g_schema_fail_io_after;




#ifndef ENOTSUP
/**
 * @brief Macro ENOTSUP.
 */
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
CDD_TEST_HELPERS_EXPORT char* cdd_test_mut_str(const char *str);
#ifndef C_CDD_STR_LIT
/**
 * @brief Macro C_CDD_STR_LIT.
 * @param[in] s Macro argument s.
 */
#define C_CDD_STR_LIT(s) cdd_test_mut_str(s)
#endif

#include <c89stringutils_string_extras.h>
#include <parson.h>
#include "cdd_test_helpers/cdd_crt_bridge.h"
#include "parse/test_simple_json.h"

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
/**
 * @brief Helper function mock_tmpfile_fuzzer.
 * @return Result status or value.
 */
static FILE *mock_tmpfile_fuzzer(void) {
    if (g_fail_io_after >= 0) {
        return fopen("/dev/null", "w+b");
    }
    return cdd_test_tmpfile_global();
}
#ifdef tmpfile
#undef tmpfile
#endif
/**
 * @brief Macro tmpfile.
 */
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
#include "emit/test_codegen_cli.h"
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
#include "ffi/test_ffi_ir_json.h"
#include "ffi/test_ffi_e2e.h"
#include "ffi/test_ffi_emitters.h"
#include "ffi/test_ffi_emitters_edge.h"
#include "ffi/test_ffi_ocaml_fortran_cobol.h"
#include "dsl/test_dsl_tokenizer.h"
#include "dsl/test_dsl_tokenizer_branches.h"
#include "dsl/test_dsl_tokenizer_macros.h"
#include "dsl/test_dsl_tokenizer_unknown_slash.h"
#include "dsl/test_dsl_tokenizer_eof_comment.h"
#include "dsl/test_dsl_parser.h"
#include "dsl/test_dsl_parser_branches.h"
#include "dsl/test_dsl_emitter.h"
#include "dsl/test_dsl_emitter_branches.h"
#include "dsl/test_dsl_integration.h"
#include "dsl/test_dsl_namespace.h"
#include "dsl/test_dsl_number.h"
#include "dsl/test_dsl_macros.h"
#include "dsl/test_dsl_modifiers.h"

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
#include "transformers/c_lowering/test_c_lowering.h"


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

#include "c_cdd/test_context.h"
#include "c_cdd/test_int128.h"
#include "test_cdd_api.h"
#include "test_log.h"
#include "test_cdd_c_error.h"
#include "transformers/safe_crt/test_safe_crt.h"
#include "transformers/safe_crt/test_safe_crt_branches.h"
#include "transformers/safe_crt/test_safe_crt_internals.h"
#include "transformers/safe_crt/test_safe_crt_internals_coverage.h"
#include "parse/test_arazzo_types.h"
#include "parse/test_arazzo_expr.h"
#include "parse/test_arazzo_eval.h"
#include "parse/test_arazzo_parser.h"
#include "parse/test_c2arazzo.h"
#include "emit/test_arazzo_client_gen.h"
#include "emit/test_arazzo_cli_gen.h"
#include "emit/test_arazzo_server_gen.h"
/* clang-format on */

/**
 * @brief Helper function GREATEST_MAIN_DEFS.
 * @return Result status or value.
 */
GREATEST_MAIN_DEFS();

/**
 * @brief Unit test test_cdd_helpers.
 * @return Greatest test result.
 */
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
    /** @brief External test variable `g_cdd_query_err_fail`. */
    extern C_CDD_EXPORT int g_cdd_query_err_fail;
    /** @brief External test variable `g_schema_realloc_fail`. */
    extern C_CDD_EXPORT int g_schema_realloc_fail;

    /** @brief External test variable `g_json_object_to_struct_fields_fail`. */
    extern C_CDD_EXPORT int g_json_object_to_struct_fields_fail;
    /** @brief External test variable `g_safe_crt_malloc_fail`. */
    extern C_CDD_EXPORT int g_safe_crt_malloc_fail;
    /** @brief External test variable `g_cdd_fprintf_fail`. */
    extern C_CDD_EXPORT int g_cdd_fprintf_fail;
    /** @brief External test variable `g_cdd_ffi_ir_calloc_fail`. */
    extern C_CDD_EXPORT int g_cdd_ffi_ir_calloc_fail;
    /** @brief External test variable `g_cdd_cst_emit_realloc_fail`. */
    extern C_CDD_EXPORT int g_cdd_cst_emit_realloc_fail;

    /** @brief External test variable `g_cdd_cst_realloc_fail`. */
    extern C_CDD_EXPORT int g_cdd_cst_realloc_fail;
    /** @brief External test variable `g_fail_io_after`. */
    extern C_CDD_EXPORT int g_fail_io_after;
    /** @brief External test variable `g_cdd_ffi_ir_toposort_fail`. */
    extern C_CDD_EXPORT int g_cdd_ffi_ir_toposort_fail;

    /** @brief External test variable `g_schema_codegen_force_fail`. */
    extern C_CDD_EXPORT int g_schema_codegen_force_fail;
    /** @brief External test variable `g_cdd_ffi_ir_malloc_fail`. */
    extern C_CDD_EXPORT int g_cdd_ffi_ir_malloc_fail;
    /** @brief External test variable `g_io_calls`. */
    extern C_CDD_EXPORT int g_io_calls;

    /** @brief External test variable `g_schema_strdup_fail`. */
    extern C_CDD_EXPORT int g_schema_strdup_fail;

    /** @brief External test variable `g_cdd_helpers_fopen_err`. */
    extern CDD_TEST_HELPERS_EXPORT int g_cdd_helpers_fopen_err;

    /* extern CDD_TEST_HELPERS_EXPORT int g_cdd_helpers_fopen_err; (moved to
     * global) */
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

    /* Cover cdd_test_mut_str NULL and >= 512 truncation branches */
    {
      char long_buf[600];
      char *mut_res;
      memset(long_buf, 'x', sizeof(long_buf) - 1);
      long_buf[sizeof(long_buf) - 1] = '\0';
      ASSERT_EQ(NULL, cdd_test_mut_str(NULL));
      mut_res = cdd_test_mut_str(long_buf);
      ASSERT(mut_res != NULL);
      ASSERT_EQ(511u, strlen(mut_res));
    }

    /* Cover cdd_test_tmpfile_global counter >= 1024 wraparound */
    {
      int k;
      for (k = 0; k < 1026; k++) {
        FILE *tf = cdd_test_tmpfile_global();
        if (tf)
          fclose(tf);
      }
    }

    /* Cover cdd_test_rmdir_recursive nested subdirectory & broken symlink */
    ASSERT_EQ(CDD_C_SUCCESS, makedirs("my_empty_dir" PATH_SEP "nested_sub"));
    ASSERT_EQ(CDD_C_SUCCESS, write_to_file("my_empty_dir" PATH_SEP
                                           "nested_sub" PATH_SEP "leaf.txt",
                                           "x"));
#if !defined(_WIN32) && !defined(__wasi__)
    if (symlink("nonexistent_target_xyz",
                "my_empty_dir" PATH_SEP "dangling_link") == 0) {
    }
#endif
    cdd_test_tmpfile_cleanup();

    g_fail_io_after = -1;
    PASS();
  }
}

/**
 * @brief Variadic helper invoking `c_cdd_crt_vfprintf`.
 * @param[in,out] fp Output stream.
 * @param[out] out_rc Optional `vfprintf` return code.
 * @param[in] fmt Format string.
 * @return Error status from `c_cdd_crt_vfprintf`.
 */
#if defined(__GNUC__) || defined(__clang__)
__attribute__((format(printf, 3, 4)))
#endif
static cdd_c_error_t
call_crt_vfprintf(FILE *fp, int *out_rc, const char *fmt, ...) {
  va_list args;
  cdd_c_error_t rc;
  va_start(args, fmt);
  rc = c_cdd_crt_vfprintf(fp, fmt, args, out_rc);
  va_end(args);
  return rc;
}

/**
 * @brief Variadic helper invoking `c_cdd_crt_vasprintf`.
 * @param[out] out_str Allocated string output.
 * @param[out] out_rc Optional `vasprintf` return code.
 * @param[in] fmt Format string.
 * @return Error status from `c_cdd_crt_vasprintf`.
 */
#if defined(__GNUC__) || defined(__clang__)
__attribute__((format(printf, 3, 4)))
#endif
static cdd_c_error_t
call_crt_vasprintf(char **out_str, int *out_rc, const char *fmt, ...) {
  va_list args;
  cdd_c_error_t rc;
  va_start(args, fmt);
  rc = c_cdd_crt_vasprintf(out_str, fmt, args, out_rc);
  va_end(args);
  return rc;
}

/**
 * @brief Variadic helper passing NULL `fmt` to `c_cdd_crt_vfprintf` and
 * `c_cdd_crt_vasprintf` with an initialized `va_list`.
 * @param[in,out] fp Output stream (or NULL to test `c_cdd_crt_vasprintf`).
 * @param[out] out_str Output string pointer.
 * @param[in] tag Dummy anchor parameter for `va_start`.
 * @return Error status from the tested function.
 */
static cdd_c_error_t call_crt_null_fmt(FILE *fp, char **out_str, int tag, ...) {
  va_list args;
  cdd_c_error_t rc;
  va_start(args, tag);
  if (fp)
    rc = c_cdd_crt_vfprintf(fp, NULL, args, NULL);
  else
    rc = c_cdd_crt_vasprintf(out_str, NULL, args, NULL);
  va_end(args);
  return rc;
}

/**
 * @brief Unit test exercising all `c_cdd_crt_*` functions and fault-injection
 * branches in `g_fail_io_after.c`.
 * @return Greatest test result.
 */
TEST test_crt_bridge_wrappers(void) {
  void *ptr = NULL;
  void *ptr2 = NULL;
  char *dup_s = NULL;
  FILE *fp = NULL;
  FILE *fp2 = NULL;
  FILE *std_fp = NULL;
  int err_val = 0;
  int io_rc = 0;
  long pos_val = 0;
  size_t n_io = 0;
  char read_buf[32];
  char *fg_res = NULL;
  fpos_t fpos;
  int fd1 = -1;
  int fd2 = -1;

  ASSERT_EQ(CDD_C_SUCCESS, c_cdd_crt_init_runtime());

  /* Heap wrappers: NULL args, OOM, countdown > 1, normal */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, c_cdd_crt_malloc(8, NULL));
  g_crt_bridge_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, c_cdd_crt_malloc(8, &ptr));
  g_crt_bridge_fail = 2;
  ASSERT_EQ(CDD_C_SUCCESS, c_cdd_crt_malloc(8, &ptr));
  c_cdd_crt_free(ptr);
  g_crt_bridge_fail = 0;
  ASSERT_EQ(CDD_C_SUCCESS, c_cdd_crt_malloc(8, &ptr));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, c_cdd_crt_realloc(ptr, 16, NULL));
  g_crt_bridge_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, c_cdd_crt_realloc(ptr, 16, &ptr2));
  g_crt_bridge_fail = 2;
  ASSERT_EQ(CDD_C_SUCCESS, c_cdd_crt_realloc(ptr, 16, &ptr2));
  g_crt_bridge_fail = 0;
  ASSERT_EQ(CDD_C_SUCCESS, c_cdd_crt_realloc(ptr2, 24, &ptr));
  c_cdd_crt_free(ptr);

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, c_cdd_crt_calloc(2, 4, NULL));
  g_crt_bridge_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, c_cdd_crt_calloc(2, 4, &ptr));
  g_crt_bridge_fail = 2;
  ASSERT_EQ(CDD_C_SUCCESS, c_cdd_crt_calloc(2, 4, &ptr));
  c_cdd_crt_free(ptr);
  g_crt_bridge_fail = 0;
  ASSERT_EQ(CDD_C_SUCCESS, c_cdd_crt_calloc(2, 4, &ptr));
  c_cdd_crt_free(ptr);

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, c_cdd_crt_strdup("hi", NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, c_cdd_crt_strdup(NULL, &dup_s));
  ASSERT_EQ(NULL, dup_s);
  g_crt_bridge_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, c_cdd_crt_strdup("hi", &dup_s));
  g_crt_bridge_fail = 2;
  ASSERT_EQ(CDD_C_SUCCESS, c_cdd_crt_strdup("hi", &dup_s));
  c_cdd_crt_free(dup_s);
  g_crt_bridge_fail = 0;
  ASSERT_EQ(CDD_C_SUCCESS, c_cdd_crt_strdup("hi", &dup_s));
  ASSERT_STR_EQ("hi", dup_s);
  c_cdd_crt_free(dup_s);

  /* fopen / fclose */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            c_cdd_crt_fopen("a.txt", "r", NULL, &err_val));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            c_cdd_crt_fopen(NULL, "r", &fp, &err_val));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            c_cdd_crt_fopen("a.txt", NULL, &fp, NULL));
  ASSERT_EQ(CDD_C_ERROR_IO,
            c_cdd_crt_fopen("nonexistent_crt_dir_xyz/missing.txt", "r", &fp,
                            &err_val));
  ASSERT_EQ(
      CDD_C_ERROR_IO,
      c_cdd_crt_fopen("nonexistent_crt_dir_xyz/missing.txt", "r", &fp, NULL));

  ASSERT_EQ(CDD_C_SUCCESS,
            c_cdd_crt_fopen("crt_bridge_tmp.txt", "w+b", &fp, &err_val));
  ASSERT_EQ(CDD_C_SUCCESS, c_cdd_crt_fclose(fp, NULL));
  ASSERT_EQ(CDD_C_SUCCESS,
            c_cdd_crt_fopen("crt_bridge_tmp.txt", "w+b", &fp, NULL));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, c_cdd_crt_fclose(NULL, &io_rc));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, c_cdd_crt_fclose(NULL, NULL));

  /* fwrite / fflush / fseek / ftell / rewind / fread / fputs / fgets / clearerr
   */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            c_cdd_crt_fwrite(NULL, 1, 1, fp, &n_io));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            c_cdd_crt_fwrite("x", 1, 1, NULL, &n_io));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            c_cdd_crt_fwrite("x", 1, 1, fp, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, c_cdd_crt_fwrite("hello\n", 1, 6, fp, &n_io));
  ASSERT_EQ(6u, n_io);

  ASSERT_EQ(CDD_C_SUCCESS, c_cdd_crt_fflush(fp, &io_rc));
  ASSERT_EQ(CDD_C_SUCCESS, c_cdd_crt_fflush(fp, NULL));
  g_crt_bridge_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_IO, c_cdd_crt_fflush(fp, &io_rc));
  g_crt_bridge_fail = 2;
  ASSERT_EQ(CDD_C_SUCCESS, c_cdd_crt_fflush(fp, NULL));
  g_crt_bridge_fail = 0;

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            c_cdd_crt_fseek(NULL, 0L, SEEK_SET, &io_rc));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            c_cdd_crt_fseek(NULL, 0L, SEEK_SET, NULL));
  ASSERT_EQ(CDD_C_ERROR_IO, c_cdd_crt_fseek(fp, -100L, SEEK_SET, &io_rc));
  ASSERT_EQ(CDD_C_SUCCESS, c_cdd_crt_fseek(fp, 0L, SEEK_SET, NULL));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, c_cdd_crt_ftell(NULL, &pos_val));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, c_cdd_crt_ftell(fp, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, c_cdd_crt_ftell(fp, &pos_val));
  g_crt_bridge_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_IO, c_cdd_crt_ftell(fp, &pos_val));
  g_crt_bridge_fail = 2;
  ASSERT_EQ(CDD_C_SUCCESS, c_cdd_crt_ftell(fp, &pos_val));
  g_crt_bridge_fail = 0;

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, c_cdd_crt_rewind(NULL));
  ASSERT_EQ(CDD_C_SUCCESS, c_cdd_crt_rewind(fp));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            c_cdd_crt_fread(NULL, 1, 1, fp, &n_io));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            c_cdd_crt_fread(read_buf, 1, 1, NULL, &n_io));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            c_cdd_crt_fread(read_buf, 1, 1, fp, NULL));
  memset(read_buf, 0, sizeof(read_buf));
  ASSERT_EQ(CDD_C_SUCCESS, c_cdd_crt_fread(read_buf, 1, 6, fp, &n_io));
  ASSERT_EQ(6u, n_io);

  ASSERT_EQ(CDD_C_SUCCESS, c_cdd_crt_fseek(fp, 0L, SEEK_END, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, c_cdd_crt_fputs(NULL, fp, &io_rc));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, c_cdd_crt_fputs("a", NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, c_cdd_crt_fputs("line2\n", fp, &io_rc));
  ASSERT_EQ(CDD_C_SUCCESS, c_cdd_crt_fputs("line3\n", fp, NULL));
  g_crt_bridge_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_IO, c_cdd_crt_fputs("fail\n", fp, &io_rc));
  g_crt_bridge_fail = 2;
  ASSERT_EQ(CDD_C_SUCCESS, c_cdd_crt_fputs("ok\n", fp, NULL));
  g_crt_bridge_fail = 0;

  ASSERT_EQ(CDD_C_SUCCESS, c_cdd_crt_rewind(fp));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            c_cdd_crt_fgets(NULL, 10, fp, &fg_res));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            c_cdd_crt_fgets(read_buf, 10, NULL, &fg_res));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            c_cdd_crt_fgets(read_buf, 10, fp, NULL));
  ASSERT_EQ(CDD_C_SUCCESS,
            c_cdd_crt_fgets(read_buf, (int)sizeof(read_buf), fp, &fg_res));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, c_cdd_crt_clearerr(NULL));
  ASSERT_EQ(CDD_C_SUCCESS, c_cdd_crt_clearerr(fp));

  /* vfprintf / vasprintf / jasprintf */
  ASSERT_EQ(CDD_C_SUCCESS, c_cdd_crt_fseek(fp, 0L, SEEK_END, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            call_crt_vfprintf(NULL, &io_rc, "%s", "x"));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, call_crt_null_fmt(fp, NULL, 0));
  ASSERT_EQ(CDD_C_SUCCESS, call_crt_vfprintf(fp, &io_rc, "%d", 42));
  ASSERT_EQ(CDD_C_SUCCESS, call_crt_vfprintf(fp, NULL, "%d", 42));
  g_crt_bridge_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_IO, call_crt_vfprintf(fp, &io_rc, "%d", 42));
  g_crt_bridge_fail = 2;
  ASSERT_EQ(CDD_C_SUCCESS, call_crt_vfprintf(fp, NULL, "%d", 42));
  g_crt_bridge_fail = 0;

  dup_s = NULL;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            call_crt_vasprintf(NULL, &io_rc, "%d", 7));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, call_crt_null_fmt(NULL, &dup_s, 0));
  g_crt_bridge_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, call_crt_vasprintf(&dup_s, &io_rc, "%d", 7));
  g_crt_bridge_fail = 2;
  ASSERT_EQ(CDD_C_SUCCESS, call_crt_vasprintf(&dup_s, NULL, "%d", 7));
  c_cdd_crt_free(dup_s);
  g_crt_bridge_fail = 0;
  ASSERT_EQ(CDD_C_SUCCESS, call_crt_vasprintf(&dup_s, &io_rc, "%d", 7));
  c_cdd_crt_free(dup_s);

  dup_s = NULL;
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            c_cdd_crt_jasprintf(NULL, "a", &io_rc));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, c_cdd_crt_jasprintf(NULL, "a", NULL));
  g_crt_bridge_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_MEMORY, c_cdd_crt_jasprintf(&dup_s, "a", &io_rc));
  g_crt_bridge_fail = 2;
  ASSERT_EQ(CDD_C_SUCCESS, c_cdd_crt_jasprintf(&dup_s, "a", NULL));
  g_crt_bridge_fail = 0;
  ASSERT_EQ(CDD_C_SUCCESS, c_cdd_crt_jasprintf(&dup_s, NULL, &io_rc));
  c_cdd_crt_free(dup_s);

  /* putenv / std_stream */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, c_cdd_crt_putenv(NULL, &io_rc));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, c_cdd_crt_putenv(NULL, NULL));
  ASSERT_EQ(CDD_C_ERROR_IO, c_cdd_crt_putenv("", &io_rc));
  ASSERT_EQ(CDD_C_SUCCESS, c_cdd_crt_putenv("CDD_CRT_TEST_ENV=1", NULL));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, c_cdd_crt_std_stream(0, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, c_cdd_crt_std_stream(0, &std_fp));
  ASSERT_EQ(CDD_C_SUCCESS, c_cdd_crt_std_stream(1, &std_fp));
  ASSERT_EQ(CDD_C_SUCCESS, c_cdd_crt_std_stream(2, &std_fp));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, c_cdd_crt_std_stream(9, &std_fp));

  /* fgetpos / fsetpos / fileno / dup / dup2 / close */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            c_cdd_crt_fgetpos(NULL, &fpos, &io_rc));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, c_cdd_crt_fgetpos(fp, NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, c_cdd_crt_fgetpos(fp, &fpos, &io_rc));
  ASSERT_EQ(CDD_C_SUCCESS, c_cdd_crt_fgetpos(fp, &fpos, NULL));
  g_crt_bridge_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_IO, c_cdd_crt_fgetpos(fp, &fpos, &io_rc));
  g_crt_bridge_fail = 2;
  ASSERT_EQ(CDD_C_SUCCESS, c_cdd_crt_fgetpos(fp, &fpos, NULL));
  g_crt_bridge_fail = 0;

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            c_cdd_crt_fsetpos(NULL, &fpos, &io_rc));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, c_cdd_crt_fsetpos(fp, NULL, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, c_cdd_crt_fsetpos(fp, &fpos, &io_rc));
  ASSERT_EQ(CDD_C_SUCCESS, c_cdd_crt_fsetpos(fp, &fpos, NULL));
  g_crt_bridge_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_IO, c_cdd_crt_fsetpos(fp, &fpos, &io_rc));
  g_crt_bridge_fail = 2;
  ASSERT_EQ(CDD_C_SUCCESS, c_cdd_crt_fsetpos(fp, &fpos, NULL));
  g_crt_bridge_fail = 0;

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, c_cdd_crt_fileno(NULL, &fd1));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, c_cdd_crt_fileno(fp, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, c_cdd_crt_fileno(fp, &fd1));
  g_crt_bridge_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_IO, c_cdd_crt_fileno(fp, &fd2));
  g_crt_bridge_fail = 2;
  ASSERT_EQ(CDD_C_SUCCESS, c_cdd_crt_fileno(fp, &fd1));
  g_crt_bridge_fail = 0;

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, c_cdd_crt_dup(fd1, NULL));
  ASSERT_EQ(CDD_C_ERROR_IO, c_cdd_crt_dup(-1, &fd2));
  ASSERT_EQ(CDD_C_SUCCESS, c_cdd_crt_dup(fd1, &fd2));

  ASSERT_EQ(CDD_C_ERROR_IO, c_cdd_crt_dup2(-1, -1, &io_rc));
  ASSERT_EQ(CDD_C_ERROR_IO, c_cdd_crt_dup2(-1, -1, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, c_cdd_crt_dup2(fd1, fd2, &io_rc));
  ASSERT_EQ(CDD_C_SUCCESS, c_cdd_crt_dup2(fd1, fd2, NULL));

  ASSERT_EQ(CDD_C_ERROR_IO, c_cdd_crt_close(-1, &io_rc));
  ASSERT_EQ(CDD_C_ERROR_IO, c_cdd_crt_close(-1, NULL));
  ASSERT_EQ(CDD_C_SUCCESS, c_cdd_crt_close(fd2, &io_rc));
  ASSERT_EQ(CDD_C_SUCCESS, c_cdd_crt_dup(fd1, &fd2));
  ASSERT_EQ(CDD_C_SUCCESS, c_cdd_crt_close(fd2, NULL));

  /* freopen + fclose fault injection */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            c_cdd_crt_freopen("crt_bridge_tmp.txt", "r", fp, NULL, &err_val));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            c_cdd_crt_freopen(NULL, "r", fp, &fp2, &err_val));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            c_cdd_crt_freopen("crt_bridge_tmp.txt", NULL, fp, &fp2, NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            c_cdd_crt_freopen("crt_bridge_tmp.txt", "r", NULL, &fp2, NULL));
  ASSERT_EQ(CDD_C_SUCCESS,
            c_cdd_crt_freopen("crt_bridge_tmp.txt", "r", fp, &fp2, &err_val));
  ASSERT_EQ(CDD_C_SUCCESS,
            c_cdd_crt_freopen("crt_bridge_tmp.txt", "r", fp2, &fp, NULL));

  /* Test freopen failure on nonexistent path (closes fp) */
  ASSERT_EQ(CDD_C_ERROR_IO,
            c_cdd_crt_freopen("nonexistent_crt_dir_xyz/missing.txt", "r", fp,
                              &fp2, &err_val));
  ASSERT_EQ(CDD_C_SUCCESS,
            c_cdd_crt_fopen("crt_bridge_tmp.txt", "r", &fp, NULL));
  ASSERT_EQ(CDD_C_ERROR_IO,
            c_cdd_crt_freopen("nonexistent_crt_dir_xyz/missing.txt", "r", fp,
                              &fp2, NULL));

  /* fclose fault injection */
  ASSERT_EQ(CDD_C_SUCCESS,
            c_cdd_crt_fopen("crt_bridge_tmp.txt", "r", &fp, NULL));
  g_crt_bridge_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_IO, c_cdd_crt_fclose(fp, &io_rc));
  ASSERT_EQ(CDD_C_SUCCESS,
            c_cdd_crt_fopen("crt_bridge_tmp.txt", "r", &fp, NULL));
  g_crt_bridge_fail = 2;
  ASSERT_EQ(CDD_C_SUCCESS, c_cdd_crt_fclose(fp, &io_rc));
  g_crt_bridge_fail = 0;

  remove("crt_bridge_tmp.txt");
  PASS();
}

/**
 * @brief Executes the cdd_helpers_suite test suite.
 */
SUITE(cdd_helpers_suite) {
  RUN_TEST(test_cdd_helpers);
  RUN_TEST(test_crt_bridge_wrappers);
}

/**
 * @brief Executes the ffi_extractor_suite test suite.
 */
SUITE(ffi_extractor_suite) {
  RUN_TEST(test_ffi_e2e_complex_codebase);
  RUN_TEST(test_dsl_tokenizer_basic);
  RUN_TEST(test_dsl_tokenizer_oom);
  RUN_TEST(test_dsl_tokenizer_exhaustive);
  RUN_TEST(test_dsl_tokenizer_oom_loop);
  RUN_TEST(test_dsl_tokenizer_free_null);
  RUN_TEST(test_dsl_tokenizer_comments);
  RUN_TEST(test_dsl_tokenizer_escapes);
  RUN_TEST(test_dsl_tokenizer_unknown_slash);
  RUN_TEST(test_dsl_tokenizer_eof_comment);
  RUN_TEST(test_dsl_tokenizer_macro_invoke);
  RUN_TEST(test_dsl_parser_basic);
  RUN_TEST(test_dsl_parser_exhaustive);
  RUN_TEST(test_dsl_parser_oom_loop);
  RUN_TEST(test_dsl_parser_enum_oom);
  RUN_TEST(test_dsl_parser_oom);
  RUN_TEST(test_dsl_parser_exhaustive_oom);
  RUN_TEST(test_dsl_parser_invalid);
  RUN_TEST(test_dsl_emitter_basic);
  RUN_TEST(test_dsl_emitter_exhaustive);

  RUN_TEST(test_dsl_emitter_invalid);
  RUN_TEST(test_dsl_emitter_null_config);
  RUN_TEST(test_dsl_emitter_io_fail);
  RUN_TEST(test_dsl_integration_basic);
  RUN_TEST(test_dsl_integration_fail);
  RUN_TEST(test_dsl_parser_namespace);
  RUN_TEST(test_dsl_parser_number);
  RUN_TEST(test_dsl_parser_macros);
  RUN_TEST(test_dsl_parser_modifiers);

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
  RUN_TEST(test_ffi_extractor_trivia);
  RUN_TEST(test_ffi_extractor_trivia_oom);
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
  RUN_TEST(test_ffi_ir_extract_mock_failures);
}

/** @brief Test definition g_accept_fail. */
extern CDD_TEST_HELPERS_EXPORT int g_accept_fail;
/** @brief Test definition g_bind_fail. */
extern CDD_TEST_HELPERS_EXPORT int g_bind_fail;
/** @brief Test definition g_listen_fail. */
extern CDD_TEST_HELPERS_EXPORT int g_listen_fail;
/** @brief Test definition g_getsockname_fail. */
extern CDD_TEST_HELPERS_EXPORT int g_getsockname_fail;
/** @brief Test definition g_pthread_create_fail. */
extern CDD_TEST_HELPERS_EXPORT int g_pthread_create_fail;
/** @brief Test definition g_socket_fail. */
extern CDD_TEST_HELPERS_EXPORT int g_socket_fail;

/**
 * @brief Helper function reset_mocks.
 */
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
  g_cdd_fail_identify_keyword_or_id = 0;
  g_schema_codegen_force_fail = 0;
  g_schema_io_calls = 0;
  g_schema_fail_io_after = -1;
  g_cdd_fail_asprintf = 0;
  g_cdd_cst_alloc_node_fail = 0;
  g_cdd_cst_alloc_token_fail = 0;
  g_cdd_cst_emit_realloc_fail = 0;
  g_cdd_cst_realloc_fail = 0;
  g_cdd_ffi_ir_calloc_fail = 0;
  g_cdd_ffi_ir_malloc_fail = 0;
  g_cdd_ffi_ir_toposort_fail = 0;
  g_cdd_fprintf_fail = 0;
  g_cdd_query_err_fail = 0;
  g_cdd_strdup_fail = 0;
  {
    {
      {
        {
          /** @brief External test variable `g_ffi_extractor_alloc_fail`. */
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
          /* extern C_CDD_EXPORT int g_io_calls; (moved to global) */
          g_io_calls = 0;
          json_set_allocation_functions(NULL, NULL);
        }
      }
    }
  }
}

/**
 * @brief Helper function test_teardown_cb.
 * @param[in,out] udata Parameter udata.
 */
static void test_teardown_cb(void *udata) {
  if (!udata)
    reset_mocks();
}

/**
 * @brief Macro RUN_SUITE_RESET.
 * @param[in] suite Macro argument suite.
 */
#define RUN_SUITE_RESET(suite)                                                 \
  do {                                                                         \
    reset_mocks();                                                             \
    SET_TEARDOWN(test_teardown_cb, NULL);                                      \
    RUN_SUITE(suite);                                                          \
  } while (0)

/** @brief External test suite `safe_crt_msvc_suite`. */
SUITE_EXTERN(safe_crt_msvc_suite);
/** @brief External test suite `crt_bridge_header_suite`. */
SUITE_EXTERN(crt_bridge_header_suite);
/** @brief External test suite `platform_crypto_and_cst_suite`. */
SUITE_EXTERN(platform_crypto_and_cst_suite);

/**
 * @brief Helper function main.
 * @param[in] argc Parameter argc.
 * @param[in,out] argv Parameter argv.
 * @return Result status or value.
 */
int main(int argc, char **argv) {
  if (cdd_suppress_gui_popups() != CDD_C_SUCCESS)
    return EXIT_FAILURE;
  if (c_cdd_crt_init_runtime() != CDD_C_SUCCESS)
    return EXIT_FAILURE;
  setvbuf(stdout, NULL, _IONBF, 0);
  atexit(cdd_test_tmpfile_cleanup);
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
    cdd_c_error_t rc = cdd_cst_parse(az_span_create_from_str(snippet), &tree);
    printf("PARSE RC = %d, num_children = %lu, capacity = %lu\n", rc,
           (unsigned long)tree->root->num_children,
           (unsigned long)tree->root->capacity);
    if (tree)
      g_cdd_last_alloc_err = cdd_cst_tree_free(tree);
  }

#if defined(_MSC_VER) && _MSC_VER <= 1400
  /* skipped */
#else

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
  RUN_SUITE_RESET(transformer_c_lowering_suite);
  RUN_SUITE_RESET(transformer_extern_c_suite);
  RUN_SUITE_RESET(transformer_extern_c_coverage_suite);
  RUN_SUITE_RESET(transformer_macros_suite);
  RUN_SUITE_RESET(transformer_error_percolator_suite);
  RUN_SUITE_RESET(transformer_error_percolator_call_sites_suite);
  RUN_SUITE_RESET(cdd_ffi_ir_suite);
  RUN_SUITE_RESET(test_ffi_ir_json_suite);
  RUN_SUITE_RESET(ffi_variadic_suite);
  RUN_SUITE_RESET(ffi_emitters_suite);
  RUN_SUITE_RESET(ffi_emitters_edge_suite);
  RUN_SUITE_RESET(ffi_ocaml_fortran_cobol_suite);
  RUN_SUITE_RESET(cli_gen_suite);
  RUN_SUITE_RESET(codegen_cli_suite);
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
  RUN_SUITE_RESET(dsl_parser_branches_suite);
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
  RUN_SUITE_RESET(arazzo_types_suite);
  RUN_SUITE_RESET(arazzo_expr_suite);
  RUN_SUITE_RESET(arazzo_eval_suite);
  RUN_SUITE_RESET(arazzo_parser_suite);
  RUN_SUITE_RESET(c2arazzo_suite);
  RUN_SUITE_RESET(arazzo_client_gen_suite);
  RUN_SUITE_RESET(arazzo_cli_gen_suite);
  RUN_SUITE_RESET(arazzo_server_gen_suite);
  RUN_SUITE_RESET(context_suite);
  RUN_SUITE_RESET(safe_crt_msvc_suite);
  RUN_SUITE_RESET(crt_bridge_header_suite);
  RUN_SUITE_RESET(platform_crypto_and_cst_suite);
  reset_mocks();
  cdd_test_tmpfile_cleanup();

#endif
  GREATEST_MAIN_END();
}

#ifndef CDD_BUILD_TESTS
#error "CDD_BUILD_TESTS IS NOT DEFINED!"
#endif
