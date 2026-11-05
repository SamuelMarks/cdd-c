/**
 * @file safe_crt_rules_str.c
 * @brief String, memory, search, and tokenizer rewriting rules for Safe CRT.
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
 * @brief Emits Safe CRT string, memory, and buffer size rules (categories 1, 2,
 * 6, 9).
 *
 * @param[in] node Call expression AST node.
 * @param[in,out] bld CST builder.
 * @param[in] is_msc Non-zero if targeting MSC Safe CRT.
 * @param[in] is_safe Safe CRT category code (1, 2, 6, or 9).
 * @param[in,out] changes Pointer to accumulated change count.
 * @return CDD_C_SUCCESS on success or error code.
 */
C_CDD_EXPORT cdd_c_error_t emit_safe_crt_str_basic(expr_t *node,
                                                   cdd_cst_builder_t *bld,
                                                   int is_msc, int is_safe,
                                                   int *changes) {
  cdd_c_error_t rc_err = CDD_C_SUCCESS;
  int _ch = 0;
  size_t k;

  if (is_safe == 1) {
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
  } else if (is_safe == 2) {
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
      rc_err = cdd_cst_bld_ident(bld, "_TRUNCATE");
      if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
        return rc_err;
    }
  } else if (is_safe == 6) {
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
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Emits Safe CRT qsort/bsearch and strtok/wcstok/_mbstok rules
 * (categories 25, 29).
 *
 * @param[in] node Call expression AST node.
 * @param[in,out] bld CST builder.
 * @param[in] is_msc Non-zero if targeting MSC Safe CRT.
 * @param[in] is_safe Safe CRT category code (25 or 29).
 * @param[in] name Function identifier name.
 * @param[in,out] ctx MSC emission context.
 * @param[in,out] changes Pointer to accumulated change count.
 * @return CDD_C_SUCCESS on success or error code.
 */
C_CDD_EXPORT cdd_c_error_t emit_safe_crt_search_tok(
    expr_t *node, cdd_cst_builder_t *bld, int is_msc, int is_safe,
    const char *name, emit_ctx_t *ctx, int *changes) {
  cdd_c_error_t rc_err = CDD_C_SUCCESS;
  int _ch = 0;
  size_t k;

  if (is_safe == 25) {
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
        *changes += _ch;
      }
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
  } else {
    if (strcmp(name, "strtok") == 0)
      ctx->needs_strtokctx = 1;
    else if (strcmp(name, "wcstok") == 0)
      ctx->needs_wcstokctx = 1;
    else
      ctx->needs_mbstokctx = 1;
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
        *changes += _ch;
      }
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
      rc_err = cdd_cst_bld_punct(bld, "&");
      if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
        return rc_err;
    }
    if (strcmp(name, "strtok") == 0) {
      rc_err = cdd_cst_bld_ident(bld, "__strtokctx");
      if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
        return rc_err;
    } else if (strcmp(name, "wcstok") == 0) {
      rc_err = cdd_cst_bld_ident(bld, "__wcstokctx");
      if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
        return rc_err;
    } else {
      rc_err = cdd_cst_bld_ident(bld, "__mbstokctx");
      if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
        return rc_err;
    }
  }

  return CDD_C_SUCCESS;
}
