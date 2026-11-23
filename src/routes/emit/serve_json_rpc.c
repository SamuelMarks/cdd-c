/**
 * @file serve_json_rpc.c
 * @brief Implementation of JSON-RPC server generation.
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"
#include "c_cdd/context.h"
#include "c_cdd/log.h"
#include "c_cdd/memory.h"
#include "serve_json_rpc.h"

#include "../parse/cli.h"
#include "../../cdd_api.h"
#include "classes/emit/cli_struct.h"
#include "classes/parse/cdd_serde.h"
#include "functions/parse/fs.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(__WATCOMC__) || defined(__DOS__) || defined(__EMSCRIPTEN__) ||     \
    defined(__wasi__)
/* No sockets on DOS/Watcom/WASI natively */
#else
#if defined(_WIN32)
#include <winsock2.h>
#include <ws2tcpip.h>
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

#if defined(_WIN32) || defined(__WATCOMC__) || defined(__DOS__) ||             \
    defined(__EMSCRIPTEN__) || defined(__wasi__)
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

#if defined(__WATCOMC__) || defined(__DOS__) || defined(__EMSCRIPTEN__) ||     \
    defined(__wasi__)
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

#if !defined(__WATCOMC__) && !defined(__DOS__) && !defined(__EMSCRIPTEN__) &&  \
    !defined(__wasi__)

/**
 * @brief Retrieves the active runtime context with step-fault injection
 * support.
 *
 * @param[out] out_ctx Output active context pointer.
 * @return CDD_C_SUCCESS on success, or error code.
 */
static cdd_c_error_t serve_get_ctx(const cdd_context_t **out_ctx) {
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  return cdd_context_get_global(out_ctx);
}

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
  cdd_c_error_t rc = serve_get_ctx(&ctx);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (ctx->socket_vt.socket_fn) {
    rc = ctx->socket_vt.socket_fn(domain, type, protocol, &fd);
    if (rc != CDD_C_SUCCESS)
      return rc;
    *out_socket = (cdd_socket_t)fd;
  } else {
    *out_socket = socket(domain, type, protocol);
  }
  if (*out_socket == INVALID_SOCKET)
    return CDD_C_ERROR_SYSTEM;
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
  int res = 0;
  cdd_c_error_t rc = serve_get_ctx(&ctx);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (ctx->socket_vt.listen_fn) {
    return ctx->socket_vt.listen_fn((int)s, backlog);
  }
  res = listen(s, backlog);
  if (res < 0)
    return CDD_C_ERROR_SYSTEM;
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
  cdd_c_error_t rc = serve_get_ctx(&ctx);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (ctx->socket_vt.accept_fn) {
    rc = ctx->socket_vt.accept_fn((int)s, &client_fd);
    if (rc != CDD_C_SUCCESS)
      return rc;
    *out_client = (cdd_socket_t)client_fd;
  } else {
    *out_client = accept(s, addr, addrlen);
  }
  if (*out_client == INVALID_SOCKET)
    return CDD_C_ERROR_SYSTEM;
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
  cdd_ssize_t sent = 0;
  size_t out_sent = 0;
  cdd_c_error_t rc = serve_get_ctx(&ctx);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (ctx->socket_vt.send_fn) {
    rc = ctx->socket_vt.send_fn((int)s, b, l, &out_sent);
    if (rc != CDD_C_SUCCESS)
      return rc;
    sent = (cdd_ssize_t)out_sent - 1;
  } else {
    sent = send(s, b, CDD_SEND_LEN_CAST(l), f);
  }
  if (sent < 0)
    return CDD_C_ERROR_SYSTEM;
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
  struct CddJsonRpcResponse env;
  char *body_json = NULL;
  char resp[2048];
  cdd_c_error_t rc;
  memset(&env, 0, sizeof(env));
  env.jsonrpc = "2.0";
  env.error.code = code;
  env.error.message = msg;
  env.id_val = id_val;
  rc = CddJsonRpcResponse_to_json(&env, &body_json);
  if (rc != CDD_C_SUCCESS)
    return rc;
