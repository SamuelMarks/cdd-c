#ifndef FS_H
#define FS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <c_cdd_export.h>

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#define PATH_SEP "\\"
#define PATH_SEP_C '\\'
#define strtok_r strtok_s
#include "c_cddConfig.h"
/*typedef void *HWND;*/
/* ref learn.microsoft.com/en-us/windows/win32/winprog/windows-data-types */
typedef void *PVOID;
typedef PVOID HANDLE;
//-typedef HANDLE HWND;

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
#else
#include <limits.h>
#include <sys/stat.h>
#include <unistd.h>

#define PATH_SEP "/"
#define PATH_SEP_C '/'
#define delete_file unlink(filename);

#endif /* defined(_MSC_VER) && !defined(__INTEL_COMPILER) */

extern C_CDD_EXPORT const char *get_basename(const char *);

extern C_CDD_EXPORT const char *get_dirname(char *);

extern C_CDD_EXPORT char *c_read_file(const char *, int *, size_t *,
                                      const char *);

extern C_CDD_EXPORT int cp(const char *, const char *);

extern C_CDD_EXPORT int makedir(const char *);

extern C_CDD_EXPORT int makedirs(const char *);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !FS_H */