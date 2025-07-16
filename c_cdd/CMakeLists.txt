###########
# Library #
###########

include(CheckIncludeFile)

set(LIBRARY_NAME "${PROJECT_NAME}")

set(Header_Files
        "c_cdd_stdbool.h"
        "code2schema.h"
        "codegen.h"
        "cst_parser.h"
        "fs.h"
        "generate_build_system.h"
        "schema2tests.h"
        "schema_codegen.h"
        "sync_code.h"
        "tokenizer.h"
)
source_group("Header Files" FILES "${Header_Files}")

set(Source_Files
        "code2schema.c"
        "codegen.c"
        "cst_parser.c"
        "fs.c"
        "generate_build_system.c"
        "schema2tests.c"
        "schema_codegen.c"
        "sync_code.c"
        "tokenizer.c"
)
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
        PRIVATE
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
