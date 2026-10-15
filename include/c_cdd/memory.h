/**
 * @file memory.h
 * @brief Memory allocation wrappers with test fault-injection support.
 */
#ifndef C_CDD_MEMORY_H
/** @brief Header guard for memory.h */
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
/** @brief Countdown counter for failing allocations in tests */
extern C_CDD_EXPORT int g_cdd_alloc_fail;
/** @brief Countdown counter for failing string duplications in tests */
extern C_CDD_EXPORT int g_cdd_strdup_fail;
#endif

#if defined(_MSC_VER)
/** @brief Cross-platform inline function specifier */
#define C_CDD_INLINE static __inline
#elif defined(__GNUC__) || defined(__clang__)
/** @brief Cross-platform inline function specifier */
#define C_CDD_INLINE static __inline__
#else
/** @brief Cross-platform inline function specifier */
#define C_CDD_INLINE static
#endif

/**
 * @brief Allocates memory with optional test fault injection.
 * @param[in] sz Number of bytes to allocate.
 * @return Allocated pointer or NULL on failure.
 */
C_CDD_INLINE void *c_cdd_malloc(size_t sz) {
#ifdef CDD_BUILD_TESTS
  if (g_cdd_alloc_fail && --g_cdd_alloc_fail == 0)
    return NULL;
#endif
  return malloc(sz);
}

/**
 * @brief Zero-allocates memory with optional test fault injection.
 * @param[in] n Number of elements.
 * @param[in] sz Size of each element.
 * @return Allocated pointer or NULL on failure.
 */
C_CDD_INLINE void *c_cdd_calloc(size_t n, size_t sz) {
#ifdef CDD_BUILD_TESTS
  if (g_cdd_alloc_fail && --g_cdd_alloc_fail == 0)
    return NULL;
#endif
  return calloc(n, sz);
}

/**
 * @brief Reallocates memory with optional test fault injection.
 * @param[in,out] p Existing pointer.
 * @param[in] sz New size in bytes.
 * @return Reallocated pointer or NULL on failure.
 */
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

/**
 * @brief Duplicates a string with optional test fault injection.
 * @param[in] s Null-terminated source string.
 * @return Newly allocated duplicate string or NULL on failure.
 */
C_CDD_INLINE char *c_cdd_strdup_macro(const char *s) {
#ifdef CDD_BUILD_TESTS
  if (g_cdd_alloc_fail && --g_cdd_alloc_fail == 0)
    return NULL;
#endif
  return (char *)memcpy(malloc(strlen(s) + 1), s, strlen(s) + 1);
}

/** @brief Allocates sz bytes of memory */
#define C_CDD_MALLOC(sz) c_cdd_malloc(sz)
/** @brief Zero-allocates n elements of sz bytes */
#define C_CDD_CALLOC(n, sz) c_cdd_calloc((n), (sz))
/** @brief Reallocates pointer p to sz bytes */
#define C_CDD_REALLOC(p, sz) c_cdd_realloc((p), (sz))
/** @brief Duplicates null-terminated string s */
#define C_CDD_STRDUP(s) c_cdd_strdup_macro(s)
/** @brief Frees allocated pointer p */
#define C_CDD_FREE(p) free(p)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_MEMORY_H */
