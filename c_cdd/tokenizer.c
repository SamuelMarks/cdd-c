#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__)
#else
#include <sys/errno.h>
#endif
#include <c_str_span.h>

#include "tokenizer.h"

static int add_token(struct TokenList *const tl, const enum TokenKind kind,
                     const uint8_t *start, const size_t length) {
  if (tl->size >= tl->capacity) {
    const size_t new_capacity = tl->capacity == 0 ? 64 : tl->capacity * 2;
    struct Token *new_tokens =
        realloc(tl->tokens, new_capacity * sizeof(struct Token));
    if (!new_tokens)
      return ENOMEM;
    tl->tokens = new_tokens;
    tl->capacity = new_capacity;
  }
  tl->tokens[tl->size].kind = kind;
  tl->tokens[tl->size].start = start;
  tl->tokens[tl->size].length = length;
  tl->size++;
  return 0;
}

static bool is_keyword(const az_span str, const size_t len,
                       enum TokenKind *const kind_out) {
  if (len == 6 && az_span_is_content_equal(str, AZ_SPAN_FROM_STR("struct"))) {
    *kind_out = TOKEN_KEYWORD_STRUCT;
    return true;
  } else if (len == 4 &&
             az_span_is_content_equal(str, AZ_SPAN_FROM_STR("enum"))) {
    *kind_out = TOKEN_KEYWORD_ENUM;
    return true;
  } else if (len == 5 &&
             az_span_is_content_equal(str, AZ_SPAN_FROM_STR("union"))) {
    *kind_out = TOKEN_KEYWORD_UNION;
    return true;
  } else {
    return false;
  }
}
int tokenize(const az_span source, struct TokenList *const out) {
  size_t pos = 0;
  const size_t length = az_span_size(source);
  const uint8_t *const base = az_span_ptr(source);

  while (pos < length) {
    const uint8_t *const c = base + pos;
    switch (*c) {
    case ' ':
    case '\t':
    case '\r':
    case '\n':
    case '\f': {
      const size_t start = pos;
      while (pos < length) {
        uint8_t ch = base[pos];
        if (!(ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n' ||
              ch == '\f'))
          break;
        pos++;
      }
      if (add_token(out, TOKEN_WHITESPACE, base + start,
                    (int32_t)(pos - start)) != 0)
        return -1;
      break;
    }

    case '/': {
      if (pos + 1 < length) {
        if (base[pos + 1] == '/') {
          const size_t start = pos;
          pos += 2;
          while (pos < length && base[pos] != '\n')
            pos++;
          if (add_token(out, TOKEN_COMMENT, base + start,
                        (int32_t)(pos - start)) != 0)
            return -1;
        } else if (base[pos + 1] == '*') {
          const size_t start = pos;
          pos += 2;
          while (pos + 1 < length &&
                 !(base[pos] == '*' && base[pos + 1] == '/'))
            pos++;
          if (pos + 1 >= length) {
            // unterminated comment
            pos = length;
            if (add_token(out, TOKEN_COMMENT, base + start,
                          (int32_t)(pos - start)) != 0)
              return -1;
          } else {
            pos += 2;
            if (add_token(out, TOKEN_COMMENT, base + start,
                          (int32_t)(pos - start)) != 0)
              return -1;
          }
        } else {
          // single slash token
          if (add_token(out, TOKEN_OTHER, base + pos, 1) != 0)
            return -1;
          pos++;
        }
      } else {
        if (add_token(out, TOKEN_OTHER, base + pos, 1) != 0)
          return -1;
        pos++;
      }
      break;
    }

    case '#': {
      const size_t start = pos;
      pos++;
      while (pos < length && base[pos] != '\n')
        pos++;
      if (add_token(out, TOKEN_MACRO, base + start, (int32_t)(pos - start)) !=
          0)
        return -1;
      break;
    }

    case '{': {
      if (add_token(out, TOKEN_LBRACE, base + pos, 1) != 0)
        return -1;
      pos++;
      break;
    }

    case '}': {
      if (add_token(out, TOKEN_RBRACE, base + pos, 1) != 0)
        return -1;
      pos++;
      break;
    }

    case ';': {
      if (add_token(out, TOKEN_SEMICOLON, base + pos, 1) != 0)
        return -1;
      pos++;
      break;
    }

    case ',': {
      if (add_token(out, TOKEN_COMMA, base + pos, 1) != 0)
        return -1;
      pos++;
      break;
    }

    case '"': { // string literal
      const size_t start = pos;
      pos++; // skip initial "
      while (pos < length) {
        uint8_t ch = base[pos];
        if (ch == '\\' && pos + 1 < length) {
          // skip escaped character
          pos += 2;
          continue;
        }
        if (ch == '"') {
          pos++;
          break;
        }
        pos++;
      }
      if (add_token(out, TOKEN_STRING_LITERAL, base + start,
                    (int32_t)(pos - start)) != 0)
        return -1;
      break;
    }

    case '\'': { // char literal
      const size_t start = pos;
      pos++; // skip initial '
      if (pos < length) {
        if (base[pos] == '\\' && pos + 1 < length) {
          pos += 2; // skip escaped char
        } else {
          pos++;
        }
      }
      if (pos < length && base[pos] == '\'') {
        pos++;
      }
      // else unterminated char literal; accept as is
      if (add_token(out, TOKEN_CHAR_LITERAL, base + start,
                    (int32_t)(pos - start)) != 0)
        return -1;
      break;
    }

    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9': { // number literal
      const size_t start = pos;
      pos++;
      while (pos < length) {
        uint8_t ch = base[pos];
        if (!uint8_t_isalnum(ch))
          break;
        pos++;
      }
      if (add_token(out, TOKEN_NUMBER_LITERAL, base + start,
                    (int32_t)(pos - start)) != 0)
        return -1;
      break;
    }

    case '_':
    case 'a':
    case 'b':
    case 'c':
    case 'd':
    case 'e':
    case 'f':
    case 'g':
    case 'h':
    case 'i':
    case 'j':
    case 'k':
    case 'l':
    case 'm':
    case 'n':
    case 'o':
    case 'p':
    case 'q':
    case 'r':
    case 's':
    case 't':
    case 'u':
    case 'v':
    case 'w':
    case 'x':
    case 'y':
    case 'z':
    case 'A':
    case 'B':
    case 'C':
    case 'D':
    case 'E':
    case 'F':
    case 'G':
    case 'H':
    case 'I':
    case 'J':
    case 'K':
    case 'L':
    case 'M':
    case 'N':
    case 'O':
    case 'P':
    case 'Q':
    case 'R':
    case 'S':
    case 'T':
    case 'U':
    case 'V':
    case 'W':
    case 'X':
    case 'Y':
    case 'Z': {
      enum TokenKind kind;
      const size_t start = pos;
      pos++;
      while (pos < length && (uint8_t_isalnum(base[pos]) || base[pos] == '_'))
        pos++;
      if (is_keyword(
              az_span_slice(source, (int32_t)start, (int32_t)(pos - start)),
              (int32_t)(pos - start), &kind)) {
        if (add_token(out, kind, base + start, (int32_t)(pos - start)) != 0)
          return -1;
      } else {
        if (add_token(out, TOKEN_IDENTIFIER, base + start,
                      (int32_t)(pos - start)) != 0)
          return -1;
      }
      break;
    }

    default: {
      // single char OTHER token
      if (add_token(out, TOKEN_OTHER, base + pos, 1) != 0)
        return -1;
      pos++;
      break;
    }
    }
  }

  return 0;
}

void free_token_list(struct TokenList *const tl) {
  free(tl->tokens);
  tl->tokens = NULL;
  tl->size = 0;
  tl->capacity = 0;
}
