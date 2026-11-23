/**
 * @file msvc_port.c
 * @brief Implementation of MSVC port transformer.
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
 * @brief Counts top-level comma-separated arguments in a call starting at child
 * `lparen_idx` of `node`, and optionally records the first comma and closing
 * `)` indices.
 *
 * @param[in] node CST node owning call tokens.
 * @param[in] lparen_idx Index of `(` child in `node->children`.
 * @param[out] out_arity Number of call arguments (0, 1, 2, ...).
 * @param[out] out_comma0 Child index of first top-level comma (if arity >= 2).
 * @param[out] out_rparen Child index of matching `)`.
 * @param[out] out_ok Set to 1 if matching `)` was found in `node`.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t msvc_inspect_call_arity(const cdd_cst_node_t *node,
                                             size_t lparen_idx, int *out_arity,
                                             size_t *out_comma0,
                                             size_t *out_rparen, int *out_ok) {
  size_t k;
  int depth = 0;
  int commas = 0;
  int non_empty = 0;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  *out_arity = 0;
  *out_comma0 = 0;
  *out_rparen = 0;
  *out_ok = 0;
  if (lparen_idx >= node->num_children ||
      node->children[lparen_idx].kind != CDD_CST_CHILD_TOKEN ||
      node->children[lparen_idx].val.token->kind != CDD_TOKEN_LPAREN) {
    return CDD_C_SUCCESS;
  }
  for (k = lparen_idx; k < node->num_children; k++) {
    if (node->children[k].kind != CDD_CST_CHILD_TOKEN) {
      non_empty = 1;
      continue;
    }
    {
      const cdd_token_t *tk = node->children[k].val.token;
      if (tk->kind == CDD_TOKEN_LPAREN) {
        depth++;
        if (depth > 1)
          non_empty = 1;
      } else if (tk->kind == CDD_TOKEN_RPAREN) {
        depth--;
        if (depth == 0) {
          *out_rparen = k;
          *out_arity = non_empty ? (commas + 1) : 0;
          *out_ok = 1;
          return CDD_C_SUCCESS;
        }
      } else {
        if (tk->length > 0)
          non_empty = 1;
        if (depth == 1 && tk->kind == CDD_TOKEN_COMMA) {
          if (commas == 0)
            *out_comma0 = k;
          commas++;
        }
      }
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Replaces POSIX identifiers with MSVC counterparts.
 *
 * @param[in,out] tree The CST tree.
 * @param[in,out] node The current node to process.
 * @param[in] config Optional transformation configuration.
 * @param[in,out] prev_token Pointer to previous token seen in traversal.
 * @param[in,out] prev_prev_token Pointer to token before previous token.
 * @param[out] needs_basetsd Set to 1 if BaseTsd.h (SSIZE_T) is needed.
 * @param[out] needs_expect Set to 1 if __builtin_expect macro polyfill is
 * needed.
 * @return CDD_C_SUCCESS on success or error code.
 */
