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
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "functions/emit/build_system.h"
#include "functions/parse/fs.h"
#include "functions/parse/str.h"
#include "c_cdd/log.h"
/* clang-format on */

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#define strdup _strdup
#endif

/* Helper macro for I/O checking */
/** @brief CHECK_IO macro */
#define CHECK_IO(x)                                                            \
  do {                                                                         \
    if ((x) < 0)                                                               \
      return EIO;                                                              \
  } while (0)

static /**
        * @brief Generates C code for write cmake content.
        */
    int
    write_cmake_content(FILE *fp, const char *project_name, int has_tests) {

  /* Standard Settings */
  CHECK_IO(fprintf(fp, "set(CMAKE_C_STANDARD 90)\n"));
  CHECK_IO(fprintf(fp, "set(CMAKE_C_STANDARD_REQUIRED ON)\n"));
  CHECK_IO(fprintf(fp, "set(CMAKE_POSITION_INDEPENDENT_CODE ON)\n\n"));

  /* Source Globbing (Simplification for generated projects) */
  CHECK_IO(fprintf(fp, "file(GLOB SOURCES \"*.c\")\n"));
  CHECK_IO(fprintf(fp, "file(GLOB HEADERS \"*.h\")\n\n"));

  CHECK_IO(
      fprintf(fp, "list(FILTER SOURCES EXCLUDE REGEX \"test_.*\\\\.c$\")\n"));
  CHECK_IO(
      fprintf(fp, "list(FILTER HEADERS EXCLUDE REGEX \"test_.*\\\\.h$\")\n\n"));

  /* Target */
  CHECK_IO(
      fprintf(fp, "add_library(%s ${SOURCES} ${HEADERS})\n\n", project_name));

  CHECK_IO(fprintf(fp, "include(GenerateExportHeader)\n"));
  CHECK_IO(fprintf(fp,
                   "generate_export_header(%s EXPORT_FILE_NAME "
                   "${CMAKE_CURRENT_BINARY_DIR}/lib_export.h EXPORT_MACRO_NAME "
                   "LIB_EXPORT)\n\n",
                   project_name));

  /* Build Option: Shared/Static */
  CHECK_IO(fprintf(fp, "if (BUILD_SHARED_LIBS)\n"));
  CHECK_IO(fprintf(fp,
                   "    target_compile_definitions(%s PRIVATE "
                   "LIB_EXPORTS)\n",
                   project_name));
  CHECK_IO(fprintf(fp, "endif()\n\n"));

  /* Dependency Logic */
  CHECK_IO(fprintf(fp, "find_package(parson REQUIRED)\n"));
  CHECK_IO(fprintf(fp, "    target_link_libraries(%s PRIVATE parson)\n\n",
                   project_name));

  CHECK_IO(fprintf(fp, "find_package(c89stringutils CONFIG REQUIRED)\n"));
  CHECK_IO(fprintf(fp,
                   "target_link_libraries(%s PRIVATE c89stringutils "
                   "c89stringutils_compiler_flags)\n\n",
                   project_name));

  CHECK_IO(fprintf(fp, "find_package(c_str_span CONFIG REQUIRED)\n"));
  CHECK_IO(fprintf(fp,
                   "target_link_libraries(%s PRIVATE c_str_span "
                   "c_str_span_compiler_flags)\n\n",
                   project_name));

  /* Network and Crypto Backend Options */
  CHECK_IO(fprintf(fp, "option(C_ABSTRACT_HTTP_USE_CURL \"Use libcurl as the "
                       "HTTP backend\" OFF)\n"));
  CHECK_IO(fprintf(fp, "option(C_ABSTRACT_HTTP_USE_OPENSSL \"Use OpenSSL for "
                       "cryptography\" OFF)\n\n"));

  CHECK_IO(fprintf(fp, "include(FetchContent)\n"));
  CHECK_IO(fprintf(
      fp,
      "if(EXISTS "
      "\"${CMAKE_CURRENT_SOURCE_DIR}/../c-abstract-http/CMakeLists.txt\")\n"));
  CHECK_IO(fprintf(
      fp,
      "    add_subdirectory(\"${CMAKE_CURRENT_SOURCE_DIR}/../c-abstract-http\" "
      "\"${CMAKE_BINARY_DIR}/c-abstract-http\")\n"));
  CHECK_IO(fprintf(
      fp, "    include_directories(SYSTEM "
          "\"${CMAKE_CURRENT_SOURCE_DIR}/../c-abstract-http/include\")\n"));
  CHECK_IO(fprintf(fp, "elseif(VCPKG_TOOLCHAIN)\n"));
  CHECK_IO(fprintf(fp, "    find_package(c-abstract-http CONFIG REQUIRED)\n"));
  CHECK_IO(fprintf(fp, "else()\n"));
  CHECK_IO(fprintf(fp, "    FetchContent_Declare(\n"));
  CHECK_IO(fprintf(fp, "        c-abstract-http\n"));
  CHECK_IO(fprintf(fp, "        GIT_REPOSITORY "
                       "https://github.com/SamuelMarks/c-abstract-http.git\n"));
  CHECK_IO(fprintf(fp, "        GIT_TAG        master\n"));
  CHECK_IO(fprintf(fp, "    )\n"));
  CHECK_IO(fprintf(fp, "    FetchContent_MakeAvailable(c-abstract-http)\n"));
  CHECK_IO(fprintf(fp, "    include_directories(SYSTEM "
                       "${c-abstract-http_SOURCE_DIR}/include)\n"));
  CHECK_IO(fprintf(fp, "endif()\n\n"));

  CHECK_IO(fprintf(fp, "target_link_libraries(%s PRIVATE c-abstract-http)\n\n",
                   project_name));

  /* Include Directories */
  CHECK_IO(fprintf(fp, "target_include_directories(%s PUBLIC\n", project_name));
  CHECK_IO(fprintf(fp, "    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>\n"));
  CHECK_IO(fprintf(fp, "    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>\n"));
  CHECK_IO(fprintf(fp, "    $<INSTALL_INTERFACE:include>\n"));
  CHECK_IO(fprintf(fp, ")\n\n"));

  /* Tests */
  if (has_tests) {
    CHECK_IO(fprintf(fp, "include(CTest)\n"));
    CHECK_IO(fprintf(fp, "if (BUILD_TESTING)\n"));
    CHECK_IO(fprintf(fp, "    enable_testing()\n"));

    CHECK_IO(fprintf(fp, "    if(VCPKG_TOOLCHAIN)\n"));
    CHECK_IO(fprintf(fp, "        find_package(greatest CONFIG REQUIRED)\n"));
    CHECK_IO(fprintf(fp, "    else()\n"));
    CHECK_IO(fprintf(fp, "        FetchContent_Declare(\n"));
    CHECK_IO(fprintf(fp, "            greatest\n"));
    CHECK_IO(fprintf(fp, "            GIT_REPOSITORY "
                         "https://github.com/SamuelMarks/greatest.git\n"));
    CHECK_IO(fprintf(fp, "            GIT_TAG        master\n"));
    CHECK_IO(fprintf(fp, "        )\n"));
    CHECK_IO(fprintf(fp, "        FetchContent_MakeAvailable(greatest)\n"));
    CHECK_IO(fprintf(fp, "    endif()\n\n"));

    CHECK_IO(
        fprintf(fp, "    file(GLOB_RECURSE TEST_SOURCES \"src/test/*.c\")\n"));
    CHECK_IO(
        fprintf(fp, "    file(GLOB_RECURSE TEST_HEADERS \"src/test/*.h\")\n"));
    CHECK_IO(fprintf(
        fp, "    add_executable(test_%s ${TEST_SOURCES} ${TEST_HEADERS})\n",
        project_name));
    CHECK_IO(fprintf(fp,
                     "    target_link_libraries(test_%s PRIVATE %s greatest)\n",
                     project_name, project_name));
    CHECK_IO(fprintf(fp,
                     "    target_include_directories(test_%s PRIVATE "
                     "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src> "
                     "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>)\n",
                     project_name));
    CHECK_IO(fprintf(fp, "    add_test(NAME test_%s COMMAND test_%s)\n",
                     project_name, project_name));
    CHECK_IO(fprintf(fp, "endif ()\n"));
  }

  return 0;
}

