include(FetchContent)

if(NOT parson_FOUND)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../parson/CMakeLists.txt")
        add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../parson" "${CMAKE_BINARY_DIR}/parson")
    else()
        set(parson_RESOLVED OFF)
        if(VCPKG_TOOLCHAIN)
            find_package(parson CONFIG QUIET)
            if(parson_FOUND)
                set(parson_RESOLVED ON)
            endif()
        endif()
        if(NOT parson_RESOLVED)
            FetchContent_Declare(
                    parson
                    URL "https://github.com/SamuelMarks/parson/archive/refs/heads/master.tar.gz"
            )

            FetchContent_MakeAvailable(parson)
        endif()
    endif()
endif()

if(TARGET parson AND NOT TARGET parson::parson)
    add_library(parson::parson ALIAS parson)
elseif(TARGET parson::parson AND NOT TARGET parson)
    add_library(parson ALIAS parson::parson)
endif()

set(parson_FOUND TRUE)
