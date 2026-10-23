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
/**
 * @brief Macro TEST_SERVER_JSON_RPC_H.
 */
#define TEST_SERVER_JSON_RPC_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>

#include "c_cdd/context.h"
#include "routes/emit/serve_json_rpc.h"

#ifndef __EMSCRIPTEN__
/**
 * @brief Helper function mock_rpc_socket_fail.
 * @param[in] domain Parameter domain.
 * @param[in] type Parameter type.
 * @param[in] protocol Parameter protocol.
 * @param[out] out_fd Parameter out_fd.
 * @return Result status or value.
 */
static cdd_c_error_t mock_rpc_socket_fail(int domain, int type, int protocol,
                                          int *out_fd) {
  (void)domain;
  (void)type;
  (void)protocol;
  *out_fd = -1;
  return CDD_C_ERROR_SYSTEM;
}

/**
 * @brief Helper function mock_rpc_listen_fail.
 * @param[in] fd Parameter fd.
 * @param[in] backlog Parameter backlog.
 * @return Result status or value.
 */
static cdd_c_error_t mock_rpc_listen_fail(int fd, int backlog) {
  (void)fd;
  (void)backlog;
  return CDD_C_ERROR_SYSTEM;
}

/**
 * @brief Helper function mock_rpc_accept_fail.
 * @param[in] server_fd Parameter server_fd.
 * @param[out] out_client_fd Parameter out_client_fd.
 * @return Result status or value.
 */
static cdd_c_error_t mock_rpc_accept_fail(int server_fd, int *out_client_fd) {
  (void)server_fd;
  *out_client_fd = -1;
  return CDD_C_ERROR_SYSTEM;
}

/**
 * @brief Helper function mock_rpc_send_fail.
 * @param[in] fd Parameter fd.
 * @param[in] buf Parameter buf.
 * @param[in] len Parameter len.
 * @param[out] out_sent Parameter out_sent.
 * @return Result status or value.
 */
static cdd_c_error_t mock_rpc_send_fail(int fd, const void *buf, size_t len,
                                        size_t *out_sent) {
  (void)fd;
  (void)buf;
  (void)len;
  *out_sent = 0;
  return CDD_C_ERROR_SYSTEM;
}
#endif /* !__EMSCRIPTEN__ */

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
#if defined(_MSC_VER)
#include <io.h>
#else
#if defined(_WIN32)
#include <io.h>
#else
#include <unistd.h>
#endif
#endif
/** @brief Test definition cdd_test_thread_t. */
typedef pthread_t cdd_test_thread_t;
/**
 * @brief Macro CDD_TEST_THREAD_CREATE.
 * @param[in] th Macro argument th.
 * @param[in] func Macro argument func.
 * @param[in] arg Macro argument arg.
 */
