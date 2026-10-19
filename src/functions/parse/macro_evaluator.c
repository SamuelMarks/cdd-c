/**
 * @file macro_evaluator.c
 * @brief Core macro evaluator definitions and implementation.
 */
/* clang-format off */
#include "c_cdd/memory.h"
#include "c_cdd/safe_crt_msvc.h"

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
 */
static void free_tok(macro_tok_t *tok) {
  if (tok->kind == TOK_STR || tok->kind == TOK_IDENT) {
    C_CDD_FREE(tok->str_val);
    tok->str_val = NULL;
  }
}

/**
 * @brief Advances the macro lexer to the next token in `lex->str`.
 *
 * @param[in,out] lex Macro lexer state.
 * @return `CDD_C_SUCCESS` on success.
 */
static cdd_c_error_t next_tok(macro_lexer_t *lex) {
  free_tok(&lex->cur);

  while (lex->pos < lex->len && isspace((unsigned char)lex->str[lex->pos])) {
    lex->pos++;
  }

  if (lex->pos >= lex->len) {
    lex->cur.kind = TOK_EOF;
    return CDD_C_SUCCESS;
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
          return CDD_C_SUCCESS;
        }
        memcpy(buf, lex->str + start, len);
        buf[len] = '\0';
        if (is_float) {
          lex->cur.kind = TOK_FLOAT;
          lex->cur.float_val = strtod(buf, NULL);
        } else {
          lex->cur.kind = TOK_INT;
#if defined(_MSC_VER) && _MSC_VER < 1800
          lex->cur.int_val = _strtoi64(buf, NULL, 0);
#else
          lex->cur.int_val = strtoll(buf, NULL, 0);
#endif
        }
        C_CDD_FREE(buf);
      }
      return CDD_C_SUCCESS;
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
 * @brief Initializes `r` as an unknown/error macro evaluation result.
 *
 * @param[out] r Evaluation result structure to reset.
 * @return `CDD_C_SUCCESS` on success.
 */
static cdd_c_error_t make_err(cdd_macro_eval_result_t *r) {
  r->type = MACRO_EVAL_TYPE_UNKNOWN;
  r->int_val = 0;
  r->float_val = 0;
  r->str_val = NULL;
  return CDD_C_SUCCESS;
}

/**
 * @brief Initializes `r` as an integer macro evaluation result.
 *
 * @param[in] v 64-bit integer value.
 * @param[out] r Evaluation result structure to populate.
 * @return `CDD_C_SUCCESS` on success.
 */
static cdd_c_error_t make_int(int64_t v, cdd_macro_eval_result_t *r) {
  make_err(r);
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
  make_err(r);
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
  parse_logical_and(p, &left);
  while (p->lex.cur.kind == TOK_PIPEPIPE) {
    cdd_macro_eval_result_t right;
    next_tok(&p->lex);
    parse_logical_and(p, &right);
    if (p->err)
      return make_err(out);
    make_int((left.type == MACRO_EVAL_TYPE_INT ? (left.int_val != 0)
                                               : (left.float_val != 0.0)) ||
                 (right.type == MACRO_EVAL_TYPE_INT ? (right.int_val != 0)
                                                    : (right.float_val != 0.0)),
             &left);
  }
  *out = left;
  return CDD_C_SUCCESS;
}

static cdd_c_error_t parse_logical_and(parser_t *p,
                                       cdd_macro_eval_result_t *out) {
  cdd_macro_eval_result_t left;
  parse_bitwise_or(p, &left);
  while (p->lex.cur.kind == TOK_AMPAMP) {
    cdd_macro_eval_result_t right;
    next_tok(&p->lex);
    parse_bitwise_or(p, &right);
    if (p->err)
      return make_err(out);
    make_int((left.type == MACRO_EVAL_TYPE_INT ? (left.int_val != 0)
                                               : (left.float_val != 0.0)) &&
                 (right.type == MACRO_EVAL_TYPE_INT ? (right.int_val != 0)
                                                    : (right.float_val != 0.0)),
             &left);
  }
  *out = left;
  return CDD_C_SUCCESS;
}

