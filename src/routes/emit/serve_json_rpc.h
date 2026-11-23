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

/**
 * @brief MCP `resources/read` parameters struct.
 */
struct CddMcpReadResourceParams {
  /** @brief Resource URI */
  const char *uri;
};

/**
 * @brief MCP `prompts/get` parameters struct.
 */
struct CddMcpGetPromptParams {
  /** @brief Prompt name */
  const char *name;
};

/**
 * @brief MCP `sampling/createMessage` parameters struct.
 */
struct CddMcpCreateMessageParams {
  /** @brief Context inclusion mode */
  const char *includeContext;
  /** @brief System prompt string */
  const char *systemPrompt;
  /** @brief Max tokens to sample */
  double maxTokens;
  /** @brief Sampling temperature */
  double temperature;
};

/**
 * @brief MCP `resources/read` content item struct.
 */
struct CddMcpResourceContent {
  /** @brief Resource URI */
  const char *uri;
  /** @brief Resource MIME type */
  const char *mimeType;
  /** @brief Resource text content */
  const char *text;
};

/**
 * @brief MCP text content item struct.
 */
struct CddMcpTextContent {
  /** @brief Content type ("text") */
  const char *type;
  /** @brief Text payload */
  const char *text;
};

/** @brief Runtime reflection descriptor for `CddJsonRpcRequest`. */
extern C_CDD_EXPORT const struct cdd_type_info CddJsonRpcRequest_type_info;
/** @brief Runtime reflection descriptor for `CddJsonRpcErrorObj`. */
extern C_CDD_EXPORT const struct cdd_type_info CddJsonRpcErrorObj_type_info;
/** @brief Runtime reflection descriptor for `CddJsonRpcResponse`. */
extern C_CDD_EXPORT const struct cdd_type_info CddJsonRpcResponse_type_info;
/** @brief Runtime reflection descriptor for `CddMcpToolCallParams`. */
extern C_CDD_EXPORT const struct cdd_type_info CddMcpToolCallParams_type_info;
/** @brief Runtime reflection descriptor for `CddMcpReadResourceParams`. */
extern C_CDD_EXPORT const struct cdd_type_info
    CddMcpReadResourceParams_type_info;
/** @brief Runtime reflection descriptor for `CddMcpGetPromptParams`. */
extern C_CDD_EXPORT const struct cdd_type_info CddMcpGetPromptParams_type_info;
/** @brief Runtime reflection descriptor for `CddMcpCreateMessageParams`. */
extern C_CDD_EXPORT const struct cdd_type_info
    CddMcpCreateMessageParams_type_info;
/** @brief Runtime reflection descriptor for `CddMcpResourceContent`. */
extern C_CDD_EXPORT const struct cdd_type_info CddMcpResourceContent_type_info;
/** @brief Runtime reflection descriptor for `CddMcpTextContent`. */
extern C_CDD_EXPORT const struct cdd_type_info CddMcpTextContent_type_info;

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
 * @brief Dispatches a parsed HTTP JSON-RPC request body string and writes the
 * HTTP JSON-RPC response to `client_fd`.
 *
 * @param[in] client_fd Connected client socket descriptor (or mocked fd).
 * @param[in] body Raw JSON-RPC request payload following HTTP headers.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t
serve_json_rpc_dispatch_body(int client_fd, const char *body);

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

/**
 * @brief Frees a JSON serialized string returned by the JSON DOM serializer.
 *
 * @param[in,out] s Heap string to release (may be NULL).
 * @return CDD_C_SUCCESS on success.
 */
extern C_CDD_EXPORT cdd_c_error_t rpc_serialized_str_free(char *s);

/**
 * @brief Releases heap fields owned by a reflected struct instance.
 *
 * @param[in,out] obj Struct instance pointer.
 * @param[in] ti Reflection type descriptor.
 * @return CDD_C_SUCCESS on success, or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t
rpc_reflect_struct_free(void *obj, const struct cdd_type_info *ti);

/**
 * @brief Looks up an optional string property on `obj`, setting `*out_str` to
 * NULL when absent or non-string.
 *
 * @param[in] obj JSON object to query (may be NULL).
 * @param[in] key Property name.
 * @param[out] out_str Output string pointer or NULL.
 * @return CDD_C_SUCCESS on success, or error code on fault injection.
 */
extern C_CDD_EXPORT cdd_c_error_t rpc_json_opt_string(const JSON_Object *obj,
                                                      const char *key,
                                                      const char **out_str);

/**
 * @brief Looks up an optional child object property on `obj`, setting
 * `*out_obj` to NULL when absent or non-object.
 *
 * @param[in] obj JSON object to query (may be NULL).
 * @param[in] key Property name.
 * @param[out] out_obj Output child object pointer or NULL.
 * @return CDD_C_SUCCESS on success, or error code on fault injection.
 */
extern C_CDD_EXPORT cdd_c_error_t rpc_json_opt_object(const JSON_Object *obj,
                                                      const char *key,
                                                      JSON_Object **out_obj);

/**
 * @brief Looks up an optional child value property on `obj`, setting `*out_val`
 * to NULL when absent.
 *
 * @param[in] obj JSON object to query (may be NULL).
 * @param[in] key Property name.
 * @param[out] out_val Output child value pointer or NULL.
 * @return CDD_C_SUCCESS on success, or error code on fault injection.
 */
