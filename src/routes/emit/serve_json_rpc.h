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
#include "../../cdd_api.h"
/* clang-format on */

/**
 * @brief Handles an MCP stdio request.
 *
 * @param[in] body The JSON-RPC request body string.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t handle_stdio_request(const char *body);

/**
 * @brief Performs server json rpc operation from a typed config struct.
 *
 * @param[in] config Configuration struct.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t
serve_json_rpc_run(const cdd_serve_json_rpc_config_t *config);

/**
 * @brief CLI entry point for running the JSON-RPC 2.0 server over HTTP or
 * stdio.
 *
 * @param[in] argc Argument count.
 * @param[in] argv Argument vector.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t serve_json_rpc_main(int argc, char **argv);

/**
 * @brief Runs the Model Context Protocol (MCP) JSON-RPC loop over standard
 * input/output.
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
