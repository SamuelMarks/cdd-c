/**
 * @file sync_code.h
 * @brief Logic for synchronizing C implementation files with header
 * declarations.
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

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !SYNC_CODE_H */
