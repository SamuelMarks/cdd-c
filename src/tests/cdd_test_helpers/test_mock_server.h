/**
 * @file test_mock_server.h
 * @brief Tests for mock_server.c
 */

#ifndef TEST_MOCK_SERVER_H
#define TEST_MOCK_SERVER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "cdd_c_error.h"
#include "cdd_test_helpers/mock_server.h"
#include <greatest.h>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <process.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

#include "c_cdd_export.h"
#include "cdd_test_helpers_export.h"
#include "c_cdd/memory.h"
/* clang-format on */

extern CDD_TEST_HELPERS_EXPORT int g_socket_fail;
extern CDD_TEST_HELPERS_EXPORT int g_bind_fail;
extern CDD_TEST_HELPERS_EXPORT int g_listen_fail;
extern CDD_TEST_HELPERS_EXPORT int g_getsockname_fail;
extern CDD_TEST_HELPERS_EXPORT int g_pthread_create_fail;
extern CDD_TEST_HELPERS_EXPORT int g_accept_fail;

static cdd_c_error_t http_get(int port) {
  int sock;
  struct sockaddr_in server_addr;
  const char *msg;
  char buf[256];

  msg = "GET /test HTTP/1.1\r\nHost: localhost\r\n\r\n";

  if (port == -1) {
    sock = -1;
  } else {
    sock = (int)socket(AF_INET, SOCK_STREAM, 0);
  }
  if (sock < 0) {
    return CDD_C_ERROR_UNKNOWN;
  }

  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons((unsigned short)port);
  server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

  if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
#if defined(_WIN32)
    closesocket(sock);
#else
    close(sock);
#endif
    return CDD_C_ERROR_UNKNOWN;
  }

#if defined(_WIN32)
  send(sock, msg, (int)strlen(msg), 0);
#else
  send(sock, msg, strlen(msg), 0);
#endif

  recv(sock, buf, sizeof(buf) - 1, 0);

#if defined(_WIN32)
  closesocket(sock);
#else
  close(sock);
#endif
  return CDD_C_SUCCESS;
}

static cdd_c_error_t http_connect_only(int port) {
  int sock;
  struct sockaddr_in server_addr;

  if (port == -1) {
    sock = -1;
  } else {
    sock = (int)socket(AF_INET, SOCK_STREAM, 0);
  }
  if (sock < 0) {
    return CDD_C_ERROR_UNKNOWN;
  }

  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons((unsigned short)port);
  server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

  if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
#if defined(_WIN32)
    closesocket(sock);
#else
    close(sock);
#endif
    return CDD_C_ERROR_UNKNOWN;
  }

#if defined(_WIN32)
  closesocket(sock);
#else
  close(sock);
#endif
  return CDD_C_SUCCESS;
}

TEST test_mock_server_basic(void) {
  MockServerPtr server = NULL;
  struct MockServerRequest req;
  int port = 0;
  cdd_c_error_t rc;

  rc = mock_server_init(&server);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = mock_server_start(server);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = mock_server_get_port(server, &port);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(port > 0);

  rc = http_get(port);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = mock_server_wait_for_request(server, &req);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_NEQ(NULL, req.raw_header);
  ASSERT(req.header_len > 0);

  rc = mock_server_request_cleanup(&req);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = http_connect_only(port);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

#if defined(_WIN32)
  Sleep(30);
#else
  usleep(30000);
#endif

  rc = mock_server_destroy(server);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  PASS();
}

TEST test_mock_server_init_errors(void) {
  MockServerPtr server = NULL;
  cdd_c_error_t rc;

  rc = mock_server_init(NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  g_accept_fail = 998;
  rc = mock_server_init(&server);
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
  ASSERT_EQ(NULL, server);
  g_accept_fail = 0;

  g_cdd_alloc_fail = 1;
  rc = mock_server_init(&server);
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
  ASSERT_EQ(NULL, server);
  g_cdd_alloc_fail = 0;

  g_cdd_alloc_fail = 2;
  rc = mock_server_init(&server);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = mock_server_destroy(server);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  g_cdd_alloc_fail = 0;

  PASS();
}

TEST test_mock_server_start_errors(void) {
  MockServerPtr server = NULL;
  cdd_c_error_t rc;

  rc = mock_server_start(NULL);
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);

  rc = mock_server_init(&server);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  g_socket_fail = 1;
  rc = mock_server_start(server);
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
  g_socket_fail = 0;

  g_bind_fail = 1;
  rc = mock_server_start(server);
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
  g_bind_fail = 0;

  g_listen_fail = 1;
  rc = mock_server_start(server);
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
  g_listen_fail = 0;

  g_getsockname_fail = 1;
  rc = mock_server_start(server);
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
  g_getsockname_fail = 0;

  g_pthread_create_fail = 1;
  rc = mock_server_start(server);
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
  g_pthread_create_fail = 0;

  rc = mock_server_start(server);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = mock_server_start(server);
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);

  rc = mock_server_destroy(server);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  PASS();
}

