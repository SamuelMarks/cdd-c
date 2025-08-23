#ifndef FS_H
#define FS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <c_cdd_export.h>

#include <errno.h>

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#define PATH_SEP "\\"
#define PATH_SEP_C '\\'
#define strtok_r strtok_s
#include "c_cddConfig.h"
/*typedef void *HWND;*/
/* ref learn.microsoft.com/en-us/windows/win32/winprog/windows-data-types */
typedef void *PVOID;
typedef PVOID HANDLE;
/* typedef HANDLE HWND; */

/*struct HWND__ {
  int unused;
};*/
typedef struct HWND__ *HWND;

/*#include <minwinbase.h>*/
#include <direct.h>
#include <io.h>
#include <sys/stat.h>
#define F_OK 0
#define access _access
#define rmdir _rmdir
#ifndef PATH_MAX
#define PATH_MAX _MAX_PATH
#endif                     /* !PATH_MAX */
#define delete_file remove /* `DeleteFile` requires winbase.h :( */
#define unlink _unlink
#ifndef W_OK
#define W_OK 0
extern C_CDD_EXPORT int path_is_unc(const char *path);
extern C_CDD_EXPORT int ascii_to_wide(const char *, wchar_t *, size_t);
extern C_CDD_EXPORT int wide_to_ascii(const wchar_t *, char *, size_t);
#endif /* !W_OK */
#else
#include <limits.h>
#include <sys/stat.h>
#include <unistd.h>

#define PATH_SEP "/"
#define PATH_SEP_C '/'
#define delete_file unlink

#endif /* defined(_MSC_VER) && !defined(__INTEL_COMPILER) */

enum FopenError {
  FOPEN_OK = 0, /* No error */
  FOPEN_INVALID_PARAMETER =
      EINVAL, /* NULL stream, filename, or mode; invalid mode */
  FOPEN_TOO_MANY_OPEN_FILES = EMFILE, /* Too many files open in this process */
  FOPEN_OUT_OF_MEMORY = ENOMEM,       /* Couldn’t allocate internal buffers */
  FOPEN_FILE_NOT_FOUND = ENOENT,      /* Input mode but file doesn’t exist */
  FOPEN_PERMISSION_DENIED =
      EACCES, /* Permission denied or directory not writable */
  FOPEN_FILENAME_TOO_LONG = ERANGE, /* Filename length exceeds FILENAME_MAX */
  FOPEN_UNKNOWN_ERROR = -1
};

extern C_CDD_EXPORT enum FopenError fopen_error_from(int);

/* TOCTOU risk so need to make filename and a FILE* at same time */
struct FilenameAndPtr {
  FILE *fh;
  char *filename;
};

extern C_CDD_EXPORT const char *get_basename(const char *);

extern C_CDD_EXPORT const char *get_dirname(char *);

extern C_CDD_EXPORT char *read_to_file(const char *, int *, size_t *,
                                       const char *);

extern C_CDD_EXPORT int cp(const char *, const char *);

extern C_CDD_EXPORT int makedir(const char *);

extern C_CDD_EXPORT int makedirs(const char *);

extern C_CDD_EXPORT int tempdir(const char **);

extern C_CDD_EXPORT void FilenameAndPtr_cleanup(struct FilenameAndPtr *);

extern C_CDD_EXPORT void
FilenameAndPtr_delete_and_cleanup(struct FilenameAndPtr *);

extern C_CDD_EXPORT int mktmpfilegetnameandfile(const char *prefix,
                                                const char *suffix,
                                                char const *mode,
                                                struct FilenameAndPtr *);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !FS_H */
