#include "cdd_test_helpers_export.h"
CDD_TEST_HELPERS_EXPORT FILE *cdd_test_tmpfile_global(void);
#ifdef _MSC_VER
#define dup2 _dup2
#define close _close
#define fileno _fileno
#define dup _dup
#endif
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

extern C_CDD_EXPORT int g_serve_json_rpc_fail_socket;
extern C_CDD_EXPORT int g_serve_json_rpc_fail_listen;
extern C_CDD_EXPORT int g_serve_json_rpc_fail_accept;
extern C_CDD_EXPORT int g_serve_json_rpc_fail_send;
extern C_CDD_EXPORT int g_serve_json_rpc_fail_handle;

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#include <process.h>
typedef HANDLE cdd_test_thread_t;
#define CDD_TEST_THREAD_CREATE(th, func, arg) ((*(th) = (HANDLE)_beginthreadex(NULL, 0, (func), (arg), 0, NULL)) == 0 ? -1 : 0)
#define CDD_TEST_THREAD_JOIN(th) (WaitForSingleObject((th), INFINITE), CloseHandle((th)))
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/socket.h>
#include <unistd.h>
typedef pthread_t cdd_test_thread_t;
#define CDD_TEST_THREAD_CREATE(th, func, arg) pthread_create((th), NULL, (func), (arg))
#define CDD_TEST_THREAD_JOIN(th) pthread_join((th), NULL)
#endif
/* clang-format on */

/**
 * @brief Tests binding failure scenario for the JSON RPC server.
 *
 * @return The result of the test.
 */
#ifndef __EMSCRIPTEN__
TEST test_serve_json_rpc_bind_fail(void) {
#if defined(_WIN32)
  SOCKET server_fd;
#else
  int server_fd;
#endif
  struct sockaddr_in addr;
  char *argv[] = {(char *)(size_t)(size_t) "serve_json_rpc_main",
                  (char *)(size_t)(size_t) "--port",
                  (char *)(size_t)(size_t) "12346"};
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
  ASSERT(rc == CDD_C_ERROR_SYSTEM || rc == CDD_C_ERROR_UNKNOWN);

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
  char *argv[] = {
      (char *)(size_t)(size_t) "serve_json_rpc_main",
      (char *)(size_t)(size_t) "--port", (char *)(size_t)(size_t) "12347",
      (char *)(size_t)(size_t) "--listen", (char *)(size_t)(size_t) "255"};
  char *argv_accept_fail[] = {
      (char *)(size_t)(size_t) "serve_json_rpc_main",
      (char *)(size_t)(size_t) "--port", (char *)(size_t)(size_t) "12349",
      (char *)(size_t)(size_t) "--listen", (char *)(size_t)(size_t) "1"};
  int argc = 5;
  int rc;

  /* Should break immediately because listen_flag is -1 */
  rc = serve_json_rpc_main(argc, argv);
  ASSERT_EQ(0, rc);

  g_serve_json_rpc_fail_accept = 1;
  rc = serve_json_rpc_main(argc, argv_accept_fail);
  g_serve_json_rpc_fail_accept = 0;
  ASSERT_EQ(0, rc);

  g_fail_io_after = -1;

  PASS();
}
#endif

/**
 * @brief Tests basic JSON RPC server logic without listening indefinitely.
 *
 * @return The result of the test.
 */
TEST test_serve_json_rpc_basic(void) {
  char *argv[] = {(char *)(size_t)(size_t) "serve_json_rpc_main",
                  (char *)(size_t)(size_t) "--port",
                  (char *)(size_t)(size_t) "12345"};
  char *argv_short[] = {
      (char *)(size_t)(size_t) "serve_json_rpc_main",
      (char *)(size_t)(size_t) "-p", (char *)(size_t)(size_t) "12348",
      (char *)(size_t)(size_t) "-l", (char *)(size_t)(size_t) "0"};
  int argc = 3;
  int rc;

  /* Since we do not pass --listen, it should bind, listen, and immediately exit
   * the loop returning 0 */
  rc = serve_json_rpc_main(argc, argv);
  /* Note: Depending on parallel testing or permission, bind might fail if port
     12345 is in use. We can just assert it doesn't crash. If bind succeeds it
     returns 0. If fails, it returns 1. */
  ASSERT(rc == CDD_C_SUCCESS || rc == CDD_C_ERROR_SYSTEM ||
         rc == CDD_C_ERROR_UNKNOWN);

  rc = serve_json_rpc_main(5, argv_short);
  ASSERT(rc == CDD_C_SUCCESS || rc == CDD_C_ERROR_SYSTEM ||
         rc == CDD_C_ERROR_UNKNOWN);

  g_fail_io_after = -1;

  PASS();
}

