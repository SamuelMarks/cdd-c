/* clang-format off */
#include "cdd_cst_factory.h"
#include "cdd_cst_mutate.h"
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

cdd_cst_node_t *cdd_cst_alloc_node(enum cdd_cst_node_kind_t kind) {
  cdd_cst_node_t *n = (cdd_cst_node_t *)calloc(1, sizeof(cdd_cst_node_t));
  if (n) {
    n->kind = kind;
  }
  return n;
}

static int track_synthesized(cdd_cst_tree_t *tree, cdd_token_t *tok) {
  if (tree->num_synthesized >= tree->synthesized_capacity) {
    size_t new_cap =
        tree->synthesized_capacity == 0 ? 128 : tree->synthesized_capacity * 2;
    cdd_token_t **new_arr = (cdd_token_t **)realloc(
        tree->synthesized_tokens, new_cap * sizeof(cdd_token_t *));
    if (!new_arr)
      return ENOMEM;
    tree->synthesized_tokens = new_arr;
    tree->synthesized_capacity = new_cap;
  }
  tree->synthesized_tokens[tree->num_synthesized++] = tok;
  return 0;
}

cdd_token_t *cdd_cst_create_token_len(cdd_cst_tree_t *tree,
                                      enum cdd_token_kind_t kind,
                                      const char *text, size_t length) {
  cdd_token_t *tok = (cdd_token_t *)calloc(1, sizeof(cdd_token_t));
  if (!tok)
    return NULL;

  tok->kind = kind;
  tok->start = (const uint8_t *)text;
  tok->length = length;

  if (track_synthesized(tree, tok) != 0) {
    free(tok);
    return NULL;
  }
  return tok;
}

cdd_token_t *cdd_cst_create_token(cdd_cst_tree_t *tree,
                                  enum cdd_token_kind_t kind,
                                  const char *text) {
  return cdd_cst_create_token_len(tree, kind, text, text ? strlen(text) : 0);
}

int cdd_cst_append_child_node(cdd_cst_node_t *parent, cdd_cst_node_t *child) {
  if (!parent || !child)
    return EINVAL;

  if (parent->num_children >= parent->capacity) {
    size_t new_cap = parent->capacity == 0 ? 8 : parent->capacity * 2;
    cdd_cst_child_t *new_arr = (cdd_cst_child_t *)realloc(
        parent->children, new_cap * sizeof(cdd_cst_child_t));
    if (!new_arr)
      return ENOMEM;
    parent->children = new_arr;
    parent->capacity = new_cap;
  }

  parent->children[parent->num_children].kind = CDD_CST_CHILD_NODE;
  parent->children[parent->num_children].val.node = child;
  child->parent = parent;
  parent->num_children++;
  return 0;
}

int cdd_cst_append_child_token(cdd_cst_node_t *parent, cdd_token_t *token) {
  if (!parent || !token)
    return EINVAL;

  if (parent->num_children >= parent->capacity) {
    size_t new_cap = parent->capacity == 0 ? 8 : parent->capacity * 2;
    cdd_cst_child_t *new_arr = (cdd_cst_child_t *)realloc(
        parent->children, new_cap * sizeof(cdd_cst_child_t));
    if (!new_arr)
      return ENOMEM;
    parent->children = new_arr;
    parent->capacity = new_cap;
  }

  parent->children[parent->num_children].kind = CDD_CST_CHILD_TOKEN;
  parent->children[parent->num_children].val.token = token;
  parent->num_children++;
  return 0;
}
static cdd_cst_node_t *cdd_cst_create_stmt_from_tokens(cdd_cst_tree_t *tree,
                                                       ...) {
  cdd_cst_node_t *n;
  va_list args;
  cdd_token_t *tok;

  (void)tree;

  n = cdd_cst_alloc_node(CDD_CST_STATEMENT);
  if (!n)
    return NULL;

  va_start(args, tree);
  while ((tok = va_arg(args, cdd_token_t *)) != NULL) {
    if (cdd_cst_append_child_token(n, tok) != 0) {
      /* Handle memory leak locally or assume caller frees. For now, best effort
       */
      va_end(args);
      return NULL;
    }
  }
  va_end(args);

  return n;
}

void cdd_cst_free_node_only(cdd_cst_node_t *node) {
  if (!node)
    return;
  if (node->children)
    free(node->children);
  free(node);
}
