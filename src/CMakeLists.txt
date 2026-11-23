###########
# Library #
###########

set(LIBRARY_NAME "c_cdd")

set(Header_Files
        "functions/parse/analysis.h"
        "mocks/c_cdd_stdbool.h"
        "classes/parse/inspector.h"
        "classes/parse/cdd_serde.h"
        "classes/parse/cdd_serde_internal.h"
        "routes/parse/cli.h"
        "routes/emit/operation.h"
        "classes/emit/schema.h"
        "classes/parse/query_projection.h"
        "classes/parse/cli_parser.h"
        "classes/parse/mapping.h"
        "classes/parse/code2schema.h"
        "classes/parse/cdd_token.h"
        "classes/parse/cdd_lexer.h"
        "classes/parse/cdd_cst_node.h"
        "classes/parse/cdd_cst_parser.h"
        "classes/parse/cdd_cst_mutate.h"
        "classes/parse/cdd_cst_scope.h"
        "classes/parse/cdd_cst_semantic.h"
        "classes/parse/cdd_cst_type_eval.h"
        "classes/parse/cdd_cst_cfg.h"
        "classes/parse/cdd_cst_escape.h"
        "classes/parse/cdd_cst_query.h"
        "../include/cdd_cst_transform.h"
        "../include/c_cdd/code_writer.h"
        "../include/c_cdd/context.h"
        "../include/c_cdd/json_dom.h"
        "../include/parson.h"
        "../include/routes/parse/cli_cst.h"
        "functions/ffi/cdd_ffi_emit_common.h"
        "classes/parse/cdd_cst_trivia.h"
        "classes/emit/cdd_cst_emit.h"
        "functions/emit/codegen.h"
        "functions/emit/build.h"
        "functions/emit/client_body.h"
        "functions/emit/client_sig.h"
        "classes/emit/enum.h"
        "classes/emit/cli_struct.h"
        "classes/emit/cli_struct_internal.h"
        "classes/emit/json.h"
        "classes/emit/form.h"
        "classes/emit/jwt.h"
        "classes/emit/oauth2_error.h"
        "functions/emit/make.h"
        "tests/emit/sdk_tests.h"
        "routes/emit/security.h"
        "classes/emit/struct.h"
        "classes/emit/struct_internal.h"
        "classes/emit/types.h"
        "routes/emit/url.h"
        "routes/emit/serve_json_rpc.h"
        "cdd_api.h"
        "functions/parse/crypto_types.h"
        "functions/parse/cst.h"
        "functions/parse/declarator.h"
        "docstrings/parse/doc.h"
        "functions/parse/fs.h"
        "functions/emit/build_system.h"
        "classes/parse/initializer.h"
        "classes/parse/numeric.h"
        "routes/emit/aggregator.h"
        "routes/emit/client_gen.h"
        "routes/emit/cli_gen.h"
        "routes/emit/client_gui_gen.h"
        "routes/emit/server_gen.h"
        "openapi/parse/openapi.h"
        "openapi/parse/openapi_types.h"
        "openapi/parse/openapi_internal.h"
        "openapi/parse/openapi_internal_components.h"
        "openapi/parse/openapi_media.h"
        "openapi/parse/openapi_schema.h"
        "openapi/parse/openapi_uri.h"
        "openapi/parse/openapi_validate.h"
        "openapi/emit/openapi.h"
        "functions/parse/preprocessor.h"
        "functions/parse/audit.h"
        "functions/parse/audit_internal.h"
        "routes/parse/sync.h"
        "functions/parse/refactor.h"
        "functions/parse/orchestrator.h"
        "functions/emit/rewriter_body.h"
        "functions/emit/rewriter_sig.h"
        "functions/emit/safe_crt.h"
        "functions/emit/diff.h"
        "tests/emit/schema2tests.h"
        "classes/emit/schema_codegen.h"
        "functions/parse/strategy.h"
        "functions/emit/sync.h"
        "functions/emit/patcher.h"
        "functions/parse/tokenizer.h"
        "functions/parse/str.h"
        "functions/parse/db_loader.h"
        "functions/parse/desig_init.h"
        "functions/parse/makefile_scraper.h"
        "functions/parse/macro_overlay.h"
        "functions/parse/decl_hoist.h"
        "functions/parse/cmake_parser.h"
        "functions/parse/vla_analyzer.h"
        "functions/parse/vcpkg_integration.h"
        "functions/ffi/cdd_ffi_emit_ada.h"
        "functions/ffi/cdd_ffi_emit_clojure.h"
        "functions/ffi/cdd_ffi_emit_cobol.h"
        "functions/ffi/cdd_ffi_emit_common_lisp.h"
        "functions/ffi/cdd_ffi_emit_cpp.h"
        "functions/ffi/cdd_ffi_emit_crystal.h"
        "functions/ffi/cdd_ffi_emit_csharp.h"
        "functions/ffi/cdd_ffi_emit_d.h"
        "functions/ffi/cdd_ffi_emit_dart.h"
        "functions/ffi/cdd_ffi_emit_delphi.h"
        "functions/ffi/cdd_ffi_emit_elixir.h"
        "functions/ffi/cdd_ffi_emit_erlang.h"
        "functions/ffi/cdd_ffi_emit_fortran.h"
        "functions/ffi/cdd_ffi_emit_fsharp.h"
        "functions/ffi/cdd_ffi_emit_go.h"
        "functions/ffi/cdd_ffi_emit_groovy.h"
        "functions/ffi/cdd_ffi_emit_haskell.h"
        "functions/ffi/cdd_ffi_emit_java.h"
        "functions/ffi/cdd_ffi_emit_julia.h"
        "functions/ffi/cdd_ffi_emit_kotlin.h"
        "functions/ffi/cdd_ffi_emit_lua.h"
        "functions/ffi/cdd_ffi_emit_matlab.h"
        "functions/ffi/cdd_ffi_emit_napi.h"
        "functions/ffi/cdd_ffi_emit_nim.h"
        "functions/ffi/cdd_ffi_emit_objc.h"
        "functions/ffi/cdd_ffi_emit_ocaml.h"
        "functions/ffi/cdd_ffi_emit_odin.h"
        "functions/ffi/cdd_ffi_emit_perl.h"
        "functions/ffi/cdd_ffi_emit_php.h"
        "functions/ffi/cdd_ffi_emit_python.h"
        "functions/ffi/cdd_ffi_emit_r.h"
        "functions/ffi/cdd_ffi_emit_racket.h"
        "functions/ffi/cdd_ffi_emit_ruby.h"
        "functions/ffi/cdd_ffi_emit_rust.h"
        "functions/ffi/cdd_ffi_emit_scala.h"
        "functions/ffi/cdd_ffi_emit_scheme.h"
        "functions/ffi/cdd_ffi_emit_swift.h"
        "functions/ffi/cdd_ffi_emit_tcl.h"
        "functions/ffi/cdd_ffi_emit_typescript.h"
        "functions/ffi/cdd_ffi_emit_vlang.h"
        "functions/ffi/cdd_ffi_emit_webassembly.h"
        "functions/ffi/cdd_ffi_emit_zig.h"
        "arazzo/emit/arazzo.h"
        "arazzo/parse/arazzo.h"
        "arazzo/parse/arazzo_eval.h"
        "arazzo/parse/arazzo_expr.h"
        "arazzo/parse/arazzo_types.h"
        "arazzo/parse/c2arazzo.h"
        "routes/emit/arazzo_cli_gen.h"
        "routes/emit/arazzo_client_gen.h"
        "routes/emit/arazzo_server_gen.h"
        "win_compat_sym.h"
)
source_group("Header Files" FILES "${Header_Files}")

