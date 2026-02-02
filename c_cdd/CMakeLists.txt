###########
# Library #
###########

include(CheckIncludeFile)

set(LIBRARY_NAME "${PROJECT_NAME}")

set(Header_Files
        "analysis.h"
        "c_cdd_stdbool.h"
        "c_inspector.h"
        "c2openapi_cli.h"
        "c2openapi_operation.h"
        "c2openapi_schema.h"
        "c_mapping.h"
        "code2schema.h"
        "codegen.h"
        "codegen_build.h"
        "codegen_client_body.h"
        "codegen_client_sig.h"
        "codegen_enum.h"
        "codegen_json.h"
        "codegen_make.h"
        "codegen_sdk_tests.h"
        "codegen_security.h"
        "codegen_struct.h"
        "codegen_types.h"
        "codegen_url.h"
        "crypto_types.h"
        "cst_parser.h"
        "declarator_parser.h"
        "doc_parser.h"
        "fs.h"
        "generate_build_system.h"
        "http_curl.h"
        "http_types.h"
        "http_winhttp.h"
        "initializer_parser.h"
        "numeric_parser.h"
        "openapi_aggregator.h"
        "openapi_client_gen.h"
        "openapi_loader.h"
        "openapi_writer.h"
        "preprocessor.h"
        "project_audit.h"
        "refactor_engine.h"
        "refactor_orchestrator.h"
        "rewriter_body.h"
        "rewriter_sig.h"
        "schema2tests.h"
        "schema_codegen.h"
        "strategy_safety.h"
        "sync_code.h"
        "text_patcher.h"
        "tokenizer.h"
)
source_group("Header Files" FILES "${Header_Files}")

set(Source_Files
        "analysis.c"
        "c_inspector.c"
        "c2openapi_cli.c"
        "c2openapi_operation.c"
        "c2openapi_schema.c"
        "c_mapping.c"
        "code2schema.c"
        "codegen.c"
        "codegen_build.c"
        "codegen_client_body.c"
        "codegen_client_sig.c"
        "codegen_enum.c"
        "codegen_json.c"
        "codegen_make.c"
        "codegen_sdk_tests.c"
        "codegen_security.c"
        "codegen_struct.c"
        "codegen_types.c"
        "codegen_url.c"
        "cst_parser.c"
        "declarator_parser.c"
        "doc_parser.c"
        "fs.c"
        "generate_build_system.c"
        "http_types.c"
        "http_winhttp.c"
        "initializer_parser.c"
        "numeric_parser.c"
        "openapi_aggregator.c"
        "openapi_client_gen.c"
        "openapi_loader.c"
        "openapi_writer.c"
        "preprocessor.c"
        "project_audit.c"
        "refactor_engine.c"
        "refactor_orchestrator.c"
        "rewriter_body.c"
        "rewriter_sig.c"
        "schema2tests.c"
        "schema_codegen.c"
        "strategy_safety.c"
        "sync_code.c"
        "text_patcher.c"
        "tokenizer.c"
        "str_utils.c"
        "transport_factory.c"
)

# Platform specific source selection to avoid duplicate symbols
if (WIN32)
    list(APPEND Source_Files "crypto_wincrypt.c")
else ()
    list(APPEND Source_Files "crypto_openssl.c")
endif ()

# Conditionally add Curl source if package found (logic below)
# For now add it, dependency checks handle the include paths
list(APPEND Source_Files "http_curl.c")

source_group("Source Files" FILES "${Source_Files}")

add_library("${LIBRARY_NAME}" "${Header_Files}" "${Source_Files}")