static cdd_c_error_t parse_bitwise_or(parser_t *p,
                                      cdd_macro_eval_result_t *out) {
  cdd_macro_eval_result_t left;
  parse_bitwise_xor(p, &left);
  while (p->lex.cur.kind == TOK_PIPE) {
    cdd_macro_eval_result_t right;
    next_tok(&p->lex);
    parse_bitwise_xor(p, &right);
    if (left.type != MACRO_EVAL_TYPE_INT || right.type != MACRO_EVAL_TYPE_INT) {
      cdd_macro_eval_result_free(&left);
      cdd_macro_eval_result_free(&right);
      p->err = 1;
      return make_err(out);
    }
    make_int(left.int_val | right.int_val, &left);
  }
  *out = left;
  return CDD_C_SUCCESS;
}

static cdd_c_error_t parse_bitwise_xor(parser_t *p,
                                       cdd_macro_eval_result_t *out) {
  cdd_macro_eval_result_t left;
  parse_bitwise_and(p, &left);
  while (p->lex.cur.kind == TOK_CARET) {
    cdd_macro_eval_result_t right;
    next_tok(&p->lex);
    parse_bitwise_and(p, &right);
    if (left.type != MACRO_EVAL_TYPE_INT || right.type != MACRO_EVAL_TYPE_INT) {
      cdd_macro_eval_result_free(&left);
      cdd_macro_eval_result_free(&right);
      p->err = 1;
      return make_err(out);
    }
    make_int(left.int_val ^ right.int_val, &left);
  }
  *out = left;
  return CDD_C_SUCCESS;
}

static cdd_c_error_t parse_bitwise_and(parser_t *p,
                                       cdd_macro_eval_result_t *out) {
  cdd_macro_eval_result_t left;
  parse_equality(p, &left);
  while (p->lex.cur.kind == TOK_AMP) {
    cdd_macro_eval_result_t right;
    next_tok(&p->lex);
    parse_equality(p, &right);
    if (left.type != MACRO_EVAL_TYPE_INT || right.type != MACRO_EVAL_TYPE_INT) {
      cdd_macro_eval_result_free(&left);
      cdd_macro_eval_result_free(&right);
      p->err = 1;
      return make_err(out);
    }
    make_int(left.int_val & right.int_val, &left);
  }
  *out = left;
  return CDD_C_SUCCESS;
}

static cdd_c_error_t parse_equality(parser_t *p, cdd_macro_eval_result_t *out) {
  cdd_macro_eval_result_t left;
  parse_relational(p, &left);
  while (p->lex.cur.kind == TOK_EQEQ || p->lex.cur.kind == TOK_NEQ) {
    macro_tok_kind_t op = p->lex.cur.kind;
    cdd_macro_eval_result_t right;
    next_tok(&p->lex);
    parse_relational(p, &right);
    if (p->err)
      return make_err(out);
    promote(&left, &right);
    if (left.type == MACRO_EVAL_TYPE_INT) {
      make_int(op == TOK_EQEQ ? (left.int_val == right.int_val)
                              : (left.int_val != right.int_val),
               &left);
    } else {
      make_int(op == TOK_EQEQ ? (left.float_val == right.float_val)
                              : (left.float_val != right.float_val),
               &left);
    }
  }
  *out = left;
  return CDD_C_SUCCESS;
}

static cdd_c_error_t parse_relational(parser_t *p,
                                      cdd_macro_eval_result_t *out) {
  cdd_macro_eval_result_t left;
  parse_shift(p, &left);
  while (p->lex.cur.kind == TOK_LT || p->lex.cur.kind == TOK_LTE ||
         p->lex.cur.kind == TOK_GT || p->lex.cur.kind == TOK_GTE) {
    macro_tok_kind_t op = p->lex.cur.kind;
    cdd_macro_eval_result_t right;
    next_tok(&p->lex);
    parse_shift(p, &right);
    if (p->err)
      return make_err(out);
    promote(&left, &right);
    if (left.type == MACRO_EVAL_TYPE_INT) {
      if (op == TOK_LT)
        make_int(left.int_val < right.int_val, &left);
      else if (op == TOK_LTE)
        make_int(left.int_val <= right.int_val, &left);
      else if (op == TOK_GT)
        make_int(left.int_val > right.int_val, &left);
      else
        make_int(left.int_val >= right.int_val, &left);
    } else {
      if (op == TOK_LT)
        make_int(left.float_val < right.float_val, &left);
      else if (op == TOK_LTE)
        make_int(left.float_val <= right.float_val, &left);
      else if (op == TOK_GT)
        make_int(left.float_val > right.float_val, &left);
      else
        make_int(left.float_val >= right.float_val, &left);
    }
  }
  *out = left;
  return CDD_C_SUCCESS;
}

