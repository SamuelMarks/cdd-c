#ifndef CDD_CST_PARSER_H
#define CDD_CST_PARSER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "cdd_cst_node.h"
#include <c_str_span.h>
#include "c_cdd_export.h"
#include "cdd_c_error.h"
/* clang-format on */

/**
 * @brief Parse tokens into a loss-less CST.
 *
 * @param source Code source.
 * @param out_tree The generated tree containing nodes and token ownership.
 * @return 0 on success, or ENOMEM/EINVAL.
 */
C_CDD_EXPORT cdd_c_error_t cdd_cst_parse(az_span source,
                                         cdd_cst_tree_t **out_tree);

/**
 * @brief Free the Concrete Syntax Tree and its constituent structures.
 *
 * @param tree Tree to free.
 */
C_CDD_EXPORT void cdd_cst_tree_free(cdd_cst_tree_t *tree);

/**
 * @brief Find the class identifier token for an ancestor class declaration.
 *
 * @param[in] node Starting node.
 * @param[out] out_tok Pointer to store found identifier token or NULL.
 * @return CDD_C_SUCCESS on success, CDD_C_ERROR_INVALID_ARGUMENT if out_tok is
 * NULL.
 */
C_CDD_EXPORT cdd_c_error_t get_class_name(cdd_cst_node_t *node,
                                          cdd_token_t **out_tok);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CDD_CST_PARSER_H */
