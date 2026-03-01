/**
 * @file analysis.c
 * @brief Implementation of static safety analysis.
 * @author Samuel Marks
 */

#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "c_cdd_stdbool.h"
#include "functions/parse/analysis.h"
#include "functions/parse/str.h"

static const struct AllocatorSpec ALLOCATOR_SPECS[] = {
    {"malloc", ALLOC_STYLE_RETURN_PTR, CHECK_PTR_NULL, 0},
    {"calloc", ALLOC_STYLE_RETURN_PTR, CHECK_PTR_NULL, 0},
    {"realloc", ALLOC_STYLE_RETURN_PTR, CHECK_PTR_NULL, 0},
    {"strdup", ALLOC_STYLE_RETURN_PTR, CHECK_PTR_NULL, 0},
    {"_strdup", ALLOC_STYLE_RETURN_PTR, CHECK_PTR_NULL, 0},
    {"strndup", ALLOC_STYLE_RETURN_PTR, CHECK_PTR_NULL, 0},
    {"asprintf", ALLOC_STYLE_ARG_PTR, CHECK_INT_NEGATIVE, 0},
    {"vasprintf", ALLOC_STYLE_ARG_PTR, CHECK_INT_NEGATIVE, 0},
    {"_mkdir", ALLOC_STYLE_RETURN_PTR, CHECK_INT_NONZERO, 0},
    {NULL, ALLOC_STYLE_RETURN_PTR, CHECK_PTR_NULL, 0}};

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
      if (list->sites[i].var_name)
        free(list->sites[i].var_name);
    }
    free(list->sites);
    list->sites = NULL;
  }
  list->size = 0;
  list->capacity = 0;
}

int allocation_site_list_add(struct AllocationSiteList *const list,
                             const size_t index, const char *var_name,
                             const int checked, const int used_before,
                             const int is_ret,
                             const struct AllocatorSpec *spec) {
  if (!list)
    return EINVAL;

  if (list->size >= list->capacity) {
    const size_t new_cap = (list->capacity == 0) ? 8 : list->capacity * 2;
    struct AllocationSite *new_sites = (struct AllocationSite *)realloc(
        list->sites, new_cap * sizeof(struct AllocationSite));
    if (!new_sites)
      return ENOMEM;
    list->sites = new_sites;
    list->capacity = new_cap;
  }

  list->sites[list->size].token_index = index;
  list->sites[list->size].is_checked = checked;
  list->sites[list->size].used_before_check = used_before;
  list->sites[list->size].is_return_stmt = is_ret;
  list->sites[list->size].spec = spec;

  if (var_name) {
    list->sites[list->size].var_name = c_cdd_strdup(var_name);
    if (!list->sites[list->size].var_name)
      return ENOMEM;
  } else {
    list->sites[list->size].var_name = NULL;
  }

  list->size++;
  return 0;
}

static char *get_assigned_var(const struct TokenList *tokens,
                              size_t assign_idx) {
  size_t i = assign_idx;
  if (assign_idx == 0)
    return NULL;
  i--;

  while (i > 0 && tokens->tokens[i].kind == TOKEN_WHITESPACE)
    i--;

  if (tokens->tokens[i].kind == TOKEN_IDENTIFIER) {
    const struct Token *tok = &tokens->tokens[i];
    char *name = (char *)malloc(tok->length + 1);
    if (!name)
      return NULL;
    memcpy(name, tok->start, tok->length);
    name[tok->length] = '\0';
    return name;
  }
  return NULL;
}

static int is_inside_condition(const struct TokenList *tokens, size_t idx) {
  size_t i = idx;
  int paren_depth = 0;

  while (i > 0) {
    i--;
    if (tokens->tokens[i].kind == TOKEN_RPAREN) {
      paren_depth++;
    } else if (tokens->tokens[i].kind == TOKEN_LPAREN) {
      if (paren_depth > 0) {
        paren_depth--;
      } else {
        size_t prev = i;
        while (prev > 0) {
          prev--;
          if (tokens->tokens[prev].kind == TOKEN_WHITESPACE)
            continue;

          /* Check for KEYWORDS if/while */
          if (tokens->tokens[prev].kind == TOKEN_KEYWORD_IF ||
              tokens->tokens[prev].kind == TOKEN_KEYWORD_WHILE) {
            return 1;
          }
          break;
        }
      }
    } else if (tokens->tokens[i].kind == TOKEN_SEMICOLON ||
               tokens->tokens[i].kind == TOKEN_LBRACE ||
               tokens->tokens[i].kind == TOKEN_RBRACE) {
      break;
    }
  }
  return 0;
}

