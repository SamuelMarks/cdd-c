include(FetchContent)

include(FindPackageHandleStandardArgs)

if (NOT TARGET parson)
    if(VCPKG_TOOLCHAIN)
        find_package(parson CONFIG REQUIRED)
    else()
        FetchContent_Declare(
                parson
                GIT_REPOSITORY https://github.com/SamuelMarks/parson.git
                GIT_TAG master
        )

        FetchContent_MakeAvailable(parson)
    endif()

    if(TARGET parson AND DEFINED parson_SOURCE_DIR)
        target_include_directories(parson INTERFACE
                $<BUILD_INTERFACE:${parson_SOURCE_DIR}>
        )
    endif()
endif ()

set(parson_LIBRARIES parson)
get_target_property(parson_INCLUDE_DIRS parson INTERFACE_INCLUDE_DIRECTORIES)

find_package_handle_standard_args(parson
        FOUND_VAR parson_FOUND
        REQUIRED_VARS
        parson_LIBRARIES
        parson_INCLUDE_DIRS
)

mark_as_advanced(parson_SOURCE_DIR parson_BINARY_DIR)
