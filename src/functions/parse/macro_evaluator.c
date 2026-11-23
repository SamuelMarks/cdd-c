/**
 * @file macro_evaluator.c
 * @brief Core macro evaluator definitions and implementation.
 */
/* clang-format off */
#include "c_cdd/memory.h"
#include "c_cdd/safe_crt_msvc.h"

#include "classes/parse/numeric.h"
#include "macro_evaluator.h"
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

/** @brief Token kinds for the preprocessor constant expression evaluator */
typedef enum {
  TOK_EOF,
  TOK_INT,
  TOK_FLOAT,
  TOK_STR,
  TOK_IDENT,
  TOK_PLUS,
  TOK_MINUS,
  TOK_STAR,
  TOK_SLASH,
  TOK_PERCENT,
  TOK_AMP,
  TOK_PIPE,
  TOK_CARET,
  TOK_TILDE,
  TOK_BANG,
  TOK_AMPAMP,
  TOK_PIPEPIPE,
  TOK_LSHIFT,
  TOK_RSHIFT,
  TOK_EQEQ,
  TOK_NEQ,
  TOK_LT,
  TOK_LTE,
  TOK_GT,
  TOK_GTE,
  TOK_LPAREN,
  TOK_RPAREN,
  TOK_ERROR
} macro_tok_kind_t;

/**
 * @brief Represents a token produced during macro expression evaluation.
 */
typedef struct {
  macro_tok_kind_t kind; /**< Token kind */
  int64_t int_val;       /**< Evaluated integer value */
  double float_val;      /**< Evaluated floating point value */
  char *str_val;         /**< Evaluated string value */
} macro_tok_t;

/**
 * @brief Lexer state for macro expression tokenization.
 */
typedef struct {
  const char *str; /**< Input string being scanned */
  size_t pos;      /**< Current character position */
  size_t len;      /**< Total string length */
  macro_tok_t cur; /**< Current token */
} macro_lexer_t;

/**
 * @brief Frees any dynamically allocated string payload in `tok`.
 *
 * @param[in,out] tok Macro token whose payload will be freed.
 * @return CDD_C_SUCCESS.
 */
