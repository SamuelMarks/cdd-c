/**
 * @file safe_crt.c
 * @brief Implementation of the Safe CRT transformer.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "transformers/safe_crt/safe_crt_internal.h"
#include "cdd_cst_transform.h"
#include "classes/parse/cdd_cst_builder.h"
#include "classes/parse/cdd_cst_factory.h"
#include "classes/parse/cdd_cst_mutate.h"

#include "c_cdd/log.h"
#include "c_cdd_export.h"
#include "c_str_span.h"
#include "classes/parse/cdd_cst_parser.h"
#include "classes/parse/cdd_cst_query.h"
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "c_cdd/safe_crt.h"
/* clang-format on */

static safe_crt_arena_t *global_arena = NULL;
cdd_cst_tree_t *safe_crt_current_tree = NULL;
emit_ctx_t *g_msc_ctx = NULL;

#ifdef CDD_BUILD_TESTS
C_CDD_EXPORT int g_safe_crt_malloc_fail = 0;
#endif

/**
 * @brief Frees all memory allocated in the safe CRT arena.
 */
void arena_free_all(void) {
  safe_crt_arena_t *node = global_arena;
  while (node) {
    safe_crt_arena_t *next = node->next;
    free(node);
    node = next;
  }
  global_arena = NULL;
}

/**
 * @brief Allocates memory from the safe CRT arena.
 *
 * @param[in] len Length in bytes to allocate.
 * @param[out] out_ptr Pointer to receive the allocated memory.
 * @return CDD_C_SUCCESS on success or error code.
 */
C_CDD_EXPORT cdd_c_error_t arena_alloc(size_t len, void **out_ptr) {
  safe_crt_arena_t *node;
  if (!out_ptr)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_ptr = NULL;
#ifdef CDD_BUILD_TESTS
  if (g_safe_crt_malloc_fail > 0 && --g_safe_crt_malloc_fail == 0) {
    node = NULL;
  } else {
#endif
    node = (safe_crt_arena_t *)malloc(sizeof(safe_crt_arena_t) + len);
#ifdef CDD_BUILD_TESTS
  }
#endif
  if (!node) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }
  node->next = global_arena;
  global_arena = node;
  *out_ptr = node->data;
  return CDD_C_SUCCESS;
}

/**
 * @brief Safely pools a string in the CST tree's string pool.
 *
 * @param[in,out] tree Target CST tree.
 * @param[in] str String to duplicate and pool.
 * @return Pointer to pooled string, or NULL on failure.
 */
C_CDD_EXPORT const char *safe_crt_pool_string_safe(cdd_cst_tree_t *tree,
                                                   const char *str) {
  char *dup;
  if (!tree || !str)
    return NULL;
#ifdef CDD_BUILD_TESTS
  if (g_safe_crt_malloc_fail > 0 && --g_safe_crt_malloc_fail == 0) {
    return NULL;
  }
#endif
  dup = strdup(str);
  if (tree->num_strings >= tree->string_capacity) {
    size_t new_cap =
        tree->string_capacity == 0 ? 32 : tree->string_capacity * 2;
    char **new_pool = NULL;
#ifdef CDD_BUILD_TESTS
    if (g_safe_crt_malloc_fail > 0) {
      g_safe_crt_malloc_fail--;
      if (g_safe_crt_malloc_fail == 0)
        new_cap = 0;
    }
#endif
    if (new_cap > 0)
      new_pool = (char **)realloc(tree->string_pool, new_cap * sizeof(char *));
    if (!new_pool) {
      free(dup);
      return NULL;
    }
    tree->string_pool = new_pool;
    tree->string_capacity = new_cap;
  }
  tree->string_pool[tree->num_strings++] = dup;
  return dup;
}

/**
 * @brief Clones a linked list of trivia nodes.
 *
 * @param[in] head Head of the trivia list.
 * @param[out] out_trivia Pointer to receive cloned trivia list head.
 * @return CDD_C_SUCCESS on success or error code.
 */
