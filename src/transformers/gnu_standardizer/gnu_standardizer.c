/**
 * @file gnu_standardizer.c
 * @brief Implementation of the GNU standardizer transformer orchestrator.
 */

/* clang-format off */
#include "gnu_standardizer_internal.h"
/* clang-format on */

/**
 * @brief Frees a shallow CST node shell (`*_free` destructor alias).
 *
 * @param[in,out] n CST node shell to free.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t gnu_node_only_free(cdd_cst_node_t *n) {
  return cdd_cst_free_node_only(n);
}

/**
 * @brief Recursively collects `CDD_CST_SYMBOL_VARIABLE` names and types from
 * scope subtree `sc` into inner (`inside func`) and outer (`inside parent_func`
 * but outside `func`) arrays.
 *
 * @param[in] sc Current scope node.
 * @param[in] parent_func Enclosing outer function CST node.
 * @param[in] func Nested function CST node.
 * @param[in] in_p Non-zero when inside `parent_func`'s scope subtree.
 * @param[in] in_f Non-zero when inside `func`'s scope subtree.
 * @param[in,out] inner_vars Buffer of inner variable names.
 * @param[in,out] io_num_inner Count of inner variables.
 * @param[in,out] outer_names Buffer of outer variable names.
 * @param[in,out] outer_types Buffer of outer variable C types.
 * @param[in,out] io_num_outer Count of outer variables.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t gnu_collect_nested_scope_vars(
    const cdd_cst_scope_t *sc, const cdd_cst_node_t *parent_func,
    const cdd_cst_node_t *func, int in_p, int in_f, char inner_vars[64][96],
    size_t *io_num_inner, char outer_names[64][96], char outer_types[64][128],
    size_t *io_num_outer) {
  const cdd_cst_symbol_t *sym;
  size_t i;
  cdd_c_error_t rc;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  if (sc->ast_node == parent_func)
    in_p = 1;
  if (sc->ast_node == func)
    in_f = 1;
  for (sym = sc->symbols; sym; sym = sym->next) {
    if (sym->kind == CDD_CST_SYMBOL_VARIABLE && strlen(sym->name) < 96) {
      if (in_f) {
        if (*io_num_inner < 64) {
          CDD_SNPRINTF(inner_vars[(*io_num_inner)++], 96, "%s", sym->name);
        }
      } else if (in_p && *io_num_outer < 64) {
        size_t idx = (*io_num_outer)++;
        CDD_SNPRINTF(outer_names[idx], 96, "%s", sym->name);
        CDD_SNPRINTF(outer_types[idx], 128, "%s",
                     sym->type_str ? sym->type_str : "int");
      }
    }
  }
  for (i = 0; i < sc->num_children; i++) {
    rc = gnu_collect_nested_scope_vars(sc->children[i], parent_func, func, in_p,
                                       in_f, inner_vars, io_num_inner,
                                       outer_names, outer_types, io_num_outer);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  return CDD_C_SUCCESS;
}
/**
 * @brief Checks if `func` (`func->parent->kind == CDD_CST_BLOCK`) is a genuine
 * `while`, `switch`) or aggregate block.
 *
 * @param[in] func Candidate `CDD_CST_FUNCTION_DEFINITION` node.
 * @param[in] name_tok Identifier token immediately preceding `(`.
 * @param[in] lparen_child_idx Index of `(` child in `func->children`.
 * @param[out] out_is_nested_fn Receives 1 if genuine nested function.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t gnu_is_real_nested_fn(const cdd_cst_node_t *func,
                                           const cdd_token_t *name_tok,
                                           size_t lparen_child_idx,
                                           int *out_is_nested_fn) {
  static const char *const k_non_fn[] = {"if",
                                         "for",
                                         "while",
                                         "switch",
                                         "catch",
                                         "sizeof",
                                         "typeof",
                                         "__typeof__",
                                         "typeof_unqual",
                                         "__attribute__",
                                         "__attribute",
                                         "__declspec",
                                         "_Alignas",
                                         "_Alignof",
                                         "__alignof",
                                         "__alignof__"};
  size_t k;
  int has_body = 0;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  *out_is_nested_fn = 0;
  if (name_tok->kind != CDD_TOKEN_IDENTIFIER || lparen_child_idx < 2) {
    return CDD_C_SUCCESS;
  }
  for (k = 0; k < sizeof(k_non_fn) / sizeof(k_non_fn[0]); k++) {
    size_t kl = strlen(k_non_fn[k]);
    if (name_tok->length == kl && memcmp(name_tok->start, k_non_fn[k], kl) == 0)
      return CDD_C_SUCCESS;
  }
  for (k = 0; k < lparen_child_idx; k++) {
    if (func->children[k].kind == CDD_CST_CHILD_TOKEN) {
      const cdd_token_t *tk = func->children[k].val.token;
      if (tk->kind == CDD_TOKEN_ASSIGN || tk->kind == CDD_TOKEN_SEMICOLON)
        return CDD_C_SUCCESS;
      if (tk->length == 7 && memcmp(tk->start, "typedef", 7) == 0)
        return CDD_C_SUCCESS;
    }
  }
  for (k = lparen_child_idx + 1; k < func->num_children; k++) {
    if (func->children[k].kind == CDD_CST_CHILD_NODE &&
        func->children[k].val.node->kind == CDD_CST_BLOCK) {
      has_body = 1;
      break;
    }
  }
  *out_is_nested_fn = has_body;
  return CDD_C_SUCCESS;
}

/**
 * @brief Collects all base token indices belonging to the subtree rooted at
 * `node` (min and max indices in `tree->base_tokens`).
 *
 * @param[in] tree CST tree.
 * @param[in] node Subtree root.
 * @param[out] out_min Minimum base token index.
 * @param[out] out_max Maximum base token index.
 * @param[out] out_found Receives 1 if at least one base token was found.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t gnu_node_token_span(const cdd_cst_tree_t *tree,
                                         const cdd_cst_node_t *node,
                                         size_t *out_min, size_t *out_max,
                                         int *out_found) {
  size_t i;
  const cdd_token_t *base = tree->base_tokens->tokens;
  size_t total = tree->base_tokens->size;
  cdd_c_error_t rc;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  for (i = 0; i < node->num_children; i++) {
    if (node->children[i].kind == CDD_CST_CHILD_TOKEN) {
      const cdd_token_t *tk = node->children[i].val.token;
      size_t bi;
      for (bi = 0; bi < total; bi++) {
        if (&base[bi] == tk) {
          if (!*out_found) {
            *out_min = bi;
            *out_found = 1;
          }
          *out_max = bi;
          break;
        }
      }
    } else {
      rc = gnu_node_token_span(tree, node->children[i].val.node, out_min,
                               out_max, out_found);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Lifts a non-trampoline nested function `func` to `static` file scope
 * before its top-level enclosing statement, synthesizing a frame struct when
 * outer local variables of `parent_func` are captured.
 *
 * @param[in,out] tree CST tree.
 * @param[in,out] gctx Optional GNU standardizer context.
 * @param[in,out] func Nested function CST node.
 * @param[in] parent_func Enclosing outer function definition (or NULL).
 * @param[in] name_tok Function name token.
 * @param[in] lparen_child_idx Child index of `(` in `func`.
 * @return CDD_C_SUCCESS or error code.
 */
