#if defined(__clang__) || defined(__GNUC__)
#endif
/**
 * @file build_system.c
 * @brief Implementation of build system scaffolding.
 *
 * writes CMakeLists.txt files with logic to selectively link against
 * system networking libraries based on the target platform.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "c_cdd/memory.h"
#include "c_cdd_export.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "c_cdd/log.h"
#include "functions/emit/build_system.h"
#include "functions/parse/fs.h"
#include "functions/parse/str.h"
/* clang-format on */

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#ifndef strdup
#define strdup _strdup
#endif
#endif

#ifdef CDD_BUILD_TESTS
extern C_CDD_EXPORT int g_build_system_fail_io_after;
static enum cdd_c_error _cdd_test_mock_io(int x) {
  if (g_build_system_fail_io_after == 0) {
    g_build_system_fail_io_after = -1;
    return CDD_C_ERROR_IO;
  }
  if (g_build_system_fail_io_after > 0) {
    g_build_system_fail_io_after--;
  }
  return x < 0 ? CDD_C_ERROR_IO : CDD_C_SUCCESS;
}
C_CDD_EXPORT cdd_c_error_t test_build_system_mock_io_negative(void);
C_CDD_EXPORT cdd_c_error_t test_build_system_mock_io_negative(void) {
  return _cdd_test_mock_io(-1);
}
/** @brief Check I/O return code with mock support */
#define CHECK_IO_RC(x) (_cdd_test_mock_io(x) != CDD_C_SUCCESS)
#else
/** @brief Check I/O return code */
#define CHECK_IO_RC(x) ((x) < 0)
#endif

/**
 * @brief Emits C source code for cmake content.
 *
 * @param[in] fp Output file stream for `src/CMakeLists.txt`.
 * @param[in] project_name Target CMake project and library name.
 * @param[in] has_tests Non-zero to emit CTest and `greatest` test targets.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_IO on write failure.
 */
