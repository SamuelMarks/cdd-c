set(LIBRARY_NAME "cdd_src_mocks")

set(Header_Files
    "emit/simple.h"
    "emit/simple_json.h"
    "c_cdd_stdbool.h"
)
source_group("Header Files" FILES ${Header_Files})

set(Source_Files
    "emit/simple.c"
    "emit/simple_json.c"
)
source_group("Source Files" FILES ${Source_Files})

add_library("${LIBRARY_NAME}" STATIC ${Header_Files} ${Source_Files})
c_cdd_apply_target_settings("${LIBRARY_NAME}")
target_compile_definitions("${LIBRARY_NAME}" PUBLIC CDD_SRC_MOCKS_STATIC_DEFINE)

if(_C_CDD_ENABLE_TESTS)
    target_compile_definitions("${LIBRARY_NAME}" PUBLIC CDD_BUILD_TESTS)
endif()

include(GenerateExportHeader)
set(_export_file "${CMAKE_CURRENT_BINARY_DIR}/${LIBRARY_NAME}_export.h")
generate_export_header("${LIBRARY_NAME}" EXPORT_FILE_NAME "${_export_file}")

target_link_libraries("${LIBRARY_NAME}" PUBLIC
    "${PROJECT_NAME}"
    c89stringutils
    "${PROJECT_NAME}_compiler_flags"
)

target_include_directories("${LIBRARY_NAME}"
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>"
    PRIVATE
        "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>"
        "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
        "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/tests>"
        "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/src>"
)

add_executable(src_mocks "parse/main.c")
c_cdd_apply_target_settings(src_mocks)
target_link_libraries(src_mocks PRIVATE "${LIBRARY_NAME}")
target_include_directories(src_mocks PRIVATE
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/tests>"
    "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/src>"
)
