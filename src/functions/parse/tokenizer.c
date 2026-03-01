/**
 * @file tokenizer.c
 * @brief Implementation of the C tokenizer with Phase 1/2 support.
 *
 * Loops through the character stream, identifying boundaries based on handling
 * C89/C99/C11/C23 syntax rules.
 * Features:
 * - Trigraph replacement (Phase 1).
 * - Line Splicing (Phase 2) even inside identifiers.
 * - Unified logical char reading stream.
 * - C23 Digit Separators (e.g. `123'456`).
 *
 * @author Samuel Marks
 */

#include <ctype.h>

#include <errno.h>

#include <stdlib.h>

#include <string.h>

#include "functions/parse/tokenizer.h"

/* --- Phase 1 & 2 Logic --- */

/**
 * @brief Check if a sequence is a trigraph and return its definition.
 *
 * @param c3 The third character of `??X`.
 * @return The replacement char or 0 if not a trigraph.
 */

static int get_trigraph_map(int c3) {

  switch (c3) {

  case '=':

    return '#';

  case '(':

    return '[';

  case '/':

    return '\\';

  case ')':

    return ']';

  case '\'':

    return '^';

  case '<':

    return '{';

  case '!':

    return '|';

  case '>':

    return '}';

  case '-':

    return '~';

  default:

    return 0;
  }
}

/**
 * @brief Peek the next logical character from the buffer.
 *
 * Handles Phase 1 (Trigraphs) and Phase 2 (Backslash-Newline Splicing).
 *
 * @param base Source buffer.
 * @param len Buffer length.
 * @param pos Current physical position.
 * @param out_consumed Output: How many physical bytes constitute this logical
 * char. Returns 0 if EOF.
 * @return The logical character (int), or EOF (-1) if end of buffer.
 */

static int peek_logical(const uint8_t *base, size_t len, size_t pos,

                        size_t *out_consumed) {

  size_t current = pos;

  while (current < len) {

    int c = base[current];

    size_t char_len = 1;

    /* Phase 1: Trigraphs */

    /* Check for ?? */

    if (c == '?' && current + 2 < len && base[current + 1] == '?') {

      int mapped = get_trigraph_map(base[current + 2]);

      if (mapped) {

        c = mapped;

        char_len = 3;
      }
    }

    /* Phase 2: Splicing */

    if (c == '\\') {

      size_t next_idx = current + char_len;

      /* Check if next logical char is newline */

      if (next_idx < len) {

        if (base[next_idx] == '\n') {

          /* \ \n */

          current = next_idx + 1; /* Skip \ and \n */

          continue; /* Loop to get next char after splice */

        } else if (base[next_idx] == '\r' && next_idx + 1 < len &&

                   base[next_idx + 1] == '\n') {

          /* \ \r \n */

          current = next_idx + 2;

          continue;
        }
      }
    }

    /* If we got here, we found a real logical char */

    *out_consumed = (current - pos) + char_len;

    return c;
  }

  *out_consumed = 0;

  return -1; /* EOF */
}

/* --- Token List Setup --- */

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

/* Helper to classify keywords */

enum TokenKind identify_keyword_or_id(const uint8_t *start, size_t len) {

  az_span s = az_span_create((uint8_t *)start, (int32_t)len);

  /* C89/C90/C99/C11/C23 Keywords */

  if (span_equals_str(s, "auto"))

    return TOKEN_KEYWORD_AUTO;

  if (span_equals_str(s, "break"))

    return TOKEN_KEYWORD_BREAK;

  if (span_equals_str(s, "case"))

    return TOKEN_KEYWORD_CASE;

  if (span_equals_str(s, "char"))

    return TOKEN_KEYWORD_CHAR;

  if (span_equals_str(s, "const"))

    return TOKEN_KEYWORD_CONST;

  if (span_equals_str(s, "continue"))

    return TOKEN_KEYWORD_CONTINUE;

  if (span_equals_str(s, "default"))

    return TOKEN_KEYWORD_DEFAULT;

  if (span_equals_str(s, "do"))

