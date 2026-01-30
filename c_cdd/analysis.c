/**
 * @file analysis.c
 * @brief Implementation of allocation analysis logic.
 * @author Samuel Marks
 */

#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <c_str_span.h>

#if defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__)
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#define strdup _strdup
#endif
#else
#include <sys/errno.h>
#endif

#include "analysis.h"

int allocation_site_list_init(struct AllocationSiteList *const list) {
  if (!list)
    return EINVAL;
  list->size = 0;
  list->capacity = 8;
  list->sites = (struct AllocationSite *)malloc(list->capacity *
                                                sizeof(struct AllocationSite));
  if (!list->sites)
    return ENOMEM;
  return 0;
}

void allocation_site_list_free(struct AllocationSiteList *const list) {
  size_t i;
  if (!list)
    return;
  if (list->sites) {
    for (i = 0; i < list->size; ++i) {
      free(list->sites[i].var_name);
    }
    free(list->sites);
    list->sites = NULL;
  }
  list->size = 0;
  list->capacity = 0;
}

/**
 * @brief Add a site to the list, resizing if necessary.
 */
static int allocation_site_list_add(struct AllocationSiteList *const list,
                                    const size_t index, const char *var_name,
                                    const int checked) {
  if (list->size >= list->capacity) {
    const size_t new_cap = list->capacity * 2;
    struct AllocationSite *new_sites = (struct AllocationSite *)realloc(
        list->sites, new_cap * sizeof(struct AllocationSite));
    if (!new_sites)
      return ENOMEM;
    list->sites = new_sites;
    list->capacity = new_cap;
  }
  list->sites[list->size].token_index = index;
  list->sites[list->size].is_checked = checked;
  list->sites[list->size].var_name = strdup(var_name);
  if (!list->sites[list->size].var_name)
    return ENOMEM;
  list->size++;
  return 0;
}

/**
 * @brief Helper to compare a token's content with a string string.
 */
static int token_equals(const struct Token *tok, const char *str) {
  const size_t len = strlen(str);
  if (tok->length != len)
    return 0;
  return strncmp((const char *)tok->start, str, len) == 0;
}

/**
 * @brief Identify the variable identifier being assigned to before the '='.
 * Searches backwards from `assign_index`.
 */
static char *get_assigned_var(const struct TokenList *tokens,
                              size_t assign_index) {
  size_t i = assign_index;
  if (assign_index == 0)
    return NULL;
  i--;

  /* Skip trailing whitespace before the = */
  while (i > 0 && tokens->tokens[i].kind == TOKEN_WHITESPACE)
    i--;

  /* Handle simple case: `var = ...` or `Type *var = ...` */
  /* The token at `i` should be the identifier. */
  if (tokens->tokens[i].kind == TOKEN_IDENTIFIER) {
    const struct Token *tok = &tokens->tokens[i];
    char *name = (char *)malloc(tok->length + 1);
    if (!name)
      return NULL;
    memcpy(name, tok->start, tok->length);
    name[tok->length] = '\0';
    return name;
  }

  /* Does not handle complex lvalues like `arr[i] = ...` yet. */
  return NULL;
}

