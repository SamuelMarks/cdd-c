/**
 * @file cli.h
 * @brief Command-line interface logic for C-to-OpenAPI generation.
 *
 * Entry point for the `c2openapi` command. Orchestrates the scanning of C
 * source files, extraction of Doxygen-style documentation, and generation of an
 * OpenAPI v3 specification file.
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_C2OPENAPI_CLI_H
#define C_CDD_C2OPENAPI_CLI_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
/* clang-format on */

/**
 * @brief Main entry point for the `c2openapi` command.
 *
 * Usage: `c2openapi [--base <openapi.json>] [--self <uri>] [--dialect <uri>]
 * <source_directory> <output_openapi.json>`
 *
 * Workflow:
 * 1. Initializes an empty OpenAPI Spec (or loads a base spec if provided).
 * 2. Recursively walks the `<source_directory>`.
 * 3. Identifies `.c` and `.h` files.
 * 4. Extracts structs/enums -> Components Registry.
 * 5. Extracts functions with `@route` and `@webhook` annotations -> Paths,
 *    Webhooks & Operations.
 * 6. Serializes the result to JSON and writes to `<output_openapi.json>`.
 *
 * @param[in] argc Argument count.
 * @param[in] argv Argument vector.
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on error.
 */
extern C_CDD_EXPORT /**
                     * @brief Executes the c2openapi cli main operation.
                     */
    int
    c2openapi_cli_main(int argc, char **argv);

/**
 * @brief Main entry point for the `to_docs_json` command.
 */
extern C_CDD_EXPORT /**
                     * @brief Executes the to docs json cli main operation.
                     */
    int
    to_docs_json_cli_main(int argc, char **argv);

/**
 * @brief Main entry point for `migrate` commands.
 */
extern C_CDD_EXPORT /**
                     * @brief Executes the migrate cli main operation.
                     */
    int
    migrate_cli_main(int argc, char **argv);

/**
 * @brief Main entry point for `db reset` command.
 */
extern C_CDD_EXPORT /**
                     * @brief Executes the db reset cli main operation.
                     */
    int
    db_reset_cli_main(int argc, char **argv);

/**
 * @brief Main entry point for `sql2c` command.
 */
extern C_CDD_EXPORT /**
                     * @brief Executes the sql2c main operation.
                     */
    int
    sql2c_main(int argc, char **argv);
extern C_CDD_EXPORT /**
                     * @brief Executes the schema dump cli main operation.
                     */
    int
    schema_dump_cli_main(int argc, char **argv);

/**
 * @brief Main entry point for `seed` command.
 */
extern C_CDD_EXPORT /**
                     * @brief Executes the seed cli main operation.
                     */
    int
    seed_cli_main(int argc, char **argv);

/**
 * @brief Main entry point for `setup_test_db` command.
 */
extern C_CDD_EXPORT /**
                     * @brief Executes the setup test db cli main operation.
                     */
    int
    setup_test_db_cli_main(int argc, char **argv);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_C2OPENAPI_CLI_H */
