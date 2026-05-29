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
#include <unistd.h>
#endif
#endif
#endif
#endif

#if defined(__WATCOMC__) || defined(__DOS__)
typedef int cdd_socket_t;
#define INVALID_SOCKET (-1)
#else
#if defined(_WIN32)
typedef SOCKET cdd_socket_t;
#else
typedef int cdd_socket_t;
#ifndef INVALID_SOCKET
#define INVALID_SOCKET (-1)
#endif
#endif
#endif

/* Helper to respond with JSON-RPC error */
static void send_rpc_error(cdd_socket_t client_fd, int code, const char *msg) {
  char resp[1024];
  sprintf(resp, "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n{\"jsonrpc\":\"2.0\",\"error\":{\"code\":%d,\"message\":\"%s\"},\"id\":null}", code, msg);
  send(client_fd, resp, (int)strlen(resp), 0);
}

static void send_rpc_success(cdd_socket_t client_fd) {
  const char *resp = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n{\"jsonrpc\":\"2.0\",\"result\":\"ok\",\"id\":null}";
  send(client_fd, resp, (int)strlen(resp), 0);
}

static void handle_request(cdd_socket_t client_fd) {
  char buffer[65536];
  int bytes_received;
  char *body;
  JSON_Value *root_val;
  JSON_Object *root_obj;
  const char *method;

  bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
  if (bytes_received <= 0) return;
  buffer[bytes_received] = '\0';

  body = strstr(buffer, "\r\n\r\n");
  if (!body) {
    send_rpc_error(client_fd, -32700, "Parse error");
    return;
  }
  body += 4;

  root_val = json_parse_string(body);
  if (!root_val) {
    send_rpc_error(client_fd, -32700, "Parse error");
    return;
  }

  root_obj = json_value_get_object(root_val);
  method = json_object_get_string(root_obj, "method");
  if (!method) {
    send_rpc_error(client_fd, -32600, "Invalid Request");
    json_value_free(root_val);
    return;
  }

  if (strcmp(method, "version") == 0) {
    const char *resp = "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n\r\n{\"jsonrpc\":\"2.0\",\"result\":\"0.0.1\",\"id\":null}";
    send(client_fd, resp, (int)strlen(resp), 0);
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
      return;
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
}

/**
 * @brief Executes the server json rpc main operation.
 */
C_CDD_EXPORT int serve_json_rpc_main(int argc, char **argv) {
#if defined(__WATCOMC__) || defined(__DOS__)
  (void)argc;
  (void)argv;
  fprintf(stderr, "Server mode is not supported on DOS.\n");
  return 1;
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
      return 1;
  }
#endif

  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd == INVALID_SOCKET)
    return 1;

  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons((unsigned short)port);

  if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    perror("bind");
    return 1;
  }

  if (listen(server_fd, 5) < 0) {
    perror("listen");
    return 1;
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

  return 0;
#endif
}

#else
#include "serve_json_rpc.h"
/* clang-format on */
int serve_json_rpc_main(int argc, char **argv) {
  (void)argc;
  (void)argv;
  return -1;
}
#endif
