/**
 * @file gnu_standardizer.c
 * @brief Implementation of the GNU standardizer transformer orchestrator.
 */

/* clang-format off */
#include "gnu_standardizer_internal.h"
/* clang-format on */

cdd_c_error_t gnu_standardize_fn(cdd_cst_tree_t *tree) {
  size_t i;
  cdd_c_error_t rc = CDD_C_SUCCESS;
  cdd_cst_query_result_t res = {0};

  if (!tree || !tree->root)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  cdd_cst_traverse_preorder(tree->root, cdd_asm_visitor, NULL);

  rc =
      cdd_cst_find_nodes_by_type(tree->root, CDD_CST_FUNCTION_DEFINITION, &res);
#ifdef CDD_BUILD_TESTS
  if (g_gnu_standardizer_fail == 34)
    rc = CDD_C_ERROR_MEMORY;
#endif
  if (rc != CDD_C_SUCCESS)
    return rc;
  for (i = 0; i < res.size; i++) {
    cdd_cst_node_t *func = res.nodes[i];
    size_t j;
    cdd_token_t *name_tok = NULL;
    for (j = 0; j < func->num_children; j++) {
      if (func->children[j].kind == CDD_CST_CHILD_TOKEN) {
        cdd_token_t *tok = func->children[j].val.token;
        if (tok->kind == CDD_TOKEN_LPAREN) {
          if (j > 0) {
            struct magic_ctx ctx;
            name_tok = func->children[j - 1].val.token;
            ctx.tree = tree;
            ctx.func_name = name_tok->start;
            ctx.func_len = name_tok->length;
            cdd_cst_traverse_preorder(func, cdd_magic_visitor, &ctx);
          }
          break;
        }
      }
    }

    /* Detect nested functions and trampolines */
    if (name_tok && func->parent->kind == CDD_CST_BLOCK) {
      struct tramp_ctx t_ctx;
      cdd_cst_node_t *parent_func;
      t_ctx.name = name_tok->start;
      t_ctx.length = name_tok->length;
      t_ctx.is_tramp = 0;
      t_ctx.func_node = func;

      /* Traverse the parent function (or the whole translation unit) to find
       * references */
      parent_func = func->parent;
      while (parent_func && parent_func->kind != CDD_CST_FUNCTION_DEFINITION) {
        parent_func = parent_func->parent;
      }
#ifdef CDD_BUILD_TESTS
      if (g_gnu_standardizer_fail == 14)
        parent_func = NULL;
#endif
      if (parent_func) {
        cdd_cst_traverse_preorder(parent_func, cdd_tramp_visitor, &t_ctx);
      } else {
        cdd_cst_traverse_preorder(tree->root, cdd_tramp_visitor, &t_ctx);
      }

      if (t_ctx.is_tramp) {
        /*
         * Lambda Lifting / Closure Conversion logic:
         * This would involve creating a shared closure struct holding local
         * variables and dynamically allocating it, or allocating a trampoline
         * executable thunk. For now, standard C89 cannot safely execute stack
         * trampolines.
         */
        free(res.nodes);
        return CDD_C_ERROR_SYSTEM;
      }
    }
  }
  free(res.nodes);
  return CDD_C_SUCCESS;
}

/**
 * @brief Applies GNU extension standardization transformations to the CST.
 */
cdd_c_error_t cdd_transform_gnu(cdd_cst_tree_t *tree,
                                const cdd_transform_config_t *config) {
  cdd_c_error_t rc;

  if (!tree || !tree->root)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  rc = gnu_standardize_fn(tree);
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc = gnu_standardize_types(tree);
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc = gnu_standardize_expr(tree);
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc = gnu_standardize_unroll(tree, config);
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc = gnu_standardize_ranges(tree);
  if (rc != CDD_C_SUCCESS)
    return rc;

  return CDD_C_SUCCESS;
}
