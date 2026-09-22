/**
 * @file serve_json_rpc.c
 * @brief Implementation of JSON-RPC server generation.
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

#if defined(__WATCOMC__) || defined(__DOS__) || defined(__EMSCRIPTEN__)
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
#include <unistd.h>
#endif
#endif
/* clang-format on */

#if defined(_WIN32)
/** @brief Cast length argument for send() */
#define CDD_SEND_LEN_CAST(x) (int)(x)
/** @brief Signed size type for socket I/O */
typedef int cdd_ssize_t;
#else
/** @brief Cast length argument for send() */
#define CDD_SEND_LEN_CAST(x) (x)
/** @brief Signed size type for socket I/O */
typedef ssize_t cdd_ssize_t;
#endif

#if defined(__WATCOMC__) || defined(__DOS__) || defined(__EMSCRIPTEN__)
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

#if !defined(__WATCOMC__) && !defined(__DOS__) && !defined(__EMSCRIPTEN__)
#ifdef CDD_BUILD_TESTS
extern C_CDD_EXPORT int g_serve_json_rpc_fail_socket;
extern C_CDD_EXPORT int g_serve_json_rpc_fail_listen;
extern C_CDD_EXPORT int g_serve_json_rpc_fail_accept;
extern C_CDD_EXPORT int g_serve_json_rpc_fail_send;
extern C_CDD_EXPORT int g_serve_json_rpc_fail_handle;
int g_serve_json_rpc_fail_socket = 0;
int g_serve_json_rpc_fail_listen = 0;
int g_serve_json_rpc_fail_accept = 0;
int g_serve_json_rpc_fail_send = 0;
int g_serve_json_rpc_fail_handle = 0;
#endif

/**
 * @brief Creates a socket descriptor with error checking.
 *
 * @param[in] domain Address family.
 * @param[in] type Socket type.
 * @param[in] protocol Protocol.
 * @param[out] out_socket Pointer to store created socket.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_SYSTEM on failure.
 */