#if defined(_MSC_VER)
  sprintf_s(resp, sizeof(resp),
            "HTTP/1.1 200 OK\r\nContent-Type: "
            "application/json\r\n\r\n%s",
            body_json);
#else
  CDD_SNPRINTF(resp, sizeof(resp),
               "HTTP/1.1 200 OK\r\nContent-Type: "
               "application/json\r\n\r\n%s",
               body_json);
#endif
  return cdd_c_keep_err(send_rpc_response(client_fd, resp),
                        rpc_serialized_str_free(body_json));
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
  struct CddJsonRpcResponse env;
  char *body_json = NULL;
  char resp[8192];
  cdd_c_error_t rc;
  memset(&env, 0, sizeof(env));
  env.jsonrpc = "2.0";
  env.result_json = result_json;
  env.id_val = id_val;
  rc = CddJsonRpcResponse_to_json(&env, &body_json);
  if (rc != CDD_C_SUCCESS)
    return rc;
#if defined(_MSC_VER)
  sprintf_s(resp, sizeof(resp),
            "HTTP/1.1 200 OK\r\nContent-Type: "
            "application/json\r\n\r\n%s",
            body_json);
#else
  CDD_SNPRINTF(resp, sizeof(resp),
               "HTTP/1.1 200 OK\r\nContent-Type: "
               "application/json\r\n\r\n%s",
               body_json);
#endif
  return cdd_c_keep_err(send_rpc_response(client_fd, resp),
                        rpc_serialized_str_free(body_json));
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
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
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
 * @brief Dispatches an MCP `tools/call` invocation over HTTP JSON-RPC.
 *
 * @param[in] client_fd Client socket descriptor.
 * @param[in] id_val Optional JSON-RPC request ID.
 * @param[in] params Optional JSON params object.
 * @return CDD_C_SUCCESS on success, or error code.
 */
static cdd_c_error_t handle_http_tool_call(cdd_socket_t client_fd,
                                           JSON_Value *id_val,
                                           const JSON_Object *params) {
  struct CddMcpToolCallParams tc_params;
  int ok = 0;
  cdd_c_error_t rc;

  rc = rpc_try_tool_call_params(params, &tc_params, &ok);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (!ok) {
    return send_rpc_error_id(client_fd, id_val, -32602,
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
      return send_rpc_error_id(client_fd, id_val, -32602,
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
      return send_rpc_error_id(client_fd, id_val, -32603,
                               "OpenAPI generation failed");
    }
    return send_rpc_result_id(
        client_fd, id_val,
        "{\"content\":[{\"type\":\"text\",\"text\":\"OpenAPI "
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
      return send_rpc_error_id(client_fd, id_val, -32602,
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
      return send_rpc_error_id(client_fd, id_val, -32603,
                               "Docs generation failed");
    }
    return send_rpc_result_id(
        client_fd, id_val,
        "{\"content\":[{\"type\":\"text\",\"text\":\"Docs generation "
        "successful\"}],\"isError\":false}");
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
      return send_rpc_error_id(client_fd, id_val, -32603,
                               "Arazzo extraction failed");
    }
    return send_rpc_result_id(client_fd, id_val, "\"ok\"");
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
      return send_rpc_error_id(client_fd, id_val, -32603,
                               "Arazzo generation failed");
    }
    return send_rpc_result_id(client_fd, id_val, "\"ok\"");
  }

  return send_rpc_error_id(client_fd, id_val, -32601, "Tool not found");
}

/**
 * @brief Sends a JSON-RPC result and then releases `dyn_str` and `root_val`
 * in defined C89 sequence order.
 *
 * @param[in] client_fd Client socket descriptor.
 * @param[in] id_val JSON-RPC request ID value (owned by `root_val`).
 * @param[in] result_json Serialized JSON result payload.
 * @param[in,out] dyn_str Optional heap string to free after sending.
 * @param[in,out] root_val Request JSON DOM root to free after sending.
 * @return CDD_C_SUCCESS on success, or error code.
 */
