/**
 * @file preprocessor_directives.c
 * @brief Preprocessor directive parsing, introspection, and conditional stack
 * logic.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"
#include "c_cdd/memory.h"
#include "c_cdd/log.h"
#include "c_cdd_stdbool.h"

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "functions/parse/fs.h"
#include "functions/parse/preprocessor.h"
#include "functions/parse/preprocessor_internal.h"
#include "functions/parse/str.h"
#include "functions/parse/tokenizer.h"
#include "c_cdd/safe_crt.h"
/* clang-format on */

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
#ifndef strdup
#define strdup _strdup
#endif
#endif

/**
 * @brief Advances past whitespace tokens in an expression.
 *
 * @param[in,out] s Expression state.
 * @return CDD_C_SUCCESS.
 */
#ifdef CDD_BUILD_TESTS
cdd_c_error_t pp_expr_skip_ws(struct ExprState *s) {
  if (g_cdd_pp_skip_ws_fail && --g_cdd_pp_skip_ws_fail == 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;
#else
static cdd_c_error_t skip_ws(struct ExprState *s) {
#endif
  if (!s || !s->tokens)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  while (s->pos < s->end && s->tokens->tokens[s->pos].kind == TOKEN_WHITESPACE)
    s->pos++;
  return CDD_C_SUCCESS;
}

/**
 * @brief Matches and consumes a specific token kind if present.
 *
 * @param[in,out] s Expression state.
 * @param[in] kind Token kind to match.
 * @param[out] out_val Pointer to store 1 if matched, 0 otherwise.
 * @return CDD_C_SUCCESS.
 */
#ifdef CDD_BUILD_TESTS
cdd_c_error_t pp_expr_match(struct ExprState *s, enum TokenKind kind,
                            int *out_val) {
  cdd_c_error_t rc;
  if (g_cdd_pp_match_fail && --g_cdd_pp_match_fail == 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;
#else
static cdd_c_error_t match(struct ExprState *s, enum TokenKind kind,
                           int *out_val) {
  cdd_c_error_t rc;
#endif
  if (!s || !out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  rc = skip_ws(s);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (s->pos < s->end && s->tokens->tokens[s->pos].kind == kind) {
    s->pos++;
    *out_val = 1;
  } else {
    *out_val = 0;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Peeks at the next non-whitespace token kind.
 */
cdd_c_error_t pp_preprocessor_peek(struct ExprState *s,
                                   enum TokenKind *out_val) {
  size_t p;

#ifdef CDD_BUILD_TESTS
  if (g_cdd_pp_peek_fail && --g_cdd_pp_peek_fail == 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;
#endif

  if (!s || !out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  p = s->pos;
  while (p < s->end && s->tokens->tokens[p].kind == TOKEN_WHITESPACE)
    p++;
  if (p >= s->end)
    *out_val = TOKEN_UNKNOWN;
  else
    *out_val = s->tokens->tokens[p].kind;
  return CDD_C_SUCCESS;
}

cdd_c_error_t pp_is_defined_macro(const struct PreprocessorContext *ctx,
                                  const struct Token *tok, int *out_val) {
  size_t i;
  int is_match = 0;

#ifdef CDD_BUILD_TESTS
  if (g_cdd_pp_is_defined_macro_fail && --g_cdd_pp_is_defined_macro_fail == 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;
#endif

  if (!out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  *out_val = 0;

  if (!ctx || !tok)
    return CDD_C_SUCCESS;

  for (i = 0; i < ctx->macro_count; ++i) {
    cdd_c_error_t rc;
    rc = token_matches_string(tok, ctx->macros[i].name, &is_match);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (is_match) {
      *out_val = 1;
      return CDD_C_SUCCESS;
    }
  }

  return CDD_C_SUCCESS;
}

/**
 * @brief Handles __has_include and __has_embed preprocessor expressions.
 *
 * @param[in,out] s Expression state.
 * @param[out] out_val Pointer to store evaluation result (1 or 0).
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
#ifdef CDD_BUILD_TESTS
cdd_c_error_t pp_handle_has_include_embed(struct ExprState *s, long *out_val) {
#else
static cdd_c_error_t handle_has_include_embed(struct ExprState *s,
                                              long *out_val) {
#endif
  int is_header = 0;
  char *path = NULL;
  char *resolved = NULL;
  long result = 0;
  int matched = 0;
  cdd_c_error_t rc;

  if (!s || !out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  *out_val = 0;
  rc = skip_ws(s);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = match(s, TOKEN_LPAREN, &matched);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (!matched) {
    s->error = 1;
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  rc = skip_ws(s);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (s->pos >= s->end) {
    s->error = 1;
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  if (s->tokens->tokens[s->pos].kind == TOKEN_STRING_LITERAL) {
    const struct Token *t = &s->tokens->tokens[s->pos];
    path = (char *)C_CDD_MALLOC(t->length - 1);
    if (!path)
      return CDD_C_ERROR_MEMORY;
    memcpy(path, t->start + 1, t->length - 2);
    path[t->length - 2] = '\0';
    s->pos++;
  } else if (s->tokens->tokens[s->pos].kind == TOKEN_LESS) {
    size_t start_p = s->pos + 1;
    size_t end_p = start_p;
    while (end_p < s->end && s->tokens->tokens[end_p].kind != TOKEN_GREATER)
      end_p++;

    if (end_p < s->end) {
      rc = pp_reconstruct_path(s->tokens, start_p, end_p, &path);
      if (rc != CDD_C_SUCCESS) {
        s->error = 1;
        return rc;
      }
      s->pos = end_p + 1;
      is_header = 1;
    } else {
      s->error = 1;
      return CDD_C_SUCCESS;
    }
  } else {
    s->error = 1;
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  while (s->pos < s->end) {
    if (s->tokens->tokens[s->pos].kind == TOKEN_RPAREN)
      break;
    s->pos++;
  }

  rc = skip_ws(s);
  if (rc != CDD_C_SUCCESS) {
    C_CDD_FREE(path);
    return rc;
  }
  rc = match(s, TOKEN_RPAREN, &matched);
  if (rc != CDD_C_SUCCESS) {
    C_CDD_FREE(path);
    return rc;
  }
  if (!matched) {
    s->error = 1;
    C_CDD_FREE(path);
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  rc = pp_resolve_path(s->ctx, s->ctx ? s->ctx->current_file_dir : NULL, path,
                       is_header, &resolved);
  if (rc != CDD_C_SUCCESS) {
    C_CDD_FREE(path);
    return rc;
  }
  result = (resolved != NULL) ? 1 : 0;
  if (resolved)
    C_CDD_FREE(resolved);
  C_CDD_FREE(path);

  *out_val = result;
  return CDD_C_SUCCESS;
}

/**
 * @brief Handles __has_c_attribute preprocessor expressions.
 *
 * @param[in,out] s Expression state.
 * @param[out] out_val Pointer to store attribute version or 0.
 * @return CDD_C_SUCCESS on success, error code on failure.
 */
#ifdef CDD_BUILD_TESTS
cdd_c_error_t pp_handle_has_c_attribute(struct ExprState *s, long *out_val) {
#else
static cdd_c_error_t handle_has_c_attribute(struct ExprState *s,
                                            long *out_val) {
#endif
  long result = 0;
  char *attr_name = NULL;
  int matched = 0;
  enum TokenKind kw_kind;
  cdd_c_error_t rc;

  if (!s || !out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  *out_val = 0;
  rc = skip_ws(s);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = match(s, TOKEN_LPAREN, &matched);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (!matched) {
    s->error = 1;
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  rc = skip_ws(s);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (s->pos < s->end) {
    if (s->tokens->tokens[s->pos].kind == TOKEN_IDENTIFIER) {
      rc = pp_token_to_string(&s->tokens->tokens[s->pos], &attr_name);
      if (rc != CDD_C_SUCCESS)
        return rc;
      s->pos++;
    } else {
      rc = identify_keyword_or_id(s->tokens->tokens[s->pos].start,
                                  s->tokens->tokens[s->pos].length, &kw_kind);
      if (rc != CDD_C_SUCCESS)
        return rc;
      if (kw_kind != TOKEN_IDENTIFIER) {
        rc = pp_token_to_string(&s->tokens->tokens[s->pos], &attr_name);
        if (rc != CDD_C_SUCCESS)
          return rc;
        s->pos++;
      }
    }
  }

  rc = skip_ws(s);
  if (rc != CDD_C_SUCCESS) {
    C_CDD_FREE(attr_name);
    return rc;
  }

  if (s->pos + 1 < s->end) {
    if (s->tokens->tokens[s->pos].kind == TOKEN_COLON &&
        s->tokens->tokens[s->pos + 1].kind == TOKEN_COLON) {
      char *scope = attr_name;
      char *name = NULL;
      s->pos += 2;
      rc = skip_ws(s);
      if (rc != CDD_C_SUCCESS) {
        C_CDD_FREE(scope);
        return rc;
      }
      if (s->pos < s->end) {
        if (s->tokens->tokens[s->pos].kind == TOKEN_IDENTIFIER) {
          rc = pp_token_to_string(&s->tokens->tokens[s->pos], &name);
          if (rc != CDD_C_SUCCESS) {
            C_CDD_FREE(scope);
            return rc;
          }
          C_CDD_FREE(name);
          s->pos++;
        }
      }
      C_CDD_FREE(scope);
      attr_name = NULL;
      result = 0;
    }
  }

  if (attr_name) {
    if (strcmp(attr_name, "deprecated") == 0)
      result = 201904L;
    else if (strcmp(attr_name, "fallthrough") == 0)
      result = 201904L;
    else if (strcmp(attr_name, "maybe_unused") == 0)
      result = 201904L;
    else if (strcmp(attr_name, "nodiscard") == 0)
      result = 201904L;
    else if (strcmp(attr_name, "noreturn") == 0)
      result = 202202L;
    else if (strcmp(attr_name, "unsequenced") == 0)
      result = 202311L;
    else if (strcmp(attr_name, "reproducible") == 0)
      result = 202311L;
    else
      result = 0;
    C_CDD_FREE(attr_name);
  }

  rc = skip_ws(s);
  if (rc != CDD_C_SUCCESS)
    return rc;
  rc = match(s, TOKEN_RPAREN, &matched);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (!matched) {
    s->error = 1;
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  *out_val = result;
  return CDD_C_SUCCESS;
}

/* --- Embed Directive Parsing --- */

cdd_c_error_t pp_parse_embed_params(const struct TokenList *tokens,
                                    size_t start, size_t end,
                                    struct PreprocessorContext *ctx,
                                    struct EmbedParams *out_params) {
  size_t i;
  enum TokenKind kw_kind;
  cdd_c_error_t rc;

  if (!tokens || !out_params)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  i = start;
  while (i < end) {
    char *name = NULL;
    char *scope = NULL;

    while (i < end && tokens->tokens[i].kind == TOKEN_WHITESPACE)
      i++;

    if (i >= end)
      break;

    if (tokens->tokens[i].kind != TOKEN_IDENTIFIER) {
      rc = identify_keyword_or_id(tokens->tokens[i].start,
                                  tokens->tokens[i].length, &kw_kind);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }

    rc = pp_token_to_string(&tokens->tokens[i], &name);
    if (rc != CDD_C_SUCCESS)
      return rc;

    i++;

    while (i < end && tokens->tokens[i].kind == TOKEN_WHITESPACE)
      i++;

    if (i + 1 < end) {
      if (tokens->tokens[i].kind == TOKEN_COLON &&
          tokens->tokens[i + 1].kind == TOKEN_COLON) {
        scope = name;
        name = NULL;
        i += 2;

        while (i < end && tokens->tokens[i].kind == TOKEN_WHITESPACE)
          i++;

        if (i < end) {
          if (tokens->tokens[i].kind == TOKEN_IDENTIFIER) {
            rc = pp_token_to_string(&tokens->tokens[i], &name);
            if (rc != CDD_C_SUCCESS) {
              C_CDD_FREE(scope);
              return rc;
            }
            i++;
          } else {
            C_CDD_FREE(scope);
            return CDD_C_ERROR_INVALID_ARGUMENT;
          }
        } else {
          C_CDD_FREE(scope);
          return CDD_C_ERROR_INVALID_ARGUMENT;
        }
      }
    }

    while (i < end && tokens->tokens[i].kind == TOKEN_WHITESPACE)
      i++;

    if (i >= end || tokens->tokens[i].kind != TOKEN_LPAREN) {
      C_CDD_FREE(name);
      if (scope)
        C_CDD_FREE(scope);
      return CDD_C_ERROR_INVALID_ARGUMENT;
    }

    {
      size_t open_idx = i;
      size_t close_idx = 0;
      int depth = 1;
      i++;

      while (i < end) {
        if (tokens->tokens[i].kind == TOKEN_LPAREN)
          depth++;
        else if (tokens->tokens[i].kind == TOKEN_RPAREN) {
          depth--;
          if (depth == 0) {
            close_idx = i;
            break;
          }
        }
        i++;
      }

      if (depth != 0) {
        C_CDD_FREE(name);
        if (scope)
          C_CDD_FREE(scope);
        return CDD_C_ERROR_INVALID_ARGUMENT;
      }

      if (strcmp(name, "limit") == 0 && !scope) {
        long val = 0;
        rc = pp_eval_expression(tokens, open_idx + 1, close_idx, ctx, &val);
        if (rc != CDD_C_SUCCESS) {
          C_CDD_FREE(name);
          return rc;
        }
        out_params->limit = val;
      } else if (strcmp(name, "prefix") == 0 && !scope) {
        rc = pp_reconstruct_path(tokens, open_idx + 1, close_idx,
                                 &out_params->prefix);
        if (rc != CDD_C_SUCCESS) {
          C_CDD_FREE(name);
          return rc;
        }
      } else if (strcmp(name, "suffix") == 0 && !scope) {
        rc = pp_reconstruct_path(tokens, open_idx + 1, close_idx,
                                 &out_params->suffix);
        if (rc != CDD_C_SUCCESS) {
          C_CDD_FREE(name);
          return rc;
        }
      } else if (strcmp(name, "if_empty") == 0 && !scope) {
        rc = pp_reconstruct_path(tokens, open_idx + 1, close_idx,
                                 &out_params->if_empty);
        if (rc != CDD_C_SUCCESS) {
          C_CDD_FREE(name);
          return rc;
        }
      }
    }

    C_CDD_FREE(name);
    if (scope)
      C_CDD_FREE(scope);
    i++;
  }

  return CDD_C_SUCCESS;
}

/* --- Include Scanning & Conditional Logic --- */

cdd_c_error_t pp_stack_push(struct ConditionalStack *st, enum CondState s) {
  if (!st)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (st->top < 31)
    st->states[++st->top] = s;

  return CDD_C_SUCCESS;
}

cdd_c_error_t pp_stack_pop(struct ConditionalStack *st) {
  if (!st)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (st->top >= 0)
    st->top--;

  return CDD_C_SUCCESS;
}

cdd_c_error_t pp_stack_peek(const struct ConditionalStack *st,
                            enum CondState *out_val) {
  if (!st || !out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (st->top >= 0)
    *out_val = st->states[st->top];
  else
    *out_val = COND_ACTIVE;

  return CDD_C_SUCCESS;
}

cdd_c_error_t pp_is_enabled(const struct ConditionalStack *st, int *out_val) {
  int i;

  if (!st || !out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  for (i = 0; i <= st->top; ++i) {
    if (st->states[i] == COND_SKIPPING || st->states[i] == COND_SATISFIED) {
      *out_val = 0;
      return CDD_C_SUCCESS;
    }
  }

  *out_val = 1;
  return CDD_C_SUCCESS;
}
