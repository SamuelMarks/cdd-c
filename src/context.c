/**
 * @file context.c
 * @brief Implementation of the dependency-injected allocator, VFS, and socket
 * context (`cdd_context_t`).
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "c_cdd/context.h"
#include "c_cdd/memory.h"
#include "c_cdd/safe_crt.h"
#include "classes/parse/cdd_cst_node.h"
#include "functions/parse/analysis.h"
#include "functions/parse/cst.h"
#include "functions/parse/fs.h"
#include "functions/parse/str.h"
#include "openapi/parse/openapi.h"

#include <stdlib.h>
#include <string.h>
/* clang-format on */

/** @brief Countdown counter for failing allocations in tests */
C_CDD_EXPORT int g_cdd_alloc_fail = 0;
/** @brief Countdown counter for failing string duplications in tests */
C_CDD_EXPORT int g_cdd_strdup_fail = 0;
/** @brief Countdown counter for failing reallocations in tests */
C_CDD_EXPORT int g_cdd_cst_realloc_fail = 0;
/** @brief Countdown counter for failing CST token allocations in tests */
C_CDD_EXPORT int g_cdd_cst_alloc_token_fail = 0;
/** @brief Countdown counter for failing CST node allocations in tests */
C_CDD_EXPORT int g_cdd_cst_alloc_node_fail = 0;
/** @brief Flag to simulate CST query result append failure in tests */
C_CDD_EXPORT int g_cdd_query_err_fail = 0;
/** @brief Countdown counter for failing pp_token_to_string allocations */
C_CDD_EXPORT int g_cdd_pp_token_to_string_fail = 0;
/** @brief Countdown counter for failing pp_resolve_path allocations */
C_CDD_EXPORT int g_cdd_pp_resolve_path_fail = 0;
/** @brief Countdown counter for failing cst_list_add reallocations */
C_CDD_EXPORT int g_cdd_fail_cst_list_add = 0;
/** @brief Countdown counter for failing Safe CRT allocations */
C_CDD_EXPORT int g_safe_crt_malloc_fail = 0;
/** @brief Flag to simulate schema strdup allocation failure */
C_CDD_EXPORT int g_schema_strdup_fail = 0;
/** @brief Flag to simulate schema realloc allocation failure */
C_CDD_EXPORT int g_schema_realloc_fail = 0;
/** @brief Countdown counter for failing CST emit reallocations */
C_CDD_EXPORT int g_cdd_cst_emit_realloc_fail = 0;
/** @brief Countdown counter for failing URL segment allocations */
C_CDD_EXPORT int g_cdd_fail_url_segment_alloc = 0;
/** @brief Flag to simulate aggregator path array realloc failure */
C_CDD_EXPORT int g_cdd_aggregator_fail_path_realloc = 0;
/** @brief Flag to simulate aggregator operations array realloc failure */
C_CDD_EXPORT int g_cdd_aggregator_fail_ops_realloc = 0;
/** @brief Flag to simulate aggregator route strdup failure */
C_CDD_EXPORT int g_cdd_aggregator_fail_route_strdup = 0;
/** @brief Counter to simulate allocation failure in allocation_site_list_init
 */
C_CDD_EXPORT int g_cdd_analysis_fail_alloc_init = 0;
/** @brief Counter to simulate allocation failure in allocation_site_list_add */
C_CDD_EXPORT int g_cdd_analysis_fail_alloc_add = 0;
/** @brief Counter to simulate variable name allocation failure in
 * allocation_site_list_add */
C_CDD_EXPORT int g_cdd_analysis_fail_alloc_varname = 0;
/** @brief Scratch pointer storing the result of the most recent allocation */
C_CDD_EXPORT void *g_cdd_last_alloc_ptr = NULL;
/** @brief Scratch pointer storing the result of the most recent string
 * duplication */
C_CDD_EXPORT char *g_cdd_last_strdup_ptr = NULL;

/**
 * @brief Default `malloc` implementation routing through the centralised
 * test countdown state.
 *
 * @param[in] size Number of bytes to allocate.
 * @param[out] out_ptr Output pointer receiving the allocated memory block.
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_MEMORY` on failure.
 */
