/**
 * @file cdd_crt_bridge.h
 * @brief Cross-DLL CRT bridge for Windows static CRT (/MT, /MTd) shared library
 * test builds.
 *
 * When `BUILD_SHARED_LIBS=ON` is combined with a static MSVC CRT (`/MT` or
 * `/MTd`), `c_cdd.dll` and `test_c_cdd.exe` each link their own private copy of
 * the C runtime with separate heap handles (`__acrt_heap`), separate file
 * descriptor tables (`__pioinfo`), and separate environment blocks
 * (`_environ`). This header routes heap, `FILE *`, file descriptor, and
 * environment operations in test translation units through `c_cdd.dll`'s CRT.
 */

#ifndef C_CDD_TESTS_CDD_CRT_BRIDGE_H
#define C_CDD_TESTS_CDD_CRT_BRIDGE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"
#include "c_cdd/suppress_gui.h"
#include "c_cdd_export.h"
#include "cdd_c_error.h"
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined(_WIN32)
#include <direct.h>
#include <io.h>
#endif
/* clang-format on */

extern C_CDD_EXPORT cdd_c_error_t c_cdd_crt_init_runtime(void);
extern C_CDD_EXPORT cdd_c_error_t c_cdd_crt_malloc(size_t sz, void **out_ptr);
extern C_CDD_EXPORT cdd_c_error_t c_cdd_crt_calloc(size_t n, size_t sz,
                                                   void **out_ptr);
extern C_CDD_EXPORT cdd_c_error_t c_cdd_crt_realloc(void *p, size_t sz,
                                                    void **out_ptr);
extern C_CDD_EXPORT cdd_c_error_t c_cdd_crt_strdup(const char *s,
                                                   char **out_str);
extern C_CDD_EXPORT void c_cdd_crt_free(void *p);
extern C_CDD_EXPORT cdd_c_error_t c_cdd_crt_fopen(const char *path,
                                                  const char *mode,
                                                  FILE **out_fp, int *out_err);
extern C_CDD_EXPORT cdd_c_error_t c_cdd_crt_fclose(FILE *fp, int *out_rc);
extern C_CDD_EXPORT cdd_c_error_t c_cdd_crt_fseek(FILE *fp, long offset,
                                                  int origin, int *out_rc);
extern C_CDD_EXPORT cdd_c_error_t c_cdd_crt_ftell(FILE *fp, long *out_pos);
extern C_CDD_EXPORT cdd_c_error_t c_cdd_crt_rewind(FILE *fp);
extern C_CDD_EXPORT cdd_c_error_t c_cdd_crt_fread(void *buf, size_t sz,
                                                  size_t n, FILE *fp,
                                                  size_t *out_read);
extern C_CDD_EXPORT cdd_c_error_t c_cdd_crt_fwrite(const void *buf, size_t sz,
                                                   size_t n, FILE *fp,
                                                   size_t *out_written);
extern C_CDD_EXPORT cdd_c_error_t c_cdd_crt_fputs(const char *s, FILE *fp,
                                                  int *out_rc);
extern C_CDD_EXPORT cdd_c_error_t c_cdd_crt_fgets(char *s, int n, FILE *fp,
                                                  char **out_res);
extern C_CDD_EXPORT cdd_c_error_t c_cdd_crt_fflush(FILE *fp, int *out_rc);
extern C_CDD_EXPORT cdd_c_error_t c_cdd_crt_clearerr(FILE *fp);
extern C_CDD_EXPORT cdd_c_error_t c_cdd_crt_vfprintf(FILE *fp, const char *fmt,
                                                     va_list args, int *out_rc);
extern C_CDD_EXPORT cdd_c_error_t c_cdd_crt_vasprintf(char **out_str,
                                                      const char *fmt,
                                                      va_list args,
                                                      int *out_rc);
extern C_CDD_EXPORT cdd_c_error_t c_cdd_crt_jasprintf(char **unto,
                                                      const char *str,
                                                      int *out_rc);
extern C_CDD_EXPORT cdd_c_error_t c_cdd_crt_putenv(const char *envstr,
                                                   int *out_rc);
