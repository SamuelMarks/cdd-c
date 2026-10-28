/**
 * @file cdd_cst_factory.c
 * @brief Implementation of CST factory allocation and construction functions.
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "cdd_cst_factory.h"
#include "c_cdd/log.h"
#include "c_cdd/memory.h"
#include "cdd_cst_mutate.h"
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/**
 * @brief Allocates a new empty CST node of the given kind.
 *
 * @param[in] kind CST node kind to allocate.
 * @param[out] out_node Output pointer receiving the allocated CST node.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t cdd_cst_alloc_node(enum cdd_cst_node_kind_t kind,
                                 cdd_cst_node_t **out_node) {
  cdd_cst_node_t *n;
  if (!out_node)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  n = (cdd_cst_node_t *)C_CDD_CALLOC(1, sizeof(cdd_cst_node_t));
  if (!n) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM");
    return CDD_C_ERROR_MEMORY;
  }
  n->kind = kind;
  *out_node = n;
  return CDD_C_SUCCESS;
}

/**
 * @brief Tracks synthesized token memory to allow tree cleanup later.
 *
 * @param[in,out] tree The CST tree owning the synthesized token.
 * @param[in] tok The synthesized token to track.
 * @return CDD_C_SUCCESS on success, or CDD_C_ERROR_MEMORY on allocation
 * failure.
 */
static cdd_c_error_t track_synthesized(cdd_cst_tree_t *tree, cdd_token_t *tok) {
  if (tree->num_synthesized >= tree->synthesized_capacity) {
    size_t new_cap =
        tree->synthesized_capacity == 0 ? 128 : tree->synthesized_capacity * 2;
    cdd_token_t **new_arr = (cdd_token_t **)C_CDD_REALLOC(
        tree->synthesized_tokens, new_cap * sizeof(cdd_token_t *));
    if (!new_arr) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM");
      return CDD_C_ERROR_MEMORY;
    }
    tree->synthesized_tokens = new_arr;
    tree->synthesized_capacity = new_cap;
  }
  tree->synthesized_tokens[tree->num_synthesized++] = tok;
  return CDD_C_SUCCESS;
}

/**
 * @brief Creates a synthetic token from a buffer and explicit byte length.
 *
 * @param[in,out] tree Owning CST tree to register the synthesized token in.
 * @param[in] kind Lexical token kind.
 * @param[in] text Pointer to token text bytes.
 * @param[in] length Length of token text in bytes.
 * @param[out] out_token Output pointer receiving the newly allocated token.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t cdd_cst_create_token_len(cdd_cst_tree_t *tree,
                                       enum cdd_token_kind_t kind,
                                       const char *text, size_t length,
                                       cdd_token_t **out_token) {
  cdd_token_t *tok;
  if (!out_token || !tree)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  tok = (cdd_token_t *)C_CDD_CALLOC(1, sizeof(cdd_token_t));
  if (!tok) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM");
    return CDD_C_ERROR_MEMORY;
  }

  tok->kind = kind;
  tok->start = (const uint8_t *)text;
  tok->length = length;

  {
    cdd_c_error_t rc = track_synthesized(tree, tok);
    if (rc != CDD_C_SUCCESS) {
      C_CDD_FREE(tok);
      return rc;
    }
  }
  *out_token = tok;
  return CDD_C_SUCCESS;
}

/**
 * @brief Creates a synthetic token from a null-terminated string.
 *
 * @param[in,out] tree Owning CST tree to register the synthesized token in.
 * @param[in] kind Lexical token kind.
 * @param[in] text Null-terminated token text string.
 * @param[out] out_token Output pointer receiving the newly allocated token.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t cdd_cst_create_token(cdd_cst_tree_t *tree,
                                   enum cdd_token_kind_t kind, const char *text,
                                   cdd_token_t **out_token) {
  return cdd_cst_create_token_len(tree, kind, text, text ? strlen(text) : 0,
                                  out_token);
}

/**
 * @brief Appends a child CST node to a parent CST node.
 *
 * @param[in,out] parent Parent CST node.
 * @param[in,out] child Child CST node to append.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t cdd_cst_append_child_node(cdd_cst_node_t *parent,
                                        cdd_cst_node_t *child) {
  if (!parent || !child)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  {
    size_t new_cap = parent->num_children + 1;
    cdd_cst_child_t *new_arr = (cdd_cst_child_t *)C_CDD_REALLOC(
        parent->children, new_cap * sizeof(cdd_cst_child_t));
    if (!new_arr) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM");
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

/**
 * @brief Appends a child lexical token to a parent CST node.
 *
 * @param[in,out] parent Parent CST node.
 * @param[in] token Lexical token to append.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
cdd_c_error_t cdd_cst_append_child_token(cdd_cst_node_t *parent,
                                         cdd_token_t *token) {
  if (!parent || !token)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (parent->num_children >= parent->capacity) {
    size_t new_cap = parent->capacity + 1;
    cdd_cst_child_t *new_arr = (cdd_cst_child_t *)C_CDD_REALLOC(
        parent->children, new_cap * sizeof(cdd_cst_child_t));
    if (!new_arr) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM");
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
 *
 * @param[in,out] node CST node structure to free.
 */
void cdd_cst_free_node_only(cdd_cst_node_t *node) {
  if (!node)
    return;
  if (node->children)
    C_CDD_FREE(node->children);
  C_CDD_FREE(node);
}

/**
 * @brief Recursively frees a CST node and all of its descendant child nodes.
 *
 * @param[in,out] node Root CST node to recursively free.
 */
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
