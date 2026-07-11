/* clang-format off */
#include "cdd_cst_mutate.h"
#include "cdd_cst_factory.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "c_cdd/log.h"
/* clang-format on */
/* LCOV_EXCL_START */

enum cdd_c_error find_child_index_mutate(cdd_cst_node_t *parent,
                                         cdd_cst_node_t *child,
                                         size_t *out_index) {
  size_t i;
  if (!parent || !child)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  for (i = 0; i < parent->num_children; i++) {
    if (parent->children[i].kind == CDD_CST_CHILD_NODE &&
        parent->children[i].val.node == child) {
      *out_index = i;
      return CDD_C_SUCCESS;
    }
  }
  return CDD_C_ERROR_NOT_FOUND;
}

enum cdd_c_error find_first_token_mutate(cdd_cst_node_t *node,
                                         cdd_token_t **out_token) {
  size_t i;
  if (!node || !out_token)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_token = NULL;
  for (i = 0; i < node->num_children; i++) {
    if (node->children[i].kind == CDD_CST_CHILD_TOKEN) {
      *out_token = node->children[i].val.token;
      return CDD_C_SUCCESS;
    } else {
      cdd_token_t *t = NULL;
      if (find_first_token_mutate(node->children[i].val.node, &t) == 0) {
        *out_token = t;
        return CDD_C_SUCCESS;
      }
    }
  }
  return CDD_C_ERROR_NOT_FOUND;
}

static enum cdd_c_error find_last_token_mutate(cdd_cst_node_t *node,
                                               cdd_token_t **out_tok) {
  size_t i;
  *out_tok = NULL;
  for (i = node->num_children; i > 0; i--) {
    size_t idx = i - 1;
    if (node->children[idx].kind == CDD_CST_CHILD_TOKEN) {
      *out_tok = node->children[idx].val.token;
      return CDD_C_SUCCESS;
    } else {
      cdd_token_t *t = NULL;
      if (find_last_token_mutate(node->children[idx].val.node, &t) == 0) {
        *out_tok = t;
        return CDD_C_SUCCESS;
      }
    }
  }
  return CDD_C_ERROR_NOT_FOUND;
}

enum cdd_c_error clone_trivia_list_mutate(cdd_trivia_t *head,
                                          cdd_trivia_t **out_trivia) {
  cdd_trivia_t *new_head = NULL;
  cdd_trivia_t *tail = NULL;
  if (!out_trivia)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_trivia = NULL;
  while (head) {
    cdd_trivia_t *t;
#ifdef CDD_BUILD_TESTS
    extern int g_cdd_cst_alloc_token_fail;
    if (g_cdd_cst_alloc_token_fail && --g_cdd_cst_alloc_token_fail == 0)
      t = NULL;
    else
#endif
      t = (cdd_trivia_t *)calloc(1, sizeof(cdd_trivia_t));
    if (!t) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      /* Need to properly loop and free it */
      while (new_head) {
        cdd_trivia_t *tmp = new_head;
        new_head = new_head->next;
        free(tmp);
      }
      return CDD_C_ERROR_MEMORY;
    }
    t->kind = head->kind;
    t->start = head->start;
    t->length = head->length;
    if (!new_head) {
      new_head = t;
      tail = t;
    } else {
      tail->next = t;
      tail = t;
    }
    head = head->next;
  }
  *out_trivia = new_head;
  return CDD_C_SUCCESS;
}

enum cdd_c_error track_synthesized_token_mutate(cdd_cst_tree_t *tree,
                                                cdd_token_t *tok) {
  if (tree->num_synthesized >= tree->synthesized_capacity) {
    size_t new_cap =
        tree->synthesized_capacity == 0 ? 16 : tree->synthesized_capacity * 2;
    cdd_token_t **new_arr;
#ifdef CDD_BUILD_TESTS
    extern int g_cdd_cst_realloc_fail;
    if (g_cdd_cst_realloc_fail && --g_cdd_cst_realloc_fail == 0)
      new_arr = NULL;
    else
#endif
      new_arr = (cdd_token_t **)realloc(tree->synthesized_tokens,
                                        new_cap * sizeof(cdd_token_t *));
    if (!new_arr) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      return CDD_C_ERROR_MEMORY;
    }
    tree->synthesized_tokens = new_arr;
    tree->synthesized_capacity = new_cap;
  }
  tree->synthesized_tokens[tree->num_synthesized++] = tok;
  return CDD_C_SUCCESS;
}

