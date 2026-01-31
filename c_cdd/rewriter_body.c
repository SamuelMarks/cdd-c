/**
 * @file rewriter_body.c
 * @brief Implementation of body rewriting and injection logic.
 * Handles stack variable injection, safety injection for allocators,
 * call-site propagation, and return statement transformation.
 * @author Samuel Marks
 */

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <c_str_span.h>

/* Use system errno if standard includes don't provide it */
#if defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__)
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#define strdup _strdup
#endif
#else
#include <sys/errno.h>
#endif

/* Assume asprintf is available via project headers/compat */
#include <c89stringutils_string_extras.h>

#include "rewriter_body.h"

/* Configuration constants */
static const char *const STATUS_VAR_NAME = "rc";
static const char *const STATUS_VAR_TYPE = "int";
static const char *const STATUS_VAR_INIT = "0";
static const char *const DEFAULT_ERROR_CODE = "ENOMEM";

/* --- Internal Structures --- */

struct Replacement {
  size_t token_start;
  size_t token_end;
  char *text;
};

struct PatchList {
  struct Replacement *items;
  size_t size;
  size_t capacity;
};

/* --- Helpers --- */

static int patch_list_init(struct PatchList *pl) {
  pl->size = 0;
  pl->capacity = 8;
  pl->items =
      (struct Replacement *)malloc(sizeof(struct Replacement) * pl->capacity);
  if (!pl->items)
    return ENOMEM;
  return 0;
}

static void patch_list_free(struct PatchList *pl) {
  size_t i;
  if (!pl)
    return;
  if (pl->items) {
    for (i = 0; i < pl->size; ++i) {
      free(pl->items[i].text);
    }
    free(pl->items);
  }
}

static int add_replacement(struct PatchList *pl, size_t start, size_t end,
                           char *text) {
  if (pl->size >= pl->capacity) {
    size_t new_cap = pl->capacity * 2;
    struct Replacement *new_items = (struct Replacement *)realloc(
        pl->items, new_cap * sizeof(struct Replacement));
    if (!new_items) {
      free(text);
      return ENOMEM;
    }
    pl->items = new_items;
    pl->capacity = new_cap;
  }
  pl->items[pl->size].token_start = start;
  pl->items[pl->size].token_end = end;
  pl->items[pl->size].text = text;
  pl->size++;
  return 0;
}

static int cmp_replacements(const void *a, const void *b) {
  const struct Replacement *ra = (const struct Replacement *)a;
  const struct Replacement *rb = (const struct Replacement *)b;
  if (ra->token_start < rb->token_start)
    return -1;
  if (ra->token_start > rb->token_start)
    return 1;
  return 0;
}

static int token_eq(const struct Token *tok, const char *s) {
  size_t len = strlen(s);
  return (tok->length == len && strncmp((const char *)tok->start, s, len) == 0);
}

static size_t find_next_token(const struct TokenList *tokens, size_t start,
                              enum TokenKind kind) {
  size_t i;
  for (i = start; i < tokens->size; ++i) {
    if (tokens->tokens[i].kind == kind)
      return i;
  }
  return tokens->size;
}

static char *tokens_to_string(const struct TokenList *tokens, size_t start,
                              size_t end) {
  size_t len = 0;
  size_t i;
  char *buf, *p;

  if (start >= end) {
    char *empty = (char *)malloc(1);
    if (empty)
      *empty = '\0';
    return empty;
  }

  for (i = start; i < end; ++i)
    len += tokens->tokens[i].length;

  buf = (char *)malloc(len + 1);
  if (!buf)
    return NULL;

  p = buf;
  for (i = start; i < end; ++i) {
    memcpy(p, tokens->tokens[i].start, tokens->tokens[i].length);
    p += tokens->tokens[i].length;
  }
  *p = '\0';
  return buf;
}

