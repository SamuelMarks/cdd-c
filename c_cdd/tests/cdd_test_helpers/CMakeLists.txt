get_filename_component(LIBRARY_NAME "${CMAKE_CURRENT_SOURCE_DIR}" NAME)

set(Header_Files "cdd_helpers.h")
source_group("${LIBRARY_NAME} Header Files" FILES "${Header_Files}")

set(Source_Files "cdd_helpers.c")
source_group("${LIBRARY_NAME} Source Files" FILES "${Source_Files}")

add_library("${LIBRARY_NAME}" "${Header_Files}" "${Source_Files}")

include(GenerateExportHeader)
set(_export_file "${CMAKE_CURRENT_BINARY_DIR}/${LIBRARY_NAME}_export.h")
generate_export_header("${LIBRARY_NAME}" EXPORT_FILE_NAME "${_export_file}")

target_include_directories(
        "${LIBRARY_NAME}"
        PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>"
)
set_target_properties(
        "${LIBRARY_NAME}"
        PROPERTIES
        LINKER_LANGUAGE
        C
)
target_link_libraries("${LIBRARY_NAME}" PUBLIC "${PROJECT_NAME}")
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
