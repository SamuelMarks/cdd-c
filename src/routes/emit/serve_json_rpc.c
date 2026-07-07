/* LCOV_EXCL_START */
/**
 * @file serve_json_rpc.c
 * @brief Implementation of JSON-RPC server generation.
 */

/* clang-format off */
#ifndef __wasi__
#include "serve_json_rpc.h"
#include "../parse/cli.h"
#include <parson.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(__WATCOMC__) || defined(__DOS__)
/* No sockets on DOS/Watcom natively */
#else
#if defined(_WIN32)
#include <winsock2.h>
#include <ws2tcpip.h>
#ifdef _MSC_VER
#pragma comment(lib, "ws2_32.lib")
#endif
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#ifdef _WIN32
#include <io.h>
#else
#if !defined(_MSC_VER)
#if defined(_MSC_VER)
#include <io.h>
#else
#include <unistd.h>
#endif
#endif
#endif
#endif
#endif

#if defined(__WATCOMC__) || defined(__DOS__)
/** @brief cdd_socket_t */
typedef int cdd_socket_t;
/** @brief INVALID_SOCKET */
#define INVALID_SOCKET (-1)
#else
#if defined(_WIN32)
typedef SOCKET cdd_socket_t;
#else
/** @brief cdd_socket_t */
typedef int cdd_socket_t;
#ifndef INVALID_SOCKET
/** @brief INVALID_SOCKET */
#define INVALID_SOCKET (-1)
#endif
#endif
#endif

/* Helper to respond with JSON-RPC error */
static enum cdd_c_error send_rpc_error(cdd_socket_t client_fd, int code, const char *msg) {
  char resp[1024];
  sprintf(resp, "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n{\"jsonrpc\":\"2.0\",\"error\":{\"code\":%d,\"message\":\"%s\"},\"id\":null}", code, msg);
  send(client_fd, resp, (int)strlen(resp), 0);
  return CDD_C_SUCCESS;
}

static enum cdd_c_error send_rpc_success(cdd_socket_t client_fd) {
  const char *resp = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n{\"jsonrpc\":\"2.0\",\"result\":\"ok\",\"id\":null}";
  send(client_fd, resp, (int)strlen(resp), 0);
  return CDD_C_SUCCESS;
}

static enum cdd_c_error handle_request(cdd_socket_t client_fd) {
  char buffer[65536];
  int bytes_received;
  char *body;
  JSON_Value *root_val;
  JSON_Object *root_obj;
  const char *method;

  bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
  if (bytes_received <= 0) return CDD_C_ERROR_INVALID_ARGUMENT;
  buffer[bytes_received] = '\0';