static cdd_c_error_t default_alloc_malloc(size_t size, void **out_ptr) {
  void *p;
  if (!out_ptr)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  p = ((g_cdd_alloc_fail && --g_cdd_alloc_fail == 0) ||
       (g_safe_crt_malloc_fail > 0 && --g_safe_crt_malloc_fail == 0) ||
       (g_cdd_fail_url_segment_alloc > 0 &&
        --g_cdd_fail_url_segment_alloc == 0) ||
       (size == 8 * sizeof(struct AllocationSite) &&
        g_cdd_analysis_fail_alloc_init > 0 &&
        --g_cdd_analysis_fail_alloc_init == 0) ||
       (g_cdd_pp_token_to_string_fail &&
        --g_cdd_pp_token_to_string_fail == 0) ||
       (g_cdd_pp_resolve_path_fail && --g_cdd_pp_resolve_path_fail == 0) ||
       (size == 4096 && g_cdd_cst_alloc_token_fail &&
        --g_cdd_cst_alloc_token_fail == 0))
          ? NULL
          : malloc(size);
  *out_ptr = p;
  return p ? CDD_C_SUCCESS : CDD_C_ERROR_MEMORY;
}

/**
 * @brief Default `calloc` implementation routing through the centralised
 * test countdown state.
 *
 * @param[in] count Number of elements.
 * @param[in] size Size of each element in bytes.
 * @param[out] out_ptr Output pointer receiving the zeroed memory block.
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_MEMORY` on failure.
 */
static cdd_c_error_t default_alloc_calloc(size_t count, size_t size,
                                          void **out_ptr) {
  void *p;
  if (!out_ptr)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  p = ((g_cdd_alloc_fail && --g_cdd_alloc_fail == 0) ||
       (size == sizeof(cdd_cst_node_t) && g_cdd_cst_alloc_node_fail &&
        --g_cdd_cst_alloc_node_fail == 0) ||
       ((size == sizeof(cdd_token_t) || size == sizeof(cdd_trivia_t)) &&
        g_cdd_cst_alloc_token_fail && --g_cdd_cst_alloc_token_fail == 0))
          ? NULL
          : calloc(count, size);
  *out_ptr = p;
  return p ? CDD_C_SUCCESS : CDD_C_ERROR_MEMORY;
}

/**
 * @brief Default `realloc` implementation routing through the centralised
 * test countdown state.
 *
 * @param[in,out] ptr Existing pointer (or `NULL`).
 * @param[in] new_size New byte size.
 * @param[out] out_ptr Output pointer receiving the resized memory block.
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_MEMORY` on failure.
 */
static cdd_c_error_t default_alloc_realloc(void *ptr, size_t new_size,
                                           void **out_ptr) {
  void *p;
  if (!out_ptr)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  p = ((g_cdd_alloc_fail && --g_cdd_alloc_fail == 0) ||
       (g_cdd_cst_realloc_fail && --g_cdd_cst_realloc_fail == 0) ||
       (g_cdd_cst_emit_realloc_fail && --g_cdd_cst_emit_realloc_fail == 0) ||
       (g_cdd_fail_url_segment_alloc > 0 &&
        --g_cdd_fail_url_segment_alloc == 0) ||
       (new_size % sizeof(struct AllocationSite) == 0 &&
        g_cdd_analysis_fail_alloc_add > 0 &&
        --g_cdd_analysis_fail_alloc_add == 0) ||
       g_schema_realloc_fail ||
       (new_size % sizeof(struct OpenAPI_Path) == 0 &&
        g_cdd_aggregator_fail_path_realloc) ||
       (new_size % sizeof(struct OpenAPI_Operation) == 0 &&
        g_cdd_aggregator_fail_ops_realloc) ||
       (new_size == 32 * sizeof(char *) && g_safe_crt_malloc_fail > 0 &&
        --g_safe_crt_malloc_fail == 0) ||
       (new_size % sizeof(struct CstNode) == 0 && g_cdd_fail_cst_list_add &&
        --g_cdd_fail_cst_list_add == 0) ||
       (new_size == 16 * sizeof(cdd_cst_node_t *) && g_cdd_query_err_fail))
          ? NULL
          : realloc(ptr, new_size);
  *out_ptr = p;
  return p ? CDD_C_SUCCESS : CDD_C_ERROR_MEMORY;
}

/**
 * @brief Default `strdup` implementation routing through the centralised
 * test countdown state.
 *
 * @param[in] str Null-terminated source string.
 * @param[out] out_str Output pointer receiving the duplicated string.
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_MEMORY` on failure.
 */