static cdd_c_error_t send_rpc_result_cleanup(cdd_socket_t client_fd,
                                             JSON_Value *id_val,
                                             const char *result_json,
                                             char *dyn_str,
                                             JSON_Value *root_val) {
  cdd_c_error_t rc = send_rpc_result_id(client_fd, id_val, result_json);
  if (rc != CDD_C_SUCCESS) {
    return cdd_c_keep_err(rc, cdd_c_keep_err(rpc_serialized_str_free(dyn_str),
                                             cdd_json_value_free(root_val)));
  }
  return cdd_c_keep_err(rpc_serialized_str_free(dyn_str),
                        cdd_json_value_free(root_val));
}

/**
 * @brief Sends a JSON-RPC error and then releases `root_val` in defined C89
 * sequence order.
 *
 * @param[in] client_fd Client socket descriptor.
 * @param[in] id_val JSON-RPC request ID value (owned by `root_val`).
 * @param[in] code JSON-RPC error code.
 * @param[in] message Error message string.
 * @param[in,out] root_val Request JSON DOM root to free after sending.
 * @return CDD_C_SUCCESS on success, or error code.
 */
static cdd_c_error_t send_rpc_error_cleanup(cdd_socket_t client_fd,
                                            JSON_Value *id_val, int code,
                                            const char *message,
                                            JSON_Value *root_val) {
  cdd_c_error_t rc = send_rpc_error_id(client_fd, id_val, code, message);
  if (rc != CDD_C_SUCCESS) {
    return cdd_c_keep_err(rc, cdd_json_value_free(root_val));
  }
  return cdd_json_value_free(root_val);
}

