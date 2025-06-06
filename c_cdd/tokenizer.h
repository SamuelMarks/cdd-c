#ifndef TOKENIZER_H
#define TOKENIZER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <c_cdd_export.h>
#if __STDC_VERSION__ >= 199901L
#include <stdbool.h>
#else
#include <c_cdd_stdbool.h>
#endif /* __STDC_VERSION__ >= 199901L */
#include <c_str_span.h>

enum C_CDD_EXPORT TokenKind {
  TOKEN_WHITESPACE,
  TOKEN_COMMENT,
  TOKEN_MACRO,
  TOKEN_KEYWORD_STRUCT,
  TOKEN_KEYWORD_ENUM,
  TOKEN_KEYWORD_UNION,
  TOKEN_IDENTIFIER,
  TOKEN_LBRACE,
  TOKEN_RBRACE,
  TOKEN_SEMICOLON,
  TOKEN_COMMA,
  TOKEN_NUMBER_LITERAL,
  TOKEN_STRING_LITERAL,
  TOKEN_CHAR_LITERAL,
  TOKEN_OTHER,
  TOKEN_UNKNOWN = -1
};

struct C_CDD_EXPORT Token {
  enum TokenKind kind;
  const uint8_t *start;
  size_t length;
};

struct C_CDD_EXPORT TokenList {
  struct Token *tokens;
  size_t size;
  size_t capacity;
};

extern C_CDD_EXPORT int tokenize(az_span, struct TokenList *);

extern C_CDD_EXPORT void free_token_list(struct TokenList *);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TOKENIZER_H */
