/**
 * @file cdd_cst_factory_format.c
 * @brief Implementation of formatting CST factory allocation functions.
 */

/* clang-format off */
#include "cdd_cst_factory.h"
#include "c_cdd/safe_crt.h"
#include "cdd_cst_mutate.h"
#include "cdd_cst_parser.h"
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "c_cdd/log.h"
/* clang-format on */

/**
 * @brief Parses a format string into a CST node.
 * @param dest_tree The destination tree.
 * @param out_node The parsed node output.
 * @param fmt The format string.
 * @param ... Additional arguments for format.
 * @return 0 on success.
 */
enum cdd_c_error cdd_cst_parse_format(cdd_cst_tree_t *dest_tree,
                                      cdd_cst_node_t **out_node,
                                      const char *fmt, ...) {
#ifdef CDD_BUILD_TESTS
  extern int g_cdd_cst_alloc_token_fail;
#endif
  char *buf;
  va_list args;
  cdd_cst_tree_t *temp_tree = NULL;
  cdd_cst_node_t *result = NULL;
  enum cdd_c_error rc;

  if (!dest_tree || !out_node || !fmt)
    return CDD_C_ERROR_INVALID_ARGUMENT;

#ifdef CDD_BUILD_TESTS
  if (g_cdd_cst_alloc_token_fail && --g_cdd_cst_alloc_token_fail == 0)
    buf = NULL;
  else
#endif
    buf = (char *)malloc(4096);
  if (!buf) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }

  va_start(args, fmt);
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat-nonliteral"
#endif
  CDD_VSNPRINTF(buf, 4096, fmt, args);
#if defined(__clang__)
#pragma clang diagnostic pop
#endif
  va_end(args);

  rc = cdd_cst_parse(az_span_create_from_str(buf), &temp_tree);
  free(buf);

  if (rc == CDD_C_SUCCESS) {
    size_t i, j;
    rc = cdd_cst_alloc_node(CDD_CST_UNKNOWN, &result);
    if (rc != CDD_C_SUCCESS) {
      cdd_cst_tree_free(temp_tree);
      *out_node = NULL;
      return rc;
    }

    for (i = 0; i < temp_tree->root->num_children; i++) {
      cdd_cst_node_t *stmt;
      cdd_cst_node_t *stmt_clone = NULL;

      if (temp_tree->root->children[i].kind == CDD_CST_CHILD_TOKEN)
        continue;

      stmt = temp_tree->root->children[i].val.node;

      rc = cdd_cst_clone_tree(dest_tree, stmt, &stmt_clone);
      if (rc != CDD_C_SUCCESS) {
        cdd_cst_free_node(result);
        cdd_cst_tree_free(temp_tree);
        *out_node = NULL;
        return rc;
      }

      for (j = 0; j < stmt_clone->num_children; j++) {
        if (stmt_clone->children[j].kind == CDD_CST_CHILD_TOKEN) {
          rc = cdd_cst_append_child_token(result,
                                          stmt_clone->children[j].val.token);
        } else {
          rc = cdd_cst_append_child_node(result,
                                         stmt_clone->children[j].val.node);
        }
        if (rc != CDD_C_SUCCESS) {
          size_t k;
          for (k = j; k < stmt_clone->num_children; k++) {
            if (stmt_clone->children[k].kind == CDD_CST_CHILD_NODE) {
              cdd_cst_free_node(stmt_clone->children[k].val.node);
            }
          }
          cdd_cst_free_node_only(stmt_clone);
          cdd_cst_free_node(result);
          cdd_cst_tree_free(temp_tree);
          *out_node = NULL;
          return rc;
        }
      }
      cdd_cst_free_node_only(stmt_clone);
    }
  }
  if (temp_tree) {
    cdd_cst_tree_free(temp_tree);
  }
  *out_node = result;
  return rc;
}