extern C_CDD_EXPORT cdd_c_error_t c_cdd_crt_std_stream(int idx, FILE **out_fp);
extern C_CDD_EXPORT cdd_c_error_t c_cdd_crt_freopen(const char *path,
                                                    const char *mode,
                                                    FILE *stream, FILE **out_fp,
                                                    int *out_err);
extern C_CDD_EXPORT cdd_c_error_t c_cdd_crt_fgetpos(FILE *fp, fpos_t *pos,
                                                    int *out_rc);
extern C_CDD_EXPORT cdd_c_error_t c_cdd_crt_fsetpos(FILE *fp, const fpos_t *pos,
                                                    int *out_rc);
extern C_CDD_EXPORT cdd_c_error_t c_cdd_crt_fileno(FILE *fp, int *out_fd);
extern C_CDD_EXPORT cdd_c_error_t c_cdd_crt_dup(int fd, int *out_fd);
extern C_CDD_EXPORT cdd_c_error_t c_cdd_crt_dup2(int fd1, int fd2, int *out_rc);
extern C_CDD_EXPORT cdd_c_error_t c_cdd_crt_close(int fd, int *out_rc);

#if defined(_MSC_VER)

/**
 * @brief Maps caller stdin/stdout/stderr to c_cdd's CRT standard streams.
 * @param[in] fp Input FILE pointer.
 * @return Corresponding FILE pointer in c_cdd's CRT.
 */
static __inline FILE *test_crt_resolve_stream(FILE *fp) {
  FILE *dll_fp = fp;
  if (!fp)
    return NULL;
  if (fp == stdin) {
    if (c_cdd_crt_std_stream(0, &dll_fp) != CDD_C_SUCCESS)
      return fp;
    return dll_fp;
  }
  if (fp == stdout) {
    if (c_cdd_crt_std_stream(1, &dll_fp) != CDD_C_SUCCESS)
      return fp;
    return dll_fp;
  }
  if (fp == stderr) {
    if (c_cdd_crt_std_stream(2, &dll_fp) != CDD_C_SUCCESS)
      return fp;
    return dll_fp;
  }
  return fp;
}

/**
 * @brief Wrapper for malloc routing to c_cdd's CRT heap.
 * @param[in] sz Bytes to allocate.
 * @return Allocated pointer or NULL.
 */
static __inline void *test_crt_malloc(size_t sz) {
  void *p = NULL;
  cdd_c_error_t rc = c_cdd_crt_malloc(sz, &p);
  if (rc != CDD_C_SUCCESS)
    return NULL;
  return p;
}

/**
 * @brief Wrapper for calloc routing to c_cdd's CRT heap.
 * @param[in] n Element count.
 * @param[in] sz Element size.
 * @return Allocated pointer or NULL.
 */
static __inline void *test_crt_calloc(size_t n, size_t sz) {
  void *p = NULL;
  cdd_c_error_t rc = c_cdd_crt_calloc(n, sz, &p);
  if (rc != CDD_C_SUCCESS)
    return NULL;
  return p;
}

/**
 * @brief Wrapper for realloc routing to c_cdd's CRT heap.
 * @param[in,out] ptr Existing pointer.
 * @param[in] sz New size in bytes.
 * @return Resized pointer or NULL.
 */
static __inline void *test_crt_realloc(void *ptr, size_t sz) {
  void *p = NULL;
  cdd_c_error_t rc = c_cdd_crt_realloc(ptr, sz, &p);
  if (rc != CDD_C_SUCCESS)
    return NULL;
  return p;
}

/**
 * @brief Wrapper for strdup routing to c_cdd's CRT heap.
 * @param[in] s Source string.
 * @return Duplicated string or NULL.
 */
static __inline char *test_crt_strdup(const char *s) {
  char *d = NULL;
  cdd_c_error_t rc = c_cdd_crt_strdup(s, &d);
  if (rc != CDD_C_SUCCESS)
    return NULL;
  return d;
}

/**
 * @brief Wrapper for fopen routing to c_cdd's CRT file table.
 * @param[in] path File path.
 * @param[in] mode Access mode.
 * @return Opened FILE pointer or NULL.
 */
