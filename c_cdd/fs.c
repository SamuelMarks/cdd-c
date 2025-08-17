#include <ctype.h>
#include <errno.h>
#include <stdio.h>

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#define _CRT_RAND_S
#endif /* defined(_MSC_VER) && !defined(__INTEL_COMPILER) */
#include <stdlib.h>
#include <string.h>

#include "fs.h"
#include "str_includes.h"

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#include <direct.h>
#include <fileapi.h>
#include <winbase.h>
#include <winerror.h>
#define strtok_r strtok_s
#define mkdir _mkdir
#define strdup _strdup

#ifdef PATHCCH_LIB
#include <pathcch.h>
#endif /* !PATHCCH_LIB */
#include <shlobj_core.h>
#else
#include <fcntl.h>
#include <libgen.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif /* defined(_MSC_VER) && !defined(__INTEL_COMPILER) */

/* <windows_utils> */

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
/* example usage
  const size_t n = strlen(s);
  wchar_t *const wide_buffer = malloc(sizeof(*wide_buffer) * n);
  const int chars_converted = c_str_to_pwstr(s, wide_buffer, n);
  if (chars_converted != -1) {
  } else {
    vfprintf(stderr, L"Conversion failed. Error code: %lu\n", GetLastError());
    return s;
  }
*/
int ascii_to_wide(const char *const s, wchar_t *ws, const size_t len) {
  if (s == NULL || ws == NULL || len == 0)
    return -1;
  {
    const int result = MultiByteToWideChar(
        /* Code Page */ CP_ACP,
        /* dwFlags */ 0,
        /* lpMultiByteStr */ s,
        /* cbMultiByte */ -1,
        /* lpWideCharStr */ ws,
        /* cchWideChar */ (int)len);

    if (result == 0)
      return -1;

    return result - 1;
  }
}

int wide_to_ascii(const wchar_t *const ws, char *s, const size_t len) {
  if (ws == NULL || s == NULL || len == 0)
    return -1;
  {
    const int result = WideCharToMultiByte(
        /* Code Page */ CP_ACP,
        /* dwFlags */ 0,
        /* lpWideCharStr */ ws,
        /* cchWideChar */ -1,
        /* lpMultiByteStr */ s,
        /* cbMultiByte */ (int)len,
        /* lpDefaultChar */ NULL,
        /* lpUsedDefaultChar */ NULL);

    if (result == 0)
      return -1;

    return result - 1;
  }
}
#endif /* defined(_MSC_VER) && !defined(__INTEL_COMPILER) */
/* </windows_utils> */

/*
 * A safe, re-entrant implementation of basename.
 * The caller is responsible for freeing the returned string.
 * Returns NULL on allocation failure.
 */
static char *get_basename_s(const char *path) {
  const char *start_p, *p;
  char *ret;
  size_t len;

  if (!path || !*path) {
    return strdup(".");
  }

  p = path + strlen(path) - 1;
  while (p > path && (*p == '/' || *p == '\\')) {
    p--;
  }

  start_p = p;
  while (start_p > path && *(start_p - 1) != '/' && *(start_p - 1) != '\\') {
    start_p--;
  }

  len = (p - start_p) + 1;
  ret = (char *)malloc(len + 1);
  if (!ret)
    return NULL;

  memcpy(ret, start_p, len);
  ret[len] = '\0';

  return ret;
}

const char *get_basename(const char *path) {
  static char bname[PATH_MAX];
  char *s_ret = get_basename_s(path);
  if (s_ret) {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
    strncpy_s(bname, PATH_MAX, s_ret, _TRUNCATE);
#else
    strncpy(bname, s_ret, PATH_MAX - 1);
    bname[PATH_MAX - 1] = '\0';
#endif
    free(s_ret);
  } else {
    bname[0] = '.';
    bname[1] = '\0';
  }
  return bname;
}

/*
 * A safe, re-entrant implementation of dirname.
 * The input `path` is NOT modified.
 * The caller is responsible for freeing the returned string.
 * Returns NULL on allocation failure.
 */
