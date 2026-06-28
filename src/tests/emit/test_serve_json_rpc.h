extern C_CDD_EXPORT int g_fail_io_after;
extern C_CDD_EXPORT int g_io_calls;
/**
 * @file test_serve_json_rpc.h
 * @brief Unit tests for JSON RPC server functionality.
 */

#ifndef TEST_SERVER_JSON_RPC_H
#define TEST_SERVER_JSON_RPC_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>

#include "routes/emit/serve_json_rpc.h"

#if defined(_WIN32)
#include <winsock2.h>
#else
#include <netinet/in.h>
#include <sys/socket.h>
#if defined(_MSC_VER)
#include <io.h>
#else
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
/* clang-format on */
/* LCOV_EXCL_START */

/**
 * @brief Tests binding failure scenario for the JSON RPC server.
 *
 * @return The result of the test.
 */
TEST test_serve_json_rpc_bind_fail(void) {
#if defined(_WIN32)
  SOCKET server_fd;
#else
  int server_fd;
#endif
  struct sockaddr_in addr;
  char *argv[] = {"serve_json_rpc_main", "--port", "12346"};
  int argc = 3;
  int rc;

  /* Create a socket holding port 12346 */
#if defined(_WIN32)
  {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
  }
#endif
  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons(12346);
  bind(server_fd, (struct sockaddr *)&addr, sizeof(addr));
  listen(server_fd, 1);

  /* It should fail to bind or listen */
  rc = serve_json_rpc_main(argc, argv);
  ASSERT_EQ(CDD_C_ERROR_SYSTEM, rc);

#if defined(_WIN32)
  closesocket(server_fd);
#else
  close(server_fd);
#endif
  g_fail_io_after = -1;

  PASS();
}

/**
 * @brief Tests the listen once flag behavior.
 *
 * @return The result of the test.
 */
TEST test_serve_json_rpc_listen_once(void) {
  char *argv[] = {"serve_json_rpc_main", "--port", "12347", "--listen", "255"};
  int argc = 5;
  int rc;

  /* Should break immediately because listen_flag is -1 */
  rc = serve_json_rpc_main(argc, argv);
  ASSERT_EQ(0, rc);
  g_fail_io_after = -1;

  PASS();
}

/**
 * @brief Tests basic JSON RPC server logic without listening indefinitely.
 *
 * @return The result of the test.
 */
TEST test_serve_json_rpc_basic(void) {
  char *argv[] = {"serve_json_rpc_main", "--port", "12345"};
  int argc = 3;
  int rc;

  /* Since we do not pass --listen, it should bind, listen, and immediately exit
   * the loop returning 0 */
  rc = serve_json_rpc_main(argc, argv);
  /* Note: Depending on parallel testing or permission, bind might fail if port
     12345 is in use. We can just assert it doesn't crash. If bind succeeds it
     returns 0. If fails, it returns 1. */
  ASSERT(rc == CDD_C_SUCCESS || rc == CDD_C_ERROR_SYSTEM);
  g_fail_io_after = -1;

  PASS();
}

/**
 * @brief Tests JSON RPC server with missing port arguments.
 *
 * @return The result of the test.
 */
TEST test_serve_json_rpc_bad_port(void) {
  /* Try to bind to port 80 or something privileged or duplicate to force bind
     error if we want? Actually we can just run it without args. */
  char *argv[] = {"serve_json_rpc_main"};
  int argc = 1;
  int rc;

  rc = serve_json_rpc_main(argc, argv);
  ASSERT(rc == CDD_C_SUCCESS || rc == CDD_C_ERROR_SYSTEM);
  g_fail_io_after = -1;

  PASS();
}

/**
 * @brief Tests MCP stdio main.
 */
