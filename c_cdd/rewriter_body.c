/**
 * @file rewriter_body.c
 * @brief High-level Orchestrator for function body rewriting.
 * @author Samuel Marks
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rewriter_body.h"
#include "str_utils.h"
#include "strategy_safety.h"
#include "text_patcher.h"
#include "tokenizer.h"

/* --- Implementation Helpers --- */

static const struct RefactoredFunction *
find_refactored_func(const struct RefactoredFunction *funcs, size_t func_count,
                     const char *name) {
  size_t i;
  if (!funcs || !name)
    return NULL;
  for (i = 0; i < func_count; ++i) {
    if (strcmp(funcs[i].name, name) == 0) {
      return &funcs[i];
    }
  }
  return NULL;
}

static char *extract_token_text(const struct Token *tok) {
  char *s = malloc(tok->length + 1);
  if (!s)
    return NULL;
  memcpy(s, tok->start, tok->length);
  s[tok->length] = '\0';
  return s;
}

static size_t find_semicolon(const struct TokenList *tokens, size_t start) {
  size_t i;
  for (i = start; i < tokens->size; ++i) {
    if (tokens->tokens[i].kind == TOKEN_SEMICOLON)
      return i;
    if (tokens->tokens[i].kind == TOKEN_LBRACE ||
        tokens->tokens[i].kind == TOKEN_RBRACE)
      return tokens->size;
  }
  return tokens->size;
}

static size_t find_stmt_start(const struct TokenList *tokens, size_t pos) {
  size_t i = pos;
  while (i > 0) {
    if (tokens->tokens[i - 1].kind == TOKEN_SEMICOLON ||
        tokens->tokens[i - 1].kind == TOKEN_LBRACE ||
        tokens->tokens[i - 1].kind == TOKEN_RBRACE) {
      return i;
    }
    i--;
  }
  return 0;
}