set(C_CDD_TRANSFORM_SOURCES
        "cdd_c_error.c"
        "log.c"
        "code_writer.c"
        "context.c"
        "functions/parse/str.c"
        "functions/parse/fs.c"
        "functions/parse/fs_dir.c"
        "classes/parse/numeric.c"
        "classes/parse/cdd_lexer.c"
        "classes/parse/cdd_cst_parser.c"
        "classes/parse/cdd_cst_parser_expr.c"
        "classes/parse/cdd_cst_mutate.c"
        "classes/parse/cdd_cst_mutate_splice.c"
        "classes/parse/cdd_cst_scope.c"
        "classes/parse/cdd_cst_semantic.c"
        "classes/parse/cdd_cst_type_eval.c"
        "classes/parse/cdd_cst_cfg.c"
        "classes/parse/cdd_cst_escape.c"
        "classes/parse/cdd_cst_factory.c"
        "classes/parse/cdd_cst_factory_format.c"
        "classes/parse/cdd_cst_builder.c"
        "classes/parse/cdd_cst_builder_trivia.c"
        "classes/parse/cdd_cst_query.c"
        "classes/parse/cdd_cst_trivia.c"
        "classes/emit/cdd_cst_emit.c"
        "functions/parse/tokenizer.c"
        "functions/parse/tokenizer_keywords.c"
        "functions/parse/tokenizer_punct.c"
        "functions/parse/cst.c"
        "functions/parse/cst_match.c"
        "functions/parse/declarator.c"
        "classes/parse/initializer.c"
        "functions/parse/decl_hoist.c"
        "functions/parse/desig_init.c"
        "functions/parse/vla_analyzer.c"
        "functions/emit/patcher.c"
        "functions/emit/weaver.c"
        "functions/emit/weaver_attributes.c"
        "transformers/extern_c/extern_c.c"
        "transformers/msvc_port/msvc_port.c"
        "transformers/c_lowering/c_lowering_legacy.c"
        "transformers/c_lowering/c_lowering_modern.c"
        "transformers/gnu_standardizer/gnu_standardizer.c"
        "transformers/gnu_standardizer/gnu_standardizer_builtins.c"
        "transformers/gnu_standardizer/gnu_standardizer_expr.c"
        "transformers/gnu_standardizer/gnu_standardizer_helpers.c"
        "transformers/gnu_standardizer/gnu_standardizer_ranges.c"
        "transformers/gnu_standardizer/gnu_standardizer_types.c"
        "transformers/gnu_standardizer/gnu_standardizer_unroll.c"
        "transformers/error_percolator/error_percolator.c"
        "transformers/error_percolator/error_percolator_call_sites.c"
        "transformers/safe_crt/safe_crt.c"
        "transformers/safe_crt/safe_crt_parse.c"
        "transformers/safe_crt/safe_crt_rules.c"
        "transformers/safe_crt/safe_crt_rules_conv.c"
        "transformers/safe_crt/safe_crt_rules_env.c"
        "transformers/safe_crt/safe_crt_rules_io.c"
        "transformers/safe_crt/safe_crt_rules_str.c"
        "transformers/macros/macros.c"
        "routes/parse/cli_cst.c"
)

