###########
# Library #
###########

include(CheckIncludeFile)

set(LIBRARY_NAME "${PROJECT_NAME}")

set(Header_Files
        "functions/parse_analysis.h"
        "mocks/c_cdd_stdbool.h"
        "classes/parse_inspector.h"
        "routes/parse_cli.h"
        "routes/emit_operation.h"
        "classes/emit_schema.h"
        "classes/parse_mapping.h"
        "classes/parse_code2schema.h"
        "functions/emit_codegen.h"
        "functions/emit_build.h"
        "functions/emit_client_body.h"
        "functions/emit_client_sig.h"
        "classes/emit_enum.h"
        "classes/emit_json.h"
        "functions/emit_make.h"
        "tests/emit_sdk_tests.h"
        "routes/emit_security.h"
        "classes/emit_struct.h"
        "classes/emit_types.h"
        "routes/emit_url.h"
        "functions/parse_crypto_types.h"
        "functions/parse_cst.h"
        "functions/parse_declarator.h"
        "docstrings/parse_doc.h"
        "functions/parse_fs.h"
        "functions/emit_build_system.h"
        "functions/parse_http_curl.h"
        "functions/parse_http_types.h"
        "functions/parse_http_winhttp.h"
        "classes/parse_initializer.h"
        "classes/parse_numeric.h"
        "routes/emit_aggregator.h"
        "routes/emit_client_gen.h"
        "openapi/parse_openapi.h"
        "openapi/emit_openapi.h"
        "functions/parse_preprocessor.h"
        "functions/parse_audit.h"
        "routes/parse_sync.h"
        "functions/parse_refactor.h"
        "functions/parse_orchestrator.h"
        "functions/emit_rewriter_body.h"
        "functions/emit_rewriter_sig.h"
        "tests/emit_schema2tests.h"
        "classes/emit_schema_codegen.h"
        "functions/parse_strategy.h"
        "functions/emit_sync.h"
        "functions/emit_patcher.h"
        "functions/parse_tokenizer.h"
        "routes/parse_url.h"
)
source_group("Header Files" FILES "${Header_Files}")

set(Source_Files
        "functions/parse_analysis.c"
        "classes/parse_inspector.c"
        "routes/parse_cli.c"
        "routes/emit_operation.c"
        "classes/emit_schema.c"
        "classes/parse_mapping.c"
        "classes/parse_code2schema.c"
        "functions/emit_codegen.c"
        "functions/emit_build.c"
        "functions/emit_client_body.c"
        "functions/emit_client_sig.c"
        "classes/emit_enum.c"
        "classes/emit_json.c"
        "functions/emit_make.c"
        "tests/emit_sdk_tests.c"
        "routes/emit_security.c"
        "classes/emit_struct.c"
        "classes/emit_types.c"
        "routes/emit_url.c"
        "functions/parse_cst.c"
        "functions/parse_declarator.c"
        "docstrings/parse_doc.c"
        "functions/parse_fs.c"
        "functions/emit_build_system.c"
        "functions/parse_http_types.c"
        "functions/parse_http_winhttp.c"
        "classes/parse_initializer.c"
        "classes/parse_numeric.c"
        "routes/emit_aggregator.c"
        "routes/emit_client_gen.c"
        "openapi/parse_openapi.c"
        "openapi/emit_openapi.c"
        "functions/parse_preprocessor.c"
        "functions/parse_audit.c"
        "routes/parse_sync.c"
        "functions/parse_refactor.c"
        "functions/parse_orchestrator.c"
        "functions/emit_rewriter_body.c"
        "functions/emit_rewriter_sig.c"
        "tests/emit_schema2tests.c"
        "classes/emit_schema_codegen.c"
        "functions/parse_strategy.c"
        "functions/emit_sync.c"
        "functions/emit_patcher.c"
        "functions/parse_tokenizer.c"
        "routes/parse_url.c"
        "functions/parse_str.c"
        "functions/parse_transport.c"
)

# Platform specific source selection to avoid duplicate symbols
if (WIN32)
    list(APPEND Source_Files "functions/parse_crypto_wincrypt.c")
else ()
    list(APPEND Source_Files "functions/parse_crypto_openssl.c")
endif ()

# Conditionally add Curl source if package found (logic below)
# For now add it, dependency checks handle the include paths
list(APPEND Source_Files "functions/parse_http_curl.c")

