/**
 * @file test_mock_server.h
 * @brief Tests for mock_server.c
 */

#ifndef TEST_MOCK_SERVER_H
#define TEST_MOCK_SERVER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "c_cdd_export.h"
#include "cdd_test_helpers/mock_server.h"
#include <greatest.h>

/* Platform specifics for a simple HTTP client to hit the server */
#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/socket.h>
#ifndef _WIN32
#include <unistd.h>
#endif
#endif

extern C_CDD_EXPORT int g_socket_fail;
extern C_CDD_EXPORT int g_bind_fail;
extern C_CDD_EXPORT int g_listen_fail;
extern C_CDD_EXPORT int g_getsockname_fail;
extern C_CDD_EXPORT int g_pthread_create_fail;
extern C_CDD_EXPORT int g_accept_fail;

static int http_get(int port);
#ifdef _WIN32
__declspec(dllimport) void __stdcall Sleep(unsigned long dwMilliseconds);
#define USLEEP(x) Sleep((x) / 1000)
#else
#ifndef _WIN32
#include <unistd.h>
#endif
#define USLEEP(x) usleep(x)
#endif
static void *background_http_get(void *arg) {
  int port = *(int *)arg;
  USLEEP(50000);
  http_get(port);
  return NULL;
}

static int http_get(int port) {
#if defined(_WIN32)
  SOCKET sock;
  WSADATA wsa;
  WSAStartup(MAKEWORD(2, 2), &wsa);
#else
  int sock;
#endif
  struct sockaddr_in server_addr;
  const char *msg = "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n";
  char buf[1024];

  sock = socket(AF_INET, SOCK_STREAM, 0);
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

  if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
#if defined(_WIN32)
    closesocket(sock);
#else
    close(sock);
#endif
    return -1;
  }

  send(sock, msg, strlen(msg), 0);
  recv(sock, buf, sizeof(buf) - 1, 0);

#if defined(_WIN32)
  closesocket(sock);
  WSACleanup();
#else
  close(sock);
#endif
  return 0;
}

TEST test_mock_server_basic(void) {
  MockServerPtr server = NULL;
  struct MockServerRequest req;
  int port;

  ASSERT_EQ(CDD_C_SUCCESS, mock_server_init(&server));
  ASSERT_EQ(CDD_C_SUCCESS, mock_server_start(server));
  ASSERT_EQ(CDD_C_SUCCESS, mock_server_get_port(server, &port));
  ASSERT(port > 0);

  ASSERT_EQ(0, http_get(port));

  ASSERT_EQ(CDD_C_SUCCESS, mock_server_wait_for_request(server, &req));
  ASSERT_NEQ(NULL, req.raw_header);

  ASSERT_EQ(CDD_C_SUCCESS, mock_server_request_cleanup(&req));
  ASSERT_EQ(CDD_C_SUCCESS, mock_server_destroy(server));

  /* Test errors */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, mock_server_destroy(NULL));

  /* Test wait fallthrough */
  ASSERT_EQ(CDD_C_SUCCESS, mock_server_init(&server));
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, mock_server_wait_for_request(server, &req));
  ASSERT_EQ(CDD_C_SUCCESS, mock_server_destroy(server));

  PASS();
}