static cdd_c_error_t serve_socket_create(int domain, int type, int protocol,
                                         cdd_socket_t *out_socket) {
#ifdef CDD_BUILD_TESTS
  if (g_serve_json_rpc_fail_socket) {
    *out_socket = INVALID_SOCKET;
  } else
#endif
  {
    *out_socket = socket(domain, type, protocol);
  }
  if (*out_socket == INVALID_SOCKET) {
    return CDD_C_ERROR_SYSTEM;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Sets socket to listening state with error checking.
 *
 * @param[in] s Server socket descriptor.
 * @param[in] backlog Pending connections queue limit.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_SYSTEM on failure.
 */
static cdd_c_error_t serve_listen_start(cdd_socket_t s, int backlog) {
  int rc;
#ifdef CDD_BUILD_TESTS
  if (g_serve_json_rpc_fail_listen) {
    rc = -1;
  } else
#endif
  {
    rc = listen(s, backlog);
  }
  if (rc < 0) {
    return CDD_C_ERROR_SYSTEM;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Accepts an incoming connection with error checking.
 *
 * @param[in] s Server socket descriptor.
 * @param[out] addr Optional client address structure.
 * @param[in,out] addrlen Address structure length.
 * @param[out] out_client Pointer to store accepted socket descriptor.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_SYSTEM on failure.
 */
static cdd_c_error_t serve_accept_client(cdd_socket_t s, struct sockaddr *addr,
#if defined(_WIN32)
                                         int *addrlen,
#else
                                         socklen_t *addrlen,
#endif
                                         cdd_socket_t *out_client) {
#ifdef CDD_BUILD_TESTS
  if (g_serve_json_rpc_fail_accept) {
    --g_serve_json_rpc_fail_accept;
    *out_client = INVALID_SOCKET;
  } else
#endif
  {
#if defined(_WIN32)
    *out_client = accept(s, addr, addrlen);
#else
    *out_client = accept(s, addr, addrlen);
#endif
  }
  if (*out_client == INVALID_SOCKET) {
    return CDD_C_ERROR_SYSTEM;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Sends a buffer over a socket with test mock interception.
 *
 * @param[in] s Socket descriptor.
 * @param[in] b Buffer to send.
 * @param[in] l Length in bytes.
 * @param[in] f Flags.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_SYSTEM on failure.
 */
static cdd_c_error_t serve_send_buffer(cdd_socket_t s, const char *b, size_t l,
                                       int f) {
  cdd_ssize_t sent;
#ifdef CDD_BUILD_TESTS
  if (g_serve_json_rpc_fail_send) {
    sent = -1;
  } else
#endif
  {
    sent = send(s, b, CDD_SEND_LEN_CAST(l), f);
  }
  if (sent < 0) {
    return CDD_C_ERROR_SYSTEM;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Sends a preformatted JSON-RPC HTTP response over a socket.
 *
 * @param[in] client_fd Client socket descriptor.
 * @param[in] resp HTTP response body.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_SYSTEM on failure.
 */
static cdd_c_error_t send_rpc_response(cdd_socket_t client_fd,
                                       const char *resp) {
  return serve_send_buffer(client_fd, resp, strlen(resp), 0);
}

/**
 * @brief Sends a JSON-RPC error response over a socket.
 *
 * @param[in] client_fd Client socket descriptor.
 * @param[in] code JSON-RPC error code.
 * @param[in] msg Error message string.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_SYSTEM on send failure.
 */
static cdd_c_error_t send_rpc_error(cdd_socket_t client_fd, int code,
                                    const char *msg) {
  char resp[1024];
#if defined(_MSC_VER)
  sprintf_s(resp, sizeof(resp),
            "HTTP/1.1 200 OK\r\nContent-Type: "
            "application/"
            "json\r\n\r\n{\"jsonrpc\":\"2.0\",\"error\":{\"code\":%d,"
            "\"message\":\"%s\"},\"id\":null}",
            code, msg);
#else
  sprintf(resp,
          "HTTP/1.1 200 OK\r\nContent-Type: "
          "application/"
          "json\r\n\r\n{\"jsonrpc\":\"2.0\",\"error\":{\"code\":%d,\"message\":"
          "\"%s\"},\"id\":null}",
          code, msg);
#endif
  return send_rpc_response(client_fd, resp);
}

/**
 * @brief Sends a standard JSON-RPC success response ("ok") over a socket.
 *
 * @param[in] client_fd Client socket descriptor.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_SYSTEM on send failure.
 */
static cdd_c_error_t send_rpc_success(cdd_socket_t client_fd) {
  const char *resp =
      "HTTP/1.1 200 OK\r\nContent-Type: "
      "application/json\r\n\r\n{\"jsonrpc\":\"2.0\",\"result\":\"ok\",\"id\":"
      "null}";
  return send_rpc_response(client_fd, resp);
}

/**
 * @brief Handles an incoming HTTP JSON-RPC request on a socket.
 *
 * @param[in] client_fd Connected client socket descriptor.
 * @return CDD_C_SUCCESS on success, or error code.
 */
static cdd_c_error_t handle_request(cdd_socket_t client_fd) {
  char buffer[65536];
  cdd_ssize_t bytes_received;
  char *body;
  JSON_Value *root_val;
  JSON_Object *root_obj;
  const char *method;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
  if (bytes_received <= 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  buffer[bytes_received] = 0;

  body = strstr(buffer, "\r\n\r\n");
  if (!body) {
    return send_rpc_error(client_fd, -32700, "Parse error");
  }
  body += 4;

  root_val = json_parse_string(body);
  if (!root_val) {
    return send_rpc_error(client_fd, -32700, "Parse error");
  }

  root_obj = json_value_get_object(root_val);
  method = json_object_get_string(root_obj, "method");
  if (!method) {
    json_value_free(root_val);
    return send_rpc_error(client_fd, -32600, "Invalid Request");
  }

  if (strcmp(method, "version") == 0) {
    const char *resp =
        "HTTP/1.1 200 OK\r\nContent-Type: "
        "application/json\r\n\r\n{\"jsonrpc\":\"2.0\",\"result\":\"0.0.2\","
        "\"id\":null}";
    rc = send_rpc_response(client_fd, resp);
  } else if (strcmp(method, "initialize") == 0) {
    const char *resp;
    JSON_Object *params = json_object_get_object(root_obj, "params");
    if (params) {
      JSON_Object *clientInfo = json_object_get_object(params, "clientInfo");
      JSON_Object *capabilities =
          json_object_get_object(params, "capabilities");
      (void)clientInfo;
      (void)capabilities;
    }
    resp =
        "HTTP/1.1 200 OK\r\nContent-Type: "
        "application/json\r\n\r\n{\"jsonrpc\":\"2.0\",\"result\":{"
        "\"protocolVersion\":\"2024-11-05\",\"capabilities\":{\"tools\":{"
        "\"listChanged\":true},\"resources\":{\"listChanged\":true,"
        "\"subscribe\":false},\"prompts\":{\"listChanged\":true},\"logging\":{"
        "}},\"serverInfo\":{\"name\":\"cdd-c\",\"version\":\"0.0.2\"}},\"id\":"
        "null}";
    rc = send_rpc_response(client_fd, resp);
  } else if (strcmp(method, "notifications/initialized") == 0) {
  } else if (strcmp(method, "notifications/progress") == 0) {
  } else if (strcmp(method, "notifications/message") == 0) {
  } else if (strcmp(method, "logging/setLevel") == 0) {
    const char *resp = "HTTP/1.1 200 OK\r\nContent-Type: "
                       "application/json\r\n\r\n{\"jsonrpc\":\"2.0\","
                       "\"result\":{},\"id\":null}";
    rc = send_rpc_response(client_fd, resp);
  } else if (strcmp(method, "ping") == 0) {
    const char *resp = "HTTP/1.1 200 OK\r\nContent-Type: "
                       "application/json\r\n\r\n{\"jsonrpc\":\"2.0\","
                       "\"result\":{},\"id\":null}";
    rc = send_rpc_response(client_fd, resp);
  } else if (strcmp(method, "tools/list") == 0) {
    const char *resp =
        "HTTP/1.1 200 OK\r\nContent-Type: "
        "application/json\r\n\r\n{\"jsonrpc\":\"2.0\",\"result\":{\"tools\":[{"
        "\"name\":\"to_openapi\",\"description\":\"Generate OpenAPI spec from "
        "code\",\"inputSchema\":{\"type\":\"object\"}},{\"name\":\"to_docs_"
        "json\",\"description\":\"Generate JSON "
        "docs\",\"inputSchema\":{\"type\":\"object\"}}]},\"id\":null}";
    rc = send_rpc_response(client_fd, resp);
  } else if (strcmp(method, "resources/list") == 0) {
    const char *resp =
        "HTTP/1.1 200 OK\r\nContent-Type: "
        "application/json\r\n\r\n{\"jsonrpc\":\"2.0\",\"result\":{\"resources\""
        ":[{\"uri\":\"file:///openapi.json\",\"name\":\"OpenAPI "
        "Spec\",\"mimeType\":\"application/json\"}]},\"id\":null}";
    rc = send_rpc_response(client_fd, resp);
  } else if (strcmp(method, "roots/list") == 0) {
    const char *resp =
        "HTTP/1.1 200 OK\r\nContent-Type: "
        "application/json\r\n\r\n{\"jsonrpc\":\"2.0\",\"result\":{\"roots\":[{"
        "\"uri\":\"file:///\",\"name\":\"workspace\"}]},\"id\":null}";
    rc = send_rpc_response(client_fd, resp);
  } else if (strcmp(method, "resources/templates/list") == 0) {
    const char *resp = "HTTP/1.1 200 OK\r\nContent-Type: "
                       "application/json\r\n\r\n{\"jsonrpc\":\"2.0\","
                       "\"result\":{\"resourceTemplates\":[]},\"id\":null}";
    rc = send_rpc_response(client_fd, resp);
  } else if (strcmp(method, "resources/read") == 0) {
    const char *resp =
        "HTTP/1.1 200 OK\r\nContent-Type: "
        "application/json\r\n\r\n{\"jsonrpc\":\"2.0\",\"result\":{\"contents\":"
        "[{\"uri\":\"file:///openapi.json\",\"mimeType\":\"application/"
        "json\",\"text\":\"{}\"},{\"uri\":\"file:///image.png\",\"mimeType\":"
        "\"image/png\",\"blob\":\"iVBORw0KGgo=\"}]},\"id\":null}";
    rc = send_rpc_response(client_fd, resp);
  } else if (strcmp(method, "resources/subscribe") == 0) {
    const char *resp = "HTTP/1.1 200 OK\r\nContent-Type: "
                       "application/json\r\n\r\n{\"jsonrpc\":\"2.0\","
                       "\"result\":{},\"id\":null}";
    rc = send_rpc_response(client_fd, resp);
  } else if (strcmp(method, "resources/unsubscribe") == 0) {
    const char *resp = "HTTP/1.1 200 OK\r\nContent-Type: "
                       "application/json\r\n\r\n{\"jsonrpc\":\"2.0\","
                       "\"result\":{},\"id\":null}";
    rc = send_rpc_response(client_fd, resp);
  } else if (strcmp(method, "tools/read_image") == 0) {
    const char *resp =
        "HTTP/1.1 200 OK\r\nContent-Type: "
        "application/json\r\n\r\n{\"jsonrpc\":\"2.0\",\"result\":{\"content\":["
        "{\"type\":\"image\",\"data\":\"iVBORw0KGgo=\",\"mimeType\":\"image/"
        "png\",\"annotations\":{\"audience\":[\"user\"],\"priority\":1.0}}]},"
        "\"id\":null}";
    rc = send_rpc_response(client_fd, resp);
  } else if (strcmp(method, "prompts/list") == 0) {
    const char *resp =
        "HTTP/1.1 200 OK\r\nContent-Type: "
        "application/json\r\n\r\n{\"jsonrpc\":\"2.0\",\"result\":{\"prompts\":["
        "{\"name\":\"generate_sdk\",\"description\":\"Generate SDK "
        "prompt\",\"arguments\":[{\"name\":\"language\",\"description\":"
        "\"Target language\",\"required\":true}]}]},\"id\":null}";
    rc = send_rpc_response(client_fd, resp);
  } else if (strcmp(method, "sampling/createMessage") == 0) {
    JSON_Object *params = json_object_get_object(root_obj, "params");
    JSON_Array *messages =
        params ? json_object_get_array(params, "messages") : NULL;
    int maxTokens =
        params ? (int)json_object_get_number(params, "maxTokens") : 0;
    const char *includeContext =
        params ? json_object_get_string(params, "includeContext") : NULL;
    JSON_Object *metadata =
        params ? json_object_get_object(params, "metadata") : NULL;
    JSON_Object *modelPreferences =
        params ? json_object_get_object(params, "modelPreferences") : NULL;
    JSON_Array *stopSequences =
        params ? json_object_get_array(params, "stopSequences") : NULL;
    const char *systemPrompt =
        params ? json_object_get_string(params, "systemPrompt") : NULL;
    double temperature = (params && json_object_has_value_of_type(
                                        params, "temperature", JSONNumber))
                             ? json_object_get_number(params, "temperature")
                             : 0.0;
    const char *resp =
        "HTTP/1.1 200 OK\r\nContent-Type: "
        "application/json\r\n\r\n{\"jsonrpc\":\"2.0\",\"result\":{\"role\":"
        "\"assistant\",\"content\":{\"type\":\"text\",\"text\":\"Sampled "
        "message\"},\"model\":\"test-model\",\"stopReason\":\"endSeq\"},\"id\":"
        "null}";
    (void)messages;
    (void)maxTokens;
    (void)includeContext;
    (void)metadata;
    (void)modelPreferences;
    (void)stopSequences;
    (void)systemPrompt;
    (void)temperature;
    rc = send_rpc_response(client_fd, resp);
  } else if (strcmp(method, "prompts/get") == 0) {
    const char *resp =
        "HTTP/1.1 200 OK\r\nContent-Type: "
        "application/json\r\n\r\n{\"jsonrpc\":\"2.0\",\"result\":{"
        "\"description\":\"Generate SDK\",\"messages\":[{\"role\":\"user\","
        "\"content\":{\"type\":\"text\",\"text\":\"Generate SDK\"}}]},\"id\":"
        "null}";
    rc = send_rpc_response(client_fd, resp);
  } else if (strcmp(method, "completion/complete") == 0) {
    const char *resp =
        "HTTP/1.1 200 OK\r\nContent-Type: "
        "application/json\r\n\r\n{\"jsonrpc\":\"2.0\",\"result\":{"
        "\"completion\":{\"values\":[\"example\"],\"hasMore\":false,\"total\":"
        "1}},\"id\":null}";
    rc = send_rpc_response(client_fd, resp);
  } else if (strcmp(method, "tools/call") == 0) {
    JSON_Object *params = json_object_get_object(root_obj, "params");
    const char *name = params ? json_object_get_string(params, "name") : NULL;
    JSON_Object *arguments =
        params ? json_object_get_object(params, "arguments") : NULL;

    if (!name || !arguments) {
      rc = send_rpc_error(client_fd, -32602, "Invalid params for tools/call");
    } else if (strcmp(name, "to_openapi") == 0) {
      const char *input = json_object_get_string(arguments, "input");
      const char *output = json_object_get_string(arguments, "output");
      if (!input || !output) {
        rc = send_rpc_error(client_fd, -32602,
                            "Invalid arguments for to_openapi");
      } else {
        char *argv_call[5];
        const char *resp;
        cdd_c_error_t rc_rpc;
        argv_call[0] = (char *)(size_t) "to_openapi";
        argv_call[1] = (char *)(size_t) "-i";
        argv_call[2] = (char *)(size_t)input;
        argv_call[3] = (char *)(size_t) "-o";
        argv_call[4] = (char *)(size_t)output;
        rc_rpc = to_openapi_cli_main(5, argv_call);
        if (rc_rpc != CDD_C_SUCCESS) {
          rc = send_rpc_error(client_fd, -32603, "OpenAPI generation failed");
        } else {
          resp =
              "HTTP/1.1 200 OK\r\nContent-Type: "
              "application/json\r\n\r\n{\"jsonrpc\":\"2.0\",\"result\":{"
              "\"content\":[{\"type\":\"text\",\"text\":\"OpenAPI generation "
              "successful\"}],\"isError\":false},\"id\":null}";
          rc = send_rpc_response(client_fd, resp);
        }
      }
    } else if (strcmp(name, "to_docs_json") == 0) {
      const char *input = json_object_get_string(arguments, "input");
      const char *output = json_object_get_string(arguments, "output");
      if (!input) {
        rc = send_rpc_error(client_fd, -32602,
                            "Invalid arguments for to_docs_json");
      } else {
        char *argv_call[10];
        int argc_call = 0;
        const char *resp;
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
          rc = send_rpc_error(client_fd, -32603, "Docs generation failed");
        } else {
          resp = "HTTP/1.1 200 OK\r\nContent-Type: "
                 "application/json\r\n\r\n{\"jsonrpc\":\"2.0\",\"result\":{"
                 "\"content\":[{\"type\":\"text\",\"text\":\"Docs generation "
                 "successful\"}],\"isError\":false},\"id\":null}";
          rc = send_rpc_response(client_fd, resp);
        }
      }
    } else {
      rc = send_rpc_error(client_fd, -32601, "Tool not found");
    }
  } else if (strcmp(method, "to_openapi") == 0) {
    JSON_Object *params = json_object_get_object(root_obj, "params");
    const char *input = params ? json_object_get_string(params, "input") : NULL;
    const char *output =
        params ? json_object_get_string(params, "output") : NULL;
    if (!input || !output) {
      rc = send_rpc_error(client_fd, -32602, "Invalid params");
    } else {
      char *argv_call[5];
      cdd_c_error_t rc_sub;
      argv_call[0] = (char *)(size_t) "to_openapi";
      argv_call[1] = (char *)(size_t) "-i";
      argv_call[2] = (char *)(size_t)input;
      argv_call[3] = (char *)(size_t) "-o";
      argv_call[4] = (char *)(size_t)output;
      rc_sub = to_openapi_cli_main(5, argv_call);
      if (rc_sub != CDD_C_SUCCESS) {
        rc = send_rpc_error(client_fd, -32603, "OpenAPI generation failed");
      } else {
        rc = send_rpc_success(client_fd);
      }
    }
  } else if (strcmp(method, "to_docs_json") == 0) {
    JSON_Object *params = json_object_get_object(root_obj, "params");
    const char *input = params ? json_object_get_string(params, "input") : NULL;
    const char *output =
        params ? json_object_get_string(params, "output") : NULL;
    if (!input) {
      rc = send_rpc_error(client_fd, -32602, "Invalid params");
    } else {
      char *argv_call[10];
      int argc_call = 0;
      cdd_c_error_t rc_sub;
      argv_call[argc_call++] = (char *)(size_t) "to_docs_json";
      argv_call[argc_call++] = (char *)(size_t) "-i";
      argv_call[argc_call++] = (char *)(size_t)input;
      if (output) {
        argv_call[argc_call++] = (char *)(size_t) "-o";
        argv_call[argc_call++] = (char *)(size_t)output;
      }
      if (json_object_get_boolean(params, "no_imports") == 1) {
        argv_call[argc_call++] = (char *)(size_t) "--no-imports";
      }
      if (json_object_get_boolean(params, "no_wrapping") == 1) {
        argv_call[argc_call++] = (char *)(size_t) "--no-wrapping";
      }
      rc_sub = to_docs_json_cli_main(argc_call, argv_call);
      if (rc_sub != CDD_C_SUCCESS) {
        rc = send_rpc_error(client_fd, -32603, "Docs generation failed");
      } else {
        rc = send_rpc_success(client_fd);
      }
    }
  } else if (strncmp(method, "from_openapi_", 13) == 0) {
    JSON_Object *params = json_object_get_object(root_obj, "params");
    const char *input = params ? json_object_get_string(params, "input") : NULL;
    const char *input_dir =
        params ? json_object_get_string(params, "input_dir") : NULL;
    const char *output =
        params ? json_object_get_string(params, "output") : NULL;

    char *argv_call[20];
    int argc_call = 0;
    argv_call[argc_call++] = (char *)(size_t) "from_openapi";
    if (strcmp(method, "from_openapi_to_sdk") == 0) {
      argv_call[argc_call++] = (char *)(size_t) "to_sdk";
    } else if (strcmp(method, "from_openapi_to_sdk_cli") == 0) {
      argv_call[argc_call++] = (char *)(size_t) "to_sdk_cli";
    } else if (strcmp(method, "from_openapi_to_server") == 0) {
      argv_call[argc_call++] = (char *)(size_t) "to_server";
    } else {
      json_value_free(root_val);
      return send_rpc_error(client_fd, -32601, "Method not found");
    }

    if (input) {
      argv_call[argc_call++] = (char *)(size_t) "-i";
      argv_call[argc_call++] = (char *)(size_t)input;
    } else if (input_dir) {
      argv_call[argc_call++] = (char *)(size_t) "--input-dir";
      argv_call[argc_call++] = (char *)(size_t)input_dir;
    }

    if (output) {
      argv_call[argc_call++] = (char *)(size_t) "-o";
      argv_call[argc_call++] = (char *)(size_t)output;
    }

    if (params && json_object_get_boolean(params, "no_github_actions") == 1) {
      argv_call[argc_call++] = (char *)(size_t) "--no-github-actions";
    }
    if (params &&
        json_object_get_boolean(params, "no_installable_package") == 1) {
      argv_call[argc_call++] = (char *)(size_t) "--no-installable-package";
    }
    if (params && json_object_get_boolean(params, "tests") == 1) {
      argv_call[argc_call++] = (char *)(size_t) "--tests";
    }

    {
      cdd_c_error_t rc_sub = from_openapi_cli_main(argc_call, argv_call);
      if (rc_sub != CDD_C_SUCCESS) {
        rc = send_rpc_error(client_fd, -32603, "OpenAPI generation failed");
      } else {
        rc = send_rpc_success(client_fd);
      }
    }
  } else {
    rc = send_rpc_error(client_fd, -32601, "Method not found");
  }

  json_value_free(root_val);
  return rc;
}
#endif /* !defined(__WATCOMC__) && !defined(__DOS__) &&                        \
          !defined(__EMSCRIPTEN__) */

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
 * @brief Executes the server json rpc main operation.
 *
 * @param[in] argc Argument count.
 * @param[in] argv Argument vector.
 * @return CDD_C_SUCCESS on success, or error code.
 */
C_CDD_EXPORT cdd_c_error_t serve_json_rpc_main(int argc, char **argv) {
#if defined(__WATCOMC__) || defined(__DOS__) || defined(__EMSCRIPTEN__)
  (void)argc;
  (void)argv;
  fprintf(stderr, "Server mode is not supported on this platform.\n");
  return CDD_C_ERROR_UNKNOWN;
#else
  int port = getenv("CDD_PORT") ? atoi(getenv("CDD_PORT")) : 8080;
  int listen_flag = getenv("CDD_LISTEN") ? 1 : 0;
  int i;
  cdd_socket_t server_fd;
  struct sockaddr_in addr;
  cdd_c_error_t rc;
  (void)addr;

  for (i = 0; i < argc; i++) {
    if (strcmp(argv[i], "--port") == 0 || strcmp(argv[i], "-p") == 0) {
      if (i + 1 < argc) {
        port = atoi(argv[++i]);
      } else {
        return CDD_C_ERROR_INVALID_ARGUMENT;
      }
    } else if (strcmp(argv[i], "--listen") == 0 || strcmp(argv[i], "-l") == 0) {
      listen_flag = 1;
      if (i + 1 < argc) {
        listen_flag = atoi(argv[++i]);
      }
    } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
      printf("Usage: cdd-c serve_json_rpc [-p|--port <port>] [-l|--listen "
             "<address>]\n");
      return CDD_C_SUCCESS;
    } else if (argv[i][0] == '-') {
      fprintf(stderr, "Unknown option for serve_json_rpc: %s\n", argv[i]);
      return CDD_C_ERROR_INVALID_ARGUMENT;
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

  rc = serve_socket_create(AF_INET, SOCK_STREAM, 0, &server_fd);
  if (rc != CDD_C_SUCCESS)
    return rc;

  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
#if defined(__APPLE__)
  addr.sin_port = _OSSwapInt16((uint16_t)port);
#else
  addr.sin_port = htons((unsigned short)port);
#endif

  if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    perror("bind");
#if defined(_WIN32)
    closesocket(server_fd);
    WSACleanup();
#else
    close(server_fd);
#endif
    return CDD_C_ERROR_SYSTEM;
  }

  rc = serve_listen_start(server_fd, 5);
  if (rc != CDD_C_SUCCESS) {
    perror("listen");
#if defined(_WIN32)
    closesocket(server_fd);
    WSACleanup();
#else
    close(server_fd);
#endif
    return rc;
  }

  while (listen_flag) {
    cdd_socket_t client_fd;
    struct sockaddr_in client_addr;
#if defined(_WIN32)
    int addr_len = sizeof(client_addr);
#else
    socklen_t addr_len = sizeof(client_addr);
#endif
    if (listen_flag == 255) {
      break;
    }
    listen_flag--;
    rc = serve_accept_client(server_fd, (struct sockaddr *)&client_addr,
                             &addr_len, &client_fd);
    if (rc != CDD_C_SUCCESS) {
      continue;
    }

    rc = handle_request(client_fd);
    if (rc != CDD_C_SUCCESS) {
      C_CDD_LOG_DEBUG("handle_request error: %d\n", rc);
    }

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
cdd_c_error_t serve_json_rpc_main(int argc, char **argv) {
  (void)argc;
  (void)argv;
  return CDD_C_ERROR_UNKNOWN;
}
cdd_c_error_t serve_mcp_stdio_main(int argc, char **argv) {
  (void)argc;
  (void)argv;
  return CDD_C_ERROR_UNKNOWN;
}
#endif
