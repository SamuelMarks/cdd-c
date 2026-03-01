cmake_minimum_required(VERSION 3.11)

project(c_cdd VERSION 0.0.1 LANGUAGES C)
enable_testing()
add_compile_definitions(_CRT_SECURE_NO_WARNINGS _CRT_NONSTDC_NO_WARNINGS)
 
if(MSVC)
    # Enforce the same C runtime library for all targets to avoid CRT mismatches.
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL" CACHE STRING "MSVC C runtime library" FORCE)
endif()

set(CMAKE_C_STANDARD 90)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_library("${PROJECT_NAME}_compiler_flags" INTERFACE)

if (NOT DEFINED MSVC_VERSION
        OR MSVC_VERSION STRGREATER "1900" # 2015
        OR NOT (CMAKE_C_COMPILER_ID STREQUAL "OpenWatcom"))
    target_compile_features("${PROJECT_NAME}_compiler_flags" INTERFACE "c_std_${CMAKE_C_STANDARD}")
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
    target_compile_features("${PROJECT_NAME}_compiler_flags" INTERFACE "c_std_${CMAKE_C_STANDARD}")

    set(gcc_like "$<COMPILE_LANG_AND_ID:C,CXX,ARMClang,AppleClang,Clang,GNU,LCC>")
    set(msvc "$<COMPILE_LANG_AND_ID:C,CXX,MSVC>")
    target_compile_options(
            "${PROJECT_NAME}_compiler_flags"
            INTERFACE
            "$<$<AND:${gcc_like},$<CONFIG:Debug>>:$<BUILD_INTERFACE:-Wshadow;-Wformat=2;-Wall;-Wno-missing-braces;-Wno-long-long;-pedantic;-Wdeclaration-after-statement;-Werror=declaration-after-statement;-fprofile-arcs;-ftest-coverage>>"
            "$<${msvc}:$<BUILD_INTERFACE:-W3;-WX;-Zi;-permissive->>"
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

    target_compile_features("${PROJECT_NAME}_compiler_flags" INTERFACE "c_std_${CMAKE_C_STANDARD}")

    # add compiler warning flags just when building this project via
    # the BUILD_INTERFACE genex
    set(gcc_like "$<COMPILE_LANG_AND_ID:C,CXX,ARMClang,AppleClang,Clang,GNU,LCC>")
    set(msvc "$<COMPILE_LANG_AND_ID:C,CXX,MSVC>")
    target_compile_options(
            "${PROJECT_NAME}_compiler_flags"
            INTERFACE
            "$<${gcc_like}:$<BUILD_INTERFACE:-Wshadow;-Wformat=2;-Wall;-pedantic;-Wdeclaration-after-statement;-Werror=declaration-after-statement>>"
            "$<${msvc}:$<BUILD_INTERFACE:-W3;-WX;-Zi;-permissive->>"
    )
endif ()

if (MSVC OR __COMPILER_MSVC OR CMAKE_C_COMPILER_ID STREQUAL "OpenWatcom")
else ()
    include("${CMAKE_SOURCE_DIR}/cmake/modules/EnsureSymbol.cmake")
    ensure_symbol(
            TARGET "${PROJECT_NAME}_compiler_flags"
            SYMBOL "asprintf"
            INCLUDES "stdio.h"
            DEFINITIONS "_GNU_SOURCE" "_BSD_SOURCE"
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

if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.11" AND NOT VCPKG_TOOLCHAIN)
    include(FetchContent)
    set(BUILD_SHARED_LIBS OFF)

    # c-str-span
    FetchContent_Declare(
        c_str_span
        GIT_REPOSITORY https://github.com/SamuelMarks/c-str-span.git
        GIT_TAG master
    )
    FetchContent_MakeAvailable(c_str_span)
    include_directories(${c_str_span_SOURCE_DIR})

    # c89stringutils
    FetchContent_Declare(
        c89stringutils
        GIT_REPOSITORY https://github.com/offscale/c89stringutils.git
        GIT_TAG master
    )
    FetchContent_MakeAvailable(c89stringutils)
    include_directories(${c89stringutils_SOURCE_DIR}/c89stringutils)

    # curl
    FetchContent_Declare(
        curl
        GIT_REPOSITORY https://github.com/curl/curl.git
        GIT_TAG curl-8_4_0
    )
    set(BUILD_CURL_EXE OFF CACHE BOOL "" FORCE)
    set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
    set(CURL_DISABLE_TESTS ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(curl)

    # parson
    FetchContent_Declare(
        parson
        GIT_REPOSITORY https://github.com/kgabis/parson.git
        GIT_TAG master
    )
    FetchContent_MakeAvailable(parson)
    include_directories(${parson_SOURCE_DIR})
    if(TARGET parson AND NOT TARGET parson::parson)
        add_library(parson::parson ALIAS parson)
    endif()
    if(TARGET curl AND NOT TARGET CURL::libcurl)
        add_library(CURL::libcurl ALIAS curl)
    elseif(TARGET libcurl AND NOT TARGET CURL::libcurl)
        add_library(CURL::libcurl ALIAS libcurl)
    endif()
    
    # We still keep the module path just in case
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")
endif (CMAKE_VERSION VERSION_GREATER_EQUAL "3.11" AND NOT VCPKG_TOOLCHAIN)

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
add_subdirectory("src")
option(BUILD_C "Build C (with flex and bison)" OFF)
if (BUILD_C)
    add_subdirectory("c")
endif (BUILD_C)

include(CTest)
option(BUILD_TESTING "Build the tests" OFF)
option(C_CDD_BUILD_TESTING "Build the tests" OFF)
if (C_CDD_BUILD_TESTING)
    add_subdirectory("src/tests")
endif (C_CDD_BUILD_TESTING)

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
