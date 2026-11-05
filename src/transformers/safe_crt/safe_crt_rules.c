/**
 * @file safe_crt_rules.c
 * @brief Core AST code emission dispatcher and size inference emitter for Safe
 * CRT transformer.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "transformers/safe_crt/safe_crt_internal.h"
#include "classes/parse/cdd_cst_builder.h"
#include "classes/parse/cdd_cst_factory.h"
#include "c_cdd/safe_crt.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

#ifdef CDD_BUILD_TESTS
C_CDD_EXPORT int g_mock_safe_crt_rules_fail = 0;

/**
 * @brief Checks and decrements the test fault injection counter for Safe CRT
 * rules.
 *
 * @param[in] current Current error status code.
 * @return CDD_C_ERROR_UNKNOWN when fault triggers, otherwise @p current.
 */
C_CDD_EXPORT cdd_c_error_t safe_crt_check_mock_fail(cdd_c_error_t current) {
  if (g_mock_safe_crt_rules_fail) {
    g_mock_safe_crt_rules_fail--;
    if (g_mock_safe_crt_rules_fail == 0) {
      return CDD_C_ERROR_UNKNOWN;
    }
  }
  return current;
}
#endif

/** @brief Alias for safe_crt_check_mock_fail */
#define check_mock_fail(x) safe_crt_check_mock_fail(x)

/**
 * @brief Emits an expression AST with stripped leading trivia.
 *
 * @param[in] node Expression AST node to emit.
 * @param[in,out] bld CST builder.
 * @param[in] is_msc Non-zero if targeting MSC Safe CRT.
 * @return CDD_C_SUCCESS on success or error code.
 */
C_CDD_EXPORT cdd_c_error_t emit_ast_bld_strip(expr_t *node,
                                              cdd_cst_builder_t *bld,
                                              int is_msc) {
  cdd_c_error_t rc = CDD_C_SUCCESS;
  size_t old_num_children =
      bld->target_node ? bld->target_node->num_children : 0;
  {
    int _ch = 0;
    rc = emit_ast_bld(node, bld, is_msc, &_ch);
  }
  if (bld->target_node && bld->target_node->num_children > old_num_children) {
    cdd_trivia_t *tr =
        bld->target_node->children[old_num_children].val.token->leading_trivia;
    while (tr) {
      cdd_trivia_t *next = tr->next;
      C_CDD_FREE(tr);
      tr = next;
    }
    bld->target_node->children[old_num_children].val.token->leading_trivia =
        NULL;
  }
  return (cdd_c_error_t)rc;
}

/**
 * @brief Emits an expression AST stripping leading ampersand if present.
 *
 * @param[in] node Expression AST node to emit.
 * @param[in,out] bld CST builder.
 * @param[in] is_msc Non-zero if targeting MSC Safe CRT.
 * @return CDD_C_SUCCESS on success or error code.
 */
C_CDD_EXPORT cdd_c_error_t emit_ast_bld_strip_ampersand(expr_t *node,
                                                        cdd_cst_builder_t *bld,
                                                        int is_msc) {
  if (node && node->type == 0 && node->tok && node->tok->length == 1 &&
      node->tok->start[0] == '&') {
    return emit_ast_bld_strip(node->next, bld, is_msc);
  }
  return emit_ast_bld_strip(node, bld, is_msc);
}

/**
 * @brief Emits the inferred buffer size expression into builder.
 *
 * @param[in,out] bld Target CST builder.
 * @param[in] dest Destination buffer expression AST node to infer size from.
 * @return CDD_C_SUCCESS on success or error code.
 */
