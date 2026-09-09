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
    set(_C89STRINGUTILS_LOCAL_DIR "")
    get_filename_component(_real_src_dir "${CMAKE_CURRENT_SOURCE_DIR}" REALPATH)
    foreach(_cand_rel
        "${CMAKE_CURRENT_SOURCE_DIR}/../c89stringutils"
        "${CMAKE_CURRENT_SOURCE_DIR}/../../c89stringutils"
        "${_real_src_dir}/../c89stringutils"
        "${_real_src_dir}/../../c89stringutils"
    )
        get_filename_component(_cand_abs "${_cand_rel}" REALPATH)
        if(EXISTS "${_cand_abs}" AND EXISTS "${_cand_abs}/CMakeLists.txt")
            set(_C89STRINGUTILS_LOCAL_DIR "${_cand_abs}")
            break()
        endif()
    endforeach()

    if(_C89STRINGUTILS_LOCAL_DIR)
        add_subdirectory("${_C89STRINGUTILS_LOCAL_DIR}" "${CMAKE_BINARY_DIR}/c89stringutils")
    else()
        set(c89stringutils_RESOLVED OFF)
        if(VCPKG_TOOLCHAIN)
            # find_package(c89stringutils CONFIG QUIET) # Skip vcpkg due to missing c89stringutils_vasprintf
            if(c89stringutils_FOUND)
                set(c89stringutils_RESOLVED ON)
            endif()
        endif()
        if(NOT c89stringutils_RESOLVED)
            FetchContent_Declare(
                    c89stringutils
                    GIT_REPOSITORY "https://github.com/offscale/c89stringutils.git"
                GIT_TAG "master"
            )

            FetchContent_MakeAvailable(c89stringutils)
            add_compile_definitions(C89STRINGUTILS_STATIC_DEFINE)
        endif()
    endif()
else()
    message(STATUS "Found c89stringutils installed at: ${c89stringutils_DIR}")
endif()

if(TARGET c89stringutils AND NOT TARGET c89stringutils::c89stringutils)
    add_library(c89stringutils::c89stringutils ALIAS c89stringutils)
endif()
