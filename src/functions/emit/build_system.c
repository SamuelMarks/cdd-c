/**
 * @file generate_build_system.c
 * @brief Implementation of build system scaffolding.
 *
 * writes CMakeLists.txt files with logic to selectively link against
 * system networking libraries based on the target platform.
 *
 * @author Samuel Marks
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "functions/emit/build_system.h"
#include "functions/parse/fs.h"
#include "functions/parse/str.h"

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#define strdup _strdup
#endif

/* Helper macro for I/O checking */
#define CHECK_IO(x)                                                            \
  do {                                                                         \
    if ((x) < 0)                                                               \
      return EIO;                                                              \
  } while (0)

static int write_cmake_content(FILE *fp, const char *project_name,
                               int has_tests) {
  CHECK_IO(fprintf(fp, "cmake_minimum_required(VERSION 3.10)\n\n"));
  CHECK_IO(fprintf(fp, "project(%s C)\n\n", project_name));

  /* Standard Settings */
  CHECK_IO(fprintf(fp, "set(CMAKE_C_STANDARD 90)\n"));
  CHECK_IO(fprintf(fp, "set(CMAKE_C_STANDARD_REQUIRED ON)\n"));
  CHECK_IO(fprintf(fp, "set(CMAKE_POSITION_INDEPENDENT_CODE ON)\n\n"));

  /* Source Globbing (Simplification for generated projects) */
  CHECK_IO(fprintf(fp, "file(GLOB_RECURSE SOURCES \"*.c\")\n"));
  CHECK_IO(fprintf(fp, "file(GLOB_RECURSE HEADERS \"*.h\")\n\n"));

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
  CHECK_IO(fprintf(fp, "if(TARGET parson::parson)\n"));
  CHECK_IO(fprintf(fp, "    target_link_libraries(%s PRIVATE parson::parson)\n",
                   project_name));
  CHECK_IO(fprintf(fp, "else()\n"));
  CHECK_IO(fprintf(fp, "    target_link_libraries(%s PRIVATE parson)\n",
                   project_name));
  CHECK_IO(fprintf(fp, "endif()\n\n"));

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
  CHECK_IO(fprintf(fp, "if (WIN32)\n"));
  CHECK_IO(fprintf(fp, "    # Windows: Link WinHTTP\n"));
  CHECK_IO(fprintf(fp, "    target_link_libraries(%s PRIVATE winhttp)\n",
                   project_name));
  CHECK_IO(fprintf(fp, "else ()\n"));
  CHECK_IO(fprintf(fp, "    # Unix/Linux: Link Curl\n"));
  CHECK_IO(fprintf(fp, "    find_package(CURL REQUIRED)\n"));
  CHECK_IO(fprintf(fp, "    target_link_libraries(%s PRIVATE CURL::libcurl)\n",
                   project_name));
  CHECK_IO(fprintf(fp, "endif ()\n\n"));

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
    CHECK_IO(fprintf(
        fp, "    file(DOWNLOAD "
            "https://raw.githubusercontent.com/silentbicycle/greatest/v1.5.0/"
            "greatest.h ${CMAKE_CURRENT_BINARY_DIR}/greatest.h)\n"));
    CHECK_IO(
        fprintf(fp, "    file(GLOB_RECURSE TEST_SOURCES \"src/test/*.c\")\n"));
    CHECK_IO(
        fprintf(fp, "    file(GLOB_RECURSE TEST_HEADERS \"src/test/*.h\")\n"));
    CHECK_IO(fprintf(
        fp, "    add_executable(test_%s ${TEST_SOURCES} ${TEST_HEADERS})\n",
        project_name));
    CHECK_IO(fprintf(fp, "    target_link_libraries(test_%s PRIVATE %s)\n",
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

int generate_cmake_project(const char *const output_path,
                           const char *const project_name, int has_tests) {
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
    if (!full_path)
      return ENOMEM;
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
    sprintf_s(full_path, len, "%s/%s", output_path, filename);
#else
    sprintf(full_path, "%s/%s", output_path, filename);
#endif
  } else {
    full_path = strdup(filename);
    if (!full_path)
      return ENOMEM;
  }

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  if (fopen_s(&fp, full_path, "w") != 0)
    fp = NULL;
#else
  fp = fopen(full_path, "w");
#endif

  if (!fp) {
    rc = errno ? errno : EIO;
    free(full_path);
    return rc;
  }

  rc = write_cmake_content(fp, project_name, has_tests);

  fclose(fp);
  free(full_path);
  return rc;
}

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
