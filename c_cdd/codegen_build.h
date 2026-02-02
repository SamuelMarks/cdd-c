/**
 * @file codegen_build.h
 * @brief Abstract interface for Build System Generation.
 *
 * Defines the contract for generating compilation scaffolding (Makefiles,
 * CMakeLists, Meson files) for the generated C SDK.
 *
 * Key responsibilities:
 * - Output configuration files for the requested build system.
 * - Inject platform-specific networking dependency logic.
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_CODEGEN_BUILD_H
#define C_CDD_CODEGEN_BUILD_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stddef.h>
#include <stdio.h>

#include "c_cdd_export.h"

/**
 * @brief Enumeration of supported build systems.
 */
enum CodegenBuildSystem {
  BUILD_SYS_CMAKE,    /**< CMake (CMakeLists.txt) */
  BUILD_SYS_MESON,    /**< Meson (meson.build) - Future Support */
  BUILD_SYS_MAKEFILE, /**< Raw Makefile - Future Support */
  BUILD_SYS_UNKNOWN   /**< Error sentinel */
};

/**
 * @brief Generic configuration for a build artifact.
 */
struct CodegenBuildConfig {
  const char *project_name; /**< Name of the project/package */
  const char *target_name;  /**< Name of the library target to compile */
  const char **src_files;   /**< Array of source filenames */
  size_t src_count;         /**< Number of source files */
  int build_shared_libs; /**< 1 to enable shared libs by default, 0 for static
                          */
};

/**
 * @brief Generate the build configuration file.
 *
 * Dispatches to the specific implementation based on the `type` argument.
 *
 * @param[in] type The desired build system format.
 * @param[in] fp The output file stream (e.g. opened "CMakeLists.txt").
 * @param[in] config The project configuration details.
 * @return 0 on success, ENOTSUP if type not supported, or EIO/EINVAL on
 * error.
 */
extern C_CDD_EXPORT int
codegen_build_generate(enum CodegenBuildSystem type, FILE *fp,
                       const struct CodegenBuildConfig *config);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_CODEGEN_BUILD_H */
