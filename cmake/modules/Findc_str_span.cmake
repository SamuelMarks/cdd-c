# This script handles finding or fetching the c-str-span header-only library.
#
# Usage:
# 1. Add this file to your project's `cmake` directory.
# 2. In your main CMakeLists.txt, add the following:
#
#    list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
#    include(Findc_str_span)
#
# This will define the following imported target if c-str-span is found or fetched:
# - c_str_span::c_str_span (INTERFACE library)

include(FetchContent)

if(NOT c_str_span_FOUND)
    FetchContent_Declare(
            c_str_span
            GIT_REPOSITORY https://github.com/SamuelMarks/c-str-span.git
            GIT_TAG        b73a50b801b70dce7ae260eb79a8a6097f9a0a15
            GIT_SHALLOW    TRUE
            #CONFIGURE_COMMAND BUILD_TESTING=OFF
            CMAKE_ARGS -DBUILD_TESTING=OFF
    )

    FetchContent_MakeAvailable(c_str_span)
else()
    message(STATUS "Found c-str-span installed at: ${c_str_span_DIR}")
endif()

if(TARGET c_str_span AND NOT TARGET c_str_span::c_str_span)
    add_library(c_str_span::c_str_span ALIAS c_str_span)
endif()
