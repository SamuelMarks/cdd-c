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
 * @file fs_dir.c
 * @brief Implementation of filesystem directory traversal and creation utilities.
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
#elif defined(__WATCOMC__) || defined(__DOS__)
#include <direct.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
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
#ifdef CDD_BUILD_TESTS
      if (g_fail_io_after == 46) {
        g_fail_io_after = -1;
        fclose(file->fh);
        file->fh = NULL;
      }
#endif
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
