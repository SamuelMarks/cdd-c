/* clang-format off */
#include "cdd_lexer.h"
#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
/* clang-format on */

static cdd_trivia_t *alloc_trivia(enum cdd_trivia_kind_t kind,
                                  const uint8_t *start, size_t length) {
  cdd_trivia_t *t = (cdd_trivia_t *)calloc(1, sizeof(cdd_trivia_t));
  if (t) {
    t->kind = kind;
    t->start = start;
    t->length = length;
  }
  return t;
}

static void append_trivia(cdd_trivia_t **head, cdd_trivia_t **tail,
                          cdd_trivia_t *t) {
  if (!*head) {
    *head = t;
    *tail = t;
  } else {
    (*tail)->next = t;
    *tail = t;
  }
}

static int is_identifier_start(int c) {
  return isalpha(c) || c == '_' || c == '$';
}

static int is_identifier_part(int c) {
  return isalnum(c) || c == '_' || c == '$';
}

static enum cdd_token_kind_t classify_identifier(const uint8_t *start,
                                                 size_t len) {
  if (len == 3 && memcmp(start, "int", 3) == 0)
    return CDD_TOKEN_KEYWORD_INT;
  if (len == 8 && memcmp(start, "__int128", 8) == 0)
    return CDD_TOKEN_KEYWORD___INT128;
  if (len == 6 && memcmp(start, "typeof", 6) == 0)
    return CDD_TOKEN_KEYWORD_TYPEOF;
  if (len == 10 && memcmp(start, "__typeof__", 10) == 0)
    return CDD_TOKEN_KEYWORD_TYPEOF;
  if (len == 13 && memcmp(start, "typeof_unqual", 13) == 0)
    return CDD_TOKEN_KEYWORD_TYPEOF;
  if (len == 17 && memcmp(start, "__typeof_unqual__", 17) == 0)
    return CDD_TOKEN_KEYWORD_TYPEOF;
  if (len == 11 && memcmp(start, "__auto_type", 11) == 0)
    return CDD_TOKEN_KEYWORD___AUTO_TYPE;
  if (len == 11 && memcmp(start, "__complex__", 11) == 0)
    return CDD_TOKEN_KEYWORD___COMPLEX__;
  if (len == 8 && memcmp(start, "__real__", 8) == 0)
    return CDD_TOKEN_KEYWORD___REAL__;
  if (len == 8 && memcmp(start, "__imag__", 8) == 0)
    return CDD_TOKEN_KEYWORD___IMAG__;
  if (len == 9 && memcmp(start, "__label__", 9) == 0)
    return CDD_TOKEN_KEYWORD___LABEL__;
  if (len == 6 && memcmp(start, "struct", 6) == 0)
    return CDD_TOKEN_KEYWORD_STRUCT;
  if (len == 2 && memcmp(start, "if", 2) == 0)
    return CDD_TOKEN_KEYWORD_IF;
  if (len == 4 && memcmp(start, "else", 4) == 0)
    return CDD_TOKEN_KEYWORD_ELSE;
  if (len == 6 && memcmp(start, "return", 6) == 0)
    return CDD_TOKEN_KEYWORD_RETURN;
  if (len == 4 && memcmp(start, "goto", 4) == 0)
    return CDD_TOKEN_KEYWORD_GOTO;
  return CDD_TOKEN_IDENTIFIER;
}

