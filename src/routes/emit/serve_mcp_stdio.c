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
#include "functions/parse/fs.h"
#include <parson.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

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
  char *id_str = id_val ? json_serialize_to_string(id_val) : NULL;
  printf("{\"jsonrpc\":\"2.0\",\"error\":{\"code\":%d,\"message\":\"%s\"},"
         "\"id\":%s}\n",
         code, msg, id_str ? id_str : "null");
  if (id_str)
    json_free_serialized_string(id_str);
  fflush(stdout);
  return CDD_C_SUCCESS;
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
  char *id_str = id_val ? json_serialize_to_string(id_val) : NULL;
  printf("{\"jsonrpc\":\"2.0\",\"result\":%s,\"id\":%s}\n", result_json,
         id_str ? id_str : "null");
  if (id_str) {
    json_free_serialized_string(id_str);
  }
  fflush(stdout);
  return CDD_C_SUCCESS;
}

/**
 * @brief Builds a dynamic JSON result string for `resources/read` over stdio.
 *
 * @param[in] params Optional JSON-RPC params object containing `"uri"`.
 * @param[out] out_json Newly allocated JSON string for the result object.
 * @return CDD_C_SUCCESS on success, or error code.
 */
static cdd_c_error_t build_stdio_resources_read(const JSON_Object *params,
                                                char **out_json) {
  const char *uri = json_object_get_string(params, "uri");
  const char *target_uri = uri ? uri : "file:///openapi.json";
  const char *file_path = NULL;
  char *content = NULL;
  size_t sz = 0;
  JSON_Value *res_val = json_value_init_object();
  JSON_Object *res_obj = json_value_get_object(res_val);
  JSON_Value *arr_val = json_value_init_array();
  JSON_Array *arr = json_value_get_array(arr_val);
  JSON_Value *item_val = json_value_init_object();
  JSON_Object *item_obj = json_value_get_object(item_val);
  char *serialized = NULL;

  if (strncmp(target_uri, "file:///", 8) == 0) {
    file_path = target_uri + 8;
  } else if (strncmp(target_uri, "file://", 7) == 0) {
    file_path = target_uri + 7;
  } else {
    file_path = target_uri;
  }

  json_object_set_string(item_obj, "uri", target_uri);
  json_object_set_string(item_obj, "mimeType", "application/json");
  if (read_to_file(file_path, "rb", &content, &sz) == CDD_C_SUCCESS) {
    json_object_set_string(item_obj, "text", content);
    C_CDD_FREE(content);
  } else {
    json_object_set_string(
        item_obj, "text",
        "{\"openapi\":\"3.2.0\",\"info\":{\"title\":\"cdd-c\",\"version\":"
        "\"0.0.3\"},\"paths\":{}}");
  }
  json_array_append_value(arr, item_val);
  json_object_set_value(res_obj, "contents", arr_val);
  serialized = json_serialize_to_string(res_val);
  json_value_free(res_val);
  *out_json = serialized;
  return CDD_C_SUCCESS;
}

/**
 * @brief Handles an MCP stdio request.
 *
 * @param[in] body The JSON-RPC request body string.
 * @return CDD_C_SUCCESS on success, or error code.
 */
