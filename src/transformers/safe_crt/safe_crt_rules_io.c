/**
 * @file safe_crt_rules_io.c
 * @brief Formatted I/O and file opening rewriting rules for Safe CRT.
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
 * @brief Emits Safe CRT snprintf/vsnprintf rules (category 3).
 *
 * @param[in] node Call expression AST node.
 * @param[in,out] bld CST builder.
 * @param[in] is_msc Non-zero if targeting MSC Safe CRT.
 * @param[in,out] changes Pointer to accumulated change count.
 * @return CDD_C_SUCCESS on success or error code.
 */
C_CDD_EXPORT cdd_c_error_t emit_safe_crt_snprintf(expr_t *node,
                                                  cdd_cst_builder_t *bld,
                                                  int is_msc, int *changes) {
  cdd_c_error_t rc_err = CDD_C_SUCCESS;
  int _ch = 0;
  size_t k;

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
    rc_err = cdd_cst_bld_ident(bld, "_TRUNCATE");
    if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
      return rc_err;
  }
  for (k = 2; k < node->num_args; k++) {
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
  return CDD_C_SUCCESS;
}

/**
 * @brief Emits Safe CRT scanf/fscanf/sscanf rules (category 7).
 *
 * @param[in] node Call expression AST node.
 * @param[in,out] bld CST builder.
 * @param[in] is_msc Non-zero if targeting MSC Safe CRT.
 * @param[in] name Function identifier name.
 * @param[in,out] changes Pointer to accumulated change count.
 * @return CDD_C_SUCCESS on success or error code.
 */
