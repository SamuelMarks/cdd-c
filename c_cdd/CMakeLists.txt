###########
# Library #
###########

set(LIBRARY_NAME "${PROJECT_NAME}")

set(Header_Files
        "c_cdd_stdbool.h"
        "c_cdd_utils.h"
        "cst.h"
        "ll.h"
        "scanner_types.h")
source_group("Header Files" FILES "${Header_Files}")

set(Source_Files
        "c_cdd_utils.c"
        "cst.c"
        "ll.c"
        "scanner_types.c")
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

if (WIN32)
    find_path(C89STRINGUTILS_INCLUDE_DIRS "c89stringutils_string_extras.h")
    target_include_directories("${LIBRARY_NAME}" PRIVATE "${C89STRINGUTILS_INCLUDE_DIRS}")
endif (WIN32)

find_package(c_str_span CONFIG REQUIRED)
target_link_libraries(
        "${LIBRARY_NAME}"
        PUBLIC
        c_str_span c_str_span_compiler_flags
)

target_link_libraries(
        "${LIBRARY_NAME}"
        PRIVATE
        "${PROJECT_NAME}_compiler_flags"
)

set_target_properties("${LIBRARY_NAME}" PROPERTIES LINKER_LANGUAGE C)


#######
# CLI #
#######

set(EXEC_NAME "${LIBRARY_NAME}_cli")

set(Cli_Source_Files "main.c")
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
        "${LIBRARY_NAME}" "${PROJECT_NAME}_compiler_flags")
if (TARGET "${DEPENDANT_LIBRARY}")
    list(APPEND installable_libs "${DEPENDANT_LIBRARY}")
endif ()
install(TARGETS ${installable_libs}
        EXPORT "${LIBRARY_NAME}Targets"
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
install(EXPORT "${LIBRARY_NAME}Targets" DESTINATION "${CMAKE_INSTALL_DATADIR}/${LIBRARY_NAME}")

install(FILES ${Header_Files} "${_export_file}"
        TYPE "INCLUDE")
