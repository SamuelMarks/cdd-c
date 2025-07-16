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
    FetchContent_Declare(
            c89stringutils
            GIT_REPOSITORY https://github.com/offscale/c89stringutils.git
            GIT_TAG        b6d73d252b2e0286acd2976fa4eb7c7f13eee577
            GIT_SHALLOW    TRUE
    )

    FetchContent_MakeAvailable(c89stringutils)
else()
    message(STATUS "Found c89stringutils installed at: ${c89stringutils_DIR}")
endif()

if(TARGET c89stringutils AND NOT TARGET c89stringutils::c89stringutils)
    add_library(c89stringutils::c89stringutils ALIAS c89stringutils)
endif()
