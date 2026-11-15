set(LIBRARY_NAME "cdd_src_mocks")

set(Header_Files "emit/simple.h" "emit/simple_json.h" "c_cdd_stdbool.h")
source_group("Header Files" FILES "${Header_Files}")

set(Source_Files "emit/simple.c" "emit/simple_json.c")
source_group("Source Files" FILES "${Source_Files}")

add_library("${LIBRARY_NAME}" STATIC "${Header_Files}" "${Source_Files}")
target_compile_definitions("${LIBRARY_NAME}" PUBLIC CDD_SRC_MOCKS_STATIC_DEFINE)

include(GenerateExportHeader)
set(_export_file "${CMAKE_CURRENT_BINARY_DIR}/${LIBRARY_NAME}_export.h")
generate_export_header("${LIBRARY_NAME}" EXPORT_FILE_NAME "${_export_file}")

if(NOT TARGET c89stringutils)
    if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.11" AND NOT VCPKG_TOOLCHAIN)
        find_package(c89stringutils REQUIRED)
    else ()
        find_package(c89stringutils CONFIG REQUIRED)
    endif ()
endif()
target_link_libraries("${LIBRARY_NAME}" PUBLIC "${PROJECT_NAME}" "c89stringutils" "c89stringutils_compiler_flags" "${PROJECT_NAME}_compiler_flags")

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
        "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>"
        "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/tests>"
        "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/src>"
)

if (BUILD_TESTING OR C_CDD_BUILD_TESTS)
    target_compile_definitions("${LIBRARY_NAME}" PUBLIC CDD_BUILD_TESTS)
endif()

set_target_properties(
        "${LIBRARY_NAME}"
        PROPERTIES
        LINKER_LANGUAGE
        C
)

add_executable(src_mocks "parse/main.c")
target_link_libraries(src_mocks "${LIBRARY_NAME}")
target_include_directories(
        src_mocks
        PRIVATE
        "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>"
        "$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/tests>"
        "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/src>"
)
