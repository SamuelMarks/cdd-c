/**
 * @file memory.h
 * @brief Memory allocation wrappers routing through the active allocator
 * context.
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

#include "cdd_c_error.h"
#include <c_cdd_export.h>
/* clang-format on */

/** @brief Countdown counter for failing allocations in tests */
extern C_CDD_EXPORT int g_cdd_alloc_fail;
/** @brief Countdown counter for failing string duplications in tests */
extern C_CDD_EXPORT int g_cdd_strdup_fail;
/** @brief Countdown counter for failing reallocations in tests */
extern C_CDD_EXPORT int g_cdd_cst_realloc_fail;
/** @brief Countdown counter for failing CST token allocations in tests */
extern C_CDD_EXPORT int g_cdd_cst_alloc_token_fail;
/** @brief Countdown counter for failing CST node allocations in tests */
extern C_CDD_EXPORT int g_cdd_cst_alloc_node_fail;
/** @brief Scratch pointer storing the result of the most recent allocation */
extern C_CDD_EXPORT void *g_cdd_last_alloc_ptr;
/** @brief Scratch pointer storing the result of the most recent string
 * duplication */
extern C_CDD_EXPORT char *g_cdd_last_strdup_ptr;

#if defined(_MSC_VER)
/** @brief Raw string duplication macro for MSVC */
#define C_CDD_RAW_STRDUP(s) _strdup(s)
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L &&              \
    !defined(__STRICT_ANSI__)
/** @brief Raw string duplication macro for POSIX/C99 */
#define C_CDD_RAW_STRDUP(s) strdup(s)
#else
/** @brief Raw string duplication macro for strict C89 */
#define C_CDD_RAW_STRDUP(s)                                                    \
  (char *)memcpy(malloc(strlen(s) + 1), (s), strlen(s) + 1)
#endif

/**
 * @brief Allocates `sz` bytes through the active allocator context.
 *
 * @param[in] sz Number of bytes to allocate.
 * @param[out] out_ptr Output pointer receiving the allocated block or `NULL`.
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_MEMORY` /
 * `CDD_C_ERROR_INVALID_ARGUMENT`.
 */
C_CDD_EXPORT cdd_c_error_t c_cdd_do_malloc(size_t sz, void **out_ptr);

/**
 * @brief Zero-allocates `n * sz` bytes through the active allocator context.
 *
 * @param[in] n Number of elements.
 * @param[in] sz Size of each element in bytes.
 * @param[out] out_ptr Output pointer receiving the allocated block or `NULL`.
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_MEMORY` /
 * `CDD_C_ERROR_INVALID_ARGUMENT`.
 */
C_CDD_EXPORT cdd_c_error_t c_cdd_do_calloc(size_t n, size_t sz, void **out_ptr);

/**
 * @brief Reallocates `p` to `sz` bytes through the active allocator context.
 *
 * @param[in,out] p Existing pointer (or `NULL`).
 * @param[in] sz New size in bytes.
 * @param[out] out_ptr Output pointer receiving the resized block or `NULL`.
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_MEMORY` /
 * `CDD_C_ERROR_INVALID_ARGUMENT`.
 */
C_CDD_EXPORT cdd_c_error_t c_cdd_do_realloc(void *p, size_t sz, void **out_ptr);

/**
 * @brief Duplicates a null-terminated string through the active allocator
 * context.
 *
 * @param[in] s Null-terminated source string.
 * @param[out] out_str Output pointer receiving the duplicated string or `NULL`.
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_MEMORY` /
 * `CDD_C_ERROR_INVALID_ARGUMENT`.
 */
C_CDD_EXPORT cdd_c_error_t c_cdd_do_strdup(const char *s, char **out_str);

/**
 * @brief Frees a memory block through the active allocator context.
 *
 * @param[in,out] p Pointer to free (or `NULL`).
 */
C_CDD_EXPORT void c_cdd_do_free(void *p);

/** @brief Allocates sz bytes of memory with optional test fault injection */
#define c_cdd_malloc(sz)                                                       \
  (c_cdd_do_malloc((sz), &g_cdd_last_alloc_ptr), g_cdd_last_alloc_ptr)

/** @brief Zero-allocates n elements of sz bytes with optional test fault
 * injection */
#define c_cdd_calloc(n, sz)                                                    \
  (c_cdd_do_calloc((n), (sz), &g_cdd_last_alloc_ptr), g_cdd_last_alloc_ptr)

/** @brief Reallocates pointer p to sz bytes with optional test fault injection
 */
#define c_cdd_realloc(p, sz)                                                   \
  (c_cdd_do_realloc((p), (sz), &g_cdd_last_alloc_ptr), g_cdd_last_alloc_ptr)

/** @brief Duplicates null-terminated string s with optional test fault
 * injection */
#define c_cdd_strdup_macro(s)                                                  \
  (c_cdd_do_strdup((s), &g_cdd_last_strdup_ptr), g_cdd_last_strdup_ptr)

/** @brief Allocates sz bytes of memory */
#define C_CDD_MALLOC(sz) c_cdd_malloc(sz)
/** @brief Zero-allocates n elements of sz bytes */
#define C_CDD_CALLOC(n, sz) c_cdd_calloc((n), (sz))
/** @brief Reallocates pointer p to sz bytes */
#define C_CDD_REALLOC(p, sz) c_cdd_realloc((p), (sz))
/** @brief Duplicates null-terminated string s */
#define C_CDD_STRDUP(s) c_cdd_strdup_macro(s)
/** @brief Frees allocated pointer p */
#define C_CDD_FREE(p) c_cdd_do_free(p)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_MEMORY_H */
