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
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../c-str-span/CMakeLists.txt")
        add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../c-str-span" "${CMAKE_BINARY_DIR}/c_str_span")
    else()
        set(c_str_span_RESOLVED OFF)
        if(VCPKG_TOOLCHAIN)
            find_package(c_str_span CONFIG QUIET)
            if(c_str_span_FOUND)
                set(c_str_span_RESOLVED ON)
            endif()
        endif()
        if(NOT c_str_span_RESOLVED)
            FetchContent_Declare(
                    c_str_span
                    URL "https://github.com/SamuelMarks/c-str-span/archive/refs/heads/master.tar.gz"
            )
            set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
            FetchContent_MakeAvailable(c_str_span)
        endif()
    endif()
else()
    message(STATUS "Found c-str-span installed at: ${c_str_span_DIR}")
endif()

if(TARGET c_str_span AND NOT TARGET c_str_span::c_str_span)
    add_library(c_str_span::c_str_span ALIAS c_str_span)
endif()
