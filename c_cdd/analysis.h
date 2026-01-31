/**
 * @file analysis.h
 * @brief Static Analysis Engine for detecting unchecked memory allocations.
 *
 * Implements a heuristic scanner that identifies:
 * - Direct allocations (malloc, calloc, strdup) not followed by a NULL check.
 * - Indirect allocations (asprintf) not checked for error return codes.
 * - Logic flow (allocation inside `if` condition vs body).
 *
 * The engine is read-only and populates a result structure for the caller
 * to act upon (report or refactor).
 *
 * @author Samuel Marks
 */

#ifndef C_CDD_ANALYSIS_H
#define C_CDD_ANALYSIS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stddef.h>

#include "c_cdd_export.h"
#include "tokenizer.h"

/**
 * @brief Defines how an allocator function indicates failure.
 */
enum CheckStyle {
  CHECK_PTR_NULL,     /**< Failure if pointer variable is NULL (e.g. malloc) */
  CHECK_INT_NEGATIVE, /**< Failure if integer return < 0 (e.g. asprintf) */
  CHECK_INT_NONZERO /**< Failure if integer return != 0 (e.g. posix_memalign) */
};

/**
 * @brief Defines where the allocated object determines success.
 */
enum AllocatorStyle {
  ALLOC_STYLE_RETURN_PTR, /**< Returns allocated pointer directly (e.g. malloc)
                           */
  ALLOC_STYLE_ARG_PTR,    /**< Writes pointer to an argument (e.g. asprintf) */
  ALLOC_STYLE_STRUCT_PTR  /**< Writes to a struct field pointer passed as arg */
};

/**
 * @brief Specification for a known allocator function.
 */
struct AllocatorSpec {
  const char *name;            /**< Function name (e.g. "malloc") */
  enum AllocatorStyle style;   /**< Style of allocation location */
  enum CheckStyle check_style; /**< How to verify success */
  int ptr_arg_index; /**< Index of pointer argument (0-based) for ARG_PTR */
};

/**
 * @brief Represents a single detected allocation event.
 */
struct AllocationSite {
  size_t token_index; /**< Index of the allocator token (e.g. 'malloc') in the
                         list */
  char *
      var_name; /**< The variable name capturing result (LHS or arg), or NULL */

  /* Analysis Results */
  int is_checked;        /**< Boolean: 1 if safely checked, 0 otherwise */
  int used_before_check; /**< Boolean: 1 if dereferenced before validation */
  int is_return_stmt; /**< Boolean: 1 if allocation is immediately returned */

  const struct AllocatorSpec
      *spec; /**< Pointer to static spec describing the allocator used */
};

/**
 * @brief Container for analysis results.
 */
struct AllocationSiteList {
  struct AllocationSite *sites; /**< Array of findings */
  size_t size;                  /**< Number of items used */
  size_t capacity;              /**< Allocated capacity */
};

/**
 * @brief Initialize an AllocationSiteList.
 * Must be called before use.
 *
 * @param[out] list The list to initialize.
 * @return 0 on success, EINVAL if NULL, ENOMEM if alloc fails.
 */
extern C_CDD_EXPORT int
allocation_site_list_init(struct AllocationSiteList *list);

/**
 * @brief Free resources associated with an AllocationSiteList.
 * Frees any `var_name` strings and the array itself.
 *
 * @param[in] list The list to clean up. Safe to call on NULL.
 */
extern C_CDD_EXPORT void
allocation_site_list_free(struct AllocationSiteList *list);

/**
 * @brief Add a finding to the list.
 * Exposed for testing or manual list construction.
 *
 * @param[in,out] list The list to append to.
 * @param[in] index Token index of the call.
 * @param[in] var_name Name of the variable assigned (copied). Can be NULL.
 * @param[in] checked 1 if check detected.
 * @param[in] used 1 if usage before check detected.
 * @param[in] is_ret 1 if returned directly.
 * @param[in] spec The allocator specification.
 * @return 0 on success, ENOMEM on failure.
 */
extern C_CDD_EXPORT int
allocation_site_list_add(struct AllocationSiteList *list, size_t index,
                         const char *var_name, int checked, int used,
                         int is_ret, const struct AllocatorSpec *spec);

/**
 * @brief Scan a token stream for memory safety patterns.
 *
 * Iterates through tokens to find calls to known allocators (`malloc`,
 * `strdup`, etc.). For each call, analyzes the surrounding context
 * (assignments, `if` statements) to determine if the result is checked for
 * failure.
 *
 * @param[in] tokens The token stream to analyze.
 * @param[out] out The results container.
 * @return 0 on success, error code on failure.
 */
extern C_CDD_EXPORT int find_allocations(const struct TokenList *tokens,
                                         struct AllocationSiteList *out);

/**
 * @brief Check if a specific allocation instance is safe.
 *
 * Detailed heuristic check called internally by `find_allocations`.
 * Exposed for unit testing specific logic scenarios.
 *
 * @param[in] tokens Token stream.
 * @param[in] alloc_idx Index of the function call token.
 * @param[in] var_name The variable receiving the result (or NULL).
 * @param[in] spec The allocator traits.
 * @param[out] used_before_check Output flag set if unsafe usage detected.
 * @return 1 if checked/safe, 0 if unchecked.
 */
extern C_CDD_EXPORT int is_checked(const struct TokenList *tokens,
                                   size_t alloc_idx, const char *var_name,
                                   const struct AllocatorSpec *spec,
                                   int *used_before_check);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* C_CDD_ANALYSIS_H */
