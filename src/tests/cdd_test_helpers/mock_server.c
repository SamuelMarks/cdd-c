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
#include <c_cddConfig.h>
#include <process.h>
/* winsock2.h must precede windef.h to prevent conflicts */
#include <winsock2.h>

#include <ws2tcpip.h>

#include "../../win_compat_sym.h"

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

static void sleep_ms(int ms) { Sleep((DWORD)ms); }

static void mutex_init(mutex_t *m) { InitializeCriticalSection(m); }
static void mutex_destroy(mutex_t *m) { DeleteCriticalSection(m); }
static void mutex_lock(mutex_t *m) { EnterCriticalSection(m); }
static void mutex_unlock(mutex_t *m) { LeaveCriticalSection(m); }

#if defined(_MSC_VER) && _MSC_VER < 1600
static void cond_init(HANDLE *c) { *c = CreateEvent(NULL, FALSE, FALSE, NULL); }
static void cond_signal(HANDLE *c) { SetEvent(*c); }
static int cond_wait(HANDLE *c, mutex_t *m) {
  LeaveCriticalSection(m);
  WaitForSingleObject(*c, INFINITE);
  EnterCriticalSection(m);
  return 0;
}
#else
static void cond_init(cond_t *c) { InitializeConditionVariable(c); }
static void cond_signal(cond_t *c) { WakeConditionVariable(c); }
static int cond_wait(cond_t *c, mutex_t *m) {
  return SleepConditionVariableCS(c, m, INFINITE) ? 0 : 1;
}
#endif

static void close_socket(socket_t s) { closesocket(s); }

static int platform_init(void) {
  WSADATA wsa;
  return WSAStartup(MAKEWORD(2, 2), &wsa);
}
static enum cdd_c_error platform_cleanup(void) {
  WSACleanup();
  return CDD_C_SUCCESS;
}

#else /* POSIX */

#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/socket.h>
#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif
/* clang-format on */

typedef int socket_t;
typedef pthread_t thread_t;
typedef pthread_mutex_t mutex_t;
typedef pthread_cond_t cond_t;

#define INVALID_SOCK (-1)
#define SOCK_ERROR (-1)
#define THREAD_FUNC_RETURN void *
#define THREAD_FUNC_ARG void *

/* LCOV_EXCL_START */
static void sleep_ms(int ms) { usleep(ms * 1000); }
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
static void mutex_init(mutex_t *m) { pthread_mutex_init(m, NULL); }
static void mutex_destroy(mutex_t *m) { pthread_mutex_destroy(m); }
static void mutex_lock(mutex_t *m) { pthread_mutex_lock(m); }
static void mutex_unlock(mutex_t *m) { pthread_mutex_unlock(m); }
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
static void cond_init(cond_t *c) { pthread_cond_init(c, NULL); }
static void cond_signal(cond_t *c) { pthread_cond_signal(c); }
static int cond_wait(cond_t *c, mutex_t *m) { return pthread_cond_wait(c, m); }
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
static void close_socket(socket_t s) { close(s); }
/* LCOV_EXCL_STOP */

/* LCOV_EXCL_START */
static int platform_init(void) { return 0; }
static enum cdd_c_error platform_cleanup(void) { return CDD_C_SUCCESS; }
/* LCOV_EXCL_STOP */

#endif

/* --- Internal Structure --- */

/** \brief mock */
struct MockServer_ {
  socket_t server_fd;   /**< Listening socket */
  int port;             /**< Bound port */
  volatile int running; /**< Loop flag */

  thread_t thread; /**< Thread handle */

  /* Request capture */
  char *captured_request; /**< Buffer for recieved data */
  size_t captured_len;    /**< Length */

  /* Sync */
  /** @brief lock */
  mutex_t lock;
  /** @brief has_request */
  /** @brief cond_req_ready */
  cond_t cond_req_ready; /* Signaled when a request is parsed */
  /** @brief has_request */
  int has_request;

  int init_success; /**< Flag for lazy winsock init tracking */
};

/* --- Thread Routine --- */

