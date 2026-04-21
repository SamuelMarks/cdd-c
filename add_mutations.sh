cat << 'H_EOF' >> src/classes/parse/cdd_cst_mutate.h

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
int cdd_cst_replace_token_child(cdd_cst_node_t *node, size_t idx, cdd_token_t *new_tok);
H_EOF

cat << 'C_EOF' >> src/classes/parse/cdd_cst_mutate.c

int cdd_cst_remove_child(cdd_cst_node_t *node, size_t idx) {
    size_t i;
    if (!node || idx >= node->num_children) return EINVAL;
    for (i = idx; i + 1 < node->num_children; i++) {
        node->children[i] = node->children[i + 1];
    }
    node->num_children--;
    return 0;
}

int cdd_cst_replace_token_child(cdd_cst_node_t *node, size_t idx, cdd_token_t *new_tok) {
    if (!node || idx >= node->num_children || !new_tok) return EINVAL;
    if (node->children[idx].kind != CDD_CST_CHILD_TOKEN) return EINVAL;
    node->children[idx].val.token = new_tok;
    return 0;
}
C_EOF
