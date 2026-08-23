#if defined(__clang__) || defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverlength-strings"
#pragma GCC diagnostic ignored "-Wlong-long"
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif
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

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CDD_CST_PARSER_H */

#if defined(__clang__) || defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
