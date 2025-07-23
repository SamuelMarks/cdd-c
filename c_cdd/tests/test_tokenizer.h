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
    return NULL;

  {
    size_t copy_len = (tok->length < (int32_t)(buf_len - 1))
                          ? tok->length
                          : (int32_t)(buf_len - 1);
    memcpy(buf, tok->start, copy_len);
    buf[copy_len] = '\0';
  }
  return buf;
}

TEST tokenize_all_tokens(void) {
  const az_span code =
      AZ_SPAN_FROM_STR("struct union enum identifier 123 'a' \"string\" "
                       "/* block */ // line \n #macro \n"
                       "{} ; , / "
                       "a->b a++ a-- a<<b a>>b a>=b a<=b a==b a!=b "
                       "a+=1 a-=1 a*=1 a/=1 a%=1 a&=1 a|=1 a^=1 a&&b a||b");
  struct TokenList tl = {NULL, 0, 0};
  int ret;
  size_t i = 0;
  int k_struct = 0, /*k_union = 0, k_enum = 0,*/ ident = 0, num = 0, chr = 0,
      str = 0;
  int comment = 0, macro = 0, other = 0, brace = 0;

  ret = tokenize(code, &tl);
  ASSERT_EQ(0, ret);
  ASSERT_GTE(tl.size, 10);

  for (i = 0; i < tl.size; i++) {
    switch (tl.tokens[i].kind) {
    case TOKEN_KEYWORD_STRUCT:
      k_struct = 1;
      break;
    /*case TOKEN_KEYWORD_UNION:
      k_union = 1;
      break;
    case TOKEN_KEYWORD_ENUM:
      k_enum = 1;
      break;*/
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
    case TOKEN_MACRO:
      macro = 1;
      break;
    case TOKEN_LBRACE:
      brace = 1;
      break;
    case TOKEN_OTHER:
      other = 1;
      break;
    default:
      break;
    }
  }
  ASSERT_GT(k_struct, 0);
  /*ASSERT_GT(k_union, 0);
  ASSERT_GT(k_enum, 0);*/
  ASSERT_GT(ident, 0);
  ASSERT_GT(num, 0);
  ASSERT_GT(chr, 0);
  ASSERT_GT(str, 0);
  ASSERT_GT(comment, 0);
  ASSERT_GT(macro, 0);
  ASSERT_GT(other, 0);
  ASSERT_GT(brace, 0);

  free_token_list(&tl);
  PASS();
}

