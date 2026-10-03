/**
 * @file cdd_cst_builder_internal.h
 * @brief Internal declarations for CST builder.
 *
 * @author Samuel Marks
 */

#ifndef CDD_CST_BUILDER_INTERNAL_H
#define CDD_CST_BUILDER_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/* clang-format off */
#include "c_cdd_export.h"
#include "cdd_c_error.h"
#include "cdd_cst_builder.h"
/* clang-format on */

/**
 * @brief Duplicates and pools a string inside the CST tree.
 *
 * @param[in,out] tree The CST tree.
 * @param[in] str The input string.
 * @param[out] out_str The pooled string pointer.
 * @return CDD_C_SUCCESS on success, or error code.
 */
cdd_c_error_t cdd_cst_pool_string(cdd_cst_tree_t *tree, const char *str,
                                  const char **out_str);

/**
 * @brief Gets the last leaf token under a CST node.
 *
 * @param[in] node The root or parent CST node.
 * @param[out] out_tok Pointer to receive the token.
 * @return CDD_C_SUCCESS on success, or error code.
 */
cdd_c_error_t cdd_cst_get_last_token(cdd_cst_node_t *node,
                                     cdd_token_t **out_tok);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !CDD_CST_BUILDER_INTERNAL_H */
