/**
 * @file extern_c.c
 * @brief Implementation of the extern C wrapper transformer.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "c_cdd/memory.h"
#include "c_cdd_export.h"
#include "c_str_span.h"
#include "cdd_cst_transform.h"
#include "classes/parse/cdd_cst_builder.h"
#include "classes/parse/cdd_cst_factory.h"
#include "classes/parse/cdd_cst_mutate.h"
#include "classes/parse/cdd_cst_parser.h"
#include "classes/parse/cdd_cst_query.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

#ifdef CDD_BUILD_TESTS
extern volatile int g_fail_io_after;
C_CDD_EXPORT volatile int g_extern_c_top_node_fail = 0;
C_CDD_EXPORT volatile int g_extern_c_bot_node_fail = 0;
C_CDD_EXPORT volatile int g_extern_c_helper_fail = 0;
#endif

static cdd_c_error_t is_global_wrapper(cdd_cst_node_t *node,
                                       int *out_is_global) {
#ifdef CDD_BUILD_TESTS
  if (g_extern_c_helper_fail == -1)
    return CDD_C_ERROR_MEMORY;
#endif

  while (node != NULL) {
    switch (node->kind) {
    case CDD_CST_DECLARATION:
    case CDD_CST_FUNCTION_DEFINITION:
    case CDD_CST_STATEMENT:
    case CDD_CST_EXPRESSION:
    case CDD_CST_BLOCK:
    case CDD_CST_CLASS_DECLARATION:
      *out_is_global = 0;
      return CDD_C_SUCCESS;
    default:
      break;
    }
    node = node->parent;
  }
  *out_is_global = 1;
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks if a CST subtree contains C declarations.
 * @param[in] node Node to check.
 * @param[out] out_has_decl Pointer to int storing 1 if declarations found, 0
 * otherwise.
 * @return CDD_C_SUCCESS on success or error code.
 */
