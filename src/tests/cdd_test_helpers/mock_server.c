/**
 * @file mock_server.c
 * @brief Implementation of the Mock Server using platform-specific
 * threading/sockets.
 *
 * Bridges C89 limitations by using `pthread` on POSIX and `Windows Threads` on
 * Win32. Implements a simple accept-recv-send loop.
 *
 * @author Samuel Marks
 */

#include "mock_server.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* --- Platform Specifics --- */

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <process.h>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

typedef SOCKET socket_t;
typedef HANDLE thread_t;
typedef CRITICAL_SECTION mutex_t;
typedef CONDITION_VARIABLE cond_t;

#define INVALID_SOCK INVALID_SOCKET
#define SOCK_ERROR SOCKET_ERROR
#define THREAD_FUNC_RETURN unsigned __stdcall
#define THREAD_FUNC_ARG void *

static void sleep_ms(int ms) { Sleep((DWORD)ms); }

static void mutex_init(mutex_t *m) { InitializeCriticalSection(m); }
static void mutex_destroy(mutex_t *m) { DeleteCriticalSection(m); }
static void mutex_lock(mutex_t *m) { EnterCriticalSection(m); }
static void mutex_unlock(mutex_t *m) { LeaveCriticalSection(m); }

static void cond_init(cond_t *c) { InitializeConditionVariable(c); }
static void cond_signal(cond_t *c) { WakeConditionVariable(c); }
static int cond_wait(cond_t *c, mutex_t *m) {
  return SleepConditionVariableCS(c, m, INFINITE) ? 0 : 1;
}

static void close_socket(socket_t s) { closesocket(s); }

static int platform_init(void) {
  WSADATA wsa;
  return WSAStartup(MAKEWORD(2, 2), &wsa);
}
static void platform_cleanup(void) { WSACleanup(); }

#else /* POSIX */

#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/socket.h>
#include <unistd.h>

typedef int socket_t;
typedef pthread_t thread_t;
typedef pthread_mutex_t mutex_t;
typedef pthread_cond_t cond_t;

#define INVALID_SOCK (-1)
#define SOCK_ERROR (-1)
#define THREAD_FUNC_RETURN void *
#define THREAD_FUNC_ARG void *

static void sleep_ms(int ms) { usleep(ms * 1000); }

static void mutex_init(mutex_t *m) { pthread_mutex_init(m, NULL); }
static void mutex_destroy(mutex_t *m) { pthread_mutex_destroy(m); }
static void mutex_lock(mutex_t *m) { pthread_mutex_lock(m); }
static void mutex_unlock(mutex_t *m) { pthread_mutex_unlock(m); }

static void cond_init(cond_t *c) { pthread_cond_init(c, NULL); }
static void cond_signal(cond_t *c) { pthread_cond_signal(c); }
static int cond_wait(cond_t *c, mutex_t *m) { return pthread_cond_wait(c, m); }

static void close_socket(socket_t s) { close(s); }

static int platform_init(void) { return 0; }
static void platform_cleanup(void) {}

#endif

/* --- Internal Structure --- */

struct MockServer_ {
  socket_t server_fd;   /**< Listening socket */
  int port;             /**< Bound port */
  volatile int running; /**< Loop flag */

  thread_t thread; /**< Thread handle */

  /* Request capture */
  char *captured_request; /**< Buffer for recieved data */
  size_t captured_len;    /**< Length */

  /* Sync */
  mutex_t lock;
  cond_t cond_req_ready; /* Signaled when a request is parsed */
  int has_request;

  int init_success; /**< Flag for lazy winsock init tracking */
};

/* --- Thread Routine --- */

static THREAD_FUNC_RETURN server_thread_func(THREAD_FUNC_ARG arg) {
  struct MockServer_ *s = (struct MockServer_ *)arg;
  const char *response = "HTTP/1.1 200 OK\r\n"
                         "Content-Type: text/plain\r\n"
                         "Content-Length: 2\r\n"
                         "\r\n"
                         "OK";

  while (s->running) {
    socket_t client_fd;
    struct sockaddr_in client_addr;
#if defined(_WIN32)
    int addr_len = sizeof(client_addr);
#else
    socklen_t addr_len = sizeof(client_addr);
#endif

    /* Blocking Accept */
    client_fd =
        accept(s->server_fd, (struct sockaddr *)&client_addr, &addr_len);

    if (client_fd == INVALID_SOCK) {
      /* If stopped, this failure is expected */
      if (!s->running)
        break;
      /* Temporary error or real error, just retry or sleep */
      sleep_ms(10);
      continue;
    }

    /* Read Request */
    {
      char buffer[4096];
      int bytes_read = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
      if (bytes_read > 0) {
        buffer[bytes_read] = '\0';

        mutex_lock(&s->lock);
        if (s->captured_request)
          free(s->captured_request);
        s->captured_request = (char *)malloc(bytes_read + 1);
        if (s->captured_request) {
          memcpy(s->captured_request, buffer, bytes_read + 1);
          s->captured_len = bytes_read;
          s->has_request = 1;
          cond_signal(&s->cond_req_ready);
        }
        mutex_unlock(&s->lock);
      }
    }

    /* Send Response */
    send(client_fd, response, (int)strlen(response), 0);

    close_socket(client_fd);
  }

  return 0;
}