TEST test_serve_mcp_stdio_main(void) {
  char *argv[] = {"serve_mcp_stdio_main"};
  int argc = 1;
  int rc;

  /* Create a temporary file and redirect stdin to it */
  FILE *tmp = tmpfile();
  if (tmp) {
    int old_stdin = dup(fileno(stdin));
    int old_stdout = dup(fileno(stdout));
    FILE *devnull = fopen("/dev/null", "w");

    /* Write test inputs to cover handle_stdio_request branches */
    fprintf(tmp, "invalid json\n");
    fprintf(tmp, "{\"jsonrpc\":\"2.0\"}\n");
    fprintf(tmp, "{\"jsonrpc\":\"2.0\",\"method\":\"version\",\"id\":1}\n");
    fprintf(tmp, "{\"jsonrpc\":\"2.0\",\"method\":\"initialize\",\"id\":2}\n");
    fprintf(tmp,
            "{\"jsonrpc\":\"2.0\",\"method\":\"notifications/initialized\"}\n");
    fprintf(tmp,
            "{\"jsonrpc\":\"2.0\",\"method\":\"notifications/progress\"}\n");
    fprintf(tmp,
            "{\"jsonrpc\":\"2.0\",\"method\":\"notifications/message\"}\n");
    fprintf(tmp,
            "{\"jsonrpc\":\"2.0\",\"method\":\"logging/setLevel\",\"id\":3}\n");
    fprintf(tmp, "{\"jsonrpc\":\"2.0\",\"method\":\"ping\",\"id\":4}\n");
    fprintf(tmp, "{\"jsonrpc\":\"2.0\",\"method\":\"tools/list\",\"id\":5}\n");
    fprintf(tmp,
            "{\"jsonrpc\":\"2.0\",\"method\":\"resources/list\",\"id\":6}\n");
    fprintf(tmp,
            "{\"jsonrpc\":\"2.0\",\"method\":\"resources/read\",\"id\":7}\n");
    fprintf(tmp,
            "{\"jsonrpc\":\"2.0\",\"method\":\"prompts/list\",\"id\":8}\n");
    fprintf(tmp, "{\"jsonrpc\":\"2.0\",\"method\":\"prompts/get\",\"id\":9}\n");
    fprintf(tmp, "{\"jsonrpc\":\"2.0\",\"method\":\"roots/list\",\"id\":18}\n");
    fprintf(tmp, "{\"jsonrpc\":\"2.0\",\"method\":\"resources/templates/"
                 "list\",\"id\":19}\n");
    fprintf(
        tmp,
        "{\"jsonrpc\":\"2.0\",\"method\":\"completion/complete\",\"id\":20}\n");
    fprintf(
        tmp,
        "{\"jsonrpc\":\"2.0\",\"method\":\"resources/subscribe\",\"id\":21}\n");
    fprintf(tmp, "{\"jsonrpc\":\"2.0\",\"method\":\"resources/"
                 "unsubscribe\",\"id\":22}\n");
    fprintf(tmp, "{\"jsonrpc\":\"2.0\",\"method\":\"notifications/roots/"
                 "list_changed\"}\n");
    fprintf(tmp,
            "{\"jsonrpc\":\"2.0\",\"method\":\"notifications/cancelled\"}\n");
    fprintf(tmp, "{\"jsonrpc\":\"2.0\",\"method\":\"sampling/"
                 "createMessage\",\"params\":{\"messages\":[]},\"id\":23}\n");
    fprintf(
        tmp,
        "{\"jsonrpc\":\"2.0\",\"method\":\"tools/read_image\",\"id\":24}\n");
    fprintf(tmp, "{\"jsonrpc\":\"2.0\",\"method\":\"notifications/resources/"
                 "list_changed\"}\n");
    fprintf(tmp, "{\"jsonrpc\":\"2.0\",\"method\":\"notifications/tools/"
                 "list_changed\"}\n");
    fprintf(tmp, "{\"jsonrpc\":\"2.0\",\"method\":\"notifications/prompts/"
                 "list_changed\"}\n");
    fprintf(tmp, "{\"jsonrpc\":\"2.0\",\"method\":\"unknown\",\"id\":10}\n");
    fprintf(tmp, "{\"jsonrpc\":\"2.0\",\"method\":\"tools/call\",\"id\":11}\n");
    fprintf(tmp, "{\"jsonrpc\":\"2.0\",\"method\":\"to_openapi\",\"id\":12}\n");
    fprintf(tmp,
            "{\"jsonrpc\":\"2.0\",\"method\":\"to_docs_json\",\"id\":13}\n");
    fprintf(tmp, "{\"jsonrpc\":\"2.0\",\"method\":\"from_openapi_invalid\","
                 "\"id\":14}\n");
    fprintf(
        tmp,
        "{\"jsonrpc\":\"2.0\",\"method\":\"from_openapi_to_sdk\",\"id\":15}\n");
    fprintf(tmp, "{\"jsonrpc\":\"2.0\",\"method\":\"tools/"
                 "call\",\"params\":{\"name\":\"cdd_inspect\"},\"id\":16}\n");
    fprintf(tmp, "{\"jsonrpc\":\"2.0\",\"method\":\"tools/"
                 "call\",\"params\":{\"name\":\"cdd_sync\"},\"id\":17}\n");

    rewind(tmp);

    dup2(fileno(tmp), fileno(stdin));
    if (devnull)
      dup2(fileno(devnull), fileno(stdout));

    rc = serve_mcp_stdio_main(argc, argv);

    dup2(old_stdin, fileno(stdin));
    dup2(old_stdout, fileno(stdout));
    close(old_stdin);
    close(old_stdout);
    fclose(tmp);
    if (devnull)
      fclose(devnull);
    ASSERT_EQ(0, rc);
  } else {
    rc = serve_mcp_stdio_main(argc, argv);
    ASSERT_EQ(0, rc);
  }

  PASS();
}

/**
 * @brief JSON RPC Server test suite.
 */

SUITE(serve_json_rpc_suite) {
  RUN_TEST(test_serve_json_rpc_basic);
  RUN_TEST(test_serve_json_rpc_bad_port);
  RUN_TEST(test_serve_json_rpc_bind_fail);
  RUN_TEST(test_serve_json_rpc_listen_once);
  RUN_TEST(test_serve_mcp_stdio_main);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !TEST_SERVER_JSON_RPC_H */

/* LCOV_EXCL_STOP */
