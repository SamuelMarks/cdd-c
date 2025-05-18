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
#else
#define PATH_SEP "/"
#define PATH_SEP_C '/'
#endif /* defined(_MSC_VER) && !defined(__INTEL_COMPILER) */

extern C_CDD_EXPORT const char *get_basename(const char *);
extern C_CDD_EXPORT char *c_read_file(const char *, int *, size_t *,
                                      const char *);

extern C_CDD_EXPORT int cp(const char *, const char *);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !FS_H */