static int is_dereference_use(const struct TokenList *tokens, size_t i) {
  if (i > 0) {
    size_t prev = i - 1;
    while (prev > 0 && tokens->tokens[prev].kind == TOKEN_WHITESPACE)
      prev--;
    if (tokens->tokens[prev].kind == TOKEN_STAR) {
      return 1;
    }
  }
  {
    size_t next = i + 1;
    while (next < tokens->size && tokens->tokens[next].kind == TOKEN_WHITESPACE)
      next++;
    if (next < tokens->size) {
      const struct Token *t = &tokens->tokens[next];
      if (t->kind == TOKEN_ARROW || t->kind == TOKEN_LBRACKET) {
        return 1;
      }
    }
  }
  return 0;
}

int is_checked(const struct TokenList *const tokens, const size_t alloc_idx,
               const char *const var_name, const struct AllocatorSpec *spec,
               int *used_before_check) {
  size_t i = alloc_idx;
  *used_before_check = 0;

  if (!tokens || !var_name)
    return 0;

  if (is_inside_condition(tokens, alloc_idx))
    return 1;

  while (i < tokens->size && tokens->tokens[i].kind != TOKEN_SEMICOLON)
    i++;
  if (i < tokens->size)
    i++;

  while (i < tokens->size) {
    const struct Token *tok = &tokens->tokens[i];
    if (tok->kind == TOKEN_RBRACE || tok->kind == TOKEN_KEYWORD_STRUCT) {
      return 0;
    }

    if (tok->kind == TOKEN_IDENTIFIER) {
      if (token_matches_string(tok, var_name)) {
        if (is_inside_condition(tokens, i))
          return 1;
        if (spec->check_style == CHECK_PTR_NULL &&
            is_dereference_use(tokens, i)) {
          *used_before_check = 1;
          return 0;
        }
      }
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

  if (out->sites == NULL) {
    if ((rc = allocation_site_list_init(out)) != 0)
      return rc;
  }

  for (i = 0; i < tokens->size; ++i) {
    const struct Token *tok = &tokens->tokens[i];

    if (tok->kind == TOKEN_IDENTIFIER) {
      const struct AllocatorSpec *spec = ALLOCATOR_SPECS;

      while (spec->name != NULL) {
        if (token_matches_string(tok, spec->name)) {
          char *var_name = NULL;
          int is_return = 0;

          {
            size_t prev = i;
            while (prev > 0) {
              prev--;
              if (tokens->tokens[prev].kind == TOKEN_WHITESPACE)
                continue;

              if (tokens->tokens[prev].kind == TOKEN_KEYWORD_RETURN) {
                is_return = 1;
              }
              break;
            }
          }

          if (is_return) {
            rc = allocation_site_list_add(out, i, NULL, 0, 0, 1, spec);
            if (rc != 0)
              return rc;
            break;
          }

          {
            size_t prev = i;
            while (prev > 0) {
              prev--;
              if (tokens->tokens[prev].kind == TOKEN_WHITESPACE)
                continue;
              if (tokens->tokens[prev].kind == TOKEN_SEMICOLON ||
                  tokens->tokens[prev].kind == TOKEN_LBRACE ||
                  tokens->tokens[prev].kind == TOKEN_RBRACE) {
                break;
              }
              if (tokens->tokens[prev].kind == TOKEN_ASSIGN) {
                var_name = get_assigned_var(tokens, prev);
                break;
              }
            }
          }

          if (var_name) {
            int used_before = 0;
            int checked = is_checked(tokens, i, var_name, spec, &used_before);
            rc = allocation_site_list_add(out, i, var_name, checked,
                                          used_before, 0, spec);
            free(var_name);
            if (rc != 0)
              return rc;
          } else {
            int checked = is_inside_condition(tokens, i);
            rc = allocation_site_list_add(out, i, NULL, checked, 0, 0, spec);
            if (rc != 0)
              return rc;
          }
          break;
        }
        spec++;
      }
    }
  }
  return 0;
}
