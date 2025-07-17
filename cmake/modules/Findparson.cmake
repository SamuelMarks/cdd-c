include(FetchContent)

include(FindPackageHandleStandardArgs)

if (NOT TARGET parson::parson)
    FetchContent_Declare(
            parson
            GIT_REPOSITORY https://github.com/kgabis/parson.git
            GIT_TAG ba29f4eda9ea7703a9f6a9cf2b0532a2605723c3
    )

    FetchContent_MakeAvailable(parson)

    target_include_directories(parson INTERFACE
            $<BUILD_INTERFACE:${parson_SOURCE_DIR}>
    )
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

if (TARGET parson AND NOT TARGET parson::parson)
    add_library(parson::parson ALIAS parson)
endif (TARGET parson AND NOT TARGET parson::parson)
