/**
 * @file cdd_cst_mutate_splice.c
 * @brief Implementation of CST splicing and searching functions.
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "c_cdd/log.h"
#include "cdd_cst_factory.h"
#include "cdd_cst_mutate.h"
#include "cdd_cst_scope.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
/* clang-format on */

/**
 * @brief Frees a CST node shell without freeing children (`*_free` alias).
 *
 * @param[in] node CST node shell to free.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t cst_node_only_free(cdd_cst_node_t *node) {
  return cdd_cst_free_node_only(node);
}

/**
 * @brief Clears `length` on any `tree->base_tokens` entries owned by a
 * spliced-out CST child and frees any consumed child CST subtrees.
 *
 * @param[in,out] tree Owning CST tree.
 * @param[in] ch Consumed CST child entry.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t cst_consumed_child_free(cdd_cst_tree_t *tree,
                                             const cdd_cst_child_t *ch) {
  cdd_c_error_t rc_free = CDD_C_SUCCESS;
  if (ch->kind == CDD_CST_CHILD_TOKEN) {
    if (tree->base_tokens) {
      cdd_token_t *bt = ch->val.token;
      if (bt && bt >= tree->base_tokens->tokens &&
          bt < tree->base_tokens->tokens + tree->base_tokens->size) {
        bt->start = (const uint8_t *)"";
        bt->length = 0;
      }
    }
  } else if (ch->val.node) {
    size_t k;
    for (k = 0; k < ch->val.node->num_children; k++) {
      rc_free =
          (cdd_c_error_t)((int)rc_free | (int)cst_consumed_child_free(
                                             tree, &ch->val.node->children[k]));
    }
    rc_free =
        (cdd_c_error_t)((int)rc_free | (int)cst_node_only_free(ch->val.node));
  }
  return rc_free;
}

/*
 * @brief Splice children of a node.
 *
 * @param[in,out] tree Owning CST tree.
 * @param[in,out] node_ptr Pointer to the CST node pointer being spliced.
 * @param[in] start_idx Starting child index for replacement.
 * @param[in] consume_count Number of existing children to replace.
 * @param[in] new_children Array of new child entries to insert.
 * @param[in] new_children_count Number of entries in `new_children`.
 * @return 0 on success.
 */