enum cdd_c_error cdd_cst_replace_node(cdd_cst_tree_t *tree,
                                      cdd_cst_node_t *old_node,
                                      cdd_cst_node_t *new_node) {
  cdd_cst_node_t *parent;
  size_t idx;
  int rc;
  cdd_token_t *old_first, *old_last, *new_first, *new_last;

  if (!tree || !old_node || !new_node)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  parent = old_node->parent;
  if (!parent)
    return CDD_C_ERROR_INVALID_ARGUMENT; /* Cannot replace root directly this
                                            way */

  rc = find_child_index_mutate(parent, old_node, &idx);
  if (rc != 0)
    return rc;

  /* Swap trivia */
  if (find_first_token_mutate(old_node, &old_first) != 0)
    old_first = NULL;
  if (find_first_token_mutate(new_node, &new_first) != 0)
    new_first = NULL;
  if (old_first && new_first && !new_first->leading_trivia) {
    new_first->leading_trivia = old_first->leading_trivia;
    old_first->leading_trivia = NULL;
  }

  if (find_last_token_mutate(old_node, &old_last) != 0)
    old_last = NULL;
  if (find_last_token_mutate(new_node, &new_last) != 0)
    new_last = NULL;
  if (old_last && new_last && !new_last->trailing_trivia) {
    new_last->trailing_trivia = old_last->trailing_trivia;
    old_last->trailing_trivia = NULL;
  }

  parent->children[idx].val.node = new_node;
  new_node->parent = parent;

  old_node->parent = NULL;

  /* Note: old_node is NOT freed here. Caller's responsibility if they want to
   * discard it. */

  return CDD_C_SUCCESS;
}

enum cdd_c_error cdd_cst_insert_child_node_at(cdd_cst_node_t *parent,
                                              size_t idx,
                                              cdd_cst_node_t *new_node) {
  if (!parent || !new_node || idx > parent->num_children)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (parent->num_children >= parent->capacity) {
#ifdef CDD_BUILD_TESTS
    size_t new_cap = parent->capacity + 1;
#else
    size_t new_cap = parent->capacity == 0 ? 8 : parent->capacity * 2;
#endif
    cdd_cst_child_t *new_arr;
#ifdef CDD_BUILD_TESTS
    extern int g_cdd_cst_realloc_fail;
    if (g_cdd_cst_realloc_fail && --g_cdd_cst_realloc_fail == 0)
      new_arr = NULL;
    else
#endif
      new_arr = (cdd_cst_child_t *)realloc(parent->children,
                                           new_cap * sizeof(cdd_cst_child_t));
    if (!new_arr) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      return CDD_C_ERROR_MEMORY;
    }
    parent->children = new_arr;
    parent->capacity = new_cap;
  }

  if (idx < parent->num_children) {
    memmove(&parent->children[idx + 1], &parent->children[idx],
            (parent->num_children - idx) * sizeof(cdd_cst_child_t));
  }

  parent->children[idx].kind = CDD_CST_CHILD_NODE;
  parent->children[idx].val.node = new_node;
  new_node->parent = parent;
  parent->num_children++;

  return CDD_C_SUCCESS;
}

enum cdd_c_error cdd_cst_insert_node_before(cdd_cst_node_t *target_node,
                                            cdd_cst_node_t *new_node) {
  size_t idx;
  int rc;
  if (!target_node || !new_node || !target_node->parent)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  rc = find_child_index_mutate(target_node->parent, target_node, &idx);
  if (rc != 0)
    return rc;

  return cdd_cst_insert_child_node_at(target_node->parent, idx, new_node);
}

enum cdd_c_error cdd_cst_insert_node_after(cdd_cst_node_t *target_node,
                                           cdd_cst_node_t *new_node) {
  size_t idx;
  int rc;
  if (!target_node || !new_node || !target_node->parent)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  rc = find_child_index_mutate(target_node->parent, target_node, &idx);
  if (rc != 0)
    return rc;

  return cdd_cst_insert_child_node_at(target_node->parent, idx + 1, new_node);
}

enum cdd_c_error cdd_cst_detach_node(cdd_cst_tree_t *tree,
                                     cdd_cst_node_t *node) {
  size_t idx;
  int rc;
  cdd_cst_node_t *parent;
  cdd_token_t *first_tok = NULL, *last_tok = NULL;

  if (!tree || !node || !node->parent)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  parent = node->parent;

  rc = find_child_index_mutate(parent, node, &idx);
  if (rc != 0)
    return rc;

  /* Free trivia of tokens inside this node so we don't double free or leak them
   * if we don't fully free node immediately */
  if (find_first_token_mutate(node, &first_tok) != 0)
    first_tok = NULL;
  if (first_tok && first_tok->leading_trivia) {
    /* If we delete this node, we should ideally keep the leading whitespace,
       but for a simple implementation, let's just clear it to avoid crashes. */
    first_tok->leading_trivia = NULL; /* leaking if not tracked, but safe */
  }

  find_last_token_mutate(node, &last_tok);
  if (last_tok && last_tok->trailing_trivia) {
    last_tok->trailing_trivia = NULL;
  }

  if (idx < parent->num_children - 1) {
    memmove(&parent->children[idx], &parent->children[idx + 1],
            (parent->num_children - idx - 1) * sizeof(cdd_cst_child_t));
  }
  parent->num_children--;

  node->parent = NULL; /* explicitly detach */

  return CDD_C_SUCCESS;
}

