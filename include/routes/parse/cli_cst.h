/**
 * @file cli_cst.h
 * @brief Core cli cst definitions and implementation.
 */
#ifndef ROUTES_PARSE_CLI_CST_H
#define ROUTES_PARSE_CLI_CST_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/* clang-format off */
#include "c_cdd_export.h"
#include "cdd_c_error.h"
/* clang-format on */

/**
 * @brief Subcommand handler for cdd-c transformer.
 *
 * @param[in] argc Argument count.
 * @param[out] argv Argument values.
 * @return 0 on success.
 */
C_CDD_EXPORT cdd_c_error_t cli_cst_transformer_main(int argc, char **argv);

/**
 * @brief Subcommand handler for standardize-gnu.
 *
 * @param[in] argc Argument count.
 * @param[out] argv Argument values.
 * @return 0 on success.
 */
C_CDD_EXPORT cdd_c_error_t cli_standardize_gnu_main(int argc, char **argv);

/**
 * @brief Subcommand handler for `cdd-c transform` multi-pass pipeline.
 *
 * Supports `--passes=...`, `--target-msvc=...`, `--vla-mode=...`,
 * `-o <out>`, `--out-dir <dir>`, `--stdin`, and `--stdout`.
 *
 * @param[in] argc Argument count.
 * @param[in] argv Argument values.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
C_CDD_EXPORT cdd_c_error_t cli_cst_transform_pipeline_main(int argc,
                                                           char **argv);

/**
 * @brief Standalone CLI dispatcher for `cdd-c-transform`
 * (`CDD_C_TRANSFORMER_ONLY`).
 *
 * Routes `transform`, `transformer`, `standardize-gnu`, `--help`, `--version`,
 * or direct `--passes=...` invocations without pulling OpenAPI/network/DB code.
 *
 * @param[in] argc Argument count (including `argv[0]` program name).
 * @param[in] argv Argument vector.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
C_CDD_EXPORT cdd_c_error_t cli_transform_dispatch_main(int argc, char **argv);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ROUTES_PARSE_CLI_CST_H */
