#ifndef TEST_TOKENIZER_H
#define TEST_TOKENIZER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <greatest.h>

#include "tokenizer.h"

static char *token_to_cstr(char *buf, size_t buf_len, const struct Token *tok) {
  size_t copy_len;
  if (buf_len == 0)
    return NULL;

  copy_len = (tok->length < (buf_len - 1)) ? tok->length : (buf_len - 1);
  memcpy(buf, tok->start, copy_len);
  buf[copy_len] = '\0';

  return buf;
}

TEST tokenize_all_tokens(void) {
  const az_span code =
      AZ_SPAN_FROM_STR("struct union enum identifier 123 'a' \"string\" "
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
  PASS();
}

/* ... existing methods ... */

TEST tokenize_c23_digit_separators(void) {
  /* Test 123'456 */
  const az_span code = AZ_SPAN_FROM_STR("123'456 0xAB'CD 0b10'10");
  struct TokenList *tl = NULL;
  int rc;
  char buf[32];

  rc = tokenize(code, &tl);
  ASSERT_EQ(0, rc);
  ASSERT(tl);
  ASSERT_EQ(5, tl->size); /* num WS num WS num */

  ASSERT_EQ(TOKEN_NUMBER_LITERAL, tl->tokens[0].kind);
  ASSERT_STR_EQ("123'456", token_to_cstr(buf, sizeof(buf), &tl->tokens[0]));

  ASSERT_EQ(TOKEN_NUMBER_LITERAL, tl->tokens[2].kind);
  ASSERT_STR_EQ("0xAB'CD", token_to_cstr(buf, sizeof(buf), &tl->tokens[2]));

  ASSERT_EQ(TOKEN_NUMBER_LITERAL, tl->tokens[4].kind);
  ASSERT_STR_EQ("0b10'10", token_to_cstr(buf, sizeof(buf), &tl->tokens[4]));

  free_token_list(tl);
  PASS();
}

TEST tokenize_digit_separator_edge_case(void) {
  /* Separator at end should NOT be included in number if not followed by digit
   */
  /* 123' -> 123 and ' (char literal start? or just punctuator?) */
  /* Actually scanning logic for number: consume if next is alnum. */
  /* If input is "123'" (eof). Peek next is -1 (not alnum). */
  /* Then loop breaks. ' remains. */
  /* Next iteration: ' starts a char literal probably. */

  const az_span code = AZ_SPAN_FROM_STR("123' 456"); // Space after '
  struct TokenList *tl = NULL;
  char buf[32];
  int rc;

  rc = tokenize(code, &tl);
  ASSERT_EQ(0, rc);

  /* 123 (num) */
  ASSERT_EQ(TOKEN_NUMBER_LITERAL, tl->tokens[0].kind);
  ASSERT_STR_EQ("123", token_to_cstr(buf, sizeof(buf), &tl->tokens[0]));

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
  PASS();
}

SUITE(tokenizer_suite) {
  RUN_TEST(tokenize_all_tokens);
  /* Use explicit forward declarations or macro magic if needed, or update this
   * list manually */
  /* ... existing runs ... */
  RUN_TEST(tokenize_c23_digit_separators);
  RUN_TEST(tokenize_digit_separator_edge_case);
}

#endif /* !TEST_TOKENIZER_H */