static cdd_c_error_t
gnu_lift_nested_function(cdd_cst_tree_t *tree,
                         struct gnu_standardizer_ctx_t *gctx,
                         cdd_cst_node_t *func, cdd_cst_node_t *parent_func,
                         cdd_token_t *name_tok, size_t lparen_child_idx) {
  cdd_cst_node_t *parent_blk = func->parent;
  cdd_cst_node_t *top_anc = func;
  size_t top_idx = 0;
  size_t nest_idx = 0;
  size_t k;
  char fn_name[128];
  struct {
    char name[96];
    char type[128];
  } caps[32];
  size_t num_caps = 0;
  size_t f_min = 0, f_max = 0;
  int f_has_span = 0;
  size_t p_min = 0, p_max = 0;
  int p_has_span = 0;
  cdd_c_error_t rc;
  cdd_c_error_t rc_free = CDD_C_SUCCESS;

  if (name_tok->length >= sizeof(fn_name)) {
    return CDD_C_SUCCESS;
  }
  memcpy(fn_name, name_tok->start, name_tok->length);
  fn_name[name_tok->length] = '\0';

  while (top_anc->parent != tree->root) {
    top_anc = top_anc->parent;
  }

  while (tree->root->children[top_idx].kind != CDD_CST_CHILD_NODE ||
         tree->root->children[top_idx].val.node != top_anc) {
    top_idx++;
  }
  while (parent_blk->children[nest_idx].kind != CDD_CST_CHILD_NODE ||
         parent_blk->children[nest_idx].val.node != func) {
    nest_idx++;
  }

  rc = gnu_node_token_span(tree, func, &f_min, &f_max, &f_has_span);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (parent_func) {
    rc = gnu_node_token_span(tree, parent_func, &p_min, &p_max, &p_has_span);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }

  /* Identify outer local variables of `parent_func` captured inside `func` */
  if (parent_func && gctx && gctx->sem_env) {
    char inner_vars[64][96];
    char outer_names[64][96];
    char outer_types[64][128];
    size_t num_inner = 0;
    size_t num_outer = 0;
    size_t body_lb = f_min;

    while (tree->base_tokens->tokens[body_lb].kind != CDD_TOKEN_LBRACE) {
      body_lb++;
    }

    rc = gnu_collect_nested_scope_vars(gctx->sem_env->global_scope, parent_func,
                                       func, 0, 0, inner_vars, &num_inner,
                                       outer_names, outer_types, &num_outer);
    if (rc != CDD_C_SUCCESS)
      return rc;

    if (num_outer > 0) {
      size_t ti;
      for (ti = body_lb + 1; ti < f_max; ti++) {
        cdd_token_t *bt = &tree->base_tokens->tokens[ti];
        size_t iv;
        size_t ov;
        int shadowed = 0;
        if (bt->kind != CDD_TOKEN_IDENTIFIER || bt->length >= 96) {
          continue;
        }
        if (tree->base_tokens->tokens[ti - 1].kind == CDD_TOKEN_DOT ||
            tree->base_tokens->tokens[ti - 1].kind == CDD_TOKEN_ARROW) {
          continue;
        }
        for (iv = 0; iv < num_inner; iv++) {
          if (strlen(inner_vars[iv]) == bt->length &&
              memcmp(inner_vars[iv], bt->start, bt->length) == 0) {
            shadowed = 1;
            break;
          }
        }
        if (shadowed)
          continue;

        for (ov = 0; ov < num_outer; ov++) {
          if (strlen(outer_names[ov]) == bt->length &&
              memcmp(outer_names[ov], bt->start, bt->length) == 0) {
            size_t ci;
            int seen = 0;
            char deref_buf[160];
            const char *pooled_deref = NULL;
            for (ci = 0; ci < num_caps; ci++) {
              if (strcmp(caps[ci].name, outer_names[ov]) == 0) {
                seen = 1;
                break;
              }
            }
            if (!seen && num_caps < 32) {
              CDD_SNPRINTF(caps[num_caps].name, sizeof(caps[0].name), "%.95s",
                           outer_names[ov]);
              CDD_SNPRINTF(caps[num_caps].type, sizeof(caps[0].type), "%.127s",
                           outer_types[ov]);
              num_caps++;
            }
            CDD_SNPRINTF(deref_buf, sizeof(deref_buf), "(*__cdd_frame->%.95s)",
                         outer_names[ov]);
            rc = cdd_pool_string_safe(tree, deref_buf, &pooled_deref);
            if (rc != CDD_C_SUCCESS)
              return rc;
            rc = replace_token_with_text(tree, bt, CDD_TOKEN_IDENTIFIER,
                                         pooled_deref, strlen(pooled_deref));
            if (rc != CDD_C_SUCCESS)
              return rc;
            break;
          }
        }
      }
    }
  }

  /* Prepend `static ` to `func`'s first token and `\n` to its closing `}` */
  {
    cdd_token_t *ft0 = &tree->base_tokens->tokens[f_min];
    cdd_token_t *ft_end = &tree->base_tokens->tokens[f_max];
    if (!(ft0->length == 6 && memcmp(ft0->start, "static", 6) == 0)) {
      char sbuf[192];
      const char *ps = NULL;
      CDD_SNPRINTF(sbuf, sizeof(sbuf), "static %.*s", (int)ft0->length,
                   (const char *)ft0->start);
      rc = cdd_pool_string_safe(tree, sbuf, &ps);
      if (rc != CDD_C_SUCCESS)
        return rc;
      rc = replace_token_with_text(tree, ft0, ft0->kind, ps, strlen(ps));
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
    {
      char ebuf[64];
      const char *pe = NULL;
      CDD_SNPRINTF(ebuf, sizeof(ebuf), "%.*s\n", (int)ft_end->length,
                   (const char *)ft_end->start);
      rc = cdd_pool_string_safe(tree, ebuf, &pe);
      if (rc != CDD_C_SUCCESS)
        return rc;
      rc = replace_token_with_text(tree, ft_end, ft_end->kind, pe, strlen(pe));
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }

  /* If capturing outer variables, emit frame struct + rewrite signature/calls
   */
  if (num_caps > 0) {
    char frame_def[2048];
    size_t fpos = 0;
    size_t ci;
    const char *pooled_fdef = NULL;
    cdd_cst_node_t *fnode = NULL;
    cdd_cst_builder_t bld;
    cdd_token_t *lp_tok = func->children[lparen_child_idx].val.token;
    size_t lp_i = (size_t)(lp_tok - tree->base_tokens->tokens);
    size_t rp_i = lp_i + 1;
    int pd = 1;

    fpos += (size_t)CDD_SNPRINTF(frame_def, sizeof(frame_def),
                                 "struct __cdd_frame_%s { ", fn_name);
    for (ci = 0; ci < num_caps; ci++) {
      fpos += (size_t)CDD_SNPRINTF(frame_def + fpos, sizeof(frame_def) - fpos,
                                   "%s *%s; ", caps[ci].type, caps[ci].name);
    }
    fpos += (size_t)CDD_SNPRINTF(frame_def + fpos, sizeof(frame_def) - fpos,
                                 "};\n");

    rc = cdd_pool_string_safe(tree, frame_def, &pooled_fdef);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = gnu_alloc_node(CDD_CST_UNKNOWN, &fnode);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = cdd_cst_builder_init(&bld, tree, fnode);
    if (rc != CDD_C_SUCCESS) {
      return cdd_c_keep_err(rc, gnu_node_only_free(fnode));
    }
    rc = cdd_cst_bld_snippet(&bld, pooled_fdef);
    if (rc != CDD_C_SUCCESS) {
      rc_free = cdd_cst_builder_free(&bld);
      return cdd_c_keep_err(
          CDD_C_ERROR_MEMORY,
          cdd_c_keep_err(rc,
                         cdd_c_keep_err(rc_free, gnu_node_only_free(fnode))));
    }
    rc = cdd_cst_splice_children(tree, &tree->root, top_idx, 0, fnode->children,
                                 fnode->num_children);
    if (rc != CDD_C_SUCCESS) {
      rc_free = cdd_cst_builder_free(&bld);
      return cdd_c_keep_err(rc,
                            cdd_c_keep_err(rc_free, gnu_node_only_free(fnode)));
    }
    gctx->sem_env->global_scope->ast_node = tree->root;
    top_idx += fnode->num_children;
    rc_free = cdd_cst_builder_free(&bld);
    rc_free = gnu_node_only_free(fnode);

    /* Update nested function parameter list to accept `__cdd_frame` */
    for (;; rp_i++) {
      if (tree->base_tokens->tokens[rp_i].kind == CDD_TOKEN_LPAREN)
        pd++;
      else if (tree->base_tokens->tokens[rp_i].kind == CDD_TOKEN_RPAREN) {
        pd--;
        if (pd == 0)
          break;
      }
    }
    {
      int is_void_params = 0;
      char p_hdr[256];
      const char *pooled_phdr = NULL;
      if (rp_i == lp_i + 1) {
        is_void_params = 1;
      } else if (rp_i == lp_i + 2 &&
                 tree->base_tokens->tokens[lp_i + 1].length == 4 &&
                 memcmp(tree->base_tokens->tokens[lp_i + 1].start, "void", 4) ==
                     0) {
        is_void_params = 1;
        rc = replace_token_with_text(tree, &tree->base_tokens->tokens[lp_i + 1],
                                     CDD_TOKEN_IDENTIFIER, "", 0);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
      CDD_SNPRINTF(p_hdr, sizeof(p_hdr),
                   "(struct __cdd_frame_%s *__cdd_frame%s", fn_name,
                   is_void_params ? "" : ", ");
      rc = cdd_pool_string_safe(tree, p_hdr, &pooled_phdr);
      if (rc != CDD_C_SUCCESS)
        return rc;
      rc = replace_token_with_text(tree, lp_tok, CDD_TOKEN_LPAREN, pooled_phdr,
                                   strlen(pooled_phdr));
      if (rc != CDD_C_SUCCESS)
        return rc;
    }

    /* Declare `struct __cdd_frame_<fn> __cdd_f_<fn>;` at start of parent_func
     */
    {
      size_t plb = p_min;
      char decl_b[512];
      const char *pooled_db = NULL;
      int lb_len;
      while (tree->base_tokens->tokens[plb].kind != CDD_TOKEN_LBRACE) {
        plb++;
      }
      lb_len = (int)(tree->base_tokens->tokens[plb].length & 63u);
      CDD_SNPRINTF(decl_b, sizeof(decl_b),
                   "%.*s struct __cdd_frame_%s __cdd_f_%s;", lb_len,
                   (const char *)tree->base_tokens->tokens[plb].start, fn_name,
                   fn_name);
      rc = cdd_pool_string_safe(tree, decl_b, &pooled_db);
      if (rc != CDD_C_SUCCESS)
        return rc;
      rc = replace_token_with_text(tree, &tree->base_tokens->tokens[plb],
                                   CDD_TOKEN_LBRACE, pooled_db,
                                   strlen(pooled_db));
      if (rc != CDD_C_SUCCESS)
        return rc;
    }

    /* Rewrite direct calls `<fn>(args)` inside `parent_func` */
    {
      size_t ti;
      for (ti = p_min; ti + 1 <= p_max; ti++) {
        cdd_token_t *ct;
        if (ti >= f_min && ti <= f_max)
          continue;
        ct = &tree->base_tokens->tokens[ti];
        if (ct->kind == CDD_TOKEN_IDENTIFIER && ct->length == strlen(fn_name) &&
            memcmp(ct->start, fn_name, ct->length) == 0) {
          size_t clp = ti + 1;
          size_t crp = clp + 1;
          int cpd = 1;
          while (crp <= p_max) {
            if (tree->base_tokens->tokens[crp].kind == CDD_TOKEN_LPAREN)
              cpd++;
            else if (tree->base_tokens->tokens[crp].kind == CDD_TOKEN_RPAREN) {
              cpd--;
              if (cpd == 0)
                break;
            }
            crp++;
          }
          if (crp <= p_max) {
            char call_pre[1024];
            char arg_pre[160];
            size_t cp_pos = 0;
            int no_args = (crp == clp + 1);
            const char *p_cpre = NULL;
            const char *p_apre = NULL;
            cp_pos += (size_t)CDD_SNPRINTF(call_pre, sizeof(call_pre), "(");
            for (ci = 0; ci < num_caps; ci++) {
              cp_pos += (size_t)CDD_SNPRINTF(call_pre + cp_pos,
                                             sizeof(call_pre) - cp_pos,
                                             "__cdd_f_%s.%s = &%s, ", fn_name,
                                             caps[ci].name, caps[ci].name);
            }
            cp_pos += (size_t)CDD_SNPRINTF(
                call_pre + cp_pos, sizeof(call_pre) - cp_pos, "%s", fn_name);
            CDD_SNPRINTF(arg_pre, sizeof(arg_pre), "(&__cdd_f_%s%s", fn_name,
                         no_args ? "" : ", ");
            rc = cdd_pool_string_safe(tree, call_pre, &p_cpre);
            if (rc != CDD_C_SUCCESS)
              return rc;
            rc = cdd_pool_string_safe(tree, arg_pre, &p_apre);
            if (rc != CDD_C_SUCCESS)
              return rc;
            rc = replace_token_with_text(tree, ct, CDD_TOKEN_IDENTIFIER, p_cpre,
                                         strlen(p_cpre));
            if (rc != CDD_C_SUCCESS)
              return rc;
            rc = replace_token_with_text(tree, &tree->base_tokens->tokens[clp],
                                         CDD_TOKEN_LPAREN, p_apre,
                                         strlen(p_apre));
            if (rc != CDD_C_SUCCESS)
              return rc;
            rc = replace_token_with_text(tree, &tree->base_tokens->tokens[crp],
                                         CDD_TOKEN_RPAREN, "))", 2);
            if (rc != CDD_C_SUCCESS)
              return rc;
            ti = crp;
          }
        }
      }
    }
  }

  /* Insert `func` into `tree->root` at `top_idx`, then detach from `parent_blk`
   */
  {
    cdd_cst_child_t moved_child;
    moved_child.kind = CDD_CST_CHILD_NODE;
    moved_child.val.node = func;
    rc =
        cdd_cst_splice_children(tree, &tree->root, top_idx, 0, &moved_child, 1);
    if (rc != CDD_C_SUCCESS) {
      func->parent = parent_blk;
      return rc;
    }
    if (gctx && gctx->sem_env) {
      gctx->sem_env->global_scope->ast_node = tree->root;
    }
  }

  for (k = nest_idx; k + 1 < parent_blk->num_children; k++) {
    parent_blk->children[k] = parent_blk->children[k + 1];
  }
  parent_blk->num_children--;
  return rc_free;
}

/*
 * @brief Standardizes GNU function-level extensions (inline assembly, magic
 * function name identifiers, control-flow graph verification, and nested
 * functions/trampolines) using semantic context.
 *
 * @param[in,out] tree CST tree to transform.
 * @param[in,out] gctx Optional GNU standardizer context.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
cdd_c_error_t gnu_standardize_fn_ctx(cdd_cst_tree_t *tree,
                                     struct gnu_standardizer_ctx_t *gctx) {
  size_t i;
  cdd_c_error_t rc;
  cdd_c_error_t rc_free = CDD_C_SUCCESS;
  cdd_cst_query_result_t res = {0};

  if (!tree || !tree->root)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (gctx && !gctx->tree)
    gctx->tree = tree;

  rc = cdd_cst_traverse_preorder(tree->root, cdd_asm_visitor, gctx);
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc =
      cdd_cst_find_nodes_by_type(tree->root, CDD_CST_FUNCTION_DEFINITION, &res);
  if (rc != CDD_C_SUCCESS)
    return rc;
  for (i = 0; i < res.size; i++) {
    cdd_cst_node_t *func = res.nodes[i];
    size_t j;
    size_t lparen_j = 0;
    cdd_token_t *name_tok = NULL;
    for (j = 0; j < func->num_children; j++) {
      if (func->children[j].kind == CDD_CST_CHILD_TOKEN) {
        cdd_token_t *tok = func->children[j].val.token;
        if (tok->kind == CDD_TOKEN_LPAREN) {
          lparen_j = j;
          if (j > 0 && func->children[j - 1].kind == CDD_CST_CHILD_TOKEN) {
            struct magic_ctx ctx;
            name_tok = func->children[j - 1].val.token;
            ctx.tree = tree;
            ctx.func_name = name_tok->start;
            ctx.func_len = name_tok->length;
            rc = cdd_cst_traverse_preorder(func, cdd_magic_visitor, &ctx);
            if (rc != CDD_C_SUCCESS) {
              C_CDD_FREE(res.nodes);
              return rc;
            }
          }
          break;
        }
      }
    }

    /* Build and validate function CFG when semantic context is active */
    if (gctx && gctx->sem_env && name_tok &&
        name_tok->kind == CDD_TOKEN_IDENTIFIER) {
      cdd_cst_cfg_t *cfg = NULL;
      rc = cdd_cst_cfg_build(func, &cfg);
      if (rc != CDD_C_SUCCESS) {
        C_CDD_FREE(res.nodes);
        return rc;
      }
      rc_free = cdd_cst_cfg_free(cfg);
    }

    /* Detect nested functions and trampolines, then lift direct-call nested
       functions to file scope */
    if (name_tok && func->parent->kind == CDD_CST_BLOCK) {
      struct tramp_ctx t_ctx;
      cdd_cst_node_t *parent_func;
      int is_real_nested = 0;
      t_ctx.name = name_tok->start;
      t_ctx.length = name_tok->length;
      t_ctx.is_tramp = 0;
      t_ctx.func_node = func;

      parent_func = func->parent;
      while (parent_func && parent_func->kind != CDD_CST_FUNCTION_DEFINITION) {
        parent_func = parent_func->parent;
      }
      rc = cdd_cst_traverse_preorder(parent_func ? parent_func : tree->root,
                                     cdd_tramp_visitor, &t_ctx);
      if (t_ctx.is_tramp) {
        C_CDD_FREE(res.nodes);
        return cdd_c_keep_err(CDD_C_ERROR_SYSTEM, rc);
      }

      rc = gnu_is_real_nested_fn(func, name_tok, lparen_j, &is_real_nested);
      if (rc != CDD_C_SUCCESS) {
        C_CDD_FREE(res.nodes);
        return rc;
      }
      if (is_real_nested) {
        rc = gnu_lift_nested_function(tree, gctx, func, parent_func, name_tok,
                                      lparen_j);
        if (rc != CDD_C_SUCCESS) {
          C_CDD_FREE(res.nodes);
          return rc;
        }
      }
    }
  }
  C_CDD_FREE(res.nodes);
  return rc_free;
}

/*
 * @brief Standardizes GNU function-level extensions without external context.
 *
 * @param[in,out] tree CST tree to transform.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
cdd_c_error_t gnu_standardize_fn(cdd_cst_tree_t *tree) {
  return gnu_standardize_fn_ctx(tree, NULL);
}

/*
 * @brief Applies GNU extension standardization transformations to the CST.
 *
 * @param[in,out] tree CST tree to transform.
 * @param[in] config Optional transformation configuration.
 * @return CDD_C_SUCCESS on success, or error code on failure.
 */
cdd_c_error_t cdd_transform_gnu(cdd_cst_tree_t *tree,
                                const cdd_transform_config_t *config) {
  struct gnu_standardizer_ctx_t gctx;
  cdd_c_error_t rc;
  cdd_c_error_t rc_free = CDD_C_SUCCESS;

  if (!tree || !tree->root)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  memset(&gctx, 0, sizeof(gctx));
  gctx.config = config;
  gctx.tree = tree;

  rc = cdd_cst_build_semantic_info_ex(
      tree, config ? config->include_dirs : NULL,
      config ? config->num_include_dirs : 0, &gctx.sem_env);
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc = gnu_standardize_fn_ctx(tree, &gctx);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  rc = gnu_standardize_types_ctx(tree, &gctx);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  rc = gnu_standardize_expr_ctx(tree, &gctx);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  rc = gnu_standardize_unroll_ctx(tree, &gctx);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  rc = gnu_standardize_ranges_ctx(tree, &gctx);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  rc = gnu_standardize_builtins_ctx(tree, &gctx);
  if (rc != CDD_C_SUCCESS)
    goto cleanup;

  if (gctx.used_int128) {
    cdd_cst_node_t *inc_node = NULL;
    cdd_cst_builder_t bld;
    rc = gnu_alloc_node(CDD_CST_UNKNOWN, &inc_node);
    if (rc != CDD_C_SUCCESS)
      goto cleanup;
    rc = cdd_cst_builder_init(&bld, tree, inc_node);
    if (rc != CDD_C_SUCCESS) {
      rc_free = gnu_node_only_free(inc_node);
      goto cleanup;
    }
    rc = cdd_cst_bld_snippet(
        &bld, "#ifndef C_CDD_INT128_MATH_H\n#include <cdd_int128.h>\n#endif\n");
    if (rc != CDD_C_SUCCESS) {
      rc_free = cdd_cst_builder_free(&bld);
      rc_free = gnu_node_only_free(inc_node);
      goto cleanup;
    }
    rc = cdd_cst_splice_children(tree, &tree->root, 0, 0, inc_node->children,
                                 inc_node->num_children);
    if (rc != CDD_C_SUCCESS) {
      rc_free = cdd_cst_builder_free(&bld);
      rc_free = gnu_node_only_free(inc_node);
      goto cleanup;
    }
    rc_free = cdd_cst_builder_free(&bld);
    rc_free = gnu_node_only_free(inc_node);
  }

  return cdd_c_keep_err(rc_free, cdd_cst_scope_env_free(gctx.sem_env));

cleanup:;
  return cdd_c_keep_err(
      rc, cdd_c_keep_err(rc_free, cdd_cst_scope_env_free(gctx.sem_env)));
}