static cdd_c_error_t default_alloc_strdup(const char *str, char **out_str) {
  char *p;
  size_t len;
  if (!out_str || !str)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  len = strlen(str) + 1;
  p = ((g_cdd_strdup_fail && --g_cdd_strdup_fail == 0) ||
       (g_safe_crt_malloc_fail > 0 && --g_safe_crt_malloc_fail == 0) ||
       (g_cdd_analysis_fail_alloc_varname > 0 &&
        --g_cdd_analysis_fail_alloc_varname == 0) ||
       g_schema_strdup_fail || g_cdd_aggregator_fail_route_strdup)
          ? NULL
          : (char *)malloc(len);
  if (p)
    memcpy(p, str, len);
  *out_str = p;
  return p ? CDD_C_SUCCESS : CDD_C_ERROR_MEMORY;
}

/**
 * @brief Default `free` implementation delegating to standard `free`.
 *
 * @param[in,out] ptr Pointer to free (or `NULL`).
 */
static void default_alloc_free(void *ptr) { free(ptr); }

/**
 * @brief Default `fopen` VFS implementation.
 *
 * @param[in] path Filesystem path.
 * @param[in] mode File access mode string.
 * @param[out] out_fp Output file handle pointer.
 * @return `CDD_C_SUCCESS` on success, or error code on failure.
 */
static cdd_c_error_t default_vfs_fopen(const char *path, const char *mode,
                                       FILE **out_fp) {
  FILE *fp = NULL;
  if (!path || !mode || !out_fp)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (g_cdd_fopen_fail && --g_cdd_fopen_fail == 0) {
    *out_fp = NULL;
    return CDD_C_ERROR_IO;
  }
#if defined(_MSC_VER)
  if (fopen_s(&fp, path, mode) != 0)
    fp = NULL;
#else
  fp = fopen(path, mode);
#endif
  if (!fp) {
    *out_fp = NULL;
    return CDD_C_ERROR_IO;
  }
  *out_fp = fp;
  return CDD_C_SUCCESS;
}

/**
 * @brief Default `fread` VFS implementation.
 *
 * @param[out] buf Destination buffer.
 * @param[in] elem_size Size of each element.
 * @param[in] count Number of elements to read.
 * @param[in,out] fp Open file stream.
 * @param[out] out_read Number of elements actually read.
 * @return `CDD_C_SUCCESS` on success, or error code on failure.
 */
static cdd_c_error_t default_vfs_fread(void *buf, size_t elem_size,
                                       size_t count, FILE *fp,
                                       size_t *out_read) {
  size_t n;
  if (!buf || !fp || !out_read)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  n = fread(buf, elem_size, count, fp);
  *out_read = n;
  return CDD_C_SUCCESS;
}

/**
 * @brief Default `fwrite` VFS implementation.
 *
 * @param[in] buf Source buffer.
 * @param[in] elem_size Size of each element.
 * @param[in] count Number of elements to write.
 * @param[in,out] fp Open file stream.
 * @param[out] out_written Number of elements actually written.
 * @return `CDD_C_SUCCESS` on success, or error code on failure.
 */
static cdd_c_error_t default_vfs_fwrite(const void *buf, size_t elem_size,
                                        size_t count, FILE *fp,
                                        size_t *out_written) {
  size_t n;
  if (!buf || !fp || !out_written)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  n = fwrite(buf, elem_size, count, fp);
  *out_written = n;
  return (n == count) ? CDD_C_SUCCESS : CDD_C_ERROR_IO;
}

/**
 * @brief Default `fclose` VFS implementation.
 *
 * @param[in,out] fp Stream to close.
 * @return `CDD_C_SUCCESS` on success, or error code on failure.
 */
static cdd_c_error_t default_vfs_fclose(FILE *fp) {
  if (!fp)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  return (fclose(fp) == 0) ? CDD_C_SUCCESS : CDD_C_ERROR_IO;
}

/**
 * @brief Default `stat` VFS implementation checking directory status.
 *
 * @param[in] path Filesystem path.
 * @param[out] out_is_dir Receives `1` if directory, `0` otherwise.
 * @return `CDD_C_SUCCESS` on success, or error code.
 */
static cdd_c_error_t default_vfs_stat(const char *path, int *out_is_dir) {
  if (!path || !out_is_dir)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  return fs_is_directory(path, out_is_dir);
}

/**
 * @brief Default `opendir` VFS placeholder implementation.
 *
 * @param[in] path Directory path.
 * @param[out] out_dir Output handle pointer.
 * @return `CDD_C_SUCCESS` if directory exists, or error code.
 */