static cdd_c_error_t replace_msvc_identifiers(
    cdd_cst_tree_t *tree, cdd_cst_node_t *node,
    const cdd_transform_config_t *config, cdd_token_t **prev_token,
    cdd_token_t **prev_prev_token, int *needs_basetsd, int *needs_expect) {
  size_t i;
  int with_awm = (config && config->with_auto_win_msvc);
  cdd_c_error_t rc = CDD_C_SUCCESS;
  for (i = 0; i < node->num_children; i++) {
    if (node->children[i].kind == CDD_CST_CHILD_TOKEN) {
      cdd_token_t *t = node->children[i].val.token;
      if (t->kind == CDD_TOKEN_IDENTIFIER) {
        int should_skip = 0;
        if (*prev_token) {
          enum cdd_token_kind_t pk = (*prev_token)->kind;
          if (pk == CDD_TOKEN_DOT || pk == CDD_TOKEN_ARROW ||
              pk == CDD_TOKEN_KEYWORD_STRUCT ||
              pk == CDD_TOKEN_PREPROC_DEFINE) {
            should_skip = 1;
          }
          /* skip variable declarations by heuristic: if previous is a type
             keyword, or previous is * and prev_prev is a type keyword */
          if (((pk >= CDD_TOKEN_KEYWORD_INT) &
               (pk <= CDD_TOKEN_KEYWORD__ACCUM)) |
              (pk == CDD_TOKEN_IDENTIFIER)) {
            should_skip = 1;
          }
          if (pk == CDD_TOKEN_STAR && *prev_prev_token) {
            enum cdd_token_kind_t ppk = (*prev_prev_token)->kind;
            if (((ppk >= CDD_TOKEN_KEYWORD_INT) &
                 (ppk <= CDD_TOKEN_KEYWORD__ACCUM)) |
                (ppk == CDD_TOKEN_IDENTIFIER)) {
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
            if (!with_awm) {
              rc = cdd_cst_create_token(tree, CDD_TOKEN_IDENTIFIER, "SSIZE_T",
                                        &new_tok);
              if (rc != CDD_C_SUCCESS)
                return rc;
              *needs_basetsd = 1;
            }
          } else if (t->length == 16 &&
                     memcmp(t->start, "__builtin_expect", 16) == 0) {
            rc = cdd_cst_create_token(tree, CDD_TOKEN_IDENTIFIER,
                                      "cdd_builtin_expect", &new_tok);
            if (rc != CDD_C_SUCCESS)
              return rc;
            *needs_expect = 1;
          } else if (t->length == 5 && memcmp(t->start, "off_t", 5) == 0 &&
                     !with_awm) {
            rc = cdd_cst_create_token(tree, CDD_TOKEN_IDENTIFIER, "_off_t",
                                      &new_tok);
            if (rc != CDD_C_SUCCESS)
              return rc;
          } else if (t->length == 5 && memcmp(t->start, "pid_t", 5) == 0 &&
                     !with_awm) {
            rc = cdd_cst_create_token(tree, CDD_TOKEN_IDENTIFIER, "int",
                                      &new_tok);
            if (rc != CDD_C_SUCCESS)
              return rc;
          } else if (t->length == 6 && memcmp(t->start, "mode_t", 6) == 0 &&
                     !with_awm) {
            rc = cdd_cst_create_token(tree, CDD_TOKEN_IDENTIFIER, "int",
                                      &new_tok);
            if (rc != CDD_C_SUCCESS)
              return rc;
          } else if (t->length == 4 && memcmp(t->start, "open", 4) == 0 &&
                     !with_awm) {
            rc = cdd_cst_create_token(tree, CDD_TOKEN_IDENTIFIER, "_open",
                                      &new_tok);
            if (rc != CDD_C_SUCCESS)
              return rc;
          } else if (t->length == 5 && memcmp(t->start, "close", 5) == 0 &&
                     !with_awm) {
            rc = cdd_cst_create_token(tree, CDD_TOKEN_IDENTIFIER, "_close",
                                      &new_tok);
            if (rc != CDD_C_SUCCESS)
              return rc;
          } else if (t->length == 4 && memcmp(t->start, "read", 4) == 0 &&
                     !with_awm) {
            rc = cdd_cst_create_token(tree, CDD_TOKEN_IDENTIFIER, "_read",
                                      &new_tok);
            if (rc != CDD_C_SUCCESS)
              return rc;
          } else if (t->length == 5 && memcmp(t->start, "write", 5) == 0 &&
                     !with_awm) {
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
          } else if (t->length == 5 && memcmp(t->start, "mkdir", 5) == 0 &&
                     !with_awm) {
            int arity = 0;
            size_t comma0 = 0;
            size_t rp = 0;
            int ok_a = 0;
            rc = msvc_inspect_call_arity(node, i + 1, &arity, &comma0, &rp,
                                         &ok_a);
            if (rc != CDD_C_SUCCESS)
              return rc;
            rc = cdd_cst_create_token(tree, CDD_TOKEN_IDENTIFIER, "_mkdir",
                                      &new_tok);
            if (rc != CDD_C_SUCCESS)
              return rc;
            if (ok_a && arity == 2) {
              size_t wk;
              for (wk = comma0; wk < rp; wk++) {
                if (node->children[wk].kind == CDD_CST_CHILD_TOKEN) {
                  cdd_token_t *wt = NULL;
                  rc = cdd_cst_create_token(tree, CDD_TOKEN_OTHER, "", &wt);
                  if (rc != CDD_C_SUCCESS)
                    return rc;
                  rc = cdd_cst_replace_token_child(node, wk, wt);
                  if (rc != CDD_C_SUCCESS)
                    return rc;
                }
              }
            }
          } else if (t->length == 7 && memcmp(t->start, "setmode", 7) == 0) {
            int arity = 0;
            size_t comma0 = 0;
            size_t rp = 0;
            int ok_a = 0;
            rc = msvc_inspect_call_arity(node, i + 1, &arity, &comma0, &rp,
                                         &ok_a);
            if (rc != CDD_C_SUCCESS)
              return rc;
            if (ok_a && arity == 1) {
              rc = cdd_cst_create_token(tree, CDD_TOKEN_IDENTIFIER,
                                        "bsd_setmode", &new_tok);
              if (rc != CDD_C_SUCCESS)
                return rc;
            } else if (ok_a && arity == 2 && !with_awm) {
              rc = cdd_cst_create_token(tree, CDD_TOKEN_IDENTIFIER, "_setmode",
                                        &new_tok);
              if (rc != CDD_C_SUCCESS)
                return rc;
            }
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
            rc = cdd_cst_replace_token_child(node, i, new_tok);
            if (rc != CDD_C_SUCCESS)
              return rc;
          }
        }
      }
      *prev_prev_token = *prev_token;
      *prev_token = t;
    } else {
      rc = replace_msvc_identifiers(tree, node->children[i].val.node, config,
                                    prev_token, prev_prev_token, needs_basetsd,
                                    needs_expect);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }
  return CDD_C_SUCCESS;
}

/*
 * @brief Ports POSIX/GNU specific syntax to MSVC equivalents.
 *
 * @param[in,out] tree CST tree to transform.
 * @param[in] config Optional transformation configuration.
 * @return 0 on success, or an error code.
 */
cdd_c_error_t cdd_transform_msvc(cdd_cst_tree_t *tree,
                                 const cdd_transform_config_t *config) {
  cdd_cst_query_result_t res;
  size_t i;
  cdd_c_error_t rc = CDD_C_SUCCESS;
  cdd_c_error_t rc_free = CDD_C_SUCCESS;
  int added_compat = (int)(config == NULL) & 0;

  if (!tree || !tree->root)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  /* 1. Wrap unistd.h and sys/time.h (skipped when --with-auto-win-msvc) */
  if (!config || !config->with_auto_win_msvc) {
    rc =
        cdd_cst_find_nodes_by_type(tree->root, CDD_CST_PREPROC_DIRECTIVE, &res);
    if (rc != CDD_C_SUCCESS) {
      return rc;
    }
    for (i = 0; i < res.size; i++) {
      cdd_cst_node_t *dir = res.nodes[i];
      {
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
                (cdd_cst_node_t *)C_CDD_CALLOC(1, sizeof(cdd_cst_node_t));
            if (!wrap_node) {
              C_CDD_FREE(res.nodes);
              return cdd_c_keep_err(CDD_C_ERROR_MEMORY, rc_free);
            }
            wrap_node->kind = CDD_CST_UNKNOWN;
            rc = cdd_cst_builder_init(&bld, tree, wrap_node);
            if (rc != CDD_C_SUCCESS) {
              cdd_c_error_t rc_f = cdd_cst_node_free(wrap_node);
              C_CDD_FREE(res.nodes);
              return cdd_c_keep_err(rc, cdd_c_keep_err(rc_free, rc_f));
            }
            rc = cdd_cst_bld_ifndef(&bld, "_MSC_VER");
            if (rc != CDD_C_SUCCESS) {
              goto wrap_fail;
            }
            rc = cdd_cst_bld_include(&bld, inc_str, 1);
            if (rc != CDD_C_SUCCESS) {
              goto wrap_fail;
            }
            if (!added_compat) {
              rc = cdd_cst_bld_else(&bld);
              if (rc != CDD_C_SUCCESS) {
                goto wrap_fail;
              }
              rc = cdd_cst_bld_include(&bld, "win_compat_sym.h", 0);
              if (rc != CDD_C_SUCCESS) {
                goto wrap_fail;
              }
              added_compat = 1;
            }
            rc = cdd_cst_bld_endif(&bld);
            if (rc != CDD_C_SUCCESS) {
              goto wrap_fail;
            }
            rc = cdd_cst_replace_node(tree, dir, wrap_node);
            if (rc != CDD_C_SUCCESS) {
              goto wrap_fail;
            }
            rc_free =
                (cdd_c_error_t)((int)rc_free | (int)cdd_cst_node_free(dir));
            rc_free =
                (cdd_c_error_t)((int)rc_free | (int)cdd_cst_builder_free(&bld));
            continue;

          wrap_fail: {
            cdd_c_error_t rc_f1 = cdd_cst_node_free(wrap_node);
            cdd_c_error_t rc_f2 = cdd_cst_builder_free(&bld);
            C_CDD_FREE(res.nodes);
            return cdd_c_keep_err(
                rc, cdd_c_keep_err(rc_free, cdd_c_keep_err(rc_f1, rc_f2)));
          }
          }
        }
      }
    }
    C_CDD_FREE(res.nodes);
  }

  /* 2. Traverse tokens and replace POSIX identifiers directly */
  {
    cdd_token_t *prev = NULL, *prev_prev = NULL;
    int needs_basetsd = 0;
    int needs_expect = 0;
    rc = replace_msvc_identifiers(tree, tree->root, config, &prev, &prev_prev,
                                  &needs_basetsd, &needs_expect);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, rc_free);
    }

    if (needs_basetsd || needs_expect) {
      /* Inject dependencies at the top of the translation unit */
      cdd_cst_builder_t bld;
      cdd_cst_node_t *deps_node =
          (cdd_cst_node_t *)C_CDD_CALLOC(1, sizeof(cdd_cst_node_t));
      if (!deps_node) {
        return cdd_c_keep_err(CDD_C_ERROR_MEMORY, rc_free);
      }
      deps_node->kind = CDD_CST_UNKNOWN;
      rc = cdd_cst_builder_init(&bld, tree, deps_node);
      if (rc != CDD_C_SUCCESS) {
        cdd_c_error_t rc_f = cdd_cst_node_free(deps_node);
        return cdd_c_keep_err(rc, cdd_c_keep_err(rc_free, rc_f));
      }
      rc = cdd_cst_bld_ifndef(&bld, "_MSC_VER");
      if (rc != CDD_C_SUCCESS) {
        goto deps_fail;
      }
      rc = cdd_cst_bld_else(&bld);
      if (rc != CDD_C_SUCCESS) {
        goto deps_fail;
      }
      if (needs_basetsd) {
        rc = cdd_cst_bld_include(&bld, "BaseTsd.h", 1);
        if (rc != CDD_C_SUCCESS) {
          goto deps_fail;
        }
      }
      if (needs_expect) {
        rc = cdd_cst_bld_snippet(&bld,
                                 "#define cdd_builtin_expect(exp, c) (exp)\n");
        if (rc != CDD_C_SUCCESS) {
          goto deps_fail;
        }
      }
      rc = cdd_cst_bld_endif(&bld);
      if (rc != CDD_C_SUCCESS) {
        goto deps_fail;
      }
      rc = cdd_cst_insert_child_node_at(tree->root, 0, deps_node);
      if (rc != CDD_C_SUCCESS) {
        goto deps_fail;
      }
      rc_free = (cdd_c_error_t)((int)rc_free | (int)cdd_cst_builder_free(&bld));
      goto after_deps;

    deps_fail: {
      cdd_c_error_t rc_f1 = cdd_cst_node_free(deps_node);
      cdd_c_error_t rc_f2 = cdd_cst_builder_free(&bld);
      return cdd_c_keep_err(
          rc, cdd_c_keep_err(rc_free, cdd_c_keep_err(rc_f1, rc_f2)));
    }
    }
  after_deps:;
  }

  /* 3. CST-accurate `--rename-main <symbol>` on file-scope `main(...)` */
  if (config && config->rename_main && config->rename_main[0] != '\0') {
    const char *pooled_rm = NULL;
    rc = cdd_pool_string_safe(tree, config->rename_main, &pooled_rm);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, rc_free);
    }
    for (i = 0; i < tree->root->num_children; i++) {
      if (tree->root->children[i].kind == CDD_CST_CHILD_NODE) {
        cdd_cst_node_t *top = tree->root->children[i].val.node;
        if (top->kind == CDD_CST_FUNCTION_DEFINITION ||
            top->kind == CDD_CST_UNKNOWN) {
          size_t k;
          int pd = 0;
          int is_td = 0;
          for (k = 0; k < top->num_children; k++) {
            if (top->children[k].kind == CDD_CST_CHILD_TOKEN) {
              cdd_token_t *tk = top->children[k].val.token;
              if (tk->length == 7 && memcmp(tk->start, "typedef", 7) == 0)
                is_td = 1;
              if (tk->kind == CDD_TOKEN_LPAREN) {
                if (pd == 0 && !is_td && k > 0 &&
                    top->children[k - 1].kind == CDD_CST_CHILD_TOKEN) {
                  cdd_token_t *nt = top->children[k - 1].val.token;
                  if (nt->kind == CDD_TOKEN_IDENTIFIER && nt->length == 4 &&
                      memcmp(nt->start, "main", 4) == 0) {
                    cdd_token_t *rt = NULL;
                    rc = cdd_cst_create_token(tree, CDD_TOKEN_IDENTIFIER,
                                              pooled_rm, &rt);
                    if (rc != CDD_C_SUCCESS) {
                      return cdd_c_keep_err(rc, rc_free);
                    }
                    rt->leading_trivia = nt->leading_trivia;
                    rt->trailing_trivia = nt->trailing_trivia;
                    nt->leading_trivia = NULL;
                    nt->trailing_trivia = NULL;
                    rc = cdd_cst_replace_token_child(top, k - 1, rt);
                    if (rc != CDD_C_SUCCESS) {
                      return cdd_c_keep_err(rc, rc_free);
                    }
                  }
                }
                pd++;
              } else if (tk->kind == CDD_TOKEN_RPAREN && pd > 0) {
                pd--;
              }
            }
          }
        }
      }
    }
  }

  /* 4. Inject `typedef int __awm_nonempty_tu;` on empty translation units */
  if (config && (config->with_auto_win_msvc ||
                 config->target_profile != CDD_TARGET_PROFILE_DEFAULT)) {
    int has_c_decl = 0;
    for (i = 0; i < tree->root->num_children; i++) {
      if (tree->root->children[i].kind == CDD_CST_CHILD_NODE) {
        cdd_cst_node_t *top = tree->root->children[i].val.node;
        if (top->kind != CDD_CST_PREPROC_DIRECTIVE) {
          size_t k;
          for (k = 0; k < top->num_children; k++) {
            if (top->children[k].kind == CDD_CST_CHILD_NODE) {
              has_c_decl = 1;
              break;
            }
            if (top->children[k].val.token->length > 0) {
              has_c_decl = 1;
              break;
            }
          }
        }
      }
      if (has_c_decl)
        break;
    }
    if (!has_c_decl) {
      cdd_cst_node_t *tu_node = NULL;
      cdd_cst_builder_t bld;
      rc = cdd_cst_alloc_node(CDD_CST_UNKNOWN, &tu_node);
      if (rc != CDD_C_SUCCESS) {
        return cdd_c_keep_err(rc, rc_free);
      }
      rc = cdd_cst_builder_init(&bld, tree, tu_node);
      if (rc != CDD_C_SUCCESS) {
        cdd_c_error_t rc_f = cdd_cst_node_free(tu_node);
        return cdd_c_keep_err(rc, cdd_c_keep_err(rc_free, rc_f));
      }
      rc = cdd_cst_bld_snippet(&bld, "typedef int __awm_nonempty_tu;\n");
      if (rc != CDD_C_SUCCESS) {
        cdd_c_error_t rc_f1 = cdd_cst_node_free(tu_node);
        cdd_c_error_t rc_f2 = cdd_cst_builder_free(&bld);
        return cdd_c_keep_err(
            rc, cdd_c_keep_err(rc_free, cdd_c_keep_err(rc_f1, rc_f2)));
      }
      rc = cdd_cst_insert_child_node_at(tree->root, tree->root->num_children,
                                        tu_node);
      if (rc != CDD_C_SUCCESS) {
        cdd_c_error_t rc_f1 = cdd_cst_node_free(tu_node);
        cdd_c_error_t rc_f2 = cdd_cst_builder_free(&bld);
        return cdd_c_keep_err(
            rc, cdd_c_keep_err(rc_free, cdd_c_keep_err(rc_f1, rc_f2)));
      }
      rc_free = (cdd_c_error_t)((int)rc_free | (int)cdd_cst_builder_free(&bld));
    }
  }

  return rc_free;
}