TEST tokenize_unterminated(void) {
  const az_span code_comment = AZ_SPAN_FROM_STR("/* not closed");
  const az_span code_str = AZ_SPAN_FROM_STR("\" not closed");
  const az_span code_char = AZ_SPAN_FROM_STR("'a");
  const az_span code_slash = AZ_SPAN_FROM_STR("/");

  struct TokenList tl = {NULL, 0, 0};

  tokenize(code_comment, &tl);
  ASSERT_GTE(tl.size, 1);
  ASSERT_EQ(TOKEN_COMMENT, tl.tokens[0].kind);
  free_token_list(&tl);

  tokenize(code_str, &tl);
  ASSERT_GTE(tl.size, 1);
  ASSERT_EQ(TOKEN_STRING_LITERAL, tl.tokens[0].kind);
  free_token_list(&tl);

  tokenize(code_char, &tl);
  ASSERT_GTE(tl.size, 1);
  ASSERT_EQ(TOKEN_CHAR_LITERAL, tl.tokens[0].kind);
  free_token_list(&tl);

  tokenize(code_slash, &tl);
  ASSERT_GTE(tl.size, 1);
  ASSERT_EQ(TOKEN_OTHER, tl.tokens[0].kind);
  free_token_list(&tl);

  PASS();
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
  const int ret = tokenize(code, &tl);
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
  char buf[64];
  ASSERT_EQ(0, ret);

  ASSERT_GTE(tl.size, 14);

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
  char buf[64];
  ASSERT_EQ(0, ret);

  ASSERT_GTE(tl.size, 16);

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

TEST tokenize_specials_and_errors(void) {
  // Covers single-char tokens, char literal, string literal, macro, etc.
  const az_span code = AZ_SPAN_FROM_STR("a 123 'x' \"foo\"\n"
                                        "#macro\n"
                                        "/*block*/\n"
                                        "//line\n"
                                        ",;{}");
  struct TokenList tl = {0};
  const int rc = tokenize(code, &tl);
  int found_comment = 0, found_macro = 0, found_str = 0, found_char = 0;
  size_t i;
  ASSERT_EQ(0, rc);
  for (i = 0; i < tl.size; ++i) {
    switch (tl.tokens[i].kind) {
    case TOKEN_COMMENT:
      found_comment = 1;
      break;
    case TOKEN_MACRO:
      found_macro = 1;
      break;
    case TOKEN_STRING_LITERAL:
      found_str = 1;
      break;
    case TOKEN_CHAR_LITERAL:
      found_char = 1;
      break;
    default:
      break;
    }
  }
  ASSERT(found_comment && found_macro && found_str && found_char);
  free_token_list(&tl);
  PASS();
}

TEST tokenizer_free_token_list_null(void) {
  struct TokenList tl0 = {NULL, 0, 0};
  free_token_list(&tl0);
  PASS();
}

TEST tokenize_various_edge_cases(void) {
  struct TokenList tl = {NULL, 0, 0};

  /* Slash at end of input */
  ASSERT_EQ(0, tokenize(AZ_SPAN_FROM_STR("test/"), &tl));
  ASSERT_EQ(2, tl.size);
  ASSERT_EQ(TOKEN_IDENTIFIER, tl.tokens[0].kind);
  ASSERT_EQ(TOKEN_OTHER, tl.tokens[1].kind);
  free_token_list(&tl);

  /* Unterminated block comment */
  ASSERT_EQ(0, tokenize(AZ_SPAN_FROM_STR("/* foo"), &tl));
  ASSERT_EQ(1, tl.size);
  ASSERT_EQ(TOKEN_COMMENT, tl.tokens[0].kind);
  free_token_list(&tl);

  /* String with escaped quote */
  ASSERT_EQ(0, tokenize(AZ_SPAN_FROM_STR("\"hello\\\"world\""), &tl));
  ASSERT_EQ(1, tl.size);
  ASSERT_EQ(TOKEN_STRING_LITERAL, tl.tokens[0].kind);
  free_token_list(&tl);

  /* Char with escaped char and unterminated */
  ASSERT_EQ(0, tokenize(AZ_SPAN_FROM_STR("'\\''a"), &tl));
  ASSERT_EQ(2, tl.size);
  ASSERT_EQ(TOKEN_CHAR_LITERAL, tl.tokens[0].kind);
  ASSERT_EQ(TOKEN_IDENTIFIER, tl.tokens[1].kind);
  free_token_list(&tl);

  /* Other characters */
  ASSERT_EQ(0, tokenize(AZ_SPAN_FROM_STR("@`$"), &tl));
  ASSERT_EQ(3, tl.size);
  ASSERT_EQ(TOKEN_OTHER, tl.tokens[0].kind);
  ASSERT_EQ(TOKEN_OTHER, tl.tokens[1].kind);
  ASSERT_EQ(TOKEN_OTHER, tl.tokens[2].kind);
  free_token_list(&tl);

  PASS();
}

TEST tokenize_realloc(void) {
  enum { n = 1025 };
  char long_string[n];
  size_t i;
  struct TokenList tl = {NULL, 0, 0};

  for (i = 0; i < n - 2; i += 2) {
    long_string[i] = 'a';
    long_string[i + 1] = ' ';
  }
  long_string[n - 1] = '\0';

  ASSERT_EQ(0, tokenize(az_span_create_from_str(long_string), &tl));
  ASSERT_GT(tl.size, 64); /* Default capacity */
  free_token_list(&tl);
  PASS();
}

/* main test suite */
SUITE(tokenizer_suite) {
  RUN_TEST(tokenize_all_tokens);
  RUN_TEST(tokenize_empty);
  RUN_TEST(tokenize_keywords_and_idents);
  RUN_TEST(tokenize_simple_struct);
  RUN_TEST(tokenize_specials_and_errors);
  RUN_TEST(tokenize_unterminated);
  RUN_TEST(tokenize_with_comments);
  RUN_TEST(tokenizer_free_token_list_null);
  RUN_TEST(tokenize_various_edge_cases);
  RUN_TEST(tokenize_realloc);
}

#endif /* !TEST_TOKENIZER_H */
