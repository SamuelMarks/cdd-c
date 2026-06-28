#ifndef CDD_CST_ESCAPE_H
#define CDD_CST_ESCAPE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include "cdd_c_error.h"
#include "cdd_cst_node.h"
#include "cdd_cst_scope.h"
/* clang-format on */

/**
 * @brief Analyzes a CST tree to determine which local variables have their
 * addresses taken or are captured by nested functions (closures).
 * @param tree The AST to analyze.
 * @param env The populated semantic scope environment.
 * @return 0 on success.
 */
C_CDD_EXPORT enum cdd_c_error cdd_cst_analyze_escapes(cdd_cst_tree_t *tree,
                                                      cdd_cst_scope_env_t *env);

/**
 * @brief Checks if a specific symbol escapes its local scope.
 * @param symbol The symbol to check.
 * @param out_escapes Populated with 1 if it escapes, 0 otherwise.
 * @return 0 on success.
 */
C_CDD_EXPORT enum cdd_c_error cdd_cst_symbol_escapes(cdd_cst_symbol_t *symbol,
                                                     int *out_escapes);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CDD_CST_ESCAPE_H */