/**
 * @brief Tests JSON RPC server with missing port arguments.
 *
 * @return The result of the test.
 */
TEST test_serve_json_rpc_bad_port(void) {
  char *argv[] = {(char *)(size_t)(size_t) "serve_json_rpc_main"};
  char *argv_help[] = {(char *)(size_t)(size_t) "serve_json_rpc_main",
                       (char *)(size_t)(size_t) "--help"};
  char *argv_help_short[] = {(char *)(size_t)(size_t) "serve_json_rpc_main",
                             (char *)(size_t)(size_t) "-h"};
  char *argv_unknown[] = {(char *)(size_t)(size_t) "serve_json_rpc_main",
                          (char *)(size_t)(size_t) "--invalid-opt"};
  char *argv_port_missing[] = {(char *)(size_t)(size_t) "serve_json_rpc_main",
                               (char *)(size_t)(size_t) "--port"};
  char *argv_p_missing[] = {(char *)(size_t)(size_t) "serve_json_rpc_main",
                            (char *)(size_t)(size_t) "-p"};
  char *argv_listen_missing[] = {(char *)(size_t)(size_t) "serve_json_rpc_main",
                                 (char *)(size_t)(size_t) "--listen"};
  int rc;

  /* Test environment variables CDD_PORT and CDD_LISTEN */
  g_serve_json_rpc_fail_socket = 1;
#if defined(_WIN32)
  _putenv("CDD_PORT=19999");
  _putenv("CDD_LISTEN=1");
#else
  setenv("CDD_PORT", "19999", 1);
  setenv("CDD_LISTEN", "1", 1);
#endif
  rc = serve_json_rpc_main(1, argv);
  ASSERT_EQ(CDD_C_ERROR_SYSTEM, rc);
#if defined(_WIN32)
  _putenv("CDD_PORT=");
  _putenv("CDD_LISTEN=");
#else
  unsetenv("CDD_PORT");
  unsetenv("CDD_LISTEN");
#endif
  g_serve_json_rpc_fail_socket = 0;

  rc = serve_json_rpc_main(1, argv);
  ASSERT(rc == CDD_C_SUCCESS || rc == CDD_C_ERROR_SYSTEM ||
         rc == CDD_C_ERROR_UNKNOWN);

  rc = serve_json_rpc_main(2, argv_help);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = serve_json_rpc_main(2, argv_help_short);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = serve_json_rpc_main(2, argv_port_missing);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  rc = serve_json_rpc_main(2, argv_p_missing);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  g_serve_json_rpc_fail_socket = 1;
  rc = serve_json_rpc_main(2, argv_listen_missing);
  g_serve_json_rpc_fail_socket = 0;
  ASSERT_EQ(CDD_C_ERROR_SYSTEM, rc);

  rc = serve_json_rpc_main(2, argv_unknown);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  g_serve_json_rpc_fail_socket = 1;
  rc = serve_json_rpc_main(1, argv);
  g_serve_json_rpc_fail_socket = 0;
  ASSERT_EQ(CDD_C_ERROR_SYSTEM, rc);

  g_serve_json_rpc_fail_listen = 1;
  rc = serve_json_rpc_main(1, argv);
  g_serve_json_rpc_fail_listen = 0;
  ASSERT_EQ(CDD_C_ERROR_SYSTEM, rc);

  g_fail_io_after = -1;

  PASS();
}

/**
 * @brief Tests MCP stdio main.
 */
