/**
 * @file serve_json_rpc.c
 * @brief Implementation of JSON-RPC server generation.
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"
#include "c_cdd/context.h"
#include "c_cdd/log.h"
#include "c_cdd/memory.h"

#ifndef __wasi__
#include "serve_json_rpc.h"
#include "../parse/cli.h"
#include "../../cdd_api.h"
#include "functions/parse/fs.h"
#include <ctype.h>
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
#if defined(_MSC_VER)
#include <io.h>
#else
#include <unistd.h>
#endif
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
  const cdd_context_t *ctx = NULL;
  int fd = -1;
  cdd_context_get_global(&ctx);
  if (ctx->socket_vt.socket_fn) {
    ctx->socket_vt.socket_fn(domain, type, protocol, &fd);
    *out_socket = (cdd_socket_t)fd;
  } else {
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
  const cdd_context_t *ctx = NULL;
  int rc;
  cdd_context_get_global(&ctx);
  if (ctx->socket_vt.listen_fn) {
    rc = -(int)ctx->socket_vt.listen_fn((int)s, backlog);
  } else {
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
  const cdd_context_t *ctx = NULL;
  int client_fd = -1;
  cdd_context_get_global(&ctx);
  if (ctx->socket_vt.accept_fn) {
    ctx->socket_vt.accept_fn((int)s, &client_fd);
    *out_client = (cdd_socket_t)client_fd;
  } else {
    *out_client = accept(s, addr, addrlen);
  }
  if (*out_client == INVALID_SOCKET) {
    return CDD_C_ERROR_SYSTEM;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Sends a buffer over a socket with VTable dispatch support.
 *
 * @param[in] s Socket descriptor.
 * @param[in] b Buffer to send.
 * @param[in] l Length in bytes.
 * @param[in] f Flags.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_SYSTEM on failure.
 */
