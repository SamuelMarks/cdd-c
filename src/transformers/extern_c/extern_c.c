/**
 * @file extern_c.c
 * @brief Implementation of the extern C wrapper transformer.
 *
 * @author Samuel Marks
 */

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
#include "c_cdd_export.h"
/* clang-format on */

/**
 * @brief Applies the extern C transformation to a given CST tree.
 *
 * @param[in,out] tree The CST tree.
 * @param[in] config The transform configuration.
 * @return 0 on success, or an error code.
 */
#ifdef CDD_BUILD_TESTS
C_CDD_EXPORT int g_extern_c_top_node_fail = 0;
C_CDD_EXPORT int g_extern_c_bot_node_fail = 0;
#endif

static int is_global_wrapper(cdd_cst_node_t *node) {
  while (node != NULL) {
    if (node->kind == CDD_CST_DECLARATION ||
        node->kind == CDD_CST_FUNCTION_DEFINITION ||
        node->kind == CDD_CST_STATEMENT || node->kind == CDD_CST_EXPRESSION ||
        node->kind == CDD_CST_BLOCK ||
        node->kind == CDD_CST_CLASS_DECLARATION) {
      return 0;
    }
    node = node->parent;
  }
  return 1;
}

static int tree_has_decl(cdd_cst_node_t *node) {
  size_t i;
  if (!node)
    return 0;
  for (i = 0; i < node->num_children; i++) {
    if (node->children[i].kind == CDD_CST_CHILD_NODE) {
      cdd_cst_node_t *child = node->children[i].val.node;
      int k = child->kind;
      if (k == CDD_CST_DECLARATION || k == CDD_CST_FUNCTION_DEFINITION ||
          k == CDD_CST_STATEMENT) {
        return 1;
      }
      if (k == CDD_CST_UNKNOWN) {
        if (child->num_children > 0 &&
            child->children[0].kind == CDD_CST_CHILD_TOKEN) {
          cdd_token_t *tk = child->children[0].val.token;
          if (tk->kind != CDD_TOKEN_OTHER && tk->kind != CDD_TOKEN_EOF) {
            return 1;
          }
        }
      }
      if (tree_has_decl(child))
        return 1;
    }
  }
  return 0;
}

