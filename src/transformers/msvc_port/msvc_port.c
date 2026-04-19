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

int cdd_transform_msvc(cdd_cst_tree_t *tree,
                       const cdd_transform_config_t *config) {
  cdd_cst_query_result_t res;
  size_t i;
  int rc;
  (void)config;

  if (!tree || !tree->root)
    return EINVAL;

  /* 1. Wrap unistd.h and sys/time.h */
  rc = cdd_cst_find_nodes_by_type(tree->root, CDD_CST_PREPROC_DIRECTIVE, &res);
  if (rc == 0) {
    for (i = 0; i < res.size; i++) {
      cdd_cst_node_t *dir = res.nodes[i];
      if (dir->num_children > 0 &&
          dir->children[0].kind == CDD_CST_CHILD_TOKEN) {
        cdd_token_t *tok = dir->children[0].val.token;
        if (tok->kind == CDD_TOKEN_PREPROC_INCLUDE && dir->num_children > 1 &&
            dir->children[1].kind == CDD_CST_CHILD_TOKEN) {
          cdd_token_t *inc_tok = dir->children[1].val.token;
          if (inc_tok->length == 10 &&
              memcmp(inc_tok->start, "<unistd.h>", 10) == 0) {
            cdd_cst_tree_t *wrap_tree = NULL;
            if (cdd_cst_parse(
                    az_span_create_from_str("#ifndef _MSC_VER\n#include "
                                            "<unistd.h>\n#else\n#include "
                                            "\"win_compat_sym.h\"\n#endif\n"),
                    &wrap_tree) == 0) {
              if (wrap_tree->root->num_children > 0) {
                cdd_cst_node_t *cloned = NULL;
                if (cdd_cst_clone_tree(tree,
                                       wrap_tree->root->children[0].val.node,
                                       &cloned) == 0) {
                  cdd_cst_replace_node(tree, dir, cloned);
                }
              }
              cdd_cst_tree_free(wrap_tree);
            }
          } else if (inc_tok->length == 12 &&
                     memcmp(inc_tok->start, "<sys/time.h>", 12) == 0) {
            cdd_cst_tree_t *wrap_tree = NULL;
            if (cdd_cst_parse(
                    az_span_create_from_str("#ifndef _MSC_VER\n#include "
                                            "<sys/time.h>\n#else\n#include "
                                            "\"win_compat_sym.h\"\n#endif\n"),
                    &wrap_tree) == 0) {
              if (wrap_tree->root->num_children > 0) {
                cdd_cst_node_t *cloned = NULL;
                if (cdd_cst_clone_tree(tree,
                                       wrap_tree->root->children[0].val.node,
                                       &cloned) == 0) {
                  cdd_cst_replace_node(tree, dir, cloned);
                }
              }
              cdd_cst_tree_free(wrap_tree);
            }
          }
        }
      }
    }
    free(res.nodes);
  }

  /* 2. Traverse tokens and replace POSIX identifiers directly */
  for (i = 0; i < tree->base_tokens->size; i++) {
    cdd_token_t *t = &tree->base_tokens->tokens[i];
    if (t->kind == CDD_TOKEN_IDENTIFIER) {
      if (t->length == 10 && memcmp(t->start, "strcasecmp", 10) == 0) {
        t->start = (const uint8_t *)"_stricmp";
        t->length = 8;
      } else if (t->length == 11 && memcmp(t->start, "strncasecmp", 11) == 0) {
        t->start = (const uint8_t *)"_strnicmp";
        t->length = 9;
      } else if (t->length == 6 && memcmp(t->start, "strdup", 6) == 0) {
        t->start = (const uint8_t *)"_strdup";
        t->length = 7;
      } else if (t->length == 7 && memcmp(t->start, "ssize_t", 7) == 0) {
        t->start = (const uint8_t *)"SSIZE_T";
        t->length = 7;
      } else if (t->length == 16 &&
                 memcmp(t->start, "__builtin_expect", 16) == 0) {
        t->start = (const uint8_t *)"cdd_builtin_expect";
        t->length = 18;
      }
    }
  }

  return 0;
}