C_CDD_EXPORT cdd_c_error_t emit_inferred_size(cdd_cst_builder_t *bld,
                                              expr_t *dest) {
  inferred_size_t info;
  cdd_c_error_t rc_err = CDD_C_SUCCESS;
  if (!bld)
    return CDD_C_SUCCESS;
  if (bld->tree) {
    current_tree = bld->tree;
  } else {
    current_tree = NULL;
  }
  rc_err = infer_buffer_size(dest, &info);
  if (!info.valid) {
    {
      rc_err = cdd_cst_bld_ident(bld, "sizeof");
      if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
        return rc_err;
    }
    {
      rc_err = cdd_cst_bld_punct(bld, "(");
      if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
        return rc_err;
    }
    {
      rc_err = emit_ast_bld_strip(dest, bld, 0);
      if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
        return rc_err;
    }
    {
      rc_err = cdd_cst_bld_punct(bld, ")");
      if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
        return rc_err;
    }
    return CDD_C_SUCCESS;
  }

  if (info.offset_expr) {
    rc_err = cdd_cst_bld_punct(bld, "(");
    if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
      return rc_err;
  }

  if (info.is_malloc == 1) {
    {
      rc_err = emit_ast_bld_strip(info.malloc_size_expr, bld, 0);
      if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
        return rc_err;
    }
  } else if (info.is_malloc == 2) {
    {
      rc_err = cdd_cst_bld_punct(bld, "(");
      if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
        return rc_err;
    }
    {
      rc_err = emit_ast_bld_strip(info.malloc_size_expr->args[0], bld, 0);
      if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
        return rc_err;
    }
    {
      rc_err = cdd_cst_bld_punct(bld, ")");
      if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
        return rc_err;
    }
    {
      rc_err = cdd_cst_bld_space(bld);
      if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
        return rc_err;
    }
    {
      rc_err = cdd_cst_bld_punct(bld, "*");
      if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
        return rc_err;
    }
    {
      rc_err = cdd_cst_bld_space(bld);
      if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
        return rc_err;
    }
    {
      rc_err = cdd_cst_bld_punct(bld, "(");
      if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
        return rc_err;
    }
    {
      rc_err = emit_ast_bld_strip(info.malloc_size_expr->args[1], bld, 0);
      if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
        return rc_err;
    }
    {
      rc_err = cdd_cst_bld_punct(bld, ")");
      if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
        return rc_err;
    }
  } else {
    cdd_token_t *ct = NULL;
    {
      rc_err = cdd_cst_bld_ident(bld, "sizeof");
      if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
        return rc_err;
    }
    {
      rc_err = cdd_cst_bld_punct(bld, "(");
      if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
        return rc_err;
    }

    rc_err = clone_token(bld->tree, info.base_tok, &ct);
    if (rc_err == CDD_C_SUCCESS) {
      if (ct->leading_trivia) {
        cdd_trivia_t *curr = ct->leading_trivia;
        while (curr) {
          cdd_trivia_t *nxt = curr->next;
          C_CDD_FREE(curr);
          curr = nxt;
        }
      }
      if (ct->trailing_trivia) {
        cdd_trivia_t *curr = ct->trailing_trivia;
        while (curr) {
          cdd_trivia_t *nxt = curr->next;
          C_CDD_FREE(curr);
          curr = nxt;
        }
      }
      ct->leading_trivia = NULL;
      ct->trailing_trivia = NULL;
      {
        rc_err = cdd_cst_append_child_token(bld->target_node, ct);
        if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
          return rc_err;
      }
    }

    {
      rc_err = cdd_cst_bld_punct(bld, ")");
      if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
        return rc_err;
    }
  }

  if (info.offset_expr) {
    {
      rc_err = cdd_cst_bld_space(bld);
      if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
        return rc_err;
    }
    {
      rc_err = cdd_cst_bld_punct(bld, "-");
      if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
        return rc_err;
    }
    {
      rc_err = cdd_cst_bld_space(bld);
      if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
        return rc_err;
    }
    {
      rc_err = cdd_cst_bld_punct(bld, "(");
      if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
        return rc_err;
    }
    {
      rc_err = emit_ast_bld_strip(info.offset_expr, bld, 0);
      if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
        return rc_err;
    }
    {
      rc_err = cdd_cst_bld_punct(bld, ")");
      if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
        return rc_err;
    }
    {
      rc_err = cdd_cst_bld_punct(bld, ")");
      if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
        return rc_err;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Classifies a C CRT function name into a Safe CRT transformation
 * category.
 *
 * @param[in] name Function identifier name.
 * @param[out] out_is_safe Pointer to receive Safe CRT category (0 if not
 * transformed).
 * @return CDD_C_SUCCESS on success.
 */
static cdd_c_error_t classify_safe_crt_call(const char *name,
                                            int *out_is_safe) {
  int is_safe = 0;
  if (strcmp(name, "strcpy") == 0 || strcmp(name, "strcat") == 0 ||
      strcmp(name, "sprintf") == 0 || strcmp(name, "vsprintf") == 0 ||
      strcmp(name, "wcscpy") == 0 || strcmp(name, "wcscat") == 0 ||
      strcmp(name, "_mbscpy") == 0 || strcmp(name, "_mbscat") == 0 ||
      strcmp(name, "swprintf") == 0 || strcmp(name, "vswprintf") == 0 ||
      strcmp(name, "_strnset") == 0 || strcmp(name, "_strset") == 0 ||
      strcmp(name, "_mbsset") == 0)
    is_safe = 1;
  else if (strcmp(name, "_strlwr") == 0 || strcmp(name, "_strupr") == 0 ||
           strcmp(name, "_mbslwr") == 0 || strcmp(name, "_mbsupr") == 0 ||
           strcmp(name, "_wcslwr") == 0 || strcmp(name, "_wcsupr") == 0 ||
           strcmp(name, "gets") == 0 || strcmp(name, "tmpnam") == 0 ||
           strcmp(name, "strlen") == 0)
    is_safe = 9;
  else if (strcmp(name, "strncpy") == 0 || strcmp(name, "strncat") == 0 ||
           strcmp(name, "wcsncpy") == 0 || strcmp(name, "wcsncat") == 0 ||
           strcmp(name, "_mbsncpy") == 0 || strcmp(name, "_mbsncat") == 0 ||
           strcmp(name, "_mbsnset") == 0)
    is_safe = 2;
  else if (strcmp(name, "snprintf") == 0 || strcmp(name, "vsnprintf") == 0 ||
           strcmp(name, "_snprintf") == 0 || strcmp(name, "_vsnprintf") == 0)
    is_safe = 3;
  else if (strcmp(name, "printf") == 0 || strcmp(name, "vprintf") == 0 ||
           strcmp(name, "vfprintf") == 0)
    is_safe = 4;
  else if (strcmp(name, "memcpy") == 0 || strcmp(name, "memmove") == 0 ||
           strcmp(name, "wmemcpy") == 0 || strcmp(name, "wmemmove") == 0)
    is_safe = 6;
  else if (strcmp(name, "scanf") == 0 || strcmp(name, "fscanf") == 0 ||
           strcmp(name, "sscanf") == 0 || strcmp(name, "vscanf") == 0 ||
           strcmp(name, "vfscanf") == 0 || strcmp(name, "vsscanf") == 0)
    is_safe = 7;
  else if (strcmp(name, "_itoa") == 0 || strcmp(name, "_ltoa") == 0 ||
           strcmp(name, "_ultoa") == 0 || strcmp(name, "_i64toa") == 0 ||
           strcmp(name, "_ui64toa") == 0 || strcmp(name, "_itow") == 0 ||
           strcmp(name, "_ltow") == 0 || strcmp(name, "_ultow") == 0)
    is_safe = 8;
  else if (strcmp(name, "_splitpath") == 0 || strcmp(name, "_wsplitpath") == 0)
    is_safe = 10;
  else if (strcmp(name, "_makepath") == 0 || strcmp(name, "_wmakepath") == 0)
    is_safe = 11;
  else if (strcmp(name, "_gcvt") == 0)
    is_safe = 19;
  else if (strcmp(name, "mbstowcs") == 0 || strcmp(name, "wcstombs") == 0)
    is_safe = 20;
  else if (strcmp(name, "wctomb") == 0)
    is_safe = 21;
  else if (strcmp(name, "_searchenv") == 0 || strcmp(name, "_wsearchenv") == 0)
    is_safe = 22;
  else if (strcmp(name, "_wgetenv") == 0 || strcmp(name, "getenv") == 0)
    is_safe = 23;
  else if (strcmp(name, "_putenv") == 0 || strcmp(name, "_wputenv") == 0)
    is_safe = 24;
  else if (strcmp(name, "strerror") == 0 || strcmp(name, "_strerror") == 0)
    is_safe = 27;
  else if (strcmp(name, "_wcserror") == 0)
    is_safe = 26;
  else if (strcmp(name, "_ecvt") == 0 || strcmp(name, "_fcvt") == 0)
    is_safe = 28;
  else if (strcmp(name, "strtok") == 0 || strcmp(name, "wcstok") == 0 ||
           strcmp(name, "_mbstok") == 0)
    is_safe = 29;
  else if (strcmp(name, "qsort") == 0 || strcmp(name, "bsearch") == 0)
    is_safe = 25;

  *out_is_safe = is_safe;
  return CDD_C_SUCCESS;
}

/**
 * @brief Emits transformed AST tokens into the CST builder.
 *
 * @param[in] node Expression node.
 * @param[in,out] bld CST builder.
 * @param[in] is_msc Non-zero if targeting MSC Safe CRT.
 * @param[out] out_changes Pointer to receive number of safe CRT changes
 * emitted.
 * @return CDD_C_SUCCESS on success or error code.
 */
cdd_c_error_t emit_ast_bld(expr_t *node, cdd_cst_builder_t *bld, int is_msc,
                           int *out_changes) {
  int changes = 0;
  size_t k;
  emit_ctx_t *ctx = is_msc ? g_msc_ctx : NULL;
  cdd_cst_tree_t *tree = bld->tree;
  cdd_c_error_t rc_err = CDD_C_SUCCESS;
  int _ch = 0;

  while (node) {
    if (node->type == 0 || (node->type == 4 && !is_msc)) {
      cdd_token_t *ct = NULL;
      rc_err = clone_token(bld->tree, node->tok, &ct);
      if (rc_err == CDD_C_SUCCESS) {
        rc_err = cdd_cst_append_child_token(bld->target_node, ct);
        if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
          return rc_err;
      }
    } else if (node->type == 2) {
      cdd_token_t *ct = NULL;
      rc_err = clone_token(bld->tree, node->tok, &ct);
      if (rc_err == CDD_C_SUCCESS) {
        rc_err = cdd_cst_append_child_token(bld->target_node, ct);
        if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
          return rc_err;
      }
      {
        rc_err = emit_ast_bld(node->args[0], bld, is_msc, &_ch);
        if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
          return rc_err;
        changes += _ch;
      }
      if (node->close_tok) {
        cdd_token_t *ct_close = NULL;
        rc_err = clone_token(bld->tree, node->close_tok, &ct_close);
        if (rc_err == CDD_C_SUCCESS) {
          rc_err = cdd_cst_append_child_token(bld->target_node, ct_close);
          if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
            return rc_err;
        }
      }
    } else if (node->type == 1 || (node->type == 5 && !is_msc)) {
      char name[128] = {0};
      size_t nlen =
          (node->tok && node->tok->length < 127) ? node->tok->length : 127;
      int is_safe = 0;

      if (node->tok)
        memcpy(name, node->tok->start, nlen);

      if (is_msc) {
        rc_err = classify_safe_crt_call(name, &is_safe);
      }

      if (is_safe > 0)
        changes++;

      if (is_safe && is_safe != 19 && is_safe != 20 && is_safe != 21 &&
          is_safe != 22 && is_safe != 23 && is_safe != 24 && is_safe != 25 &&
          is_safe != 26 && is_safe != 27 && is_safe != 28 && is_safe != 29) {
        char safe_name[256];
        cdd_token_t *ct = NULL;
        if (strcmp(name, "strlen") == 0) {
#if defined(_MSC_VER)
          strcpy_s(safe_name, sizeof(safe_name), "strnlen_s");
#else
          strcpy(safe_name, "strnlen_s");
#endif
        } else {
          CDD_SNPRINTF(safe_name, sizeof(safe_name), "%s_s", name);
        }

        rc_err = clone_token(bld->tree, node->tok, &ct);
        if (rc_err == CDD_C_SUCCESS) {
          const char *pooled = NULL;
          if (safe_crt_pool_string_safe(tree, safe_name, &pooled) ==
              CDD_C_SUCCESS) {
            ct->start = (const uint8_t *)pooled;
            ct->length = strlen(pooled);
          }
          {
            rc_err = cdd_cst_append_child_token(bld->target_node, ct);
            if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
              return rc_err;
          }
        }
        {
          rc_err = cdd_cst_bld_punct(bld, "(");
          if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
            return rc_err;
        }
      } else if (!is_safe) {
        cdd_token_t *ct = NULL;
        rc_err = clone_token(bld->tree, node->tok, &ct);
        if (rc_err == CDD_C_SUCCESS) {
          rc_err = cdd_cst_append_child_token(bld->target_node, ct);
          if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
            return rc_err;
        }
        {
          rc_err = cdd_cst_bld_punct(bld, "(");
          if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
            return rc_err;
        }
      } else {
        cdd_token_t *ct = NULL;
        rc_err = clone_token(bld->tree, node->tok, &ct);
        if (rc_err == CDD_C_SUCCESS) {
          ct->length = 0;
          ct->start = (const uint8_t *)"";
          {
            rc_err = cdd_cst_append_child_token(bld->target_node, ct);
            if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
              return rc_err;
          }
        }
      }

      if ((is_safe == 1 && node->num_args >= 2) ||
          (is_safe == 2 && node->num_args >= 3) ||
          (is_safe == 6 && node->num_args >= 3) ||
          (is_safe == 9 && node->num_args == 1)) {
        rc_err = emit_safe_crt_str_basic(node, bld, is_msc, is_safe, &changes);
        if (rc_err != CDD_C_SUCCESS)
          return rc_err;
      } else if (is_safe == 3 && node->num_args >= 3) {
        rc_err = emit_safe_crt_snprintf(node, bld, is_msc, &changes);
        if (rc_err != CDD_C_SUCCESS)
          return rc_err;
      } else if (is_safe == 8 && node->num_args >= 3) {
        rc_err = emit_safe_crt_itoa(node, bld, is_msc, &changes);
        if (rc_err != CDD_C_SUCCESS)
          return rc_err;
      } else if ((is_safe == 10 || is_safe == 11) && node->num_args >= 5) {
        rc_err = emit_safe_crt_path(node, bld, is_msc, is_safe, &changes);
        if (rc_err != CDD_C_SUCCESS)
          return rc_err;
      } else if ((is_safe == 19 && node->num_args == 3) ||
                 (is_safe == 20 && node->num_args == 3) ||
                 (is_safe == 21 && node->num_args == 2)) {
        rc_err =
            emit_safe_crt_gcvt_mbs(node, bld, is_msc, is_safe, name, &changes);
        if (rc_err != CDD_C_SUCCESS)
          return rc_err;
      } else if ((is_safe == 22 && node->num_args == 3) ||
                 (is_safe == 23 && node->num_args == 1) ||
                 (is_safe == 24 && node->num_args == 1)) {
        rc_err =
            emit_safe_crt_env(node, bld, is_msc, is_safe, name, ctx, &changes);
        if (rc_err != CDD_C_SUCCESS)
          return rc_err;
      } else if ((is_safe == 25 &&
                  (node->num_args == 4 || node->num_args == 5)) ||
                 is_safe == 29) {
        rc_err = emit_safe_crt_search_tok(node, bld, is_msc, is_safe, name, ctx,
                                          &changes);
        if (rc_err != CDD_C_SUCCESS)
          return rc_err;
      } else if (is_safe == 26 || is_safe == 27 || is_safe == 28) {
        rc_err = emit_safe_crt_err_fcvt(node, bld, is_msc, is_safe, name, ctx,
                                        &changes);
        if (rc_err != CDD_C_SUCCESS)
          return rc_err;
      } else if (is_safe == 7) {
        rc_err = emit_safe_crt_scanf(node, bld, is_msc, name, &changes);
        if (rc_err != CDD_C_SUCCESS)
          return rc_err;
      } else {
        for (k = 0; k < node->num_args; k++) {
          if (k > 0) {
            {
              rc_err = cdd_cst_bld_punct(bld, ",");
              if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
                return rc_err;
            }
            {
              rc_err = cdd_cst_bld_space(bld);
              if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
                return rc_err;
            }
          }
          {
            rc_err = emit_ast_bld(node->args[k], bld, is_msc, &_ch);
            if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
              return rc_err;
            changes += _ch;
          }
        }
      }

      {
        cdd_token_t *ct_close = NULL;
        if (node->close_tok && clone_token(bld->tree, node->close_tok,
                                           &ct_close) == CDD_C_SUCCESS) {
          {
            rc_err = cdd_cst_append_child_token(bld->target_node, ct_close);
            if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
              return rc_err;
          }
        } else {
          {
            rc_err = cdd_cst_bld_punct(bld, ")");
            if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
              return rc_err;
          }
        }
      }
    } else if (node->type == 3) {
      rc_err = emit_safe_crt_fopen(node, bld, is_msc, &changes);
      if (rc_err != CDD_C_SUCCESS)
        return rc_err;
    }
    node = node->next;
  }
  if (out_changes)
    *out_changes = changes;
  return CDD_C_SUCCESS;
}