int cdd_lexer_tokenize(az_span source, cdd_token_list_t **out_list) {
  cdd_token_list_t *list;
  const uint8_t *base = az_span_ptr(source);
  size_t len = (size_t)az_span_size(source);
  size_t pos = 0;
  size_t line = 1;
  size_t column = 1;
  cdd_trivia_t *pending_trivia_head = NULL;
  cdd_trivia_t *pending_trivia_tail = NULL;
  cdd_token_t *prev_token = NULL;

  if (!out_list)
    return EINVAL;

  list = (cdd_token_list_t *)calloc(1, sizeof(cdd_token_list_t));
  if (!list)
    return ENOMEM;

  list->capacity = 64;
  list->tokens = (cdd_token_t *)calloc(list->capacity, sizeof(cdd_token_t));
  if (!list->tokens) {
    free(list);
    return ENOMEM;
  }

  while (pos < len) {
    int c = base[pos];

    if (isspace(c)) {
      size_t start = pos;
      int is_newline = 0;
      while (pos < len && isspace(base[pos])) {
        if (base[pos] == '\n') {
          is_newline = 1;
          line++;
          column = 1;
        } else if (base[pos] != '\r') {
          column++;
        }
        pos++;
      }
      {
        cdd_trivia_t *t =
            alloc_trivia(is_newline ? TRIVIA_NEWLINE : TRIVIA_WHITESPACE,
                         base + start, pos - start);
        if (!t)
          goto error;
        if (!is_newline && prev_token && !pending_trivia_head) {
          cdd_trivia_t *tail = prev_token->trailing_trivia;
          if (!tail) {
            prev_token->trailing_trivia = t;
          } else {
            while (tail->next)
              tail = tail->next;
            tail->next = t;
          }
        } else {
          append_trivia(&pending_trivia_head, &pending_trivia_tail, t);
          prev_token = NULL;
        }
      }
      continue;
    }

    if (c == '/' && pos + 1 < len &&
        (base[pos + 1] == '/' || base[pos + 1] == '*')) {
      size_t start = pos;
      int is_block = (base[pos + 1] == '*');
      pos += 2;
      column += 2;
      if (is_block) {
        while (pos < len) {
          if (base[pos] == '\n') {
            line++;
            column = 1;
          } else if (base[pos] != '\r') {
            column++;
          }
          if (base[pos] == '*' && pos + 1 < len && base[pos + 1] == '/') {
            pos += 2;
            column += 2;
            break;
          }
          pos++;
        }
      } else {
        while (pos < len && base[pos] != '\n') {
          column++;
          pos++;
        }
      }
      {
        cdd_trivia_t *t =
            alloc_trivia(is_block ? TRIVIA_BLOCK_COMMENT : TRIVIA_LINE_COMMENT,
                         base + start, pos - start);
        if (!t)
          goto error;
        append_trivia(&pending_trivia_head, &pending_trivia_tail, t);
      }
      continue;
    }

    if (list->size >= list->capacity) {
      size_t new_cap = list->capacity * 2;
      cdd_token_t *new_arr =
          (cdd_token_t *)realloc(list->tokens, new_cap * sizeof(cdd_token_t));
      if (!new_arr)
        goto error;
      memset(new_arr + list->capacity, 0,
             (new_cap - list->capacity) * sizeof(cdd_token_t));
      list->tokens = new_arr;
      list->capacity = new_cap;
    }

    {
      cdd_token_t *tok = &list->tokens[list->size];
      tok->start = base + pos;
      tok->offset = pos;
      tok->line = line;
      tok->column = column;
      tok->leading_trivia = pending_trivia_head;
      pending_trivia_head = NULL;
      pending_trivia_tail = NULL;

      if (is_identifier_start(c)) {
        while (pos < len && is_identifier_part(base[pos])) {
          pos++;
          column++;
        }
        tok->length = pos - tok->offset;
        tok->kind = classify_identifier(tok->start, tok->length);
      } else if (isdigit(c)) {
        while (pos < len && (isalnum(base[pos]) || base[pos] == '.')) {
          pos++;
          column++;
        }
        tok->length = pos - tok->offset;
        tok->kind = CDD_TOKEN_NUMBER;
      } else if (c == '"' || c == '\'') {
        int quote = c;
        pos++;
        column++;
        while (pos < len) {
          if (base[pos] == '\\') {
            if (pos + 1 < len && base[pos + 1] == '\n') {
              pos += 2;
              line++;
              column = 1;
            } else if (pos + 2 < len && base[pos + 1] == '\r' &&
                       base[pos + 2] == '\n') {
              pos += 3;
              line++;
              column = 1;
            } else if (pos + 1 < len && base[pos + 1] == '\r') {
              pos += 2;
              line++;
              column = 1;
            } else {
              pos += 2;
              column += 2;
            }
          } else if (base[pos] == quote) {
            pos++;
            column++;
            break;
          } else {
            if (base[pos] == '\n') {
              line++;
              column = 1;
            } else if (base[pos] != '\r') {
              column++;
            }
            pos++;
          }
        }
        tok->length = pos - tok->offset;
        tok->kind = (quote == '"') ? CDD_TOKEN_STRING : CDD_TOKEN_CHAR;
      } else if (c == '#') {
        size_t id_start = pos + 1;
        while (id_start < len && isspace(base[id_start]) &&
               base[id_start] != '\n') {
          id_start++;
        }
        if (id_start < len && isalpha(base[id_start])) {
          size_t id_end = id_start;
          size_t id_len;
          while (id_end < len && isalpha(base[id_end]))
            id_end++;
          id_len = id_end - id_start;
          if (id_len == 7 && memcmp(base + id_start, "include", 7) == 0)
            tok->kind = CDD_TOKEN_PREPROC_INCLUDE;
          else if (id_len == 12 &&
                   memcmp(base + id_start, "include_next", 12) == 0)
            tok->kind = CDD_TOKEN_PREPROC_INCLUDE;
          else if (id_len == 6 && memcmp(base + id_start, "define", 6) == 0)
            tok->kind = CDD_TOKEN_PREPROC_DEFINE;
          else if (id_len == 5 && memcmp(base + id_start, "ifdef", 5) == 0)
            tok->kind = CDD_TOKEN_PREPROC_IFDEF;
          else if (id_len == 2 && memcmp(base + id_start, "if", 2) == 0)
            tok->kind = CDD_TOKEN_PREPROC_IFDEF;
          else if (id_len == 4 && memcmp(base + id_start, "elif", 4) == 0)
            tok->kind = CDD_TOKEN_PREPROC_ELIF;
          else if (id_len == 4 && memcmp(base + id_start, "else", 4) == 0)
            tok->kind = CDD_TOKEN_PREPROC_ELSE;
          else if (id_len == 6 && memcmp(base + id_start, "ifndef", 6) == 0)
            tok->kind = CDD_TOKEN_PREPROC_IFNDEF;
          else if (id_len == 5 && memcmp(base + id_start, "endif", 5) == 0)
            tok->kind = CDD_TOKEN_PREPROC_ENDIF;
          else if (id_len == 6 && memcmp(base + id_start, "pragma", 6) == 0)
            tok->kind = CDD_TOKEN_PREPROC_PRAGMA;
          else
            tok->kind = CDD_TOKEN_OTHER;
        } else {
          tok->kind = CDD_TOKEN_OTHER;
        }
        while (pos < len) {
          if (base[pos] == '\\' && pos + 1 < len && base[pos + 1] == '\n') {
            pos += 2;
            line++;
            column = 1;
          } else if (base[pos] == '\\' && pos + 2 < len &&
                     base[pos + 1] == '\r' && base[pos + 2] == '\n') {
            pos += 3;
            line++;
            column = 1;
          } else if (base[pos] == '\n') {
            break;
          } else {
            if (base[pos] != '\r') {
              column++;
            }
            pos++;
          }
        }
        tok->length = pos - tok->offset;
      } else {
        tok->length = 1;
        pos++;
        column++;
        switch (c) {
        case '{':
          tok->kind = CDD_TOKEN_LBRACE;
          break;
        case '}':
          tok->kind = CDD_TOKEN_RBRACE;
          break;
        case '(':
          tok->kind = CDD_TOKEN_LPAREN;
          break;
        case ')':
          tok->kind = CDD_TOKEN_RPAREN;
          break;
        case '[':
          tok->kind = CDD_TOKEN_LBRACKET;
          break;
        case ']':
          tok->kind = CDD_TOKEN_RBRACKET;
          break;
        case ';':
          tok->kind = CDD_TOKEN_SEMICOLON;
          break;
        case ',':
          tok->kind = CDD_TOKEN_COMMA;
          break;
        case '.':
          tok->kind = CDD_TOKEN_DOT;
          break;
        case '+':
          tok->kind = CDD_TOKEN_PLUS;
          break;
        case '-':
          if (pos < len && base[pos] == '>') {
            tok->length++;
            pos++;
            column++;
            tok->kind = CDD_TOKEN_ARROW;
          } else {
            tok->kind = CDD_TOKEN_MINUS;
          }
          break;
        case '*':
          tok->kind = CDD_TOKEN_STAR;
          break;
        case '/':
          tok->kind = CDD_TOKEN_SLASH;
          break;
        case '=':
          if (pos < len && base[pos] == '=') {
            tok->length++;
            pos++;
            column++;
            tok->kind = CDD_TOKEN_EQ;
          } else {
            tok->kind = CDD_TOKEN_ASSIGN;
          }
          break;
        case '!':
          if (pos < len && base[pos] == '=') {
            tok->length++;
            pos++;
            column++;
            tok->kind = CDD_TOKEN_NEQ;
          } else {
            tok->kind = CDD_TOKEN_OTHER;
          }
          break;
        default:
          tok->kind = CDD_TOKEN_OTHER;
          break;
        }
      }
      prev_token = tok;
      list->size++;
    }
  }

  if (pending_trivia_head) {
    if (list->size > 0) {
      cdd_trivia_t *tail = list->tokens[list->size - 1].trailing_trivia;
      if (!tail) {
        list->tokens[list->size - 1].trailing_trivia = pending_trivia_head;
      } else {
        while (tail->next)
          tail = tail->next;
        tail->next = pending_trivia_head;
      }
    } else {
      cdd_token_t *tok = &list->tokens[0];
      tok->kind = CDD_TOKEN_EOF;
      tok->start = base + pos;
      tok->length = 0;
      tok->offset = pos;
      tok->line = line;
      tok->column = column;
      tok->leading_trivia = pending_trivia_head;
      list->size = 1;
    }
  }

  *out_list = list;
  return 0;

error:
  cdd_lexer_free_token_list(list);
  return ENOMEM;
}

void cdd_lexer_free_token_list(cdd_token_list_t *list) {
  size_t i;
  if (!list)
    return;
  if (list->tokens) {
    for (i = 0; i < list->size; i++) {
      cdd_trivia_t *t = list->tokens[i].leading_trivia;
      while (t) {
        cdd_trivia_t *n = t->next;
        free(t);
        t = n;
      }
      t = list->tokens[i].trailing_trivia;
      while (t) {
        cdd_trivia_t *n = t->next;
        free(t);
        t = n;
      }
    }
    free(list->tokens);
  }
  free(list);
}
