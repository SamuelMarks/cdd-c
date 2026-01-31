/**
 * @file rewriter_body.c
 * @brief Implementation of body rewriting and injection logic.
 * Handles stack variable injection, allocation checks, function call updates,
 * call-site transformation, and return statement transformation.
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

/* --- Internal Structures --- */

/**
 * @brief Represents a text replacement at a specific token index.
 */
struct Replacement {
  size_t token_start; /**< Start index of tokens to replace (inclusive) */
  size_t token_end;   /**< End index of tokens to replace (exclusive) */
  char *text;         /**< Null-terminated replacement text */
};

/**
 * @brief List of pending replacements.
 */
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

/**
 * @brief Add a replacement request.
 * Takes ownership of `text`.
 */
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

/**
 * @brief Compare function for qsort. Sorts by start index ascending.
 */
static int cmp_replacements(const void *a, const void *b) {
  const struct Replacement *ra = (const struct Replacement *)a;
  const struct Replacement *rb = (const struct Replacement *)b;
  if (ra->token_start < rb->token_start)
    return -1;
  if (ra->token_start > rb->token_start)
    return 1;
  return 0;
}

/**
 * @brief Helper to find token matching string.
 */
static int token_eq(const struct Token *tok, const char *s) {
  size_t len = strlen(s);
  return (tok->length == len && strncmp((const char *)tok->start, s, len) == 0);
}

/**
 * @brief Find index of next occurrence of a specific token kind.
 */
static size_t find_next_token(const struct TokenList *tokens, size_t start,
                              enum TokenKind kind) {
  size_t i;
  for (i = start; i < tokens->size; ++i) {
    if (tokens->tokens[i].kind == kind)
      return i;
  }
  return tokens->size;
}

/**
 * @brief Convert a range of tokens back to a string (joins them).
 */
