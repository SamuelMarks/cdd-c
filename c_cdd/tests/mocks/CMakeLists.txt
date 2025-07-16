set(LIBRARY_NAME "simple_mocks")

set(Header_Files "simple.h" "simple_json.h" "c_cdd_stdbool.h")
source_group("Header Files" FILES "${Header_Files}")

set(Source_Files "simple.c" "simple_json.c")
source_group("Source Files" FILES "${Source_Files}")

add_library("${LIBRARY_NAME}" SHARED "${Header_Files}" "${Source_Files}")

include(GenerateExportHeader)
set(_export_file "${CMAKE_CURRENT_BINARY_DIR}/${LIBRARY_NAME}_export.h")
generate_export_header("${LIBRARY_NAME}" EXPORT_FILE_NAME "${_export_file}")

if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.11" AND NOT DEFINED VCPKG_TOOLCHAIN)
    find_package(c89stringutils REQUIRED)
else ()
    find_package(c89stringutils CONFIG REQUIRED)
endif ()
target_link_libraries("${LIBRARY_NAME}" PUBLIC "c89stringutils" "c89stringutils_compiler_flags")

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
target_include_directories(
        "${LIBRARY_NAME}"
        PRIVATE
        ${parson_INCLUDE_DIRS}
)

target_include_directories(
        "${LIBRARY_NAME}"
        PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>"
)

target_include_directories(
        "${LIBRARY_NAME}"
        PRIVATE
        "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>"
)
set_target_properties(
        "${LIBRARY_NAME}"
        PROPERTIES
        LINKER_LANGUAGE
        C
)

add_executable(mocks "main.c")
target_link_libraries(mocks "${LIBRARY_NAME}")
target_include_directories(
        "${LIBRARY_NAME}"
        PRIVATE
        ${parson_INCLUDE_DIRS}
)
