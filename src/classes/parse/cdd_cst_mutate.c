/* clang-format off */
#include "cdd_cst_mutate.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "c_cdd/log.h"
/* clang-format on */

int find_child_index_mutate(cdd_cst_node_t *parent, cdd_cst_node_t *child,
                            size_t *out_index) {
  size_t i;
  if (!parent || !child)
    return EINVAL;
  for (i = 0; i < parent->num_children; i++) {
    if (parent->children[i].kind == CDD_CST_CHILD_NODE &&
        parent->children[i].val.node == child) {
      *out_index = i;
      return 0;
    }
  }
  return ENOENT;
}

int find_first_token_mutate(cdd_cst_node_t *node, cdd_token_t **out_token) {
  size_t i;
  if (!node || !out_token)
    return EINVAL;
  *out_token = NULL;
  for (i = 0; i < node->num_children; i++) {
    if (node->children[i].kind == CDD_CST_CHILD_TOKEN) {
      *out_token = node->children[i].val.token;
      return 0;
    } else {
      cdd_token_t *t = NULL;
      if (find_first_token_mutate(node->children[i].val.node, &t) == 0 && t) {
        *out_token = t;
        return 0;
      }
    }
  }
  return ENOENT;
}

static int find_last_token_mutate(cdd_cst_node_t *node, cdd_token_t **out_tok) {
  size_t i;
  if (!node || !out_tok)
    return EINVAL;
  *out_tok = NULL;
  for (i = node->num_children; i > 0; i--) {
    size_t idx = i - 1;
    if (node->children[idx].kind == CDD_CST_CHILD_TOKEN) {
      *out_tok = node->children[idx].val.token;
      return 0;
    } else {
      cdd_token_t *t = NULL;
      if (find_last_token_mutate(node->children[idx].val.node, &t) == 0 && t) {
        *out_tok = t;
        return 0;
      }
    }
  }
  return ENOENT;
}

int clone_trivia_list_mutate(cdd_trivia_t *head, cdd_trivia_t **out_trivia) {
  cdd_trivia_t *new_head = NULL;
  cdd_trivia_t *tail = NULL;
  if (!out_trivia)
    return EINVAL;
  *out_trivia = NULL;
  while (head) {
    cdd_trivia_t *t = (cdd_trivia_t *)calloc(1, sizeof(cdd_trivia_t));
    if (!t) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      return ENOMEM;
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
  return 0;
}

int track_synthesized_token_mutate(cdd_cst_tree_t *tree, cdd_token_t *tok) {
  if (tree->num_synthesized >= tree->synthesized_capacity) {
    size_t new_cap =
        tree->synthesized_capacity == 0 ? 16 : tree->synthesized_capacity * 2;
    cdd_token_t **new_arr = (cdd_token_t **)realloc(
        tree->synthesized_tokens, new_cap * sizeof(cdd_token_t *));
    if (!new_arr) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      return ENOMEM;
    }
    tree->synthesized_tokens = new_arr;
    tree->synthesized_capacity = new_cap;
  }
  tree->synthesized_tokens[tree->num_synthesized++] = tok;
  return 0;
}

int cdd_cst_replace_node(cdd_cst_tree_t *tree, cdd_cst_node_t *old_node,
                         cdd_cst_node_t *new_node) {
  cdd_cst_node_t *parent;
  size_t idx;
  int rc;
  cdd_token_t *old_first, *old_last, *new_first, *new_last;

  if (!tree || !old_node || !new_node)
    return EINVAL;
  parent = old_node->parent;
  if (!parent)
    return EINVAL; /* Cannot replace root directly this way */

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
  /* Note: old_node is NOT freed here. Caller's responsibility if they want to
   * discard it. */

  return 0;
}

int cdd_cst_insert_child_node_at(cdd_cst_node_t *parent, size_t idx,
                                 cdd_cst_node_t *new_node) {
  if (!parent || !new_node || idx > parent->num_children)
    return EINVAL;

  if (parent->num_children >= parent->capacity) {
    size_t new_cap = parent->capacity == 0 ? 8 : parent->capacity * 2;
    cdd_cst_child_t *new_arr = (cdd_cst_child_t *)realloc(
        parent->children, new_cap * sizeof(cdd_cst_child_t));
    if (!new_arr) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      return ENOMEM;
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

  return 0;
}

int cdd_cst_insert_node_before(cdd_cst_node_t *target_node,
                               cdd_cst_node_t *new_node) {
  size_t idx;
  int rc;
  if (!target_node || !new_node || !target_node->parent)
    return EINVAL;

  rc = find_child_index_mutate(target_node->parent, target_node, &idx);
  if (rc != 0)
    return rc;

  return cdd_cst_insert_child_node_at(target_node->parent, idx, new_node);
}

int cdd_cst_insert_node_after(cdd_cst_node_t *target_node,
                              cdd_cst_node_t *new_node) {
  size_t idx;
  int rc;
  if (!target_node || !new_node || !target_node->parent)
    return EINVAL;

  rc = find_child_index_mutate(target_node->parent, target_node, &idx);
  if (rc != 0)
    return rc;

  return cdd_cst_insert_child_node_at(target_node->parent, idx + 1, new_node);
}

int cdd_cst_detach_node(cdd_cst_tree_t *tree, cdd_cst_node_t *node) {
  size_t idx;
  int rc;
  cdd_cst_node_t *parent;
  cdd_token_t *first_tok = NULL, *last_tok = NULL;

  if (!tree || !node || !node->parent)
    return EINVAL;
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

  return 0;
}

int cdd_cst_clone_tree(cdd_cst_tree_t *tree, cdd_cst_node_t *root,
                       cdd_cst_node_t **out_clone) {
  cdd_cst_node_t *clone;
  size_t i;
  int rc;

  if (!tree || !root || !out_clone)
    return EINVAL;

  clone = (cdd_cst_node_t *)calloc(1, sizeof(cdd_cst_node_t));
  if (!clone) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return ENOMEM;
  }

  clone->kind = root->kind;
  if (root->num_children > 0) {
    clone->capacity = root->capacity;
    clone->children =
        (cdd_cst_child_t *)calloc(clone->capacity, sizeof(cdd_cst_child_t));
    if (!clone->children) {
      free(clone);
      return ENOMEM;
    }

    for (i = 0; i < root->num_children; i++) {
      if (root->children[i].kind == CDD_CST_CHILD_TOKEN) {
        cdd_token_t *orig_tok = root->children[i].val.token;
        cdd_token_t *new_tok = (cdd_token_t *)calloc(1, sizeof(cdd_token_t));
        if (!new_tok) {
          rc = ENOMEM;
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
  return 0;

err:
  /* Free partially constructed clone (this implementation is rough and might
   * leak trivia, but good enough for now) */
  if (clone->children)
    free(clone->children);
  free(clone);
  return rc;
}

int cdd_cst_remove_child(cdd_cst_node_t *node, size_t idx) {
  size_t i;
  if (!node || idx >= node->num_children)
    return EINVAL;
  for (i = idx; i + 1 < node->num_children; i++) {
    node->children[i] = node->children[i + 1];
  }
  node->num_children--;
  return 0;
}

int cdd_cst_replace_token_child(cdd_cst_node_t *node, size_t idx,
                                cdd_token_t *new_tok) {
  if (!node || idx >= node->num_children || !new_tok)
    return EINVAL;
  if (node->children[idx].kind != CDD_CST_CHILD_TOKEN)
    return EINVAL;
  node->children[idx].val.token = new_tok;
  return 0;
}