C_CDD_EXPORT cdd_c_error_t clone_trivia(cdd_trivia_t *head,
                                        cdd_trivia_t **out_trivia) {
  cdd_trivia_t *new_head = NULL;
  cdd_trivia_t *tail = NULL;
  if (!out_trivia)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_trivia = NULL;

  while (head) {
    cdd_trivia_t *tr;
#ifdef CDD_BUILD_TESTS
    if (g_safe_crt_malloc_fail > 0 && --g_safe_crt_malloc_fail == 0) {
      tr = NULL;
    } else {
#endif
      tr = (cdd_trivia_t *)calloc(1, sizeof(cdd_trivia_t));
#ifdef CDD_BUILD_TESTS
    }
#endif
    if (!tr) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      while (new_head) {
        cdd_trivia_t *n = new_head->next;
        free(new_head);
        new_head = n;
      }
      return CDD_C_ERROR_MEMORY;
    }
    tr->kind = head->kind;
    tr->start = head->start;
    tr->length = head->length;
    if (!new_head)
      new_head = tr;
    else
      tail->next = tr;
    tail = tr;
    head = head->next;
  }
  *out_trivia = new_head;
  return CDD_C_SUCCESS;
}

/**
 * @brief Clones a CST token including its trivia.
 *
 * @param[in,out] tree Target CST tree.
 * @param[in] tok Token to clone.
 * @param[out] out_token Pointer to receive cloned token.
 * @return CDD_C_SUCCESS on success or error code.
 */
C_CDD_EXPORT cdd_c_error_t clone_token(cdd_cst_tree_t *tree, cdd_token_t *tok,
                                       cdd_token_t **out_token) {
  cdd_token_t *ct = NULL;
  if (!tok || !out_token)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_token = NULL;
  cdd_cst_create_token_len(tree, tok->kind, (const char *)tok->start,
                           tok->length, &ct);
  if (!ct)
    return CDD_C_ERROR_MEMORY;
  clone_trivia(tok->leading_trivia, &ct->leading_trivia);
  clone_trivia(tok->trailing_trivia, &ct->trailing_trivia);
  *out_token = ct;
  return CDD_C_SUCCESS;
}

/**
 * @brief Extracts the indentation string from a token's leading trivia.
 *
 * @param[in] tok Target token.
 * @param[out] out_indent Buffer of at least 64 bytes to receive indentation
 * string.
 */
C_CDD_EXPORT void get_indent_string(cdd_token_t *tok, char *out_indent) {
  cdd_trivia_t *tr;
  cdd_trivia_t *last_ws = NULL;

  out_indent[0] = '\0';
  if (!tok) {
#if defined(_MSC_VER)
    strcpy_s(out_indent, 64, "  ");
#else
    strcpy(out_indent, "  ");
#endif
    return;
  }

  tr = tok->leading_trivia;

  while (tr) {
    if (tr->kind == TRIVIA_WHITESPACE) {
      last_ws = tr;
    } else if (tr->kind == TRIVIA_NEWLINE) {
      last_ws = NULL;
    }
    tr = tr->next;
  }

  if (last_ws) {
    size_t len = last_ws->length < 63 ? last_ws->length : 63;
    memcpy(out_indent, last_ws->start, len);
    out_indent[len] = '\0';
  } else {
#if defined(_MSC_VER)
    strcpy_s(out_indent, 64, "  ");
#else
    strcpy(out_indent, "  ");
#endif
  }
}

/**
 * @brief Runs the Safe CRT transformation pass over a CST syntax tree.
 *
 * @param[in,out] tree CST syntax tree to transform.
 * @param[in] config Transformer configuration options.
 * @return CDD_C_SUCCESS on success or error code.
 */
