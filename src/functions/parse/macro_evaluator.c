/* clang-format off */
#include "macro_evaluator.h"
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

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

typedef struct {
  macro_tok_kind_t kind;
  long long int_val;
  double float_val;
  char *str_val;
} macro_tok_t;

typedef struct {
  const char *str;
  size_t pos;
  size_t len;
  macro_tok_t cur;
} macro_lexer_t;

static void free_tok(macro_tok_t *tok) {
  if ((tok->kind == TOK_STR || tok->kind == TOK_IDENT) && tok->str_val) {
    free(tok->str_val);
    tok->str_val = NULL;
  }
}

static void next_tok(macro_lexer_t *lex) {
  free_tok(&lex->cur);

  while (lex->pos < lex->len && isspace((unsigned char)lex->str[lex->pos])) {
    lex->pos++;
  }

  if (lex->pos >= lex->len) {
    lex->cur.kind = TOK_EOF;
    return;
  }

  {
    char c = lex->str[lex->pos];
    char nc = (lex->pos + 1 < lex->len) ? lex->str[lex->pos + 1] : '\0';

    if (isdigit((unsigned char)c) || (c == '.' && isdigit((unsigned char)nc))) {
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
            if (lex->pos < lex->len &&
                (lex->str[lex->pos] == '+' || lex->str[lex->pos] == '-'))
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
        char *buf = (char *)malloc(len + 1);
        if (!buf) {
          lex->cur.kind = TOK_ERROR;
          return;
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
        free(buf);
      }
      return;
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
        lex->cur.str_val = (char *)malloc(len + 1);
        if (lex->cur.str_val) {
          memcpy(lex->cur.str_val, lex->str + start, len);
          lex->cur.str_val[len] = '\0';
        } else {
          lex->cur.kind = TOK_ERROR;
        }
      }
      return;
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
        lex->cur.str_val = (char *)malloc(len + 1);
        if (lex->cur.str_val) {
          memcpy(lex->cur.str_val, lex->str + start, len);
          lex->cur.str_val[len] = '\0';
        }
        lex->pos++;
      } else {
        lex->cur.kind = TOK_ERROR;
      }
      return;
    }

    lex->pos++;
    switch (c) {
    case '+':
      lex->cur.kind = TOK_PLUS;
      return;
    case '-':
      lex->cur.kind = TOK_MINUS;
      return;
    case '*':
      lex->cur.kind = TOK_STAR;
      return;
    case '/':
      lex->cur.kind = TOK_SLASH;
      return;
    case '%':
      lex->cur.kind = TOK_PERCENT;
      return;
    case '~':
      lex->cur.kind = TOK_TILDE;
      return;
    case '^':
      lex->cur.kind = TOK_CARET;
      return;
    case '(':
      lex->cur.kind = TOK_LPAREN;
      return;
    case ')':
      lex->cur.kind = TOK_RPAREN;
      return;
    case '&':
      if (nc == '&') {
        lex->pos++;
        lex->cur.kind = TOK_AMPAMP;
      } else {
        lex->cur.kind = TOK_AMP;
      }
      return;
    case '|':
      if (nc == '|') {
        lex->pos++;
        lex->cur.kind = TOK_PIPEPIPE;
      } else {
        lex->cur.kind = TOK_PIPE;
      }
      return;
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
      return;
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
      return;
    case '=':
      if (nc == '=') {
        lex->pos++;
        lex->cur.kind = TOK_EQEQ;
      } else {
        lex->cur.kind = TOK_ERROR;
      }
      return;
    case '!':
      if (nc == '=') {
        lex->pos++;
        lex->cur.kind = TOK_NEQ;
      } else {
        lex->cur.kind = TOK_BANG;
      }
      return;
    default:
      lex->cur.kind = TOK_ERROR;
      return;
    }
  }
}

typedef struct {
  macro_lexer_t lex;
  struct PreprocessorContext *ctx;
  int err;
} parser_t;

static cdd_macro_eval_result_t parse_expr(parser_t *p);
static cdd_macro_eval_result_t parse_logical_or(parser_t *p);
static cdd_macro_eval_result_t parse_logical_and(parser_t *p);
static cdd_macro_eval_result_t parse_bitwise_or(parser_t *p);
static cdd_macro_eval_result_t parse_bitwise_xor(parser_t *p);
static cdd_macro_eval_result_t parse_bitwise_and(parser_t *p);
static cdd_macro_eval_result_t parse_equality(parser_t *p);
static cdd_macro_eval_result_t parse_relational(parser_t *p);
static cdd_macro_eval_result_t parse_shift(parser_t *p);
static cdd_macro_eval_result_t parse_additive(parser_t *p);
static cdd_macro_eval_result_t parse_multiplicative(parser_t *p);
static cdd_macro_eval_result_t parse_unary(parser_t *p);
static cdd_macro_eval_result_t parse_primary(parser_t *p);

