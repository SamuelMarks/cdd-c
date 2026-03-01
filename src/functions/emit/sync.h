/**
 * @file sync_code.h
 * @brief Logic for synchronizing C implementation files with header
 * declarations, and patching headers to match refactored implementations.
 * @author Samuel Marks
 */

#ifndef SYNC_CODE_H
#define SYNC_CODE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <c_cdd_export.h>

/**
 * @brief Entry point for sync_code command.
 * Parses the provided header file and updates/generates implementation
 * functions in the provided source file.
 *
 * @param[in] argc Argument count.
 * @param[in] argv Argument vector. argv[0] is header path, argv[1] is impl
 * source path.
 * @return 0 on success, non-zero (errno/EXIT_FAILURE) on failure.
 */
extern C_CDD_EXPORT int sync_code_main(int argc, char **argv);

/**
 * @brief Patch a header file to match refactored function signatures from the
 * implementation.
 *
 * Reads the refactored source code string, extracts function signatures,
 * and updates corresponding prototypes in the header file on disk.
 *
 * @param[in] header_path Path to the header file to patch.
 * @param[in] refactored_source The full content of the refactored .c file.
 * @return 0 on success, error code on failure.
 */
extern C_CDD_EXPORT int patch_header_from_source(const char *header_path,
                                                 const char *refactored_source);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !SYNC_CODE_H */
