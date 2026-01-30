/**
 * @file tokenizer.c
 * @brief Implementation of the lexical analyzer.
 * @author Samuel Marks
 */

#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__)
#else
#include <sys/errno.h>
#endif
#include <c_str_span.h>

#include "tokenizer.h"

/**
 * @brief Append a new token to the TokenList, resizing if necessary.
 *
 * @param[in] tl The list to append to.
 * @param[in] kind The classification of the new token.
 * @param[in] start Pointer to the start of the token text.
 * @param[in] length Byte length of the token.
 * @return 0 on success, ENOMEM on allocation failure.
 */
static int add_token(struct TokenList *const tl, const enum TokenKind kind,
                     const uint8_t *start, const size_t length) {
  if (tl->size >= tl->capacity) {
    const size_t new_capacity = tl->capacity == 0 ? 64 : tl->capacity * 2;
    struct Token *new_tokens = (struct Token *)realloc(
        tl->tokens, new_capacity * sizeof(struct Token));
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

/**
 * @brief Check if span matches a specific keyword and set token kind.
 *
 * @param[in] str Span of text to check.
 * @param[in] len Length of text.
 * @param[out] kind_out classification if match found.
 * @return true if match found, false otherwise.
 */
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
  const size_t length = (size_t)az_span_size(source);
  const uint8_t *const base = az_span_ptr(source);
  int rc;

  /* Initialize out structure to be safe if not already zeroed */
  if (out->capacity == 0) {
    out->tokens = NULL;
    out->size = 0;
  }

  while (pos < length) {
    const uint8_t c = base[pos];
    const size_t start = pos;

    if (isspace((unsigned char)c)) {
      pos++;
      while (pos < length && isspace((unsigned char)base[pos])) {
        pos++;
      }
      rc = add_token(out, TOKEN_WHITESPACE, base + start, pos - start);
      if (rc != 0)
        return rc;

    } else if (c == '/' && pos + 1 < length && base[pos + 1] == '/') {
      /* Single line comment */
      pos += 2;
      while (pos < length && base[pos] != '\n') {
        pos++;
      }
      rc = add_token(out, TOKEN_COMMENT, base + start, pos - start);
      if (rc != 0)
        return rc;

    } else if (c == '/' && pos + 1 < length && base[pos + 1] == '*') {
      /* Block comment */
      pos += 2;
      while (pos + 1 < length && !(base[pos] == '*' && base[pos + 1] == '/')) {
        pos++;
      }
      if (pos + 1 < length) {
        pos += 2; /* Include trailing asterisk-slash */
      } else {
        pos = length; /* Unterminated comment takes rest of file */
      }
      rc = add_token(out, TOKEN_COMMENT, base + start, pos - start);
      if (rc != 0)
        return rc;

    } else if (c == '#') {
      pos++;
      while (pos < length && base[pos] != '\n') {
        pos++;
      }
      rc = add_token(out, TOKEN_MACRO, base + start, pos - start);
      if (rc != 0)
        return rc;

    } else if (isalpha((unsigned char)c) || c == '_') {
      /* Identifier or Keyword */
      pos++;
      while (pos < length &&
             (isalnum((unsigned char)base[pos]) || base[pos] == '_')) {
        pos++;
      }
      {
        enum TokenKind kind;
        if (is_keyword(az_span_slice(source, (int32_t)start, (int32_t)pos),
                       pos - start, &kind)) {
          rc = add_token(out, kind, base + start, pos - start);
        } else {
          rc = add_token(out, TOKEN_IDENTIFIER, base + start, pos - start);
        }
        if (rc != 0)
          return rc;
      }

    } else if (isdigit((unsigned char)c)) {
      pos++;
      while (pos < length && isdigit((unsigned char)base[pos])) {
        pos++;
      }
      rc = add_token(out, TOKEN_NUMBER_LITERAL, base + start, pos - start);
      if (rc != 0)
        return rc;

    } else if (c == '"') {
      pos++;
      while (pos < length && base[pos] != '"') {
        if (base[pos] == '\\' && pos + 1 < length)
          pos++;
        pos++;
      }
      if (pos < length)
        pos++;
      rc = add_token(out, TOKEN_STRING_LITERAL, base + start, pos - start);
      if (rc != 0)
        return rc;

    } else if (c == '\'') {
      pos++;
      while (pos < length && base[pos] != '\'') {
        if (base[pos] == '\\' && pos + 1 < length)
          pos++;
        pos++;
      }
      if (pos < length)
        pos++;
      rc = add_token(out, TOKEN_CHAR_LITERAL, base + start, pos - start);
      if (rc != 0)
        return rc;

    } else {
      enum TokenKind k = TOKEN_UNKNOWN;
      switch (c) {
      case '{':
        k = TOKEN_LBRACE;
        break;
      case '}':
        k = TOKEN_RBRACE;
        break;
      case ';':
        k = TOKEN_SEMICOLON;
        break;
      case ',':
        k = TOKEN_COMMA;
        break;
      default:
        k = TOKEN_OTHER;
        break;
      }
      rc = add_token(out, k, base + start, 1);
      if (rc != 0)
        return rc;
      pos++;
    }
  }
  return 0;
}

void free_token_list(struct TokenList *const tl) {
  if (tl == NULL)
    return;
  free(tl->tokens);
  tl->tokens = NULL;
  tl->size = 0;
  tl->capacity = 0;
}
