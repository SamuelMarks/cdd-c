/**
 * @file cdd_cst_factory_format.c
 * @brief Implementation of formatting CST factory allocation functions.
 */

/* clang-format off */
#include "cdd_cst_factory.h"
#include "cdd_cst_mutate.h"
#include "cdd_cst_parser.h"
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "c_cdd/log.h"
/* clang-format on */

int cdd_cst_parse_format(cdd_cst_tree_t *dest_tree, cdd_cst_node_t **out_node,
                         const char *fmt, ...) {
#ifdef CDD_BUILD_TESTS
  extern int g_cdd_cst_alloc_token_fail;
#endif
  char *buf;
  va_list args;
  cdd_cst_tree_t *temp_tree = NULL;
  cdd_cst_node_t *result = NULL;
  int rc;

  if (!dest_tree || !out_node || !fmt)
    return EINVAL;

#ifdef CDD_BUILD_TESTS
  if (g_cdd_cst_alloc_token_fail)
    buf = NULL;
  else
#endif
    buf = (char *)malloc(4096);
  if (!buf) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return ENOMEM;
  }

  va_start(args, fmt);
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat-nonliteral"
#endif
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  vsnprintf_s(buf, 4096, _TRUNCATE, fmt, args);
#else
  vsnprintf(buf, 4096, fmt, args);
#endif
#if defined(__clang__)
#pragma clang diagnostic pop
#endif
  va_end(args);

  rc = cdd_cst_parse(az_span_create_from_str(buf), &temp_tree);
  free(buf);

  if (rc == 0 && temp_tree && temp_tree->root) {
    size_t i, j;
    cdd_cst_alloc_node(CDD_CST_UNKNOWN, &result);
    if (!result) {
      cdd_cst_tree_free(temp_tree);
      *out_node = NULL;
      return ENOENT;
    }

    for (i = 0; i < temp_tree->root->num_children; i++) {
      cdd_cst_node_t *stmt;
      cdd_cst_node_t *stmt_clone = NULL;

      if (temp_tree->root->children[i].kind == CDD_CST_CHILD_TOKEN)
        continue;

      stmt = temp_tree->root->children[i].val.node;
      if (!stmt)
        continue;

      if (cdd_cst_clone_tree(dest_tree, stmt, &stmt_clone) == 0 && stmt_clone) {
        for (j = 0; j < stmt_clone->num_children; j++) {
          if (stmt_clone->children[j].kind == CDD_CST_CHILD_TOKEN) {
            cdd_cst_append_child_token(result,
                                       stmt_clone->children[j].val.token);
          } else {
            cdd_cst_append_child_node(result, stmt_clone->children[j].val.node);
          }
        }
        cdd_cst_free_node_only(stmt_clone);
      }
    }
  }
  if (temp_tree) {
    cdd_cst_tree_free(temp_tree);
  }
  *out_node = result;
  return rc;
}
