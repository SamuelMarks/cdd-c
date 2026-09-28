/**
 * @file serve_mcp_stdio.c
 * @brief Implementation of MCP stdio server transport.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"
#include "c_cdd/log.h"

#ifndef __wasi__
#include "serve_json_rpc.h"
#include "../parse/cli.h"
#include <parson.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

#ifdef CDD_BUILD_TESTS
extern C_CDD_EXPORT int g_serve_json_rpc_fail_handle;
#endif

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

#ifdef CDD_BUILD_TESTS
  if (g_serve_json_rpc_fail_handle) {
    return CDD_C_ERROR_UNKNOWN;
  }
#endif

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
    rc = send_stdio_rpc_response(id_val, "\"0.0.2\"");
  } else if (strcmp(method, "initialize") == 0) {
    rc = send_stdio_rpc_response(
        id_val,
        "{\"protocolVersion\":\"2024-11-05\",\"capabilities\":{"
        "\"tools\":{\"listChanged\":true},\"resources\":{\"subscribe\":"
        "true,\"listChanged\":true},\"prompts\":{\"listChanged\":true},"
        "\"logging\":{}},\"serverInfo\":{\"name\":\"cdd-c\",\"version\":"
        "\"0.0.2\"}}");
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
        id_val, "{\"tools\":[{\"name\":\"to_openapi\",\"description\":"
                "\"Generate OpenAPI spec from "
                "code\",\"inputSchema\":{\"type\":\"object\"}},{\"name\":\"to_"
                "docs_json\",\"description\":\"Generate JSON "
                "docs\",\"inputSchema\":{\"type\":\"object\"}}]}");
  } else if (strcmp(method, "resources/list") == 0) {
    rc = send_stdio_rpc_response(
        id_val, "{\"resources\":[{\"uri\":\"file:///openapi.json\",\"name\":"
                "\"OpenAPI Spec\",\"mimeType\":\"application/json\"}]}");
  } else if (strcmp(method, "roots/list") == 0) {
    rc = send_stdio_rpc_response(
        id_val, "{\"roots\":[{\"uri\":\"file:///\",\"name\":\"workspace\"}]}");
  } else if (strcmp(method, "resources/templates/list") == 0) {
    rc = send_stdio_rpc_response(id_val, "{\"resourceTemplates\":[]}");
  } else if (strcmp(method, "resources/read") == 0) {
    rc = send_stdio_rpc_response(
        id_val, "{\"contents\":[{\"uri\":\"file:///openapi.json\",\"mimeType\":"
                "\"application/json\",\"text\":\"{}\"},{\"uri\":\"file:///"
                "image.png\",\"mimeType\":\"image/png\",\"blob\":"
                "\"iVBORw0KGgo=\"}]}");
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
        id_val, "{\"prompts\":[{\"name\":\"generate_sdk\",\"description\":"
                "\"Generate SDK "
                "prompt\",\"arguments\":[{\"name\":\"language\","
                "\"description\":\"Target language\",\"required\":true}]}]}");
  } else if (strcmp(method, "sampling/createMessage") == 0) {
    rc = send_stdio_rpc_response(
        id_val,
        "{\"role\":\"assistant\",\"content\":{\"type\":\"text\","
        "\"text\":\"Sampled "
        "message\"},\"model\":\"test-model\",\"stopReason\":\"endSeq\"}");
  } else if (strcmp(method, "prompts/get") == 0) {
    rc = send_stdio_rpc_response(
        id_val,
        "{\"description\":\"Generate SDK\",\"messages\":[{\"role\":\"user\","
        "\"content\":{\"type\":\"text\",\"text\":\"Generate SDK\"}}]}");
  } else if (strcmp(method, "completion/complete") == 0) {
    rc = send_stdio_rpc_response(
        id_val, "{\"completion\":{\"values\":[\"example\"],\"hasMore\":false,"
                "\"total\":1}}");
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
        char *argv_call[5];
        cdd_c_error_t rc_rpc;
        argv_call[0] = (char *)(size_t) "to_openapi";
        argv_call[1] = (char *)(size_t) "-i";
        argv_call[2] = (char *)(size_t)input;
        argv_call[3] = (char *)(size_t) "-o";
        argv_call[4] = (char *)(size_t)output;
        rc_rpc = to_openapi_cli_main(5, argv_call);
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
        char *argv_call[10];
        int argc_call = 0;
        cdd_c_error_t rc_rpc;
        argv_call[argc_call++] = (char *)(size_t) "to_docs_json";
        argv_call[argc_call++] = (char *)(size_t) "-i";
        argv_call[argc_call++] = (char *)(size_t)input;
        if (output) {
          argv_call[argc_call++] = (char *)(size_t) "-o";
          argv_call[argc_call++] = (char *)(size_t)output;
        }
        if (json_object_get_boolean(arguments, "no_imports") == 1) {
          argv_call[argc_call++] = (char *)(size_t) "--no-imports";
        }
        if (json_object_get_boolean(arguments, "no_wrapping") == 1) {
          argv_call[argc_call++] = (char *)(size_t) "--no-wrapping";
        }
        rc_rpc = to_docs_json_cli_main(argc_call, argv_call);
        if (rc_rpc != CDD_C_SUCCESS) {
          rc = send_stdio_rpc_error(id_val, -32603, "Docs generation failed");
        } else {
          rc = send_stdio_rpc_response(
              id_val, "{\"content\":[{\"type\":\"text\",\"text\":\"Docs "
                      "generation successful\"}],\"isError\":false}");
        }
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
 * @brief Executes the MCP stdio main operation.
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
    if (rc_rpc != CDD_C_SUCCESS) {
      return rc_rpc;
    }
  }
  return rc_rpc;
}

#else
cdd_c_error_t handle_stdio_request(const char *body) {
  (void)body;
  return CDD_C_ERROR_UNKNOWN;
}
cdd_c_error_t serve_mcp_stdio_main(int argc, char **argv) {
  (void)argc;
  (void)argv;
  return CDD_C_ERROR_UNKNOWN;
}
#endif
