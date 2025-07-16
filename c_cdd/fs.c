#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fs.h"

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#include <direct.h>
#include <fileapi.h>
#include <winbase.h>
#define strtok_r strtok_s
#define mkdir _mkdir

#ifdef PATHCCH_LIB
#include <pathcch.h>
#endif /* !PATHCCH_LIB */

#else
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <sys/stat.h>
#include <sys/types.h>
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

const char *get_dirname(char *path) {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#ifdef PATHCCH_LIB
  const size_t n = strlen(path);

  wchar_t *wtext = malloc(sizeof wtext * (n + 1));

  size_t outSize;
  LPWSTR ptr = NULL;
  HRESULT rc;
  mbstowcs_s(&outSize, wtext, n + 1, path, n - 1);

  ptr = wtext;
  rc = PathCchRemoveFileSpec(ptr, n);
  free(ptr);
  free(wtext);
  if (rc != S_OK)
    return path;
  return NULL;
#else
  char *dir;
  size_t len;
  char *p;

  if (path == NULL || *path == '\0') {
    dir = (char *)malloc(2);
    if (dir != NULL)
      strcpy_s(dir, 2, ".");
    return dir;
  }

  len = strlen(path);
  dir = (char *)malloc(len + 1);
  if (dir == NULL)
    return NULL;

  /* Use strcpy_s for safer copy */
  strcpy_s(dir, len + 1, path);

  /* Strip trailing slashes */
  while (len > 1 && (dir[len - 1] == '\\' || dir[len - 1] == '/')) {
    dir[len - 1] = '\0';
    len--;
  }

  /* Find last directory separator */
  p = strrchr(dir, PATH_SEP_C);
  if (p == NULL)
    p = dir;

  if (p == NULL) {
    free(dir);
    dir = (char *)malloc(2);
    if (dir)
      strcpy_s(dir, 2, ".");
    return dir;
  }

  /* If root like "C:\" or "\\" */
  if (p == dir)
    p[1] = '\0';
  else
    *p = '\0';

  return dir;
#endif /* PATHCCH_LIB */
#else
  return dirname(path);
#endif
}

enum { FILE_OK, FILE_NOT_EXIST, FILE_TOO_LARGE, FILE_READ_ERROR };

/* originally from https://stackoverflow.com/a/54057690 */
char *c_read_file(const char *const f_name, int *err, size_t *f_size,
                  const char *const mode) {
  char *buffer;
  size_t length;
  FILE *f;
  size_t read_length;

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
  length = ftell(f);
  fseek(f, 0, SEEK_SET);

  /* 1 GiB; best not to load a whole large file in one string */
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

int cp(const char *const to, const char *const from) {
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

int makedir(const char *const p) {
  int rc = EXIT_SUCCESS;
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  if (mkdir(p) == -1) {
#else
  if (mkdir(p, S_IRWXU | S_IRWXG | S_IRWXO) == -1) {
    fprintf(stderr, "Error: %s\n", strerror(errno));
#endif
    rc = EXIT_FAILURE;
  }
  return rc;
}

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
int makedirs(const char *const p) {
  if (CreateDirectoryA(p, NULL)) {
    return EXIT_SUCCESS;
  } else {
    return EXIT_FAILURE;
  }
}
#else
/* else follows (mostly)
 * https://gist.github.com/JonathonReinhart/8c0d90191c38af2dcadb102c4e202950 */
/* Make a directory; already existing dir okay */
static int maybe_mkdir(const char *const path, const mode_t mode) {
  struct stat st;
  errno = 0;

  /* Try to make the directory */
  if (mkdir(path, mode) == 0)
    return 0;

  /* If it fails for any reason but EEXIST, fail */
  if (errno != EEXIST)
    return -1;

  /* Check if the existing path is a directory */
  if (stat(path, &st) != 0)
    return -1;

  /* If not, fail with ENOTDIR */
  if (!S_ISDIR(st.st_mode)) {
    errno = ENOTDIR;
    return -1;
  }

  errno = 0;
  return 0;
}

int makedirs(const char *const path) {
  /* Adapted from http://stackoverflow.com/a/2336245/119527 */
  char *_path = NULL;
  char *p;
  int result = -1;
  mode_t mode = 0777;

  errno = 0;

  _path = strdup(path);
  if (_path == NULL)
    goto out;

  /* Iterate the string */
  for (p = _path + 1; *p; p++) {
    if (*p == '/') {
      /* Temporarily truncate */
      *p = '\0';

      if (maybe_mkdir(_path, mode) != 0)
        goto out;

      *p = '/';
    }
  }

  if (maybe_mkdir(_path, mode) != 0)
    goto out;

  result = 0;

out:
  free(_path);
  return result;
}
#endif