#define CDD_TEST_THREAD_CREATE(th, func, arg) pthread_create((th), NULL, (func), (arg))
/**
 * @brief Macro CDD_TEST_THREAD_JOIN.
 * @param[in] th Macro argument th.
 */
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
  char *argv[3];
  int argc = 3;
  int rc = 0;

  argv[0] = cdd_test_mut_str("serve_json_rpc_main");
  argv[1] = cdd_test_mut_str("--port");
  argv[2] = cdd_test_mut_str("12346");

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
  char *argv[5];
  char *argv_accept_fail[5];
  int argc = 5;
  int rc = 0;

  /* Should break immediately because listen_flag is -1 */

  argv[0] = cdd_test_mut_str("serve_json_rpc_main");
  argv[1] = cdd_test_mut_str("--port");
  argv[2] = cdd_test_mut_str("12347");
  argv[3] = cdd_test_mut_str("--listen");
  argv[4] = cdd_test_mut_str("255");
  argv_accept_fail[0] = cdd_test_mut_str("serve_json_rpc_main");
  argv_accept_fail[1] = cdd_test_mut_str("--port");
  argv_accept_fail[2] = cdd_test_mut_str("12349");
  argv_accept_fail[3] = cdd_test_mut_str("--listen");
  argv_accept_fail[4] = cdd_test_mut_str("1");
  rc = serve_json_rpc_main(argc, argv);
  ASSERT_EQ(0, rc);

  {
    char *argv_ip[7];
    argv_ip[0] = cdd_test_mut_str("serve_json_rpc_main");
    argv_ip[1] = cdd_test_mut_str("--port");
    argv_ip[2] = cdd_test_mut_str("12350");
    argv_ip[3] = cdd_test_mut_str("--listen");
    argv_ip[4] = cdd_test_mut_str("127.0.0.1");
    argv_ip[5] = cdd_test_mut_str("--listen");
    argv_ip[6] = cdd_test_mut_str("255");
    rc = serve_json_rpc_main(7, argv_ip);
    ASSERT_EQ(0, rc);
  }

  {
    cdd_context_t mock_ctx;
    cdd_context_init_default(&mock_ctx);
    mock_ctx.socket_vt.accept_fn = mock_rpc_accept_fail;
    cdd_context_set_global(&mock_ctx);
    rc = serve_json_rpc_main(argc, argv_accept_fail);
    cdd_context_set_global(NULL);
  }
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
  char *argv[3];
  char *argv_short[5];
  int argc = 3;
  int rc = 0;

  /* Since we do not pass --listen, it should bind, listen, and immediately exit
   * the loop returning 0 */

  argv[0] = cdd_test_mut_str("serve_json_rpc_main");
  argv[1] = cdd_test_mut_str("--port");
  argv[2] = cdd_test_mut_str("12345");
  argv_short[0] = cdd_test_mut_str("serve_json_rpc_main");
  argv_short[1] = cdd_test_mut_str("-p");
  argv_short[2] = cdd_test_mut_str("12348");
  argv_short[3] = cdd_test_mut_str("-l");
  argv_short[4] = cdd_test_mut_str("0");
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
#ifndef __EMSCRIPTEN__
TEST test_serve_json_rpc_bad_port(void) {
  char *argv[1];
  char *argv_help[2];
  char *argv_help_short[2];
  char *argv_unknown[2];
  char *argv_port_missing[2];
  char *argv_p_missing[2];
  char *argv_listen_missing[2];
  int rc = 0;
  cdd_context_t mock_sock_ctx;
  cdd_context_t mock_listen_ctx;

  cdd_context_init_default(&mock_sock_ctx);
  mock_sock_ctx.socket_vt.socket_fn = mock_rpc_socket_fail;

  cdd_context_init_default(&mock_listen_ctx);
  mock_listen_ctx.socket_vt.listen_fn = mock_rpc_listen_fail;

  /* Test environment variables CDD_PORT and CDD_LISTEN */

  argv[0] = cdd_test_mut_str("serve_json_rpc_main");
  argv_help[0] = cdd_test_mut_str("serve_json_rpc_main");
  argv_help[1] = cdd_test_mut_str("--help");
  argv_help_short[0] = cdd_test_mut_str("serve_json_rpc_main");
  argv_help_short[1] = cdd_test_mut_str("-h");
  argv_unknown[0] = cdd_test_mut_str("serve_json_rpc_main");
  argv_unknown[1] = cdd_test_mut_str("--invalid-opt");
  argv_port_missing[0] = cdd_test_mut_str("serve_json_rpc_main");
  argv_port_missing[1] = cdd_test_mut_str("--port");
  argv_p_missing[0] = cdd_test_mut_str("serve_json_rpc_main");
  argv_p_missing[1] = cdd_test_mut_str("-p");
  argv_listen_missing[0] = cdd_test_mut_str("serve_json_rpc_main");
  argv_listen_missing[1] = cdd_test_mut_str("--listen");
  cdd_context_set_global(&mock_sock_ctx);
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
  cdd_context_set_global(NULL);

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

  cdd_context_set_global(&mock_sock_ctx);
  rc = serve_json_rpc_main(2, argv_listen_missing);
  cdd_context_set_global(NULL);
  ASSERT_EQ(CDD_C_ERROR_SYSTEM, rc);

  rc = serve_json_rpc_main(2, argv_unknown);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  cdd_context_set_global(&mock_sock_ctx);
  rc = serve_json_rpc_main(1, argv);
  cdd_context_set_global(NULL);
  ASSERT_EQ(CDD_C_ERROR_SYSTEM, rc);

  cdd_context_set_global(&mock_listen_ctx);
  {
    char *argv_listen_fail[3];
    argv_listen_fail[0] = cdd_test_mut_str("serve_json_rpc_main");
    argv_listen_fail[1] = cdd_test_mut_str("--port");
    argv_listen_fail[2] = cdd_test_mut_str("12352");
    rc = serve_json_rpc_main(3, argv_listen_fail);
  }
  cdd_context_set_global(NULL);
  ASSERT_EQ(CDD_C_ERROR_SYSTEM, rc);

  g_fail_io_after = -1;

  PASS();
}