static char *get_dirname_s(const char *path) {
  char *ret;
  const char *p;
  size_t len;

  if (!path || !*path)
    return strdup(".");

  p = path + strlen(path) - 1;
  while (p > path && (*p == '/' || *p == '\\')) {
    p--;
  }

  while (p > path && *p != '/' && *p != '\\') {
    p--;
  }

  /* If path is like "/foo", p now points to '/'. We want to return "/" */
  if (p == path && (*p == '/' || *p == '\\')) {
    len = 1;
  } else {
    len = p - path;
  }

  if (len == 0) {
    return strdup(".");
  }

  ret = (char *)malloc(len + 1);
  if (!ret)
    return NULL;

  memcpy(ret, path, len);
  ret[len] = '\0';
  return ret;
}

const char *get_dirname(char *path) {
  static char dname[PATH_MAX];
  char *s_ret = get_dirname_s(path);
  if (s_ret) {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
    strncpy_s(dname, PATH_MAX, s_ret, _TRUNCATE);
#else
    strncpy(dname, s_ret, PATH_MAX - 1);
    dname[PATH_MAX - 1] = '\0';
#endif
    free(s_ret);
  } else {
    dname[0] = '.';
    dname[1] = '\0';
  }
  return dname;
}

enum { FILE_OK, FILE_NOT_EXIST, FILE_TOO_LARGE, FILE_READ_ERROR };

#define READ_CHUNK_SIZE 4096

char *read_to_file(const char *const f_name, int *err, size_t *f_size,
                   const char *const mode) {
  char *buffer = NULL;
  size_t total_read = 0;
  size_t capacity = 0;
  FILE *f = NULL;
  size_t read_now;

  if (!f_name || !mode || !err || !f_size) {
    if (err)
      *err = FILE_NOT_EXIST;
    return NULL;
  }

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  {
    errno_t e;
    e = fopen_s(&f, f_name, mode);
    if (e != 0 || f == NULL) {
      *err = FILE_NOT_EXIST;
      return NULL;
    }
  }
#else
  f = fopen(f_name, mode);
  if (f == NULL) {
    *err = FILE_NOT_EXIST;
    return NULL;
  }
#endif

  do {
    if (total_read + READ_CHUNK_SIZE + 1 > capacity) {
      size_t new_capacity = capacity == 0 ? READ_CHUNK_SIZE + 1 : capacity * 2;
      char *new_buffer = (char *)realloc(buffer, new_capacity);
      if (!new_buffer) {
        free(buffer);
        fclose(f);
        *err = FILE_READ_ERROR; /* Represents ENOMEM */
        return NULL;
      }
      buffer = new_buffer;
      capacity = new_capacity;
    }
    read_now = fread(buffer + total_read, 1, READ_CHUNK_SIZE, f);
    total_read += read_now;
  } while (read_now == READ_CHUNK_SIZE);

  if (ferror(f)) {
    free(buffer);
    fclose(f);
    *err = FILE_READ_ERROR;
    return NULL;
  }

  fclose(f);
  *err = FILE_OK;
  buffer[total_read] = '\0';
  *f_size = total_read;
  return buffer;
}

int cp(const char *const to, const char *const from) {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  return CopyFile(from, to, TRUE) ? 0 : -1;
#else
  int fd_to, fd_from;
  char buf[4096];
  ssize_t nread;
  int saved_errno;
  char *out_ptr;
  ssize_t nwritten;

  fd_from = open(from, O_RDONLY);
  if (fd_from < 0)
    return -1;

  fd_to = open(to, O_WRONLY | O_CREAT | O_EXCL, 0666);
  if (fd_to < 0)
    goto out_error;

  while ((nread = read(fd_from, buf, sizeof buf)) > 0) {
    out_ptr = buf;
    do {
      nwritten = write(fd_to, out_ptr, nread);
      if (nwritten >= 0) {
        nread -= nwritten;
        out_ptr += nwritten;
      } else if (errno != EINTR) {
        goto out_error;
      }
    } while (nread > 0);
  }

  if (nread == 0) {
    if (close(fd_to) < 0) {
      fd_to = -1;
      goto out_error;
    }
    close(fd_from);
    return EXIT_SUCCESS;
  }

out_error:
  saved_errno = errno;
  close(fd_from);
  if (fd_to >= 0)
    close(fd_to);
  errno = saved_errno;
  return EXIT_FAILURE;
#endif
}

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#define c_stat _stat
#define IS_DIR(mode) (((mode) & _S_IFMT) == _S_IFDIR)
#else
#define c_stat stat
#define IS_DIR(mode) S_ISDIR(mode)
#endif

/*
 * Make a directory, return 0 for success.
 * It's okay if the directory already exists.
 * It's an error if the path exists but is not a directory.
 */
