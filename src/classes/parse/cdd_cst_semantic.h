#ifndef CDD_CST_SEMANTIC_H
#define CDD_CST_SEMANTIC_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include "cdd_cst_node.h"
#include "cdd_cst_scope.h"
/* clang-format on */

/**
 * @brief Build semantic scope and symbol information from a CST.
 * @param tree The CST tree to analyze.
 * @param out_env The populated scope environment.
 * @return 0 on success.
 */
C_CDD_EXPORT int cdd_cst_build_semantic_info(cdd_cst_tree_t *tree,
                                             cdd_cst_scope_env_t **out_env);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CDD_CST_SEMANTIC_H */
