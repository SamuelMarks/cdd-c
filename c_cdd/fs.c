#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fs.h"

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#include <direct.h>
#include <fileapi.h>
#include <winbase.h>
#define SAFE_STRCPY(dest, size, src) strcpy_s(dest, size, src)
#define SAFE_STRDUP(src) _strdup(src)
#else
#include <fcntl.h>
#include <libgen.h>
#include <sys/types.h>
#include <unistd.h>
#define SAFE_STRCPY(dest, size, src) strcpy(dest, src)
#define SAFE_STRDUP(src) strdup(src)
#endif

const char *get_basename(const char *const path) {
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
  static char dname[PATH_MAX];
  char *p;
  size_t len;

  if (path == NULL || *path == '\0') {
    SAFE_STRCPY(dname, sizeof(dname), ".");
    return dname;
  }

  len = strlen(path);
  if (len >= sizeof(dname)) {
    errno = ENAMETOOLONG;
    return NULL;
  }
  SAFE_STRCPY(dname, sizeof(dname), path);
  p = dname + len;

  while (p > dname && (*(p - 1) == '/' || *(p - 1) == '\\')) {
    p--;
  }
  *p = '\0';

  if (*dname == '\0') { /* Path was all slashes e.g. "///" */
    SAFE_STRCPY(dname, sizeof(dname), PATH_SEP);
    return dname;
  }

  /* Find last separator */
  p = strrchr(dname, '/');
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  {
    char *p2 = strrchr(dname, '\\');
    if (p < p2)
      p = p2;
  }
#endif

  if (p == NULL) { /* No separator */
    SAFE_STRCPY(dname, sizeof(dname), ".");
  } else if (p == dname) { /* Root like /foo */
    dname[1] = '\0';
  }
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  else if (p == dname + 2 && dname[1] == ':') { /* Drive root like C:\foo */
    *(p + 1) = '\0'; /* keep the backslash */
  }
#endif
  else { /* Normal case */
    *p = '\0';
  }

  return dname;
}

enum { FILE_OK, FILE_NOT_EXIST, FILE_TOO_LARGE, FILE_READ_ERROR };

char *c_read_file(const char *const f_name, int *err, size_t *f_size,
                  const char *const mode) {
  char *buffer;
  size_t length;
  FILE *f;
  size_t read_length;

  if (!f_name || !mode || !err || !f_size) {
    if (err)
      *err = FILE_NOT_EXIST;
    return NULL;
  }
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  {
    errno_t e = fopen_s(&f, f_name, mode);
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
  length = (size_t)ftell(f);
  fseek(f, 0, SEEK_SET);

  if (length > 1073741824) {
    *err = FILE_TOO_LARGE;
    fclose(f);
    return NULL;
  }

  buffer = (char *)malloc(length + 1);
  if (!buffer) {
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

  fd_from = open(from, O_RDONLY);
  if (fd_from < 0)
    return -1;

  fd_to = open(to, O_WRONLY | O_CREAT | O_EXCL, 0666);
  if (fd_to < 0)
    goto out_error;

  while ((nread = read(fd_from, buf, sizeof buf)) > 0) {
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

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#define c_stat _stat
#define IS_DIR(mode) (((mode)&_S_IFMT) == _S_IFDIR)
#else
#define c_stat stat
#define IS_DIR(mode) S_ISDIR(mode)
#endif

/* Make a directory; already existing dir okay */
static int maybe_mkdir(const char *const path) {
  struct c_stat st;
  int res;

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  res = _mkdir(path);
#else
  res = mkdir(path, 0777);
#endif

  if (res == 0)
    return 0;

  if (errno != EEXIST) {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
    if (errno == EINVAL && strlen(path) == 2 && path[1] == ':') {
      if (c_stat(path, &st) == 0 && IS_DIR(st.st_mode)) {
        return 0;
      }
    }
#endif
    return -1;
  }

  if (c_stat(path, &st) != 0)
    return -1;

  if (!IS_DIR(st.st_mode)) {
    errno = ENOTDIR;
    return -1;
  }

  return 0;
}

int makedirs(const char *const path) {
  char *_path = NULL;
  char *p;
  int result = -1;

  if (path == NULL || *path == '\0')
    return EXIT_FAILURE;

  errno = 0;

  _path = SAFE_STRDUP(path);
  if (_path == NULL)
    goto out;

  for (p = _path; *p; p++) {
    if (*p == '/' || *p == '\\') {
      if (p == _path)
        continue;
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
      if (p > _path && *(p - 1) == ':')
        continue;
#endif
      *p = '\0';
      if (maybe_mkdir(_path) != 0) {
        *p = PATH_SEP_C;
        goto out;
      }
      *p = PATH_SEP_C;
    }
  }

  if (maybe_mkdir(_path) != 0)
    goto out;

  result = 0;

out:
  free(_path);
  return result;
}

int makedir(const char *const p) {
  if (p == NULL || *p == '\0')
    return EXIT_FAILURE;
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  return _mkdir(p) == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
#else
  return mkdir(p, 0777) == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
#endif
}