static cdd_macro_eval_result_t make_err(void) {
  cdd_macro_eval_result_t r;
  r.type = MACRO_EVAL_TYPE_UNKNOWN;
  r.int_val = 0;
  r.float_val = 0;
  r.str_val = NULL;
  return r;
}

static cdd_macro_eval_result_t make_int(long long v) {
  cdd_macro_eval_result_t r = make_err();
  r.type = MACRO_EVAL_TYPE_INT;
  r.int_val = v;
  return r;
}

static cdd_macro_eval_result_t make_float(double v) {
  cdd_macro_eval_result_t r = make_err();
  r.type = MACRO_EVAL_TYPE_FLOAT;
  r.float_val = v;
  return r;
}

static void promote(cdd_macro_eval_result_t *a, cdd_macro_eval_result_t *b) {
  if (a->type == MACRO_EVAL_TYPE_FLOAT && b->type == MACRO_EVAL_TYPE_INT) {
    b->type = MACRO_EVAL_TYPE_FLOAT;
    b->float_val = (double)b->int_val;
  } else if (a->type == MACRO_EVAL_TYPE_INT &&
             b->type == MACRO_EVAL_TYPE_FLOAT) {
    a->type = MACRO_EVAL_TYPE_FLOAT;
    a->float_val = (double)a->int_val;
  }
}

static cdd_macro_eval_result_t parse_expr(parser_t *p) {
  return parse_logical_or(p);
}

static cdd_macro_eval_result_t parse_logical_or(parser_t *p) {
  cdd_macro_eval_result_t left = parse_logical_and(p);
  while (p->lex.cur.kind == TOK_PIPEPIPE) {
    cdd_macro_eval_result_t right;
    next_tok(&p->lex);
    right = parse_logical_and(p);
    if (p->err)
      return make_err();
    left = make_int(
        (left.type == MACRO_EVAL_TYPE_INT ? left.int_val : left.float_val) ||
        (right.type == MACRO_EVAL_TYPE_INT ? right.int_val : right.float_val));
  }
  return left;
}

static cdd_macro_eval_result_t parse_logical_and(parser_t *p) {
  cdd_macro_eval_result_t left = parse_bitwise_or(p);
  while (p->lex.cur.kind == TOK_AMPAMP) {
    cdd_macro_eval_result_t right;
    next_tok(&p->lex);
    right = parse_bitwise_or(p);
    if (p->err)
      return make_err();
    left = make_int(
        (left.type == MACRO_EVAL_TYPE_INT ? left.int_val : left.float_val) &&
        (right.type == MACRO_EVAL_TYPE_INT ? right.int_val : right.float_val));
  }
  return left;
}

static cdd_macro_eval_result_t parse_bitwise_or(parser_t *p) {
  cdd_macro_eval_result_t left = parse_bitwise_xor(p);
  while (p->lex.cur.kind == TOK_PIPE) {
    cdd_macro_eval_result_t right;
    next_tok(&p->lex);
    right = parse_bitwise_xor(p);
    if (p->err || left.type != MACRO_EVAL_TYPE_INT ||
        right.type != MACRO_EVAL_TYPE_INT) {
      p->err = 1;
      return make_err();
    }
    left = make_int(left.int_val | right.int_val);
  }
  return left;
}

static cdd_macro_eval_result_t parse_bitwise_xor(parser_t *p) {
  cdd_macro_eval_result_t left = parse_bitwise_and(p);
  while (p->lex.cur.kind == TOK_CARET) {
    cdd_macro_eval_result_t right;
    next_tok(&p->lex);
    right = parse_bitwise_and(p);
    if (p->err || left.type != MACRO_EVAL_TYPE_INT ||
        right.type != MACRO_EVAL_TYPE_INT) {
      p->err = 1;
      return make_err();
    }
    left = make_int(left.int_val ^ right.int_val);
  }
  return left;
}

static cdd_macro_eval_result_t parse_bitwise_and(parser_t *p) {
  cdd_macro_eval_result_t left = parse_equality(p);
  while (p->lex.cur.kind == TOK_AMP) {
    cdd_macro_eval_result_t right;
    next_tok(&p->lex);
    right = parse_equality(p);
    if (p->err || left.type != MACRO_EVAL_TYPE_INT ||
        right.type != MACRO_EVAL_TYPE_INT) {
      p->err = 1;
      return make_err();
    }
    left = make_int(left.int_val & right.int_val);
  }
  return left;
}

