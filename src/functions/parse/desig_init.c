/**
 * @file desig_init.c
 * @brief Implementation of the C99 designated initializer scanner.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "c_cdd/memory.h"
#include "c_cdd_export.h"
#include "functions/parse/desig_init.h"
/* clang-format on */

/**
 * @brief Duplicates a string up to a specified number of characters.
 *
 * @param[in] s Source string
 * @param[in] n Maximum characters to copy
 * @param[out] _out_val Pointer to receive allocated string
 * @return CDD_C_SUCCESS on success, error enum on failure
 */
static cdd_c_error_t c_cdd_strndup(const char *s, size_t n, char **_out_val) {
  char *d = NULL;
  if (!s || !_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  d = (char *)(size_t)C_CDD_MALLOC(n + 1);
  if (!d) {
    *_out_val = NULL;
    return CDD_C_ERROR_MEMORY;
  }
  memcpy(d, s, n);
  d[n] = '\0';
  *_out_val = d;
  return CDD_C_SUCCESS;
}

/**
 * @brief Initializes a designated initializer list.
 *
 * @param[out] list Designated initializer list to initialize
 * @return CDD_C_SUCCESS on success, error enum on failure
 */
cdd_c_error_t desig_init_list_init(struct DesigInitList *list) {
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
 * @param[in,out] list Designated initializer list to release
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
  desig_init_list_init(list);
}

/**
 * @brief Executes the scan for designated initializers operation.
 *
 * @param[in] tokens Token list to scan
 * @param[out] list Designated initializer list to populate
 * @return CDD_C_SUCCESS on success, error enum on failure
 */
cdd_c_error_t scan_for_designated_initializers(const struct TokenList *tokens,
                                               struct DesigInitList *list) {
  size_t i = 0;
  size_t *brace_stack = NULL;
  size_t brace_depth = 0;
  size_t brace_cap = 0;
  cdd_c_error_t res = CDD_C_SUCCESS;

  if (!tokens || !list)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  while (i < tokens->size) {
    if (tokens->tokens[i].kind == TOKEN_LBRACE) {
      if (brace_depth >= brace_cap) {
        size_t *new_stack;
        brace_cap = brace_cap == 0 ? 16 : brace_cap * 2;
        new_stack =
            (size_t *)C_CDD_REALLOC(brace_stack, brace_cap * sizeof(size_t));
        if (!new_stack) {
          res = CDD_C_ERROR_MEMORY;
          goto cleanup;
        }
        brace_stack = new_stack;
      }
      brace_stack[brace_depth++] = i;
    } else if (tokens->tokens[i].kind == TOKEN_RBRACE) {
      if (brace_depth > 0) {
        brace_depth--;
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
            struct DesigInitSite *new_sites;
            size_t new_cap = list->capacity == 0 ? 4 : list->capacity * 2;
            new_sites = (struct DesigInitSite *)C_CDD_REALLOC(
                list->sites, new_cap * sizeof(struct DesigInitSite));
            if (!new_sites) {
              res = CDD_C_ERROR_MEMORY;
              goto cleanup;
            }
            list->sites = new_sites;
            list->capacity = new_cap;
          }

          {
            struct DesigInitSite *s = &list->sites[list->count];
            s->start_token_idx = dot_idx;
            s->end_token_idx = expr_end;
            s->brace_start_idx = brace_stack[brace_depth - 1];
            s->brace_end_idx = 0;
            s->field_name = NULL;
            s->value_expr = NULL;

            res =
                c_cdd_strndup((const char *)tokens->tokens[ident_idx].start,
                              tokens->tokens[ident_idx].length, &s->field_name);
            if (res != CDD_C_SUCCESS)
              goto cleanup;

            if (expr_end > expr_start) {
              const char *e_start =
                  (const char *)tokens->tokens[expr_start].start;
              const char *e_end =
                  (const char *)tokens->tokens[expr_end - 1].start +
                  tokens->tokens[expr_end - 1].length;
              res = c_cdd_strndup(e_start, (size_t)(e_end - e_start),
                                  &s->value_expr);
              if (res != CDD_C_SUCCESS) {
                free(s->field_name);
                s->field_name = NULL;
                goto cleanup;
              }
            } else {
              res = c_cdd_strndup("", 0, &s->value_expr);
              if (res != CDD_C_SUCCESS) {
                free(s->field_name);
                s->field_name = NULL;
                goto cleanup;
              }
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

#ifdef CDD_BUILD_TESTS
/**
 * @brief Helper for testing internal error conditions.
 *
 * @return CDD_C_SUCCESS on success, error enum on failure
 */
C_CDD_EXPORT cdd_c_error_t test_desig_init_internal_errors(void);
C_CDD_EXPORT cdd_c_error_t test_desig_init_internal_errors(void) {
  char *out = NULL;
  cdd_c_error_t err1 = c_cdd_strndup(NULL, 0, &out);
  cdd_c_error_t err2 = c_cdd_strndup("a", 1, NULL);
  return (cdd_c_error_t)((err1 ^ CDD_C_ERROR_INVALID_ARGUMENT) |
                         (err2 ^ CDD_C_ERROR_INVALID_ARGUMENT));
}
#endif