/* --- Wrapper API --- */

MockServerPtr mock_server_init(void) {
  struct MockServer_ *s;
  if (platform_init() != 0)
    return NULL;

  s = (struct MockServer_ *)calloc(1, sizeof(struct MockServer_));
  if (!s)
    return NULL;

  s->server_fd = INVALID_SOCK;
  s->running = 0;
  s->init_success = 1;

  mutex_init(&s->lock);
  cond_init(&s->cond_req_ready);

  return s;
}

void mock_server_destroy(MockServerPtr server) {
  if (!server)
    return;

  /* Stop thread if running */
  if (server->running) {
    server->running = 0;
    /* Force accept to unblock by shutting down and closing socket */
    if (server->server_fd != INVALID_SOCK) {
#if defined(_WIN32)
      shutdown(server->server_fd, SD_BOTH);
#else
      shutdown(server->server_fd, SHUT_RDWR);
#endif
      close_socket(server->server_fd);
      server->server_fd = INVALID_SOCK;
    }

#if defined(_WIN32)
    WaitForSingleObject(server->thread, INFINITE);
    CloseHandle(server->thread);
#else
    pthread_join(server->thread, NULL);
#endif
  }

  if (server->server_fd != INVALID_SOCK) {
    close_socket(server->server_fd);
  }

  mutex_destroy(&server->lock);
  /* cond_destroy not strictly needed in simple pthread wrapper or windows CV */

  if (server->captured_request)
    free(server->captured_request);

  free(server);
  platform_cleanup();
}

int mock_server_start(MockServerPtr server) {
  struct sockaddr_in addr;
#if defined(_WIN32)
  int addr_len = sizeof(addr);
#else
  socklen_t addr_len = sizeof(addr);
#endif

  if (!server || server->running)
    return -1;

  server->server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server->server_fd == INVALID_SOCK)
    return -1;

  /* Bind to loopback, port 0 (ephemeral) */
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  addr.sin_port = 0;

  if (bind(server->server_fd, (struct sockaddr *)&addr, sizeof(addr)) ==
      SOCK_ERROR) {
    close_socket(server->server_fd);
    return -1;
  }

  /* Listen */
  if (listen(server->server_fd, 1) == SOCK_ERROR) {
    close_socket(server->server_fd);
    return -1;
  }

  /* Retrieve assigned port */
  if (getsockname(server->server_fd, (struct sockaddr *)&addr, &addr_len) ==
      SOCK_ERROR) {
    close_socket(server->server_fd);
    return -1;
  }
  server->port = ntohs(addr.sin_port);

  /* Launch Thread */
  server->running = 1;

#if defined(_WIN32)
  server->thread =
      (HANDLE)_beginthreadex(NULL, 0, server_thread_func, server, 0, NULL);
  if (server->thread == 0) {
    server->running = 0;
    close_socket(server->server_fd);
    return -1;
  }
#else
  if (pthread_create(&server->thread, NULL, server_thread_func, server) != 0) {
    server->running = 0;
    close_socket(server->server_fd);
    return -1;
  }
#endif

  return 0;
}

int mock_server_get_port(MockServerPtr server) {
  return server ? server->port : 0;
}

int mock_server_wait_for_request(MockServerPtr server,
                                 struct MockServerRequest *out_req) {
  if (!server || !out_req)
    return -1;

  mutex_lock(&server->lock);
  while (!server->has_request && server->running) {
    cond_wait(&server->cond_req_ready, &server->lock);
  }

  if (server->has_request && server->captured_request) {
    /* Copy data out */
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
    out_req->raw_header = _strdup(server->captured_request);
#else
    out_req->raw_header = strdup(server->captured_request);
#endif
    out_req->header_len = server->captured_len;

    /* Consume it */
    free(server->captured_request);
    server->captured_request = NULL;
    server->has_request = 0;

    mutex_unlock(&server->lock);
    return 0;
  }

  mutex_unlock(&server->lock);
  return -1;
}

void mock_server_request_cleanup(struct MockServerRequest *req) {
  if (req && req->raw_header) {
    free(req->raw_header);
    req->raw_header = NULL;
  }
}
