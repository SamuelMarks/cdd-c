/**
 * @file serve_mcp_stdio.c
 * @brief Implementation of MCP stdio server transport.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"
#include "c_cdd/log.h"
#include "c_cdd/memory.h"
#include "serve_json_rpc.h"

#include "../parse/cli.h"
#include "../../cdd_api.h"
#include "classes/emit/cli_struct.h"
#include "classes/parse/cdd_serde.h"
#include "functions/parse/fs.h"
#include <c89stringutils_string_extras.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/** @brief Reflected fields for `CddJsonRpcRequest`. */
static const struct cdd_field_info g_cdd_json_rpc_request_fields[] = {
    {"jsonrpc",
     "jsonrpc",
     "const char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct CddJsonRpcRequest, jsonrpc),
     sizeof(const char *),
     0,
     0,
     1,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"method",
     "method",
     "const char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct CddJsonRpcRequest, method),
     sizeof(const char *),
     0,
     0,
     1,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0}};

const struct cdd_type_info CddJsonRpcRequest_type_info = {
    "CddJsonRpcRequest",
    sizeof(struct CddJsonRpcRequest),
    0,
    g_cdd_json_rpc_request_fields,
    sizeof(g_cdd_json_rpc_request_fields) /
        sizeof(g_cdd_json_rpc_request_fields[0]),
    0};

/** @brief Reflected fields for `CddJsonRpcErrorObj`. */
static const struct cdd_field_info g_cdd_json_rpc_error_fields[] = {
    {"code",
     "code",
     "int",
     NULL,
     CDD_FIELD_KIND_INTEGER,
     offsetof(struct CddJsonRpcError, code),
     sizeof(int),
     0,
     0,
     1,
     0,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"message",
     "message",
     "const char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct CddJsonRpcError, message),
     sizeof(const char *),
     0,
     0,
     1,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0}};

const struct cdd_type_info CddJsonRpcErrorObj_type_info = {
    "CddJsonRpcErrorObj",
    sizeof(struct CddJsonRpcError),
    0,
    g_cdd_json_rpc_error_fields,
    sizeof(g_cdd_json_rpc_error_fields) /
        sizeof(g_cdd_json_rpc_error_fields[0]),
    0};

/** @brief Reflected fields for `CddJsonRpcResponse`. */
static const struct cdd_field_info g_cdd_json_rpc_response_fields[] = {
    {"jsonrpc",
     "jsonrpc",
     "const char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct CddJsonRpcResponse, jsonrpc),
     sizeof(const char *),
     0,
     0,
     1,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"result_json",
     "result",
     "const char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct CddJsonRpcResponse, result_json),
     sizeof(const char *),
     0,
     0,
     0,
     0,
     NULL,
     1,
     1,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0}};

const struct cdd_type_info CddJsonRpcResponse_type_info = {
    "CddJsonRpcResponse",
    sizeof(struct CddJsonRpcResponse),
    0,
    g_cdd_json_rpc_response_fields,
    sizeof(g_cdd_json_rpc_response_fields) /
        sizeof(g_cdd_json_rpc_response_fields[0]),
    0};

/** @brief Reflected fields for `CddMcpToolCallParams`. */
static const struct cdd_field_info g_cdd_mcp_tool_call_params_fields[] = {
    {"name",
     "name",
     "const char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct CddMcpToolCallParams, name),
     sizeof(const char *),
     0,
     0,
     1,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0}};

const struct cdd_type_info CddMcpToolCallParams_type_info = {
    "CddMcpToolCallParams",
    sizeof(struct CddMcpToolCallParams),
    0,
    g_cdd_mcp_tool_call_params_fields,
    sizeof(g_cdd_mcp_tool_call_params_fields) /
        sizeof(g_cdd_mcp_tool_call_params_fields[0]),
    0};

/** @brief Reflected fields for `CddMcpReadResourceParams`. */
static const struct cdd_field_info g_cdd_mcp_read_resource_params_fields[] = {
    {"uri",
     "uri",
     "const char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct CddMcpReadResourceParams, uri),
     sizeof(const char *),
     0,
     0,
     0,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0}};

const struct cdd_type_info CddMcpReadResourceParams_type_info = {
    "CddMcpReadResourceParams",
    sizeof(struct CddMcpReadResourceParams),
    0,
    g_cdd_mcp_read_resource_params_fields,
    sizeof(g_cdd_mcp_read_resource_params_fields) /
        sizeof(g_cdd_mcp_read_resource_params_fields[0]),
    0};

/** @brief Reflected fields for `CddMcpGetPromptParams`. */
static const struct cdd_field_info g_cdd_mcp_get_prompt_params_fields[] = {
    {"name",
     "name",
     "const char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct CddMcpGetPromptParams, name),
     sizeof(const char *),
     0,
     0,
     0,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0}};

const struct cdd_type_info CddMcpGetPromptParams_type_info = {
    "CddMcpGetPromptParams",
    sizeof(struct CddMcpGetPromptParams),
    0,
    g_cdd_mcp_get_prompt_params_fields,
    sizeof(g_cdd_mcp_get_prompt_params_fields) /
        sizeof(g_cdd_mcp_get_prompt_params_fields[0]),
    0};