static cdd_c_error_t default_vfs_opendir(const char *path, void **out_dir) {
  int is_dir = 0;
  cdd_c_error_t rc;
  if (!path || !out_dir)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = fs_is_directory(path, &is_dir);
  if (!is_dir)
    return CDD_C_ERROR_NOT_FOUND;
  *out_dir = (void *)1;
  return rc;
}

/** @brief Built-in default dependency-injection context. */
static const cdd_context_t g_default_context = {
    {default_alloc_malloc, default_alloc_calloc, default_alloc_realloc,
     default_alloc_strdup, default_alloc_free},
    {default_vfs_fopen, default_vfs_fread, default_vfs_fwrite,
     default_vfs_fclose, default_vfs_stat, default_vfs_opendir},
    {NULL, NULL, NULL, NULL, NULL, NULL, NULL},
    NULL};

/** @brief Active global dependency-injection context pointer. */
static const cdd_context_t *g_active_context = &g_default_context;

/**
 * @brief Allocates `sz` bytes through the active allocator context.
 *
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_MEMORY` /
 * `CDD_C_ERROR_INVALID_ARGUMENT`.
 */
cdd_c_error_t c_cdd_do_malloc(size_t sz, void **out_ptr) {
  return g_active_context->allocator.malloc_fn(sz, out_ptr);
}

/**
 * @brief Zero-allocates `n * sz` bytes through the active allocator context.
 *
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_MEMORY` /
 * `CDD_C_ERROR_INVALID_ARGUMENT`.
 */
cdd_c_error_t c_cdd_do_calloc(size_t n, size_t sz, void **out_ptr) {
  return g_active_context->allocator.calloc_fn(n, sz, out_ptr);
}

/**
 * @brief Reallocates `p` to `sz` bytes through the active allocator context.
 *
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_MEMORY` /
 * `CDD_C_ERROR_INVALID_ARGUMENT`.
 */
cdd_c_error_t c_cdd_do_realloc(void *p, size_t sz, void **out_ptr) {
  return g_active_context->allocator.realloc_fn(p, sz, out_ptr);
}

/**
 * @brief Duplicates a null-terminated string through the active allocator
 * context.
 *
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_MEMORY` /
 * `CDD_C_ERROR_INVALID_ARGUMENT`.
 */
cdd_c_error_t c_cdd_do_strdup(const char *s, char **out_str) {
  if (g_cdd_alloc_fail && --g_cdd_alloc_fail == 0) {
    *out_str = NULL;
    return CDD_C_ERROR_MEMORY;
  }
  return g_active_context->allocator.strdup_fn(s, out_str);
}

/**
 * @brief Frees a memory block through the active allocator context.
 *
 * @param[in,out] p Pointer to free (or `NULL`).
 */
void c_cdd_do_free(void *p) { g_active_context->allocator.free_fn(p); }

/**
 * @brief Initializes `ctx` with the default standard C allocator, VFS, and
 * socket VTables.
 *
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_INVALID_ARGUMENT` if
 * `ctx` is `NULL`.
 */
cdd_c_error_t cdd_context_init_default(cdd_context_t *ctx) {
  if (!ctx)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *ctx = g_default_context;
  return CDD_C_SUCCESS;
}

/**
 * @brief Sets the active process/thread-local `cdd_context_t` override (or
 * resets to the built-in default when `ctx == NULL`).
 *
 * @return `CDD_C_SUCCESS`.
 */
cdd_c_error_t cdd_context_set_global(const cdd_context_t *ctx) {
  g_active_context = ctx ? ctx : &g_default_context;
  return CDD_C_SUCCESS;
}

/** @brief Countdown counter for failing VFS fopen operations */
C_CDD_EXPORT int g_cdd_fopen_fail = 0;

/**
 * @brief Opens a file through the active `cdd_context_t` VFS `fopen_fn` hook.
 *
 * @param[in] path Filesystem path.
 * @param[in] mode File access mode string.
 * @param[out] out_fp Output file handle pointer.
 * @return `CDD_C_SUCCESS` on success, or error code on failure.
 */
cdd_c_error_t cdd_vfs_fopen(const char *path, const char *mode, FILE **out_fp) {
  return g_active_context->vfs.fopen_fn(path, mode, out_fp);
}

/**
 * @brief Retrieves the currently active `cdd_context_t` (never `NULL`).
 *
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_INVALID_ARGUMENT` if
 * `out_ctx` is `NULL`.
 */
cdd_c_error_t cdd_context_get_global(const cdd_context_t **out_ctx) {
  if (!out_ctx)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_ctx = g_active_context;
  return CDD_C_SUCCESS;
}