static char *tokens_to_string(const struct TokenList *tokens, size_t start,
                              size_t end) {
  size_t len = 0;
  size_t i;
  char *buf, *p;

  /* Defensive check */
  if (start >= end) {
    char *empty = malloc(1);
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

/* --- Logic --- */

/**
 * @brief Check if a variable with the given name is declared in the token
 * scope.
 *
 * Simple heuristic: scans for IDENTIFIER matching name.
 * Does not perform full scope analysis (args are implicit if this is
 * body-only).
 *
 * @param[in] tokens Token list.
 * @param[in] name Variable name to check.
 * @return 1 if found, 0 otherwise.
 */
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

/**
 * @brief Inject a stack variable declaration at the top of the block.
 *
 * Assumes C89 compliance requirement: typically declarations must be at the
 * start of the block. We insert immediately after the first '{'.
 *
 * @param[in] tokens The token stream.
 * @param[in] patches The patch list to modify.
 * @param[in] type Type string (e.g. "int").
 * @param[in] name Variable name (e.g. "rc").
 * @param[in] init_val Initial value (e.g. "0").
 * @return 0 on success, ENOMEM on failure.
 */
static int inject_stack_variable(const struct TokenList *tokens,
                                 struct PatchList *patches, const char *type,
                                 const char *name, const char *init_val) {
  size_t i;
  /* Find first LBRACE */
  for (i = 0; i < tokens->size; ++i) {
    if (tokens->tokens[i].kind == TOKEN_LBRACE) {
      /* Inject after this token */
      char *decl = NULL;
      /* Format: " int name = init; " */
      if (asprintf(&decl, " %s %s = %s;", type, name, init_val) == -1) {
        return ENOMEM;
      }
      return add_replacement(patches, i + 1, i + 1, decl);
    }
  }
  return 0; /* No brace found, possibly abstract declaration or error, ignore */
}

/**
 * @brief Identify if the LHS of an assignment (`... =`) looks like a
 * declaration.
 */
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

  /* Heuristic: multiple tokens -> decl */
  if (token_count >= 2) {
    size_t k;
    for (k = start_idx; k < eq_idx; ++k) {
      const struct Token *t = &tokens->tokens[k];
      /* Check for [ or . or -> */
      if (t->kind == TOKEN_LBRACE || (t->length == 1 && *t->start == '.') ||
          (t->length == 2 && strncmp((char *)t->start, "->", 2) == 0)) {
        return 0; /* Likely assignment */
      }
    }
    return 1; /* Assume Decl */
  }

  return 0; /* Default to Assignment (single token) */
}

/**
 * @brief Generate check injections for unchecked allocations.
 */
static int process_allocations(const struct TokenList *tokens,
                               const struct AllocationSiteList *allocs,
                               struct PatchList *patches) {
  size_t i;
  for (i = 0; i < allocs->size; ++i) {
    size_t semi_idx;
    char *injection = NULL;

    if (allocs->sites[i].is_checked)
      continue;

    semi_idx =
        find_next_token(tokens, allocs->sites[i].token_index, TOKEN_SEMICOLON);

    if (semi_idx >= tokens->size)
      continue;

    if (asprintf(&injection, " if (!%s) { return ENOMEM; }",
                 allocs->sites[i].var_name) == -1) {
      return ENOMEM;
    }

    if (add_replacement(patches, semi_idx + 1, semi_idx + 1, injection) != 0) {
      return ENOMEM;
    }
  }
  return 0;
}

/**
 * @brief Identify and rewrite calls to refactored functions.
 * Returns *needs_stack_var = 1 if a rewrite logic requires `rc`.
 */
static int process_function_calls(const struct TokenList *tokens,
                                  const struct RefactoredFunction *funcs,
                                  size_t func_count, struct PatchList *patches,
                                  int *needs_stack_var) {
  size_t i;

  /* Iterate over tokens to find function calls */
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

    /* Look ahead for '(' */
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
        rparen--; /* Points to ')' */

        if (depth != 0)
          continue;

        {
          size_t semi = find_next_token(tokens, rparen, TOKEN_SEMICOLON);
          if (semi >= tokens->size)
            continue;

          /* Case 1: REF_VOID_TO_INT
             Original: func(...);
             Refactored: rc = func(...); if (rc != 0) return rc;
          */
          if (target->type == REF_VOID_TO_INT) {
            char *original_call = tokens_to_string(tokens, i, rparen + 1);
            char *replacement = NULL;
            if (!original_call)
              return ENOMEM;

            if (asprintf(&replacement, "%s = %s; if (%s != 0) return %s;",
                         STATUS_VAR_NAME, original_call, STATUS_VAR_NAME,
                         STATUS_VAR_NAME) == -1) {
              free(original_call);
              return ENOMEM;
            }
            free(original_call);

            /* Replace from func start to semi (inclusive) or just before?
               We found semi. We replace the whole statement `func(...);`
               Replace range: [i, semi+1] (which includes semi) */
            if (add_replacement(patches, i, semi + 1, replacement) != 0)
              return ENOMEM;

            if (needs_stack_var)
              *needs_stack_var = 1;
            i = semi; /* Advance loop */
          }
          /* Case 2: REF_PTR_TO_INT_OUT
             Original: var = func(args);
             Refactored: rc = func(args, &var); if (rc != 0) return rc;
          */
          else if (target->type == REF_PTR_TO_INT_OUT) {
            size_t eq_idx = i;
            int found_eq = 0;
            size_t statement_start = 0;

            /* Scan backwards for '=' to identify assignment context */
            while (eq_idx > 0) {
              eq_idx--;
              if (tokens->tokens[eq_idx].kind == TOKEN_WHITESPACE)
                continue;

              if (tokens->tokens[eq_idx].kind == TOKEN_OTHER &&
                  tokens->tokens[eq_idx].length == 1 &&
                  *tokens->tokens[eq_idx].start == '=') {
                found_eq = 1;
                break;
              }
              if (tokens->tokens[eq_idx].kind == TOKEN_SEMICOLON ||
                  tokens->tokens[eq_idx].kind == TOKEN_LBRACE ||
                  tokens->tokens[eq_idx].kind == TOKEN_RBRACE) {
                break;
              }
            }

            if (found_eq) {
              /* Found LHS: var = func(...) */
              statement_start = eq_idx;
              while (statement_start > 0) {
                size_t prev = statement_start - 1;
                if (tokens->tokens[prev].kind == TOKEN_SEMICOLON ||
                    tokens->tokens[prev].kind == TOKEN_LBRACE ||
                    tokens->tokens[prev].kind == TOKEN_RBRACE) {
                  break;
                }
                statement_start--;
              }

              /* Extract variable name from LHS */
              {
                /* Find last identifier before eq */
                size_t var_idx = eq_idx;
                int found_var = 0;
                while (var_idx > statement_start) {
                  var_idx--;
                  if (tokens->tokens[var_idx].kind == TOKEN_WHITESPACE)
                    continue;
                  if (tokens->tokens[var_idx].kind == TOKEN_IDENTIFIER) {
                    found_var = 1;
                    break;
                  }
                }

                if (found_var) {
                  size_t real_var_idx = var_idx;
                  /* (Note: var_idx loop above already skips whitespace backward
                     from eq, so real_var_idx points to the ID) */

                  if (tokens->tokens[real_var_idx].kind == TOKEN_IDENTIFIER) {
                    char *var_name = tokens_to_string(tokens, real_var_idx,
                                                      real_var_idx + 1);
                    char *args = tokens_to_string(tokens, lparen + 1, rparen);
                    char *replacement = NULL;
                    char *comma = "";
                    size_t replace_start;
                    int is_decl =
                        is_declaration(tokens, statement_start, eq_idx);

                    int has_args = 0;
                    size_t k;
                    for (k = lparen + 1; k < rparen; k++) {
                      if (tokens->tokens[k].kind != TOKEN_WHITESPACE) {
                        has_args = 1;
                        break;
                      }
                    }
                    if (has_args)
                      comma = ", ";

                    if (!var_name || !args) {
                      free(var_name);
                      free(args);
                      return ENOMEM;
                    }

                    if (is_decl) {
                      /* Declaration: `Type var = func();`
                         -> `Type var; rc = func(&var); if (rc) return rc;`
                      */
                      /* We replace from '=' onwards to ';' */
                      /* e.g. " = func(args);" -> "; rc = func(args, &var);..."
                       */

                      if (asprintf(
                              &replacement,
                              "; %s = %s(%s%s&%s); if (%s != 0) return %s;",
                              STATUS_VAR_NAME, target->name, args, comma,
                              var_name, STATUS_VAR_NAME,
                              STATUS_VAR_NAME) == -1) {
                        free(var_name);
                        free(args);
                        return ENOMEM;
                      }
                      /* apply from eq_idx (the '=') to semi+1 */
                      if (add_replacement(patches, eq_idx, semi + 1,
                                          replacement) != 0) {
                        free(var_name);
                        free(args);
                        return ENOMEM;
                      }
                    } else {
                      /* Assignment: `var = func();`
                         -> `rc = func(&var); if (rc) return rc;`
                      */

                      if (asprintf(&replacement,
                                   "%s = %s(%s%s&%s); if (%s != 0) return %s;",
                                   STATUS_VAR_NAME, target->name, args, comma,
                                   var_name, STATUS_VAR_NAME,
                                   STATUS_VAR_NAME) == -1) {
                        free(var_name);
                        free(args);
                        return ENOMEM;
                      }

                      /* Skip leading whitespace in replacement range from
                       * statement start */
                      replace_start = statement_start;
                      while (replace_start < eq_idx &&
                             tokens->tokens[replace_start].kind ==
                                 TOKEN_WHITESPACE) {
                        replace_start++;
                      }

                      /* Replace full statement `var = func();` */
                      if (add_replacement(patches, replace_start, semi + 1,
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

                    i = semi;
                  }
                }
              }
            } else {
              /* Case 3: Immediate Use / Return / Standalone?
                 Standalone: `func(args);` (Return value ignored, leak?)
                 Return: `return func(args);`
                 If just `func(args);` but returns ptr -> logic above for
                 finding eq fails. If it is just `char *p = func(args);` we
                 caught it. What about `return func(args);` ?

                 Find precedent 'return' token.
              */
              size_t prev_chk = i;
              while (prev_chk > 0) {
                prev_chk--;
                if (tokens->tokens[prev_chk].kind == TOKEN_WHITESPACE)
                  continue;
                if (tokens->tokens[prev_chk].kind == TOKEN_IDENTIFIER &&
                    token_eq(&tokens->tokens[prev_chk], "return")) {

                  /* Found `return func(args);` */
                  /* Transform to:
                     `Type tmp; rc = func(args, &tmp); if (rc != 0) return rc;
                     return tmp;` We need to know the Type. The
                     `RefactoredFunction` struct doesn't strictly carry Type
                     info yet, only refactor type enum. If we don't know the
                     type, we can't create `tmp`. However, maybe we can use
                     `void *` or inject a placeholder? Or maybe we assume we are
                     converting `char* func()` to `int func(char**)` and the
                     caller `return func()` signature matches.

                     If the caller function returns `char*`, checking ref_func
                     `func` which also returns `char*`: The caller must also be
                     refactored? 'Call-Site Rewriter' normally assumes we are
                     inside a function being refactored OR we are fixing a
                     client. If we are just fixing client code without changing
                     client signature: We can't return NULL/pointer if we got an
                     int `rc`. But `func` now returns `int`. Original: `return
                     func();` (returns ptr) New: `func` returns int (error).

                     Standard Fix pattern:
                     Client code `f()` calls `g()`. `g` changes to return int.
                     `f` should normally propagate.
                     If `f` signature is also being changed
                     (TRANSFORM_RET_PTR_TO_ARG), then `return func()` inside `f`
                     needs to become: `rc = func(..., &out_val); *out = out_val;
                     return rc;`

                     If `f` is NOT being changed (TRANSFORM_NONE), we have a
                     type mismatch. This rewriter handles the BODY. If
                     `transform` arg is set, we know our new signature.

                     Let's assume we are rewriting `f` which is also being
                     converted.
                  */
                  /* For now, handle simple `return func();` -> error
                     propagation if void? RefactorType is the target function's
                     change. If target is VOID_TO_INT: `return func();` ->
                     `return func();`? Original `void func()` -> `int func()`.
                     Caller `void caller() { return func(); }` (valid in C if
                     func is void). New `int caller() { return func(); }`. This
                     just works naturally if types align? But if we want to
                     check `rc`: `rc = func(); if (rc) return rc; return 0;`
                  */
                  /* Implementation complexity High: requires knowing context
                     return type. For 'Medium' scope defined here (Call-Site
                     Rewriter), we focus on assignment/call. We will skip
                     aggressive return-statement rewriting of calls until Type
                     info is available.
                  */
                }
                break;
              }
            }
          }
        }
      }
    }
  }
  return 0;
}

/**
 * @brief Rewrite return statements based on function transformation spec.
 */
static int process_return_statements(const struct TokenList *tokens,
                                     const struct SignatureTransform *transform,
                                     struct PatchList *patches) {
  size_t i;

  if (!transform || transform->type == TRANSFORM_NONE)
    return 0;

  for (i = 0; i < tokens->size; ++i) {
    if (tokens->tokens[i].kind == TOKEN_IDENTIFIER &&
        token_eq(&tokens->tokens[i], "return")) {

      size_t semi = find_next_token(tokens, i, TOKEN_SEMICOLON);
      if (semi >= tokens->size)
        continue;

      if (transform->type == TRANSFORM_VOID_TO_INT) {
        char *replacement = NULL;
        if (asprintf(&replacement, "return %s;", transform->success_code) == -1)
          return ENOMEM;

        if (add_replacement(patches, i, semi + 1, replacement) != 0)
          return ENOMEM;

      } else if (transform->type == TRANSFORM_RET_PTR_TO_ARG) {
        char *expr = NULL;
        char *replacement = NULL;

        size_t expr_start = i + 1;
        while (expr_start < semi &&
               tokens->tokens[expr_start].kind == TOKEN_WHITESPACE)
          expr_start++;

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
        } else {
          if (asprintf(&replacement, "{ *%s = %s; return %s; }",
                       transform->arg_name, expr,
                       transform->success_code) == -1) {
            free(expr);
            return ENOMEM;
          }
        }
        free(expr);

        if (add_replacement(patches, i, semi + 1, replacement) != 0)
          return ENOMEM;
      }
    }
  }
  return 0;
}

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
  }

  /* Check/Inject variable */
  if (needs_stack_var) {
    if (!variable_exists(tokens, STATUS_VAR_NAME)) {
      rc = inject_stack_variable(tokens, &patches, STATUS_VAR_TYPE,
                                 STATUS_VAR_NAME, STATUS_VAR_INIT);
      if (rc != 0)
        goto cleanup;
    }
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

        current_token = rep->token_end;
        p_idx++;

        while (p_idx < patches.size &&
               patches.items[p_idx].token_start < current_token) {
          p_idx++;
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
