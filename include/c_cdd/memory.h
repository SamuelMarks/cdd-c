#ifndef C_CDD_MEMORY_H
#define C_CDD_MEMORY_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include <stdlib.h>
#include <string.h>
/* clang-format on */

#include <c_cdd_export.h>

extern C_CDD_EXPORT int g_cdd_alloc_fail;
#if 0
#else
#define C_CDD_MALLOC(sz) malloc(sz)
#define C_CDD_CALLOC(n, sz) calloc((n), (sz))
#define C_CDD_REALLOC(p, sz) realloc((p), (sz))
#define C_CDD_STRDUP(s)                                                        \
  ((char *)memcpy(malloc(strlen(s) + 1), (s), strlen(s) + 1))
#endif /* CDD_BUILD_TESTS */

#define C_CDD_FREE(p) free(p)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_MEMORY_H */