#ifndef __EMSCRIPTEN__
TEST test_serve_mcp_stdio_main(void) {
  char *argv[] = {(char *)(size_t)(size_t) "serve_mcp_stdio_main"};
  int argc = 1;
  int rc;
  FILE *tmp = NULL;
  int old_stdin;
  int old_stdout;
  FILE *devnull = NULL;
  char mcp_tmp_path[64];
  static int mcp_tmp_counter = 0;

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, handle_stdio_request(NULL));
  g_serve_json_rpc_fail_handle = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, handle_stdio_request("{\"jsonrpc\":\"2.0\"}"));
  g_serve_json_rpc_fail_handle = 0;

#if defined(_MSC_VER)
  sprintf_s(mcp_tmp_path, sizeof(mcp_tmp_path), "test_mcp_stdio_tmp_%d.txt",
            ++mcp_tmp_counter);
#else
  sprintf(mcp_tmp_path, "test_mcp_stdio_tmp_%d.txt", ++mcp_tmp_counter);
#endif
#if defined(_MSC_VER)
  if (fopen_s(&tmp, mcp_tmp_path, "w") != 0)
    tmp = NULL;
#else
  tmp = fopen(mcp_tmp_path, "w");
#endif
  ASSERT_NEQ(NULL, tmp);
  {
    old_stdin = dup(fileno(stdin));
    old_stdout = dup(fileno(stdout));
#if defined(_MSC_VER)
    if (fopen_s(&devnull, "/dev/null", "w") != 0)
      devnull = NULL;
#else
    devnull = fopen("/dev/null", "w");
#endif
    fprintf(tmp, "invalid json\n");
    fprintf(tmp, "{\"jsonrpc\":\"2.0\"}\n");
    fprintf(tmp, "{\"jsonrpc\":\"2.0\",\"method\":\"version\"}\n");
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
    fprintf(tmp, "{\"jsonrpc\":\"2.0\",\"method\":\"prompts/list\"}\n");
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
    fprintf(tmp, "{\"jsonrpc\":\"2.0\",\"method\":\"unknown\"}\n");
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
    fprintf(tmp, "{\"jsonrpc\":\"2.0\",\"method\":\"tools/call\",\"id\":11}\n");
    fprintf(tmp, "{\"jsonrpc\":\"2.0\",\"method\":\"tools/"
                 "call\",\"params\":{\"name\":\"unknown_tool\",\"arguments\":{}"
                 "}}\n");
    fprintf(tmp, "{\"jsonrpc\":\"2.0\",\"method\":\"tools/"
                 "call\",\"params\":{\"name\":\"unknown_tool\",\"arguments\":{}"
                 "},\"id\":115}\n");
    fprintf(tmp, "{\"jsonrpc\":\"2.0\",\"method\":\"tools/"
                 "call\",\"params\":{\"name\":\"to_openapi\",\"arguments\":{}},"
                 "\"id\":111}\n");
    fprintf(tmp, "{\"jsonrpc\":\"2.0\",\"method\":\"tools/"
                 "call\",\"params\":{\"name\":\"to_openapi\",\"arguments\":{"
                 "\"input\":\"my_empty_dir\"}},\"id\":121}\n");
    fprintf(tmp, "{\"jsonrpc\":\"2.0\",\"method\":\"tools/"
                 "call\",\"params\":{\"name\":\"to_openapi\",\"arguments\":{"
                 "\"input\":\"nonexistent_dir_123\",\"output\":\"out.json\"}},"
                 "\"id\":119}\n");
    fprintf(tmp, "{\"jsonrpc\":\"2.0\",\"method\":\"tools/"
                 "call\",\"params\":{\"name\":\"to_docs_json\",\"arguments\":{}"
                 "},\"id\":113}\n");
    fprintf(tmp, "{\"jsonrpc\":\"2.0\",\"method\":\"tools/"
                 "call\",\"params\":{\"name\":\"to_docs_json\",\"arguments\":{"
                 "\"output\":\"out.json\"}},\"id\":117}\n");
    fprintf(tmp, "{\"jsonrpc\":\"2.0\",\"method\":\"tools/"
                 "call\",\"params\":{\"name\":\"to_docs_json\",\"arguments\":{"
                 "\"input\":\"nonexistent_file_123.json\"}},\"id\":118}\n");
    fprintf(tmp, "{\"jsonrpc\":\"2.0\",\"method\":\"tools/"
                 "call\",\"params\":{\"name\":\"to_docs_json\",\"arguments\":{"
                 "\"input\":\"src/tests/mocks/emit/"
                 "simple.schema.json\",\"output\":\"docs.json\",\"no_imports\":"
                 "true,\"no_wrapping\":true}},\"id\":114}\n");
    fprintf(
        tmp,
        "{\"jsonrpc\":\"2.0\",\"method\":\"tools/"
        "call\",\"params\":{\"name\":\"to_openapi\",\"arguments\":{\"input\":"
        "\"my_empty_dir\",\"output\":\"out.json\"}},\"id\":112}\n");
    fprintf(tmp, "{\"jsonrpc\":\"2.0\",\"method\":\"tools/"
                 "call\",\"params\":{\"name\":\"to_docs_json\",\"arguments\":{"
                 "\"input\":\"src/tests/mocks/emit/"
                 "simple.schema.json\"}},\"id\":116}\n");
    fclose(tmp);
    tmp = NULL;

#if defined(_MSC_VER)
    {
      FILE *f_re = NULL;
      freopen_s(&f_re, mcp_tmp_path, "r", stdin);
    }
#else
    freopen(mcp_tmp_path, "r", stdin);
#endif
    if (devnull)
      dup2(fileno(devnull), fileno(stdout));

    rc = serve_mcp_stdio_main(argc, argv);

#if defined(_MSC_VER)
    {
      FILE *f_re = NULL;
      freopen_s(&f_re, "CONIN$", "r", stdin);
    }
#elif defined(_WIN32)
    freopen("CONIN$", "r", stdin);
#else
    freopen("/dev/tty", "r", stdin);
#endif
    dup2(old_stdout, fileno(stdout));
    close(old_stdin);
    close(old_stdout);
    remove(mcp_tmp_path);
    if (devnull)
      fclose(devnull);
    ASSERT_EQ(0, rc);
  }

  /* Test serve_mcp_stdio_main error exit when handle_stdio_request fails */
  {
    FILE *tmp_err = NULL;
    char mcp_err_path[64];
#if defined(_MSC_VER)
    sprintf_s(mcp_err_path, sizeof(mcp_err_path), "test_mcp_stdio_err_%d.txt",
              ++mcp_tmp_counter);
    if (fopen_s(&tmp_err, mcp_err_path, "w") != 0)
      tmp_err = NULL;
#else
    sprintf(mcp_err_path, "test_mcp_stdio_err_%d.txt", ++mcp_tmp_counter);
    tmp_err = fopen(mcp_err_path, "w");
#endif
    ASSERT_NEQ(NULL, tmp_err);
    fprintf(tmp_err, "{\"jsonrpc\":\"2.0\",\"method\":\"ping\"}\n");
    fclose(tmp_err);

    old_stdin = dup(fileno(stdin));
#if defined(_MSC_VER)
    {
      FILE *f_re = NULL;
      freopen_s(&f_re, mcp_err_path, "r", stdin);
    }
#else
    freopen(mcp_err_path, "r", stdin);
#endif
    g_serve_json_rpc_fail_handle = 1;
    rc = serve_mcp_stdio_main(argc, argv);
    g_serve_json_rpc_fail_handle = 0;
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);

