/**
 * @file generate_build_system.h
 * @brief Utilities for scaffolding build configurations (CMake).
 *
 * Provides functionality to generate `CMakeLists.txt` files for generated
 * projects. Handles dependency logic for different platforms (linking
 * WinHTTP on Windows vs Curl on Unix).
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_GENERATE_BUILD_SYSTEM_H
#define C_CDD_GENERATE_BUILD_SYSTEM_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "c_cdd_export.h"

/**
 * @brief CLI Entry point for generate_build_system command.
 *
 * Usage: `generate_build_system <type> <out_dir> <name> [test_file]`
 *
 * @param[in] argc Argument count.
 * @param[in] argv Argument vector.
 * @return 0 on success, EXIT_FAILURE on error.
 */
extern C_CDD_EXPORT int generate_build_system_main(int argc, char **argv);

/**
 * @brief Generate a CMakeLists.txt file.
 *
 * @param[in] output_path Directory/Filename to write the file to.
 * @param[in] project_name Name of the project/library target.
 * @param[in] has_tests 1 if test scaffolding should be included.
 * @return 0 on success, error code (errno) on failure.
 */
extern C_CDD_EXPORT int generate_cmake_project(const char *output_path,
                                               const char *project_name,
                                               int has_tests);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_GENERATE_BUILD_SYSTEM_H */
