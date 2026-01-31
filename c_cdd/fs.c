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
#include <stdlib.h>
#include <string.h>

#include "fs.h"
#include "str_includes.h"

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#define _CRT_RAND_S
#include <direct.h>
#include <fileapi.h>
#include <winbase.h>
#include <winerror.h>
/* strtok_s is defined as a macro for strtok_r in fs.h if needed, or by system
 * headers */
#define mkdir _mkdir
#ifndef strdup
#define strdup _strdup
#endif

#ifdef PATHCCH_LIB
#include <pathcch.h>
#endif /* !PATHCCH_LIB */
#include <shlobj_core.h>
#else /* Not MSVC */
#include <dirent.h>
#include <fcntl.h>
#include <inttypes.h>
#include <libgen.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#endif /* defined(_MSC_VER) && !defined(__INTEL_COMPILER) */

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#include <inttypes.h>
#endif

/* <windows_utils> */

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
int ascii_to_wide(const char *const s, wchar_t *ws, const size_t buf_cap,
                  size_t *out_len) {
  int result;
  if (s == NULL || ws == NULL || buf_cap == 0 || out_len == NULL) {
    return EINVAL;
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
    return EIO;
  }

  *out_len = (size_t)result - 1; /* Result includes null terminator count */
  return 0;
}

int wide_to_ascii(const wchar_t *const ws, char *s, const size_t buf_cap,
                  size_t *out_len) {
  int result;
  if (ws == NULL || s == NULL || buf_cap == 0 || out_len == NULL) {
    return EINVAL;
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
    return EIO;
  }

  *out_len = (size_t)result - 1;
  return 0;
}
#endif /* defined(_MSC_VER) && !defined(__INTEL_COMPILER) */
/* </windows_utils> */

/**
 * @brief Duplicate a string.
 * Helper for cross-platform compatibility to ensure ENOMEM is handled cleanly
 * if we were wrapping it, but here we just blindly use strdup.
 * Caller checks for NULL return.
 */
static char *c_cdd_strdup(const char *s) {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  return _strdup(s);
#else
  return strdup(s);
#endif
}

int get_basename(const char *path, char **out) {
  const char *start_p, *p;
  size_t len;
  char *ret;

  if (out == NULL)
    return EINVAL;

  if (!path || !*path) {
    *out = c_cdd_strdup(".");
    return *out == NULL ? ENOMEM : 0;
  }

  p = path + strlen(path) - 1;
  /* Skip trailing separators */
  while (p > path && (*p == '/' || *p == '\\')) {
    p--;
  }

  /* Check if it was all separators (e.g. "///") -> returns "/" */
  if (p == path && (*p == '/' || *p == '\\')) {
    *out = (char *)malloc(2);
    if (!*out)
      return ENOMEM;
    (*out)[0] = PATH_SEP_C;
    (*out)[1] = '\0';
    return 0;
  }

  start_p = p;
  /* Move back to start of filename */
  while (start_p > path && *(start_p - 1) != '/' && *(start_p - 1) != '\\') {
    start_p--;
  }

  len = (size_t)(p - start_p) + 1;
  ret = (char *)malloc(len + 1);
  if (!ret)
    return ENOMEM;

  memcpy(ret, start_p, len);
  ret[len] = '\0';

  *out = ret;
  return 0;
}