#if defined(_MSC_VER)
    {
      FILE *f_re = NULL;
      freopen_s(&f_re, "CONIN$", "r", stdin);
    }
#elif defined(_WIN32)
    freopen("CONIN$", "r", stdin);
#else
    freopen("/dev/tty", "r", stdin);
#endif
    close(old_stdin);
    remove(mcp_err_path);
  }

  PASS();
}
#endif

/**
 * @brief JSON RPC Server test suite.
 */

#if defined(_WIN32)
static unsigned __stdcall rpc_server_http_thread_func(void *arg) {
  char *argv[] = {
      (char *)(size_t)(size_t) "serve_json_rpc_main",
      (char *)(size_t)(size_t) "--port", (char *)(size_t)(size_t) "19985",
      (char *)(size_t)(size_t) "--listen", (char *)(size_t)(size_t) "58"};
  (void)arg;
  serve_json_rpc_main(5, argv);
  return 0;
}
#else
static void *rpc_server_http_thread_func(void *arg) {
  char *argv[] = {
      (char *)(size_t)(size_t) "serve_json_rpc_main",
      (char *)(size_t)(size_t) "--port", (char *)(size_t)(size_t) "19985",
      (char *)(size_t)(size_t) "--listen", (char *)(size_t)(size_t) "58"};
  (void)arg;
  serve_json_rpc_main(5, argv);
  return NULL;
}
#endif

