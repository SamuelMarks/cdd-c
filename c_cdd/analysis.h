/**
 * @file analysis.h
 * @brief Analysis logic to detect unchecked memory allocations.
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
 * @brief Represents a site where memory allocation occurs.
 */
struct C_CDD_EXPORT AllocationSite {
  size_t token_index; /**< Index of the allocator token (e.g. 'malloc') in the
                         list */
  char *var_name;     /**< The variable name being assigned to */
  int is_checked;     /**< Boolean: 1 if checked against NULL, 0 otherwise */
};

/**
 * @brief Dynamic list of allocation sites.
 */
struct C_CDD_EXPORT AllocationSiteList {
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
 *
 * @param[in] list The list to clean up.
 */
extern C_CDD_EXPORT void
allocation_site_list_free(struct AllocationSiteList *list);

/**
 * @brief Find all occurrences of unchecked allocations in the token stream.
 * Scans for `malloc`, `realloc`, `calloc` calls, identifies the assigned
 * variable, and determines if that variable is checked for NULL before use.
 *
 * @param[in] tokens The token stream to analyze.
 * @param[out] out The destination list to populate with findings.
 * @return 0 on success, error code on failure.
 */
extern C_CDD_EXPORT int find_allocations(const struct TokenList *tokens,
                                         struct AllocationSiteList *out);

/**
 * @brief Check if a specific allocation is safely checked.
 *
 * @param[in] tokens The token stream.
 * @param[in] alloc_idx The index of the allocation function call token.
 * @param[in] var_name The variable name expected to be checked.
 * @return 1 if checked, 0 if unchecked or used before check.
 */
extern C_CDD_EXPORT int is_checked(const struct TokenList *tokens,
                                   size_t alloc_idx, const char *var_name);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ANALYSIS_H */