static int maybe_mkdir(const char *const path) {
  struct c_stat st;
  int res;

#if defined(_MSC_VER)
  res = _mkdir(path);
#else
  res = mkdir(path, 0777);
#endif

  if (res == 0)
    return 0;

  if (errno != EEXIST)
    return -1;

  if (c_stat(path, &st) != 0)
    return -1;

  if (!IS_DIR(st.st_mode)) {
    errno = ENOTDIR;
    return -1;
  }
  return 0;
}

int makedirs(const char *const path) {
  char *dup_path, *p;

  if (path == NULL || *path == '\0') {
    errno = EINVAL;
    return EXIT_FAILURE;
  }
#if defined(_MSC_VER)
  if ((strlen(path) == 1 && (path[0] == '/' || path[0] == '\\')) ||
      (strlen(path) == 2 && path[1] == ':') ||
      (strlen(path) == 3 && path[1] == ':' &&
       (path[2] == '/' || path[2] == '\\'))) {
    return EXIT_SUCCESS;
  }
#else
  if (strlen(path) == 1 && path[0] == '/')
    return EXIT_SUCCESS;
#endif

  dup_path = strdup(path);
  if (dup_path == NULL)
    return EXIT_FAILURE;

  for (p = dup_path; *p; ++p) {
    if (*p == '/' || *p == '\\') {
      if (p == dup_path)
        continue;
      *p = '\0';
      if (maybe_mkdir(dup_path) != 0) {
        free(dup_path);
        return EXIT_FAILURE;
      }
      *p = PATH_SEP_C;
    }
  }

  if (maybe_mkdir(dup_path) != 0) {
    free(dup_path);
    return EXIT_FAILURE;
  }

  free(dup_path);
  return EXIT_SUCCESS;
}

int makedir(const char *const p) {
  if (p == NULL || *p == '\0')
    return EXIT_FAILURE;
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  return _mkdir(p) == 0
#else
  return mkdir(p, 0777) == 0
#endif
             ? EXIT_SUCCESS
             : EXIT_FAILURE;
}

int tempdir(const char **tmpdir) {
  char pathname[L_tmpnam + 1];
  char *ptr;

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  errno_t err = tmpnam_s(pathname, L_tmpnam_s);
  if (err)
    return err;
  *tmpdir = get_dirname(strdup(pathname));
  return EXIT_SUCCESS;
#else
  ptr = tmpnam(pathname);
#endif

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  *tmpdir = get_dirname(ptr);
#else
  *tmpdir = dirname(ptr);
#endif
  return EXIT_SUCCESS;
}

int mktmpfilegetnameandfile(const char *prefix, const char *suffix,
                            char const *mode, const char **temp_filename,
                            FILE **temp_fh) {
  size_t i;
  const char *tmpdir;
  int rc = tempdir(&tmpdir);
  char *tmpfilename;
  if (rc != 0)
    return rc;
  for (i = 9; i != 0; --i) {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
    {
      unsigned int number;
      errno_t err = rand_s(&number);
      if (err)
        return err;
      asprintf(&tmpfilename, "%s%c%s%u%s", tmpdir, PATH_SEP_C, prefix, number,
               suffix);
    }
#else
    {
      uint32_t number = arc4random();
      asprintf(&tmpfilename, "%s%c%s%" PRIu32 "%s", tmpdir, PATH_SEP_C, prefix,
               number, suffix);
    }
#endif
    if (access(tmpfilename, F_OK) != 0) {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
      {
        errno_t err = fopen_s(temp_fh, tmpfilename, mode);
        if (err != 0 || temp_fh == NULL) {
          fprintf(stderr, "Failed to open %s\n", tmpfilename);
          free(*temp_fh);
          return EXIT_FAILURE;
        }
      }
#else
      *temp_fh = fopen(tmpfilename, mode);
      if (!*temp_fh) {
        fprintf(stderr, "Failed to open %s", tmpfilename);
        free(*temp_fh);
        return EXIT_FAILURE;
      }
#endif
      *temp_filename = tmpfilename;
      return EXIT_SUCCESS;
    }
    free(tmpfilename);
  }
  return EXIT_FAILURE;
}

#ifdef _MSC_VER
int path_is_unc(const char *path) {
  return strlen(path) > 2 && path[0] == '\\' && path[1] == '\\';
}
#endif /* _MSC_VER */