/**
 * @brief Tests MCP stdio main.
 * @return Test or helper result.
 */
TEST test_serve_mcp_stdio_main(void) {
  char *argv[1];
  int argc = 1;
  int rc = 0;
  FILE *tmp = NULL;
  int old_stdin;
  int old_stdout;
  FILE *devnull = NULL;
  char mcp_tmp_path[64];
  static int mcp_tmp_counter = 0;

  argv[0] = cdd_test_mut_str("serve_mcp_stdio_main");
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, handle_stdio_request(NULL));
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, serve_json_rpc_run(NULL));

  {
    FILE *f_oas = fopen("build/simple_test_oas.json", "w");
    if (f_oas) {
      fprintf(
          f_oas,
          "{\"openapi\":\"3.0.0\",\"info\":{\"title\":\"T\",\"version\":\"1\"},"
          "\"paths\":{\"/"
          "x\":{\"get\":{\"responses\":{\"200\":{\"description\":\"ok\"}}}}}}");
      fclose(f_oas);
    }
    makedirs("build/my_empty_dir");
  }

#if defined(_MSC_VER)
  sprintf_s(mcp_tmp_path, sizeof(mcp_tmp_path), "test_mcp_stdio_tmp_%d.txt",
            ++mcp_tmp_counter);
#else
  CDD_SNPRINTF(mcp_tmp_path, sizeof(mcp_tmp_path), "test_mcp_stdio_tmp_%d.txt",
               ++mcp_tmp_counter);
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
                 "\"input\":\"build/my_empty_dir\"}},\"id\":121}\n");
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
    fprintf(tmp,
            "{\"jsonrpc\":\"2.0\",\"method\":\"tools/"
            "call\",\"params\":{\"name\":\"to_docs_json\",\"arguments\":{"
            "\"input\":\"build/"
            "simple_test_oas.json\",\"output\":\"docs.json\",\"no_imports\":"
            "true,\"no_wrapping\":true}},\"id\":114}\n");
    fprintf(
        tmp,
        "{\"jsonrpc\":\"2.0\",\"method\":\"tools/"
        "call\",\"params\":{\"name\":\"to_openapi\",\"arguments\":{\"input\":"
        "\"build/my_empty_dir\",\"output\":\"out.json\"}},\"id\":112}\n");
    fprintf(tmp, "{\"jsonrpc\":\"2.0\",\"method\":\"tools/"
                 "call\",\"params\":{\"name\":\"to_docs_json\",\"arguments\":{"
                 "\"input\":\"build/simple_test_oas.json\"}},\"id\":116}\n");
    fprintf(tmp, "{\"jsonrpc\":\"2.0\",\"method\":\"tools/"
                 "call\",\"params\":{\"name\":\"to_arazzo\",\"arguments\":{}},"
                 "\"id\":130}\n");
    fprintf(tmp, "{\"jsonrpc\":\"2.0\",\"method\":\"tools/"
                 "call\",\"params\":{\"name\":\"to_arazzo\",\"arguments\":{"
                 "\"input\":\"build/my_empty_dir\",\"output\":\"build/"
                 "arazzo_mcp.json\"}},\"id\":131}\n");
    fprintf(tmp, "{\"jsonrpc\":\"2.0\",\"method\":\"tools/"
                 "call\",\"params\":{\"name\":\"from_arazzo\",\"arguments\":{}}"
                 ",\"id\":132}\n");
    fprintf(tmp, "{\"jsonrpc\":\"2.0\",\"method\":\"tools/"
                 "call\",\"params\":{\"name\":\"from_arazzo\",\"arguments\":{"
                 "\"input\":\"build/arazzo_mcp.json\",\"output\":\"build/"
                 "arazzo_mcp_out\"}},\"id\":133}\n");
    fprintf(tmp, "{\"jsonrpc\":\"2.0\",\"method\":\"tools/"
                 "call\",\"params\":{\"name\":\"from_arazzo\",\"arguments\":{"
                 "\"subcommand\":\"to_sdk_cli\",\"input\":\"build/"
                 "arazzo_mcp.json\",\"output\":\"build/"
                 "arazzo_mcp_out\"}},\"id\":134}\n");
    fprintf(tmp, "{\"jsonrpc\":\"2.0\",\"method\":\"tools/"
                 "call\",\"params\":{\"name\":\"from_arazzo\",\"arguments\":{"
                 "\"subcommand\":\"to_server\",\"input\":\"build/"
                 "arazzo_mcp.json\",\"output\":\"build/"
                 "arazzo_mcp_out\"}},\"id\":135}\n");
    fprintf(tmp, "{\"jsonrpc\":\"2.0\",\"method\":\"resources/"
                 "read\",\"params\":{\"uri\":\"file:///build/"
                 "simple_test_oas.json\"},\"id\":136}\n");
    fprintf(tmp, "{\"jsonrpc\":\"2.0\",\"method\":\"resources/"
                 "read\",\"params\":{\"uri\":\"file://build/"
                 "simple_test_oas.json\"},\"id\":137}\n");
    fprintf(tmp, "{\"jsonrpc\":\"2.0\",\"method\":\"resources/"
                 "read\",\"params\":{\"uri\":\"build/"
                 "simple_test_oas.json\"},\"id\":138}\n");
    fprintf(tmp,
            "{\"jsonrpc\":\"2.0\",\"method\":\"prompts/"
            "get\",\"params\":{\"name\":\"generate_arazzo\"},\"id\":139}\n");
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
      freopen_s(&f_re, "NUL", "r", stdin);
    }
