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
 * @param[in] c3 The third character of `??X`.
 * @param[out] out_mapped The replacement char or 0 if not a trigraph.
 * @return CDD_C_SUCCESS on success.
 */
static cdd_c_error_t get_trigraph_map(int c3, int *out_mapped) {
  switch (c3) {
  case '=':
    *out_mapped = '#';
    break;
  case '(':
    *out_mapped = '[';
    break;
  case '/':
    *out_mapped = '\\';
    break;
  case ')':
    *out_mapped = ']';
    break;
  case '\'':
    *out_mapped = '^';
    break;
  case '<':
    *out_mapped = '{';
    break;
  case '!':
    *out_mapped = '|';
    break;
  case '>':
    *out_mapped = '}';
    break;
  case '-':
    *out_mapped = '~';
    break;
  default:
    *out_mapped = 0;
    break;
  }
  return CDD_C_SUCCESS;
}

/**
 * @brief Peek the next logical character from the buffer.
 *
 * Handles Phase 1 (Trigraphs) and Phase 2 (Backslash-Newline Splicing).
 *
 * @param[in] base Source buffer.
 * @param[in] len Buffer length.
 * @param[in] pos Current physical position.
 * @param[out] out_consumed How many physical bytes constitute this logical
 * char.
 * @param[out] out_char The logical character, or -1 if end of buffer.
 * @return CDD_C_SUCCESS on success.
 */