static char *get_first_arg_name(const struct TokenList *tokens,
                                size_t lparen_idx) {
  size_t i = lparen_idx + 1;
  while (i < tokens->size && tokens->tokens[i].kind == TOKEN_WHITESPACE)
    i++;

  if (i < tokens->size && tokens->tokens[i].kind == TOKEN_IDENTIFIER) {
    size_t next = i + 1;
    while (next < tokens->size && tokens->tokens[next].kind == TOKEN_WHITESPACE)
      next++;
    if (next < tokens->size && (tokens->tokens[next].kind == TOKEN_COMMA ||
                                tokens->tokens[next].kind == TOKEN_RPAREN)) {
      return tokens_to_string(tokens, i, i + 1);
    }
  }
  return NULL;
}

static int variable_exists(const struct TokenList *tokens, const char *name) {
  size_t i;
  for (i = 0; i < tokens->size; ++i) {
    if (tokens->tokens[i].kind == TOKEN_IDENTIFIER) {
      if (token_eq(&tokens->tokens[i], name)) {
        return 1;
      }
    }
  }
  return 0;
}

static int is_declaration(const struct TokenList *tokens, size_t start_idx,
                          size_t eq_idx) {
  size_t i = start_idx;
  int token_count = 0;
  int has_type_keyword = 0;
  int starts_deref = 0;

  while (i < eq_idx) {
    if (tokens->tokens[i].kind == TOKEN_WHITESPACE) {
      i++;
      continue;
    }

    if (token_count == 0) {
      if (tokens->tokens[i].kind == TOKEN_KEYWORD_STRUCT ||
          tokens->tokens[i].kind == TOKEN_KEYWORD_ENUM ||
          tokens->tokens[i].kind == TOKEN_KEYWORD_UNION ||
          token_eq(&tokens->tokens[i], "int") ||
          token_eq(&tokens->tokens[i], "char") ||
          token_eq(&tokens->tokens[i], "void") ||
          token_eq(&tokens->tokens[i], "float") ||
          token_eq(&tokens->tokens[i], "double") ||
          token_eq(&tokens->tokens[i], "long") ||
          token_eq(&tokens->tokens[i], "short") ||
          token_eq(&tokens->tokens[i], "unsigned") ||
          token_eq(&tokens->tokens[i], "signed") ||
          token_eq(&tokens->tokens[i], "const") ||
          token_eq(&tokens->tokens[i], "static") ||
          token_eq(&tokens->tokens[i], "extern")) {
        has_type_keyword = 1;
      } else if (tokens->tokens[i].length == 1 &&
                 *tokens->tokens[i].start == '*') {
        starts_deref = 1;
      }
    }
    token_count++;
    i++;
  }

  if (has_type_keyword)
    return 1;
  if (starts_deref)
    return 0;

  if (token_count >= 2) {
    size_t k;
    for (k = start_idx; k < eq_idx; ++k) {
      const struct Token *t = &tokens->tokens[k];
      if (t->kind == TOKEN_LBRACE || (t->length == 1 && *t->start == '.') ||
          (t->length == 2 && strncmp((char *)t->start, "->", 2) == 0) ||
          (t->length == 1 && *t->start == '[')) {
        return 0;
      }
    }
    return 1;
  }

  return 0;
}

/* --- Logic Implementations --- */

/**
 * @brief Rewrite realloc patterns to include safety check using a temporary
 * variable. Converts: `p = realloc(p, n);` To:
 * `{ void *_safe_tmp = realloc(p, n); if (!_safe_tmp) return ENOMEM; p =
 * _safe_tmp; }`
 */
