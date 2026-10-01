/**
 * @file preprocessor_eval.c
 * @brief Preprocessor expression evaluator recursive-descent parser.
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

cdd_c_error_t pp_parse_primary(struct ExprState *s, long *out_val) {
  int matched = 0;
  int is_id_or_kw = 0;
  cdd_c_error_t rc;

#ifdef CDD_BUILD_TESTS
  if (g_cdd_pp_primary_fail && --g_cdd_pp_primary_fail == 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;
#endif

  if (!s || !out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  *out_val = 0;
  rc = skip_ws(s);
  if (rc != CDD_C_SUCCESS)
    return rc;

  if (s->pos >= s->end) {
    s->error = 1;
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  rc = match(s, TOKEN_LPAREN, &matched);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (matched) {
    long val = 0;
    rc = parse_expr(s, &val);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = match(s, TOKEN_RPAREN, &matched);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (!matched) {
      s->error = 1;
      return CDD_C_ERROR_INVALID_ARGUMENT;
    }
    *out_val = val;
    return CDD_C_SUCCESS;
  }

  if (s->tokens->tokens[s->pos].kind == TOKEN_NUMBER_LITERAL) {
    char *txt = NULL;
    long val = 0;

    rc = pp_token_to_string(&s->tokens->tokens[s->pos], &txt);
    if (rc != CDD_C_SUCCESS)
      return rc;

    if (strlen(txt) > 2 && txt[0] == '0' &&
        (tolower((unsigned char)txt[1]) == 'b')) {
      char *endptr;
      val = strtol(txt + 2, &endptr, 2);
    } else {
      val = strtol(txt, NULL, 0);
    }
    C_CDD_FREE(txt);
    s->pos++;
    *out_val = val;
    return CDD_C_SUCCESS;
  }

  if (s->tokens->tokens[s->pos].kind == TOKEN_IDENTIFIER)
    is_id_or_kw = 1;
  else if (s->tokens->tokens[s->pos].kind <= TOKEN_KEYWORD_DECLSPEC)
    is_id_or_kw = 1;

  if (is_id_or_kw) {
    const struct Token *tok = &s->tokens->tokens[s->pos];
    int is_match = 0;

    rc = token_matches_string(tok, "__has_include", &is_match);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (is_match) {
      s->pos++;
      return handle_has_include_embed(s, out_val);
    }

    rc = token_matches_string(tok, "__has_embed", &is_match);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (is_match) {
      s->pos++;
      return handle_has_include_embed(s, out_val);
    }

    rc = token_matches_string(tok, "__has_c_attribute", &is_match);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (is_match) {
      s->pos++;
      return handle_has_c_attribute(s, out_val);
    }

    /* Macro value lookup */
    {
      size_t i;
      long val = 0;

      if (s->ctx) {
        for (i = 0; i < s->ctx->macro_count; ++i) {
          if (!s->ctx->macros[i].is_function_like && s->ctx->macros[i].value) {
            rc = token_matches_string(tok, s->ctx->macros[i].name, &is_match);
            if (rc != CDD_C_SUCCESS)
              return rc;
            if (is_match) {
              char *endptr;
              val = strtol(s->ctx->macros[i].value, &endptr, 0);
              break;
            }
          }
        }
      }
      s->pos++;
      *out_val = val;
      return CDD_C_SUCCESS;
    }
  }

  s->error = 1;
  s->pos++;
  *out_val = 0;
  return CDD_C_ERROR_INVALID_ARGUMENT;
}