    return TOKEN_KEYWORD_DO;

  if (span_equals_str(s, "double"))

    return TOKEN_KEYWORD_DOUBLE;

  if (span_equals_str(s, "else"))

    return TOKEN_KEYWORD_ELSE;

  if (span_equals_str(s, "enum"))

    return TOKEN_KEYWORD_ENUM;

  if (span_equals_str(s, "extern"))

    return TOKEN_KEYWORD_EXTERN;

  if (span_equals_str(s, "float"))

    return TOKEN_KEYWORD_FLOAT;

  if (span_equals_str(s, "for"))

    return TOKEN_KEYWORD_FOR;

  if (span_equals_str(s, "goto"))

    return TOKEN_KEYWORD_GOTO;

  if (span_equals_str(s, "if"))

    return TOKEN_KEYWORD_IF;

  if (span_equals_str(s, "inline"))

    return TOKEN_KEYWORD_INLINE;

  if (span_equals_str(s, "int"))

    return TOKEN_KEYWORD_INT;

  if (span_equals_str(s, "long"))

    return TOKEN_KEYWORD_LONG;

  if (span_equals_str(s, "register"))

    return TOKEN_KEYWORD_REGISTER;

  if (span_equals_str(s, "restrict"))

    return TOKEN_KEYWORD_RESTRICT;

  if (span_equals_str(s, "return"))

    return TOKEN_KEYWORD_RETURN;

  if (span_equals_str(s, "short"))

    return TOKEN_KEYWORD_SHORT;

  if (span_equals_str(s, "signed"))

    return TOKEN_KEYWORD_SIGNED;

  if (span_equals_str(s, "sizeof"))

    return TOKEN_KEYWORD_SIZEOF;

  if (span_equals_str(s, "static"))

    return TOKEN_KEYWORD_STATIC;

  if (span_equals_str(s, "struct"))

    return TOKEN_KEYWORD_STRUCT;

  if (span_equals_str(s, "switch"))

    return TOKEN_KEYWORD_SWITCH;

  if (span_equals_str(s, "typedef"))

    return TOKEN_KEYWORD_TYPEDEF;

  if (span_equals_str(s, "union"))

    return TOKEN_KEYWORD_UNION;

  if (span_equals_str(s, "unsigned"))

    return TOKEN_KEYWORD_UNSIGNED;

  if (span_equals_str(s, "void"))

    return TOKEN_KEYWORD_VOID;

  if (span_equals_str(s, "volatile"))

    return TOKEN_KEYWORD_VOLATILE;

  if (span_equals_str(s, "while"))

    return TOKEN_KEYWORD_WHILE;

  if (span_equals_str(s, "_Alignas"))

    return TOKEN_KEYWORD_ALIGNAS;

  if (span_equals_str(s, "_Alignof"))

    return TOKEN_KEYWORD_ALIGNOF;

  if (span_equals_str(s, "_Atomic"))

    return TOKEN_KEYWORD_ATOMIC;

  if (span_equals_str(s, "_Bool"))

    return TOKEN_KEYWORD_BOOL;

  if (span_equals_str(s, "_Complex"))

    return TOKEN_KEYWORD_COMPLEX;

  if (span_equals_str(s, "_Imaginary"))

    return TOKEN_KEYWORD_IMAGINARY;

  if (span_equals_str(s, "_Noreturn"))

    return TOKEN_KEYWORD_NORETURN;

  if (span_equals_str(s, "_Static_assert"))

    return TOKEN_KEYWORD_STATIC_ASSERT;

  if (span_equals_str(s, "_Thread_local"))

    return TOKEN_KEYWORD_THREAD_LOCAL;

  /* Extensions found in common headers */

  if (span_equals_str(s, "__inline"))

    return TOKEN_KEYWORD_INLINE;

  if (span_equals_str(s, "__restrict"))

    return TOKEN_KEYWORD_RESTRICT;

  /* C23 standard keywords */

  if (span_equals_str(s, "alignas"))

    return TOKEN_KEYWORD_ALIGNAS;

