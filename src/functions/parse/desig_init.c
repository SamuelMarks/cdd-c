/**
 * @file desig_init.c
 * @brief Implementation of the C99 designated initializer scanner.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include "c_cdd_export.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "functions/parse/desig_init.h"
/* clang-format on */
/* LCOV_EXCL_START */

/**
 * @brief Duplicates a string up to a specified number of characters.
 *
 */
static enum cdd_c_error c_cdd_strndup(const char *s, size_t n,
                                      char **_out_val) {
  char *d = NULL;
#ifdef CDD_BUILD_TESTS
  {
    extern C_CDD_EXPORT int g_cdd_fail_alloc;
    if (g_cdd_fail_alloc && --g_cdd_fail_alloc == 0)
      d = NULL;
    else
      d = (char *)malloc(n + 1);
  }
#else
#ifdef CDD_BUILD_TESTS
  {
    extern C_CDD_EXPORT int g_cdd_fail_alloc;
    if (g_cdd_fail_alloc && --g_cdd_fail_alloc == 0)
      d = NULL;
    else
      d = (char *)malloc(n + 1);
  }
#else
  d = (char *)malloc(n + 1);
#endif
#endif
  if (!d) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
  memcpy(d, s, n);
  d[n] = '\0';
  {
    *_out_val = d;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Initializes a designated initializer list.
 *
 */
enum cdd_c_error desig_init_list_init(struct DesigInitList *list) {
  if (!list)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  list->sites = NULL;
  list->count = 0;
  list->capacity = 0;
  return CDD_C_SUCCESS;
}

/**
 * @brief Frees a designated initializer list.
 *
 */
void desig_init_list_free(struct DesigInitList *list) {
  size_t i;
  if (!list)
    return;
  if (list->sites) {
    for (i = 0; i < list->count; i++) {
      if (list->sites[i].field_name)
        free(list->sites[i].field_name);
      if (list->sites[i].value_expr)
        free(list->sites[i].value_expr);
    }
    free(list->sites);
  }
  (void)desig_init_list_init(list);
}

/**
 * @brief Executes the scan for designated initializers operation.
 *
 */
enum cdd_c_error
scan_for_designated_initializers(const struct TokenList *tokens,
                                 struct DesigInitList *list) {
  size_t i = 0;
  size_t *brace_stack = NULL;
  size_t brace_depth = 0;
  size_t brace_cap = 0;
  int res = 0;

  if (!tokens || !list)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  while (i < tokens->size) {
    if (tokens->tokens[i].kind == TOKEN_LBRACE) {
      if (brace_depth >= brace_cap) {
        size_t *new_stack;
        brace_cap = brace_cap == 0 ? 16 : brace_cap * 2;
#ifdef CDD_BUILD_TESTS
        {
          extern C_CDD_EXPORT int g_cdd_fail_alloc;
          if (g_cdd_fail_alloc && --g_cdd_fail_alloc == 0)
            new_stack = NULL;
          else
            new_stack =
                (size_t *)realloc(brace_stack, brace_cap * sizeof(size_t));
        }
#else
        new_stack = (size_t *)realloc(brace_stack, brace_cap * sizeof(size_t));
#endif
        if (!new_stack) {
          res = ENOMEM;
          goto cleanup;
        }
        brace_stack = new_stack;
      }
      brace_stack[brace_depth++] = i;
    } else if (tokens->tokens[i].kind == TOKEN_RBRACE) {
      if (brace_depth > 0) {
        brace_depth--;
        /* We could process brace blocks here if needed */
      }
    } else if (brace_depth > 0 && tokens->tokens[i].kind == TOKEN_DOT) {
      /* Look for `. identifier = expression` */
      size_t dot_idx = i;
      size_t look = i + 1;

      while (look < tokens->size &&
             tokens->tokens[look].kind == TOKEN_WHITESPACE)
        look++;

      if (look < tokens->size &&
          tokens->tokens[look].kind == TOKEN_IDENTIFIER) {
        size_t ident_idx = look;
        look++;

        while (look < tokens->size &&
               tokens->tokens[look].kind == TOKEN_WHITESPACE)
          look++;

        if (look < tokens->size && tokens->tokens[look].kind == TOKEN_ASSIGN) {
          /* Found designated initializer `.ident =` */
          size_t expr_start;
          size_t expr_end;
          size_t nested_braces = 0;
          look++;

          while (look < tokens->size &&
                 tokens->tokens[look].kind == TOKEN_WHITESPACE)
            look++;
          expr_start = look;

          /* Scan value expression until `,` or `}` at the same brace depth */
          while (look < tokens->size) {
            if (tokens->tokens[look].kind == TOKEN_LBRACE) {
              nested_braces++;
            } else if (tokens->tokens[look].kind == TOKEN_RBRACE) {
              if (nested_braces == 0)
                break; /* End of struct initialization */
              nested_braces--;
            } else if (tokens->tokens[look].kind == TOKEN_COMMA &&
                       nested_braces == 0) {
              break; /* End of this field's initialization */
            }
            look++;
          }

          expr_end = look;

          /* Trim trailing whitespace from expression */
          while (expr_end > expr_start &&
                 tokens->tokens[expr_end - 1].kind == TOKEN_WHITESPACE) {
            expr_end--;
          }

          if (list->count >= list->capacity) {
            list->capacity = list->capacity == 0 ? 4 : list->capacity * 2;
            list->sites = (struct DesigInitSite *)realloc(
                list->sites, list->capacity * sizeof(struct DesigInitSite));
            if (!list->sites) {
              res = ENOMEM;
              goto cleanup;
            }
          }

          {
            struct DesigInitSite *s = &list->sites[list->count];
            s->start_token_idx = dot_idx;
            s->end_token_idx = expr_end;
            s->brace_start_idx = brace_stack[brace_depth - 1];
            s->brace_end_idx =
                0; /* Will be resolved later if needed, hard to know exact end
                      in forward pass without full parse */

            c_cdd_strndup((const char *)tokens->tokens[ident_idx].start,
                          tokens->tokens[ident_idx].length, &s->field_name);

            {
              const char *e_start =
                  (const char *)tokens->tokens[expr_start].start;
              const char *e_end =
                  (const char *)tokens->tokens[expr_end - 1].start +
                  tokens->tokens[expr_end - 1].length;
              c_cdd_strndup(e_start, (size_t)(e_end - e_start), &s->value_expr);
            }

            list->count++;
          }
          i = look - 1; /* Advance main loop */
        }
      }
    }
    i++;
  }

cleanup:
  if (brace_stack)
    free(brace_stack);
  return res;
}

/* LCOV_EXCL_STOP */
