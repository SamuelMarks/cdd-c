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

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "mock_server.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <c_cddConfig.h>
#include <process.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "../../win_compat_sym.h"
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

#if defined(_WIN32)
typedef int cdd_ssize_t;
typedef SOCKET socket_t;
typedef HANDLE thread_t;
typedef CRITICAL_SECTION mutex_t;
#if defined(_MSC_VER) && _MSC_VER < 1600
typedef HANDLE cond_t;
#else
typedef CONDITION_VARIABLE cond_t;
#endif
#define INVALID_SOCK INVALID_SOCKET
#define SOCK_ERROR SOCKET_ERROR
#define THREAD_FUNC_RETURN unsigned __stdcall
#define THREAD_FUNC_ARG void *
#else
typedef ssize_t cdd_ssize_t;
typedef int socket_t;
typedef pthread_t thread_t;
typedef pthread_mutex_t mutex_t;
typedef pthread_cond_t cond_t;
#define INVALID_SOCK (-1)
#define SOCK_ERROR (-1)
#define THREAD_FUNC_RETURN void *
#define THREAD_FUNC_ARG void *
#endif

CDD_TEST_HELPERS_EXPORT int g_socket_fail = 0;
CDD_TEST_HELPERS_EXPORT int g_bind_fail = 0;
CDD_TEST_HELPERS_EXPORT int g_listen_fail = 0;
CDD_TEST_HELPERS_EXPORT int g_getsockname_fail = 0;
CDD_TEST_HELPERS_EXPORT int g_pthread_create_fail = 0;
CDD_TEST_HELPERS_EXPORT int g_accept_fail = 0;

static socket_t mock_socket(int domain, int type, int protocol) {
  if (g_socket_fail)
    return INVALID_SOCK;
  return socket(domain, type, protocol);
}

static int mock_bind(socket_t sockfd, const struct sockaddr *addr,
                     int addrlen) {
  if (g_bind_fail)
    return SOCK_ERROR;
  return bind(sockfd, addr, (socklen_t)addrlen);
}

static int mock_listen(socket_t sockfd, int backlog) {
  if (g_listen_fail)
    return SOCK_ERROR;
  return listen(sockfd, backlog);
}

static int mock_getsockname(socket_t sockfd, struct sockaddr *addr,
                            void *addrlen) {
  if (g_getsockname_fail)
    return SOCK_ERROR;
#if defined(_WIN32)
  return getsockname(sockfd, addr, (int *)addrlen);
#else
  return getsockname(sockfd, addr, (socklen_t *)addrlen);
#endif
}

#if defined(_WIN32)
static uintptr_t mock_beginthreadex(void *security, unsigned stack_size,
                                    unsigned(__stdcall *start_address)(void *),
                                    void *arglist, unsigned initflag,
                                    unsigned *thrdaddr) {
  if (g_pthread_create_fail)
    return 0;
  return _beginthreadex(security, stack_size, start_address, arglist, initflag,
                        thrdaddr);
}
#define BEGIN_THREAD mock_beginthreadex
#else
static int mock_pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                               void *(*start_routine)(void *), void *arg) {
  if (g_pthread_create_fail)
    return 1;
  return pthread_create(thread, attr, start_routine, arg);
}
#define PTHREAD_CREATE mock_pthread_create
#endif

static socket_t mock_accept(socket_t sockfd, struct sockaddr *addr,
                            void *addrlen) {
  if (g_accept_fail == 1) {
    g_accept_fail = 0;
    return INVALID_SOCK;
  }
#if defined(_WIN32)
  return accept(sockfd, addr, (int *)addrlen);
#else
  return accept(sockfd, addr, (socklen_t *)addrlen);
#endif
}

#undef socket
#define socket mock_socket
#undef bind
#define bind mock_bind
#undef listen
#define listen mock_listen
#undef getsockname
#define getsockname mock_getsockname
#undef accept
#define accept mock_accept

static void *mock_calloc(size_t count, size_t size) {
  if (g_cdd_alloc_fail && --g_cdd_alloc_fail == 0)
    return NULL;
  return calloc(count, size);
}

static void *mock_malloc(size_t size) {
  if (g_cdd_alloc_fail && --g_cdd_alloc_fail == 0)
    return NULL;
  return malloc(size);
}

#undef calloc
#define calloc mock_calloc
#undef malloc
#define malloc mock_malloc

static void sleep_ms(int ms) {
#if defined(_WIN32)
  Sleep((DWORD)ms);
#else
  usleep((useconds_t)ms * 1000);
#endif
}

