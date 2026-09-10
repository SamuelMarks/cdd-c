/**
 * @file test_tokenizer.h
 * @brief Unit tests for the tokenizer.
 */

#ifndef TEST_TOKENIZER_H
#define TEST_TOKENIZER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
#include "cdd_c_error.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <greatest.h>

#include "functions/parse/tokenizer.h"
/* clang-format on */

static cdd_c_error_t token_to_cstr(char *buf, size_t buf_len,
                                   const struct Token *tok, char **_out_val) {
  size_t copy_len;
  if (buf_len == 0) {
    *_out_val = NULL;
    return 0;
  }

  copy_len = (tok->length < (buf_len - 1)) ? tok->length : (buf_len - 1);
  memcpy(buf, tok->start, copy_len);
  buf[copy_len] = '\0';

  {
    *_out_val = buf;
    return 0;
  }
}

TEST tokenize_all_tokens(void) {
  const az_span code = az_span_create_from_str((
      char *)(size_t)(size_t) "struct union enum identifier 123 'a' \"string\" "
                              "/* block */ // line \n # \n"
                              "{} ; , / ");
  struct TokenList *tl = NULL;
  int ret;
  size_t i = 0;
  int k_struct = 0, ident = 0, num = 0, chr = 0, str = 0;
  int comment = 0, hash = 0, slash = 0, brace = 0;

  ret = tokenize(code, &tl);
  ASSERT_EQ(0, ret);
  ASSERT(tl != NULL);
  ASSERT_GTE(tl->size, 10);

  for (i = 0; i < tl->size; i++) {
    switch (tl->tokens[i].kind) {
    case TOKEN_KEYWORD_STRUCT:
      k_struct = 1;
      break;
    case TOKEN_IDENTIFIER:
      ident = 1;
      break;
    case TOKEN_NUMBER_LITERAL:
      num = 1;
      break;
    case TOKEN_CHAR_LITERAL:
      chr = 1;
      break;
    case TOKEN_STRING_LITERAL:
      str = 1;
      break;
    case TOKEN_COMMENT:
      comment = 1;
      break;
    case TOKEN_HASH:
      hash = 1;
      break;
    case TOKEN_LBRACE:
      brace = 1;
      break;
    case TOKEN_SLASH:
      slash = 1;
      break;
    default:
      break;
    }
  }
  ASSERT_GT(k_struct, 0);
  ASSERT_GT(ident, 0);
  ASSERT_GT(num, 0);
  ASSERT_GT(chr, 0);
  ASSERT_GT(str, 0);
  ASSERT_GT(comment, 0);
  ASSERT_GT(hash, 0);
  ASSERT_GT(slash, 0);
  ASSERT_GT(brace, 0);

  free_token_list(tl);
  g_fail_io_after = -1;
  PASS();
}

/* ... existing methods ... */

TEST tokenize_c23_digit_separators(void) {
  char *_ast_token_to_cstr_0 = NULL;
  char *_ast_token_to_cstr_1 = NULL;
  char *_ast_token_to_cstr_2 = NULL;
  /* Test 123'456 */
  const az_span code = az_span_create_from_str(
      (char *)(size_t)(size_t) "123'456 0xAB'CD 0b10'10");
  struct TokenList *tl = NULL;
  int rc;
  char buf[32];

  rc = tokenize(code, &tl);
  (void)rc;
  ASSERT_EQ(0, rc);
  ASSERT(tl);
  ASSERT_EQ(5, tl->size); /* num WS num WS num */

  ASSERT_EQ(TOKEN_NUMBER_LITERAL, tl->tokens[0].kind);
  ASSERT_STR_EQ("123'456", (token_to_cstr(buf, sizeof(buf), &tl->tokens[0],
                                          &_ast_token_to_cstr_0),
                            _ast_token_to_cstr_0));

  ASSERT_EQ(TOKEN_NUMBER_LITERAL, tl->tokens[2].kind);
  ASSERT_STR_EQ("0xAB'CD", (token_to_cstr(buf, sizeof(buf), &tl->tokens[2],
                                          &_ast_token_to_cstr_1),
                            _ast_token_to_cstr_1));

  ASSERT_EQ(TOKEN_NUMBER_LITERAL, tl->tokens[4].kind);
  ASSERT_STR_EQ("0b10'10", (token_to_cstr(buf, sizeof(buf), &tl->tokens[4],
                                          &_ast_token_to_cstr_2),
                            _ast_token_to_cstr_2));

  free_token_list(tl);
  g_fail_io_after = -1;
  PASS();
}