int is_checked(const struct TokenList *const tokens, const size_t alloc_idx,
               const char *const var_name) {
  size_t i = alloc_idx;

  if (!tokens || !var_name)
    return 0;

  /* Skip past the function call arguments to find end of statement roughly.
     Start scanning for checks after the malloc call. */
  while (i < tokens->size) {
    if (tokens->tokens[i].kind == TOKEN_SEMICOLON) {
      i++;
      break;
    }
    i++;
  }

  while (i < tokens->size) {
    const struct Token *tok = &tokens->tokens[i];

    if (tok->kind == TOKEN_KEYWORD_STRUCT || tok->kind == TOKEN_KEYWORD_ENUM ||
        tok->kind == TOKEN_KEYWORD_UNION) {
      /* type keywords are safe */
    } else if (tok->kind == TOKEN_WHITESPACE || tok->kind == TOKEN_COMMENT) {
      /* skip */
    } else if (token_equals(tok, "if") || token_equals(tok, "while")) {
      /* Check condition */
      size_t j = i + 1;
      while (j < tokens->size && tokens->tokens[j].kind == TOKEN_WHITESPACE)
        j++;

      if (j < tokens->size && tokens->tokens[j].kind == TOKEN_LPAREN) {
        int depth = 1;
        j++;
        while (j < tokens->size && depth > 0) {
          if (tokens->tokens[j].kind == TOKEN_IDENTIFIER) {
            /* If we see the variable usage inside an if/while condition,
               we assume it's being checked. */
            if (token_equals(&tokens->tokens[j], var_name)) {
              return 1;
            }
          } else if (tokens->tokens[j].kind == TOKEN_LPAREN) {
            depth++;
          } else if (tokens->tokens[j].kind == TOKEN_RPAREN) {
            depth--;
          }
          j++;
        }
      }
      /* If if-block ends without finding var, continue searching. */
    } else if (tok->kind == TOKEN_IDENTIFIER) {
      /* If we find the variable being used (and not in a check) */
      if (token_equals(tok, var_name)) {
        /* Check if it's a dereference or property access */
        /* Scan forward to next non-whitespace */
        size_t next = i + 1;
        while (next < tokens->size &&
               tokens->tokens[next].kind == TOKEN_WHITESPACE)
          next++;

        if (next < tokens->size) {
          const struct Token *next_tok = &tokens->tokens[next];
          /* usage: var->field, var[0], *var (preceding * handled differently)
           */
          /* check for -> or [ */
          if ((next_tok->length == 2 &&
               strncmp((char *)next_tok->start, "->", 2) == 0) ||
              next_tok->kind ==
                  TOKEN_LBRACE /* array subscript often [ but tokenizer might
                                  call it TOKEN_OTHER or specialized */
              /* The tokenizer has TOKEN_OTHER for [ currently, let's assume
                 dereference via -> implies unchecked use. */
          ) {
            return 0;
          }
          /* Check simple assignment `var = ...` which overwrites safely? */
          if (next_tok->kind == TOKEN_OTHER && next_tok->length == 1 &&
              *next_tok->start == '=') {
            /* Reassigned without check -> effectively unchecked allocation
               (memory leak likely), but for null-safety it matches "unchecked".
             */
            return 0;
          }
        }

        /* Look behind for dereference `*var` */
        {
          size_t prev = i;
          while (prev > alloc_idx &&
                 tokens->tokens[prev - 1].kind == TOKEN_WHITESPACE)
            prev--;
          if (prev > alloc_idx) {
            const struct Token *prev_tok = &tokens->tokens[prev - 1];
            if (prev_tok->kind == TOKEN_OTHER && prev_tok->length == 1 &&
                *prev_tok->start == '*') {
              return 0; /* Dereference detected before check */
            }
          }
        }
      }
    } else if (tok->kind == TOKEN_RBRACE) {
      /* End of scope/function. If we haven't found a check yet, it's unchecked.
       */
      return 0;
    }

    i++;
  }

  return 0;
}

int find_allocations(const struct TokenList *const tokens,
                     struct AllocationSiteList *const out) {
  size_t i;
  int rc;

  if (!tokens || !out)
    return EINVAL;

  /* Re-initialize output to be safe */
  if (out->sites == NULL) {
    rc = allocation_site_list_init(out);
    if (rc != 0)
      return rc;
  }

  for (i = 0; i < tokens->size; ++i) {
    const struct Token *tok = &tokens->tokens[i];

    if (tok->kind == TOKEN_IDENTIFIER) {
      if (token_equals(tok, "malloc") || token_equals(tok, "realloc") ||
          token_equals(tok, "calloc") || token_equals(tok, "strdup")) {

        /* Look for preceding assignment '=' */
        size_t j = i;
        while (j > 0) {
          j--;
          if (tokens->tokens[j].kind == TOKEN_WHITESPACE)
            continue;
          if (tokens->tokens[j].kind == TOKEN_SEMICOLON ||
              tokens->tokens[j].kind == TOKEN_LBRACE ||
              tokens->tokens[j].kind == TOKEN_RBRACE) {
            /* Hit boundary without finding assignment */
            break;
          }
          if (tokens->tokens[j].kind == TOKEN_OTHER &&
              tokens->tokens[j].length == 1 &&
              *tokens->tokens[j].start == '=') {
            /* Found assignment. Now find variable name. */
            char *var_name = get_assigned_var(tokens, j);
            if (var_name) {
              int checked = is_checked(tokens, i, var_name);
              rc = allocation_site_list_add(out, i, var_name, checked);
              free(var_name);
              if (rc != 0)
                return rc;
            }
            break;
          }
        }
      }
    }
  }

  return 0;
}
