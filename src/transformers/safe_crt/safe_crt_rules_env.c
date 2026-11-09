/**
 * @file safe_crt_rules_env.c
 * @brief Path and environment variable rewriting rules for Safe CRT.
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

/** @brief Alias for safe_crt_check_mock_fail */
#define check_mock_fail(x) safe_crt_check_mock_fail(x)

/**
 * @brief Emits Safe CRT _splitpath/_makepath rules (categories 10, 11).
 *
 * @param[in] node Call expression AST node.
 * @param[in,out] bld CST builder.
 * @param[in] is_msc Non-zero if targeting MSC Safe CRT.
 * @param[in] is_safe Safe CRT category code (10 or 11).
 * @param[in,out] changes Pointer to accumulated change count.
 * @return CDD_C_SUCCESS on success or error code.
 */
C_CDD_EXPORT cdd_c_error_t emit_safe_crt_path(expr_t *node,
                                              cdd_cst_builder_t *bld,
                                              int is_msc, int is_safe,
                                              int *changes) {
  cdd_c_error_t rc_err = CDD_C_SUCCESS;
  int _ch = 0;
  size_t k;

  if (is_safe == 10) {
    {
      rc_err = emit_ast_bld(node->args[0], bld, is_msc, &_ch);
      if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
        return rc_err;
      *changes += _ch;
    }
    for (k = 1; k < 5; k++) {
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
      {
        rc_err = emit_ast_bld(node->args[k], bld, is_msc, &_ch);
        if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
          return rc_err;
        *changes += _ch;
      }
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
      {
        int is_zero = 0;
        g_cdd_last_alloc_err = expr_is_null_or_zero(node->args[k], &is_zero);
        if (is_zero) {
          rc_err = cdd_cst_bld_int(bld, 0);
          if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
            return rc_err;
        } else {
          rc_err = emit_inferred_size(bld, node->args[k]);
          if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
            return rc_err;
        }
      }
    }
  } else {
    {
      rc_err = emit_ast_bld(node->args[0], bld, is_msc, &_ch);
      if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
        return rc_err;
      *changes += _ch;
    }
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
    {
      rc_err = emit_inferred_size(bld, node->args[0]);
      if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
        return rc_err;
    }
    for (k = 1; k < node->num_args; k++) {
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
      {
        rc_err = emit_ast_bld(node->args[k], bld, is_msc, &_ch);
        if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
          return rc_err;
        *changes += _ch;
      }
    }
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Emits Safe CRT _searchenv, getenv, and _putenv rules (categories 22,
 * 23, 24).
 *
 * @param[in] node Call expression AST node.
 * @param[in,out] bld CST builder.
 * @param[in] is_msc Non-zero if targeting MSC Safe CRT.
 * @param[in] is_safe Safe CRT category code (22, 23, or 24).
 * @param[in] name Function identifier name.
 * @param[in,out] ctx MSC emission context.
 * @param[in,out] changes Pointer to accumulated change count.
 * @return CDD_C_SUCCESS on success or error code.
 */
C_CDD_EXPORT cdd_c_error_t emit_safe_crt_env(expr_t *node,
                                             cdd_cst_builder_t *bld, int is_msc,
                                             int is_safe, const char *name,
                                             emit_ctx_t *ctx, int *changes) {
  cdd_c_error_t rc_err = CDD_C_SUCCESS;
  int _ch = 0;

  if (is_safe == 22) {
    {
      const char *pooled_nm = NULL;
      rc_err = safe_crt_pool_string_safe(bld->tree, name, &pooled_nm);
      rc_err = cdd_cst_bld_ident(bld, pooled_nm);
      if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
        return rc_err;
    }
    {
      rc_err = cdd_cst_bld_ident(bld, "_s");
      if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
        return rc_err;
    }
    {
      rc_err = cdd_cst_bld_punct(bld, "(");
      if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
        return rc_err;
    }
    {
      rc_err = emit_ast_bld(node->args[0], bld, is_msc, &_ch);
      if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
        return rc_err;
      *changes += _ch;
    }
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
    {
      rc_err = emit_ast_bld(node->args[1], bld, is_msc, &_ch);
      if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
        return rc_err;
      *changes += _ch;
    }
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
    {
      rc_err = emit_ast_bld(node->args[2], bld, is_msc, &_ch);
      if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
        return rc_err;
      *changes += _ch;
    }
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
    {
      rc_err = emit_inferred_size(bld, node->args[2]);
      if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
        return rc_err;
    }
    if (strcmp(name, "_wsearchenv") == 0) {
      {
        rc_err = cdd_cst_bld_space(bld);
        if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
          return rc_err;
      }
      {
        rc_err = cdd_cst_bld_punct(bld, "/");
        if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
          return rc_err;
      }
      {
        rc_err = cdd_cst_bld_space(bld);
        if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
          return rc_err;
      }
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
        rc_err = cdd_cst_bld_ident(bld, "wchar_t");
        if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
          return rc_err;
      }
      {
        rc_err = cdd_cst_bld_punct(bld, ")");
        if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
          return rc_err;
      }
    }
  } else if (is_safe == 23) {
    if (strcmp(name, "_wgetenv") == 0) {
      ctx->needs_wgetenv_ptr = 1;
      {
        rc_err = cdd_cst_bld_punct(bld, "(");
        if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
          return rc_err;
      }
      {
        rc_err = cdd_cst_bld_ident(bld, "_wdupenv_s");
        if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
          return rc_err;
      }
      {
        rc_err = cdd_cst_bld_punct(bld, "(");
        if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
          return rc_err;
      }
      {
        rc_err = cdd_cst_bld_punct(bld, "&");
        if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
          return rc_err;
      }
      {
        rc_err = cdd_cst_bld_ident(bld, "__wgetenv_ptr");
        if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
          return rc_err;
      }
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
      {
        rc_err = cdd_cst_bld_ident(bld, "NULL");
        if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
          return rc_err;
      }
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
      {
        rc_err = emit_ast_bld(node->args[0], bld, is_msc, &_ch);
        if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
          return rc_err;
        *changes += _ch;
      }
      {
        rc_err = cdd_cst_bld_punct(bld, ")");
        if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
          return rc_err;
      }
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
      {
        rc_err = cdd_cst_bld_ident(bld, "__wgetenv_ptr");
        if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
          return rc_err;
      }
      {
        rc_err = cdd_cst_bld_punct(bld, ")");
        if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
          return rc_err;
      }
    } else {
      ctx->needs_getenv_ptr = 1;
      {
        rc_err = cdd_cst_bld_punct(bld, "(");
        if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
          return rc_err;
      }
      {
        rc_err = cdd_cst_bld_ident(bld, "_dupenv_s");
        if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
          return rc_err;
      }
      {
        rc_err = cdd_cst_bld_punct(bld, "(");
        if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
          return rc_err;
      }
      {
        rc_err = cdd_cst_bld_punct(bld, "&");
        if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
          return rc_err;
      }
      {
        rc_err = cdd_cst_bld_ident(bld, "__getenv_ptr");
        if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
          return rc_err;
      }
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
      {
        rc_err = cdd_cst_bld_ident(bld, "NULL");
        if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
          return rc_err;
      }
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
      {
        rc_err = emit_ast_bld(node->args[0], bld, is_msc, &_ch);
        if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
          return rc_err;
        *changes += _ch;
      }
      {
        rc_err = cdd_cst_bld_punct(bld, ")");
        if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
          return rc_err;
      }
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
      {
        rc_err = cdd_cst_bld_ident(bld, "__getenv_ptr");
        if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
          return rc_err;
      }
      {
        rc_err = cdd_cst_bld_punct(bld, ")");
        if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
          return rc_err;
      }
    }
  } else {
    int split = 0;
    expr_t *str_node = node->args[0];
    if (str_node && str_node->type == 0 && str_node->tok) {
      if (str_node->tok->kind == CDD_TOKEN_IDENTIFIER &&
          str_node->tok->length == 1 && str_node->tok->start[0] == 'L') {
        str_node = str_node->next;
      }
      if (str_node && str_node->type == 0 && str_node->tok &&
          str_node->tok->kind == CDD_TOKEN_STRING) {
        const char *s = (const char *)str_node->tok->start;
        size_t slen = str_node->tok->length;
        char *eq = (char *)memchr(s, '=', slen);
        if (eq && (s[0] == '"' || (s[0] == 'L' && s[1] == '"'))) {
          split = 1;
          rc_err = cdd_cst_bld_ident(
              bld, strcmp(name, "_wputenv") == 0 ? "_wputenv_s" : "_putenv_s");
          {
            rc_err = cdd_cst_bld_punct(bld, "(");
            if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
              return rc_err;
          }
          {
            char left[256] = {0};
            char right[256] = {0};
            size_t ll = (size_t)(eq - s);
            size_t rl = slen - (size_t)(eq - s) - 1;
            if (strcmp(name, "_wputenv") == 0) {
              left[0] = 'L';
              left[1] = '"';
              memcpy(left + 2, s + 1, ll - 1);
              left[ll + 1] = '"';
              right[0] = 'L';
              right[1] = '"';
              memcpy(right + 2, eq + 1, rl);
            } else {
              memcpy(left, s, ll);
              left[ll] = '"';
              right[0] = '"';
              memcpy(right + 1, eq + 1, rl);
            }
            {
              const char *pooled_l = NULL;
              rc_err = safe_crt_pool_string_safe(bld->tree, left, &pooled_l);
              rc_err = cdd_cst_bld_token(bld, CDD_TOKEN_STRING, pooled_l);
            }
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
            {
              const char *pooled_r = NULL;
              rc_err = safe_crt_pool_string_safe(bld->tree, right, &pooled_r);
              rc_err = cdd_cst_bld_token(bld, CDD_TOKEN_STRING, pooled_r);
            }
            {
              rc_err = cdd_cst_bld_punct(bld, ")");
              if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
                return rc_err;
            }
          }
        }
      }
    }
    if (!split) {
      {
        const char *pooled_nm = NULL;
        rc_err = safe_crt_pool_string_safe(bld->tree, name, &pooled_nm);
        rc_err = cdd_cst_bld_ident(bld, pooled_nm);
        if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
          return rc_err;
      }
      {
        rc_err = cdd_cst_bld_punct(bld, "(");
        if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
          return rc_err;
      }
      {
        rc_err = emit_ast_bld(node->args[0], bld, is_msc, &_ch);
        if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
          return rc_err;
        *changes += _ch;
      }
    }
  }

  return CDD_C_SUCCESS;
}