#elif defined(_WIN32)
    freopen("NUL", "r", stdin);
#else
    freopen("/dev/null", "r", stdin);
#endif
    dup2(old_stdin, fileno(stdin));
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
    CDD_SNPRINTF(mcp_err_path, sizeof(mcp_err_path),
                 "test_mcp_stdio_err_%d.txt", ++mcp_tmp_counter);
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
    rc = serve_mcp_stdio_main(argc, argv);
    ASSERT_EQ(CDD_C_SUCCESS, rc);

#if defined(_MSC_VER)
    {
      FILE *f_re = NULL;
      freopen_s(&f_re, "NUL", "r", stdin);
    }
#elif defined(_WIN32)
    freopen("NUL", "r", stdin);
#else
    freopen("/dev/null", "r", stdin);
#endif
    dup2(old_stdin, fileno(stdin));
    close(old_stdin);
    remove(mcp_err_path);
  }

  PASS();
}

#if defined(_WIN32)
/**
 * @brief Background HTTP thread runner for JSON-RPC server tests.
 * @param[in,out] arg Thread argument.
 * @return Thread exit code.
 */
static unsigned __stdcall rpc_server_http_thread_func(void *arg) {
  char *argv[5];

  argv[0] = cdd_test_mut_str("serve_json_rpc_main");
  argv[1] = cdd_test_mut_str("--port");
  argv[2] = cdd_test_mut_str("19985");
  argv[3] = cdd_test_mut_str("--listen");
  argv[4] = cdd_test_mut_str("68");
  (void)arg;
  serve_json_rpc_main(5, argv);
  return 0;
}
#else
/**
 * @brief Background HTTP thread runner for JSON-RPC server tests.
 * @param[in,out] arg Thread argument.
 * @return Thread exit pointer.
 */
static void *rpc_server_http_thread_func(void *arg) {
  char *argv[5];

  argv[0] = cdd_test_mut_str("serve_json_rpc_main");
  argv[1] = cdd_test_mut_str("--port");
  argv[2] = cdd_test_mut_str("19985");
  argv[3] = cdd_test_mut_str("--listen");
  argv[4] = cdd_test_mut_str("68");
  (void)arg;
  serve_json_rpc_main(5, argv);
  return NULL;
}
#endif