static cdd_c_error_t parse_shift(parser_t *p, cdd_macro_eval_result_t *out) {
  cdd_macro_eval_result_t left;
  parse_additive(p, &left);
  while (p->lex.cur.kind == TOK_LSHIFT || p->lex.cur.kind == TOK_RSHIFT) {
    macro_tok_kind_t op = p->lex.cur.kind;
    cdd_macro_eval_result_t right;
    next_tok(&p->lex);
    parse_additive(p, &right);
    if (left.type != MACRO_EVAL_TYPE_INT || right.type != MACRO_EVAL_TYPE_INT) {
      cdd_macro_eval_result_free(&left);
      cdd_macro_eval_result_free(&right);
      p->err = 1;
      return make_err(out);
    }
    if (op == TOK_LSHIFT)
      make_int(left.int_val << right.int_val, &left);
    else
      make_int(left.int_val >> right.int_val, &left);
  }
  *out = left;
  return CDD_C_SUCCESS;
}

static cdd_c_error_t parse_additive(parser_t *p, cdd_macro_eval_result_t *out) {
  cdd_macro_eval_result_t left;
  parse_multiplicative(p, &left);
  while (p->lex.cur.kind == TOK_PLUS || p->lex.cur.kind == TOK_MINUS) {
    macro_tok_kind_t op = p->lex.cur.kind;
    cdd_macro_eval_result_t right;
    next_tok(&p->lex);
    parse_multiplicative(p, &right);
    if (p->err) {
      cdd_macro_eval_result_free(&left);
      cdd_macro_eval_result_free(&right);
      return make_err(out);
    }
    if (left.type == MACRO_EVAL_TYPE_STRING ||
        right.type == MACRO_EVAL_TYPE_STRING) {
      cdd_macro_eval_result_free(&left);
      cdd_macro_eval_result_free(&right);
      p->err = 1;
      return make_err(out);
    }
    promote(&left, &right);
    if (left.type == MACRO_EVAL_TYPE_INT) {
      if (op == TOK_PLUS)
        make_int(left.int_val + right.int_val, &left);
      else
        make_int(left.int_val - right.int_val, &left);
    } else {
      if (op == TOK_PLUS)
        make_float(left.float_val + right.float_val, &left);
      else
        make_float(left.float_val - right.float_val, &left);
    }
  }
  *out = left;
  return CDD_C_SUCCESS;
}

static cdd_c_error_t parse_multiplicative(parser_t *p,
                                          cdd_macro_eval_result_t *out) {
  cdd_macro_eval_result_t left;
  parse_unary(p, &left);
  while (p->lex.cur.kind == TOK_STAR || p->lex.cur.kind == TOK_SLASH ||
         p->lex.cur.kind == TOK_PERCENT) {
    macro_tok_kind_t op = p->lex.cur.kind;
    cdd_macro_eval_result_t right;
    next_tok(&p->lex);
    parse_unary(p, &right);
    if (p->err) {
      cdd_macro_eval_result_free(&left);
      cdd_macro_eval_result_free(&right);
      return make_err(out);
    }
    if (left.type == MACRO_EVAL_TYPE_STRING ||
        right.type == MACRO_EVAL_TYPE_STRING) {
      cdd_macro_eval_result_free(&left);
      cdd_macro_eval_result_free(&right);
      p->err = 1;
      return make_err(out);
    }
    if (op == TOK_PERCENT) {
      if (left.type != MACRO_EVAL_TYPE_INT || right.int_val == 0) {
        cdd_macro_eval_result_free(&left);
        cdd_macro_eval_result_free(&right);
        p->err = 1;
        return make_err(out);
      }
      make_int(left.int_val % right.int_val, &left);
    } else {
      promote(&left, &right);
      if (left.type == MACRO_EVAL_TYPE_INT) {
        if (op == TOK_SLASH && right.int_val == 0) {
          cdd_macro_eval_result_free(&left);
          cdd_macro_eval_result_free(&right);
          p->err = 1;
          return make_err(out);
        }
        if (op == TOK_STAR)
          make_int(left.int_val * right.int_val, &left);
        else
          make_int(left.int_val / right.int_val, &left);
      } else {
        if (op == TOK_STAR)
          make_float(left.float_val * right.float_val, &left);
        else
          make_float(left.float_val / right.float_val, &left);
      }
    }
  }
  *out = left;
  return CDD_C_SUCCESS;
}

