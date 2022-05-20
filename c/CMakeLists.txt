###########
# Library #
###########

set(LIBRARY_NAME "c")

include(FindFLEX)
include(FindBISON)

if (CMAKE_HOST_SYSTEM_NAME MATCHES "Darwin")
    execute_process(
            COMMAND brew --prefix bison
            RESULT_VARIABLE BREW_BISON
            OUTPUT_VARIABLE BREW_BISON_PREFIX
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if (BREW_BISON EQUAL 0 AND EXISTS "${BREW_BISON_PREFIX}")
        message(STATUS "Found Bison keg installed by Homebrew at ${BREW_BISON_PREFIX}")
        set(BISON_EXECUTABLE "${BREW_BISON_PREFIX}/bin/bison")
    endif()

    execute_process(
            COMMAND brew --prefix flex
            RESULT_VARIABLE BREW_FLEX
            OUTPUT_VARIABLE BREW_FLEX_PREFIX
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if (BREW_FLEX EQUAL 0 AND EXISTS "${BREW_FLEX_PREFIX}")
        message(STATUS "Found Flex keg installed by Homebrew at ${BREW_FLEX_PREFIX}")
        set(FLEX_EXECUTABLE "${BREW_FLEX_PREFIX}/bin/flex")
        set(FL_LIBRARY "${BREW_FLEX_PREFIX}/lib/libfl.dylib")
    endif ()
endif ()

find_package(BISON)
find_package(FLEX)

BISON_TARGET(MyParser c11.y "${CMAKE_CURRENT_BINARY_DIR}/parser.c")
FLEX_TARGET(MyScanner c11.l  "${CMAKE_CURRENT_BINARY_DIR}/lexer.c")
ADD_FLEX_BISON_DEPENDENCY(MyScanner MyParser)

set(Header_Files "${CMAKE_CURRENT_BINARY_DIR}/parser.h") #"${CMAKE_CURRENT_BINARY_DIR}/lexer.c")
source_group("Header Files" FILES "${Header_Files}")

set(Source_Files "${CMAKE_CURRENT_BINARY_DIR}/parser.c" "${CMAKE_CURRENT_BINARY_DIR}/lexer.c")
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
target_link_libraries(
        "${LIBRARY_NAME}"
        INTERFACE
        "${PROJECT_NAME}_compiler_flags"
)
set_target_properties("${LIBRARY_NAME}" PROPERTIES LINKER_LANGUAGE C)

include(GenerateExportHeader)
set(_export_file "${CMAKE_CURRENT_BINARY_DIR}/${LIBRARY_NAME}_export.h")
generate_export_header("${LIBRARY_NAME}" EXPORT_FILE_NAME "${_export_file}")

#######
# CLI #
#######

set(EXEC_NAME "${LIBRARY_NAME}_cli")

set(Cli_Source_Files "main.c")
source_group("${EXEC_NAME} Source Files" FILES "${Source_Files}")

add_executable(
        "${EXEC_NAME}"
        "${Cli_Source_Files}"
        ${BISON_MyParser_OUTPUTS}
        ${FLEX_MyScanner_OUTPUTS}
)
target_include_directories(
        "${LIBRARY_NAME}"
        PRIVATE
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)
target_link_libraries("${EXEC_NAME}" ${FLEX_LIBRARIES})

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
install(FILES "${_export_file}" ${Header_Files}
        TYPE "INCLUDE")