if(CDD_C_TRANSFORMER_ONLY)
    set(TRANSFORM_LIBRARY_NAME "c_cdd_transform")
    set(Transform_Source_Files ${C_CDD_TRANSFORM_SOURCES})
    if(_C_CDD_ENABLE_TESTS)
        list(APPEND Transform_Source_Files "tests/mocks/g_fail_io_after.c")
    endif()

    add_library("${TRANSFORM_LIBRARY_NAME}" STATIC ${Transform_Source_Files})
    add_library("${LIBRARY_NAME}" ALIAS "${TRANSFORM_LIBRARY_NAME}")
    c_cdd_apply_target_settings("${TRANSFORM_LIBRARY_NAME}")

    target_compile_definitions(
        "${TRANSFORM_LIBRARY_NAME}"
        PUBLIC
            CDD_C_TRANSFORMER_ONLY=1
            C_CDD_STATIC_DEFINE
    )
    if(_C_CDD_ENABLE_TESTS)
        target_compile_definitions("${TRANSFORM_LIBRARY_NAME}" PUBLIC CDD_BUILD_TESTS)
    endif()

    include(GNUInstallDirs)
    target_include_directories(
        "${TRANSFORM_LIBRARY_NAME}"
        PUBLIC
            "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
            "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
            "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/mocks>"
            "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>"
            "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>"
            "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
    )

    target_link_libraries(
        "${TRANSFORM_LIBRARY_NAME}"
        PRIVATE
            c89stringutils
        PUBLIC
            c_str_span
            "${PROJECT_NAME}_compiler_flags"
    )

    include(GenerateExportHeader)
    set(_export_file "${CMAKE_CURRENT_BINARY_DIR}/${LIBRARY_NAME}_export.h")
    generate_export_header(
        "${TRANSFORM_LIBRARY_NAME}"
        BASE_NAME "${LIBRARY_NAME}"
        EXPORT_FILE_NAME "${_export_file}"
    )

    add_executable("cdd-c-transform" "bin_cdd.c")
    c_cdd_apply_target_settings("cdd-c-transform")
    target_link_libraries("cdd-c-transform" PRIVATE "${TRANSFORM_LIBRARY_NAME}")

    set(_transform_cli_targets "cdd-c-transform")
    if(NOT CMAKE_SYSTEM_NAME STREQUAL "DOS")
        add_executable("cdd-c" "bin_cdd.c")
        c_cdd_apply_target_settings("cdd-c")
        target_link_libraries("cdd-c" PRIVATE "${TRANSFORM_LIBRARY_NAME}")
        list(APPEND _transform_cli_targets "cdd-c")
    endif()

    export(
        TARGETS ${_transform_cli_targets}
        FILE "${PROJECT_BINARY_DIR}/cdd-c-transformTargets.cmake"
    )

    if(C_CDD_INSTALL)
        set(_transform_export_targets "${TRANSFORM_LIBRARY_NAME}" "${PROJECT_NAME}_compiler_flags")
        if(C_CDD_IS_TOP_LEVEL)
            foreach(_dep_tgt c89stringutils c89stringutils_compiler_flags c_str_span c_str_span_compiler_flags)
                if(TARGET ${_dep_tgt})
                    get_target_property(_is_imp ${_dep_tgt} IMPORTED)
                    get_target_property(_is_alias ${_dep_tgt} ALIASED_TARGET)
                    if(NOT _is_imp AND NOT _is_alias)
                        list(APPEND _transform_export_targets ${_dep_tgt})
                    endif()
                endif()
            endforeach()
        endif()

        install(TARGETS ${_transform_export_targets}
            EXPORT "${LIBRARY_NAME}Targets"
            ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
            LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
            RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
        )
        install(TARGETS ${_transform_cli_targets}
            RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
        )
        install(EXPORT "${LIBRARY_NAME}Targets"
            DESTINATION "${CMAKE_INSTALL_DATADIR}/${LIBRARY_NAME}"
        )
        install(FILES
            "${PROJECT_SOURCE_DIR}/include/cdd_cst_transform.h"
            "${PROJECT_SOURCE_DIR}/include/cdd_c_error.h"
            "${_export_file}"
            DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
        )
    endif()
    return()
