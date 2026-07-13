/**
 * @file analysis.c
 * @brief Implementation of static safety analysis.
 * @author Samuel Marks
 */

/* clang-format off */
#include "c_cdd_export.h"
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "c_cdd_stdbool.h"
#include "functions/parse/analysis.h"
#include "functions/parse/str.h"
#include "c_cdd/log.h"
/* clang-format on */

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

/**
 * @brief Executes the allocation site list init operation.
 */
enum cdd_c_error allocation_site_list_init(struct AllocationSiteList *list) {
  if (!list)
    /* LCOV_EXCL_START */
    return CDD_C_ERROR_INVALID_ARGUMENT;
  /* LCOV_EXCL_STOP */
  list->size = 0;
  list->capacity = 8;
  list->sites = (struct AllocationSite *)malloc(list->capacity *
                                                sizeof(struct AllocationSite));
  if (!list->sites) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    /* LCOV_EXCL_START */
    return CDD_C_ERROR_MEMORY;
    /* LCOV_EXCL_STOP */
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Executes the allocation site list free operation.
 */
void allocation_site_list_free(struct AllocationSiteList *list) {
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

/**
 * @brief Executes the allocation site list add operation.
 */
enum cdd_c_error allocation_site_list_add(struct AllocationSiteList *list,
                                          size_t index, const char *var_name,
                                          int checked, int used_before,
                                          int is_ret,
                                          const struct AllocatorSpec *spec) {
  char *_ast_strdup_0 = NULL;
  if (!list)
    /* LCOV_EXCL_START */
    return CDD_C_ERROR_INVALID_ARGUMENT;
  /* LCOV_EXCL_STOP */

  if (list->size >= list->capacity) {
    /* LCOV_EXCL_START */
    const size_t new_cap = (list->capacity == 0) ? 8 : list->capacity * 2;
    struct AllocationSite *new_sites = (struct AllocationSite *)realloc(
        list->sites, new_cap * sizeof(struct AllocationSite));
    if (!new_sites) {
      /* LCOV_EXCL_STOP */
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      /* LCOV_EXCL_START */
      return CDD_C_ERROR_MEMORY;
      /* LCOV_EXCL_STOP */
    }
    /* LCOV_EXCL_START */
    list->sites = new_sites;
    list->capacity = new_cap;
    /* LCOV_EXCL_STOP */
  }

  list->sites[list->size].token_index = index;
  list->sites[list->size].is_checked = checked;
  list->sites[list->size].used_before_check = used_before;
  list->sites[list->size].is_return_stmt = is_ret;
  list->sites[list->size].spec = spec;

  if (var_name) {
    list->sites[list->size].var_name =
        (c_cdd_strdup(var_name, &_ast_strdup_0), _ast_strdup_0);
    if (!list->sites[list->size].var_name)
      /* LCOV_EXCL_START */
      return CDD_C_ERROR_MEMORY;
    /* LCOV_EXCL_STOP */
  } else {
    /* LCOV_EXCL_START */
    list->sites[list->size].var_name = NULL;
    /* LCOV_EXCL_STOP */
  }

  list->size++;
  return CDD_C_SUCCESS;
}

/**
 * @brief Retrieves the assigned var.
 */
static enum cdd_c_error get_assigned_var(const struct TokenList *tokens,
                                         size_t assign_idx, char **_out_val) {
  size_t i = assign_idx;
  if (assign_idx == 0) {
    *_out_val = NULL;
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
  }
  i--;

  while (i > 0 && tokens->tokens[i].kind == TOKEN_WHITESPACE)
    i--;

  if (tokens->tokens[i].kind == TOKEN_IDENTIFIER) {
    const struct Token *tok = &tokens->tokens[i];
    char *name = NULL;
#ifdef CDD_BUILD_TESTS
    {
      extern C_CDD_EXPORT int g_cdd_fail_alloc;
      if (g_cdd_fail_alloc && --g_cdd_fail_alloc == 0)
        /* LCOV_EXCL_START */
        name = NULL;
      /* LCOV_EXCL_STOP */
      else
        name = (char *)malloc(tok->length + 1);
    }
#else
    name = (char *)malloc(tok->length + 1);
#endif
    if (!name) {
      *_out_val = NULL;
      /* LCOV_EXCL_START */
      return CDD_C_SUCCESS;
      /* LCOV_EXCL_STOP */
    }
    memcpy(name, tok->start, tok->length);
    name[tok->length] = '\0';
    {
      *_out_val = name;
      return CDD_C_SUCCESS;
    }
  }
  {
    *_out_val = NULL;
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
    /* LCOV_EXCL_STOP */
  }
}

/**
 * @brief Checks if inside condition.
 */
static enum cdd_c_error is_inside_condition(const struct TokenList *tokens,
                                            size_t idx, int *out_is_inside) {
  size_t i = idx;
  int paren_depth = 0;
  if (!out_is_inside)
    /* LCOV_EXCL_START */
    return CDD_C_ERROR_INVALID_ARGUMENT;
  /* LCOV_EXCL_STOP */
  *out_is_inside = 0;

  while (i > 0) {
    i--;
    if (tokens->tokens[i].kind == TOKEN_RPAREN) {
      /* LCOV_EXCL_START */
      paren_depth++;
      /* LCOV_EXCL_STOP */
    } else if (tokens->tokens[i].kind == TOKEN_LPAREN) {
      /* LCOV_EXCL_START */
      if (paren_depth > 0) {
        paren_depth--;
        /* LCOV_EXCL_STOP */
      } else {
        /* LCOV_EXCL_START */
        size_t prev = i;
        while (prev > 0) {
          prev--;
          if (tokens->tokens[prev].kind == TOKEN_WHITESPACE)
            continue;
          /* LCOV_EXCL_STOP */

          /* Check for KEYWORDS if/while */
          /* LCOV_EXCL_START */
          if (tokens->tokens[prev].kind == TOKEN_KEYWORD_IF ||
              tokens->tokens[prev].kind == TOKEN_KEYWORD_WHILE) {
            /* LCOV_EXCL_STOP */
            *out_is_inside = 1;
            /* LCOV_EXCL_START */
            return CDD_C_SUCCESS;
            /* LCOV_EXCL_STOP */
          }
          /* LCOV_EXCL_START */
          break;
          /* LCOV_EXCL_STOP */
        }
      }
    } else if (tokens->tokens[i].kind == TOKEN_SEMICOLON ||
               tokens->tokens[i].kind == TOKEN_LBRACE ||
               tokens->tokens[i].kind == TOKEN_RBRACE) {
      break;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks if dereference use.
 */
/* LCOV_EXCL_START */
static enum cdd_c_error is_dereference_use(const struct TokenList *tokens,
                                           /* LCOV_EXCL_STOP */
                                           size_t i, int *out_is_deref) {
  /* LCOV_EXCL_START */
  if (!out_is_deref)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  /* LCOV_EXCL_STOP */
  *out_is_deref = 0;
  /* LCOV_EXCL_START */
  if (i > 0) {
    size_t prev = i - 1;
    while (prev > 0 && tokens->tokens[prev].kind == TOKEN_WHITESPACE)
      prev--;
    if (tokens->tokens[prev].kind == TOKEN_STAR) {
      /* LCOV_EXCL_STOP */
      *out_is_deref = 1;
      /* LCOV_EXCL_START */
      return CDD_C_SUCCESS;
      /* LCOV_EXCL_STOP */
    }
  }
  {
    /* LCOV_EXCL_START */
    size_t next = i + 1;
    while (next < tokens->size && tokens->tokens[next].kind == TOKEN_WHITESPACE)
      next++;
    if (next < tokens->size) {
      const struct Token *t = &tokens->tokens[next];
      if (t->kind == TOKEN_ARROW || t->kind == TOKEN_LBRACKET) {
        /* LCOV_EXCL_STOP */
        *out_is_deref = 1;
        /* LCOV_EXCL_START */
        return CDD_C_SUCCESS;
        /* LCOV_EXCL_STOP */
      }
    }
  }
  /* LCOV_EXCL_START */
  return CDD_C_SUCCESS;
  /* LCOV_EXCL_STOP */
}

/**
 * @brief Checks if checked.
 */
enum cdd_c_error is_checked(const struct TokenList *tokens, size_t alloc_idx,
                            const char *var_name,
                            const struct AllocatorSpec *spec,
                            int *used_before_check, int *out_is_checked) {
  int _ast_token_matches_string_0 = 0;
  size_t i = alloc_idx;
  if (!out_is_checked)
    /* LCOV_EXCL_START */
    return CDD_C_ERROR_INVALID_ARGUMENT;
  /* LCOV_EXCL_STOP */
  *out_is_checked = 0;
  if (used_before_check)
    *used_before_check = 0;

  if (!tokens || !var_name)
    /* LCOV_EXCL_START */
    return CDD_C_SUCCESS;
  /* LCOV_EXCL_STOP */

  {
    int is_inside = 0;
    is_inside_condition(tokens, alloc_idx, &is_inside);
    if (is_inside) {
      *out_is_checked = 1;
      /* LCOV_EXCL_START */
      return CDD_C_SUCCESS;
      /* LCOV_EXCL_STOP */
    }
  }

  while (i < tokens->size && tokens->tokens[i].kind != TOKEN_SEMICOLON)
    i++;
  if (i < tokens->size)
    i++;

  while (i < tokens->size) {
    /* LCOV_EXCL_START */
    const struct Token *tok = &tokens->tokens[i];
    if (tok->kind == TOKEN_RBRACE || tok->kind == TOKEN_KEYWORD_STRUCT) {
      return CDD_C_SUCCESS;
      /* LCOV_EXCL_STOP */
    }

    /* LCOV_EXCL_START */
    if (tok->kind == TOKEN_IDENTIFIER) {
      if ((token_matches_string(tok, var_name, &_ast_token_matches_string_0),
           /* LCOV_EXCL_STOP */
           _ast_token_matches_string_0)) {
        /* LCOV_EXCL_START */
        int is_inside = 0;
        is_inside_condition(tokens, i, &is_inside);
        if (is_inside) {
          /* LCOV_EXCL_STOP */
          *out_is_checked = 1;
          /* LCOV_EXCL_START */
          return CDD_C_SUCCESS;
          /* LCOV_EXCL_STOP */
        }
        /* LCOV_EXCL_START */
        if (spec->check_style == CHECK_PTR_NULL) {
          int is_deref = 0;
          is_dereference_use(tokens, i, &is_deref);
          if (is_deref) {
            if (used_before_check)
              /* LCOV_EXCL_STOP */
              *used_before_check = 1;
            /* LCOV_EXCL_START */
            return CDD_C_SUCCESS;
            /* LCOV_EXCL_STOP */
          }
        }
      }
    }
    /* LCOV_EXCL_START */
    i++;
    /* LCOV_EXCL_STOP */
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Retrieves the allocations.
 */
enum cdd_c_error find_allocations(const struct TokenList *tokens,
                                  struct AllocationSiteList *out) {
  int _ast_token_matches_string_1 = 0;
  char *_ast_get_assigned_var_2 = NULL;
  size_t i;
  int rc;

  if (!tokens || !out)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (out->sites == NULL) {
    if ((rc = allocation_site_list_init(out)) != 0)
      /* LCOV_EXCL_START */
      return rc;
    /* LCOV_EXCL_STOP */
  }

  for (i = 0; i < tokens->size; ++i) {
    const struct Token *tok = &tokens->tokens[i];

    if (tok->kind == TOKEN_IDENTIFIER) {
      const struct AllocatorSpec *spec = ALLOCATOR_SPECS;

      while (spec->name != NULL) {
        if ((token_matches_string(tok, spec->name,
                                  &_ast_token_matches_string_1),
             _ast_token_matches_string_1)) {
          char *var_name = NULL;
          int is_return = 0;

          {
            size_t prev = i;
            while (prev > 0) {
              prev--;
              if (tokens->tokens[prev].kind == TOKEN_WHITESPACE)
                continue;

              if (tokens->tokens[prev].kind == TOKEN_KEYWORD_RETURN) {
                /* LCOV_EXCL_START */
                is_return = 1;
                /* LCOV_EXCL_STOP */
              }
              break;
            }
          }

          if (is_return) {
            /* LCOV_EXCL_START */
            rc = allocation_site_list_add(out, i, NULL, 0, 0, 1, spec);
            if (rc != 0)
              return rc;
            break;
            /* LCOV_EXCL_STOP */
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
                var_name =
                    (get_assigned_var(tokens, prev, &_ast_get_assigned_var_2),
                     _ast_get_assigned_var_2);
                break;
              }
            }
          }

          if (var_name) {
            int used_before = 0;
            int checked = 0;
            is_checked(tokens, i, var_name, spec, &used_before, &checked);
            rc = allocation_site_list_add(out, i, var_name, checked,
                                          used_before, 0, spec);
            free(var_name);
            if (rc != 0)
              /* LCOV_EXCL_START */
              return rc;
            /* LCOV_EXCL_STOP */
          } else {
            /* LCOV_EXCL_START */
            int checked = 0;
            is_inside_condition(tokens, i, &checked);
            rc = allocation_site_list_add(out, i, NULL, checked, 0, 0, spec);
            if (rc != 0)
              return rc;
            /* LCOV_EXCL_STOP */
          }
          break;
        }
        spec++;
      }
    }
  }
  return CDD_C_SUCCESS;
}
