message(STATUS "RUNNING cdd-c CMakeLists.txt")

cmake_minimum_required(VERSION 3.11)
project(c_cdd VERSION 0.0.1 LANGUAGES C)

if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    option(C_CDD_BUILD_TESTS "Build tests" ON)
else()
    option(C_CDD_BUILD_TESTS "Build tests" OFF)
endif()

if(CMAKE_C_COMPILER_ID MATCHES "Watcom")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -zm -zc -mh")
endif()

if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.11")
    include(FetchContent)
    if(VCPKG_TOOLCHAIN)
        find_package(cfs CONFIG QUIET)
        if(NOT cfs_FOUND)
            FetchContent_Declare(
                cfs
                URL https://github.com/SamuelMarks/c-fs/archive/refs/heads/master.tar.gz
            )
            FetchContent_MakeAvailable(cfs)
            include_directories(SYSTEM ${cfs_SOURCE_DIR}/include)
        endif()
    else()
        FetchContent_Declare(
            cfs
            URL https://github.com/SamuelMarks/c-fs/archive/refs/heads/master.tar.gz
        )
        FetchContent_MakeAvailable(cfs)
        include_directories(SYSTEM ${cfs_SOURCE_DIR}/include)
    endif()
endif()
if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.11")
    include(FetchContent)
endif()
if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.11")
    include(FetchContent)
endif()

if(CMAKE_C_COMPILER_ID MATCHES "Watcom")
    function(target_compile_features)
    endfunction()
endif()

if(MSVC AND MSVC_VERSION LESS 1600)
    include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include/msvc")
    link_directories("C:/Program Files/Microsoft SDKs/Windows/v6.0A/Lib")
endif()

add_compile_definitions(_CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_WARNINGS)

if (MSVC AND MSVC_VERSION LESS 1800)
    add_compile_definitions(HAS_STDBOOL _STDBOOL_H "bool=unsigned char" "true=1" "false=0")
endif()
 

option(BUILD_SHARED_LIBS "Build shared library" OFF)
option(C_CDD_USE_UNICODE "Build with UNICODE support" ON)
option(C_CDD_USE_ANSI "Build with ANSI string support (disables UNICODE)" OFF)
option(C_CDD_MULTI_THREADED "Multi-threaded support" ON)
option(C_CDD_USE_LTO "Enable Link-Time Optimization" OFF)

if(MSVC)
    option(C_CDD_STATIC_CRT "Use static CRT (/MT, /MTd) instead of shared (/MD, /MDd)" OFF)
    set(C_CDD_RUNTIME_CHECKS "" CACHE STRING "Runtime checks (/RTC1, /RTCs, /RTCu or empty)")
    set_property(CACHE C_CDD_RUNTIME_CHECKS PROPERTY STRINGS "" "/RTC1" "/RTCs" "/RTCu")
endif()

if(C_CDD_USE_LTO)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT ipo_supported OUTPUT error)
    if(ipo_supported)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
    endif()
endif()

