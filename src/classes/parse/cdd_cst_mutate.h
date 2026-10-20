/**
 * @file cdd_cst_mutate.h
 * @brief Concrete Syntax Tree (CST) mutate implementation and utilities.
 */
#ifndef CDD_CST_MUTATE_H
#define CDD_CST_MUTATE_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
/* clang-format off */
#include "c_cdd_export.h"
#include "cdd_c_error.h"
#include "cdd_cst_node.h"
/* clang-format on */

/**
 * @brief Swaps old_node with new_node in the parent's children array.
 *        Original leading/trailing trivia of old_node is preserved and attached
 * to new_node.
 * @param[in,out] tree The CST tree.
 * @param[in,out] old_node The node to be replaced.
 * @param[in,out] new_node The new node to insert.
 * @return 0 on success.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_cst_replace_node(
    cdd_cst_tree_t *tree, cdd_cst_node_t *old_node, cdd_cst_node_t *new_node);

/**
 * @brief Inserts new_node into the parent's children array before target.
 * @param[in,out] target_node The node to insert before.
 * @param[in,out] new_node The node to insert.
 * @return 0 on success.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_cst_insert_node_before(
    cdd_cst_node_t *target_node, cdd_cst_node_t *new_node);

/**
 * @brief Inserts new_node into the parent's children array after target.
 * @param[in,out] target_node The node to insert after.
 * @param[in,out] new_node The node to insert.
 * @return 0 on success.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_cst_insert_node_after(
    cdd_cst_node_t *target_node, cdd_cst_node_t *new_node);

/**
 * @brief Detaches target node from its parent.
 *        Safely cleans up orphaned adjacent whitespace/newlines.
 * @param[in,out] tree The CST tree.
 * @param[in,out] node The node to detach.
 * @return 0 on success.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_cst_detach_node(cdd_cst_tree_t *tree,
                                                      cdd_cst_node_t *node);

/**
 * @brief Clones a tree deeply (including tokens and trivia).
 *        The tokens are added to the tree's synthesized_tokens array.
 * @param[in,out] tree The CST tree.
 * @param[in,out] root The root node to clone.
 * @param[out] out_clone The cloned node.
 * @return 0 on success.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_cst_clone_tree(
    cdd_cst_tree_t *tree, cdd_cst_node_t *root, cdd_cst_node_t **out_clone);

/**
 * @brief Splices a range of child entries in a CST node with new children.
 *
 * @param[in,out] tree The CST tree.
 * @param[in,out] node_ptr Pointer to the CST node whose children are spliced.
 * @param[in] start_idx Starting index of children to replace.
 * @param[in] consume_count Number of existing children to remove.
 * @param[in] new_children Array of new child entries to insert.
 * @param[in] new_children_count Number of new child entries.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_cst_splice_children(
    cdd_cst_tree_t *tree, cdd_cst_node_t **node_ptr, size_t start_idx,
    size_t consume_count, cdd_cst_child_t *new_children,
    size_t new_children_count);

/**
 * @brief Finds the CST node and child index that directly owns a given token.
 *
 * @param[in] root Root CST node to search.
 * @param[in] tok Token to locate.
 * @param[out] out_idx Optional pointer to receive the child index in the node.
 * @param[out] out_node Pointer to receive the parent CST node of the token.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
extern C_CDD_EXPORT cdd_c_error_t
cdd_cst_find_node_for_token(cdd_cst_node_t *root, cdd_token_t *tok,
                            size_t *out_idx, cdd_cst_node_t **out_node);

/**
 * @brief Finds the index of a child node within its parent's children array.
 * @param[in] parent Parent CST node.
 * @param[in] child Child CST node to locate.
 * @param[out] out_index Pointer to receive the child index.
 * @return CDD_C_SUCCESS on success or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t find_child_index_mutate(
    cdd_cst_node_t *parent, cdd_cst_node_t *child, size_t *out_index);

/**
 * @brief Finds the first lexical token within a CST subtree.
 * @param[in] node Root CST node of subtree.
 * @param[out] out_token Pointer to receive the first token.
 * @return CDD_C_SUCCESS on success or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t
find_first_token_mutate(cdd_cst_node_t *node, cdd_token_t **out_token);

/**
 * @brief Deep-clones a linked list of lexical trivia.
 * @param[in] head Head of the trivia list to clone.
 * @param[out] out_trivia Pointer to receive the cloned trivia list.
 * @return CDD_C_SUCCESS on success or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t
clone_trivia_list_mutate(cdd_trivia_t *head, cdd_trivia_t **out_trivia);

/**
 * @brief Deep-clones a single CST token including its leading and trailing
 * trivia.
 * @param[in] tok Token to clone.
 * @param[out] out_token Pointer to receive the cloned token.
 * @return CDD_C_SUCCESS on success or error code.
 */
extern cdd_c_error_t clone_token_mutate(cdd_token_t *tok,
                                        cdd_token_t **out_token);

/**
 * @brief Inserts a child CST node at a specific index in a parent node.
 * @param[in,out] parent Parent CST node.
 * @param[in] idx Index at which to insert the new child node.
 * @param[in] new_node Child CST node to insert.
 * @return CDD_C_SUCCESS on success or error code.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_cst_insert_child_node_at(
    cdd_cst_node_t *parent, size_t idx, cdd_cst_node_t *new_node);

/**
 * @brief Removes a child entry at a specific index from a parent CST node.
 * @param[in,out] parent Parent CST node.
 * @param[in] idx Index of the child entry to remove.
 * @return CDD_C_SUCCESS on success or error code.
 */
extern cdd_c_error_t remove_child_at_mutate(cdd_cst_node_t *parent, size_t idx);

/**
 * @brief Tracks a synthesized token in the CST tree's synthesized_tokens array.
 * @param[in,out] tree Target CST tree.
 * @param[in] tok Synthesized token to register for cleanup.
 * @return CDD_C_SUCCESS on success or error code.
 */
extern cdd_c_error_t track_synthesized_token_mutate(cdd_cst_tree_t *tree,
                                                    cdd_token_t *tok);

/**
 * @brief Removes a child from a node's children array in-place.
 * @param[in,out] node The node to modify.
 * @param[in] idx The index of the child to remove.
 * @return 0 on success.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_cst_remove_child(cdd_cst_node_t *node,
                                                       size_t idx);

/**
 * @brief Replaces a token child with a new token in-place.
 * @param[in,out] node The node to modify.
 * @param[in] idx The index of the child to replace.
 * @param[in,out] new_tok The new token.
 * @return 0 on success.
 */
extern C_CDD_EXPORT cdd_c_error_t cdd_cst_replace_token_child(
    cdd_cst_node_t *node, size_t idx, cdd_token_t *new_tok);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* CDD_CST_MUTATE_H */