static __inline FILE *test_crt_fopen(const char *path, const char *mode) {
  FILE *f = NULL;
  int err = 0;
  cdd_c_error_t rc = c_cdd_crt_fopen(path, mode, &f, &err);
  if (rc != CDD_C_SUCCESS && err != 0)
    errno = err;
  return f;
}

/**
 * @brief Wrapper for fopen_s routing to c_cdd's CRT file table.
 * @param[out] out_fp Output FILE pointer.
 * @param[in] path File path.
 * @param[in] mode Access mode.
 * @return 0 on success, or errno code.
 */
static __inline int test_crt_fopen_s(FILE **out_fp, const char *path,
                                     const char *mode) {
  int err = 0;
  cdd_c_error_t rc;
  if (!out_fp)
    return EINVAL;
  rc = c_cdd_crt_fopen(path, mode, out_fp, &err);
  if (rc != CDD_C_SUCCESS && err == 0)
    err = EINVAL;
  if (err != 0)
    errno = err;
  return err;
}

/**
 * @brief Wrapper for freopen routing to c_cdd's CRT file table.
 * @param[in] path File path.
 * @param[in] mode Access mode.
 * @param[in,out] stream Existing FILE stream.
 * @return Reopened FILE pointer or NULL.
 */
static __inline FILE *test_crt_freopen(const char *path, const char *mode,
                                       FILE *stream) {
  FILE *f = NULL;
  int err = 0;
  cdd_c_error_t rc =
      c_cdd_crt_freopen(path, mode, test_crt_resolve_stream(stream), &f, &err);
  if (rc != CDD_C_SUCCESS && err != 0)
    errno = err;
  return f;
}

/**
 * @brief Wrapper for freopen_s routing to c_cdd's CRT file table.
 * @param[out] out_fp Output FILE pointer.
 * @param[in] path File path.
 * @param[in] mode Access mode.
 * @param[in,out] stream Existing FILE stream.
 * @return 0 on success, or errno code.
 */
static __inline int test_crt_freopen_s(FILE **out_fp, const char *path,
                                       const char *mode, FILE *stream) {
  int err = 0;
  cdd_c_error_t rc;
  if (!out_fp)
    return EINVAL;
  rc = c_cdd_crt_freopen(path, mode, test_crt_resolve_stream(stream), out_fp,
                         &err);
  if (rc != CDD_C_SUCCESS && err == 0)
    err = EINVAL;
  if (err != 0)
    errno = err;
  return err;
}

/**
 * @brief Wrapper for fclose routing to c_cdd's CRT file table.
 * @param[in,out] fp File stream.
 * @return 0 on success, or EOF.
 */
static __inline int test_crt_fclose(FILE *fp) {
  int out_rc = EOF;
  cdd_c_error_t rc = c_cdd_crt_fclose(test_crt_resolve_stream(fp), &out_rc);
  if (rc != CDD_C_SUCCESS)
    return out_rc;
  return 0;
}

/**
 * @brief Wrapper for fseek routing to c_cdd's CRT file table.
 * @param[in,out] fp File stream.
 * @param[in] offset Byte offset.
 * @param[in] origin Seek origin.
 * @return 0 on success, or non-zero on error.
 */
static __inline int test_crt_fseek(FILE *fp, long offset, int origin) {
  int out_rc = -1;
  cdd_c_error_t rc =
      c_cdd_crt_fseek(test_crt_resolve_stream(fp), offset, origin, &out_rc);
  if (rc != CDD_C_SUCCESS)
    return out_rc;
  return 0;
}

/**
 * @brief Wrapper for ftell routing to c_cdd's CRT file table.
 * @param[in,out] fp File stream.
 * @return Current byte offset, or -1L on error.
 */
static __inline long test_crt_ftell(FILE *fp) {
  long pos = -1L;
  cdd_c_error_t rc = c_cdd_crt_ftell(test_crt_resolve_stream(fp), &pos);
  if (rc != CDD_C_SUCCESS)
    return -1L;
  return pos;
}

/**
 * @brief Wrapper for rewind routing to c_cdd's CRT file table.
 * @param[in,out] fp File stream.
 */
static __inline void test_crt_rewind(FILE *fp) {
  cdd_c_error_t rc = c_cdd_crt_rewind(test_crt_resolve_stream(fp));
  if (rc != CDD_C_SUCCESS)
    return;
}

