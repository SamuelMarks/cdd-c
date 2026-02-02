/**
 * @file codegen_build.c
 * @brief Implementation of the Abstract Build System Generator.
 *
 * Provides concrete implementations for supported build systems (currently
 * CMake) and a dispatch mechanism to select between them.
 *
 * @author Samuel Marks
 */

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "codegen_build.h"

#define CHECK_IO(x)                                                            \
  do {                                                                         \
    if ((x) < 0)                                                               \
      return EIO;                                                              \
  } while (0)

/* --- CMake Implementation --- */

/**
 * @brief Generate a CMakeLists.txt file.
 *
 * Writes standard CMake directives to compile a C library. includes logic to
 * detect the target platform (WIN32) and link appropriate network libraries
 * (WinInet vs Libcurl).
 *
 * @param[in] fp Output stream.
 * @param[in] config Build configuration.
 * @return 0 on success, EIO/EINVAL on failure.
 */
static int generate_cmake(FILE *const fp,
                          const struct CodegenBuildConfig *const config) {
  size_t i;

  if (!fp || !config || !config->project_name || !config->target_name) {
    return EINVAL;
  }

  /* 1. Header */
  CHECK_IO(fprintf(fp, "cmake_minimum_required(VERSION 3.10)\n"));
  CHECK_IO(fprintf(fp, "project(%s C)\n\n", config->project_name));

  /* 2. Options */
  CHECK_IO(fprintf(fp, "option(BUILD_SHARED_LIBS \"Build shared libs\" %s)\n\n",
                   config->build_shared_libs ? "ON" : "OFF"));

  /* 3. Library Target */
  CHECK_IO(fprintf(fp, "add_library(%s", config->target_name));
  if (config->src_files && config->src_count > 0) {
    for (i = 0; i < config->src_count; ++i) {
      CHECK_IO(fprintf(fp, " %s", config->src_files[i]));
    }
  }
  CHECK_IO(fprintf(fp, ")\n\n"));

  /* 4. Include Directories */
  CHECK_IO(fprintf(fp, "target_include_directories(%s PUBLIC\n",
                   config->target_name));
  CHECK_IO(fprintf(
      fp, "    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>\n"
          "    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>\n)\n\n"));

  /* 5. Platform-specific Backend Selection */
  /* This logic is hardcoded for the C-CDD Network Abstraction Layer
   * requirements
   */
  CHECK_IO(fprintf(fp, "if(WIN32)\n"));
  CHECK_IO(fprintf(fp, "    # Windows: Use native WinInet\n"));
  CHECK_IO(fprintf(fp, "    target_link_libraries(%s PRIVATE wininet)\n",
                   config->target_name));
  CHECK_IO(fprintf(fp,
                   "    target_compile_definitions(%s PRIVATE USE_WININET)\n",
                   config->target_name));
  CHECK_IO(fprintf(fp, "else()\n"));
  CHECK_IO(fprintf(fp, "    # POSIX/Default: Use libcurl\n"));
  CHECK_IO(fprintf(fp, "    find_package(CURL REQUIRED)\n"));
  CHECK_IO(fprintf(fp, "    target_link_libraries(%s PRIVATE CURL::libcurl)\n",
                   config->target_name));
  CHECK_IO(fprintf(fp, "endif()\n\n"));

  /* 6. Standard Installation Rules */
  CHECK_IO(fprintf(fp, "include(GNUInstallDirs)\n"));
  CHECK_IO(fprintf(fp,
                   "install(TARGETS %s EXPORT %sTargets\n"
                   "    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}\n"
                   "    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}\n"
                   "    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}\n"
                   ")\n",
                   config->target_name, config->project_name));

  return 0;
}

/* --- Public Dispatcher --- */

int codegen_build_generate(enum CodegenBuildSystem type, FILE *const fp,
                           const struct CodegenBuildConfig *const config) {
  if (!fp || !config)
    return EINVAL;

  switch (type) {
  case BUILD_SYS_CMAKE:
    return generate_cmake(fp, config);
  case BUILD_SYS_MESON:    /* Future extension */
  case BUILD_SYS_MAKEFILE: /* Future extension */
  case BUILD_SYS_UNKNOWN:
  default:
    return ENOTSUP;
  }
}
