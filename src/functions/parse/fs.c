/** @brief Enable rand_s support in MSVC CRT */
#define _CRT_RAND_S
/* clang-format off */
#include "c_cdd/memory.h"
#include <stdlib.h>
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#define _CRT_RAND_S
#endif

#include "c_cdd/safe_crt_msvc.h"

#include "functions/parse/str.h"
/**
 * @file fs.c
 * @brief Implementation of filesystem utilities.
 * Includes POSIX and Windows specific implementations for recursive directory
 * walking.
 * @author Samuel Marks
 */

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "c_cdd/log.h"
#include "functions/parse/fs.h"
#include "functions/str_includes.h"

#ifdef CDD_BUILD_TESTS
extern int g_fail_io_after;
extern int g_io_calls;
#endif

/**
 * @brief Join directory and filename into a path.
 */
cdd_c_error_t fs_path_join(const char *dir, const char *name,
                           char **out_path) {
  size_t dir_len, name_len;
  char *res;

  if (!dir || !name || !out_path)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  dir_len = strlen(dir);
  name_len = strlen(name);
  res = (char *)(size_t)C_CDD_MALLOC(dir_len + 1 + name_len + 1);
  if (!res)
    return CDD_C_ERROR_MEMORY;

#if defined(_MSC_VER)
  sprintf_s(res, dir_len + 1 + name_len + 1, "%s%c%s", dir, PATH_SEP_C, name);
#else
  sprintf(res, "%s%c%s", dir, PATH_SEP_C, name);
#endif
  *out_path = res;
  return CDD_C_SUCCESS;
}

/**
 * @brief Format a temporary file path with prefix, number, and suffix.
 */
cdd_c_error_t format_tmp_filename(const char *dir, const char *prefix,
                                  unsigned long num, const char *suffix,
                                  char **out_path) {
  size_t total_len;
  const char *pfx = prefix ? prefix : "";
  const char *sfx = suffix ? suffix : "";
  char num_buf[32];
  char *res;

  if (!dir || !out_path)
    return CDD_C_ERROR_INVALID_ARGUMENT;

#if defined(_MSC_VER)
  sprintf_s(num_buf, sizeof(num_buf), "%lu", num);
#else
  sprintf(num_buf, "%lu", num);
#endif

  total_len = strlen(dir) + 1 + strlen(pfx) + strlen(num_buf) + strlen(sfx) + 1;
  res = (char *)(size_t)C_CDD_MALLOC(total_len);
  if (!res)
    return CDD_C_ERROR_MEMORY;

#if defined(_MSC_VER)
  sprintf_s(res, total_len, "%s%c%s%s%s", dir, PATH_SEP_C, pfx, num_buf, sfx);
#else
  sprintf(res, "%s%c%s%s%s", dir, PATH_SEP_C, pfx, num_buf, sfx);
#endif
  *out_path = res;
  return CDD_C_SUCCESS;
}

/**
 * @brief Helper to convert internal standard library errno to cdd_c_error_t.
 */
cdd_c_error_t errno_to_cdd_error(int err) {
  if (err == 0) return CDD_C_SUCCESS;
  if (err == ENOENT) return CDD_C_ERROR_NOT_FOUND;
  if (err == ENOMEM) return CDD_C_ERROR_MEMORY;
  if (err == EINVAL) return CDD_C_ERROR_INVALID_ARGUMENT;
  return CDD_C_ERROR_IO;
}

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#if !defined(_MSC_VER) || defined(__INTEL_COMPILER)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <fileapi.h>
#include <winnls.h>
#include <winsock2.h>
#endif
#endif
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#include "c_cddConfig.h"
/* windef.h must precede winbase.h to prevent DWORD redefinition errors */
#include "win_compat_sym.h"
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>

#include <direct.h>
#if !defined(_MSC_VER) || _MSC_VER >= 1900
#include <fileapi.h>
#endif
#include <winerror.h>
/* strtok_s is defined as a macro for strtok_r in fs.h if needed, or by system
 * headers */

/* <windows_utils> */
#ifndef strdup
#ifndef strdup
#define strdup _strdup
#endif
#endif

