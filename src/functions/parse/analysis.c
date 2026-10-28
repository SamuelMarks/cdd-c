/**
 * @file analysis.c
 * @brief Implementation of static safety analysis.
 * @author Samuel Marks
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include "c_cdd_export.h"
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "c_cdd/log.h"
#include "c_cdd/memory.h"
#include "c_cdd_stdbool.h"
#include "classes/parse/cdd_token.h"
#include "functions/parse/analysis.h"
#include "functions/parse/str.h"
/* clang-format on */

/** @brief Table of known C memory allocator specifications. */
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
 * @brief Initializes a list of tracked memory allocation sites.
 *
 * @param[out] list Pointer to the AllocationSiteList structure to initialize.
 * @return `CDD_C_SUCCESS` on success, or an error code on failure.
 */
cdd_c_error_t allocation_site_list_init(struct AllocationSiteList *list) {
  if (!list)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  list->size = 0;
  list->capacity = 8;
  list->sites = (struct AllocationSite *)C_CDD_MALLOC(
      list->capacity * sizeof(struct AllocationSite));
  if (!list->sites) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Releases memory held by an allocation site list.
 *
 * @param[in,out] list Pointer to the AllocationSiteList structure to free.
 */
void allocation_site_list_free(struct AllocationSiteList *list) {
  size_t i;
  if (!list)
    return;
  if (list->sites) {
    for (i = 0; i < list->size; ++i) {
      if (list->sites[i].var_name)
        C_CDD_FREE(list->sites[i].var_name);
    }
    C_CDD_FREE(list->sites);
    list->sites = NULL;
  }
  list->size = 0;
  list->capacity = 0;
}

/**
 * @brief Records a newly discovered memory allocation site.
 *
 * @param[in,out] list Target AllocationSiteList container.
 * @param[in] index Token index of the allocation call.
 * @param[in] var_name Assigned variable identifier name (may be NULL).
 * @param[in] checked Non-zero if checked for NULL/error.
 * @param[in] used_before Non-zero if dereferenced before being checked.
 * @param[in] is_ret Non-zero if directly returned.
 * @param[in] spec Matching AllocatorSpec descriptor.
 * @return `CDD_C_SUCCESS` on success, or an error code on failure.
 */
cdd_c_error_t allocation_site_list_add(struct AllocationSiteList *list,
                                       size_t index, const char *var_name,
                                       int checked, int used_before, int is_ret,
                                       const struct AllocatorSpec *spec) {
  if (!list)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (list->size >= list->capacity) {
    const size_t new_cap = (list->capacity == 0) ? 8 : list->capacity * 2;
    struct AllocationSite *new_sites = (struct AllocationSite *)C_CDD_REALLOC(
        list->sites, new_cap * sizeof(struct AllocationSite));
    if (!new_sites) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      return CDD_C_ERROR_MEMORY;
    }
    list->sites = new_sites;
    list->capacity = new_cap;
  }

  list->sites[list->size].token_index = index;
  list->sites[list->size].is_checked = checked;
  list->sites[list->size].used_before_check = used_before;
  list->sites[list->size].is_return_stmt = is_ret;
  list->sites[list->size].spec = spec;

  if (var_name) {
    c_cdd_strdup(var_name, &list->sites[list->size].var_name);
    if (!list->sites[list->size].var_name)
      return CDD_C_ERROR_MEMORY;
  } else {
    list->sites[list->size].var_name = NULL;
  }

  list->size++;
  return CDD_C_SUCCESS;
}

/**
 * @brief Retrieves the variable identifier on the left-hand side of an
 * assignment token.
 *
 * @param[in] tokens Token stream.
 * @param[in] assign_idx Index of the `=` assignment token.
 * @param[out] _out_val Output pointer receiving the newly allocated variable
 * name (or NULL).
 * @return `CDD_C_SUCCESS` on success.
 */
