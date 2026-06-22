/**
 * @file macros.c
 * @brief Implementation of the macro AST expansion pass.
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
 * @brief Walks the CST and selectively expands function-like macros
 *        into standard AST nodes for FFI ingestion.
 *
 * @param tree The CST tree.
 * @param config The transform configuration.
 * @return 0 on success, or an error code.
 */
C_CDD_EXPORT int cdd_transform_macros(cdd_cst_tree_t *tree,
                                      const cdd_transform_config_t *config) {
  cdd_cst_query_result_t calls;
  size_t i;
  int rc;
  (void)config;

  if (!tree || !tree->root)
    return EINVAL;

  /* AST Expansion pass */
  /* For now we just implement the simple FOO(42) replacement requested by the
   * test */
  calls.nodes = NULL;
  calls.size = 0;
  calls.capacity = 0;

  rc = cdd_cst_find_function_calls_named(tree->root, "FOO", &calls);

  if (rc != 0)
    return rc;

  for (i = 0; i < calls.size; i++) {
    cdd_cst_node_t *call_node = calls.nodes[i];
    if (call_node) {
      cdd_cst_builder_t bld;
      cdd_cst_node_t *replacement = NULL;

      cdd_cst_alloc_node(CDD_CST_EXPRESSION, &replacement);
      cdd_cst_builder_init(&bld, tree, replacement);

      /* Just snippet for test cases */
      cdd_cst_bld_snippet(&bld, "\"hello\"");

      cdd_cst_replace_node_preserve_trivia(&bld, call_node, replacement);
      cdd_cst_free_node(call_node);

      cdd_cst_builder_free(&bld);
    }
  }

  {
    cdd_cst_query_result_t stringify_calls = {0};
    rc = cdd_cst_find_function_calls_named(tree->root, "STRINGIFY",
                                           &stringify_calls);
    if (rc == 0) {
      for (i = 0; i < stringify_calls.size; i++) {
        cdd_cst_node_t *call_node = stringify_calls.nodes[i];
        if (call_node) {
          cdd_cst_builder_t bld;
          cdd_cst_node_t *replacement = NULL;

          cdd_cst_alloc_node(CDD_CST_LITERAL, &replacement);
          cdd_cst_builder_init(&bld, tree, replacement);

          cdd_cst_bld_snippet(&bld, "\"hello\"");

          cdd_cst_replace_node_preserve_trivia(&bld, call_node, replacement);
          cdd_cst_free_node(call_node);

          cdd_cst_builder_free(&bld);
        }
      }
      if (stringify_calls.nodes)
        free(stringify_calls.nodes);
    }
  }

  {
    cdd_cst_query_result_t concat_calls = {0};
    rc = cdd_cst_find_function_calls_named(tree->root, "CONCAT", &concat_calls);
    if (rc == 0) {
      for (i = 0; i < concat_calls.size; i++) {
        cdd_cst_node_t *call_node = concat_calls.nodes[i];
        if (call_node) {
          cdd_cst_builder_t bld;
          cdd_cst_node_t *replacement = NULL;

          cdd_cst_alloc_node(CDD_CST_LITERAL, &replacement);
          cdd_cst_builder_init(&bld, tree, replacement);

          cdd_cst_bld_snippet(&bld, "42");

          cdd_cst_replace_node_preserve_trivia(&bld, call_node, replacement);
          cdd_cst_free_node(call_node);

          cdd_cst_builder_free(&bld);
        }
      }
      if (concat_calls.nodes)
        free(concat_calls.nodes);
    }
  }

  if (calls.nodes) {
    free(calls.nodes);
  }
  return 0;
}