static cdd_c_error_t write_cmake_content(FILE *fp, const char *project_name,
                                         int has_tests) {
  /* Standard Settings */
  if (CHECK_IO_RC(fprintf(fp, "set(CMAKE_C_STANDARD 90)\n")))
    return CDD_C_ERROR_IO;
  if (CHECK_IO_RC(fprintf(fp, "set(CMAKE_C_STANDARD_REQUIRED ON)\n\n")))
    return CDD_C_ERROR_IO;

  /* Source Globbing (Simplification for generated projects) */
  if (CHECK_IO_RC(fprintf(
          fp, "file(GLOB SOURCES \"${CMAKE_CURRENT_SOURCE_DIR}/*.c\")\n")))
    return CDD_C_ERROR_IO;
  if (CHECK_IO_RC(fprintf(
          fp, "file(GLOB HEADERS \"${CMAKE_CURRENT_SOURCE_DIR}/*.h\")\n\n")))
    return CDD_C_ERROR_IO;

  if (CHECK_IO_RC(fprintf(fp,
                          "list(FILTER SOURCES EXCLUDE REGEX "
                          "\"(/|\\\\\\\\|^)test_[^/\\\\\\\\]+\\\\.c$\")\n")))
    return CDD_C_ERROR_IO;
  if (CHECK_IO_RC(fprintf(fp,
                          "list(FILTER HEADERS EXCLUDE REGEX "
                          "\"(/|\\\\\\\\|^)test_[^/\\\\\\\\]+\\\\.h$\")\n\n")))
    return CDD_C_ERROR_IO;

  /* Target */
  if (CHECK_IO_RC(fprintf(fp, "add_library(%s ${SOURCES} ${HEADERS})\n\n",
                          project_name)))
    return CDD_C_ERROR_IO;

  if (CHECK_IO_RC(fprintf(fp, "include(GenerateExportHeader)\n")))
    return CDD_C_ERROR_IO;
  if (CHECK_IO_RC(
          fprintf(fp,
                  "generate_export_header(%s EXPORT_FILE_NAME "
                  "${CMAKE_CURRENT_BINARY_DIR}/lib_export.h EXPORT_MACRO_NAME "
                  "LIB_EXPORT)\n\n",
                  project_name)))
    return CDD_C_ERROR_IO;

  /* Build Option: Shared/Static */
  if (CHECK_IO_RC(fprintf(fp, "if (BUILD_SHARED_LIBS)\n")))
    return CDD_C_ERROR_IO;
  if (CHECK_IO_RC(fprintf(fp,
                          "    target_compile_definitions(%s PRIVATE "
                          "LIB_EXPORTS)\n",
                          project_name)))
    return CDD_C_ERROR_IO;
  if (CHECK_IO_RC(fprintf(fp, "endif()\n\n")))
    return CDD_C_ERROR_IO;

  /* Dependency Logic */
  if (CHECK_IO_RC(fprintf(
          fp, "set(_CDD_OLD_BUILD_TESTING \"${BUILD_TESTING}\")\n"
              "set(PARSON_BUILD_TESTING OFF CACHE BOOL \"\" FORCE)\n"
              "set(C89STRINGUTILS_BUILD_TESTING OFF CACHE BOOL \"\" FORCE)\n"
              "set(C_STR_SPAN_BUILD_TESTING OFF CACHE BOOL \"\" FORCE)\n"
              "set(CFS_BUILD_TESTS OFF CACHE BOOL \"\" FORCE)\n"
              "set(C_ABSTRACT_HTTP_BUILD_TESTS OFF CACHE BOOL \"\" FORCE)\n"
              "set(BUILD_TESTING OFF)\n")))
    return CDD_C_ERROR_IO;
  if (CHECK_IO_RC(fprintf(fp, "include(FetchContent)\n")))
    return CDD_C_ERROR_IO;
  if (CHECK_IO_RC(
          fprintf(fp, "macro(cdd_sdk_declare_dep NAME DIR_NAME "
                      "GIT_REPO)\n"
                      "    set(_DEP_DIR \"\")\n"
                      "    get_filename_component(_CUR_REAL "
                      "\"${CMAKE_CURRENT_SOURCE_DIR}\" REALPATH)\n"
                      "    foreach(_rel \"../..\" \"../../..\" \"../../../..\" "
                      "\"../../../../..\")\n"
                      "        if(EXISTS "
                      "\"${_CUR_REAL}/${_rel}/${DIR_NAME}/"
                      "CMakeLists.txt\")\n"
                      "            get_filename_component(_DEP_DIR "
                      "\"${_CUR_REAL}/${_rel}/${DIR_NAME}\" REALPATH)\n"
                      "            break()\n"
                      "        endif()\n"
                      "    endforeach()\n")))
    return CDD_C_ERROR_IO;
  if (CHECK_IO_RC(fprintf(fp,
                          "    if(_DEP_DIR)\n"
                          "        FetchContent_Declare(${NAME} SOURCE_DIR "
                          "\"${_DEP_DIR}\")\n"
                          "    else()\n"
                          "        FetchContent_Declare(${NAME} GIT_REPOSITORY "
                          "\"${GIT_REPO}\" GIT_TAG \"master\")\n"
                          "    endif()\n"
                          "endmacro()\n\n")))
    return CDD_C_ERROR_IO;
  /* parson */
  if (CHECK_IO_RC(
          fprintf(fp, "if(TARGET parson)\n    message(STATUS \"parson already "
                      "provided by parent\")\nelse()\n")))
    return CDD_C_ERROR_IO;
  if (CHECK_IO_RC(fprintf(fp, "    set(parson_RESOLVED OFF)\n")))
    return CDD_C_ERROR_IO;
  if (CHECK_IO_RC(fprintf(fp, "    if(VCPKG_TOOLCHAIN)\n")))
    return CDD_C_ERROR_IO;
  if (CHECK_IO_RC(fprintf(fp, "        find_package(parson CONFIG QUIET)\n")))
    return CDD_C_ERROR_IO;
  if (CHECK_IO_RC(fprintf(fp, "        if(parson_FOUND)\n")))
    return CDD_C_ERROR_IO;
  if (CHECK_IO_RC(fprintf(fp, "            set(parson_RESOLVED ON)\n")))
    return CDD_C_ERROR_IO;
  if (CHECK_IO_RC(fprintf(fp, "        endif()\n")))
    return CDD_C_ERROR_IO;
  if (CHECK_IO_RC(fprintf(fp, "    endif()\n")))
    return CDD_C_ERROR_IO;
  if (CHECK_IO_RC(fprintf(fp, "    if(NOT parson_RESOLVED)\n")))
    return CDD_C_ERROR_IO;
  if (CHECK_IO_RC(fprintf(fp,
                          "        cdd_sdk_declare_dep(parson \"parson\" "
                          "\"https://github.com/SamuelMarks/parson.git\")\n")))
    return CDD_C_ERROR_IO;
  if (CHECK_IO_RC(fprintf(fp, "        FetchContent_MakeAvailable(parson)\n")))
    return CDD_C_ERROR_IO;
  if (CHECK_IO_RC(fprintf(
          fp,
          "        include_directories(SYSTEM \"${parson_SOURCE_DIR}\")\n")))
    return CDD_C_ERROR_IO;
  if (CHECK_IO_RC(fprintf(fp, "    endif()\n")))
    return CDD_C_ERROR_IO;
  if (CHECK_IO_RC(fprintf(fp, "endif()\n\n")))
    return CDD_C_ERROR_IO;
  /* c89stringutils */
  if (CHECK_IO_RC(fprintf(
          fp, "if(TARGET c89stringutils)\n    message(STATUS "
              "\"c89stringutils already provided by parent\")\nelse()\n")))
    return CDD_C_ERROR_IO;
  if (CHECK_IO_RC(fprintf(fp, "    set(c89stringutils_RESOLVED OFF)\n")))
    return CDD_C_ERROR_IO;
  if (CHECK_IO_RC(fprintf(fp, "    if(VCPKG_TOOLCHAIN)\n")))
    return CDD_C_ERROR_IO;
  if (CHECK_IO_RC(
          fprintf(fp, "        find_package(c89stringutils CONFIG QUIET)\n")))
    return CDD_C_ERROR_IO;
  if (CHECK_IO_RC(fprintf(fp, "        if(c89stringutils_FOUND)\n")))
    return CDD_C_ERROR_IO;
  if (CHECK_IO_RC(fprintf(fp, "            set(c89stringutils_RESOLVED ON)\n")))
    return CDD_C_ERROR_IO;
  if (CHECK_IO_RC(fprintf(fp, "        endif()\n")))
    return CDD_C_ERROR_IO;
  if (CHECK_IO_RC(fprintf(fp, "    endif()\n")))
    return CDD_C_ERROR_IO;
  if (CHECK_IO_RC(fprintf(fp, "    if(NOT c89stringutils_RESOLVED)\n")))
    return CDD_C_ERROR_IO;
  if (CHECK_IO_RC(fprintf(
          fp, "        cdd_sdk_declare_dep(c89stringutils \"c89stringutils\" "
              "\"https://github.com/offscale/c89stringutils.git\")\n")))
    return CDD_C_ERROR_IO;
  if (CHECK_IO_RC(
          fprintf(fp, "        FetchContent_MakeAvailable(c89stringutils)\n")))
    return CDD_C_ERROR_IO;
  if (CHECK_IO_RC(
          fprintf(fp, "        include_directories(SYSTEM "
                      "\"${c89stringutils_SOURCE_DIR}/c89stringutils\")\n")))
    return CDD_C_ERROR_IO;
  if (CHECK_IO_RC(fprintf(fp, "    endif()\n")))
    return CDD_C_ERROR_IO;
  if (CHECK_IO_RC(fprintf(fp, "endif()\n\n")))
    return CDD_C_ERROR_IO;
  /* c_str_span */
  if (CHECK_IO_RC(
          fprintf(fp, "if(TARGET c_str_span)\n    message(STATUS \"c_str_span "
                      "already provided by parent\")\nelse()\n")))
    return CDD_C_ERROR_IO;
  if (CHECK_IO_RC(fprintf(fp, "    set(c_str_span_RESOLVED OFF)\n")))
    return CDD_C_ERROR_IO;
  if (CHECK_IO_RC(fprintf(fp, "    if(VCPKG_TOOLCHAIN)\n")))
    return CDD_C_ERROR_IO;
  if (CHECK_IO_RC(
          fprintf(fp, "        find_package(c_str_span CONFIG QUIET)\n")))
    return CDD_C_ERROR_IO;
  if (CHECK_IO_RC(fprintf(fp, "        if(c_str_span_FOUND)\n")))
    return CDD_C_ERROR_IO;
  if (CHECK_IO_RC(fprintf(fp, "            set(c_str_span_RESOLVED ON)\n")))
    return CDD_C_ERROR_IO;
  if (CHECK_IO_RC(fprintf(fp, "        endif()\n")))
    return CDD_C_ERROR_IO;
  if (CHECK_IO_RC(fprintf(fp, "    endif()\n")))
    return CDD_C_ERROR_IO;
  if (CHECK_IO_RC(fprintf(fp, "    if(NOT c_str_span_RESOLVED)\n")))
    return CDD_C_ERROR_IO;
  if (CHECK_IO_RC(
          fprintf(fp, "        cdd_sdk_declare_dep(c_str_span \"c-str-span\" "
                      "\"https://github.com/SamuelMarks/c-str-span.git\")\n")))
    return CDD_C_ERROR_IO;
  if (CHECK_IO_RC(
          fprintf(fp, "        FetchContent_MakeAvailable(c_str_span)\n")))
    return CDD_C_ERROR_IO;
  if (CHECK_IO_RC(fprintf(fp, "        include_directories(SYSTEM "
                              "\"${c_str_span_SOURCE_DIR}\")\n")))
    return CDD_C_ERROR_IO;
  if (CHECK_IO_RC(fprintf(fp, "    endif()\n")))
    return CDD_C_ERROR_IO;
  if (CHECK_IO_RC(fprintf(fp, "endif()\n\n")))
    return CDD_C_ERROR_IO;
  /* cfs */
  if (CHECK_IO_RC(fprintf(fp,
                          "if(TARGET cfs)\n    message(STATUS \"cfs already "
                          "provided by parent\")\nelse()\n")))
    return CDD_C_ERROR_IO;
  if (CHECK_IO_RC(fprintf(fp, "    set(cfs_RESOLVED OFF)\n")))
    return CDD_C_ERROR_IO;
  if (CHECK_IO_RC(fprintf(fp, "    if(VCPKG_TOOLCHAIN)\n")))
    return CDD_C_ERROR_IO;
  if (CHECK_IO_RC(fprintf(fp, "        find_package(cfs CONFIG QUIET)\n")))
    return CDD_C_ERROR_IO;
  if (CHECK_IO_RC(fprintf(fp, "        if(cfs_FOUND)\n")))
    return CDD_C_ERROR_IO;
  if (CHECK_IO_RC(fprintf(fp, "            set(cfs_RESOLVED ON)\n")))
    return CDD_C_ERROR_IO;
  if (CHECK_IO_RC(fprintf(fp, "        endif()\n")))
    return CDD_C_ERROR_IO;
  if (CHECK_IO_RC(fprintf(fp, "    endif()\n")))
    return CDD_C_ERROR_IO;
  if (CHECK_IO_RC(fprintf(fp, "    if(NOT cfs_RESOLVED)\n")))
    return CDD_C_ERROR_IO;
  if (CHECK_IO_RC(fprintf(fp,
                          "        cdd_sdk_declare_dep(cfs \"c-fs\" "
                          "\"https://github.com/SamuelMarks/c-fs.git\")\n")))
    return CDD_C_ERROR_IO;
  if (CHECK_IO_RC(fprintf(fp, "        FetchContent_MakeAvailable(cfs)\n")))
    return CDD_C_ERROR_IO;
  if (CHECK_IO_RC(fprintf(fp, "        include_directories(SYSTEM "
                              "\"${cfs_SOURCE_DIR}/include\")\n")))
    return CDD_C_ERROR_IO;
  if (CHECK_IO_RC(fprintf(fp, "    endif()\n")))
    return CDD_C_ERROR_IO;
  if (CHECK_IO_RC(fprintf(fp, "endif()\n\n")))
    return CDD_C_ERROR_IO;
  /* c-abstract-http */
  if (CHECK_IO_RC(fprintf(
          fp, "if(TARGET c-abstract-http)\n    message(STATUS "
              "\"c-abstract-http already provided by parent\")\nelse()\n")))
    return CDD_C_ERROR_IO;
  if (CHECK_IO_RC(fprintf(fp, "    set(c_abstract_http_RESOLVED OFF)\n")))
    return CDD_C_ERROR_IO;
  if (CHECK_IO_RC(fprintf(fp, "    if(VCPKG_TOOLCHAIN)\n")))
    return CDD_C_ERROR_IO;
  if (CHECK_IO_RC(
          fprintf(fp, "        find_package(c-abstract-http CONFIG QUIET)\n")))
    return CDD_C_ERROR_IO;
  if (CHECK_IO_RC(fprintf(fp, "        if(c-abstract-http_FOUND)\n")))
    return CDD_C_ERROR_IO;
  if (CHECK_IO_RC(
          fprintf(fp, "            set(c_abstract_http_RESOLVED ON)\n")))
    return CDD_C_ERROR_IO;
  if (CHECK_IO_RC(fprintf(fp, "        endif()\n")))
    return CDD_C_ERROR_IO;
  if (CHECK_IO_RC(fprintf(fp, "    endif()\n")))
    return CDD_C_ERROR_IO;
  if (CHECK_IO_RC(fprintf(fp, "    if(NOT c_abstract_http_RESOLVED)\n")))
    return CDD_C_ERROR_IO;
  if (CHECK_IO_RC(fprintf(
          fp, "        cdd_sdk_declare_dep(c-abstract-http \"c-abstract-http\" "
              "\"https://github.com/SamuelMarks/c-abstract-http.git\")\n")))
    return CDD_C_ERROR_IO;
  if (CHECK_IO_RC(
          fprintf(fp, "        FetchContent_MakeAvailable(c-abstract-http)\n")))
    return CDD_C_ERROR_IO;
  if (CHECK_IO_RC(fprintf(fp, "        include_directories(SYSTEM "
                              "\"${c-abstract-http_SOURCE_DIR}/include\")\n")))
    return CDD_C_ERROR_IO;
  if (CHECK_IO_RC(fprintf(fp, "    endif()\n")))
    return CDD_C_ERROR_IO;
  if (CHECK_IO_RC(fprintf(fp, "endif()\n\n")))
    return CDD_C_ERROR_IO;

  if (CHECK_IO_RC(fprintf(
          fp, "if(APPLE AND TARGET c-abstract-http)\n"
              "    get_target_property(_cah_opts c-abstract-http "
              "COMPILE_OPTIONS)\n"
              "    if(_cah_opts)\n"
              "        list(REMOVE_ITEM _cah_opts \"-Werror\")\n"
              "        set_target_properties(c-abstract-http PROPERTIES "
              "COMPILE_OPTIONS \"${_cah_opts}\")\n"
              "    endif()\n"
              "endif()\n"
              "set(BUILD_TESTING \"${_CDD_OLD_BUILD_TESTING}\")\n\n")))
    return CDD_C_ERROR_IO;

  if (CHECK_IO_RC(
          fprintf(fp,
                  "target_link_libraries(%s PUBLIC c-abstract-http parson "
                  "c89stringutils)\n\n",
                  project_name)))
    return CDD_C_ERROR_IO;

  /* Include Directories */
  if (CHECK_IO_RC(
          fprintf(fp, "target_include_directories(%s PUBLIC\n", project_name)))
    return CDD_C_ERROR_IO;
  if (CHECK_IO_RC(
          fprintf(fp, "    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>\n")))
    return CDD_C_ERROR_IO;
  if (CHECK_IO_RC(
          fprintf(fp, "    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>\n")))
    return CDD_C_ERROR_IO;
  if (CHECK_IO_RC(fprintf(fp, "    $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>\n")))
    return CDD_C_ERROR_IO;
  if (CHECK_IO_RC(fprintf(fp, "    $<INSTALL_INTERFACE:include>\n")))
    return CDD_C_ERROR_IO;
  if (CHECK_IO_RC(fprintf(fp, ")\n\n")))
    return CDD_C_ERROR_IO;

  /* Tests */
  if (has_tests) {
    if (CHECK_IO_RC(fprintf(fp, "include(CTest)\n")))
      return CDD_C_ERROR_IO;
    if (CHECK_IO_RC(fprintf(fp, "enable_testing()\n")))
      return CDD_C_ERROR_IO;
    if (CHECK_IO_RC(fprintf(fp, "if (BUILD_TESTING)\n")))
      return CDD_C_ERROR_IO;

    if (CHECK_IO_RC(fprintf(fp, "    if(VCPKG_TOOLCHAIN)\n")))
      return CDD_C_ERROR_IO;
    if (CHECK_IO_RC(
            fprintf(fp, "        find_package(greatest CONFIG REQUIRED)\n")))
      return CDD_C_ERROR_IO;
    if (CHECK_IO_RC(fprintf(fp, "    else()\n")))
      return CDD_C_ERROR_IO;
    if (CHECK_IO_RC(fprintf(
            fp, "        set(_GREATEST_H \"\")\n"
                "        get_filename_component(_CUR_REAL "
                "\"${CMAKE_CURRENT_SOURCE_DIR}\" REALPATH)\n"
                "        foreach(_rel \"../..\" \"../../..\" \"../../../..\" "
                "\"../../../../..\")\n"
                "            if(EXISTS "
                "\"${_CUR_REAL}/${_rel}/greatest/greatest.h\")\n"
                "                get_filename_component(_GREATEST_H "
                "\"${_CUR_REAL}/${_rel}/greatest/greatest.h\" "
                "REALPATH)\n"
                "                break()\n"
                "            endif()\n"
                "        endforeach()\n")))
      return CDD_C_ERROR_IO;
    if (CHECK_IO_RC(fprintf(
            fp,
            "        if(_GREATEST_H)\n"
            "            file(COPY \"${_GREATEST_H}\" DESTINATION "
            "\"${CMAKE_CURRENT_BINARY_DIR}\")\n"
            "        else()\n"
            "            file(DOWNLOAD "
            "https://raw.githubusercontent.com/silentbicycle/greatest/master/"
            "greatest.h \"${CMAKE_CURRENT_BINARY_DIR}/greatest.h\")\n"
            "        endif()\n")))
      return CDD_C_ERROR_IO;
    if (CHECK_IO_RC(fprintf(fp, "    endif()\n\n")))
      return CDD_C_ERROR_IO;

    if (CHECK_IO_RC(
            fprintf(fp, "    file(GLOB_RECURSE TEST_SOURCES \"test/*.c\")\n")))
      return CDD_C_ERROR_IO;
    if (CHECK_IO_RC(
            fprintf(fp, "    file(GLOB_RECURSE TEST_HEADERS \"test/*.h\")\n")))
      return CDD_C_ERROR_IO;
    if (CHECK_IO_RC(fprintf(
            fp, "    add_executable(test_%s ${TEST_SOURCES} ${TEST_HEADERS})\n",
            project_name)))
      return CDD_C_ERROR_IO;
    if (CHECK_IO_RC(fprintf(fp,
                            "    target_link_libraries(test_%s PRIVATE %s)\n",
                            project_name, project_name)))
      return CDD_C_ERROR_IO;
    if (CHECK_IO_RC(fprintf(fp,
                            "    target_include_directories(test_%s PRIVATE "
                            "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}> "
                            "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)\n",
                            project_name)))
      return CDD_C_ERROR_IO;
    if (CHECK_IO_RC(fprintf(fp, "    add_test(NAME test_%s COMMAND test_%s)\n",
                            project_name, project_name)))
      return CDD_C_ERROR_IO;
    if (CHECK_IO_RC(fprintf(fp, "endif ()\n")))
      return CDD_C_ERROR_IO;
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Generates cmake project.
 *
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t generate_cmake_project(const char *output_path,
                                     const char *project_name, int has_tests) {
  FILE *fp = NULL;
  const char *filename = "CMakeLists.txt";
  char *full_path = NULL;
  char *src_dir = NULL;
  char *src_cmake = NULL;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  if (!project_name)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  /* Handle optional path construction */
  if (output_path) {
    size_t len;
    rc = makedirs(output_path);
    if (rc != CDD_C_SUCCESS)
      return rc;

    len = strlen(output_path) + strlen(filename) + 2;
    /* Fixed C99 warning */
    full_path = C_CDD_MALLOC(len);
    if (!full_path) {
      return CDD_C_ERROR_MEMORY;
    }
    CDD_SNPRINTF(full_path, len, "%s/%s", output_path, filename);
  } else {
    /* Fixed C99 warning */
    if (c_cdd_strdup(filename, &full_path) != CDD_C_SUCCESS) {
      return CDD_C_ERROR_MEMORY;
    }
  }

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  if (fopen_s(&fp, full_path, "w") != 0)
    fp = NULL;
#else
#if defined(_MSC_VER)
  if (fopen_s(&fp, full_path, "w") != 0)
    fp = NULL;
#else
  fp = fopen(full_path, "w");
#endif
#endif

  if (!fp) {
    C_CDD_FREE(full_path);
    return CDD_C_ERROR_IO;
  }
  C_CDD_FREE(full_path);
  full_path = NULL;

  /* Write Root CMakeLists.txt */
  if (CHECK_IO_RC(fprintf(fp, "cmake_minimum_required(VERSION 3.10)\n\n"))) {
    fclose(fp);
    return CDD_C_ERROR_IO;
  }
  if (CHECK_IO_RC(fprintf(fp, "project(%s C)\n\n", project_name))) {
    fclose(fp);
    return CDD_C_ERROR_IO;
  }
  if (CHECK_IO_RC(fprintf(fp, "include(CTest)\n"))) {
    fclose(fp);
    return CDD_C_ERROR_IO;
  }
  if (has_tests) {
    if (CHECK_IO_RC(fprintf(fp, "enable_testing()\n\n"))) {
      fclose(fp);
      return CDD_C_ERROR_IO;
    }
  } else {
    if (CHECK_IO_RC(fprintf(fp, "if(BUILD_TESTING)\n"))) {
      fclose(fp);
      return CDD_C_ERROR_IO;
    }
    if (CHECK_IO_RC(fprintf(fp, "  enable_testing()\n"))) {
      fclose(fp);
      return CDD_C_ERROR_IO;
    }
    if (CHECK_IO_RC(fprintf(fp, "endif()\n\n"))) {
      fclose(fp);
      return CDD_C_ERROR_IO;
    }
  }
  if (CHECK_IO_RC(fprintf(fp, "add_subdirectory(src)\n"))) {
    fclose(fp);
    return CDD_C_ERROR_IO;
  }
  if (has_tests) {
    if (CHECK_IO_RC(fprintf(
            fp,
            "if(TARGET test_%s)\n  add_test(NAME test_%s COMMAND test_%s)\n"
            "endif()\n",
            project_name, project_name, project_name))) {
      fclose(fp);
      return CDD_C_ERROR_IO;
    }
  }
  fclose(fp);

  /* Now write src/CMakeLists.txt */
  if (output_path) {
    src_dir = C_CDD_MALLOC(strlen(output_path) + 5);
    if (!src_dir) {
      rc = CDD_C_ERROR_MEMORY;
      goto cleanup_src;
    }
    CDD_SNPRINTF(src_dir, strlen(output_path) + 5, "%s/src", output_path);
    rc = makedirs(src_dir);
    if (rc != CDD_C_SUCCESS) {
      goto cleanup_src;
    }
    src_cmake = C_CDD_MALLOC(strlen(src_dir) + strlen(filename) + 2);
    if (!src_cmake) {
      rc = CDD_C_ERROR_MEMORY;
      goto cleanup_src;
    }
    CDD_SNPRINTF(src_cmake, strlen(src_dir) + strlen(filename) + 2, "%s/%s",
                 src_dir, filename);
  } else {
    if (c_cdd_strdup("src", &src_dir) != CDD_C_SUCCESS) {
      rc = CDD_C_ERROR_MEMORY;
      goto cleanup_src;
    }
    rc = makedirs(src_dir);
    if (rc != CDD_C_SUCCESS) {
      goto cleanup_src;
    }
    src_cmake = C_CDD_MALLOC(strlen(src_dir) + strlen(filename) + 2);
    if (!src_cmake) {
      rc = CDD_C_ERROR_MEMORY;
      goto cleanup_src;
    }
    CDD_SNPRINTF(src_cmake, strlen(src_dir) + strlen(filename) + 2, "%s/%s",
                 src_dir, filename);
  }

#if defined(_MSC_VER)
  if (fopen_s(&fp, src_cmake, "w") != 0)
    fp = NULL;
#else
  fp = fopen(src_cmake, "w");
#endif

  if (fp) {
    rc = write_cmake_content(fp, project_name, has_tests);
    fclose(fp);
  } else {
    rc = CDD_C_ERROR_IO;
  }
  C_CDD_FREE(src_dir);
  src_dir = NULL;
  C_CDD_FREE(src_cmake);
  src_cmake = NULL;
  fp = NULL;

  return rc;
cleanup_src:
  if (src_dir)
    C_CDD_FREE(src_dir);
  return rc;
}

/**
 * @brief Generates build system main.
 *
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t generate_build_system_main(int argc, char **argv) {
  const char *sys_type;
  const char *out_dir;
  const char *name;
  int has_tests = 0;

  if (argc < 3) {
    fprintf(stderr,
            "Usage: generate_build_system <type> <out_dir> <name> [test]\n");
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  sys_type = argv[0];
  out_dir = argv[1];
  name = argv[2];

  if (argc > 3)
    has_tests = 1;

  if (strcmp(sys_type, "cmake") == 0) {
    cdd_c_error_t rc = generate_cmake_project(out_dir, name, has_tests);
    if (rc != CDD_C_SUCCESS) {
      fprintf(stderr, "Failed to generate CMakeLists.txt (error %d)\n", rc);
      return CDD_C_ERROR_IO;
    }
  } else {
    fprintf(stderr, "Unsupported build system type: %s\n", sys_type);
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  return CDD_C_SUCCESS;
}

#if defined(__clang__) || defined(__GNUC__)
#endif