static cdd_c_error_t parse_unary(parser_t *p, cdd_macro_eval_result_t *out) {
  if (p->lex.cur.kind == TOK_PLUS) {
    next_tok(&p->lex);
    return parse_unary(p, out);
  }
  if (p->lex.cur.kind == TOK_MINUS) {
    cdd_macro_eval_result_t r;
    next_tok(&p->lex);
    parse_unary(p, &r);
    if (r.type == MACRO_EVAL_TYPE_INT)
      r.int_val = -r.int_val;
    else
      r.float_val = -r.float_val;
    *out = r;
    return CDD_C_SUCCESS;
  }
  if (p->lex.cur.kind == TOK_TILDE) {
    cdd_macro_eval_result_t r;
    next_tok(&p->lex);
    parse_unary(p, &r);
    if (r.type != MACRO_EVAL_TYPE_INT) {
      p->err = 1;
      return make_err(out);
    }
    r.int_val = ~r.int_val;
    *out = r;
    return CDD_C_SUCCESS;
  }
  if (p->lex.cur.kind == TOK_BANG) {
    cdd_macro_eval_result_t r;
    next_tok(&p->lex);
    parse_unary(p, &r);
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
  make_err(&r);
  if (p->lex.cur.kind == TOK_INT) {
    make_int(p->lex.cur.int_val, &r);
    next_tok(&p->lex);
  } else if (p->lex.cur.kind == TOK_FLOAT) {
    make_float(p->lex.cur.float_val, &r);
    next_tok(&p->lex);
  } else if (p->lex.cur.kind == TOK_STR) {
    r.type = MACRO_EVAL_TYPE_STRING;
    r.str_val = strdup(p->lex.cur.str_val);
    next_tok(&p->lex);
  } else if (p->lex.cur.kind == TOK_IDENT) {
    /* macro reference */
    struct MacroDef *def = NULL;
    {
      size_t i;
      for (i = 0; i < p->ctx->macro_count; i++) {
        if (strcmp(p->ctx->macros[i].name, p->lex.cur.str_val) == 0) {
          def = &p->ctx->macros[i];
          break;
        }
      }
    }
    if (def) {
      cdd_macro_eval_result_t ref_res;
      if (cdd_macro_evaluate(p->ctx, def->value, &ref_res) == 0) {
        r = ref_res;
      } else {
        p->err = 1;
      }
    } else {
      /* Unknown identifier, typical C preprocessor treats it as 0 */
      make_int(0, &r);
    }
    next_tok(&p->lex);
  } else if (p->lex.cur.kind == TOK_LPAREN) {
    next_tok(&p->lex);
    parse_expr(p, &r);
    if (p->lex.cur.kind == TOK_RPAREN) {
      next_tok(&p->lex);
    } else {
      p->err = 1;
    }
  } else {
    p->err = 1;
  }
  *out = r;
  return CDD_C_SUCCESS;
}

/**
 * @brief Evaluates a C macro value expression into an integer, float, or string
 * constant.
 *
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_INVALID_ARGUMENT` on
 * parse/evaluation failure.
 */
cdd_c_error_t cdd_macro_evaluate(struct PreprocessorContext *ctx,
                                 const char *expression,
                                 cdd_macro_eval_result_t *out_result) {
  parser_t p;
  cdd_macro_eval_result_t r;

  if (!ctx || !expression)
    return CDD_C_ERROR_INVALID_ARGUMENT;

  p.ctx = ctx;
  p.err = 0;
  p.lex.str = expression;
  p.lex.pos = 0;
  p.lex.len = strlen(expression);
  p.lex.cur.kind = TOK_EOF;
  p.lex.cur.str_val = NULL;

  next_tok(&p.lex);

  if (p.lex.cur.kind == TOK_EOF) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  parse_expr(&p, &r);

  free_tok(&p.lex.cur);

  if (p.err || p.lex.cur.kind != TOK_EOF) {
    cdd_macro_eval_result_free(&r);
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  *out_result = r;
  return CDD_C_SUCCESS;
}

/**
 * @brief Frees any dynamically allocated string payload in `result`.
 */
void cdd_macro_eval_result_free(cdd_macro_eval_result_t *result) {
  if (result->str_val) {
    C_CDD_FREE(result->str_val);
    result->str_val = NULL;
  }
}
