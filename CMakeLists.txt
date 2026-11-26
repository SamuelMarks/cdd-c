cmake_minimum_required(VERSION 3.15...4.0)

foreach(_pol CMP0069 CMP0077 CMP0091 CMP0092 CMP0135 CMP0141)
    if(POLICY ${_pol})
        cmake_policy(SET ${_pol} NEW)
    endif()
endforeach()

project(c_cdd VERSION 0.0.5 LANGUAGES C)

if(DEFINED PROJECT_IS_TOP_LEVEL)
    set(C_CDD_IS_TOP_LEVEL "${PROJECT_IS_TOP_LEVEL}")
elseif(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(C_CDD_IS_TOP_LEVEL TRUE)
else()
    set(C_CDD_IS_TOP_LEVEL FALSE)
endif()

string(TOUPPER "${PROJECT_NAME}" PROJECT_PREFIX)
string(REPLACE "-" "_" PROJECT_PREFIX "${PROJECT_PREFIX}")

# Capture CLI coverage flags before stripping them from directory scope so they
# do not leak into third-party FetchContent subprojects.
set(_C_CDD_CLI_HAD_COVERAGE FALSE)
if(C_CDD_IS_TOP_LEVEL AND "${CMAKE_C_FLAGS};${CMAKE_C_FLAGS_DEBUG}" MATCHES "(--coverage|-fprofile-arcs|-ftest-coverage)")
    set(_C_CDD_CLI_HAD_COVERAGE TRUE)
endif()

foreach(_flag_var
    CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
    CMAKE_C_FLAGS_RELWITHDEBINFO CMAKE_C_FLAGS_MINSIZEREL
    CMAKE_EXE_LINKER_FLAGS CMAKE_EXE_LINKER_FLAGS_DEBUG
    CMAKE_SHARED_LINKER_FLAGS CMAKE_SHARED_LINKER_FLAGS_DEBUG)
    if(DEFINED ${_flag_var})
        string(REPLACE "--coverage" "" ${_flag_var} "${${_flag_var}}")
        string(REPLACE "-fprofile-arcs" "" ${_flag_var} "${${_flag_var}}")
        string(REPLACE "-ftest-coverage" "" ${_flag_var} "${${_flag_var}}")
    endif()
endforeach()

# Consume optional CI / cross-project matrix cache variables to prevent unused warnings
set(CDD_CHARSET "" CACHE STRING "c-ci matrix override for character set")
set(CDD_CRT_LINKAGE "" CACHE STRING "c-ci matrix override for MSVC CRT linkage")
set(CDD_DEPS "" CACHE STRING "c-ci matrix override for dependency source")
set(CDD_LTO "" CACHE STRING "c-ci matrix override for LTO")
set(CDD_MSVC_RTC "" CACHE STRING "c-ci matrix override for MSVC runtime checks")
set(CDD_THREADING "" CACHE STRING "c-ci matrix override for threading mode")
mark_as_advanced(CDD_CHARSET CDD_CRT_LINKAGE CDD_DEPS CDD_LTO CDD_MSVC_RTC CDD_THREADING)
foreach(_unused_cli_var
    CDD_C_BUILD_TESTING
    CDD_C_TESTING
    cdd_c_TESTING
    CMAKE_CXX_COMPILER
    CMAKE_DISABLE_FIND_PACKAGE_Brotli
    CMAKE_DISABLE_FIND_PACKAGE_CURL
    CMAKE_DISABLE_FIND_PACKAGE_SQLite3
    CMAKE_DISABLE_FIND_PACKAGE_ZLIB)
    if(DEFINED ${_unused_cli_var})
        set(_c_cdd_ignore "${${_unused_cli_var}}")
    endif()
endforeach()
unset(_c_cdd_ignore)

# ==============================================================================
# 1. Build System Options
# ==============================================================================
if(CMAKE_SYSTEM_NAME STREQUAL "DOS")
    set(_cdd_tf_only_default ON)
else()
    set(_cdd_tf_only_default OFF)
endif()
option(CDD_C_TRANSFORMER_ONLY "Build only standalone CST/AST transformer library and CLI" ${_cdd_tf_only_default})

if(CMAKE_SYSTEM_NAME STREQUAL "DOS" OR EMSCRIPTEN OR CMAKE_SYSTEM_NAME STREQUAL "WASI")
    option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
else()
    option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
endif()

if(C_CDD_IS_TOP_LEVEL AND NOT CDD_C_TRANSFORMER_ONLY)
    set(_cdd_tests_default ON)
    if(DEFINED BUILD_TESTING AND NOT BUILD_TESTING)
        set(_cdd_tests_default OFF)
    endif()
else()
    set(_cdd_tests_default OFF)
    if(C_CDD_IS_TOP_LEVEL AND CDD_C_TRANSFORMER_ONLY AND BUILD_TESTING)
        set(_cdd_tests_default ON)
    endif()
endif()
option(C_CDD_BUILD_TESTS "Build cdd-c test suite" ${_cdd_tests_default})
option(BUILD_TESTING_${PROJECT_NAME} "Build cdd-c test suite" ${C_CDD_BUILD_TESTS})

set(_C_CDD_ENABLE_TESTS FALSE)
if(C_CDD_IS_TOP_LEVEL AND C_CDD_BUILD_TESTS AND BUILD_TESTING_${PROJECT_NAME})
    if(NOT DEFINED BUILD_TESTING OR BUILD_TESTING)
        set(_C_CDD_ENABLE_TESTS TRUE)
    endif()
endif()

option(ENABLE_COVERAGE "Enable coverage reporting" OFF)
option(${PROJECT_PREFIX}_COVERAGE "Enable coverage reporting" ${ENABLE_COVERAGE})
option(C_CDD_USE_ASAN "Enable AddressSanitizer" OFF)
option(C_CDD_USE_UBSAN "Enable UndefinedBehaviorSanitizer" OFF)
option(C_CDD_INSTALL "Generate install and CMake package export rules" ON)

set(${PROJECT_PREFIX}_CRT_LINKAGE "Shared" CACHE STRING "MSVC CRT Linkage: Shared or Static")
set_property(CACHE ${PROJECT_PREFIX}_CRT_LINKAGE PROPERTY STRINGS "Shared" "Static")

set(${PROJECT_PREFIX}_CHARSET "Unicode" CACHE STRING "Charset to use: Unicode, ANSI, or MBCS")
set_property(CACHE ${PROJECT_PREFIX}_CHARSET PROPERTY STRINGS "Unicode" "ANSI" "MBCS")

if(CMAKE_SYSTEM_NAME STREQUAL "WASI" OR CMAKE_SYSTEM_NAME STREQUAL "DOS")
    set(_cdd_default_threading "Single-threaded")
else()
    set(_cdd_default_threading "Multi-threaded")
endif()
set(${PROJECT_PREFIX}_THREADING "${_cdd_default_threading}" CACHE STRING "Threading: Multi-threaded or Single-threaded")
set_property(CACHE ${PROJECT_PREFIX}_THREADING PROPERTY STRINGS "Multi-threaded" "Single-threaded")

option(${PROJECT_PREFIX}_LTO "Enable Link-Time Optimization (LTO)" OFF)
option(${PROJECT_PREFIX}_ENABLE_LTO "Enable Link-Time Optimization (LTO)" OFF)

set(${PROJECT_PREFIX}_MSVC_RTC "" CACHE STRING "MSVC Runtime Checks: RTC1, RTCs, RTCu, or empty")
set_property(CACHE ${PROJECT_PREFIX}_MSVC_RTC PROPERTY STRINGS "" "RTC1" "RTCs" "RTCu" "OFF")

set(CDD_SQLITE "OFF" CACHE STRING "SQLite backend mode: OFF, LINKED, or DYNAMIC")
set_property(CACHE CDD_SQLITE PROPERTY STRINGS "OFF" "LINKED" "DYNAMIC")

set(CDD_MYSQL "OFF" CACHE STRING "MySQL backend mode: OFF, LINKED, or DYNAMIC")
set_property(CACHE CDD_MYSQL PROPERTY STRINGS "OFF" "LINKED" "DYNAMIC")

# ==============================================================================
# 2. Option Normalization & Validation
# ==============================================================================
set(_C_CDD_EFFECTIVE_CHARSET "${${PROJECT_PREFIX}_CHARSET}")
if(NOT "${CDD_CHARSET}" STREQUAL "")
    set(_C_CDD_EFFECTIVE_CHARSET "${CDD_CHARSET}")
endif()
string(TOUPPER "${_C_CDD_EFFECTIVE_CHARSET}" _C_CDD_CHARSET_UPPER)
if(_C_CDD_CHARSET_UPPER STREQUAL "UNICODE")
    set(_C_CDD_EFFECTIVE_CHARSET "UNICODE")
elseif(_C_CDD_CHARSET_UPPER STREQUAL "ANSI" OR _C_CDD_CHARSET_UPPER STREQUAL "MBCS")
    set(_C_CDD_EFFECTIVE_CHARSET "ANSI")
else()
    message(FATAL_ERROR "Invalid ${PROJECT_PREFIX}_CHARSET='${_C_CDD_EFFECTIVE_CHARSET}'. Expected Unicode or ANSI.")
endif()

set(_C_CDD_EFFECTIVE_THREADING "${${PROJECT_PREFIX}_THREADING}")
if(NOT "${CDD_THREADING}" STREQUAL "")
    set(_C_CDD_EFFECTIVE_THREADING "${CDD_THREADING}")
endif()
string(TOUPPER "${_C_CDD_EFFECTIVE_THREADING}" _C_CDD_THREADING_UPPER)
if(_C_CDD_THREADING_UPPER MATCHES "^(MULTI|MULTI-THREADED|MULTITHREADED|ON|TRUE|1)$")
    set(_C_CDD_USE_MULTITHREAD TRUE)
elseif(_C_CDD_THREADING_UPPER MATCHES "^(SINGLE|SINGLE-THREADED|SINGLETHREADED|OFF|FALSE|0)$")
    set(_C_CDD_USE_MULTITHREAD FALSE)
else()
    message(FATAL_ERROR "Invalid ${PROJECT_PREFIX}_THREADING='${_C_CDD_EFFECTIVE_THREADING}'. Expected Multi-threaded or Single-threaded.")
endif()

set(_C_CDD_EFFECTIVE_CRT "${${PROJECT_PREFIX}_CRT_LINKAGE}")
if(NOT "${CDD_CRT_LINKAGE}" STREQUAL "")
    set(_C_CDD_EFFECTIVE_CRT "${CDD_CRT_LINKAGE}")
endif()
set(_C_CDD_MSVC_RUNTIME_LIBRARY "")
if(MSVC)
    if(DEFINED CMAKE_MSVC_RUNTIME_LIBRARY AND NOT "${CMAKE_MSVC_RUNTIME_LIBRARY}" STREQUAL "")
        set(_C_CDD_MSVC_RUNTIME_LIBRARY "${CMAKE_MSVC_RUNTIME_LIBRARY}")
    else()
        string(TOUPPER "${_C_CDD_EFFECTIVE_CRT}" _C_CDD_CRT_UPPER)
        if(_C_CDD_CRT_UPPER STREQUAL "STATIC" OR _C_CDD_CRT_UPPER STREQUAL "MT")
            set(_C_CDD_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
        elseif(_C_CDD_CRT_UPPER STREQUAL "MTD")
            set(_C_CDD_MSVC_RUNTIME_LIBRARY "MultiThreadedDebug")
        elseif(_C_CDD_CRT_UPPER STREQUAL "SHARED" OR _C_CDD_CRT_UPPER STREQUAL "MD")
            set(_C_CDD_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
        elseif(_C_CDD_CRT_UPPER STREQUAL "MDD")
            set(_C_CDD_MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL")
        else()
            message(FATAL_ERROR "Invalid ${PROJECT_PREFIX}_CRT_LINKAGE='${_C_CDD_EFFECTIVE_CRT}'. Expected Shared or Static.")
        endif()
    endif()
endif()

set(_C_CDD_EFFECTIVE_RTC "${${PROJECT_PREFIX}_MSVC_RTC}")
if(NOT "${CDD_MSVC_RTC}" STREQUAL "")
    set(_C_CDD_EFFECTIVE_RTC "${CDD_MSVC_RTC}")
endif()
set(_C_CDD_RTC_FLAG "")
if(MSVC AND NOT "${_C_CDD_EFFECTIVE_RTC}" STREQUAL "")
    string(TOUPPER "${_C_CDD_EFFECTIVE_RTC}" _C_CDD_RTC_UPPER)
    if(_C_CDD_RTC_UPPER STREQUAL "OFF" OR _C_CDD_RTC_UPPER STREQUAL "NONE")
        set(_C_CDD_RTC_FLAG "")
    elseif(_C_CDD_RTC_UPPER STREQUAL "RTC1" OR _C_CDD_RTC_UPPER STREQUAL "/RTC1" OR _C_CDD_RTC_UPPER STREQUAL "1")
        set(_C_CDD_RTC_FLAG "/RTC1")
    elseif(_C_CDD_RTC_UPPER STREQUAL "RTCS" OR _C_CDD_RTC_UPPER STREQUAL "/RTCS" OR _C_CDD_RTC_UPPER STREQUAL "S")
        set(_C_CDD_RTC_FLAG "/RTCs")
    elseif(_C_CDD_RTC_UPPER STREQUAL "RTCU" OR _C_CDD_RTC_UPPER STREQUAL "/RTCU" OR _C_CDD_RTC_UPPER STREQUAL "U")
        set(_C_CDD_RTC_FLAG "/RTCu")
    else()
        message(FATAL_ERROR "Invalid ${PROJECT_PREFIX}_MSVC_RTC='${_C_CDD_EFFECTIVE_RTC}'. Expected RTC1, RTCs, RTCu, or empty.")
    endif()
endif()

if(MSVC AND C_CDD_IS_TOP_LEVEL AND NOT "${_C_CDD_RTC_FLAG}" STREQUAL "")
    foreach(_cfg_var
        CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
        CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO)
        if(DEFINED ${_cfg_var})
            string(REGEX REPLACE "[/-]O(1|2|x|b1|b2)" "" ${_cfg_var} "${${_cfg_var}}")
            string(REGEX REPLACE "[/-]RTC[1suc]+" "" ${_cfg_var} "${${_cfg_var}}")
            string(REGEX REPLACE "[/-]GL" "" ${_cfg_var} "${${_cfg_var}}")
        endif()
    endforeach()
endif()

set(_C_CDD_IS_WATCOM_16 FALSE)
if(CMAKE_C_COMPILER_ID MATCHES "Watcom")
    get_filename_component(_c_cdd_ow_cc_name "${CMAKE_C_COMPILER}" NAME)
    if(_c_cdd_ow_cc_name MATCHES "^wcl(\\.exe)?$"
       OR CMAKE_SYSTEM_PROCESSOR STREQUAL "I86"
       OR CMAKE_SIZEOF_VOID_P EQUAL 2)
        set(_C_CDD_IS_WATCOM_16 TRUE)
    endif()
    unset(_c_cdd_ow_cc_name)
endif()

# Ensure uniform Large Memory Model (-ml) across all targets and FetchContent
# subprojects in 16-bit DOS builds so pointer/struct ABIs (e.g. az_span) match.
# Strip -zc because combining -ml + -zm + -zc in 16-bit OpenWatcom places
# string literals in module_TEXT while loading CS from func_TEXT.
if(C_CDD_IS_TOP_LEVEL AND _C_CDD_IS_WATCOM_16)
    string(REGEX REPLACE "(^| )-m[smch]( |$)" "\\1-ml\\2" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
    string(REGEX REPLACE "(^| )-m[smch]( |$)" "\\1-ml\\2" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
    if(NOT CMAKE_C_FLAGS MATCHES "(^| )-m[lh]( |$)")
        string(APPEND CMAKE_C_FLAGS " -ml")
    endif()
    string(REGEX REPLACE "(^| )-zc( |$)" "\\1\\2" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
    string(REGEX REPLACE "(^| )-zc( |$)" "\\1\\2" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
    string(REGEX REPLACE "(^| )-zt=[0-9]+( |$)" "\\1-zt=32\\2" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
    string(REPLACE "-d2" "-d0" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
    string(REPLACE "-od" "-os" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
    string(REPLACE "debug all" "" CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG}")
endif()

set(_C_CDD_MSVC_DEBUG_FORMAT "")
if(MSVC)
    if(ENABLE_COVERAGE OR ${PROJECT_PREFIX}_COVERAGE)
        set(_C_CDD_MSVC_DEBUG_FORMAT "ProgramDatabase")
    else()
        set(_C_CDD_MSVC_DEBUG_FORMAT "$<$<CONFIG:Debug,RelWithDebInfo>:Embedded>")
    endif()
endif()

set(_C_CDD_WANT_LTO FALSE)
if(${PROJECT_PREFIX}_LTO OR ${PROJECT_PREFIX}_ENABLE_LTO OR CDD_LTO OR CMAKE_INTERPROCEDURAL_OPTIMIZATION)
    if("${_C_CDD_RTC_FLAG}" STREQUAL ""
       AND NOT CMAKE_C_COMPILER_ID MATCHES "Watcom"
       AND NOT CMAKE_SYSTEM_NAME STREQUAL "DOS")
        set(_C_CDD_WANT_LTO TRUE)
    endif()
endif()

set(_C_CDD_IPO_SUPPORTED FALSE)
if(_C_CDD_WANT_LTO)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT _C_CDD_IPO_SUPPORTED OUTPUT _C_CDD_IPO_ERROR LANGUAGES C)
    if(NOT _C_CDD_IPO_SUPPORTED)
        message(WARNING "LTO/IPO requested for c_cdd, but is not supported by this toolchain: ${_C_CDD_IPO_ERROR}")
    endif()
endif()

if(_C_CDD_USE_MULTITHREAD
   AND NOT WIN32
   AND NOT EMSCRIPTEN
   AND NOT CMAKE_SYSTEM_NAME STREQUAL "WASI"
   AND NOT CMAKE_SYSTEM_NAME STREQUAL "DOS")
    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads QUIET)
endif()

# ==============================================================================
# 3. Target-Scoped Compiler Flags, Feature Detection, Coverage, and Sanitizers
# ==============================================================================
add_library("${PROJECT_NAME}_compiler_flags" INTERFACE)
add_library(c_cdd_internal_flags INTERFACE)

set(_C_CDD_IS_MSVC_FRONTEND FALSE)
set(_C_CDD_IS_GNU_FRONTEND FALSE)
if(MSVC OR CMAKE_C_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
    set(_C_CDD_IS_MSVC_FRONTEND TRUE)
elseif(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang|AppleClang|ARMClang|LCC")
    set(_C_CDD_IS_GNU_FRONTEND TRUE)
endif()

if(_C_CDD_IS_MSVC_FRONTEND)
    target_compile_options(c_cdd_internal_flags INTERFACE
        /W4
        /Gy
        $<$<BOOL:${C_CDD_IS_TOP_LEVEL}>:/WX>
    )
    if(NOT "${_C_CDD_RTC_FLAG}" STREQUAL "")
        target_compile_options(c_cdd_internal_flags INTERFACE "/Od" "${_C_CDD_RTC_FLAG}")
    endif()
    if(MSVC AND MSVC_VERSION LESS 1800)
        target_include_directories(c_cdd_internal_flags SYSTEM INTERFACE
            "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/msvc>"
        )
    endif()
elseif(_C_CDD_IS_GNU_FRONTEND)
    target_compile_options(c_cdd_internal_flags INTERFACE
        -fno-common
        -Wall
        -Wextra
        -Wformat=2
        -Wcast-align
        -Wcast-qual
        -Wpointer-arith
        -Wmissing-prototypes
        -Wstrict-prototypes
        -Wmissing-declarations
        -Wold-style-definition
        -Wdeclaration-after-statement
        -Wunused-result
        -pedantic
        $<$<BOOL:${C_CDD_IS_TOP_LEVEL}>:-Werror>
    )
elseif(CMAKE_C_COMPILER_ID MATCHES "Watcom")
    if(_C_CDD_IS_WATCOM_16)
        target_compile_options(c_cdd_internal_flags INTERFACE -ml -os -d0 -zm -zt=32)
    else()
        target_compile_options(c_cdd_internal_flags INTERFACE -zm -zc)
    endif()
    target_compile_definitions(c_cdd_internal_flags INTERFACE __inline__=__inline)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "WASI")
    target_compile_definitions(c_cdd_internal_flags INTERFACE
        _WASI_EMULATED_GETPID
        _WASI_EMULATED_SIGNAL
        _WASI_EMULATED_PROCESS_CLOCKS
    )
    target_compile_options(c_cdd_internal_flags INTERFACE
        -ffunction-sections
        -fdata-sections
    )
    target_link_options(c_cdd_internal_flags INTERFACE
        -lwasi-emulated-getpid
        -lwasi-emulated-signal
        -lwasi-emulated-process-clocks
        -Wl,--gc-sections
    )
elseif(EMSCRIPTEN)
    target_link_options(c_cdd_internal_flags INTERFACE
        "-sALLOW_MEMORY_GROWTH=1"
        "-sSTACK_SIZE=524288"
        "-sNODERAWFS=1"
    )
endif()

# Platform POSIX / BSD / Darwin feature-test macros & isolated asprintf check
set(_C_CDD_PLATFORM_DEFS "")
if(APPLE)
    list(APPEND _C_CDD_PLATFORM_DEFS _DARWIN_C_SOURCE _POSIX_C_SOURCE=200809L)
elseif(NOT WIN32 AND NOT MSVC AND NOT CMAKE_C_COMPILER_ID MATCHES "Watcom" AND NOT CMAKE_SYSTEM_NAME STREQUAL "DOS")
    include(CheckCSourceCompiles)
    check_c_source_compiles("
        #if defined(__linux__) || defined(__linux)
        # include <features.h>
        #endif
        #if defined(__GLIBC__) && (__GLIBC__ > 2 || (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 20))
        #error glibc >= 2.20
        #endif
        int main(void) { return 0; }
    " C_CDD_USE_BSD_SOURCE)
    if(C_CDD_USE_BSD_SOURCE)
        list(APPEND _C_CDD_PLATFORM_DEFS _BSD_SOURCE _POSIX_C_SOURCE=200809L)
    else()
        list(APPEND _C_CDD_PLATFORM_DEFS _DEFAULT_SOURCE _POSIX_C_SOURCE=200809L)
    endif()
endif()

if(_C_CDD_PLATFORM_DEFS)
    target_compile_definitions(c_cdd_internal_flags INTERFACE ${_C_CDD_PLATFORM_DEFS})
endif()

function(c_cdd_detect_asprintf)
    if(MSVC OR WIN32 OR CMAKE_C_COMPILER_ID MATCHES "Watcom" OR CMAKE_SYSTEM_NAME STREQUAL "DOS")
        return()
    endif()
    include(CheckSymbolExists)
    set(CMAKE_REQUIRED_DEFINITIONS "")
    foreach(_def IN LISTS _C_CDD_PLATFORM_DEFS)
        list(APPEND CMAKE_REQUIRED_DEFINITIONS "-D${_def}")
    endforeach()
    check_symbol_exists("asprintf" "stdio.h" C_CDD_HAVE_ASPRINTF)
    if(C_CDD_HAVE_ASPRINTF)
        target_compile_definitions(c_cdd_internal_flags INTERFACE HAVE_ASPRINTF)
    endif()
endfunction()
c_cdd_detect_asprintf()

# Coverage (target-scoped, never pollutes subprojects)
set(_C_CDD_COVERAGE_ACTIVE FALSE)
if(C_CDD_IS_TOP_LEVEL AND (ENABLE_COVERAGE OR ${PROJECT_PREFIX}_COVERAGE OR _C_CDD_CLI_HAD_COVERAGE))
    set(_C_CDD_COVERAGE_ACTIVE TRUE)
endif()

if(_C_CDD_COVERAGE_ACTIVE)
    add_library(c_cdd_internal_coverage INTERFACE)
    if(_C_CDD_IS_GNU_FRONTEND)
        target_compile_options(c_cdd_internal_coverage INTERFACE --coverage -O0 -g)
        target_link_options(c_cdd_internal_coverage INTERFACE --coverage)
        find_program(GCOVR_PATH gcovr)
        if(GCOVR_PATH)
            add_custom_target(coverage
                COMMAND "${CMAKE_CTEST_COMMAND}" --output-on-failure
                COMMAND "${GCOVR_PATH}"
                        "${PROJECT_BINARY_DIR}"
                        -r "${PROJECT_SOURCE_DIR}"
                        --filter "${PROJECT_SOURCE_DIR}/src/"
                        --filter "${PROJECT_SOURCE_DIR}/include/"
                        --exclude "${PROJECT_SOURCE_DIR}/src/tests/"
                        --exclude "${PROJECT_SOURCE_DIR}/src/mocks/"
                        --html --html-details -o coverage.html
                        --fail-under-line 100
                        --fail-under-branch 100
                        --merge-mode-functions=separate
                WORKING_DIRECTORY "${PROJECT_BINARY_DIR}"
                COMMENT "Running gcovr and asserting 100% line and branch coverage"
            )
        endif()
    elseif(MSVC)
        target_link_options(c_cdd_internal_coverage INTERFACE /DEBUG)
    endif()
endif()

# Sanitizers (target-scoped)
if((C_CDD_USE_ASAN OR C_CDD_USE_UBSAN) AND _C_CDD_IS_GNU_FRONTEND)
    add_library(c_cdd_internal_sanitizers INTERFACE)
    if(C_CDD_USE_ASAN)
        target_compile_options(c_cdd_internal_sanitizers INTERFACE -fsanitize=address -fno-omit-frame-pointer -g -O1)
        target_link_options(c_cdd_internal_sanitizers INTERFACE -fsanitize=address)
    endif()
    if(C_CDD_USE_UBSAN)
        target_compile_options(c_cdd_internal_sanitizers INTERFACE -fsanitize=undefined -fno-omit-frame-pointer -g -O1)
        target_link_options(c_cdd_internal_sanitizers INTERFACE -fsanitize=undefined)
    endif()
endif()

# Helper to apply all target-scoped settings uniformly to every cdd-c target
function(c_cdd_apply_target_settings target_name)
    if(NOT TARGET "${target_name}")
        return()
    endif()
    get_target_property(_tgt_type "${target_name}" TYPE)
    if(_tgt_type STREQUAL "INTERFACE_LIBRARY")
        return()
    endif()

    target_link_libraries("${target_name}" PRIVATE "$<BUILD_INTERFACE:c_cdd_internal_flags>")
    if(TARGET c_cdd_internal_coverage)
        target_link_libraries("${target_name}" PRIVATE "$<BUILD_INTERFACE:c_cdd_internal_coverage>")
    endif()
    if(TARGET c_cdd_internal_sanitizers)
        target_link_libraries("${target_name}" PRIVATE "$<BUILD_INTERFACE:c_cdd_internal_sanitizers>")
    endif()

    set_property(TARGET "${target_name}" PROPERTY LINKER_LANGUAGE C)

    if(NOT CMAKE_C_COMPILER_ID MATCHES "Watcom" AND "c_std_90" IN_LIST CMAKE_C_COMPILE_FEATURES)
        set_target_properties("${target_name}" PROPERTIES
            C_STANDARD 90
            C_STANDARD_REQUIRED ON
            C_EXTENSIONS OFF
        )
    endif()

    if(NOT CMAKE_C_COMPILER_ID MATCHES "Watcom" AND NOT CMAKE_GENERATOR MATCHES "Visual Studio 8 2005")
        set_property(TARGET "${target_name}" PROPERTY C_VISIBILITY_PRESET hidden)
    endif()

    if(_tgt_type MATCHES "^(STATIC_LIBRARY|SHARED_LIBRARY|OBJECT_LIBRARY)$")
        if(NOT WIN32 AND NOT CYGWIN AND NOT MINGW AND NOT CMAKE_C_COMPILER_ID MATCHES "Watcom" AND NOT CMAKE_SYSTEM_NAME STREQUAL "DOS")
            set_property(TARGET "${target_name}" PROPERTY POSITION_INDEPENDENT_CODE ON)
        endif()
    endif()

    if(C_CDD_IS_TOP_LEVEL)
        set_target_properties("${target_name}" PROPERTIES
            ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}"
            LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}"
            RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}"
        )
    endif()

    if(_C_CDD_EFFECTIVE_CHARSET STREQUAL "UNICODE")
        target_compile_definitions("${target_name}" PRIVATE UNICODE _UNICODE C_CDD_UNICODE=1)
    elseif(_C_CDD_EFFECTIVE_CHARSET STREQUAL "ANSI")
        if(WIN32)
            target_compile_definitions("${target_name}" PRIVATE _MBCS C_CDD_ANSI=1)
        else()
            target_compile_definitions("${target_name}" PRIVATE C_CDD_ANSI=1)
        endif()
    endif()

    if(_C_CDD_USE_MULTITHREAD)
        target_compile_definitions("${target_name}" PRIVATE C_CDD_MULTITHREADED=1)
        if(TARGET Threads::Threads
           AND NOT WIN32
           AND NOT EMSCRIPTEN
           AND NOT CMAKE_SYSTEM_NAME STREQUAL "WASI"
           AND NOT CMAKE_SYSTEM_NAME STREQUAL "DOS")
            target_link_libraries("${target_name}" PRIVATE Threads::Threads)
        endif()
    else()
        target_compile_definitions("${target_name}" PRIVATE C_CDD_SINGLE_THREADED=1)
    endif()

    if(_C_CDD_IPO_SUPPORTED)
        set_property(TARGET "${target_name}" PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
    endif()

    if(MSVC)
        if(NOT "${_C_CDD_MSVC_RUNTIME_LIBRARY}" STREQUAL "")
            set_property(TARGET "${target_name}" PROPERTY MSVC_RUNTIME_LIBRARY "${_C_CDD_MSVC_RUNTIME_LIBRARY}")
        endif()
        if(POLICY CMP0141 AND NOT "${_C_CDD_MSVC_DEBUG_FORMAT}" STREQUAL "")
            set_property(TARGET "${target_name}" PROPERTY MSVC_DEBUG_INFORMATION_FORMAT "${_C_CDD_MSVC_DEBUG_FORMAT}")
        endif()
    endif()
endfunction()

# Mark third-party dependency headers as SYSTEM on the target itself (no global include_directories)
function(c_cdd_mark_system_includes target_name)
    if(NOT TARGET "${target_name}")
        return()
    endif()
    get_target_property(_aliased "${target_name}" ALIASED_TARGET)
    if(_aliased)
        return()
    endif()
    get_target_property(_inc "${target_name}" INTERFACE_INCLUDE_DIRECTORIES)
    if(_inc)
        set_target_properties("${target_name}" PROPERTIES INTERFACE_SYSTEM_INCLUDE_DIRECTORIES "${_inc}")
    endif()
endfunction()

function(c_cdd_configure_fetched_target target_name static_define_macro)
    if(NOT TARGET "${target_name}")
        return()
    endif()
    get_target_property(_aliased "${target_name}" ALIASED_TARGET)
    if(_aliased)
        return()
    endif()
    c_cdd_mark_system_includes("${target_name}")
    get_target_property(_dep_type "${target_name}" TYPE)
    get_target_property(_is_imported "${target_name}" IMPORTED)
    if(_is_imported)
        if(_dep_type STREQUAL "STATIC_LIBRARY" AND NOT "${static_define_macro}" STREQUAL "")
            target_compile_definitions("${target_name}" INTERFACE "${static_define_macro}")
        endif()
        return()
    endif()
    if(_dep_type STREQUAL "INTERFACE_LIBRARY")
        return()
    endif()
    if(_dep_type STREQUAL "STATIC_LIBRARY" AND NOT "${static_define_macro}" STREQUAL "")
        target_compile_definitions("${target_name}" PUBLIC "${static_define_macro}")
    endif()
    if(NOT WIN32 AND NOT CYGWIN AND NOT MINGW AND NOT CMAKE_C_COMPILER_ID MATCHES "Watcom" AND NOT CMAKE_SYSTEM_NAME STREQUAL "DOS")
        set_property(TARGET "${target_name}" PROPERTY POSITION_INDEPENDENT_CODE ON)
    endif()
    if(CMAKE_SYSTEM_NAME STREQUAL "WASI")
        target_compile_definitions("${target_name}" PRIVATE
            _WASI_EMULATED_GETPID
            _WASI_EMULATED_SIGNAL
            _WASI_EMULATED_PROCESS_CLOCKS
        )
    endif()
    if(_C_CDD_IS_WATCOM_16)
        target_compile_options("${target_name}" PRIVATE -ml -os -d0 -zm -zt=32)
    endif()
    if(MSVC)
        if(MSVC_VERSION LESS 1800)
            target_include_directories("${target_name}" SYSTEM PRIVATE
                "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/msvc>"
            )
        endif()
        if(NOT "${_C_CDD_MSVC_RUNTIME_LIBRARY}" STREQUAL "")
            set_property(TARGET "${target_name}" PROPERTY MSVC_RUNTIME_LIBRARY "${_C_CDD_MSVC_RUNTIME_LIBRARY}")
        endif()
    endif()
endfunction()

# ==============================================================================
# 4. Dependencies (FetchContent_MakeAvailable / vcpkg, master branch)
# ==============================================================================
include(FetchContent)

if(APPLE)
    set(C89STRINGUTILS_STRERROR_R_CHAR_P OFF CACHE BOOL "" FORCE)
elseif(NOT MSVC AND NOT CMAKE_C_COMPILER_ID MATCHES "Watcom")
    set(C89STRINGUTILS_HAVE_ASPRINTF OFF CACHE BOOL "" FORCE)
    set(C89STRINGUTILS_HAVE_VASPRINTF OFF CACHE BOOL "" FORCE)
    set(C89STRINGUTILS_HAVE_STRCASESTR OFF CACHE BOOL "" FORCE)
    if(EMSCRIPTEN OR CMAKE_SYSTEM_NAME STREQUAL "WASI" OR CMAKE_SYSTEM_NAME MATCHES "BSD|DragonFly")
        set(C89STRINGUTILS_STRERROR_R_CHAR_P OFF CACHE BOOL "" FORCE)
    endif()
endif()

set(CFS_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(CFS_BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(CFS_INSTALL OFF CACHE BOOL "" FORCE)
set(CFS_CHARSET "${_C_CDD_EFFECTIVE_CHARSET}" CACHE STRING "" FORCE)
if(_C_CDD_USE_MULTITHREAD)
    set(CFS_THREADING "Multi-threaded" CACHE STRING "" FORCE)
else()
    set(CFS_THREADING "Single-threaded" CACHE STRING "" FORCE)
endif()
if(MSVC)
    set(CFS_CRT_LINKAGE "${_C_CDD_EFFECTIVE_CRT}" CACHE STRING "" FORCE)
    set(CFS_MSVC_RTC "${_C_CDD_EFFECTIVE_RTC}" CACHE STRING "" FORCE)
endif()

macro(c_cdd_fetch_dependency NAME LOCAL_DIR GITHUB_REPO REQUIRE_CMAKELISTS)
    string(TOUPPER "${NAME}" _DEP_UPPER)
    string(REPLACE "-" "_" _DEP_UPPER "${_DEP_UPPER}")
    set(_DEP_LOCAL_DIR "")
    if("${FETCHCONTENT_SOURCE_DIR_${_DEP_UPPER}}" STREQUAL "")
        get_filename_component(_real_src_dir "${CMAKE_CURRENT_SOURCE_DIR}" REALPATH)
        foreach(_cand_rel
            "${CMAKE_CURRENT_SOURCE_DIR}/../${LOCAL_DIR}"
            "${CMAKE_CURRENT_SOURCE_DIR}/../../${LOCAL_DIR}"
            "${_real_src_dir}/../${LOCAL_DIR}"
            "${_real_src_dir}/../../${LOCAL_DIR}")
            if(EXISTS "${_cand_rel}")
                get_filename_component(_cand_abs "${_cand_rel}" REALPATH)
                if(NOT ${REQUIRE_CMAKELISTS} OR EXISTS "${_cand_abs}/CMakeLists.txt")
                    set(_DEP_LOCAL_DIR "${_cand_abs}")
                    break()
                endif()
            endif()
        endforeach()
    endif()

    if(_DEP_LOCAL_DIR)
        FetchContent_Declare(
            ${NAME}
            SOURCE_DIR "${_DEP_LOCAL_DIR}"
        )
    else()
        FetchContent_Declare(
            ${NAME}
            GIT_REPOSITORY "https://github.com/${GITHUB_REPO}.git"
            GIT_TAG "master"
        )
    endif()

    set(_c_cdd_saved_bt_defined FALSE)
    if(DEFINED BUILD_TESTING)
        set(_c_cdd_saved_bt "${BUILD_TESTING}")
        set(_c_cdd_saved_bt_defined TRUE)
    endif()
    set(_c_cdd_saved_bsl "${BUILD_SHARED_LIBS}")
    set(_c_cdd_saved_pvm_defined FALSE)
    if(DEFINED CMAKE_POLICY_VERSION_MINIMUM)
        set(_c_cdd_saved_pvm "${CMAKE_POLICY_VERSION_MINIMUM}")
        set(_c_cdd_saved_pvm_defined TRUE)
    else()
        set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
    endif()

    set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
    set(BUILD_SHARED_LIBS OFF)
    FetchContent_MakeAvailable(${NAME})
    set(BUILD_SHARED_LIBS "${_c_cdd_saved_bsl}")

    if(_c_cdd_saved_pvm_defined)
        set(CMAKE_POLICY_VERSION_MINIMUM "${_c_cdd_saved_pvm}")
    else()
        unset(CMAKE_POLICY_VERSION_MINIMUM)
    endif()
    if(_c_cdd_saved_bt_defined)
        set(BUILD_TESTING "${_c_cdd_saved_bt}" CACHE BOOL "" FORCE)
    else()
        unset(BUILD_TESTING CACHE)
    endif()
endmacro()

set(_C_CDD_ALLOW_VCPKG FALSE)
if(VCPKG_TOOLCHAIN AND NOT "${CDD_DEPS}" STREQUAL "FETCHCONTENT")
    set(_C_CDD_ALLOW_VCPKG TRUE)
endif()

# 4a. c-fs (SamuelMarks/c-fs@master)
if(NOT CDD_C_TRANSFORMER_ONLY AND NOT TARGET cfs)
    if(_C_CDD_ALLOW_VCPKG)
        find_package(cfs CONFIG QUIET)
    endif()
    if(NOT TARGET cfs)
        c_cdd_fetch_dependency(cfs "c-fs" "SamuelMarks/c-fs" TRUE)
    endif()
endif()
c_cdd_configure_fetched_target(cfs "CFS_STATIC_DEFINE")

# 4b. c-str-span (SamuelMarks/c-str-span@master)
if(NOT TARGET c_str_span)
    if(_C_CDD_ALLOW_VCPKG)
        find_package(c_str_span CONFIG QUIET)
        if(TARGET c_str_span::c_str_span AND NOT TARGET c_str_span)
            add_library(c_str_span ALIAS c_str_span::c_str_span)
        endif()
    endif()
    if(NOT TARGET c_str_span)
        c_cdd_fetch_dependency(c_str_span "c-str-span" "SamuelMarks/c-str-span" TRUE)
    endif()
endif()
c_cdd_configure_fetched_target(c_str_span "C_STR_SPAN_STATIC_DEFINE")

# 4c. c89stringutils (offscale/c89stringutils@master)
if(NOT TARGET c89stringutils)
    c_cdd_fetch_dependency(c89stringutils "c89stringutils" "offscale/c89stringutils" TRUE)
endif()
c_cdd_configure_fetched_target(c89stringutils "C89STRINGUTILS_STATIC_DEFINE")

# 4d. greatest (SamuelMarks/greatest@master) for test suite
if(_C_CDD_ENABLE_TESTS)
    c_cdd_fetch_dependency(greatest "greatest" "SamuelMarks/greatest" FALSE)
endif()

# ==============================================================================
# 5. Generated Configuration Header & Subdirectories
# ==============================================================================
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/config.h.in"
    "${PROJECT_BINARY_DIR}/src/${PROJECT_NAME}Config.h"
)
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/config.h.in"
    "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.h"
)

add_subdirectory("src")

if(_C_CDD_ENABLE_TESTS)
    find_program(MEMORYCHECK_COMMAND NAMES valgrind)
    set(MEMORYCHECK_COMMAND_OPTIONS "--leak-check=full --error-exitcode=1")
    include(CTest)
    enable_testing()
    add_subdirectory("src/tests")
endif()

# ==============================================================================
# 6. Install & Package Configuration Rules
# ==============================================================================
if(C_CDD_INSTALL)
    include(GNUInstallDirs)
    include(CMakePackageConfigHelpers)

    set(_C_CDD_PACKAGE_FIND_DEPS "")
    if(_C_CDD_USE_MULTITHREAD
       AND NOT WIN32
       AND NOT EMSCRIPTEN
       AND NOT CMAKE_SYSTEM_NAME STREQUAL "WASI"
       AND NOT CMAKE_SYSTEM_NAME STREQUAL "DOS")
        string(APPEND _C_CDD_PACKAGE_FIND_DEPS "find_dependency(Threads)
")
    endif()
    if(_C_CDD_LINKED_OPENSSL)
        string(APPEND _C_CDD_PACKAGE_FIND_DEPS "find_dependency(OpenSSL)
")
    endif()

    install(
        FILES "${PROJECT_BINARY_DIR}/src/${PROJECT_NAME}Config.h"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    )

    write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
        COMPATIBILITY SameMajorVersion
    )

    configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Config.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
        INSTALL_DESTINATION "${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}"
    )

    install(
        FILES
            "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
            "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
        DESTINATION "${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}"
    )
endif()

# ==============================================================================
# 7. Documentation (Top-Level Only)
# ==============================================================================
if(C_CDD_IS_TOP_LEVEL)
    find_package(Doxygen QUIET)
    if(DOXYGEN_FOUND AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in")
        configure_file(
            "${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in"
            "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile"
            @ONLY
        )
        add_custom_target(doc
            COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/docs"
            COMMAND "${DOXYGEN_EXECUTABLE}" "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile"
            WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
            COMMENT "Generating API documentation with Doxygen"
            VERBATIM
        )
    endif()
endif()
