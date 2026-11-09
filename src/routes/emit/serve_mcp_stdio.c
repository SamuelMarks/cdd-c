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

cdd_c_error_t CddJsonRpcResponse_to_json(const struct CddJsonRpcResponse *resp,
                                         char **out_json) {
  JSON_Value *root_val = NULL;
  JSON_Object *root_obj = NULL;
  JSON_Value *id_copy = NULL;
  cdd_c_error_t rc;

  rc = cdd_json_value_init_object(&root_val);
  rc = cdd_json_value_get_object(root_val, &root_obj);
  rc = cdd_json_object_set_string(root_obj, "jsonrpc", resp->jsonrpc);
  if (resp->result_json) {
    JSON_Value *res_val = NULL;
    rc = cdd_json_parse_string(resp->result_json, &res_val);
    rc = cdd_json_object_set_value(root_obj, "result", res_val);
  } else {
    JSON_Value *err_val = NULL;
    JSON_Object *err_obj = NULL;
    rc = cdd_json_value_init_object(&err_val);
    rc = cdd_json_value_get_object(err_val, &err_obj);
    rc = cdd_reflect_to_json_object(&resp->error, &CddJsonRpcErrorObj_type_info,
                                    err_obj);
    rc = cdd_json_object_set_value(root_obj, "error", err_val);
  }
  if (resp->id_val) {
    rc = cdd_json_value_deep_copy(resp->id_val, &id_copy);
    rc = cdd_json_object_set_value(root_obj, "id", id_copy);
  } else {
    rc = cdd_json_object_set_null(root_obj, "id");
  }
  rc = cdd_json_serialize_to_string(root_val, out_json);
  g_cdd_last_alloc_err = cdd_json_value_free(root_val);
  return rc;
}

cdd_c_error_t
CddMcpToolCallParams_from_jsonObject(const JSON_Object *obj,
                                     struct CddMcpToolCallParams *out) {
  const char *name = NULL;
  JSON_Object *arguments = NULL;
  memset(out, 0, sizeof(*out));
  if (!obj)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (cdd_json_object_get_string(obj, "name", &name) != CDD_C_SUCCESS)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (cdd_json_object_get_object(obj, "arguments", &arguments) != CDD_C_SUCCESS)
    return CDD_C_ERROR_INVALID_ARGUMENT;
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
  printf("%s\n", json_out);
  free(json_out);
  fflush(stdout);
  return rc;
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
  printf("%s\n", json_out);
  free(json_out);
  fflush(stdout);
  return rc;
}

cdd_c_error_t build_mcp_resources_read(const JSON_Object *params,
                                       char **out_json) {
  const char *uri = NULL;
  const char *target_uri;
  const char *file_path = NULL;
  char *content = NULL;
  size_t sz = 0;
  JSON_Value *res_val = NULL;
  JSON_Value *arr_val = NULL;
  JSON_Value *item_val = NULL;
  char *serialized = NULL;
  cdd_c_error_t rc;

  rc = cdd_json_object_get_string(params, "uri", &uri);
  target_uri = uri ? uri : "file:///openapi.json";

  rc = cdd_json_value_init_object(&res_val);
  rc = cdd_json_value_init_array(&arr_val);
  rc = cdd_json_value_init_object(&item_val);

  if (strncmp(target_uri, "file:///", 8) == 0) {
    file_path = target_uri + 8;
  } else if (strncmp(target_uri, "file://", 7) == 0) {
    file_path = target_uri + 7;
  } else {
    file_path = target_uri;
  }

  rc = cdd_json_object_set_string(item_val->object_val, "uri", target_uri);
  rc = cdd_json_object_set_string(item_val->object_val, "mimeType",
                                  "application/json");
  if (read_to_file(file_path, "rb", &content, &sz) == CDD_C_SUCCESS) {
    rc = cdd_json_object_set_string(item_val->object_val, "text", content);
    C_CDD_FREE(content);
  } else {
    rc = cdd_json_object_set_string(
        item_val->object_val, "text",
        "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"cdd-c\",\"version\":"
        "\"0.0.5\"},\"paths\":{}}");
  }
  rc = cdd_json_array_append_value(arr_val->array_val, item_val);
  rc = cdd_json_object_set_value(res_val->object_val, "contents", arr_val);
  rc = cdd_json_serialize_to_string(res_val, &serialized);
  g_cdd_last_alloc_err = cdd_json_value_free(res_val);
  *out_json = serialized;
  return rc;
}

