/**
 * @file tokenizer.c
 * @brief Implementation of the C tokenizer.
 */

/* clang-format off */
#include "c_cdd/safe_crt_msvc.h"

#if defined(__clang__)
#endif

#include <ctype.h>

#include <errno.h>

#include <stdlib.h>

#include <string.h>

#include "c_cdd/log.h"
#include "c_cdd/memory.h"
#include "functions/parse/tokenizer.h"
#include "functions/parse/tokenizer_keywords.h"
#include "functions/parse/tokenizer_punct.h"
#include <stdio.h>
/* clang-format on */

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

    return CDD_C_SUCCESS;
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

int peek_logical(const uint8_t *base, size_t len, size_t pos,
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

  *out_consumed = (current > pos) ? (current - pos) : 0;

  return -1; /* EOF */
}

/* --- Token List Setup --- */

/**
 * @brief Executes the token list add operation.
 */
cdd_c_error_t token_list_add(struct TokenList *tl, const enum TokenKind kind,
                             const uint8_t *start, const size_t length) {

  if (!tl)

    return CDD_C_ERROR_INVALID_ARGUMENT;

  if (tl->size >= tl->capacity) {

    const size_t new_cap = (tl->capacity == 0) ? 64 : tl->capacity * 2;

    struct Token *new_arr =

        (struct Token *)C_CDD_REALLOC(tl->tokens,
                                      new_cap * sizeof(struct Token));

    if (!new_arr) {
      C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
      return CDD_C_ERROR_MEMORY;
    }

    tl->tokens = new_arr;

    tl->capacity = new_cap;
  }

  tl->tokens[tl->size].kind = kind;

  tl->tokens[tl->size].start = start;

  tl->tokens[tl->size].length = length;

  tl->size++;

  return CDD_C_SUCCESS;
}

/* --- Main Public API --- */

/**
 * @brief Executes the token find next operation.
 */
cdd_c_error_t token_find_next(const struct TokenList *list, size_t start_idx,
                              size_t end_idx, const enum TokenKind kind,
                              size_t *_out_val) {
  size_t i, limit;
#ifdef CDD_BUILD_TESTS
  extern C_CDD_EXPORT int g_cdd_fail_token_find_next;
  if (g_cdd_fail_token_find_next && --g_cdd_fail_token_find_next == 0)
    return CDD_C_ERROR_INVALID_ARGUMENT;
#endif
  if (!_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *_out_val = SIZE_MAX;
  if (!list || start_idx >= list->size) {
    return CDD_C_SUCCESS;
  }
  limit = (end_idx < list->size) ? end_idx : list->size;

  for (i = start_idx; i < limit; ++i) {

    if (list->tokens[i].kind == kind)

    {
      *_out_val = i;
      return CDD_C_SUCCESS;
    }
  }

  {
    *_out_val = limit;
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Frees the memory associated with token list.
 */
void free_token_list(struct TokenList *tl) {

  if (!tl)

    return;

  if (tl->tokens) {

    C_CDD_FREE(tl->tokens);

    tl->tokens = NULL;
  }

  C_CDD_FREE(tl);
}

/**
 * @brief Executes the token matches string operation.
 */
cdd_c_error_t token_matches_string(const struct Token *tok, const char *match,
                                   int *_out_val) {
  size_t m_len;
  size_t i_tok = 0, i_match = 0;
  const uint8_t *t;
#ifdef CDD_BUILD_TESTS
  extern C_CDD_EXPORT int g_cdd_fail_token_matches_string;
  if (g_cdd_fail_token_matches_string &&
      --g_cdd_fail_token_matches_string == 0) {
    return CDD_C_ERROR_INVALID_ARGUMENT;
  }
#endif
  if (!_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *_out_val = 0;
  if (!tok || !match) {
    return CDD_C_SUCCESS;
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
      *_out_val = 0;
      return CDD_C_SUCCESS;
    }

    i_tok += adv;

    i_match++;
  }

  {
    *_out_val = (i_tok >= tok->length && i_match == m_len);
    return CDD_C_SUCCESS;
  }
}

/**
 * @brief Executes the tokenize operation.
 */
cdd_c_error_t tokenize(az_span source, struct TokenList **out) {
  enum TokenKind _ast_identify_keyword_or_id_57;
  int _ast_token_matches_string_58 = 0;
  int _ast_token_matches_string_59 = 0;
  int _ast_token_matches_string_60 = 0;
  int _ast_token_matches_string_61 = 0;

  struct TokenList *list = NULL;

  const uint8_t *base;

  size_t len, pos = 0;

  cdd_c_error_t rc = CDD_C_SUCCESS;

  if (!out)

    return CDD_C_ERROR_INVALID_ARGUMENT;

  base = az_span_ptr(source);

  len = (size_t)az_span_size(source);

  list = (struct TokenList *)C_CDD_CALLOC(1, sizeof(struct TokenList));

  if (!list) {
    C_CDD_LOG_DEBUG("ENOMEM: OOM\n");
    return CDD_C_ERROR_MEMORY;
  }

  while (pos < len) {

    size_t consumed;
    size_t peek_con;

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

                isdigit(peek_logical(base, len, pos + consumed, &peek_con)))) {

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
      rc = tokenize_punct(base, len, &pos, start, c, consumed, list);
    }

  check_rc:

    if (rc != CDD_C_SUCCESS) {

      free_token_list(list);

      *out = NULL;

      return rc;
    }
  }

  *out = list;

  return CDD_C_SUCCESS;
}
#if defined(__clang__)
#endif