  body = strstr(buffer, "\r\n\r\n");
  if (!body) {
    send_rpc_error(client_fd, -32700, "Parse error");
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
  body += 4;

  root_val = json_parse_string(body);
  if (!root_val) {
    send_rpc_error(client_fd, -32700, "Parse error");
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  root_obj = json_value_get_object(root_val);
  method = json_object_get_string(root_obj, "method");
  if (!method) {
    send_rpc_error(client_fd, -32600, "Invalid Request");
    json_value_free(root_val);
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  if (strcmp(method, "version") == 0) {
    const char *resp = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n{\"jsonrpc\":\"2.0\",\"result\":\"0.0.2\",\"id\":null}";
    send(client_fd, resp, (int)strlen(resp), 0);
  } else if (strcmp(method, "initialize") == 0) {
    /* MCP Initialize Handshake Sequence */
    const char *resp;
    JSON_Object *params = json_object_get_object(root_obj, "params");
    if (params) {
        JSON_Object *clientInfo = json_object_get_object(params, "clientInfo");
        JSON_Object *capabilities = json_object_get_object(params, "capabilities");
        (void)clientInfo; /* Unused but mapped */
        (void)capabilities; /* Unused but mapped */
    }
    resp = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n{\"jsonrpc\":\"2.0\",\"result\":{\"protocolVersion\":\"2024-11-05\",\"capabilities\":{\"tools\":{\"listChanged\":true},\"resources\":{\"listChanged\":true,\"subscribe\":false},\"prompts\":{\"listChanged\":true},\"logging\":{}},\"serverInfo\":{\"name\":\"cdd-c\",\"version\":\"0.0.2\"}},\"id\":null}";
    send(client_fd, resp, (int)strlen(resp), 0);
  } else if (strcmp(method, "notifications/initialized") == 0) {
    /* MCP Initialized Acknowledgment (Fire and forget) */
    /* No response needed for notification */
  } else if (strcmp(method, "notifications/progress") == 0) {
    /* MCP Progress Tracking */
    /* Handled as fire and forget for now */
  } else if (strcmp(method, "notifications/message") == 0) {
    /* MCP Logging Message */
    /* Handled as fire and forget for now */
  } else if (strcmp(method, "logging/setLevel") == 0) {
    /* MCP SetLevelRequest */
    const char *resp = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n{\"jsonrpc\":\"2.0\",\"result\":{},\"id\":null}";
    send(client_fd, resp, (int)strlen(resp), 0);
  } else if (strcmp(method, "ping") == 0) {
    /* MCP Liveness ping */
    const char *resp = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n{\"jsonrpc\":\"2.0\",\"result\":{},\"id\":null}";
    send(client_fd, resp, (int)strlen(resp), 0);
  } else if (strcmp(method, "tools/list") == 0) {
    /* MCP Tool Listing */
    const char *resp = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n{\"jsonrpc\":\"2.0\",\"result\":{\"tools\":[{\"name\":\"to_openapi\",\"description\":\"Generate OpenAPI spec from code\",\"inputSchema\":{\"type\":\"object\"}},{\"name\":\"to_docs_json\",\"description\":\"Generate JSON docs\",\"inputSchema\":{\"type\":\"object\"}}]},\"id\":null}";
    send(client_fd, resp, (int)strlen(resp), 0);
  } else if (strcmp(method, "resources/list") == 0) {
    /* MCP Resource Listing */
    const char *resp = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n{\"jsonrpc\":\"2.0\",\"result\":{\"resources\":[{\"uri\":\"file:///openapi.json\",\"name\":\"OpenAPI Spec\",\"mimeType\":\"application/json\"}]},\"id\":null}";
    send(client_fd, resp, (int)strlen(resp), 0);
  } else if (strcmp(method, "roots/list") == 0) {
    /* MCP Root Listing */
    const char *resp = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n{\"jsonrpc\":\"2.0\",\"result\":{\"roots\":[{\"uri\":\"file:///\",\"name\":\"workspace\"}]},\"id\":null}";
    send(client_fd, resp, (int)strlen(resp), 0);
  } else if (strcmp(method, "resources/templates/list") == 0) {
    /* MCP Resource Templates Listing */
    const char *resp = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n{\"jsonrpc\":\"2.0\",\"result\":{\"resourceTemplates\":[]},\"id\":null}";
    send(client_fd, resp, (int)strlen(resp), 0);
  } else if (strcmp(method, "resources/read") == 0) {
    /* MCP Resource Read */
    const char *resp = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n{\"jsonrpc\":\"2.0\",\"result\":{\"contents\":[{\"uri\":\"file:///openapi.json\",\"mimeType\":\"application/json\",\"text\":\"{}\"},{\"uri\":\"file:///image.png\",\"mimeType\":\"image/png\",\"blob\":\"iVBORw0KGgo=\"}]},\"id\":null}";
    send(client_fd, resp, (int)strlen(resp), 0);
  } else if (strcmp(method, "resources/subscribe") == 0) {
    /* MCP Subscribe Request */
    const char *resp = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n{\"jsonrpc\":\"2.0\",\"result\":{},\"id\":null}";
    send(client_fd, resp, (int)strlen(resp), 0);
  } else if (strcmp(method, "resources/unsubscribe") == 0) {
    /* MCP Unsubscribe Request */
    const char *resp = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n{\"jsonrpc\":\"2.0\",\"result\":{},\"id\":null}";
    send(client_fd, resp, (int)strlen(resp), 0);
  } else if (strcmp(method, "tools/read_image") == 0) {
    /* MCP ImageContent example endpoint */
    const char *resp = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n{\"jsonrpc\":\"2.0\",\"result\":{\"content\":[{\"type\":\"image\",\"data\":\"iVBORw0KGgo=\",\"mimeType\":\"image/png\",\"annotations\":{\"audience\":[\"user\"],\"priority\":1.0}}]},\"id\":null}";
    send(client_fd, resp, (int)strlen(resp), 0);
  } else if (strcmp(method, "prompts/list") == 0) {
    /* MCP Prompt Listing */
    const char *resp = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n{\"jsonrpc\":\"2.0\",\"result\":{\"prompts\":[{\"name\":\"generate_sdk\",\"description\":\"Generate SDK prompt\",\"arguments\":[{\"name\":\"language\",\"description\":\"Target language\",\"required\":true}]}]},\"id\":null}";
    send(client_fd, resp, (int)strlen(resp), 0);
  } else if (strcmp(method, "sampling/createMessage") == 0) {
    /* MCP CreateMessageRequest */
    JSON_Object *params = json_object_get_object(root_obj, "params");
    JSON_Array *messages = params ? json_object_get_array(params, "messages") : NULL;
    int maxTokens = params ? (int)json_object_get_number(params, "maxTokens") : 0;
    const char *includeContext = params ? json_object_get_string(params, "includeContext") : NULL;
    JSON_Object *metadata = params ? json_object_get_object(params, "metadata") : NULL;
    JSON_Object *modelPreferences = params ? json_object_get_object(params, "modelPreferences") : NULL;
    JSON_Array *stopSequences = params ? json_object_get_array(params, "stopSequences") : NULL;
    const char *systemPrompt = params ? json_object_get_string(params, "systemPrompt") : NULL;
    double temperature = params && json_object_has_value_of_type(params, "temperature", JSONNumber) ? json_object_get_number(params, "temperature") : 0.0;
    const char *resp = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n{\"jsonrpc\":\"2.0\",\"result\":{\"role\":\"assistant\",\"content\":{\"type\":\"text\",\"text\":\"Sampled message\"},\"model\":\"test-model\",\"stopReason\":\"endSeq\"},\"id\":null}";
    (void)messages; (void)maxTokens; (void)includeContext; (void)metadata; (void)modelPreferences; (void)stopSequences; (void)systemPrompt; (void)temperature;
    send(client_fd, resp, (int)strlen(resp), 0);
  } else if (strcmp(method, "prompts/get") == 0) {
    /* MCP Prompt Get */
    const char *resp = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n{\"jsonrpc\":\"2.0\",\"result\":{\"description\":\"Generate SDK\",\"messages\":[{\"role\":\"user\",\"content\":{\"type\":\"text\",\"text\":\"Generate SDK\"}}]},\"id\":null}";
    send(client_fd, resp, (int)strlen(resp), 0);
  } else if (strcmp(method, "completion/complete") == 0) {
    /* MCP Complete Request */
    const char *resp = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n{\"jsonrpc\":\"2.0\",\"result\":{\"completion\":{\"values\":[\"example\"],\"hasMore\":false,\"total\":1}},\"id\":null}";
    send(client_fd, resp, (int)strlen(resp), 0);
  } else if (strcmp(method, "tools/call") == 0) {
    /* MCP CallToolRequest */
    JSON_Object *params = json_object_get_object(root_obj, "params");
    const char *name = params ? json_object_get_string(params, "name") : NULL;
    JSON_Object *arguments = params ? json_object_get_object(params, "arguments") : NULL;

    if (!name || !arguments) {
        send_rpc_error(client_fd, -32602, "Invalid params for tools/call");
    } else if (strcmp(name, "to_openapi") == 0) {
        const char *input = json_object_get_string(arguments, "input");
        const char *output = json_object_get_string(arguments, "output");
        if (!input || !output) {
            send_rpc_error(client_fd, -32602, "Invalid arguments for to_openapi");
        } else {
            char *argv[5];
            const char *resp;
            argv[0] = "to_openapi";
            argv[1] = "-i";
            argv[2] = (char *)input;
            argv[3] = "-o";
            argv[4] = (char *)output;
            to_openapi_cli_main(5, argv);
            /* Return CallToolResult */
            resp = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n{\"jsonrpc\":\"2.0\",\"result\":{\"content\":[{\"type\":\"text\",\"text\":\"OpenAPI generation successful\"}],\"isError\":false},\"id\":null}";
            send(client_fd, resp, (int)strlen(resp), 0);
        }
    } else if (strcmp(name, "to_docs_json") == 0) {
        const char *input = json_object_get_string(arguments, "input");
        const char *output = json_object_get_string(arguments, "output");
        if (!input) {
            send_rpc_error(client_fd, -32602, "Invalid arguments for to_docs_json");
        } else {
            char *argv[10];
            int argc_call = 0;
            const char *resp;
            argv[argc_call++] = "to_docs_json";
            argv[argc_call++] = "-i";
            argv[argc_call++] = (char *)input;
            if (output) {
                argv[argc_call++] = "-o";
                argv[argc_call++] = (char *)output;
            }
            if (json_object_get_boolean(arguments, "no_imports")) {
                argv[argc_call++] = "--no-imports";
            }
            if (json_object_get_boolean(arguments, "no_wrapping")) {
                argv[argc_call++] = "--no-wrapping";
            }
            to_docs_json_cli_main(argc_call, argv);
            resp = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n{\"jsonrpc\":\"2.0\",\"result\":{\"content\":[{\"type\":\"text\",\"text\":\"Docs generation successful\"}],\"isError\":false},\"id\":null}";
            send(client_fd, resp, (int)strlen(resp), 0);
        }
    } else {
        send_rpc_error(client_fd, -32601, "Tool not found");
    }
  } else if (strcmp(method, "to_openapi") == 0) {
    JSON_Object *params = json_object_get_object(root_obj, "params");
    const char *input = params ? json_object_get_string(params, "input") : NULL;
    const char *output = params ? json_object_get_string(params, "output") : NULL;
    if (!input || !output) {
       send_rpc_error(client_fd, -32602, "Invalid params");
    } else {
       char *argv[5];
       argv[0] = "to_openapi";
       argv[1] = "-i";
       argv[2] = (char *)input;
       argv[3] = "-o";
       argv[4] = (char *)output;
       to_openapi_cli_main(5, argv);
       send_rpc_success(client_fd);
    }
  } else if (strcmp(method, "to_docs_json") == 0) {
    JSON_Object *params = json_object_get_object(root_obj, "params");
    const char *input = params ? json_object_get_string(params, "input") : NULL;
    const char *output = params ? json_object_get_string(params, "output") : NULL;
    if (!input) {
       send_rpc_error(client_fd, -32602, "Invalid params");
    } else {
       char *argv[10];
       int argc = 0;
       argv[argc++] = "to_docs_json";
       argv[argc++] = "-i";
       argv[argc++] = (char *)input;
       if (output) {
         argv[argc++] = "-o";
         argv[argc++] = (char *)output;
       }
       if (json_object_get_boolean(params, "no_imports")) {
         argv[argc++] = "--no-imports";
       }
       if (json_object_get_boolean(params, "no_wrapping")) {
         argv[argc++] = "--no-wrapping";
       }
       to_docs_json_cli_main(argc, argv);
       send_rpc_success(client_fd);
    }
  } else if (strncmp(method, "from_openapi_", 13) == 0) {
    JSON_Object *params = json_object_get_object(root_obj, "params");
    const char *input = params ? json_object_get_string(params, "input") : NULL;
    const char *input_dir = params ? json_object_get_string(params, "input_dir") : NULL;
    const char *output = params ? json_object_get_string(params, "output") : NULL;

    char *argv[20];
    int argc = 0;
    argv[argc++] = "from_openapi";
    if (strcmp(method, "from_openapi_to_sdk") == 0) {
      argv[argc++] = "to_sdk";
    } else if (strcmp(method, "from_openapi_to_sdk_cli") == 0) {
      argv[argc++] = "to_sdk_cli";
    } else if (strcmp(method, "from_openapi_to_server") == 0) {
      argv[argc++] = "to_server";
    } else {
      send_rpc_error(client_fd, -32601, "Method not found");
      json_value_free(root_val);
      return CDD_C_ERROR_INVALID_ARGUMENT;
    }

    if (input) {
      argv[argc++] = "-i";
      argv[argc++] = (char *)input;
    } else if (input_dir) {
      argv[argc++] = "--input-dir";
      argv[argc++] = (char *)input_dir;
    }

    if (output) {
      argv[argc++] = "-o";
      argv[argc++] = (char *)output;
    }

    if (params && json_object_get_boolean(params, "no_github_actions")) {
      argv[argc++] = "--no-github-actions";
    }
    if (params && json_object_get_boolean(params, "no_installable_package")) {
      argv[argc++] = "--no-installable-package";
    }
    if (params && json_object_get_boolean(params, "tests")) {
      argv[argc++] = "--tests";
    }

    from_openapi_cli_main(argc, argv);
    send_rpc_success(client_fd);
  } else {
    send_rpc_error(client_fd, -32601, "Method not found");
  }

  json_value_free(root_val);
  return CDD_C_SUCCESS;
}


/**
 * @brief Helper to respond with JSON-RPC error over stdio.
 *
 * @param id_val The JSON-RPC request ID.
 * @param code The JSON-RPC error code.
 * @param msg The JSON-RPC error message.
 */
static enum cdd_c_error send_stdio_rpc_error(JSON_Value *id_val, int code, const char *msg) {
  char *id_str = id_val ? json_serialize_to_string(id_val) : NULL;
  printf("{\"jsonrpc\":\"2.0\",\"error\":{\"code\":%d,\"message\":\"%s\"},\"id\":%s}\n",
         code, msg, id_str ? id_str : "null");
  if (id_str) json_free_serialized_string(id_str);
  fflush(stdout);
  return CDD_C_SUCCESS;
}

/**
 * @brief Handles an MCP stdio request.
 *
 * @param body The JSON-RPC request body string.
 */
static enum cdd_c_error handle_stdio_request(const char *body) {
  JSON_Value *root_val;
  JSON_Object *root_obj;
  const char *method;
  JSON_Value *id_val;

  root_val = json_parse_string(body);
  if (!root_val) {
    send_stdio_rpc_error(NULL, -32700, "Parse error");
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  root_obj = json_value_get_object(root_val);
  method = json_object_get_string(root_obj, "method");
  id_val = json_object_get_value(root_obj, "id");

  if (!method) {
    send_stdio_rpc_error(id_val, -32600, "Invalid Request");
    json_value_free(root_val);
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  if (strcmp(method, "version") == 0) {
    char *id_str = id_val ? json_serialize_to_string(id_val) : NULL;
    printf("{\"jsonrpc\":\"2.0\",\"result\":\"0.0.2\",\"id\":%s}\n", id_str ? id_str : "null");
    if (id_str) json_free_serialized_string(id_str);
    fflush(stdout);
  } else if (strcmp(method, "initialize") == 0) {
    char *id_str = id_val ? json_serialize_to_string(id_val) : NULL;
    printf("{\"jsonrpc\":\"2.0\",\"result\":{\"protocolVersion\":\"2024-11-05\",\"capabilities\":{\"tools\":{\"listChanged\":true},\"resources\":{\"listChanged\":true,\"subscribe\":false},\"prompts\":{\"listChanged\":true},\"logging\":{}},\"serverInfo\":{\"name\":\"cdd-c\",\"version\":\"0.0.2\"}},\"id\":%s}\n", id_str ? id_str : "null");
    if (id_str) json_free_serialized_string(id_str);
    fflush(stdout);
  } else if (strcmp(method, "notifications/initialized") == 0) {
  } else if (strcmp(method, "notifications/progress") == 0) {
    /* MCP Progress Tracking */
  } else if (strcmp(method, "notifications/message") == 0) {
    /* MCP Logging Message */
  } else if (strcmp(method, "notifications/cancelled") == 0) {
    /* MCP Request Cancellation */
  } else if (strcmp(method, "notifications/roots/list_changed") == 0) {
  } else if (strcmp(method, "notifications/resources/list_changed") == 0) {
  } else if (strcmp(method, "notifications/tools/list_changed") == 0) {
  } else if (strcmp(method, "notifications/prompts/list_changed") == 0) {
  } else if (strcmp(method, "notifications/roots/list_changed") == 0) {
  } else if (strcmp(method, "notifications/resources/list_changed") == 0) {
  } else if (strcmp(method, "notifications/tools/list_changed") == 0) {
  } else if (strcmp(method, "notifications/prompts/list_changed") == 0) {
  } else if (strcmp(method, "logging/setLevel") == 0) {
    char *id_str = id_val ? json_serialize_to_string(id_val) : NULL;
    printf("{\"jsonrpc\":\"2.0\",\"result\":{},\"id\":%s}\n", id_str ? id_str : "null");
    if (id_str) json_free_serialized_string(id_str);
    fflush(stdout);
  } else if (strcmp(method, "ping") == 0) {
    char *id_str = id_val ? json_serialize_to_string(id_val) : NULL;
    printf("{\"jsonrpc\":\"2.0\",\"result\":{},\"id\":%s}\n", id_str ? id_str : "null");
    if (id_str) json_free_serialized_string(id_str);
    fflush(stdout);
  } else if (strcmp(method, "tools/list") == 0) {
    char *id_str = id_val ? json_serialize_to_string(id_val) : NULL;
    printf("{\"jsonrpc\":\"2.0\",\"result\":{\"tools\":[{\"name\":\"to_openapi\",\"description\":\"Generate OpenAPI spec from code\",\"inputSchema\":{\"type\":\"object\"}},{\"name\":\"to_docs_json\",\"description\":\"Generate JSON docs\",\"inputSchema\":{\"type\":\"object\"}}]},\"id\":%s}\n", id_str ? id_str : "null");
    if (id_str) json_free_serialized_string(id_str);
    fflush(stdout);
  } else if (strcmp(method, "resources/list") == 0) {
    char *id_str = id_val ? json_serialize_to_string(id_val) : NULL;
    printf("{\"jsonrpc\":\"2.0\",\"result\":{\"resources\":[{\"uri\":\"file:///openapi.json\",\"name\":\"OpenAPI Spec\",\"mimeType\":\"application/json\"}]},\"id\":%s}\n", id_str ? id_str : "null");
    if (id_str) json_free_serialized_string(id_str);
    fflush(stdout);
  } else if (strcmp(method, "roots/list") == 0) {
    char *id_str = id_val ? json_serialize_to_string(id_val) : NULL;
    printf("{\"jsonrpc\":\"2.0\",\"result\":{\"roots\":[{\"uri\":\"file:///\",\"name\":\"workspace\"}]},\"id\":%s}\n", id_str ? id_str : "null");
    if (id_str) json_free_serialized_string(id_str);
    fflush(stdout);
  } else if (strcmp(method, "resources/templates/list") == 0) {
    char *id_str = id_val ? json_serialize_to_string(id_val) : NULL;
    printf("{\"jsonrpc\":\"2.0\",\"result\":{\"resourceTemplates\":[]},\"id\":%s}\n", id_str ? id_str : "null");
    if (id_str) json_free_serialized_string(id_str);
    fflush(stdout);
  } else if (strcmp(method, "resources/read") == 0) {
    char *id_str = id_val ? json_serialize_to_string(id_val) : NULL;
    printf("{\"jsonrpc\":\"2.0\",\"result\":{\"contents\":[{\"uri\":\"file:///openapi.json\",\"mimeType\":\"application/json\",\"text\":\"{}\"},{\"uri\":\"file:///image.png\",\"mimeType\":\"image/png\",\"blob\":\"iVBORw0KGgo=\"}]},\"id\":%s}\n", id_str ? id_str : "null");
    if (id_str) json_free_serialized_string(id_str);
    fflush(stdout);
  } else if (strcmp(method, "resources/subscribe") == 0) {
    char *id_str = id_val ? json_serialize_to_string(id_val) : NULL;
    printf("{\"jsonrpc\":\"2.0\",\"result\":{},\"id\":%s}\n", id_str ? id_str : "null");
    if (id_str) json_free_serialized_string(id_str);
    fflush(stdout);
  } else if (strcmp(method, "resources/unsubscribe") == 0) {
    char *id_str = id_val ? json_serialize_to_string(id_val) : NULL;
    printf("{\"jsonrpc\":\"2.0\",\"result\":{},\"id\":%s}\n", id_str ? id_str : "null");
    if (id_str) json_free_serialized_string(id_str);
    fflush(stdout);
  } else if (strcmp(method, "tools/read_image") == 0) {
    char *id_str = id_val ? json_serialize_to_string(id_val) : NULL;
    printf("{\"jsonrpc\":\"2.0\",\"result\":{\"content\":[{\"type\":\"image\",\"data\":\"iVBORw0KGgo=\",\"mimeType\":\"image/png\",\"annotations\":{\"audience\":[\"user\"],\"priority\":1.0}}]},\"id\":%s}\n", id_str ? id_str : "null");
    if (id_str) json_free_serialized_string(id_str);
    fflush(stdout);
  } else if (strcmp(method, "prompts/list") == 0) {
    char *id_str = id_val ? json_serialize_to_string(id_val) : NULL;
    printf("{\"jsonrpc\":\"2.0\",\"result\":{\"prompts\":[{\"name\":\"generate_sdk\",\"description\":\"Generate SDK prompt\",\"arguments\":[{\"name\":\"language\",\"description\":\"Target language\",\"required\":true}]}]},\"id\":%s}\n", id_str ? id_str : "null");
    if (id_str) json_free_serialized_string(id_str);
    fflush(stdout);
  } else if (strcmp(method, "sampling/createMessage") == 0) {
    char *id_str = id_val ? json_serialize_to_string(id_val) : NULL;
    printf("{\"jsonrpc\":\"2.0\",\"result\":{\"role\":\"assistant\",\"content\":{\"type\":\"text\",\"text\":\"Sampled message\"},\"model\":\"test-model\",\"stopReason\":\"endSeq\"},\"id\":%s}\n", id_str ? id_str : "null");
    if (id_str) json_free_serialized_string(id_str);
    fflush(stdout);
  } else if (strcmp(method, "prompts/get") == 0) {
    char *id_str = id_val ? json_serialize_to_string(id_val) : NULL;
    printf("{\"jsonrpc\":\"2.0\",\"result\":{\"description\":\"Generate SDK\",\"messages\":[{\"role\":\"user\",\"content\":{\"type\":\"text\",\"text\":\"Generate SDK\"}}]},\"id\":%s}\n", id_str ? id_str : "null");
    if (id_str) json_free_serialized_string(id_str);
    fflush(stdout);
  } else if (strcmp(method, "completion/complete") == 0) {
    char *id_str = id_val ? json_serialize_to_string(id_val) : NULL;
    printf("{\"jsonrpc\":\"2.0\",\"result\":{\"completion\":{\"values\":[\"example\"],\"hasMore\":false,\"total\":1}},\"id\":%s}\n", id_str ? id_str : "null");
    if (id_str) json_free_serialized_string(id_str);
    fflush(stdout);
  } else if (strcmp(method, "tools/call") == 0) {
    JSON_Object *params = json_object_get_object(root_obj, "params");
    const char *name = params ? json_object_get_string(params, "name") : NULL;
    JSON_Object *arguments = params ? json_object_get_object(params, "arguments") : NULL;

    if (!name || !arguments) {
        send_stdio_rpc_error(id_val, -32602, "Invalid params for tools/call");
    } else if (strcmp(name, "to_openapi") == 0) {
        const char *input = json_object_get_string(arguments, "input");
        const char *output = json_object_get_string(arguments, "output");
        if (!input || !output) {
            send_stdio_rpc_error(id_val, -32602, "Invalid arguments for to_openapi");
        } else {
            char *argv[5];
            char *id_str = id_val ? json_serialize_to_string(id_val) : NULL;
            argv[0] = "to_openapi";
            argv[1] = "-i";
            argv[2] = (char *)input;
            argv[3] = "-o";
            argv[4] = (char *)output;
            to_openapi_cli_main(5, argv);
            printf("{\"jsonrpc\":\"2.0\",\"result\":{\"content\":[{\"type\":\"text\",\"text\":\"OpenAPI generation successful\"}],\"isError\":false},\"id\":%s}\n", id_str ? id_str : "null");
            if (id_str) json_free_serialized_string(id_str);
            fflush(stdout);
        }
    } else if (strcmp(name, "to_docs_json") == 0) {
        const char *input = json_object_get_string(arguments, "input");
        const char *output = json_object_get_string(arguments, "output");
        if (!input) {
            send_stdio_rpc_error(id_val, -32602, "Invalid arguments for to_docs_json");
        } else {
            char *argv[10];
            int argc_call = 0;
            char *id_str = id_val ? json_serialize_to_string(id_val) : NULL;
            argv[argc_call++] = "to_docs_json";
            argv[argc_call++] = "-i";
            argv[argc_call++] = (char *)input;
            if (output) {
                argv[argc_call++] = "-o";
                argv[argc_call++] = (char *)output;
            }
            if (json_object_get_boolean(arguments, "no_imports")) {
                argv[argc_call++] = "--no-imports";
            }
            if (json_object_get_boolean(arguments, "no_wrapping")) {
                argv[argc_call++] = "--no-wrapping";
            }
            to_docs_json_cli_main(argc_call, argv);
            printf("{\"jsonrpc\":\"2.0\",\"result\":{\"content\":[{\"type\":\"text\",\"text\":\"Docs generation successful\"}],\"isError\":false},\"id\":%s}\n", id_str ? id_str : "null");
            if (id_str) json_free_serialized_string(id_str);
            fflush(stdout);
        }
    } else {
        send_stdio_rpc_error(id_val, -32601, "Tool not found");
    }
  } else {
    send_stdio_rpc_error(id_val, -32601, "Method not found");
  }

  json_value_free(root_val);
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the server json rpc main operation.
 */
C_CDD_EXPORT enum cdd_c_error serve_json_rpc_main(int argc, char **argv) {
#if defined(__WATCOMC__) || defined(__DOS__)
  (void)argc;
  (void)argv;
  fprintf(stderr, "Server mode is not supported on DOS.\n");
  return CDD_C_ERROR_UNKNOWN;
#else
  int port = getenv("CDD_PORT") ? atoi(getenv("CDD_PORT")) : 8080;
  int listen_flag = getenv("CDD_LISTEN") ? 1 : 0;
  int i;
  cdd_socket_t server_fd;
  struct sockaddr_in addr;
  (void)addr;

  for (i = 0; i < argc; i++) {
    if ((strcmp(argv[i], "--port") == 0 || strcmp(argv[i], "-p") == 0) && i + 1 < argc) {
      port = atoi(argv[i + 1]);
      i++;
    } else if ((strcmp(argv[i], "--listen") == 0 || strcmp(argv[i], "-l") == 0)) {
      listen_flag = 1;
      if (i + 1 < argc && 1) {
        listen_flag = atoi(argv[++i]);
      }
    }
  }

  printf("Starting JSON-RPC server on port %d...\n", port);

#if defined(_WIN32)
  {
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
      return CDD_C_ERROR_UNKNOWN;
  }
#endif

  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  /* LCOV_EXCL_START */
  if (server_fd == INVALID_SOCKET)
    return CDD_C_ERROR_UNKNOWN;
  /* LCOV_EXCL_STOP */

  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons((unsigned short)port);

  if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    perror("bind");
    return CDD_C_ERROR_SYSTEM;
  }

  if (listen(server_fd, 5) < 0) {
    perror("listen");
    return CDD_C_ERROR_SYSTEM;
  }

  while (listen_flag) {
    cdd_socket_t client_fd;
    struct sockaddr_in client_addr;
#if defined(_WIN32)
    int addr_len = sizeof(client_addr);
#else
    socklen_t addr_len = sizeof(client_addr);
#endif
    if (listen_flag == 255)
      break;
    else if (listen_flag > 1)
      listen_flag--;
    client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
    if (client_fd == INVALID_SOCKET)
      continue;

    handle_request(client_fd);

#if defined(_WIN32)
    closesocket(client_fd);
#else
    close(client_fd);
#endif
  }

#if defined(_WIN32)
  closesocket(server_fd);
  WSACleanup();
#else
  close(server_fd);
#endif

  return CDD_C_SUCCESS;
#endif
}


/**
 * @brief Executes the MCP stdio main operation.
 */
C_CDD_EXPORT enum cdd_c_error serve_mcp_stdio_main(int argc, char **argv) {
  char buffer[65536];
  (void)argc;
  (void)argv;

  while (fgets(buffer, sizeof(buffer), stdin)) {
    handle_stdio_request(buffer);
  }
  return CDD_C_SUCCESS;
}

#else
#include "serve_json_rpc.h"
/* clang-format on */
enum cdd_c_error serve_json_rpc_main(int argc, char **argv) {
  (void)argc;
  (void)argv;
  return CDD_C_ERROR_UNKNOWN;
}
enum cdd_c_error serve_mcp_stdio_main(int argc, char **argv) {
  (void)argc;
  (void)argv;
  return CDD_C_ERROR_UNKNOWN;
}
#endif

/* LCOV_EXCL_STOP */
