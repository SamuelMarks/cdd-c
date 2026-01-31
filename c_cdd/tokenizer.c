/**
 * @file tokenizer.c
 * @brief Implementation of the C tokenizer.
 *
 * Loops through the character stream, identifying boundaries based on handling
 * C89 syntax rules including comments, string literals (with escapes), and
 * keywords.
 *
 * @author Samuel Marks
 */

#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "tokenizer.h"

/* --- Internal Helpers --- */

static int token_list_add(struct TokenList *const tl, const enum TokenKind kind,
                          const uint8_t *start, const size_t length) {
  if (!tl)
    return EINVAL;

  if (tl->size >= tl->capacity) {
    const size_t new_cap = (tl->capacity == 0) ? 64 : tl->capacity * 2;
    struct Token *new_arr =
        (struct Token *)realloc(tl->tokens, new_cap * sizeof(struct Token));
    if (!new_arr)
      return ENOMEM;
    tl->tokens = new_arr;
    tl->capacity = new_cap;
  }

  tl->tokens[tl->size].kind = kind;
  tl->tokens[tl->size].start = start;
  tl->tokens[tl->size].length = length;
  tl->size++;
  return 0;
}

static bool span_equals_str(const az_span span, const char *str) {
  return az_span_is_content_equal(span, az_span_create_from_str((char *)str));
}

static enum TokenKind identify_keyword_or_id(const uint8_t *start, size_t len) {
  az_span s = az_span_create((uint8_t *)start, (int32_t)len);

  if (span_equals_str(s, "struct"))
    return TOKEN_KEYWORD_STRUCT;
  if (span_equals_str(s, "enum"))
    return TOKEN_KEYWORD_ENUM;
  if (span_equals_str(s, "union"))
    return TOKEN_KEYWORD_UNION;

  return TOKEN_IDENTIFIER;
}

/* --- Public API --- */

bool token_matches_string(const struct Token *const tok,
                          const char *const match) {
  size_t len;
  if (!tok || !match)
    return false;

  len = strlen(match);
  if (tok->length != len)
    return false;

  return strncmp((const char *)tok->start, match, len) == 0;
}

size_t token_find_next(const struct TokenList *const list, size_t start_idx,
                       size_t end_idx, const enum TokenKind kind) {
  size_t i;
  size_t limit = (end_idx < list->size) ? end_idx : list->size;

  for (i = start_idx; i < limit; ++i) {
    if (list->tokens[i].kind == kind)
      return i;
  }
  return limit; /* Not found */
}

void free_token_list(struct TokenList *const tl) {
  if (!tl)
    return;
  if (tl->tokens) {
    free(tl->tokens);
    tl->tokens = NULL;
  }
  free(tl);
}

int tokenize(const az_span source, struct TokenList **const out) {
  struct TokenList *list = NULL;
  const uint8_t *base;
  size_t len, pos = 0;
  int rc = 0;

  if (!out)
    return EINVAL;

  base = az_span_ptr(source);
  len = (size_t)az_span_size(source);

  list = (struct TokenList *)calloc(1, sizeof(struct TokenList));
  if (!list)
    return ENOMEM;

  while (pos < len) {
    const uint8_t c = base[pos];
    const size_t start = pos;

    if (isspace((unsigned char)c)) {
      pos++;
      while (pos < len && isspace((unsigned char)base[pos]))
        pos++;
      rc = token_list_add(list, TOKEN_WHITESPACE, base + start, pos - start);

    } else if (c == '/' && pos + 1 < len) {
      if (base[pos + 1] == '/') { /* Single line */
        pos += 2;
        while (pos < len && base[pos] != '\n')
          pos++;
        rc = token_list_add(list, TOKEN_COMMENT, base + start, pos - start);
      } else if (base[pos + 1] == '*') { /* Block */
        pos += 2;
        while (pos + 1 < len && !(base[pos] == '*' && base[pos + 1] == '/'))
          pos++;
        if (pos + 1 < len)
          pos += 2;
        else
          pos = len;
        rc = token_list_add(list, TOKEN_COMMENT, base + start, pos - start);
      } else {
        goto lex_single_char;
      }

    } else if (c == '#') {
      pos++;
      while (pos < len && base[pos] != '\n')
        pos++;
      rc = token_list_add(list, TOKEN_MACRO, base + start, pos - start);

    } else if (isalpha((unsigned char)c) || c == '_') {
      pos++;
      while (pos < len &&
             (isalnum((unsigned char)base[pos]) || base[pos] == '_'))
        pos++;
      {
        size_t id_len = pos - start;
        enum TokenKind k = identify_keyword_or_id(base + start, id_len);
        rc = token_list_add(list, k, base + start, id_len);
      }

    } else if (isdigit((unsigned char)c)) {
      pos++;
      /* Minimal check: consume digits. Complex floats/hex handled basically
       * here */
      while (pos < len &&
             (isalnum((unsigned char)base[pos]) || base[pos] == '.'))
        pos++;
      rc =
          token_list_add(list, TOKEN_NUMBER_LITERAL, base + start, pos - start);

    } else if (c == '"') {
      pos++;
      while (pos < len && base[pos] != '"') {
        if (base[pos] == '\\' && pos + 1 < len)
          pos++;
        pos++;
      }
      if (pos < len)
        pos++; /* Consume closing */
      rc =
          token_list_add(list, TOKEN_STRING_LITERAL, base + start, pos - start);

    } else if (c == '\'') {
      pos++;
      while (pos < len && base[pos] != '\'') {
        if (base[pos] == '\\' && pos + 1 < len)
          pos++;
        pos++;
      }
      if (pos < len)
        pos++;
      rc = token_list_add(list, TOKEN_CHAR_LITERAL, base + start, pos - start);

    } else {
    lex_single_char: {
      enum TokenKind k = TOKEN_OTHER;
      switch (c) {
      case '{':
        k = TOKEN_LBRACE;
        break;
      case '}':
        k = TOKEN_RBRACE;
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
      default:
        k = TOKEN_OTHER;
        break;
      }
      pos++;
      rc = token_list_add(list, k, base + start, 1);
    }
    }

    if (rc != 0) {
      free_token_list(list);
      *out = NULL;
      return rc;
    }
  }

  *out = list;
  return 0;
}