static int process_realloc_safety(const struct TokenList *tokens,
                                  const struct AllocationSite *site,
                                  struct PatchList *patches, size_t semi_idx) {
  char *arg1 = NULL;
  size_t call_idx = site->token_index;
  size_t lparen_idx;
  size_t assign_op_idx = 0;
  size_t stmt_start = 0;
  int is_self_assign = 0;

  if (!site->var_name)
    return 0;

  {
    size_t i = call_idx;
    while (i > 0) {
      i--;
      if (tokens->tokens[i].kind == TOKEN_OTHER &&
          tokens->tokens[i].length == 1 && *tokens->tokens[i].start == '=') {
        assign_op_idx = i;
        break;
      }
      if (tokens->tokens[i].kind == TOKEN_SEMICOLON ||
          tokens->tokens[i].kind == TOKEN_LBRACE ||
          tokens->tokens[i].kind == TOKEN_RBRACE) {
        break;
      }
    }
  }

  if (assign_op_idx == 0)
    return 0;

  stmt_start = assign_op_idx;
  while (stmt_start > 0) {
    size_t prev = stmt_start - 1;
    if (tokens->tokens[prev].kind == TOKEN_SEMICOLON ||
        tokens->tokens[prev].kind == TOKEN_LBRACE ||
        tokens->tokens[prev].kind == TOKEN_RBRACE) {
      break;
    }
    stmt_start--;
  }
  while (stmt_start < assign_op_idx &&
         tokens->tokens[stmt_start].kind == TOKEN_WHITESPACE)
    stmt_start++;

  lparen_idx = find_next_token(tokens, call_idx, TOKEN_LPAREN);
  if (lparen_idx >= tokens->size)
    return 0;

  arg1 = get_first_arg_name(tokens, lparen_idx);
  if (arg1 && strcmp(arg1, site->var_name) == 0) {
    is_self_assign = 1;
  }
  free(arg1);

  if (is_self_assign) {
    char *call_expr = NULL;
    char *replacement = NULL;

    call_expr = tokens_to_string(tokens, call_idx, semi_idx);
    if (!call_expr)
      return ENOMEM;

    if (asprintf(&replacement,
                 "{ void *_safe_tmp = %s; if (!_safe_tmp) return %s; %s = "
                 "_safe_tmp; }",
                 call_expr, DEFAULT_ERROR_CODE, site->var_name) == -1) {
      free(call_expr);
      return ENOMEM;
    }
    free(call_expr);

    return add_replacement(patches, stmt_start, semi_idx + 1, replacement);
  }
  return 0;
}

static int process_allocations(const struct TokenList *tokens,
                               const struct AllocationSiteList *allocs,
                               struct PatchList *patches) {
  size_t i;
  for (i = 0; i < allocs->size; ++i) {
    size_t semi_idx;
    char *injection = NULL;
    const struct AllocationSite *site = &allocs->sites[i];

    if (site->is_checked)
      continue;

    semi_idx = find_next_token(tokens, site->token_index, TOKEN_SEMICOLON);
    if (semi_idx >= tokens->size)
      continue;

    /* Handle realloc specially if assignment target matches input pointer */
    if (strcmp(site->spec->name, "realloc") == 0) {
      size_t old_size = patches->size;
      int rc_realloc = process_realloc_safety(tokens, site, patches, semi_idx);
      if (rc_realloc != 0)
        return rc_realloc;
      /* If realloc patch was added, skip standard Injection */
      if (patches->size > old_size)
        continue;
    }

    /* Check valid var_name before injection attempt */
    if (!site->var_name)
      continue;

    if (site->spec->check_style == CHECK_PTR_NULL) {
      if (asprintf(&injection, " if (!%s) { return %s; }", site->var_name,
                   DEFAULT_ERROR_CODE) == -1)
        return ENOMEM;
    } else if (site->spec->check_style == CHECK_INT_NEGATIVE) {
      if (asprintf(&injection, " if (%s < 0) { return %s; }", site->var_name,
                   DEFAULT_ERROR_CODE) == -1)
        return ENOMEM;
    } else if (site->spec->check_style == CHECK_INT_NONZERO) {
      if (asprintf(&injection, " if (%s != 0) { return %s; }", site->var_name,
                   DEFAULT_ERROR_CODE) == -1)
        return ENOMEM;
    } else {
      /* Unknown style, skip */
      continue;
    }

    if (add_replacement(patches, semi_idx + 1, semi_idx + 1, injection) != 0) {
      return ENOMEM;
    }
  }
  return 0;
}

