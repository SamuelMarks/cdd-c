/**
 * @file tokenizer_punct.c
 * @brief Lexical analyzer punctuators and comment processing.
 *
 * @author Samuel Marks
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#include <stddef.h>
#include <stdint.h>

#include "c_cdd/log.h"
#include "cdd_c_error.h"
#include "functions/parse/tokenizer.h"
#include "functions/parse/tokenizer_punct.h"
/* clang-format on */

/**
 * @brief Tokenizes punctuators and comments starting with character c.
 */
cdd_c_error_t tokenize_punct(const uint8_t *base, size_t len, size_t *pos,
                             size_t start, int c, size_t consumed,
                             struct TokenList *list) {
  enum TokenKind k = TOKEN_OTHER;
  size_t extra = 0;
  int next_c;
  size_t next_con;
  size_t cur_pos;
  cdd_c_error_t rc;

  if (!base || !pos || !list) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }

  cur_pos = *pos + consumed;
  next_c = peek_logical(base, len, cur_pos, &next_con);

  switch (c) {
  case '{':
    k = TOKEN_LBRACE;
    break;

  case '}':
    k = TOKEN_RBRACE;
    break;

  case '[':
    k = TOKEN_LBRACKET;
    break;

  case ']':
    k = TOKEN_RBRACKET;
    break;

  case '(':
    k = TOKEN_LPAREN;
    break;

  case ')':
    k = TOKEN_RPAREN;
    break;

  case ';':
    k = TOKEN_SEMICOLON;
    break;

  case ',':
    k = TOKEN_COMMA;
    break;

  case '~':
    k = TOKEN_TILDE;
    break;

  case '?':
    k = TOKEN_QUESTION;
    break;

  case ':':
    if (next_c == '>') {
      k = TOKEN_RBRACKET;
      extra = next_con;
    } else {
      k = TOKEN_COLON;
    }
    break;

  case '/':
    if (next_c == '/') {
      cur_pos += next_con;
      while (cur_pos < len) {
        size_t c_con;
        int lc = peek_logical(base, len, cur_pos, &c_con);
        if (lc == '\n' || lc == -1) {
          break;
        }
        cur_pos += c_con;
      }
      *pos = cur_pos;
      rc = token_list_add(list, TOKEN_COMMENT, base + start, cur_pos - start);
      return rc;
    } else if (next_c == '*') {
      cur_pos += next_con;
      while (cur_pos < len) {
        size_t c_con;
        int lc = peek_logical(base, len, cur_pos, &c_con);
        cur_pos += c_con;
        if (lc == '*') {
          size_t slash_con;
          if (peek_logical(base, len, cur_pos, &slash_con) == '/') {
            cur_pos += slash_con;
            break;
          }
        }
        if (lc == -1) {
          break;
        }
      }
      *pos = cur_pos;
      rc = token_list_add(list, TOKEN_COMMENT, base + start, cur_pos - start);
      return rc;
    } else if (next_c == '=') {
      k = TOKEN_DIV_ASSIGN;
      extra = next_con;
    } else {
      k = TOKEN_SLASH;
    }
    break;

  case '=':
    if (next_c == '=') {
      k = TOKEN_EQ;
      extra = next_con;
    } else {
      k = TOKEN_ASSIGN;
    }
    break;

  case '!':
    if (next_c == '=') {
      k = TOKEN_NEQ;
      extra = next_con;
    } else {
      k = TOKEN_BANG;
    }
    break;

  case '+':
    if (next_c == '+') {
      k = TOKEN_INC;
      extra = next_con;
    } else if (next_c == '=') {
      k = TOKEN_PLUS_ASSIGN;
      extra = next_con;
    } else {
      k = TOKEN_PLUS;
    }
    break;

  case '-':
    if (next_c == '-') {
      k = TOKEN_DEC;
      extra = next_con;
    } else if (next_c == '>') {
      k = TOKEN_ARROW;
      extra = next_con;
    } else if (next_c == '=') {
      k = TOKEN_MINUS_ASSIGN;
      extra = next_con;
    } else {
      k = TOKEN_MINUS;
    }
    break;

  case '*':
    if (next_c == '=') {
      k = TOKEN_MUL_ASSIGN;
      extra = next_con;
    } else {
      k = TOKEN_STAR;
    }
    break;

  case '%':
    if (next_c == '=') {
      k = TOKEN_MOD_ASSIGN;
      extra = next_con;
    } else if (next_c == '>') {
      k = TOKEN_RBRACE;
      extra = next_con;
    } else if (next_c == ':') {
      size_t c3_con, c4_con;
      int c3 = peek_logical(base, len, cur_pos + next_con, &c3_con);
      if (c3 == '%' && peek_logical(base, len, cur_pos + next_con + c3_con,
                                    &c4_con) == ':') {
        k = TOKEN_HASH_HASH;
        extra = next_con + c3_con + c4_con;
      } else {
        k = TOKEN_HASH;
        extra = next_con;
      }
    } else {
      k = TOKEN_PERCENT;
    }
    break;

  case '<':
    if (next_c == '=') {
      k = TOKEN_LEQ;
      extra = next_con;
    } else if (next_c == '<') {
      size_t c3_con;
      int c3 = peek_logical(base, len, cur_pos + next_con, &c3_con);
      if (c3 == '=') {
        k = TOKEN_LSHIFT_ASSIGN;
        extra = next_con + c3_con;
      } else {
        k = TOKEN_LSHIFT;
        extra = next_con;
      }
    } else if (next_c == '%') {
      k = TOKEN_LBRACE;
      extra = next_con;
    } else if (next_c == ':') {
      k = TOKEN_LBRACKET;
      extra = next_con;
    } else {
      k = TOKEN_LESS;
    }
    break;

  case '>':
    if (next_c == '=') {
      k = TOKEN_GEQ;
      extra = next_con;
    } else if (next_c == '>') {
      size_t c3_con;
      int c3 = peek_logical(base, len, cur_pos + next_con, &c3_con);
      if (c3 == '=') {
        k = TOKEN_RSHIFT_ASSIGN;
        extra = next_con + c3_con;
      } else {
        k = TOKEN_RSHIFT;
        extra = next_con;
      }
    } else {
      k = TOKEN_GREATER;
    }
    break;

  case '&':
    if (next_c == '&') {
      k = TOKEN_LOGICAL_AND;
      extra = next_con;
    } else if (next_c == '=') {
      k = TOKEN_AND_ASSIGN;
      extra = next_con;
    } else {
      k = TOKEN_AMP;
    }
    break;

  case '|':
    if (next_c == '|') {
      k = TOKEN_LOGICAL_OR;
      extra = next_con;
    } else if (next_c == '=') {
      k = TOKEN_OR_ASSIGN;
      extra = next_con;
    } else {
      k = TOKEN_PIPE;
    }
    break;

  case '^':
    if (next_c == '=') {
      k = TOKEN_XOR_ASSIGN;
      extra = next_con;
    } else {
      k = TOKEN_CARET;
    }
    break;

  case '.': {
    size_t peek_con;
    if (next_c == '.' &&
        peek_logical(base, len, cur_pos + next_con, &peek_con) == '.') {
      k = TOKEN_ELLIPSIS;
      extra = next_con + peek_con;
    } else {
      k = TOKEN_DOT;
    }
    break;
  }

  default:
    k = TOKEN_OTHER;
    break;
  }

  cur_pos += extra;
  *pos = cur_pos;
  rc = token_list_add(list, k, base + start, cur_pos - start);
  return rc;
}
