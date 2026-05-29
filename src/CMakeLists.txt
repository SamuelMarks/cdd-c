add_compile_definitions(_CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_WARNINGS)
###########
# Library #
###########

include(CheckIncludeFile)

set(LIBRARY_NAME "${PROJECT_NAME}")

set(Header_Files
        "functions/parse/analysis.h"
        "mocks/c_cdd_stdbool.h"
        "classes/parse/inspector.h"
        "routes/parse/cli.h"
        "routes/emit/operation.h"
        "classes/emit/schema.h"
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
        "../include/routes/parse/cli_cst.h"
        "classes/parse/cdd_cst_trivia.h"
        "classes/emit/cdd_cst_emit.h"
        "functions/emit/codegen.h"
        "functions/emit/build.h"
        "functions/emit/client_body.h"
        "functions/emit/client_sig.h"
        "classes/emit/enum.h"
        "classes/emit/json.h"
        "classes/emit/form.h"
        "classes/emit/jwt.h"
        "classes/emit/oauth2_error.h"
        "functions/emit/make.h"
        "tests/emit/sdk_tests.h"
        "routes/emit/security.h"
        "classes/emit/struct.h"
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
        "openapi/emit/openapi.h"
        "functions/parse/preprocessor.h"
        "functions/parse/audit.h"
        "routes/parse/sync.h"
        "functions/parse/refactor.h"
        "functions/parse/orchestrator.h"
        "functions/emit/rewriter_body.h"
        "functions/emit/rewriter_sig.h"
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
        "win_compat_sym.h"
)
source_group("Header Files" FILES "${Header_Files}")

set(Source_Files
        "functions/parse/analysis.c"
        "classes/parse/inspector.c"
        "routes/parse/cli.c"
        "routes/emit/operation.c"
        "classes/emit/schema.c"
        "classes/parse/mapping.c"
        "classes/parse/code2schema.c"
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
        "classes/emit/json.c"
        "classes/emit/standalone_json.c"
        "classes/emit/form.c"
        "classes/emit/jwt.c"
        "classes/emit/oauth2_error.c"
        "functions/emit/make.c"
        "tests/emit/sdk_tests.c"
        "routes/emit/security.c"
        "classes/emit/struct.c"
        "classes/emit/types.c"
        "routes/emit/url.c"
        "routes/emit/serve_json_rpc.c"
        "cdd_api.c"
        "functions/parse/cst.c"
        "functions/parse/declarator.c"
        "docstrings/parse/doc.c"
        "functions/parse/fs.c"
        "functions/emit/build_system.c"
        "classes/parse/initializer.c"
        "classes/parse/numeric.c"
        "routes/emit/aggregator.c"
        "routes/emit/client_gen.c"
        "routes/emit/cli_gen.c"
        "routes/emit/client_gui_gen.c"
        "routes/emit/server_gen.c"
        "openapi/parse/openapi.c"
        "openapi/emit/openapi.c"
        "functions/parse/preprocessor.c"
        "functions/parse/audit.c"
        "routes/parse/sync.c"
        "functions/parse/refactor.c"
        "functions/parse/orchestrator.c"
        "functions/emit/rewriter_body.c"
        "functions/emit/rewriter_sig.c"
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
        "functions/parse/main.c"
)

# Platform specific source selection to avoid duplicate symbols
if (WIN32)
    list(APPEND Source_Files "functions/parse/crypto_wincrypt.c")
elseif (APPLE)
    if(NOT HTTP_ONLY)
        list(APPEND Source_Files "functions/parse/crypto_apple.c")
    endif()
else ()
    if(NOT HTTP_ONLY)
        list(APPEND Source_Files "functions/parse/crypto_openssl.c")
    endif()
endif ()



if (C_CDD_USE_LIBCURL)
    list(APPEND Source_Files)
endif ()

source_group("Source Files" FILES "${Source_Files}")

add_library("${LIBRARY_NAME}" "${Header_Files}" "${Source_Files}")