static int process_return_statements(const struct TokenList *tokens,
                                     const struct SignatureTransform *transform,
                                     struct PatchList *patches) {
  size_t i;

  for (i = 0; i < tokens->size; ++i) {
    if (tokens->tokens[i].kind == TOKEN_IDENTIFIER &&
        token_eq(&tokens->tokens[i], "return")) {

      size_t semi = find_next_token(tokens, i, TOKEN_SEMICOLON);
      if (semi >= tokens->size)
        continue;

      if (transform->type == TRANSFORM_VOID_TO_INT) {
        char *replacement = NULL;
        if (asprintf(&replacement, "return %s;", transform->success_code) ==
            -1) {
          return ENOMEM;
        }
        if (add_replacement(patches, i, semi + 1, replacement) != 0) {
          return ENOMEM;
        }

      } else if (transform->type == TRANSFORM_RET_PTR_TO_ARG) {
        char *expr = NULL;
        char *replacement = NULL;
        size_t expr_start = i + 1;
        while (expr_start < semi &&
               tokens->tokens[expr_start].kind == TOKEN_WHITESPACE) {
          expr_start++;
        }

        if (expr_start == semi) {
          expr = strdup("");
        } else {
          expr = tokens_to_string(tokens, expr_start, semi);
        }

        if (!expr)
          return ENOMEM;

        if (transform->error_code && strcmp(expr, "NULL") == 0) {
          if (asprintf(&replacement, "return %s;", transform->error_code) ==
              -1) {
            free(expr);
            return ENOMEM;
          }
        } else if (transform->error_code && strcmp(expr, "0") == 0) {
          if (asprintf(&replacement, "return %s;", transform->error_code) ==
              -1) {
            free(expr);
            return ENOMEM;
          }
        } else {
          if (transform->return_type) {
            /* Safer injection: Check allocation success logic */
            if (asprintf(&replacement,
                         "{ %s _val = %s; if (!_val) return %s; *%s = _val; "
                         "return %s; }",
                         transform->return_type, expr,
                         transform->error_code ? transform->error_code
                                               : DEFAULT_ERROR_CODE,
                         transform->arg_name ? transform->arg_name : "out",
                         transform->success_code) == -1) {
              free(expr);
              return ENOMEM;
            }
          } else {
            /* Fallback (potentially legacy behavior if type unavailable) */
            if (asprintf(&replacement, "{ *%s = %s; return %s; }",
                         transform->arg_name ? transform->arg_name : "out",
                         expr, transform->success_code) == -1) {
              free(expr);
              return ENOMEM;
            }
          }
        }
        free(expr);

        if (add_replacement(patches, i, semi + 1, replacement) != 0) {
          return ENOMEM;
        }
      }
    }
  }
  return 0;
}

static int inject_final_return(const struct TokenList *tokens,
                               const struct SignatureTransform *transform,
                               struct PatchList *patches) {
  if (transform->type != TRANSFORM_VOID_TO_INT)
    return 0;

  /* Look for the final closing brace */
  if (tokens->size > 0 &&
      tokens->tokens[tokens->size - 1].kind == TOKEN_RBRACE) {
    size_t rbrace_idx = tokens->size - 1;
    char *injection = NULL;

    /* We unconditionally inject return success.
       If the function had void return, it falls through to this success code.
     */
    if (asprintf(&injection, " return %s;", transform->success_code) == -1) {
      return ENOMEM;
    }

    return add_replacement(patches, rbrace_idx, rbrace_idx, injection);
  }
  return 0;
}

