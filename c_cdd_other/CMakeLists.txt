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
        "schema_codegen.h"
        "sync_code.h"
        "tokenizer.h"

#        "c_cdd_utils.h"
#        "cst.h"
#        "cst_parser_helpers.h"
#        "cst_parser_types.h"
#        "str_includes.h"
#        "tokenizer.h"
#        "tokenizer_helpers.h"
#        "tokenizer_types.h"
)
source_group("Header Files" FILES "${Header_Files}")

set(Source_Files
        "code2schema.c"
        "codegen.c"
        "cst_parser.c"
        "schema_codegen.c"
        "sync_code.c"
        "tokenizer.c"

#        "c_cdd_utils.c"
#        "cst.c"
#        "cst_parser_helpers.c"
#        "cst_parser_types.c"
#        "tokenizer.c"
#        "tokenizer_helpers.c"
#        "tokenizer_types.c"
)
source_group("Source Files" FILES "${Source_Files}")

add_library("${LIBRARY_NAME}" "${Header_Files}" "${Source_Files}")

include(GNUInstallDirs)
target_include_directories(
        "${LIBRARY_NAME}"
        PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

if (NOT DEFINED HAVE_ASPRINTF)
    if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.11" AND NOT DEFINED VCPKG_TOOLCHAIN)
        find_package(c89stringutils REQUIRED)
    else ()
        find_package(c89stringutils CONFIG REQUIRED)
    endif ()
    target_link_libraries("${LIBRARY_NAME}" PRIVATE c89stringutils c89stringutils_compiler_flags)
endif (NOT DEFINED HAVE_ASPRINTF)

if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.11" AND NOT DEFINED VCPKG_TOOLCHAIN AND VCPKG_TOOLCHAIN)
    find_package(c_str_span REQUIRED)
else ()
    find_package(c_str_span CONFIG REQUIRED)
endif ()


target_link_libraries(
        "${LIBRARY_NAME}"
        PRIVATE
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