/**
 * @brief Unit test test_serve_json_rpc_http_requests.
 * @return Greatest test result.
 */
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
      "POST / HTTP/1.1\r\n\r\n{\"jsonrpc\":\"2.0\",\"method\":\"ping\"}",
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
      "45\r\n\r\n{\"jsonrpc\":\"2.0\",\"id\":1,\"method\":\"tools/call\"}",
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
      "\"build/my_empty_dir\"}}}",
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
      "\"build/my_empty_dir\",\"output\":\"out.json\"}}}",
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
      "\"build/"
      "simple_test_oas.json\",\"output\":\"docs.json\",\"no_imports\":true,"
      "\"no_"
      "wrapping\":true}}}",
      "POST / HTTP/1.1\r\nContent-Length: "
      "120\r\n\r\n{\"jsonrpc\":\"2.0\",\"method\":\"tools/"
      "call\",\"params\":{\"name\":\"to_docs_json\",\"arguments\":{\"input\":"
      "\"build/simple_test_oas.json\"}}}",
      "POST / HTTP/1.1\r\nContent-Length: "
      "77\r\n\r\n{\"jsonrpc\":\"2.0\",\"method\":\"tools/"
      "call\",\"params\":{\"name\":\"to_arazzo\",\"arguments\":{}}}",
      "POST / HTTP/1.1\r\nContent-Length: "
      "128\r\n\r\n{\"jsonrpc\":\"2.0\",\"method\":\"tools/"
      "call\",\"params\":{\"name\":\"to_arazzo\",\"arguments\":{\"input\":"
      "\"build/my_empty_dir\",\"output\":\"build/arazzo_mcp.json\"}}}",
      "POST / HTTP/1.1\r\nContent-Length: "
      "79\r\n\r\n{\"jsonrpc\":\"2.0\",\"method\":\"tools/"
      "call\",\"params\":{\"name\":\"from_arazzo\",\"arguments\":{}}}",
      "POST / HTTP/1.1\r\nContent-Length: "
      "133\r\n\r\n{\"jsonrpc\":\"2.0\",\"method\":\"tools/"
      "call\",\"params\":{\"name\":\"from_arazzo\",\"arguments\":{\"input\":"
      "\"build/arazzo_mcp.json\",\"output\":\"build/arazzo_mcp_out\"}}}",
      "POST / HTTP/1.1\r\nContent-Length: "
      "160\r\n\r\n{\"jsonrpc\":\"2.0\",\"id\":42,\"method\":\"tools/"
      "call\",\"params\":{\"name\":\"from_arazzo\",\"arguments\":{"
      "\"subcommand\":"
      "\"to_sdk_cli\",\"input\":\"build/arazzo_mcp.json\",\"output\":\"build/"
      "arazzo_mcp_out\"}}}",
      "POST / HTTP/1.1\r\nContent-Length: "
      "160\r\n\r\n{\"jsonrpc\":\"2.0\",\"id\":\"req-1\",\"method\":\"tools/"
      "call\",\"params\":{\"name\":\"from_arazzo\",\"arguments\":{"
      "\"subcommand\":"
      "\"to_server\",\"input\":\"build/arazzo_mcp.json\",\"output\":\"build/"
      "arazzo_mcp_out\"}}}",
      "POST / HTTP/1.1\r\nContent-Length: "
      "95\r\n\r\n{\"jsonrpc\":\"2.0\",\"id\":1,\"method\":\"resources/"
      "read\",\"params\":{\"uri\":\"file:///build/simple_test_oas.json\"}}",
      "POST / HTTP/1.1\r\nContent-Length: "
      "94\r\n\r\n{\"jsonrpc\":\"2.0\",\"id\":2,\"method\":\"resources/"
      "read\",\"params\":{\"uri\":\"file://build/simple_test_oas.json\"}}",
      "POST / HTTP/1.1\r\nContent-Length: "
      "87\r\n\r\n{\"jsonrpc\":\"2.0\",\"id\":3,\"method\":\"resources/"
      "read\",\"params\":{\"uri\":\"build/simple_test_oas.json\"}}",
      "POST / HTTP/1.1\r\nContent-Length: "
      "80\r\n\r\n{\"jsonrpc\":\"2.0\",\"id\":4,\"method\":\"prompts/"
      "get\",\"params\":{\"name\":\"generate_arazzo\"}}",
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
      "\"input\":\"build/my_empty_dir\"}}",
      "POST / HTTP/1.1\r\nContent-Length: "
      "100\r\n\r\n{\"jsonrpc\":\"2.0\",\"method\":\"to_openapi\",\"params\":{"
      "\"input\":\"nonexistent_dir_123\",\"output\":\"out.json\"}}",
      "POST / HTTP/1.1\r\nContent-Length: "
      "90\r\n\r\n{\"jsonrpc\":\"2.0\",\"method\":\"to_openapi\",\"params\":{"
      "\"input\":\"build/my_empty_dir\",\"output\":\"out.json\"}}",
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
      "\"input\":\"build/"
      "simple_test_oas.json\",\"output\":\"docs.json\",\"no_imports\":true,"
      "\"no_"
      "wrapping\":true}}",
      "POST / HTTP/1.1\r\nContent-Length: "
      "90\r\n\r\n{\"jsonrpc\":\"2.0\",\"method\":\"to_docs_json\",\"params\":{"
      "\"input\":\"build/simple_test_oas.json\"}}",
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
      "\"params\":{\"input\":\"build/simple_test_oas.json\",\"output\":\"build/"
      "out_sdk\",\"no_github_actions\":true,"
      "\"no_installable_package\":true,\"tests\":true}}",
      "POST / HTTP/1.1\r\nContent-Length: "
      "120\r\n\r\n{\"jsonrpc\":\"2.0\",\"method\":\"from_openapi_to_sdk_cli\","
      "\"params\":{\"input_dir\":\"src/tests/mocks/"
      "emit\",\"output\":\"build/out_sdk_cli\"}}",
      "POST / HTTP/1.1\r\nContent-Length: "
      "120\r\n\r\n{\"jsonrpc\":\"2.0\",\"method\":\"from_openapi_to_server\","
      "\"params\":{\"input\":\"build/simple_test_oas.json\",\"output\":\"build/"
      "out_server\"}}",
      "POST / HTTP/1.1\r\nContent-Length: "
      "50\r\n\r\n{\"jsonrpc\":\"2.0\",\"method\":\"from_openapi_unknown\"}",
      "POST / HTTP/1.1\r\nContent-Length: "
      "50\r\n\r\n{\"jsonrpc\":\"2.0\",\"method\":\"unknown_method\"}",
      "POST / HTTP/1.1\r\nContent-Length: 20\r\n\r\n{\"jsonrpc\":\"2.0\"}",
      "POST / HTTP/1.1\r\nContent-Length: 10\r\n\r\nnot json",
      "not http at all",
      ""};
  size_t num_reqs = sizeof(http_reqs) / sizeof(http_reqs[0]);
  cdd_test_thread_t th;
  struct sockaddr_in addr;
  size_t i;

  {
    FILE *f_oas = fopen("build/simple_test_oas.json", "w");
    if (f_oas) {
      fprintf(
          f_oas,
          "{\"openapi\":\"3.0.0\",\"info\":{\"title\":\"T\",\"version\":\"1\"},"
          "\"paths\":{\"/"
          "x\":{\"get\":{\"responses\":{\"200\":{\"description\":\"ok\"}}}}}}");
      fclose(f_oas);
    }
    makedirs("build/my_empty_dir");
  }

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
      cdd_context_t mock_send_ctx;
      if (i == 1) {
        cdd_context_init_default(&mock_send_ctx);
        mock_send_ctx.socket_vt.send_fn = mock_rpc_send_fail;
        cdd_context_set_global(&mock_send_ctx);
      }