enum cdd_c_error cdd_transform_extern_c(cdd_cst_tree_t *tree,
                                        const cdd_transform_config_t *config) {
  cdd_cst_query_result_t res;
  size_t i;
  enum cdd_c_error rc = CDD_C_SUCCESS;
  int found_cpp = 0;
  size_t insert_idx = 0;
  cdd_cst_node_t *target_parent = NULL;
  cdd_token_t *first_token_of_file = NULL;
  int has_decl = 0;
  (void)config;

  if (!tree || !tree->root)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  /* Verify there are actual C declarations to wrap */
  has_decl = tree_has_decl(tree->root);
  if (!has_decl)
    return CDD_C_SUCCESS;

  /* 1. Check if __cplusplus is already checked globally */
  rc =
      cdd_cst_find_nodes_by_type(tree->root, CDD_CST_PREPROC_CONDITIONAL, &res);
  if (rc == 0) {
    for (i = 0; i < res.size; i++) {
      cdd_cst_node_t *dir = res.nodes[i];
      if (is_global_wrapper(dir) && dir->num_children > 0 &&
          dir->children[0].kind == CDD_CST_CHILD_TOKEN) {
        cdd_token_t *tok = dir->children[0].val.token;
        if (tok->kind == CDD_TOKEN_PREPROC_IFDEF) {
          size_t k;
          for (k = 0; k + 11 <= tok->length; k++) {
            if (memcmp(tok->start + k, "__cplusplus", 11) == 0) {
              found_cpp = 1;
              break;
            }
          }
          if (found_cpp)
            break;
        }
      }
    }
    free(res.nodes);
  }

  if (!found_cpp) {
    rc =
        cdd_cst_find_nodes_by_type(tree->root, CDD_CST_PREPROC_DIRECTIVE, &res);
    if (rc == 0) {
      for (i = 0; i < res.size; i++) {
        cdd_cst_node_t *dir = res.nodes[i];
        if (is_global_wrapper(dir) && dir->num_children > 0 &&
            dir->children[0].kind == CDD_CST_CHILD_TOKEN) {
          cdd_token_t *tok = dir->children[0].val.token;
          if (tok->kind == CDD_TOKEN_PREPROC_IFDEF) {
            size_t k;
            for (k = 0; k + 11 <= tok->length; k++) {
              if (memcmp(tok->start + k, "__cplusplus", 11) == 0) {
                found_cpp = 1;
                break;
              }
            }
            if (!found_cpp) {
              size_t j;
              for (j = 1; j < dir->num_children; j++) {
                if (dir->children[j].kind == CDD_CST_CHILD_TOKEN) {
                  cdd_token_t *ident = dir->children[j].val.token;
                  if (ident->kind == CDD_TOKEN_IDENTIFIER &&
                      ident->length == 11 &&
                      memcmp(ident->start, "__cplusplus", 11) == 0) {
                    found_cpp = 1;
                    break;
                  }
                }
              }
            }
            if (found_cpp)
              break;
          }
        }
      }
      free(res.nodes);
    }
  }

  if (found_cpp)
    return CDD_C_SUCCESS;

  /* Find the very first token in the file so we can transplant comments if
   * needed */
  for (i = 0; i < tree->root->num_children; i++) {
    if (tree->root->children[i].kind == CDD_CST_CHILD_NODE) {
      if (tree->root->children[i].val.node->num_children > 0 &&
          tree->root->children[i].val.node->children[0].kind ==
              CDD_CST_CHILD_TOKEN) {
        first_token_of_file =
            tree->root->children[i].val.node->children[0].val.token;
        break;
      }
    } else if (tree->root->children[i].kind == CDD_CST_CHILD_TOKEN) {
      first_token_of_file = tree->root->children[i].val.token;
      break;
    }
  }

  /* 2. Find insertion boundary: After #includes and comments, before C
   * declarations */
  target_parent = tree->root;
  if (tree->root->num_children > 0 &&
      tree->root->children[0].kind == CDD_CST_CHILD_NODE &&
      tree->root->children[0].val.node->kind == CDD_CST_PREPROC_CONDITIONAL) {
    target_parent = tree->root->children[0].val.node;
  }

  for (i = 0; i < target_parent->num_children; i++) {
    int should_break = 0;
    if (target_parent->children[i].kind == CDD_CST_CHILD_NODE) {
      cdd_cst_node_t *child = target_parent->children[i].val.node;
      if (child->kind == CDD_CST_PREPROC_DIRECTIVE) {
        if (child->num_children > 0 &&
            child->children[0].kind == CDD_CST_CHILD_TOKEN) {
          cdd_token_t *tok = child->children[0].val.token;
          if (tok->kind == CDD_TOKEN_PREPROC_INCLUDE ||
              tok->kind == CDD_TOKEN_PREPROC_DEFINE ||
              tok->kind == CDD_TOKEN_PREPROC_PRAGMA) {
            insert_idx = i + 1;
          }
        }
      } else if (child->kind == CDD_CST_UNKNOWN) {
        if (child->num_children > 0 &&
            child->children[0].kind == CDD_CST_CHILD_TOKEN) {
          cdd_token_t *tok = child->children[0].val.token;
          if (tok->kind == CDD_TOKEN_OTHER) {
            /* It's a comment block */
          } else {
            should_break = 1;
          }
        }
      } else {
        should_break = 1;
      }
    } else if (target_parent->children[i].kind == CDD_CST_CHILD_TOKEN) {
      cdd_token_t *tok = target_parent->children[i].val.token;
      if (tok->kind == CDD_TOKEN_PREPROC_IFDEF ||
          tok->kind == CDD_TOKEN_PREPROC_IFNDEF ||
          tok->kind == CDD_TOKEN_PREPROC_DEFINE ||
          tok->kind == CDD_TOKEN_PREPROC_PRAGMA) {
        insert_idx = i + 1;
      } else if (tok->kind != CDD_TOKEN_EOF) {
        should_break = 1;
      }
    }
    if (should_break) {
      break;
    }
  }

  /* 3. Synthesize Top Nodes and process late includes */
  {
    cdd_cst_builder_t bld;
    int in_extern_c = 0;
    size_t j;
    cdd_cst_node_t *top_node =
        (cdd_cst_node_t *)calloc(1, sizeof(cdd_cst_node_t));
    if (top_node) {
      top_node->kind = CDD_CST_UNKNOWN;
      cdd_cst_builder_init(&bld, tree, top_node);
      cdd_cst_bld_newline(&bld);
      cdd_cst_bld_extern_c_open(&bld);
#ifdef CDD_BUILD_TESTS
      if (g_extern_c_top_node_fail)
        bld.error_state = 1;
#endif
      if (bld.error_state == 0) {
        /* Transplant leading trivia from the first token to our top_node so
         * comments stay at the top */
        if (insert_idx == 0 && first_token_of_file &&
            top_node->num_children > 0 &&
            top_node->children[0].kind == CDD_CST_CHILD_TOKEN) {
          cdd_token_t *top_first_tok = top_node->children[0].val.token;
          top_first_tok->leading_trivia = first_token_of_file->leading_trivia;
          first_token_of_file->leading_trivia = NULL;
        }

        if (insert_idx < target_parent->num_children) {
          rc =
              cdd_cst_insert_child_node_at(target_parent, insert_idx, top_node);
          if (rc != CDD_C_SUCCESS)
            return rc;
        } else {
          rc = cdd_cst_append_child_node(target_parent, top_node);
          if (rc != CDD_C_SUCCESS)
            return rc;
        }
        in_extern_c = 1;
      } else {
        free(top_node->children);
        free(top_node);
      }
      cdd_cst_builder_free(&bld);
    }

    /* Process children to find late includes that need to be excluded from
     * extern "C" */
    for (j = insert_idx + 1; j < target_parent->num_children; j++) {
      if (target_parent->children[j].kind == CDD_CST_CHILD_NODE) {
        cdd_cst_node_t *child = target_parent->children[j].val.node;
        if (child->kind == CDD_CST_PREPROC_DIRECTIVE) {
          if (child->num_children > 0 &&
              child->children[0].kind == CDD_CST_CHILD_TOKEN) {
            cdd_token_t *tok = child->children[0].val.token;
            if (tok->kind == CDD_TOKEN_PREPROC_INCLUDE) {
              /* Late include found! Close before it, open after it. */
              cdd_cst_node_t *close_node =
                  (cdd_cst_node_t *)calloc(1, sizeof(cdd_cst_node_t));
              cdd_cst_node_t *reopen_node =
                  (cdd_cst_node_t *)calloc(1, sizeof(cdd_cst_node_t));
              if (close_node && reopen_node && in_extern_c) {
                close_node->kind = CDD_CST_UNKNOWN;
                cdd_cst_builder_init(&bld, tree, close_node);
                cdd_cst_bld_newline(&bld);
                cdd_cst_bld_extern_c_close(&bld);
                rc = cdd_cst_insert_child_node_at(target_parent, j, close_node);
                if (rc != CDD_C_SUCCESS)
                  return rc;
                j++; /* skip the close node we just inserted */
                in_extern_c = 0;

                reopen_node->kind = CDD_CST_UNKNOWN;
                cdd_cst_builder_init(&bld, tree, reopen_node);
                cdd_cst_bld_newline(&bld);
                cdd_cst_bld_extern_c_open(&bld);
                rc = cdd_cst_insert_child_node_at(target_parent, j + 1,
                                                  reopen_node);
                if (rc != CDD_C_SUCCESS)
                  return rc;
                j++; /* skip the reopen node we just inserted */
                in_extern_c = 1;
              } else {
                if (close_node)
                  free(close_node);
                if (reopen_node)
                  free(reopen_node);
              }
            }
          }
        }
      }
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
#ifdef CDD_BUILD_TESTS
      if (g_extern_c_bot_node_fail)
        bld.error_state = 1;
#endif
      if (bld.error_state == 0) {
        if (target_parent->num_children > 0) {
          size_t bot_insert_idx = target_parent->num_children;
          cdd_cst_child_t last_child =
              target_parent->children[target_parent->num_children - 1];
          if (last_child.kind == CDD_CST_CHILD_TOKEN &&
              (last_child.val.token->kind == CDD_TOKEN_EOF ||
               last_child.val.token->kind == CDD_TOKEN_PREPROC_ENDIF)) {
            bot_insert_idx--;
          }
          if (bot_insert_idx < target_parent->num_children) {
            rc = cdd_cst_insert_child_node_at(target_parent, bot_insert_idx,
                                              bot_node);
            if (rc != CDD_C_SUCCESS)
              return rc;
          } else {
            rc = cdd_cst_append_child_node(target_parent, bot_node);
            if (rc != CDD_C_SUCCESS)
              return rc;
          }
        } else {
          rc = cdd_cst_append_child_node(target_parent, bot_node);
          if (rc != CDD_C_SUCCESS)
            return rc;
        }
      } else {
        free(bot_node->children);
        free(bot_node);
      }
      cdd_cst_builder_free(&bld);
    }
  }

  return CDD_C_SUCCESS;
}
