#ifndef CDD_CST_PARSER_H
#define CDD_CST_PARSER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "cdd_cst_node.h"
#include <c_str_span.h>
/* clang-format on */

/**
 * @brief Parse tokens into a loss-less CST.
 *
 * @param source Code source.
 * @param out_tree The generated tree containing nodes and token ownership.
 * @return 0 on success, or ENOMEM/EINVAL.
 */
int cdd_cst_parse(az_span source, cdd_cst_tree_t **out_tree);

/**
 * @brief Free the Concrete Syntax Tree and its constituent structures.
 *
 * @param tree Tree to free.
 */
void cdd_cst_tree_free(cdd_cst_tree_t *tree);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CDD_CST_PARSER_H */
