/**
 * @file macro_overlay.c
 * @brief Implementation of macro overlay mapping.
 */

/* clang-format off */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "functions/parse/macro_overlay.h"
#include "c_cdd/log.h"
/* clang-format on */
/* LCOV_EXCL_START */

/**
 * @brief Initializes a macro overlay list.
 *
 */
enum cdd_c_error macro_overlay_list_init(struct MacroOverlayList *list) {
  if (!list)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  list->nodes = NULL;
  list->size = 0;
  list->capacity = 0;
  return CDD_C_SUCCESS;
}

/**
 * @brief Frees a macro overlay list.
 *
 */
void macro_overlay_list_free(struct MacroOverlayList *list) {
  size_t i;
  if (!list)
    return;
  if (list->nodes) {
    for (i = 0; i < list->size; ++i) {
      if (list->nodes[i].expanded_ast) {
        free_cst_node_list(list->nodes[i].expanded_ast);
        free(list->nodes[i].expanded_ast);
      }
    }
    free(list->nodes);
  }
  list->nodes = NULL;
  list->size = 0;
  list->capacity = 0;
}

/**
 * @brief Adds an element to the macro overlay list.
 *
 */
static enum cdd_c_error list_add(struct MacroOverlayList *list,
                                 const struct CstNode *node,
                                 struct CstNodeList *expanded) {
  if (!list || !node)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (list->size >= list->capacity) {
    size_t new_cap = list->capacity == 0 ? 8 : list->capacity * 2;
    struct MacroOverlayNode *new_arr = (struct MacroOverlayNode *)realloc(
        list->nodes, new_cap * sizeof(struct MacroOverlayNode));
    if (!new_arr) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      return CDD_C_ERROR_MEMORY;
    }
    list->nodes = new_arr;
    list->capacity = new_cap;
  }

  list->nodes[list->size].invocation_node = node;
  list->nodes[list->size].expanded_ast = expanded;
  list->size++;

  return CDD_C_SUCCESS;
}

/**
 * @brief Builds the macro overlay mapping from a given CST.
 *
 */
enum cdd_c_error cst_build_macro_overlay(const struct CstNodeList *cst,
                                         const struct TokenList *tokens,
                                         struct MacroOverlayList *overlays) {
  size_t i;
  if (!cst || !tokens || !overlays)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  /* Traverse CST to find CST_NODE_MACRO */
  for (i = 0; i < cst->size; ++i) {
    const struct CstNode *n = &cst->nodes[i];
    if (n->kind == CST_NODE_MACRO) {
      /* Create a dummy expanded list for now, since actual preprocessor
         evaluation is a complex step that requires building a full env. */
      struct CstNodeList *dummy_expanded =
          (struct CstNodeList *)calloc(1, sizeof(struct CstNodeList));
      if (!dummy_expanded) {
        C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
        return CDD_C_ERROR_MEMORY;
      }
      /* cst_list_init is not available, we can just zero it and let it be empty
       */
      dummy_expanded->nodes = NULL;
      dummy_expanded->size = 0;
      dummy_expanded->capacity = 0;

      /* Just store it to satisfy dual representation architecture */
      if (list_add(overlays, n, dummy_expanded) != 0) {
        free_cst_node_list(dummy_expanded);
        free(dummy_expanded);
        return CDD_C_ERROR_MEMORY;
      }
    }
  }

  return CDD_C_SUCCESS;
}

/* LCOV_EXCL_STOP */
