/* clang-format off */
#include "cdd_cst_transform.h"
#include "classes/parse/cdd_cst_mutate.h"
#include "classes/parse/cdd_cst_builder.h"
#include "classes/parse/cdd_cst_factory.h"
#include "classes/parse/cdd_cst_parser.h"
#include "classes/parse/cdd_cst_query.h"
#include "c_str_span.h"
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
/* clang-format on */

int cdd_transform_extern_c(cdd_cst_tree_t *tree,
                           const cdd_transform_config_t *config) {
  cdd_cst_query_result_t res;
  size_t i;
  int rc;
  int found_cpp = 0;
  cdd_cst_node_t *insert_after_node = NULL;
  (void)config;

  if (!tree || !tree->root)
    return EINVAL;

  /* 1. Check if __cplusplus is already checked */
  rc = cdd_cst_find_nodes_by_type(tree->root, CDD_CST_PREPROC_DIRECTIVE, &res);
  if (rc == 0) {
    for (i = 0; i < res.size; i++) {
      cdd_cst_node_t *dir = res.nodes[i];
      if (dir->num_children > 0 &&
          dir->children[0].kind == CDD_CST_CHILD_TOKEN) {
        cdd_token_t *tok = dir->children[0].val.token;
        if (tok->kind == CDD_TOKEN_PREPROC_IFDEF) {
          if (dir->num_children > 1 &&
              dir->children[1].kind == CDD_CST_CHILD_TOKEN) {
            cdd_token_t *ident = dir->children[1].val.token;
            if (ident->length == 11 &&
                memcmp(ident->start, "__cplusplus", 11) == 0) {
              found_cpp = 1;
              break;
            }
          }
        }
      }
    }
    free(res.nodes);
  }

  if (found_cpp)
    return 0;

  /* 2. Find insertion boundary: After #includes and comments, before C
   * declarations */
  for (i = 0; i < tree->root->num_children; i++) {
    cdd_cst_node_t *child = tree->root->children[i].val.node;
    if (child->kind == CDD_CST_PREPROC_DIRECTIVE) {
      if (child->num_children > 0 &&
          child->children[0].kind == CDD_CST_CHILD_TOKEN) {
        cdd_token_t *tok = child->children[0].val.token;
        if (tok->kind == CDD_TOKEN_PREPROC_INCLUDE) {
          insert_after_node = child;
        }
      }
    } else if (child->kind == CDD_CST_DECLARATION ||
               child->kind == CDD_CST_FUNCTION_DEFINITION) {
      break;
    }
  }

  /* 3. Synthesize Top Nodes */
  {
    cdd_cst_builder_t bld;
    cdd_cst_node_t *top_node =
        (cdd_cst_node_t *)calloc(1, sizeof(cdd_cst_node_t));
    if (top_node) {
      top_node->kind = CDD_CST_UNKNOWN;
      cdd_cst_builder_init(&bld, tree, top_node);
      cdd_cst_bld_newline(&bld);
      cdd_cst_bld_extern_c_open(&bld);
      if (!cdd_cst_builder_has_error(&bld)) {
        if (insert_after_node) {
          cdd_cst_insert_node_after(insert_after_node, top_node);
        } else if (tree->root->num_children > 0) {
          cdd_cst_insert_node_before(tree->root->children[0].val.node,
                                     top_node);
        }
      } else {
        free(top_node->children);
        free(top_node);
      }
      cdd_cst_builder_free(&bld);
    }
  }

  /* 4. Synthesize Bottom Nodes */
  {
    cdd_cst_builder_t bld;
    cdd_cst_node_t *bot_node =
        (cdd_cst_node_t *)calloc(1, sizeof(cdd_cst_node_t));
    if (bot_node) {
      bot_node->kind = CDD_CST_UNKNOWN;
      cdd_cst_builder_init(&bld, tree, bot_node);
      cdd_cst_bld_newline(&bld);
      cdd_cst_bld_extern_c_close(&bld);
      if (!cdd_cst_builder_has_error(&bld)) {
        if (tree->root->num_children > 0) {
          cdd_cst_node_t *last_node =
              tree->root->children[tree->root->num_children - 1].val.node;
          if (last_node != bot_node) {
            cdd_cst_insert_node_after(last_node, bot_node);
          }
        } else {
          cdd_cst_append_child_node(tree->root, bot_node);
        }
      } else {
        free(bot_node->children);
        free(bot_node);
      }
      cdd_cst_builder_free(&bld);
    }
  }

  return 0;
}