#ifdef PATHCCH_LIB
#include <pathcch.h>
#endif /* !PATHCCH_LIB */
#if defined(_MSC_VER) && _MSC_VER < 1900
/* #include <shlobj.h> */
#else
/* #include <shlobj_core.h> */
#endif
#elif defined(__WATCOMC__) || defined(__DOS__)
#include <direct.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
/* No dirent.h on DOS Watcom natively */
#else /* POSIX */
#include <dirent.h>
#include <fcntl.h>
#if !defined(_MSC_VER) || _MSC_VER >= 1800
#include <inttypes.h>
#endif
#include <libgen.h>
#include <sys/stat.h>
#include <sys/types.h>
#if defined(_MSC_VER)
#include <io.h>
#else
#include "c_cdd/log.h"
#include <unistd.h>
/* clang-format on */
#endif
#endif /* defined(_MSC_VER) && !defined(__INTEL_COMPILER) */

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
/* Windows specific logic or typedefs if needed */
#endif

/* <windows_utils> */

#if defined(_WIN32)
/**
 * @brief Executes the ascii to wide operation.
 */
cdd_c_error_t ascii_to_wide(const char *s, wchar_t *ws, size_t buf_cap,
                            size_t *out_len) {
  int result;
  if (s == NULL || ws == NULL || buf_cap == 0 || out_len == NULL) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  result = MultiByteToWideChar(
      /* Code Page */ CP_ACP,
      /* dwFlags */ 0,
      /* lpMultiByteStr */ s,
      /* cbMultiByte */ -1,
      /* lpWideCharStr */ ws,
      /* cchWideChar */ (int)buf_cap);

  if (result == 0) {
    /* GetLastError() could be mapped, but for now generic failure */
    return CDD_C_ERROR_IO;
  }

  *out_len = (size_t)result - 1; /* Result includes null terminator count */
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the wide to ascii operation.
 */
cdd_c_error_t wide_to_ascii(const wchar_t *ws, char *s, size_t buf_cap,
                            size_t *out_len) {
  int result;
  if (ws == NULL || s == NULL || buf_cap == 0 || out_len == NULL) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  result = WideCharToMultiByte(
      /* Code Page */ CP_ACP,
      /* dwFlags */ 0,
      /* lpWideCharStr */ ws,
      /* cchWideChar */ -1,
      /* lpMultiByteStr */ s,
      /* cbMultiByte */ (int)buf_cap,
      /* lpDefaultChar */ NULL,
      /* lpUsedDefaultChar */ NULL);

  if (result == 0) {
    return CDD_C_ERROR_IO;
  }

  *out_len = (size_t)result - 1;
  return CDD_C_SUCCESS;
}
#endif /* defined(_WIN32) */
/* </windows_utils> */

/**
 * @brief Duplicate a string.
 * Helper for cross-platform compatibility to ensure ENOMEM is handled cleanly
 * if we were wrapping it, but here we just blindly use strdup.
 * Caller checks for NULL return.
 */

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#define c_stat_func _stat32
#define IS_DIR(mode) (((mode) & _S_IFMT) == _S_IFDIR)
#else
#define c_stat_func stat
/** @brief IS_DIR definition */
#define IS_DIR(mode) S_ISDIR(mode)
#endif

/**
 * @brief Executes the fs is directory operation.
 */
cdd_c_error_t fs_is_directory(const char *path, int *out_is_dir) {
  c_stat st;
  if (!out_is_dir)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_is_dir = 0;
  if (!path)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (c_stat_func(path, &st) != 0) {
    return CDD_C_SUCCESS; /* Not an error to not exist, just not a dir */
  }
  *out_is_dir = IS_DIR(st.st_mode);
  return CDD_C_SUCCESS;
}

/**
 * @brief Retrieves the basename.
 */
cdd_c_error_t get_basename(const char *path, char **out) {
  char *_ast_strdup_0 = NULL;
  const char *start_p, *p;
  size_t len;
  char *ret;

  if (!out || !path)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (*path == '\0') {
    *out = (c_cdd_strdup(".", &_ast_strdup_0), _ast_strdup_0);
    return *out == NULL ? CDD_C_ERROR_MEMORY : CDD_C_SUCCESS;
  }

  p = path + strlen(path) - 1;
  /* Skip trailing separators */
  while (p > path && (*p == '/' || *p == '\\')) {
    p--;
  }

  /* Check if it was all separators (e.g. "///") -> returns "/" */
  if (p == path && (*p == '/' || *p == '\\')) {
    *out = (char *)(size_t)C_CDD_MALLOC(2);
    if (!*out)
      return CDD_C_ERROR_MEMORY;
    (*out)[0] = '/';
    (*out)[1] = '\0';
    return CDD_C_SUCCESS;
  }

  start_p = p;
  /* Move back to start of filename */
  while (start_p > path && *(start_p - 1) != '/' && *(start_p - 1) != '\\') {
    start_p--;
  }

  len = (size_t)(p - start_p) + 1;
  ret = (char *)(size_t)C_CDD_MALLOC(len + 1);
  if (!ret) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }

  memcpy(ret, start_p, len);
  ret[len] = '\0';

  *out = ret;
  return CDD_C_SUCCESS;
}