/**
 * @brief Generates cmake project.
 */
int generate_cmake_project(const char *output_path, const char *project_name,
                           int has_tests) {
  FILE *fp = NULL;
  const char *filename = "CMakeLists.txt";
  char *full_path = NULL;
  int rc = 0;

  if (!project_name)
    return EINVAL;

  /* Handle optional path construction */
  if (output_path) {
    size_t len;
    rc = makedirs(output_path);
    if (rc != 0)
      return rc;

    len = strlen(output_path) + strlen(filename) + 2;
    full_path = malloc(len);
    if (!full_path) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      return ENOMEM;
    }
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
    sprintf_s(full_path, len, "%s/%s", output_path, filename);
#else
    sprintf(full_path, "%s/%s", output_path, filename);
#endif
  } else {
    full_path = strdup(filename);
    if (!full_path) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      return ENOMEM;
    }
  }

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  if (fopen_s(&fp, full_path, "w") != 0)
    fp = NULL;
#else
#if defined(_MSC_VER)
  fopen_s(&fp, full_path, "w");
#else
#if defined(_MSC_VER)
  fopen_s(&fp, full_path, "w");
#else
#if defined(_MSC_VER)
  fopen_s(&fp, full_path, "w");
#else
#if defined(_MSC_VER)
  fopen_s(&fp, full_path, "w");
