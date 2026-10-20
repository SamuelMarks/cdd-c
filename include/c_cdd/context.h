/**
 * @file context.h
 * @brief Dependency-injected allocator, virtual filesystem (VFS), and socket
 * VTable abstractions (`cdd_context_t`).
 *
 * Provides deterministic, thread-safe dependency injection for memory
 * allocation, file I/O, and network socket operations without polluting
 * production modules with global fault-injection counters.
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_CONTEXT_H
#define C_CDD_CONTEXT_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include "cdd_c_error.h"
#include <stddef.h>
#include <stdio.h>
/* clang-format on */

/**
 * @brief Virtual function table for memory allocation operations.
 */
struct cdd_allocator_vtable_t {
  /**
   * @brief Allocates `size` uninitialized bytes.
   *
   * @param[in] size Number of bytes to allocate.
   * @param[out] out_ptr Output pointer receiving the allocated block.
   * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_MEMORY` on failure.
   */
  cdd_c_error_t (*malloc_fn)(size_t size, void **out_ptr);

  /**
   * @brief Allocates zero-initialized array of `count` elements of `size`
   * bytes.
   *
   * @param[in] count Number of elements.
   * @param[in] size Size of each element in bytes.
   * @param[out] out_ptr Output pointer receiving the allocated block.
   * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_MEMORY` on failure.
   */
  cdd_c_error_t (*calloc_fn)(size_t count, size_t size, void **out_ptr);

  /**
   * @brief Resizes an existing allocation `ptr` to `new_size` bytes.
   *
   * @param[in] ptr Existing allocation pointer (or `NULL`).
   * @param[in] new_size Requested new size in bytes.
   * @param[out] out_ptr Output pointer receiving the resized block.
   * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_MEMORY` on failure.
   */
  cdd_c_error_t (*realloc_fn)(void *ptr, size_t new_size, void **out_ptr);

  /**
   * @brief Duplicates a null-terminated string `str`.
   *
   * @param[in] str Input null-terminated string.
   * @param[out] out_str Output pointer receiving the duplicated string.
   * @return `CDD_C_SUCCESS` on success, or error code on failure.
   */
  cdd_c_error_t (*strdup_fn)(const char *str, char **out_str);

  /**
   * @brief Releases a previously allocated memory block.
   *
   * @param[in] ptr Pointer to free (may be `NULL`).
   */
  void (*free_fn)(void *ptr);
};

/**
 * @brief Virtual function table for filesystem (VFS) operations.
 */
struct cdd_vfs_vtable_t {
  /**
   * @brief Opens a file at `path` with `mode`.
   *
   * @param[in] path File path string.
   * @param[in] mode Standard `fopen` mode string.
   * @param[out] out_fp Output `FILE *` handle.
   * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_IO` on failure.
   */
  cdd_c_error_t (*fopen_fn)(const char *path, const char *mode, FILE **out_fp);

  /**
   * @brief Reads up to `count` elements of `elem_size` bytes from `fp`.
   *
   * @param[out] buf Destination buffer.
   * @param[in] elem_size Size of each element in bytes.
   * @param[in] count Maximum number of elements to read.
   * @param[in,out] fp Open `FILE *` stream.
   * @param[out] out_read Number of elements actually read.
   * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_IO` on failure.
   */
  cdd_c_error_t (*fread_fn)(void *buf, size_t elem_size, size_t count, FILE *fp,
                            size_t *out_read);

  /**
   * @brief Writes `count` elements of `elem_size` bytes to `fp`.
   *
   * @param[in] buf Source buffer.
   * @param[in] elem_size Size of each element in bytes.
   * @param[in] count Number of elements to write.
   * @param[in,out] fp Open `FILE *` stream.
   * @param[out] out_written Number of elements actually written.
   * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_IO` on failure.
   */
  cdd_c_error_t (*fwrite_fn)(const void *buf, size_t elem_size, size_t count,
                             FILE *fp, size_t *out_written);

  /**
   * @brief Closes an open `FILE *` stream.
   *
   * @param[in,out] fp Stream to close.
   * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_IO` on failure.
   */
  cdd_c_error_t (*fclose_fn)(FILE *fp);

  /**
   * @brief Queries whether `path` exists and whether it is a directory.
   *
   * @param[in] path Filesystem path to inspect.
   * @param[out] out_is_dir Receives `1` if directory, `0` if regular file.
   * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_NOT_FOUND` if missing.
   */
  cdd_c_error_t (*stat_fn)(const char *path, int *out_is_dir);

  /**
   * @brief Opens a directory handle for iteration.
   *
   * @param[in] path Directory path to open.
   * @param[out] out_dir Opaque directory handle.
   * @return `CDD_C_SUCCESS` on success, or error code on failure.
   */
  cdd_c_error_t (*opendir_fn)(const char *path, void **out_dir);
};