if(MSVC)
    # Enforce the same C runtime library for all targets to avoid CRT mismatches.
    if(C_CDD_STATIC_CRT)
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>" CACHE STRING "MSVC C runtime library" FORCE)
    else()
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL" CACHE STRING "MSVC C runtime library" FORCE)
    endif()

    if(CMAKE_VERSION VERSION_LESS "3.15")
        foreach(flag_var
                CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
                CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO
                CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
                CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
            if(C_CDD_STATIC_CRT)
                if(${flag_var} MATCHES "/MD")
                    string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
                endif()
            else()
                if(${flag_var} MATCHES "/MT")
                    string(REGEX REPLACE "/MT" "/MD" ${flag_var} "${${flag_var}}")
                endif()
            endif()
        endforeach()
    endif()
endif()

set(CMAKE_C_STANDARD 90)
if (NOT (CMAKE_C_COMPILER_ID MATCHES "Watcom"))
    set(CMAKE_C_STANDARD_REQUIRED ON)
endif()

add_library("${PROJECT_NAME}_compiler_flags" INTERFACE)

if(C_CDD_USE_UNICODE)
    target_compile_definitions("${PROJECT_NAME}_compiler_flags" INTERFACE UNICODE _UNICODE)
endif()

if(C_CDD_MULTI_THREADED)
    if(NOT CMAKE_C_COMPILER_ID MATCHES "Watcom")
        target_compile_options("${PROJECT_NAME}_compiler_flags" INTERFACE "$<$<NOT:$<COMPILE_LANG_AND_ID:C,CXX,MSVC>>:-pthread>")
    endif()
endif()

if(MSVC)
    if(NOT C_CDD_RUNTIME_CHECKS STREQUAL "")
        target_compile_options("${PROJECT_NAME}_compiler_flags" INTERFACE "${C_CDD_RUNTIME_CHECKS}")
    endif()
endif()

if(C_CDD_USE_ASAN)
    target_compile_options("${PROJECT_NAME}_compiler_flags" INTERFACE "$<$<COMPILE_LANG_AND_ID:C,CXX,ARMClang,AppleClang,Clang,GNU,LCC>:-fsanitize=address>")
    target_link_options("${PROJECT_NAME}_compiler_flags" INTERFACE "$<$<COMPILE_LANG_AND_ID:C,CXX,ARMClang,AppleClang,Clang,GNU,LCC>:-fsanitize=address>")
endif()
if(C_CDD_USE_UBSAN)
    target_compile_options("${PROJECT_NAME}_compiler_flags" INTERFACE "$<$<COMPILE_LANG_AND_ID:C,CXX,ARMClang,AppleClang,Clang,GNU,LCC>:-fsanitize=undefined>")
    target_link_options("${PROJECT_NAME}_compiler_flags" INTERFACE "$<$<COMPILE_LANG_AND_ID:C,CXX,ARMClang,AppleClang,Clang,GNU,LCC>:-fsanitize=undefined>")
endif()


if (NOT DEFINED MSVC_VERSION
        OR MSVC_VERSION STRGREATER "1900" # 2015
        OR NOT (CMAKE_C_COMPILER_ID STREQUAL "OpenWatcom"))
    if (NOT (CMAKE_C_COMPILER_ID MATCHES "Watcom"))
    target_compile_features("${PROJECT_NAME}_compiler_flags" INTERFACE "c_std_${CMAKE_C_STANDARD}")
endif()
elseif ((MSVC_VERSION STRLESS_EQUAL "1900"
        OR CMAKE_C_COMPILER_ID STREQUAL "OpenWatcom"
        OR CYGWIN)
        AND NOT EXISTS "${CMAKE_BINARY_DIR}/c89stringutils_export.h")
    file(COPY "${CMAKE_SOURCE_DIR}/c89stringutils/c89stringutils_export_pregen.h"
            DESTINATION "${CMAKE_BINARY_DIR}")
    file(RENAME "${CMAKE_BINARY_DIR}/c89stringutils_export_pregen.h"
            "${CMAKE_BINARY_DIR}/c89stringutils_export.h")
endif ()
if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.15")
    if (NOT (CMAKE_C_COMPILER_ID MATCHES "Watcom"))
    target_compile_features("${PROJECT_NAME}_compiler_flags" INTERFACE "c_std_${CMAKE_C_STANDARD}")
endif()

    set(gcc_like "$<COMPILE_LANG_AND_ID:C,CXX,ARMClang,AppleClang,Clang,GNU,LCC>")
    set(msvc "$<COMPILE_LANG_AND_ID:C,CXX,MSVC>")
    target_compile_options(
            "${PROJECT_NAME}_compiler_flags"
            INTERFACE
            "$<$<AND:${gcc_like},$<CONFIG:Debug>>:$<BUILD_INTERFACE:-Wshadow;-Wformat=2;-Wall;-Wextra;-Wcast-align;-Wpointer-arith;-Wmissing-prototypes;-Wstrict-prototypes;-Wmissing-declarations;-Wno-missing-braces;-pedantic;-Wno-long-long;-Wno-overlength-strings;-Wno-unused-value;-Wno-unused-variable;-fprofile-arcs;-ftest-coverage>>"
            "$<${msvc}:$<BUILD_INTERFACE:-W4;-Zi;-wd4996;-wd4127;-wd4244;-wd4028;-wd4201;-wd4214;-wd4115>>"
    )
    target_link_options(
            "${PROJECT_NAME}_compiler_flags"
            INTERFACE
            "$<$<AND:${gcc_like},$<CONFIG:Debug>>:$<BUILD_INTERFACE:--coverage>>"
    )
endif (CMAKE_VERSION VERSION_GREATER_EQUAL "3.15")
# Set the build directories
if (CMAKE_SYSTEM_NAME STREQUAL "Windows"
        OR CMAKE_SYSTEM_NAME STREQUAL "CYGWIN"
        OR CMAKE_SYSTEM_NAME MATCHES "MINGW.*")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
else ()
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
endif ()

if (CMAKE_GENERATOR MATCHES "Visual Studio 8 2005.*")
    set(COMPILE_LANG_AND_ID "MSVC")
else ()
    set(CMAKE_C_VISIBILITY_PRESET hidden)
    set(CMAKE_CXX_VISIBILITY_PRESET hidden)
    set(CMAKE_VISIBILITY_INLINES_HIDDEN YES)

    if (NOT (CMAKE_C_COMPILER_ID MATCHES "Watcom"))
    target_compile_features("${PROJECT_NAME}_compiler_flags" INTERFACE "c_std_${CMAKE_C_STANDARD}")
endif()

    # add compiler warning flags just when building this project via
    # the BUILD_INTERFACE genex
    set(gcc_like "$<COMPILE_LANG_AND_ID:C,CXX,ARMClang,AppleClang,Clang,GNU,LCC>")
    set(msvc "$<COMPILE_LANG_AND_ID:C,CXX,MSVC>")
    target_compile_options(
            "${PROJECT_NAME}_compiler_flags"
            INTERFACE
            "$<${gcc_like}:$<BUILD_INTERFACE:-Wshadow;-Wformat=2;-Wall;-Wextra;-Wcast-align;-Wpointer-arith;-Wmissing-prototypes;-Wstrict-prototypes;-Wmissing-declarations;-pedantic;-Wno-long-long;-Wno-overlength-strings;-Wno-unused-value;-Wno-unused-variable;>>"
            "$<${msvc}:$<BUILD_INTERFACE:-W4;-Zi;-wd4996;-wd4127;-wd4244;-wd4028;-wd4201;-wd4214;-wd4115>>"
    )
endif ()

if (MSVC OR __COMPILER_MSVC OR CMAKE_C_COMPILER_ID STREQUAL "OpenWatcom")
else ()
    include(CheckCSourceCompiles)
    check_c_source_compiles("
    #if defined(__linux__) || defined(__linux)
    # include <features.h>
    #endif
    #if defined(__GLIBC__) && (__GLIBC__ > 2 || (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 20))
    #error glibc >= 2.20
    #endif
    int main(void) { return 0; }
    " USE_BSD_SOURCE_INSTEAD_OF_DEFAULT)

    if (USE_BSD_SOURCE_INSTEAD_OF_DEFAULT)
        add_compile_definitions(_GNU_SOURCE _BSD_SOURCE)
    else ()
        add_compile_definitions(_GNU_SOURCE _DEFAULT_SOURCE)
    endif ()
    include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/EnsureSymbol.cmake")
    ensure_symbol(
            TARGET "${PROJECT_NAME}_compiler_flags"
            SYMBOL "asprintf"
            INCLUDES "stdio.h"
    )
endif (MSVC OR __COMPILER_MSVC OR CMAKE_C_COMPILER_ID STREQUAL "OpenWatcom")
if (DEFINED HAVE_ASPRINTF)
    set(CMAKE_C_EXTENSIONS ON)
    add_compile_definitions(HAVE_ASPRINTF)
endif (DEFINED HAVE_ASPRINTF)

if (NOT DEFINED TARGET_ARCH)
    include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/get_arch.cmake")
    get_arch()
endif (NOT DEFINED TARGET_ARCH)

if (WIN32 OR EMSCRIPTEN OR APPLE OR CMAKE_SYSTEM_NAME STREQUAL "DOS")
    option(C_CDD_USE_LIBCURL "Enable libcurl (defaults to `ON` on non-Windows)" OFF)
else ()
    option(C_CDD_USE_LIBCURL "Enable libcurl (defaults to `ON` on non-Windows)" ON)
endif ()

if (EMSCRIPTEN OR CMAKE_SYSTEM_NAME STREQUAL "DOS")
    option(HTTP_ONLY "Only build HTTP client/server (no OpenSSL)" ON)
endif ()

if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.11" AND NOT VCPKG_TOOLCHAIN)
    include(FetchContent)
    set(BUILD_SHARED_LIBS OFF)

    # c-str-span

    if(FALSE AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../c-str-span/CMakeLists.txt")
        add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../c-str-span" "${CMAKE_BINARY_DIR}/c_str_span")
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../c-str-span/c_str_span/c_str_span.h")
            file(READ "${CMAKE_CURRENT_SOURCE_DIR}/../c-str-span/c_str_span/c_str_span.h" span_content)
            string(REPLACE "// size must be >= 0" "/* size must be >= 0 */" span_content "${span_content}")
            string(REPLACE "/* Size = 1024 */" "Size = 1024" span_content "${span_content}")
            string(REPLACE "// Size = 1024" "Size = 1024" span_content "${span_content}")
            file(WRITE "${CMAKE_CURRENT_SOURCE_DIR}/../c-str-span/c_str_span/c_str_span.h" "${span_content}")
        endif()
        include_directories(SYSTEM "${CMAKE_CURRENT_SOURCE_DIR}/../c-str-span")
    elseif(VCPKG_TOOLCHAIN)
        find_package(c_str_span CONFIG REQUIRED)
    else()
        FetchContent_Declare(
            c_str_span
            GIT_REPOSITORY https://github.com/SamuelMarks/c-str-span.git
            GIT_TAG master
        )
        FetchContent_MakeAvailable(c_str_span)
        if(EXISTS "${c_str_span_SOURCE_DIR}/c_str_span/c_str_span.h")
            file(READ "${c_str_span_SOURCE_DIR}/c_str_span/c_str_span.h" span_content)
            string(REPLACE "// size must be >= 0" "/* size must be >= 0 */" span_content "${span_content}")
            string(REPLACE "/* Size = 1024 */" "Size = 1024" span_content "${span_content}")
            string(REPLACE "// Size = 1024" "Size = 1024" span_content "${span_content}")
            file(WRITE "${c_str_span_SOURCE_DIR}/c_str_span/c_str_span.h" "${span_content}")
        endif()
        include_directories(SYSTEM ${c_str_span_SOURCE_DIR})
    endif()

    # c89stringutils
    if(FALSE AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../c89stringutils/CMakeLists.txt")
        add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../c89stringutils" "${CMAKE_BINARY_DIR}/c89stringutils")
        include_directories(SYSTEM "${CMAKE_CURRENT_SOURCE_DIR}/../c89stringutils/c89stringutils")
    elseif(VCPKG_TOOLCHAIN)
        find_package(c89stringutils CONFIG REQUIRED)
    else()
        FetchContent_Declare(
            c89stringutils
            GIT_REPOSITORY https://github.com/offscale/c89stringutils.git
            GIT_TAG master
        )
        FetchContent_MakeAvailable(c89stringutils)
        include_directories(SYSTEM ${c89stringutils_SOURCE_DIR}/c89stringutils)
    endif()

    # curl
    if (C_CDD_USE_LIBCURL)
        FetchContent_Declare(
            curl
            GIT_REPOSITORY https://github.com/curl/curl.git
            GIT_TAG curl-8_4_0
        )
        set(BUILD_CURL_EXE OFF CACHE BOOL "" FORCE)
                set(CURL_DISABLE_TESTS ON CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(curl)
    endif ()

    # parson
    if(FALSE AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../parson/CMakeLists.txt")
        add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/../parson" "${CMAKE_BINARY_DIR}/parson")
        include_directories(SYSTEM "${CMAKE_CURRENT_SOURCE_DIR}/../parson")
        if(TARGET parson AND NOT TARGET parson::parson)
            add_library(parson::parson ALIAS parson)
        endif()
    elseif(VCPKG_TOOLCHAIN)
        find_package(parson CONFIG REQUIRED)
    else()
        FetchContent_Declare(
            parson
            GIT_REPOSITORY https://github.com/SamuelMarks/parson.git
            GIT_TAG master
        )
        set(OLD_BSL ${BUILD_SHARED_LIBS})
        set(BUILD_SHARED_LIBS OFF)
        FetchContent_MakeAvailable(parson)
        set(BUILD_SHARED_LIBS ${OLD_BSL})
        include_directories(SYSTEM ${parson_SOURCE_DIR})
        if(TARGET parson AND NOT TARGET parson::parson)
            add_library(parson::parson ALIAS parson)
        endif()
    endif()
    if (C_CDD_USE_LIBCURL)
        if(TARGET curl AND NOT TARGET CURL::libcurl)
            add_library(CURL::libcurl ALIAS curl)
        elseif(TARGET libcurl AND NOT TARGET CURL::libcurl)
            add_library(CURL::libcurl ALIAS libcurl)
        endif()
    endif ()
    
    endif (CMAKE_VERSION VERSION_GREATER_EQUAL "3.11" AND NOT VCPKG_TOOLCHAIN)

    if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.11")
        # We still keep the module path just in case
        list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")


    endif()


# configure a header file to pass the version number only
configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/config.h.in"
        "${PROJECT_NAME}Config.h"
)

#===============================================================================
# 4. ADD SUB-TARGETS
# Doing this at the end so that all definitions and link/include paths are
# available for the sub-projects.
#===============================================================================
message(STATUS "RUNNING add_subdirectory(src)")
add_subdirectory("src")




find_program(MEMORYCHECK_COMMAND NAMES valgrind)
set(MEMORYCHECK_COMMAND_OPTIONS "--trace-children=yes --leak-check=full --error-exitcode=1")
option(C_CDD_USE_ASAN "Enable AddressSanitizer" OFF)
option(C_CDD_USE_UBSAN "Enable UndefinedBehaviorSanitizer" OFF)
include(CTest)
if (C_CDD_BUILD_TESTS AND NOT CMAKE_SYSTEM_NAME STREQUAL "DOS")
    enable_testing()
    add_subdirectory("src/tests")
endif()

###########
# Install #
###########

include(GNUInstallDirs)

install(
        FILES "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.h"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)
include(InstallRequiredSystemLibraries)
set(CPACK_BUNDLE_NAME "${PROJECT_NAME}")
set(CPACK_PACKAGE_VENDOR "SamuelMarks")
set(CPACK_PACKAGE_DESCRIPTION "Frontend for C, concentrating on: generation from code; single-file analysis; modification; and emission (to C code).")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${CPACK_PACKAGE_DESCRIPTION_SUMMARY}")
if (APPLE)
    set(CPACK_BUNDLE_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Info.plist")
    set(CPACK_BUNDLE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Info.plist")
    set(CPACK_PACKAGE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/cmake/CustomVolumeIcon.icns")
endif (APPLE)
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/cmake/license.txt")
set(CPACK_PACKAGE_VERSION_MAJOR "${${PROJECT_NAME}_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${${PROJECT_NAME}_VERSION_MINOR}")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/cmake/README.txt")
set(CPACK_RESOURCE_FILE_WELCOME "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Welcome.txt")
set(CPACK_PACKAGE_CONTACT "https://github.com/SamuelMarks/c_cdd")

include(CPack)
include(CMakePackageConfigHelpers)

# generate the config file that is includes the exports
configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Config.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
        INSTALL_DESTINATION "${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}"
        NO_SET_AND_CHECK_MACRO
        NO_CHECK_REQUIRED_COMPONENTS_MACRO
)

# generate the version file for the config file
write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
        VERSION "${${PROJECT_NAME}_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}"
        COMPATIBILITY AnyNewerVersion
)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
        DESTINATION "${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}")

find_package(Doxygen)
if (DOXYGEN_FOUND)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
    add_custom_target(doc
        ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen" VERBATIM
    )
endif()
message(STATUS "TARGET_ARCH is ${TARGET_ARCH}")









