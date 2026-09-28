/**
 * @file gnu_standardizer_ranges.c
 * @brief Case ranges, initializers, and builtin expect standardization pass.
 */

/* clang-format off */
#include "gnu_standardizer_internal.h"
/* clang-format on */

cdd_c_error_t gnu_standardize_ranges(cdd_cst_tree_t *tree) {
  size_t i;
  cdd_c_error_t rc = CDD_C_SUCCESS;

  for (i = 0; i < tree->base_tokens->size; i++) {
    cdd_token_t *t = &tree->base_tokens->tokens[i];
    if (t->kind == CDD_TOKEN_COMMA) {
      /* Strip trailing commas before RBRACE */
      size_t next_idx = i + 1;
      while (next_idx < tree->base_tokens->size &&
             tree->base_tokens->tokens[next_idx].length == 0) {
        next_idx++;
      }
      if (next_idx < tree->base_tokens->size &&
          tree->base_tokens->tokens[next_idx].kind == CDD_TOKEN_RBRACE) {
        rc = replace_token_with_text(tree, t, t->kind, "", 0);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
    } else if (t->kind == CDD_TOKEN_LBRACE && i + 1 < tree->base_tokens->size &&
               tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_RBRACE) {
      /* Determine if it's an empty struct or empty initializer */
      int is_initializer = 1;
      size_t k;
      int depth = 0;
      for (k = i; k-- > 0;) {
        cdd_token_t *tk = &tree->base_tokens->tokens[k];
        if (tk->kind == CDD_TOKEN_RBRACE)
          depth++;
        else if (tk->kind == CDD_TOKEN_LBRACE)
          depth--;

        if (depth == 0) {
          if (tk->kind == CDD_TOKEN_KEYWORD_STRUCT ||
              (tk->kind == CDD_TOKEN_IDENTIFIER && tk->length == 5 &&
               memcmp(tk->start, "union", 5) == 0) ||
              (tk->kind == CDD_TOKEN_IDENTIFIER && tk->length == 4 &&
               memcmp(tk->start, "enum", 4) == 0)) {
            is_initializer = 0;
            break;
          } else if (tk->kind == CDD_TOKEN_SEMICOLON) {
            break;
          } else if (tk->kind == CDD_TOKEN_ASSIGN) {
            is_initializer = 1;
            break;
          }
        }
      }

      if (i > 0 && tree->base_tokens->tokens[i - 1].kind == CDD_TOKEN_COLON) {
        /* Empty block following a case or default label */
        tree->base_tokens->tokens[i].start = (const uint8_t *)";";
        tree->base_tokens->tokens[i].length = 1;
        tree->base_tokens->tokens[i + 1].start = (const uint8_t *)"";
        tree->base_tokens->tokens[i + 1].length = 0;
      } else if (is_initializer) {
        tree->base_tokens->tokens[i + 1].start = (const uint8_t *)" 0 }";
        tree->base_tokens->tokens[i + 1].length = 4;
      } else {
        tree->base_tokens->tokens[i + 1].start =
            (const uint8_t *)" char _pad; }";
        tree->base_tokens->tokens[i + 1].length = 13;
      }
    } else if (t->kind == CDD_TOKEN_DOT && i > 1 &&
               i + 2 < tree->base_tokens->size) {
      /* Look for ... */
      if (tree->base_tokens->tokens[i + 1].kind == CDD_TOKEN_DOT &&
          tree->base_tokens->tokens[i + 2].kind == CDD_TOKEN_DOT) {

        cdd_token_t *prev = &tree->base_tokens->tokens[i - 1];
        cdd_token_t *next = &tree->base_tokens->tokens[i + 3];
        int is_case = 0;
        int is_range_init = 0;
        size_t is_case_idx = 0;
        size_t is_range_idx = 0;
        int is_neg_start = 0;
        int is_neg_end = 0;
        int start_val = 0;
        int end_val = 0;

        /* Check for negative numbers */
        if (prev->kind == CDD_TOKEN_NUMBER) {
          if (tree->base_tokens->tokens[i - 2].kind == CDD_TOKEN_MINUS) {
            is_neg_start = 1;
          }
        }
        if (next->kind == CDD_TOKEN_MINUS && i + 4 < tree->base_tokens->size &&
            tree->base_tokens->tokens[i + 4].kind == CDD_TOKEN_NUMBER) {
          is_neg_end = 1;
          next = &tree->base_tokens->tokens[i + 4];
        }

        if (prev->kind == CDD_TOKEN_NUMBER && next->kind == CDD_TOKEN_NUMBER) {
          char buf[64];
          memcpy(buf, prev->start, prev->length);
          buf[prev->length] = '\0';
          start_val = atoi(buf);
          if (is_neg_start)
            start_val = -start_val;

          memcpy(buf, next->start, next->length);
          buf[next->length] = '\0';
          end_val = atoi(buf);
          if (is_neg_end)
            end_val = -end_val;

          /* Determine context: case or range init */
          {
            size_t k;
            for (k = i - 1; k-- > 0;) {
              if (tree->base_tokens->tokens[k].length == 4 &&
                  memcmp(tree->base_tokens->tokens[k].start, "case", 4) == 0) {
                is_case = 1;
                is_case_idx = k;
                break;
              } else if (tree->base_tokens->tokens[k].kind ==
                         CDD_TOKEN_LBRACKET) {
                is_range_init = 1;
                is_range_idx = k;
                break;
              }
            }
          }

          if (is_case) {
            char *heap_buf;
            size_t alloc_sz;
            char *p;
            int v;
            const char *pooled;
            if (start_val > end_val) {
              alloc_sz = 128;
            } else {
              alloc_sz = ((size_t)(end_val - start_val + 2)) * 32 + 128;
            }
            rc = gnu_malloc(alloc_sz, (void **)&heap_buf);
            if (rc != CDD_C_SUCCESS)
              return rc;
            p = heap_buf;
            /* If overlapping case, add a warning comment */
            /* For now we just generate all cases. Semantic error detection
               of overlaps is usually left to the underlying compiler, but
               we add a warning. */
            if (start_val > end_val) {
#if defined(_MSC_VER)
              strcpy_s(p, alloc_sz - (size_t)(p - heap_buf),
                       "/* WARNING: Invalid case range */ ");
#else
              strcpy(p, "/* WARNING: Invalid case range */ ");
#endif
              p += 34;
            }
            for (v = start_val; v <= end_val; v++) {
#if defined(_MSC_VER)
              strcpy_s(p, alloc_sz - (size_t)(p - heap_buf), "case ");
#else
              strcpy(p, "case ");
#endif
              p += 5;
              if (cdd_append_int(p, v, &p) != 0) {
                free(heap_buf);
                return CDD_C_ERROR_MEMORY;
              }
              if (v != end_val) {
#if defined(_MSC_VER)
                strcpy_s(p, alloc_sz - (size_t)(p - heap_buf), ": ");
#else
                strcpy(p, ": ");
#endif
                p += 2;
              }
            }
            if (is_neg_start) {
              tree->base_tokens->tokens[i - 2].length = 0;
            }
            pooled = pool_string_safe(tree, heap_buf);
            if (!pooled) {
              free(heap_buf);
              return CDD_C_ERROR_MEMORY;
            }
            tree->base_tokens->tokens[is_case_idx].start =
                (const uint8_t *)pooled;
            tree->base_tokens->tokens[is_case_idx].length = strlen(heap_buf);
            free(heap_buf);
            rc = replace_token_with_text(tree, prev, prev->kind, "", 0);
            if (rc != CDD_C_SUCCESS)
              return rc;
            rc = replace_token_with_text(tree, t, t->kind, "", 0);
            if (rc != CDD_C_SUCCESS)
              return rc;
            tree->base_tokens->tokens[i + 1].length = 0;
            tree->base_tokens->tokens[i + 2].length = 0;
            if (is_neg_end) {
              tree->base_tokens->tokens[i + 3].length = 0;
            }
            rc = replace_token_with_text(tree, next, next->kind, "", 0);
            if (rc != CDD_C_SUCCESS)
              return rc;
          } else if (is_range_init && start_val <= end_val) {
            char *heap_buf;
            size_t alloc_sz = 0;
            char *p;
            int v;
            /* Extract the assigned value */
            cdd_token_t *assign_val = NULL;
            if (next + 3 <=
                    &tree->base_tokens
                         ->tokens[(unsigned long)tree->base_tokens->size - 1] &&
                (next + 1)->kind == CDD_TOKEN_RBRACKET &&
                (next + 2)->kind == CDD_TOKEN_ASSIGN) {
              assign_val = next + 3;
            }

            if (assign_val) {
              const char *pooled;
              alloc_sz = ((size_t)(end_val - start_val + 2)) *
                             (32 + assign_val->length) +
                         1;
              rc = gnu_malloc(alloc_sz, (void **)&heap_buf);
              if (rc != CDD_C_SUCCESS)
                return rc;
              p = heap_buf;
              for (v = start_val; v <= end_val; v++) {
                *p++ = '[';
                if (cdd_append_int(p, v, &p) != 0) {
                  free(heap_buf);
                  return CDD_C_ERROR_MEMORY;
                }
#if defined(_MSC_VER)
                strcpy_s(p, alloc_sz - (size_t)(p - heap_buf), "] = ");
#else
                strcpy(p, "] = ");
#endif
                p += 4;
                memcpy(p, assign_val->start, assign_val->length);
                p += assign_val->length;
                if (v != end_val) {
#if defined(_MSC_VER)
                  strcpy_s(p, alloc_sz - (size_t)(p - heap_buf), ", ");
#else
                  strcpy(p, ", ");
#endif
                  p += 2;
                }
              }
              *p = '\0';

              pooled = pool_string_safe(tree, heap_buf);
              if (!pooled) {
                free(heap_buf);
                return CDD_C_ERROR_MEMORY;
              }
              tree->base_tokens->tokens[is_range_idx].start =
                  (const uint8_t *)pooled;
              tree->base_tokens->tokens[is_range_idx].length = strlen(heap_buf);
              free(heap_buf);

              rc = replace_token_with_text(tree, prev, prev->kind, "", 0);
              if (rc != CDD_C_SUCCESS)
                return rc;
              rc = replace_token_with_text(tree, t, t->kind, "", 0);
              if (rc != CDD_C_SUCCESS)
                return rc;
              rc = replace_token_with_text(
                  tree, &tree->base_tokens->tokens[i + 1],
                  tree->base_tokens->tokens[i + 1].kind, "", 0);
              if (rc != CDD_C_SUCCESS)
                return rc;
              rc = replace_token_with_text(
                  tree, &tree->base_tokens->tokens[i + 2],
                  tree->base_tokens->tokens[i + 2].kind, "", 0);
              if (rc != CDD_C_SUCCESS)
                return rc;
              rc = replace_token_with_text(tree, next, next->kind, "", 0);
              if (rc != CDD_C_SUCCESS)
                return rc;
              rc = replace_token_with_text(tree, next + 1, (next + 1)->kind, "",
                                           0);
              if (rc != CDD_C_SUCCESS)
                return rc;
              rc = replace_token_with_text(tree, next + 2, (next + 2)->kind, "",
                                           0);
              if (rc != CDD_C_SUCCESS)
                return rc;
              rc = replace_token_with_text(tree, assign_val, assign_val->kind,
                                           "", 0);
              if (rc != CDD_C_SUCCESS)
                return rc;
            }
          }
        }
      }
    }

    /* builtin expect */
    if (t->kind == CDD_TOKEN_IDENTIFIER && t->length == 16 &&
        memcmp(t->start, "__builtin_expect", 16) == 0) {
      size_t j = i + 1;
#ifdef CDD_BUILD_TESTS
      if (g_gnu_standardizer_fail == 18) {
        tree->base_tokens->tokens[j].length = 0;
      }
#endif
      while (j < tree->base_tokens->size &&
             tree->base_tokens->tokens[j].length == 0) {
        j++;
      }
      if (j < tree->base_tokens->size) {
        if (tree->base_tokens->tokens[j].kind == CDD_TOKEN_LPAREN) {
          int depth = 1;
          size_t comma_idx = 0;
          size_t k;
          for (k = j + 1; k < tree->base_tokens->size; k++) {
            if (tree->base_tokens->tokens[k].kind == CDD_TOKEN_LPAREN) {
              depth++;
            } else if (tree->base_tokens->tokens[k].kind == CDD_TOKEN_RPAREN) {
              depth--;
              if (depth == 0) {
                if (comma_idx > 0) {
                  t->length = 0;
                  {
                    size_t wipe;
                    for (wipe = comma_idx; wipe < k; wipe++) {
                      tree->base_tokens->tokens[wipe].length = 0;
                    }
                  }
                }
                break;
              }
            } else if (tree->base_tokens->tokens[k].kind == CDD_TOKEN_COMMA &&
                       depth == 1) {
              comma_idx = k;
            }
          }
        }
      }
    }
  }
  return CDD_C_SUCCESS;
}
