/* clang-format off */
#include "cdd_cst_transform.h"
#include "classes/parse/cdd_cst_mutate.h"
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
    cdd_cst_tree_t *top_tree = NULL;
    const char *top_str =
        "\n#ifdef __cplusplus\nextern \"C\" {\n#endif /* __cplusplus */\n";
    if (cdd_cst_parse(az_span_create_from_str((char *)top_str), &top_tree) ==
        0) {
      if (top_tree->root->num_children > 0) {
        cdd_cst_node_t *cloned = NULL;
        /* Insert all children of the parsed fragment */
        if (cdd_cst_node_clone(tree, top_tree->root->children[0].val.node,
                               &cloned) == 0) {
          if (insert_after_node) {
            cdd_cst_node_insert_after(insert_after_node, cloned);
          } else if (tree->root->num_children > 0) {
            cdd_cst_node_insert_before(tree->root->children[0].val.node,
                                       cloned);
          }
        }
      }
      cdd_cst_tree_free(top_tree);
    }
  }

  /* 4. Synthesize Bottom Nodes */
  {
    cdd_cst_tree_t *bot_tree = NULL;
    const char *bot_str = "\n#ifdef __cplusplus\n}\n#endif /* __cplusplus */\n";
    if (cdd_cst_parse(az_span_create_from_str((char *)bot_str), &bot_tree) ==
        0) {
      if (bot_tree->root->num_children > 0) {
        cdd_cst_node_t *cloned = NULL;
        if (cdd_cst_node_clone(tree, bot_tree->root->children[0].val.node,
                               &cloned) == 0) {
          if (tree->root->num_children > 0) {
            cdd_cst_node_t *last_node =
                tree->root->children[tree->root->num_children - 1].val.node;
            cdd_cst_node_insert_after(last_node, cloned);
          }
        }
      }
      cdd_cst_tree_free(bot_tree);
    }
  }

  return 0;
}