/** @brief Reflected fields for `CddMcpCreateMessageParams`. */
static const struct cdd_field_info g_cdd_mcp_create_message_params_fields[] = {
    {"includeContext",
     "includeContext",
     "const char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct CddMcpCreateMessageParams, includeContext),
     sizeof(const char *),
     0,
     0,
     0,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"systemPrompt",
     "systemPrompt",
     "const char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct CddMcpCreateMessageParams, systemPrompt),
     sizeof(const char *),
     0,
     0,
     0,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"maxTokens",
     "maxTokens",
     "double",
     NULL,
     CDD_FIELD_KIND_NUMBER,
     offsetof(struct CddMcpCreateMessageParams, maxTokens),
     sizeof(double),
     0,
     0,
     0,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"temperature",
     "temperature",
     "double",
     NULL,
     CDD_FIELD_KIND_NUMBER,
     offsetof(struct CddMcpCreateMessageParams, temperature),
     sizeof(double),
     0,
     0,
     0,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0}};

const struct cdd_type_info CddMcpCreateMessageParams_type_info = {
    "CddMcpCreateMessageParams",
    sizeof(struct CddMcpCreateMessageParams),
    0,
    g_cdd_mcp_create_message_params_fields,
    sizeof(g_cdd_mcp_create_message_params_fields) /
        sizeof(g_cdd_mcp_create_message_params_fields[0]),
    0};

/** @brief Reflected fields for `CddMcpResourceContent`. */
static const struct cdd_field_info g_cdd_mcp_resource_content_fields[] = {
    {"uri",
     "uri",
     "const char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct CddMcpResourceContent, uri),
     sizeof(const char *),
     0,
     0,
     1,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"mimeType",
     "mimeType",
     "const char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct CddMcpResourceContent, mimeType),
     sizeof(const char *),
     0,
     0,
     1,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"text",
     "text",
     "const char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct CddMcpResourceContent, text),
     sizeof(const char *),
     0,
     0,
     1,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0}};

const struct cdd_type_info CddMcpResourceContent_type_info = {
    "CddMcpResourceContent",
    sizeof(struct CddMcpResourceContent),
    0,
    g_cdd_mcp_resource_content_fields,
    sizeof(g_cdd_mcp_resource_content_fields) /
        sizeof(g_cdd_mcp_resource_content_fields[0]),
    0};

/** @brief Reflected fields for `CddMcpTextContent`. */
static const struct cdd_field_info g_cdd_mcp_text_content_fields[] = {
    {"type",
     "type",
     "const char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct CddMcpTextContent, type),
     sizeof(const char *),
     0,
     0,
     1,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0},
    {"text",
     "text",
     "const char *",
     NULL,
     CDD_FIELD_KIND_STRING,
     offsetof(struct CddMcpTextContent, text),
     sizeof(const char *),
     0,
     0,
     1,
     0,
     NULL,
     1,
     0,
     0,
     0,
     0,
     0,
     NULL,
     NULL,
     0,
     CDD_FIELD_KIND_UNKNOWN,
     NULL,
     0,
     NULL,
     NULL,
     NULL,
     NULL,
     0,
     0,
     0,
     0,
     0,
     0.0,
     0,
     0.0,
     0,
     0,
     0,
     0,
     0,
     0,
     0}};

const struct cdd_type_info CddMcpTextContent_type_info = {
    "CddMcpTextContent",
    sizeof(struct CddMcpTextContent),
    0,
    g_cdd_mcp_text_content_fields,
    sizeof(g_cdd_mcp_text_content_fields) /
        sizeof(g_cdd_mcp_text_content_fields[0]),
    0};

cdd_c_error_t rpc_serialized_str_free(char *s) {
  return cdd_json_free_serialized_string(s);
}

cdd_c_error_t rpc_reflect_struct_free(void *obj,
                                      const struct cdd_type_info *ti) {
#ifdef CDD_BUILD_TESTS
  int saved_step = g_cdd_step_fail;
  cdd_c_error_t rc;
  g_cdd_step_fail = 0;
  rc = cdd_reflect_struct_cleanup(obj, ti);
  g_cdd_step_fail = saved_step;
  return rc;
#else
  return cdd_reflect_struct_cleanup(obj, ti);
#endif
}

cdd_c_error_t rpc_json_opt_string(const JSON_Object *obj, const char *key,
                                  const char **out_str) {
  cdd_c_error_t rc;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  *out_str = NULL;
  rc = cdd_json_object_get_string(obj, key, out_str);
  if (rc == CDD_C_SUCCESS)
    return rc;
  return CDD_C_SUCCESS;
}

cdd_c_error_t rpc_json_opt_object(const JSON_Object *obj, const char *key,
                                  JSON_Object **out_obj) {
  cdd_c_error_t rc;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  *out_obj = NULL;
  rc = cdd_json_object_get_object(obj, key, out_obj);
  if (rc == CDD_C_SUCCESS)
    return rc;
  return CDD_C_SUCCESS;
}

