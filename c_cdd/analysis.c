/**
 * @file analysis.c
 * @brief Implementation of allocation analysis logic.
 * Supports complex allocator specifications, return-statement detection,
 * and usage-before-check analysis.
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

/* --- Registry of Allocators --- */

static const struct AllocatorSpec ALLOCATOR_SPECS[] = {
    /* Standard Pointers */
    {"malloc", ALLOC_STYLE_RETURN_PTR, CHECK_PTR_NULL, 0},
    {"calloc", ALLOC_STYLE_RETURN_PTR, CHECK_PTR_NULL, 0},
    {"realloc", ALLOC_STYLE_RETURN_PTR, CHECK_PTR_NULL, 0},
    {"strdup", ALLOC_STYLE_RETURN_PTR, CHECK_PTR_NULL, 0},
    {"strndup", ALLOC_STYLE_RETURN_PTR, CHECK_PTR_NULL, 0},
    {"realpath", ALLOC_STYLE_RETURN_PTR, CHECK_PTR_NULL, 0},
    {"get_current_dir_name", ALLOC_STYLE_RETURN_PTR, CHECK_PTR_NULL, 0},
    {"getpass", ALLOC_STYLE_RETURN_PTR, CHECK_PTR_NULL, 0},
    {"backtrace_symbols", ALLOC_STYLE_RETURN_PTR, CHECK_PTR_NULL, 0},
    /* Struct Pointers (Static or Alloc, treat as Ptr Check) */
    {"getpwnam", ALLOC_STYLE_RETURN_PTR, CHECK_PTR_NULL, 0},
    {"getpwuid", ALLOC_STYLE_RETURN_PTR, CHECK_PTR_NULL, 0},
    {"getgrnam", ALLOC_STYLE_RETURN_PTR, CHECK_PTR_NULL, 0},
    {"getgrgid", ALLOC_STYLE_RETURN_PTR, CHECK_PTR_NULL, 0},
    {"getspnam", ALLOC_STYLE_RETURN_PTR, CHECK_PTR_NULL, 0},
    {"gethostbyname", ALLOC_STYLE_RETURN_PTR, CHECK_PTR_NULL, 0},
    {"gethostbyaddr", ALLOC_STYLE_RETURN_PTR, CHECK_PTR_NULL, 0},
    /* Argument & Return Int Check based allocators */
    {"asprintf", ALLOC_STYLE_ARG_PTR, CHECK_INT_NEGATIVE, 0},
    {"vasprintf", ALLOC_STYLE_ARG_PTR, CHECK_INT_NEGATIVE, 0},
    {"getline", ALLOC_STYLE_ARG_PTR, CHECK_INT_NEGATIVE, 0},
    {"getdelim", ALLOC_STYLE_ARG_PTR, CHECK_INT_NEGATIVE, 0},
    {"scandir", ALLOC_STYLE_ARG_PTR, CHECK_INT_NEGATIVE, 1},
    {"alphasort", ALLOC_STYLE_RETURN_PTR, CHECK_PTR_NULL, 0},
    {"glob", ALLOC_STYLE_STRUCT_PTR, CHECK_INT_NONZERO, 3},
    /* Sentinel */
    {NULL, ALLOC_STYLE_RETURN_PTR, CHECK_PTR_NULL, 0}};

/* --- Implementation --- */

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

static int allocation_site_list_add(struct AllocationSiteList *const list,
                                    const size_t index, const char *var_name,
                                    const int checked, const int used_before,
                                    const int is_ret,
                                    const struct AllocatorSpec *spec) {
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
  list->sites[list->size].used_before_check = used_before;
  list->sites[list->size].is_return_stmt = is_ret;
  list->sites[list->size].spec = spec;
  if (var_name) {
    list->sites[list->size].var_name = strdup(var_name);
    if (!list->sites[list->size].var_name)
      return ENOMEM;
  } else {
    list->sites[list->size].var_name = NULL;
  }
  list->size++;
  return 0;
}

/* Comparison Helpers */
static int token_equals(const struct Token *tok, const char *str) {
  const size_t len = strlen(str);
  if (tok->length != len)
    return 0;
  return strncmp((const char *)tok->start, str, len) == 0;
}