/* LCOV_EXCL_START */
static THREAD_FUNC_RETURN server_thread_func(THREAD_FUNC_ARG arg) {
  struct MockServer_ *s = (struct MockServer_ *)arg;
  const char *response = "HTTP/1.1 200 OK\r\n"
                         /* LCOV_EXCL_STOP */
                         "Content-Type: text/plain\r\n"
                         "Content-Length: 2\r\n"
                         "\r\n"
                         "OK";

  /* LCOV_EXCL_START */
  while (s->running) {
    /* LCOV_EXCL_STOP */
    socket_t client_fd;
    struct sockaddr_in client_addr;
#if defined(_WIN32)
    int addr_len = sizeof(client_addr);
#else
    /* LCOV_EXCL_START */
    socklen_t addr_len = sizeof(client_addr);
/* LCOV_EXCL_STOP */
#endif

    /* Blocking Accept */
    client_fd =
        /* LCOV_EXCL_START */
        accept(s->server_fd, (struct sockaddr *)&client_addr, &addr_len);
    /* LCOV_EXCL_STOP */

    /* LCOV_EXCL_START */
    if (client_fd == INVALID_SOCK) {
      /* LCOV_EXCL_STOP */
      /* If stopped, this failure is expected */
      /* LCOV_EXCL_START */
      if (!s->running)
        break;
      /* LCOV_EXCL_STOP */
      /* Temporary error or real error, just retry or sleep */
      /* LCOV_EXCL_START */
      sleep_ms(10);
      continue;
      /* LCOV_EXCL_STOP */
    }

    /* Read Request */
    {
      char buffer[4096];
      int bytes_read;
      sleep_ms(100); /* Wait for body packets (e.g. from WinHTTP) */
                     /* LCOV_EXCL_START */
      bytes_read = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
      if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        /* LCOV_EXCL_STOP */

        /* LCOV_EXCL_START */
        mutex_lock(&s->lock);
        if (s->captured_request)
          free(s->captured_request);
        s->captured_request = (char *)malloc(bytes_read + 1);
        if (s->captured_request) {
          memcpy(s->captured_request, buffer, bytes_read + 1);
          s->captured_len = bytes_read;
          s->has_request = 1;
          cond_signal(&s->cond_req_ready);
          /* LCOV_EXCL_STOP */
        }
        /* LCOV_EXCL_START */
        mutex_unlock(&s->lock);
        /* LCOV_EXCL_STOP */
      }
    }

    /* Send Response */
    /* LCOV_EXCL_START */
    send(client_fd, response, (int)strlen(response), 0);
    /* LCOV_EXCL_STOP */

    /* LCOV_EXCL_START */
    close_socket(client_fd);
    /* LCOV_EXCL_STOP */
  }

  /* LCOV_EXCL_START */
  return 0;
  /* LCOV_EXCL_STOP */
}

/* --- Wrapper API --- */

/* LCOV_EXCL_START */
enum cdd_c_error mock_server_init(MockServerPtr *out) {
  /* LCOV_EXCL_STOP */
  struct MockServer_ *s;
  /* LCOV_EXCL_START */
  if (platform_init() != 0) {
    if (out)
      /* LCOV_EXCL_STOP */
      *out = NULL;
    /* LCOV_EXCL_START */
    return CDD_C_ERROR_UNKNOWN;
    /* LCOV_EXCL_STOP */
  }

  /* LCOV_EXCL_START */
  s = (struct MockServer_ *)calloc(1, sizeof(struct MockServer_));
  if (!s) {
    if (out)
      /* LCOV_EXCL_STOP */
      *out = NULL;
    /* LCOV_EXCL_START */
    return CDD_C_ERROR_UNKNOWN;
    /* LCOV_EXCL_STOP */
  }

  /* LCOV_EXCL_START */
  s->server_fd = INVALID_SOCK;
  s->running = 0;
  s->init_success = 1;
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  mutex_init(&s->lock);
  cond_init(&s->cond_req_ready);
  /* LCOV_EXCL_STOP */

  *out = s;
  /* LCOV_EXCL_START */
  return CDD_C_SUCCESS;
  /* LCOV_EXCL_STOP */
}

/* LCOV_EXCL_START */
void mock_server_destroy(MockServerPtr server) {
  if (!server)
    return;
  /* LCOV_EXCL_STOP */

  /* Stop thread if running */
  /* LCOV_EXCL_START */
  if (server->running) {
    server->running = 0;
    /* LCOV_EXCL_STOP */
    /* Force accept to unblock by shutting down and closing socket */
    /* LCOV_EXCL_START */
    if (server->server_fd != INVALID_SOCK) {
/* LCOV_EXCL_STOP */
#if defined(_WIN32)
      shutdown(server->server_fd, SD_BOTH);
#else
      /* LCOV_EXCL_START */
      shutdown(server->server_fd, SHUT_RDWR);
/* LCOV_EXCL_STOP */
#endif
      /* LCOV_EXCL_START */
      close_socket(server->server_fd);
      server->server_fd = INVALID_SOCK;
      /* LCOV_EXCL_STOP */
    }

#if defined(_WIN32)
    WaitForSingleObject(server->thread, INFINITE);
    CloseHandle(server->thread);
#else
    /* LCOV_EXCL_START */
    pthread_join(server->thread, NULL);
/* LCOV_EXCL_STOP */
#endif
  }

  /* LCOV_EXCL_START */
  if (server->server_fd != INVALID_SOCK) {
    close_socket(server->server_fd);
    /* LCOV_EXCL_STOP */
  }

  /* LCOV_EXCL_START */
  mutex_destroy(&server->lock);
  /* LCOV_EXCL_STOP */
  /* cond_destroy not strictly needed in simple pthread wrapper or windows CV */

  /* LCOV_EXCL_START */
  if (server->captured_request)
    free(server->captured_request);
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  free(server);
  platform_cleanup();
  /* LCOV_EXCL_STOP */
}