cdd_c_error_t pp_parse_unary(struct ExprState *s, long *out_val) {
  int matched = 0;
  cdd_c_error_t rc;

#ifdef CDD_BUILD_TESTS
  if (g_cdd_pp_unary_fail && --g_cdd_pp_unary_fail == 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;
#endif

  if (!s || !out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  *out_val = 0;
  rc = skip_ws(s);
  if (rc != CDD_C_SUCCESS)
    return rc;

  rc = match(s, TOKEN_BANG, &matched);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (matched) {
    long rhs = 0;
    rc = parse_unary(s, &rhs);
    if (rc != CDD_C_SUCCESS)
      return rc;
    *out_val = !rhs;
    return CDD_C_SUCCESS;
  }

  rc = match(s, TOKEN_TILDE, &matched);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (matched) {
    long rhs = 0;
    rc = parse_unary(s, &rhs);
    if (rc != CDD_C_SUCCESS)
      return rc;
    *out_val = ~rhs;
    return CDD_C_SUCCESS;
  }

  rc = match(s, TOKEN_MINUS, &matched);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (matched) {
    long rhs = 0;
    rc = parse_unary(s, &rhs);
    if (rc != CDD_C_SUCCESS)
      return rc;
    *out_val = -rhs;
    return CDD_C_SUCCESS;
  }

  rc = match(s, TOKEN_PLUS, &matched);
  if (rc != CDD_C_SUCCESS)
    return rc;
  if (matched) {
    long rhs = 0;
    rc = parse_unary(s, &rhs);
    if (rc != CDD_C_SUCCESS)
      return rc;
    *out_val = +rhs;
    return CDD_C_SUCCESS;
  }

  if (s->pos < s->end && s->tokens->tokens[s->pos].kind == TOKEN_IDENTIFIER) {
    int is_match = 0;
    rc = token_matches_string(&s->tokens->tokens[s->pos], "defined", &is_match);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (is_match) {
      int has_paren = 0;
      long result = 0;

      s->pos++;
      rc = skip_ws(s);
      if (rc != CDD_C_SUCCESS)
        return rc;

      rc = match(s, TOKEN_LPAREN, &matched);
      if (rc != CDD_C_SUCCESS)
        return rc;
      if (matched)
        has_paren = 1;

      rc = skip_ws(s);
      if (rc != CDD_C_SUCCESS)
        return rc;

      if (s->pos < s->end &&
          s->tokens->tokens[s->pos].kind == TOKEN_IDENTIFIER) {
        int def = 0;
        rc = pp_is_defined_macro(s->ctx, &s->tokens->tokens[s->pos], &def);
        if (rc != CDD_C_SUCCESS)
          return rc;
        if (def)
          result = 1;
        s->pos++;
      } else {
        s->error = 1;
        return CDD_C_ERROR_INVALID_ARGUMENT;
      }

      if (has_paren) {
        rc = match(s, TOKEN_RPAREN, &matched);
        if (rc != CDD_C_SUCCESS)
          return rc;
        if (!matched) {
          s->error = 1;
          return CDD_C_ERROR_INVALID_ARGUMENT;
        }
      }

      *out_val = result;
      return CDD_C_SUCCESS;
    }
  }

  return parse_primary(s, out_val);
}

cdd_c_error_t pp_parse_multiplicative(struct ExprState *s, long *out_val) {
  long val = 0;
  int matched = 0;
  cdd_c_error_t rc;

#ifdef CDD_BUILD_TESTS
  if (g_cdd_pp_multiplicative_fail && --g_cdd_pp_multiplicative_fail == 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;
#endif

  if (!s || !out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  *out_val = 0;

  rc = parse_unary(s, &val);
  if (rc != CDD_C_SUCCESS)
    return rc;

  while (s->pos < s->end && !s->error) {
    rc = match(s, TOKEN_STAR, &matched);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (matched) {
      long rhs = 0;
      rc = parse_unary(s, &rhs);
      if (rc != CDD_C_SUCCESS)
        return rc;
      val *= rhs;
      continue;
    }

    rc = match(s, TOKEN_SLASH, &matched);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (matched) {
      long divisor = 0;
      rc = parse_unary(s, &divisor);
      if (rc != CDD_C_SUCCESS)
        return rc;
      if (divisor == 0)
        val = 0;
      else
        val /= divisor;
      continue;
    }

    rc = match(s, TOKEN_PERCENT, &matched);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (matched) {
      long divisor = 0;
      rc = parse_unary(s, &divisor);
      if (rc != CDD_C_SUCCESS)
        return rc;
      if (divisor == 0)
        val = 0;
      else
        val %= divisor;
      continue;
    }

    break;
  }

  *out_val = val;
  return CDD_C_SUCCESS;
}

cdd_c_error_t pp_parse_additive(struct ExprState *s, long *out_val) {
  long val = 0;
  int matched = 0;
  cdd_c_error_t rc;

#ifdef CDD_BUILD_TESTS
  if (g_cdd_pp_additive_fail && --g_cdd_pp_additive_fail == 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;
#endif

  if (!s || !out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  *out_val = 0;

  rc = parse_multiplicative(s, &val);
  if (rc != CDD_C_SUCCESS)
    return rc;

  while (s->pos < s->end && !s->error) {
    rc = match(s, TOKEN_PLUS, &matched);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (matched) {
      long rhs = 0;
      rc = parse_multiplicative(s, &rhs);
      if (rc != CDD_C_SUCCESS)
        return rc;
      val += rhs;
      continue;
    }

    rc = match(s, TOKEN_MINUS, &matched);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (matched) {
      long rhs = 0;
      rc = parse_multiplicative(s, &rhs);
      if (rc != CDD_C_SUCCESS)
        return rc;
      val -= rhs;
      continue;
    }

    break;
  }

  *out_val = val;
  return CDD_C_SUCCESS;
}

cdd_c_error_t pp_parse_shift(struct ExprState *s, long *out_val) {
  long val = 0;
  int matched = 0;
  cdd_c_error_t rc;

#ifdef CDD_BUILD_TESTS
  if (g_cdd_pp_shift_fail && --g_cdd_pp_shift_fail == 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;
#endif

  if (!s || !out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  *out_val = 0;

  rc = parse_additive(s, &val);
  if (rc != CDD_C_SUCCESS)
    return rc;

  while (s->pos < s->end && !s->error) {
    rc = match(s, TOKEN_LSHIFT, &matched);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (matched) {
      long rhs = 0;
      rc = parse_additive(s, &rhs);
      if (rc != CDD_C_SUCCESS)
        return rc;
      val <<= rhs;
      continue;
    }

    rc = match(s, TOKEN_RSHIFT, &matched);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (matched) {
      long rhs = 0;
      rc = parse_additive(s, &rhs);
      if (rc != CDD_C_SUCCESS)
        return rc;
      val >>= rhs;
      continue;
    }

    break;
  }

  *out_val = val;
  return CDD_C_SUCCESS;
}

cdd_c_error_t pp_parse_relational(struct ExprState *s, long *out_val) {
  long val = 0;
  enum TokenKind k;
  int matched = 0;
  cdd_c_error_t rc;

#ifdef CDD_BUILD_TESTS
  if (g_cdd_pp_relational_fail && --g_cdd_pp_relational_fail == 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;
#endif

  if (!s || !out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  *out_val = 0;

  rc = parse_shift(s, &val);
  if (rc != CDD_C_SUCCESS)
    return rc;

  while (s->pos < s->end && !s->error) {
    rc = pp_preprocessor_peek(s, &k);
    if (rc != CDD_C_SUCCESS)
      return rc;

    if (k == TOKEN_LEQ) {
      long rhs = 0;
      rc = match(s, k, &matched);
      if (rc != CDD_C_SUCCESS)
        return rc;
      rc = parse_shift(s, &rhs);
      if (rc != CDD_C_SUCCESS)
        return rc;
      val = (val <= rhs);
    } else if (k == TOKEN_GEQ) {
      long rhs = 0;
      rc = match(s, k, &matched);
      if (rc != CDD_C_SUCCESS)
        return rc;
      rc = parse_shift(s, &rhs);
      if (rc != CDD_C_SUCCESS)
        return rc;
      val = (val >= rhs);
    } else if (k == TOKEN_LESS) {
      long rhs = 0;
      rc = match(s, k, &matched);
      if (rc != CDD_C_SUCCESS)
        return rc;
      rc = parse_shift(s, &rhs);
      if (rc != CDD_C_SUCCESS)
        return rc;
      val = (val < rhs);
    } else if (k == TOKEN_GREATER) {
      long rhs = 0;
      rc = match(s, k, &matched);
      if (rc != CDD_C_SUCCESS)
        return rc;
      rc = parse_shift(s, &rhs);
      if (rc != CDD_C_SUCCESS)
        return rc;
      val = (val > rhs);
    } else {
      break;
    }
  }

  *out_val = val;
  return CDD_C_SUCCESS;
}

cdd_c_error_t pp_parse_equality(struct ExprState *s, long *out_val) {
  long val = 0;
  int matched = 0;
  cdd_c_error_t rc;

#ifdef CDD_BUILD_TESTS
  if (g_cdd_pp_equality_fail && --g_cdd_pp_equality_fail == 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;
#endif

  if (!s || !out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  *out_val = 0;

  rc = parse_relational(s, &val);
  if (rc != CDD_C_SUCCESS)
    return rc;

  while (s->pos < s->end && !s->error) {
    rc = match(s, TOKEN_EQ, &matched);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (matched) {
      long rhs = 0;
      rc = parse_relational(s, &rhs);
      if (rc != CDD_C_SUCCESS)
        return rc;
      val = (val == rhs);
      continue;
    }

    rc = match(s, TOKEN_NEQ, &matched);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (matched) {
      long rhs = 0;
      rc = parse_relational(s, &rhs);
      if (rc != CDD_C_SUCCESS)
        return rc;
      val = (val != rhs);
      continue;
    }

    break;
  }

  *out_val = val;
  return CDD_C_SUCCESS;
}

cdd_c_error_t pp_parse_logic_and(struct ExprState *s, long *out_val) {
  long val = 0;
  int matched = 0;
  cdd_c_error_t rc;

#ifdef CDD_BUILD_TESTS
  if (g_cdd_pp_logic_and_fail && --g_cdd_pp_logic_and_fail == 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;
#endif

  if (!s || !out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  *out_val = 0;

  rc = parse_equality(s, &val);
  if (rc != CDD_C_SUCCESS)
    return rc;

  while (s->pos < s->end && !s->error) {
    rc = match(s, TOKEN_LOGICAL_AND, &matched);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (!matched)
      break;
    {
      long rhs = 0;
      rc = parse_equality(s, &rhs);
      if (rc != CDD_C_SUCCESS)
        return rc;
      val = (val && rhs);
    }
  }

  *out_val = val;
  return CDD_C_SUCCESS;
}

cdd_c_error_t pp_parse_logic_or(struct ExprState *s, long *out_val) {
  long val = 0;
  int matched = 0;
  cdd_c_error_t rc;

  if (!s || !out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  *out_val = 0;

  rc = parse_logic_and(s, &val);
  if (rc != CDD_C_SUCCESS)
    return rc;

  while (s->pos < s->end && !s->error) {
    rc = match(s, TOKEN_LOGICAL_OR, &matched);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (!matched)
      break;
    {
      long rhs = 0;
      rc = parse_logic_and(s, &rhs);
      if (rc != CDD_C_SUCCESS)
        return rc;
      val = (val || rhs);
    }
  }

  *out_val = val;
  return CDD_C_SUCCESS;
}

cdd_c_error_t pp_parse_expr(struct ExprState *s, long *out_val) {
  if (!s || !out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  return parse_logic_or(s, out_val);
}

cdd_c_error_t pp_eval_expression(const struct TokenList *tokens,
                                 size_t start_idx, size_t end_idx,
                                 const struct PreprocessorContext *ctx,
                                 long *result) {
  struct ExprState s;
  cdd_c_error_t rc;

#ifdef CDD_BUILD_TESTS
  if (g_cdd_pp_eval_expr_fail && --g_cdd_pp_eval_expr_fail == 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;
#endif

  if (!tokens || !result || start_idx > end_idx || end_idx > tokens->size)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  s.tokens = tokens;
  s.pos = start_idx;
  s.end = end_idx;
  s.ctx = ctx;
  s.error = 0;

  rc = parse_expr(&s, result);
  if (rc != CDD_C_SUCCESS)
    return rc;

  if (s.error)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  return CDD_C_SUCCESS;
}
