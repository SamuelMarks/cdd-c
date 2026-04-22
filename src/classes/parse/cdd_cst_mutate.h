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

int cdd_cst_splice_children(cdd_cst_tree_t *tree, cdd_cst_node_t **node_ptr,
                            size_t start_idx, size_t consume_count,
                            cdd_cst_child_t *new_children,
                            size_t new_children_count);
cdd_cst_node_t *cdd_cst_find_node_for_token(cdd_cst_node_t *root,
                                            cdd_token_t *tok, size_t *out_idx);

#ifdef __cplusplus
}
#endif /* __cplusplus */

/**
 * @brief Removes a child from a node's children array in-place.
 * @param node The node to modify.
 * @param idx The index of the child to remove.
 * @return 0 on success.
 */
int cdd_cst_remove_child(cdd_cst_node_t *node, size_t idx);

/**
 * @brief Replaces a token child with a new token in-place.
 * @param node The node to modify.
 * @param idx The index of the child to replace.
 * @param new_tok The new token.
 * @return 0 on success.
 */
int cdd_cst_replace_token_child(cdd_cst_node_t *node, size_t idx,
                                cdd_token_t *new_tok);
#endif /* CDD_CST_MUTATE_H */