/**
 * @brief Wrapper for fgetpos routing to c_cdd's CRT file table.
 * @param[in,out] fp File stream.
 * @param[out] pos Output position.
 * @return 0 on success, or non-zero on error.
 */
static __inline int test_crt_fgetpos(FILE *fp, fpos_t *pos) {
  int out_rc = -1;
  cdd_c_error_t rc =
      c_cdd_crt_fgetpos(test_crt_resolve_stream(fp), pos, &out_rc);
  if (rc != CDD_C_SUCCESS)
    return out_rc;
  return 0;
}

/**
 * @brief Wrapper for fsetpos routing to c_cdd's CRT file table.
 * @param[in,out] fp File stream.
 * @param[in] pos Input position.
 * @return 0 on success, or non-zero on error.
 */
static __inline int test_crt_fsetpos(FILE *fp, const fpos_t *pos) {
  int out_rc = -1;
  cdd_c_error_t rc =
      c_cdd_crt_fsetpos(test_crt_resolve_stream(fp), pos, &out_rc);
  if (rc != CDD_C_SUCCESS)
    return out_rc;
  return 0;
}

/**
 * @brief Wrapper for fread routing to c_cdd's CRT file table.
 * @param[out] buf Destination buffer.
 * @param[in] sz Element size.
 * @param[in] n Element count.
 * @param[in,out] fp File stream.
 * @return Number of elements read.
 */
static __inline size_t test_crt_fread(void *buf, size_t sz, size_t n,
                                      FILE *fp) {
  size_t r = 0;
  cdd_c_error_t rc =
      c_cdd_crt_fread(buf, sz, n, test_crt_resolve_stream(fp), &r);
  if (rc != CDD_C_SUCCESS)
    return 0;
  return r;
}

/**
 * @brief Wrapper for fwrite routing to c_cdd's CRT file table.
 * @param[in] buf Source buffer.
 * @param[in] sz Element size.
 * @param[in] n Element count.
 * @param[in,out] fp File stream.
 * @return Number of elements written.
 */
static __inline size_t test_crt_fwrite(const void *buf, size_t sz, size_t n,
                                       FILE *fp) {
  size_t w = 0;
  cdd_c_error_t rc =
      c_cdd_crt_fwrite(buf, sz, n, test_crt_resolve_stream(fp), &w);
  if (rc != CDD_C_SUCCESS)
    return 0;
  return w;
}

/**
 * @brief Wrapper for fputs routing to c_cdd's CRT file table.
 * @param[in] s String to write.
 * @param[in,out] fp File stream.
 * @return Non-negative on success, or EOF.
 */
static __inline int test_crt_fputs(const char *s, FILE *fp) {
  int out_rc = EOF;
  cdd_c_error_t rc = c_cdd_crt_fputs(s, test_crt_resolve_stream(fp), &out_rc);
  if (rc != CDD_C_SUCCESS)
    return EOF;
  return out_rc;
}

/**
 * @brief Wrapper for fgets routing to c_cdd's CRT file table.
 * @param[out] s Destination buffer.
 * @param[in] n Buffer capacity.
 * @param[in,out] fp File stream.
 * @return s on success, or NULL.
 */
static __inline char *test_crt_fgets(char *s, int n, FILE *fp) {
  char *res = NULL;
  cdd_c_error_t rc = c_cdd_crt_fgets(s, n, test_crt_resolve_stream(fp), &res);
  if (rc != CDD_C_SUCCESS)
    return NULL;
  return res;
}

/**
 * @brief Wrapper for fflush routing to c_cdd's CRT file table.
 * @param[in,out] fp File stream.
 * @return 0 on success, or EOF.
 */
static __inline int test_crt_fflush(FILE *fp) {
  int out_rc = 0;
  cdd_c_error_t rc;
  if (!fp || fp == stdout || fp == stderr)
    fflush(fp);
  rc = c_cdd_crt_fflush(test_crt_resolve_stream(fp), &out_rc);
  if (rc != CDD_C_SUCCESS)
    return out_rc;
  return 0;
}

/**
 * @brief Wrapper for clearerr routing to c_cdd's CRT file table.
 * @param[in,out] fp File stream.
 */