cdd_c_error_t peek_logical(const uint8_t *base, size_t len, size_t pos,
                           size_t *out_consumed, int *out_char) {
  size_t current = pos;

  while (current < len) {
    int c = base[current];
    size_t char_len = 1;

    /* Phase 1: Trigraphs */
    /* Check for ?? */
    if (c == '?' && current + 2 < len && base[current + 1] == '?') {
      int mapped = 0;
      get_trigraph_map(base[current + 2], &mapped);
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
          continue;               /* Loop to get next char after splice */
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
    *out_char = c;
    return CDD_C_SUCCESS;
  }

  *out_consumed = (current > pos) ? (current - pos) : 0;
  *out_char = -1; /* EOF */
  return CDD_C_SUCCESS;
}

/* --- Token List Setup --- */

/**
 * @brief Appends a token to a dynamic token list.
 *
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_MEMORY` on OOM.
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

  memset(&tl->tokens[tl->size], 0, sizeof(struct Token));
  tl->tokens[tl->size].kind = kind;

  tl->tokens[tl->size].start = start;

  tl->tokens[tl->size].length = length;

  tl->size++;

  return CDD_C_SUCCESS;
}

/* --- Main Public API --- */

/**
 * @brief Searches forward in a token list for the next token of a given kind.
 *
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_INVALID_ARGUMENT` if
 * NULL.
 */
cdd_c_error_t token_find_next(const struct TokenList *list, size_t start_idx,
                              size_t end_idx, const enum TokenKind kind,
                              size_t *_out_val) {
  size_t i, limit;
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
 * @brief Checks whether a token at a given index matches a literal string.
 *
 * @return `CDD_C_SUCCESS` on success, or `CDD_C_ERROR_INVALID_ARGUMENT` if
 * NULL.
 */
cdd_c_error_t token_matches_string(const struct Token *tok, const char *match,
                                   int *_out_val) {
  size_t m_len;
  size_t i_tok = 0, i_match = 0;
  const uint8_t *t;
  if (!_out_val)
    return CDD_C_ERROR_INVALID_ARGUMENT;
  *_out_val = 0;
  if (!tok || !match) {
    return CDD_C_SUCCESS;
  }

  m_len = strlen(match);

  t = tok->start;

  while (i_tok < tok->length && i_match < m_len) {

    int c = -1;

    size_t adv;

    peek_logical(t + i_tok, tok->length - i_tok, 0, &adv, &c);

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
 * @brief Lexes a C source string into a list of tokens including trivia.
 *
 * @return `CDD_C_SUCCESS` on success, or an error code on failure.
 */
cdd_c_error_t tokenize(az_span source, struct TokenList **out) {
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
    int peek_dot_c = -1;

    int c = -1;

    size_t start = pos;

    peek_logical(base, len, pos, &consumed, &c);

    if (c == -1)

      break;

    if (c == '.') {
      peek_logical(base, len, pos + consumed, &peek_con, &peek_dot_c);
    }

    if (isspace(c)) {

      pos += consumed;

      while (pos < len) {

        int nc = -1;

        peek_logical(base, len, pos, &consumed, &nc);

        if (nc != -1 && isspace(nc)) {

          pos += consumed;

        } else {

          break;
        }
      }

      rc = token_list_add(list, TOKEN_WHITESPACE, base + start, pos - start);

    } else if (c == '#') {

      size_t next_con;
      int next_hash_c = -1;

      pos += consumed;

      peek_logical(base, len, pos, &next_con, &next_hash_c);

      if (next_hash_c == '#') {

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

        int u = -1;

        peek_logical(base, len, pos + consumed, &u_con, &u);

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

          int h = -1;

          peek_logical(base, len, pos + consumed + u_con, &hex_con, &h);

          if (h == -1 || !isxdigit(h)) {

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

        int nc = -1;

        peek_logical(base, len, pos, &nc_con, &nc);

        if (nc != -1 && (isalnum(nc) || nc == '_' || nc == '\\')) {

          if (nc == '\\') {

            /* UCN logic: must be \ u|U hex... */

            size_t u_con, hex_con;

            int u = -1;

            peek_logical(base, len, pos + nc_con, &u_con, &u);

            if (u == 'u' || u == 'U') {

              /* Peek first hex char to ensure it is valid ID part */

              int h = -1;

              peek_logical(base, len, pos + nc_con + u_con, &hex_con, &h);

              if (h == -1 || !isxdigit(h)) {

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

        enum TokenKind k = TOKEN_IDENTIFIER;

        /* Check raw text for common keywords */

        identify_keyword_or_id(base + start, id_len, &k);

        /* If still ID, check strict logic for spliced keywords if needed */

        if (k == TOKEN_IDENTIFIER) {
          int matched = 0;

          tmp_tok.start = base + start;

          tmp_tok.length = id_len;

          /* Add checks for spliced keywords if necessary */

          token_matches_string(&tmp_tok, "int", &matched);
          if (matched) {
            k = TOKEN_KEYWORD_INT;
          } else {
            token_matches_string(&tmp_tok, "return", &matched);
            if (matched) {
              k = TOKEN_KEYWORD_RETURN;
            } else {
              token_matches_string(&tmp_tok, "switch", &matched);
              if (matched) {
                k = TOKEN_KEYWORD_SWITCH;
              } else {
                token_matches_string(&tmp_tok, "if", &matched);
                if (matched) {
                  k = TOKEN_KEYWORD_IF;
                }
              }
            }
          }
        }

        rc = token_list_add(list, k, base + start, id_len);
      }

    } else if (isdigit(c) ||
               (c == '.' && peek_dot_c != -1 && isdigit(peek_dot_c))) {

      if (c == '.') {
        int dummy_c = -1;
        peek_logical(base, len, pos, &consumed, &dummy_c);
      }

      pos += consumed;

      while (pos < len) {

        int nc = -1;

        peek_logical(base, len, pos, &consumed, &nc);

        /* C23 Digit Separators: Allow '\'' inside number if followed by alnum

         */

        if (nc == '\'') {

          size_t next_peek_con;

          int next_peek = -1;

          peek_logical(base, len, pos + consumed, &next_peek_con, &next_peek);

          if (next_peek != -1 && isalnum(next_peek)) {

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

        int nc = -1;

        peek_logical(base, len, pos, &consumed, &nc);

        if (nc == -1)

          break;

        pos += consumed;

        if (nc == '\\') {
          int esc_c = -1;
          peek_logical(base, len, pos, &consumed, &esc_c);

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
