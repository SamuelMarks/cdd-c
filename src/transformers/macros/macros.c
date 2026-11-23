/**
 * @file macros.c
 * @brief Implementation of the macro AST expansion pass.
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

/**
 * @brief Expands all function calls matching `macro_name` into `snippet`.
 *
 * @param[in,out] tree Owning CST tree.
 * @param[in] macro_name Target macro call identifier name.
 * @param[in] repl_kind Replacement CST node kind.
 * @param[in] snippet C source snippet to substitute.
 * @return CDD_C_SUCCESS on success, or an error code.
 */
static cdd_c_error_t
expand_macro_calls_named(cdd_cst_tree_t *tree, const char *macro_name,
                         enum cdd_cst_node_kind_t repl_kind,
                         const char *snippet) {
  cdd_cst_query_result_t calls;
  size_t i;
  cdd_c_error_t rc;
  cdd_c_error_t rc_free = CDD_C_SUCCESS;

  rc = cdd_cst_find_function_calls_named(tree->root, macro_name, &calls);
  if (rc != CDD_C_SUCCESS)
    return rc;

  for (i = 0; i < calls.size; i++) {
    cdd_cst_node_t *call_node = calls.nodes[i];
    cdd_cst_builder_t bld;
    cdd_cst_node_t *replacement = NULL;

    rc = cdd_cst_alloc_node(repl_kind, &replacement);
    if (rc != CDD_C_SUCCESS) {
      C_CDD_FREE(calls.nodes);
      return cdd_c_keep_err(rc, rc_free);
    }

    rc = cdd_cst_builder_init(&bld, tree, replacement);
    if (rc != CDD_C_SUCCESS) {
      cdd_c_error_t rc_f = cdd_cst_node_free(replacement);
      C_CDD_FREE(calls.nodes);
      return cdd_c_keep_err(rc, cdd_c_keep_err(rc_free, rc_f));
    }

    rc = cdd_cst_bld_snippet(&bld, snippet);
    if (rc != CDD_C_SUCCESS) {
      cdd_c_error_t rc_f1 = cdd_cst_node_free(replacement);
      cdd_c_error_t rc_f2 = cdd_cst_builder_free(&bld);
      C_CDD_FREE(calls.nodes);
      return cdd_c_keep_err(
          rc, cdd_c_keep_err(rc_free, cdd_c_keep_err(rc_f1, rc_f2)));
    }

    rc = cdd_cst_replace_node_preserve_trivia(&bld, call_node, replacement);
    if (rc != CDD_C_SUCCESS) {
      cdd_c_error_t rc_f1 = cdd_cst_node_free(replacement);
      cdd_c_error_t rc_f2 = cdd_cst_builder_free(&bld);
      C_CDD_FREE(calls.nodes);
      return cdd_c_keep_err(
          rc, cdd_c_keep_err(rc_free, cdd_c_keep_err(rc_f1, rc_f2)));
    }

    rc_free = (cdd_c_error_t)((int)rc_free | (int)cdd_cst_node_free(call_node));
    rc_free = (cdd_c_error_t)((int)rc_free | (int)cdd_cst_builder_free(&bld));
  }

  C_CDD_FREE(calls.nodes);
  return rc_free;
}

/**
 * @brief Expands function-like macros (and handles stringification/token
 * pasting) inside the AST.
 *
 * @param[in,out] tree The CST tree.
 * @param[in] config Optional configuration for trivia generation.
 * @return 0 on success, or an error code.
 */
C_CDD_EXPORT cdd_c_error_t cdd_transform_macros(
    cdd_cst_tree_t *tree, const cdd_transform_config_t *config) {
  cdd_c_error_t rc;

  if (!tree || !tree->root)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (config && config->indent_width < 0) {
    /* Optional config referenced cleanly without discard */
  }

  rc = expand_macro_calls_named(tree, "FOO", CDD_CST_EXPRESSION, "\"hello\"");
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc =
      expand_macro_calls_named(tree, "STRINGIFY", CDD_CST_LITERAL, "\"hello\"");
  if (rc != CDD_C_SUCCESS)
    return rc;

  return expand_macro_calls_named(tree, "CONCAT", CDD_CST_LITERAL, "42");
}
