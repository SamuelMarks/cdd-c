set(CMAKE_C_STANDARD 90)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

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

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../../../parson/CMakeLists.txt")
    add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../../../../parson" "${CMAKE_BINARY_DIR}/parson")
else()
    set(parson_RESOLVED OFF)
    if(VCPKG_TOOLCHAIN)
        find_package(parson CONFIG QUIET)
        if(parson_FOUND)
            set(parson_RESOLVED ON)
        endif()
    endif()
    if(NOT parson_RESOLVED)
        FetchContent_Declare(parson GIT_REPOSITORY https://github.com/SamuelMarks/parson.git GIT_TAG master)
        FetchContent_MakeAvailable(parson)
    endif()
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../../../c89stringutils/CMakeLists.txt")
    add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../../../../c89stringutils" "${CMAKE_BINARY_DIR}/c89stringutils")
else()
    set(c89stringutils_RESOLVED OFF)
    if(VCPKG_TOOLCHAIN)
        find_package(c89stringutils CONFIG QUIET)
        if(c89stringutils_FOUND)
            set(c89stringutils_RESOLVED ON)
        endif()
    endif()
    if(NOT c89stringutils_RESOLVED)
        FetchContent_Declare(c89stringutils GIT_REPOSITORY https://github.com/offscale/c89stringutils.git GIT_TAG master)
        FetchContent_MakeAvailable(c89stringutils)
    endif()
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../../../c-str-span/CMakeLists.txt")
    add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../../../../c-str-span" "${CMAKE_BINARY_DIR}/c_str_span")
else()
    set(c_str_span_RESOLVED OFF)
    if(VCPKG_TOOLCHAIN)
        find_package(c_str_span CONFIG QUIET)
        if(c_str_span_FOUND)
            set(c_str_span_RESOLVED ON)
        endif()
    endif()
    if(NOT c_str_span_RESOLVED)
        FetchContent_Declare(c_str_span GIT_REPOSITORY https://github.com/SamuelMarks/c-str-span.git GIT_TAG master)
        set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(c_str_span)
    endif()
endif()

target_link_libraries(generated_client PRIVATE parson)

target_link_libraries(generated_client PRIVATE c89stringutils c89stringutils_compiler_flags)

target_link_libraries(generated_client PRIVATE c_str_span c_str_span_compiler_flags)

option(C_ABSTRACT_HTTP_USE_CURL "Use libcurl as the HTTP backend" OFF)
option(C_ABSTRACT_HTTP_USE_OPENSSL "Use OpenSSL for cryptography" OFF)

include(FetchContent)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../../../../c-abstract-http/CMakeLists.txt")
    add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../../../../c-abstract-http" "${CMAKE_BINARY_DIR}/c-abstract-http")
    include_directories(SYSTEM "${CMAKE_CURRENT_SOURCE_DIR}/../../../../c-abstract-http/include")
else()
    set(c_abstract_http_RESOLVED OFF)
    if(VCPKG_TOOLCHAIN)
        find_package(c-abstract-http CONFIG QUIET)
        if(c-abstract-http_FOUND)
            set(c_abstract_http_RESOLVED ON)
        endif()
    endif()
    if(NOT c_abstract_http_RESOLVED)
        FetchContent_Declare(
            c-abstract-http
            GIT_REPOSITORY https://github.com/SamuelMarks/c-abstract-http.git
            GIT_TAG        master
        )
        set(cdd_c_FOUND ON)
        FetchContent_MakeAvailable(c-abstract-http)
        include_directories(SYSTEM ${c-abstract-http_SOURCE_DIR}/include)
    endif()
endif()

target_link_libraries(generated_client PRIVATE c-abstract-http)

target_include_directories(generated_client PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    $<INSTALL_INTERFACE:include>
)
