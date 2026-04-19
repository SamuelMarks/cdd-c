/* clang-format off */
#include "cdd_cst_factory.h"
#include "cdd_cst_mutate.h"
#include "cdd_cst_parser.h"
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

cdd_cst_node_t *cdd_cst_parse_format(cdd_cst_tree_t *dest_tree, const char *fmt, ...) {
    char *buf = (char *)malloc(4096);
    if (!buf) return NULL;
    va_list args;
    cdd_cst_tree_t *temp_tree = NULL;
    cdd_cst_node_t *result = NULL;
    int rc;

    va_start(args, fmt);
    vsnprintf(buf, 4096, fmt, args);
    va_end(args);

    rc = cdd_cst_parse(az_span_create_from_str(buf), &temp_tree);
    if (rc == 0 && temp_tree && temp_tree->root) {
        size_t i, j;
        result = cdd_cst_alloc_node(CDD_CST_UNKNOWN);
        if (!result) {
            cdd_cst_tree_free(temp_tree);
            return NULL;
        }

        for (i = 0; i < temp_tree->root->num_children; i++) {
            cdd_cst_node_t *stmt = temp_tree->root->children[i].val.node;
            cdd_cst_node_t *stmt_clone = NULL;
            if (!stmt) continue;
            
            if (cdd_cst_clone_tree(dest_tree, stmt, &stmt_clone) == 0 && stmt_clone) {
                for (j = 0; j < stmt_clone->num_children; j++) {
                    if (stmt_clone->children[j].kind == CDD_CST_CHILD_TOKEN) {
                        cdd_cst_append_child_token(result, stmt_clone->children[j].val.token);
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
    return result;
}
