/**
 * @file safe_crt_rules.c
 * @brief AST code emission and rewriting rules for Safe CRT transformer.
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
  rc = (cdd_c_error_t)emit_ast_bld(node, bld, is_msc);
  if (bld->target_node && bld->target_node->num_children > old_num_children) {
    cdd_trivia_t *tr =
        bld->target_node->children[old_num_children].val.token->leading_trivia;
    while (tr) {
      cdd_trivia_t *next = tr->next;
      free(tr);
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
 * @param[in,out] bld CST builder.
 * @param[in] dest Destination buffer expression.
 */
C_CDD_EXPORT void emit_inferred_size(cdd_cst_builder_t *bld, expr_t *dest) {
  inferred_size_t info;
  if (!bld)
    return;
  if (bld->tree) {
    current_tree = bld->tree;
  } else {
    current_tree = NULL;
  }
  info = infer_buffer_size(dest);
  if (!info.valid) {
    cdd_cst_bld_ident(bld, "sizeof");
    cdd_cst_bld_punct(bld, "(");
    emit_ast_bld_strip(dest, bld, 0);
    cdd_cst_bld_punct(bld, ")");
    return;
  }

  if (info.offset_expr)
    cdd_cst_bld_punct(bld, "(");

  if (info.is_malloc == 1) {
    emit_ast_bld_strip(info.malloc_size_expr, bld, 0);
  } else if (info.is_malloc == 2) {
    cdd_cst_bld_punct(bld, "(");
    emit_ast_bld_strip(info.malloc_size_expr->args[0], bld, 0);
    cdd_cst_bld_punct(bld, ")");
    cdd_cst_bld_space(bld);
    cdd_cst_bld_punct(bld, "*");
    cdd_cst_bld_space(bld);
    cdd_cst_bld_punct(bld, "(");
    emit_ast_bld_strip(info.malloc_size_expr->args[1], bld, 0);
    cdd_cst_bld_punct(bld, ")");
  } else {
    cdd_token_t *ct = NULL;
    cdd_cst_bld_ident(bld, "sizeof");
    cdd_cst_bld_punct(bld, "(");

    clone_token(bld->tree, info.base_tok, &ct);
    if (ct) {
      if (ct->leading_trivia) {
        cdd_trivia_t *curr = ct->leading_trivia;
        while (curr) {
          cdd_trivia_t *nxt = curr->next;
          free(curr);
          curr = nxt;
        }
      }
      if (ct->trailing_trivia) {
        cdd_trivia_t *curr = ct->trailing_trivia;
        while (curr) {
          cdd_trivia_t *nxt = curr->next;
          free(curr);
          curr = nxt;
        }
      }
      ct->leading_trivia = NULL;
      ct->trailing_trivia = NULL;
      cdd_cst_append_child_token(bld->target_node, ct);
    }

    cdd_cst_bld_punct(bld, ")");
  }

  if (info.offset_expr) {
    cdd_cst_bld_space(bld);
    cdd_cst_bld_punct(bld, "-");
    cdd_cst_bld_space(bld);
    cdd_cst_bld_punct(bld, "(");
    emit_ast_bld_strip(info.offset_expr, bld, 0);
    cdd_cst_bld_punct(bld, ")");
    cdd_cst_bld_punct(bld, ")");
  }
}

/**
 * @brief Emits transformed AST tokens into the CST builder.
 *
 * @param[in] node Expression node.
 * @param[in,out] bld CST builder.
 * @param[in] is_msc Non-zero if targeting MSC Safe CRT.
 * @return Number of safe CRT changes emitted.
 */