TEST test_mock_server_get_port_cases(void) {
  MockServerPtr server = NULL;
  int port = 999;
  cdd_c_error_t rc;

  rc = mock_server_get_port(NULL, &port);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, port);

  rc = mock_server_get_port(NULL, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  rc = mock_server_init(&server);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = mock_server_get_port(server, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  rc = mock_server_destroy(server);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  PASS();
}

TEST test_mock_server_destroy_cases(void) {
  MockServerPtr server = NULL;
  int port = 0;
  cdd_c_error_t rc;

  rc = mock_server_destroy(NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  rc = mock_server_init(&server);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = mock_server_destroy(server);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = mock_server_init(&server);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = mock_server_start(server);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = mock_server_get_port(server, &port);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = http_get(port);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = mock_server_destroy(server);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  PASS();
}

TEST test_mock_server_wait_errors(void) {
  MockServerPtr server = NULL;
  struct MockServerRequest req;
  cdd_c_error_t rc;

  rc = mock_server_wait_for_request(NULL, &req);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  rc = mock_server_init(&server);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = mock_server_wait_for_request(server, NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  rc = mock_server_wait_for_request(server, &req);
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);

  rc = mock_server_start(server);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  g_accept_fail = 999;
  rc = mock_server_wait_for_request(server, &req);
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
  g_accept_fail = 0;

  rc = mock_server_destroy(server);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  PASS();
}

TEST test_mock_server_cleanup_cases(void) {
  struct MockServerRequest req;
  cdd_c_error_t rc;

  rc = mock_server_request_cleanup(NULL);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  req.raw_header = NULL;
  req.header_len = 0;
  rc = mock_server_request_cleanup(&req);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  PASS();
}

TEST test_mock_server_thread_branches(void) {
  MockServerPtr server = NULL;
  struct MockServerRequest req;
  int port = 0;
  cdd_c_error_t rc;

  rc = mock_server_init(&server);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = mock_server_start(server);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = mock_server_get_port(server, &port);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  g_accept_fail = 1;
  rc = http_get(port);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = http_get(port);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = mock_server_wait_for_request(server, &req);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = mock_server_request_cleanup(&req);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  g_cdd_alloc_fail = 1;
  rc = http_get(port);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  g_cdd_alloc_fail = 0;

  g_cdd_alloc_fail = 2;
  rc = http_get(port);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  g_cdd_alloc_fail = 0;

  rc = mock_server_destroy(server);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  PASS();
}

struct ClientThreadArg {
  int port;
  cdd_c_error_t result;
};

#if defined(_WIN32)
static unsigned __stdcall client_thread_func(void *arg) {
  struct ClientThreadArg *cta = (struct ClientThreadArg *)arg;
  Sleep(20);
  cta->result = http_get(cta->port);
  return 0;
}
#else
static void *client_thread_func(void *arg) {
  struct ClientThreadArg *cta = (struct ClientThreadArg *)arg;
  usleep(20000);
  cta->result = http_get(cta->port);
  return NULL;
}
#endif

TEST test_mock_server_concurrent_wait(void) {
  MockServerPtr server = NULL;
  struct MockServerRequest req;
  int port = 0;
  cdd_c_error_t rc;
  struct ClientThreadArg cta;
#if defined(_WIN32)
  HANDLE client_tid;
#else
  pthread_t client_tid;
#endif

  rc = mock_server_init(&server);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = mock_server_start(server);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  rc = mock_server_get_port(server, &port);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  cta.port = port;
  cta.result = CDD_C_ERROR_UNKNOWN;

#if defined(_WIN32)
  client_tid =
      (HANDLE)_beginthreadex(NULL, 0, client_thread_func, &cta, 0, NULL);
  ASSERT_NEQ(0, client_tid);
#else
  ASSERT_EQ(0, pthread_create(&client_tid, NULL, client_thread_func, &cta));
#endif

  rc = mock_server_wait_for_request(server, &req);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

#if defined(_WIN32)
  WaitForSingleObject(client_tid, INFINITE);
  CloseHandle(client_tid);
#else
  pthread_join(client_tid, NULL);
#endif

  ASSERT_EQ(CDD_C_SUCCESS, cta.result);
  rc = mock_server_request_cleanup(&req);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  rc = mock_server_destroy(server);
  ASSERT_EQ(CDD_C_SUCCESS, rc);

  PASS();
}

TEST test_mock_server_client_errors(void) {
  cdd_c_error_t rc;

  rc = http_get(-1);
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);

  rc = http_get(1);
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);

  rc = http_connect_only(-1);
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);

  rc = http_connect_only(1);
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);

  PASS();
}

SUITE(c_cdd_mock_server_suite) {
  RUN_TEST(test_mock_server_basic);
  RUN_TEST(test_mock_server_init_errors);
  RUN_TEST(test_mock_server_start_errors);
  RUN_TEST(test_mock_server_get_port_cases);
  RUN_TEST(test_mock_server_destroy_cases);
  RUN_TEST(test_mock_server_wait_errors);
  RUN_TEST(test_mock_server_cleanup_cases);
  RUN_TEST(test_mock_server_thread_branches);
  RUN_TEST(test_mock_server_concurrent_wait);
  RUN_TEST(test_mock_server_client_errors);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !TEST_MOCK_SERVER_H */
