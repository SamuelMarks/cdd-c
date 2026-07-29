/**
 * @file make.c
 * @brief Implementation of build system generation.
 * @author Samuel Marks
 */

/* clang-format off */
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "functions/emit/make.h"
#include <stdarg.h>
/* clang-format on */

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4127) /* conditional expression is constant */
#endif

/** @brief CHECK_IO definition */
#ifndef CHECK_IO
#define CHECK_IO(x)                                                            \
  do {                                                                         \
    if ((x) < 0)                                                               \
      return CDD_C_ERROR_IO;                                                   \
  } while (0)
#endif
enum cdd_c_error codegen_make_generate(FILE *fp,
                                       const struct MakeConfig *config) {
  size_t i;

  if (!fp || !config || !config->project_name)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  /* Header */
  CHECK_IO(
      fprintf(fp, "cmake_minimum_required(VERSION %s)\n",
              config->min_cmake_version ? config->min_cmake_version : "3.10"));
  CHECK_IO(fprintf(fp, "project(%s VERSION 0.0.2 LANGUAGES C)\n\n",
                   config->project_name));

  /* Core Sources */
  CHECK_IO(fprintf(fp, "set(SOURCES\n"));
  CHECK_IO(fprintf(fp, "    \"%s.c\"\n",
                   config->project_name)); /* Assumes lib matches proj */
  CHECK_IO(fprintf(fp, "    \"str_utils.c\"\n"));
  CHECK_IO(fprintf(fp, "    \"fs.c\"\n"));

  if (config->extra_sources) {
    for (i = 0; i < config->extra_source_count; ++i) {
      if (config->extra_sources[i]) {
        CHECK_IO(fprintf(fp, "    \"%s\"\n", config->extra_sources[i]));
      }
    }
  }
  CHECK_IO(fprintf(fp, ")\n\n"));

  /* Platform Logic */
  CHECK_IO(fprintf(
      fp, "if(EMSCRIPTEN OR CMAKE_SYSTEM_NAME STREQUAL \\\"DOS\\\")\n"));
  CHECK_IO(fprintf(fp, "    list(APPEND SOURCES \"crypto_standalone.c\")\n"));
  CHECK_IO(fprintf(fp, "elseif(WIN32)\n"));
  CHECK_IO(fprintf(fp, "    list(APPEND SOURCES \"crypto_wincrypt.c\")\n"));
  CHECK_IO(fprintf(fp, "    add_compile_definitions(USE_WINHTTP)\n"));
  CHECK_IO(fprintf(fp, "elseif(ANDROID)\n"));
  CHECK_IO(fprintf(fp, "    list(APPEND SOURCES \"crypto_openssl.c\")\n"));
  CHECK_IO(fprintf(fp, "elseif(APPLE)\n"));
  CHECK_IO(fprintf(fp, "    list(APPEND SOURCES \"crypto_apple.c\")\n"));
  CHECK_IO(fprintf(fp, "else()\n"));
  CHECK_IO(fprintf(fp, "    list(APPEND SOURCES \"crypto_openssl.c\")\n"));
  CHECK_IO(fprintf(fp, "    find_package(CURL REQUIRED)\n"));
  CHECK_IO(fprintf(fp, "    find_package(OpenSSL REQUIRED)\n"));
  CHECK_IO(fprintf(fp, "endif()\n"));

  /* Dependencies (Common) */
  CHECK_IO(fprintf(fp, "find_package(parson CONFIG REQUIRED)\n\n"));

  /* Target Definition */
  CHECK_IO(fprintf(fp, "add_library(%s ${SOURCES})\n", config->project_name));

  /* Link Libraries */
  CHECK_IO(fprintf(fp, "if(WIN32)\n"));
  CHECK_IO(fprintf(fp,
                   "    target_link_libraries(%s PRIVATE winhttp crypt32)\n",
                   config->project_name));
  CHECK_IO(fprintf(fp, "else()\n"));
  CHECK_IO(fprintf(fp,
                   "    target_link_libraries(%s PRIVATE CURL::libcurl "
                   "OpenSSL::SSL OpenSSL::Crypto)\n",
                   config->project_name));
  CHECK_IO(fprintf(fp, "endif()\n"));

  CHECK_IO(fprintf(fp, "target_link_libraries(%s PRIVATE parson)\n\n",
                   config->project_name));

  /* Install Rules */
  CHECK_IO(fprintf(fp, "include(GNUInstallDirs)\n"));
  CHECK_IO(fprintf(fp, "install(TARGETS %s EXPORT %sTargets\n",
                   config->project_name, config->project_name));
  CHECK_IO(
      fprintf(fp, "        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}\n"));
  CHECK_IO(
      fprintf(fp, "        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})\n"));

  return CDD_C_SUCCESS;
}
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