static cdd_macro_eval_result_t parse_equality(parser_t *p) {
  cdd_macro_eval_result_t left = parse_relational(p);
  while (p->lex.cur.kind == TOK_EQEQ || p->lex.cur.kind == TOK_NEQ) {
    macro_tok_kind_t op = p->lex.cur.kind;
    cdd_macro_eval_result_t right;
    next_tok(&p->lex);
    right = parse_relational(p);
    if (p->err)
      return make_err();
    promote(&left, &right);
    if (left.type == MACRO_EVAL_TYPE_INT) {
      left = make_int(op == TOK_EQEQ ? (left.int_val == right.int_val)
                                     : (left.int_val != right.int_val));
    } else {
      left = make_int(op == TOK_EQEQ ? (left.float_val == right.float_val)
                                     : (left.float_val != right.float_val));
    }
  }
  return left;
}

static cdd_macro_eval_result_t parse_relational(parser_t *p) {
  cdd_macro_eval_result_t left = parse_shift(p);
  while (p->lex.cur.kind == TOK_LT || p->lex.cur.kind == TOK_LTE ||
         p->lex.cur.kind == TOK_GT || p->lex.cur.kind == TOK_GTE) {
    macro_tok_kind_t op = p->lex.cur.kind;
    cdd_macro_eval_result_t right;
    next_tok(&p->lex);
    right = parse_shift(p);
    if (p->err)
      return make_err();
    promote(&left, &right);
    if (left.type == MACRO_EVAL_TYPE_INT) {
      if (op == TOK_LT)
        left = make_int(left.int_val < right.int_val);
      else if (op == TOK_LTE)
        left = make_int(left.int_val <= right.int_val);
      else if (op == TOK_GT)
        left = make_int(left.int_val > right.int_val);
      else
        left = make_int(left.int_val >= right.int_val);
    } else {
      if (op == TOK_LT)
        left = make_int(left.float_val < right.float_val);
      else if (op == TOK_LTE)
        left = make_int(left.float_val <= right.float_val);
      else if (op == TOK_GT)
        left = make_int(left.float_val > right.float_val);
      else
        left = make_int(left.float_val >= right.float_val);
    }
  }
  return left;
}

static cdd_macro_eval_result_t parse_shift(parser_t *p) {
  cdd_macro_eval_result_t left = parse_additive(p);
  while (p->lex.cur.kind == TOK_LSHIFT || p->lex.cur.kind == TOK_RSHIFT) {
    macro_tok_kind_t op = p->lex.cur.kind;
    cdd_macro_eval_result_t right;
    next_tok(&p->lex);
    right = parse_additive(p);
    if (p->err || left.type != MACRO_EVAL_TYPE_INT ||
        right.type != MACRO_EVAL_TYPE_INT) {
      p->err = 1;
      return make_err();
    }
    if (op == TOK_LSHIFT)
      left = make_int(left.int_val << right.int_val);
    else
      left = make_int(left.int_val >> right.int_val);
  }
  return left;
}

static cdd_macro_eval_result_t parse_additive(parser_t *p) {
  cdd_macro_eval_result_t left = parse_multiplicative(p);
  while (p->lex.cur.kind == TOK_PLUS || p->lex.cur.kind == TOK_MINUS) {
    macro_tok_kind_t op = p->lex.cur.kind;
    cdd_macro_eval_result_t right;
    next_tok(&p->lex);
    right = parse_multiplicative(p);
    if (p->err)
      return make_err();
    promote(&left, &right);
    if (left.type == MACRO_EVAL_TYPE_INT) {
      if (op == TOK_PLUS)
        left = make_int(left.int_val + right.int_val);
      else
        left = make_int(left.int_val - right.int_val);
    } else {
      if (op == TOK_PLUS)
        left = make_float(left.float_val + right.float_val);
      else
        left = make_float(left.float_val - right.float_val);
    }
  }
  return left;
}

static cdd_macro_eval_result_t parse_multiplicative(parser_t *p) {
  cdd_macro_eval_result_t left = parse_unary(p);
  while (p->lex.cur.kind == TOK_STAR || p->lex.cur.kind == TOK_SLASH ||
         p->lex.cur.kind == TOK_PERCENT) {
    macro_tok_kind_t op = p->lex.cur.kind;
    cdd_macro_eval_result_t right;
    next_tok(&p->lex);
    right = parse_unary(p);
    if (p->err)
      return make_err();
    if (op == TOK_PERCENT) {
      if (left.type != MACRO_EVAL_TYPE_INT ||
          right.type != MACRO_EVAL_TYPE_INT || right.int_val == 0) {
        p->err = 1;
        return make_err();
      }
      left = make_int(left.int_val % right.int_val);
    } else {
      promote(&left, &right);
      if (left.type == MACRO_EVAL_TYPE_INT) {
        if (op == TOK_SLASH && right.int_val == 0) {
          p->err = 1;
          return make_err();
        }
        if (op == TOK_STAR)
          left = make_int(left.int_val * right.int_val);
        else
          left = make_int(left.int_val / right.int_val);
      } else {
        if (op == TOK_STAR)
          left = make_float(left.float_val * right.float_val);
        else
          left = make_float(left.float_val / right.float_val);
      }
    }
  }
  return left;
}