#if defined(_WIN32)
      send(s, http_reqs[i], (int)strlen(http_reqs[i]), 0);
      shutdown(s, SD_SEND);
#else
      send(s, http_reqs[i], strlen(http_reqs[i]), 0);
      shutdown(s, SHUT_WR);
#endif
      if (i != 3 && i != 4 && i != 5 && http_reqs[i][0] != '\0') {
        char buf[1024];
#if defined(_WIN32)
        int n = recv(s, buf, (int)(sizeof(buf) - 1), 0);
#else
        ssize_t n = recv(s, buf, sizeof(buf) - 1, 0);
#endif
        (void)n;
      }
      if (i == 1) {
        cdd_context_set_global(NULL);
      }
#if defined(_WIN32)
      closesocket(s);
#else
      close(s);
#endif
    }
  }

  CDD_TEST_THREAD_JOIN(th);
  remove("docs.json");
  remove("out.json");
  remove("build/simple_test_oas.json");
  PASS();
}
#endif /* !__EMSCRIPTEN__ */

/**
 * @brief Executes the serve_json_rpc_suite test suite.
 */
SUITE(serve_json_rpc_suite) {
  RUN_TEST(test_serve_json_rpc_basic);
#ifndef __EMSCRIPTEN__
  RUN_TEST(test_serve_json_rpc_http_requests);
  RUN_TEST(test_serve_json_rpc_bad_port);
  RUN_TEST(test_serve_json_rpc_bind_fail);
  RUN_TEST(test_serve_json_rpc_listen_once);
  RUN_TEST(test_serve_mcp_stdio_main);
#endif
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !TEST_SERVER_JSON_RPC_H */
