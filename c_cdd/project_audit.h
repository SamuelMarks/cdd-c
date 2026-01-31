/**
 * @file project_audit.h
 * @brief Analysis and auditing for C projects.
 * Provides functionality to walk directories, scan files, and report on memory
 * allocation safety.
 * @author Samuel Marks
 */

#ifndef PROJECT_AUDIT_H
#define PROJECT_AUDIT_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <c_cdd_export.h>
#include <stddef.h>

/**
 * @brief Statistics collected during an audit.
 */
struct C_CDD_EXPORT AuditStats {
  size_t files_scanned;         /**< Number of C files analyzed */
  size_t allocations_checked;   /**< Count of safe (checked) allocations */
  size_t allocations_unchecked; /**< Count of unsafe (unchecked) allocations */
  size_t functions_returning_alloc; /**< Count of functions directly returning
                                       new allocations */
};

/**
 * @brief Initialize audit statistics to zero.
 * @param[out] stats Pointer to structure.
 */
extern C_CDD_EXPORT void audit_stats_init(struct AuditStats *stats);

/**
 * @brief Recursively audit a C project directory for allocation safety.
 *
 * Scans all `.c` files.
 *
 * @param[in] root_path The root directory of the project.
 * @param[out] stats The statistics structure to update.
 * @return 0 on success, error code on failure.
 */
extern C_CDD_EXPORT int audit_project(const char *root_path,
                                      struct AuditStats *stats);

/**
 * @brief Generate a JSON report string from audit stats.
 * Caller must free the returned string.
 *
 * @param[in] stats The statistics.
 * @return Allocated string containing JSON, or NULL on error.
 */
extern C_CDD_EXPORT char *audit_print_json(const struct AuditStats *stats);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* PROJECT_AUDIT_H */
