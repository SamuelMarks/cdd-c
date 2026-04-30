#ifndef CDD_CST_TRIVIA_H
#define CDD_CST_TRIVIA_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include "cdd_cst_node.h"
#include <stddef.h>
/* clang-format on */

/**
 * @brief Configuration for trivia generators.
 */
typedef struct cdd_cst_format_config_t cdd_cst_format_config_t;
/** @brief Struct definition */
struct cdd_cst_format_config_t {
  /** @brief field */
  /** @brief field */
  int use_tabs;
  /** @brief field */
  /** @brief field */
  size_t indent_width;
};

/**
 * @brief Analyzes a tree to detect indentation configuration.
 * @param tree The CST tree.
 * @param out_config Populated format configuration.
 * @return 0 on success.
 */
C_CDD_EXPORT int
cdd_cst_detect_format_config(cdd_cst_tree_t *tree,
                             cdd_cst_format_config_t *out_config);

/**
 * @brief Generates trivia nodes (newlines and indentation).
 * @param tree The tree (to track synthesized tokens if needed).
 * @param config Formatting config.
 * @param indent_level How many levels to indent.
 * @param out_trivia Head of the generated trivia list.
 * @return 0 on success.
 */
C_CDD_EXPORT int
cdd_cst_generate_indent_trivia(cdd_cst_tree_t *tree,
                               const cdd_cst_format_config_t *config,
                               size_t indent_level, cdd_trivia_t **out_trivia);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CDD_CST_TRIVIA_H */