#else
  fp = fopen(full_path, "w");
#endif
#endif
#endif
#endif
#endif

  if (!fp) {
    rc = errno ? errno : EIO;
    free(full_path);
    return rc;
  }

  /* Write Root CMakeLists.txt */
  fprintf(fp, "cmake_minimum_required(VERSION 3.10)\n\n");
  fprintf(fp, "project(%s C)\n\n", project_name);
  fprintf(fp, "add_subdirectory(src)\n");
  fclose(fp);

  /* Now write src/CMakeLists.txt */
  {
    char *src_dir = NULL;
    char *src_cmake = NULL;
    if (output_path) {
      src_dir = malloc(strlen(output_path) + 5);
      sprintf(src_dir, "%s/src", output_path);
      makedirs(src_dir);
      src_cmake = malloc(strlen(src_dir) + strlen(filename) + 2);
      sprintf(src_cmake, "%s/%s", src_dir, filename);
    } else {
      src_dir = strdup("src");
      makedirs(src_dir);
      src_cmake = malloc(strlen(src_dir) + strlen(filename) + 2);
      sprintf(src_cmake, "%s/%s", src_dir, filename);
    }

    fp = fopen(src_cmake, "w");
    if (fp) {
      rc = write_cmake_content(fp, project_name, has_tests);
      fclose(fp);
    } else {
      rc = errno ? errno : EIO;
    }
    free(src_dir);
    free(src_cmake);
  }
  fp = NULL;

  fclose(fp);
  free(full_path);
  return rc;
}

/**
 * @brief Generates build system main.
 */
int generate_build_system_main(int argc, char **argv) {
  const char *sys_type;
  const char *out_dir;
  const char *name;
  int has_tests = 0;

  if (argc < 3) {
    fprintf(stderr,
            "Usage: generate_build_system <type> <out_dir> <name> [test]\n");
    return EXIT_FAILURE;
  }

  sys_type = argv[0];
  out_dir = argv[1];
  name = argv[2];

  if (argc > 3)
    has_tests = 1;

  if (strcmp(sys_type, "cmake") == 0) {
    int rc = generate_cmake_project(out_dir, name, has_tests);
    if (rc != 0) {
      fprintf(stderr, "Failed to generate CMakeLists.txt (error %d)\n", rc);
      return EXIT_FAILURE;
    }
  } else {
    fprintf(stderr, "Unsupported build system type: %s\n", sys_type);
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
