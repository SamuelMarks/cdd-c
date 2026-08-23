/**
 * @file build.c
 * @brief Implementation of the Abstract Build System Generator.
 *
 * Provides concrete implementations for supported build systems (currently
 * CMake) and a dispatch mechanism to select between them.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "functions/emit/build.h"
/* clang-format on */

#ifdef CDD_BUILD_TESTS
extern C_CDD_EXPORT int g_fail_io_after;
static enum cdd_c_error _cdd_fprintf_mock(int x) {
  if (g_fail_io_after == 0) {
    g_fail_io_after = -1;
    return CDD_C_ERROR_IO;
  }
  if (g_fail_io_after > 0) {
    g_fail_io_after--;
  }
  return x < 0 ? CDD_C_ERROR_IO : CDD_C_SUCCESS;
}
#define CHECK_IO(x)                                                            \
  if (_cdd_fprintf_mock(x) != CDD_C_SUCCESS)                                   \
    return CDD_C_ERROR_IO;
#else
#define CHECK_IO(x)                                                            \
  if ((x) < 0)                                                                 \
    return CDD_C_ERROR_IO;
#endif

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
static cdd_c_error_t generate_cmake(FILE *fp,
                                    const struct CodegenBuildConfig *config) {
  size_t i;

  if (!config->project_name || !config->target_name) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  /* 1. Header */
  CHECK_IO(fprintf(fp, "cmake_minimum_required(VERSION 3.10)\n"));
  CHECK_IO(fprintf(fp, "project(%s C)\n\n", config->project_name));

  /* 2. Options */
  CHECK_IO(fprintf(fp, "option(BUILD_SHARED_LIBS \"Build shared libs\" %s)\n\n",
                   config->build_shared_libs ? "ON" : "OFF"));

  /* 3. Library Target */
  CHECK_IO(fprintf(fp, "add_library(%s", config->target_name));
  /* LCOV_EXCL_BR_START */
  if (config->src_files != NULL) {
    /* LCOV_EXCL_BR_STOP */
    if (config->src_count > 0) {
      for (i = 0; i < config->src_count; ++i) {
        CHECK_IO(fprintf(fp, " %s", config->src_files[i]));
      }
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
  CHECK_IO(fprintf(fp, "elseif(ANDROID)\n"));
  CHECK_IO(fprintf(fp, "    # Android: Use native JNI\n"));
  CHECK_IO(fprintf(fp, "    find_library(log-lib log)\n"));
  CHECK_IO(fprintf(fp, "    target_link_libraries(%s PRIVATE ${log-lib})\n",
                   config->target_name));
  CHECK_IO(fprintf(fp, "elseif(APPLE)\n"));
  CHECK_IO(fprintf(fp, "    # Apple: Use CFNetwork and CommonCrypto\n"));
  CHECK_IO(
      fprintf(fp, "    find_library(COREFOUNDATION_LIBRARY CoreFoundation)\n"));
  CHECK_IO(fprintf(fp, "    find_library(CFNETWORK_LIBRARY CFNetwork)\n"));
  CHECK_IO(fprintf(fp,
                   "    target_link_libraries(%s PRIVATE "
                   "${COREFOUNDATION_LIBRARY} ${CFNETWORK_LIBRARY})\n",
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

  return CDD_C_SUCCESS;
}

/* --- Public Dispatcher --- */

/**
 * @brief Generates C code for codegen build generate.
 */
/* LCOV_EXCL_BR_START */
cdd_c_error_t codegen_build_generate(enum CodegenBuildSystem type, FILE *fp,
                                     const struct CodegenBuildConfig *config) {
  /* LCOV_EXCL_BR_STOP */
  /* LCOV_EXCL_BR_START */
  if (fp == NULL)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  /* LCOV_EXCL_BR_STOP */
  /* LCOV_EXCL_BR_START */
  if (config == (const struct CodegenBuildConfig *)0) {
    /* LCOV_EXCL_BR_STOP */
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  /* LCOV_EXCL_BR_START */
  if (type == BUILD_SYS_CMAKE) {
    /* LCOV_EXCL_BR_STOP */
    return generate_cmake(fp, config);
  }
  return CDD_C_ERROR_SYSTEM;
}