TEST test_mock_server_errors(void) {
  MockServerPtr server = NULL;
  int port = 0;
  struct MockServerRequest req;

  g_cdd_alloc_fail = 1;
  {
    cdd_c_error_t rc = mock_server_init(&server);
    printf("RET: %d\n", rc);
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
  }
  ASSERT_EQ(NULL, server);
  g_cdd_alloc_fail = 0;

  ASSERT_EQ(CDD_C_SUCCESS, mock_server_init(&server));
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, mock_server_start(NULL));

  g_socket_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, mock_server_start(server));
  g_socket_fail = 0;

  g_bind_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, mock_server_start(server));
  g_bind_fail = 0;

  g_listen_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, mock_server_start(server));
  g_listen_fail = 0;

  g_getsockname_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, mock_server_start(server));
  g_getsockname_fail = 0;

  g_pthread_create_fail = 1;
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, mock_server_start(server));
  g_pthread_create_fail = 0;

  /* Test mock_server_get_port edge cases */
  mock_server_get_port(NULL, &port);
  ASSERT_EQ(0, port);

  /* Test mock_server_wait_for_request edge cases */
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, mock_server_wait_for_request(NULL, &req));
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, mock_server_wait_for_request(server, NULL));

  ASSERT_EQ(CDD_C_SUCCESS, mock_server_destroy(server));

  /* Test accept fail */
  ASSERT_EQ(CDD_C_SUCCESS, mock_server_init(&server));
  ASSERT_EQ(CDD_C_SUCCESS, mock_server_start(server));
  g_accept_fail = 1;
  USLEEP(50000);
  ASSERT_EQ(CDD_C_SUCCESS, mock_server_destroy(server));
  g_accept_fail = 0;

  /* Test out == NULL on mock_server_init fail */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, mock_server_init(NULL));

  g_cdd_alloc_fail = 1;
  {
    cdd_c_error_t rc = mock_server_init(&server);
    printf("RET: %d\n", rc);
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
  }
  g_cdd_alloc_fail = 0;

  /* Test free captured_request on destroy */
  ASSERT_EQ(CDD_C_SUCCESS, mock_server_init(&server));
  ASSERT_EQ(CDD_C_SUCCESS, mock_server_start(server));
  mock_server_get_port(server, &port);
  ASSERT_EQ(0, http_get(port));
  USLEEP(150000);
  ASSERT_EQ(CDD_C_SUCCESS, mock_server_destroy(server));

  /* Test second request replacing first */
  ASSERT_EQ(CDD_C_SUCCESS, mock_server_init(&server));
  ASSERT_EQ(CDD_C_SUCCESS, mock_server_start(server));
  mock_server_get_port(server, &port);
  ASSERT_EQ(0, http_get(port));
  USLEEP(150000);
  ASSERT_EQ(0, http_get(port));
  USLEEP(150000);
  ASSERT_EQ(CDD_C_SUCCESS, mock_server_destroy(server));

  /* Test platform_init failure */
  g_accept_fail = 998;
  {
    cdd_c_error_t rc = mock_server_init(&server);
    printf("RET: %d\n", rc);
    ASSERT_EQ(CDD_C_ERROR_UNKNOWN, rc);
  }
  g_accept_fail = 0;

  /* Test cond_wait by hitting it from another thread */
  ASSERT_EQ(CDD_C_SUCCESS, mock_server_init(&server));
  ASSERT_EQ(CDD_C_SUCCESS, mock_server_start(server));
  mock_server_get_port(server, &port);
#if defined(_WIN32)
  {
    HANDLE hThread = CreateThread(
        NULL, 0, (LPTHREAD_START_ROUTINE)background_http_get, &port, 0, NULL);
    ASSERT_EQ(CDD_C_SUCCESS, mock_server_wait_for_request(server, &req));
    ASSERT_NEQ(NULL, req.raw_header);
    ASSERT_EQ(CDD_C_SUCCESS, mock_server_request_cleanup(&req));
    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);
  }
#else
  {
    pthread_t th;
    pthread_create(&th, NULL, background_http_get, &port);
    ASSERT_EQ(CDD_C_SUCCESS, mock_server_wait_for_request(server, &req));
    ASSERT_NEQ(NULL, req.raw_header);
    ASSERT_EQ(CDD_C_SUCCESS, mock_server_request_cleanup(&req));
    pthread_join(th, NULL);
  }
#endif
  ASSERT_EQ(CDD_C_SUCCESS, mock_server_destroy(server));

  /* Test wait fallthrough */
  ASSERT_EQ(CDD_C_SUCCESS, mock_server_init(&server));
  ASSERT_EQ(CDD_C_ERROR_UNKNOWN, mock_server_wait_for_request(server, &req));
  ASSERT_EQ(CDD_C_SUCCESS, mock_server_destroy(server));

  /* Test connect failure (bad port) */
  ASSERT_EQ(-1, http_get(-1));

  PASS();
}

#ifndef _WIN32
#ifndef _WIN32
SUITE(c_cdd_mock_server_suite) {
  RUN_TEST(test_mock_server_basic);
  RUN_TEST(test_mock_server_errors);
}
#else
SUITE(c_cdd_mock_server_suite) {}
#endif
#else
SUITE(c_cdd_mock_server_suite) {}
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !TEST_MOCK_SERVER_H */
