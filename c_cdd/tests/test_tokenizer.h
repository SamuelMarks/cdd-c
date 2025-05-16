#ifndef TEST_TOKENIZER_H
#define TEST_TOKENIZER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <greatest.h>

#include "tokenizer.h"

/**
 * Helper: copy token text (az_span) into null-terminated C string buffer.
 */
static char *token_to_cstr(char *buf, size_t buf_len, const struct Token *tok) {
  if (buf_len == 0)
    return NULL; // safety

  int32_t copy_len = (tok->length < (int32_t)(buf_len - 1))
                         ? tok->length
                         : (int32_t)(buf_len - 1);
  memcpy(buf, tok->start, copy_len);
  buf[copy_len] = '\0';
  return buf;
}

TEST tokenize_simple_struct(void) {
  const az_span code = AZ_SPAN_FROM_STR("struct MyStruct {};");
  struct TokenList tl = {NULL, 0, 0};
  int ret = tokenize(code, &tl);
  char buf[64];

  ASSERT_EQ(0, ret);

  ASSERT_GTE(tl.size, 2);

  ASSERT_EQ(TOKEN_KEYWORD_STRUCT, tl.tokens[0].kind);
  ASSERT_STR_EQ("struct", token_to_cstr(buf, sizeof(buf), &tl.tokens[0]));

  ASSERT_EQ(TOKEN_WHITESPACE, tl.tokens[1].kind);

  ASSERT_EQ(TOKEN_IDENTIFIER, tl.tokens[2].kind);
  ASSERT_STR_EQ("MyStruct", token_to_cstr(buf, sizeof(buf), &tl.tokens[2]));

  ASSERT_EQ(TOKEN_WHITESPACE, tl.tokens[3].kind);

  ASSERT_EQ(TOKEN_LBRACE, tl.tokens[4].kind);
  ASSERT_STR_EQ("{", token_to_cstr(buf, sizeof(buf), &tl.tokens[4]));

  ASSERT_EQ(TOKEN_RBRACE, tl.tokens[5].kind);
  ASSERT_STR_EQ("}", token_to_cstr(buf, sizeof(buf), &tl.tokens[5]));

  ASSERT_EQ(TOKEN_SEMICOLON, tl.tokens[6].kind);
  ASSERT_STR_EQ(";", token_to_cstr(buf, sizeof(buf), &tl.tokens[6]));

  free_token_list(&tl);
  PASS();
}

TEST tokenize_empty(void) {
  const az_span code = AZ_SPAN_FROM_STR("");
  struct TokenList tl = {NULL, 0, 0};
  int ret = tokenize(code, &tl);
  ASSERT_EQ(0, ret);
  ASSERT_EQ(0, tl.size);
  ASSERT(tl.tokens == NULL);

  free_token_list(&tl);
  PASS();
}

TEST tokenize_keywords_and_idents(void) {
  const az_span code = AZ_SPAN_FROM_STR("enum Color { RED, GREEN, BLUE };");
  struct TokenList tl = {NULL, 0, 0};

  int ret = tokenize(code, &tl);
  ASSERT_EQ(0, ret);

  ASSERT_GTE(tl.size, 14);

  char buf[64];
  ASSERT_EQ(TOKEN_KEYWORD_ENUM, tl.tokens[0].kind);
  ASSERT_STR_EQ("enum", token_to_cstr(buf, sizeof(buf), &tl.tokens[0]));

  ASSERT_EQ(TOKEN_WHITESPACE, tl.tokens[1].kind);

  ASSERT_EQ(TOKEN_IDENTIFIER, tl.tokens[2].kind);
  ASSERT_STR_EQ("Color", token_to_cstr(buf, sizeof(buf), &tl.tokens[2]));

  ASSERT_EQ(TOKEN_WHITESPACE, tl.tokens[3].kind);

  ASSERT_EQ(TOKEN_LBRACE, tl.tokens[4].kind);
  ASSERT_STR_EQ("{", token_to_cstr(buf, sizeof(buf), &tl.tokens[4]));

  ASSERT_EQ(TOKEN_WHITESPACE, tl.tokens[5].kind);

  ASSERT_EQ(TOKEN_IDENTIFIER, tl.tokens[6].kind);
  ASSERT_STR_EQ("RED", token_to_cstr(buf, sizeof(buf), &tl.tokens[6]));

  ASSERT_EQ(TOKEN_COMMA, tl.tokens[7].kind);
  ASSERT_STR_EQ(",", token_to_cstr(buf, sizeof(buf), &tl.tokens[7]));

  ASSERT_EQ(TOKEN_WHITESPACE, tl.tokens[8].kind);

  ASSERT_EQ(TOKEN_IDENTIFIER, tl.tokens[9].kind);
  ASSERT_STR_EQ("GREEN", token_to_cstr(buf, sizeof(buf), &tl.tokens[9]));

  ASSERT_EQ(TOKEN_COMMA, tl.tokens[10].kind);
  ASSERT_STR_EQ(",", token_to_cstr(buf, sizeof(buf), &tl.tokens[10]));

  ASSERT_EQ(TOKEN_WHITESPACE, tl.tokens[11].kind);

  ASSERT_EQ(TOKEN_IDENTIFIER, tl.tokens[12].kind);
  ASSERT_STR_EQ("BLUE", token_to_cstr(buf, sizeof(buf), &tl.tokens[12]));

  ASSERT_EQ(TOKEN_WHITESPACE, tl.tokens[13].kind);

  ASSERT_EQ(TOKEN_SEMICOLON, tl.tokens[15].kind);
  ASSERT_STR_EQ(";", token_to_cstr(buf, sizeof(buf), &tl.tokens[15]));

  free_token_list(&tl);
  PASS();
}

