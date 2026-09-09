set(CMAKE_C_STANDARD 90)
set(CMAKE_C_STANDARD_REQUIRED ON)

file(GLOB SOURCES "*.c")
file(GLOB HEADERS "*.h")

list(FILTER SOURCES EXCLUDE REGEX "test_.*\\.c$")
list(FILTER HEADERS EXCLUDE REGEX "test_.*\\.h$")

add_library(generated_client ${SOURCES} ${HEADERS})

include(GenerateExportHeader)
generate_export_header(generated_client EXPORT_FILE_NAME ${CMAKE_CURRENT_BINARY_DIR}/lib_export.h EXPORT_MACRO_NAME LIB_EXPORT)

if (BUILD_SHARED_LIBS)
    target_compile_definitions(generated_client PRIVATE LIB_EXPORTS)
endif()

include(FetchContent)

get_filename_component(_real_src_dir "${CMAKE_CURRENT_SOURCE_DIR}" REALPATH)

if (TARGET parson)
    message(STATUS "parson already provided by parent")
    if(TARGET parson AND NOT TARGET parson::parson)
        add_library(parson::parson ALIAS parson)
    endif()
else()
    set(_PARSON_LOCAL_DIR "")
    foreach(_cand_rel
        "${CMAKE_CURRENT_SOURCE_DIR}/../../../../parson"
        "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../parson"
        "${_real_src_dir}/../../../../parson"
        "${_real_src_dir}/../../../../../parson"
    )
        get_filename_component(_cand_abs "${_cand_rel}" REALPATH)
        if(EXISTS "${_cand_abs}" AND EXISTS "${_cand_abs}/CMakeLists.txt")
            set(_PARSON_LOCAL_DIR "${_cand_abs}")
            break()
        endif()
    endforeach()

    if(_PARSON_LOCAL_DIR)
        add_subdirectory("${_PARSON_LOCAL_DIR}" "${CMAKE_BINARY_DIR}/parson")
        if(TARGET parson AND NOT TARGET parson::parson)
            add_library(parson::parson ALIAS parson)
        endif()
    else()
        set(parson_RESOLVED OFF)
        if(VCPKG_TOOLCHAIN)
            find_package(parson CONFIG QUIET)
            if(parson_FOUND)
                set(parson_RESOLVED ON)
            endif()
        endif()
        if(NOT parson_RESOLVED)
            FetchContent_Declare(parson
                GIT_REPOSITORY "https://github.com/SamuelMarks/parson.git"
                GIT_TAG "master"
            )
            FetchContent_MakeAvailable(parson)
            if(TARGET parson AND NOT TARGET parson::parson)
                add_library(parson::parson ALIAS parson)
            endif()
        endif()
    endif()
endif()

if (TARGET c89stringutils)
    message(STATUS "c89stringutils already provided by parent")
else()
    set(_C89STRINGUTILS_LOCAL_DIR "")
    foreach(_cand_rel
        "${CMAKE_CURRENT_SOURCE_DIR}/../../../../c89stringutils"
        "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../c89stringutils"
        "${_real_src_dir}/../../../../c89stringutils"
        "${_real_src_dir}/../../../../../c89stringutils"
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
            find_package(c89stringutils CONFIG QUIET)
            if(c89stringutils_FOUND)
                set(c89stringutils_RESOLVED ON)
            endif()
        endif()
        if(NOT c89stringutils_RESOLVED)
            FetchContent_Declare(c89stringutils
                GIT_REPOSITORY "https://github.com/offscale/c89stringutils.git"
                GIT_TAG "master"
            )
            FetchContent_MakeAvailable(c89stringutils)
        endif()
    endif()
endif()

if (TARGET c_str_span)
    message(STATUS "c_str_span already provided by parent")
else()
    set(_C_STR_SPAN_LOCAL_DIR "")
    foreach(_cand_rel
        "${CMAKE_CURRENT_SOURCE_DIR}/../../../../c-str-span"
        "${CMAKE_CURRENT_SOURCE_DIR}/../../../../../c-str-span"
        "${_real_src_dir}/../../../../c-str-span"
        "${_real_src_dir}/../../../../../c-str-span"
    )
        get_filename_component(_cand_abs "${_cand_rel}" REALPATH)
        if(EXISTS "${_cand_abs}" AND EXISTS "${_cand_abs}/CMakeLists.txt")
            set(_C_STR_SPAN_LOCAL_DIR "${_cand_abs}")
            break()
        endif()
    endforeach()

    if(_C_STR_SPAN_LOCAL_DIR)
        add_subdirectory("${_C_STR_SPAN_LOCAL_DIR}" "${CMAKE_BINARY_DIR}/c_str_span")
    else()
        set(c_str_span_RESOLVED OFF)
        if(VCPKG_TOOLCHAIN)
            find_package(c_str_span CONFIG QUIET)
            if(c_str_span_FOUND)
                set(c_str_span_RESOLVED ON)
            endif()
        endif()
        if(NOT c_str_span_RESOLVED)
            FetchContent_Declare(c_str_span
                GIT_REPOSITORY "https://github.com/SamuelMarks/c-str-span.git"
                GIT_TAG "master"
            )
            set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
            FetchContent_MakeAvailable(c_str_span)
        endif()
    endif()
endif()

target_link_libraries(generated_client PRIVATE parson::parson)

target_link_libraries(generated_client PRIVATE c89stringutils c89stringutils_compiler_flags)

target_link_libraries(generated_client PRIVATE c_str_span c_str_span_compiler_flags c_cdd)


target_include_directories(generated_client PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    $<INSTALL_INTERFACE:include>
)