static void mutex_init(mutex_t *m) {
#if defined(_WIN32)
  InitializeCriticalSection(m);
#else
  pthread_mutex_init(m, NULL);
#endif
}

static void mutex_destroy(mutex_t *m) {
#if defined(_WIN32)
  DeleteCriticalSection(m);
#else
  pthread_mutex_destroy(m);
#endif
}

static void mutex_lock(mutex_t *m) {
#if defined(_WIN32)
  EnterCriticalSection(m);
#else
  pthread_mutex_lock(m);
#endif
}

static void mutex_unlock(mutex_t *m) {
#if defined(_WIN32)
  LeaveCriticalSection(m);
#else
  pthread_mutex_unlock(m);
#endif
}

static void cond_init(cond_t *c) {
#if defined(_WIN32)
#if defined(_MSC_VER) && _MSC_VER < 1600
  *c = CreateEvent(NULL, FALSE, FALSE, NULL);
#else
  InitializeConditionVariable(c);
#endif
#else
  pthread_cond_init(c, NULL);
#endif
}

static void cond_signal(cond_t *c) {
#if defined(_WIN32)
#if defined(_MSC_VER) && _MSC_VER < 1600
  SetEvent(*c);
#else
  WakeConditionVariable(c);
#endif
#else
  pthread_cond_signal(c);
#endif
}

static void close_socket(socket_t s) {
#if defined(_WIN32)
  closesocket(s);
#else
  close(s);
#endif
}

static cdd_c_error_t platform_init(void) {
  if (g_accept_fail == 998)
    return CDD_C_ERROR_UNKNOWN;
#if defined(_WIN32)
  {
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
      return CDD_C_ERROR_UNKNOWN;
  }
#endif
  return CDD_C_SUCCESS;
}

static cdd_c_error_t platform_cleanup(void) {
#if defined(_WIN32)
  WSACleanup();
#endif
  return CDD_C_SUCCESS;
}

/* --- Internal Structure --- */

/**
 * @brief Internal server structure.
 */
struct MockServer_ {
  socket_t server_fd;     /**< Listening socket */
  int port;               /**< Bound port */
  volatile int running;   /**< Loop flag */
  thread_t thread;        /**< Thread handle */
  char *captured_request; /**< Buffer for received data */
  size_t captured_len;    /**< Length */
  mutex_t lock;           /**< Mutex */
  cond_t cond_req_ready;  /**< Condition variable */
  int has_request;        /**< Request flag */
  int init_success;       /**< Init flag */
};

static void cond_wait_internal(cond_t *c, mutex_t *m,
                               struct MockServer_ *server) {
  if (g_accept_fail == 999) {
    server->running = 0;
    return;
  }
#if defined(_WIN32)
#if defined(_MSC_VER) && _MSC_VER < 1600
  LeaveCriticalSection(m);
  WaitForSingleObject(*c, INFINITE);
  EnterCriticalSection(m);
#else
  SleepConditionVariableCS(c, m, INFINITE);
#endif
#else
  pthread_cond_wait(c, m);
#endif
}

/* --- Thread Routine --- */

static THREAD_FUNC_RETURN server_thread_func(THREAD_FUNC_ARG arg) {
  struct MockServer_ *s;
  const char *response;

  s = (struct MockServer_ *)arg;
  response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: "
             "2\r\n\r\nOK";

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
      if (!s->running)
        break;
      sleep_ms(10);
      continue;
    }

    /* Read Request */
    {
      char buffer[4096];
      cdd_ssize_t bytes_read;
      sleep_ms(10);
      bytes_read = (cdd_ssize_t)recv(client_fd, buffer, sizeof(buffer) - 1, 0);
      if (bytes_read > 0) {
        buffer[bytes_read] = '\0';

        mutex_lock(&s->lock);
        if (s->captured_request) {
          free(s->captured_request);
          s->captured_request = NULL;
        }
        s->captured_request = (char *)(size_t)malloc((size_t)bytes_read + 1);
        if (s->captured_request) {
          memcpy(s->captured_request, buffer, (size_t)bytes_read + 1);
          s->captured_len = (size_t)bytes_read;
          s->has_request = 1;
          cond_signal(&s->cond_req_ready);
        }
        mutex_unlock(&s->lock);
      }
    }

    /* Send Response */
#if defined(_WIN32)
    send(client_fd, response, (int)strlen(response), 0);
#else
    send(client_fd, response, strlen(response), 0);
#endif

    close_socket(client_fd);
  }

  return 0;
}

/* --- Wrapper API --- */

