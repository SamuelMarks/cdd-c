/**
 * @file make.c
 * @brief Implementation of build system generation.
 * @author Samuel Marks
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "c_cdd/code_writer.h"
#include "functions/emit/make.h"
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/** @brief CHECK_IO definition */
#define CHECK_IO(x)                                                            \
  for (; (int)(x) < 0;)                                                        \
  return CDD_C_ERROR_IO

#undef FPRINTF_HOOK
#ifdef CDD_BUILD_TESTS
/** @brief FPRINTF_HOOK macro */
#define FPRINTF_HOOK cdd_io_fprintf_hook
#else
/** @brief FPRINTF_HOOK macro */
#define FPRINTF_HOOK fprintf
#endif

/**
 * @brief Emits a standalone `CMakeLists.txt` build configuration to `fp`.
 *
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t codegen_make_generate(FILE *fp, const struct MakeConfig *config) {
  size_t i;

  if (!fp || !config || !config->project_name)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  /* Header */
  CHECK_IO(FPRINTF_HOOK(fp, "cmake_minimum_required(VERSION %s)\n",
                        config->min_cmake_version ? config->min_cmake_version
                                                  : "3.10"));
  CHECK_IO(FPRINTF_HOOK(fp, "project(%s VERSION 0.0.4 LANGUAGES C)\n\n",
                        config->project_name));

  /* Core Sources */
  CHECK_IO(FPRINTF_HOOK(fp, "set(SOURCES\n"));
  CHECK_IO(FPRINTF_HOOK(fp, "    \"%s.c\"\n",
                        config->project_name)); /* Assumes lib matches proj */
  CHECK_IO(FPRINTF_HOOK(fp, "    \"str_utils.c\"\n"));
  CHECK_IO(FPRINTF_HOOK(fp, "    \"fs.c\"\n"));

  if (config->extra_sources) {
    for (i = 0; i < config->extra_source_count; ++i) {
      if (config->extra_sources[i]) {
        CHECK_IO(FPRINTF_HOOK(fp, "    \"%s\"\n", config->extra_sources[i]));
      }
    }
  }
  CHECK_IO(FPRINTF_HOOK(fp, ")\n\n"));

  /* Platform Logic */
  CHECK_IO(FPRINTF_HOOK(
      fp, "if(EMSCRIPTEN OR CMAKE_SYSTEM_NAME STREQUAL \\\"DOS\\\")\n"));
  CHECK_IO(
      FPRINTF_HOOK(fp, "    list(APPEND SOURCES \"crypto_standalone.c\")\n"));
  CHECK_IO(FPRINTF_HOOK(fp, "elseif(WIN32)\n"));
  CHECK_IO(
      FPRINTF_HOOK(fp, "    list(APPEND SOURCES \"crypto_wincrypt.c\")\n"));
  CHECK_IO(FPRINTF_HOOK(fp, "    add_compile_definitions(USE_WINHTTP)\n"));
  CHECK_IO(FPRINTF_HOOK(fp, "elseif(ANDROID)\n"));
  CHECK_IO(FPRINTF_HOOK(fp, "    list(APPEND SOURCES \"crypto_openssl.c\")\n"));
  CHECK_IO(FPRINTF_HOOK(fp, "elseif(APPLE)\n"));
  CHECK_IO(FPRINTF_HOOK(fp, "    list(APPEND SOURCES \"crypto_apple.c\")\n"));
  CHECK_IO(FPRINTF_HOOK(fp, "else()\n"));
  CHECK_IO(FPRINTF_HOOK(fp, "    list(APPEND SOURCES \"crypto_openssl.c\")\n"));
  CHECK_IO(FPRINTF_HOOK(fp, "    find_package(CURL REQUIRED)\n"));
  CHECK_IO(FPRINTF_HOOK(fp, "    find_package(OpenSSL REQUIRED)\n"));
  CHECK_IO(FPRINTF_HOOK(fp, "endif()\n"));

  /* Dependencies (Common) */
  CHECK_IO(FPRINTF_HOOK(fp, "find_package(parson CONFIG REQUIRED)\n\n"));

  /* Target Definition */
  CHECK_IO(
      FPRINTF_HOOK(fp, "add_library(%s ${SOURCES})\n", config->project_name));

  /* Link Libraries */
  CHECK_IO(FPRINTF_HOOK(fp, "if(WIN32)\n"));
  CHECK_IO(FPRINTF_HOOK(
      fp, "    target_link_libraries(%s PRIVATE winhttp crypt32)\n",
      config->project_name));
  CHECK_IO(FPRINTF_HOOK(fp, "else()\n"));
  CHECK_IO(FPRINTF_HOOK(fp,
                        "    target_link_libraries(%s PRIVATE CURL::libcurl "
                        "OpenSSL::SSL OpenSSL::Crypto)\n",
                        config->project_name));
  CHECK_IO(FPRINTF_HOOK(fp, "endif()\n"));

  CHECK_IO(FPRINTF_HOOK(fp, "target_link_libraries(%s PRIVATE parson)\n\n",
                        config->project_name));

  /* Install Rules */
  CHECK_IO(FPRINTF_HOOK(fp, "include(GNUInstallDirs)\n"));
  CHECK_IO(FPRINTF_HOOK(fp, "install(TARGETS %s EXPORT %sTargets\n",
                        config->project_name, config->project_name));
  CHECK_IO(FPRINTF_HOOK(
      fp, "        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}\n"));
  CHECK_IO(FPRINTF_HOOK(
      fp, "        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})\n"));

  return CDD_C_SUCCESS;
}