include(GNUInstallDirs)
target_include_directories(
        "${LIBRARY_NAME}"
        PUBLIC
        "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/mocks>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>"
        "$<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>"
        "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

# Dependencies
if (BUILD_TESTING OR C_CDD_BUILD_TESTS)
target_compile_definitions("${LIBRARY_NAME}" PUBLIC CDD_BUILD_TESTS)
if (C_CDD_USE_LIBCURL)
    if(NOT TARGET CURL::libcurl)
        if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.11" AND NOT VCPKG_TOOLCHAIN)
            find_package(CURL REQUIRED)
        else ()
            find_package(CURL CONFIG QUIET)
        endif ()
    endif()
    if (NOT HTTP_ONLY)
        if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.11" AND NOT VCPKG_TOOLCHAIN)
            find_package(OpenSSL CONFIG QUIET)
            if(NOT OPENSSL_FOUND)
                find_package(OpenSSL QUIET)
            endif()
            if(NOT OPENSSL_FOUND)
                find_package(PkgConfig QUIET)
                if(PKG_CONFIG_FOUND)
                    pkg_check_modules(PC_OPENSSL QUIET openssl)
                    if(PC_OPENSSL_FOUND)
                        find_path(OPENSSL_INCLUDE_DIR NAMES openssl/ssl.h HINTS ${PC_OPENSSL_INCLUDE_DIRS})
                        find_library(OPENSSL_SSL_LIBRARY NAMES ssl libssl HINTS ${PC_OPENSSL_LIBRARY_DIRS})
                        find_library(OPENSSL_CRYPTO_LIBRARY NAMES crypto libcrypto HINTS ${PC_OPENSSL_LIBRARY_DIRS})
                        if(OPENSSL_INCLUDE_DIR AND OPENSSL_SSL_LIBRARY AND OPENSSL_CRYPTO_LIBRARY)
                            set(OPENSSL_FOUND TRUE)
                            set(OPENSSL_INCLUDE_DIRS ${OPENSSL_INCLUDE_DIR})
                            set(OPENSSL_LIBRARIES ${OPENSSL_SSL_LIBRARY} ${OPENSSL_CRYPTO_LIBRARY})
                        endif()
                    endif()
                endif()
            endif()
            if(NOT OPENSSL_FOUND)
                find_package(OpenSSL CONFIG QUIET)
            if(NOT OPENSSL_FOUND)
                find_package(OpenSSL QUIET)
            endif()
            if(NOT OPENSSL_FOUND)
                find_package(PkgConfig QUIET)
                if(PKG_CONFIG_FOUND)
                    pkg_check_modules(PC_OPENSSL QUIET openssl)
                    if(PC_OPENSSL_FOUND)
                        find_path(OPENSSL_INCLUDE_DIR NAMES openssl/ssl.h HINTS )
                        find_library(OPENSSL_SSL_LIBRARY NAMES ssl libssl HINTS )
                        find_library(OPENSSL_CRYPTO_LIBRARY NAMES crypto libcrypto HINTS )
                        if(OPENSSL_INCLUDE_DIR AND OPENSSL_SSL_LIBRARY AND OPENSSL_CRYPTO_LIBRARY)
                            set(OPENSSL_FOUND TRUE)
                            set(OPENSSL_INCLUDE_DIRS )
                            set(OPENSSL_LIBRARIES  )
                        endif()
                    endif()
                endif()
            endif()
            if(NOT OPENSSL_FOUND)
                find_package(OpenSSL REQUIRED)
            endif()
            endif()
        else ()
            find_package(OpenSSL CONFIG QUIET)
        endif()
    endif()
    target_link_libraries("${LIBRARY_NAME}" PRIVATE CURL::libcurl)
    target_compile_definitions("${LIBRARY_NAME}" PUBLIC C_CDD_USE_LIBCURL)
endif ()
endif ()

if (WIN32)
    # WinCrypt is part of system libs usually
    # crypto_wincrypt might need linking against Crypt32.lib
    # http_winhttp needs Winhttp.lib
    # you can use crypto backend from windows with curl I think, TODO
    if (BUILD_TESTING OR C_CDD_BUILD_TESTS)
    if (C_CDD_USE_LIBCURL)
        if(TARGET OpenSSL::SSL)
        target_link_libraries("${LIBRARY_NAME}" PRIVATE OpenSSL::SSL OpenSSL::Crypto)
    else()
        target_link_libraries("${LIBRARY_NAME}" PRIVATE ${OPENSSL_LIBRARIES})
    endif()
    endif (C_CDD_USE_LIBCURL)
    endif ()
elseif (APPLE)
    # Apple uses CommonCrypto, no OpenSSL needed
else ()
    if (BUILD_TESTING OR C_CDD_BUILD_TESTS)
    if (C_CDD_USE_LIBCURL AND NOT HTTP_ONLY)
        if(TARGET OpenSSL::SSL)
            target_link_libraries("${LIBRARY_NAME}" PRIVATE OpenSSL::SSL OpenSSL::Crypto)
        else()
            target_link_libraries("${LIBRARY_NAME}" PRIVATE ${OPENSSL_LIBRARIES})
        endif()
    elseif (NOT HTTP_ONLY)
        if(TARGET OpenSSL::SSL)
            target_link_libraries("${LIBRARY_NAME}" PRIVATE OpenSSL::SSL OpenSSL::Crypto)
        else()
            target_link_libraries("${LIBRARY_NAME}" PRIVATE ${OPENSSL_LIBRARIES} crypto ssl)
        endif()
    endif ()
    endif ()
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
    if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.11" AND NOT VCPKG_TOOLCHAIN)
        find_package(c89stringutils REQUIRED)
    else ()
        find_package(c89stringutils CONFIG QUIET)
    endif ()
endif()
target_link_libraries("${LIBRARY_NAME}" PRIVATE cfs c89stringutils)


if(NOT TARGET c_str_span)
    if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.11" AND NOT VCPKG_TOOLCHAIN)
    find_package(c_str_span REQUIRED)
else ()
    find_package(c_str_span CONFIG QUIET)
endif ()
endif()

target_link_libraries(
        "${LIBRARY_NAME}"
        PUBLIC
        c_str_span c_str_span_compiler_flags
)
if(NOT TARGET parson)
    if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.11" AND NOT VCPKG_TOOLCHAIN)
    find_package(parson REQUIRED)
else ()
    find_package(parson CONFIG QUIET)
endif ()
endif()
target_link_libraries("${LIBRARY_NAME}" PRIVATE parson::parson)



if(CDD_SQLITE STREQUAL "LINKED")
    find_package(unofficial-sqlite3 CONFIG QUIET)
    if(TARGET unofficial::sqlite3::sqlite3)
        target_link_libraries("${LIBRARY_NAME}" PRIVATE unofficial::sqlite3::sqlite3)
        target_compile_definitions("${LIBRARY_NAME}" PRIVATE USE_SQLITE_LINKED)
    else()
        find_package(SQLite3 QUIET)
        if(SQLite3_FOUND)
            target_link_libraries("${LIBRARY_NAME}" PRIVATE SQLite::SQLite3)
            target_compile_definitions("${LIBRARY_NAME}" PRIVATE USE_SQLITE_LINKED)
        endif()
    endif()
elseif(CDD_SQLITE STREQUAL "DYNAMIC")
    target_compile_definitions("${LIBRARY_NAME}" PRIVATE USE_SQLITE_DYNAMIC)
    if(NOT WIN32)
        target_link_libraries("${LIBRARY_NAME}" PRIVATE ${CMAKE_DL_LIBS})
    endif()
endif()

if(CDD_MYSQL STREQUAL "LINKED")
    target_compile_definitions("${LIBRARY_NAME}" PRIVATE USE_MYSQL_LINKED)
elseif(CDD_MYSQL STREQUAL "DYNAMIC")
    target_compile_definitions("${LIBRARY_NAME}" PRIVATE USE_MYSQL_DYNAMIC)
    if(NOT WIN32)
        target_link_libraries("${LIBRARY_NAME}" PRIVATE ${CMAKE_DL_LIBS})
    endif()
endif()
if(NOT WIN32)
    target_link_libraries("${LIBRARY_NAME}" PRIVATE ${CMAKE_DL_LIBS})
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

set(EXEC_NAME "cdd-c")

set(Cli_Source_Files "bin_cdd.c")
source_group("${EXEC_NAME} Source Files" FILES "${Source_Files}")

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
if (NOT CMAKE_SYSTEM_NAME STREQUAL "DOS")
add_executable("${EXEC_NAME}" "${Cli_Source_Files}")

target_link_libraries("${EXEC_NAME}" PRIVATE "${LIBRARY_NAME}")
set_target_properties("${EXEC_NAME}" PROPERTIES LINKER_LANGUAGE C)
endif()
endif()


if(NOT TARGET parson)
    if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.11" AND NOT VCPKG_TOOLCHAIN)
    find_package(parson REQUIRED)
else ()
    find_package(parson CONFIG QUIET)
endif ()
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

if (WIN32 AND MINGW)
    target_link_libraries("${LIBRARY_NAME}" PRIVATE ws2_32 winhttp wininet crypt32 advapi32 secur32 bcrypt)
endif()

if (APPLE)
    find_library(COREFOUNDATION_LIBRARY CoreFoundation)
    find_library(CFNETWORK_LIBRARY CFNetwork)
    target_link_libraries("${LIBRARY_NAME}" PRIVATE ${COREFOUNDATION_LIBRARY} ${CFNETWORK_LIBRARY})
endif()
