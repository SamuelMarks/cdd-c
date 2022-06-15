#########################
# Dependencies download #
#########################

set(GREATEST_CRC32C "c66c379f")
set(GREATEST_SHA256 "1292593d95c35eeccc89ffa1c91d6fe53b49f81cbf2c2b7758842b7f3186fcc2")
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

set(GREATEST_URL "https://raw.githubusercontent.com/silentbicycle/greatest/11a6af1/greatest.h")
file(DOWNLOAD "${GREATEST_URL}" "${GREATEST_FILE}"
        EXPECTED_HASH "SHA256=${GREATEST_SHA256}")

################
# Test library #
################

set(LIBRARY_NAME "test_c_cdd_lib")

set(Header_Files
        "test_comment.h"
        "test_function.h"
        "test_ll.h"
        "test_literal_str.h"
        "test_macro.h"
        "test_struct.h"
        "test_tokenizer.h"
        "test_whitespace.h")
source_group("${LIBRARY_NAME} Header Files" FILES "${Header_Files}")

add_library("${LIBRARY_NAME}" INTERFACE "${Header_Files}")

target_include_directories(
        "${LIBRARY_NAME}"
        INTERFACE
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>"
        "$<BUILD_INTERFACE:${DOWNLOAD_DIR}>"
)
set_target_properties(
        "${LIBRARY_NAME}"
        PROPERTIES
        LINKER_LANGUAGE
        C
)

if (WIN32)
    find_path(C89STRINGUTILS_INCLUDE_DIRS "c89stringutils_string_extras.h")
    target_include_directories("${LIBRARY_NAME}" INTERFACE "${C89STRINGUTILS_INCLUDE_DIRS}")
endif (WIN32)

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
