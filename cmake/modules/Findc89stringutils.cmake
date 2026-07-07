# This script handles finding or fetching the c89stringutils library.
#
# Usage:
# 1. Add this file to your project's `cmake` directory.
# 2. In your main CMakeLists.txt, add the following:
#
#    list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
#    include(Findc89stringutils)
#
# This will define the following imported target if c89stringutils is found or fetched:
# - c89stringutils::c89stringutils (static library)

include(FetchContent)

if(NOT c89stringutils_FOUND)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../c89stringutils/CMakeLists.txt")
        add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../c89stringutils" "${CMAKE_BINARY_DIR}/c89stringutils")
    else()
        set(c89stringutils_RESOLVED OFF)
        if(VCPKG_TOOLCHAIN)
            find_package(c89stringutils CONFIG QUIET)
            if(c89stringutils_FOUND)
                set(c89stringutils_RESOLVED ON)
            endif()
        endif()
        if(NOT c89stringutils_RESOLVED)
            FetchContent_Declare(
                    c89stringutils
                    URL "https://github.com/offscale/c89stringutils/archive/refs/heads/master.tar.gz"
            )

            FetchContent_MakeAvailable(c89stringutils)
        endif()
    endif()
else()
    message(STATUS "Found c89stringutils installed at: ${c89stringutils_DIR}")
endif()

if(TARGET c89stringutils AND NOT TARGET c89stringutils::c89stringutils)
    add_library(c89stringutils::c89stringutils ALIAS c89stringutils)
endif()