cdd_c_error_t build_mcp_prompts_get(const JSON_Object *params,
                                    char **out_json) {
  const char *prompt_name = NULL;
  const char *name;
  char desc_buf[256];
  char text_buf[256];
  JSON_Value *res_val = NULL;
  JSON_Value *msgs_val = NULL;
  JSON_Value *msg_val = NULL;
  JSON_Value *cnt_val = NULL;
  char *serialized = NULL;
  cdd_c_error_t rc;

  rc = cdd_json_object_get_string(params, "name", &prompt_name);
  name = prompt_name ? prompt_name : "generate_sdk";
#if defined(_MSC_VER)
  sprintf_s(desc_buf, sizeof(desc_buf), "Prompt %s", name);
  sprintf_s(text_buf, sizeof(text_buf), "Execute %s", name);
#else
  CDD_SNPRINTF(desc_buf, sizeof(desc_buf), "Prompt %s", name);
  CDD_SNPRINTF(text_buf, sizeof(text_buf), "Execute %s", name);
#endif
  rc = cdd_json_value_init_object(&res_val);
  rc = cdd_json_value_init_array(&msgs_val);
  rc = cdd_json_value_init_object(&msg_val);
  rc = cdd_json_value_init_object(&cnt_val);
  rc = cdd_json_object_set_string(res_val->object_val, "description", desc_buf);
  rc = cdd_json_object_set_string(msg_val->object_val, "role", "user");
  rc = cdd_json_object_set_string(cnt_val->object_val, "type", "text");
  rc = cdd_json_object_set_string(cnt_val->object_val, "text", text_buf);
  rc = cdd_json_object_set_value(msg_val->object_val, "content", cnt_val);
  rc = cdd_json_array_append_value(msgs_val->array_val, msg_val);
  rc = cdd_json_object_set_value(res_val->object_val, "messages", msgs_val);
  rc = cdd_json_serialize_to_string(res_val, &serialized);
  g_cdd_last_alloc_err = cdd_json_value_free(res_val);
  *out_json = serialized;
  return rc;
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
  cdd_c_error_t rc = CDD_C_SUCCESS;

  if (!body) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  if (cdd_json_parse_string(body, &root_val) != CDD_C_SUCCESS) {
    return send_stdio_rpc_error(NULL, -32700, "Parse error");
  }

  rc = cdd_json_value_get_object(root_val, &root_obj);
  rc = cdd_json_object_get_string(root_obj, "method", &method);
  rc = cdd_json_object_get_value(root_obj, "id", &id_val);

  if (!method) {
    g_cdd_last_alloc_err = cdd_json_value_free(root_val);
    return send_stdio_rpc_error(id_val, -32600, "Invalid Request");
  }

  if (strcmp(method, "version") == 0) {
    rc = send_stdio_rpc_response(id_val, "\"0.0.5\"");
  } else if (strcmp(method, "initialize") == 0) {
    rc = send_stdio_rpc_response(
        id_val,
        "{\"protocolVersion\":\"2024-11-05\",\"capabilities\":{"
        "\"tools\":{\"listChanged\":true},\"resources\":{\"subscribe\":"
        "true,\"listChanged\":true},\"prompts\":{\"listChanged\":true},"
        "\"logging\":{}},\"serverInfo\":{\"name\":\"cdd-c\",\"version\":"
        "\"0.0.5\"}}");
  } else if (strcmp(method, "notifications/initialized") == 0) {
  } else if (strcmp(method, "notifications/progress") == 0) {
  } else if (strcmp(method, "notifications/message") == 0) {
  } else if (strcmp(method, "notifications/cancelled") == 0) {
  } else if (strcmp(method, "notifications/roots/list_changed") == 0) {
  } else if (strcmp(method, "notifications/resources/list_changed") == 0) {
  } else if (strcmp(method, "notifications/tools/list_changed") == 0) {
  } else if (strcmp(method, "notifications/prompts/list_changed") == 0) {
  } else if (strcmp(method, "logging/setLevel") == 0) {
    rc = send_stdio_rpc_response(id_val, "{}");
  } else if (strcmp(method, "ping") == 0) {
    rc = send_stdio_rpc_response(id_val, "{}");
  } else if (strcmp(method, "tools/list") == 0) {
    rc = send_stdio_rpc_response(
        id_val,
        "{\"tools\":[{\"name\":\"to_openapi\",\"description\":\"Generate "
        "OpenAPI spec from code\",\"inputSchema\":{\"type\":\"object\"}},"
        "{\"name\":\"to_docs_json\",\"description\":\"Generate JSON docs\","
        "\"inputSchema\":{\"type\":\"object\"}},{\"name\":\"to_arazzo\","
        "\"description\":\"Extract Arazzo spec from C code\",\"inputSchema\":"
        "{\"type\":\"object\"}},{\"name\":\"from_arazzo\",\"description\":"
        "\"Generate C workflows from Arazzo spec\",\"inputSchema\":"
        "{\"type\":\"object\"}}]}");
  } else if (strcmp(method, "resources/list") == 0) {
    rc = send_stdio_rpc_response(
        id_val,
        "{\"resources\":[{\"uri\":\"file:///openapi.json\",\"name\":\"OpenAPI "
        "Spec\",\"mimeType\":\"application/json\"},{\"uri\":"
        "\"file:///arazzo.json\",\"name\":\"Arazzo Workflow Spec\","
        "\"mimeType\":\"application/json\"}]}");
  } else if (strcmp(method, "roots/list") == 0) {
    rc = send_stdio_rpc_response(
        id_val,
        "{\"roots\":[{\"uri\":\"file:///workspace\",\"name\":\"workspace\"}]}");
  } else if (strcmp(method, "resources/templates/list") == 0) {
    rc = send_stdio_rpc_response(
        id_val,
        "{\"resourceTemplates\":[{\"uriTemplate\":\"file:///{path}\","
        "\"name\":\"Workspace File\",\"mimeType\":\"application/json\"}]}");
  } else if (strcmp(method, "resources/read") == 0) {
    JSON_Object *params = NULL;
    char *res_str = NULL;
    rc = cdd_json_object_get_object(root_obj, "params", &params);
    rc = build_mcp_resources_read(params, &res_str);
    rc = send_stdio_rpc_response(id_val, res_str);
    g_cdd_last_alloc_err = cdd_json_free_serialized_string(res_str);
  } else if (strcmp(method, "resources/subscribe") == 0) {
    rc = send_stdio_rpc_response(id_val, "{}");
  } else if (strcmp(method, "resources/unsubscribe") == 0) {
    rc = send_stdio_rpc_response(id_val, "{}");
  } else if (strcmp(method, "tools/read_image") == 0) {
    rc = send_stdio_rpc_response(
        id_val, "{\"content\":[{\"type\":\"image\",\"data\":\"iVBORw0KGgo=\","
                "\"mimeType\":\"image/png\",\"annotations\":{\"audience\":["
                "\"user\"],\"priority\":1.0}}]}");
  } else if (strcmp(method, "prompts/list") == 0) {
    rc = send_stdio_rpc_response(
        id_val,
        "{\"prompts\":[{\"name\":\"generate_sdk\",\"description\":\"Generate "
        "SDK prompt\",\"arguments\":[{\"name\":\"language\",\"description\":"
        "\"Target "
        "language\",\"required\":true}]},{\"name\":\"generate_arazzo\","
        "\"description\":\"Generate Arazzo workflow orchestrator\","
        "\"arguments\":[{\"name\":\"subcommand\",\"description\":"
        "\"to_sdk, to_sdk_cli, or to_server\",\"required\":false}]}]}");
  } else if (strcmp(method, "sampling/createMessage") == 0) {
    rc = send_stdio_rpc_response(
        id_val,
        "{\"role\":\"assistant\",\"content\":{\"type\":\"text\","
        "\"text\":\"Sampled "
        "message\"},\"model\":\"test-model\",\"stopReason\":\"endSeq\"}");
  } else if (strcmp(method, "prompts/get") == 0) {
    JSON_Object *params = NULL;
    char *prompt_str = NULL;
    rc = cdd_json_object_get_object(root_obj, "params", &params);
    rc = build_mcp_prompts_get(params, &prompt_str);
    rc = send_stdio_rpc_response(id_val, prompt_str);
    g_cdd_last_alloc_err = cdd_json_free_serialized_string(prompt_str);
  } else if (strcmp(method, "completion/complete") == 0) {
    rc = send_stdio_rpc_response(
        id_val,
        "{\"completion\":{\"values\":[\"to_sdk\",\"to_sdk_cli\",\"to_server\","
        "\"to_openapi\",\"to_arazzo\",\"from_arazzo\"],\"hasMore\":false,"
        "\"total\":6}}");
  } else if (strcmp(method, "tools/call") == 0) {
    JSON_Object *params = NULL;
    struct CddMcpToolCallParams tc_params;
    rc = cdd_json_object_get_object(root_obj, "params", &params);

    if (CddMcpToolCallParams_from_jsonObject(params, &tc_params) !=
        CDD_C_SUCCESS) {
      rc =
          send_stdio_rpc_error(id_val, -32602, "Invalid params for tools/call");
    } else if (strcmp(tc_params.name, "to_openapi") == 0) {
      cdd_to_openapi_config_t cfg;
      memset(&cfg, 0, sizeof(cfg));
      rc = cdd_reflect_from_json_object(tc_params.arguments,
                                        &cdd_to_openapi_config_type_info, &cfg);
      if (!cfg.input || !cfg.output) {
        rc = send_stdio_rpc_error(id_val, -32602,
                                  "Invalid arguments for to_openapi");
      } else {
        cdd_c_error_t rc_rpc = cdd_generate_to_openapi(&cfg);
        if (rc_rpc != CDD_C_SUCCESS) {
          rc =
              send_stdio_rpc_error(id_val, -32603, "OpenAPI generation failed");
        } else {
          rc = send_stdio_rpc_response(
              id_val, "{\"content\":[{\"type\":\"text\",\"text\":\"OpenAPI "
                      "generation successful\"}],\"isError\":false}");
        }
      }
      rc = cdd_reflect_struct_cleanup(&cfg, &cdd_to_openapi_config_type_info);
    } else if (strcmp(tc_params.name, "to_docs_json") == 0) {
      cdd_docs_json_config_t cfg;
      memset(&cfg, 0, sizeof(cfg));
      rc = cdd_reflect_from_json_object(tc_params.arguments,
                                        &cdd_docs_json_config_type_info, &cfg);
      if (!cfg.input) {
        rc = send_stdio_rpc_error(id_val, -32602,
                                  "Invalid arguments for to_docs_json");
      } else {
        cdd_c_error_t rc_rpc = cdd_generate_docs_json(&cfg);
        if (rc_rpc != CDD_C_SUCCESS) {
          rc = send_stdio_rpc_error(id_val, -32603, "Docs generation failed");
        } else {
          rc = send_stdio_rpc_response(
              id_val, "{\"content\":[{\"type\":\"text\",\"text\":\"Docs "
                      "generation successful\"}],\"isError\":false}");
        }
      }
      rc = cdd_reflect_struct_cleanup(&cfg, &cdd_docs_json_config_type_info);
    } else if (strcmp(tc_params.name, "to_arazzo") == 0) {
      cdd_to_arazzo_config_t cfg;
      memset(&cfg, 0, sizeof(cfg));
      rc = cdd_reflect_from_json_object(tc_params.arguments,
                                        &cdd_to_arazzo_config_type_info, &cfg);
      if (cdd_generate_to_arazzo(&cfg) != CDD_C_SUCCESS) {
        rc = send_stdio_rpc_error(id_val, -32603, "Arazzo extraction failed");
      } else {
        rc = send_stdio_rpc_response(
            id_val, "{\"content\":[{\"type\":\"text\",\"text\":\"Arazzo "
                    "extraction successful\"}],\"isError\":false}");
      }
      rc = cdd_reflect_struct_cleanup(&cfg, &cdd_to_arazzo_config_type_info);
    } else if (strcmp(tc_params.name, "from_arazzo") == 0) {
      cdd_from_arazzo_config_t cfg;
      const char *subcmd = NULL;
      rc = cdd_json_object_get_string(tc_params.arguments, "subcommand",
                                      &subcmd);
      if (!subcmd)
        subcmd = "";
      memset(&cfg, 0, sizeof(cfg));
      rc = cdd_reflect_from_json_object(
          tc_params.arguments, &cdd_from_arazzo_config_type_info, &cfg);
      if (strcmp(subcmd, "to_sdk_cli") == 0) {
        cfg.subcommand = CDD_ARAZZO_SUBCOMMAND_TO_SDK_CLI;
      } else if (strcmp(subcmd, "to_server") == 0) {
        cfg.subcommand = CDD_ARAZZO_SUBCOMMAND_TO_SERVER;
      } else {
        cfg.subcommand = CDD_ARAZZO_SUBCOMMAND_TO_SDK;
      }
      if (cdd_generate_from_arazzo(&cfg) != CDD_C_SUCCESS) {
        rc = send_stdio_rpc_error(id_val, -32603, "Arazzo generation failed");
      } else {
        rc = send_stdio_rpc_response(
            id_val, "{\"content\":[{\"type\":\"text\",\"text\":\"Arazzo "
                    "generation successful\"}],\"isError\":false}");
      }
      rc = cdd_reflect_struct_cleanup(&cfg, &cdd_from_arazzo_config_type_info);
    } else {
      rc = send_stdio_rpc_error(id_val, -32601, "Tool not found");
    }
  } else {
    rc = send_stdio_rpc_error(id_val, -32601, "Method not found");
  }

  g_cdd_last_alloc_err = cdd_json_value_free(root_val);
  return rc;
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
  const char *sug = NULL;
  cdd_c_error_t rc_rpc = cdd_cli_suggest_closest(
      argv[0], (const char *const *)argv, (size_t)argc, &sug);

  while (fgets(buffer, sizeof(buffer), stdin)) {
    rc_rpc = handle_stdio_request(buffer);
  }
  return rc_rpc;
}