include(GNUInstallDirs)
target_include_directories(
        "${LIBRARY_NAME}"
        PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>"
        "$<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>"
        "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

# Dependencies
if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.11" AND NOT DEFINED VCPKG_TOOLCHAIN)
    find_package(CURL REQUIRED)
    find_package(OpenSSL REQUIRED)
else ()
    find_package(CURL CONFIG REQUIRED)
    find_package(OpenSSL CONFIG REQUIRED)
endif ()

target_link_libraries("${LIBRARY_NAME}" PRIVATE CURL::libcurl)

if (WIN32)
    # WinCrypt is part of system libs usually
    # crypto_wincrypt might need linking against Crypt32.lib
    # http_winhttp needs Winhttp.lib
else ()
    target_link_libraries("${LIBRARY_NAME}" PRIVATE OpenSSL::SSL OpenSSL::Crypto)
endif ()

if (MSVC)
    find_library(PATHCCH_LIB
            NAMES Pathcch
            HINTS
            ${CMAKE_SYSTEM_LIBRARY_PATH}
            PATH_SUFFIXES
            "um\\x64"
            "um\\x86"
    )
    if(PATHCCH_LIB)
      target_link_libraries("${LIBRARY_NAME}" PUBLIC "${PATHCCH_LIB}")
      add_definitions(-DPATHCCH_LIB)
    else ()
        find_library(API_MS_WIN_CORE_PATH_LIBRARY
                NAMES API-MS-Win-Core-Path-l1-1-0.dll
                PATHS ENV PATH
                NO_DEFAULT_PATH
        )

        if(API_MS_WIN_CORE_PATH_LIBRARY)
            message(STATUS "API-MS-Win-Core-Path-l1-1-0.dll found: ${API_MS_WIN_CORE_PATH_LIBRARY}")
            set(PATHCCH_LIB "${API_MS_WIN_CORE_PATH_LIBRARY}")
            target_compile_definitions("${LIBRARY_NAME}" PUBLIC "${API_MS_WIN_CORE_PATH_LIBRARY}")
            add_definitions(-DPATHCCH_LIB)
        else()
            message(STATUS "API-MS-Win-Core-Path-l1-1-0.dll not found")
        endif()
        find_library(KERNELBASE_LIBRARY
                NAMES KernelBase
                HINTS ENV PATH
                PATHS "C:/Windows/System32"
                DOC "Path to KernelBase.dll"
        )

        if(KERNELBASE_LIBRARY)
            message(STATUS "KernelBase.dll found at: ${KERNELBASE_LIBRARY}")
            target_compile_definitions("${LIBRARY_NAME}" PUBLIC "${KERNELBASE_LIBRARY}")
            add_definitions(-DPATHCCH_LIB)
        else()
            message(STATUS "KernelBase.dll not found.")
            set(PATHCCH_LIB "${KERNELBASE_LIBRARY}")
        endif()
    endif (PATHCCH_LIB)
endif (MSVC)

if (NOT DEFINED HAVE_ASPRINTF)
    if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.11" AND NOT DEFINED VCPKG_TOOLCHAIN)
        find_package(c89stringutils REQUIRED)
    else ()
        find_package(c89stringutils CONFIG REQUIRED)
    endif ()
    target_link_libraries("${LIBRARY_NAME}" PRIVATE c89stringutils c89stringutils_compiler_flags)
endif (NOT DEFINED HAVE_ASPRINTF)

if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.11" AND NOT DEFINED VCPKG_TOOLCHAIN)
    find_package(c_str_span REQUIRED)
else ()
    find_package(c_str_span CONFIG REQUIRED)
endif ()

target_link_libraries(
        "${LIBRARY_NAME}"
        PUBLIC
        c_str_span c_str_span_compiler_flags
)
if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.11" AND NOT DEFINED VCPKG_TOOLCHAIN)
    find_package(parson REQUIRED)
else ()
    find_package(parson CONFIG REQUIRED)
endif ()
target_link_libraries(
        "${LIBRARY_NAME}"
        PRIVATE
        "parson::parson"
)

target_link_libraries(
        "${LIBRARY_NAME}"
        PUBLIC
        "${PROJECT_NAME}_compiler_flags"
)

option(DEBUG_SCANNER "Whether to print debug messages for the scan phase" OFF)
if (DEBUG_SCANNER)
    message(STATUS "DEBUG_SCANNER = ${DEBUG_SCANNER}")
    target_compile_definitions(
            "${LIBRARY_NAME}"
            PUBLIC
            "-DDEBUG_SCANNER=1"
    )
endif (DEBUG_SCANNER)

set_target_properties("${LIBRARY_NAME}" PROPERTIES LINKER_LANGUAGE C)

#######
# CLI #
#######

set(EXEC_NAME "${LIBRARY_NAME}_cli")

set(Cli_Source_Files "main.c" str_includes.h)
source_group("${EXEC_NAME} Source Files" FILES "${Source_Files}")

add_executable("${EXEC_NAME}" "${Cli_Source_Files}")

target_link_libraries("${EXEC_NAME}" PRIVATE "${LIBRARY_NAME}")
set_target_properties("${EXEC_NAME}" PROPERTIES LINKER_LANGUAGE C)

if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.11" AND NOT DEFINED VCPKG_TOOLCHAIN)
    find_package(parson REQUIRED)
else ()
    find_package(parson CONFIG REQUIRED)
endif ()

include(GenerateExportHeader)
set(_export_file "${CMAKE_CURRENT_BINARY_DIR}/${LIBRARY_NAME}_export.h")
generate_export_header("${LIBRARY_NAME}" EXPORT_FILE_NAME "${_export_file}")

#################
# Install rules #
#################

include(GNUInstallDirs)

# setup the version numbering
set_property(TARGET "${LIBRARY_NAME}" PROPERTY VERSION "${${PROJECT_NAME}_VERSION}")
set_property(TARGET "${LIBRARY_NAME}" PROPERTY SOVERSION "${${PROJECT_NAME}_VERSION_MAJOR}")

set(installable_libs # "${EXEC_NAME}"
        "${LIBRARY_NAME}")
if (TARGET "${DEPENDANT_LIBRARY}")
    list(APPEND installable_libs "${DEPENDANT_LIBRARY}")
endif ()
if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.13")
    list(APPEND installable_libs "${PROJECT_NAME}_compiler_flags")
endif ()
install(TARGETS ${installable_libs}
        EXPORT "${LIBRARY_NAME}Targets"
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
install(EXPORT "${LIBRARY_NAME}Targets" DESTINATION "${CMAKE_INSTALL_DATADIR}/${LIBRARY_NAME}")

install(FILES ${Header_Files} "${_export_file}"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")