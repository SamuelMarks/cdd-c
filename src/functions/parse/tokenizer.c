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

static int token_list_add(struct TokenList *tl, const enum TokenKind kind,

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

static int span_equals_str(const az_span span, const char *str,
                           bool *_out_val) {

  {
    *_out_val =
        az_span_is_content_equal(span, az_span_create_from_str((char *)str));
    return 0;
  }
}

/* Helper to classify keywords */

int identify_keyword_or_id(const uint8_t *start, size_t len,
                           enum TokenKind *_out_val) {
  bool _ast_span_equals_str_0;
  bool _ast_span_equals_str_1;
  bool _ast_span_equals_str_2;
  bool _ast_span_equals_str_3;
  bool _ast_span_equals_str_4;
  bool _ast_span_equals_str_5;
  bool _ast_span_equals_str_6;
  bool _ast_span_equals_str_7;
  bool _ast_span_equals_str_8;
  bool _ast_span_equals_str_9;
  bool _ast_span_equals_str_10;
  bool _ast_span_equals_str_11;
  bool _ast_span_equals_str_12;
  bool _ast_span_equals_str_13;
  bool _ast_span_equals_str_14;
  bool _ast_span_equals_str_15;
  bool _ast_span_equals_str_16;
  bool _ast_span_equals_str_17;
  bool _ast_span_equals_str_18;
  bool _ast_span_equals_str_19;
  bool _ast_span_equals_str_20;
  bool _ast_span_equals_str_21;
  bool _ast_span_equals_str_22;
  bool _ast_span_equals_str_23;
  bool _ast_span_equals_str_24;
  bool _ast_span_equals_str_25;
  bool _ast_span_equals_str_26;
  bool _ast_span_equals_str_27;
  bool _ast_span_equals_str_28;
  bool _ast_span_equals_str_29;
  bool _ast_span_equals_str_30;
  bool _ast_span_equals_str_31;
  bool _ast_span_equals_str_32;
  bool _ast_span_equals_str_33;
  bool _ast_span_equals_str_34;
  bool _ast_span_equals_str_35;
  bool _ast_span_equals_str_36;
  bool _ast_span_equals_str_37;
  bool _ast_span_equals_str_38;
  bool _ast_span_equals_str_39;
  bool _ast_span_equals_str_40;
  bool _ast_span_equals_str_41;
  bool _ast_span_equals_str_42;
  bool _ast_span_equals_str_43;
  bool _ast_span_equals_str_44;
  bool _ast_span_equals_str_45;
  bool _ast_span_equals_str_46;
  bool _ast_span_equals_str_47;
  bool _ast_span_equals_str_48;
  bool _ast_span_equals_str_49;
  bool _ast_span_equals_str_50;
  bool _ast_span_equals_str_51;
  bool _ast_span_equals_str_52;
  bool _ast_span_equals_str_53;
  bool _ast_span_equals_str_54;
  bool _ast_span_equals_str_55;
  bool _ast_span_equals_str_56;

  az_span s = az_span_create((uint8_t *)start, (int32_t)len);

  /* C89/C90/C99/C11/C23 Keywords */

  if ((span_equals_str(s, "auto", &_ast_span_equals_str_0),
       _ast_span_equals_str_0))

  {
    *_out_val = TOKEN_KEYWORD_AUTO;
    return 0;
  }

  if ((span_equals_str(s, "break", &_ast_span_equals_str_1),
       _ast_span_equals_str_1))

  {
    *_out_val = TOKEN_KEYWORD_BREAK;
    return 0;
  }

  if ((span_equals_str(s, "case", &_ast_span_equals_str_2),
       _ast_span_equals_str_2))

  {
    *_out_val = TOKEN_KEYWORD_CASE;
    return 0;
  }

  if ((span_equals_str(s, "char", &_ast_span_equals_str_3),
       _ast_span_equals_str_3))

  {
    *_out_val = TOKEN_KEYWORD_CHAR;
    return 0;
  }

  if ((span_equals_str(s, "const", &_ast_span_equals_str_4),
       _ast_span_equals_str_4))

  {
    *_out_val = TOKEN_KEYWORD_CONST;
    return 0;
  }

  if ((span_equals_str(s, "continue", &_ast_span_equals_str_5),
       _ast_span_equals_str_5))

  {
    *_out_val = TOKEN_KEYWORD_CONTINUE;
    return 0;
  }

  if ((span_equals_str(s, "default", &_ast_span_equals_str_6),
       _ast_span_equals_str_6))

  {
    *_out_val = TOKEN_KEYWORD_DEFAULT;
    return 0;
  }

  if ((span_equals_str(s, "do", &_ast_span_equals_str_7),
       _ast_span_equals_str_7))

  {
    *_out_val = TOKEN_KEYWORD_DO;
    return 0;
  }

  if ((span_equals_str(s, "double", &_ast_span_equals_str_8),
       _ast_span_equals_str_8))

  {
    *_out_val = TOKEN_KEYWORD_DOUBLE;
    return 0;
  }

  if ((span_equals_str(s, "else", &_ast_span_equals_str_9),
       _ast_span_equals_str_9))

  {
    *_out_val = TOKEN_KEYWORD_ELSE;
    return 0;
  }

  if ((span_equals_str(s, "enum", &_ast_span_equals_str_10),
       _ast_span_equals_str_10))

  {
    *_out_val = TOKEN_KEYWORD_ENUM;
    return 0;
  }

  if ((span_equals_str(s, "extern", &_ast_span_equals_str_11),
       _ast_span_equals_str_11))

  {
    *_out_val = TOKEN_KEYWORD_EXTERN;
    return 0;
  }

  if ((span_equals_str(s, "float", &_ast_span_equals_str_12),
       _ast_span_equals_str_12))

  {
    *_out_val = TOKEN_KEYWORD_FLOAT;
    return 0;
  }

  if ((span_equals_str(s, "for", &_ast_span_equals_str_13),
       _ast_span_equals_str_13))

  {
    *_out_val = TOKEN_KEYWORD_FOR;
    return 0;
  }

  if ((span_equals_str(s, "goto", &_ast_span_equals_str_14),
       _ast_span_equals_str_14))

  {
    *_out_val = TOKEN_KEYWORD_GOTO;
    return 0;
  }

  if ((span_equals_str(s, "if", &_ast_span_equals_str_15),
       _ast_span_equals_str_15))

  {
    *_out_val = TOKEN_KEYWORD_IF;
    return 0;
  }

  if ((span_equals_str(s, "inline", &_ast_span_equals_str_16),
       _ast_span_equals_str_16))

  {
    *_out_val = TOKEN_KEYWORD_INLINE;
    return 0;
  }

  if ((span_equals_str(s, "int", &_ast_span_equals_str_17),
       _ast_span_equals_str_17))

  {
    *_out_val = TOKEN_KEYWORD_INT;
    return 0;
  }

  if ((span_equals_str(s, "long", &_ast_span_equals_str_18),
       _ast_span_equals_str_18))

  {
    *_out_val = TOKEN_KEYWORD_LONG;
    return 0;
  }

  if ((span_equals_str(s, "register", &_ast_span_equals_str_19),
       _ast_span_equals_str_19))

  {
    *_out_val = TOKEN_KEYWORD_REGISTER;
    return 0;
  }

  if ((span_equals_str(s, "restrict", &_ast_span_equals_str_20),
       _ast_span_equals_str_20))

  {
    *_out_val = TOKEN_KEYWORD_RESTRICT;
    return 0;
  }

  if ((span_equals_str(s, "return", &_ast_span_equals_str_21),
       _ast_span_equals_str_21))

  {
    *_out_val = TOKEN_KEYWORD_RETURN;
    return 0;
  }

  if ((span_equals_str(s, "short", &_ast_span_equals_str_22),
       _ast_span_equals_str_22))

  {
    *_out_val = TOKEN_KEYWORD_SHORT;
    return 0;
  }

  if ((span_equals_str(s, "signed", &_ast_span_equals_str_23),
       _ast_span_equals_str_23))

  {
    *_out_val = TOKEN_KEYWORD_SIGNED;
    return 0;
  }

  if ((span_equals_str(s, "sizeof", &_ast_span_equals_str_24),
       _ast_span_equals_str_24))

  {
    *_out_val = TOKEN_KEYWORD_SIZEOF;
    return 0;
  }

  if ((span_equals_str(s, "static", &_ast_span_equals_str_25),
       _ast_span_equals_str_25))

  {
    *_out_val = TOKEN_KEYWORD_STATIC;
    return 0;
  }

  if ((span_equals_str(s, "struct", &_ast_span_equals_str_26),
       _ast_span_equals_str_26))

  {
    *_out_val = TOKEN_KEYWORD_STRUCT;
    return 0;
  }

  if ((span_equals_str(s, "switch", &_ast_span_equals_str_27),
       _ast_span_equals_str_27))

  {
    *_out_val = TOKEN_KEYWORD_SWITCH;
    return 0;
  }

  if ((span_equals_str(s, "typedef", &_ast_span_equals_str_28),
       _ast_span_equals_str_28))

  {
    *_out_val = TOKEN_KEYWORD_TYPEDEF;
    return 0;
  }

  if ((span_equals_str(s, "union", &_ast_span_equals_str_29),
       _ast_span_equals_str_29))

  {
    *_out_val = TOKEN_KEYWORD_UNION;
    return 0;
  }

  if ((span_equals_str(s, "unsigned", &_ast_span_equals_str_30),
       _ast_span_equals_str_30))

  {
    *_out_val = TOKEN_KEYWORD_UNSIGNED;
    return 0;
  }

  if ((span_equals_str(s, "void", &_ast_span_equals_str_31),
       _ast_span_equals_str_31))

  {
    *_out_val = TOKEN_KEYWORD_VOID;
    return 0;
  }

  if ((span_equals_str(s, "volatile", &_ast_span_equals_str_32),
       _ast_span_equals_str_32))

  {
    *_out_val = TOKEN_KEYWORD_VOLATILE;
    return 0;
  }

  if ((span_equals_str(s, "while", &_ast_span_equals_str_33),
       _ast_span_equals_str_33))

  {
    *_out_val = TOKEN_KEYWORD_WHILE;
    return 0;
  }

  if ((span_equals_str(s, "_Alignas", &_ast_span_equals_str_34),
       _ast_span_equals_str_34))

  {
    *_out_val = TOKEN_KEYWORD_ALIGNAS;
    return 0;
  }

  if ((span_equals_str(s, "_Alignof", &_ast_span_equals_str_35),
       _ast_span_equals_str_35))

  {
    *_out_val = TOKEN_KEYWORD_ALIGNOF;
    return 0;
  }

  if ((span_equals_str(s, "_Atomic", &_ast_span_equals_str_36),
       _ast_span_equals_str_36))

  {
    *_out_val = TOKEN_KEYWORD_ATOMIC;
    return 0;
  }

  if ((span_equals_str(s, "_Bool", &_ast_span_equals_str_37),
       _ast_span_equals_str_37))

  {
    *_out_val = TOKEN_KEYWORD_BOOL;
    return 0;
  }

  if ((span_equals_str(s, "_Complex", &_ast_span_equals_str_38),
       _ast_span_equals_str_38))

  {
    *_out_val = TOKEN_KEYWORD_COMPLEX;
    return 0;
  }

  if ((span_equals_str(s, "_Imaginary", &_ast_span_equals_str_39),
       _ast_span_equals_str_39))

  {
    *_out_val = TOKEN_KEYWORD_IMAGINARY;
    return 0;
  }

  if ((span_equals_str(s, "_Noreturn", &_ast_span_equals_str_40),
       _ast_span_equals_str_40))

  {
    *_out_val = TOKEN_KEYWORD_NORETURN;
    return 0;
  }

  if ((span_equals_str(s, "_Static_assert", &_ast_span_equals_str_41),
       _ast_span_equals_str_41))

  {
    *_out_val = TOKEN_KEYWORD_STATIC_ASSERT;
    return 0;
  }

  if ((span_equals_str(s, "_Thread_local", &_ast_span_equals_str_42),
       _ast_span_equals_str_42))

  {
    *_out_val = TOKEN_KEYWORD_THREAD_LOCAL;
    return 0;
  }

  /* Extensions found in common headers */

  if ((span_equals_str(s, "__inline", &_ast_span_equals_str_43),
       _ast_span_equals_str_43))

  {
    *_out_val = TOKEN_KEYWORD_INLINE;
    return 0;
  }

  if ((span_equals_str(s, "__restrict", &_ast_span_equals_str_44),
       _ast_span_equals_str_44))

  {
    *_out_val = TOKEN_KEYWORD_RESTRICT;
    return 0;
  }

  /* C23 standard keywords */

  if ((span_equals_str(s, "alignas", &_ast_span_equals_str_45),
       _ast_span_equals_str_45))

  {
    *_out_val = TOKEN_KEYWORD_ALIGNAS;
    return 0;
  }

  if ((span_equals_str(s, "alignof", &_ast_span_equals_str_46),
       _ast_span_equals_str_46))

  {
    *_out_val = TOKEN_KEYWORD_ALIGNOF;
    return 0;
  }

  if ((span_equals_str(s, "bool", &_ast_span_equals_str_47),
       _ast_span_equals_str_47))

  {
    *_out_val = TOKEN_KEYWORD_BOOL;
    return 0;
  }

  if ((span_equals_str(s, "constexpr", &_ast_span_equals_str_48),
       _ast_span_equals_str_48))

  {
    *_out_val = TOKEN_KEYWORD_CONSTEXPR;
    return 0;
  }

  if ((span_equals_str(s, "false", &_ast_span_equals_str_49),
       _ast_span_equals_str_49))

  {
    *_out_val = TOKEN_KEYWORD_FALSE;
    return 0;
  }

  if ((span_equals_str(s, "nullptr", &_ast_span_equals_str_50),
       _ast_span_equals_str_50))

  {
    *_out_val = TOKEN_KEYWORD_NULLPTR;
    return 0;
  }

  if ((span_equals_str(s, "static_assert", &_ast_span_equals_str_51),
       _ast_span_equals_str_51))

  {
    *_out_val = TOKEN_KEYWORD_STATIC_ASSERT;
    return 0;
  }

  if ((span_equals_str(s, "thread_local", &_ast_span_equals_str_52),
       _ast_span_equals_str_52))

  {
    *_out_val = TOKEN_KEYWORD_THREAD_LOCAL;
    return 0;
  }

  if ((span_equals_str(s, "true", &_ast_span_equals_str_53),
       _ast_span_equals_str_53))

  {
    *_out_val = TOKEN_KEYWORD_TRUE;
    return 0;
  }

  if ((span_equals_str(s, "typeof", &_ast_span_equals_str_54),
       _ast_span_equals_str_54))

  {
    *_out_val = TOKEN_KEYWORD_TYPEOF;
    return 0;
  }

  if ((span_equals_str(s, "embed", &_ast_span_equals_str_55),
       _ast_span_equals_str_55))

  {
    *_out_val = TOKEN_KEYWORD_EMBED;
    return 0;
  }

  if ((span_equals_str(s, "_Pragma", &_ast_span_equals_str_56),
       _ast_span_equals_str_56))

  {
    *_out_val = TOKEN_KEYWORD_PRAGMA_OP;
    return 0;
  }

  {
    *_out_val = TOKEN_IDENTIFIER;
    return 0;
  }
}

/* --- Main Public API --- */

int token_find_next(const struct TokenList *list, size_t start_idx,

                    size_t end_idx, const enum TokenKind kind,
                    size_t *_out_val) {

  size_t i;

  size_t limit = (end_idx < list->size) ? end_idx : list->size;

  for (i = start_idx; i < limit; ++i) {

    if (list->tokens[i].kind == kind)

    {
      *_out_val = i;
      return 0;
    }
  }

  {
    *_out_val = limit;
    return 0;
  }
}

void free_token_list(struct TokenList *tl) {

  if (!tl)

    return;

  if (tl->tokens) {

    free(tl->tokens);

    tl->tokens = NULL;
  }

  free(tl);
}

int token_matches_string(const struct Token *tok,

                         const char *match, bool *_out_val) {

  size_t m_len;

  size_t i_tok = 0, i_match = 0;

  const uint8_t *t;

  if (!tok || !match)

  {
    *_out_val = false;
    return 0;
  }

  m_len = strlen(match);

  t = tok->start;

  while (i_tok < tok->length && i_match < m_len) {

    int c;

    size_t adv;

    c = peek_logical(t + i_tok, tok->length - i_tok, 0, &adv);

    if (c == -1)

      break;

    if (c != match[i_match])

    {
      *_out_val = false;
      return 0;
    }

    i_tok += adv;

    i_match++;
  }

  {
    *_out_val = (i_tok >= tok->length && i_match == m_len);
    return 0;
  }
}

int tokenize(const az_span source, struct TokenList **const out) {
  enum TokenKind _ast_identify_keyword_or_id_57;
  bool _ast_token_matches_string_58;
  bool _ast_token_matches_string_59;
  bool _ast_token_matches_string_60;
  bool _ast_token_matches_string_61;

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

        k = (identify_keyword_or_id(base + start, id_len,
                                    &_ast_identify_keyword_or_id_57),
             _ast_identify_keyword_or_id_57);

        /* If still ID, check strict logic for spliced keywords if needed */

        if (k == TOKEN_IDENTIFIER) {

          tmp_tok.start = base + start;

          tmp_tok.length = id_len;

          /* Add checks for spliced keywords if necessary */

          if ((token_matches_string(&tmp_tok, "int",
                                    &_ast_token_matches_string_58),
               _ast_token_matches_string_58))

            k = TOKEN_KEYWORD_INT;

          else if ((token_matches_string(&tmp_tok, "return",
                                         &_ast_token_matches_string_59),
                    _ast_token_matches_string_59))

            k = TOKEN_KEYWORD_RETURN;

          else if ((token_matches_string(&tmp_tok, "switch",
                                         &_ast_token_matches_string_60),
                    _ast_token_matches_string_60))

            k = TOKEN_KEYWORD_SWITCH;

          else if ((token_matches_string(&tmp_tok, "if",
                                         &_ast_token_matches_string_61),
                    _ast_token_matches_string_61))

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
