#include "server_json_rpc.h"
#include <parson.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

int server_json_rpc_main(int argc, char **argv) {
  int port = 8080;
  int listen_flag = 0;
  int i;
  int server_fd;
  struct sockaddr_in addr;
  (void)addr;

  for (i = 0; i < argc; i++) {
    if (strcmp(argv[i], "--port") == 0 && i + 1 < argc) {
      port = atoi(argv[i + 1]);
      i++;
    } else if (strcmp(argv[i], "--listen") == 0) {
      listen_flag = 1;
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
  if (server_fd < 0)
    return 1;

  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons(port);

  if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    perror("bind");
    return 1;
  }

  if (listen(server_fd, 5) < 0) {
    perror("listen");
    return 1;
  }

  while (listen_flag) {
    int client_fd;
    struct sockaddr_in client_addr;
#if defined(_WIN32)
    int addr_len = sizeof(client_addr);
#else
    socklen_t addr_len = sizeof(client_addr);
#endif
    client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
    if (client_fd < 0)
      continue;

    {
      const char *resp = "HTTP/1.1 200 OK\r\nContent-Type: "
                         "application/json\r\n\r\n{\"jsonrpc\": \"2.0\", "
                         "\"result\": \"ok\", \"id\": 1}";
      send(client_fd, resp, strlen(resp), 0);
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

  return 0;
}