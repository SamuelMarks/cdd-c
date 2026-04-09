#ifndef CDD_CST_TRANSFORM_H
#define CDD_CST_TRANSFORM_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "classes/parse/cdd_cst_node.h"
#include <stddef.h>
/* clang-format on */

/**
 * @brief Configuration for transformations.
 */
typedef struct cdd_transform_config_t {
  int use_tabs;     /**< 1 if tabs should be used for indentation */
  int indent_width; /**< Number of spaces (or tabs) per indentation level */
} cdd_transform_config_t;

/**
 * @brief Adds extern "C" wrapping to the parsed syntax tree.
 *
 * @param tree The CST tree.
 * @param config Optional configuration for trivia generation.
 * @return 0 on success, or an error code.
 */
int cdd_transform_extern_c(cdd_cst_tree_t *tree,
                           const cdd_transform_config_t *config);

/**
 * @brief Ports POSIX/GNU specific syntax to MSVC equivalents.
 *
 * @param tree The CST tree.
 * @param config Optional configuration for trivia generation.
 * @return 0 on success, or an error code.
 */
int cdd_transform_msvc(cdd_cst_tree_t *tree,
                       const cdd_transform_config_t *config);

/**
 * @brief Standardizes GNU-specific extensions (like __attribute__) into
 * standard C.
 *
 * @param tree The CST tree.
 * @param config Optional configuration for trivia generation.
 * @return 0 on success, or an error code.
 */
int cdd_transform_gnu(cdd_cst_tree_t *tree,
                      const cdd_transform_config_t *config);

/**
 * @brief Percolates errors by rewriting function signatures and returning int.
 *
 * @param tree The CST tree.
 * @param config Optional configuration for trivia generation.
 * @return 0 on success, or an error code.
 */
int cdd_transform_percolate_errors(cdd_cst_tree_t *tree,
                                   const cdd_transform_config_t *config);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CDD_CST_TRANSFORM_H */