static cdd_macro_eval_result_t parse_unary(parser_t *p) {
  if (p->lex.cur.kind == TOK_PLUS) {
    next_tok(&p->lex);
    return parse_unary(p);
  }
  if (p->lex.cur.kind == TOK_MINUS) {
    cdd_macro_eval_result_t r;
    next_tok(&p->lex);
    r = parse_unary(p);
    if (r.type == MACRO_EVAL_TYPE_INT)
      r.int_val = -r.int_val;
    else if (r.type == MACRO_EVAL_TYPE_FLOAT)
      r.float_val = -r.float_val;
    return r;
  }
  if (p->lex.cur.kind == TOK_TILDE) {
    cdd_macro_eval_result_t r;
    next_tok(&p->lex);
    r = parse_unary(p);
    if (r.type != MACRO_EVAL_TYPE_INT) {
      p->err = 1;
      return make_err();
    }
    r.int_val = ~r.int_val;
    return r;
  }
  if (p->lex.cur.kind == TOK_BANG) {
    cdd_macro_eval_result_t r;
    next_tok(&p->lex);
    r = parse_unary(p);
    if (r.type == MACRO_EVAL_TYPE_INT)
      r.int_val = !r.int_val;
    else if (r.type == MACRO_EVAL_TYPE_FLOAT)
      r.int_val = !r.float_val;
    r.type = MACRO_EVAL_TYPE_INT;
    return r;
  }
  return parse_primary(p);
}

static cdd_macro_eval_result_t parse_primary(parser_t *p) {
  cdd_macro_eval_result_t r = make_err();
  if (p->lex.cur.kind == TOK_INT) {
    r = make_int(p->lex.cur.int_val);
    next_tok(&p->lex);
  } else if (p->lex.cur.kind == TOK_FLOAT) {
    r = make_float(p->lex.cur.float_val);
    next_tok(&p->lex);
  } else if (p->lex.cur.kind == TOK_STR) {
    r.type = MACRO_EVAL_TYPE_STRING;
    r.str_val = strdup(p->lex.cur.str_val);
    next_tok(&p->lex);
  } else if (p->lex.cur.kind == TOK_IDENT) {
    /* macro reference */
    struct MacroDef *def = NULL;
    if (p->ctx && p->lex.cur.str_val) {
      size_t i;
      for (i = 0; i < p->ctx->macro_count; i++) {
        if (strcmp(p->ctx->macros[i].name, p->lex.cur.str_val) == 0) {
          def = &p->ctx->macros[i];
          break;
        }
      }
    }
    if (def && def->value && !def->is_function_like) {
      cdd_macro_eval_result_t ref_res;
      if (cdd_macro_evaluate(p->ctx, def->value, &ref_res) == 0) {
        r = ref_res;
      } else {
        p->err = 1;
      }
    } else {
      /* Unknown identifier, typical C preprocessor treats it as 0 */
      r = make_int(0);
    }
    next_tok(&p->lex);
  } else if (p->lex.cur.kind == TOK_LPAREN) {
    next_tok(&p->lex);
    r = parse_expr(p);
    if (p->lex.cur.kind == TOK_RPAREN) {
      next_tok(&p->lex);
    } else {
      p->err = 1;
    }
  } else {
    p->err = 1;
  }
  return r;
}

enum cdd_c_error cdd_macro_evaluate(struct PreprocessorContext *ctx,
                                    const char *expression,
                                    cdd_macro_eval_result_t *out_result) {
  parser_t p;
  cdd_macro_eval_result_t r;

  if (!expression || !out_result)
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

  r = parse_expr(&p);

  free_tok(&p.lex.cur);

  if (p.err || p.lex.cur.kind != TOK_EOF) {
    cdd_macro_eval_result_free(&r);
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  *out_result = r;
  return CDD_C_SUCCESS;
}

void cdd_macro_eval_result_free(cdd_macro_eval_result_t *result) {
  if (result && result->str_val) {
    free(result->str_val);
    result->str_val = NULL;
  }
}