/* LCOV_EXCL_START */
enum cdd_c_error mock_server_start(MockServerPtr server) {
  /* LCOV_EXCL_STOP */
  struct sockaddr_in addr;
#if defined(_WIN32)
  int addr_len = sizeof(addr);
#else
  /* LCOV_EXCL_START */
  socklen_t addr_len = sizeof(addr);
/* LCOV_EXCL_STOP */
#endif

  /* LCOV_EXCL_START */
  if (!server || server->running)
    return CDD_C_ERROR_UNKNOWN;
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  server->server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server->server_fd == INVALID_SOCK)
    return CDD_C_ERROR_UNKNOWN;
  /* LCOV_EXCL_STOP */

  /* Bind to loopback, port 0 (ephemeral) */
  /* LCOV_EXCL_START */
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  addr.sin_port = 0;
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  if (bind(server->server_fd, (struct sockaddr *)&addr, sizeof(addr)) ==
      /* LCOV_EXCL_STOP */
      SOCK_ERROR) {
    /* LCOV_EXCL_START */
    close_socket(server->server_fd);
    return CDD_C_ERROR_UNKNOWN;
    /* LCOV_EXCL_STOP */
  }

  /* Listen */
  /* LCOV_EXCL_START */
  if (listen(server->server_fd, 1) == SOCK_ERROR) {
    close_socket(server->server_fd);
    return CDD_C_ERROR_UNKNOWN;
    /* LCOV_EXCL_STOP */
  }

  /* Retrieve assigned port */
  /* LCOV_EXCL_START */
  if (getsockname(server->server_fd, (struct sockaddr *)&addr, &addr_len) ==
      /* LCOV_EXCL_STOP */
      SOCK_ERROR) {
    /* LCOV_EXCL_START */
    close_socket(server->server_fd);
    return CDD_C_ERROR_UNKNOWN;
    /* LCOV_EXCL_STOP */
  }
  /* LCOV_EXCL_START */
  server->port = ntohs(addr.sin_port);
  /* LCOV_EXCL_STOP */

  /* Launch Thread */
  /* LCOV_EXCL_START */
  server->running = 1;
  /* LCOV_EXCL_STOP */

#if defined(_WIN32)
  server->thread =
      (HANDLE)_beginthreadex(NULL, 0, server_thread_func, server, 0, NULL);
  if (server->thread == 0) {
    server->running = 0;
    close_socket(server->server_fd);
    return CDD_C_ERROR_UNKNOWN;
  }
#else
  /* LCOV_EXCL_START */
  if (pthread_create(&server->thread, NULL, server_thread_func, server) != 0) {
    server->running = 0;
    close_socket(server->server_fd);
    return CDD_C_ERROR_UNKNOWN;
    /* LCOV_EXCL_STOP */
  }
#endif

  /* LCOV_EXCL_START */
  return CDD_C_SUCCESS;
  /* LCOV_EXCL_STOP */
}

/* LCOV_EXCL_START */
enum cdd_c_error mock_server_get_port(MockServerPtr server, int *out_port) {
  if (server)
    /* LCOV_EXCL_STOP */
    *out_port = server->port;
  else
    *out_port = 0;
  /* LCOV_EXCL_START */
  return CDD_C_SUCCESS;
  /* LCOV_EXCL_STOP */
}

enum cdd_c_error
/* LCOV_EXCL_START */
mock_server_wait_for_request(MockServerPtr server,
                             /* LCOV_EXCL_STOP */
                             struct MockServerRequest *out_req) {
  /* LCOV_EXCL_START */
  if (!server || !out_req)
    return CDD_C_ERROR_UNKNOWN;
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  mutex_lock(&server->lock);
  while (!server->has_request && server->running) {
    cond_wait(&server->cond_req_ready, &server->lock);
    /* LCOV_EXCL_STOP */
  }

  /* LCOV_EXCL_START */
  if (server->has_request && server->captured_request) {
    /* LCOV_EXCL_STOP */
    /* Copy data out */
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
    out_req->raw_header = _strdup(server->captured_request);
#else
    /* LCOV_EXCL_START */
    out_req->raw_header = strdup(server->captured_request);
/* LCOV_EXCL_STOP */
#endif
    /* LCOV_EXCL_START */
    out_req->header_len = server->captured_len;
    /* LCOV_EXCL_STOP */

    /* Consume it */
    /* LCOV_EXCL_START */
    free(server->captured_request);
    server->captured_request = NULL;
    server->has_request = 0;
    /* LCOV_EXCL_STOP */

    /* LCOV_EXCL_START */
    mutex_unlock(&server->lock);
    return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
  }

  /* LCOV_EXCL_START */
  mutex_unlock(&server->lock);
  return CDD_C_ERROR_UNKNOWN;
  /* LCOV_EXCL_STOP */
}

/* LCOV_EXCL_START */
enum cdd_c_error mock_server_request_cleanup(struct MockServerRequest *req) {
  if (req && req->raw_header) {
    free(req->raw_header);
    req->raw_header = NULL;
    /* LCOV_EXCL_STOP */
  }
  /* LCOV_EXCL_START */
  return CDD_C_SUCCESS;
  /* LCOV_EXCL_STOP */
}
