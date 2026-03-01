/**
 * @file codegen_make.h
 * @brief Generating build scripts (CMakeLists) for the SDK.
 * @author Samuel Marks
 */

#ifndef C_CDD_CODEGEN_MAKE_H
#define C_CDD_CODEGEN_MAKE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdio.h>

#include "c_cdd_export.h"

/**
 * @brief Configuration for the build file generator.
 */
struct MakeConfig {
  const char
      *project_name; /**< Name of the project/library (e.g. "petstore") */
  const char *min_cmake_version; /**< Minimum CMake version (e.g. "3.10") */

  /* Additional source files to include in the build */
  char **extra_sources;
  size_t extra_source_count;
};

/**
 * @brief Generate a CMakeLists.txt file.
 *
 * Emits a complete CMake build script that:
 * 1. Defines the project.
 * 2. Detects platform (Windows vs POSIX) to set conditional sources.
 * 3. Finds required packages (CURL, OpenSSL, Parson).
 * 4. Defines the library target.
 * 5. Configures installation/exports.
 *
 * @param[in] fp Output file stream.
 * @param[in] config Configuration for generation.
 * @return 0 on success, EINVAL if args invalid, EIO on write failure.
 */
extern C_CDD_EXPORT int codegen_make_generate(FILE *fp,
                                              const struct MakeConfig *config);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_CODEGEN_MAKE_H */