/**
 * @brief Retrieves the dirname.
 */
cdd_c_error_t get_dirname(const char *path, char **out) {
  char *_ast_strdup_1 = NULL;
  char *_ast_strdup_2 = NULL;
  const char *p;
  size_t len;
  char *ret;

  if (!out || !path)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (*path == '\0') {
    *out = (c_cdd_strdup(".", &_ast_strdup_1), _ast_strdup_1);
    return *out == NULL ? CDD_C_ERROR_MEMORY : CDD_C_SUCCESS;
  }

  p = path + strlen(path) - 1;
  /* Skip trailing separators */
  while (p > path && (*p == '/' || *p == '\\')) {
    p--;
  }

  /* Move back until separator found */
  while (p > path && *p != '/' && *p != '\\') {
    p--;
  }

  /* If path is like "/foo", p now points to '/'. We want to return "/" (or "\"
   * on win) */
  if (p == path) {
    if (*p == '/' || *p == '\\') {
      len = 1; /* Root */
      ret = (char *)(size_t)C_CDD_MALLOC(2);
      if (!ret)
        return CDD_C_ERROR_MEMORY;
      ret[0] = '/';
      ret[1] = '\0';
      *out = ret;
      return CDD_C_SUCCESS;
    } else {
      /* No separator found, e.g. "foo" -> "." */
      *out = (c_cdd_strdup(".", &_ast_strdup_2), _ast_strdup_2);
      return *out ? CDD_C_SUCCESS : CDD_C_ERROR_MEMORY;
    }
  } else {
    /* If we stopped at a separator, that's the end of dirname.
     * Unless it's like "C:\", but this logic treats leading slash as root
     * length 1. Strip trailing separators from the dirname, e.g. "a//" -> "a".
     */
    while (p > path && (*p == '/' || *p == '\\')) {
      p--;
    }
    len = (size_t)(p - path) + 1;
  }

  ret = (char *)(size_t)C_CDD_MALLOC(len + 1);
  if (!ret) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }

  memcpy(ret, path, len);
  ret[len] = '\0';
  *out = ret;
  return CDD_C_SUCCESS;
}

enum { READ_CHUNK_SIZE = 4096 };

/**
 * @brief Executes the fopen error from operation.
 */