/* Determine if we are inside `if (...)` or `while (...)` */
static int is_inside_condition(const struct TokenList *tokens, size_t idx) {
  int depth = 0;
  size_t i = idx;

  while (i > 0) {
    i--;
    if (tokens->tokens[i].kind == TOKEN_RPAREN) {
      depth++;
    } else if (tokens->tokens[i].kind == TOKEN_LPAREN) {
      if (depth > 0) {
        depth--;
      } else {
        /* unmatched open paren: check preceding keyword */
        size_t prev = i;
        while (prev > 0) {
          prev--;
          if (tokens->tokens[prev].kind == TOKEN_WHITESPACE)
            continue;
          if (token_equals(&tokens->tokens[prev], "if") ||
              token_equals(&tokens->tokens[prev], "while")) {
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

/* Get variable name assigned to: `var = ...` */
static char *get_assigned_var(const struct TokenList *tokens,
                              size_t assign_index) {
  size_t i = assign_index;
  if (assign_index == 0)
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

/* Get argument name from call */
static char *get_argument_var(const struct TokenList *tokens,
                              size_t function_idx, int arg_target_index) {
  size_t i = function_idx + 1;
  int current_arg = 0;
  int paren_depth = 0;
  int found_lparen = 0;

  while (i < tokens->size) {
    if (tokens->tokens[i].kind == TOKEN_WHITESPACE) {
      i++;
      continue;
    }
    if (tokens->tokens[i].kind == TOKEN_LPAREN) {
      found_lparen = 1;
      i++;
      break;
    }
    break;
  }

  if (!found_lparen)
    return NULL;

  while (i < tokens->size && current_arg <= arg_target_index) {
    if (tokens->tokens[i].kind == TOKEN_LPAREN) {
      paren_depth++;
    } else if (tokens->tokens[i].kind == TOKEN_RPAREN) {
      if (paren_depth > 0)
        paren_depth--;
      else
        return NULL;
    } else if (tokens->tokens[i].kind == TOKEN_COMMA && paren_depth == 0) {
      current_arg++;
      if (current_arg > arg_target_index)
        break;
      i++;
      continue;
    }

    if (current_arg == arg_target_index) {
      size_t j = i;
      while (j < tokens->size && tokens->tokens[j].kind == TOKEN_WHITESPACE)
        j++;

      if (j < tokens->size && tokens->tokens[j].kind == TOKEN_OTHER &&
          tokens->tokens[j].length == 1 && *tokens->tokens[j].start == '&') {
        j++;
        while (j < tokens->size && tokens->tokens[j].kind == TOKEN_WHITESPACE)
          j++;
      }

      if (j < tokens->size && tokens->tokens[j].kind == TOKEN_IDENTIFIER) {
        const struct Token *tok = &tokens->tokens[j];
        char *name = (char *)malloc(tok->length + 1);
        if (!name)
          return NULL;
        memcpy(name, tok->start, tok->length);
        name[tok->length] = '\0';
        return name;
      }
      return NULL;
    }
    i++;
  }
  return NULL;
}

/**
 * @brief Check if a variable is used (dereferenced) at current token position.
 */
static int is_dereference_use(const struct TokenList *tokens, size_t i,
                              const char *var_name) {
  if (i > 0) {
    size_t prev = i - 1;
    while (prev > 0 && tokens->tokens[prev].kind == TOKEN_WHITESPACE)
      prev--;
    if (tokens->tokens[prev].kind == TOKEN_OTHER &&
        tokens->tokens[prev].length == 1 &&
        *tokens->tokens[prev].start == '*') {
      return 1;
    }
  }

  {
    size_t next = i + 1;
    while (next < tokens->size && tokens->tokens[next].kind == TOKEN_WHITESPACE)
      next++;
    if (next < tokens->size) {
      const struct Token *t = &tokens->tokens[next];
      if ((t->length == 2 && strncmp((char *)t->start, "->", 2) == 0) ||
          t->kind == TOKEN_LBRACE) {
        return 1;
      }
      if (t->kind == TOKEN_OTHER && t->length == 1 && *t->start == '[') {
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

  if (is_inside_condition(tokens, alloc_idx)) {
    return 1;
  }

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
    } else if (tok->kind == TOKEN_WHITESPACE || tok->kind == TOKEN_COMMENT) {
    } else if (token_equals(tok, "if") || token_equals(tok, "while")) {
      size_t j = i + 1;
      while (j < tokens->size && tokens->tokens[j].kind == TOKEN_WHITESPACE)
        j++;

      if (j < tokens->size && tokens->tokens[j].kind == TOKEN_LPAREN) {
        int depth = 1;
        j++;
        while (j < tokens->size && depth > 0) {
          if (tokens->tokens[j].kind == TOKEN_IDENTIFIER) {
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
    } else if (tok->kind == TOKEN_IDENTIFIER) {
      if (token_equals(tok, var_name)) {

        /*
         * Check for dereference usage before checking for
         * assignment/reassignment. *p = 5 is a usage, not a reset.
         */
        if (spec->check_style == CHECK_PTR_NULL) {
          if (is_dereference_use(tokens, i, var_name)) {
            *used_before_check = 1;
            return 0;
          }
        }

        /* Check for Reassignment (p = ...) */
        {
          size_t next = i + 1;
          while (next < tokens->size &&
                 tokens->tokens[next].kind == TOKEN_WHITESPACE)
            next++;

          if (next < tokens->size && tokens->tokens[next].kind == TOKEN_OTHER &&
              tokens->tokens[next].length == 1 &&
              *tokens->tokens[next].start == '=') {
            return 0; /* Variable reassigned without check */
          }
        }
      }
    } else if (tok->kind == TOKEN_RBRACE) {
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

  if (out->sites == NULL) {
    rc = allocation_site_list_init(out);
    if (rc != 0)
      return rc;
  }

  for (i = 0; i < tokens->size; ++i) {
    const struct Token *tok = &tokens->tokens[i];

    if (tok->kind == TOKEN_IDENTIFIER) {
      const struct AllocatorSpec *spec = ALLOCATOR_SPECS;
      while (spec->name != NULL) {
        if (token_equals(tok, spec->name)) {
          char *var_name = NULL;
          int is_return = 0;

          {
            size_t prev = i;
            while (prev > 0) {
              prev--;
              if (tokens->tokens[prev].kind == TOKEN_WHITESPACE)
                continue;

              if (tokens->tokens[prev].kind == TOKEN_IDENTIFIER &&
                  token_equals(&tokens->tokens[prev], "return")) {
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

          if (spec->style == ALLOC_STYLE_RETURN_PTR) {
            size_t j = i;
            while (j > 0) {
              j--;
              if (tokens->tokens[j].kind == TOKEN_WHITESPACE)
                continue;
              if (tokens->tokens[j].kind == TOKEN_SEMICOLON ||
                  tokens->tokens[j].kind == TOKEN_LBRACE ||
                  tokens->tokens[j].kind == TOKEN_RBRACE) {
                break;
              }
              if (tokens->tokens[j].kind == TOKEN_OTHER &&
                  tokens->tokens[j].length == 1 &&
                  *tokens->tokens[j].start == '=') {
                var_name = get_assigned_var(tokens, j);
                break;
              }
            }
          } else if (spec->style == ALLOC_STYLE_ARG_PTR ||
                     spec->style == ALLOC_STYLE_STRUCT_PTR) {
            if (spec->check_style == CHECK_INT_NEGATIVE ||
                spec->check_style == CHECK_INT_NONZERO) {
              size_t j = i;
              while (j > 0) {
                j--;
                if (tokens->tokens[j].kind == TOKEN_WHITESPACE)
                  continue;
                if (tokens->tokens[j].kind == TOKEN_SEMICOLON ||
                    tokens->tokens[j].kind == TOKEN_LBRACE ||
                    tokens->tokens[j].kind == TOKEN_RBRACE)
                  break;
                if (tokens->tokens[j].kind == TOKEN_OTHER &&
                    tokens->tokens[j].length == 1 &&
                    *tokens->tokens[j].start == '=') {
                  var_name = get_assigned_var(tokens, j);
                  break;
                }
              }
              /* Fallback: if no assignment variable found (e.g. inline check
                 inside if), try to grab the argument pointer variable so we
                 have a record that an allocation happened. */
              if (!var_name) {
                var_name = get_argument_var(tokens, i, spec->ptr_arg_index);
              }
            } else {
              var_name = get_argument_var(tokens, i, spec->ptr_arg_index);
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
          }
          break;
        }
        spec++;
      }
    }
  }

  return 0;
}