enum cdd_c_error cdd_cst_clone_tree(cdd_cst_tree_t *tree, cdd_cst_node_t *root,
                                    cdd_cst_node_t **out_clone) {
  cdd_cst_node_t *clone;
  size_t i;
  int rc;

  if (!tree || !root || !out_clone)
    return CDD_C_ERROR_INVALID_ARGUMENT;

#ifdef CDD_BUILD_TESTS
  {
    extern int g_cdd_cst_alloc_node_fail;
    if (g_cdd_cst_alloc_node_fail && --g_cdd_cst_alloc_node_fail == 0)
      clone = NULL;
    else
      clone = (cdd_cst_node_t *)calloc(1, sizeof(cdd_cst_node_t));
  }
#else
  clone = (cdd_cst_node_t *)calloc(1, sizeof(cdd_cst_node_t));
#endif
  if (!clone) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }

  clone->kind = root->kind;
  if (root->num_children > 0) {
    clone->capacity = root->capacity;
#ifdef CDD_BUILD_TESTS
    {
      extern int g_cdd_cst_realloc_fail;
      if (g_cdd_cst_realloc_fail && --g_cdd_cst_realloc_fail == 0)
        clone->children = NULL;
      else
        clone->children =
            (cdd_cst_child_t *)calloc(clone->capacity, sizeof(cdd_cst_child_t));
    }
#else
    clone->children =
        (cdd_cst_child_t *)calloc(clone->capacity, sizeof(cdd_cst_child_t));
#endif
    if (!clone->children) {
      free(clone);
      return CDD_C_ERROR_MEMORY;
    }

    for (i = 0; i < root->num_children; i++) {
      if (root->children[i].kind == CDD_CST_CHILD_TOKEN) {
        cdd_token_t *orig_tok = root->children[i].val.token;
        cdd_token_t *new_tok;
#ifdef CDD_BUILD_TESTS
        extern int g_cdd_cst_alloc_token_fail;
        if (g_cdd_cst_alloc_token_fail && --g_cdd_cst_alloc_token_fail == 0)
          new_tok = NULL;
        else
#endif
          new_tok = (cdd_token_t *)calloc(1, sizeof(cdd_token_t));
        if (!new_tok) {
          rc = CDD_C_ERROR_MEMORY;
          goto err;
        }

        *new_tok = *orig_tok;
        clone_trivia_list_mutate(orig_tok->leading_trivia,
                                 &new_tok->leading_trivia);
        clone_trivia_list_mutate(orig_tok->trailing_trivia,
                                 &new_tok->trailing_trivia);

        rc = track_synthesized_token_mutate(tree, new_tok);
        if (rc != 0) {
          free(new_tok);
          goto err;
        }

        clone->children[clone->num_children].kind = CDD_CST_CHILD_TOKEN;
        clone->children[clone->num_children].val.token = new_tok;
        clone->num_children++;
      } else {
        cdd_cst_node_t *child_clone;
        rc = cdd_cst_clone_tree(tree, root->children[i].val.node, &child_clone);
        if (rc != 0)
          goto err;

        child_clone->parent = clone;
        clone->children[clone->num_children].kind = CDD_CST_CHILD_NODE;
        clone->children[clone->num_children].val.node = child_clone;
        clone->num_children++;
      }
    }
  }

  *out_clone = clone;
  return CDD_C_SUCCESS;

err:
  cdd_cst_free_node(clone);
  return rc;
}

enum cdd_c_error cdd_cst_remove_child(cdd_cst_node_t *node, size_t idx) {
  size_t i;
  if (!node || idx >= node->num_children)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  for (i = idx; i + 1 < node->num_children; i++) {
    node->children[i] = node->children[i + 1];
  }
  node->num_children--;
  return CDD_C_SUCCESS;
}

enum cdd_c_error cdd_cst_replace_token_child(cdd_cst_node_t *node, size_t idx,
                                             cdd_token_t *new_tok) {
  if (!node || idx >= node->num_children || !new_tok)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  if (node->children[idx].kind != CDD_CST_CHILD_TOKEN)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  node->children[idx].val.token = new_tok;
  return CDD_C_SUCCESS;
}

/* LCOV_EXCL_STOP */