TEST tokenize_with_comments(void) {
  const az_span code = AZ_SPAN_FROM_STR(
      "/* comment */\nstruct S { int x; }; // trailing comment");
  struct TokenList tl = {NULL, 0, 0};
  int ret = tokenize(code, &tl);
  ASSERT_EQ(0, ret);

  ASSERT_GTE(tl.size, 16);

  char buf[64];
  ASSERT_EQ(TOKEN_COMMENT, tl.tokens[0].kind);
  ASSERT_STR_EQ("/* comment */",
                token_to_cstr(buf, sizeof(buf), &tl.tokens[0]));

  ASSERT_EQ(TOKEN_WHITESPACE, tl.tokens[1].kind);

  /*ASSERT_EQ(TOKEN_KEYWORD_STRUCT, tl.tokens[2].kind);
  ASSERT_STR_EQ("struct", token_to_cstr(buf, sizeof(buf), &tl.tokens[2]));*/

  ASSERT_EQ(TOKEN_WHITESPACE, tl.tokens[3].kind);

  ASSERT_EQ(TOKEN_IDENTIFIER, tl.tokens[4].kind);
  ASSERT_STR_EQ("S", token_to_cstr(buf, sizeof(buf), &tl.tokens[4]));

  /* Additional tokens test for coverage */
  ASSERT_EQ(TOKEN_WHITESPACE, tl.tokens[5].kind);

  ASSERT_EQ(TOKEN_LBRACE, tl.tokens[6].kind);
  ASSERT_STR_EQ("{", token_to_cstr(buf, sizeof(buf), &tl.tokens[6]));

  ASSERT_EQ(TOKEN_WHITESPACE, tl.tokens[7].kind);

  /*ASSERT_EQ(TOKEN_KEYWORD_INT, tl.tokens[8].kind);
  ASSERT_STR_EQ("int", token_to_cstr(buf, sizeof(buf), &tl.tokens[8]));*/

  ASSERT_EQ(TOKEN_WHITESPACE, tl.tokens[9].kind);

  ASSERT_EQ(TOKEN_IDENTIFIER, tl.tokens[10].kind);
  ASSERT_STR_EQ("x", token_to_cstr(buf, sizeof(buf), &tl.tokens[10]));

  ASSERT_EQ(TOKEN_SEMICOLON, tl.tokens[11].kind);
  ASSERT_STR_EQ(";", token_to_cstr(buf, sizeof(buf), &tl.tokens[11]));

  ASSERT_EQ(TOKEN_WHITESPACE, tl.tokens[12].kind);

  ASSERT_EQ(TOKEN_RBRACE, tl.tokens[13].kind);
  ASSERT_STR_EQ("}", token_to_cstr(buf, sizeof(buf), &tl.tokens[13]));

  ASSERT_EQ(TOKEN_SEMICOLON, tl.tokens[14].kind);
  ASSERT_STR_EQ(";", token_to_cstr(buf, sizeof(buf), &tl.tokens[14]));

  ASSERT_EQ(TOKEN_WHITESPACE, tl.tokens[15].kind);

  ASSERT_EQ(TOKEN_COMMENT, tl.tokens[16].kind);
  ASSERT_STR_EQ("// trailing comment",
                token_to_cstr(buf, sizeof(buf), &tl.tokens[16]));

  free_token_list(&tl);
  PASS();
}

/* main test suite */
SUITE(tokenizer_suite) {
  RUN_TEST(tokenize_simple_struct);
  RUN_TEST(tokenize_empty);
  RUN_TEST(tokenize_keywords_and_idents);
  RUN_TEST(tokenize_with_comments);
}

#endif /* !TEST_TOKENIZER_H */
