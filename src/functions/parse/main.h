/**
 * @file main.h
 * @brief Core main definitions and implementation.
 */
#ifndef C_CDD_MAIN_H
#define C_CDD_MAIN_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/* clang-format off */
#include "c_cdd_export.h"
#include "cdd_c_error.h"
/* clang-format on */
/**
 * @brief Main entry point function.
 *
 * @param[in] argc Argument count.
 * @param[in] argv Argument vector.
 * @return `CDD_C_SUCCESS` on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_main(int argc, char **argv);

/**
 * @brief CLI handler for `from_arazzo`.
 *
 * @param[in] argc Argument count.
 * @param[in] argv Argument vector.
 * @return `CDD_C_SUCCESS` on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t from_arazzo_cli_main(int argc, char **argv);

/**
 * @brief CLI handler for `to_arazzo`.
 *
 * @param[in] argc Argument count.
 * @param[in] argv Argument vector.
 * @return `CDD_C_SUCCESS` on success, or error code on failure.
 */
extern C_CDD_EXPORT cdd_c_error_t to_arazzo_cli_main(int argc, char **argv);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
