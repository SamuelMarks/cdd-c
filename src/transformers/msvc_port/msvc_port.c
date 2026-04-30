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
/* clang-format on */

static void replace_msvc_identifiers(cdd_cst_tree_t *tree,
                                     cdd_cst_node_t *node) {
  size_t i;
  if (!node)
    return;
  for (i = 0; i < node->num_children; i++) {
    if (node->children[i].kind == CDD_CST_CHILD_TOKEN) {
      cdd_token_t *t = node->children[i].val.token;
      if (t->kind == CDD_TOKEN_IDENTIFIER) {
        cdd_token_t *new_tok = NULL;
        if (t->length == 10 && memcmp(t->start, "strcasecmp", 10) == 0) {
          cdd_cst_create_token(tree, CDD_TOKEN_IDENTIFIER, "_stricmp",
                               &new_tok);
        } else if (t->length == 11 &&
                   memcmp(t->start, "strncasecmp", 11) == 0) {
          cdd_cst_create_token(tree, CDD_TOKEN_IDENTIFIER, "_strnicmp",
                               &new_tok);
        } else if (t->length == 6 && memcmp(t->start, "strdup", 6) == 0) {
          cdd_cst_create_token(tree, CDD_TOKEN_IDENTIFIER, "_strdup", &new_tok);
        } else if (t->length == 7 && memcmp(t->start, "ssize_t", 7) == 0) {
          cdd_cst_create_token(tree, CDD_TOKEN_IDENTIFIER, "SSIZE_T", &new_tok);
        } else if (t->length == 16 &&
                   memcmp(t->start, "__builtin_expect", 16) == 0) {
          cdd_cst_create_token(tree, CDD_TOKEN_IDENTIFIER, "cdd_builtin_expect",
                               &new_tok);
        }
        if (new_tok) {
          new_tok->leading_trivia = t->leading_trivia;
          new_tok->trailing_trivia = t->trailing_trivia;
          t->leading_trivia = NULL;
          t->trailing_trivia = NULL;
          cdd_cst_replace_token_child(node, i, new_tok);
        }
      }
    } else if (node->children[i].kind == CDD_CST_CHILD_NODE) {
      replace_msvc_identifiers(tree, node->children[i].val.node);
    }
  }
}

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
        if (tok->kind == CDD_TOKEN_PREPROC_INCLUDE) {
          const char *inc_str = NULL;
          size_t t_i;
          int has_unistd = 0;
          int has_sys_time = 0;

          for (t_i = 0; t_i + 10 <= tok->length; ++t_i) {
            if (memcmp(tok->start + t_i, "<unistd.h>", 10) == 0) {
              has_unistd = 1;
              break;
            }
          }
          for (t_i = 0; t_i + 12 <= tok->length; ++t_i) {
            if (memcmp(tok->start + t_i, "<sys/time.h>", 12) == 0) {
              has_sys_time = 1;
              break;
            }
          }

          if (has_unistd) {
            inc_str = "unistd.h";
          } else if (has_sys_time) {
            inc_str = "sys/time.h";
          }

          if (inc_str) {
            cdd_cst_builder_t bld;
            cdd_cst_node_t *wrap_node =
                (cdd_cst_node_t *)calloc(1, sizeof(cdd_cst_node_t));
            if (wrap_node) {
              wrap_node->kind = CDD_CST_UNKNOWN;
              cdd_cst_builder_init(&bld, tree, wrap_node);
              cdd_cst_bld_ifndef(&bld, "_MSC_VER");
              cdd_cst_bld_include(&bld, inc_str, 1);
              cdd_cst_bld_else(&bld);
              cdd_cst_bld_include(&bld, "win_compat_sym.h", 0);
              cdd_cst_bld_endif(&bld);

              if (!cdd_cst_builder_has_error(&bld)) {
                cdd_cst_replace_node(tree, dir, wrap_node);
              } else {
                free(wrap_node->children);
                free(wrap_node);
              }
              cdd_cst_builder_free(&bld);
            }
          }
        }
      }
    }
    free(res.nodes);
  }

  /* 2. Traverse tokens and replace POSIX identifiers directly */
  replace_msvc_identifiers(tree, tree->root);

  return 0;
}