  if (span_equals_str(s, "alignof"))

    return TOKEN_KEYWORD_ALIGNOF;

  if (span_equals_str(s, "bool"))

    return TOKEN_KEYWORD_BOOL;

  if (span_equals_str(s, "constexpr"))

    return TOKEN_KEYWORD_CONSTEXPR;

  if (span_equals_str(s, "false"))

    return TOKEN_KEYWORD_FALSE;

  if (span_equals_str(s, "nullptr"))

    return TOKEN_KEYWORD_NULLPTR;

  if (span_equals_str(s, "static_assert"))

    return TOKEN_KEYWORD_STATIC_ASSERT;

  if (span_equals_str(s, "thread_local"))

    return TOKEN_KEYWORD_THREAD_LOCAL;

  if (span_equals_str(s, "true"))

    return TOKEN_KEYWORD_TRUE;

  if (span_equals_str(s, "typeof"))

    return TOKEN_KEYWORD_TYPEOF;

  if (span_equals_str(s, "embed"))

    return TOKEN_KEYWORD_EMBED;

  if (span_equals_str(s, "_Pragma"))

    return TOKEN_KEYWORD_PRAGMA_OP;

  return TOKEN_IDENTIFIER;
}

/* --- Main Public API --- */

size_t token_find_next(const struct TokenList *const list, size_t start_idx,

                       size_t end_idx, const enum TokenKind kind) {

  size_t i;

  size_t limit = (end_idx < list->size) ? end_idx : list->size;

  for (i = start_idx; i < limit; ++i) {

    if (list->tokens[i].kind == kind)

      return i;
  }

  return limit;
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

bool token_matches_string(const struct Token *const tok,

                          const char *const match) {

  size_t m_len;

  size_t i_tok = 0, i_match = 0;

  const uint8_t *t;

  if (!tok || !match)

    return false;

  m_len = strlen(match);

  t = tok->start;

  while (i_tok < tok->length && i_match < m_len) {

    int c;

    size_t adv;

    c = peek_logical(t + i_tok, tok->length - i_tok, 0, &adv);

    if (c == -1)

      break;

    if (c != match[i_match])

      return false;

    i_tok += adv;

    i_match++;
  }

  return (i_tok >= tok->length && i_match == m_len);
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

    size_t consumed;

    int c = peek_logical(base, len, pos, &consumed);

    size_t start = pos;

    if (c == -1)

      break;

    if (isspace(c)) {

      pos += consumed;

      while (pos < len) {

        int nc = peek_logical(base, len, pos, &consumed);

        if (nc != -1 && isspace(nc)) {

          pos += consumed;

        } else {

          break;
        }
      }

      rc = token_list_add(list, TOKEN_WHITESPACE, base + start, pos - start);

    } else if (c == '#') {

      size_t next_con;

      pos += consumed;

      if (peek_logical(base, len, pos, &next_con) == '#') {

        pos += next_con;

        rc = token_list_add(list, TOKEN_HASH_HASH, base + start, pos - start);

      } else {

        rc = token_list_add(list, TOKEN_HASH, base + start, pos - start);
      }

    } else if (isalpha(c) || c == '_' || c == '\\') {

      /* Potential Identifier or Keyword */

      /* Identify if this is a UCN escape starting an ID */

      if (c == '\\') {

        size_t u_con;

        int u = peek_logical(base, len, pos + consumed, &u_con);

        if (u != 'u' && u != 'U') {

          /* Not a UCN start (\ is not followed by u/U), token OTHER/PUNCT(none)

           */

          pos += consumed;

          rc = token_list_add(list, TOKEN_OTHER, base + start, consumed);

          goto check_rc;
        }

        /* Verify subsequent hex digit */

        {

          size_t hex_con;

          int h = peek_logical(base, len, pos + consumed + u_con, &hex_con);

          if (!isxdigit(h)) {

            /* Invalid UCN start: \u not followed by hex */

            pos += consumed;

            rc = token_list_add(list, TOKEN_OTHER, base + start, consumed);

            goto check_rc;
          }
        }
      }

      pos += consumed;

      while (pos < len) {

        size_t nc_con;

        int nc = peek_logical(base, len, pos, &nc_con);

        if (nc != -1 && (isalnum(nc) || nc == '_' || nc == '\\')) {

          if (nc == '\\') {

            /* UCN logic: must be \ u|U hex... */

            size_t u_con, hex_con;

            int u = peek_logical(base, len, pos + nc_con, &u_con);

            if (u == 'u' || u == 'U') {

              /* Peek first hex char to ensure it is valid ID part */

              int h = peek_logical(base, len, pos + nc_con + u_con, &hex_con);

              if (!isxdigit(h)) {

                break; /* Not a valid UCN, break ID here */
              }

            } else {

              break;
            }
          }

          pos += nc_con;

        } else {

          break;
        }
      }

      {

        size_t id_len = pos - start;

        struct Token tmp_tok;

        enum TokenKind k;

        /* Check raw text for common keywords */

        k = identify_keyword_or_id(base + start, id_len);

        /* If still ID, check strict logic for spliced keywords if needed */

        if (k == TOKEN_IDENTIFIER) {

          tmp_tok.start = base + start;

          tmp_tok.length = id_len;

          /* Add checks for spliced keywords if necessary */

          if (token_matches_string(&tmp_tok, "int"))

            k = TOKEN_KEYWORD_INT;

          else if (token_matches_string(&tmp_tok, "return"))

            k = TOKEN_KEYWORD_RETURN;

          else if (token_matches_string(&tmp_tok, "switch"))

            k = TOKEN_KEYWORD_SWITCH;

          else if (token_matches_string(&tmp_tok, "if"))

            k = TOKEN_KEYWORD_IF;
        }

        rc = token_list_add(list, k, base + start, id_len);
      }

    } else if (isdigit(c) ||

               (c == '.' &&

                isdigit(peek_logical(base, len, pos + consumed, &consumed)))) {

      if (c == '.') {

        peek_logical(base, len, pos, &consumed);
      }

      pos += consumed;

      while (pos < len) {

        int nc = peek_logical(base, len, pos, &consumed);

        /* C23 Digit Separators: Allow '\'' inside number if followed by alnum

         */

        if (nc == '\'') {

          size_t next_peek_con;

          int next_peek =

              peek_logical(base, len, pos + consumed, &next_peek_con);

          if (isalnum(next_peek)) {

            /* Valid separator `123'456` */

            pos += consumed; /* Consume quote */

            continue;

          } else {

            /* `'` not followed by digit/letter, end of number */

            break;
          }
        }

        if (nc != -1 && (isalnum(nc) || nc == '_' || nc == '.')) {

          pos += consumed;

        } else {

          break;
        }
      }

      rc =

          token_list_add(list, TOKEN_NUMBER_LITERAL, base + start, pos - start);

    } else if (c == '"' || c == '\'') {

      int quote = c;

      pos += consumed;

      while (pos < len) {

        int nc = peek_logical(base, len, pos, &consumed);

        if (nc == -1)

          break;

        pos += consumed;

        if (nc == '\\') {

          peek_logical(base, len, pos, &consumed);

          pos += consumed;

        } else if (nc == quote) {

          break;
        }
      }

      rc = token_list_add(

          list, (quote == '"' ? TOKEN_STRING_LITERAL : TOKEN_CHAR_LITERAL),

          base + start, pos - start);

    } else {

      enum TokenKind k = TOKEN_OTHER;

      size_t extra = 0;

      int next_c;

      size_t next_con;

      pos += consumed;

      next_c = peek_logical(base, len, pos, &next_con);

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

        } else

          k = TOKEN_COLON;

        break;

      case '/':

        if (next_c == '/') {

          pos += next_con;

          while (pos < len) {

            int lc = peek_logical(base, len, pos, &consumed);

            if (lc == '\n' || lc == -1)

              break;

            pos += consumed;
          }

          rc = token_list_add(list, TOKEN_COMMENT, base + start, pos - start);

          goto check_rc;

        } else if (next_c == '*') {

          pos += next_con;

          while (pos < len) {

            int lc = peek_logical(base, len, pos, &consumed);

            pos += consumed;

            if (lc == '*') {

              if (peek_logical(base, len, pos, &consumed) == '/') {

                pos += consumed;

                break;
              }
            }

            if (lc == -1)

              break;
          }

          rc = token_list_add(list, TOKEN_COMMENT, base + start, pos - start);

          goto check_rc;

        } else if (next_c == '=') {

          k = TOKEN_DIV_ASSIGN;

          extra = next_con;

        } else

          k = TOKEN_SLASH;

        break;

      case '=':

        if (next_c == '=') {

          k = TOKEN_EQ;

          extra = next_con;

        } else

          k = TOKEN_ASSIGN;

        break;

      case '!':

        if (next_c == '=') {

          k = TOKEN_NEQ;

          extra = next_con;

        } else

          k = TOKEN_BANG;

        break;

      case '+':

        if (next_c == '+') {

          k = TOKEN_INC;

          extra = next_con;

        } else if (next_c == '=') {

          k = TOKEN_PLUS_ASSIGN;

          extra = next_con;

        } else

          k = TOKEN_PLUS;

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

        } else

          k = TOKEN_MINUS;

        break;

      case '*':

        if (next_c == '=') {

          k = TOKEN_MUL_ASSIGN;

          extra = next_con;

        } else

          k = TOKEN_STAR;

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

          int c3 = peek_logical(base, len, pos + next_con, &c3_con);

          if (c3 == '%' && peek_logical(base, len, pos + next_con + c3_con,

                                        &c4_con) == ':') {

            k = TOKEN_HASH_HASH;

            extra = next_con + c3_con + c4_con;

          } else {

            k = TOKEN_HASH;

            extra = next_con;
          }

        } else

          k = TOKEN_PERCENT;

        break;

      case '<':

        if (next_c == '=') {

          k = TOKEN_LEQ;

          extra = next_con;

        } else if (next_c == '<') {

          size_t c3_con;

          int c3 = peek_logical(base, len, pos + next_con, &c3_con);

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

        } else

          k = TOKEN_LESS;

        break;

      case '>':

        if (next_c == '=') {

          k = TOKEN_GEQ;

          extra = next_con;

        } else if (next_c == '>') {

          size_t c3_con;

          int c3 = peek_logical(base, len, pos + next_con, &c3_con);

          if (c3 == '=') {

            k = TOKEN_RSHIFT_ASSIGN;

            extra = next_con + c3_con;

          } else {

            k = TOKEN_RSHIFT;

            extra = next_con;
          }

        } else

          k = TOKEN_GREATER;

        break;

      case '&':

        if (next_c == '&') {

          k = TOKEN_LOGICAL_AND;

          extra = next_con;

        } else if (next_c == '=') {

          k = TOKEN_AND_ASSIGN;

          extra = next_con;

        } else

          k = TOKEN_AMP;

        break;

      case '|':

        if (next_c == '|') {

          k = TOKEN_LOGICAL_OR;

          extra = next_con;

        } else if (next_c == '=') {

          k = TOKEN_OR_ASSIGN;

          extra = next_con;

        } else

          k = TOKEN_PIPE;

        break;

      case '^':

        if (next_c == '=') {

          k = TOKEN_XOR_ASSIGN;

          extra = next_con;

        } else

          k = TOKEN_CARET;

        break;

      case '.':

        if (next_c == '.' &&

            peek_logical(base, len, pos + next_con, &consumed) == '.') {

          k = TOKEN_ELLIPSIS;

          extra = next_con + consumed;

        } else

          k = TOKEN_DOT;

        break;
      }

      pos += extra;

      rc = token_list_add(list, k, base + start, pos - start);
    }

  check_rc:

    if (rc != 0) {

      free_token_list(list);

      *out = NULL;

      return rc;
    }
  }

  *out = list;

  return 0;
}
