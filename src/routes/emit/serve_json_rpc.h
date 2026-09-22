#ifndef SERVER_JSON_RPC_H
#define SERVER_JSON_RPC_H
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @file serve_json_rpc.h
 * @brief Declarations for the JSON RPC and MCP server.
 */

/* clang-format off */
#include "c_cdd_export.h"
#include "cdd_c_error.h"
/* clang-format on */

/**
 * @brief Handles an MCP stdio request.
 *
 * @param[in] body The JSON-RPC request body string.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t handle_stdio_request(const char *body);

/**
 * @brief Executes the server json rpc main operation.
 *
 * @param[in] argc Argument count.
 * @param[in] argv Argument vector.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t serve_json_rpc_main(int argc, char **argv);

/**
 * @brief Executes the MCP stdio main operation.
 *
 * @param[in] argc Argument count.
 * @param[in] argv Argument vector.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t serve_mcp_stdio_main(int argc, char **argv);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