TEST test_tokenizer_error_handling(void) {
  int is_match = 0;
  size_t next_idx = 0;
  enum TokenKind kind;
  struct Token dummy_tok;

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            token_matches_string(NULL, "match", NULL));
  ASSERT_EQ(0, token_matches_string(NULL, "match", &is_match));
  ASSERT_EQ(0, is_match);

  /* Create dummy token for testing */
  dummy_tok.start = (const uint8_t *)"match";
  dummy_tok.length = 5;
  dummy_tok.kind = TOKEN_IDENTIFIER;
  ASSERT_EQ(0, token_matches_string(&dummy_tok, "match", &is_match));
  ASSERT_EQ(1, is_match);

  /* identify_keyword_or_id segfaults when we pass start=NULL but out_val is not
     null? Let's check.
  */
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            token_find_next(NULL, 0, 10, TOKEN_IDENTIFIER, NULL));

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            token_find_next(NULL, 0, 10, TOKEN_IDENTIFIER, NULL));
  ASSERT_EQ(0, token_find_next(NULL, 0, 10, TOKEN_IDENTIFIER, &next_idx));
  ASSERT_EQ(SIZE_MAX, next_idx);

  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT,
            identify_keyword_or_id(NULL, 5, NULL));
  ASSERT_EQ(0, identify_keyword_or_id(NULL, 5, &kind));
  ASSERT_EQ(TOKEN_IDENTIFIER, kind);
  g_fail_io_after = -1;

  PASS();
}

TEST tokenize_digit_separator_edge_case(void) {
  char *_ast_token_to_cstr_3 = NULL;
  /* Separator at end should NOT be included in number if not followed by digit
   */
  /* 123' -> 123 and ' (char literal start? or just punctuator?) */
  /* Actually scanning logic for number: consume if next is alnum. */
  /* If input is "123'" (eof). Peek next is -1 (not alnum). */
  /* Then loop breaks. ' remains. */
  /* Next iteration: ' starts a char literal probably. */

  const az_span code = az_span_create_from_str(
      (char *)(size_t)(size_t) "123' 456"); /* Space after ' */
  struct TokenList *tl = NULL;
  char buf[32];
  int rc;

  rc = tokenize(code, &tl);
  (void)rc;
  ASSERT_EQ(0, rc);

  /* 123 (num) */
  ASSERT_EQ(TOKEN_NUMBER_LITERAL, tl->tokens[0].kind);
  ASSERT_STR_EQ("123", (token_to_cstr(buf, sizeof(buf), &tl->tokens[0],
                                      &_ast_token_to_cstr_3),
                        _ast_token_to_cstr_3));

  /* ' (char literal start, likely unterminated or just ' ) */
  /* Logic: c == '\'' -> consume until next ' */
  /* Here: ' 456 (space is not next ' so it consumes space... then 456... ) */
  /* Actually tokenizer logic for char literal: consumes until next quote. */
  /* "123' 456" -> ' consumes space, 4, 5, 6... EOF? error? */
  /* Actually it waits for next '. If no next ', it consumes till EOF and
   * returns CHAR_LITERAL. */
  /* So it becomes: [123] [' 456] */

  ASSERT_EQ(TOKEN_CHAR_LITERAL, tl->tokens[1].kind);

  free_token_list(tl);
  g_fail_io_after = -1;
  PASS();
}

