/**
 * @file gnu_standardizer_unroll.c
 * @brief Statement unrolling and VLA standardization pass for GNU standardizer.
 */

/* clang-format off */
#include "gnu_standardizer_internal.h"
/* clang-format on */

/** @brief Struct definition */
typedef struct {
  /** @brief field */
  const uint8_t *name;
  /** @brief field */
  size_t length;
  /** @brief field */
  char rename[64];
  /** @brief field */
  int depth;
} local_label_t;

/** @brief Struct definition */
typedef struct {
  /** @brief field */
  const uint8_t *name;
  /** @brief field */
  size_t length;
  /** @brief field */
  int depth;
} vla_t;

/** @brief Struct definition */
typedef struct {
  /** @brief field */
  const uint8_t *var_name;
  /** @brief field */
  size_t var_length;
  /** @brief field */
  const uint8_t *func_name;
  /** @brief field */
  size_t func_length;
  /** @brief field */
  int depth;
} cleanup_t;

cdd_c_error_t gnu_standardize_unroll(cdd_cst_tree_t *tree,
                                     const cdd_transform_config_t *config) {
  local_label_t local_labels[MAX_LOCAL_LABELS];
  vla_t vlas[MAX_VLAS];
  cleanup_t cleanups[MAX_CLEANUPS];
  size_t num_local_labels = 0;
  size_t num_vlas = 0;
  size_t num_cleanups = 0;
  int current_depth = 0;
  int label_counter = 0;
  size_t i;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  for (i = 0; i < tree->base_tokens->size; i++) {
    cdd_token_t *t = &tree->base_tokens->tokens[i];
    if (t->kind == CDD_TOKEN_LBRACE) {
      current_depth++;
    } else if (t->kind == CDD_TOKEN_IDENTIFIER && t->length == 13 &&
               memcmp(t->start, "__attribute__", 13) == 0) {
      if (i + 8 < tree->base_tokens->size &&
          tree->base_tokens->tokens[i + 4].kind == CDD_TOKEN_LPAREN &&
          tree->base_tokens->tokens[i + 8].kind == CDD_TOKEN_RPAREN) {
        cdd_token_t *attr = &tree->base_tokens->tokens[i + 3];
        if (attr->length == 7 && memcmp(attr->start, "cleanup", 7) == 0) {
          cdd_token_t *func_val = &tree->base_tokens->tokens[i + 5];
          cdd_token_t *var_tok = NULL;
          size_t k;
          for (k = i + 9; k < tree->base_tokens->size; k++) {
            if (tree->base_tokens->tokens[k].kind == CDD_TOKEN_SEMICOLON ||
                tree->base_tokens->tokens[k].kind == CDD_TOKEN_ASSIGN ||
                tree->base_tokens->tokens[k].kind == CDD_TOKEN_COMMA) {
              size_t back;
              for (back = k; back > i + 8; back--) {
                if (tree->base_tokens->tokens[back].kind ==
                    CDD_TOKEN_IDENTIFIER) {
                  var_tok = &tree->base_tokens->tokens[back];
                  break;
                }
              }
              break;
            }
          }
          if (var_tok && num_cleanups < MAX_CLEANUPS) {
            cleanups[num_cleanups].var_name = var_tok->start;
            cleanups[num_cleanups].var_length = var_tok->length;
            cleanups[num_cleanups].func_name = func_val->start;
            cleanups[num_cleanups].func_length = func_val->length;
            cleanups[num_cleanups].depth = current_depth;
            num_cleanups++;
          }

          {
            size_t tk_idx;
            for (tk_idx = 0; tk_idx <= 8; tk_idx++) {
              rc = replace_token_with_text(
                  tree, &tree->base_tokens->tokens[i + tk_idx],
                  tree->base_tokens->tokens[i + tk_idx].kind, "", 0);
              if (rc != CDD_C_SUCCESS)
                return rc;
            }
          }
          i += 8;
        }
      }
    } else if (t->kind == CDD_TOKEN_RBRACE) {
      while (num_local_labels > 0 &&
             local_labels[num_local_labels - 1].depth == current_depth) {
        num_local_labels--;
      }
      {
        char buf[4096] = {0};
        char *p = buf;
        int appended = 0;
        size_t child_idx;
        cdd_cst_node_t *owning_node = NULL;
        rc = gnu_find_node(tree->root, t, &child_idx, &owning_node);
#ifdef CDD_BUILD_TESTS
        if (g_gnu_standardizer_fail == 10)
          rc = CDD_C_ERROR_INVALID_ARGUMENT;
#endif
        if (rc != CDD_C_SUCCESS)
          return rc;

        /* Run cleanups in reverse order */
        while (num_cleanups > 0 &&
               cleanups[num_cleanups - 1].depth == current_depth) {
          cdd_cst_node_t *temp = NULL;
          cdd_cst_builder_t bld;
          rc = gnu_alloc_node(CDD_CST_UNKNOWN, &temp);
          if (rc != CDD_C_SUCCESS)
            return rc;
          cdd_cst_builder_init(&bld, tree, temp);
          cdd_cst_bld_ident(
              &bld,
              pool_string_safe_len(
                  tree, (const char *)cleanups[num_cleanups - 1].func_name,
                  cleanups[num_cleanups - 1].func_length));
          cdd_cst_bld_punct(&bld, "(");
          cdd_cst_bld_punct(&bld, "&");
          cdd_cst_bld_ident(
              &bld, pool_string_safe_len(
                        tree, (const char *)cleanups[num_cleanups - 1].var_name,
                        cleanups[num_cleanups - 1].var_length));
          cdd_cst_bld_punct(&bld, ")");
          cdd_cst_bld_punct(&bld, ";");
          if (bld.error_state != 0) {
            cdd_cst_builder_free(&bld);
            cdd_cst_free_node_only(temp);
            return CDD_C_ERROR_MEMORY;
          }
          cdd_cst_splice_children(tree, &owning_node, child_idx, 0,
                                  temp->children, temp->num_children);
          cdd_cst_builder_free(&bld);
          cdd_cst_free_node_only(temp);
          num_cleanups--;
          appended = 1;
        }

        if (config && config->fallback_vla_to_malloc) {
          while (num_vlas > 0 && vlas[num_vlas - 1].depth == current_depth) {
            cdd_cst_node_t *temp = NULL;
            cdd_cst_builder_t bld;
            rc = gnu_alloc_node(CDD_CST_UNKNOWN, &temp);
            if (rc != CDD_C_SUCCESS)
              return rc;
            cdd_cst_builder_init(&bld, tree, temp);
            cdd_cst_bld_ident(&bld, "free");
            cdd_cst_bld_punct(&bld, "(");
            cdd_cst_bld_ident(&bld,
                              pool_string_safe_len(
                                  tree, (const char *)vlas[num_vlas - 1].name,
                                  vlas[num_vlas - 1].length));
            cdd_cst_bld_punct(&bld, ")");
            cdd_cst_bld_punct(&bld, ";");
            if (bld.error_state != 0) {
              cdd_cst_builder_free(&bld);
              cdd_cst_free_node_only(temp);
              return CDD_C_ERROR_MEMORY;
            }
            cdd_cst_splice_children(tree, &owning_node, child_idx, 0,
                                    temp->children, temp->num_children);
            cdd_cst_builder_free(&bld);
            cdd_cst_free_node_only(temp);
            num_vlas--;
            appended = 1;
          }
        }

        if (appended) {
          char *heap_buf;
          const char *pooled;
#if defined(_MSC_VER)
          strcat_s(p, sizeof(buf) - (size_t)(p - buf), " }");
#else
          strcat(p, " }");
#endif
          rc = gnu_malloc(strlen(buf) + 1, (void **)&heap_buf);
          if (rc != CDD_C_SUCCESS)
            return rc;
#if defined(_MSC_VER)
          strcpy_s(heap_buf, strlen(buf) + 1, buf);
#else
          strcpy(heap_buf, buf);
#endif
#ifdef CDD_BUILD_TESTS
          if (g_gnu_standardizer_fail == 17)
            pooled = NULL;
          else
#endif
            pooled = pool_string_safe(tree, heap_buf);
          if (!pooled) {
            free(heap_buf);
            return CDD_C_ERROR_MEMORY;
          }
#ifdef CDD_BUILD_TESTS
          if (g_gnu_standardizer_fail != 8)
#endif
          {
            rc = replace_token_with_text(tree, t, t->kind, pooled,
                                         strlen(heap_buf));
            if (rc != CDD_C_SUCCESS) {
              free(heap_buf);
              return rc;
            }
          }
          free(heap_buf);
        }
      }
      current_depth--;
    } else if (t->kind == CDD_TOKEN_KEYWORD_RETURN) {
      if (num_cleanups > 0 ||
          (config && config->fallback_vla_to_malloc && num_vlas > 0)) {
        size_t k;
        for (k = i + 1; k < tree->base_tokens->size; k++) {
          if (tree->base_tokens->tokens[k].kind == CDD_TOKEN_SEMICOLON) {
            int has_expr = (k > i + 1);
            char buf[4096] = {0};
            char *p = buf;
            size_t c_idx, v_idx;

            if (has_expr) {
              rc = replace_token_with_text(tree, t, t->kind,
                                           "{ __auto_type __cdd_ret = (", 27);
              if (rc != CDD_C_SUCCESS)
                return rc;
#if defined(_MSC_VER)
              strcpy_s(p, sizeof(buf) - (size_t)(p - buf), "); ");
#else
              strcpy(p, "); ");
#endif
              p += 3;
            } else {
              rc = replace_token_with_text(tree, t, t->kind, "{ ", 2);
              if (rc != CDD_C_SUCCESS)
                return rc;
            }

            for (c_idx = num_cleanups; c_idx > 0; c_idx--) {
              p += CDD_SNPRINTF(p, sizeof(buf) - (size_t)(p - buf),
                                "%.*s(&%.*s); ",
                                (int)cleanups[c_idx - 1].func_length,
                                cleanups[c_idx - 1].func_name,
                                (int)cleanups[c_idx - 1].var_length,
                                cleanups[c_idx - 1].var_name);
            }
            if (config && config->fallback_vla_to_malloc) {
              for (v_idx = num_vlas; v_idx > 0; v_idx--) {
                p += CDD_SNPRINTF(p, sizeof(buf) - (size_t)(p - buf),
                                  "free(%.*s); ", (int)vlas[v_idx - 1].length,
                                  vlas[v_idx - 1].name);
              }
            }

            if (has_expr) {
#if defined(_MSC_VER)
              strcpy_s(p, sizeof(buf) - (size_t)(p - buf),
                       "return __cdd_ret; }");
#else
              strcpy(p, "return __cdd_ret; }");
#endif
              p += 19;
            } else {
#if defined(_MSC_VER)
              strcpy_s(p, sizeof(buf) - (size_t)(p - buf), "return; }");
#else
              strcpy(p, "return; }");
#endif
              p += 9;
            }
            {
              char *dup;
              const char *pooled;
              rc = gnu_malloc(strlen(buf) + 1, (void **)&dup);
              if (rc != CDD_C_SUCCESS)
                return rc;
#if defined(_MSC_VER)
              strcpy_s(dup, strlen(buf) + 1, buf);
#else
              strcpy(dup, buf);
#endif
              pooled = pool_string_safe(tree, dup);
              free(dup);
              if (!pooled)
                return CDD_C_ERROR_MEMORY;
#ifdef CDD_BUILD_TESTS
              if (g_gnu_standardizer_fail != 11)
#endif
              {
                rc = replace_token_with_text(
                    tree, &tree->base_tokens->tokens[k],
                    tree->base_tokens->tokens[k].kind, pooled, strlen(buf));
                if (rc != CDD_C_SUCCESS)
                  return rc;
              }
            }
            break;
          }
        }
      }
    } else if (t->kind == CDD_TOKEN_KEYWORD_GOTO) {
      if (num_cleanups > 0) {
        rc = replace_token_with_text(
            tree, t, t->kind,
            "/* warning: goto cross-scope cleanups unsupported */ goto", 57);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }

      if (num_vlas > 0) {
        rc = replace_token_with_text(
            tree, t, t->kind,
            "/* warning: goto crossing VLA scopes unsupported */ goto", 56);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }

      if (tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_STAR) {
        rc = replace_token_with_text(
            tree, t, t->kind,
            "/* warning: computed goto converting jump tables to switch "
            "internally unsupported */ goto",
            89);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
    } else if (t->kind == CDD_TOKEN_IDENTIFIER && t->length == 7 &&
               memcmp(t->start, "longjmp", 7) == 0) {
      if (num_cleanups > 0) {
        rc = replace_token_with_text(
            tree, t, t->kind,
            "/* warning: longjmp bypasses cleanups */ longjmp", 48);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
    } else if (t->kind == CDD_TOKEN_KEYWORD___LABEL__) {
      rc = replace_token_with_text(tree, t, t->kind, "", 0);
      if (rc != CDD_C_SUCCESS)
        return rc;
      i++;
      while (i < tree->base_tokens->size) {
        cdd_token_t *nt = &tree->base_tokens->tokens[i];
        if (nt->kind == CDD_TOKEN_IDENTIFIER) {
          if (num_local_labels < MAX_LOCAL_LABELS) {
            local_labels[num_local_labels].name = nt->start;
            local_labels[num_local_labels].length = nt->length;
            local_labels[num_local_labels].depth = current_depth;
            {
              char *p = local_labels[num_local_labels].rename;
#if defined(_MSC_VER)
              strcpy_s(p, sizeof(local_labels[num_local_labels].rename),
                       "__cdd_ll_");
#else
              strcpy(p, "__cdd_ll_");
#endif
              p += 9;
              memcpy(p, nt->start, nt->length);
              p += nt->length;
              *p++ = '_';
              if (cdd_append_int(p, ++label_counter, &p) != 0)
                return CDD_C_ERROR_MEMORY;
            }
            num_local_labels++;
          }
          rc = replace_token_with_text(tree, nt, nt->kind, "", 0);
          if (rc != CDD_C_SUCCESS)
            return rc;
        } else if (nt->kind == CDD_TOKEN_COMMA) {
          rc = replace_token_with_text(tree, nt, nt->kind, "", 0);
          if (rc != CDD_C_SUCCESS)
            return rc;
        } else if (nt->kind == CDD_TOKEN_SEMICOLON) {
          rc = replace_token_with_text(tree, nt, nt->kind, "", 0);
          if (rc != CDD_C_SUCCESS)
            return rc;
          break;
        } else {
          break;
        }
        i++;
      }
    } else if (t->kind == CDD_TOKEN_IDENTIFIER && num_local_labels > 0) {
      /* Check if it's a label definition, a goto target, or an
       * address-of-label */
      int is_label_ref = 0;
      if (tree->base_tokens->tokens[i - 1].kind == CDD_TOKEN_KEYWORD_GOTO) {
        is_label_ref = 1;
      } else if (tree->base_tokens->tokens[i - 1].start[0] == '&' &&
                 tree->base_tokens->tokens[i - 2].start[0] == '&') {
        is_label_ref = 1;
      } else if (tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_COLON) {
        is_label_ref = 1;
      }

      if (is_label_ref) {
        size_t j;
        for (j = num_local_labels; j-- > 0;) {
          if (local_labels[j].length == t->length &&
              memcmp(local_labels[j].name, t->start, t->length) == 0) {
            char *dup;
            const char *pooled;
            rc = gnu_malloc(strlen(local_labels[j].rename) + 1, (void **)&dup);
            if (rc != CDD_C_SUCCESS)
              return rc;
#if defined(_MSC_VER)
            strcpy_s(dup, strlen(local_labels[j].rename) + 1,
                     local_labels[j].rename);
#else
            strcpy(dup, local_labels[j].rename);
#endif
            pooled = pool_string_safe(tree, dup);
            free(dup);
            if (!pooled)
              return CDD_C_ERROR_MEMORY;
#ifdef CDD_BUILD_TESTS
            if (g_gnu_standardizer_fail != 12)
#endif
            {
              rc = replace_token_with_text(tree, t, t->kind, pooled,
                                           strlen(local_labels[j].rename));
              if (rc != CDD_C_SUCCESS)
                return rc;
            }
            break;
          }
        }
      }
    }

    /* Existing unroll logic */
    if (t->kind == CDD_TOKEN_LPAREN && i + 1 < tree->base_tokens->size &&
        tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_LBRACE) {
      /* Remove ({ */
      rc = replace_token_with_text(tree, t, t->kind, "", 0);
      if (rc != CDD_C_SUCCESS)
        return rc;
      rc =
          replace_token_with_text(tree, &tree->base_tokens->tokens[i + 1],
                                  tree->base_tokens->tokens[i + 1].kind, "", 0);
      if (rc != CDD_C_SUCCESS)
        return rc;
    } else if (t->kind == CDD_TOKEN_RBRACE && i + 1 < tree->base_tokens->size &&
               tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_RPAREN) {
      /* Remove }) */
      rc = replace_token_with_text(tree, t, t->kind, "", 0);
      if (rc != CDD_C_SUCCESS)
        return rc;
      rc =
          replace_token_with_text(tree, &tree->base_tokens->tokens[i + 1],
                                  tree->base_tokens->tokens[i + 1].kind, "", 0);
      if (rc != CDD_C_SUCCESS)
        return rc;
    } else if (t->kind == CDD_TOKEN_LBRACKET && i > 0) {
      cdd_token_t *prev = &tree->base_tokens->tokens[i - 1];
      cdd_token_t *next = &tree->base_tokens->tokens[i + 1];
      cdd_token_t *semi = &tree->base_tokens->tokens[i + 2];
      if (prev->kind == CDD_TOKEN_IDENTIFIER &&
          next->kind == CDD_TOKEN_IDENTIFIER) {

        /* Count dimensions */
        int dims = 0;
        size_t k = i;
        char size_expr[256] = {0};
        char *p_sz = size_expr;
        while (k + 2 < tree->base_tokens->size &&
               tree->base_tokens->tokens[k].kind == CDD_TOKEN_LBRACKET &&
               tree->base_tokens->tokens[k + 1].kind == CDD_TOKEN_IDENTIFIER &&
               tree->base_tokens->tokens[k + 2].kind == CDD_TOKEN_RBRACKET) {
          if (dims > 0) {
            *p_sz++ = ' ';
            *p_sz++ = '*';
            *p_sz++ = ' ';
          }
          memcpy(p_sz, tree->base_tokens->tokens[k + 1].start,
                 tree->base_tokens->tokens[k + 1].length);
          p_sz += tree->base_tokens->tokens[k + 1].length;
          dims++;
          k += 3;
        }
        *p_sz = '\0';
        if (dims > 0 && k < tree->base_tokens->size) {
          cdd_token_t *end_tok = &tree->base_tokens->tokens[k];
          if (end_tok->kind == CDD_TOKEN_SEMICOLON) {

            if (config && config->fallback_vla_to_malloc) {
              {
                size_t child_idx;
                cdd_cst_node_t *owning_node = NULL;
                rc = gnu_find_node(tree->root, prev, &child_idx, &owning_node);
                if (rc != CDD_C_SUCCESS)
                  return rc;
                {
                  cdd_cst_node_t *temp = NULL;
                  cdd_cst_builder_t bld;
                  rc = gnu_alloc_node(CDD_CST_UNKNOWN, &temp);
                  if (rc != CDD_C_SUCCESS)
                    return rc;
                  cdd_cst_builder_init(&bld, tree, temp);
                  cdd_cst_bld_punct(&bld, "*");
                  cdd_cst_bld_ident(
                      &bld, pool_string_safe_len(
                                tree, (const char *)prev->start, prev->length));
                  cdd_cst_bld_space(&bld);
                  cdd_cst_bld_punct(&bld, "=");
                  cdd_cst_bld_space(&bld);
                  cdd_cst_bld_ident(&bld, "malloc");
                  cdd_cst_bld_punct(&bld, "(");
                  cdd_cst_bld_punct(&bld, "(");
                  cdd_cst_bld_snippet(&bld, size_expr);
                  cdd_cst_bld_punct(&bld, ")");
                  cdd_cst_bld_space(&bld);
                  cdd_cst_bld_punct(&bld, "*");
                  cdd_cst_bld_space(&bld);
                  cdd_cst_bld_ident(&bld, "sizeof");
                  cdd_cst_bld_punct(&bld, "(");
                  cdd_cst_bld_punct(&bld, "*");
                  cdd_cst_bld_ident(
                      &bld, pool_string_safe_len(
                                tree, (const char *)prev->start, prev->length));
                  cdd_cst_bld_punct(&bld, ")");
                  cdd_cst_bld_punct(&bld, ")");
                  cdd_cst_bld_punct(&bld, ";");
                  if (bld.error_state != 0) {
                    cdd_cst_builder_free(&bld);
                    cdd_cst_free_node_only(temp);
                    return CDD_C_ERROR_MEMORY;
                  }
                  cdd_cst_splice_children(tree, &owning_node, child_idx, 0,
                                          temp->children, temp->num_children);
                  cdd_cst_builder_free(&bld);
                  cdd_cst_free_node_only(temp);
                }
              }
              if (num_vlas < MAX_VLAS) {
                vlas[num_vlas].name = prev->start;
                vlas[num_vlas].length = prev->length;
                vlas[num_vlas].depth = current_depth;
                num_vlas++;
              }
              rc = replace_token_with_text(tree, prev, prev->kind, "", 0);
              if (rc != CDD_C_SUCCESS)
                return rc;
              rc = replace_token_with_text(tree, end_tok, end_tok->kind, "", 0);
              if (rc != CDD_C_SUCCESS)
                return rc;
            } else {
              {
                size_t child_idx;
                cdd_cst_node_t *owning_node = NULL;
                rc = gnu_find_node(tree->root, prev, &child_idx, &owning_node);
                if (rc != CDD_C_SUCCESS)
                  return rc;
                {
                  cdd_cst_node_t *temp = NULL;
                  cdd_cst_builder_t bld;
                  rc = gnu_alloc_node(CDD_CST_UNKNOWN, &temp);
                  if (rc != CDD_C_SUCCESS)
                    return rc;
                  cdd_cst_builder_init(&bld, tree, temp);
                  cdd_cst_bld_punct(&bld, "*");
                  cdd_cst_bld_ident(
                      &bld, pool_string_safe_len(
                                tree, (const char *)prev->start, prev->length));
                  cdd_cst_bld_space(&bld);
                  cdd_cst_bld_punct(&bld, "=");
                  cdd_cst_bld_space(&bld);
                  cdd_cst_bld_ident(&bld, "alloca");
                  cdd_cst_bld_punct(&bld, "(");
                  cdd_cst_bld_punct(&bld, "(");
                  cdd_cst_bld_snippet(&bld, size_expr);
                  cdd_cst_bld_punct(&bld, ")");
                  cdd_cst_bld_space(&bld);
                  cdd_cst_bld_punct(&bld, "*");
                  cdd_cst_bld_space(&bld);
                  cdd_cst_bld_ident(&bld, "sizeof");
                  cdd_cst_bld_punct(&bld, "(");
                  cdd_cst_bld_punct(&bld, "*");
                  cdd_cst_bld_ident(
                      &bld, pool_string_safe_len(
                                tree, (const char *)prev->start, prev->length));
                  cdd_cst_bld_punct(&bld, ")");
                  cdd_cst_bld_punct(&bld, ")");
                  cdd_cst_bld_punct(&bld, ";");
                  if (bld.error_state != 0) {
                    cdd_cst_builder_free(&bld);
                    cdd_cst_free_node_only(temp);
                    return CDD_C_ERROR_MEMORY;
                  }
                  cdd_cst_splice_children(tree, &owning_node, child_idx, 0,
                                          temp->children, temp->num_children);
                  cdd_cst_builder_free(&bld);
                  cdd_cst_free_node_only(temp);
                }
                rc = replace_token_with_text(tree, prev, prev->kind, "", 0);
                if (rc != CDD_C_SUCCESS)
                  return rc;
                rc = replace_token_with_text(tree, end_tok, end_tok->kind, "",
                                             0);
                if (rc != CDD_C_SUCCESS)
                  return rc;
              }
            }
            {
              size_t wipe;
              for (wipe = i; wipe < k; wipe++) {
                tree->base_tokens->tokens[wipe].length = 0;
              }
            }
          } else if (end_tok->kind == CDD_TOKEN_COMMA ||
                     end_tok->kind == CDD_TOKEN_RPAREN) {
            /* VLA parameter */
            size_t wipe;
            for (wipe = i; wipe < k; wipe++) {
              if (tree->base_tokens->tokens[wipe].kind ==
                  CDD_TOKEN_IDENTIFIER) {
                tree->base_tokens->tokens[wipe].length = 0;
              }
            }
          }
        }
      } else if (prev->kind == CDD_TOKEN_IDENTIFIER &&
                 next->kind == CDD_TOKEN_NUMBER && next->length == 1 &&
                 next->start[0] == '0' && semi->kind == CDD_TOKEN_RBRACKET &&
                 i + 3 < tree->base_tokens->size &&
                 tree->base_tokens->tokens[i + 3].kind == CDD_TOKEN_SEMICOLON) {
        /* Found zero-length array pattern `int arr[0];` */
        /* Look forward for RBRACE to see if it is at the end of a struct */
        size_t fwd;
        int found_rbrace = 0;
        for (fwd = i + 4; fwd < tree->base_tokens->size; fwd++) {
#ifdef CDD_BUILD_TESTS
          if (g_gnu_standardizer_fail == 20 && fwd == i + 4) {
            tree->base_tokens->tokens[fwd].length = 0;
          }
#endif
          if (tree->base_tokens->tokens[fwd].length == 0) {
            continue;
          } else if (tree->base_tokens->tokens[fwd].kind == CDD_TOKEN_RBRACE) {
            found_rbrace = 1;
            break;
          } else {
            break;
          }
        }
        if (found_rbrace) {
          /* Flexible Array Member polyfill -> change [0] to [1] */
          rc = replace_token_with_text(tree, next, next->kind, "1", 1);
          if (rc != CDD_C_SUCCESS)
            return rc;
        } else {
          /* Reject zero-length array in the middle of a struct */
          rc = replace_token_with_text(
              tree, next, next->kind,
              "-1 /* zero-length array in middle of struct */", 46);
          if (rc != CDD_C_SUCCESS)
            return rc;
        }
      }
    }
  }
  return CDD_C_SUCCESS;
}