source_group("Source Files" FILES "${Source_Files}")

add_library("${LIBRARY_NAME}" "${Header_Files}" "${Source_Files}")

include(GNUInstallDirs)
target_include_directories(
        "${LIBRARY_NAME}"
        PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/mocks>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>"
        "$<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>"
        "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

if (WIN32)
    option(C_CDD_USE_LIBCURL "Enable libcurl (defaults to `ON` on non-Windows)" OFF)
else ()
    option(C_CDD_USE_LIBCURL "Enable libcurl (defaults to `ON` on non-Windows)" ON)
endif ()

# Dependencies
if (C_CDD_USE_LIBCURL)
    if(NOT TARGET CURL::libcurl)
        if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.11" AND NOT DEFINED VCPKG_TOOLCHAIN)
            find_package(CURL REQUIRED)
        else ()
            find_package(CURL CONFIG REQUIRED)
        endif ()
    endif()
    if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.11" AND NOT DEFINED VCPKG_TOOLCHAIN)
        find_package(OpenSSL REQUIRED)
    else ()
        find_package(OpenSSL CONFIG REQUIRED)
    endif()
    target_link_libraries("${LIBRARY_NAME}" PRIVATE CURL::libcurl)
endif ()

if (WIN32)
    # WinCrypt is part of system libs usually
    # crypto_wincrypt might need linking against Crypt32.lib
    # http_winhttp needs Winhttp.lib
    # you can use crypto backend from windows with curl I think, TODO
    if (C_CDD_USE_LIBCURL)
        if(TARGET OpenSSL::SSL)
        target_link_libraries("${LIBRARY_NAME}" PRIVATE OpenSSL::SSL OpenSSL::Crypto)
    else()
        target_link_libraries("${LIBRARY_NAME}" PRIVATE ${OPENSSL_LIBRARIES})
    endif()
    endif (C_CDD_USE_LIBCURL)
else ()
    if(TARGET OpenSSL::SSL)
        target_link_libraries("${LIBRARY_NAME}" PRIVATE OpenSSL::SSL OpenSSL::Crypto)
    else()
        target_link_libraries("${LIBRARY_NAME}" PRIVATE ${OPENSSL_LIBRARIES})
    endif()
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

if(NOT TARGET c89stringutils)
    if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.11" AND NOT DEFINED VCPKG_TOOLCHAIN)
        find_package(c89stringutils REQUIRED)
    else ()
        find_package(c89stringutils CONFIG REQUIRED)
    endif ()
endif()
target_link_libraries("${LIBRARY_NAME}" PRIVATE c89stringutils c89stringutils_compiler_flags)


if(NOT TARGET c_str_span)
    if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.11" AND NOT DEFINED VCPKG_TOOLCHAIN)
    find_package(c_str_span REQUIRED)
else ()
    find_package(c_str_span CONFIG REQUIRED)
endif ()
endif()

target_link_libraries(
        "${LIBRARY_NAME}"
        PUBLIC
        c_str_span c_str_span_compiler_flags
)
if(NOT TARGET parson)
    if(NOT TARGET parson)
    if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.11" AND NOT DEFINED VCPKG_TOOLCHAIN)
    find_package(parson REQUIRED)
else ()
    find_package(parson CONFIG REQUIRED)
endif ()
endif()
endif()
if(TARGET parson::parson)
    target_link_libraries("${LIBRARY_NAME}" PRIVATE parson::parson)
else()
    target_link_libraries("${LIBRARY_NAME}" PRIVATE parson)
endif()

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

set(Cli_Source_Files "functions/parse_main.c")
source_group("${EXEC_NAME} Source Files" FILES "${Source_Files}")

add_executable("${EXEC_NAME}" "${Cli_Source_Files}")

target_link_libraries("${EXEC_NAME}" PRIVATE "${LIBRARY_NAME}")
set_target_properties("${EXEC_NAME}" PROPERTIES LINKER_LANGUAGE C)

if(NOT TARGET parson)
    if(NOT TARGET parson)
    if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.11" AND NOT DEFINED VCPKG_TOOLCHAIN)
    find_package(parson REQUIRED)
else ()
    find_package(parson CONFIG REQUIRED)
endif ()
endif()
endif()

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