/**
 * @brief Virtual function table for network socket operations.
 */
struct cdd_socket_vtable_t {
  /**
   * @brief Creates a network socket descriptor.
   *
   * @param[in] domain Address family (`AF_INET`, etc.).
   * @param[in] type Socket type (`SOCK_STREAM`, etc.).
   * @param[in] protocol Protocol number.
   * @param[out] out_fd Output socket descriptor.
   * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_IO` on failure.
   */
  cdd_c_error_t (*socket_fn)(int domain, int type, int protocol, int *out_fd);

  /**
   * @brief Binds `fd` to `host` and `port`.
   *
   * @param[in] fd Socket descriptor.
   * @param[in] host Bind address string.
   * @param[in] port Bind TCP port.
   * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_IO` on failure.
   */
  cdd_c_error_t (*bind_fn)(int fd, const char *host, int port);

  /**
   * @brief Marks `fd` as a passive listening socket.
   *
   * @param[in] fd Socket descriptor.
   * @param[in] backlog Maximum pending connections queue length.
   * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_IO` on failure.
   */
  cdd_c_error_t (*listen_fn)(int fd, int backlog);

  /**
   * @brief Accepts an incoming client connection on `fd`.
   *
   * @param[in] fd Listening socket descriptor.
   * @param[out] out_client_fd Accepted client socket descriptor.
   * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_IO` on failure.
   */
  cdd_c_error_t (*accept_fn)(int fd, int *out_client_fd);

  /**
   * @brief Receives up to `len` bytes from `fd` into `buf`.
   *
   * @param[in] fd Connected socket descriptor.
   * @param[out] buf Destination buffer.
   * @param[in] len Buffer capacity in bytes.
   * @param[out] out_received Number of bytes received.
   * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_IO` on failure.
   */
  cdd_c_error_t (*recv_fn)(int fd, void *buf, size_t len, size_t *out_received);

  /**
   * @brief Sends `len` bytes from `buf` over `fd`.
   *
   * @param[in] fd Connected socket descriptor.
   * @param[in] buf Source buffer.
   * @param[in] len Number of bytes to send.
   * @param[out] out_sent Number of bytes sent.
   * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_IO` on failure.
   */
  cdd_c_error_t (*send_fn)(int fd, const void *buf, size_t len,
                           size_t *out_sent);

  /**
   * @brief Closes a socket descriptor.
   *
   * @param[in] fd Socket descriptor to close.
   * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_IO` on failure.
   */
  cdd_c_error_t (*close_fn)(int fd);
};

/**
 * @brief Aggregated dependency-injection runtime context (`cdd_context_t`).
 */
typedef struct cdd_context_t {
  struct cdd_allocator_vtable_t allocator; /**< Memory allocator VTable */
  struct cdd_vfs_vtable_t vfs;             /**< Virtual filesystem VTable */
  struct cdd_socket_vtable_t socket_vt;    /**< Network socket VTable */
  void *user_data;                         /**< Optional caller state */
} cdd_context_t;

/**
 * @brief Initializes `ctx` with the default standard C allocator, VFS, and
 * socket VTables.
 *
 * @param[out] ctx Context structure to initialize.
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_INVALID_ARGUMENT` if
 * `ctx` is `NULL`.
 */
C_CDD_EXPORT cdd_c_error_t cdd_context_init_default(cdd_context_t *ctx);

/**
 * @brief Sets the active process/thread-local `cdd_context_t` override (or
 * resets to the built-in default when `ctx == NULL`).
 *
 * @param[in] ctx Custom context to activate, or `NULL` to restore defaults.
 * @return `CDD_C_SUCCESS`.
 */
C_CDD_EXPORT cdd_c_error_t cdd_context_set_global(const cdd_context_t *ctx);

/**
 * @brief Retrieves the currently active `cdd_context_t` (never `NULL`).
 *
 * @param[out] out_ctx Output pointer receiving the active `cdd_context_t *`.
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_INVALID_ARGUMENT` if
 * `out_ctx` is `NULL`.
 */
C_CDD_EXPORT cdd_c_error_t
cdd_context_get_global(const cdd_context_t **out_ctx);

/** @brief Countdown counter for failing VFS fopen operations */
extern C_CDD_EXPORT int g_cdd_fopen_fail;

/**
 * @brief Opens a file through the active `cdd_context_t` VFS `fopen_fn` hook.
 *
 * @param[in] path Filesystem path.
 * @param[in] mode File access mode string.
 * @param[out] out_fp Output file handle pointer.
 * @return `CDD_C_SUCCESS` on success, or error code on failure.
 */
C_CDD_EXPORT cdd_c_error_t cdd_vfs_fopen(const char *path, const char *mode,
                                         FILE **out_fp);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_CONTEXT_H */
