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
        "functions/emit/codegen.h"
        "functions/emit/build.h"
        "functions/emit/client_body.h"
        "functions/emit/client_sig.h"
        "classes/emit/enum.h"
        "classes/emit/json.h"
        "functions/emit/make.h"
        "tests/emit/sdk_tests.h"
        "routes/emit/security.h"
        "classes/emit/struct.h"
        "classes/emit/types.h"
        "routes/emit/url.h"
        "functions/parse/crypto_types.h"
        "functions/parse/cst.h"
        "functions/parse/declarator.h"
        "docstrings/parse/doc.h"
        "functions/parse/fs.h"
        "functions/emit/build_system.h"
        "functions/parse/http_curl.h"
        "functions/parse/http_types.h"
        "functions/parse/http_winhttp.h"
        "classes/parse/initializer.h"
        "classes/parse/numeric.h"
        "routes/emit/aggregator.h"
        "routes/emit/client_gen.h"
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
        "routes/parse/url.h"
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
        "functions/emit/codegen.c"
        "functions/emit/build.c"
        "functions/emit/client_body.c"
        "functions/emit/client_sig.c"
        "classes/emit/enum.c"
        "classes/emit/json.c"
        "functions/emit/make.c"
        "tests/emit/sdk_tests.c"
        "routes/emit/security.c"
        "classes/emit/struct.c"
        "classes/emit/types.c"
        "routes/emit/url.c"
        "functions/parse/cst.c"
        "functions/parse/declarator.c"
        "docstrings/parse/doc.c"
        "functions/parse/fs.c"
        "functions/emit/build_system.c"
        "functions/parse/http_types.c"
        "functions/parse/http_winhttp.c"
        "classes/parse/initializer.c"
        "classes/parse/numeric.c"
        "routes/emit/aggregator.c"
        "routes/emit/client_gen.c"
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
        "functions/parse/transport.c"
)

# Platform specific source selection to avoid duplicate symbols
if (WIN32)
    list(APPEND Source_Files "functions/parse/crypto_wincrypt.c")
else ()
    list(APPEND Source_Files "functions/parse/crypto_openssl.c")
endif ()

if (WIN32)
    option(C_CDD_USE_LIBCURL "Enable libcurl (defaults to `ON` on non-Windows)" OFF)
else ()
    option(C_CDD_USE_LIBCURL "Enable libcurl (defaults to `ON` on non-Windows)" ON)
endif ()

if (C_CDD_USE_LIBCURL)
    list(APPEND Source_Files "functions/parse/http_curl.c")
endif ()

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

set(EXEC_NAME "cdd-c")

set(Cli_Source_Files "functions/parse/main.c")
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