endif()

set(Source_Files
        "arazzo/emit/arazzo.c"
        "arazzo/parse/arazzo.c"
        "arazzo/parse/arazzo_eval.c"
        "arazzo/parse/arazzo_expr.c"
        "arazzo/parse/arazzo_free.c"
        "arazzo/parse/c2arazzo.c"
        "docstrings/parse/doc_arazzo.c"
        "routes/emit/arazzo_cli_gen.c"
        "routes/emit/arazzo_client_gen.c"
        "routes/emit/arazzo_server_gen.c"
        "classes/emit/types_array.c"
        "classes/parse/cdd_cst_builder_trivia.c"
        "classes/parse/cdd_cst_parser_expr.c"
        "docstrings/parse/doc_operations.c"
        "docstrings/parse/doc_scanner.c"
        "docstrings/parse/doc_security.c"
        "docstrings/parse/doc_tags.c"
        "functions/emit/client_body_cookie.c"
        "functions/emit/client_body_form.c"
        "functions/emit/client_body_header.c"
        "functions/emit/client_body_media.c"
        "functions/emit/client_body_multipart.c"
        "functions/emit/client_body_payload.c"
        "functions/emit/client_sig_media.c"
        "functions/emit/client_sig_param.c"
        "functions/emit/client_sig_types.c"
        "functions/ffi/cdd_ffi_ir_templates.c"
        "functions/ffi/cdd_ffi_ir_types.c"
        "functions/parse/cst_match.c"
        "functions/parse/fs_dir.c"
        "functions/parse/preprocessor_directives.c"
        "functions/parse/preprocessor_eval.c"
        "functions/parse/tokenizer_keywords.c"
        "functions/parse/tokenizer_punct.c"
        "openapi/emit/openapi_components.c"
        "openapi/emit/openapi_info.c"
        "openapi/emit/openapi_parameters.c"
        "openapi/emit/openapi_paths.c"
        "openapi/emit/openapi_schemas.c"
        "routes/emit/operation_body.c"
        "routes/emit/operation_param.c"
        "routes/emit/operation_response.c"
        "routes/emit/serve_mcp_stdio.c"
        "routes/emit/url_path.c"
        "routes/emit/url_query.c"
        "routes/emit/url_serialization.c"
        "routes/parse/cli_meta.c"
        "routes/parse/cli_security.c"
        "routes/parse/sync_gen.c"
        "transformers/c_lowering/c_lowering_legacy.c"
        "transformers/c_lowering/c_lowering_modern.c"
        "transformers/error_percolator/error_percolator_call_sites.c"
        "transformers/gnu_standardizer/gnu_standardizer_builtins.c"
        "transformers/gnu_standardizer/gnu_standardizer_expr.c"
        "transformers/gnu_standardizer/gnu_standardizer_helpers.c"
        "transformers/gnu_standardizer/gnu_standardizer_ranges.c"
        "transformers/gnu_standardizer/gnu_standardizer_types.c"
        "transformers/gnu_standardizer/gnu_standardizer_unroll.c"
        "transformers/safe_crt/safe_crt_parse.c"
        "transformers/safe_crt/safe_crt_rules.c"
        "transformers/safe_crt/safe_crt_rules_conv.c"
        "transformers/safe_crt/safe_crt_rules_env.c"
        "transformers/safe_crt/safe_crt_rules_io.c"
        "transformers/safe_crt/safe_crt_rules_str.c"
        "log.c"
        "code_writer.c"
        "context.c"
        "cdd_c_error.c"
        "json_dom.c"
        "functions/ffi/cdd_ffi_emit_common.c"
        "functions/parse/analysis.c"
        "classes/parse/inspector.c"
        "classes/parse/cdd_serde.c"
        "classes/parse/cdd_serde_lexer.c"
        "classes/parse/cdd_serde_container.c"
        "classes/parse/cdd_serde_field.c"
        "routes/parse/cli.c"
        "routes/emit/operation.c"
        "classes/emit/schema.c"
        "classes/parse/query_projection.c"
        "classes/parse/cli_parser.c"
        "classes/parse/mapping.c"
        "classes/parse/code2schema.c"
        "classes/parse/code2schema_defaults.c"
        "classes/parse/code2schema_enum.c"
        "classes/parse/code2schema_extras.c"
        "classes/parse/code2schema_mapper.c"
        "classes/parse/code2schema_merge.c"
        "classes/parse/code2schema_names.c"
        "classes/parse/code2schema_parser.c"
        "classes/parse/code2schema_props.c"
        "classes/parse/code2schema_unions.c"
        "classes/parse/code2schema_utils.c"
        "classes/parse/code2schema_writer.c"
        "classes/parse/cdd_lexer.c"
        "classes/parse/cdd_cst_parser.c"
        "classes/parse/cdd_cst_mutate.c"
          "classes/parse/cdd_cst_mutate_splice.c"
          "classes/parse/cdd_cst_scope.c"
          "classes/parse/cdd_cst_semantic.c"
          "classes/parse/cdd_cst_type_eval.c"
          "classes/parse/cdd_cst_cfg.c"
          "classes/parse/cdd_cst_escape.c"
  "classes/parse/cdd_cst_factory.c"
          "classes/parse/cdd_cst_builder.c"
          "classes/parse/cdd_cst_factory_format.c"
        "classes/parse/cdd_cst_query.c"
        "transformers/extern_c/extern_c.c"
        "transformers/msvc_port/msvc_port.c"
        "transformers/gnu_standardizer/gnu_standardizer.c"
        "transformers/error_percolator/error_percolator.c"
        "transformers/safe_crt/safe_crt.c"
        "transformers/macros/macros.c"
        "routes/parse/cli_cst.c"
        "classes/parse/cdd_cst_trivia.c"
        "classes/emit/cdd_cst_emit.c"
        "functions/emit/codegen.c"
        "functions/emit/build.c"
        "functions/emit/build_system.c"
        "functions/emit/cst_printer.c"
            "functions/emit/diff_generator.c"
        "functions/emit/weaver.c"
        "functions/emit/weaver_attributes.c"
        "functions/emit/client_body.c"
        "functions/emit/client_sig.c"
        "classes/emit/enum.c"
        "classes/emit/cli_struct.c"
        "classes/emit/cli_struct_help.c"
        "classes/emit/cli_struct_parse.c"
        "classes/emit/cli_struct_serialize.c"
        "classes/emit/json.c"
        "classes/emit/standalone_json.c"
        "classes/emit/form.c"
        "classes/emit/jwt.c"
        "classes/emit/oauth2_error.c"
        "functions/emit/make.c"
        "routes/emit/security.c"
        "classes/emit/struct.c"
        "classes/emit/struct_fields.c"
        "classes/emit/struct_reflect_emit.c"
        "classes/emit/struct_reflect_access.c"
        "classes/emit/struct_reflect_registry.c"
        "classes/emit/struct_reflect_validate.c"
        "classes/emit/struct_reflect_lifecycle.c"
        "classes/emit/struct_reflect_json.c"
        "classes/emit/types.c"
        "routes/emit/url.c"
        "routes/emit/serve_json_rpc.c"
        "cdd_api.c"
        "functions/parse/cst.c"
        "functions/parse/declarator.c"
        "docstrings/parse/doc.c"
        "functions/parse/fs.c"
        "classes/parse/initializer.c"
        "classes/parse/numeric.c"
        "routes/emit/aggregator.c"
        "routes/emit/client_gen.c"
        "routes/emit/client_gen_helpers.c"
        "routes/emit/client_gen_mcp.c"
        "routes/emit/client_gen_mocks.c"
        "routes/emit/client_gen_models.c"
        "routes/emit/client_gen_operations.c"
        "routes/emit/client_gen_preamble.c"
        "routes/emit/client_gen_url_utils_c1.c"
        "routes/emit/client_gen_url_utils_c2.c"
        "routes/emit/client_gen_url_utils_h.c"
        "routes/emit/cli_gen.c"
        "routes/emit/client_gui_gen.c"
        "routes/emit/server_gen.c"
        "openapi/parse/openapi.c"
        "openapi/parse/openapi_init.c"
        "openapi/parse/openapi_free.c"
        "openapi/parse/openapi_free_components.c"
        "openapi/parse/openapi_utils.c"
        "openapi/parse/openapi_examples.c"
        "openapi/parse/openapi_uri.c"
        "openapi/parse/openapi_find.c"
        "openapi/parse/openapi_copy_schema.c"
        "openapi/parse/openapi_copy_components.c"
        "openapi/parse/openapi_copy_operations.c"
        "openapi/parse/openapi_metadata.c"
        "openapi/parse/openapi_schemas.c"
        "openapi/parse/openapi_schema_ref.c"
        "openapi/parse/openapi_schema_registry.c"
        "openapi/parse/openapi_security.c"
        "openapi/parse/openapi_headers_links.c"
        "openapi/parse/openapi_parameters.c"
        "openapi/parse/openapi_media.c"
        "openapi/parse/openapi_responses.c"
        "openapi/parse/openapi_components.c"
        "openapi/parse/openapi_paths.c"
        "openapi/parse/openapi_validation.c"
        "openapi/emit/openapi.c"
        "functions/parse/preprocessor.c"
        "functions/parse/macro_evaluator.c"
        "functions/parse/audit.c"
        "routes/parse/sync.c"
        "functions/parse/refactor.c"
        "functions/parse/orchestrator.c"
        "functions/emit/rewriter_body.c"
        "functions/emit/rewriter_sig.c"
        "functions/emit/safe_crt.c"
        "functions/emit/diff.c"
        "tests/emit/schema2tests.c"
        "classes/emit/schema_codegen.c"
        "functions/parse/strategy.c"
        "functions/emit/sync.c"
        "functions/emit/patcher.c"
        "functions/parse/tokenizer.c"
        "routes/parse/url.c"
        "functions/parse/str.c"
        "functions/parse/db_loader.c"
        "functions/parse/desig_init.c"
        "functions/parse/makefile_scraper.c"
        "functions/parse/macro_overlay.c"
        "functions/parse/decl_hoist.c"
        "functions/parse/cmake_parser.c"
        "functions/parse/vla_analyzer.c"
        "functions/parse/vcpkg_integration.c"
        "functions/ffi/cdd_ffi_ir.c"
        "functions/ffi/cdd_ffi_ir_extractor.c"
        "functions/ffi/cdd_ffi_ir_json.c"
        "functions/ffi/cdd_ffi_emit_ir_json.c"
        "functions/dsl/cdd_dsl_tokenizer.c"
        "functions/dsl/cdd_dsl_parser.c"
        "functions/dsl/cdd_ffi_emit_dsl.c"
        "functions/ffi/cdd_ffi_emit_ada.c"
        "functions/ffi/cdd_ffi_emit_clojure.c"
        "functions/ffi/cdd_ffi_emit_cobol.c"
        "functions/ffi/cdd_ffi_emit_common_lisp.c"
        "functions/ffi/cdd_ffi_emit_cpp.c"
        "functions/ffi/cdd_ffi_emit_crystal.c"
        "functions/ffi/cdd_ffi_emit_csharp.c"
        "functions/ffi/cdd_ffi_emit_d.c"
        "functions/ffi/cdd_ffi_emit_dart.c"
        "functions/ffi/cdd_ffi_emit_delphi.c"
        "functions/ffi/cdd_ffi_emit_elixir.c"
        "functions/ffi/cdd_ffi_emit_erlang.c"
        "functions/ffi/cdd_ffi_emit_fortran.c"
        "functions/ffi/cdd_ffi_emit_fsharp.c"
        "functions/ffi/cdd_ffi_emit_go.c"
        "functions/ffi/cdd_ffi_emit_groovy.c"
        "functions/ffi/cdd_ffi_emit_haskell.c"
        "functions/ffi/cdd_ffi_emit_java.c"
        "functions/ffi/cdd_ffi_emit_julia.c"
        "functions/ffi/cdd_ffi_emit_kotlin.c"
        "functions/ffi/cdd_ffi_emit_lua.c"
        "functions/ffi/cdd_ffi_emit_matlab.c"
        "functions/ffi/cdd_ffi_emit_napi.c"
        "functions/ffi/cdd_ffi_emit_nim.c"
        "functions/ffi/cdd_ffi_emit_objc.c"
        "functions/ffi/cdd_ffi_emit_ocaml.c"
        "functions/ffi/cdd_ffi_emit_odin.c"
        "functions/ffi/cdd_ffi_emit_perl.c"
        "functions/ffi/cdd_ffi_emit_php.c"
        "functions/ffi/cdd_ffi_emit_python.c"
        "functions/ffi/cdd_ffi_emit_r.c"
        "functions/ffi/cdd_ffi_emit_racket.c"
        "functions/ffi/cdd_ffi_emit_ruby.c"
        "functions/ffi/cdd_ffi_emit_rust.c"
        "functions/ffi/cdd_ffi_emit_scala.c"
        "functions/ffi/cdd_ffi_emit_scheme.c"
        "functions/ffi/cdd_ffi_emit_swift.c"
        "functions/ffi/cdd_ffi_emit_tcl.c"
        "functions/ffi/cdd_ffi_emit_typescript.c"
        "functions/ffi/cdd_ffi_emit_vlang.c"
        "functions/ffi/cdd_ffi_emit_webassembly.c"
        "functions/ffi/cdd_ffi_emit_zig.c"
        "functions/ffi/cdd_ffi_variadic.c"
        "functions/parse/main.c"
        "tests/mocks/g_fail_io_after.c"
)

