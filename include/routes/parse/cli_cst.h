#ifndef ROUTES_PARSE_CLI_CST_H
#define ROUTES_PARSE_CLI_CST_H

/* clang-format off */
#include "c_cdd_export.h"
/* clang-format on */


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief Subcommand handler for cdd-c transformer.
 *
 * @param argc Argument count.
 * @param argv Argument values.
 * @return 0 on success.
 */
C_CDD_EXPORT int cli_cst_transformer_main(int argc, char **argv);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ROUTES_PARSE_CLI_CST_H */
