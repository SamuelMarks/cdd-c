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
/* clang-format on */

/* extern C_CDD_EXPORT int g_socket_fail; (moved to global) */
/* extern C_CDD_EXPORT int g_bind_fail; (moved to global) */
/* extern C_CDD_EXPORT int g_listen_fail; (moved to global) */
/* extern C_CDD_EXPORT int g_getsockname_fail; (moved to global) */
/* extern C_CDD_EXPORT int g_pthread_create_fail; (moved to global) */
/* extern C_CDD_EXPORT int g_accept_fail; (moved to global) */

/* Removed unmatched #if 0 */
/* http_get */

#if 0
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
#endif

#if 0

#endif

#ifndef _WIN32
#ifndef _WIN32
SUITE(c_cdd_mock_server_suite) {
  /* RUN_TEST(test_mock_server_basic); */
  /*  */
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