TEST test_serve_json_rpc_http_requests(void) {
  static const char *const http_reqs[] = {
      "POST / HTTP/1.1\r\nContent-Length: "
      "35\r\n\r\n{\"jsonrpc\":\"2.0\",\"method\":\"version\"}",
      "POST / HTTP/1.1\r\nContent-Length: "
      "100\r\n\r\n{\"jsonrpc\":\"2.0\",\"method\":\"initialize\",\"params\":{"
      "\"clientInfo\":{},\"capabilities\":{}}}",
      "POST / HTTP/1.1\r\nContent-Length: "
      "100\r\n\r\n{\"jsonrpc\":\"2.0\",\"method\":\"initialize\"}",
      "POST / HTTP/1.1\r\nContent-Length: "
      "50\r\n\r\n{\"jsonrpc\":\"2.0\",\"method\":\"notifications/"
      "initialized\"}",
      "POST / HTTP/1.1\r\nContent-Length: "
      "50\r\n\r\n{\"jsonrpc\":\"2.0\",\"method\":\"notifications/progress\"}",
      "POST / HTTP/1.1\r\nContent-Length: "
      "50\r\n\r\n{\"jsonrpc\":\"2.0\",\"method\":\"notifications/message\"}",
      "POST / HTTP/1.1\r\nContent-Length: "
      "50\r\n\r\n{\"jsonrpc\":\"2.0\",\"method\":\"logging/setLevel\"}",
      "POST / HTTP/1.1\r\nContent-Length: "
      "35\r\n\r\n{\"jsonrpc\":\"2.0\",\"method\":\"ping\"}",
      "POST / HTTP/1.1\r\nContent-Length: "
      "35\r\n\r\n{\"jsonrpc\":\"2.0\",\"method\":\"tools/list\"}",
      "POST / HTTP/1.1\r\nContent-Length: "
      "35\r\n\r\n{\"jsonrpc\":\"2.0\",\"method\":\"resources/list\"}",
      "POST / HTTP/1.1\r\nContent-Length: "
      "35\r\n\r\n{\"jsonrpc\":\"2.0\",\"method\":\"roots/list\"}",
      "POST / HTTP/1.1\r\nContent-Length: "
      "40\r\n\r\n{\"jsonrpc\":\"2.0\",\"method\":\"resources/templates/list\"}",
      "POST / HTTP/1.1\r\nContent-Length: "
      "35\r\n\r\n{\"jsonrpc\":\"2.0\",\"method\":\"resources/read\"}",
      "POST / HTTP/1.1\r\nContent-Length: "
      "40\r\n\r\n{\"jsonrpc\":\"2.0\",\"method\":\"resources/subscribe\"}",
      "POST / HTTP/1.1\r\nContent-Length: "
      "40\r\n\r\n{\"jsonrpc\":\"2.0\",\"method\":\"resources/unsubscribe\"}",
      "POST / HTTP/1.1\r\nContent-Length: "
      "40\r\n\r\n{\"jsonrpc\":\"2.0\",\"method\":\"tools/read_image\"}",
      "POST / HTTP/1.1\r\nContent-Length: "
      "35\r\n\r\n{\"jsonrpc\":\"2.0\",\"method\":\"prompts/list\"}",
      "POST / HTTP/1.1\r\nContent-Length: "
      "120\r\n\r\n{\"jsonrpc\":\"2.0\",\"method\":\"sampling/"
      "createMessage\",\"params\":{\"messages\":[],\"maxTokens\":10,"
      "\"includeContext\":\"all\"}}",
      "POST / HTTP/1.1\r\nContent-Length: "
      "45\r\n\r\n{\"jsonrpc\":\"2.0\",\"method\":\"sampling/createMessage\"}",
      "POST / HTTP/1.1\r\nContent-Length: "
      "220\r\n\r\n{\"jsonrpc\":\"2.0\",\"method\":\"sampling/"
      "createMessage\",\"params\":{\"messages\":[],\"maxTokens\":10,"
      "\"includeContext\":\"all\",\"metadata\":{},\"modelPreferences\":{},"
      "\"stopSequences\":[],\"systemPrompt\":\"hi\",\"temperature\":0.7}}",
      "POST / HTTP/1.1\r\nContent-Length: "
      "35\r\n\r\n{\"jsonrpc\":\"2.0\",\"method\":\"prompts/get\"}",
      "POST / HTTP/1.1\r\nContent-Length: "
      "40\r\n\r\n{\"jsonrpc\":\"2.0\",\"method\":\"completion/complete\"}",
      "POST / HTTP/1.1\r\nContent-Length: "
      "35\r\n\r\n{\"jsonrpc\":\"2.0\",\"method\":\"tools/call\"}",
      "POST / HTTP/1.1\r\nContent-Length: "
      "65\r\n\r\n{\"jsonrpc\":\"2.0\",\"method\":\"tools/"
      "call\",\"params\":{\"arguments\":{}}}",
      "POST / HTTP/1.1\r\nContent-Length: "
      "60\r\n\r\n{\"jsonrpc\":\"2.0\",\"method\":\"tools/"
      "call\",\"params\":{\"name\":\"to_openapi\"}}",
      "POST / HTTP/1.1\r\nContent-Length: "
      "80\r\n\r\n{\"jsonrpc\":\"2.0\",\"method\":\"tools/"
      "call\",\"params\":{\"name\":\"to_openapi\",\"arguments\":{}}}",
      "POST / HTTP/1.1\r\nContent-Length: "
      "85\r\n\r\n{\"jsonrpc\":\"2.0\",\"method\":\"tools/"
      "call\",\"params\":{\"name\":\"to_openapi\",\"arguments\":{\"input\":"
      "\"my_empty_dir\"}}}",
      "POST / HTTP/1.1\r\nContent-Length: "
      "90\r\n\r\n{\"jsonrpc\":\"2.0\",\"method\":\"tools/"
      "call\",\"params\":{\"name\":\"to_openapi\",\"arguments\":{\"output\":"
      "\"out\"}}}",
      "POST / HTTP/1.1\r\nContent-Length: "
      "125\r\n\r\n{\"jsonrpc\":\"2.0\",\"method\":\"tools/"
      "call\",\"params\":{\"name\":\"to_openapi\",\"arguments\":{\"input\":"
      "\"nonexistent_dir_123\",\"output\":\"out.json\"}}}",
      "POST / HTTP/1.1\r\nContent-Length: "
      "120\r\n\r\n{\"jsonrpc\":\"2.0\",\"method\":\"tools/"
      "call\",\"params\":{\"name\":\"to_openapi\",\"arguments\":{\"input\":"
      "\"my_empty_dir\",\"output\":\"out.json\"}}}",
      "POST / HTTP/1.1\r\nContent-Length: "
      "80\r\n\r\n{\"jsonrpc\":\"2.0\",\"method\":\"tools/"
      "call\",\"params\":{\"name\":\"to_docs_json\",\"arguments\":{}}}",
      "POST / HTTP/1.1\r\nContent-Length: "
      "90\r\n\r\n{\"jsonrpc\":\"2.0\",\"method\":\"tools/"
      "call\",\"params\":{\"name\":\"to_docs_json\",\"arguments\":{\"output\":"
      "\"out\"}}}",
      "POST / HTTP/1.1\r\nContent-Length: "
      "115\r\n\r\n{\"jsonrpc\":\"2.0\",\"method\":\"tools/"
      "call\",\"params\":{\"name\":\"to_docs_json\",\"arguments\":{\"input\":"
      "\"nonexistent_file_123.json\"}}}",
      "POST / HTTP/1.1\r\nContent-Length: "
      "160\r\n\r\n{\"jsonrpc\":\"2.0\",\"method\":\"tools/"
      "call\",\"params\":{\"name\":\"to_docs_json\",\"arguments\":{\"input\":"
      "\"src/tests/mocks/emit/"
      "simple.schema.json\",\"output\":\"docs.json\",\"no_imports\":true,\"no_"
      "wrapping\":true}}}",
      "POST / HTTP/1.1\r\nContent-Length: "
      "120\r\n\r\n{\"jsonrpc\":\"2.0\",\"method\":\"tools/"
      "call\",\"params\":{\"name\":\"to_docs_json\",\"arguments\":{\"input\":"
      "\"src/tests/mocks/emit/simple.schema.json\"}}}",
      "POST / HTTP/1.1\r\nContent-Length: "
      "80\r\n\r\n{\"jsonrpc\":\"2.0\",\"method\":\"tools/"
      "call\",\"params\":{\"name\":\"unknown\",\"arguments\":{}}}",
      "POST / HTTP/1.1\r\nContent-Length: "
      "35\r\n\r\n{\"jsonrpc\":\"2.0\",\"method\":\"to_openapi\"}",
      "POST / HTTP/1.1\r\nContent-Length: "
      "70\r\n\r\n{\"jsonrpc\":\"2.0\",\"method\":\"to_openapi\",\"params\":{"
      "\"output\":\"out\"}}",
      "POST / HTTP/1.1\r\nContent-Length: "
      "75\r\n\r\n{\"jsonrpc\":\"2.0\",\"method\":\"to_openapi\",\"params\":{"
      "\"input\":\"my_empty_dir\"}}",
      "POST / HTTP/1.1\r\nContent-Length: "
      "100\r\n\r\n{\"jsonrpc\":\"2.0\",\"method\":\"to_openapi\",\"params\":{"
      "\"input\":\"nonexistent_dir_123\",\"output\":\"out.json\"}}",
      "POST / HTTP/1.1\r\nContent-Length: "
      "90\r\n\r\n{\"jsonrpc\":\"2.0\",\"method\":\"to_openapi\",\"params\":{"
      "\"input\":\"my_empty_dir\",\"output\":\"out.json\"}}",
      "POST / HTTP/1.1\r\nContent-Length: "
      "35\r\n\r\n{\"jsonrpc\":\"2.0\",\"method\":\"to_docs_json\"}",
      "POST / HTTP/1.1\r\nContent-Length: "
      "70\r\n\r\n{\"jsonrpc\":\"2.0\",\"method\":\"to_docs_json\",\"params\":{"
      "\"output\":\"out\"}}",
      "POST / HTTP/1.1\r\nContent-Length: "
      "90\r\n\r\n{\"jsonrpc\":\"2.0\",\"method\":\"to_docs_json\",\"params\":{"
      "\"input\":\"nonexistent_file_123.json\"}}",
      "POST / HTTP/1.1\r\nContent-Length: "
      "150\r\n\r\n{\"jsonrpc\":\"2.0\",\"method\":\"to_docs_json\",\"params\":{"
      "\"input\":\"src/tests/mocks/emit/"
      "simple.schema.json\",\"output\":\"docs.json\",\"no_imports\":true,\"no_"
      "wrapping\":true}}",
      "POST / HTTP/1.1\r\nContent-Length: "
      "90\r\n\r\n{\"jsonrpc\":\"2.0\",\"method\":\"to_docs_json\",\"params\":{"
      "\"input\":\"src/tests/mocks/emit/simple.schema.json\"}}",
      "POST / HTTP/1.1\r\nContent-Length: "
      "45\r\n\r\n{\"jsonrpc\":\"2.0\",\"method\":\"from_openapi_to_sdk\"}",
      "POST / HTTP/1.1\r\nContent-Length: "
      "90\r\n\r\n{\"jsonrpc\":\"2.0\",\"method\":\"from_openapi_to_sdk\","
      "\"params\":{\"input\":\"nonexistent.json\"}}",
      "POST / HTTP/1.1\r\nContent-Length: "
      "75\r\n\r\n{\"jsonrpc\":\"2.0\",\"method\":\"from_openapi_to_sdk\","
      "\"params\":{\"output\":\"out\"}}",
      "POST / HTTP/1.1\r\nContent-Length: "
      "180\r\n\r\n{\"jsonrpc\":\"2.0\",\"method\":\"from_openapi_to_sdk\","
      "\"params\":{\"input\":\"src/tests/mocks/emit/"
      "simple.schema.json\",\"output\":\"out_sdk\",\"no_github_actions\":true,"
      "\"no_installable_package\":true,\"tests\":true}}",
      "POST / HTTP/1.1\r\nContent-Length: "
      "120\r\n\r\n{\"jsonrpc\":\"2.0\",\"method\":\"from_openapi_to_sdk_cli\","
      "\"params\":{\"input_dir\":\"src/tests/mocks/"
      "emit\",\"output\":\"out_sdk_cli\"}}",
      "POST / HTTP/1.1\r\nContent-Length: "
      "120\r\n\r\n{\"jsonrpc\":\"2.0\",\"method\":\"from_openapi_to_server\","
      "\"params\":{\"input\":\"src/tests/mocks/emit/"
      "simple.schema.json\",\"output\":\"out_server\"}}",
      "POST / HTTP/1.1\r\nContent-Length: "
      "50\r\n\r\n{\"jsonrpc\":\"2.0\",\"method\":\"from_openapi_unknown\"}",
      "POST / HTTP/1.1\r\nContent-Length: "
      "50\r\n\r\n{\"jsonrpc\":\"2.0\",\"method\":\"unknown_method\"}",
      "POST / HTTP/1.1\r\nContent-Length: 20\r\n\r\n{\"jsonrpc\":\"2.0\"}",
      "POST / HTTP/1.1\r\nContent-Length: 10\r\n\r\nnot json",
      "not http at all"};
  size_t num_reqs = sizeof(http_reqs) / sizeof(http_reqs[0]);
  cdd_test_thread_t th;
  struct sockaddr_in addr;
  size_t i;

  ASSERT_EQ(0, CDD_TEST_THREAD_CREATE(&th, rpc_server_http_thread_func, NULL));
#if defined(_WIN32)
  Sleep(150);
#else
  usleep(150000);
#endif

  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(19985);
  addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

  for (i = 0; i < num_reqs; i++) {
#if defined(_WIN32)
    SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
#else
    int s = socket(AF_INET, SOCK_STREAM, 0);
#endif
    if (connect(s, (struct sockaddr *)&addr, sizeof(addr)) == 0) {
      if (i == 1) {
        g_serve_json_rpc_fail_send = 1;
      }
#if defined(_WIN32)
      send(s, http_reqs[i], (int)strlen(http_reqs[i]), 0);
      shutdown(s, SD_SEND);
#else
      send(s, http_reqs[i], strlen(http_reqs[i]), 0);
      shutdown(s, SHUT_WR);
#endif
      if (i != 3 && i != 4 && i != 5) {
        char buf[1024];
#if defined(_WIN32)
        int n = recv(s, buf, (int)(sizeof(buf) - 1), 0);
#else
        ssize_t n = recv(s, buf, sizeof(buf) - 1, 0);
#endif
        (void)n;
      }
      if (i == 1) {
        g_serve_json_rpc_fail_send = 0;
      }
#if defined(_WIN32)
      closesocket(s);
#else
      close(s);
#endif
    }
  }

  /* Dummy connect to wake up and exit server loop */
  {
#if defined(_WIN32)
    SOCKET s_dummy = socket(AF_INET, SOCK_STREAM, 0);
#else
    int s_dummy = socket(AF_INET, SOCK_STREAM, 0);
#endif
    if (connect(s_dummy, (struct sockaddr *)&addr, sizeof(addr)) == 0) {
#if defined(_WIN32)
      closesocket(s_dummy);
#else
      close(s_dummy);
#endif
    }
  }

  CDD_TEST_THREAD_JOIN(th);
  PASS();
}

SUITE(serve_json_rpc_suite) {
  RUN_TEST(test_serve_json_rpc_http_requests);
  RUN_TEST(test_serve_json_rpc_basic);
  RUN_TEST(test_serve_json_rpc_bad_port);
#ifndef __EMSCRIPTEN__
  RUN_TEST(test_serve_json_rpc_bind_fail);
  RUN_TEST(test_serve_json_rpc_listen_once);
  RUN_TEST(test_serve_mcp_stdio_main);
#endif
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !TEST_SERVER_JSON_RPC_H */