cdd_c_error_t rpc_json_opt_value(const JSON_Object *obj, const char *key,
                                 JSON_Value **out_val) {
  cdd_c_error_t rc;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  *out_val = NULL;
  rc = cdd_json_object_get_value(obj, key, out_val);
  if (rc == CDD_C_SUCCESS)
    return rc;
  return CDD_C_SUCCESS;
}

cdd_c_error_t rpc_json_opt_bool(const JSON_Object *obj, const char *key,
                                int *out_bool) {
  int bval = 0;
  cdd_c_error_t rc;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  *out_bool = 0;
  rc = cdd_json_object_get_boolean(obj, key, &bval);
  if (rc == CDD_C_SUCCESS) {
    *out_bool = (bval == 1) ? 1 : 0;
    return rc;
  }
  return CDD_C_SUCCESS;
}

cdd_c_error_t rpc_json_try_parse(const char *body, JSON_Value **out_val,
                                 int *out_ok) {
  cdd_c_error_t rc;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  *out_val = NULL;
  *out_ok = 0;
  rc = cdd_json_parse_string(body, out_val);
  if (rc == CDD_C_SUCCESS) {
    *out_ok = 1;
    return rc;
  }
  return CDD_C_SUCCESS;
}

cdd_c_error_t rpc_try_tool_call_params(const JSON_Object *params,
                                       struct CddMcpToolCallParams *out_tc,
                                       int *out_ok) {
  cdd_c_error_t rc;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  *out_ok = 0;
  rc = CddMcpToolCallParams_from_jsonObject(params, out_tc);
  if (rc == CDD_C_SUCCESS) {
    *out_ok = 1;
    return rc;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Attempts to read a file into `*out_content`, leaving `*out_content` as
 * NULL when unreadable.
 *
 * @param[in] file_path Path to inspect.
 * @param[out] out_content Heap buffer on success, or NULL.
 * @return CDD_C_SUCCESS on completion, or error code on fault injection.
 */
static cdd_c_error_t rpc_try_read_file(const char *file_path,
                                       char **out_content) {
  size_t sz = 0;
  cdd_c_error_t rc;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  *out_content = NULL;
  rc = read_to_file(file_path, "rb", out_content, &sz);
  if (rc == CDD_C_SUCCESS)
    return rc;
  return CDD_C_SUCCESS;
}

cdd_c_error_t rpc_exec_to_openapi(const cdd_to_openapi_config_t *cfg,
                                  int *out_ok) {
  cdd_c_error_t rc;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  *out_ok = 0;
  rc = cdd_generate_to_openapi(cfg);
  if (rc == CDD_C_SUCCESS) {
    *out_ok = 1;
    return rc;
  }
  return CDD_C_SUCCESS;
}

cdd_c_error_t rpc_exec_docs_json(const cdd_docs_json_config_t *cfg,
                                 int *out_ok) {
  cdd_c_error_t rc;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  *out_ok = 0;
  rc = cdd_generate_docs_json(cfg);
  if (rc == CDD_C_SUCCESS) {
    *out_ok = 1;
    return rc;
  }
  return CDD_C_SUCCESS;
}

cdd_c_error_t rpc_exec_to_arazzo(const cdd_to_arazzo_config_t *cfg,
                                 int *out_ok) {
  cdd_c_error_t rc;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  *out_ok = 0;
  rc = cdd_generate_to_arazzo(cfg);
  if (rc == CDD_C_SUCCESS) {
    *out_ok = 1;
    return rc;
  }
  return CDD_C_SUCCESS;
}

cdd_c_error_t rpc_parse_from_arazzo_args(const JSON_Object *args,
                                         cdd_from_arazzo_config_t *out_cfg) {
  const char *subcmd = NULL;
  cdd_c_error_t rc;

  memset(out_cfg, 0, sizeof(*out_cfg));
  rc = rpc_json_opt_string(args, "subcommand", &subcmd);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (subcmd && strcmp(subcmd, "to_sdk_cli") == 0) {
    out_cfg->subcommand = CDD_ARAZZO_SUBCOMMAND_TO_SDK_CLI;
  } else if (subcmd && strcmp(subcmd, "to_server") == 0) {
    out_cfg->subcommand = CDD_ARAZZO_SUBCOMMAND_TO_SERVER;
  } else {
    out_cfg->subcommand = CDD_ARAZZO_SUBCOMMAND_TO_SDK;
  }
  rc = rpc_json_opt_string(args, "input", &out_cfg->input);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = rpc_json_opt_string(args, "openapi_input", &out_cfg->openapi_input);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = rpc_json_opt_string(args, "output", &out_cfg->output);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = rpc_json_opt_string(args, "func_prefix", &out_cfg->func_prefix);
  if (rc != CDD_C_SUCCESS)
    return rc;
  return CDD_C_SUCCESS;
}

cdd_c_error_t rpc_exec_from_arazzo(const cdd_from_arazzo_config_t *cfg,
                                   int *out_ok) {
  cdd_c_error_t rc;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  *out_ok = 0;
  rc = cdd_generate_from_arazzo(cfg);
  if (rc == CDD_C_SUCCESS) {
    *out_ok = 1;
    return rc;
  }
  return CDD_C_SUCCESS;
}

cdd_c_error_t rpc_exec_from_openapi(const cdd_from_openapi_config_t *cfg,
                                    int *out_ok) {
  cdd_c_error_t rc;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  *out_ok = 0;
  rc = cdd_generate_from_openapi(cfg);
  if (rc == CDD_C_SUCCESS) {
    *out_ok = 1;
    return rc;
  }
  return CDD_C_SUCCESS;
}

cdd_c_error_t CddJsonRpcResponse_to_json(const struct CddJsonRpcResponse *resp,
                                         char **out_json) {
  JSON_Value *root_val = NULL;
  JSON_Value *sub_val = NULL;
  JSON_Object *root_obj = NULL;
  JSON_Object *err_obj = NULL;
  cdd_c_error_t rc;

  rc = cdd_json_value_init_object(&root_val);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = cdd_json_value_get_object(root_val, &root_obj);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;
  rc = cdd_json_object_set_string(root_obj, "jsonrpc", resp->jsonrpc);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  if (resp->result_json) {
    rc = cdd_json_parse_string(resp->result_json, &sub_val);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    rc = cdd_json_object_set_value(root_obj, "result", sub_val);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    sub_val = NULL;
  } else {
    rc = cdd_json_value_init_object(&sub_val);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    rc = cdd_json_value_get_object(sub_val, &err_obj);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    rc = cdd_reflect_to_json_object(&resp->error, &CddJsonRpcErrorObj_type_info,
                                    err_obj);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    rc = cdd_json_object_set_value(root_obj, "error", sub_val);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    sub_val = NULL;
  }

  if (resp->id_val) {
    rc = cdd_json_value_deep_copy(resp->id_val, &sub_val);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    rc = cdd_json_object_set_value(root_obj, "id", sub_val);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    sub_val = NULL;
  } else {
    rc = cdd_json_object_set_null(root_obj, "id");
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
  }

  rc = cdd_json_serialize_to_string(root_val, out_json);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

cleanup:;
  return cdd_c_keep_err(rc, cdd_c_keep_err(cdd_json_value_free(sub_val),
                                           cdd_json_value_free(root_val)));
}

cdd_c_error_t
CddMcpToolCallParams_from_jsonObject(const JSON_Object *obj,
                                     struct CddMcpToolCallParams *out) {
  const char *name = NULL;
  JSON_Object *arguments = NULL;
  cdd_c_error_t rc;
  memset(out, 0, sizeof(*out));
  if (!obj)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = cdd_json_object_get_string(obj, "name", &name);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = cdd_json_object_get_object(obj, "arguments", &arguments);
  if (rc != CDD_C_SUCCESS)
    return rc;
  out->name = name;
  out->arguments = arguments;
  return CDD_C_SUCCESS;
}

/**
 * @brief Helper to respond with JSON-RPC error over stdio.
 *
 * @param[in] id_val The JSON-RPC request ID.
 * @param[in] code The JSON-RPC error code.
 * @param[in] msg The JSON-RPC error message.
 * @return CDD_C_SUCCESS on success, or error code.
 */
static cdd_c_error_t send_stdio_rpc_error(JSON_Value *id_val, int code,
                                          const char *msg) {
  struct CddJsonRpcResponse env;
  char *json_out = NULL;
  cdd_c_error_t rc;
  memset(&env, 0, sizeof(env));
  env.jsonrpc = "2.0";
  env.error.code = code;
  env.error.message = msg;
  env.id_val = id_val;
  rc = CddJsonRpcResponse_to_json(&env, &json_out);
  if (rc != CDD_C_SUCCESS)
    return rc;
  printf("%s\n", json_out);
  fflush(stdout);
  return rpc_serialized_str_free(json_out);
}

/**
 * @brief Helper to respond with JSON-RPC success over stdio.
 *
 * @param[in] id_val The JSON-RPC request ID.
 * @param[in] result_json Serialized JSON result string.
 * @return CDD_C_SUCCESS on success, or error code.
 */
static cdd_c_error_t send_stdio_rpc_response(JSON_Value *id_val,
                                             const char *result_json) {
  struct CddJsonRpcResponse env;
  char *json_out = NULL;
  cdd_c_error_t rc;
  memset(&env, 0, sizeof(env));
  env.jsonrpc = "2.0";
  env.result_json = result_json;
  env.id_val = id_val;
  rc = CddJsonRpcResponse_to_json(&env, &json_out);
  if (rc != CDD_C_SUCCESS)
    return rc;
  printf("%s\n", json_out);
  fflush(stdout);
  return rpc_serialized_str_free(json_out);
}

cdd_c_error_t build_mcp_resources_read(const JSON_Object *params,
                                       char **out_json) {
  struct CddMcpReadResourceParams rp;
  struct CddMcpResourceContent item_data;
  const char *target_uri;
  const char *file_path = NULL;
  char *content = NULL;
  JSON_Value *res_val = NULL;
  JSON_Value *arr_val = NULL;
  JSON_Value *item_val = NULL;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  *out_json = NULL;
  memset(&rp, 0, sizeof(rp));
  if (params) {
    rc = cdd_reflect_from_json_object(params,
                                      &CddMcpReadResourceParams_type_info, &rp);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
  }
  target_uri = rp.uri ? rp.uri : "file:///openapi.json";

  rc = cdd_json_value_init_object(&res_val);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;
  rc = cdd_json_value_init_array(&arr_val);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;
  rc = cdd_json_value_init_object(&item_val);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  if (strncmp(target_uri, "file:///", 8) == 0) {
    file_path = target_uri + 8;
  } else if (strncmp(target_uri, "file://", 7) == 0) {
    file_path = target_uri + 7;
  } else {
    file_path = target_uri;
  }

  rc = rpc_try_read_file(file_path, &content);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  item_data.uri = target_uri;
  item_data.mimeType = "application/json";
  item_data.text =
      content
          ? content
          : "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"cdd-c\",\"version\":"
            "\"0.0.5\"},\"paths\":{}}";

  rc = cdd_reflect_to_json_object(&item_data, &CddMcpResourceContent_type_info,
                                  item_val->object_val);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  rc = cdd_json_array_append_value(arr_val->array_val, item_val);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;
  item_val = NULL;

  rc = cdd_json_object_set_value(res_val->object_val, "contents", arr_val);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;
  arr_val = NULL;

  rc = cdd_json_serialize_to_string(res_val, out_json);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

cleanup:
  C_CDD_FREE(content);
  return cdd_c_keep_err(
      rc,
      cdd_c_keep_err(
          cdd_json_value_free(item_val),
          cdd_c_keep_err(
              cdd_json_value_free(arr_val),
              cdd_c_keep_err(cdd_json_value_free(res_val),
                             rpc_reflect_struct_free(
                                 &rp, &CddMcpReadResourceParams_type_info)))));
}

cdd_c_error_t build_mcp_prompts_get(const JSON_Object *params,
                                    char **out_json) {
  struct CddMcpGetPromptParams pp;
  struct CddMcpTextContent cnt_data;
  const char *name;
  char desc_buf[256];
  char text_buf[256];
  JSON_Value *res_val = NULL;
  JSON_Value *msgs_val = NULL;
  JSON_Value *msg_val = NULL;
  JSON_Value *cnt_val = NULL;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  *out_json = NULL;
  memset(&pp, 0, sizeof(pp));
  if (params) {
    rc = cdd_reflect_from_json_object(params, &CddMcpGetPromptParams_type_info,
                                      &pp);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
  }
  name = pp.name ? pp.name : "generate_sdk";
#if defined(_MSC_VER)
  sprintf_s(desc_buf, sizeof(desc_buf), "Prompt %s", name);
  sprintf_s(text_buf, sizeof(text_buf), "Execute %s", name);
#else
  CDD_SNPRINTF(desc_buf, sizeof(desc_buf), "Prompt %s", name);
  CDD_SNPRINTF(text_buf, sizeof(text_buf), "Execute %s", name);
#endif
  cnt_data.type = "text";
  cnt_data.text = text_buf;

  rc = cdd_json_value_init_object(&res_val);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;
  rc = cdd_json_value_init_array(&msgs_val);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;
  rc = cdd_json_value_init_object(&msg_val);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;
  rc = cdd_json_value_init_object(&cnt_val);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  rc = cdd_json_object_set_string(res_val->object_val, "description", desc_buf);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;
  rc = cdd_json_object_set_string(msg_val->object_val, "role", "user");
  if (rc != CDD_C_SUCCESS)
    goto cleanup;
  rc = cdd_reflect_to_json_object(&cnt_data, &CddMcpTextContent_type_info,
                                  cnt_val->object_val);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  rc = cdd_json_object_set_value(msg_val->object_val, "content", cnt_val);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;
  cnt_val = NULL;

  rc = cdd_json_array_append_value(msgs_val->array_val, msg_val);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;
  msg_val = NULL;

  rc = cdd_json_object_set_value(res_val->object_val, "messages", msgs_val);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;
  msgs_val = NULL;

  rc = cdd_json_serialize_to_string(res_val, out_json);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

cleanup:;
  return cdd_c_keep_err(
      rc, cdd_c_keep_err(
              cdd_json_value_free(cnt_val),
              cdd_c_keep_err(
                  cdd_json_value_free(msg_val),
                  cdd_c_keep_err(
                      cdd_json_value_free(msgs_val),
                      cdd_c_keep_err(
                          cdd_json_value_free(res_val),
                          rpc_reflect_struct_free(
                              &pp, &CddMcpGetPromptParams_type_info))))));
}

/**
 * @brief Dispatches an MCP `tools/call` invocation over stdio.
 *
 * @param[in] id_val Optional JSON-RPC request ID.
 * @param[in] params Optional JSON params object.
 * @return CDD_C_SUCCESS on success, or error code.
 */
static cdd_c_error_t handle_stdio_tool_call(JSON_Value *id_val,
                                            const JSON_Object *params) {
  struct CddMcpToolCallParams tc_params;
  int ok = 0;
  cdd_c_error_t rc;

  rc = rpc_try_tool_call_params(params, &tc_params, &ok);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (!ok) {
    return send_stdio_rpc_error(id_val, -32602,
                                "Invalid params for tools/call");
  }

  if (strcmp(tc_params.name, "to_openapi") == 0) {
    cdd_to_openapi_config_t cfg;
    int gen_ok = 0;
    memset(&cfg, 0, sizeof(cfg));
    rc = rpc_json_opt_string(tc_params.arguments, "input", &cfg.input);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = rpc_json_opt_string(tc_params.arguments, "output", &cfg.output);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (!cfg.input || !cfg.output) {
      return send_stdio_rpc_error(id_val, -32602,
                                  "Invalid arguments for to_openapi");
    }
    rc = rpc_json_opt_string(tc_params.arguments, "base_file", &cfg.base_file);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = rpc_json_opt_string(tc_params.arguments, "self_uri", &cfg.self_uri);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = rpc_json_opt_string(tc_params.arguments, "dialect_uri",
                             &cfg.dialect_uri);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = rpc_exec_to_openapi(&cfg, &gen_ok);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (!gen_ok) {
      return send_stdio_rpc_error(id_val, -32603, "OpenAPI generation failed");
    }
    return send_stdio_rpc_response(
        id_val, "{\"content\":[{\"type\":\"text\",\"text\":\"OpenAPI "
                "generation successful\"}],\"isError\":false}");
  }

  if (strcmp(tc_params.name, "to_docs_json") == 0) {
    cdd_docs_json_config_t cfg;
    int bval = 0;
    int gen_ok = 0;
    memset(&cfg, 0, sizeof(cfg));
    rc = rpc_json_opt_string(tc_params.arguments, "input", &cfg.input);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (!cfg.input) {
      return send_stdio_rpc_error(id_val, -32602,
                                  "Invalid arguments for to_docs_json");
    }
    rc = rpc_json_opt_string(tc_params.arguments, "output", &cfg.output);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = rpc_json_opt_bool(tc_params.arguments, "no_imports", &bval);
    if (rc != CDD_C_SUCCESS)
      return rc;
    cfg.no_imports = (bval == 1) ? CDD_TRUE : CDD_FALSE;
    rc = rpc_json_opt_bool(tc_params.arguments, "no_wrapping", &bval);
    if (rc != CDD_C_SUCCESS)
      return rc;
    cfg.no_wrapping = (bval == 1) ? CDD_TRUE : CDD_FALSE;
    rc = rpc_exec_docs_json(&cfg, &gen_ok);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (!gen_ok) {
      return send_stdio_rpc_error(id_val, -32603, "Docs generation failed");
    }
    return send_stdio_rpc_response(
        id_val, "{\"content\":[{\"type\":\"text\",\"text\":\"Docs "
                "generation successful\"}],\"isError\":false}");
  }

  if (strcmp(tc_params.name, "to_arazzo") == 0) {
    cdd_to_arazzo_config_t cfg;
    int gen_ok = 0;
    memset(&cfg, 0, sizeof(cfg));
    rc = rpc_json_opt_string(tc_params.arguments, "input", &cfg.input);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = rpc_json_opt_string(tc_params.arguments, "output", &cfg.output);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = rpc_exec_to_arazzo(&cfg, &gen_ok);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (!gen_ok) {
      return send_stdio_rpc_error(id_val, -32603, "Arazzo extraction failed");
    }
    return send_stdio_rpc_response(
        id_val, "{\"content\":[{\"type\":\"text\",\"text\":\"Arazzo "
                "extraction successful\"}],\"isError\":false}");
  }

  if (strcmp(tc_params.name, "from_arazzo") == 0) {
    cdd_from_arazzo_config_t cfg;
    int gen_ok = 0;
    rc = rpc_parse_from_arazzo_args(tc_params.arguments, &cfg);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = rpc_exec_from_arazzo(&cfg, &gen_ok);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (!gen_ok) {
      return send_stdio_rpc_error(id_val, -32603, "Arazzo generation failed");
    }
    return send_stdio_rpc_response(
        id_val, "{\"content\":[{\"type\":\"text\",\"text\":\"Arazzo "
                "generation successful\"}],\"isError\":false}");
  }

  return send_stdio_rpc_error(id_val, -32601, "Tool not found");
}

/**
 * @brief Sends an MCP stdio JSON-RPC response and then releases `dyn_str` and
 * `root_val` in defined C89 sequence order.
 *
 * @param[in] id_val JSON-RPC request ID value (owned by `root_val`).
 * @param[in] result_json Serialized JSON result payload.
 * @param[in,out] dyn_str Optional heap string to free after sending.
 * @param[in,out] root_val Request JSON DOM root to free after sending.
 * @return CDD_C_SUCCESS on success, or error code.
 */
static cdd_c_error_t send_stdio_rpc_response_cleanup(JSON_Value *id_val,
                                                     const char *result_json,
                                                     char *dyn_str,
                                                     JSON_Value *root_val) {
  cdd_c_error_t rc = send_stdio_rpc_response(id_val, result_json);
  if (rc != CDD_C_SUCCESS) {
    return cdd_c_keep_err(rc, cdd_c_keep_err(rpc_serialized_str_free(dyn_str),
                                             cdd_json_value_free(root_val)));
  }
  return cdd_c_keep_err(rpc_serialized_str_free(dyn_str),
                        cdd_json_value_free(root_val));
}

/**
 * @brief Sends an MCP stdio JSON-RPC error and then releases `root_val` in
 * defined C89 sequence order.
 *
 * @param[in] id_val JSON-RPC request ID value (owned by `root_val`).
 * @param[in] code JSON-RPC error code.
 * @param[in] message Error message string.
 * @param[in,out] root_val Request JSON DOM root to free after sending.
 * @return CDD_C_SUCCESS on success, or error code.
 */
static cdd_c_error_t send_stdio_rpc_error_cleanup(JSON_Value *id_val, int code,
                                                  const char *message,
                                                  JSON_Value *root_val) {
  cdd_c_error_t rc = send_stdio_rpc_error(id_val, code, message);
  if (rc != CDD_C_SUCCESS) {
    return cdd_c_keep_err(rc, cdd_json_value_free(root_val));
  }
  return cdd_json_value_free(root_val);
}

/**
 * @brief Handles an MCP stdio request.
 *
 * @param[in] body The JSON-RPC request body string.
 * @return CDD_C_SUCCESS on success, or error code.
 */
C_CDD_EXPORT cdd_c_error_t handle_stdio_request(const char *body) {
  JSON_Value *root_val = NULL;
  JSON_Object *root_obj = NULL;
  const char *method = NULL;
  JSON_Value *id_val = NULL;
  char *dyn_str = NULL;
  int parse_ok = 0;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  if (!body)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  rc = rpc_json_try_parse(body, &root_val, &parse_ok);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (!parse_ok) {
    return send_stdio_rpc_error(NULL, -32700, "Parse error");
  }

  rc = cdd_json_value_get_object(root_val, &root_obj);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;
  rc = rpc_json_opt_string(root_obj, "method", &method);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;
  rc = rpc_json_opt_value(root_obj, "id", &id_val);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  if (!method) {
    return send_stdio_rpc_error_cleanup(id_val, -32600, "Invalid Request",
                                        root_val);
  }

  if (strcmp(method, "version") == 0) {
    return send_stdio_rpc_response_cleanup(id_val, "\"0.0.5\"", NULL, root_val);
  }
  if (strcmp(method, "initialize") == 0) {
    return send_stdio_rpc_response_cleanup(
        id_val,
        "{\"protocolVersion\":\"2024-11-05\",\"capabilities\":{"
        "\"tools\":{\"listChanged\":true},\"resources\":{\"subscribe\":"
        "true,\"listChanged\":true},\"prompts\":{\"listChanged\":true},"
        "\"logging\":{}},\"serverInfo\":{\"name\":\"cdd-c\",\"version\":"
        "\"0.0.5\"}}",
        NULL, root_val);
  }
  if (strcmp(method, "notifications/initialized") == 0 ||
      strcmp(method, "notifications/progress") == 0 ||
      strcmp(method, "notifications/message") == 0 ||
      strcmp(method, "notifications/cancelled") == 0 ||
      strcmp(method, "notifications/roots/list_changed") == 0 ||
      strcmp(method, "notifications/resources/list_changed") == 0 ||
      strcmp(method, "notifications/tools/list_changed") == 0 ||
      strcmp(method, "notifications/prompts/list_changed") == 0) {
    return cdd_json_value_free(root_val);
  }
  if (strcmp(method, "logging/setLevel") == 0 || strcmp(method, "ping") == 0 ||
      strcmp(method, "resources/subscribe") == 0 ||
      strcmp(method, "resources/unsubscribe") == 0) {
    return send_stdio_rpc_response_cleanup(id_val, "{}", NULL, root_val);
  }
  if (strcmp(method, "tools/list") == 0) {
    return send_stdio_rpc_response_cleanup(
        id_val,
        "{\"tools\":[{\"name\":\"to_openapi\",\"description\":\"Generate "
        "OpenAPI spec from code\",\"inputSchema\":{\"type\":\"object\"}},"
        "{\"name\":\"to_docs_json\",\"description\":\"Generate JSON docs\","
        "\"inputSchema\":{\"type\":\"object\"}},{\"name\":\"to_arazzo\","
        "\"description\":\"Extract Arazzo spec from C "
        "code\",\"inputSchema\":"
        "{\"type\":\"object\"}},{\"name\":\"from_arazzo\",\"description\":"
        "\"Generate C workflows from Arazzo spec\",\"inputSchema\":"
        "{\"type\":\"object\"}}]}",
        NULL, root_val);
  }
  if (strcmp(method, "resources/list") == 0) {
    return send_stdio_rpc_response_cleanup(
        id_val,
        "{\"resources\":[{\"uri\":\"file:///"
        "openapi.json\",\"name\":\"OpenAPI "
        "Spec\",\"mimeType\":\"application/json\"},{\"uri\":"
        "\"file:///arazzo.json\",\"name\":\"Arazzo Workflow Spec\","
        "\"mimeType\":\"application/json\"}]}",
        NULL, root_val);
  }
  if (strcmp(method, "roots/list") == 0) {
    return send_stdio_rpc_response_cleanup(
        id_val,
        "{\"roots\":[{\"uri\":\"file:///"
        "workspace\",\"name\":\"workspace\"}]}",
        NULL, root_val);
  }
  if (strcmp(method, "resources/templates/list") == 0) {
    return send_stdio_rpc_response_cleanup(
        id_val,
        "{\"resourceTemplates\":[{\"uriTemplate\":\"file:///{path}\","
        "\"name\":\"Workspace File\",\"mimeType\":\"application/json\"}]}",
        NULL, root_val);
  }
  if (strcmp(method, "resources/read") == 0) {
    JSON_Object *params = NULL;
    rc = rpc_json_opt_object(root_obj, "params", &params);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    rc = build_mcp_resources_read(params, &dyn_str);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    return send_stdio_rpc_response_cleanup(id_val, dyn_str, dyn_str, root_val);
  }
  if (strcmp(method, "tools/read_image") == 0) {
    return send_stdio_rpc_response_cleanup(
        id_val,
        "{\"content\":[{\"type\":\"image\",\"data\":\"iVBORw0KGgo=\","
        "\"mimeType\":\"image/png\",\"annotations\":{\"audience\":["
        "\"user\"],\"priority\":1.0}}]}",
        NULL, root_val);
  }
  if (strcmp(method, "prompts/list") == 0) {
    return send_stdio_rpc_response_cleanup(
        id_val,
        "{\"prompts\":[{\"name\":\"generate_sdk\",\"description\":"
        "\"Generate "
        "SDK "
        "prompt\",\"arguments\":[{\"name\":\"language\",\"description\":"
        "\"Target "
        "language\",\"required\":true}]},{\"name\":\"generate_arazzo\","
        "\"description\":\"Generate Arazzo workflow orchestrator\","
        "\"arguments\":[{\"name\":\"subcommand\",\"description\":"
        "\"to_sdk, to_sdk_cli, or to_server\",\"required\":false}]}]}",
        NULL, root_val);
  }
  if (strcmp(method, "sampling/createMessage") == 0) {
    return send_stdio_rpc_response_cleanup(
        id_val,
        "{\"role\":\"assistant\",\"content\":{\"type\":\"text\","
        "\"text\":\"Sampled "
        "message\"},\"model\":\"test-model\",\"stopReason\":\"endSeq\"}",
        NULL, root_val);
  }
  if (strcmp(method, "prompts/get") == 0) {
    JSON_Object *params = NULL;
    rc = rpc_json_opt_object(root_obj, "params", &params);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    rc = build_mcp_prompts_get(params, &dyn_str);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    return send_stdio_rpc_response_cleanup(id_val, dyn_str, dyn_str, root_val);
  }
  if (strcmp(method, "completion/complete") == 0) {
    return send_stdio_rpc_response_cleanup(
        id_val,
        "{\"completion\":{\"values\":[\"to_sdk\",\"to_sdk_cli\",\"to_"
        "server\","
        "\"to_openapi\",\"to_arazzo\",\"from_arazzo\"],\"hasMore\":false,"
        "\"total\":6}}",
        NULL, root_val);
  }
  if (strcmp(method, "tools/call") == 0) {
    JSON_Object *params = NULL;
    rc = rpc_json_opt_object(root_obj, "params", &params);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    rc = handle_stdio_tool_call(id_val, params);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    return cdd_json_value_free(root_val);
  }
  return send_stdio_rpc_error_cleanup(id_val, -32601, "Method not found",
                                      root_val);

cleanup:;
  return cdd_c_keep_err(rc, cdd_c_keep_err(rpc_serialized_str_free(dyn_str),
                                           cdd_json_value_free(root_val)));
}

/**
 * @brief Runs the Model Context Protocol (MCP) JSON-RPC loop over standard
 * input/output.
 *
 * @param[in] argc Argument count.
 * @param[in] argv Argument vector.
 * @return CDD_C_SUCCESS on success, or error code.
 */
C_CDD_EXPORT cdd_c_error_t serve_mcp_stdio_main(int argc, char **argv) {
  char buffer[65536];
  struct CddMcpToolCallParams dummy_cfg;
  int is_help = 0;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  memset(&dummy_cfg, 0, sizeof(dummy_cfg));
  if (argc > 1) {
    rc = cdd_cli_parse_reflected_argv(argc, argv, 0,
                                      &CddMcpToolCallParams_type_info,
                                      &dummy_cfg, &is_help, NULL);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (is_help) {
      puts("Usage: cdd-c serve_mcp_stdio");
      return CDD_C_SUCCESS;
    }
  }

  while (fgets(buffer, sizeof(buffer), stdin)) {
    rc = handle_stdio_request(buffer);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  return CDD_C_SUCCESS;
}
