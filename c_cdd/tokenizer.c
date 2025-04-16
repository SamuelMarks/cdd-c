#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <sys/errno.h>

#include "tokenizer.h"

static int add_token(struct TokenList *const tl, const enum TokenKind kind,
                     const char *const start, const size_t length) {
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

static bool is_keyword(const char *str, const size_t len,
                       enum TokenKind *const kind_out) {
  if (len == 6 && strncmp(str, "struct", len) == 0) {
    *kind_out = TOKEN_KEYWORD_STRUCT;
    return true;
  } else if (len == 4 && strncmp(str, "enum", len) == 0) {
    *kind_out = TOKEN_KEYWORD_ENUM;
    return true;
  } else if (len == 5 && strncmp(str, "union", len) == 0) {
    *kind_out = TOKEN_KEYWORD_UNION;
    return true;
  } else {
    return false;
  }
}

int tokenize(const char *source, size_t length, struct TokenList *const out) {
  size_t pos = 0;
  while (pos < length) {
    const char c = source[pos];
    switch (c) {
    case ' ':
    case '\t':
    case '\r':
    case '\n':
    case '\f': {
      const size_t start = pos;
      while (pos < length && (source[pos] == ' ' || source[pos] == '\t' ||
                              source[pos] == '\r' || source[pos] == '\n' ||
                              source[pos] == '\f'))
        pos++;
      if (add_token(out, TOKEN_WHITESPACE, source + start, pos - start) != 0)
        return -1;
      continue;
    }
    case '/': {
      if (pos + 1 < length) {
        if (source[pos + 1] == '/') {
          const size_t start = pos;
          pos += 2;
          while (pos < length && source[pos] != '\n')
            pos++;
          if (add_token(out, TOKEN_COMMENT, source + start, pos - start) != 0)
            return -1;
          continue;
        } else if (source[pos + 1] == '*') {
          const size_t start = pos;
          pos += 2;
          while (pos + 1 < length &&
                 !(source[pos] == '*' && source[pos + 1] == '/'))
            pos++;
          if (pos + 1 >= length) {
            pos = length; /* unterminated comment */
            if (add_token(out, TOKEN_COMMENT, source + start, pos - start) != 0)
              return -1;
            break;
          }
          pos += 2;
          if (add_token(out, TOKEN_COMMENT, source + start, pos - start) != 0)
            return -1;
          continue;
        }
      }
      /* Single slash is OTHER token */
      if (add_token(out, TOKEN_OTHER, source + pos, 1) != 0)
        return -1;
      pos++;
      continue;
    }
    case '#': {
      const size_t start = pos;
      pos++;
      while (pos < length && source[pos] != '\n')
        pos++;
      if (add_token(out, TOKEN_MACRO, source + start, pos - start) != 0)
        return -1;
      continue;
    }
    case '{': {
      if (add_token(out, TOKEN_LBRACE, source + pos, 1) != 0)
        return -1;
      pos++;
      continue;
    }
    case '}': {
      if (add_token(out, TOKEN_RBRACE, source + pos, 1) != 0)
        return -1;
      pos++;
      continue;
    }
    case ';': {
      if (add_token(out, TOKEN_SEMICOLON, source + pos, 1) != 0)
        return -1;
      pos++;
      continue;
    }
    case ',': {
      if (add_token(out, TOKEN_COMMA, source + pos, 1) != 0)
        return -1;
      pos++;
      continue;
    }
      /* TODO: Quotes and checking if whole WORD is a number */

    case '_':
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
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
      const size_t start = pos;
      enum TokenKind kind;
      pos++;
      while (pos < length &&
             (isalnum((unsigned char)source[pos]) || source[pos] == '_'))
        pos++;
      if (is_keyword(source + start, pos - start, &kind)) {
        if (add_token(out, kind, source + start, pos - start) != 0)
          return -1;
      } else {
        if (add_token(out, TOKEN_IDENTIFIER, source + start, pos - start) != 0)
          return -1;
      }
      break;
    }
    default: {
      /* single char OTHER token */
      if (add_token(out, TOKEN_OTHER, source + pos, 1) != 0)
        return -1;
      pos++;
    } break;
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
