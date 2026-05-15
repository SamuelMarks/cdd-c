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
if(VCPKG_TOOLCHAIN)
    find_package(parson REQUIRED)
    find_package(c89stringutils CONFIG REQUIRED)
    find_package(c_str_span CONFIG REQUIRED)
else()
    FetchContent_Declare(parson GIT_REPOSITORY https://github.com/SamuelMarks/parson.git GIT_TAG master)
    FetchContent_MakeAvailable(parson)
    FetchContent_Declare(c89stringutils GIT_REPOSITORY https://github.com/SamuelMarks/c89stringutils.git GIT_TAG master)
    FetchContent_MakeAvailable(c89stringutils)
    FetchContent_Declare(c_str_span GIT_REPOSITORY https://github.com/SamuelMarks/c_str_span.git GIT_TAG master)
    FetchContent_MakeAvailable(c_str_span)
endif()

target_link_libraries(generated_client PRIVATE parson)

target_link_libraries(generated_client PRIVATE c89stringutils c89stringutils_compiler_flags)

target_link_libraries(generated_client PRIVATE c_str_span c_str_span_compiler_flags)

option(C_ABSTRACT_HTTP_USE_CURL "Use libcurl as the HTTP backend" OFF)
option(C_ABSTRACT_HTTP_USE_OPENSSL "Use OpenSSL for cryptography" OFF)

include(FetchContent)
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../c-abstract-http/CMakeLists.txt")
    add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../c-abstract-http" "${CMAKE_BINARY_DIR}/c-abstract-http")
    include_directories(SYSTEM "${CMAKE_CURRENT_SOURCE_DIR}/../c-abstract-http/include")
elseif(VCPKG_TOOLCHAIN)
    find_package(c-abstract-http CONFIG REQUIRED)
else()
    FetchContent_Declare(
        c-abstract-http
        GIT_REPOSITORY https://github.com/SamuelMarks/c-abstract-http.git
        GIT_TAG        master
    )
    FetchContent_MakeAvailable(c-abstract-http)
    include_directories(SYSTEM ${c-abstract-http_SOURCE_DIR}/include)
endif()

target_link_libraries(generated_client PRIVATE c-abstract-http)

target_include_directories(generated_client PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    $<INSTALL_INTERFACE:include>
)

