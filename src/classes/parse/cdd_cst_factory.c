/**
 * @file cdd_cst_factory.c
 * @brief Implementation of CST factory allocation and construction functions.
 */

/* clang-format off */
#include "cdd_cst_factory.h"
#include "cdd_cst_mutate.h"
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "c_cdd/log.h"
/* clang-format on */
#ifdef CDD_BUILD_TESTS
C_CDD_EXPORT int g_cdd_cst_alloc_token_fail = 0;
C_CDD_EXPORT int g_cdd_cst_realloc_fail = 0;
C_CDD_EXPORT int g_cdd_cst_alloc_node_fail = 0;
#endif

enum cdd_c_error cdd_cst_alloc_node(enum cdd_cst_node_kind_t kind,
                                    cdd_cst_node_t **out_node) {
  cdd_cst_node_t *n;
  if (!out_node)
    return CDD_C_ERROR_INVALID_ARGUMENT;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_cst_alloc_node_fail && --g_cdd_cst_alloc_node_fail == 0)
    n = NULL;
  else
#endif
    n = (cdd_cst_node_t *)calloc(1, sizeof(cdd_cst_node_t));
  if (!n) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }
  n->kind = kind;
  *out_node = n;
  return CDD_C_SUCCESS;
}

/**
 * @brief Tracks synthesized token memory to allow tree cleanup later.
 */
static enum cdd_c_error track_synthesized(cdd_cst_tree_t *tree,
                                          cdd_token_t *tok) {
  if (tree->num_synthesized >= tree->synthesized_capacity) {
    size_t new_cap =
        tree->synthesized_capacity == 0 ? 128 : tree->synthesized_capacity * 2;
    cdd_token_t **new_arr;
#ifdef CDD_BUILD_TESTS
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

enum cdd_c_error cdd_cst_create_token_len(cdd_cst_tree_t *tree,
                                          enum cdd_token_kind_t kind,
                                          const char *text, size_t length,
                                          cdd_token_t **out_token) {
  cdd_token_t *tok;
  if (!out_token || !tree)
    return CDD_C_ERROR_INVALID_ARGUMENT;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_cst_alloc_token_fail) {
    tok = NULL;
  } else {
#endif
    tok = (cdd_token_t *)calloc(1, sizeof(cdd_token_t));
#ifdef CDD_BUILD_TESTS
  }
#endif
  if (!tok) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }

  tok->kind = kind;
  tok->start = (const uint8_t *)text;
  tok->length = length;

  if (track_synthesized(tree, tok) != 0) {
    free(tok);
    return CDD_C_ERROR_MEMORY;
  }
  *out_token = tok;
  return CDD_C_SUCCESS;
}

enum cdd_c_error cdd_cst_create_token(cdd_cst_tree_t *tree,
                                      enum cdd_token_kind_t kind,
                                      const char *text,
                                      cdd_token_t **out_token) {
  return cdd_cst_create_token_len(tree, kind, text, text ? strlen(text) : 0,
                                  out_token);
}

enum cdd_c_error cdd_cst_append_child_node(cdd_cst_node_t *parent,
                                           cdd_cst_node_t *child) {
  if (!parent || !child)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (parent->num_children >= parent->capacity) {
#ifdef CDD_BUILD_TESTS
    size_t new_cap = parent->capacity + 1;
#else
    size_t new_cap = parent->capacity == 0 ? 8 : parent->capacity * 2;
#endif
    cdd_cst_child_t *new_arr;
#ifdef CDD_BUILD_TESTS
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

  parent->children[parent->num_children].kind = CDD_CST_CHILD_NODE;
  parent->children[parent->num_children].val.node = child;
  child->parent = parent;
  parent->num_children++;
  return CDD_C_SUCCESS;
}

enum cdd_c_error cdd_cst_append_child_token(cdd_cst_node_t *parent,
                                            cdd_token_t *token) {
  if (!parent || !token)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (parent->num_children >= parent->capacity) {
#ifdef CDD_BUILD_TESTS
    size_t new_cap = parent->capacity + 1;
#else
    size_t new_cap = parent->capacity == 0 ? 8 : parent->capacity * 2;
#endif
    cdd_cst_child_t *new_arr;
#ifdef CDD_BUILD_TESTS
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

  parent->children[parent->num_children].kind = CDD_CST_CHILD_TOKEN;
  parent->children[parent->num_children].val.token = token;
  parent->num_children++;
  return CDD_C_SUCCESS;
}

/**
 * @brief Frees a node structure, but not its children.
 * @param node The node to free.
 */
void cdd_cst_free_node_only(cdd_cst_node_t *node) {
  if (!node)
    return;
  if (node->children)
    free(node->children);
  free(node);
}

void cdd_cst_free_node(cdd_cst_node_t *node) {
  size_t i;
  if (!node)
    return;
  for (i = 0; i < node->num_children; i++) {
    if (node->children[i].kind == CDD_CST_CHILD_NODE) {
      cdd_cst_free_node(node->children[i].val.node);
    }
  }
  cdd_cst_free_node_only(node);
}