extern C_CDD_EXPORT cdd_c_error_t rpc_json_opt_value(const JSON_Object *obj,
                                                     const char *key,
                                                     JSON_Value **out_val);

/**
 * @brief Looks up an optional boolean property on `obj`, setting `*out_bool` to
 * 1 when true or 0 otherwise.
 *
 * @param[in] obj JSON object to query (may be NULL).
 * @param[in] key Property name.
 * @param[out] out_bool Output boolean value (0 or 1).
 * @return CDD_C_SUCCESS on success, or error code on fault injection.
 */
extern C_CDD_EXPORT cdd_c_error_t rpc_json_opt_bool(const JSON_Object *obj,
                                                    const char *key,
                                                    int *out_bool);

/**
 * @brief Attempts to parse `body` into `*out_val`, setting `*out_ok` to 1 on
 * valid JSON or 0 on syntax error while percolating OOM/fault errors.
 *
 * @param[in] body Raw JSON string.
 * @param[out] out_val Parsed JSON value on success, or NULL.
 * @param[out] out_ok Receives 1 when parsed as a JSON object, 0 otherwise.
 * @return CDD_C_SUCCESS on completion, or error code on OOM/fault injection.
 */
extern C_CDD_EXPORT cdd_c_error_t rpc_json_try_parse(const char *body,
                                                     JSON_Value **out_val,
                                                     int *out_ok);

/**
 * @brief Attempts to extract MCP `tools/call` parameters from `params`, setting
 * `*out_ok` to 1 when valid or 0 when missing/invalid.
 *
 * @param[in] params Optional JSON params object.
 * @param[out] out_tc Populated `CddMcpToolCallParams` struct.
 * @param[out] out_ok Receives 1 when valid, 0 otherwise.
 * @return CDD_C_SUCCESS on completion, or error code on fault injection.
 */
extern C_CDD_EXPORT cdd_c_error_t
rpc_try_tool_call_params(const JSON_Object *params,
                         struct CddMcpToolCallParams *out_tc, int *out_ok);

/**
 * @brief Executes `cdd_generate_to_openapi` for RPC dispatch, setting `*out_ok`
 * to 1 on success or 0 on application failure.
 *
 * @param[in] cfg Configuration struct.
 * @param[out] out_ok Receives 1 on success, 0 on generation failure.
 * @return CDD_C_SUCCESS on completion, or error code on fault injection.
 */
extern C_CDD_EXPORT cdd_c_error_t
rpc_exec_to_openapi(const cdd_to_openapi_config_t *cfg, int *out_ok);

/**
 * @brief Executes `cdd_generate_docs_json` for RPC dispatch, setting `*out_ok`
 * to 1 on success or 0 on application failure.
 *
 * @param[in] cfg Configuration struct.
 * @param[out] out_ok Receives 1 on success, 0 on generation failure.
 * @return CDD_C_SUCCESS on completion, or error code on fault injection.
 */
extern C_CDD_EXPORT cdd_c_error_t
rpc_exec_docs_json(const cdd_docs_json_config_t *cfg, int *out_ok);

/**
 * @brief Executes `cdd_generate_to_arazzo` for RPC dispatch, setting `*out_ok`
 * to 1 on success or 0 on application failure.
 *
 * @param[in] cfg Configuration struct.
 * @param[out] out_ok Receives 1 on success, 0 on generation failure.
 * @return CDD_C_SUCCESS on completion, or error code on fault injection.
 */
extern C_CDD_EXPORT cdd_c_error_t
rpc_exec_to_arazzo(const cdd_to_arazzo_config_t *cfg, int *out_ok);

/**
 * @brief Populates a `cdd_from_arazzo_config_t` from optional MCP tool call
 * JSON arguments, mapping string `"subcommand"` values (`"to_sdk"`,
 * `"to_sdk_cli"`, `"to_server"`) to enum constants.
 *
 * @param[in] args Optional JSON arguments object.
 * @param[out] out_cfg Populated `cdd_from_arazzo_config_t` struct.
 * @return CDD_C_SUCCESS on success, or error code on fault injection.
 */
extern C_CDD_EXPORT cdd_c_error_t rpc_parse_from_arazzo_args(
    const JSON_Object *args, cdd_from_arazzo_config_t *out_cfg);

/**
 * @brief Executes `cdd_generate_from_arazzo` for RPC dispatch, setting
 * `*out_ok` to 1 on success or 0 on application failure.
 *
 * @param[in] cfg Configuration struct.
 * @param[out] out_ok Receives 1 on success, 0 on generation failure.
 * @return CDD_C_SUCCESS on completion, or error code on fault injection.
 */
extern C_CDD_EXPORT cdd_c_error_t
rpc_exec_from_arazzo(const cdd_from_arazzo_config_t *cfg, int *out_ok);

/**
 * @brief Executes `cdd_generate_from_openapi` for RPC dispatch, setting
 * `*out_ok` to 1 on success or 0 on application failure.
 *
 * @param[in] cfg Configuration struct.
 * @param[out] out_ok Receives 1 on success, 0 on generation failure.
 * @return CDD_C_SUCCESS on completion, or error code on fault injection.
 */
extern C_CDD_EXPORT cdd_c_error_t
rpc_exec_from_openapi(const cdd_from_openapi_config_t *cfg, int *out_ok);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
