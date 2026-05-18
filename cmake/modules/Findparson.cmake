include(FetchContent)

if(NOT parson_FOUND)
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
endif()

if(TARGET parson AND NOT TARGET parson::parson)
    add_library(parson::parson ALIAS parson)
elseif(TARGET parson::parson AND NOT TARGET parson)
    add_library(parson ALIAS parson::parson)
endif()

set(parson_FOUND TRUE)
