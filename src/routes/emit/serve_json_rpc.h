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
#include "c_cdd/json_dom.h"
#include "../../cdd_api.h"
/* clang-format on */

/**
 * @brief JSON-RPC 2.0 Request envelope.
 */
struct CddJsonRpcRequest {
  /** @brief Protocol version ("2.0") */
  const char *jsonrpc;
  /** @brief RPC method name */
  const char *method;
  /** @brief Optional RPC params object */
  const JSON_Object *params;
  /** @brief Optional RPC request ID */
  const JSON_Value *id_val;
};

/**
 * @brief JSON-RPC 2.0 Error object.
 */
typedef struct CddJsonRpcError {
  /** @brief JSON-RPC error code */
  int code;
  /** @brief JSON-RPC error message */
  const char *message;
} CddJsonRpcErrorObj;

/**
 * @brief JSON-RPC 2.0 Response envelope.
 */
struct CddJsonRpcResponse {
  /** @brief Protocol version ("2.0") */
  const char *jsonrpc;
  /** @brief Pre-serialized result JSON string (NULL when error is set) */
  const char *result_json;
  /** @brief Error payload (used when result_json is NULL) */
  struct CddJsonRpcError error;
  /** @brief Request ID JSON_Value (or NULL) */
  const JSON_Value *id_val;
};

/**
 * @brief MCP `tools/call` parameters struct.
 */
struct CddMcpToolCallParams {
  /** @brief Tool name */
  const char *name;
  /** @brief Tool arguments object */
  const JSON_Object *arguments;
};

/** @brief Runtime reflection descriptor for `CddJsonRpcRequest`. */
extern C_CDD_EXPORT const struct cdd_type_info CddJsonRpcRequest_type_info;
/** @brief Runtime reflection descriptor for `CddJsonRpcErrorObj`. */
extern C_CDD_EXPORT const struct cdd_type_info CddJsonRpcErrorObj_type_info;
/** @brief Runtime reflection descriptor for `CddJsonRpcResponse`. */
extern C_CDD_EXPORT const struct cdd_type_info CddJsonRpcResponse_type_info;
/** @brief Runtime reflection descriptor for `CddMcpToolCallParams`. */
extern C_CDD_EXPORT const struct cdd_type_info CddMcpToolCallParams_type_info;

/**
 * @brief Builds a dynamic JSON result string for MCP `resources/read`.
 *
 * @param[in] params Optional JSON-RPC params object containing `"uri"`.
 * @param[out] out_json Newly allocated JSON string for the result object.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t
build_mcp_resources_read(const JSON_Object *params, char **out_json);

/**
 * @brief Builds a dynamic JSON result string for MCP `prompts/get`.
 *
 * @param[in] params Optional JSON-RPC params object containing `"name"`.
 * @param[out] out_json Newly allocated JSON string for the result object.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t
build_mcp_prompts_get(const JSON_Object *params, char **out_json);

/**
 * @brief Serializes a `CddJsonRpcResponse` envelope to a JSON string.
 *
 * @param[in] resp Response envelope.
 * @param[out] out_json Newly allocated JSON string.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t CddJsonRpcResponse_to_json(
    const struct CddJsonRpcResponse *resp, char **out_json);

/**
 * @brief Deserializes MCP `tools/call` params from a `JSON_Object`.
 *
 * @param[in] obj JSON_Object containing `"name"` and `"arguments"`.
 * @param[out] out Populated `CddMcpToolCallParams` struct.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t CddMcpToolCallParams_from_jsonObject(
    const JSON_Object *obj, struct CddMcpToolCallParams *out);

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
