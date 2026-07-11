/**
 * @file msvc_port.c
 * @brief Implementation of MSVC port transformer.
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
 * @brief Replaces POSIX identifiers with MSVC counterparts.
 *
 * @param[in,out] tree The CST tree.
 * @param[in,out] node The current node to process.
 */

static enum cdd_c_error replace_msvc_identifiers(
    cdd_cst_tree_t *tree, cdd_cst_node_t *node, cdd_token_t **prev_token,
    cdd_token_t **prev_prev_token, int *needs_basetsd, int *needs_expect) {
  size_t i;
  enum cdd_c_error rc = CDD_C_SUCCESS;
  for (i = 0; i < node->num_children; i++) {
    if (node->children[i].kind == CDD_CST_CHILD_TOKEN) {
      cdd_token_t *t = node->children[i].val.token;
      if (t->kind == CDD_TOKEN_IDENTIFIER) {
        int should_skip = 0;
        if (prev_token && *prev_token) {
          enum cdd_token_kind_t pk = (*prev_token)->kind;
          if (pk == CDD_TOKEN_DOT || pk == CDD_TOKEN_ARROW ||
              pk == CDD_TOKEN_KEYWORD_STRUCT ||
              pk == CDD_TOKEN_PREPROC_DEFINE) {
            should_skip = 1;
          }
          /* skip variable declarations by heuristic: if previous is a type
             keyword, or previous is * and prev_prev is a type keyword */
          if ((pk >= CDD_TOKEN_KEYWORD_INT && pk <= CDD_TOKEN_KEYWORD__ACCUM) ||
              pk == CDD_TOKEN_IDENTIFIER) {
            should_skip = 1;
          }
          if (pk == CDD_TOKEN_STAR && prev_prev_token && *prev_prev_token) {
            enum cdd_token_kind_t ppk = (*prev_prev_token)->kind;
            if ((ppk >= CDD_TOKEN_KEYWORD_INT &&
                 ppk <= CDD_TOKEN_KEYWORD__ACCUM) ||
                ppk == CDD_TOKEN_IDENTIFIER) {
              should_skip = 1;
            }
          }
        }

        if (!should_skip) {
          cdd_token_t *new_tok = NULL;
          if (t->length == 10 && memcmp(t->start, "strcasecmp", 10) == 0) {
            rc = cdd_cst_create_token(tree, CDD_TOKEN_IDENTIFIER, "_stricmp",
                                      &new_tok);
            if (rc != CDD_C_SUCCESS)
              return rc;
          } else if (t->length == 11 &&
                     memcmp(t->start, "strncasecmp", 11) == 0) {
            rc = cdd_cst_create_token(tree, CDD_TOKEN_IDENTIFIER, "_strnicmp",
                                      &new_tok);
            if (rc != CDD_C_SUCCESS)
              return rc;
          } else if (t->length == 6 && memcmp(t->start, "strdup", 6) == 0) {
            rc = cdd_cst_create_token(tree, CDD_TOKEN_IDENTIFIER, "_strdup",
                                      &new_tok);
            if (rc != CDD_C_SUCCESS)
              return rc;
          } else if (t->length == 7 && memcmp(t->start, "ssize_t", 7) == 0) {
            rc = cdd_cst_create_token(tree, CDD_TOKEN_IDENTIFIER, "SSIZE_T",
                                      &new_tok);
            if (rc != CDD_C_SUCCESS)
              return rc;
            if (needs_basetsd)
              *needs_basetsd = 1;
          } else if (t->length == 16 &&
                     memcmp(t->start, "__builtin_expect", 16) == 0) {
            rc = cdd_cst_create_token(tree, CDD_TOKEN_IDENTIFIER,
                                      "cdd_builtin_expect", &new_tok);
            if (rc != CDD_C_SUCCESS)
              return rc;
            if (needs_expect)
              *needs_expect = 1;
          } else if (t->length == 5 && memcmp(t->start, "off_t", 5) == 0) {
            rc = cdd_cst_create_token(tree, CDD_TOKEN_IDENTIFIER, "_off_t",
                                      &new_tok);
            if (rc != CDD_C_SUCCESS)
              return rc;
          } else if (t->length == 5 && memcmp(t->start, "pid_t", 5) == 0) {
            rc = cdd_cst_create_token(tree, CDD_TOKEN_IDENTIFIER, "int",
                                      &new_tok);
            if (rc != CDD_C_SUCCESS)
              return rc;
          } else if (t->length == 6 && memcmp(t->start, "mode_t", 6) == 0) {
            rc = cdd_cst_create_token(tree, CDD_TOKEN_IDENTIFIER, "int",
                                      &new_tok);
            if (rc != CDD_C_SUCCESS)
              return rc;
          } else if (t->length == 4 && memcmp(t->start, "open", 4) == 0) {
            rc = cdd_cst_create_token(tree, CDD_TOKEN_IDENTIFIER, "_open",
                                      &new_tok);
            if (rc != CDD_C_SUCCESS)
              return rc;
          } else if (t->length == 5 && memcmp(t->start, "close", 5) == 0) {
            rc = cdd_cst_create_token(tree, CDD_TOKEN_IDENTIFIER, "_close",
                                      &new_tok);
            if (rc != CDD_C_SUCCESS)
              return rc;
          } else if (t->length == 4 && memcmp(t->start, "read", 4) == 0) {
            rc = cdd_cst_create_token(tree, CDD_TOKEN_IDENTIFIER, "_read",
                                      &new_tok);
            if (rc != CDD_C_SUCCESS)
              return rc;
          } else if (t->length == 5 && memcmp(t->start, "write", 5) == 0) {
            rc = cdd_cst_create_token(tree, CDD_TOKEN_IDENTIFIER, "_write",
                                      &new_tok);
            if (rc != CDD_C_SUCCESS)
              return rc;
          } else if (t->length == 6 && memcmp(t->start, "fileno", 6) == 0) {
            rc = cdd_cst_create_token(tree, CDD_TOKEN_IDENTIFIER, "_fileno",
                                      &new_tok);
            if (rc != CDD_C_SUCCESS)
              return rc;
          } else if (t->length == 6 && memcmp(t->start, "unlink", 6) == 0) {
            rc = cdd_cst_create_token(tree, CDD_TOKEN_IDENTIFIER, "_unlink",
                                      &new_tok);
            if (rc != CDD_C_SUCCESS)
              return rc;
          } else if (t->length == 5 && memcmp(t->start, "mkdir", 5) == 0) {
            rc = cdd_cst_create_token(tree, CDD_TOKEN_IDENTIFIER, "_mkdir",
                                      &new_tok);
            if (rc != CDD_C_SUCCESS)
              return rc;
          } else if (t->length == 5 && memcmp(t->start, "rmdir", 5) == 0) {
            rc = cdd_cst_create_token(tree, CDD_TOKEN_IDENTIFIER, "_rmdir",
                                      &new_tok);
            if (rc != CDD_C_SUCCESS)
              return rc;
          } else if (t->length == 6 && memcmp(t->start, "getcwd", 6) == 0) {
            rc = cdd_cst_create_token(tree, CDD_TOKEN_IDENTIFIER, "_getcwd",
                                      &new_tok);
            if (rc != CDD_C_SUCCESS)
              return rc;
          } else if (t->length == 8 && memcmp(t->start, "snprintf", 8) == 0) {
            rc = cdd_cst_create_token(tree, CDD_TOKEN_IDENTIFIER, "_snprintf",
                                      &new_tok);
            if (rc != CDD_C_SUCCESS)
              return rc;
          } else if (t->length == 8 && memcmp(t->start, "strtok_r", 8) == 0) {
            rc = cdd_cst_create_token(tree, CDD_TOKEN_IDENTIFIER, "strtok_s",
                                      &new_tok);
            if (rc != CDD_C_SUCCESS)
              return rc;
          } else if (t->length == 5 && memcmp(t->start, "isnan", 5) == 0) {
            rc = cdd_cst_create_token(tree, CDD_TOKEN_IDENTIFIER, "_isnan",
                                      &new_tok);
            if (rc != CDD_C_SUCCESS)
              return rc;
          }
          if (new_tok) {
            new_tok->leading_trivia = t->leading_trivia;
            new_tok->trailing_trivia = t->trailing_trivia;
            t->leading_trivia = NULL;
            t->trailing_trivia = NULL;
            cdd_cst_replace_token_child(node, i, new_tok);
          }
        }
      }
      if (prev_token) {
        if (prev_prev_token) {
          *prev_prev_token = *prev_token;
        }
        *prev_token = t;
      }
    } else if (node->children[i].kind == CDD_CST_CHILD_NODE) {
      (void)replace_msvc_identifiers(tree, node->children[i].val.node,
                                     prev_token, prev_prev_token, needs_basetsd,
                                     needs_expect);
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the MSVC port transformer on a CST tree.
 *
 * @param[in,out] tree The CST tree.
 * @param[in] config The transformer configuration.
 * @return 0 on success, or an error code on failure.
 */
#ifdef CDD_BUILD_TESTS
C_CDD_EXPORT int g_msvc_port_bld_fail = 0;
#endif

enum cdd_c_error cdd_transform_msvc(cdd_cst_tree_t *tree,
                                    const cdd_transform_config_t *config) {
  cdd_cst_query_result_t res;
  size_t i;
  enum cdd_c_error rc = CDD_C_SUCCESS;

  int added_compat = 0;
  (void)config;

  if (!tree || !tree->root)
    return CDD_C_ERROR_INVALID_ARGUMENT;

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
              if (!added_compat) {
                cdd_cst_bld_else(&bld);
                cdd_cst_bld_include(&bld, "win_compat_sym.h", 0);
                added_compat = 1;
              }
              cdd_cst_bld_endif(&bld);

#ifdef CDD_BUILD_TESTS
              if (g_msvc_port_bld_fail)
                bld.error_state = 1;
#endif

              if (bld.error_state == 0) {
                cdd_cst_replace_node(tree, dir, wrap_node);
                cdd_cst_free_node(dir);
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
  {
    cdd_token_t *prev = NULL, *prev_prev = NULL;
    int needs_basetsd = 0;
    int needs_expect = 0;
    replace_msvc_identifiers(tree, tree->root, &prev, &prev_prev,
                             &needs_basetsd, &needs_expect);

    if (needs_basetsd || needs_expect) {
      /* Inject dependencies at the top of the translation unit */
      cdd_cst_builder_t bld;
      cdd_cst_node_t *deps_node =
          (cdd_cst_node_t *)calloc(1, sizeof(cdd_cst_node_t));
      if (deps_node) {
        deps_node->kind = CDD_CST_UNKNOWN;
        cdd_cst_builder_init(&bld, tree, deps_node);
        cdd_cst_bld_ifndef(&bld, "_MSC_VER");
        cdd_cst_bld_else(&bld);
        if (needs_basetsd) {
          cdd_cst_bld_include(&bld, "BaseTsd.h", 1);
        }
        if (needs_expect) {
          cdd_cst_bld_snippet(&bld,
                              "#define cdd_builtin_expect(exp, c) (exp)\n");
        }
        cdd_cst_bld_endif(&bld);
        if (bld.error_state == 0) {
          /* insert at start of root's children */
#ifdef CDD_BUILD_TESTS
          extern int g_msvc_port_bld_fail;
          if (g_msvc_port_bld_fail == 2) {
            rc = CDD_C_ERROR_MEMORY;
          } else {
            rc = cdd_cst_insert_child_node_at(tree->root, 0, deps_node);
          }
#else
          rc = cdd_cst_insert_child_node_at(tree->root, 0, deps_node);
#endif
          if (rc != CDD_C_SUCCESS) {
            free(deps_node->children);
            free(deps_node);
            cdd_cst_builder_free(&bld);
            return rc;
          }
        } else {
          free(deps_node->children);
          free(deps_node);
        }
        cdd_cst_builder_free(&bld);
      }
    }
  }

  return CDD_C_SUCCESS;
}