static cdd_c_error_t serve_send_buffer(cdd_socket_t s, const char *b, size_t l,
                                       int f) {
  const cdd_context_t *ctx = NULL;
  cdd_ssize_t sent;
  size_t out_sent = 0;
  cdd_context_get_global(&ctx);
  if (ctx->socket_vt.send_fn) {
    sent = -(cdd_ssize_t)ctx->socket_vt.send_fn((int)s, b, l, &out_sent);
  } else {
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
 * @brief Sends a JSON-RPC error response over an HTTP socket, echoing `id_val`.
 *
 * @param[in] client_fd Client socket descriptor.
 * @param[in] id_val Optional JSON-RPC request ID value (integer, string, or
 * null).
 * @param[in] code JSON-RPC error code.
 * @param[in] msg Error message string.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_SYSTEM on send failure.
 */
static cdd_c_error_t send_rpc_error_id(cdd_socket_t client_fd,
                                       JSON_Value *id_val, int code,
                                       const char *msg) {
  char resp[2048];
  char *id_str = json_serialize_to_string(id_val);
#if defined(_MSC_VER)
  sprintf_s(resp, sizeof(resp),
            "HTTP/1.1 200 OK\r\nContent-Type: "
            "application/"
            "json\r\n\r\n{\"jsonrpc\":\"2.0\",\"error\":{\"code\":%d,"
            "\"message\":\"%s\"},\"id\":%s}",
            code, msg, id_str ? id_str : "null");
#else
  CDD_SNPRINTF(
      resp, sizeof(resp),
      "HTTP/1.1 200 OK\r\nContent-Type: "
      "application/"
      "json\r\n\r\n{\"jsonrpc\":\"2.0\",\"error\":{\"code\":%d,\"message\":"
      "\"%s\"},\"id\":%s}",
      code, msg, id_str ? id_str : "null");
#endif
  json_free_serialized_string(id_str);
  return send_rpc_response(client_fd, resp);
}

/**
 * @brief Sends a JSON-RPC result response over an HTTP socket, echoing
 * `id_val`.
 *
 * @param[in] client_fd Client socket descriptor.
 * @param[in] id_val Optional JSON-RPC request ID value (integer, string, or
 * null).
 * @param[in] result_json Serialized JSON result payload.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_SYSTEM on send failure.
 */
static cdd_c_error_t send_rpc_result_id(cdd_socket_t client_fd,
                                        JSON_Value *id_val,
                                        const char *result_json) {
  char resp[8192];
  char *id_str = json_serialize_to_string(id_val);
#if defined(_MSC_VER)
  sprintf_s(resp, sizeof(resp),
            "HTTP/1.1 200 OK\r\nContent-Type: "
            "application/json\r\n\r\n{\"jsonrpc\":\"2.0\",\"result\":%s,"
            "\"id\":%s}",
            result_json, id_str ? id_str : "null");
#else
  CDD_SNPRINTF(resp, sizeof(resp),
               "HTTP/1.1 200 OK\r\nContent-Type: "
               "application/json\r\n\r\n{\"jsonrpc\":\"2.0\",\"result\":%s,"
               "\"id\":%s}",
               result_json, id_str ? id_str : "null");
#endif
  json_free_serialized_string(id_str);
  return send_rpc_response(client_fd, resp);
}

/**
 * @brief Parses the `Content-Length` header value from an HTTP request header
 * block.
 *
 * @param[in] headers Start of HTTP headers buffer.
 * @param[in] header_len Length of headers in bytes.
 * @param[out] out_len Parsed Content-Length value (0 if absent).
 * @return CDD_C_SUCCESS on success.
 */
static cdd_c_error_t parse_content_length(const char *headers,
                                          size_t header_len, size_t *out_len) {
  size_t i, k;
  const char *prefix = "content-length:";
  *out_len = 0;
  for (i = 0; i + 15 <= header_len; ++i) {
    for (k = 0; k < 15 && tolower((unsigned char)headers[i + k]) == prefix[k];
         ++k) {
    }
    if (k == 15) {
      *out_len = (size_t)atol(headers + i + 15);
      break;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Builds a dynamic JSON result string for `resources/read` based on the
 * requested URI.
 *
 * @param[in] params Optional JSON-RPC params object containing `"uri"`.
 * @param[out] out_json Newly allocated JSON string for the result object.
 * @return CDD_C_SUCCESS on success, or error code.
 */
static cdd_c_error_t build_resources_read_result(const JSON_Object *params,
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
        "\"0.0.4\"},\"paths\":{}}");
  }
  json_array_append_value(arr, item_val);
  json_object_set_value(res_obj, "contents", arr_val);
  serialized = json_serialize_to_string(res_val);
  json_value_free(res_val);
  *out_json = serialized;
  return CDD_C_SUCCESS;
}

/**
 * @brief Handles an incoming HTTP JSON-RPC request on a socket.
 *
 * @param[in] client_fd Connected client socket descriptor.
 * @return CDD_C_SUCCESS on success, or error code.
 */
static cdd_c_error_t handle_request(cdd_socket_t client_fd) {
  char buffer[65536];
  size_t total_received = 0;
  cdd_ssize_t bytes_received;
  char *body = NULL;
  size_t content_len = 0;
  JSON_Value *root_val;
  JSON_Object *root_obj;
  JSON_Value *id_val;
  const char *method;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  for (;;) {
    bytes_received =
        recv(client_fd, buffer + total_received,
             CDD_SEND_LEN_CAST(sizeof(buffer) - 1 - total_received), 0);
    if (bytes_received <= 0)
      break;
    total_received += (size_t)bytes_received;
    buffer[total_received] = 0;
    body = strstr(buffer, "\r\n\r\n");
    if (body) {
      parse_content_length(buffer, (size_t)(body - buffer), &content_len);
      body += 4;
      if ((size_t)(buffer + total_received - body) >= content_len)
        break;
    }
  }
  if (total_received == 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (!body)
    return send_rpc_error_id(client_fd, NULL, -32700, "Parse error");

  root_val = json_parse_string(body);
  if (!root_val) {
    return send_rpc_error_id(client_fd, NULL, -32700, "Parse error");
  }

  root_obj = json_value_get_object(root_val);
  id_val = json_object_get_value(root_obj, "id");
  method = json_object_get_string(root_obj, "method");
  if (!method) {
    rc = send_rpc_error_id(client_fd, id_val, -32600, "Invalid Request");
    json_value_free(root_val);
    return rc;
  }

  if (strcmp(method, "version") == 0) {
    rc = send_rpc_result_id(client_fd, id_val, "\"0.0.4\"");
  } else if (strcmp(method, "initialize") == 0) {
    JSON_Object *params = json_object_get_object(root_obj, "params");
    if (params) {
      JSON_Object *clientInfo = json_object_get_object(params, "clientInfo");
      JSON_Object *capabilities =
          json_object_get_object(params, "capabilities");
      (void)clientInfo;
      (void)capabilities;
    }
    rc = send_rpc_result_id(
        client_fd, id_val,
        "{\"protocolVersion\":\"2024-11-05\",\"capabilities\":{\"tools\":{"
        "\"listChanged\":true},\"resources\":{\"listChanged\":true,"
        "\"subscribe\":false},\"prompts\":{\"listChanged\":true},\"logging\":{"
        "}},\"serverInfo\":{\"name\":\"cdd-c\",\"version\":\"0.0.4\"}}");
  } else if (strcmp(method, "notifications/initialized") == 0) {
  } else if (strcmp(method, "notifications/progress") == 0) {
  } else if (strcmp(method, "notifications/message") == 0) {
  } else if (strcmp(method, "logging/setLevel") == 0) {
    rc = send_rpc_result_id(client_fd, id_val, "{}");
  } else if (strcmp(method, "ping") == 0) {
    rc = send_rpc_result_id(client_fd, id_val, "{}");
  } else if (strcmp(method, "tools/list") == 0) {
    rc = send_rpc_result_id(
        client_fd, id_val,
        "{\"tools\":[{\"name\":\"to_openapi\",\"description\":\"Generate "
        "OpenAPI spec from code\",\"inputSchema\":{\"type\":\"object\"}},"
        "{\"name\":\"to_docs_json\",\"description\":\"Generate JSON docs\","
        "\"inputSchema\":{\"type\":\"object\"}},{\"name\":\"to_arazzo\","
        "\"description\":\"Extract Arazzo spec from C code\",\"inputSchema\":"
        "{\"type\":\"object\"}},{\"name\":\"from_arazzo\",\"description\":"
        "\"Generate C workflows from Arazzo spec\",\"inputSchema\":"
        "{\"type\":\"object\"}}]}");
  } else if (strcmp(method, "resources/list") == 0) {
    rc = send_rpc_result_id(
        client_fd, id_val,
        "{\"resources\":[{\"uri\":\"file:///openapi.json\",\"name\":\"OpenAPI "
        "Spec\",\"mimeType\":\"application/json\"},{\"uri\":"
        "\"file:///arazzo.json\",\"name\":\"Arazzo Workflow Spec\","
        "\"mimeType\":\"application/json\"}]}");
  } else if (strcmp(method, "roots/list") == 0) {
    rc = send_rpc_result_id(
        client_fd, id_val,
        "{\"roots\":[{\"uri\":\"file:///workspace\",\"name\":\"workspace\"}]}");
  } else if (strcmp(method, "resources/templates/list") == 0) {
    rc = send_rpc_result_id(
        client_fd, id_val,
        "{\"resourceTemplates\":[{\"uriTemplate\":\"file:///{path}\","
        "\"name\":\"Workspace File\",\"mimeType\":\"application/json\"}]}");
  } else if (strcmp(method, "resources/read") == 0) {
    JSON_Object *params = json_object_get_object(root_obj, "params");
    char *res_str = NULL;
    build_resources_read_result(params, &res_str);
    rc = send_rpc_result_id(client_fd, id_val, res_str);
    json_free_serialized_string(res_str);
  } else if (strcmp(method, "resources/subscribe") == 0) {
    rc = send_rpc_result_id(client_fd, id_val, "{}");
  } else if (strcmp(method, "resources/unsubscribe") == 0) {
    rc = send_rpc_result_id(client_fd, id_val, "{}");
  } else if (strcmp(method, "tools/read_image") == 0) {
    rc = send_rpc_result_id(
        client_fd, id_val,
        "{\"content\":[{\"type\":\"image\",\"data\":\"iVBORw0KGgo=\","
        "\"mimeType\":\"image/png\",\"annotations\":{\"audience\":[\"user\"],"
        "\"priority\":1.0}}]}");
  } else if (strcmp(method, "prompts/list") == 0) {
    rc = send_rpc_result_id(
        client_fd, id_val,
        "{\"prompts\":[{\"name\":\"generate_sdk\",\"description\":\"Generate "
        "SDK prompt\",\"arguments\":[{\"name\":\"language\",\"description\":"
        "\"Target "
        "language\",\"required\":true}]},{\"name\":\"generate_arazzo\","
        "\"description\":\"Generate Arazzo workflow orchestrator\","
        "\"arguments\":[{\"name\":\"subcommand\",\"description\":"
        "\"to_sdk, to_sdk_cli, or to_server\",\"required\":false}]}]}");
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
    (void)messages;
    (void)maxTokens;
    (void)includeContext;
    (void)metadata;
    (void)modelPreferences;
    (void)stopSequences;
    (void)systemPrompt;
    (void)temperature;
    rc = send_rpc_result_id(
        client_fd, id_val,
        "{\"role\":\"assistant\",\"content\":{\"type\":\"text\",\"text\":"
        "\"Sampled message\"},\"model\":\"test-model\",\"stopReason\":"
        "\"endSeq\"}");
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
    rc = send_rpc_result_id(client_fd, id_val, prompt_buf);
  } else if (strcmp(method, "completion/complete") == 0) {
    rc = send_rpc_result_id(
        client_fd, id_val,
        "{\"completion\":{\"values\":[\"to_sdk\",\"to_sdk_cli\",\"to_server\","
        "\"to_openapi\",\"to_arazzo\",\"from_arazzo\"],\"hasMore\":false,"
        "\"total\":6}}");
  } else if (strcmp(method, "tools/call") == 0) {
    JSON_Object *params = json_object_get_object(root_obj, "params");
    const char *name = params ? json_object_get_string(params, "name") : NULL;
    JSON_Object *arguments =
        params ? json_object_get_object(params, "arguments") : NULL;

    if (!name || !arguments) {
      rc = send_rpc_error_id(client_fd, id_val, -32602,
                             "Invalid params for tools/call");
    } else if (strcmp(name, "to_openapi") == 0) {
      const char *input = json_object_get_string(arguments, "input");
      const char *output = json_object_get_string(arguments, "output");
      if (!input || !output) {
        rc = send_rpc_error_id(client_fd, id_val, -32602,
                               "Invalid arguments for to_openapi");
      } else {
        cdd_to_openapi_config_t cfg;
        cdd_c_error_t rc_rpc;
        memset(&cfg, 0, sizeof(cfg));
        cfg.input = input;
        cfg.output = output;
        rc_rpc = cdd_generate_to_openapi(&cfg);
        if (rc_rpc != CDD_C_SUCCESS) {
          rc = send_rpc_error_id(client_fd, id_val, -32603,
                                 "OpenAPI generation failed");
        } else {
          rc = send_rpc_result_id(
              client_fd, id_val,
              "{\"content\":[{\"type\":\"text\",\"text\":\"OpenAPI generation "
              "successful\"}],\"isError\":false}");
        }
      }
    } else if (strcmp(name, "to_docs_json") == 0) {
      const char *input = json_object_get_string(arguments, "input");
      const char *output = json_object_get_string(arguments, "output");
      if (!input) {
        rc = send_rpc_error_id(client_fd, id_val, -32602,
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
          rc = send_rpc_error_id(client_fd, id_val, -32603,
                                 "Docs generation failed");
        } else {
          rc = send_rpc_result_id(
              client_fd, id_val,
              "{\"content\":[{\"type\":\"text\",\"text\":\"Docs generation "
              "successful\"}],\"isError\":false}");
        }
      }
    } else if (strcmp(name, "to_arazzo") == 0) {
      cdd_to_arazzo_config_t cfg;
      memset(&cfg, 0, sizeof(cfg));
      cfg.input = json_object_get_string(arguments, "input");
      cfg.output = json_object_get_string(arguments, "output");
      if (cdd_generate_to_arazzo(&cfg) != CDD_C_SUCCESS) {
        rc = send_rpc_error_id(client_fd, id_val, -32603,
                               "Arazzo extraction failed");
      } else {
        rc = send_rpc_result_id(client_fd, id_val, "\"ok\"");
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
        rc = send_rpc_error_id(client_fd, id_val, -32603,
                               "Arazzo generation failed");
      } else {
        rc = send_rpc_result_id(client_fd, id_val, "\"ok\"");
      }
    } else {
      rc = send_rpc_error_id(client_fd, id_val, -32601, "Tool not found");
    }
  } else if (strcmp(method, "to_openapi") == 0) {
    JSON_Object *params = json_object_get_object(root_obj, "params");
    const char *input = params ? json_object_get_string(params, "input") : NULL;
    const char *output =
        params ? json_object_get_string(params, "output") : NULL;
    if (!input || !output) {
      rc = send_rpc_error_id(client_fd, id_val, -32602, "Invalid params");
    } else {
      cdd_to_openapi_config_t cfg;
      cdd_c_error_t rc_sub;
      memset(&cfg, 0, sizeof(cfg));
      cfg.input = input;
      cfg.output = output;
      rc_sub = cdd_generate_to_openapi(&cfg);
      if (rc_sub != CDD_C_SUCCESS) {
        rc = send_rpc_error_id(client_fd, id_val, -32603,
                               "OpenAPI generation failed");
      } else {
        rc = send_rpc_result_id(client_fd, id_val, "\"ok\"");
      }
    }
  } else if (strcmp(method, "to_docs_json") == 0) {
    JSON_Object *params = json_object_get_object(root_obj, "params");
    const char *input = params ? json_object_get_string(params, "input") : NULL;
    const char *output =
        params ? json_object_get_string(params, "output") : NULL;
    if (!input) {
      rc = send_rpc_error_id(client_fd, id_val, -32602, "Invalid params");
    } else {
      cdd_docs_json_config_t cfg;
      cdd_c_error_t rc_sub;
      memset(&cfg, 0, sizeof(cfg));
      cfg.input = input;
      cfg.output = output;
      if (json_object_get_boolean(params, "no_imports") == 1) {
        cfg.no_imports = CDD_TRUE;
      }
      if (json_object_get_boolean(params, "no_wrapping") == 1) {
        cfg.no_wrapping = CDD_TRUE;
      }
      rc_sub = cdd_generate_docs_json(&cfg);
      if (rc_sub != CDD_C_SUCCESS) {
        rc = send_rpc_error_id(client_fd, id_val, -32603,
                               "Docs generation failed");
      } else {
        rc = send_rpc_result_id(client_fd, id_val, "\"ok\"");
      }
    }
  } else if (strncmp(method, "from_openapi_", 13) == 0) {
    JSON_Object *params = json_object_get_object(root_obj, "params");
    const char *input = params ? json_object_get_string(params, "input") : NULL;
    const char *input_dir =
        params ? json_object_get_string(params, "input_dir") : NULL;
    const char *output =
        params ? json_object_get_string(params, "output") : NULL;
    cdd_from_openapi_config_t cfg;
    memset(&cfg, 0, sizeof(cfg));

    if (strcmp(method, "from_openapi_to_sdk") == 0) {
      cfg.subcommand = CDD_OPENAPI_SUBCOMMAND_TO_SDK;
    } else if (strcmp(method, "from_openapi_to_sdk_cli") == 0) {
      cfg.subcommand = CDD_OPENAPI_SUBCOMMAND_TO_SDK_CLI;
    } else if (strcmp(method, "from_openapi_to_server") == 0) {
      cfg.subcommand = CDD_OPENAPI_SUBCOMMAND_TO_SERVER;
    } else {
      rc = send_rpc_error_id(client_fd, id_val, -32601, "Method not found");
      json_value_free(root_val);
      return rc;
    }

    if (input) {
      cfg.input = input;
    } else if (input_dir) {
      cfg.input_dir = input_dir;
    }

    if (output) {
      cfg.output = output;
    }

    if (params && json_object_get_boolean(params, "no_github_actions") == 1) {
      cfg.no_github_actions = CDD_TRUE;
    }
    if (params &&
        json_object_get_boolean(params, "no_installable_package") == 1) {
      cfg.no_installable_package = CDD_TRUE;
    }
    if (params && json_object_get_boolean(params, "tests") == 1) {
      cfg.tests = CDD_TRUE;
    }

    {
      cdd_c_error_t rc_sub = cdd_generate_from_openapi(&cfg);
      if (rc_sub != CDD_C_SUCCESS) {
        rc = send_rpc_error_id(client_fd, id_val, -32603,
                               "OpenAPI generation failed");
      } else {
        rc = send_rpc_result_id(client_fd, id_val, "\"ok\"");
      }
    }
  } else {
    rc = send_rpc_error_id(client_fd, id_val, -32601, "Method not found");
  }

  json_value_free(root_val);
  return rc;
}
#endif /* !defined(__WATCOMC__) && !defined(__DOS__) &&                        \
          !defined(__EMSCRIPTEN__) */

/**
 * @brief Performs server json rpc operation from a typed config struct.
 *
 * @param[in] config Configuration struct.
 * @return CDD_C_SUCCESS on success, or error code.
 */
C_CDD_EXPORT cdd_c_error_t
serve_json_rpc_run(const cdd_serve_json_rpc_config_t *config) {
#if defined(__WATCOMC__) || defined(__DOS__) || defined(__EMSCRIPTEN__)
  (void)config;
  fprintf(stderr, "Server mode is not supported on this platform.\n");
  return CDD_C_ERROR_UNKNOWN;
#else
  int port;
  size_t remaining_requests;
  cdd_socket_t server_fd;
  struct sockaddr_in addr;
  cdd_c_error_t rc;

  if (!config)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  port = (config->port > 0) ? config->port : 8080;
  remaining_requests = config->max_requests;

  printf("Starting JSON-RPC server on %s:%d...\n",
         config->bind_address ? config->bind_address : "0.0.0.0", port);

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
  if (config->bind_address && strcmp(config->bind_address, "0.0.0.0") != 0) {
#if defined(_WIN32)
    if (inet_pton(AF_INET, config->bind_address, &addr.sin_addr) != 1) {
      addr.sin_addr.s_addr = htonl(INADDR_ANY);
    }
#else
    addr.sin_addr.s_addr = inet_addr(config->bind_address);
#endif
  } else {
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
  }
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

  while (remaining_requests > 0) {
    cdd_socket_t client_fd;
    struct sockaddr_in client_addr;
#if defined(_WIN32)
    int addr_len = sizeof(client_addr);
#else
    socklen_t addr_len = sizeof(client_addr);
#endif
    remaining_requests--;
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
 * @brief CLI entry point for running the JSON-RPC 2.0 server over HTTP or
 * stdio.
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
  cdd_serve_json_rpc_config_t cfg;
  int i;

  memset(&cfg, 0, sizeof(cfg));
  cfg.port = getenv("CDD_PORT") ? atoi(getenv("CDD_PORT")) : 8080;
  cfg.bind_address = "0.0.0.0";
  cfg.max_requests =
      getenv("CDD_LISTEN") ? (size_t)atoi(getenv("CDD_LISTEN")) : 0;

  for (i = 0; i < argc; i++) {
    if (strcmp(argv[i], "--port") == 0 || strcmp(argv[i], "-p") == 0) {
      if (i + 1 < argc) {
        cfg.port = atoi(argv[++i]);
      } else {
        return CDD_C_ERROR_INVALID_ARGUMENT;
      }
    } else if (strcmp(argv[i], "--listen") == 0 || strcmp(argv[i], "-l") == 0) {
      cfg.max_requests = 1;
      if (i + 1 < argc) {
        const char *arg_val = argv[++i];
        if (strchr(arg_val, '.') != NULL) {
          cfg.bind_address = arg_val;
        } else {
          int parsed = atoi(arg_val);
          cfg.max_requests =
              (parsed == 255 || parsed <= 0) ? 0 : (size_t)parsed;
        }
      }
    } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
      puts("Usage: cdd-c serve_json_rpc [args]");
      puts("");
      puts("Options:");
      puts("  -p, --port <port>       Port to listen on (default: 8080)");
      puts("  -l, --listen <address>  Address or connection limit to bind");
      return CDD_C_SUCCESS;
    } else if (argv[i][0] == '-') {
      fprintf(stderr, "Unknown option for serve_json_rpc: %s\n", argv[i]);
      return CDD_C_ERROR_INVALID_ARGUMENT;
    }
  }

  return serve_json_rpc_run(&cfg);
#endif
}

#else
cdd_c_error_t serve_json_rpc_run(const cdd_serve_json_rpc_config_t *config) {
  (void)config;
  return CDD_C_ERROR_UNKNOWN;
}

cdd_c_error_t serve_json_rpc_main(int argc, char **argv) {
  (void)argc;
  (void)argv;
  return CDD_C_ERROR_UNKNOWN;
}
#endif
