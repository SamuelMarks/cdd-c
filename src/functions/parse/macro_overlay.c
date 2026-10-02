/**
 * @file macro_overlay.c
 * @brief Implementation of macro overlay mapping.
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "c_cdd/log.h"
#include "functions/parse/macro_overlay.h"
/* clang-format on */

#ifdef CDD_BUILD_TESTS
C_CDD_EXPORT int g_cdd_macro_overlay_fail_realloc = 0;
C_CDD_EXPORT int g_cdd_macro_overlay_fail_calloc = 0;
C_CDD_EXPORT int g_cdd_macro_overlay_fail_inner_realloc = 0;
#endif

/**
 * @brief Initializes a macro overlay list.
 *
 */
cdd_c_error_t macro_overlay_list_init(struct MacroOverlayList *list) {
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
static cdd_c_error_t list_add(struct MacroOverlayList *list,
                              const struct CstNode *node,
                              struct CstNodeList *expanded) {

  if (list->size >= list->capacity) {
    size_t new_cap = list->capacity == 0 ? 8 : list->capacity * 2;
    struct MacroOverlayNode *new_arr;
#ifdef CDD_BUILD_TESTS
    {
      extern C_CDD_EXPORT int g_cdd_macro_overlay_fail_realloc;
      if (g_cdd_macro_overlay_fail_realloc == 1) {
        new_arr = NULL;
        g_cdd_macro_overlay_fail_realloc = 0;
      } else {
        if (g_cdd_macro_overlay_fail_realloc > 1) {
          g_cdd_macro_overlay_fail_realloc--;
        }
        new_arr = (struct MacroOverlayNode *)realloc(
            list->nodes, new_cap * sizeof(struct MacroOverlayNode));
      }
    }
#else
    new_arr = (struct MacroOverlayNode *)realloc(
        list->nodes, new_cap * sizeof(struct MacroOverlayNode));
#endif
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
cdd_c_error_t cst_build_macro_overlay(const struct CstNodeList *cst,
                                      const struct TokenList *tokens,
                                      struct MacroOverlayList *overlays) {
  size_t i;
  cdd_c_error_t rc;
  if (!cst || !tokens || !overlays)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  /* Traverse CST to find CST_NODE_MACRO */
  for (i = 0; i < cst->size; ++i) {
    const struct CstNode *n = &cst->nodes[i];
    if (n->kind == CST_NODE_MACRO) {
      struct CstNodeList *expanded;
#ifdef CDD_BUILD_TESTS
      {
        extern C_CDD_EXPORT int g_cdd_macro_overlay_fail_calloc;
        if (g_cdd_macro_overlay_fail_calloc) {
          expanded = NULL;
          g_cdd_macro_overlay_fail_calloc = 0;
        } else {
          expanded =
              (struct CstNodeList *)calloc(1, sizeof(struct CstNodeList));
        }
      }
#else
      expanded = (struct CstNodeList *)calloc(1, sizeof(struct CstNodeList));
#endif
      if (!expanded) {
        return CDD_C_ERROR_MEMORY;
      }

      /* Initialize the expanded list */
      expanded->nodes = NULL;
      expanded->size = 0;
      expanded->capacity = 0;

      /* The legacy CST_NODE doesn't have a value union. We use tokens bounds to
       * expand. */
      if (n->start_token < n->end_token && n->end_token <= tokens->size) {
        size_t j;
        for (j = n->start_token; j < n->end_token; j++) {
          struct CstNode expanded_node;
          memset(&expanded_node, 0, sizeof(struct CstNode));
          expanded_node.kind = CST_NODE_OTHER;
          expanded_node.start = tokens->tokens[j].start;
          expanded_node.length = tokens->tokens[j].length;
          expanded_node.start_token = j;
          expanded_node.end_token = j + 1;

          if (expanded->size >= expanded->capacity) {
            size_t new_cap =
                expanded->capacity == 0 ? 4 : expanded->capacity * 2;

#ifdef CDD_BUILD_TESTS
            struct CstNode *new_arr = NULL;
            extern C_CDD_EXPORT int g_cdd_macro_overlay_fail_inner_realloc;
            printf("inner realloc: %d\n",
                   g_cdd_macro_overlay_fail_inner_realloc);
            if (g_cdd_macro_overlay_fail_inner_realloc == 1) {
              g_cdd_macro_overlay_fail_inner_realloc = 0;
            } else {
              if (g_cdd_macro_overlay_fail_inner_realloc > 1)
                g_cdd_macro_overlay_fail_inner_realloc--;
              new_arr = (struct CstNode *)realloc(
                  expanded->nodes, new_cap * sizeof(struct CstNode));
            }
#else
            struct CstNode *new_arr = (struct CstNode *)realloc(
                expanded->nodes, new_cap * sizeof(struct CstNode));
#endif
            if (!new_arr) {
              free_cst_node_list(expanded);
              free(expanded);
              return CDD_C_ERROR_MEMORY;
            }
            expanded->nodes = new_arr;
            expanded->capacity = new_cap;
          }
          expanded->nodes[expanded->size++] = expanded_node;
        }
      }

      rc = list_add(overlays, n, expanded);
      printf("list_add returned %d\n", rc);
      if (rc != CDD_C_SUCCESS) {
        free_cst_node_list(expanded);
        free(expanded);
        return rc;
      }
    }
  }

  return CDD_C_SUCCESS;
}
