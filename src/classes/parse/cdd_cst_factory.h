#ifndef CDD_CST_FACTORY_H
#define CDD_CST_FACTORY_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include "cdd_cst_node.h"
#include <stddef.h>
/* clang-format on */

/**
 * @brief Allocates a new empty node of the given kind.
 * @param kind The node kind.
 * @param out_node The new node.
 * @return 0 on success.
 */
C_CDD_EXPORT int cdd_cst_alloc_node(enum cdd_cst_node_kind_t kind,
                                    cdd_cst_node_t **out_node);

/**
 * @brief Creates a synthetic token. The text must be a static string literal or
 * outlive the tree!
 * @param tree The CST tree.
 * @param kind The token kind.
 * @param text The literal string text.
 * @param out_token The new token.
 * @return 0 on success.
 */
C_CDD_EXPORT int cdd_cst_create_token(cdd_cst_tree_t *tree,
                                      enum cdd_token_kind_t kind,
                                      const char *text,
                                      cdd_token_t **out_token);

/**
 * @brief Creates a synthetic token from a non-null terminated buffer.
 * @param tree The CST tree.
 * @param kind The token kind.
 * @param text The text buffer.
 * @param length The text length.
 * @param out_token The new token.
 * @return 0 on success.
 */
C_CDD_EXPORT int cdd_cst_create_token_len(cdd_cst_tree_t *tree,
                                          enum cdd_token_kind_t kind,
                                          const char *text, size_t length,
                                          cdd_token_t **out_token);

/**
 * @brief Appends a child node to a parent node.
 * @param parent The parent node.
 * @param child The child node.
 * @return 0 on success.
 */
C_CDD_EXPORT int cdd_cst_append_child_node(cdd_cst_node_t *parent,
                                           cdd_cst_node_t *child);

/**
 * @brief Appends a child token to a parent node.
 * @param parent The parent node.
 * @param token The child token.
 * @return 0 on success.
 */
C_CDD_EXPORT int cdd_cst_append_child_token(cdd_cst_node_t *parent,
                                            cdd_token_t *token);

/**
 * @brief Recursively frees a node and all of its descendant nodes.
 * @param node The node to free.
 */
C_CDD_EXPORT void cdd_cst_free_node(cdd_cst_node_t *node);

#ifdef __cplusplus
}
#endif /* __cplusplus */

C_CDD_EXPORT int cdd_cst_parse_format(cdd_cst_tree_t *dest_tree,
                                      cdd_cst_node_t **out_node,
                                      const char *fmt, ...);
C_CDD_EXPORT void cdd_cst_free_node_only(cdd_cst_node_t *node);
#endif /* CDD_CST_FACTORY_H */