int emit_ast_bld(expr_t *node, cdd_cst_builder_t *bld, int is_msc) {
  int changes = 0;
  size_t k;
  emit_ctx_t *ctx = is_msc ? g_msc_ctx : NULL;
  cdd_cst_tree_t *tree = bld->tree;

  while (node) {
    if (node->type == 0 || (node->type == 4 && !is_msc)) {
      cdd_token_t *ct = NULL;
      clone_token(bld->tree, node->tok, &ct);
      if (ct)
        cdd_cst_append_child_token(bld->target_node, ct);
    } else if (node->type == 2) {
      cdd_token_t *ct = NULL;
      clone_token(bld->tree, node->tok, &ct);
      if (ct)
        cdd_cst_append_child_token(bld->target_node, ct);
      changes += emit_ast_bld(node->args[0], bld, is_msc);
      if (node->close_tok) {
        cdd_token_t *ct_close = NULL;
        clone_token(bld->tree, node->close_tok, &ct_close);
        if (ct_close)
          cdd_cst_append_child_token(bld->target_node, ct_close);
      }
    } else if (node->type == 1 || (node->type == 5 && !is_msc)) {
      char name[128] = {0};
      size_t nlen =
          (node->tok && node->tok->length < 127) ? node->tok->length : 127;
      int is_safe = 0;

      if (node->tok)
        memcpy(name, node->tok->start, nlen);

      if (is_msc) {
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
                 strcmp(name, "_mbsncpy") == 0 ||
                 strcmp(name, "_mbsncat") == 0 || strcmp(name, "_mbsnset") == 0)
          is_safe = 2;
        else if (strcmp(name, "snprintf") == 0 ||
                 strcmp(name, "vsnprintf") == 0 ||
                 strcmp(name, "_snprintf") == 0 ||
                 strcmp(name, "_vsnprintf") == 0)
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
        else if (strcmp(name, "_splitpath") == 0 ||
                 strcmp(name, "_wsplitpath") == 0)
          is_safe = 10;
        else if (strcmp(name, "_makepath") == 0 ||
                 strcmp(name, "_wmakepath") == 0)
          is_safe = 11;
        else if (strcmp(name, "_gcvt") == 0)
          is_safe = 19;
        else if (strcmp(name, "mbstowcs") == 0 || strcmp(name, "wcstombs") == 0)
          is_safe = 20;
        else if (strcmp(name, "wctomb") == 0)
          is_safe = 21;
        else if (strcmp(name, "_searchenv") == 0 ||
                 strcmp(name, "_wsearchenv") == 0)
          is_safe = 22;
        else if (strcmp(name, "_wgetenv") == 0 || strcmp(name, "getenv") == 0)
          is_safe = 23;
        else if (strcmp(name, "_putenv") == 0 || strcmp(name, "_wputenv") == 0)
          is_safe = 24;
        else if (strcmp(name, "strerror") == 0 ||
                 strcmp(name, "_strerror") == 0)
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

        clone_token(bld->tree, node->tok, &ct);
        if (ct) {
          char *pooled = (char *)(size_t)malloc(strlen(safe_name) + 1);
#if defined(_MSC_VER)
          strcpy_s(pooled, strlen(safe_name) + 1, safe_name);
#else
          strcpy(pooled, safe_name);
#endif
          if (tree->num_strings >= tree->string_capacity) {
            tree->string_capacity =
                tree->string_capacity == 0 ? 32 : tree->string_capacity * 2;
            tree->string_pool = (char **)realloc(
                tree->string_pool, tree->string_capacity * sizeof(char *));
          }
          tree->string_pool[tree->num_strings++] = pooled;
          ct->start = (const uint8_t *)pooled;
          ct->length = strlen(pooled);
          cdd_cst_append_child_token(bld->target_node, ct);
        }
        cdd_cst_bld_punct(bld, "(");
      } else if (!is_safe) {
        cdd_token_t *ct = NULL;
        clone_token(bld->tree, node->tok, &ct);
        if (ct)
          cdd_cst_append_child_token(bld->target_node, ct);
        cdd_cst_bld_punct(bld, "(");
      } else {
        cdd_token_t *ct = NULL;
        clone_token(bld->tree, node->tok, &ct);
        if (ct) {
          ct->length = 0;
          ct->start = (const uint8_t *)"";
          cdd_cst_append_child_token(bld->target_node, ct);
        }
      }

      if (is_safe == 1 && node->num_args >= 2) {
        changes += emit_ast_bld(node->args[0], bld, is_msc);
        cdd_cst_bld_punct(bld, ",");
        cdd_cst_bld_space(bld);
        emit_inferred_size(bld, node->args[0]);
        for (k = 1; k < node->num_args; k++) {
          cdd_cst_bld_punct(bld, ",");
          cdd_cst_bld_space(bld);
          changes += emit_ast_bld(node->args[k], bld, is_msc);
        }
      } else if (is_safe == 2 && node->num_args >= 3) {
        changes += emit_ast_bld(node->args[0], bld, is_msc);
        cdd_cst_bld_punct(bld, ",");
        cdd_cst_bld_space(bld);
        emit_inferred_size(bld, node->args[0]);
        cdd_cst_bld_punct(bld, ",");
        cdd_cst_bld_space(bld);
        changes += emit_ast_bld(node->args[1], bld, is_msc);
        cdd_cst_bld_punct(bld, ",");
        cdd_cst_bld_space(bld);
        cdd_cst_bld_ident(bld, "_TRUNCATE");
      } else if (is_safe == 3 && node->num_args >= 3) {
        changes += emit_ast_bld(node->args[0], bld, is_msc);
        cdd_cst_bld_punct(bld, ",");
        cdd_cst_bld_space(bld);
        changes += emit_ast_bld(node->args[1], bld, is_msc);
        cdd_cst_bld_punct(bld, ",");
        cdd_cst_bld_space(bld);
        cdd_cst_bld_ident(bld, "_TRUNCATE");
        for (k = 2; k < node->num_args; k++) {
          cdd_cst_bld_punct(bld, ",");
          cdd_cst_bld_space(bld);
          changes += emit_ast_bld(node->args[k], bld, is_msc);
        }
      } else if (is_safe == 6 && node->num_args >= 3) {
        changes += emit_ast_bld(node->args[0], bld, is_msc);
        cdd_cst_bld_punct(bld, ",");
        cdd_cst_bld_space(bld);
        emit_inferred_size(bld, node->args[0]);
        cdd_cst_bld_punct(bld, ",");
        cdd_cst_bld_space(bld);
        changes += emit_ast_bld(node->args[1], bld, is_msc);
        cdd_cst_bld_punct(bld, ",");
        cdd_cst_bld_space(bld);
        changes += emit_ast_bld(node->args[2], bld, is_msc);
      } else if (is_safe == 8 && node->num_args >= 3) {
        changes += emit_ast_bld(node->args[0], bld, is_msc);
        cdd_cst_bld_punct(bld, ",");
        cdd_cst_bld_space(bld);
        changes += emit_ast_bld(node->args[1], bld, is_msc);
        cdd_cst_bld_punct(bld, ",");
        cdd_cst_bld_space(bld);
        emit_inferred_size(bld, node->args[1]);
        cdd_cst_bld_punct(bld, ",");
        cdd_cst_bld_space(bld);
        changes += emit_ast_bld(node->args[2], bld, is_msc);
      } else if (is_safe == 9 && node->num_args == 1) {
        changes += emit_ast_bld(node->args[0], bld, is_msc);
        cdd_cst_bld_punct(bld, ",");
        cdd_cst_bld_space(bld);
        emit_inferred_size(bld, node->args[0]);
      } else if (is_safe == 10 && node->num_args >= 5) {
        changes += emit_ast_bld(node->args[0], bld, is_msc);
        for (k = 1; k < 5; k++) {
          cdd_cst_bld_punct(bld, ",");
          cdd_cst_bld_space(bld);
          changes += emit_ast_bld(node->args[k], bld, is_msc);
          cdd_cst_bld_punct(bld, ",");
          cdd_cst_bld_space(bld);
          if (expr_is_null_or_zero(node->args[k])) {
            cdd_cst_bld_int(bld, 0);
          } else {
            emit_inferred_size(bld, node->args[k]);
          }
        }
      } else if (is_safe == 11 && node->num_args >= 5) {
        changes += emit_ast_bld(node->args[0], bld, is_msc);
        cdd_cst_bld_punct(bld, ",");
        cdd_cst_bld_space(bld);
        emit_inferred_size(bld, node->args[0]);
        for (k = 1; k < node->num_args; k++) {
          cdd_cst_bld_punct(bld, ",");
          cdd_cst_bld_space(bld);
          changes += emit_ast_bld(node->args[k], bld, is_msc);
        }
      } else if (is_safe == 19 && node->num_args == 3) {
        cdd_cst_bld_punct(bld, "(");
        cdd_cst_bld_ident(bld, "_gcvt_s");
        cdd_cst_bld_punct(bld, "(");
        changes += emit_ast_bld(node->args[2], bld, is_msc);
        cdd_cst_bld_punct(bld, ",");
        cdd_cst_bld_space(bld);
        emit_inferred_size(bld, node->args[2]);
        cdd_cst_bld_punct(bld, ",");
        cdd_cst_bld_space(bld);
        changes += emit_ast_bld(node->args[0], bld, is_msc);
        cdd_cst_bld_punct(bld, ",");
        cdd_cst_bld_space(bld);
        changes += emit_ast_bld(node->args[1], bld, is_msc);
        cdd_cst_bld_punct(bld, ")");
        cdd_cst_bld_punct(bld, ",");
        cdd_cst_bld_space(bld);
        emit_ast_bld_strip(node->args[2], bld, 0);
        cdd_cst_bld_punct(bld, ")");
      } else if (is_safe == 20 && node->num_args == 3) {
        cdd_cst_bld_punct(bld, "(");
        cdd_cst_bld_ident(bld, safe_crt_pool_string_safe(bld->tree, name));
        cdd_cst_bld_ident(bld, "_s");
        cdd_cst_bld_punct(bld, "(");
        cdd_cst_bld_ident(bld, "NULL");
        cdd_cst_bld_punct(bld, ",");
        cdd_cst_bld_space(bld);
        changes += emit_ast_bld(node->args[0], bld, is_msc);
        cdd_cst_bld_punct(bld, ",");
        cdd_cst_bld_space(bld);
        cdd_cst_bld_punct(bld, "(");
        emit_inferred_size(bld, node->args[0]);
        if (strcmp(name, "wcstombs") == 0) {
          cdd_cst_bld_punct(bld, ")");
        } else {
          cdd_cst_bld_punct(bld, ")");
          cdd_cst_bld_space(bld);
          cdd_cst_bld_punct(bld, "/");
          cdd_cst_bld_space(bld);
          cdd_cst_bld_ident(bld, "sizeof");
          cdd_cst_bld_punct(bld, "(");
          cdd_cst_bld_ident(bld, "wchar_t");
          cdd_cst_bld_punct(bld, ")");
        }
        cdd_cst_bld_punct(bld, ",");
        cdd_cst_bld_space(bld);
        changes += emit_ast_bld(node->args[1], bld, is_msc);
        cdd_cst_bld_punct(bld, ",");
        cdd_cst_bld_space(bld);
        changes += emit_ast_bld(node->args[2], bld, is_msc);
        cdd_cst_bld_punct(bld, ")");
        cdd_cst_bld_punct(bld, ",");
        cdd_cst_bld_space(bld);
        emit_ast_bld_strip(node->args[0], bld, 0);
        cdd_cst_bld_punct(bld, ")");
      } else if (is_safe == 21 && node->num_args == 2) {
        cdd_cst_bld_punct(bld, "(");
        cdd_cst_bld_ident(bld, "wctomb_s");
        cdd_cst_bld_punct(bld, "(");
        cdd_cst_bld_ident(bld, "NULL");
        cdd_cst_bld_punct(bld, ",");
        cdd_cst_bld_space(bld);
        changes += emit_ast_bld(node->args[0], bld, is_msc);
        cdd_cst_bld_punct(bld, ",");
        cdd_cst_bld_space(bld);
        emit_inferred_size(bld, node->args[0]);
        cdd_cst_bld_punct(bld, ",");
        cdd_cst_bld_space(bld);
        changes += emit_ast_bld(node->args[1], bld, is_msc);
        cdd_cst_bld_punct(bld, ")");
        cdd_cst_bld_punct(bld, ",");
        cdd_cst_bld_space(bld);
        emit_ast_bld_strip(node->args[0], bld, 0);
        cdd_cst_bld_punct(bld, ")");
      } else if (is_safe == 22 && node->num_args == 3) {
        cdd_cst_bld_ident(bld, safe_crt_pool_string_safe(bld->tree, name));
        cdd_cst_bld_ident(bld, "_s");
        cdd_cst_bld_punct(bld, "(");
        changes += emit_ast_bld(node->args[0], bld, is_msc);
        cdd_cst_bld_punct(bld, ",");
        cdd_cst_bld_space(bld);
        changes += emit_ast_bld(node->args[1], bld, is_msc);
        cdd_cst_bld_punct(bld, ",");
        cdd_cst_bld_space(bld);
        changes += emit_ast_bld(node->args[2], bld, is_msc);
        cdd_cst_bld_punct(bld, ",");
        cdd_cst_bld_space(bld);
        emit_inferred_size(bld, node->args[2]);
        if (strcmp(name, "_wsearchenv") == 0) {
          cdd_cst_bld_space(bld);
          cdd_cst_bld_punct(bld, "/");
          cdd_cst_bld_space(bld);
          cdd_cst_bld_ident(bld, "sizeof");
          cdd_cst_bld_punct(bld, "(");
          cdd_cst_bld_ident(bld, "wchar_t");
          cdd_cst_bld_punct(bld, ")");
        }
      } else if (is_safe == 23 && node->num_args == 1) {
        if (strcmp(name, "_wgetenv") == 0) {
          ctx->needs_wgetenv_ptr = 1;
          cdd_cst_bld_punct(bld, "(");
          cdd_cst_bld_ident(bld, "_wdupenv_s");
          cdd_cst_bld_punct(bld, "(");
          cdd_cst_bld_punct(bld, "&");
          cdd_cst_bld_ident(bld, "__wgetenv_ptr");
          cdd_cst_bld_punct(bld, ",");
          cdd_cst_bld_space(bld);
          cdd_cst_bld_ident(bld, "NULL");
          cdd_cst_bld_punct(bld, ",");
          cdd_cst_bld_space(bld);
          changes += emit_ast_bld(node->args[0], bld, is_msc);
          cdd_cst_bld_punct(bld, ")");
          cdd_cst_bld_punct(bld, ",");
          cdd_cst_bld_space(bld);
          cdd_cst_bld_ident(bld, "__wgetenv_ptr");
          cdd_cst_bld_punct(bld, ")");
        } else {
          ctx->needs_getenv_ptr = 1;
          cdd_cst_bld_punct(bld, "(");
          cdd_cst_bld_ident(bld, "_dupenv_s");
          cdd_cst_bld_punct(bld, "(");
          cdd_cst_bld_punct(bld, "&");
          cdd_cst_bld_ident(bld, "__getenv_ptr");
          cdd_cst_bld_punct(bld, ",");
          cdd_cst_bld_space(bld);
          cdd_cst_bld_ident(bld, "NULL");
          cdd_cst_bld_punct(bld, ",");
          cdd_cst_bld_space(bld);
          changes += emit_ast_bld(node->args[0], bld, is_msc);
          cdd_cst_bld_punct(bld, ")");
          cdd_cst_bld_punct(bld, ",");
          cdd_cst_bld_space(bld);
          cdd_cst_bld_ident(bld, "__getenv_ptr");
          cdd_cst_bld_punct(bld, ")");
        }
      } else if (is_safe == 24 && node->num_args == 1) {
        /* _putenv. We need to split the "A=B" string if it is a literal */
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
              cdd_cst_bld_ident(bld, strcmp(name, "_wputenv") == 0
                                         ? "_wputenv_s"
                                         : "_putenv_s");
              cdd_cst_bld_punct(bld, "(");
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
                cdd_cst_bld_token(bld, CDD_TOKEN_STRING,
                                  safe_crt_pool_string_safe(bld->tree, left));
                cdd_cst_bld_punct(bld, ",");
                cdd_cst_bld_space(bld);
                cdd_cst_bld_token(bld, CDD_TOKEN_STRING,
                                  safe_crt_pool_string_safe(bld->tree, right));
                cdd_cst_bld_punct(bld, ")");
              }
            }
          }
        }
        if (!split) {
          cdd_cst_bld_ident(bld, safe_crt_pool_string_safe(bld->tree, name));
          cdd_cst_bld_punct(bld, "(");
          changes += emit_ast_bld(node->args[0], bld, is_msc);
        }
      } else if (is_safe == 25 &&
                 (node->num_args == 4 || node->num_args == 5)) {
        cdd_cst_bld_ident(bld, safe_crt_pool_string_safe(bld->tree, name));
        cdd_cst_bld_ident(bld, "_s");
        cdd_cst_bld_punct(bld, "(");
        for (k = 0; k < node->num_args; k++) {
          if (k > 0) {
            cdd_cst_bld_punct(bld, ",");
            cdd_cst_bld_space(bld);
          }
          changes += emit_ast_bld(node->args[k], bld, is_msc);
        }
        cdd_cst_bld_punct(bld, ",");
        cdd_cst_bld_space(bld);
        cdd_cst_bld_ident(bld, "NULL");
      } else if (is_safe == 29) {
        if (strcmp(name, "strtok") == 0)
          ctx->needs_strtokctx = 1;
        else if (strcmp(name, "wcstok") == 0)
          ctx->needs_wcstokctx = 1;
        else
          ctx->needs_mbstokctx = 1;
        cdd_cst_bld_ident(bld, safe_crt_pool_string_safe(bld->tree, name));
        cdd_cst_bld_ident(bld, "_s");
        cdd_cst_bld_punct(bld, "(");
        for (k = 0; k < node->num_args; k++) {
          if (k > 0) {
            cdd_cst_bld_punct(bld, ",");
            cdd_cst_bld_space(bld);
          }
          changes += emit_ast_bld(node->args[k], bld, is_msc);
        }
        cdd_cst_bld_punct(bld, ",");
        cdd_cst_bld_space(bld);
        cdd_cst_bld_punct(bld, "&");
        if (strcmp(name, "strtok") == 0)
          cdd_cst_bld_ident(bld, "__strtokctx");
        else if (strcmp(name, "wcstok") == 0)
          cdd_cst_bld_ident(bld, "__wcstokctx");
        else
          cdd_cst_bld_ident(bld, "__mbstokctx");
      } else if (is_safe == 26 || is_safe == 27 || is_safe == 28) {
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
        cdd_cst_bld_punct(bld, "(");
        cdd_cst_bld_punct(bld, "(");
        cdd_cst_bld_ident(bld, safe_crt_pool_string_safe(bld->tree, name));
        cdd_cst_bld_ident(bld, "_s");
        cdd_cst_bld_punct(bld, "(");
        cdd_cst_bld_ident(bld, safe_crt_pool_string_safe(bld->tree, bufname));
        cdd_cst_bld_punct(bld, ",");
        cdd_cst_bld_space(bld);
        cdd_cst_bld_ident(bld, is_safe == 28 ? "128" : "94");
        for (k = 0; k < node->num_args; k++) {
          cdd_cst_bld_punct(bld, ",");
          cdd_cst_bld_space(bld);
          changes += emit_ast_bld(node->args[k], bld, is_msc);
        }
        cdd_cst_bld_punct(bld, ")");
        cdd_cst_bld_punct(bld, ",");
        cdd_cst_bld_space(bld);
        cdd_cst_bld_ident(bld, safe_crt_pool_string_safe(bld->tree, bufname));
        cdd_cst_bld_punct(bld, ")");
      } else if (is_safe == 7) {
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
            const char *start =
                (const char *)node->args[format_idx]->tok->start;
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
              cdd_cst_bld_punct(bld, ",");
              cdd_cst_bld_space(bld);
            }
            changes += emit_ast_bld(node->args[k], bld, is_msc);

            if (k > format_idx) {
              int arg_idx = (int)(k - (format_idx + 1));
              if (arg_idx < num_specifiers && needs_size[arg_idx]) {
                cdd_cst_bld_punct(bld, ",");
                cdd_cst_bld_space(bld);
                cdd_cst_bld_punct(bld, "(");
                cdd_cst_bld_ident(bld, "unsigned");
                cdd_cst_bld_punct(bld, ")");
                cdd_cst_bld_ident(bld, "sizeof");
                cdd_cst_bld_punct(bld, "(");
                emit_ast_bld_strip_ampersand(node->args[k], bld, 0);
                cdd_cst_bld_punct(bld, ")");
              }
            }
          }
        } else {
          if (node->num_args > 0) {
            changes += emit_ast_bld(node->args[0], bld, is_msc);
          }
        }
      } else {
        for (k = 0; k < node->num_args; k++) {
          if (k > 0) {
            cdd_cst_bld_punct(bld, ",");
            cdd_cst_bld_space(bld);
          }
          changes += emit_ast_bld(node->args[k], bld, is_msc);
        }
      }

      {
        cdd_token_t *ct_close = NULL;
        if (node->close_tok && clone_token(bld->tree, node->close_tok,
                                           &ct_close) == CDD_C_SUCCESS) {
          cdd_cst_append_child_token(bld->target_node, ct_close);
        } else {
          cdd_cst_bld_punct(bld, ")");
        }
      }
    } else if (node->type == 3) {
      changes++;
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
            clone_token(bld->tree, t->tok, &ct);
            if (ct)
              cdd_cst_append_child_token(bld->target_node, ct);
            t = t->next;
          }
          cdd_cst_bld_punct(bld, ";");
          cdd_cst_bld_newline(bld);
          cdd_cst_bld_ident(bld,
                            safe_crt_pool_string_safe(bld->tree, safe_call));
          cdd_cst_bld_punct(bld, "(");
          cdd_cst_bld_punct(bld, "&");
          {
            cdd_token_t *ct = NULL;
            cdd_cst_create_token_len(bld->tree, last_t->tok->kind,
                                     (const char *)last_t->tok->start,
                                     last_t->tok->length, &ct);
            if (ct) {
              cdd_cst_append_child_token(bld->target_node, ct);
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
            cdd_cst_create_token_len(bld->tree, CDD_TOKEN_OTHER, "", 0,
                                     &ct_space);
            clone_trivia(node->tok->leading_trivia, &ct_space->leading_trivia);
            cdd_cst_append_child_token(bld->target_node, ct_space);
          }
          cdd_cst_bld_ident(bld,
                            safe_crt_pool_string_safe(bld->tree, safe_call));
          cdd_cst_bld_punct(bld, "(");
          cdd_cst_bld_punct(bld, "&");
          while (t != node) {
            cdd_token_t *ct = NULL;
            clone_token(bld->tree, t->tok, &ct);
            if (ct)
              cdd_cst_append_child_token(bld->target_node, ct);
            t = t->next;
          }
        }
        if (call->num_args >= 1) {
          cdd_cst_bld_punct(bld, ",");
          cdd_cst_bld_space(bld);
          emit_ast_bld(call->args[0], bld, is_msc);
        }
        if (call->num_args >= 2) {
          cdd_cst_bld_punct(bld, ",");
          cdd_cst_bld_space(bld);
          emit_ast_bld(call->args[1], bld, is_msc);
        }
        if (call->num_args >= 3) {
          cdd_cst_bld_punct(bld, ",");
          cdd_cst_bld_space(bld);
          emit_ast_bld(call->args[2], bld, is_msc);
        }
        cdd_cst_bld_punct(bld, ")");
        if (node->tok && node->tok->trailing_trivia) {
          cdd_token_t *ct_space = NULL;
          cdd_cst_create_token_len(bld->tree, CDD_TOKEN_OTHER, "", 0,
                                   &ct_space);
          clone_trivia(node->tok->trailing_trivia, &ct_space->trailing_trivia);
          cdd_cst_append_child_token(bld->target_node, ct_space);
        }
      } else {
        if (node->tok && node->tok->leading_trivia) {
          cdd_token_t *ct_space = NULL;
          cdd_cst_create_token_len(bld->tree, CDD_TOKEN_OTHER, "", 0,
                                   &ct_space);
          clone_trivia(node->tok->leading_trivia, &ct_space->leading_trivia);
          cdd_cst_append_child_token(bld->target_node, ct_space);
        }
        cdd_cst_bld_punct(bld, "=");
        if (node->tok && node->tok->trailing_trivia) {
          cdd_token_t *ct_space = NULL;
          cdd_cst_create_token_len(bld->tree, CDD_TOKEN_OTHER, "", 0,
                                   &ct_space);
          clone_trivia(node->tok->trailing_trivia, &ct_space->trailing_trivia);
          cdd_cst_append_child_token(bld->target_node, ct_space);
        }
      }
    }
    node = node->next;
  }
  return changes;
}