static int process_function_calls(const struct TokenList *tokens,
                                  const struct RefactoredFunction *funcs,
                                  size_t func_count, struct PatchList *patches,
                                  int *needs_stack_var) {
  size_t i;

  for (i = 0; i < tokens->size; ++i) {
    size_t f_idx;
    const struct RefactoredFunction *target = NULL;

    if (tokens->tokens[i].kind != TOKEN_IDENTIFIER)
      continue;

    for (f_idx = 0; f_idx < func_count; ++f_idx) {
      if (token_eq(&tokens->tokens[i], funcs[f_idx].name)) {
        target = &funcs[f_idx];
        break;
      }
    }

    if (!target)
      continue;

    {
      size_t next = i + 1;
      while (next < tokens->size &&
             tokens->tokens[next].kind == TOKEN_WHITESPACE)
        next++;

      if (next >= tokens->size || tokens->tokens[next].kind != TOKEN_LPAREN) {
        continue;
      }

      {
        size_t lparen = next;
        size_t rparen = lparen + 1;
        int depth = 1;

        while (rparen < tokens->size && depth > 0) {
          if (tokens->tokens[rparen].kind == TOKEN_LPAREN)
            depth++;
          else if (tokens->tokens[rparen].kind == TOKEN_RPAREN)
            depth--;
          rparen++;
        }
        rparen--;

        if (depth != 0)
          continue;

        if (target->type == REF_VOID_TO_INT) {
          size_t semi = find_next_token(tokens, rparen, TOKEN_SEMICOLON);
          if (semi < tokens->size) {
            size_t prev = i;
            int is_standalone = 1;
            while (prev > 0) {
              prev--;
              if (tokens->tokens[prev].kind == TOKEN_SEMICOLON ||
                  tokens->tokens[prev].kind == TOKEN_LBRACE ||
                  tokens->tokens[prev].kind == TOKEN_RBRACE)
                break;
              if (tokens->tokens[prev].kind != TOKEN_WHITESPACE) {
                is_standalone = 0;
                break;
              }
            }

            if (is_standalone) {
              char *call_str = tokens_to_string(tokens, i, rparen + 1);
              char *replacement = NULL;
              if (!call_str)
                return ENOMEM;
              if (asprintf(&replacement, "%s = %s; if (%s != 0) return %s;",
                           STATUS_VAR_NAME, call_str, STATUS_VAR_NAME,
                           STATUS_VAR_NAME) == -1) {
                free(call_str);
                return ENOMEM;
              }
              free(call_str);
              if (add_replacement(patches, i, semi + 1, replacement) != 0)
                return ENOMEM;
              if (needs_stack_var)
                *needs_stack_var = 1;
              i = semi;
              continue;
            }
          }
        } else if (target->type == REF_PTR_TO_INT_OUT) {
          size_t stmt_start = i;
          size_t eq_idx = 0;
          int found_eq = 0;

          {
            size_t cur = i;
            while (cur > 0) {
              cur--;
              if (tokens->tokens[cur].kind == TOKEN_SEMICOLON ||
                  tokens->tokens[cur].kind == TOKEN_LBRACE ||
                  tokens->tokens[cur].kind == TOKEN_RBRACE) {
                stmt_start = cur + 1;
                break;
              }
              if (tokens->tokens[cur].kind == TOKEN_OTHER &&
                  tokens->tokens[cur].length == 1 &&
                  *tokens->tokens[cur].start == '=') {
                found_eq = 1;
                eq_idx = cur;
              }
            }
            while (stmt_start < i &&
                   tokens->tokens[stmt_start].kind == TOKEN_WHITESPACE)
              stmt_start++;
          }

          if (found_eq) {
            size_t var_idx = eq_idx;
            int found_var = 0;
            while (var_idx > stmt_start) {
              var_idx--;
              if (tokens->tokens[var_idx].kind == TOKEN_IDENTIFIER) {
                found_var = 1;
                break;
              }
            }

            if (found_var) {
              char *var_name = tokens_to_string(tokens, var_idx, var_idx + 1);
              char *args = tokens_to_string(tokens, lparen + 1, rparen);
              char *replacement = NULL;
              char *comma = "";
              int is_args_empty = 1;
              size_t k;
              size_t semi = find_next_token(tokens, rparen, TOKEN_SEMICOLON);

              for (k = lparen + 1; k < rparen; k++)
                if (tokens->tokens[k].kind != TOKEN_WHITESPACE)
                  is_args_empty = 0;
              if (!is_args_empty)
                comma = ", ";

              if (is_declaration(tokens, stmt_start, eq_idx)) {
                if (asprintf(&replacement,
                             "; %s = %s(%s%s&%s); if (%s != 0) return %s;",
                             STATUS_VAR_NAME, target->name, args, comma,
                             var_name, STATUS_VAR_NAME,
                             STATUS_VAR_NAME) == -1) {
                  free(var_name);
                  free(args);
                  return ENOMEM;
                }
                if (add_replacement(patches, eq_idx, semi + 1, replacement) !=
                    0) {
                  free(var_name);
                  free(args);
                  return ENOMEM;
                }
              } else {
                if (asprintf(&replacement,
                             "%s = %s(%s%s&%s); if (%s != 0) return %s;",
                             STATUS_VAR_NAME, target->name, args, comma,
                             var_name, STATUS_VAR_NAME,
                             STATUS_VAR_NAME) == -1) {
                  free(var_name);
                  free(args);
                  return ENOMEM;
                }
                if (add_replacement(patches, stmt_start, semi + 1,
                                    replacement) != 0) {
                  free(var_name);
                  free(args);
                  return ENOMEM;
                }
              }
              if (needs_stack_var)
                *needs_stack_var = 1;
              free(var_name);
              free(args);
              continue;
            }
          } else {
            int is_nested = 0;
            size_t k = i;
            while (k > 0) {
              k--;
              if (tokens->tokens[k].kind == TOKEN_LPAREN) {
                is_nested = 1;
                break;
              }
              if (tokens->tokens[k].kind == TOKEN_SEMICOLON ||
                  tokens->tokens[k].kind == TOKEN_LBRACE)
                break;
            }

            if (is_nested && target->original_return_type) {
              static int tmp_counter = 0;
              char *tmp_var = NULL;
              char *hoist_code = NULL;
              char *args = tokens_to_string(tokens, lparen + 1, rparen);
              char *comma = "";
              int is_args_empty = 1;
              size_t m;

              for (m = lparen + 1; m < rparen; m++)
                if (tokens->tokens[m].kind != TOKEN_WHITESPACE)
                  is_args_empty = 0;
              if (!is_args_empty)
                comma = ", ";

              if (asprintf(&tmp_var, "_tmp_cdd_%d", tmp_counter++) == -1) {
                free(args);
                return ENOMEM;
              }

              if (asprintf(&hoist_code,
                           "%s %s; %s = %s(%s%s&%s); if (%s != 0) return %s; ",
                           target->original_return_type, tmp_var,
                           STATUS_VAR_NAME, target->name, args, comma, tmp_var,
                           STATUS_VAR_NAME, STATUS_VAR_NAME) == -1) {
                free(args);
                free(tmp_var);
                return ENOMEM;
              }

              {
                size_t st = i;
                while (st > 0) {
                  st--;
                  if (tokens->tokens[st].kind == TOKEN_SEMICOLON ||
                      tokens->tokens[st].kind == TOKEN_LBRACE ||
                      tokens->tokens[st].kind == TOKEN_RBRACE) {
                    st++;
                    break;
                  }
                }
                while (st < i && tokens->tokens[st].kind == TOKEN_WHITESPACE)
                  st++;

                if (add_replacement(patches, st, st, hoist_code) != 0) {
                  free(args);
                  free(tmp_var);
                  return ENOMEM;
                }
              }

              if (add_replacement(patches, i, rparen + 1, strdup(tmp_var)) !=
                  0) {
                free(args);
                free(tmp_var);
                return ENOMEM;
              }

              if (needs_stack_var)
                *needs_stack_var = 1;
              free(args);
              free(tmp_var);
              continue;
            }
          }
        }
      }
    }
  }
  return 0;
}

