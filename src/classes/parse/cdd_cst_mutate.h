#ifndef CDD_CST_MUTATE_H
#define CDD_CST_MUTATE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "cdd_cst_node.h"
/* clang-format on */

/**
 * @brief Swaps old_node with new_node in the parent's children array.
 *        Original leading/trailing trivia of old_node is preserved and attached
 * to new_node.
 * @param tree The CST tree.
 * @param old_node The node to be replaced.
 * @param new_node The new node to insert.
 * @return 0 on success.
 */
int cdd_cst_replace_node(cdd_cst_tree_t *tree, cdd_cst_node_t *old_node,
                         cdd_cst_node_t *new_node);

/**
 * @brief Inserts new_node into the parent's children array before target.
 * @param target_node The node to insert before.
 * @param new_node The node to insert.
 * @return 0 on success.
 */
int cdd_cst_insert_node_before(cdd_cst_node_t *target_node,
                               cdd_cst_node_t *new_node);

/**
 * @brief Inserts new_node into the parent's children array after target.
 * @param target_node The node to insert after.
 * @param new_node The node to insert.
 * @return 0 on success.
 */
int cdd_cst_insert_node_after(cdd_cst_node_t *target_node,
                              cdd_cst_node_t *new_node);

/**
 * @brief Detaches target node from its parent.
 *        Safely cleans up orphaned adjacent whitespace/newlines.
 * @param tree The CST tree.
 * @param node The node to detach.
 * @return 0 on success.
 */
int cdd_cst_detach_node(cdd_cst_tree_t *tree, cdd_cst_node_t *node);

/**
 * @brief Clones a tree deeply (including tokens and trivia).
 *        The tokens are added to the tree's synthesized_tokens array.
 * @param tree The CST tree.
 * @param root The root node to clone.
 * @param out_clone The cloned node.
 * @return 0 on success.
 */
int cdd_cst_clone_tree(cdd_cst_tree_t *tree, cdd_cst_node_t *root,
                       cdd_cst_node_t **out_clone);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CDD_CST_MUTATE_H */
