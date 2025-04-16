#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <c_cdd_export.h>
#include <stddef.h>

C_CDD_EXPORT enum TokenKind {
  TOKEN_UNKNOWN = 0,
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
  TOKEN_OTHER,
};

C_CDD_EXPORT struct Token {
  enum TokenKind kind;
  const char *start;
  size_t length;
};

C_CDD_EXPORT struct TokenList {
  struct Token *tokens;
  size_t size;
  size_t capacity;
};

extern C_CDD_EXPORT int tokenize(const char *, size_t, struct TokenList *);

extern C_CDD_EXPORT void free_token_list(struct TokenList *);

#endif /* TOKENIZER_H */
