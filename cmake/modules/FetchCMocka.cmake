# This script handles finding or fetching the CMocka unit testing library.
#
# Usage:
# 1. Add this file to your project's `cmake` directory.
# 2. In your main CMakeLists.txt, add the following:
#
#    list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
#    include(FetchCMocka)
#
# This will define the following imported targets if CMocka is found or fetched:
# - cmocka::cmocka         (shared library)
# - cmocka::cmocka-static  (static library)

include(FetchContent)

find_package(cmocka QUIET)

if(NOT cmocka_FOUND)
    message(STATUS "CMocka not found via find_package. Fetching from git...")

    # --- Fetch CMocka from Git Repository ---
    FetchContent_Declare(
            cmocka
            GIT_REPOSITORY https://github.com/SamuelMarks/cmocka.git
            GIT_TAG        3b20eadfe4a2b96281d5f8c58790b22f323da9fb
            GIT_SHALLOW    TRUE
    )

    # --- Configure CMocka Build Options ---
    # Before making the content available, we can set CMake cache variables
    # to control the build configuration of the fetched project.
    #
    # WITH_STATIC_LIB=ON:  Ensures the static library is built.
    # WITH_EXAMPLES=OFF:   We don't need to build their examples.
    # UNIT_TESTING=OFF:    We don't need to build their own tests.
    #
    # We use set() with CACHE INTERNAL to avoid polluting the user's CMake cache view.
    set(WITH_STATIC_LIB ON CACHE BOOL "Build the static cmocka library" FORCE)
    set(WITH_EXAMPLES OFF CACHE BOOL "Build cmocka examples" FORCE)
    set(UNIT_TESTING OFF CACHE BOOL "Build the unit tests" FORCE)

    FetchContent_MakeAvailable(cmocka)
else()
    message(STATUS "Found CMocka installed at: ${cmocka_DIR}")
endif()

if(TARGET cmocka-shared AND NOT TARGET cmocka::cmocka)
    add_library(cmocka::cmocka ALIAS cmocka-shared)
endif()

if(TARGET cmocka-static AND NOT TARGET cmocka::cmocka-static)
    add_library(cmocka::cmocka-static ALIAS cmocka-static)
endif()
