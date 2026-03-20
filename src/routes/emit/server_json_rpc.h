#ifndef SERVER_JSON_RPC_H
#define SERVER_JSON_RPC_H
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief Generates the entry point for the JSON RPC server.
 */
/* clang-format off */
#include "c_cdd_export.h"
/* clang-format on */

extern C_CDD_EXPORT int server_json_rpc_main(int argc, char **argv);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
