#ifndef C_CDD_MEMORY_H
#define C_CDD_MEMORY_H

/* clang-format off */
#include "cdd_c_error.h"
#include <stdlib.h>
#include <string.h>
/* clang-format on */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef CDD_BUILD_TESTS

extern int g_cdd_alloc_fail_countdown;
extern void *test_malloc(size_t size);
extern void *test_calloc(size_t count, size_t size);
extern void *test_realloc(void *ptr, size_t size);
extern char *test_strdup(const char *str);

#ifndef C_CDD_MALLOC
#define C_CDD_MALLOC test_malloc
#endif
#ifndef C_CDD_CALLOC
#define C_CDD_CALLOC test_calloc
#endif
#ifndef C_CDD_REALLOC
#define C_CDD_REALLOC test_realloc
#endif
#ifndef C_CDD_FREE
#define C_CDD_FREE free
#endif
#ifndef C_CDD_STRDUP
#define C_CDD_STRDUP test_strdup
#endif

#else /* CDD_BUILD_TESTS */

#ifndef C_CDD_MALLOC
#define C_CDD_MALLOC malloc
#endif

#ifndef C_CDD_CALLOC
#define C_CDD_CALLOC calloc
#endif

#ifndef C_CDD_REALLOC
#define C_CDD_REALLOC realloc
#endif

#ifndef C_CDD_FREE
#define C_CDD_FREE free
#endif

#ifndef C_CDD_STRDUP
#if defined(_MSC_VER)
#define C_CDD_STRDUP _strdup
#else
#define C_CDD_STRDUP strdup
#endif
#endif

#endif /* CDD_BUILD_TESTS */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_MEMORY_H */