cdd_c_error_t mock_server_init(MockServerPtr *out) {
  cdd_c_error_t rc;
  struct MockServer_ *s;

  if (!out)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  rc = platform_init();
  if (rc != CDD_C_SUCCESS) {
    *out = NULL;
    return rc;
  }

  s = (struct MockServer_ *)calloc(1, sizeof(struct MockServer_));
  if (!s) {
    *out = NULL;
    return CDD_C_ERROR_UNKNOWN;
  }

  s->server_fd = INVALID_SOCK;
  s->running = 0;
  s->init_success = 1;

  mutex_init(&s->lock);
  cond_init(&s->cond_req_ready);

  *out = s;
  return CDD_C_SUCCESS;
}

cdd_c_error_t mock_server_destroy(MockServerPtr server) {
  if (!server)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  /* Stop thread if running */
  if (server->running) {
    server->running = 0;
    /* Force accept to unblock by shutting down and closing socket */
#if defined(_WIN32)
    shutdown(server->server_fd, SD_BOTH);
#else
    shutdown(server->server_fd, SHUT_RDWR);
#endif
    close_socket(server->server_fd);
    server->server_fd = INVALID_SOCK;

#if defined(_WIN32)
    WaitForSingleObject(server->thread, INFINITE);
    CloseHandle(server->thread);
#else
    pthread_join(server->thread, NULL);
#endif
  }

  if (server->server_fd != INVALID_SOCK) {
    close_socket(server->server_fd);
    server->server_fd = INVALID_SOCK;
  }

  mutex_destroy(&server->lock);

  if (server->captured_request) {
    free(server->captured_request);
    server->captured_request = NULL;
  }

  free(server);
  return platform_cleanup();
}

cdd_c_error_t mock_server_start(MockServerPtr server) {
  struct sockaddr_in addr;
#if defined(_WIN32)
  int addr_len = sizeof(addr);
#else
  socklen_t addr_len = sizeof(addr);
#endif

  if (!server || server->running)
    return CDD_C_ERROR_UNKNOWN;

  server->server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server->server_fd == INVALID_SOCK)
    return CDD_C_ERROR_UNKNOWN;

  /* Bind to loopback, port 0 (ephemeral) */
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  addr.sin_port = 0;

  if (bind(server->server_fd, (struct sockaddr *)&addr, sizeof(addr)) ==
      SOCK_ERROR) {
    close_socket(server->server_fd);
    server->server_fd = INVALID_SOCK;
    return CDD_C_ERROR_UNKNOWN;
  }

  /* Listen */
  if (listen(server->server_fd, 1) == SOCK_ERROR) {
    close_socket(server->server_fd);
    server->server_fd = INVALID_SOCK;
    return CDD_C_ERROR_UNKNOWN;
  }

  /* Retrieve assigned port */
  if (getsockname(server->server_fd, (struct sockaddr *)&addr, &addr_len) ==
      SOCK_ERROR) {
    close_socket(server->server_fd);
    server->server_fd = INVALID_SOCK;
    return CDD_C_ERROR_UNKNOWN;
  }
  server->port = (int)ntohs(addr.sin_port);

  /* Launch Thread */
  server->running = 1;

#if defined(_WIN32)
  server->thread =
      (HANDLE)BEGIN_THREAD(NULL, 0, server_thread_func, server, 0, NULL);
  if (server->thread == 0) {
    server->running = 0;
    return CDD_C_ERROR_UNKNOWN;
  }
#else
  if (PTHREAD_CREATE(&server->thread, NULL, server_thread_func, server) != 0) {
    server->running = 0;
    return CDD_C_ERROR_UNKNOWN;
  }
#endif

  return CDD_C_SUCCESS;
}

cdd_c_error_t mock_server_get_port(MockServerPtr server, int *out_port) {
  if (!out_port)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (server)
    *out_port = server->port;
  else
    *out_port = 0;
  return CDD_C_SUCCESS;
}

cdd_c_error_t mock_server_wait_for_request(MockServerPtr server,
                                           struct MockServerRequest *out_req) {
  if (!server || !out_req)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  mutex_lock(&server->lock);

  while (!server->has_request && server->running) {
    cond_wait_internal(&server->cond_req_ready, &server->lock, server);
  }

  if (server->has_request) {
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
    return CDD_C_SUCCESS;
  }

  mutex_unlock(&server->lock);
  return CDD_C_ERROR_UNKNOWN;
}

cdd_c_error_t mock_server_request_cleanup(struct MockServerRequest *req) {
  if (req && req->raw_header) {
    free(req->raw_header);
    req->raw_header = NULL;
  }
  return CDD_C_SUCCESS;
}