C_CDD_EXPORT cdd_c_error_t serve_json_rpc_dispatch_body(int client_fd_int,
                                                        const char *body) {
  cdd_socket_t client_fd = (cdd_socket_t)client_fd_int;
  JSON_Value *root_val = NULL;
  JSON_Object *root_obj = NULL;
  JSON_Value *id_val = NULL;
  const char *method = NULL;
  char *dyn_str = NULL;
  int parse_ok = 0;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  if (!body)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  rc = rpc_json_try_parse(body, &root_val, &parse_ok);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (!parse_ok) {
    return send_rpc_error_id(client_fd, NULL, -32700, "Parse error");
  }

  rc = cdd_json_value_get_object(root_val, &root_obj);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;
  rc = rpc_json_opt_value(root_obj, "id", &id_val);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;
  rc = rpc_json_opt_string(root_obj, "method", &method);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;
  if (!method) {
    return send_rpc_error_cleanup(client_fd, id_val, -32600, "Invalid Request",
                                  root_val);
  }

  if (strcmp(method, "version") == 0) {
    static const char k_ver_json[] = {34, '0', '.', '0', '.', '5', 34, 0};
    return send_rpc_result_cleanup(client_fd, id_val, k_ver_json, NULL,
                                   root_val);
  }
  if (strcmp(method, "initialize") == 0) {
    return send_rpc_result_cleanup(
        client_fd, id_val,
        "{\"protocolVersion\":\"2024-11-05\",\"capabilities\":{\"tools\":{"
        "\"listChanged\":true},\"resources\":{\"listChanged\":true,"
        "\"subscribe\":false},\"prompts\":{\"listChanged\":true},"
        "\"logging\":{}},\"serverInfo\":{\"name\":\"cdd-c\",\"version\":"
        "\"0.0.5\"}}",
        NULL, root_val);
  }
  if (strcmp(method, "notifications/initialized") == 0 ||
      strcmp(method, "notifications/progress") == 0 ||
      strcmp(method, "notifications/message") == 0) {
    return cdd_json_value_free(root_val);
  }
  if (strcmp(method, "logging/setLevel") == 0 || strcmp(method, "ping") == 0 ||
      strcmp(method, "resources/subscribe") == 0 ||
      strcmp(method, "resources/unsubscribe") == 0) {
    return send_rpc_result_cleanup(client_fd, id_val, "{}", NULL, root_val);
  }
  if (strcmp(method, "tools/list") == 0) {
    return send_rpc_result_cleanup(
        client_fd, id_val,
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
    return send_rpc_result_cleanup(
        client_fd, id_val,
        "{\"resources\":[{\"uri\":\"file:///"
        "openapi.json\",\"name\":\"OpenAPI "
        "Spec\",\"mimeType\":\"application/json\"},{\"uri\":"
        "\"file:///arazzo.json\",\"name\":\"Arazzo Workflow Spec\","
        "\"mimeType\":\"application/json\"}]}",
        NULL, root_val);
  }
  if (strcmp(method, "roots/list") == 0) {
    return send_rpc_result_cleanup(client_fd, id_val,
                                   "{\"roots\":[{\"uri\":\"file:///"
                                   "workspace\",\"name\":\"workspace\"}]}",
                                   NULL, root_val);
  }
  if (strcmp(method, "resources/templates/list") == 0) {
    return send_rpc_result_cleanup(
        client_fd, id_val,
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
    return send_rpc_result_cleanup(client_fd, id_val, dyn_str, dyn_str,
                                   root_val);
  }
  if (strcmp(method, "tools/read_image") == 0) {
    return send_rpc_result_cleanup(
        client_fd, id_val,
        "{\"content\":[{\"type\":\"image\",\"data\":\"iVBORw0KGgo=\","
        "\"mimeType\":\"image/"
        "png\",\"annotations\":{\"audience\":[\"user\"],"
        "\"priority\":1.0}}]}",
        NULL, root_val);
  }
  if (strcmp(method, "prompts/list") == 0) {
    return send_rpc_result_cleanup(
        client_fd, id_val,
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
    JSON_Object *params = NULL;
    struct CddMcpCreateMessageParams cm_params;
    memset(&cm_params, 0, sizeof(cm_params));
    rc = rpc_json_opt_object(root_obj, "params", &params);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    if (params) {
      rc = cdd_reflect_from_json_object(
          params, &CddMcpCreateMessageParams_type_info, &cm_params);
      if (rc != CDD_C_SUCCESS) {
        return cdd_c_keep_err(
            rc, cdd_c_keep_err(
                    rpc_reflect_struct_free(
                        &cm_params, &CddMcpCreateMessageParams_type_info),
                    cdd_json_value_free(root_val)));
      }
    }
    rc = rpc_reflect_struct_free(&cm_params,
                                 &CddMcpCreateMessageParams_type_info);
    return cdd_c_keep_err(
        rc,
        send_rpc_result_cleanup(
            client_fd, id_val,
            "{\"role\":\"assistant\",\"content\":{\"type\":\"text\",\"text\":"
            "\"Sampled message\"},\"model\":\"test-model\",\"stopReason\":"
            "\"endSeq\"}",
            NULL, root_val));
  }
  if (strcmp(method, "prompts/get") == 0) {
    JSON_Object *params = NULL;
    rc = rpc_json_opt_object(root_obj, "params", &params);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    rc = build_mcp_prompts_get(params, &dyn_str);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    return send_rpc_result_cleanup(client_fd, id_val, dyn_str, dyn_str,
                                   root_val);
  }
  if (strcmp(method, "completion/complete") == 0) {
    return send_rpc_result_cleanup(
        client_fd, id_val,
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
    rc = handle_http_tool_call(client_fd, id_val, params);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    return cdd_json_value_free(root_val);
  }
  if (strcmp(method, "to_openapi") == 0) {
    JSON_Object *params = NULL;
    const char *input = NULL;
    const char *output = NULL;
    rc = rpc_json_opt_object(root_obj, "params", &params);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    rc = rpc_json_opt_string(params, "input", &input);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    rc = rpc_json_opt_string(params, "output", &output);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    if (!input || !output) {
      return send_rpc_error_cleanup(client_fd, id_val, -32602, "Invalid params",
                                    root_val);
    }
    {
      cdd_to_openapi_config_t cfg;
      int gen_ok = 0;
      memset(&cfg, 0, sizeof(cfg));
      cfg.input = input;
      cfg.output = output;
      rc = rpc_exec_to_openapi(&cfg, &gen_ok);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
      if (!gen_ok) {
        return send_rpc_error_cleanup(client_fd, id_val, -32603,
                                      "OpenAPI generation failed", root_val);
      }
      return send_rpc_result_cleanup(client_fd, id_val, "\"ok\"", NULL,
                                     root_val);
    }
  }
  if (strcmp(method, "to_docs_json") == 0) {
    JSON_Object *params = NULL;
    const char *input = NULL;
    const char *output = NULL;
    rc = rpc_json_opt_object(root_obj, "params", &params);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    rc = rpc_json_opt_string(params, "input", &input);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    rc = rpc_json_opt_string(params, "output", &output);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    if (!input) {
      return send_rpc_error_cleanup(client_fd, id_val, -32602, "Invalid params",
                                    root_val);
    }
    {
      cdd_docs_json_config_t cfg;
      int bval = 0;
      int gen_ok = 0;
      memset(&cfg, 0, sizeof(cfg));
      cfg.input = input;
      cfg.output = output;
      rc = rpc_json_opt_bool(params, "no_imports", &bval);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
      cfg.no_imports = (bval == 1) ? CDD_TRUE : CDD_FALSE;
      rc = rpc_json_opt_bool(params, "no_wrapping", &bval);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
      cfg.no_wrapping = (bval == 1) ? CDD_TRUE : CDD_FALSE;
      rc = rpc_exec_docs_json(&cfg, &gen_ok);
      if (rc != CDD_C_SUCCESS)
        goto cleanup;
      if (!gen_ok) {
        return send_rpc_error_cleanup(client_fd, id_val, -32603,
                                      "Docs generation failed", root_val);
      }
      return send_rpc_result_cleanup(client_fd, id_val, "\"ok\"", NULL,
                                     root_val);
    }
  }
  if (strncmp(method, "from_openapi_", 13) == 0) {
    JSON_Object *params = NULL;
    const char *input = NULL;
    const char *input_dir = NULL;
    const char *output = NULL;
    cdd_from_openapi_config_t cfg;
    int bval = 0;
    int gen_ok = 0;
    memset(&cfg, 0, sizeof(cfg));
    rc = rpc_json_opt_object(root_obj, "params", &params);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    rc = rpc_json_opt_string(params, "input", &input);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    rc = rpc_json_opt_string(params, "input_dir", &input_dir);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    rc = rpc_json_opt_string(params, "output", &output);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;

    if (strcmp(method, "from_openapi_to_sdk") == 0) {
      cfg.subcommand = CDD_OPENAPI_SUBCOMMAND_TO_SDK;
    } else if (strcmp(method, "from_openapi_to_sdk_cli") == 0) {
      cfg.subcommand = CDD_OPENAPI_SUBCOMMAND_TO_SDK_CLI;
    } else if (strcmp(method, "from_openapi_to_server") == 0) {
      cfg.subcommand = CDD_OPENAPI_SUBCOMMAND_TO_SERVER;
    } else {
      return send_rpc_error_cleanup(client_fd, id_val, -32601,
                                    "Method not found", root_val);
    }

    if (input) {
      cfg.input = input;
    } else if (input_dir) {
      cfg.input_dir = input_dir;
    }
    if (output)
      cfg.output = output;

    rc = rpc_json_opt_bool(params, "no_github_actions", &bval);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    cfg.no_github_actions = (bval == 1) ? CDD_TRUE : CDD_FALSE;
    rc = rpc_json_opt_bool(params, "no_installable_package", &bval);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    cfg.no_installable_package = (bval == 1) ? CDD_TRUE : CDD_FALSE;
    rc = rpc_json_opt_bool(params, "tests", &bval);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    cfg.tests = (bval == 1) ? CDD_TRUE : CDD_FALSE;

    rc = rpc_exec_from_openapi(&cfg, &gen_ok);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    if (!gen_ok) {
      return send_rpc_error_cleanup(client_fd, id_val, -32603,
                                    "OpenAPI generation failed", root_val);
    }
    return send_rpc_result_cleanup(client_fd, id_val, "\"ok\"", NULL, root_val);
  }
  return send_rpc_error_cleanup(client_fd, id_val, -32601, "Method not found",
                                root_val);

cleanup:;
  return cdd_c_keep_err(rc, cdd_c_keep_err(rpc_serialized_str_free(dyn_str),
                                           cdd_json_value_free(root_val)));
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
  cdd_c_error_t rc;

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
      rc = parse_content_length(buffer, (size_t)(body - buffer), &content_len);
      if (rc != CDD_C_SUCCESS)
        return rc;
      body += 4;
      if ((size_t)(buffer + total_received - body) >= content_len)
        break;
    }
  }
  if (total_received == 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (!body) {
    return send_rpc_error_id(client_fd, NULL, -32700, "Parse error");
  }
  return serve_json_rpc_dispatch_body((int)client_fd, body);
}
/**
 * @brief Attempts to accept a client socket inside the server loop, setting
 * `*out_accepted` to 1 on success or 0 on transient accept failure.
 *
 * @param[in] server_fd Listening socket descriptor.
 * @param[out] addr Client address storage.
 * @param[in,out] addrlen Client address length.
 * @param[out] out_client Accepted client descriptor.
 * @param[out] out_accepted Receives 1 when accepted, 0 otherwise.
 * @return CDD_C_SUCCESS on completion, or error code on fault injection.
 */
static cdd_c_error_t try_accept_client(cdd_socket_t server_fd,
                                       struct sockaddr *addr,
#if defined(_WIN32)
                                       int *addrlen,
#else
                                       socklen_t *addrlen,
#endif
                                       cdd_socket_t *out_client,
                                       int *out_accepted) {
  cdd_c_error_t rc;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  *out_accepted = 0;
  rc = serve_accept_client(server_fd, addr, addrlen, out_client);
  if (rc == CDD_C_SUCCESS) {
    *out_accepted = 1;
    return rc;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Processes and closes one accepted client connection without aborting
 * the listener loop on client disconnect or malformed payload.
 *
 * @param[in] client_fd Accepted client descriptor.
 * @return CDD_C_SUCCESS on completion, or error code on fault injection.
 */
static cdd_c_error_t try_handle_request(cdd_socket_t client_fd) {
  cdd_c_error_t rc;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0) {
#if defined(_WIN32)
    closesocket(client_fd);
#else
    close(client_fd);
#endif
    return CDD_C_ERROR_UNKNOWN;
  }
#endif
  rc = handle_request(client_fd);
  if (rc == CDD_C_SUCCESS) {
#if defined(_WIN32)
    closesocket(client_fd);
#else
    close(client_fd);
#endif
    return rc;
  }
#if defined(_WIN32)
  closesocket(client_fd);
#else
  close(client_fd);
#endif
  C_CDD_LOG_DEBUG("handle_request error: %d\n", rc);
  return CDD_C_SUCCESS;
}
#endif /* !defined(__WATCOMC__) && !defined(__DOS__) &&                        \
          !defined(__EMSCRIPTEN__) && !defined(__wasi__) */

#if defined(__WATCOMC__) || defined(__DOS__) || defined(__EMSCRIPTEN__) ||     \
    defined(__wasi__)
C_CDD_EXPORT cdd_c_error_t serve_json_rpc_dispatch_body(int client_fd_int,
                                                        const char *body) {
  if (client_fd_int < -1 || !body)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  return CDD_C_ERROR_UNKNOWN;
}
#endif

/**
 * @brief Performs server json rpc operation from a typed config struct.
 *
 * @param[in] config Configuration struct.
 * @return CDD_C_SUCCESS on success, or error code.
 */
C_CDD_EXPORT cdd_c_error_t
serve_json_rpc_run(const cdd_serve_json_rpc_config_t *config) {
#if defined(__WATCOMC__) || defined(__DOS__) || defined(__EMSCRIPTEN__) ||     \
    defined(__wasi__)
  if (!config)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  fprintf(stderr, "Server mode is not supported on this platform.\n");
  return CDD_C_ERROR_UNKNOWN;
#else
  int port;
  size_t remaining_requests;
  cdd_socket_t server_fd;
  struct sockaddr_in addr;
  cdd_c_error_t rc = CDD_C_SUCCESS;

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

#if !defined(_WIN32)
  {
    int reuse = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (const void *)&reuse,
               (socklen_t)sizeof(reuse));
  }
#endif

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
    rc = CDD_C_ERROR_SYSTEM;
    goto cleanup_srv;
  }

  rc = serve_listen_start(server_fd, SOMAXCONN);
  if (rc != CDD_C_SUCCESS) {
    perror("listen");
    goto cleanup_srv;
  }

  while (remaining_requests > 0) {
    cdd_socket_t client_fd;
    struct sockaddr_in client_addr;
    int accepted = 0;
#if defined(_WIN32)
    int addr_len = sizeof(client_addr);
#else
    socklen_t addr_len = sizeof(client_addr);
#endif
    remaining_requests--;
    rc = try_accept_client(server_fd, (struct sockaddr *)&client_addr,
                           &addr_len, &client_fd, &accepted);
    if (rc != CDD_C_SUCCESS)
      goto cleanup_srv;
    if (!accepted)
      continue;

    rc = try_handle_request(client_fd);
    if (rc != CDD_C_SUCCESS)
      goto cleanup_srv;
  }

cleanup_srv:
#if defined(_WIN32)
  closesocket(server_fd);
  WSACleanup();
#else
  close(server_fd);
#endif
  return rc;
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
#if defined(__WATCOMC__) || defined(__DOS__) || defined(__EMSCRIPTEN__) ||     \
    defined(__wasi__)
  if (argc < 0 || !argv)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  fprintf(stderr, "Server mode is not supported on this platform.\n");
  return CDD_C_ERROR_UNKNOWN;
#else
  cdd_serve_json_rpc_config_t cfg;
  int parse_argc = argc;
  int is_help = 0;
  int first_unk = -1;
  cdd_c_error_t rc;

  memset(&cfg, 0, sizeof(cfg));
  cfg.port = getenv("CDD_PORT") ? atoi(getenv("CDD_PORT")) : 8080;
  cfg.bind_address = NULL;
  cfg.max_requests =
      getenv("CDD_LISTEN") ? (size_t)atoi(getenv("CDD_LISTEN")) : 0;

  if (parse_argc >= 2 && (strcmp(argv[parse_argc - 1], "-l") == 0 ||
                          strcmp(argv[parse_argc - 1], "--listen") == 0)) {
    cfg.max_requests = 1;
    parse_argc -= 1;
  }

  rc = cdd_cli_parse_reflected_argv(parse_argc, argv, 1,
                                    &cdd_serve_json_rpc_config_type_info, &cfg,
                                    &is_help, &first_unk);
  if (rc != CDD_C_SUCCESS)
    return rc;

  if (is_help) {
    puts("Usage: cdd-c serve_json_rpc [args]");
    puts("");
    puts("Options:");
    puts("  -p, --port <port>       Port to listen on (default: 8080)");
    puts("  -l, --listen <address>  Address or connection limit to bind");
    return CDD_C_SUCCESS;
  }
  if (first_unk >= 0 && argv[first_unk][0] == '-') {
    fprintf(stderr, "Unknown option for serve_json_rpc: %s\n", argv[first_unk]);
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
  if (cfg.bind_address) {
    if (strchr(cfg.bind_address, '.') == NULL) {
      int parsed = atoi(cfg.bind_address);
      cfg.max_requests = (parsed == 255 || parsed <= 0) ? 0 : (size_t)parsed;
      cfg.bind_address = "0.0.0.0";
    } else {
      cfg.max_requests = 1;
    }
  } else {
    cfg.bind_address = "0.0.0.0";
  }

  return serve_json_rpc_run(&cfg);
#endif
}
