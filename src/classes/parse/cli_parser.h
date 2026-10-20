/**
 * @file cli_parser.h
 * @brief Parses C `getopt` loops into a CLI Intermediate Representation.
 */

#ifndef C_CDD_CLI_PARSER_H
#define C_CDD_CLI_PARSER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/* clang-format off */
#include "c_cdd_export.h"
#include "cdd_c_error.h"
#include "functions/parse/cst.h"
#include <stddef.h>
/* clang-format on */

/**
 * @brief Represents a single CLI option/flag.
 */
struct CliOption {
  char short_flag; /**< Single character flag (e.g. 'h'), 0 if none */
  char *long_flag; /**< Long string flag (e.g. "help"), NULL if none */
  int has_arg; /**< 0: no_argument, 1: required_argument, 2: optional_argument
                */
  char *description;         /**< Extracted help description */
  char *mapped_struct_field; /**< Extracted variable assignment target */
};

/**
 * @brief Represents a CLI Command (typically a `main` function).
 */
struct CliCommand {
  char *name;                /**< Name of the command */
  struct CliOption *options; /**< Array of detected options */
  size_t n_options;          /**< Number of options */
};

/**
 * @brief Initialize a CLI Command IR.
 *
 * @param[out] cmd Pointer to the CLI command structure to initialize.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
C_CDD_EXPORT cdd_c_error_t cli_command_init(struct CliCommand *cmd);

/**
 * @brief Free resources associated with a CLI Command IR.
 *
 * @param[in,out] cmd Pointer to the CLI command structure to free.
 */
C_CDD_EXPORT void cli_command_free(struct CliCommand *cmd);

/**
 * @brief Parses an AST/CST to extract CLI options and arguments.
 *
 * @param[in] nodes The root CST node (e.g. function body of main).
 * @param[in] tokens The token list.
 * @param[in,out] cmd The output command.
 * @return 0 on success, non-zero on failure.
 */
C_CDD_EXPORT cdd_c_error_t
cst_extract_cli_command(const struct CstNodeList *nodes,
                        const struct TokenList *tokens, struct CliCommand *cmd);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_CLI_PARSER_H */
