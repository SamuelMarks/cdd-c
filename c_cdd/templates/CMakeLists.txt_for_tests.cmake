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

##################
# Top-level test #
##################

set(EXEC_NAME "test_${LIBRARY_NAME}")

set(Header_Files "test_${LIBRARY_NAME}.h")
source_group("${EXEC_NAME} Header Files" FILES "${Header_Files}")

set(Source_Files "test_main.c")
source_group("${EXEC_NAME} Source Files" FILES "${Source_Files}")

add_executable("${EXEC_NAME}" "${Header_Files}" "${Source_Files}")

include(GNUInstallDirs)
target_include_directories(
        "${EXEC_NAME}"
        PRIVATE
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>"
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
        "$<BUILD_INTERFACE:${DOWNLOAD_DIR}>"
)
set_target_properties(
        "${EXEC_NAME}"
        PROPERTIES
        LINKER_LANGUAGE
        C
)

find_package(parson CONFIG REQUIRED)
target_link_libraries(
        "${EXEC_NAME}"
        PRIVATE
        "parson::parson"
)
target_link_libraries(
        "${EXEC_NAME}"
        PUBLIC
        "${LIBRARY_NAME}"
)

find_package(c89stringutils CONFIG REQUIRED)
target_link_libraries("${EXEC_NAME}" PRIVATE c89stringutils c89stringutils_compiler_flags)

add_test(NAME "${EXEC_NAME}" COMMAND "${EXEC_NAME}")
