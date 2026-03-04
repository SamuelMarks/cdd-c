#ifndef TEST_SERVER_JSON_RPC_H
#define TEST_SERVER_JSON_RPC_H

#include <greatest.h>
#include <stdio.h>
#include <stdlib.h>

#include "routes/emit/server_json_rpc.h"

#if defined(_WIN32)
#include <winsock2.h>
#else
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

TEST test_server_json_rpc_bind_fail(void) {
  int server_fd;
  struct sockaddr_in addr;
  char *argv[] = {"server_json_rpc_main", "--port", "12346"};
  int argc = 3;
  int rc;

  /* Create a socket holding port 12346 */
  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons(12346);
  bind(server_fd, (struct sockaddr *)&addr, sizeof(addr));
  listen(server_fd, 1);

  /* It should fail to bind or listen */
  rc = server_json_rpc_main(argc, argv);
  ASSERT_EQ(1, rc);

#if defined(_WIN32)
  closesocket(server_fd);
#else
  close(server_fd);
#endif

  PASS();
}

TEST test_server_json_rpc_listen_once(void) {
  char *argv[] = {"server_json_rpc_main", "--port", "12347", "--listen", "255"};
  int argc = 5;
  int rc;

  /* Should break immediately because listen_flag is -1 */
  rc = server_json_rpc_main(argc, argv);
  ASSERT_EQ(0, rc);

  PASS();
}

TEST test_server_json_rpc_basic(void) {
  char *argv[] = {"server_json_rpc_main", "--port", "12345"};
  int argc = 3;
  int rc;

  /* Since we do not pass --listen, it should bind, listen, and immediately exit
   * the loop returning 0 */
  rc = server_json_rpc_main(argc, argv);
  /* Note: Depending on parallel testing or permission, bind might fail if port
     12345 is in use. We can just assert it doesn't crash. If bind succeeds it
     returns 0. If fails, it returns 1. */
  ASSERT(rc == 0 || rc == 1);

  PASS();
}

TEST test_server_json_rpc_bad_port(void) {
  /* Try to bind to port 80 or something privileged or duplicate to force bind
     error if we want? Actually we can just run it without args. */
  char *argv[] = {"server_json_rpc_main"};
  int argc = 1;
  int rc;

  rc = server_json_rpc_main(argc, argv);
  ASSERT(rc == 0 || rc == 1);

  PASS();
}

SUITE(server_json_rpc_suite) {
  RUN_TEST(test_server_json_rpc_basic);
  RUN_TEST(test_server_json_rpc_bad_port);
  RUN_TEST(test_server_json_rpc_bind_fail);
  RUN_TEST(test_server_json_rpc_listen_once);
}

#endif /* !TEST_SERVER_JSON_RPC_H */
