#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fs.h"

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

const char *get_basename(const char *const path) {
  /* BSD licensed OpenBSD implementation from lib/libc/gen/basename.c
   * @ ff5bc0. */
  static char bname[PATH_MAX];
  size_t len;
  const char *endp, *startp;

  if (path == NULL || *path == '\0') {
    bname[0] = '.';
    bname[1] = '\0';
    return bname;
  }

  endp = path + strlen(path) - 1;
  while (endp > path && (*endp == '/' || *endp == '\\'))
    endp--;

  if (endp == path && (*endp == '/' || *endp == '\\')) {
    bname[0] = *endp;
    bname[1] = '\0';
    return bname;
  }

  startp = endp;
  while (startp > path && (*(startp - 1) != '/' && *(startp - 1) != '\\'))
    startp--;

  len = endp - startp + 1;
  if (len >= sizeof(bname)) {
    errno = ENAMETOOLONG;
    return NULL;
  }
  memcpy(bname, startp, len);
  bname[len] = '\0';
  return bname;
}

const char *get_dirname(char *path) {
  static char dot[] = ".";
  size_t len;
  char *p;

  if (path == NULL || *path == '\0') {
    return dot;
  }
  len = strlen(path);

  /* Strip trailing slashes */
  p = path + len - 1;
  while (p >= path && (*p == '/' || *p == '\\')) {
    p--;
  }
  *(p + 1) = '\0';
  len = p - path + 1;

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  if (len == 2 && path[1] == ':') { /* "C:" */
    return path;
  }
  if (path_is_unc(path)) {
    char *p_sep = path + 2;
    int sep_count = 0;
    while (*p_sep) {
      if (*p_sep == '\\' || *p_sep == '/') {
        sep_count++;
      }
      p_sep++;
    }
    if (sep_count < 2) {
      return path;
    }
  }
#endif

  /* Find the last separator */
  p = path + len - 1;
  while (p >= path && *p != '/' && *p != '\\') {
    p--;
  }

  if (p < path) {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
    if (len > 1 && path[1] == ':') {
      path[2] = '\0';
      return path;
    }
#endif
    return dot;
  }

  if (p == path) { /* "/foo" -> "/" */
    *(p + 1) = '\0';
    return path;
  }

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  if (p == path + 2 && path[1] == ':') { /* "C:\foo" -> "C:\" */
    *(p + 1) = '\0';
    return path;
  }
#endif

  *p = '\0';
  return path;
}

enum { FILE_OK, FILE_NOT_EXIST, FILE_TOO_LARGE, FILE_READ_ERROR };

char *c_read_file(const char *const f_name, int *err, size_t *f_size,
                  const char *const mode) {
  char *buffer;
  size_t length;
  FILE *f;
  size_t read_length;
  long length_long;

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
  if (!f) {
    *err = FILE_NOT_EXIST;
    return NULL;
  }
#endif
  fseek(f, 0, SEEK_END);
  length_long = ftell(f);
  if (length_long < 0) {
    *err = FILE_READ_ERROR;
    fclose(f);
    return NULL;
  }
  length = (size_t)length_long;
  fseek(f, 0, SEEK_SET);

  if (length > 1073741824) {
    *err = FILE_TOO_LARGE;
    fclose(f);
    return NULL;
  }

  buffer = (char *)malloc(length + 1);
  if (!buffer) {
    *err = FILE_READ_ERROR;
    fclose(f);
    return NULL;
  }

  if (length > 0) {
    read_length = fread(buffer, 1, length, f);
    if (length != read_length) {
      free(buffer);
      fclose(f);
      *err = FILE_READ_ERROR;
      return NULL;
    }
  }

  fclose(f);
  *err = FILE_OK;
  buffer[length] = '\0';
  *f_size = length;
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

#ifdef _MSC_VER
int path_is_unc(const char *path) {
  return strlen(path) > 2 && path[0] == '\\' && path[1] == '\\';
}
#endif /* _MSC_VER */