# Platform-specific crypto backend selection
if(EMSCRIPTEN OR CMAKE_SYSTEM_NAME STREQUAL "DOS" OR CMAKE_SYSTEM_NAME STREQUAL "WASI")
    list(APPEND Source_Files "functions/parse/crypto_wasm.c")
elseif(WIN32)
    list(APPEND Source_Files "functions/parse/crypto_wincrypt.c")
elseif(APPLE)
    list(APPEND Source_Files "functions/parse/crypto_apple.c")
else()
    list(APPEND Source_Files "functions/parse/crypto_openssl.c")
endif()

source_group("Source Files" FILES ${Source_Files})

add_library("${LIBRARY_NAME}" ${Header_Files} ${Source_Files})
c_cdd_apply_target_settings("${LIBRARY_NAME}")

if(_C_CDD_ENABLE_TESTS)
    target_compile_definitions("${LIBRARY_NAME}" PUBLIC CDD_BUILD_TESTS)
endif()

set(cdd_c_RESOLVED ON CACHE BOOL "Prevent c-abstract-http from fetching cdd-c" FORCE)

include(GNUInstallDirs)
target_include_directories(
    "${LIBRARY_NAME}"
    PUBLIC
        "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/mocks>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>"
        "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

# Platform system & crypto libraries
set(_C_CDD_LINKED_OPENSSL FALSE PARENT_SCOPE)
if(WIN32)
    target_link_libraries("${LIBRARY_NAME}" PRIVATE ws2_32 crypt32 advapi32)
elseif(NOT APPLE AND NOT EMSCRIPTEN AND NOT CMAKE_SYSTEM_NAME STREQUAL "DOS" AND NOT CMAKE_SYSTEM_NAME STREQUAL "WASI")
    find_package(OpenSSL QUIET)
    if(OpenSSL_FOUND AND TARGET OpenSSL::Crypto)
        target_link_libraries("${LIBRARY_NAME}" PRIVATE OpenSSL::Crypto)
        set(_C_CDD_LINKED_OPENSSL TRUE PARENT_SCOPE)
    else()
        target_compile_definitions("${LIBRARY_NAME}" PRIVATE CDD_NO_OPENSSL)
    endif()
endif()

target_link_libraries(
    "${LIBRARY_NAME}"
    PRIVATE
        cfs
        c89stringutils
    PUBLIC
        c_str_span
        c_str_span_compiler_flags
        "${PROJECT_NAME}_compiler_flags"
)

if(CDD_SQLITE STREQUAL "LINKED")
    find_package(unofficial-sqlite3 CONFIG QUIET)
    if(TARGET unofficial::sqlite3::sqlite3)
        target_link_libraries("${LIBRARY_NAME}" PRIVATE unofficial::sqlite3::sqlite3)
        target_compile_definitions("${LIBRARY_NAME}" PRIVATE USE_SQLITE_LINKED)
    else()
        find_package(SQLite3 QUIET)
        if(SQLite3_FOUND AND TARGET SQLite::SQLite3)
            target_link_libraries("${LIBRARY_NAME}" PRIVATE SQLite::SQLite3)
            target_compile_definitions("${LIBRARY_NAME}" PRIVATE USE_SQLITE_LINKED)
        endif()
    endif()
elseif(CDD_SQLITE STREQUAL "DYNAMIC")
    target_compile_definitions("${LIBRARY_NAME}" PRIVATE USE_SQLITE_DYNAMIC)
endif()

if(CDD_MYSQL STREQUAL "LINKED")
    target_compile_definitions("${LIBRARY_NAME}" PRIVATE USE_MYSQL_LINKED)
elseif(CDD_MYSQL STREQUAL "DYNAMIC")
    target_compile_definitions("${LIBRARY_NAME}" PRIVATE USE_MYSQL_DYNAMIC)
endif()

if(NOT WIN32 AND NOT CMAKE_SYSTEM_NAME STREQUAL "WASI" AND NOT CMAKE_SYSTEM_NAME STREQUAL "DOS")
    target_link_libraries("${LIBRARY_NAME}" PRIVATE ${CMAKE_DL_LIBS})
endif()

#######
# CLI #
#######

add_library("c_cdd_transform" INTERFACE)
target_link_libraries("c_cdd_transform" INTERFACE "${LIBRARY_NAME}")

set(EXEC_NAME "cdd-c")
set(TRANSFORM_EXEC_NAME "cdd-c-transform")
set(Cli_Source_Files "bin_cdd.c")

if(NOT CMAKE_SYSTEM_NAME STREQUAL "DOS")
    add_executable("${EXEC_NAME}" ${Cli_Source_Files})
    c_cdd_apply_target_settings("${EXEC_NAME}")
    if(_C_CDD_ENABLE_TESTS)
        target_link_libraries("${EXEC_NAME}" PRIVATE cdd_test_helpers)
    else()
        target_link_libraries("${EXEC_NAME}" PRIVATE "${LIBRARY_NAME}")
    endif()

    add_executable("${TRANSFORM_EXEC_NAME}" ${Cli_Source_Files})
    c_cdd_apply_target_settings("${TRANSFORM_EXEC_NAME}")
    target_compile_definitions("${TRANSFORM_EXEC_NAME}" PRIVATE CDD_C_TRANSFORMER_ONLY=1)
    target_link_libraries("${TRANSFORM_EXEC_NAME}" PRIVATE "c_cdd_transform")
    if(_C_CDD_ENABLE_TESTS)
        target_link_libraries("${TRANSFORM_EXEC_NAME}" PRIVATE cdd_test_helpers)
    endif()

    export(
        TARGETS "${TRANSFORM_EXEC_NAME}" "${EXEC_NAME}"
        FILE "${PROJECT_BINARY_DIR}/cdd-c-transformTargets.cmake"
    )
endif()

include(GenerateExportHeader)
set(_export_file "${CMAKE_CURRENT_BINARY_DIR}/${LIBRARY_NAME}_export.h")
generate_export_header("${LIBRARY_NAME}" EXPORT_FILE_NAME "${_export_file}")

#################
# Install rules #
#################

set_target_properties("${LIBRARY_NAME}" PROPERTIES
    VERSION "${${PROJECT_NAME}_VERSION}"
    SOVERSION "${${PROJECT_NAME}_VERSION_MAJOR}"
)

if(C_CDD_INSTALL)
    set(_exportable_libs "${LIBRARY_NAME}" "c_cdd_transform" "${PROJECT_NAME}_compiler_flags")
    if(C_CDD_IS_TOP_LEVEL)
        foreach(_dep_tgt cfs c89stringutils c89stringutils_compiler_flags c_str_span c_str_span_compiler_flags)
            if(TARGET ${_dep_tgt})
                get_target_property(_is_imp ${_dep_tgt} IMPORTED)
                get_target_property(_is_alias ${_dep_tgt} ALIASED_TARGET)
                if(NOT _is_imp AND NOT _is_alias)
                    list(APPEND _exportable_libs ${_dep_tgt})
                endif()
            endif()
        endforeach()
    endif()

    install(TARGETS ${_exportable_libs}
        EXPORT "${LIBRARY_NAME}Targets"
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    )

    set(_install_cli_bins "")
    if(TARGET "${EXEC_NAME}")
        list(APPEND _install_cli_bins "${EXEC_NAME}")
    endif()
    if(TARGET "${TRANSFORM_EXEC_NAME}")
        list(APPEND _install_cli_bins "${TRANSFORM_EXEC_NAME}")
    endif()
    if(_install_cli_bins)
        install(TARGETS ${_install_cli_bins}
            RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
        )
    endif()

    install(EXPORT "${LIBRARY_NAME}Targets"
        DESTINATION "${CMAKE_INSTALL_DATADIR}/${LIBRARY_NAME}"
    )

    install(FILES "${_export_file}"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    )
    install(DIRECTORY "${PROJECT_SOURCE_DIR}/include/"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    )
    install(FILES
        "classes/parse/cdd_cst_node.h"
        "classes/parse/cdd_token.h"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/classes/parse"
    )
endif()