static __inline void test_crt_clearerr(FILE *fp) {
  cdd_c_error_t rc;
  if (fp == stdin || fp == stdout || fp == stderr)
    clearerr(fp);
  rc = c_cdd_crt_clearerr(test_crt_resolve_stream(fp));
  if (rc != CDD_C_SUCCESS)
    return;
}

/**
 * @brief Wrapper for fileno routing to c_cdd's CRT file table.
 * @param[in] fp File stream.
 * @return File descriptor, or -1 on error.
 */
static __inline int test_crt_fileno(FILE *fp) {
  int fd = -1;
  cdd_c_error_t rc = c_cdd_crt_fileno(test_crt_resolve_stream(fp), &fd);
  if (rc != CDD_C_SUCCESS)
    return -1;
  return fd;
}

/**
 * @brief Wrapper for dup routing to c_cdd's CRT file table.
 * @param[in] fd File descriptor.
 * @return Duplicated file descriptor, or -1 on error.
 */
static __inline int test_crt_dup(int fd) {
  int out_fd = -1;
  cdd_c_error_t rc = c_cdd_crt_dup(fd, &out_fd);
  if (rc != CDD_C_SUCCESS)
    return -1;
  return out_fd;
}

/**
 * @brief Wrapper for dup2 routing to c_cdd's CRT file table.
 * @param[in] fd1 Source file descriptor.
 * @param[in] fd2 Target file descriptor.
 * @return 0 on success, or -1 on error.
 */
static __inline int test_crt_dup2(int fd1, int fd2) {
  int out_rc = -1;
  cdd_c_error_t rc = c_cdd_crt_dup2(fd1, fd2, &out_rc);
  if (rc != CDD_C_SUCCESS)
    return -1;
  return out_rc;
}

/**
 * @brief Wrapper for close routing to c_cdd's CRT file table.
 * @param[in] fd File descriptor.
 * @return 0 on success, or -1 on error.
 */
static __inline int test_crt_close(int fd) {
  int out_rc = -1;
  cdd_c_error_t rc = c_cdd_crt_close(fd, &out_rc);
  if (rc != CDD_C_SUCCESS)
    return -1;
  return out_rc;
}

/**
 * @brief Wrapper for _putenv updating both caller and c_cdd CRT environments.
 * @param[in] envstr "KEY=VALUE" string.
 * @return 0 on success, or -1 on error.
 */
static __inline int test_crt_putenv(const char *envstr) {
  int out_rc = -1;
  cdd_c_error_t rc;
  _putenv(envstr);
  rc = c_cdd_crt_putenv(envstr, &out_rc);
  if (rc != CDD_C_SUCCESS)
    return -1;
  return out_rc;
}

/**
 * @brief Wrapper for vfprintf routing to c_cdd's CRT file table.
 * @param[in,out] fp File stream.
 * @param[in] fmt Format string.
 * @param[in] args Variadic argument list.
 * @return Number of characters written, or negative on error.
 */
static __inline int test_crt_vfprintf(FILE *fp, const char *fmt, va_list args) {
  int out_rc = -1;
  cdd_c_error_t rc =
      c_cdd_crt_vfprintf(test_crt_resolve_stream(fp), fmt, args, &out_rc);
  if (rc != CDD_C_SUCCESS && out_rc >= 0)
    out_rc = -1;
  return out_rc;
}

/**
 * @brief Wrapper for fprintf routing to c_cdd's CRT file table.
 * @param[in,out] fp File stream.
 * @param[in] fmt Format string.
 * @return Number of characters written, or negative on error.
 */
static __inline int test_crt_fprintf(FILE *fp, const char *fmt, ...) {
  va_list args;
  int out_rc = -1;
  va_start(args, fmt);
  out_rc = test_crt_vfprintf(fp, fmt, args);
  va_end(args);
  return out_rc;
}

/**
 * @brief Wrapper for vasprintf routing to c_cdd's CRT heap.
 * @param[out] out_str Output allocated string.
 * @param[in] fmt Format string.
 * @param[in] args Variadic argument list.
 * @return Number of characters written, or -1 on error.
 */