C_CDD_EXPORT cdd_c_error_t handle_stdio_request(const char *body) {
  JSON_Value *root_val;
  JSON_Object *root_obj;
  const char *method;
  JSON_Value *id_val;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  if (!body) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  root_val = json_parse_string(body);
  if (!root_val) {
    return send_stdio_rpc_error(NULL, -32700, "Parse error");
  }

  root_obj = json_value_get_object(root_val);
  method = json_object_get_string(root_obj, "method");
  id_val = json_object_get_value(root_obj, "id");

  if (!method) {
    json_value_free(root_val);
    return send_stdio_rpc_error(id_val, -32600, "Invalid Request");
  }

  if (strcmp(method, "version") == 0) {
    rc = send_stdio_rpc_response(id_val, "\"0.0.3\"");
  } else if (strcmp(method, "initialize") == 0) {
    rc = send_stdio_rpc_response(
        id_val,
        "{\"protocolVersion\":\"2024-11-05\",\"capabilities\":{"
        "\"tools\":{\"listChanged\":true},\"resources\":{\"subscribe\":"
        "true,\"listChanged\":true},\"prompts\":{\"listChanged\":true},"
        "\"logging\":{}},\"serverInfo\":{\"name\":\"cdd-c\",\"version\":"
        "\"0.0.3\"}}");
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
    JSON_Object *params = json_object_get_object(root_obj, "params");
    char *res_str = NULL;
    rc = build_stdio_resources_read(params, &res_str);
    rc = send_stdio_rpc_response(id_val, res_str);
    json_free_serialized_string(res_str);
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
    JSON_Object *params = json_object_get_object(root_obj, "params");
    const char *prompt_name =
        params ? json_object_get_string(params, "name") : NULL;
    char prompt_buf[1024];
    CDD_SNPRINTF(
        prompt_buf, sizeof(prompt_buf),
        "{\"description\":\"Prompt %s\",\"messages\":[{\"role\":\"user\","
        "\"content\":{\"type\":\"text\",\"text\":\"Execute %s\"}}]}",
        prompt_name ? prompt_name : "generate_sdk",
        prompt_name ? prompt_name : "generate_sdk");
    rc = send_stdio_rpc_response(id_val, prompt_buf);
  } else if (strcmp(method, "completion/complete") == 0) {
    rc = send_stdio_rpc_response(
        id_val,
        "{\"completion\":{\"values\":[\"to_sdk\",\"to_sdk_cli\",\"to_server\","
        "\"to_openapi\",\"to_arazzo\",\"from_arazzo\"],\"hasMore\":false,"
        "\"total\":6}}");
  } else if (strcmp(method, "tools/call") == 0) {
    JSON_Object *params = json_object_get_object(root_obj, "params");
    const char *name = params ? json_object_get_string(params, "name") : NULL;
    JSON_Object *arguments =
        params ? json_object_get_object(params, "arguments") : NULL;

    if (!name || !arguments) {
      rc =
          send_stdio_rpc_error(id_val, -32602, "Invalid params for tools/call");
    } else if (strcmp(name, "to_openapi") == 0) {
      const char *input = json_object_get_string(arguments, "input");
      const char *output = json_object_get_string(arguments, "output");
      if (!input || !output) {
        rc = send_stdio_rpc_error(id_val, -32602,
                                  "Invalid arguments for to_openapi");
      } else {
        cdd_to_openapi_config_t cfg;
        cdd_c_error_t rc_rpc;
        memset(&cfg, 0, sizeof(cfg));
        cfg.input = input;
        cfg.output = output;
        rc_rpc = cdd_generate_to_openapi(&cfg);
        if (rc_rpc != CDD_C_SUCCESS) {
          rc =
              send_stdio_rpc_error(id_val, -32603, "OpenAPI generation failed");
        } else {
          rc = send_stdio_rpc_response(
              id_val, "{\"content\":[{\"type\":\"text\",\"text\":\"OpenAPI "
                      "generation successful\"}],\"isError\":false}");
        }
      }
    } else if (strcmp(name, "to_docs_json") == 0) {
      const char *input = json_object_get_string(arguments, "input");
      const char *output = json_object_get_string(arguments, "output");
      if (!input) {
        rc = send_stdio_rpc_error(id_val, -32602,
                                  "Invalid arguments for to_docs_json");
      } else {
        cdd_docs_json_config_t cfg;
        cdd_c_error_t rc_rpc;
        memset(&cfg, 0, sizeof(cfg));
        cfg.input = input;
        cfg.output = output;
        if (json_object_get_boolean(arguments, "no_imports") == 1) {
          cfg.no_imports = CDD_TRUE;
        }
        if (json_object_get_boolean(arguments, "no_wrapping") == 1) {
          cfg.no_wrapping = CDD_TRUE;
        }
        rc_rpc = cdd_generate_docs_json(&cfg);
        if (rc_rpc != CDD_C_SUCCESS) {
          rc = send_stdio_rpc_error(id_val, -32603, "Docs generation failed");
        } else {
          rc = send_stdio_rpc_response(
              id_val, "{\"content\":[{\"type\":\"text\",\"text\":\"Docs "
                      "generation successful\"}],\"isError\":false}");
        }
      }
    } else if (strcmp(name, "to_arazzo") == 0) {
      cdd_to_arazzo_config_t cfg;
      memset(&cfg, 0, sizeof(cfg));
      cfg.input = json_object_get_string(arguments, "input");
      cfg.output = json_object_get_string(arguments, "output");
      if (cdd_generate_to_arazzo(&cfg) != CDD_C_SUCCESS) {
        rc = send_stdio_rpc_error(id_val, -32603, "Arazzo extraction failed");
      } else {
        rc = send_stdio_rpc_response(
            id_val, "{\"content\":[{\"type\":\"text\",\"text\":\"Arazzo "
                    "extraction successful\"}],\"isError\":false}");
      }
    } else if (strcmp(name, "from_arazzo") == 0) {
      cdd_from_arazzo_config_t cfg;
      const char *subcmd = json_object_get_string(arguments, "subcommand");
      if (!subcmd)
        subcmd = "";
      memset(&cfg, 0, sizeof(cfg));
      if (strcmp(subcmd, "to_sdk_cli") == 0) {
        cfg.subcommand = CDD_ARAZZO_SUBCOMMAND_TO_SDK_CLI;
      } else if (strcmp(subcmd, "to_server") == 0) {
        cfg.subcommand = CDD_ARAZZO_SUBCOMMAND_TO_SERVER;
      } else {
        cfg.subcommand = CDD_ARAZZO_SUBCOMMAND_TO_SDK;
      }
      cfg.input = json_object_get_string(arguments, "input");
      cfg.output = json_object_get_string(arguments, "output");
      if (cdd_generate_from_arazzo(&cfg) != CDD_C_SUCCESS) {
        rc = send_stdio_rpc_error(id_val, -32603, "Arazzo generation failed");
      } else {
        rc = send_stdio_rpc_response(
            id_val, "{\"content\":[{\"type\":\"text\",\"text\":\"Arazzo "
                    "generation successful\"}],\"isError\":false}");
      }
    } else {
      rc = send_stdio_rpc_error(id_val, -32601, "Tool not found");
    }
  } else {
    rc = send_stdio_rpc_error(id_val, -32601, "Method not found");
  }

  json_value_free(root_val);
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
  cdd_c_error_t rc_rpc = CDD_C_SUCCESS;
  (void)argc;
  (void)argv;

  while (fgets(buffer, sizeof(buffer), stdin)) {
    rc_rpc = handle_stdio_request(buffer);
  }
  return rc_rpc;
}