cdd_c_error_t cdd_cst_splice_children(cdd_cst_tree_t *tree,
                                      cdd_cst_node_t **node_ptr,
                                      size_t start_idx, size_t consume_count,
                                      cdd_cst_child_t *new_children,
                                      size_t new_children_count) {
  cdd_cst_node_t *node = node_ptr ? *node_ptr : NULL;
  cdd_cst_node_t *new_node = NULL;
  size_t i;
  cdd_c_error_t rc;
  cdd_c_error_t rc_free = CDD_C_SUCCESS;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  if (!tree || !node)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (start_idx > node->num_children ||
      consume_count > node->num_children - start_idx) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  rc = cdd_cst_alloc_node(node->kind, &new_node);
  if (rc != CDD_C_SUCCESS) {
    C_CDD_LOG_DEBUG("cdd_cst_alloc_node failed: %d\n", (int)rc);
    return rc;
  }

  for (i = 0; i < start_idx; i++) {
    if (node->children[i].kind == CDD_CST_CHILD_TOKEN) {
      rc = cdd_cst_append_child_token(new_node, node->children[i].val.token);
      if (rc != CDD_C_SUCCESS)
        goto fail;
    } else {
      rc = cdd_cst_append_child_node(new_node, node->children[i].val.node);
      if (rc != CDD_C_SUCCESS)
        goto fail;
    }
  }

  for (i = 0; i < new_children_count; i++) {
    if (new_children[i].kind == CDD_CST_CHILD_TOKEN) {
      rc = cdd_cst_append_child_token(new_node, new_children[i].val.token);
      if (rc != CDD_C_SUCCESS)
        goto fail;
    } else {
      rc = cdd_cst_append_child_node(new_node, new_children[i].val.node);
      if (rc != CDD_C_SUCCESS)
        goto fail;
    }
  }

  for (i = start_idx + consume_count; i < node->num_children; i++) {
    if (node->children[i].kind == CDD_CST_CHILD_TOKEN) {
      rc = cdd_cst_append_child_token(new_node, node->children[i].val.token);
      if (rc != CDD_C_SUCCESS)
        goto fail;
    } else {
      rc = cdd_cst_append_child_node(new_node, node->children[i].val.node);
      if (rc != CDD_C_SUCCESS)
        goto fail;
    }
  }

  if (tree->active_sem_env) {
    rc = cdd_cst_scope_remap_ast_node(tree->active_sem_env, node, new_node);
    if (rc != CDD_C_SUCCESS)
      goto fail;
  }

  if (node == tree->root) {
    tree->root = new_node;
  } else {
    rc = cdd_cst_replace_node(tree, node, new_node);
    if (rc != CDD_C_SUCCESS)
      goto fail;
  }

  if (consume_count > 0 && new_children_count > 0 &&
      node->children[start_idx].kind == CDD_CST_CHILD_TOKEN &&
      node->children[start_idx].val.token &&
      new_children[0].kind == CDD_CST_CHILD_TOKEN) {
    cdd_token_t *old_t = node->children[start_idx].val.token;
    cdd_token_t *rep_t = new_children[0].val.token;
    rep_t->line = old_t->line;
    rep_t->column = old_t->column;
    rep_t->offset = old_t->offset;
  }

  for (i = start_idx; i < start_idx + consume_count; i++) {
    rc_free = (cdd_c_error_t)((int)rc_free | (int)cst_consumed_child_free(
                                                 tree, &node->children[i]));
  }
  rc_free = (cdd_c_error_t)((int)rc_free | (int)cst_node_only_free(node));
  *node_ptr = new_node;
  return rc_free;

fail: {
  cdd_c_error_t rc_f = cst_node_only_free(new_node);
  return cdd_c_keep_err(rc, rc_f);
}
}

/*
 * @brief Finds a node containing a given token.
 *
 * @param[in] root Root CST node to search.
 * @param[in] tok Target token pointer to locate.
 * @param[out] out_idx Output pointer receiving the child index within the
 * parent node.
 * @param[out] out_node Output pointer receiving the parent CST node containing
 * `tok`.
 * @return 0 on success.
 */
cdd_c_error_t cdd_cst_find_node_for_token(cdd_cst_node_t *root,
                                          cdd_token_t *tok, size_t *out_idx,
                                          cdd_cst_node_t **out_node) {
  size_t i;
  cdd_c_error_t rc;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  if (!out_node)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_node = NULL;
  if (!root || !tok || !out_idx)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  for (i = 0; i < root->num_children; i++) {
    if (root->children[i].kind == CDD_CST_CHILD_TOKEN &&
        root->children[i].val.token &&
        (root->children[i].val.token == tok ||
         (tok->line != 0 && root->children[i].val.token->line == tok->line &&
          root->children[i].val.token->column == tok->column &&
          root->children[i].val.token->offset == tok->offset))) {
      *out_idx = i;
      *out_node = root;
      return CDD_C_SUCCESS;
    } else if (root->children[i].kind == CDD_CST_CHILD_NODE) {
      cdd_cst_node_t *found = NULL;
      rc = cdd_cst_find_node_for_token(root->children[i].val.node, tok, out_idx,
                                       &found);
      if (rc != CDD_C_SUCCESS && rc != CDD_C_ERROR_NOT_FOUND)
        return rc;
      if (rc == CDD_C_SUCCESS) {
        *out_node = found;
        return CDD_C_SUCCESS;
      }
    }
  }
  return CDD_C_ERROR_NOT_FOUND;
}
