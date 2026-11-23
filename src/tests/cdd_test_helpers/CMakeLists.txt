get_filename_component(LIBRARY_NAME "${CMAKE_CURRENT_SOURCE_DIR}" NAME)

set(Header_Files
    "cdd_crt_bridge.h"
    "cdd_helpers.h"
    "mock_server.h"
)
source_group("${LIBRARY_NAME} Header Files" FILES ${Header_Files})

set(Source_Files
    "cdd_helpers.c"
    "mock_server.c"
)
source_group("${LIBRARY_NAME} Source Files" FILES ${Source_Files})

add_library("${LIBRARY_NAME}" STATIC ${Header_Files} ${Source_Files})
c_cdd_apply_target_settings("${LIBRARY_NAME}")
target_compile_definitions("${LIBRARY_NAME}" PUBLIC CDD_TEST_HELPERS_STATIC_DEFINE)

include(GenerateExportHeader)
set(_export_file "${CMAKE_CURRENT_BINARY_DIR}/${LIBRARY_NAME}_export.h")
generate_export_header("${LIBRARY_NAME}" EXPORT_FILE_NAME "${_export_file}")

target_include_directories("${LIBRARY_NAME}" PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>"
    "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>"
    "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/src>"
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>"
)

target_link_libraries("${LIBRARY_NAME}" PUBLIC "${PROJECT_NAME}")

if(NOT WIN32 AND NOT EMSCRIPTEN AND NOT CMAKE_SYSTEM_NAME STREQUAL "WASI" AND NOT CMAKE_SYSTEM_NAME STREQUAL "DOS")
    find_package(Threads QUIET)
    if(Threads_FOUND AND TARGET Threads::Threads)
        target_link_libraries("${LIBRARY_NAME}" PUBLIC Threads::Threads)
    endif()
endif()

if(WIN32)
    target_link_libraries("${LIBRARY_NAME}" PUBLIC ws2_32)
endif()
