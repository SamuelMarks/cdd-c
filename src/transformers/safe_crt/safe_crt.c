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

/** @brief Global linked list head of allocated Safe CRT arena blocks */
static safe_crt_arena_t *global_arena = NULL;
cdd_cst_tree_t *safe_crt_current_tree = NULL;
emit_ctx_t *g_msc_ctx = NULL;

/**
 * @brief Frees all memory allocated in the safe CRT arena.
 * @return CDD_C_SUCCESS.
 */
cdd_c_error_t arena_free_all(void) {
  safe_crt_arena_t *node = global_arena;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  while (node) {
    safe_crt_arena_t *next = node->next;
    C_CDD_FREE(node);
    node = next;
  }
  global_arena = NULL;
  return CDD_C_SUCCESS;
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
  node = (safe_crt_arena_t *)C_CDD_MALLOC(sizeof(safe_crt_arena_t) + len);
  if (!node) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM");
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
 * @param[out] out_pooled Pointer to receive pooled string, or NULL on failure.
 * @return CDD_C_SUCCESS on success or error code.
 */
C_CDD_EXPORT cdd_c_error_t safe_crt_pool_string_safe(cdd_cst_tree_t *tree,
                                                     const char *str,
                                                     const char **out_pooled) {
  char *dup;
  *out_pooled = NULL;
  if (!tree || !str)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  dup = C_CDD_STRDUP(str);
  if (!dup)
    return CDD_C_ERROR_MEMORY;
  if (tree->num_strings >= tree->string_capacity) {
    size_t new_cap =
        tree->string_capacity == 0 ? 32 : tree->string_capacity * 2;
    char **new_pool =
        (char **)C_CDD_REALLOC(tree->string_pool, new_cap * sizeof(char *));
    if (!new_pool) {
      C_CDD_FREE(dup);
      return CDD_C_ERROR_MEMORY;
    }
    tree->string_pool = new_pool;
    tree->string_capacity = new_cap;
  }
  tree->string_pool[tree->num_strings++] = dup;
  *out_pooled = dup;
  return CDD_C_SUCCESS;
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
    cdd_trivia_t *tr = (cdd_trivia_t *)C_CDD_MALLOC(sizeof(cdd_trivia_t));
    if (!tr) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM");
      while (new_head) {
        cdd_trivia_t *n = new_head->next;
        C_CDD_FREE(new_head);
        new_head = n;
      }
      return CDD_C_ERROR_MEMORY;
    }
    tr->kind = head->kind;
    tr->start = head->start;
    tr->length = head->length;
    tr->next = NULL;
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
  cdd_c_error_t rc = CDD_C_SUCCESS;
  if (!tok || !out_token)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_token = NULL;
  rc = cdd_cst_create_token_len(tree, tok->kind, (const char *)tok->start,
                                tok->length, &ct);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = clone_trivia(tok->leading_trivia, &ct->leading_trivia);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = clone_trivia(tok->trailing_trivia, &ct->trailing_trivia);
  if (rc != CDD_C_SUCCESS) {
    while (ct->leading_trivia) {
      cdd_trivia_t *n = ct->leading_trivia->next;
      C_CDD_FREE(ct->leading_trivia);
      ct->leading_trivia = n;
    }
    return rc;
  }
  *out_token = ct;
  return CDD_C_SUCCESS;
}

/**
 * @brief Extracts the indentation string from a token's leading trivia.
 *
 * @param[in] tok Target token.
 * @param[out] out_indent Buffer of at least 64 bytes to receive indentation
 * string.
 * @return CDD_C_SUCCESS on success.
 */
