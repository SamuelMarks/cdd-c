set(LIBRARY_NAME "simple_mocks")

set(Header_Files "simple.h")
source_group("Header Files" FILES "${Header_Files}")

set(Source_Files "simple.c")
source_group("Source Files" FILES "${Source_Files}")

add_library("${LIBRARY_NAME}" SHARED "${Header_Files}" "${Source_Files}")
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
