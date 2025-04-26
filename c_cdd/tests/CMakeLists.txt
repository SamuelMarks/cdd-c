#########################
# Dependencies download #
#########################

set(GREATEST_CRC32C "c66c379f")
set(GREATEST_SHA256 "b3f89ecad2301c64c580f328cc03a74d92f4cd4bf42ab18d3d4bfb7b8be5b948")
set(DOWNLOAD_DIR "${PROJECT_BINARY_DIR}/test_downloads")

file(TO_NATIVE_PATH "${DOWNLOAD_DIR}" DOWNLOAD_DIR)
if (CMAKE_SYSTEM_NAME STREQUAL "Windows" AND NOT MSYS AND NOT CYGWIN)
    string(REPLACE "\\" "\\\\" DOWNLOAD_DIR "${DOWNLOAD_DIR}")
endif (CMAKE_SYSTEM_NAME STREQUAL "Windows" AND NOT MSYS AND NOT CYGWIN)

file(MAKE_DIRECTORY "${DOWNLOAD_DIR}")
set(GREATEST_BASEFILENAME "greatest.h")
set(GREATEST_FILE "${DOWNLOAD_DIR}/${GREATEST_BASEFILENAME}")
file(TO_NATIVE_PATH "${GREATEST_FILE}" GREATEST_FILE)
if (CMAKE_SYSTEM_NAME STREQUAL "Windows" AND NOT MSYS AND NOT CYGWIN)
    string(REPLACE "\\" "\\\\" GREATEST_FILE "${GREATEST_FILE}")
endif ()

set(GREATEST_URL "https://raw.githubusercontent.com/SamuelMarks/greatest/cmake-and-msvc/greatest.h")
file(DOWNLOAD "${GREATEST_URL}" "${GREATEST_FILE}"
        EXPECTED_HASH "SHA256=${GREATEST_SHA256}")

################
# Test library #
################

set(LIBRARY_NAME "test_c_cdd_lib")

set(Header_Files
#        "test_comment.h"
#        "test_function.h"
#        "test_literal_str.h"
#        "test_macro.h"
        "test_parsing.h"
        "test_json_from_and_to.h"
#        "test_struct.h"
#        "test_tokenizer.h"
#        "test_whitespace.h"
)
source_group("${LIBRARY_NAME} Header Files" FILES "${Header_Files}")

#add_library("${LIBRARY_NAME}" INTERFACE)
#target_sources("${LIBRARY_NAME}" INTERFACE "${Header_Files}")
#
#if(MSVC)
#    add_custom_target("${LIBRARY_NAME}.headers" SOURCES "${Header_Files}")
#endif()

add_library("${LIBRARY_NAME}" INTERFACE)
add_custom_target("${LIBRARY_NAME}-target" SOURCES "${Header_Files}")
set_property(TARGET "${LIBRARY_NAME}-target" PROPERTY FOLDER "${CMAKE_CURRENT_SOURCE_DIR}")

target_include_directories(
        "${LIBRARY_NAME}"
        INTERFACE
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/mocks>"
        "$<BUILD_INTERFACE:${DOWNLOAD_DIR}>"
)
if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.13")
    set_target_properties(
            "${LIBRARY_NAME}"
            PROPERTIES
            LINKER_LANGUAGE
            C
    )
endif ()

add_subdirectory("cdd_test_helpers")
target_link_libraries("${LIBRARY_NAME}" INTERFACE "cdd_test_helpers")

##################
# Top-level test #
##################

set(EXEC_NAME "test_c_cdd")

set(Source_Files "${EXEC_NAME}.c")
source_group("${EXEC_NAME} Source Files" FILES "${Source_Files}")

add_executable("${EXEC_NAME}" "${Source_Files}")

target_include_directories(
        "${EXEC_NAME}"
        PRIVATE
        "$<BUILD_INTERFACE:${DOWNLOAD_DIR}>"
)
set_target_properties(
        "${EXEC_NAME}"
        PROPERTIES
        LINKER_LANGUAGE
        C
)
target_link_libraries("${EXEC_NAME}" PUBLIC "${LIBRARY_NAME}")

find_package(c_str_span CONFIG REQUIRED)
target_link_libraries(
        "${EXEC_NAME}"
        PRIVATE
        c_str_span c_str_span_compiler_flags
)

add_test(NAME "${EXEC_NAME}" COMMAND "${EXEC_NAME}")

#################################
# Additional tests and/or mocks #
#################################

add_subdirectory("mocks")

target_link_libraries("${LIBRARY_NAME}" INTERFACE "simple_mocks")
