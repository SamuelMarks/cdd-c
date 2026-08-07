/**
 * @file rewriter_body.c
 * @brief High-level Orchestrator for function body rewriting.
 * @author Samuel Marks
 */

/* clang-format off */
#include "c_cdd_export.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include "c_cdd/memory.h"
#include <string.h>

#include "functions/emit/patcher.h"
#include "functions/emit/rewriter_body.h"
#include "functions/parse/str.h"
#include "functions/parse/strategy.h"
#include "functions/parse/tokenizer.h"
#include "win_compat_sym.h"
/* clang-format on */

/* --- Implementation Helpers --- */

static cdd_c_error_t
find_refactored_func(const struct RefactoredFunction *funcs, size_t func_count,
                     const char *name,
                     const struct RefactoredFunction **_out_val) {
  size_t i;
  for (i = 0; i < func_count; ++i) {
    if (strcmp(funcs[i].name, name) == 0) {
      {
        *_out_val = &funcs[i];
        return CDD_C_SUCCESS;
      }
    }
  }
  {
    *_out_val = NULL;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Extracts token text.
 */
static cdd_c_error_t extract_token_text(const struct Token *tok,
                                        char **_out_val) {
  char *s = C_CDD_MALLOC(tok->length + 1);
  if (!s) {
    *_out_val = NULL;
    return CDD_C_ERROR_MEMORY;
  }
  memcpy(s, tok->start, tok->length);
  s[tok->length] = '\0';
  {
    *_out_val = s;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Retrieves the semicolon.
 */
static cdd_c_error_t find_semicolon(const struct TokenList *tokens,
                                    size_t start, size_t *_out_val) {
  size_t i;
  for (i = start; i < tokens->size; ++i) {
    if (tokens->tokens[i].kind == TOKEN_SEMICOLON) {
      *_out_val = i;
      return CDD_C_SUCCESS;
    }
    if (tokens->tokens[i].kind == TOKEN_LBRACE ||
        tokens->tokens[i].kind == TOKEN_RBRACE) {
      *_out_val = tokens->size;
      return CDD_C_SUCCESS;
    }
  }
  {
    *_out_val = tokens->size;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Retrieves the stmt start.
 */
static cdd_c_error_t find_stmt_start(const struct TokenList *tokens, size_t pos,
                                     size_t *_out_val) {
  size_t i = pos;
  while (i > 0) {
    if (tokens->tokens[i - 1].kind == TOKEN_SEMICOLON ||
        tokens->tokens[i - 1].kind == TOKEN_LBRACE ||
        tokens->tokens[i - 1].kind == TOKEN_RBRACE) {
      {
        *_out_val = i;
        return CDD_C_SUCCESS;
      }
    }
    i--;
  }
  {
    *_out_val = 0;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Executes the join tokens range operation.
 */
static cdd_c_error_t join_tokens_range(const struct TokenList *tokens,
                                       size_t start, size_t end,
                                       char **_out_val) {
  size_t len = 0;
  size_t i;
  char *buf, *p;

  if (start >= end) {
    c_cdd_strdup("", _out_val);
    return CDD_C_SUCCESS;
  }

  for (i = start; i < end; ++i)
    len += tokens->tokens[i].length;

  buf = C_CDD_MALLOC(len + 1);
  if (!buf) {
    *_out_val = NULL;
    return CDD_C_ERROR_MEMORY;
  }

  p = buf;
  for (i = start; i < end; ++i) {
    memcpy(p, tokens->tokens[i].start, tokens->tokens[i].length);
    p += tokens->tokens[i].length;
  }
  *p = '\0';
  {
    *_out_val = buf;
    return CDD_C_SUCCESS;
  }
}

/* --- Core Logic --- */

/**
 * @brief Executes the rewrite body operation.
 */
cdd_c_error_t rewrite_body(const struct TokenList *tokens,
                           const struct AllocationSiteList *allocs,
                           const struct RefactoredFunction *funcs,
                           size_t func_count,
                           const struct SignatureTransform *transform,
                           char **out_code) {
  struct PatchList patches;
  int rc;
  size_t i;
  int injected_rc = 0;
  size_t tmp_var_counter = 0;

  if (!tokens || !out_code) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  /* 1. Initialize Patcher */
#ifdef CDD_BUILD_TESTS
  {
    extern C_CDD_EXPORT int g_cdd_fail_alloc;
    if (g_cdd_fail_alloc == 1)
      return CDD_C_ERROR_MEMORY;
  }
#endif
  if (patch_list_init(&patches) != 0) {
    return CDD_C_ERROR_MEMORY;
  }

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
        char *name_str = NULL;
        const struct RefactoredFunction *rf = NULL;
        rc = extract_token_text(id_tok, &name_str);
        if (rc != CDD_C_SUCCESS) {
          patch_list_free(&patches);
          return rc;
        }
        find_refactored_func(funcs, func_count, name_str, &rf);
        C_CDD_FREE(name_str);

        if (rf) {
          /* Check if it's a function call lookup: ID + LPAREN */
          size_t next = i + 1;
          while (next < tokens->size &&
                 tokens->tokens[next].kind == TOKEN_WHITESPACE) {
            next++;
          }

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
              if (tokens->tokens[k].kind == TOKEN_LPAREN) {
                arg_depth++;
              } else if (tokens->tokens[k].kind == TOKEN_RPAREN)
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
                    rc = extract_token_text(&tokens->tokens[n], &lhs_name);
                    if (rc != CDD_C_SUCCESS)
                      goto cleanup;
                    break;
                  }
                }

                if (lhs_name) {
                  size_t semi;

                  if (is_decl) {
                    {
                      char *tmp = NULL;
                      rc = c_cdd_strdup("; rc =", &tmp);
                      if (rc != CDD_C_SUCCESS)
                        goto cleanup;
                      rc = patch_list_add(&patches, eq_idx, eq_idx + 1, tmp);

                      if (rc != CDD_C_SUCCESS)
                        goto cleanup;
                    };
                  } else {
                    {
                      char *tmp = NULL;
                      rc = c_cdd_strdup("rc =", &tmp);
                      if (rc != CDD_C_SUCCESS)
                        goto cleanup;
                      rc = patch_list_add(&patches, lhs_start, eq_idx + 1, tmp);

                      if (rc != CDD_C_SUCCESS)
                        goto cleanup;
                    };
                  }

                  /* Append arg */
                  {
                    char *arg_append;
                    int is_empty = 1;
                    size_t a;
                    arg_append = C_CDD_MALLOC(strlen(lhs_name) + 10);
                    if (!arg_append) {
                      rc = CDD_C_ERROR_MEMORY;
                      goto cleanup;
                    }
                    /* Check if empty args */
                    for (a = lparen + 1; a < rparen; a++)
                      if (tokens->tokens[a].kind != TOKEN_WHITESPACE)
                        is_empty = 0;

                    if (is_empty)
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
                      sprintf_s(arg_append, strlen(lhs_name) + 10, "&%s",
                                lhs_name);
#else
                      sprintf(arg_append, "&%s", lhs_name);
#endif
                    else
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
                      sprintf_s(arg_append, strlen(lhs_name) + 10, ", &%s",
                                lhs_name);
#else
                      sprintf(arg_append, ", &%s", lhs_name);
#endif
                    rc = patch_list_add(&patches, rparen, rparen, arg_append);

                    if (rc != CDD_C_SUCCESS)
                      goto cleanup;
                  }

                  /* Append check */
                  find_semicolon(tokens, rparen, &semi);
                  if (semi < tokens->size) {
                    char *tmp = NULL;
                    rc = c_cdd_strdup(" if (rc != 0) return rc;", &tmp);
                    if (rc != CDD_C_SUCCESS)
                      goto cleanup;
                    rc = patch_list_add(&patches, semi + 1, semi + 1, tmp);

                    if (rc != CDD_C_SUCCESS)
                      goto cleanup;
                  };

                  C_CDD_FREE(lhs_name);
                  injected_rc = 1;
                }
              }

            } else if (prev < i &&
                       (tokens->tokens[prev].kind == TOKEN_SEMICOLON ||
                        tokens->tokens[prev].kind == TOKEN_LBRACE ||
                        tokens->tokens[prev].kind == TOKEN_RBRACE)) {
              /* Case 2: Statement */
              size_t semi;
              {
                char *tmp = NULL;
                rc = c_cdd_strdup("rc = ", &tmp);
                if (rc != CDD_C_SUCCESS)
                  goto cleanup;
                rc = patch_list_add(&patches, i, i, tmp);

                if (rc != CDD_C_SUCCESS)
                  goto cleanup;
              };
              find_semicolon(tokens, next, &semi);
              if (semi < tokens->size) {
                {
                  char *tmp = NULL;
                  rc = c_cdd_strdup(" if (rc != 0) return rc;", &tmp);
                  if (rc != CDD_C_SUCCESS)
                    goto cleanup;
                  rc = patch_list_add(&patches, semi + 1, semi + 1, tmp);

                  if (rc != CDD_C_SUCCESS)
                    goto cleanup;
                };
              }
              injected_rc = 1;

            } else {
              /* Case 3: Nested Call (Expression) */
              /* Hoisting strategy */
              char tmp_var[64];
              char *call_args = NULL;
              char *injection = NULL;
              size_t stmt_start = 0;
              find_stmt_start(tokens, i, &stmt_start);

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
              sprintf_s(tmp_var, sizeof(tmp_var),
                        "_tmp_cdd_%" CDD_SIZE_T_FMT "",
                        (size_t)(tmp_var_counter++));
#else
              sprintf(tmp_var, "_tmp_cdd_%" CDD_SIZE_T_FMT "",
                      (size_t)(tmp_var_counter++));
#endif

              /* Extract original args */
              rc = join_tokens_range(tokens, lparen + 1, rparen, &call_args);
              if (rc != CDD_C_SUCCESS)
                goto cleanup;

              /* Prepare injection */
              injection = C_CDD_MALLOC(1024);
              if (!injection) {
                rc = CDD_C_ERROR_MEMORY;
                goto cleanup;
              }
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
              sprintf_s(injection, 1024,
                        "%s %s; rc = %s(%s%s&%s); if (rc != 0) return "
                        "rc;\n  ",
                        rf->original_return_type, tmp_var, rf->name, call_args,
                        (strlen(call_args) > 0 ? ", " : ""), tmp_var);
#else
              sprintf(injection,
                      "%s %s; rc = %s(%s%s&%s); if (rc != 0) return "
                      "rc;\n  ",
                      rf->original_return_type, tmp_var, rf->name, call_args,
                      (strlen(call_args) > 0 ? ", " : ""), tmp_var);

#endif
              /* Inject before statement */
              rc = patch_list_add(&patches, stmt_start, stmt_start, injection);

              if (rc != CDD_C_SUCCESS)
                goto cleanup;
              /* Replace call with var */
              {
                char *tmp = NULL;
                rc = c_cdd_strdup(tmp_var, &tmp);
                if (rc != CDD_C_SUCCESS)
                  goto cleanup;
                rc = patch_list_add(&patches, i, rparen + 1, tmp);

                if (rc != CDD_C_SUCCESS)
                  goto cleanup;
              };

              C_CDD_FREE(call_args);
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
            {
              char *tmp = NULL;
              rc = c_cdd_strdup("return 0", &tmp);
              if (rc != CDD_C_SUCCESS)
                goto cleanup;
              rc = patch_list_add(&patches, i, next, tmp);

              if (rc != CDD_C_SUCCESS)
                goto cleanup;
            };
          }
        } else if (transform->type == TRANSFORM_RET_PTR_TO_ARG) {
          size_t semi = 0;
          find_semicolon(tokens, i, &semi);
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
              /* Transform: return C_CDD_MALLOC(...) -> { Type _safe_ret =
               * C_CDD_MALLOC(...); if(!_safe_ret) return CDD_C_ERROR_MEMORY;
               * *out = _safe_ret; return CDD_C_SUCCESS; } */
              /* Extract expr between return (i) and semi (inclusive of nothing,
               * wait range excludes return kw) */
              char *expr = NULL;
              char *replacement = NULL;
              rc = join_tokens_range(tokens, i + 1, semi, &expr);
              if (rc != CDD_C_SUCCESS)
                goto cleanup;

              replacement = (char *)C_CDD_MALLOC(strlen(expr) + 256);
              if (!replacement) {
                C_CDD_FREE(expr);
                rc = CDD_C_ERROR_MEMORY;
                goto cleanup;
              }
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
              sprintf_s(replacement, strlen(expr) + 256,
                        "{ %s _safe_ret = %s; if (!_safe_ret) return %s; *%s = "
                        "_safe_ret; return %s; }",
                        transform->return_type, expr, transform->error_code,
                        transform->arg_name, transform->success_code);
#else
              sprintf(replacement,
                      "{ %s _safe_ret = %s; if (!_safe_ret) return %s; *%s = "
                      "_safe_ret; return %s; }",
                      transform->return_type, expr, transform->error_code,
                      transform->arg_name, transform->success_code);

#endif
              /* Replace entire statement "return ...;" */
              rc = patch_list_add(&patches, i, semi + 1, replacement);

              if (rc != CDD_C_SUCCESS)
                goto cleanup;
              C_CDD_FREE(expr);
            } else {
              /* Replace return val; -> *out = val; return CDD_C_SUCCESS; */
              {
                char *tmp = NULL;
                rc = c_cdd_strdup("*out =", &tmp);
                if (rc != CDD_C_SUCCESS)
                  goto cleanup;
                rc = patch_list_add(&patches, i, i + 1, tmp);

                if (rc != CDD_C_SUCCESS)
                  goto cleanup;
              };
              {
                char *tmp = NULL;
                rc = c_cdd_strdup("; return CDD_C_SUCCESS;", &tmp);
                if (rc != CDD_C_SUCCESS)
                  goto cleanup;
                rc = patch_list_add(&patches, semi, semi + 1, tmp);

                if (rc != CDD_C_SUCCESS)
                  goto cleanup;
              };
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
          {
            char *tmp = NULL;
            rc = c_cdd_strdup(" return CDD_C_SUCCESS; ", &tmp);
            if (rc != CDD_C_SUCCESS)
              goto cleanup;
            rc = patch_list_add(&patches, last, last, tmp);

            if (rc != CDD_C_SUCCESS)
              goto cleanup;
          };
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
      {
        char *tmp = NULL;
        rc = c_cdd_strdup("\n  cdd_c_error_t rc = CDD_C_SUCCESS;", &tmp);
        if (rc != CDD_C_SUCCESS)
          goto cleanup;
        rc = patch_list_add(&patches, k + 1, k + 1, tmp);

        if (rc != CDD_C_SUCCESS)
          goto cleanup;
      };
    }
  }

  /* 6. Execute Patching */
  rc = patch_list_apply(&patches, tokens, out_code);

cleanup:
  patch_list_free(&patches);
  return rc;
}