C_CDD_EXPORT cdd_c_error_t emit_safe_crt_scanf(expr_t *node,
                                               cdd_cst_builder_t *bld,
                                               int is_msc, const char *name,
                                               int *changes) {
  cdd_c_error_t rc_err = CDD_C_SUCCESS;
  int _ch = 0;
  size_t k;
  size_t format_idx = 0;

  if (strcmp(name, "fscanf") == 0 || strcmp(name, "sscanf") == 0 ||
      strcmp(name, "vfscanf") == 0 || strcmp(name, "vsscanf") == 0) {
    format_idx = 1;
  }

  if (format_idx < node->num_args) {
    int needs_size[32] = {0};
    int num_specifiers = 0;

    if (node->args[format_idx] && node->args[format_idx]->type == 0 &&
        node->args[format_idx]->tok &&
        node->args[format_idx]->tok->kind == CDD_TOKEN_STRING) {
      const char *start = (const char *)node->args[format_idx]->tok->start;
      const char *end = start + node->args[format_idx]->tok->length;
      while (start < end) {
        if (*start == '%') {
          start++;
          if (*start == '%') {
            start++;
            continue;
          }
          if (*start == '*') {
            start++;
            while (start < end && !isalpha((unsigned char)*start) &&
                   *start != '[') {
              start++;
            }
            if (start < end)
              start++;
            continue;
          }
          while (start < end && !isalpha((unsigned char)*start) &&
                 *start != '[')
            start++;
          if (start < end) {
            if (*start == 's' || *start == 'S' || *start == 'c' ||
                *start == 'C' || *start == '[') {
              if (num_specifiers < 32)
                needs_size[num_specifiers] = 1;
            }
            num_specifiers++;
            if (*start == '[') {
              start++;
              if (*start == '^')
                start++;
              if (*start == ']')
                start++;
              while (start < end && *start != ']')
                start++;
            }
            start++;
          }
        } else {
          start++;
        }
      }
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

      if (k > format_idx) {
        int arg_idx = (int)(k - (format_idx + 1));
        if (arg_idx < num_specifiers && needs_size[arg_idx]) {
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
            rc_err = cdd_cst_bld_ident(bld, "unsigned");
            if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
              return rc_err;
          }
          {
            rc_err = cdd_cst_bld_punct(bld, ")");
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
            rc_err = emit_ast_bld_strip_ampersand(node->args[k], bld, 0);
            if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
              return rc_err;
          }
          {
            rc_err = cdd_cst_bld_punct(bld, ")");
            if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
              return rc_err;
          }
        }
      }
    }
  } else {
    if (node->num_args > 0) {
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

/**
 * @brief Emits Safe CRT fopen/freopen/_wfopen/tmpfile assignment rules (node
 * type 3).
 *
 * @param[in] node Assignment expression AST node of type 3.
 * @param[in,out] bld CST builder.
 * @param[in] is_msc Non-zero if targeting MSC Safe CRT.
 * @param[in,out] changes Pointer to accumulated change count.
 * @return CDD_C_SUCCESS on success or error code.
 */
C_CDD_EXPORT cdd_c_error_t emit_safe_crt_fopen(expr_t *node,
                                               cdd_cst_builder_t *bld,
                                               int is_msc, int *changes) {
  cdd_c_error_t rc_err = CDD_C_SUCCESS;
  int _ch = 0;

  (*changes)++;
  if (is_msc) {
    expr_t *lhs = node->args[0];
    expr_t *call = node->args[1];
    size_t nlen = 0;
    int is_decl = 0;
    int token_count = 0;
    int has_dot_arrow_bracket = 0;
    int first_is_star = 0;
    expr_t *last_t = NULL;
    expr_t *t_iter = lhs;

    while (t_iter != node) {
      if (t_iter->tok) {
        if (token_count == 0 && t_iter->tok->kind == CDD_TOKEN_STAR)
          first_is_star = 1;
        if (t_iter->tok->kind == CDD_TOKEN_DOT ||
            t_iter->tok->kind == CDD_TOKEN_ARROW ||
            t_iter->tok->kind == CDD_TOKEN_LBRACKET)
          has_dot_arrow_bracket = 1;
      }
      token_count++;
      last_t = t_iter;
      t_iter = t_iter->next;
    }

    if (token_count > 1 && !has_dot_arrow_bracket && !first_is_star)
      is_decl = 1;

    if (is_decl) {
      char safe_call[32];
      char call_name[16];
      expr_t *t = lhs;
      memset(call_name, 0, sizeof(call_name));
      nlen = call->tok->length;
      memcpy(call_name, call->tok->start, nlen);
      CDD_SNPRINTF(safe_call, sizeof(safe_call), "%s_s", call_name);
      while (t != node) {
        cdd_token_t *ct = NULL;
        rc_err = clone_token(bld->tree, t->tok, &ct);
        if (rc_err == CDD_C_SUCCESS) {
          rc_err = cdd_cst_append_child_token(bld->target_node, ct);
          if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
            return rc_err;
        }
        t = t->next;
      }
      rc_err = cdd_cst_bld_punct(bld, ";");
      {
        rc_err = cdd_cst_bld_newline(bld);
        if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
          return rc_err;
      }
      {
        const char *pooled_sc = NULL;
        rc_err = safe_crt_pool_string_safe(bld->tree, safe_call, &pooled_sc);
        rc_err = cdd_cst_bld_ident(bld, pooled_sc);
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
        cdd_token_t *ct = NULL;
        rc_err = cdd_cst_create_token_len(bld->tree, last_t->tok->kind,
                                          (const char *)last_t->tok->start,
                                          last_t->tok->length, &ct);
        if (rc_err == CDD_C_SUCCESS) {
          {
            rc_err = cdd_cst_append_child_token(bld->target_node, ct);
            if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
              return rc_err;
          }
        }
      }
    } else {
      char safe_call[32];
      char call_name[16];
      expr_t *t = lhs;
      memset(call_name, 0, sizeof(call_name));
      nlen = call->tok->length;
      memcpy(call_name, call->tok->start, nlen);
      CDD_SNPRINTF(safe_call, sizeof(safe_call), "%s_s", call_name);

      if (node->tok && node->tok->leading_trivia) {
        cdd_token_t *ct_space = NULL;
        rc_err = cdd_cst_create_token_len(bld->tree, CDD_TOKEN_OTHER, "", 0,
                                          &ct_space);
        rc_err =
            clone_trivia(node->tok->leading_trivia, &ct_space->leading_trivia);
        {
          rc_err = cdd_cst_append_child_token(bld->target_node, ct_space);
          if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
            return rc_err;
        }
      }
      {
        const char *pooled_sc = NULL;
        rc_err = safe_crt_pool_string_safe(bld->tree, safe_call, &pooled_sc);
        rc_err = cdd_cst_bld_ident(bld, pooled_sc);
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
      while (t != node) {
        cdd_token_t *ct = NULL;
        rc_err = clone_token(bld->tree, t->tok, &ct);
        if (rc_err == CDD_C_SUCCESS) {
          rc_err = cdd_cst_append_child_token(bld->target_node, ct);
          if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
            return rc_err;
        }
        t = t->next;
      }
    }
    if (call->num_args >= 1) {
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
        rc_err = emit_ast_bld(call->args[0], bld, is_msc, &_ch);
        if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
          return rc_err;
      }
    }
    if (call->num_args >= 2) {
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
        rc_err = emit_ast_bld(call->args[1], bld, is_msc, &_ch);
        if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
          return rc_err;
      }
    }
    if (call->num_args >= 3) {
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
        rc_err = emit_ast_bld(call->args[2], bld, is_msc, &_ch);
        if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
          return rc_err;
      }
    }
    {
      rc_err = cdd_cst_bld_punct(bld, ")");
      if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
        return rc_err;
    }
    if (node->tok && node->tok->trailing_trivia) {
      cdd_token_t *ct_space = NULL;
      rc_err = cdd_cst_create_token_len(bld->tree, CDD_TOKEN_OTHER, "", 0,
                                        &ct_space);
      rc_err =
          clone_trivia(node->tok->trailing_trivia, &ct_space->trailing_trivia);
      {
        rc_err = cdd_cst_append_child_token(bld->target_node, ct_space);
        if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
          return rc_err;
      }
    }
  } else {
    if (node->tok && node->tok->leading_trivia) {
      cdd_token_t *ct_space = NULL;
      rc_err = cdd_cst_create_token_len(bld->tree, CDD_TOKEN_OTHER, "", 0,
                                        &ct_space);
      rc_err =
          clone_trivia(node->tok->leading_trivia, &ct_space->leading_trivia);
      {
        rc_err = cdd_cst_append_child_token(bld->target_node, ct_space);
        if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
          return rc_err;
      }
    }
    {
      rc_err = cdd_cst_bld_punct(bld, "=");
      if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
        return rc_err;
    }
    if (node->tok && node->tok->trailing_trivia) {
      cdd_token_t *ct_space = NULL;
      rc_err = cdd_cst_create_token_len(bld->tree, CDD_TOKEN_OTHER, "", 0,
                                        &ct_space);
      rc_err =
          clone_trivia(node->tok->trailing_trivia, &ct_space->trailing_trivia);
      {
        rc_err = cdd_cst_append_child_token(bld->target_node, ct_space);
        if ((rc_err = check_mock_fail(rc_err)) != CDD_C_SUCCESS)
          return rc_err;
      }
    }
  }

  return CDD_C_SUCCESS;
}