int get_dirname(const char *path, char **out) {
  const char *p;
  size_t len;
  char *ret;

  if (out == NULL)
    return EINVAL;

  if (!path || !*path) {
    *out = c_cdd_strdup(".");
    return *out == NULL ? ENOMEM : 0;
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
    } else {
      /* No separator found, e.g. "foo" -> "." */
      *out = c_cdd_strdup(".");
      return *out ? 0 : ENOMEM;
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

  if (len == 0) {
    *out = c_cdd_strdup(".");
    return *out ? 0 : ENOMEM;
  }

  ret = (char *)malloc(len + 1);
  if (!ret)
    return ENOMEM;

  memcpy(ret, path, len);
  ret[len] = '\0';
  *out = ret;
  return 0;
}

enum { READ_CHUNK_SIZE = 4096 };

enum FopenError fopen_error_from(int fopen_error) {
  switch (fopen_error) {
  case 0:
    return FOPEN_OK;
  case EINVAL:
    return FOPEN_INVALID_PARAMETER;
  case EMFILE:
    return FOPEN_TOO_MANY_OPEN_FILES;
  case ENOMEM:
    return FOPEN_OUT_OF_MEMORY;
  case ENOENT:
    return FOPEN_FILE_NOT_FOUND;
  case EACCES:
    return FOPEN_PERMISSION_DENIED;
  case ERANGE:
    return FOPEN_FILENAME_TOO_LONG;
  default:
    return FOPEN_UNKNOWN_ERROR;
  }
}

int read_to_file(const char *path, const char *mode, char **out_data,
                 size_t *out_size) {
  FILE *f = NULL;
  int internal_rc = 0;

  if (!path || !mode || !out_data || !out_size)
    return EINVAL;

  *out_data = NULL;
  *out_size = 0;

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  {
    errno_t e = fopen_s(&f, path, mode);
    if (e != 0)
      return (int)e;
    if (f == NULL)
      return FOPEN_UNKNOWN_ERROR;
  }
#else
  f = fopen(path, mode);
  if (f == NULL)
    return errno;
#endif

  internal_rc = read_from_fh(f, out_data, out_size);

  /* Preserve read error if it occurred, but also check fclose error */
  if (fclose(f) != 0) {
    if (internal_rc == 0) {
      internal_rc = errno; /* Return fclose error if read was OK */
    }
  }

  return internal_rc;
}

int read_from_fh(FILE *fh, char **out_data, size_t *out_size) {
  char *buffer = NULL;
  size_t total_read = 0;
  size_t capacity = 0;
  size_t read_now;
  int rc = 0;

  if (!fh || !out_data || !out_size)
    return EINVAL;

  do {
    /* If buffer handles huge files, standard capacity doubling is fine */
    if (total_read + READ_CHUNK_SIZE + 1 > capacity) {
      size_t new_capacity = capacity == 0 ? READ_CHUNK_SIZE + 1 : capacity * 2;
      char *new_buffer = (char *)realloc(buffer, new_capacity);
      if (!new_buffer) {
        free(buffer);
        return ENOMEM;
      }
      buffer = new_buffer;
      capacity = new_capacity;
    }
    read_now = fread(buffer + total_read, 1, READ_CHUNK_SIZE, fh);
    total_read += read_now;
  } while (read_now == READ_CHUNK_SIZE);

  if (ferror(fh)) {
    rc = errno;
    /* If errno is not set by fread failure (rare but possible), default to EIO
     */
    if (rc == 0)
      rc = EIO;
    free(buffer);
    return rc;
  }

  /* Null terminate just in case text usage */
  if (buffer) {
    buffer[total_read] = '\0';
  } else {
    /* Empty file case, allocate distinct empty string */
    buffer = (char *)malloc(1);
    if (!buffer)
      return ENOMEM;
    buffer[0] = '\0';
  }

  *out_data = buffer;
  *out_size = total_read;
  return 0;
}

int cp(const char *dst, const char *src) {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  if (!CopyFile(src, dst, TRUE))
    return (int)GetLastError(); /* Return Windows error code as int */
  return 0;
#else
  int fd_to, fd_from;
  char buf[READ_CHUNK_SIZE];
  ssize_t nread;
  int saved_errno;
  char *out_ptr;
  ssize_t nwritten;
  int ret_val = 0;

  fd_from = open(src, O_RDONLY);
  if (fd_from < 0)
    return errno;

  /* O_EXCL to fail if dest exists */
  fd_to = open(dst, O_WRONLY | O_CREAT | O_EXCL, 0666);
  if (fd_to < 0) {
    saved_errno = errno;
    close(fd_from);
    return saved_errno;
  }

  while ((nread = read(fd_from, buf, sizeof(buf))) > 0) {
    out_ptr = buf;
    do {
      nwritten = write(fd_to, out_ptr, nread);
      if (nwritten >= 0) {
        nread -= nwritten;
        out_ptr += nwritten;
      } else if (errno != EINTR) {
        ret_val = errno;
        goto out_error;
      }
    } while (nread > 0);
  }

  if (nread < 0) {
    ret_val = errno;
  }

out_error:
  if (ret_val != 0) {
    /* If error, try to preserve it */
    saved_errno = ret_val;
    close(fd_to);
    close(fd_from);
    return saved_errno;
  }

  if (close(fd_to) < 0) {
    saved_errno = errno;
    close(fd_from);
    return saved_errno;
  }
  close(fd_from);
  return 0;
#endif
}

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#define c_stat_func _stat
#define IS_DIR(mode) (((mode) & _S_IFMT) == _S_IFDIR)
#else
#define c_stat_func stat
#define IS_DIR(mode) S_ISDIR(mode)
#endif

static int maybe_mkdir(const char *const path) {
  c_stat st;
  int res;

#if defined(_MSC_VER)
  res = _mkdir(path);
#else
  res = mkdir(path, 0777);
#endif

  if (res == 0)
    return 0;

  /* Directories already existing is not an error for recursion generally,
     but if it exists and is not a dir, it is. */
  if (errno == EEXIST) {
    if (c_stat_func(path, &st) != 0)
      return errno;
    if (!IS_DIR(st.st_mode))
      return ENOTDIR;
    return 0;
  }

  return errno;
}

int makedirs(const char *path) {
  char *dup_path, *p;
  int rc = 0;

  if (path == NULL || *path == '\0') {
    return EINVAL;
  }

  /* Check usage of Windows drive letters or root paths which don't need
   * creation */
#if defined(_MSC_VER)
  if ((strlen(path) == 1 && (path[0] == '/' || path[0] == '\\')) ||
      (strlen(path) == 2 && path[1] == ':') ||
      (strlen(path) == 3 && path[1] == ':' &&
       (path[2] == '/' || path[2] == '\\'))) {
    return 0;
  }
#else
  if (strlen(path) == 1 && path[0] == '/')
    return 0;
#endif

  dup_path = c_cdd_strdup(path);
  if (dup_path == NULL)
    return ENOMEM;

  for (p = dup_path; *p; ++p) {
    if (*p == '/' || *p == '\\') {
      if (p == dup_path)
        continue;
      *p = '\0';
      rc = maybe_mkdir(dup_path);
      if (rc != 0) {
        free(dup_path);
        return rc;
      }
      *p = PATH_SEP_C;
    }
  }

  rc = maybe_mkdir(dup_path);
  free(dup_path);
  return rc;
}

int makedir(const char *path) {
  if (path == NULL || *path == '\0')
    return EINVAL;
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  if (_mkdir(path) == 0)
    return 0;
#else
  if (mkdir(path, 0777) == 0)
    return 0;
#endif
  return errno;
}

int tempdir(char **out_path) {
  char pathname[L_tmpnam + 1];
  char *ptr;

  if (!out_path)
    return EINVAL;

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  {
    errno_t err = tmpnam_s(pathname, L_tmpnam_s);
    if (err)
      return (int)err;
    ptr = pathname;
  }
#else
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif /* defined(__GNUC__) || defined(__clang__) */
  ptr = tmpnam(pathname);
#if defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif /* defined(__GNUC__) || defined(__clang__) */
  if (ptr == NULL)
    return errno ? errno : EIO;
#endif

  /* get_dirname allocates now, assuming tempfile names are paths */
  return get_dirname(ptr, out_path);
}

void FilenameAndPtr_cleanup(struct FilenameAndPtr *file) {
  if (file == NULL)
    return;
  if (file->fh) {
    fclose(file->fh);
    file->fh = NULL;
  }
  if (file->filename) {
    free(file->filename);
    file->filename = NULL;
  }
}

void FilenameAndPtr_delete_and_cleanup(struct FilenameAndPtr *file) {
  if (file == NULL)
    return;
  if (file->filename) {
    /* Ideally we unlink before freeing memory */
    unlink(file->filename);
  }
  FilenameAndPtr_cleanup(file);
}

int mktmpfilegetnameandfile(const char *prefix, const char *suffix,
                            const char *mode, struct FilenameAndPtr *file) {
  uint8_t i;
  char *tmpdir_path = NULL;
  char *tmpfilename = NULL;
  int rc;

  if (!file)
    return EINVAL;

  rc = tempdir(&tmpdir_path);
  if (rc != 0)
    return rc;

  for (i = 9; i != 0; --i) {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
    {
      unsigned int number;
      errno_t err = rand_s(&number);
      if (err) {
        free(tmpdir_path);
        return err;
      }
      if (asprintf(&tmpfilename, "%s%c%s%u%s", tmpdir_path, PATH_SEP_C,
                   prefix == NULL ? "" : prefix, number,
                   suffix == NULL ? "" : suffix) == -1) {
        free(tmpdir_path);
        return ENOMEM;
      }
    }
#else
    {
      /* Using arc4random on random-equipped systems, or simple rand if generic.
       * The assumption is arc4random is available in this env based on previous
       * context. */
      uint32_t number = arc4random();
      if (asprintf(&tmpfilename, "%s%c%s%" PRIu32 "%s", tmpdir_path, PATH_SEP_C,
                   prefix == NULL ? "" : prefix, number,
                   suffix == NULL ? "" : suffix) == -1) {
        free(tmpdir_path);
        return ENOMEM;
      }
    }
#endif

    if (access(tmpfilename, F_OK) != 0) {
      /* File does not exist, try to open */
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
      errno_t err = fopen_s(&file->fh, tmpfilename, mode);
      if (err != 0 || file->fh == NULL) {
        free(tmpfilename);
        /* Try next iteration */
        continue;
      }
#else
      file->fh = fopen(tmpfilename, mode);
      if (!file->fh) {
        free(tmpfilename);
        continue;
      }
#endif
      /* Success */
      file->filename = tmpfilename;
      free(tmpdir_path);
      return 0;
    }
    free(tmpfilename);
  }

  free(tmpdir_path);
  return EEXIST; /* Or simple general failure */
}

#ifdef _MSC_VER
int path_is_unc(const char *path) {
  return strlen(path) > 2 && path[0] == '\\' && path[1] == '\\';
}
#endif /* _MSC_VER */

/* --- Directory Walking Implementation --- */

int walk_directory(const char *path, fs_walk_cb cb, void *user_data) {
  char *full_path = NULL;
  c_stat st;
  int rc = 0;

  if (!path || !cb)
    return EINVAL;

  /* Check if root exists and is directory */
  if (c_stat_func(path, &st) != 0)
    return errno;

  if (!IS_DIR(st.st_mode)) {
    /* Not a directory, just call back once?
       Usually walk implies directory traversal.
       If user passed a file, we process it. */
    return cb(path, user_data);
  }

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  {
    /* Windows implementation using _findfirst / _findnext */
    struct _finddata_t file_info;
    intptr_t handle;
    char *search_path = NULL;

    /* Create search path: path + "\*" or "\*.*" */
    if (asprintf(&search_path, "%s\\*", path) == -1) {
      return ENOMEM;
    }

    handle = _findfirst(search_path, &file_info);
    free(search_path);

    if (handle == -1) {
      /* Empty dir or error? ENOENT means strict empty or not found. */
      if (errno == ENOENT)
        return 0; /* Empty usually behaves like this */
      return errno;
    }

    do {
      if (strcmp(file_info.name, ".") == 0 ||
          strcmp(file_info.name, "..") == 0) {
        continue;
      }

      if (asprintf(&full_path, "%s\\%s", path, file_info.name) == -1) {
        _findclose(handle);
        return ENOMEM;
      }

      if (file_info.attrib & _A_SUBDIR) {
        rc = walk_directory(full_path, cb, user_data);
      } else {
        rc = cb(full_path, user_data);
      }
      free(full_path);

      if (rc != 0) {
        _findclose(handle);
        return rc;
      }

    } while (_findnext(handle, &file_info) == 0);

    _findclose(handle);
  }
#else
  {
    /* POSIX implementation using opendir / readdir */
    DIR *d = opendir(path);
    struct dirent *entry;

    if (!d)
      return errno;

    while ((entry = readdir(d)) != NULL) {
      if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
        continue;
      }

      if (asprintf(&full_path, "%s/%s", path, entry->d_name) == -1) {
        closedir(d);
        return ENOMEM;
      }

      /* Need to stat to determine if dir or file, d_type is not standard posix
         everywhere (though common). Safe approach is stat. */
      if (c_stat_func(full_path, &st) == 0) {
        if (IS_DIR(st.st_mode)) {
          rc = walk_directory(full_path, cb, user_data);
        } else {
          rc = cb(full_path, user_data);
        }
      } else {
        /* Failed to stat? Skip or warning. */
      }

      free(full_path);

      if (rc != 0) {
        closedir(d);
        return rc;
      }
    }
    closedir(d);
  }
#endif

  return 0;
}
