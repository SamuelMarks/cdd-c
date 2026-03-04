/**
 * @file server_gen.c
 * @brief Implementation of server wrapper generation.
 */

#include "server_gen.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/** @brief openapi_server_generate definition */
int openapi_server_generate(const struct OpenAPI_Spec *spec,
                            const struct OpenApiClientConfig *config) {
  char path[1024];
  FILE *fp;
  size_t i, j;

  snprintf(path, sizeof(path), "%s_server.c", config->filename_base);
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  if (fopen_s(&fp, path, "w") != 0)
    fp = NULL;
#else
#if defined(_MSC_VER)
  fopen_s(&fp, path, "w");
#else
  fp = fopen(path, "w");
#endif
#endif
  if (!fp)
    return -1;

  fprintf(fp, "#include <stdio.h>\n");
  fprintf(fp, "#include <stdlib.h>\n");
  fprintf(fp, "#include <string.h>\n");
  fprintf(fp, "#if defined(_WIN32)\n");
  fprintf(fp, "#include <winsock2.h>\n");
  fprintf(fp, "#include <ws2tcpip.h>\n");
  fprintf(fp, "#pragma comment(lib, \"ws2_32.lib\")\n");
  fprintf(fp, "#else\n");
  fprintf(fp, "#include <arpa/inet.h>\n");
  fprintf(fp, "#include <netinet/in.h>\n");
  fprintf(fp, "#include <sys/socket.h>\n");
  fprintf(fp, "#include <unistd.h>\n");
  fprintf(fp, "#endif\n\n");

  /* PostgreSQL includes */
  fprintf(fp, "#ifdef USE_LIBPQ\n");
  fprintf(fp, "#include <libpq-fe.h>\n");
  fprintf(fp, "PGconn *db_conn = NULL;\n");
  fprintf(fp, "void init_db() {\n");
  fprintf(fp,
          "    db_conn = PQconnectdb(\"user=postgres dbname=postgres\");\n");
  fprintf(fp, "    if (PQstatus(db_conn) != CONNECTION_OK) {\n");
  fprintf(fp, "        fprintf(stderr, \"Connection to database failed: %%s\", "
              "PQerrorMessage(db_conn));\n");
  fprintf(fp, "        PQfinish(db_conn);\n");
  fprintf(fp, "        exit(1);\n");
  fprintf(fp, "    }\n");
  fprintf(fp, "}\n");
  fprintf(fp, "#endif\n\n");

  /* Route handlers */
  for (i = 0; i < spec->n_paths; i++) {
    for (j = 0; j < spec->paths[i].n_operations; j++) {
      const char *opId = spec->paths[i].operations[j].operation_id;
      if (opId) {
        fprintf(fp, "void handle_%s(int client_fd) {\n", opId);
        fprintf(fp,
                "    const char *resp = \"HTTP/1.1 200 OK\\r\\nContent-Type: "
                "application/json\\r\\n\\r\\n{\\\"status\\\": \\\"%s "
                "called\\\"}\";\n",
                opId);
        fprintf(fp, "    send(client_fd, resp, strlen(resp), 0);\n");
        fprintf(fp, "}\n\n");
      }
    }
  }

  fprintf(fp, "void route_request(int client_fd, const char *req) {\n");
  fprintf(fp, "    if (req == NULL) return;\n");

  /* Simple string matching router */
  for (i = 0; i < spec->n_paths; i++) {
    for (j = 0; j < spec->paths[i].n_operations; j++) {
      const char *opId = spec->paths[i].operations[j].operation_id;
      const char *method = "";
      switch (spec->paths[i].operations[j].verb) {
      case OA_VERB_GET:
        method = "GET";
        break;
      case OA_VERB_POST:
        method = "POST";
        break;
      case OA_VERB_PUT:
        method = "PUT";
        break;
      case OA_VERB_DELETE:
        method = "DELETE";
        break;
      case OA_VERB_OPTIONS:
        method = "OPTIONS";
        break;
      case OA_VERB_HEAD:
        method = "HEAD";
        break;
      case OA_VERB_PATCH:
        method = "PATCH";
        break;
      case OA_VERB_TRACE:
        method = "TRACE";
        break;
      default:
        method = "GET";
        break;
      }
      if (opId) {
        fprintf(
            fp,
            "    if (strncmp(req, \"%s %s \", strlen(\"%s %s \")) == 0) {\n",
            method, spec->paths[i].route, method, spec->paths[i].route);
        fprintf(fp, "        handle_%s(client_fd);\n", opId);
        fprintf(fp, "        return;\n");
        fprintf(fp, "    }\n");
      }
    }
  }

  fprintf(fp, "    /* 404 */\n");
  fprintf(fp, "    const char *not_found = \"HTTP/1.1 404 Not "
              "Found\\r\\nContent-Type: text/plain\\r\\n\\r\\nNot Found\";\n");
  fprintf(fp, "    send(client_fd, not_found, strlen(not_found), 0);\n");
  fprintf(fp, "}\n\n");

  fprintf(fp, "int main(int argc, char **argv) {\n");
  fprintf(fp, "  int port = 8080;\n");
  fprintf(fp, "  int server_fd;\n");
  fprintf(fp, "  struct sockaddr_in addr;\n");

  fprintf(fp, "#ifdef USE_LIBPQ\n");
  fprintf(fp, "  init_db();\n");
  fprintf(fp, "#endif\n\n");

  fprintf(fp, "#if defined(_WIN32)\n");
  fprintf(fp, "  {\n");
  fprintf(fp, "    WSADATA wsa;\n");
  fprintf(fp, "    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return 1;\n");
  fprintf(fp, "  }\n");
  fprintf(fp, "#endif\n\n");

  fprintf(fp, "  server_fd = socket(AF_INET, SOCK_STREAM, 0);\n");
  fprintf(fp, "  if (server_fd < 0) return 1;\n");
  fprintf(fp, "  memset(&addr, 0, sizeof(addr));\n");
  fprintf(fp, "  addr.sin_family = AF_INET;\n");
  fprintf(fp, "  addr.sin_addr.s_addr = htonl(INADDR_ANY);\n");
  fprintf(fp, "  addr.sin_port = htons(port);\n\n");

  fprintf(fp, "  if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < "
              "0) return 1;\n");
  fprintf(fp, "  if (listen(server_fd, 5) < 0) return 1;\n");
  fprintf(fp, "  printf(\"Server listening on port %%d\\n\", port);\n\n");

  fprintf(fp, "  while (1) {\n");
  fprintf(fp, "    int client_fd;\n");
  fprintf(fp, "    struct sockaddr_in client_addr;\n");
  fprintf(fp, "#if defined(_WIN32)\n");
  fprintf(fp, "    int addr_len = sizeof(client_addr);\n");
  fprintf(fp, "#else\n");
  fprintf(fp, "    socklen_t addr_len = sizeof(client_addr);\n");
  fprintf(fp, "#endif\n");
  fprintf(fp, "    char buffer[4096] = {0};\n");
  fprintf(fp, "    client_fd = accept(server_fd, (struct sockaddr "
              "*)&client_addr, &addr_len);\n");
  fprintf(fp, "    if (client_fd < 0) continue;\n");
  fprintf(fp, "    recv(client_fd, buffer, sizeof(buffer) - 1, 0);\n");
  fprintf(fp, "    route_request(client_fd, buffer);\n");

  fprintf(fp, "#if defined(_WIN32)\n");
  fprintf(fp, "    closesocket(client_fd);\n");
  fprintf(fp, "#else\n");
  fprintf(fp, "    close(client_fd);\n");
  fprintf(fp, "#endif\n");
  fprintf(fp, "  }\n");
  fprintf(fp, "  return 0;\n");
  fprintf(fp, "}\n");

  fclose(fp);
  return 0;
}
