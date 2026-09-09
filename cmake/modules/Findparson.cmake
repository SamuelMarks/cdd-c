include(FetchContent)

if(NOT parson_FOUND)
    set(_PARSON_LOCAL_DIR "")
    get_filename_component(_real_src_dir "${CMAKE_CURRENT_SOURCE_DIR}" REALPATH)
    foreach(_cand_rel
        "${CMAKE_CURRENT_SOURCE_DIR}/../parson"
        "${CMAKE_CURRENT_SOURCE_DIR}/../../parson"
        "${_real_src_dir}/../parson"
        "${_real_src_dir}/../../parson"
    )
        get_filename_component(_cand_abs "${_cand_rel}" REALPATH)
        if(EXISTS "${_cand_abs}" AND EXISTS "${_cand_abs}/CMakeLists.txt")
            set(_PARSON_LOCAL_DIR "${_cand_abs}")
            break()
        endif()
    endforeach()

    if(_PARSON_LOCAL_DIR)
        add_subdirectory("${_PARSON_LOCAL_DIR}" "${CMAKE_BINARY_DIR}/parson")
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
                    GIT_REPOSITORY "https://github.com/SamuelMarks/parson.git"
                GIT_TAG "master"
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