cdd_c_error_t cdd_transform_safe_crt(cdd_cst_tree_t *tree,
                                     const cdd_transform_config_t *config) {
  cdd_cst_query_result_t res;
  cdd_c_error_t rc = CDD_C_SUCCESS;
  size_t i;
  int replaced_any;
  char indent[64];
  cdd_cst_builder_t msc_bld, else_bld, bld;
  cdd_cst_node_t *msc_node = NULL, *else_node = NULL, *new_node = NULL;
  int msc_changes = 0;
  int dummy_found = 0;
  cdd_trivia_t *saved_trivia = NULL;
  emit_ctx_t msc_ctx;

  (void)config;

  if (!tree || !tree->root)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (current_tree != tree) {
    arena_free_all();
    current_tree = tree;
  }

  do {
    replaced_any = 0;
    rc = cdd_cst_find_nodes_by_type(tree->root, CDD_CST_UNKNOWN, &res);
    if (rc != CDD_C_SUCCESS) {
      arena_free_all();
      current_tree = NULL;
      return (cdd_c_error_t)rc;
    }

    for (i = 0; i < res.size; i++) {
      cdd_cst_node_t *stmt = res.nodes[i];
      size_t idx = 0;
      expr_t *ast;
      cdd_token_t *first_tok = NULL;
      cdd_c_error_t chk_rc = CDD_C_SUCCESS;

      if (stmt->num_children > 0 &&
          stmt->children[0].kind == CDD_CST_CHILD_TOKEN) {
        first_tok = stmt->children[0].val.token;
      }

      {
        cdd_trivia_t *tr = first_tok ? first_tok->leading_trivia : NULL;
        int skip = 0;
        while (tr) {
          if (tr->length >= 16 &&
              memcmp(tr->start, "/*CDD_SAFE_CRT*/", 16) == 0) {
            skip = 1;
            break;
          }
          tr = tr->next;
        }
        if (skip)
          continue;
      }
      parse_expr_ast(stmt, &idx, 0, &ast);
      dummy_found = 0;
      rc = find_and_mark_fopen(ast, &dummy_found);
      if (rc != CDD_C_SUCCESS)
        goto loop_err;
      rc = check_unsupported_calls(ast);
      if (rc != CDD_C_SUCCESS)
        goto loop_err;

      chk_rc = check_needs_transform(ast);
      if (chk_rc) {
        msc_node = NULL;
        else_node = NULL;
        new_node = NULL;
        msc_changes = 0;

        saved_trivia = first_tok->leading_trivia;
        memset(&msc_ctx, 0, sizeof(msc_ctx));
        msc_ctx.is_msc = 1;

        get_indent_string(first_tok, indent);

        first_tok->leading_trivia = NULL;

        rc = cdd_cst_alloc_node(CDD_CST_PREPROC_CONDITIONAL, &new_node);
        if (rc != CDD_C_SUCCESS)
          goto loop_err;
        cdd_cst_builder_init(&bld, tree, new_node);
        cdd_cst_bld_newline(&bld);
        cdd_cst_bld_ifdef(&bld, "_MSC_VER");
        cdd_cst_bld_block_comment(&bld, "CDD_SAFE_CRT");
        cdd_cst_bld_space(&bld);

        rc = cdd_cst_alloc_node(CDD_CST_UNKNOWN, &msc_node);
        if (rc != CDD_C_SUCCESS)
          goto loop_err;
        cdd_cst_builder_init(&msc_bld, tree, msc_node);
        g_msc_ctx = &msc_ctx;
        msc_changes = 0;
        rc = emit_ast_bld(ast, &msc_bld, 1, &msc_changes);
        if (rc != CDD_C_SUCCESS)
          goto loop_err;
        g_msc_ctx = NULL;

        rc = cdd_cst_alloc_node(CDD_CST_UNKNOWN, &else_node);
        if (rc != CDD_C_SUCCESS)
          goto loop_err;
        cdd_cst_builder_init(&else_bld, tree, else_node);
        rc = emit_ast_bld(ast, &else_bld, 0, NULL);
        if (rc != CDD_C_SUCCESS)
          goto loop_err;

        first_tok->leading_trivia = saved_trivia;

        if (msc_changes > 0) {
          size_t _ci;
          if (msc_ctx.needs_errbuf || msc_ctx.needs_wcserrbuf ||
              msc_ctx.needs_strtokctx || msc_ctx.needs_wcstokctx ||
              msc_ctx.needs_mbstokctx || msc_ctx.needs_ecvtbuf ||
              msc_ctx.needs_fcvtbuf || msc_ctx.needs_getenv_ptr ||
              msc_ctx.needs_wgetenv_ptr) {
            cdd_cst_bld_punct(&bld, "{");
            cdd_cst_bld_newline(&bld);
            if (msc_ctx.needs_errbuf) {
              cdd_cst_bld_token(&bld, CDD_TOKEN_OTHER,
                                safe_crt_pool_string_safe(tree, indent));
              cdd_cst_bld_ident(&bld, "char");
              cdd_cst_bld_space(&bld);
              cdd_cst_bld_ident(&bld, "__errbuf");
              cdd_cst_bld_punct(&bld, "[");
              cdd_cst_bld_int(&bld, 94);
              cdd_cst_bld_punct(&bld, "]");
              cdd_cst_bld_punct(&bld, ";");
              cdd_cst_bld_newline(&bld);
            }
            if (msc_ctx.needs_wcserrbuf) {
              cdd_cst_bld_token(&bld, CDD_TOKEN_OTHER,
                                safe_crt_pool_string_safe(tree, indent));
              cdd_cst_bld_ident(&bld, "wchar_t");
              cdd_cst_bld_space(&bld);
              cdd_cst_bld_ident(&bld, "__wcserrbuf");
              cdd_cst_bld_punct(&bld, "[");
              cdd_cst_bld_int(&bld, 94);
              cdd_cst_bld_punct(&bld, "]");
              cdd_cst_bld_punct(&bld, ";");
              cdd_cst_bld_newline(&bld);
            }
            if (msc_ctx.needs_strtokctx) {
              cdd_cst_bld_token(&bld, CDD_TOKEN_OTHER,
                                safe_crt_pool_string_safe(tree, indent));
              cdd_cst_bld_ident(&bld, "char");
              cdd_cst_bld_space(&bld);
              cdd_cst_bld_punct(&bld, "*");
              cdd_cst_bld_ident(&bld, "__strtokctx");
              cdd_cst_bld_space(&bld);
              cdd_cst_bld_punct(&bld, "=");
              cdd_cst_bld_space(&bld);
              cdd_cst_bld_ident(&bld, "NULL");
              cdd_cst_bld_punct(&bld, ";");
              cdd_cst_bld_newline(&bld);
            }
            if (msc_ctx.needs_wcstokctx) {
              cdd_cst_bld_token(&bld, CDD_TOKEN_OTHER,
                                safe_crt_pool_string_safe(tree, indent));
              cdd_cst_bld_ident(&bld, "wchar_t");
              cdd_cst_bld_space(&bld);
              cdd_cst_bld_punct(&bld, "*");
              cdd_cst_bld_ident(&bld, "__wcstokctx");
              cdd_cst_bld_space(&bld);
              cdd_cst_bld_punct(&bld, "=");
              cdd_cst_bld_space(&bld);
              cdd_cst_bld_ident(&bld, "NULL");
              cdd_cst_bld_punct(&bld, ";");
              cdd_cst_bld_newline(&bld);
            }
            if (msc_ctx.needs_mbstokctx) {
              cdd_cst_bld_token(&bld, CDD_TOKEN_OTHER,
                                safe_crt_pool_string_safe(tree, indent));
              cdd_cst_bld_ident(&bld, "unsigned");
              cdd_cst_bld_space(&bld);
              cdd_cst_bld_ident(&bld, "char");
              cdd_cst_bld_space(&bld);
              cdd_cst_bld_punct(&bld, "*");
              cdd_cst_bld_ident(&bld, "__mbstokctx");
              cdd_cst_bld_space(&bld);
              cdd_cst_bld_punct(&bld, "=");
              cdd_cst_bld_space(&bld);
              cdd_cst_bld_ident(&bld, "NULL");
              cdd_cst_bld_punct(&bld, ";");
              cdd_cst_bld_newline(&bld);
            }
            if (msc_ctx.needs_ecvtbuf) {
              cdd_cst_bld_token(&bld, CDD_TOKEN_OTHER,
                                safe_crt_pool_string_safe(tree, indent));
              cdd_cst_bld_ident(&bld, "char");
              cdd_cst_bld_space(&bld);
              cdd_cst_bld_ident(&bld, "__ecvtbuf");
              cdd_cst_bld_punct(&bld, "[");
              cdd_cst_bld_int(&bld, 128);
              cdd_cst_bld_punct(&bld, "]");
              cdd_cst_bld_punct(&bld, ";");
              cdd_cst_bld_newline(&bld);
            }
            if (msc_ctx.needs_fcvtbuf) {
              cdd_cst_bld_token(&bld, CDD_TOKEN_OTHER,
                                safe_crt_pool_string_safe(tree, indent));
              cdd_cst_bld_ident(&bld, "char");
              cdd_cst_bld_space(&bld);
              cdd_cst_bld_ident(&bld, "__fcvtbuf");
              cdd_cst_bld_punct(&bld, "[");
              cdd_cst_bld_int(&bld, 128);
              cdd_cst_bld_punct(&bld, "]");
              cdd_cst_bld_punct(&bld, ";");
              cdd_cst_bld_newline(&bld);
            }

            if (msc_ctx.needs_getenv_ptr) {
              cdd_cst_bld_token(&bld, CDD_TOKEN_OTHER,
                                safe_crt_pool_string_safe(tree, indent));
              cdd_cst_bld_ident(&bld, "char");
              cdd_cst_bld_space(&bld);
              cdd_cst_bld_punct(&bld, "*");
              cdd_cst_bld_ident(&bld, "__getenv_ptr");
              cdd_cst_bld_punct(&bld, ";");
              cdd_cst_bld_newline(&bld);
            }
            if (msc_ctx.needs_wgetenv_ptr) {
              cdd_cst_bld_token(&bld, CDD_TOKEN_OTHER,
                                safe_crt_pool_string_safe(tree, indent));
              cdd_cst_bld_ident(&bld, "wchar_t");
              cdd_cst_bld_space(&bld);
              cdd_cst_bld_punct(&bld, "*");
              cdd_cst_bld_ident(&bld, "__wgetenv_ptr");
              cdd_cst_bld_punct(&bld, ";");
              cdd_cst_bld_newline(&bld);
            }
            cdd_cst_bld_token(&bld, CDD_TOKEN_OTHER,
                              safe_crt_pool_string_safe(tree, indent));
            cdd_cst_bld_space(&bld);
          }

          for (_ci = 0; _ci < msc_node->num_children; _ci++) {
            cdd_cst_append_child_token(bld.target_node,
                                       msc_node->children[_ci].val.token);
          }

          if (msc_ctx.needs_errbuf || msc_ctx.needs_wcserrbuf ||
              msc_ctx.needs_strtokctx || msc_ctx.needs_wcstokctx ||
              msc_ctx.needs_mbstokctx || msc_ctx.needs_ecvtbuf ||
              msc_ctx.needs_fcvtbuf || msc_ctx.needs_getenv_ptr ||
              msc_ctx.needs_wgetenv_ptr) {
            cdd_cst_bld_newline(&bld);
            cdd_cst_bld_token(&bld, CDD_TOKEN_OTHER,
                              safe_crt_pool_string_safe(tree, indent));
            cdd_cst_bld_punct(&bld, "}");
          }
          cdd_cst_bld_newline(&bld);
          cdd_cst_bld_else(&bld);
          cdd_cst_bld_block_comment(&bld, "CDD_SAFE_CRT");
          cdd_cst_bld_space(&bld);

          for (_ci = 0; _ci < else_node->num_children; _ci++) {
            cdd_cst_append_child_token(bld.target_node,
                                       else_node->children[_ci].val.token);
          }
          cdd_cst_bld_newline(&bld);
          cdd_cst_bld_endif(&bld);

#ifdef CDD_BUILD_TESTS
          if (g_safe_crt_malloc_fail == 6)
            bld.error_state = 1;
#endif
          if (bld.error_state == 0) {
            cdd_cst_replace_node(tree, stmt, new_node);
            cdd_cst_free_node(stmt);
            replaced_any = 1;
            cdd_cst_free_node_only(msc_node);
            cdd_cst_free_node_only(else_node);
          } else {
            cdd_cst_free_node(new_node);
            cdd_cst_free_node(msc_node);
            cdd_cst_free_node(else_node);
          }
        } else {
          cdd_cst_free_node(new_node);
          cdd_cst_free_node(msc_node);
          cdd_cst_free_node(else_node);
        }

        cdd_cst_builder_free(&msc_bld);
        cdd_cst_builder_free(&else_bld);
        cdd_cst_builder_free(&bld);

        if (replaced_any) {
          break;
        }
      }
    }

    free(res.nodes);
  } while (replaced_any);

  arena_free_all();
  current_tree = NULL;
  return CDD_C_SUCCESS;

loop_err:
  free(res.nodes);
  arena_free_all();
  current_tree = NULL;
  return (cdd_c_error_t)rc;
}