C_CDD_EXPORT cdd_c_error_t cdd_tree_has_decl(cdd_cst_node_t *node,
                                             int *out_has_decl) {
  size_t i;
  cdd_c_error_t rc = CDD_C_SUCCESS;

#ifdef CDD_BUILD_TESTS
  if (g_extern_c_helper_fail > 0) {
    if (--g_extern_c_helper_fail == 0)
      return CDD_C_ERROR_MEMORY;
  }
#endif

  *out_has_decl = 0;
  if (!node)
    return CDD_C_SUCCESS;

  for (i = 0; i < node->num_children; i++) {
    if (node->children[i].kind == CDD_CST_CHILD_NODE) {
      cdd_cst_node_t *child = node->children[i].val.node;
      enum cdd_cst_node_kind_t k;
      if (!child)
        continue;
      k = (enum cdd_cst_node_kind_t)child->kind;
      switch (k) {
      case CDD_CST_DECLARATION:
      case CDD_CST_FUNCTION_DEFINITION:
      case CDD_CST_STATEMENT:
        *out_has_decl = 1;
        return CDD_C_SUCCESS;
      case CDD_CST_UNKNOWN:
        if (child->num_children > 0) {
          if (child->children[0].kind == CDD_CST_CHILD_TOKEN) {
            cdd_token_t *tk = child->children[0].val.token;
            if (tk->kind == CDD_TOKEN_OTHER || tk->kind == CDD_TOKEN_EOF) {
              /* Comment or EOF */
            } else {
              *out_has_decl = 1;
              return CDD_C_SUCCESS;
            }
          }
        }
        break;
      default:
        break;
      }
      rc = cdd_tree_has_decl(child, out_has_decl);
      if (rc != CDD_C_SUCCESS)
        return rc;
      if (*out_has_decl)
        return CDD_C_SUCCESS;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks if a CST node represents an #ifdef __cplusplus guard.
 * @param[in] dir Node to check.
 * @param[out] out_is_cpp Pointer to int storing 1 if guard found, 0 otherwise.
 * @return CDD_C_SUCCESS on success or error code.
 */
C_CDD_EXPORT cdd_c_error_t cdd_check_node_is_cpp_guard(cdd_cst_node_t *dir,
                                                       int *out_is_cpp) {
  size_t j;
  cdd_token_t *tok;
  size_t k;

  *out_is_cpp = 0;
  if (!dir)
    return CDD_C_SUCCESS;
  if (dir->num_children == 0)
    return CDD_C_SUCCESS;
  if (dir->children[0].kind != CDD_CST_CHILD_TOKEN)
    return CDD_C_SUCCESS;
  if (dir->children[0].val.token->kind != CDD_TOKEN_PREPROC_IFDEF)
    return CDD_C_SUCCESS;

  tok = dir->children[0].val.token;
  for (k = 0; k + 11 <= tok->length; k++) {
    if (memcmp(tok->start + k, "__cplusplus", 11) == 0) {
      *out_is_cpp = 1;
      return CDD_C_SUCCESS;
    }
  }

  for (j = 1; j < dir->num_children; j++) {
    if (dir->children[j].kind == CDD_CST_CHILD_TOKEN) {
      cdd_token_t *ident = dir->children[j].val.token;
      if (ident->kind == CDD_TOKEN_IDENTIFIER && ident->length == 11 &&
          memcmp(ident->start, "__cplusplus", 11) == 0) {
        *out_is_cpp = 1;
        return CDD_C_SUCCESS;
      }
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Transforms CST tree to ensure proper extern "C" wrapping.
 * @param[in,out] tree Concrete syntax tree to transform.
 * @param[in] config Transformation configuration.
 * @return CDD_C_SUCCESS on success or error code.
 */
cdd_c_error_t cdd_transform_extern_c(cdd_cst_tree_t *tree,
                                     const cdd_transform_config_t *config) {
  cdd_cst_query_result_t res;
  size_t i;
  cdd_c_error_t rc = CDD_C_SUCCESS;
  int found_cpp = 0;
  size_t insert_idx = 0;
  cdd_cst_node_t *target_parent = NULL;
  cdd_token_t *first_token_of_file = NULL;
  int has_decl = 0;
  (void)config;

  if (!tree || !tree->root)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  /* Verify there are actual C declarations to wrap */
  rc = cdd_tree_has_decl(tree->root, &has_decl);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (!has_decl)
    return CDD_C_SUCCESS;

  /* 1. Check if __cplusplus is already checked globally */
  rc =
      cdd_cst_find_nodes_by_type(tree->root, CDD_CST_PREPROC_CONDITIONAL, &res);
  if (rc != CDD_C_SUCCESS)
    return rc;

  for (i = 0; i < res.size; i++) {
    int is_global = 0;
    cdd_cst_node_t *dir = res.nodes[i];
    rc = is_global_wrapper(dir, &is_global);
    if (rc != CDD_C_SUCCESS) {
      C_CDD_FREE(res.nodes);
      return rc;
    }
    if (is_global) {
      (void)cdd_check_node_is_cpp_guard(dir, &found_cpp);
      if (found_cpp)
        break;
    }
  }
  C_CDD_FREE(res.nodes);

  if (!found_cpp) {
    rc =
        cdd_cst_find_nodes_by_type(tree->root, CDD_CST_PREPROC_DIRECTIVE, &res);
    if (rc != CDD_C_SUCCESS)
      return rc;

    for (i = 0; i < res.size; i++) {
      int is_global = 0;
      cdd_cst_node_t *dir = res.nodes[i];
      rc = is_global_wrapper(dir, &is_global);
      if (rc != CDD_C_SUCCESS) {
        C_CDD_FREE(res.nodes);
        return rc;
      }
      if (is_global) {
        (void)cdd_check_node_is_cpp_guard(dir, &found_cpp);
        if (found_cpp)
          break;
      }
    }
    C_CDD_FREE(res.nodes);
  }

  if (found_cpp)
    return CDD_C_SUCCESS;

  /* Find the very first token in the file so we can transplant comments if
   * needed */
  for (i = 0; i < tree->root->num_children; i++) {
    if (tree->root->children[i].kind == CDD_CST_CHILD_NODE) {
      cdd_cst_node_t *ch = tree->root->children[i].val.node;
      if (ch->num_children > 0) {
        if (ch->children[0].kind == CDD_CST_CHILD_TOKEN) {
          first_token_of_file = ch->children[0].val.token;
          break;
        }
      }
    } else {
      first_token_of_file = tree->root->children[i].val.token;
      break;
    }
  }

  /* 2. Find insertion boundary: After #includes and comments, before C
   * declarations */
  target_parent = tree->root;
  if (tree->root->children[0].kind == CDD_CST_CHILD_NODE) {
    cdd_cst_node_t *first_child = tree->root->children[0].val.node;
    if (first_child->kind == CDD_CST_PREPROC_CONDITIONAL) {
      target_parent = first_child;
    }
  }

  for (i = 0; i < target_parent->num_children; i++) {
    int should_break = 0;
    if (target_parent->children[i].kind == CDD_CST_CHILD_NODE) {
      cdd_cst_node_t *child = target_parent->children[i].val.node;
      switch (child->kind) {
      case CDD_CST_PREPROC_DIRECTIVE:
        if (child->num_children > 0 &&
            child->children[0].kind == CDD_CST_CHILD_TOKEN) {
          cdd_token_t *tok = child->children[0].val.token;
          switch (tok->kind) {
          case CDD_TOKEN_PREPROC_INCLUDE:
          case CDD_TOKEN_PREPROC_DEFINE:
          case CDD_TOKEN_PREPROC_PRAGMA:
            insert_idx = i + 1;
            break;
          default:
            break;
          }
        }
        break;
      case CDD_CST_UNKNOWN:
        if (child->num_children > 0) {
          cdd_token_t *tok;
          if (child->children[0].kind != CDD_CST_CHILD_TOKEN) {
            should_break = 1;
            break;
          }
          tok = child->children[0].val.token;
          if (tok->kind != CDD_TOKEN_OTHER) {
            should_break = 1;
          }
        }
        break;
      default:
        should_break = 1;
        break;
      }
    } else {
      cdd_token_t *tok = target_parent->children[i].val.token;
      switch (tok->kind) {
      case CDD_TOKEN_PREPROC_IFDEF:
      case CDD_TOKEN_PREPROC_IFNDEF:
      case CDD_TOKEN_PREPROC_DEFINE:
      case CDD_TOKEN_PREPROC_PRAGMA:
        insert_idx = i + 1;
        break;
      default:
        if (tok->kind != CDD_TOKEN_EOF) {
          should_break = 1;
        }
        break;
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
        (cdd_cst_node_t *)C_CDD_CALLOC(1, sizeof(cdd_cst_node_t));
    if (!top_node)
      return CDD_C_ERROR_MEMORY;

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
      if (insert_idx == 0 && first_token_of_file) {
        cdd_token_t *top_first_tok = top_node->children[0].val.token;
        top_first_tok->leading_trivia = first_token_of_file->leading_trivia;
        first_token_of_file->leading_trivia = NULL;
      }

      if (insert_idx < target_parent->num_children) {
        rc = cdd_cst_insert_child_node_at(target_parent, insert_idx, top_node);
        if (rc != CDD_C_SUCCESS) {
          cdd_cst_free_node(top_node);
          cdd_cst_builder_free(&bld);
          return rc;
        }
      } else {
        rc = cdd_cst_append_child_node(target_parent, top_node);
        if (rc != CDD_C_SUCCESS) {
          cdd_cst_free_node(top_node);
          cdd_cst_builder_free(&bld);
          return rc;
        }
      }
      in_extern_c = 1;
    } else {
      cdd_cst_free_node(top_node);
    }
    cdd_cst_builder_free(&bld);

    /* Process children to find late includes that need to be excluded from
     * extern "C" */
    for (j = insert_idx + 1; j < target_parent->num_children; j++) {
      if (target_parent->children[j].kind == CDD_CST_CHILD_NODE) {
        cdd_cst_node_t *child = target_parent->children[j].val.node;
        if (child->kind == CDD_CST_PREPROC_DIRECTIVE) {
          if (child->num_children > 0 &&
              child->children[0].kind == CDD_CST_CHILD_TOKEN) {
            cdd_token_t *tok = child->children[0].val.token;
            if (tok->kind == CDD_TOKEN_PREPROC_INCLUDE && in_extern_c) {
              /* Late include found! Close before it, open after it. */
              cdd_cst_node_t *close_node;
              cdd_cst_node_t *reopen_node;

              close_node =
                  (cdd_cst_node_t *)C_CDD_CALLOC(1, sizeof(cdd_cst_node_t));
              if (!close_node)
                return CDD_C_ERROR_MEMORY;

              reopen_node =
                  (cdd_cst_node_t *)C_CDD_CALLOC(1, sizeof(cdd_cst_node_t));
              if (!reopen_node) {
                cdd_cst_free_node(close_node);
                return CDD_C_ERROR_MEMORY;
              }

              close_node->kind = CDD_CST_UNKNOWN;
              cdd_cst_builder_init(&bld, tree, close_node);
              cdd_cst_bld_newline(&bld);
              cdd_cst_bld_extern_c_close(&bld);
              rc = cdd_cst_insert_child_node_at(target_parent, j, close_node);
              cdd_cst_builder_free(&bld);
              if (rc != CDD_C_SUCCESS) {
                cdd_cst_free_node(close_node);
                cdd_cst_free_node(reopen_node);
                return rc;
              }
              j++; /* skip the close node we just inserted */
              in_extern_c = 0;

              reopen_node->kind = CDD_CST_UNKNOWN;
              cdd_cst_builder_init(&bld, tree, reopen_node);
              cdd_cst_bld_newline(&bld);
              cdd_cst_bld_extern_c_open(&bld);
              rc = cdd_cst_insert_child_node_at(target_parent, j + 1,
                                                reopen_node);
              cdd_cst_builder_free(&bld);
              if (rc != CDD_C_SUCCESS) {
                cdd_cst_free_node(reopen_node);
                return rc;
              }
              j += 2; /* skip the include node AND the reopen node we just
                         inserted */
              in_extern_c = 1;
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
        (cdd_cst_node_t *)C_CDD_CALLOC(1, sizeof(cdd_cst_node_t));
    if (!bot_node)
      return CDD_C_ERROR_MEMORY;

    bot_node->kind = CDD_CST_UNKNOWN;
    cdd_cst_builder_init(&bld, tree, bot_node);
    cdd_cst_bld_newline(&bld);
    cdd_cst_bld_extern_c_close(&bld);
#ifdef CDD_BUILD_TESTS
    if (g_extern_c_bot_node_fail == 1)
      bld.error_state = 1;
#endif
    if (bld.error_state == 0) {
      if (target_parent->num_children > 0) {
        size_t bot_insert_idx = target_parent->num_children;
        cdd_cst_child_t last_child =
            target_parent->children[target_parent->num_children - 1];
        if (last_child.kind == CDD_CST_CHILD_TOKEN) {
          switch (last_child.val.token->kind) {
          case CDD_TOKEN_EOF:
          case CDD_TOKEN_PREPROC_ENDIF:
            bot_insert_idx--;
            break;
          default:
            break;
          }
        }
        if (bot_insert_idx < target_parent->num_children) {
#ifdef CDD_BUILD_TESTS
          if (g_fail_io_after == 12345)
            rc = CDD_C_ERROR_MEMORY;
          else
#endif
            rc = cdd_cst_insert_child_node_at(target_parent, bot_insert_idx,
                                              bot_node);
          if (rc != CDD_C_SUCCESS) {
            cdd_cst_free_node(bot_node);
            cdd_cst_builder_free(&bld);
            return rc;
          }
        } else {
#ifdef CDD_BUILD_TESTS
          if (g_fail_io_after == 12346)
            rc = CDD_C_ERROR_MEMORY;
          else
#endif
            rc = cdd_cst_append_child_node(target_parent, bot_node);
          if (rc != CDD_C_SUCCESS) {
            cdd_cst_free_node(bot_node);
            cdd_cst_builder_free(&bld);
            return rc;
          }
        }
      } else {
        rc = cdd_cst_append_child_node(target_parent, bot_node);
        if (rc != CDD_C_SUCCESS) {
          cdd_cst_free_node(bot_node);
          cdd_cst_builder_free(&bld);
          return rc;
        }
      }
    } else {
      cdd_cst_free_node(bot_node);
    }
    cdd_cst_builder_free(&bld);
  }

  return CDD_C_SUCCESS;
}