static cdd_c_error_t get_assigned_var(const struct TokenList *tokens,
                                      size_t assign_idx, char **_out_val) {
  size_t i = assign_idx;
  if (assign_idx == 0) {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
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
      if (g_cdd_fail_alloc == 1) {
        name = NULL;
        g_cdd_fail_alloc = 0;
      } else {
        if (g_cdd_fail_alloc > 1) {
          g_cdd_fail_alloc--;
        }
        name = (char *)C_CDD_MALLOC(tok->length + 1);
      }
    }
#else
    name = (char *)C_CDD_MALLOC(tok->length + 1);
#endif
    if (!name) {
      *_out_val = NULL;
      return CDD_C_SUCCESS;
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
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Checks if the token at `idx` appears inside an `if` or `while`
 * condition expression.
 *
 * @param[in] tokens Token stream.
 * @param[in] idx Token index to inspect.
 * @param[out] out_is_inside Output pointer receiving 1 if inside condition, 0
 * otherwise.
 * @return `CDD_C_SUCCESS` on success, or an error code on failure.
 */
static cdd_c_error_t is_inside_condition(const struct TokenList *tokens,
                                         size_t idx, int *out_is_inside) {
  size_t i = idx;
  int paren_depth = 0;
#ifdef CDD_BUILD_TESTS
  extern C_CDD_EXPORT int g_cdd_fail_is_inside_condition;
  if (g_cdd_fail_is_inside_condition && --g_cdd_fail_is_inside_condition == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  *out_is_inside = 0;

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
          if (tokens->tokens[prev].kind == TOKEN_KEYWORD_IF) {
            *out_is_inside = 1;
            return CDD_C_SUCCESS;
          }
          if (tokens->tokens[prev].kind == TOKEN_KEYWORD_WHILE) {
            *out_is_inside = 1;
            return CDD_C_SUCCESS;
          }
          break;
        }
      }
    } else if (tokens->tokens[i].kind == TOKEN_SEMICOLON) {
      break;
    } else if (tokens->tokens[i].kind == TOKEN_LBRACE) {
      break;
    } else if (tokens->tokens[i].kind == TOKEN_RBRACE) {
      break;
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Checks if the identifier token at `i` is being dereferenced (`*x`,
 * `x->`, or `x[`).
 *
 * @param[in] tokens Token stream.
 * @param[in] i Token index of the identifier.
 * @param[out] out_is_deref Output pointer receiving 1 if dereferenced, 0
 * otherwise.
 * @return `CDD_C_SUCCESS` on success, or an error code on failure.
 */
static cdd_c_error_t is_dereference_use(const struct TokenList *tokens,
                                        size_t i, int *out_is_deref) {
#ifdef CDD_BUILD_TESTS
  extern C_CDD_EXPORT int g_cdd_fail_is_dereference_use;
  if (g_cdd_fail_is_dereference_use && --g_cdd_fail_is_dereference_use == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  *out_is_deref = 0;
  {
    size_t prev = i - 1;
    for (;;) {
      if (tokens->tokens[prev].kind != TOKEN_WHITESPACE) {
        break;
      }
      prev--;
    }
    if (tokens->tokens[prev].kind == TOKEN_STAR) {
      *out_is_deref = 1;
      return CDD_C_SUCCESS;
    }
  }
  {
    size_t next = i + 1;
    while (next < tokens->size && tokens->tokens[next].kind == TOKEN_WHITESPACE)
      next++;
    if (next < tokens->size) {
      const struct Token *t = &tokens->tokens[next];
      if (t->kind == TOKEN_ARROW) {
        *out_is_deref = 1;
        return CDD_C_SUCCESS;
      }
      if (t->kind == TOKEN_LBRACKET) {
        *out_is_deref = 1;
        return CDD_C_SUCCESS;
      }
    }
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Determines whether an allocated variable is checked for NULL before
 * being dereferenced.
 *
 * @param[in] tokens Lexical token list.
 * @param[in] alloc_idx Token index of the allocation call.
 * @param[in] var_name Assigned variable identifier name.
 * @param[in] spec Matching AllocatorSpec descriptor.
 * @param[out] used_before_check Output pointer receiving 1 if dereferenced
 * before check, 0 otherwise.
 * @param[out] out_is_checked Output pointer receiving 1 if checked, 0
 * otherwise.
 * @return `CDD_C_SUCCESS` on success, or an error code on failure.
 */
cdd_c_error_t is_checked(const struct TokenList *tokens, size_t alloc_idx,
                         const char *var_name, const struct AllocatorSpec *spec,
                         int *used_before_check, int *out_is_checked) {
  size_t i = alloc_idx;
#ifdef CDD_BUILD_TESTS
  extern C_CDD_EXPORT int g_cdd_fail_is_checked;
  if (g_cdd_fail_is_checked && --g_cdd_fail_is_checked == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  if (!out_is_checked)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *out_is_checked = 0;
  if (used_before_check)
    *used_before_check = 0;

  if (!tokens || !var_name)
    return CDD_C_SUCCESS;

  {
    int is_inside = 0;
    {
      cdd_c_error_t rc_an = is_inside_condition(tokens, alloc_idx, &is_inside);
      if (rc_an != CDD_C_SUCCESS)
        return rc_an;
    }
    if (is_inside) {
      *out_is_checked = 1;
      return CDD_C_SUCCESS;
    }
  }

  while (i < tokens->size && tokens->tokens[i].kind != TOKEN_SEMICOLON)
    i++;
  if (i < tokens->size)
    i++;

  while (i < tokens->size) {
    const struct Token *tok = &tokens->tokens[i];
    if (tok->kind == TOKEN_RBRACE) {
      return CDD_C_SUCCESS;
    }
    if (tok->kind == TOKEN_KEYWORD_STRUCT) {
      return CDD_C_SUCCESS;
    }

    if (tok->kind == TOKEN_IDENTIFIER) {
      int matched = 0;
      token_matches_string(tok, var_name, &matched);
      if (matched) {
        int is_inside = 0;
        {
          cdd_c_error_t rc_an = is_inside_condition(tokens, i, &is_inside);
          if (rc_an != CDD_C_SUCCESS)
            return rc_an;
        }
        if (is_inside) {
          *out_is_checked = 1;
          return CDD_C_SUCCESS;
        }
        if (spec->check_style == CHECK_PTR_NULL) {
          int is_deref = 0;
          {
            cdd_c_error_t rc_an = is_dereference_use(tokens, i, &is_deref);
            if (rc_an != CDD_C_SUCCESS)
              return rc_an;
          }
          if (is_deref) {
            if (used_before_check)
              *used_before_check = 1;
            return CDD_C_SUCCESS;
          }
        }
      }
    }
    i++;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Scans a token stream to identify all memory allocation calls and their
 * check status.
 *
 * @param[in] tokens Lexical token list to scan.
 * @param[in,out] out Target AllocationSiteList container to populate.
 * @return `CDD_C_SUCCESS` on success, or an error code on failure.
 */
cdd_c_error_t find_allocations(const struct TokenList *tokens,
                               struct AllocationSiteList *out) {
  size_t i;
  cdd_c_error_t rc;

  if (!tokens || !out)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (out->sites == NULL) {
    if ((rc = allocation_site_list_init(out)) != CDD_C_SUCCESS)
      return rc;
  }

  for (i = 0; i < tokens->size; ++i) {
    const struct Token *tok = &tokens->tokens[i];

    if (tok->kind == TOKEN_IDENTIFIER) {
      const struct AllocatorSpec *spec = ALLOCATOR_SPECS;

      while (spec->name != NULL) {
        int matched = 0;
        token_matches_string(tok, spec->name, &matched);
        if (matched) {
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
            if (rc != CDD_C_SUCCESS)
              return rc;
            break;
          }

          {
            size_t prev = i;
            while (prev > 0) {
              prev--;
              switch (tokens->tokens[prev].kind) {
              case TOKEN_WHITESPACE:
                continue;
              case TOKEN_SEMICOLON:
              case TOKEN_LBRACE:
              case TOKEN_RBRACE:
                break;
              case TOKEN_ASSIGN:
                get_assigned_var(tokens, prev, &var_name);
                break;
              default:
                continue;
              }
              break;
            }
          }

          if (var_name) {
            int used_before = 0;
            int checked = 0;
            {
              cdd_c_error_t rc_an =
                  is_checked(tokens, i, var_name, spec, &used_before, &checked);
              if (rc_an != CDD_C_SUCCESS) {
                C_CDD_FREE(var_name);
                return rc_an;
              }
            }
            rc = allocation_site_list_add(out, i, var_name, checked,
                                          used_before, 0, spec);
            C_CDD_FREE(var_name);
            if (rc != CDD_C_SUCCESS)
              return rc;
          } else {
            int checked = 0;
            {
              cdd_c_error_t rc_an = is_inside_condition(tokens, i, &checked);
              if (rc_an != CDD_C_SUCCESS)
                return rc_an;
            }
            rc = allocation_site_list_add(out, i, NULL, checked, 0, 0, spec);
            if (rc != CDD_C_SUCCESS)
              return rc;
          }
          break;
        }
        spec++;
      }
    }
  }
  return CDD_C_SUCCESS;
}
