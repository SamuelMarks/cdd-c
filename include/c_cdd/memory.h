#ifndef C_CDD_MEMORY_H
#define C_CDD_MEMORY_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/* clang-format off */
#include <stdlib.h>
#include <string.h>

#include <c_cdd_export.h>
/* clang-format on */

#ifdef CDD_BUILD_TESTS
extern C_CDD_EXPORT int g_cdd_alloc_fail;
extern C_CDD_EXPORT int g_cdd_strdup_fail;
#endif

#if defined(_MSC_VER)
#define C_CDD_INLINE static __inline
#elif defined(__GNUC__) || defined(__clang__)
#define C_CDD_INLINE static __inline__
#else
#define C_CDD_INLINE static
#endif

C_CDD_INLINE void *c_cdd_malloc(size_t sz) {
#ifdef CDD_BUILD_TESTS
  if (g_cdd_alloc_fail && --g_cdd_alloc_fail == 0)
    return NULL;
#endif
  return malloc(sz);
}

C_CDD_INLINE void *c_cdd_calloc(size_t n, size_t sz) {
#ifdef CDD_BUILD_TESTS
  if (g_cdd_alloc_fail && --g_cdd_alloc_fail == 0)
    return NULL;
#endif
  return calloc(n, sz);
}

C_CDD_INLINE void *c_cdd_realloc(void *p, size_t sz) {
#ifdef CDD_BUILD_TESTS
  if (g_cdd_alloc_fail) {
    if (--g_cdd_alloc_fail == 0) {
      return NULL;
    }
  }
#endif
  return realloc(p, sz);
}

C_CDD_INLINE char *c_cdd_strdup_macro(const char *s) {
#ifdef CDD_BUILD_TESTS
  if (g_cdd_alloc_fail && --g_cdd_alloc_fail == 0)
    return NULL;
#endif
  return (char *)memcpy(malloc(strlen(s) + 1), s, strlen(s) + 1);
}

#define C_CDD_MALLOC(sz) c_cdd_malloc(sz)
#define C_CDD_CALLOC(n, sz) c_cdd_calloc((n), (sz))
#define C_CDD_REALLOC(p, sz) c_cdd_realloc((p), (sz))
#define C_CDD_STRDUP(s) c_cdd_strdup_macro(s)
#define C_CDD_FREE(p) free(p)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_MEMORY_H */
