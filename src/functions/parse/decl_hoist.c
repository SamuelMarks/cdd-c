/**
 * @file decl_hoist.c
 * @brief Implementation of the declaration hoisting scanner.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "functions/parse/decl_hoist.h"
/* clang-format on */

#ifdef CDD_BUILD_TESTS
C_CDD_EXPORT int g_cdd_fail_alloc_decl_hoist = 0;
#endif

/**
 * @brief Initializes a hoist site list.
 *
 */
cdd_c_error_t hoist_site_list_init(struct HoistSiteList *list) {
  if (list) {
    list->sites = NULL;
    list->count = 0;
    list->capacity = 0;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Frees a hoist site list.
 *
 */
void hoist_site_list_free(struct HoistSiteList *list) {
  if (!list)
    return;
  if (list->sites) {
    free(list->sites);
  }
  (void)hoist_site_list_init(list);
}

/**
 * @brief Checks if a token kind is a basic type keyword.
 *
 */
static cdd_c_error_t is_basic_type_keyword(enum TokenKind k,
                                           int *out_is_basic) {
  *out_is_basic = 0;
  switch (k) {
  case TOKEN_KEYWORD_INT:
  case TOKEN_KEYWORD_CHAR:
  case TOKEN_KEYWORD_SHORT:
  case TOKEN_KEYWORD_LONG:
  case TOKEN_KEYWORD_FLOAT:
  case TOKEN_KEYWORD_DOUBLE:
  case TOKEN_KEYWORD_SIGNED:
  case TOKEN_KEYWORD_UNSIGNED:
  case TOKEN_KEYWORD_VOID:
    *out_is_basic = 1;
    return CDD_C_SUCCESS;
  default:
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Scans for mixed declarations in a token list.
 *
 * Mixed declarations are variable declarations that occur after non-declaration
 * statements within the same block (not strictly conforming to C89/C90).
 *
 */
cdd_c_error_t scan_for_mixed_declarations(const struct TokenList *tokens,
                                          struct HoistSiteList *list) {
  size_t i = 0;
  size_t current_block_start = 0;
  int has_seen_statement_in_block = 0;
  size_t depth = 0;

  if (!tokens || !list)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  while (i < tokens->size) {
    size_t stmt_start = i;
    int is_decl = 0;

    /* Skip leading whitespace for statement */
    while (i < tokens->size && tokens->tokens[i].kind == TOKEN_WHITESPACE) {
      i++;
    }
    if (i >= tokens->size)
      break;
    stmt_start = i;

    if (tokens->tokens[i].kind == TOKEN_LBRACE) {
      depth++;
      current_block_start = i;
      has_seen_statement_in_block = 0;
      i++;
      continue;
    } else if (tokens->tokens[i].kind == TOKEN_RBRACE) {
      if (depth > 0)
        depth--;
      i++;
      continue;
    }

    /* Are we inside a block? */
    if (depth > 0) {
      /* Identify if this statement is a declaration */
      int is_basic = 0;
      is_basic_type_keyword(tokens->tokens[i].kind, &is_basic);
      if (is_basic) {
        is_decl = 1;
      } else if (tokens->tokens[i].kind == TOKEN_KEYWORD_STRUCT) {
        is_decl = 1;
      } else if (tokens->tokens[i].kind == TOKEN_KEYWORD_UNION) {
        is_decl = 1;
      } else if (tokens->tokens[i].kind == TOKEN_KEYWORD_ENUM) {
        is_decl = 1;
      } else if (tokens->tokens[i].kind == TOKEN_IDENTIFIER) {
        /* Typedef assumption */
        size_t look = i + 1;
        for (;;) {
          if (look >= tokens->size)
            break;
          if (tokens->tokens[look].kind != TOKEN_WHITESPACE)
            break;
          look++;
        }
        if (look < tokens->size) {
          if (tokens->tokens[look].kind == TOKEN_IDENTIFIER) {
            is_decl = 1;
          } else if (tokens->tokens[look].kind == TOKEN_STAR) {
            is_decl = 1;
          }
        }
      }

      /* Scan to end of statement (semicolon) */
      for (;;) {
        if (i >= tokens->size)
          break;
        if (tokens->tokens[i].kind == TOKEN_SEMICOLON)
          break;
        if (tokens->tokens[i].kind == TOKEN_LBRACE)
          break;
        if (tokens->tokens[i].kind == TOKEN_RBRACE)
          break;
        i++;
      }

      if (i < tokens->size && tokens->tokens[i].kind == TOKEN_SEMICOLON) {
        i++; /* Consume semicolon */

        if (is_decl) {
          if (has_seen_statement_in_block) {
            /* We found a mixed declaration! */
            if (list->count >= list->capacity) {
              struct HoistSite *new_sites;
              list->capacity = list->capacity == 0 ? 4 : list->capacity * 2;
#ifdef CDD_BUILD_TESTS
              {
                extern C_CDD_EXPORT int g_cdd_fail_alloc_decl_hoist;
                if (g_cdd_fail_alloc_decl_hoist == 1) {
                  new_sites = NULL;
                  g_cdd_fail_alloc_decl_hoist = 0;
                } else {
                  if (g_cdd_fail_alloc_decl_hoist > 1) {
                    g_cdd_fail_alloc_decl_hoist--;
                  }
                  new_sites = (struct HoistSite *)realloc(
                      list->sites, list->capacity * sizeof(struct HoistSite));
                }
              }
#else
              new_sites = (struct HoistSite *)realloc(
                  list->sites, list->capacity * sizeof(struct HoistSite));
#endif
              if (!new_sites)
                return CDD_C_ERROR_MEMORY;
              list->sites = new_sites;
            }
            list->sites[list->count].start_token_idx = stmt_start;
            list->sites[list->count].end_token_idx = i;
            list->sites[list->count].target_block_idx = current_block_start;
            list->count++;
          }
        } else {
          has_seen_statement_in_block = 1;
        }
      }
    } else {
      /* Not inside a block, just skip token */
      i++;
    }
  }

  return CDD_C_SUCCESS;
}
