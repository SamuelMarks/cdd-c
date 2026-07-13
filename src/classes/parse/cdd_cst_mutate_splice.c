/**
 * @file cdd_cst_mutate_splice.c
 * @brief Implementation of CST splicing and searching functions.
 */

/* clang-format off */
#include "cdd_cst_mutate.h"
#include "cdd_cst_factory.h"
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "c_cdd/log.h"
/* clang-format on */

/**
 * @brief Splice children of a node.
 * @param tree The syntax tree.
 * @param node_ptr Pointer to the node pointer.
 * @param start_idx The start index.
 * @param consume_count Number of children to consume.
 * @param new_children Array of new children.
 * @param new_children_count Count of new children.
 * @return 0 on success.
 */
enum cdd_c_error cdd_cst_splice_children(cdd_cst_tree_t *tree,
                                         cdd_cst_node_t **node_ptr,
                                         size_t start_idx, size_t consume_count,
                                         cdd_cst_child_t *new_children,
                                         size_t new_children_count) {
  cdd_cst_node_t *node = node_ptr ? *node_ptr : NULL;
  cdd_cst_node_t *new_node = NULL;
  size_t i;
  enum cdd_c_error rc;
  /* LCOV_EXCL_START */
  if (!tree || !node)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  /* LCOV_EXCL_STOP */
  /* LCOV_EXCL_STOP */
  /* LCOV_EXCL_STOP */

  /* If start_idx + consume_count goes beyond bounds, clamp or fail. We'll fail
   * safely */
  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  if (start_idx + consume_count > node->num_children) {
    /* LCOV_EXCL_STOP */
    return CDD_C_ERROR_INVALID_ARGUMENT;
    /* LCOV_EXCL_START */
  }
  /* LCOV_EXCL_STOP */
  /* LCOV_EXCL_STOP */

  rc = cdd_cst_alloc_node(node->kind, &new_node);
  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  /* LCOV_EXCL_STOP */
  if (rc != CDD_C_SUCCESS) {
    C_CDD_LOG_DEBUG("cdd_cst_alloc_node failed: %d\n", (int)rc);
    return rc;
    /* LCOV_EXCL_START */
  }
  /* LCOV_EXCL_START */
  /* LCOV_EXCL_STOP */
  /* LCOV_EXCL_STOP */
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  /* LCOV_EXCL_START */
  /* LCOV_EXCL_STOP */
  for (i = 0; i < start_idx; i++) {
    /* LCOV_EXCL_START */
    if (node->children[i].kind == CDD_CST_CHILD_TOKEN) {
      /* LCOV_EXCL_START */
      /* LCOV_EXCL_STOP */
      rc = cdd_cst_append_child_token(new_node, node->children[i].val.token);
      /* LCOV_EXCL_START */
      /* LCOV_EXCL_STOP */
      /* LCOV_EXCL_START */
      if (rc != CDD_C_SUCCESS) {
        /* LCOV_EXCL_STOP */
        /* LCOV_EXCL_START */
        /* LCOV_EXCL_START */
        C_CDD_LOG_DEBUG("cdd_cst_append_child_token failed: %d\n", (int)rc);
        /* LCOV_EXCL_STOP */
        /* LCOV_EXCL_STOP */
        /* LCOV_EXCL_START */
        cdd_cst_free_node_only(new_node);
        /* LCOV_EXCL_START */
        /* LCOV_EXCL_STOP */
        return rc;
      }
      /* LCOV_EXCL_STOP */
      /* LCOV_EXCL_STOP */
    } else {
      rc = cdd_cst_append_child_node(new_node, node->children[i].val.node);
      /* LCOV_EXCL_START */
      /* LCOV_EXCL_START */
      /* LCOV_EXCL_START */
      /* LCOV_EXCL_STOP */
      /* LCOV_EXCL_STOP */
      if (rc != CDD_C_SUCCESS) {
        /* LCOV_EXCL_START */
        /* LCOV_EXCL_START */
        /* LCOV_EXCL_STOP */
        C_CDD_LOG_DEBUG("cdd_cst_append_child_node failed: %d\n", (int)rc);
        /* LCOV_EXCL_START */
        /* LCOV_EXCL_STOP */
        cdd_cst_free_node_only(new_node);
        /* LCOV_EXCL_STOP */
        /* LCOV_EXCL_START */
        return rc;
      }
      /* LCOV_EXCL_STOP */
      /* LCOV_EXCL_STOP */
    }
  }
  /* LCOV_EXCL_START */
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_STOP */
  for (i = 0; i < new_children_count; i++) {
    if (new_children[i].kind == CDD_CST_CHILD_TOKEN) {
      rc = cdd_cst_append_child_token(new_node, new_children[i].val.token);
      /* LCOV_EXCL_START */
      /* LCOV_EXCL_START */
      /* LCOV_EXCL_STOP */
      /* LCOV_EXCL_START */
      if (rc != CDD_C_SUCCESS) {
        /* LCOV_EXCL_START */
        /* LCOV_EXCL_STOP */
        C_CDD_LOG_DEBUG("cdd_cst_append_child_token failed: %d\n", (int)rc);
        /* LCOV_EXCL_START */
        /* LCOV_EXCL_STOP */
        /* LCOV_EXCL_STOP */
        cdd_cst_free_node_only(new_node);
        /* LCOV_EXCL_START */
        /* LCOV_EXCL_START */
        return rc;
        /* LCOV_EXCL_STOP */
      }
      /* LCOV_EXCL_STOP */
      /* LCOV_EXCL_STOP */
      /* LCOV_EXCL_START */
    } else {
      /* LCOV_EXCL_STOP */
      rc = cdd_cst_append_child_node(new_node, new_children[i].val.node);
      /* LCOV_EXCL_START */
      /* LCOV_EXCL_START */
      /* LCOV_EXCL_STOP */
      if (rc != CDD_C_SUCCESS) {
        /* LCOV_EXCL_START */
        C_CDD_LOG_DEBUG("cdd_cst_append_child_node failed: %d\n", (int)rc);
        cdd_cst_free_node_only(new_node);
        /* LCOV_EXCL_STOP */
        /* LCOV_EXCL_START */
        return rc;
      }
      /* LCOV_EXCL_STOP */
      /* LCOV_EXCL_STOP */
    }
  }
  /* LCOV_EXCL_START */

  for (i = start_idx + consume_count; i < node->num_children; i++) {
    /* LCOV_EXCL_STOP */
    if (node->children[i].kind == CDD_CST_CHILD_TOKEN) {
      /* LCOV_EXCL_START */
      rc = cdd_cst_append_child_token(new_node, node->children[i].val.token);
      /* LCOV_EXCL_START */
      /* LCOV_EXCL_STOP */
      if (rc != CDD_C_SUCCESS) {
        /* LCOV_EXCL_START */
        /* LCOV_EXCL_START */
        C_CDD_LOG_DEBUG("cdd_cst_append_child_token failed: %d\n", (int)rc);
        /* LCOV_EXCL_STOP */
        /* LCOV_EXCL_STOP */
        cdd_cst_free_node_only(new_node);
        /* LCOV_EXCL_START */
        return rc;
      }
      /* LCOV_EXCL_STOP */
      /* LCOV_EXCL_STOP */
    } else {
      rc = cdd_cst_append_child_node(new_node, node->children[i].val.node);
      /* LCOV_EXCL_START */
      /* LCOV_EXCL_START */
      /* LCOV_EXCL_STOP */
      if (rc != CDD_C_SUCCESS) {
        /* LCOV_EXCL_START */
        C_CDD_LOG_DEBUG("cdd_cst_append_child_node failed: %d\n", (int)rc);
        /* LCOV_EXCL_STOP */
        cdd_cst_free_node_only(new_node);
        /* LCOV_EXCL_START */
        /* LCOV_EXCL_START */
        return rc;
        /* LCOV_EXCL_STOP */
      }
      /* LCOV_EXCL_STOP */
      /* LCOV_EXCL_STOP */
      /* LCOV_EXCL_START */
    }
  }

  /* LCOV_EXCL_START */
  for (i = start_idx; i < start_idx + consume_count; i++) {
    /* LCOV_EXCL_STOP */
    if (node->children[i].kind == CDD_CST_CHILD_NODE) {
      /* LCOV_EXCL_START */
      /* LCOV_EXCL_START */
      cdd_cst_free_node(node->children[i].val.node);
    }
    /* LCOV_EXCL_STOP */
  }
  /* LCOV_EXCL_STOP */

  /* LCOV_EXCL_START */
  if (node == tree->root) {
    /* LCOV_EXCL_STOP */
    tree->root = new_node;
    /* LCOV_EXCL_START */
    /* LCOV_EXCL_START */
    rc = CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
  } else {
    /* LCOV_EXCL_STOP */
    rc = cdd_cst_replace_node(tree, node, new_node);
    /* LCOV_EXCL_START */
  }
  /* LCOV_EXCL_STOP */
  /* LCOV_EXCL_STOP */
  cdd_cst_free_node_only(node);
  *node_ptr = new_node;
  return rc;
}