cdd_c_error_t fopen_error_from(int fopen_error, FopenError_t *_out_val) {
  if (!_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  switch (fopen_error) {
  case 0: {
    *_out_val = FOPEN_OK;
    return CDD_C_SUCCESS;
  }
  case EINVAL: {
    *_out_val = FOPEN_INVALID_PARAMETER;
    return CDD_C_SUCCESS;
  }
  case EMFILE: {
    *_out_val = FOPEN_TOO_MANY_OPEN_FILES;
    return CDD_C_SUCCESS;
  }
  case ENOMEM: {
    *_out_val = FOPEN_OUT_OF_MEMORY;
    return CDD_C_SUCCESS;
  }
  case ENOENT: {
    *_out_val = FOPEN_FILE_NOT_FOUND;
    return CDD_C_SUCCESS;
  }
  case EACCES: {
    *_out_val = FOPEN_PERMISSION_DENIED;
    return CDD_C_SUCCESS;
  }
  case ERANGE: {
    *_out_val = FOPEN_FILENAME_TOO_LONG;
    return CDD_C_SUCCESS;
  }
  default: {
    *_out_val = FOPEN_UNKNOWN_ERROR;
    return CDD_C_SUCCESS;
  }
  }
}

/**
 * @brief Executes the fs write to file operation.
 */
cdd_c_error_t fs_write_to_file(const char *path, const char *content) {
  FILE *f;
  cdd_c_error_t rc = CDD_C_SUCCESS;
  int close_rc;
  int fputs_res;

  if (!path || !content)
    return CDD_C_ERROR_INVALID_ARGUMENT;

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  if (fopen_s(&f, path, "w") != 0)
    f = NULL;
#else
#if defined(_MSC_VER)
  fopen_s(&f, path, "w");
#else
#if defined(_MSC_VER)
  fopen_s(&f, path, "w");
#else
#if defined(_MSC_VER)
  fopen_s(&f, path, "w");
#else
#if defined(_MSC_VER)
  fopen_s(&f, path, "w");
#else
#if defined(_MSC_VER)
  fopen_s(&f, path, "w");
#else
  f = fopen(path, "w");
#endif
#endif
#endif
#endif
#endif
#endif

  if (!f) {
    return errno_to_cdd_error(errno);
  }

#ifdef CDD_BUILD_TESTS
  fputs_res = (g_fail_io_after == 88) ? -1 : fputs(content, f);
#else
  fputs_res = fputs(content, f);
#endif
  if (fputs_res < 0)
    rc = CDD_C_ERROR_IO;

  close_rc = fclose(f);
#ifdef CDD_BUILD_TESTS
  if (g_fail_io_after == 89) {
    close_rc = -1;
    errno = EIO;
  }
  if (g_fail_io_after == 85) {
    rc = CDD_C_ERROR_IO;
    close_rc = -1;
    errno = EIO;
  }
#endif
  if (close_rc != 0 && rc == CDD_C_SUCCESS)
    rc = errno_to_cdd_error(errno);

  return rc;
}

/**
 * @brief Executes the read to file operation.
 */
cdd_c_error_t read_to_file(const char *path, const char *mode, char **out_data,
                           size_t *out_size) {
  FILE *f = NULL;
  cdd_c_error_t internal_rc = CDD_C_SUCCESS;
  int close_rc;

  if (!path || !mode || !out_data || !out_size) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  *out_data = NULL;
  *out_size = 0;

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  {
    errno_t e = fopen_s(&f, path, mode);
    if (e != 0) {
      return errno_to_cdd_error((int)e);
    }
    if (f == NULL)
      return CDD_C_ERROR_IO;
  }
#else
#if defined(_MSC_VER)
  fopen_s(&f, path, mode);
#else
#if defined(_MSC_VER)
  fopen_s(&f, path, mode);
#else
#if defined(_MSC_VER)
  fopen_s(&f, path, mode);
#else
#if defined(_MSC_VER)
  fopen_s(&f, path, mode);
#else
#if defined(_MSC_VER)
  fopen_s(&f, path, mode);
#else
#if defined(_MSC_VER)
  fopen_s(&f, path, mode);
#else
  f = fopen(path, mode);
#endif
#endif
#endif
#endif
#endif
#endif
  if (f == NULL)
    return errno_to_cdd_error(errno);
#endif

  internal_rc = read_from_fh(f, out_data, out_size);

  close_rc = fclose(f);
#ifdef CDD_BUILD_TESTS
  if (g_fail_io_after == 86) {
    close_rc = -1;
    errno = EIO;
  }
  if (g_fail_io_after == 87) {
    internal_rc = CDD_C_ERROR_IO;
    close_rc = -1;
    errno = EIO;
  }
#endif
  /* Preserve read error if it occurred, but also check fclose error */
  if (close_rc != 0) {
    if (internal_rc == CDD_C_SUCCESS) {
      internal_rc =
          errno_to_cdd_error(errno); /* Return fclose error if read was OK */
    }
  }

  if (internal_rc != CDD_C_SUCCESS && *out_data != NULL) {
    C_CDD_FREE(*out_data);
    *out_data = NULL;
    *out_size = 0;
  }

  return internal_rc;
}

/**
 * @brief Executes the read from fh operation.
 */
cdd_c_error_t read_from_fh(FILE *fh, char **out_data, size_t *out_size) {
  char *buffer = NULL;
  size_t total_read = 0;
  size_t capacity = 0;
  size_t read_now;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  if (!fh || !out_data || !out_size)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  do {
    /* If buffer handles huge files, standard capacity doubling is fine */
    if (total_read + READ_CHUNK_SIZE + 1 > capacity) {
      size_t new_capacity = capacity == 0 ? READ_CHUNK_SIZE + 1 : capacity * 2;
      char *new_buffer = (char *)(size_t)C_CDD_REALLOC(buffer, new_capacity);
      if (!new_buffer) {
        C_CDD_FREE(buffer);
        return CDD_C_ERROR_MEMORY;
      }
      buffer = new_buffer;
      capacity = new_capacity;
    }
    read_now = fread(buffer + total_read, 1, READ_CHUNK_SIZE, fh);
    total_read += read_now;
  } while (read_now == READ_CHUNK_SIZE);

#ifdef CDD_BUILD_TESTS
  if (ferror(fh) || g_fail_io_after == 98 || g_fail_io_after == 99) {
    if (g_fail_io_after == 99)
      errno = 0;
    else if (g_fail_io_after == 98)
      errno = EIO;
#else
  if (ferror(fh)) {
#endif
    rc = errno_to_cdd_error(errno);
    /* If errno is not set by fread failure (rare but possible), default to EIO
     */
    if (rc == CDD_C_SUCCESS)
      rc = CDD_C_ERROR_IO;
    C_CDD_FREE(buffer);
    return rc;
  }

  /* Null terminate just in case text usage */
  buffer[total_read] = '\0';

  *out_data = buffer;
  *out_size = total_read;
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the cp operation.
 */
cdd_c_error_t cp(const char *dst, const char *src) {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  if (!dst || !src)
    return CDD_C_ERROR_INVALID_ARGUMENT;
#ifdef CDD_BUILD_TESTS
  if (g_fail_io_after == 66 || g_fail_io_after == 77 || g_fail_io_after == 64)
    return CDD_C_ERROR_IO;
#endif
  if (!CopyFileA(src, dst, TRUE))
    return errno_to_cdd_error((int)GetLastError());
  return CDD_C_SUCCESS;
#else
  int fd_to, fd_from;
  char buf[READ_CHUNK_SIZE];
  ssize_t nread;
  cdd_c_error_t saved_errno;
  char *out_ptr;
  ssize_t nwritten;
  cdd_c_error_t ret_val = CDD_C_SUCCESS;
  int close_res;

  if (!dst || !src)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  fd_from = open(src, O_RDONLY);
  if (fd_from < 0)
    return errno_to_cdd_error(errno);

  /* O_EXCL to fail if dest exists */
  fd_to = open(dst, O_WRONLY | O_CREAT | O_EXCL, 0666);
  if (fd_to < 0) {
    saved_errno = errno_to_cdd_error(errno);
    close(fd_from);
    return saved_errno;
  }

  while ((nread = read(fd_from, buf, sizeof(buf))) > 0) {
    out_ptr = buf;
    do {
      nwritten = write(fd_to, out_ptr, (unsigned int)nread);
#ifdef CDD_BUILD_TESTS
      if (g_fail_io_after == 66) {
        nwritten = -1;
        errno = EIO;
      }
      if (g_fail_io_after == 63) {
        nwritten = 1;
        g_fail_io_after = -1;
      }
      if (g_fail_io_after == 62) {
        nwritten = -1;
        errno = EINTR;
        g_fail_io_after = -1;
      }
#endif
      if (nwritten >= 0) {
        nread -= nwritten;
        out_ptr += nwritten;
      } else if (errno != EINTR) {
        ret_val = errno_to_cdd_error(errno);
        goto out_error;
      }
    } while (nread > 0);
  }

#ifdef CDD_BUILD_TESTS
  if (g_fail_io_after == 77) {
    nread = -1;
    errno = EIO;
  }
#endif

  if (nread < 0) {
    ret_val = errno_to_cdd_error(errno);
  }

out_error:
  if (ret_val != CDD_C_SUCCESS) {
    /* If error, try to preserve it */
    saved_errno = ret_val;
    close(fd_to);
    close(fd_from);
    return saved_errno;
  }

  close_res = close(fd_to);
#ifdef CDD_BUILD_TESTS
  if (g_fail_io_after == 64) {
    close_res = -1;
    errno = EIO;
  }
#endif
  if (close_res < 0) {
    saved_errno = errno_to_cdd_error(errno);
    close(fd_from);
    return saved_errno;
  }
  close(fd_from);
  return CDD_C_SUCCESS;
#endif
}

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#define c_stat_func _stat32
#define IS_DIR(mode) (((mode) & _S_IFMT) == _S_IFDIR)
#else
#define c_stat_func stat
/** @brief IS_DIR definition */
#define IS_DIR(mode) S_ISDIR(mode)
#endif

/**
 * @brief Executes the maybe mkdir operation.
 */
static cdd_c_error_t maybe_mkdir(const char *path) {
  c_stat st;
  int res;
  int stat_res;

#ifdef CDD_BUILD_TESTS
  if (g_fail_io_after >= 0 && ++g_io_calls >= g_fail_io_after)
    return CDD_C_ERROR_IO;
#endif

#if defined(_WIN32)
  res = _mkdir(path);
#elif defined(__WATCOMC__) || defined(__DOS__)
  res = mkdir(path);
#else
  res = mkdir(path, 0777);
#endif

#ifdef CDD_BUILD_TESTS
  if (g_fail_io_after == 55) {
    res = -1;
    errno = ENOENT;
  }
#endif

  if (res == 0)
    return CDD_C_SUCCESS;

  /* Directories already existing is not an error for recursion generally,
     but if it exists and is not a dir, it is. */
  if (errno == EEXIST) {
    stat_res = c_stat_func(path, &st);
#ifdef CDD_BUILD_TESTS
    if (g_fail_io_after == 53) {
      stat_res = -1;
      errno = EIO;
    }
#endif
    if (stat_res != 0)
      return errno_to_cdd_error(errno);
    if (!IS_DIR(st.st_mode))
      return CDD_C_ERROR_IO;
    return CDD_C_SUCCESS;
  }

  return errno_to_cdd_error(errno);
}

/**
 * @brief Executes the makedirs operation.
 */
cdd_c_error_t makedirs(const char *path) {
  char *_ast_strdup_4 = NULL;
  char *dup_path, *p;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  if (path == NULL || *path == '\0') {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  /* Check usage of Windows drive letters or root paths which don't need
   * creation */
#if defined(_MSC_VER)
  if ((strlen(path) == 1 && (path[0] == '/' || path[0] == '\\')) ||
      (strlen(path) == 2 && path[1] == ':') ||
      (strlen(path) == 3 && path[1] == ':' &&
       (path[2] == '/' || path[2] == '\\'))) {
    return CDD_C_SUCCESS;
  }
#else
  if (strlen(path) == 1 && path[0] == '/')
    return CDD_C_SUCCESS;
#endif

  dup_path = (c_cdd_strdup(path, &_ast_strdup_4), _ast_strdup_4);
  if (dup_path == NULL) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }

  p = dup_path;
#if defined(_WIN32)
  if (strlen(dup_path) >= 2 && dup_path[1] == ':') {
    p += 2;
    if (*p == '/' || *p == '\\')
      p++;
  }
#endif

  for (; *p; ++p) {
    if (*p == '/' || *p == '\\') {
      if (p == dup_path)
        continue;
      *p = '\0';
      rc = maybe_mkdir(dup_path);
      if (rc != CDD_C_SUCCESS) {
        C_CDD_FREE(dup_path);
        return rc;
      }
      *p = PATH_SEP_C;
    }
  }

  rc = maybe_mkdir(dup_path);
  C_CDD_FREE(dup_path);
  return rc;
}

/**
 * @brief Executes the makedir operation.
 */
cdd_c_error_t makedir(const char *path) {
  if (path == NULL || *path == '\0')
    return CDD_C_ERROR_INVALID_ARGUMENT;
#if defined(_WIN32)
  if (_mkdir(path) == 0)
    return CDD_C_SUCCESS;
#elif defined(__WATCOMC__) || defined(__DOS__)
  if (mkdir(path) == 0)
    return CDD_C_SUCCESS;
#else
  if (mkdir(path, 0777) == 0)
    return CDD_C_SUCCESS;
#endif
  return errno_to_cdd_error(errno);
}

/**
 * @brief Executes the tempdir operation.
 */
cdd_c_error_t tempdir(char **out_path) {
  char *_ast_strdup_5 = NULL;
  const char *env;

  if (!out_path)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  env = getenv("TMPDIR");
  if (!env || *env == '\0')
    env = getenv("TMP");
  if (!env || *env == '\0')
    env = getenv("TEMP");
  if (env && *env != '\0') {
    *out_path = (c_cdd_strdup(env, &_ast_strdup_5), _ast_strdup_5);
    return *out_path ? CDD_C_SUCCESS : CDD_C_ERROR_MEMORY;
  }

#if defined(_WIN32)
  {
    DWORD len;
    len = GetTempPathA(0, NULL);
    if (len == 0)
      return CDD_C_ERROR_IO;
    *out_path = C_CDD_MALLOC(len + 1);
    if (!*out_path)
      return CDD_C_ERROR_MEMORY;
    if (GetTempPathA(len + 1, *out_path) == 0) {
      C_CDD_FREE(*out_path);
      *out_path = NULL;
      return CDD_C_ERROR_IO;
    }
    c_cdd_str_trim_trailing_whitespace(*out_path);
    /* Remove trailing backslash if it exists, as dirname does */
    len = (DWORD)strlen(*out_path);
    if (len > 0 &&
        ((*out_path)[len - 1] == '\\' || (*out_path)[len - 1] == '/')) {
      (*out_path)[len - 1] = '\0';
    }
    return CDD_C_SUCCESS;
  }
#else
  {
#ifdef P_tmpdir
    *out_path = (c_cdd_strdup(P_tmpdir, &_ast_strdup_5), _ast_strdup_5);
#else
    *out_path = (c_cdd_strdup("/tmp", &_ast_strdup_5), _ast_strdup_5);
#endif
    return *out_path ? CDD_C_SUCCESS : CDD_C_ERROR_MEMORY;
  }
#endif
}

/**
 * @brief Executes the FilenameAndPtr cleanup operation.
 */
cdd_c_error_t FilenameAndPtr_cleanup(struct FilenameAndPtr *file) {
  if (!file)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (file->fh) {
    fclose(file->fh);
    file->fh = NULL;
  }
  if (file->filename) {
    C_CDD_FREE(file->filename);
    file->filename = NULL;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the FilenameAndPtr delete and cleanup operation.
 */
cdd_c_error_t FilenameAndPtr_delete_and_cleanup(struct FilenameAndPtr *file) {
  if (!file)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (file->filename) {
    /* Ideally we unlink before freeing memory */
    unlink(file->filename);
  }
  return FilenameAndPtr_cleanup(file);
}

/**
 * @brief Executes the mktmpfilegetnameandfile operation.
 */
cdd_c_error_t mktmpfilegetnameandfile(const char *prefix, const char *suffix,
                                      const char *mode,
                                      struct FilenameAndPtr *file) {
  unsigned char i;
  char *tmpdir_path = NULL;
  char *tmpfilename = NULL;
  cdd_c_error_t rc;
  int access_res;

  if (!file || !mode)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  file->fh = NULL;
  file->filename = NULL;

  rc = tempdir(&tmpdir_path);
  if (rc != CDD_C_SUCCESS)
    return rc;

  for (i = 9; i != 0; --i) {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
    {
      unsigned int number;
      errno_t err = rand_s(&number);
      if (err) {
        C_CDD_FREE(tmpdir_path);
        return errno_to_cdd_error((int)err);
      }
      rc = format_tmp_filename(tmpdir_path, prefix, (unsigned long)number,
                               suffix, &tmpfilename);
      if (rc != CDD_C_SUCCESS) {
        C_CDD_FREE(tmpdir_path);
        return rc;
      }
    }
#else
    {
      uint32_t number = (uint32_t)rand();
      rc = format_tmp_filename(tmpdir_path, prefix, (unsigned long)number,
                               suffix, &tmpfilename);
      if (rc != CDD_C_SUCCESS) {
        C_CDD_FREE(tmpdir_path);
        return rc;
      }
    }
#endif

    access_res = access(tmpfilename, F_OK);
#ifdef CDD_BUILD_TESTS
    if (g_fail_io_after == 44) {
      g_fail_io_after = -1;
      access_res = 0;
    }
    if (g_fail_io_after == 45) {
      access_res = 0;
    }
#endif

    if (access_res != 0) {
      /* File does not exist, try to open */
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
      errno_t err = fopen_s(&file->fh, tmpfilename, mode);
      if (err != 0 || file->fh == NULL) {
        C_CDD_FREE(tmpfilename);
        /* Try next iteration */
        continue;
      }
#else
      file->fh = fopen(tmpfilename, mode);
      if (!file->fh) {
        C_CDD_FREE(tmpfilename);
        continue;
      }
#endif
      /* Success */
      file->filename = tmpfilename;
      C_CDD_FREE(tmpdir_path);
      return CDD_C_SUCCESS;
    }
    C_CDD_FREE(tmpfilename);
  }

  C_CDD_FREE(tmpdir_path);
  return CDD_C_ERROR_IO; /* Or simple general failure */
}

#ifdef _MSC_VER
/**
 * @brief Executes the path is unc operation.
 */
cdd_c_error_t path_is_unc(const char *path, int *out_is_unc) {
  if (!out_is_unc)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (!path) {
    *out_is_unc = 0;
    return CDD_C_SUCCESS;
  }
  *out_is_unc = (strlen(path) > 2 && path[0] == '\\' && path[1] == '\\');
  return CDD_C_SUCCESS;
}
#endif /* _MSC_VER */

/* --- Directory Walking Implementation --- */

/**
 * @brief Executes the walk directory operation.
 */
cdd_c_error_t walk_directory(const char *path, fs_walk_cb cb, void *user_data) {
  char *full_path = NULL;
  c_stat st;
  cdd_c_error_t rc = CDD_C_SUCCESS;
  int stat_res;

  if (!path || !cb)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  /* Check if root exists and is directory */
  if (c_stat_func(path, &st) != 0)
    return errno_to_cdd_error(errno);

  if (!IS_DIR(st.st_mode)) {
    /* Not a directory, just call back once?
       Usually walk implies directory traversal.
       If user passed a file, we process it. */
    return cb(path, user_data);
  }

  (void)full_path;
  (void)stat_res;

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  {
    /* Windows implementation using _findfirst / _findnext */
    struct _finddata_t file_info;
    intptr_t handle;
    char *search_path = NULL;

    /* Create search path: path + "\*" or "\*.*" */
    if (asprintf(&search_path, "%s\\*", path) == -1) {
      return CDD_C_ERROR_MEMORY;
    }

    handle = _findfirst(search_path, &file_info);
    free(search_path);

#ifdef CDD_BUILD_TESTS
    if (g_fail_io_after == 33) {
      handle = -1;
      errno = EACCES;
    }
#endif

    if (handle == -1) {
      /* Empty dir or error? ENOENT means strict empty or not found. */
      if (errno == ENOENT)
        return CDD_C_SUCCESS; /* Empty usually behaves like this */
      return errno_to_cdd_error(errno);
    }

    do {
      if (strcmp(file_info.name, ".") == 0 ||
          strcmp(file_info.name, "..") == 0) {
        continue;
      }

#ifdef CDD_BUILD_TESTS
      if (g_fail_io_after == 32) {
        _findclose(handle);
        return CDD_C_ERROR_IO;
      }
#endif

      rc = fs_path_join(path, file_info.name, &full_path);
      if (rc != CDD_C_SUCCESS) {
        _findclose(handle);
        return rc;
      }

      if (file_info.attrib & _A_SUBDIR) {
        rc = walk_directory(full_path, cb, user_data);
      } else {
        rc = cb(full_path, user_data);
      }
      C_CDD_FREE(full_path);

      if (rc != CDD_C_SUCCESS) {
        _findclose(handle);
        return rc;
      }

    } while (_findnext(handle, &file_info) == 0);

    _findclose(handle);
  }
#elif defined(__WATCOMC__) || defined(__DOS__)
  return CDD_C_ERROR_SYSTEM;
#else
  {
    /* POSIX implementation using opendir / readdir */
    DIR *d;
    struct dirent *entry;
#ifdef CDD_BUILD_TESTS
    if (g_fail_io_after == 33) {
      d = NULL;
      errno = EACCES;
    } else {
      d = opendir(path);
    }
#else
    d = opendir(path);
#endif

    if (!d)
      return errno_to_cdd_error(errno);

    while ((entry = readdir(d)) != NULL) {
      if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
        continue;
      }

      rc = fs_path_join(path, entry->d_name, &full_path);
      if (rc != CDD_C_SUCCESS) {
        closedir(d);
        return rc;
      }

      /* Need to stat to determine if dir or file, d_type is not standard posix
         everywhere (though common). Safe approach is stat. */
      stat_res = c_stat_func(full_path, &st);
#ifdef CDD_BUILD_TESTS
      if (g_fail_io_after == 32) {
        stat_res = -1;
        errno = EIO;
      }
#endif
      if (stat_res == 0) {
        if (IS_DIR(st.st_mode)) {
          rc = walk_directory(full_path, cb, user_data);
        } else {
          rc = cb(full_path, user_data);
        }
      } else {
        rc = errno_to_cdd_error(errno);
      }

      C_CDD_FREE(full_path);

      if (rc != CDD_C_SUCCESS) {
        closedir(d);
        return rc;
      }
    }
    closedir(d);
  }
#endif

  return CDD_C_SUCCESS;
}