TEST test_tokenize_all_keywords(void) {
  const char *src =
      "auto break case char const continue default do double else enum extern "
      "float for goto if inline __inline int long register restrict "
      "__restrict return short signed sizeof static struct switch typedef "
      "union unsigned void volatile while _Bool bool _Complex _Imaginary "
      "_Atomic _Thread_local thread_local _Alignas alignas _Alignof alignof "
      "_Noreturn constexpr _Static_assert static_assert typeof nullptr true "
      "false embed _Pragma __attribute__ __declspec";
  const az_span code = az_span_create_from_str((char *)(size_t)src);
  struct TokenList *tl = NULL;
  cdd_c_error_t rc;
  size_t i;
  enum TokenKind kinds[59];
  size_t expected_kinds = 0;

  kinds[expected_kinds++] = TOKEN_KEYWORD_AUTO;
  kinds[expected_kinds++] = TOKEN_KEYWORD_BREAK;
  kinds[expected_kinds++] = TOKEN_KEYWORD_CASE;
  kinds[expected_kinds++] = TOKEN_KEYWORD_CHAR;
  kinds[expected_kinds++] = TOKEN_KEYWORD_CONST;
  kinds[expected_kinds++] = TOKEN_KEYWORD_CONTINUE;
  kinds[expected_kinds++] = TOKEN_KEYWORD_DEFAULT;
  kinds[expected_kinds++] = TOKEN_KEYWORD_DO;
  kinds[expected_kinds++] = TOKEN_KEYWORD_DOUBLE;
  kinds[expected_kinds++] = TOKEN_KEYWORD_ELSE;
  kinds[expected_kinds++] = TOKEN_KEYWORD_ENUM;
  kinds[expected_kinds++] = TOKEN_KEYWORD_EXTERN;
  kinds[expected_kinds++] = TOKEN_KEYWORD_FLOAT;
  kinds[expected_kinds++] = TOKEN_KEYWORD_FOR;
  kinds[expected_kinds++] = TOKEN_KEYWORD_GOTO;
  kinds[expected_kinds++] = TOKEN_KEYWORD_IF;
  kinds[expected_kinds++] = TOKEN_KEYWORD_INLINE;
  kinds[expected_kinds++] = TOKEN_KEYWORD_INLINE;
  kinds[expected_kinds++] = TOKEN_KEYWORD_INT;
  kinds[expected_kinds++] = TOKEN_KEYWORD_LONG;
  kinds[expected_kinds++] = TOKEN_KEYWORD_REGISTER;
  kinds[expected_kinds++] = TOKEN_KEYWORD_RESTRICT;
  kinds[expected_kinds++] = TOKEN_KEYWORD_RESTRICT;
  kinds[expected_kinds++] = TOKEN_KEYWORD_RETURN;
  kinds[expected_kinds++] = TOKEN_KEYWORD_SHORT;
  kinds[expected_kinds++] = TOKEN_KEYWORD_SIGNED;
  kinds[expected_kinds++] = TOKEN_KEYWORD_SIZEOF;
  kinds[expected_kinds++] = TOKEN_KEYWORD_STATIC;
  kinds[expected_kinds++] = TOKEN_KEYWORD_STRUCT;
  kinds[expected_kinds++] = TOKEN_KEYWORD_SWITCH;
  kinds[expected_kinds++] = TOKEN_KEYWORD_TYPEDEF;
  kinds[expected_kinds++] = TOKEN_KEYWORD_UNION;
  kinds[expected_kinds++] = TOKEN_KEYWORD_UNSIGNED;
  kinds[expected_kinds++] = TOKEN_KEYWORD_VOID;
  kinds[expected_kinds++] = TOKEN_KEYWORD_VOLATILE;
  kinds[expected_kinds++] = TOKEN_KEYWORD_WHILE;
  kinds[expected_kinds++] = TOKEN_KEYWORD_BOOL;
  kinds[expected_kinds++] = TOKEN_KEYWORD_BOOL;
  kinds[expected_kinds++] = TOKEN_KEYWORD_COMPLEX;
  kinds[expected_kinds++] = TOKEN_KEYWORD_IMAGINARY;
  kinds[expected_kinds++] = TOKEN_KEYWORD_ATOMIC;
  kinds[expected_kinds++] = TOKEN_KEYWORD_THREAD_LOCAL;
  kinds[expected_kinds++] = TOKEN_KEYWORD_THREAD_LOCAL;
  kinds[expected_kinds++] = TOKEN_KEYWORD_ALIGNAS;
  kinds[expected_kinds++] = TOKEN_KEYWORD_ALIGNAS;
  kinds[expected_kinds++] = TOKEN_KEYWORD_ALIGNOF;
  kinds[expected_kinds++] = TOKEN_KEYWORD_ALIGNOF;
  kinds[expected_kinds++] = TOKEN_KEYWORD_NORETURN;
  kinds[expected_kinds++] = TOKEN_KEYWORD_CONSTEXPR;
  kinds[expected_kinds++] = TOKEN_KEYWORD_STATIC_ASSERT;
  kinds[expected_kinds++] = TOKEN_KEYWORD_STATIC_ASSERT;
  kinds[expected_kinds++] = TOKEN_KEYWORD_TYPEOF;
  kinds[expected_kinds++] = TOKEN_KEYWORD_NULLPTR;
  kinds[expected_kinds++] = TOKEN_KEYWORD_TRUE;
  kinds[expected_kinds++] = TOKEN_KEYWORD_FALSE;
  kinds[expected_kinds++] = TOKEN_KEYWORD_EMBED;
  kinds[expected_kinds++] = TOKEN_KEYWORD_PRAGMA_OP;
  kinds[expected_kinds++] = TOKEN_KEYWORD_ATTRIBUTE;
  kinds[expected_kinds++] = TOKEN_KEYWORD_DECLSPEC;

  rc = tokenize(code, &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(tl);

  {
    size_t kw_idx = 0;
    for (i = 0; i < tl->size; ++i) {
      if (tl->tokens[i].kind != TOKEN_WHITESPACE) {
        ASSERT_LT(kw_idx, expected_kinds);
        ASSERT_EQ(kinds[kw_idx], tl->tokens[i].kind);
        kw_idx++;
      }
    }
    ASSERT_EQ(expected_kinds, kw_idx);
  }

  free_token_list(tl);
  g_fail_io_after = -1;
  PASS();
}

TEST test_tokenize_operators_and_digraphs(void) {
  const char *src =
      "{ } [ ] ( ) ; , ~ ? : :> / /= = == ! != + ++ += - -- -> -= * *= % %= "
      "%> %: %:%: < <= << <<= <% <: > >= >> >>= & && &= | || |= ^ ^= . ... "
      "# ##";
  const az_span code = az_span_create_from_str((char *)(size_t)src);
  struct TokenList *tl = NULL;
  cdd_c_error_t rc;
  size_t i;
  enum TokenKind ops[49];
  size_t expected_ops = 0;

  ops[expected_ops++] = TOKEN_LBRACE;
  ops[expected_ops++] = TOKEN_RBRACE;
  ops[expected_ops++] = TOKEN_LBRACKET;
  ops[expected_ops++] = TOKEN_RBRACKET;
  ops[expected_ops++] = TOKEN_LPAREN;
  ops[expected_ops++] = TOKEN_RPAREN;
  ops[expected_ops++] = TOKEN_SEMICOLON;
  ops[expected_ops++] = TOKEN_COMMA;
  ops[expected_ops++] = TOKEN_TILDE;
  ops[expected_ops++] = TOKEN_QUESTION;
  ops[expected_ops++] = TOKEN_COLON;
  ops[expected_ops++] = TOKEN_RBRACKET; /* :> */
  ops[expected_ops++] = TOKEN_SLASH;
  ops[expected_ops++] = TOKEN_DIV_ASSIGN;
  ops[expected_ops++] = TOKEN_ASSIGN;
  ops[expected_ops++] = TOKEN_EQ;
  ops[expected_ops++] = TOKEN_BANG;
  ops[expected_ops++] = TOKEN_NEQ;
  ops[expected_ops++] = TOKEN_PLUS;
  ops[expected_ops++] = TOKEN_INC;
  ops[expected_ops++] = TOKEN_PLUS_ASSIGN;
  ops[expected_ops++] = TOKEN_MINUS;
  ops[expected_ops++] = TOKEN_DEC;
  ops[expected_ops++] = TOKEN_ARROW;
  ops[expected_ops++] = TOKEN_MINUS_ASSIGN;
  ops[expected_ops++] = TOKEN_STAR;
  ops[expected_ops++] = TOKEN_MUL_ASSIGN;
  ops[expected_ops++] = TOKEN_PERCENT;
  ops[expected_ops++] = TOKEN_MOD_ASSIGN;
  ops[expected_ops++] = TOKEN_RBRACE;    /* %> */
  ops[expected_ops++] = TOKEN_HASH;      /* %: */
  ops[expected_ops++] = TOKEN_HASH_HASH; /* %:%: */
  ops[expected_ops++] = TOKEN_LESS;
  ops[expected_ops++] = TOKEN_LEQ;
  ops[expected_ops++] = TOKEN_LSHIFT;
  ops[expected_ops++] = TOKEN_LSHIFT_ASSIGN;
  ops[expected_ops++] = TOKEN_LBRACE;   /* <% */
  ops[expected_ops++] = TOKEN_LBRACKET; /* <: */
  ops[expected_ops++] = TOKEN_GREATER;
  ops[expected_ops++] = TOKEN_GEQ;
  ops[expected_ops++] = TOKEN_RSHIFT;
  ops[expected_ops++] = TOKEN_RSHIFT_ASSIGN;
  ops[expected_ops++] = TOKEN_AMP;
  ops[expected_ops++] = TOKEN_LOGICAL_AND;
  ops[expected_ops++] = TOKEN_AND_ASSIGN;
  ops[expected_ops++] = TOKEN_PIPE;
  ops[expected_ops++] = TOKEN_LOGICAL_OR;
  ops[expected_ops++] = TOKEN_OR_ASSIGN;
  ops[expected_ops++] = TOKEN_CARET;

  rc = tokenize(code, &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(tl);

  {
    size_t op_idx = 0;
    for (i = 0; i < tl->size && op_idx < expected_ops; ++i) {
      if (tl->tokens[i].kind != TOKEN_WHITESPACE) {
        ASSERT_EQ(ops[op_idx], tl->tokens[i].kind);
        op_idx++;
      }
    }
    ASSERT_EQ(expected_ops, op_idx);
  }

  free_token_list(tl);
  g_fail_io_after = -1;
  PASS();
}

TEST test_tokenize_comments_and_ucn(void) {
  struct TokenList *tl = NULL;
  cdd_c_error_t rc;

  /* Line comment, block comment, unterminated block comment */
  rc = tokenize(
      az_span_create_from_str((char *)(size_t) "// line\n/* block * with * "
                                               "stars */\n/* unterminated"),
      &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(tl);
  ASSERT_EQ(TOKEN_COMMENT, tl->tokens[0].kind);
  ASSERT_EQ(TOKEN_WHITESPACE, tl->tokens[1].kind);
  ASSERT_EQ(TOKEN_COMMENT, tl->tokens[2].kind);
  ASSERT_EQ(TOKEN_WHITESPACE, tl->tokens[3].kind);
  ASSERT_EQ(TOKEN_COMMENT, tl->tokens[4].kind);
  free_token_list(tl);

  /* UCN at identifier start: \u0041 (A) */
  rc = tokenize(az_span_create_from_str((char *)(size_t) "\\u0041bc"), &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(tl);
  ASSERT_EQ(TOKEN_IDENTIFIER, tl->tokens[0].kind);
  free_token_list(tl);

  /* UCN \U00000041 at start */
  rc = tokenize(az_span_create_from_str((char *)(size_t) "\\U00000041bc"), &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(tl);
  ASSERT_EQ(TOKEN_IDENTIFIER, tl->tokens[0].kind);
  free_token_list(tl);

  /* \ followed by non-u/U -> TOKEN_OTHER */
  rc = tokenize(az_span_create_from_str((char *)(size_t) "\\z"), &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(tl);
  ASSERT_EQ(TOKEN_OTHER, tl->tokens[0].kind);
  ASSERT_EQ(TOKEN_IDENTIFIER, tl->tokens[1].kind);
  free_token_list(tl);

  /* \u not followed by hex -> TOKEN_OTHER */
  rc = tokenize(az_span_create_from_str((char *)(size_t) "\\uz"), &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(tl);
  ASSERT_EQ(TOKEN_OTHER, tl->tokens[0].kind);
  free_token_list(tl);

  /* UCN inside identifier: abc\u0041def */
  rc = tokenize(az_span_create_from_str((char *)(size_t) "abc\\u0041def"), &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(tl);
  ASSERT_EQ(TOKEN_IDENTIFIER, tl->tokens[0].kind);
  free_token_list(tl);

  /* UCN inside identifier with non-hex after \u: abc\uz */
  rc = tokenize(az_span_create_from_str((char *)(size_t) "abc\\uz"), &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(tl);
  ASSERT_EQ(TOKEN_IDENTIFIER, tl->tokens[0].kind);
  free_token_list(tl);

  /* Identifier with \non-u inside: abc\x */
  rc = tokenize(az_span_create_from_str((char *)(size_t) "abc\\x"), &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(tl);
  ASSERT_EQ(TOKEN_IDENTIFIER, tl->tokens[0].kind);
  free_token_list(tl);

  /* UCN inside identifier with U: abc\U00000041def */
  rc = tokenize(az_span_create_from_str((char *)(size_t) "abc\\U00000041def"),
                &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(tl);
  ASSERT_EQ(TOKEN_IDENTIFIER, tl->tokens[0].kind);
  free_token_list(tl);

  /* UCN inside identifier with U followed by non-hex: abc\Uz */
  rc = tokenize(az_span_create_from_str((char *)(size_t) "abc\\Uz"), &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(tl);
  ASSERT_EQ(TOKEN_IDENTIFIER, tl->tokens[0].kind);
  free_token_list(tl);

  /* Line comment at EOF without newline */
  rc = tokenize(az_span_create_from_str((char *)(size_t) "// at eof"), &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(tl);
  ASSERT_EQ(TOKEN_COMMENT, tl->tokens[0].kind);
  free_token_list(tl);

  /* Block comment with star then EOF */
  rc = tokenize(az_span_create_from_str((char *)(size_t) "/* at *"), &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(tl);
  ASSERT_EQ(TOKEN_COMMENT, tl->tokens[0].kind);
  free_token_list(tl);

  /* Digraph %: followed by % but not : */
  rc = tokenize(az_span_create_from_str((char *)(size_t) "%: %x"), &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(tl);
  ASSERT_EQ(TOKEN_HASH, tl->tokens[0].kind);
  free_token_list(tl);

  /* Digraph %: followed immediately by % then non-colon */
  rc = tokenize(az_span_create_from_str((char *)(size_t) "%:%a"), &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(tl);
  ASSERT_EQ(TOKEN_HASH, tl->tokens[0].kind);
  free_token_list(tl);

  /* Dot dot followed by non-dot */
  rc = tokenize(az_span_create_from_str((char *)(size_t) ".."), &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(tl);
  ASSERT_EQ(TOKEN_DOT, tl->tokens[0].kind);
  ASSERT_EQ(TOKEN_DOT, tl->tokens[1].kind);
  free_token_list(tl);

  /* Identifier ending with splice at EOF */
  rc = tokenize(az_span_create_from_str((char *)(size_t)("id"
                                                         "\\"
                                                         "\n")),
                &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(tl);
  ASSERT_EQ(TOKEN_IDENTIFIER, tl->tokens[0].kind);
  free_token_list(tl);

  /* Line comment ending with splice at EOF */
  rc = tokenize(az_span_create_from_str((char *)(size_t)("// comm"
                                                         "\\"
                                                         "\n")),
                &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(tl);
  ASSERT_EQ(TOKEN_COMMENT, tl->tokens[0].kind);
  free_token_list(tl);

  /* Block comment ending with splice at EOF */
  rc = tokenize(az_span_create_from_str((char *)(size_t)("/* comm"
                                                         "\\"
                                                         "\n")),
                &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(tl);
  ASSERT_EQ(TOKEN_COMMENT, tl->tokens[0].kind);
  free_token_list(tl);

  g_fail_io_after = -1;
  PASS();
}

TEST test_tokenize_spliced_keywords_and_numbers(void) {
  struct TokenList *tl = NULL;
  cdd_c_error_t rc;

  /* Number with underscore */
  rc = tokenize(az_span_create_from_str((char *)(size_t) "123_456"), &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(tl);
  ASSERT_EQ(TOKEN_NUMBER_LITERAL, tl->tokens[0].kind);
  free_token_list(tl);

  /* Spliced return, switch, if */
  rc = tokenize(
      az_span_create_from_str((char *)(size_t) "re\\\nturn swi\\\ntch i\\\nf"),
      &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(tl);
  ASSERT_EQ(TOKEN_KEYWORD_RETURN, tl->tokens[0].kind);
  ASSERT_EQ(TOKEN_WHITESPACE, tl->tokens[1].kind);
  ASSERT_EQ(TOKEN_KEYWORD_SWITCH, tl->tokens[2].kind);
  ASSERT_EQ(TOKEN_WHITESPACE, tl->tokens[3].kind);
  ASSERT_EQ(TOKEN_KEYWORD_IF, tl->tokens[4].kind);
  free_token_list(tl);

  /* Number starting with dot */
  rc = tokenize(az_span_create_from_str((char *)(size_t) ".456 123.789"), &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(tl);
  ASSERT_EQ(TOKEN_NUMBER_LITERAL, tl->tokens[0].kind);
  ASSERT_EQ(TOKEN_WHITESPACE, tl->tokens[1].kind);
  ASSERT_EQ(TOKEN_NUMBER_LITERAL, tl->tokens[2].kind);
  free_token_list(tl);

  /* Dot not followed by digit */
  rc = tokenize(az_span_create_from_str((char *)(size_t) ".foo ..."), &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(tl);
  ASSERT_EQ(TOKEN_DOT, tl->tokens[0].kind);
  ASSERT_EQ(TOKEN_IDENTIFIER, tl->tokens[1].kind);
  ASSERT_EQ(TOKEN_WHITESPACE, tl->tokens[2].kind);
  ASSERT_EQ(TOKEN_ELLIPSIS, tl->tokens[3].kind);
  free_token_list(tl);

  /* String and char literals with escape sequences */
  rc = tokenize(az_span_create_from_str(
                    (char *)(size_t) "\"hello \\\" world\" 'a' '\\\\'"),
                &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(tl);
  ASSERT_EQ(TOKEN_STRING_LITERAL, tl->tokens[0].kind);
  ASSERT_EQ(TOKEN_WHITESPACE, tl->tokens[1].kind);
  ASSERT_EQ(TOKEN_CHAR_LITERAL, tl->tokens[2].kind);
  ASSERT_EQ(TOKEN_WHITESPACE, tl->tokens[3].kind);
  ASSERT_EQ(TOKEN_CHAR_LITERAL, tl->tokens[4].kind);
  free_token_list(tl);

  /* Unterminated string and char at EOF */
  rc =
      tokenize(az_span_create_from_str((char *)(size_t) "\"unterminated"), &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(tl);
  ASSERT_EQ(TOKEN_STRING_LITERAL, tl->tokens[0].kind);
  free_token_list(tl);

  rc = tokenize(az_span_create_from_str((char *)(size_t) "'unterminated"), &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(tl);
  ASSERT_EQ(TOKEN_CHAR_LITERAL, tl->tokens[0].kind);
  free_token_list(tl);

  g_fail_io_after = -1;
  PASS();
}

TEST test_tokenizer_oom_and_edge_cases(void) {
  extern C_CDD_EXPORT int g_cdd_alloc_fail;
  extern C_CDD_EXPORT int g_cdd_fail_token_matches_string;
  struct TokenList *tl = NULL;
  cdd_c_error_t rc;
  int is_match = 0;
  size_t next_idx = 0;
  struct Token dummy_tok;
  const az_span many_tokens = az_span_create_from_str(
      (char *)(size_t) "; ; ; ; ; ; ; ; ; ; ; ; ; ; ; ; ; ; ; ; "
                       "; ; ; ; ; ; ; ; ; ; ; ; ; ; ; ; ; ; ; ; "
                       "; ; ; ; ; ; ; ; ; ; ; ; ; ; ; ; ; ; ; ; "
                       "; ; ; ; ; ; ; ; ; ; ; ; ; ; ; ; ; ; ; ; ");

  /* tokenize with NULL out */
  rc = tokenize(az_span_create_from_str((char *)(size_t) "int x;"), NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  /* tokenize OOM on calloc */
  g_cdd_alloc_fail = 1;
  rc = tokenize(az_span_create_from_str((char *)(size_t) "int x;"), &tl);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  g_cdd_alloc_fail = 0;

  /* tokenize OOM on token list growth (> 64 tokens) */
  g_cdd_alloc_fail = 2;
  rc = tokenize(many_tokens, &tl);
  ASSERT_EQ(CDD_C_ERROR_MEMORY, rc);
  g_cdd_alloc_fail = 0;

  /* token_matches_string mock failure */
  dummy_tok.start = (const uint8_t *)"return";
  dummy_tok.length = 6;
  dummy_tok.kind = TOKEN_KEYWORD_RETURN;
  g_cdd_fail_token_matches_string = 1;
  rc = token_matches_string(&dummy_tok, "return", &is_match);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  g_cdd_fail_token_matches_string = 0;

  /* token_matches_string with mismatched length / prefix */
  rc = token_matches_string(&dummy_tok, "ret", &is_match);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, is_match);

  rc = token_matches_string(&dummy_tok, "returns", &is_match);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, is_match);

  /* token_matches_string with NULL */
  rc = token_matches_string(&dummy_tok, NULL, &is_match);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, is_match);

  /* token_find_next found vs not found */
  rc = tokenize(az_span_create_from_str((char *)(size_t) "int x = 10;"), &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(tl);

  rc = token_find_next(tl, 0, tl->size, TOKEN_NUMBER_LITERAL, &next_idx);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_NEQ(SIZE_MAX, next_idx);

  rc = token_find_next(tl, 0, tl->size, TOKEN_TILDE, &next_idx);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(tl->size, next_idx);

  rc = token_find_next(tl, 100, tl->size, TOKEN_TILDE, &next_idx);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(SIZE_MAX, next_idx);

  /* token_find_next with end_idx < list->size */
  rc = token_find_next(tl, 0, 1, TOKEN_NUMBER_LITERAL, &next_idx);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(1, next_idx);

  free_token_list(tl);
  free_token_list(NULL);

  /* token_list_add with NULL */
  rc = token_list_add(NULL, TOKEN_IDENTIFIER, NULL, 0);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);

  /* span_equals_str edge cases */
  rc = span_equals_str(many_tokens, "foo", NULL);
  ASSERT_EQ(CDD_C_ERROR_INVALID_ARGUMENT, rc);
  {
    int eq_res = 1;
    rc = span_equals_str(many_tokens, NULL, &eq_res);
    ASSERT_EQ(CDD_C_SUCCESS, rc);
    ASSERT_EQ(0, eq_res);
  }

  /* token_matches_string with splice at EOF */
  dummy_tok.start = (const uint8_t *)"\\\n";
  dummy_tok.length = 2;
  dummy_tok.kind = TOKEN_IDENTIFIER;
  rc = token_matches_string(&dummy_tok, "x", &is_match);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT_EQ(0, is_match);

  /* tokenize with whitespace ending in splice */
  rc = tokenize(az_span_create_from_str((char *)(size_t) "   \\\n"), &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(tl);
  free_token_list(tl);

  /* tokenize with number ending in splice */
  rc = tokenize(az_span_create_from_str((char *)(size_t) "123\\\n"), &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(tl);
  free_token_list(tl);

  /* tokenize with splice at start */
  rc = tokenize(az_span_create_from_str((char *)(size_t) "\\\n"), &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(tl);
  free_token_list(tl);

  /* tokenize with string ending in splice at EOF */
  rc = tokenize(az_span_create_from_str((char *)(size_t) "\"hello\\\n"), &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(tl);
  free_token_list(tl);

  g_fail_io_after = -1;
  PASS();
}

SUITE(tokenizer_suite) {
  RUN_TEST(tokenize_all_tokens);
  /* Use explicit forward declarations or macro magic if needed, or update this
   * list manually */
  /* ... existing runs ... */
  RUN_TEST(tokenize_c23_digit_separators);
  RUN_TEST(tokenize_digit_separator_edge_case);
  RUN_TEST(test_tokenizer_error_handling);
  RUN_TEST(test_tokenize_all_keywords);
  RUN_TEST(test_tokenize_operators_and_digraphs);
  RUN_TEST(test_tokenize_comments_and_ucn);
  RUN_TEST(test_tokenize_spliced_keywords_and_numbers);
  RUN_TEST(test_tokenizer_oom_and_edge_cases);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !TEST_TOKENIZER_H */