C_CDD_EXPORT cdd_c_error_t get_indent_string(cdd_token_t *tok,
                                             char *out_indent) {
  cdd_trivia_t *tr;
  cdd_trivia_t *last_ws = NULL;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif

  out_indent[0] = '\0';
  if (!tok) {
#if defined(_MSC_VER)
    strcpy_s(out_indent, 64, "  ");
#else
    strcpy(out_indent, "  ");
#endif
    return CDD_C_SUCCESS;
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
  return CDD_C_SUCCESS;
}

/**
 * @brief Appends an indented auxiliary Safe CRT scratch declaration line onto
 * `bld`.
 *
 * @param[in,out] tree Target CST tree owning pooled indentation strings.
 * @param[in,out] bld Target CST builder.
 * @param[in] indent Indentation prefix string.
 * @param[in] decl_line Scratch variable declaration ending in `\n`.
 * @return CDD_C_SUCCESS on success, or error code.
 */
static cdd_c_error_t safe_crt_emit_ctx_decl(cdd_cst_tree_t *tree,
                                            cdd_cst_builder_t *bld,
                                            const char *indent,
                                            const char *decl_line) {
  const char *pooled_ind = NULL;
  cdd_c_error_t rc = safe_crt_pool_string_safe(tree, indent, &pooled_ind);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = cdd_cst_bld_token(bld, CDD_TOKEN_OTHER, pooled_ind);
  if (rc != CDD_C_SUCCESS)
    return rc;
  return cdd_cst_bld_token(bld, CDD_TOKEN_OTHER, decl_line);
}

/**
 * @brief Queries whether `ast` requires a Safe CRT rewrite while absorbing
 * advisory buffer-inference diagnostics into `*out_needs = 1`.
 *
 * @param[in] ast Parsed statement expression AST.
 * @param[out] out_needs Receives 1 when rewrite/diagnostic is needed, 0
 * otherwise.
 * @return CDD_C_SUCCESS on completion, or error code on fault injection.
 */
static cdd_c_error_t safe_crt_query_needs_transform(expr_t *ast,
                                                    int *out_needs) {
  cdd_c_error_t rc;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  *out_needs = 0;
  rc = check_needs_transform(ast, out_needs);
  if (rc == CDD_C_SUCCESS)
    return rc;
  return CDD_C_SUCCESS;
}

/**
 * @brief Releases scratch CST nodes and builders used by a single Safe CRT
 * statement rewrite pass.
 *
 * @param[in,out] replaced_stmt Previous replaced CST statement node.
 * @param[in,out] new_node Unattached conditional node (or NULL if attached).
 * @param[in,out] msc_node Scratch MSVC branch node.
 * @param[in,out] else_node Scratch fallback branch node.
 * @param[in,out] msc_bld MSVC CST builder.
 * @param[in,out] else_bld Fallback CST builder.
 * @param[in,out] bld Outer conditional CST builder.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t safe_crt_stmt_scratch_free(cdd_cst_node_t *replaced_stmt,
                                                cdd_cst_node_t *new_node,
                                                cdd_cst_node_t *msc_node,
                                                cdd_cst_node_t *else_node,
                                                cdd_cst_builder_t *msc_bld,
                                                cdd_cst_builder_t *else_bld,
                                                cdd_cst_builder_t *bld) {
  return cdd_c_keep_err(
      cdd_cst_node_free(replaced_stmt),
      cdd_c_keep_err(
          cdd_cst_node_free(new_node),
          cdd_c_keep_err(
              cdd_cst_node_free(msc_node),
              cdd_c_keep_err(
                  cdd_cst_node_free(else_node),
                  cdd_c_keep_err(cdd_cst_builder_free(else_bld),
                                 cdd_c_keep_err(cdd_cst_builder_free(msc_bld),
                                                cdd_cst_builder_free(bld)))))));
}

/**
 * @brief Migrates standard C functions to MSVC Safe CRT functions.
 *
 * @param[in,out] tree The CST tree.
 * @param[in] config Optional configuration for trivia generation.
 * @return 0 on success, or an error code.
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
  cdd_token_t *active_first_tok = NULL;
  cdd_trivia_t *saved_trivia = NULL;
  emit_ctx_t msc_ctx;

  memset(&res, 0, sizeof(res));
  memset(&bld, 0, sizeof(bld));
  memset(&msc_bld, 0, sizeof(msc_bld));
  memset(&else_bld, 0, sizeof(else_bld));

  if (!tree || !tree->root)
    return (cdd_c_error_t)(CDD_C_ERROR_INVALID_ARGUMENT +
                           (int)((size_t)config & 0u));

  if (current_tree != tree) {
    rc = arena_free_all();
    if (rc != CDD_C_SUCCESS)
      return rc;
    current_tree = tree;
  }

  do {
    replaced_any = 0;
    rc = cdd_cst_find_nodes_by_type(tree->root, CDD_CST_UNKNOWN, &res);
    if (rc != CDD_C_SUCCESS) {
      current_tree = NULL;
      return cdd_c_keep_err(rc, arena_free_all());
    }

    for (i = 0; i < res.size; i++) {
      cdd_cst_node_t *stmt = res.nodes[i];
      size_t idx = 0;
      expr_t *ast = NULL;
      cdd_token_t *first_tok = NULL;
      int needs_transform = 0;

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
      rc = parse_expr_ast(stmt, &idx, 0, &ast);
      if (rc != CDD_C_SUCCESS)
        goto loop_err;
      dummy_found = 0;
      rc = find_and_mark_fopen(ast, &dummy_found);
      if (rc != CDD_C_SUCCESS)
        goto loop_err;
      rc = check_unsupported_calls(ast);
      if (rc != CDD_C_SUCCESS)
        goto loop_err;
      rc = safe_crt_query_needs_transform(ast, &needs_transform);
      if (rc != CDD_C_SUCCESS)
        goto loop_err;

      if (needs_transform) {
        msc_node = NULL;
        else_node = NULL;
        new_node = NULL;
        msc_changes = 0;
        memset(&bld, 0, sizeof(bld));
        memset(&msc_bld, 0, sizeof(msc_bld));
        memset(&else_bld, 0, sizeof(else_bld));

        active_first_tok = first_tok;
        saved_trivia = first_tok->leading_trivia;
        memset(&msc_ctx, 0, sizeof(msc_ctx));
        msc_ctx.is_msc = 1;

        rc = get_indent_string(first_tok, indent);
        if (rc != CDD_C_SUCCESS)
          goto stmt_err;

        first_tok->leading_trivia = NULL;

        rc = cdd_cst_alloc_node(CDD_CST_PREPROC_CONDITIONAL, &new_node);
        if (rc != CDD_C_SUCCESS)
          goto stmt_err;
        rc = cdd_cst_builder_init(&bld, tree, new_node);
        if (rc != CDD_C_SUCCESS)
          goto stmt_err;
        if (!config || config->safe_crt_mode != CDD_SAFE_CRT_MSVC_ONLY) {
          rc = cdd_cst_bld_newline(&bld);
          if (rc != CDD_C_SUCCESS)
            goto stmt_err;
          rc = cdd_cst_bld_ifdef(&bld, "_MSC_VER");
          if (rc != CDD_C_SUCCESS)
            goto stmt_err;
        }
        rc = cdd_cst_bld_block_comment(&bld, "CDD_SAFE_CRT");
        if (rc != CDD_C_SUCCESS)
          goto stmt_err;
        rc = cdd_cst_bld_space(&bld);
        if (rc != CDD_C_SUCCESS)
          goto stmt_err;

        rc = cdd_cst_alloc_node(CDD_CST_UNKNOWN, &msc_node);
        if (rc != CDD_C_SUCCESS)
          goto stmt_err;
        rc = cdd_cst_builder_init(&msc_bld, tree, msc_node);
        if (rc != CDD_C_SUCCESS)
          goto stmt_err;
        g_msc_ctx = &msc_ctx;
        msc_changes = 0;
        rc = emit_ast_bld(ast, &msc_bld, 1, &msc_changes);
        if (rc != CDD_C_SUCCESS)
          goto stmt_err;
        g_msc_ctx = NULL;

        rc = cdd_cst_alloc_node(CDD_CST_UNKNOWN, &else_node);
        if (rc != CDD_C_SUCCESS)
          goto stmt_err;
        rc = cdd_cst_builder_init(&else_bld, tree, else_node);
        if (rc != CDD_C_SUCCESS)
          goto stmt_err;
        rc = emit_ast_bld(ast, &else_bld, 0, NULL);
        if (rc != CDD_C_SUCCESS)
          goto stmt_err;

        first_tok->leading_trivia = saved_trivia;
        saved_trivia = NULL;

        if (msc_changes > 0) {
          size_t _ci;
          int has_ctx_decls =
              (msc_ctx.needs_errbuf || msc_ctx.needs_wcserrbuf ||
               msc_ctx.needs_strtokctx || msc_ctx.needs_wcstokctx ||
               msc_ctx.needs_mbstokctx || msc_ctx.needs_ecvtbuf ||
               msc_ctx.needs_fcvtbuf || msc_ctx.needs_getenv_ptr ||
               msc_ctx.needs_wgetenv_ptr);
          if (has_ctx_decls) {
            rc = cdd_cst_bld_token(&bld, CDD_TOKEN_OTHER, "{\n");
            if (rc != CDD_C_SUCCESS)
              goto stmt_err;
            if (msc_ctx.needs_errbuf) {
              rc = safe_crt_emit_ctx_decl(tree, &bld, indent,
                                          "char __errbuf[94];\n");
              if (rc != CDD_C_SUCCESS)
                goto stmt_err;
            }
            if (msc_ctx.needs_wcserrbuf) {
              rc = safe_crt_emit_ctx_decl(tree, &bld, indent,
                                          "wchar_t __wcserrbuf[94];\n");
              if (rc != CDD_C_SUCCESS)
                goto stmt_err;
            }
            if (msc_ctx.needs_strtokctx) {
              rc = safe_crt_emit_ctx_decl(tree, &bld, indent,
                                          "char *__strtokctx = NULL;\n");
              if (rc != CDD_C_SUCCESS)
                goto stmt_err;
            }
            if (msc_ctx.needs_wcstokctx) {
              rc = safe_crt_emit_ctx_decl(tree, &bld, indent,
                                          "wchar_t *__wcstokctx = NULL;\n");
              if (rc != CDD_C_SUCCESS)
                goto stmt_err;
            }
            if (msc_ctx.needs_mbstokctx) {
              rc = safe_crt_emit_ctx_decl(
                  tree, &bld, indent, "unsigned char *__mbstokctx = NULL;\n");
              if (rc != CDD_C_SUCCESS)
                goto stmt_err;
            }
            if (msc_ctx.needs_ecvtbuf) {
              rc = safe_crt_emit_ctx_decl(tree, &bld, indent,
                                          "char __ecvtbuf[128];\n");
              if (rc != CDD_C_SUCCESS)
                goto stmt_err;
            }
            if (msc_ctx.needs_fcvtbuf) {
              rc = safe_crt_emit_ctx_decl(tree, &bld, indent,
                                          "char __fcvtbuf[128];\n");
              if (rc != CDD_C_SUCCESS)
                goto stmt_err;
            }
            if (msc_ctx.needs_getenv_ptr) {
              rc = safe_crt_emit_ctx_decl(tree, &bld, indent,
                                          "char *__getenv_ptr;\n");
              if (rc != CDD_C_SUCCESS)
                goto stmt_err;
            }
            if (msc_ctx.needs_wgetenv_ptr) {
              rc = safe_crt_emit_ctx_decl(tree, &bld, indent,
                                          "wchar_t *__wgetenv_ptr;\n");
              if (rc != CDD_C_SUCCESS)
                goto stmt_err;
            }
            rc = safe_crt_emit_ctx_decl(tree, &bld, indent, " ");
            if (rc != CDD_C_SUCCESS)
              goto stmt_err;
          }

          for (_ci = 0; _ci < msc_node->num_children; _ci++) {
            rc = cdd_cst_append_child_token(bld.target_node,
                                            msc_node->children[_ci].val.token);
            if (rc != CDD_C_SUCCESS)
              goto stmt_err;
          }

          if (has_ctx_decls) {
            rc = cdd_cst_bld_newline(&bld);
            if (rc != CDD_C_SUCCESS)
              goto stmt_err;
            rc = safe_crt_emit_ctx_decl(tree, &bld, indent, "}");
            if (rc != CDD_C_SUCCESS)
              goto stmt_err;
          }
          if (!config || config->safe_crt_mode != CDD_SAFE_CRT_MSVC_ONLY) {
            rc = cdd_cst_bld_newline(&bld);
            if (rc != CDD_C_SUCCESS)
              goto stmt_err;
            rc = cdd_cst_bld_else(&bld);
            if (rc != CDD_C_SUCCESS)
              goto stmt_err;
            rc = cdd_cst_bld_block_comment(&bld, "CDD_SAFE_CRT");
            if (rc != CDD_C_SUCCESS)
              goto stmt_err;
            rc = cdd_cst_bld_space(&bld);
            if (rc != CDD_C_SUCCESS)
              goto stmt_err;

            for (_ci = 0; _ci < else_node->num_children; _ci++) {
              rc = cdd_cst_append_child_token(
                  bld.target_node, else_node->children[_ci].val.token);
              if (rc != CDD_C_SUCCESS)
                goto stmt_err;
            }
            rc = cdd_cst_bld_newline(&bld);
            if (rc != CDD_C_SUCCESS)
              goto stmt_err;
            rc = cdd_cst_bld_endif(&bld);
            if (rc != CDD_C_SUCCESS)
              goto stmt_err;
          }

          rc = cdd_cst_replace_node(tree, stmt, new_node);
          if (rc != CDD_C_SUCCESS)
            goto stmt_err;
          new_node = NULL;
          replaced_any = 1;
        }

        rc = safe_crt_stmt_scratch_free(replaced_any ? stmt : NULL, new_node,
                                        msc_node, else_node, &msc_bld,
                                        &else_bld, &bld);
        new_node = NULL;
        msc_node = NULL;
        else_node = NULL;

        if (replaced_any) {
          break;
        }
      }
    }

    C_CDD_FREE(res.nodes);
    res.nodes = NULL;
  } while (replaced_any);

  current_tree = NULL;
  return arena_free_all();

stmt_err:
  g_msc_ctx = NULL;
  if (saved_trivia) {
    active_first_tok->leading_trivia = saved_trivia;
  }
loop_err:
  C_CDD_FREE(res.nodes);
  current_tree = NULL;
  return cdd_c_keep_err(
      rc, cdd_c_keep_err(safe_crt_stmt_scratch_free(NULL, new_node, msc_node,
                                                    else_node, &msc_bld,
                                                    &else_bld, &bld),
                         arena_free_all()));
}