static int inject_stack_variable(const struct TokenList *tokens,
                                 struct PatchList *patches, const char *type,
                                 const char *name, const char *init_val) {
  size_t i;
  for (i = 0; i < tokens->size; ++i) {
    if (tokens->tokens[i].kind == TOKEN_LBRACE) {
      char *decl = NULL;
      if (asprintf(&decl, " %s %s = %s;", type, name, init_val) == -1) {
        return ENOMEM;
      }
      return add_replacement(patches, i + 1, i + 1, decl);
    }
  }
  return 0;
}

/* --- Public API --- */

int rewrite_body(const struct TokenList *tokens,
                 const struct AllocationSiteList *allocs,
                 const struct RefactoredFunction *funcs, size_t func_count,
                 const struct SignatureTransform *transform, char **out_code) {
  struct PatchList patches;
  int rc;
  char *output = NULL;
  size_t out_len = 0;
  size_t current_token = 0;
  size_t p_idx = 0;
  int needs_stack_var = 0;

  if (!tokens || !out_code)
    return EINVAL;

  if (patch_list_init(&patches) != 0)
    return ENOMEM;

  if (allocs) {
    rc = process_allocations(tokens, allocs, &patches);
    if (rc != 0)
      goto cleanup;
  }

  if (funcs && func_count > 0) {
    rc = process_function_calls(tokens, funcs, func_count, &patches,
                                &needs_stack_var);
    if (rc != 0)
      goto cleanup;
  }

  if (transform) {
    rc = process_return_statements(tokens, transform, &patches);
    if (rc != 0)
      goto cleanup;

    rc = inject_final_return(tokens, transform, &patches);
    if (rc != 0)
      goto cleanup;
  }

  if (needs_stack_var && !variable_exists(tokens, STATUS_VAR_NAME)) {
    rc = inject_stack_variable(tokens, &patches, STATUS_VAR_TYPE,
                               STATUS_VAR_NAME, STATUS_VAR_INIT);
    if (rc != 0)
      goto cleanup;
  }

  qsort(patches.items, patches.size, sizeof(struct Replacement),
        cmp_replacements);

  {
    size_t cap = 1024;
    output = (char *)malloc(cap);
    if (!output) {
      rc = ENOMEM;
      goto cleanup;
    }
    output[0] = '\0';

    while (current_token < tokens->size) {
      if (p_idx < patches.size &&
          patches.items[p_idx].token_start == current_token) {
        struct Replacement *rep = &patches.items[p_idx];
        size_t rep_len = strlen(rep->text);

        while (out_len + rep_len + 1 > cap) {
          char *tmp;
          cap = cap * 2 + rep_len;
          tmp = (char *)realloc(output, cap);
          if (!tmp) {
            rc = ENOMEM;
            goto cleanup;
          }
          output = tmp;
        }
        memcpy(output + out_len, rep->text, rep_len);
        out_len += rep_len;
        output[out_len] = '\0';

        if (rep->token_start < rep->token_end) {
          current_token = rep->token_end;
        }
        p_idx++;

        while (p_idx < patches.size &&
               patches.items[p_idx].token_start == current_token &&
               patches.items[p_idx].token_start ==
                   patches.items[p_idx].token_end) {
          break;
        }

      } else {
        const struct Token *tok = &tokens->tokens[current_token];
        size_t tok_len = tok->length;

        while (out_len + tok_len + 1 > cap) {
          char *tmp;
          cap *= 2;
          tmp = (char *)realloc(output, cap);
          if (!tmp) {
            rc = ENOMEM;
            goto cleanup;
          }
          output = tmp;
        }
        memcpy(output + out_len, tok->start, tok_len);
        out_len += tok_len;
        output[out_len] = '\0';

        current_token++;
      }
    }
  }

  *out_code = output;
  output = NULL;
  rc = 0;

cleanup:
  patch_list_free(&patches);
  if (output)
    free(output);
  return rc;
}
