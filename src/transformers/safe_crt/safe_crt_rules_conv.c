/**
 * @file safe_crt_rules_conv.c
 * @brief Numeric, multibyte, and error buffer conversion rewriting rules for
 * Safe CRT.
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
 * @brief Emits Safe CRT _itoa/_ltoa/_ultoa/_itow rules (category 8).
 *
 * @param[in] node Call expression AST node.
 * @param[in,out] bld CST builder.
 * @param[in] is_msc Non-zero if targeting MSC Safe CRT.
 * @param[in,out] changes Pointer to accumulated change count.
 * @return CDD_C_SUCCESS on success or error code.
 */
C_CDD_EXPORT cdd_c_error_t emit_safe_crt_itoa(expr_t *node,
                                              cdd_cst_builder_t *bld,
                                              int is_msc, int *changes) {
  cdd_c_error_t rc_err = CDD_C_SUCCESS;
  int _ch = 0;

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
    rc_err = emit_inferred_size(bld, node->args[1]);
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
    rc_err = emit_ast_bld(node->args[2], bld, is_msc, &_ch);
    if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
      return rc_err;
    *changes += _ch;
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Emits Safe CRT _gcvt, mbstowcs/wcstombs, and wctomb rules (categories
 * 19, 20, 21).
 *
 * @param[in] node Call expression AST node.
 * @param[in,out] bld CST builder.
 * @param[in] is_msc Non-zero if targeting MSC Safe CRT.
 * @param[in] is_safe Safe CRT category code (19, 20, or 21).
 * @param[in] name Function identifier name.
 * @param[in,out] changes Pointer to accumulated change count.
 * @return CDD_C_SUCCESS on success or error code.
 */
C_CDD_EXPORT cdd_c_error_t emit_safe_crt_gcvt_mbs(expr_t *node,
                                                  cdd_cst_builder_t *bld,
                                                  int is_msc, int is_safe,
                                                  const char *name,
                                                  int *changes) {
  cdd_c_error_t rc_err = CDD_C_SUCCESS;
  int _ch = 0;

  if (is_safe == 19) {
    {
      rc_err = cdd_cst_bld_punct(bld, "(");
      if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
        return rc_err;
    }
    {
      rc_err = cdd_cst_bld_ident(bld, "_gcvt_s");
      if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
        return rc_err;
    }
    {
      rc_err = cdd_cst_bld_punct(bld, "(");
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
      rc_err = emit_ast_bld_strip(node->args[2], bld, 0);
      if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
        return rc_err;
    }
    {
      rc_err = cdd_cst_bld_punct(bld, ")");
      if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
        return rc_err;
    }
  } else if (is_safe == 20) {
    {
      rc_err = cdd_cst_bld_punct(bld, "(");
      if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
        return rc_err;
    }
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
      rc_err = cdd_cst_bld_punct(bld, "(");
      if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
        return rc_err;
    }
    {
      rc_err = emit_inferred_size(bld, node->args[0]);
      if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
        return rc_err;
    }
    if (strcmp(name, "wcstombs") == 0) {
      {
        rc_err = cdd_cst_bld_punct(bld, ")");
        if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
          return rc_err;
      }
    } else {
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
      rc_err = emit_ast_bld_strip(node->args[0], bld, 0);
      if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
        return rc_err;
    }
    {
      rc_err = cdd_cst_bld_punct(bld, ")");
      if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
        return rc_err;
    }
  } else {
    {
      rc_err = cdd_cst_bld_punct(bld, "(");
      if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
        return rc_err;
    }
    {
      rc_err = cdd_cst_bld_ident(bld, "wctomb_s");
      if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
        return rc_err;
    }
    {
      rc_err = cdd_cst_bld_punct(bld, "(");
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
      rc_err = emit_ast_bld_strip(node->args[0], bld, 0);
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
 * @brief Emits Safe CRT strerror/_wcserror/_ecvt/_fcvt rules (categories 26,
 * 27, 28).
 *
 * @param[in] node Call expression AST node.
 * @param[in,out] bld CST builder.
 * @param[in] is_msc Non-zero if targeting MSC Safe CRT.
 * @param[in] is_safe Safe CRT category code (26, 27, or 28).
 * @param[in] name Function identifier name.
 * @param[in,out] ctx MSC emission context.
 * @param[in,out] changes Pointer to accumulated change count.
 * @return CDD_C_SUCCESS on success or error code.
 */
C_CDD_EXPORT cdd_c_error_t emit_safe_crt_err_fcvt(
    expr_t *node, cdd_cst_builder_t *bld, int is_msc, int is_safe,
    const char *name, emit_ctx_t *ctx, int *changes) {
  cdd_c_error_t rc_err = CDD_C_SUCCESS;
  int _ch = 0;
  size_t k;
  const char *bufname;

  if (is_safe == 26) {
    bufname = "__wcserrbuf";
    ctx->needs_wcserrbuf = 1;
  } else if (is_safe == 27) {
    bufname = "__errbuf";
    ctx->needs_errbuf = 1;
  } else {
    if (strcmp(name, "_ecvt") == 0) {
      bufname = "__ecvtbuf";
      ctx->needs_ecvtbuf = 1;
    } else {
      bufname = "__fcvtbuf";
      ctx->needs_fcvtbuf = 1;
    }
  }
  {
    rc_err = cdd_cst_bld_punct(bld, "(");
    if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
      return rc_err;
  }
  {
    rc_err = cdd_cst_bld_punct(bld, "(");
    if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
      return rc_err;
  }
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
    const char *pooled_bn = NULL;
    rc_err = safe_crt_pool_string_safe(bld->tree, bufname, &pooled_bn);
    rc_err = cdd_cst_bld_ident(bld, pooled_bn);
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
    rc_err = cdd_cst_bld_ident(bld, is_safe == 28 ? "128" : "94");
    if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
      return rc_err;
  }
  for (k = 0; k < node->num_args; k++) {
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
    const char *pooled_bn = NULL;
    rc_err = safe_crt_pool_string_safe(bld->tree, bufname, &pooled_bn);
    rc_err = cdd_cst_bld_ident(bld, pooled_bn);
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