static char *join_tokens_range(const struct TokenList *tokens, size_t start,
                               size_t end) {
  size_t len = 0;
  size_t i;
  char *buf, *p;

  if (start >= end)
    return c_cdd_strdup("");

  for (i = start; i < end; ++i)
    len += tokens->tokens[i].length;

  buf = malloc(len + 1);
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

/* --- Core Logic --- */

int rewrite_body(const struct TokenList *const tokens,
                 const struct AllocationSiteList *const allocs,
                 const struct RefactoredFunction *funcs, size_t func_count,
                 const struct SignatureTransform *transform, char **out_code) {
  struct PatchList patches;
  int rc;
  size_t i;
  int injected_rc = 0;
  size_t tmp_var_counter = 0;

  /* 1. Initialize Patcher */
  if (patch_list_init(&patches) != 0)
    return ENOMEM;

  /* 2. Apply Safety Strategies */
  if (allocs) {
    rc = strategy_inject_safety_checks(tokens, allocs, &patches);
    if (rc != 0) {
      patch_list_free(&patches);
      return rc;
    }
  }

  /* 3. Rewrite Calls (Propagated) */
  if (funcs) {
    for (i = 0; i < tokens->size; ++i) {
      if (tokens->tokens[i].kind == TOKEN_IDENTIFIER) {
        const struct Token *id_tok = &tokens->tokens[i];
        char *name_str = extract_token_text(id_tok);
        const struct RefactoredFunction *rf =
            find_refactored_func(funcs, func_count, name_str);
        free(name_str);

        if (rf) {
          /* Check if it's a function call lookup: ID + LPAREN */
          size_t next = i + 1;
          while (next < tokens->size &&
                 tokens->tokens[next].kind == TOKEN_WHITESPACE)
            next++;

          if (next < tokens->size &&
              tokens->tokens[next].kind == TOKEN_LPAREN) {
            size_t call_start = i;
            size_t lparen = next;
            size_t rparen;
            size_t k;
            size_t arg_depth = 1;
            size_t prev;

            (void)call_start;

            /* Find RPAREN */
            k = lparen + 1;
            while (k < tokens->size && arg_depth > 0) {
              if (tokens->tokens[k].kind == TOKEN_LPAREN)
                arg_depth++;
              else if (tokens->tokens[k].kind == TOKEN_RPAREN)
                arg_depth--;
              k++;
            }
            rparen = k - 1;

            /* Determine context:
               1. Assignment (lhs = call())
               2. Statement (call();)
               3. Nested/Expression (call() as arg) */

            prev = i;
            while (prev > 0) {
              prev--;
              if (tokens->tokens[prev].kind != TOKEN_WHITESPACE)
                break;
            }

            if (prev < i && tokens->tokens[prev].kind == TOKEN_ASSIGN) {
              /* Case 1: Assignment */
              size_t eq_idx = prev;
              size_t lhs_start = prev;
              int is_decl = 0;

              /* Backtrack to find LHS var */
              while (lhs_start > 0) {
                if (tokens->tokens[lhs_start - 1].kind == TOKEN_SEMICOLON ||
                    tokens->tokens[lhs_start - 1].kind == TOKEN_LBRACE ||
                    tokens->tokens[lhs_start - 1].kind == TOKEN_RBRACE)
                  break;
                if (tokens->tokens[lhs_start - 1].kind ==
                        TOKEN_KEYWORD_STRUCT ||
                    (tokens->tokens[lhs_start - 1].kind == TOKEN_IDENTIFIER &&
                     lhs_start - 1 < prev - 1)) /* heuristics */
                  is_decl = 1;
                lhs_start--;
              }
              /* Trim leading WS */
              while (lhs_start < prev &&
                     tokens->tokens[lhs_start].kind == TOKEN_WHITESPACE)
                lhs_start++;

              /* Naive LHS name extraction: last identifier before = */
              {
                char *lhs_name = NULL;
                size_t n = eq_idx;
                while (n > lhs_start) {
                  n--;
                  if (tokens->tokens[n].kind == TOKEN_IDENTIFIER) {
                    lhs_name = extract_token_text(&tokens->tokens[n]);
                    break;
                  }
                }

                if (lhs_name) {
                  size_t semi;

                  if (is_decl) {
                    patch_list_add(&patches, eq_idx, eq_idx + 1,
                                   strdup("; rc ="));
                  } else {
                    patch_list_add(&patches, lhs_start, eq_idx + 1,
                                   strdup("rc ="));
                  }

                  /* Append arg */
                  {
                    char *arg_append = malloc(strlen(lhs_name) + 10);
                    /* Check if empty args */
                    int is_empty = 1;
                    size_t a;
                    for (a = lparen + 1; a < rparen; a++)
                      if (tokens->tokens[a].kind != TOKEN_WHITESPACE)
                        is_empty = 0;

                    if (is_empty)
                      sprintf(arg_append, "&%s", lhs_name);
                    else
                      sprintf(arg_append, ", &%s", lhs_name);
                    patch_list_add(&patches, rparen, rparen, arg_append);
                  }

                  /* Append check */
                  semi = find_semicolon(tokens, rparen);
                  if (semi < tokens->size)
                    patch_list_add(&patches, semi + 1, semi + 1,
                                   strdup(" if (rc != 0) return rc;"));

                  free(lhs_name);
                  injected_rc = 1;
                }
              }

            } else if (prev < i &&
                       (tokens->tokens[prev].kind == TOKEN_SEMICOLON ||
                        tokens->tokens[prev].kind == TOKEN_LBRACE ||
                        tokens->tokens[prev].kind == TOKEN_RBRACE)) {
              /* Case 2: Statement */
              size_t semi;
              patch_list_add(&patches, i, i, strdup("rc = "));
              semi = find_semicolon(tokens, next);
              if (semi < tokens->size) {
                patch_list_add(&patches, semi + 1, semi + 1,
                               strdup(" if (rc != 0) return rc;"));
              }
              injected_rc = 1;

            } else {
              /* Case 3: Nested Call (Expression) */
              /* Hoisting strategy */
              char tmp_var[64];
              char *call_args = NULL;
              char *injection = NULL;
              size_t stmt_start = find_stmt_start(tokens, i);

              sprintf(tmp_var, "_tmp_cdd_%zu", tmp_var_counter++);

              /* Extract original args */
              call_args = join_tokens_range(tokens, lparen + 1, rparen);

              /* Prepare injection */
#ifdef HAVE_ASPRINTF
              asprintf(&injection,
                       "%s %s; rc = %s(%s%s&%s); if (rc != 0) "
                       "return rc;\n  ",
                       rf->original_return_type, tmp_var, rf->name, call_args,
                       (strlen(call_args) > 0 ? ", " : ""), tmp_var);
#else
              injection = malloc(1024);
              sprintf(injection,
                      "%s %s; rc = %s(%s%s&%s); if (rc != 0) return "
                      "rc;\n  ",
                      rf->original_return_type, tmp_var, rf->name, call_args,
                      (strlen(call_args) > 0 ? ", " : ""), tmp_var);
#endif
              /* Inject before statement */
              patch_list_add(&patches, stmt_start, stmt_start, injection);

              /* Replace call with var */
              patch_list_add(&patches, i, rparen + 1, strdup(tmp_var));

              free(call_args);
              injected_rc = 1;
            }
          }
        }
      }
    }
  }

  /* 4. Transform Returns */
  if (transform) {
    for (i = 0; i < tokens->size; ++i) {
      if (tokens->tokens[i].kind == TOKEN_KEYWORD_RETURN) {

        if (transform->type == TRANSFORM_VOID_TO_INT) {
          size_t next = i + 1;
          while (next < tokens->size &&
                 tokens->tokens[next].kind == TOKEN_WHITESPACE)
            next++;
          if (next < tokens->size &&
              tokens->tokens[next].kind == TOKEN_SEMICOLON) {
            patch_list_add(&patches, i, next, strdup("return 0"));
          }
        } else if (transform->type == TRANSFORM_RET_PTR_TO_ARG) {
          size_t semi = find_semicolon(tokens, i);
          if (semi < tokens->size) {
            /* Fix: Check for inline unchecked alloc in return statement */
            int contains_alloc = 0;
            if (allocs) {
              size_t k;
              for (k = 0; k < allocs->size; k++) {
                /* If site is between return and semicolon */
                if (allocs->sites[k].token_index > i &&
                    allocs->sites[k].token_index < semi) {
                  contains_alloc = 1;
                  break;
                }
              }
            }

            if (contains_alloc) {
              /* Transform: return malloc(...) -> { Type _safe_ret =
               * malloc(...); if(!_safe_ret) return ENOMEM; *out = _safe_ret;
               * return 0; } */
              /* Extract expr between return (i) and semi (inclusive of nothing,
               * wait range excludes return kw) */
              char *expr = join_tokens_range(tokens, i + 1, semi);
              char *replacement = NULL;

#ifdef HAVE_ASPRINTF
              asprintf(&replacement,
                       "{ %s _safe_ret = %s; if (!_safe_ret) return %s; *%s = "
                       "_safe_ret; return %s; }",
                       transform->return_type, expr, transform->error_code,
                       transform->arg_name, transform->success_code);
#else
              replacement = malloc(strlen(expr) + 256);
              sprintf(replacement,
                      "{ %s _safe_ret = %s; if (!_safe_ret) return %s; *%s = "
                      "_safe_ret; return %s; }",
                      transform->return_type, expr, transform->error_code,
                      transform->arg_name, transform->success_code);
#endif
              /* Replace entire statement "return ...;" */
              patch_list_add(&patches, i, semi + 1, replacement);
              free(expr);
            } else {
              /* Replace return val; -> *out = val; return 0; */
              patch_list_add(&patches, i, i + 1, strdup("*out ="));
              patch_list_add(&patches, semi, semi + 1, strdup("; return 0;"));
            }
          }
        }
      }
    }

    if (transform->type == TRANSFORM_VOID_TO_INT && tokens->size > 0) {
      size_t last = tokens->size - 1;
      while (last > 0 && tokens->tokens[last].kind != TOKEN_RBRACE)
        last--;

      if (tokens->tokens[last].kind == TOKEN_RBRACE) {
        size_t prev_stmt = last;
        int has_ret = 0;
        while (prev_stmt > 0) {
          prev_stmt--;
          if (tokens->tokens[prev_stmt].kind == TOKEN_KEYWORD_RETURN) {
            has_ret = 1;
            break;
          }
          if (tokens->tokens[prev_stmt].kind == TOKEN_SEMICOLON ||
              tokens->tokens[prev_stmt].kind == TOKEN_RBRACE ||
              tokens->tokens[prev_stmt].kind == TOKEN_LBRACE)
            break;
        }
        if (!has_ret) {
          patch_list_add(&patches, last, last, strdup(" return 0; "));
        }
      }
    }
  }

  /* 5. Inject `int rc = 0;` if needed */
  if (injected_rc) {
    size_t k = 0;
    while (k < tokens->size && tokens->tokens[k].kind != TOKEN_LBRACE)
      k++;
    if (k < tokens->size) {
      patch_list_add(&patches, k + 1, k + 1, strdup("\n  int rc = 0;"));
    }
  }

  /* 6. Execute Patching */
  rc = patch_list_apply(&patches, tokens, out_code);

  patch_list_free(&patches);
  return rc;
}
