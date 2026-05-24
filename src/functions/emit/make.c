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
/* clang-format on */

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4127) /* conditional expression is constant */
#endif

/** @brief CHECK_IO definition */
#ifdef CDD_BUILD_TESTS
extern int g_cdd_fprintf_fail;
static int check_io_helper_make(int rc) {
  if (g_cdd_fprintf_fail && --g_cdd_fprintf_fail == 0)
    return -1;
  return rc;
}
#define CHECK_IO(x)                                                            \
  do {                                                                         \
    if (check_io_helper_make(x) < 0)                                           \
      return EIO;                                                              \
  } while (0)
#else
#define CHECK_IO(x)                                                            \
  do {                                                                         \
    if ((x) < 0)                                                               \
      return EIO;                                                              \
  } while (0)
#endif

int codegen_make_generate(FILE *fp, const struct MakeConfig *config) {
  size_t i;

  if (!fp || !config || !config->project_name)
    return EINVAL;

  /* Header */
  CHECK_IO(
      fprintf(fp, "cmake_minimum_required(VERSION %s)\n",
              config->min_cmake_version ? config->min_cmake_version : "3.10"));
  CHECK_IO(fprintf(fp, "project(%s VERSION 0.0.1 LANGUAGES C)\n\n",
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
  CHECK_IO(fprintf(fp, "if(WIN32)\n"));
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
  CHECK_IO(fprintf(fp, "endif()\n\n"));

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

  return 0;
}
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
