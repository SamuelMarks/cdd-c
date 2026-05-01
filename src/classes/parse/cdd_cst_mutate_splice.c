/* clang-format off */
#include "cdd_cst_mutate.h"
#include "cdd_cst_factory.h"
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "c_cdd/log.h"
/* clang-format on */

int cdd_cst_splice_children(cdd_cst_tree_t *tree, cdd_cst_node_t **node_ptr,
                            size_t start_idx, size_t consume_count,
                            cdd_cst_child_t *new_children,
                            size_t new_children_count) {
  cdd_cst_node_t *node = node_ptr ? *node_ptr : NULL;
  cdd_cst_node_t *new_node = NULL;
  size_t i;
  int rc;
  if (!tree || !node)
    return EINVAL;

  cdd_cst_alloc_node(node->kind, &new_node);
  if (!new_node) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return ENOMEM;
  }

  for (i = 0; i < start_idx; i++) {
    if (node->children[i].kind == CDD_CST_CHILD_TOKEN) {
      cdd_cst_append_child_token(new_node, node->children[i].val.token);
    } else {
      cdd_cst_append_child_node(new_node, node->children[i].val.node);
    }
  }

  for (i = 0; i < new_children_count; i++) {
    if (new_children[i].kind == CDD_CST_CHILD_TOKEN) {
      cdd_cst_append_child_token(new_node, new_children[i].val.token);
    } else {
      cdd_cst_append_child_node(new_node, new_children[i].val.node);
    }
  }

  for (i = start_idx + consume_count; i < node->num_children; i++) {
    if (node->children[i].kind == CDD_CST_CHILD_TOKEN) {
      cdd_cst_append_child_token(new_node, node->children[i].val.token);
    } else {
      cdd_cst_append_child_node(new_node, node->children[i].val.node);
    }
  }

  rc = cdd_cst_replace_node(tree, node, new_node);
  if (rc == 0 && node_ptr)
    *node_ptr = new_node;
  return rc;
}

int cdd_cst_find_node_for_token(cdd_cst_node_t *root, cdd_token_t *tok,
                                size_t *out_idx, cdd_cst_node_t **out_node) {
  size_t i;
  if (!out_node)
    return EINVAL;
  *out_node = NULL;
  if (!root || !tok || !out_idx)
    return EINVAL;

  for (i = 0; i < root->num_children; i++) {
    if (root->children[i].kind == CDD_CST_CHILD_TOKEN &&
        root->children[i].val.token == tok) {
      *out_idx = i;
      *out_node = root;
      return 0;
    } else if (root->children[i].kind == CDD_CST_CHILD_NODE) {
      cdd_cst_node_t *found = NULL;
      if (cdd_cst_find_node_for_token(root->children[i].val.node, tok, out_idx,
                                      &found) == 0 &&
          found) {
        *out_node = found;
        return 0;
      }
    }
  }
  return ENOENT;
}