static __inline int test_crt_vasprintf(char **out_str, const char *fmt,
                                       va_list args) {
  int out_rc = -1;
  cdd_c_error_t rc = c_cdd_crt_vasprintf(out_str, fmt, args, &out_rc);
  if (rc != CDD_C_SUCCESS)
    return -1;
  return out_rc;
}

/**
 * @brief Wrapper for asprintf routing to c_cdd's CRT heap.
 * @param[out] out_str Output allocated string.
 * @param[in] fmt Format string.
 * @return Number of characters written, or -1 on error.
 */
static __inline int test_crt_asprintf(char **out_str, const char *fmt, ...) {
  va_list args;
  int out_rc = -1;
  cdd_c_error_t rc;
  va_start(args, fmt);
  rc = c_cdd_crt_vasprintf(out_str, fmt, args, &out_rc);
  if (rc != CDD_C_SUCCESS)
    out_rc = -1;
  va_end(args);
  return out_rc;
}

/**
 * @brief Wrapper for jasprintf routing to c_cdd's CRT heap.
 * @param[in,out] unto Target string pointer.
 * @param[in] fmt Format string.
 * @return Number of characters written, or -1 on error.
 */
static __inline int test_crt_jasprintf(char **unto, const char *fmt, ...) {
  va_list args;
  char *new_part = NULL;
  int out_rc = -1;
  cdd_c_error_t rc;
  va_start(args, fmt);
  rc = c_cdd_crt_vasprintf(&new_part, fmt, args, &out_rc);
  va_end(args);
  if (rc != CDD_C_SUCCESS || !new_part)
    return -1;
  rc = c_cdd_crt_jasprintf(unto, new_part, &out_rc);
  c_cdd_crt_free(new_part);
  if (rc != CDD_C_SUCCESS)
    return -1;
  return out_rc;
}

#ifndef CDD_CRT_BRIDGE_NO_MACROS
#undef malloc
#define malloc test_crt_malloc
#undef calloc
#define calloc test_crt_calloc
#undef realloc
#define realloc test_crt_realloc
#undef free
#define free c_cdd_crt_free
#undef strdup
#define strdup test_crt_strdup
#undef _strdup
#define _strdup test_crt_strdup
#undef fopen
#define fopen test_crt_fopen
#undef fopen_s
#define fopen_s test_crt_fopen_s
#undef freopen
#define freopen test_crt_freopen
#undef freopen_s
#define freopen_s test_crt_freopen_s
#undef fclose
#define fclose test_crt_fclose
#undef fseek
#define fseek test_crt_fseek
#undef ftell
#define ftell test_crt_ftell
#undef rewind
#define rewind test_crt_rewind
#undef fgetpos
#define fgetpos test_crt_fgetpos
#undef fsetpos
#define fsetpos test_crt_fsetpos
#undef fread
#define fread test_crt_fread
#undef fwrite
#define fwrite test_crt_fwrite
#undef fputs
#define fputs test_crt_fputs
#undef fgets
#define fgets test_crt_fgets
#undef fflush
#define fflush test_crt_fflush
#undef clearerr
#define clearerr test_crt_clearerr
#undef _fileno
#define _fileno test_crt_fileno
#undef fileno
#define fileno _fileno
#undef _dup
#define _dup test_crt_dup
#undef dup
#define dup _dup
#undef _dup2
#define _dup2 test_crt_dup2
#undef dup2
#define dup2 _dup2
#undef _close
#define _close test_crt_close
#undef close
#define close _close
#undef _putenv
#define _putenv test_crt_putenv
#undef putenv
#define putenv test_crt_putenv
#undef vfprintf
#define vfprintf test_crt_vfprintf
#undef fprintf
#define fprintf test_crt_fprintf
#undef asprintf
#define asprintf test_crt_asprintf
#undef c89stringutils_asprintf
#define c89stringutils_asprintf test_crt_asprintf
#undef c89stringutils_vasprintf
#define c89stringutils_vasprintf test_crt_vasprintf
#undef c89stringutils_jasprintf
#define c89stringutils_jasprintf test_crt_jasprintf
#endif /* !CDD_CRT_BRIDGE_NO_MACROS */

#endif /* defined(_MSC_VER) */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !C_CDD_TESTS_CDD_CRT_BRIDGE_H */