static cdd_c_error_t macro_tok_free(macro_tok_t *tok) {
  if (tok->kind == TOK_STR || tok->kind == TOK_IDENT) {
    C_CDD_FREE(tok->str_val);
    tok->str_val = NULL;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Advances the macro lexer to the next token in `lex->str`.
 *
 * @param[in,out] lex Macro lexer state.
 * @return `CDD_C_SUCCESS` on success.
 */
static cdd_c_error_t next_tok(macro_lexer_t *lex) {
  cdd_c_error_t rc_free;
  cdd_c_error_t rc;
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  rc_free = macro_tok_free(&lex->cur);

  while (lex->pos < lex->len && isspace((unsigned char)lex->str[lex->pos])) {
    lex->pos++;
  }

  if (lex->pos >= lex->len) {
    lex->cur.kind = TOK_EOF;
    return cdd_c_keep_err(CDD_C_SUCCESS, rc_free);
  }

  {
    char c = lex->str[lex->pos];
    char nc = (lex->pos + 1 < lex->len) ? lex->str[lex->pos + 1] : '\0';

    if (isdigit((unsigned char)c) || c == '.') {
      size_t start = lex->pos;
      int is_float = 0;
      int is_hex = 0;
      if (c == '0' && (nc == 'x' || nc == 'X')) {
        is_hex = 1;
        lex->pos += 2;
      }
      while (lex->pos < lex->len) {
        char ch = lex->str[lex->pos];
        if (is_hex) {
          if (isxdigit((unsigned char)ch))
            lex->pos++;
          else
            break;
        } else {
          if (isdigit((unsigned char)ch))
            lex->pos++;
          else if (ch == '.') {
            is_float = 1;
            lex->pos++;
          } else if (ch == 'e' || ch == 'E') {
            is_float = 1;
            lex->pos++;
            if (lex->str[lex->pos] == '+' || lex->str[lex->pos] == '-')
              lex->pos++;
          } else
            break;
        }
      }
      /* parse suffix like U, L, LL, f */
      while (lex->pos < lex->len) {
        char ch = lex->str[lex->pos];
        if (ch == 'u' || ch == 'U' || ch == 'l' || ch == 'L' || ch == 'f' ||
            ch == 'F')
          lex->pos++;
        else
          break;
      }

      {
        size_t len = lex->pos - start;
        char *buf = (char *)C_CDD_MALLOC(len + 1);
        if (!buf) {
          lex->cur.kind = TOK_ERROR;
          return cdd_c_keep_err(CDD_C_SUCCESS, rc_free);
        }
        memcpy(buf, lex->str + start, len);
        buf[len] = '\0';
        if (is_float) {
          lex->cur.kind = TOK_FLOAT;
          lex->cur.float_val = strtod(buf, NULL);
        } else {
          struct NumericValue nv;
          lex->cur.kind = TOK_INT;
          rc = parse_numeric_literal(buf, &nv);
          if (rc != CDD_C_SUCCESS) {
            C_CDD_FREE(buf);
            return cdd_c_keep_err(rc, rc_free);
          }
          lex->cur.int_val = (int64_t)nv.data.integer.value;
        }
        C_CDD_FREE(buf);
      }
      return cdd_c_keep_err(CDD_C_SUCCESS, rc_free);
    }

    if (isalpha((unsigned char)c) || c == '_') {
      size_t start = lex->pos;
      while (lex->pos < lex->len &&
             (isalnum((unsigned char)lex->str[lex->pos]) ||
              lex->str[lex->pos] == '_')) {
        lex->pos++;
      }
      {
        size_t len = lex->pos - start;
        lex->cur.kind = TOK_IDENT;
        lex->cur.str_val = (char *)C_CDD_MALLOC(len + 1);
        if (!lex->cur.str_val) {
          lex->cur.kind = TOK_ERROR;
          return CDD_C_SUCCESS;
        }
        memcpy(lex->cur.str_val, lex->str + start, len);
        lex->cur.str_val[len] = '\0';
      }
      return CDD_C_SUCCESS;
    }

    if (c == '"') {
      size_t start = ++lex->pos;
      while (lex->pos < lex->len && lex->str[lex->pos] != '"') {
        if (lex->str[lex->pos] == '\\')
          lex->pos++; /* skip escaped */
        lex->pos++;
      }
      if (lex->pos < lex->len) {
        size_t len = lex->pos - start;
        lex->cur.kind = TOK_STR;
        lex->cur.str_val = (char *)C_CDD_MALLOC(len + 1);
        if (!lex->cur.str_val) {
          lex->cur.kind = TOK_ERROR;
          return CDD_C_SUCCESS;
        }
        memcpy(lex->cur.str_val, lex->str + start, len);
        lex->cur.str_val[len] = '\0';
        lex->pos++;
      } else {
        lex->cur.kind = TOK_ERROR;
      }
      return CDD_C_SUCCESS;
    }

    lex->pos++;
    switch (c) {
    case '+':
      lex->cur.kind = TOK_PLUS;
      return CDD_C_SUCCESS;
    case '-':
      lex->cur.kind = TOK_MINUS;
      return CDD_C_SUCCESS;
    case '*':
      lex->cur.kind = TOK_STAR;
      return CDD_C_SUCCESS;
    case '/':
      lex->cur.kind = TOK_SLASH;
      return CDD_C_SUCCESS;
    case '%':
      lex->cur.kind = TOK_PERCENT;
      return CDD_C_SUCCESS;
    case '~':
      lex->cur.kind = TOK_TILDE;
      return CDD_C_SUCCESS;
    case '^':
      lex->cur.kind = TOK_CARET;
      return CDD_C_SUCCESS;
    case '(':
      lex->cur.kind = TOK_LPAREN;
      return CDD_C_SUCCESS;
    case ')':
      lex->cur.kind = TOK_RPAREN;
      return CDD_C_SUCCESS;
    case '&':
      if (nc == '&') {
        lex->pos++;
        lex->cur.kind = TOK_AMPAMP;
      } else {
        lex->cur.kind = TOK_AMP;
      }
      return CDD_C_SUCCESS;
    case '|':
      if (nc == '|') {
        lex->pos++;
        lex->cur.kind = TOK_PIPEPIPE;
      } else {
        lex->cur.kind = TOK_PIPE;
      }
      return CDD_C_SUCCESS;
    case '<':
      if (nc == '<') {
        lex->pos++;
        lex->cur.kind = TOK_LSHIFT;
      } else if (nc == '=') {
        lex->pos++;
        lex->cur.kind = TOK_LTE;
      } else {
        lex->cur.kind = TOK_LT;
      }
      return CDD_C_SUCCESS;
    case '>':
      if (nc == '>') {
        lex->pos++;
        lex->cur.kind = TOK_RSHIFT;
      } else if (nc == '=') {
        lex->pos++;
        lex->cur.kind = TOK_GTE;
      } else {
        lex->cur.kind = TOK_GT;
      }
      return CDD_C_SUCCESS;
    case '=':
      if (nc == '=') {
        lex->pos++;
        lex->cur.kind = TOK_EQEQ;
      } else {
        lex->cur.kind = TOK_ERROR;
      }
      return CDD_C_SUCCESS;
    case '!':
      if (nc == '=') {
        lex->pos++;
        lex->cur.kind = TOK_NEQ;
      } else {
        lex->cur.kind = TOK_BANG;
      }
      return CDD_C_SUCCESS;
    default:
      lex->cur.kind = TOK_ERROR;
      return CDD_C_SUCCESS;
    }
  }
}

/**
 * @brief Recursive descent parser state for macro expressions.
 */
typedef struct {
  macro_lexer_t lex;               /**< Underlying macro lexer */
  struct PreprocessorContext *ctx; /**< Active preprocessor context */
  int err; /**< Error flag: 0 on success, non-zero on error */
} parser_t;

/**
 * @brief Parses a top-level macro expression.
 *
 * @param[in,out] p Macro parser state.
 * @param[out] out Evaluated expression result.
 * @return `CDD_C_SUCCESS` on success.
 */
static cdd_c_error_t parse_expr(parser_t *p, cdd_macro_eval_result_t *out);

/**
 * @brief Parses a logical OR (`||`) macro expression.
 *
 * @param[in,out] p Macro parser state.
 * @param[out] out Evaluated expression result.
 * @return `CDD_C_SUCCESS` on success.
 */
static cdd_c_error_t parse_logical_or(parser_t *p,
                                      cdd_macro_eval_result_t *out);

/**
 * @brief Parses a logical AND (`&&`) macro expression.
 *
 * @param[in,out] p Macro parser state.
 * @param[out] out Evaluated expression result.
 * @return `CDD_C_SUCCESS` on success.
 */
static cdd_c_error_t parse_logical_and(parser_t *p,
                                       cdd_macro_eval_result_t *out);

/**
 * @brief Parses a bitwise OR (`|`) macro expression.
 *
 * @param[in,out] p Macro parser state.
 * @param[out] out Evaluated expression result.
 * @return `CDD_C_SUCCESS` on success.
 */
static cdd_c_error_t parse_bitwise_or(parser_t *p,
                                      cdd_macro_eval_result_t *out);

/**
 * @brief Parses a bitwise XOR (`^`) macro expression.
 *
 * @param[in,out] p Macro parser state.
 * @param[out] out Evaluated expression result.
 * @return `CDD_C_SUCCESS` on success.
 */
static cdd_c_error_t parse_bitwise_xor(parser_t *p,
                                       cdd_macro_eval_result_t *out);

/**
 * @brief Parses a bitwise AND (`&`) macro expression.
 *
 * @param[in,out] p Macro parser state.
 * @param[out] out Evaluated expression result.
 * @return `CDD_C_SUCCESS` on success.
 */
static cdd_c_error_t parse_bitwise_and(parser_t *p,
                                       cdd_macro_eval_result_t *out);

/**
 * @brief Parses an equality (`==`, `!=`) macro expression.
 *
 * @param[in,out] p Macro parser state.
 * @param[out] out Evaluated expression result.
 * @return `CDD_C_SUCCESS` on success.
 */
static cdd_c_error_t parse_equality(parser_t *p, cdd_macro_eval_result_t *out);

/**
 * @brief Parses a relational (`<`, `<=`, `>`, `>=`) macro expression.
 *
 * @param[in,out] p Macro parser state.
 * @param[out] out Evaluated expression result.
 * @return `CDD_C_SUCCESS` on success.
 */
static cdd_c_error_t parse_relational(parser_t *p,
                                      cdd_macro_eval_result_t *out);

/**
 * @brief Parses a bitwise shift (`<<`, `>>`) macro expression.
 *
 * @param[in,out] p Macro parser state.
 * @param[out] out Evaluated expression result.
 * @return `CDD_C_SUCCESS` on success.
 */
static cdd_c_error_t parse_shift(parser_t *p, cdd_macro_eval_result_t *out);

/**
 * @brief Parses an additive (`+`, `-`) macro expression.
 *
 * @param[in,out] p Macro parser state.
 * @param[out] out Evaluated expression result.
 * @return `CDD_C_SUCCESS` on success.
 */
static cdd_c_error_t parse_additive(parser_t *p, cdd_macro_eval_result_t *out);

/**
 * @brief Parses a multiplicative (`*`, `/`, `%`) macro expression.
 *
 * @param[in,out] p Macro parser state.
 * @param[out] out Evaluated expression result.
 * @return `CDD_C_SUCCESS` on success.
 */
static cdd_c_error_t parse_multiplicative(parser_t *p,
                                          cdd_macro_eval_result_t *out);

/**
 * @brief Parses a unary (`+`, `-`, `~`, `!`) macro expression.
 *
 * @param[in,out] p Macro parser state.
 * @param[out] out Evaluated expression result.
 * @return `CDD_C_SUCCESS` on success.
 */
static cdd_c_error_t parse_unary(parser_t *p, cdd_macro_eval_result_t *out);

/**
 * @brief Parses a primary literal, identifier, or parenthesized macro
 * expression.
 *
 * @param[in,out] p Macro parser state.
 * @param[out] out Evaluated expression result.
 * @return `CDD_C_SUCCESS` on success.
 */
static cdd_c_error_t parse_primary(parser_t *p, cdd_macro_eval_result_t *out);

/**
 * @brief Initializes `r` as an integer macro evaluation result.
 *
 * @param[in] v 64-bit integer value.
 * @param[out] r Evaluation result structure to populate.
 * @return `CDD_C_SUCCESS` on success.
 */
static cdd_c_error_t make_int(int64_t v, cdd_macro_eval_result_t *r) {
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  memset(r, 0, sizeof(*r));
  r->type = MACRO_EVAL_TYPE_INT;
  r->int_val = v;
  return CDD_C_SUCCESS;
}

/**
 * @brief Initializes `r` as a floating-point macro evaluation result.
 *
 * @param[in] v Double-precision floating-point value.
 * @param[out] r Evaluation result structure to populate.
 * @return `CDD_C_SUCCESS` on success.
 */
static cdd_c_error_t make_float(double v, cdd_macro_eval_result_t *r) {
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  memset(r, 0, sizeof(*r));
  r->type = MACRO_EVAL_TYPE_FLOAT;
  r->float_val = v;
  return CDD_C_SUCCESS;
}

/**
 * @brief Promotes mixed integer/float operands to floating-point.
 *
 * @param[in,out] a Left operand evaluation result.
 * @param[in,out] b Right operand evaluation result.
 * @return `CDD_C_SUCCESS` on success.
 */
static cdd_c_error_t promote(cdd_macro_eval_result_t *a,
                             cdd_macro_eval_result_t *b) {
#ifdef CDD_BUILD_TESTS
  if (g_cdd_step_fail > 0 && --g_cdd_step_fail == 0)
    return CDD_C_ERROR_UNKNOWN;
#endif
  if (a->type == MACRO_EVAL_TYPE_FLOAT && b->type == MACRO_EVAL_TYPE_INT) {
    b->type = MACRO_EVAL_TYPE_FLOAT;
    b->float_val = (double)b->int_val;
  } else if (a->type == MACRO_EVAL_TYPE_INT &&
             b->type == MACRO_EVAL_TYPE_FLOAT) {
    a->type = MACRO_EVAL_TYPE_FLOAT;
    a->float_val = (double)a->int_val;
  }
  return CDD_C_SUCCESS;
}

static cdd_c_error_t parse_expr(parser_t *p, cdd_macro_eval_result_t *out) {
  return parse_logical_or(p, out);
}

static cdd_c_error_t parse_logical_or(parser_t *p,
                                      cdd_macro_eval_result_t *out) {
  cdd_macro_eval_result_t left;
  cdd_c_error_t rc;
  memset(&left, 0, sizeof(left));
  rc = parse_logical_and(p, &left);
  if (rc != CDD_C_SUCCESS)
    return rc;

  while (p->lex.cur.kind == TOK_PIPEPIPE) {
    cdd_macro_eval_result_t right;
    int lv, rv;
    memset(&right, 0, sizeof(right));
    rc = next_tok(&p->lex);
    if (rc != CDD_C_SUCCESS) {
      cdd_c_error_t rc_f1 = cdd_macro_eval_result_free(&left);
      return cdd_c_keep_err(rc, rc_f1);
    }
    rc = parse_logical_and(p, &right);
    if (rc != CDD_C_SUCCESS) {
      cdd_c_error_t rc_f1 = cdd_macro_eval_result_free(&left);
      return cdd_c_keep_err(rc, rc_f1);
    }
    lv = (left.type == MACRO_EVAL_TYPE_INT ? (left.int_val != 0)
                                           : (left.float_val != 0.0));
    rv = (right.type == MACRO_EVAL_TYPE_INT ? (right.int_val != 0)
                                            : (right.float_val != 0.0));
    {
      cdd_c_error_t rc_f1 = cdd_macro_eval_result_free(&left);
      cdd_c_error_t rc_f2 = cdd_macro_eval_result_free(&right);
      rc = make_int(lv || rv, &left);
      if (rc != CDD_C_SUCCESS) {
        return cdd_c_keep_err(rc, cdd_c_keep_err(rc_f1, rc_f2));
      }
    }
  }
  *out = left;
  return CDD_C_SUCCESS;
}

static cdd_c_error_t parse_logical_and(parser_t *p,
                                       cdd_macro_eval_result_t *out) {
  cdd_macro_eval_result_t left;
  cdd_c_error_t rc;
  memset(&left, 0, sizeof(left));
  rc = parse_bitwise_or(p, &left);
  if (rc != CDD_C_SUCCESS)
    return rc;

  while (p->lex.cur.kind == TOK_AMPAMP) {
    cdd_macro_eval_result_t right;
    int lv, rv;
    memset(&right, 0, sizeof(right));
    rc = next_tok(&p->lex);
    if (rc != CDD_C_SUCCESS) {
      cdd_c_error_t rc_f1 = cdd_macro_eval_result_free(&left);
      return cdd_c_keep_err(rc, rc_f1);
    }
    rc = parse_bitwise_or(p, &right);
    if (rc != CDD_C_SUCCESS) {
      cdd_c_error_t rc_f1 = cdd_macro_eval_result_free(&left);
      return cdd_c_keep_err(rc, rc_f1);
    }
    lv = (left.type == MACRO_EVAL_TYPE_INT ? (left.int_val != 0)
                                           : (left.float_val != 0.0));
    rv = (right.type == MACRO_EVAL_TYPE_INT ? (right.int_val != 0)
                                            : (right.float_val != 0.0));
    {
      cdd_c_error_t rc_f1 = cdd_macro_eval_result_free(&left);
      cdd_c_error_t rc_f2 = cdd_macro_eval_result_free(&right);
      rc = make_int(lv && rv, &left);
      if (rc != CDD_C_SUCCESS) {
        return cdd_c_keep_err(rc, cdd_c_keep_err(rc_f1, rc_f2));
      }
    }
  }
  *out = left;
  return CDD_C_SUCCESS;
}

static cdd_c_error_t parse_bitwise_or(parser_t *p,
                                      cdd_macro_eval_result_t *out) {
  cdd_macro_eval_result_t left;
  cdd_c_error_t rc;
  memset(&left, 0, sizeof(left));
  rc = parse_bitwise_xor(p, &left);
  if (rc != CDD_C_SUCCESS)
    return rc;

  while (p->lex.cur.kind == TOK_PIPE) {
    cdd_macro_eval_result_t right;
    memset(&right, 0, sizeof(right));
    rc = next_tok(&p->lex);
    if (rc != CDD_C_SUCCESS) {
      cdd_c_error_t rc_f1 = cdd_macro_eval_result_free(&left);
      return cdd_c_keep_err(rc, rc_f1);
    }
    rc = parse_bitwise_xor(p, &right);
    if (rc != CDD_C_SUCCESS) {
      cdd_c_error_t rc_f1 = cdd_macro_eval_result_free(&left);
      return cdd_c_keep_err(rc, rc_f1);
    }
    if (left.type != MACRO_EVAL_TYPE_INT || right.type != MACRO_EVAL_TYPE_INT) {
      cdd_c_error_t rc_f1 = cdd_macro_eval_result_free(&left);
      cdd_c_error_t rc_f2 = cdd_macro_eval_result_free(&right);
      return cdd_c_keep_err(CDD_C_ERROR_INVALID_ARGUMENT,
                            cdd_c_keep_err(rc_f1, rc_f2));
    }
    rc = make_int(left.int_val | right.int_val, &left);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  *out = left;
  return CDD_C_SUCCESS;
}

static cdd_c_error_t parse_bitwise_xor(parser_t *p,
                                       cdd_macro_eval_result_t *out) {
  cdd_macro_eval_result_t left;
  cdd_c_error_t rc;
  memset(&left, 0, sizeof(left));
  rc = parse_bitwise_and(p, &left);
  if (rc != CDD_C_SUCCESS)
    return rc;

  while (p->lex.cur.kind == TOK_CARET) {
    cdd_macro_eval_result_t right;
    memset(&right, 0, sizeof(right));
    rc = next_tok(&p->lex);
    if (rc != CDD_C_SUCCESS) {
      cdd_c_error_t rc_f1 = cdd_macro_eval_result_free(&left);
      return cdd_c_keep_err(rc, rc_f1);
    }
    rc = parse_bitwise_and(p, &right);
    if (rc != CDD_C_SUCCESS) {
      cdd_c_error_t rc_f1 = cdd_macro_eval_result_free(&left);
      return cdd_c_keep_err(rc, rc_f1);
    }
    if (left.type != MACRO_EVAL_TYPE_INT || right.type != MACRO_EVAL_TYPE_INT) {
      cdd_c_error_t rc_f1 = cdd_macro_eval_result_free(&left);
      cdd_c_error_t rc_f2 = cdd_macro_eval_result_free(&right);
      return cdd_c_keep_err(CDD_C_ERROR_INVALID_ARGUMENT,
                            cdd_c_keep_err(rc_f1, rc_f2));
    }
    rc = make_int(left.int_val ^ right.int_val, &left);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  *out = left;
  return CDD_C_SUCCESS;
}

static cdd_c_error_t parse_bitwise_and(parser_t *p,
                                       cdd_macro_eval_result_t *out) {
  cdd_macro_eval_result_t left;
  cdd_c_error_t rc;
  memset(&left, 0, sizeof(left));
  rc = parse_equality(p, &left);
  if (rc != CDD_C_SUCCESS)
    return rc;

  while (p->lex.cur.kind == TOK_AMP) {
    cdd_macro_eval_result_t right;
    memset(&right, 0, sizeof(right));
    rc = next_tok(&p->lex);
    if (rc != CDD_C_SUCCESS) {
      cdd_c_error_t rc_f1 = cdd_macro_eval_result_free(&left);
      return cdd_c_keep_err(rc, rc_f1);
    }
    rc = parse_equality(p, &right);
    if (rc != CDD_C_SUCCESS) {
      cdd_c_error_t rc_f1 = cdd_macro_eval_result_free(&left);
      return cdd_c_keep_err(rc, rc_f1);
    }
    if (left.type != MACRO_EVAL_TYPE_INT || right.type != MACRO_EVAL_TYPE_INT) {
      cdd_c_error_t rc_f1 = cdd_macro_eval_result_free(&left);
      cdd_c_error_t rc_f2 = cdd_macro_eval_result_free(&right);
      return cdd_c_keep_err(CDD_C_ERROR_INVALID_ARGUMENT,
                            cdd_c_keep_err(rc_f1, rc_f2));
    }
    rc = make_int(left.int_val & right.int_val, &left);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  *out = left;
  return CDD_C_SUCCESS;
}

static cdd_c_error_t parse_equality(parser_t *p, cdd_macro_eval_result_t *out) {
  cdd_macro_eval_result_t left;
  cdd_c_error_t rc;
  memset(&left, 0, sizeof(left));
  rc = parse_relational(p, &left);
  if (rc != CDD_C_SUCCESS)
    return rc;

  while (p->lex.cur.kind == TOK_EQEQ || p->lex.cur.kind == TOK_NEQ) {
    macro_tok_kind_t op = p->lex.cur.kind;
    cdd_macro_eval_result_t right;
    int eq_res;
    memset(&right, 0, sizeof(right));
    rc = next_tok(&p->lex);
    if (rc != CDD_C_SUCCESS) {
      cdd_c_error_t rc_f1 = cdd_macro_eval_result_free(&left);
      return cdd_c_keep_err(rc, rc_f1);
    }
    rc = parse_relational(p, &right);
    if (rc != CDD_C_SUCCESS) {
      cdd_c_error_t rc_f1 = cdd_macro_eval_result_free(&left);
      return cdd_c_keep_err(rc, rc_f1);
    }
    rc = promote(&left, &right);
    if (rc != CDD_C_SUCCESS) {
      cdd_c_error_t rc_f1 = cdd_macro_eval_result_free(&left);
      cdd_c_error_t rc_f2 = cdd_macro_eval_result_free(&right);
      return cdd_c_keep_err(rc, cdd_c_keep_err(rc_f1, rc_f2));
    }
    eq_res = (left.type == MACRO_EVAL_TYPE_INT)
                 ? (op == TOK_EQEQ ? (left.int_val == right.int_val)
                                   : (left.int_val != right.int_val))
                 : (op == TOK_EQEQ ? (left.float_val == right.float_val)
                                   : (left.float_val != right.float_val));
    {
      cdd_c_error_t rc_f1 = cdd_macro_eval_result_free(&left);
      cdd_c_error_t rc_f2 = cdd_macro_eval_result_free(&right);
      rc = make_int(eq_res, &left);
      if (rc != CDD_C_SUCCESS) {
        return cdd_c_keep_err(rc, cdd_c_keep_err(rc_f1, rc_f2));
      }
    }
  }
  *out = left;
  return CDD_C_SUCCESS;
}

static cdd_c_error_t parse_relational(parser_t *p,
                                      cdd_macro_eval_result_t *out) {
  cdd_macro_eval_result_t left;
  cdd_c_error_t rc;
  memset(&left, 0, sizeof(left));
  rc = parse_shift(p, &left);
  if (rc != CDD_C_SUCCESS)
    return rc;

  while (p->lex.cur.kind == TOK_LT || p->lex.cur.kind == TOK_LTE ||
         p->lex.cur.kind == TOK_GT || p->lex.cur.kind == TOK_GTE) {
    macro_tok_kind_t op = p->lex.cur.kind;
    cdd_macro_eval_result_t right;
    int rel_res;
    memset(&right, 0, sizeof(right));
    rc = next_tok(&p->lex);
    if (rc != CDD_C_SUCCESS) {
      cdd_c_error_t rc_f1 = cdd_macro_eval_result_free(&left);
      return cdd_c_keep_err(rc, rc_f1);
    }
    rc = parse_shift(p, &right);
    if (rc != CDD_C_SUCCESS) {
      cdd_c_error_t rc_f1 = cdd_macro_eval_result_free(&left);
      return cdd_c_keep_err(rc, rc_f1);
    }
    rc = promote(&left, &right);
    if (rc != CDD_C_SUCCESS) {
      cdd_c_error_t rc_f1 = cdd_macro_eval_result_free(&left);
      cdd_c_error_t rc_f2 = cdd_macro_eval_result_free(&right);
      return cdd_c_keep_err(rc, cdd_c_keep_err(rc_f1, rc_f2));
    }
    if (left.type == MACRO_EVAL_TYPE_INT) {
      if (op == TOK_LT)
        rel_res = (left.int_val < right.int_val);
      else if (op == TOK_LTE)
        rel_res = (left.int_val <= right.int_val);
      else if (op == TOK_GT)
        rel_res = (left.int_val > right.int_val);
      else
        rel_res = (left.int_val >= right.int_val);
    } else {
      if (op == TOK_LT)
        rel_res = (left.float_val < right.float_val);
      else if (op == TOK_LTE)
        rel_res = (left.float_val <= right.float_val);
      else if (op == TOK_GT)
        rel_res = (left.float_val > right.float_val);
      else
        rel_res = (left.float_val >= right.float_val);
    }
    {
      cdd_c_error_t rc_f1 = cdd_macro_eval_result_free(&left);
      cdd_c_error_t rc_f2 = cdd_macro_eval_result_free(&right);
      rc = make_int(rel_res, &left);
      if (rc != CDD_C_SUCCESS) {
        return cdd_c_keep_err(rc, cdd_c_keep_err(rc_f1, rc_f2));
      }
    }
  }
  *out = left;
  return CDD_C_SUCCESS;
}

static cdd_c_error_t parse_shift(parser_t *p, cdd_macro_eval_result_t *out) {
  cdd_macro_eval_result_t left;
  cdd_c_error_t rc;
  memset(&left, 0, sizeof(left));
  rc = parse_additive(p, &left);
  if (rc != CDD_C_SUCCESS)
    return rc;

  while (p->lex.cur.kind == TOK_LSHIFT || p->lex.cur.kind == TOK_RSHIFT) {
    macro_tok_kind_t op = p->lex.cur.kind;
    cdd_macro_eval_result_t right;
    int64_t sh_val;
    memset(&right, 0, sizeof(right));
    rc = next_tok(&p->lex);
    if (rc != CDD_C_SUCCESS) {
      cdd_c_error_t rc_f1 = cdd_macro_eval_result_free(&left);
      return cdd_c_keep_err(rc, rc_f1);
    }
    rc = parse_additive(p, &right);
    if (rc != CDD_C_SUCCESS) {
      cdd_c_error_t rc_f1 = cdd_macro_eval_result_free(&left);
      return cdd_c_keep_err(rc, rc_f1);
    }
    if (left.type != MACRO_EVAL_TYPE_INT || right.type != MACRO_EVAL_TYPE_INT) {
      cdd_c_error_t rc_f1 = cdd_macro_eval_result_free(&left);
      cdd_c_error_t rc_f2 = cdd_macro_eval_result_free(&right);
      return cdd_c_keep_err(CDD_C_ERROR_INVALID_ARGUMENT,
                            cdd_c_keep_err(rc_f1, rc_f2));
    }
    sh_val = (op == TOK_LSHIFT) ? (left.int_val << right.int_val)
                                : (left.int_val >> right.int_val);
    rc = make_int(sh_val, &left);
    if (rc != CDD_C_SUCCESS)
      return rc;
  }
  *out = left;
  return CDD_C_SUCCESS;
}

static cdd_c_error_t parse_additive(parser_t *p, cdd_macro_eval_result_t *out) {
  cdd_macro_eval_result_t left;
  cdd_c_error_t rc;
  memset(&left, 0, sizeof(left));
  rc = parse_multiplicative(p, &left);
  if (rc != CDD_C_SUCCESS)
    return rc;

  while (p->lex.cur.kind == TOK_PLUS || p->lex.cur.kind == TOK_MINUS) {
    macro_tok_kind_t op = p->lex.cur.kind;
    cdd_macro_eval_result_t right;
    memset(&right, 0, sizeof(right));
    rc = next_tok(&p->lex);
    if (rc != CDD_C_SUCCESS) {
      cdd_c_error_t rc_f1 = cdd_macro_eval_result_free(&left);
      return cdd_c_keep_err(rc, rc_f1);
    }
    rc = parse_multiplicative(p, &right);
    if (rc != CDD_C_SUCCESS) {
      cdd_c_error_t rc_f1 = cdd_macro_eval_result_free(&left);
      return cdd_c_keep_err(rc, rc_f1);
    }
    if (left.type == MACRO_EVAL_TYPE_STRING ||
        right.type == MACRO_EVAL_TYPE_STRING) {
      cdd_c_error_t rc_f1 = cdd_macro_eval_result_free(&left);
      cdd_c_error_t rc_f2 = cdd_macro_eval_result_free(&right);
      return cdd_c_keep_err(CDD_C_ERROR_INVALID_ARGUMENT,
                            cdd_c_keep_err(rc_f1, rc_f2));
    }
    rc = promote(&left, &right);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (left.type == MACRO_EVAL_TYPE_INT) {
      int64_t iv = (op == TOK_PLUS) ? (left.int_val + right.int_val)
                                    : (left.int_val - right.int_val);
      rc = make_int(iv, &left);
      if (rc != CDD_C_SUCCESS)
        return rc;
    } else {
      double fv = (op == TOK_PLUS) ? (left.float_val + right.float_val)
                                   : (left.float_val - right.float_val);
      rc = make_float(fv, &left);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
  }
  *out = left;
  return CDD_C_SUCCESS;
}

static cdd_c_error_t parse_multiplicative(parser_t *p,
                                          cdd_macro_eval_result_t *out) {
  cdd_macro_eval_result_t left;
  cdd_c_error_t rc;
  memset(&left, 0, sizeof(left));
  rc = parse_unary(p, &left);
  if (rc != CDD_C_SUCCESS)
    return rc;

  while (p->lex.cur.kind == TOK_STAR || p->lex.cur.kind == TOK_SLASH ||
         p->lex.cur.kind == TOK_PERCENT) {
    macro_tok_kind_t op = p->lex.cur.kind;
    cdd_macro_eval_result_t right;
    memset(&right, 0, sizeof(right));
    rc = next_tok(&p->lex);
    if (rc != CDD_C_SUCCESS) {
      cdd_c_error_t rc_f1 = cdd_macro_eval_result_free(&left);
      return cdd_c_keep_err(rc, rc_f1);
    }
    rc = parse_unary(p, &right);
    if (rc != CDD_C_SUCCESS) {
      cdd_c_error_t rc_f1 = cdd_macro_eval_result_free(&left);
      return cdd_c_keep_err(rc, rc_f1);
    }
    if (left.type == MACRO_EVAL_TYPE_STRING ||
        right.type == MACRO_EVAL_TYPE_STRING) {
      cdd_c_error_t rc_f1 = cdd_macro_eval_result_free(&left);
      cdd_c_error_t rc_f2 = cdd_macro_eval_result_free(&right);
      return cdd_c_keep_err(CDD_C_ERROR_INVALID_ARGUMENT,
                            cdd_c_keep_err(rc_f1, rc_f2));
    }
    if (op == TOK_PERCENT) {
      if (left.type != MACRO_EVAL_TYPE_INT || right.int_val == 0) {
        return CDD_C_ERROR_INVALID_ARGUMENT;
      }
      rc = make_int(left.int_val % right.int_val, &left);
      if (rc != CDD_C_SUCCESS)
        return rc;
    } else {
      rc = promote(&left, &right);
      if (rc != CDD_C_SUCCESS)
        return rc;
      if (left.type == MACRO_EVAL_TYPE_INT) {
        int64_t iv;
        if (op == TOK_SLASH && right.int_val == 0) {
          return CDD_C_ERROR_INVALID_ARGUMENT;
        }
        iv = (op == TOK_STAR) ? (left.int_val * right.int_val)
                              : (left.int_val / right.int_val);
        rc = make_int(iv, &left);
        if (rc != CDD_C_SUCCESS)
          return rc;
      } else {
        double fv = (op == TOK_STAR) ? (left.float_val * right.float_val)
                                     : (left.float_val / right.float_val);
        rc = make_float(fv, &left);
        if (rc != CDD_C_SUCCESS)
          return rc;
      }
    }
  }
  *out = left;
  return CDD_C_SUCCESS;
}

static cdd_c_error_t parse_unary(parser_t *p, cdd_macro_eval_result_t *out) {
  cdd_c_error_t rc;
  if (p->lex.cur.kind == TOK_PLUS) {
    rc = next_tok(&p->lex);
    if (rc != CDD_C_SUCCESS)
      return rc;
    return parse_unary(p, out);
  }
  if (p->lex.cur.kind == TOK_MINUS) {
    cdd_macro_eval_result_t r;
    memset(&r, 0, sizeof(r));
    rc = next_tok(&p->lex);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = parse_unary(p, &r);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (r.type == MACRO_EVAL_TYPE_INT)
      r.int_val = -r.int_val;
    else
      r.float_val = -r.float_val;
    *out = r;
    return CDD_C_SUCCESS;
  }
  if (p->lex.cur.kind == TOK_TILDE) {
    cdd_macro_eval_result_t r;
    memset(&r, 0, sizeof(r));
    rc = next_tok(&p->lex);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = parse_unary(p, &r);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (r.type != MACRO_EVAL_TYPE_INT) {
      cdd_c_error_t rc_f = cdd_macro_eval_result_free(&r);
      return cdd_c_keep_err(CDD_C_ERROR_INVALID_ARGUMENT, rc_f);
    }
    r.int_val = ~r.int_val;
    *out = r;
    return CDD_C_SUCCESS;
  }
  if (p->lex.cur.kind == TOK_BANG) {
    cdd_macro_eval_result_t r;
    memset(&r, 0, sizeof(r));
    rc = next_tok(&p->lex);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = parse_unary(p, &r);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (r.type == MACRO_EVAL_TYPE_INT)
      r.int_val = !r.int_val;
    else
      r.int_val = (r.float_val == 0.0);
    r.type = MACRO_EVAL_TYPE_INT;
    *out = r;
    return CDD_C_SUCCESS;
  }
  return parse_primary(p, out);
}

static cdd_c_error_t parse_primary(parser_t *p, cdd_macro_eval_result_t *out) {
  cdd_macro_eval_result_t r;
  cdd_c_error_t rc;
  memset(&r, 0, sizeof(r));
  memset(out, 0, sizeof(*out));

  if (p->lex.cur.kind == TOK_INT) {
    rc = make_int(p->lex.cur.int_val, &r);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = next_tok(&p->lex);
    if (rc != CDD_C_SUCCESS)
      return rc;
  } else if (p->lex.cur.kind == TOK_FLOAT) {
    rc = make_float(p->lex.cur.float_val, &r);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = next_tok(&p->lex);
    if (rc != CDD_C_SUCCESS)
      return rc;
  } else if (p->lex.cur.kind == TOK_STR) {
    r.type = MACRO_EVAL_TYPE_STRING;
    r.str_val = C_CDD_STRDUP(p->lex.cur.str_val);
    if (!r.str_val) {
      return CDD_C_ERROR_MEMORY;
    }
    rc = next_tok(&p->lex);
    if (rc != CDD_C_SUCCESS) {
      cdd_c_error_t rc_f = cdd_macro_eval_result_free(&r);
      return cdd_c_keep_err(rc, rc_f);
    }
  } else if (p->lex.cur.kind == TOK_IDENT) {
    struct MacroDef *def = NULL;
    size_t i;
    for (i = 0; i < p->ctx->macro_count; i++) {
      if (strcmp(p->ctx->macros[i].name, p->lex.cur.str_val) == 0) {
        def = &p->ctx->macros[i];
        break;
      }
    }
    if (def) {
      cdd_macro_eval_result_t ref_res;
      rc = cdd_macro_evaluate(p->ctx, def->value, &ref_res);
      if (rc != CDD_C_SUCCESS)
        return rc;
      r = ref_res;
    } else {
      rc = make_int(0, &r);
      if (rc != CDD_C_SUCCESS)
        return rc;
    }
    rc = next_tok(&p->lex);
    if (rc != CDD_C_SUCCESS) {
      cdd_c_error_t rc_f = cdd_macro_eval_result_free(&r);
      return cdd_c_keep_err(rc, rc_f);
    }
  } else if (p->lex.cur.kind == TOK_LPAREN) {
    rc = next_tok(&p->lex);
    if (rc != CDD_C_SUCCESS)
      return rc;
    rc = parse_expr(p, &r);
    if (rc != CDD_C_SUCCESS)
      return rc;
    if (p->lex.cur.kind != TOK_RPAREN) {
      cdd_c_error_t rc_f = cdd_macro_eval_result_free(&r);
      return cdd_c_keep_err(CDD_C_ERROR_INVALID_ARGUMENT, rc_f);
    }
    rc = next_tok(&p->lex);
    if (rc != CDD_C_SUCCESS) {
      cdd_c_error_t rc_f = cdd_macro_eval_result_free(&r);
      return cdd_c_keep_err(rc, rc_f);
    }
  } else {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
  *out = r;
  return CDD_C_SUCCESS;
}

/*
 * @brief Evaluates a C macro value expression into an integer, float, or string
 * constant.
 *
 * @param[in] ctx Preprocessor context containing defined macros.
 * @param[in] expression C macro value expression string to evaluate.
 * @param[out] out_result Output structure receiving the evaluated constant
 * value.
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_INVALID_ARGUMENT` on
 * parse/evaluation failure.
 */
cdd_c_error_t cdd_macro_evaluate(struct PreprocessorContext *ctx,
                                 const char *expression,
                                 cdd_macro_eval_result_t *out_result) {
  parser_t p;
  cdd_macro_eval_result_t r;
  cdd_c_error_t rc;

  if (out_result) {
    memset(out_result, 0, sizeof(*out_result));
  }
  if (!ctx || !expression || !out_result)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  memset(&r, 0, sizeof(r));
  p.ctx = ctx;
  p.err = 0;
  p.lex.str = expression;
  p.lex.pos = 0;
  p.lex.len = strlen(expression);
  p.lex.cur.kind = TOK_EOF;
  p.lex.cur.str_val = NULL;

  rc = next_tok(&p.lex);
  if (rc != CDD_C_SUCCESS) {
    goto cleanup;
  }

  if (p.lex.cur.kind == TOK_EOF) {
    rc = CDD_C_ERROR_INVALID_ARGUMENT;
    goto cleanup;
  }

  rc = parse_expr(&p, &r);
  if (rc != CDD_C_SUCCESS) {
    goto cleanup;
  }

  if (p.lex.cur.kind != TOK_EOF) {
    rc = CDD_C_ERROR_INVALID_ARGUMENT;
    goto cleanup;
  }

  *out_result = r;
  memset(&r, 0, sizeof(r));

cleanup: {
  cdd_c_error_t rc_f1 = macro_tok_free(&p.lex.cur);
  cdd_c_error_t rc_f2 = cdd_macro_eval_result_free(&r);
  return cdd_c_keep_err(rc, cdd_c_keep_err(rc_f1, rc_f2));
}
}

/*
 * @brief Frees any dynamically allocated string payload in `result`.
 *
 * @param[in,out] result Pointer to the evaluated macro result structure to
 * free.
 * @return CDD_C_SUCCESS.
 */
cdd_c_error_t cdd_macro_eval_result_free(cdd_macro_eval_result_t *result) {
  if (result && result->str_val) {
    C_CDD_FREE(result->str_val);
    result->str_val = NULL;
  }
  return CDD_C_SUCCESS;
}