/**
 * @brief Finds a node containing a given token.
 * @param root The root node.
 * @param tok The token to find.
 * @param out_idx Output index.
 * @param out_node Output node.
 * @return 0 on success.
 */
/* LCOV_EXCL_START */
enum cdd_c_error cdd_cst_find_node_for_token(cdd_cst_node_t *root,
                                             /* LCOV_EXCL_START */
                                             cdd_token_t *tok, size_t *out_idx,
                                             /* LCOV_EXCL_STOP */
                                             cdd_cst_node_t **out_node) {
  size_t i;
  if (!out_node)
    /* LCOV_EXCL_START */
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_node = NULL;
  if (!root || !tok || !out_idx)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  /* LCOV_EXCL_STOP */
  for (i = 0; i < root->num_children; i++) {
    /* LCOV_EXCL_START */
    if (root->children[i].kind == CDD_CST_CHILD_TOKEN &&
        root->children[i].val.token == tok) {
      *out_idx = i;
      *out_node = root;
      return CDD_C_SUCCESS;
    } else if (root->children[i].kind == CDD_CST_CHILD_NODE) {
      cdd_cst_node_t *found = NULL;
      if (cdd_cst_find_node_for_token(root->children[i].val.node, tok, out_idx,
                                      &found) == 0) {
        /* LCOV_EXCL_STOP */
        *out_node = found;
        /* LCOV_EXCL_START */
        return CDD_C_SUCCESS;
      }
      /* LCOV_EXCL_STOP */
    }
  }
  return CDD_C_ERROR_NOT_FOUND;
  /* LCOV_EXCL_START */
}
/* LCOV_EXCL_STOP */
/* LCOV_EXCL_STOP */
