/**
 * @file analysis.h
 * @brief Analysis logic to detect unchecked memory allocations.
 * Includes support for return-based (malloc) and argument-based (asprintf)
 * allocators, as well as distinct failure check styles (NULL pointer vs
 * negative int).
 * @author Samuel Marks
 */

#ifndef ANALYSIS_H
#define ANALYSIS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <c_cdd_export.h>
#include <stddef.h>

#include "tokenizer.h"

/**
 * @brief Defines how an allocator function indicates failure.
 */
enum CheckStyle {
  CHECK_PTR_NULL,     /**< Failure if pointer variable is NULL (e.g. malloc) */
  CHECK_INT_NEGATIVE, /**< Failure if integer return < 0 (e.g. asprintf,
                         scandir) */
  CHECK_INT_NONZERO /**< Failure if integer return != 0 (e.g. glob, getaddrinfo)
                     */
};

/**
 * @brief Defines where the allocated object can be found relative to the call.
 */
enum AllocatorStyle {
  ALLOC_STYLE_RETURN_PTR, /**< Returns allocated pointer directly (e.g. malloc)
                           */
  ALLOC_STYLE_ARG_PTR,    /**< Writes pointer to an argument (e.g. asprintf,
                             scandir) */
  ALLOC_STYLE_STRUCT_PTR  /**< Writes to a struct field pointer passed as arg
                             (e.g. glob) */
};

/**
 * @brief Specification for a known allocator function.
 */
struct AllocatorSpec {
  const char *name;            /**< Function name (e.g. "malloc") */
  enum AllocatorStyle style;   /**< Style of allocation location */
  enum CheckStyle check_style; /**< How to verify success */
  int ptr_arg_index;           /**< Index of pointer argument (0-based) for
                                  ARG_PTR/STRUCT_PTR */
};

/**
 * @brief Represents a site where memory allocation occurs.
 */
struct AllocationSite {
  size_t token_index; /**< Index of the allocator token (e.g. 'malloc') in the
                         list */
  char *var_name; /**< The variable name capturing result (LHS or arg), or NULL
                     if return-stmt/unassigned */
  int is_checked; /**< Boolean: 1 if checked, 0 otherwise */
  int used_before_check; /**< Boolean: 1 if dereferenced before validation */
  int is_return_stmt; /**< Boolean: 1 if allocation is immediately returned */
  const struct AllocatorSpec
      *spec; /**< Pointer to static spec describing the allocator used */
};

/**
 * @brief Dynamic list of allocation sites.
 */
struct AllocationSiteList {
  struct AllocationSite *sites; /**< Array of AllocationSite structures */
  size_t size;                  /**< Number of items used */
  size_t capacity;              /**< Allocated capacity */
};

/**
 * @brief Initialize an AllocationSiteList.
 *
 * @param[out] list The list to initialize.
 * @return 0 on success, error code (EINVAL/ENOMEM) on failure.
 */
extern C_CDD_EXPORT int
allocation_site_list_init(struct AllocationSiteList *list);

/**
 * @brief Free memory associated with an AllocationSiteList.
 * Does not free the static AllocatorSpec pointers.
 *
 * @param[in] list The list to clean up.
 */
extern C_CDD_EXPORT void
allocation_site_list_free(struct AllocationSiteList *list);

/**
 * @brief Find all occurrences of unchecked allocations in the token stream.
 * Scans for known allocators, identifies the assigned variable (or return
 * context), and determines if that variable is checked for failure using the
 * appropriate style.
 *
 * @param[in] tokens The token stream to analyze.
 * @param[out] out The destination list to populate with findings.
 * @return 0 on success, error code on failure.
 */
extern C_CDD_EXPORT int find_allocations(const struct TokenList *tokens,
                                         struct AllocationSiteList *out);

/**
 * @brief Check if a specific allocation is safely checked.
 * Checks if the variable is used in a condition subsequent to allocation,
 * or if the allocation itself occurs within a condition.
 *
 * @param[in] tokens The token stream.
 * @param[in] alloc_idx The index of the allocation function call token.
 * @param[in] var_name The variable name expected to be checked.
 * @param[in] spec The allocator specification (determines check logic).
 * @param[out] used_before_check Set to 1 if dereferenced before check.
 * @return 1 if checked, 0 if unchecked or used before check.
 */
extern C_CDD_EXPORT int is_checked(const struct TokenList *tokens,
                                   size_t alloc_idx, const char *var_name,
                                   const struct AllocatorSpec *spec,
                                   int *used_before_check);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ANALYSIS_H */
