#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fs.h"

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#define PATH_SEP "\\"
#define PATH_SEP_C '\\'
#define strtok_r strtok_s
#include <winbase.h>
#else
#define PATH_SEP "/"
#define PATH_SEP_C '/'
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#endif /* defined(_MSC_VER) && !defined(__INTEL_COMPILER) */

const char *get_basename(const char *const path) {
  /*char *pch;
  char *ans;
  pch = strtok_r(path, PATH_SEP, &ans);
  if (pch == NULL)
    ans = path;
  while (pch != NULL) {
    ans = pch;
    pch = strtok_r(NULL, PATH_SEP, &ans);
  }
  return ans;*/

  const char *last_slash;
  if (path == NULL)
    return path;

  last_slash = strrchr(path, PATH_SEP_C);

  if (last_slash != NULL) {
    return last_slash + 1;
  } else {
    return path;
  }
}

#define FILE_OK 0
#define FILE_NOT_EXIST 1
#define FILE_TOO_LARGE 2
#define FILE_READ_ERROR 3

/* originally from https://stackoverflow.com/a/54057690 */
char *c_read_file(const char *f_name, int *err, size_t *f_size,
                  const char *mode) {
  char *buffer;
  size_t length;
  FILE *f;
  size_t read_length;

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  {
    errno_t err = fopen_s(&f, f_name, mode);
    if (err != 0 || fp == NULL) {
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
  length = ftell(f);
  fseek(f, 0, SEEK_SET);

  // 1 GiB; best not to load a whole large file in one string
  if (length > 1073741824) {
    *err = FILE_TOO_LARGE;

    return NULL;
  }

  buffer = (char *)malloc(length + 1);

  if (length) {
    read_length = fread(buffer, 1, length, f);

    if (length != read_length) {
      free(buffer);
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

int cp(const char *to, const char *from) {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  return CopyFile(from, to, FALSE);
#else
  int fd_to, fd_from;
  char buf[4096];
  ssize_t nread;
  int saved_errno;

  fd_from = open(from, O_RDONLY);
  if (fd_from < 0)
    return -1;

  fd_to = open(to, O_WRONLY | O_CREAT | O_EXCL, 0666);
  if (fd_to < 0)
    goto out_error;

  while (nread = read(fd_from, buf, sizeof buf), nread > 0) {
    char *out_ptr = buf;
    ssize_t nwritten;

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
