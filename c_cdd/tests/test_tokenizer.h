#ifndef TEST_TOKENIZER_H
#define TEST_TOKENIZER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <greatest.h>

#include "tokenizer.h"

/**
 * @brief Helper: copy token text (az_span) into null-terminated C string
 * buffer.
 *
 * @param[out] buf Buffer to copy to.
 * @param[in] buf_len Size of buffer.
 * @param[in] tok The token to stringify.
 * @return Pointer to buf or NULL on error.
 */
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
                       "/* block */ // line \n #macro \n"
                       "{} ; , / ");
  struct TokenList *tl = NULL;
  int ret;
  size_t i = 0;
  int k_struct = 0, ident = 0, num = 0, chr = 0, str = 0;
  int comment = 0, macro = 0, other = 0, brace = 0;

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
  ASSERT_GT(ident, 0);
  ASSERT_GT(num, 0);
  ASSERT_GT(chr, 0);
  ASSERT_GT(str, 0);
  ASSERT_GT(comment, 0);
  ASSERT_GT(macro, 0);
  ASSERT_GT(other, 0);
  ASSERT_GT(brace, 0);

  free_token_list(tl);
  PASS();
}

TEST tokenize_unterminated(void) {
  const az_span code_comment = AZ_SPAN_FROM_STR("/* not closed");
  const az_span code_str = AZ_SPAN_FROM_STR("\" not closed");
  const az_span code_char = AZ_SPAN_FROM_STR("'a");
  const az_span code_slash = AZ_SPAN_FROM_STR("/");

  struct TokenList *tl = NULL;

  ASSERT_EQ(0, tokenize(code_comment, &tl));
  ASSERT(tl != NULL);
  ASSERT_GTE(tl->size, 1);
  ASSERT_EQ(TOKEN_COMMENT, tl->tokens[0].kind);
  free_token_list(tl);
  tl = NULL;

  ASSERT_EQ(0, tokenize(code_str, &tl));
  ASSERT(tl != NULL);
  ASSERT_GTE(tl->size, 1);
  ASSERT_EQ(TOKEN_STRING_LITERAL, tl->tokens[0].kind);
  free_token_list(tl);
  tl = NULL;

  ASSERT_EQ(0, tokenize(code_char, &tl));
  ASSERT(tl != NULL);
  ASSERT_GTE(tl->size, 1);
  ASSERT_EQ(TOKEN_CHAR_LITERAL, tl->tokens[0].kind);
  free_token_list(tl);
  tl = NULL;

  ASSERT_EQ(0, tokenize(code_slash, &tl));
  ASSERT(tl != NULL);
  ASSERT_GTE(tl->size, 1);
  ASSERT_EQ(TOKEN_OTHER, tl->tokens[0].kind);
  free_token_list(tl);

  PASS();
}

TEST tokenize_simple_struct(void) {
  const az_span code = AZ_SPAN_FROM_STR("struct MyStruct {};");
  struct TokenList *tl = NULL;
  int ret = tokenize(code, &tl);
  char buf[64];

  ASSERT_EQ(0, ret);
  ASSERT(tl != NULL);

  ASSERT_GTE(tl->size, 2);

  ASSERT_EQ(TOKEN_KEYWORD_STRUCT, tl->tokens[0].kind);
  ASSERT_STR_EQ("struct", token_to_cstr(buf, sizeof(buf), &tl->tokens[0]));

  ASSERT_EQ(TOKEN_WHITESPACE, tl->tokens[1].kind);

  ASSERT_EQ(TOKEN_IDENTIFIER, tl->tokens[2].kind);
  ASSERT_STR_EQ("MyStruct", token_to_cstr(buf, sizeof(buf), &tl->tokens[2]));

  ASSERT_EQ(TOKEN_WHITESPACE, tl->tokens[3].kind);

  ASSERT_EQ(TOKEN_LBRACE, tl->tokens[4].kind);
  ASSERT_STR_EQ("{", token_to_cstr(buf, sizeof(buf), &tl->tokens[4]));

  ASSERT_EQ(TOKEN_RBRACE, tl->tokens[5].kind);
  ASSERT_STR_EQ("}", token_to_cstr(buf, sizeof(buf), &tl->tokens[5]));

  ASSERT_EQ(TOKEN_SEMICOLON, tl->tokens[6].kind);
  ASSERT_STR_EQ(";", token_to_cstr(buf, sizeof(buf), &tl->tokens[6]));

  free_token_list(tl);
  PASS();
}

TEST tokenize_empty(void) {
  const az_span code = AZ_SPAN_FROM_STR("");
  struct TokenList *tl = NULL;
  const int ret = tokenize(code, &tl);
  ASSERT_EQ(0, ret);
  ASSERT(tl != NULL);
  ASSERT_EQ(0, tl->size);
  ASSERT(tl->tokens == NULL);

  free_token_list(tl);
  PASS();
}

TEST tokenize_keywords_and_idents(void) {
  const az_span code = AZ_SPAN_FROM_STR("enum Color { RED, GREEN, BLUE };");
  struct TokenList *tl = NULL;

  int ret = tokenize(code, &tl);
  char buf[64];
  ASSERT_EQ(0, ret);
  ASSERT(tl != NULL);

  ASSERT_GTE(tl->size, 14);

  ASSERT_EQ(TOKEN_KEYWORD_ENUM, tl->tokens[0].kind);
  ASSERT_STR_EQ("enum", token_to_cstr(buf, sizeof(buf), &tl->tokens[0]));

  ASSERT_EQ(TOKEN_WHITESPACE, tl->tokens[1].kind);

  ASSERT_EQ(TOKEN_IDENTIFIER, tl->tokens[2].kind);
  ASSERT_STR_EQ("Color", token_to_cstr(buf, sizeof(buf), &tl->tokens[2]));

  ASSERT_EQ(TOKEN_WHITESPACE, tl->tokens[3].kind);

  ASSERT_EQ(TOKEN_LBRACE, tl->tokens[4].kind);
  ASSERT_STR_EQ("{", token_to_cstr(buf, sizeof(buf), &tl->tokens[4]));

  ASSERT_EQ(TOKEN_WHITESPACE, tl->tokens[5].kind);

  ASSERT_EQ(TOKEN_IDENTIFIER, tl->tokens[6].kind);
  ASSERT_STR_EQ("RED", token_to_cstr(buf, sizeof(buf), &tl->tokens[6]));

  ASSERT_EQ(TOKEN_COMMA, tl->tokens[7].kind);
  ASSERT_STR_EQ(",", token_to_cstr(buf, sizeof(buf), &tl->tokens[7]));

  ASSERT_EQ(TOKEN_WHITESPACE, tl->tokens[8].kind);

  ASSERT_EQ(TOKEN_IDENTIFIER, tl->tokens[9].kind);
  ASSERT_STR_EQ("GREEN", token_to_cstr(buf, sizeof(buf), &tl->tokens[9]));

  ASSERT_EQ(TOKEN_COMMA, tl->tokens[10].kind);
  ASSERT_STR_EQ(",", token_to_cstr(buf, sizeof(buf), &tl->tokens[10]));

  ASSERT_EQ(TOKEN_WHITESPACE, tl->tokens[11].kind);

  ASSERT_EQ(TOKEN_IDENTIFIER, tl->tokens[12].kind);
  ASSERT_STR_EQ("BLUE", token_to_cstr(buf, sizeof(buf), &tl->tokens[12]));

  ASSERT_EQ(TOKEN_WHITESPACE, tl->tokens[13].kind);

  /* Note: index 14 is the RBRACE */
  ASSERT_EQ(TOKEN_SEMICOLON, tl->tokens[15].kind);
  ASSERT_STR_EQ(";", token_to_cstr(buf, sizeof(buf), &tl->tokens[15]));

  free_token_list(tl);
  PASS();
}

TEST tokenize_with_comments(void) {
  const az_span code = AZ_SPAN_FROM_STR(
      "/* comment */\nstruct S { int x; }; // trailing comment");
  struct TokenList *tl = NULL;
  int ret = tokenize(code, &tl);
  char buf[64];
  ASSERT_EQ(0, ret);
  ASSERT(tl != NULL);

  ASSERT_GTE(tl->size, 16);

  ASSERT_EQ(TOKEN_COMMENT, tl->tokens[0].kind);
  ASSERT_STR_EQ("/* comment */",
                token_to_cstr(buf, sizeof(buf), &tl->tokens[0]));

  ASSERT_EQ(TOKEN_WHITESPACE, tl->tokens[1].kind);

  ASSERT_EQ(TOKEN_KEYWORD_STRUCT, tl->tokens[2].kind);
  ASSERT_STR_EQ("struct", token_to_cstr(buf, sizeof(buf), &tl->tokens[2]));

  ASSERT_EQ(TOKEN_WHITESPACE, tl->tokens[3].kind);

  ASSERT_EQ(TOKEN_IDENTIFIER, tl->tokens[4].kind);
  ASSERT_STR_EQ("S", token_to_cstr(buf, sizeof(buf), &tl->tokens[4]));

  ASSERT_EQ(TOKEN_WHITESPACE, tl->tokens[5].kind);

  ASSERT_EQ(TOKEN_LBRACE, tl->tokens[6].kind);
  ASSERT_STR_EQ("{", token_to_cstr(buf, sizeof(buf), &tl->tokens[6]));

  ASSERT_EQ(TOKEN_WHITESPACE, tl->tokens[7].kind);

  ASSERT_EQ(TOKEN_IDENTIFIER, tl->tokens[8].kind);
  ASSERT_STR_EQ("int", token_to_cstr(buf, sizeof(buf), &tl->tokens[8]));

  ASSERT_EQ(TOKEN_WHITESPACE, tl->tokens[9].kind);

  ASSERT_EQ(TOKEN_IDENTIFIER, tl->tokens[10].kind);
  ASSERT_STR_EQ("x", token_to_cstr(buf, sizeof(buf), &tl->tokens[10]));

  ASSERT_EQ(TOKEN_SEMICOLON, tl->tokens[11].kind);
  ASSERT_STR_EQ(";", token_to_cstr(buf, sizeof(buf), &tl->tokens[11]));

  ASSERT_EQ(TOKEN_WHITESPACE, tl->tokens[12].kind);

  ASSERT_EQ(TOKEN_RBRACE, tl->tokens[13].kind);
  ASSERT_STR_EQ("}", token_to_cstr(buf, sizeof(buf), &tl->tokens[13]));

  ASSERT_EQ(TOKEN_SEMICOLON, tl->tokens[14].kind);
  ASSERT_STR_EQ(";", token_to_cstr(buf, sizeof(buf), &tl->tokens[14]));

  ASSERT_EQ(TOKEN_WHITESPACE, tl->tokens[15].kind);

  ASSERT_EQ(TOKEN_COMMENT, tl->tokens[16].kind);
  ASSERT_STR_EQ("// trailing comment",
                token_to_cstr(buf, sizeof(buf), &tl->tokens[16]));

  free_token_list(tl);
  PASS();
}

TEST tokenize_specials_and_errors(void) {
  const az_span code = AZ_SPAN_FROM_STR("a 123 'x' \"foo\"\n"
                                        "#macro\n"
                                        "/*block*/\n"
                                        "//line\n"
                                        ",;{}");
  struct TokenList *tl = NULL;
  const int rc = tokenize(code, &tl);
  int found_comment = 0, found_macro = 0, found_str = 0, found_char = 0;
  size_t i;
  ASSERT_EQ(0, rc);
  ASSERT(tl != NULL);

  for (i = 0; i < tl->size; ++i) {
    switch (tl->tokens[i].kind) {
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
  free_token_list(tl);
  PASS();
}

TEST tokenizer_free_token_list_null(void) {
  /* Helper to test cleanup */
  free_token_list(NULL);
  PASS();
}

TEST tokenize_various_edge_cases(void) {
  struct TokenList *tl = NULL;

  ASSERT_EQ(0, tokenize(AZ_SPAN_FROM_STR("test/"), &tl));
  ASSERT(tl != NULL);
  ASSERT_EQ(2, tl->size);
  ASSERT_EQ(TOKEN_IDENTIFIER, tl->tokens[0].kind);
  ASSERT_EQ(TOKEN_OTHER, tl->tokens[1].kind);
  free_token_list(tl);
  tl = NULL;

  ASSERT_EQ(0, tokenize(AZ_SPAN_FROM_STR("/* foo"), &tl));
  ASSERT(tl != NULL);
  ASSERT_EQ(1, tl->size);
  ASSERT_EQ(TOKEN_COMMENT, tl->tokens[0].kind);
  free_token_list(tl);
  tl = NULL;

  ASSERT_EQ(0, tokenize(AZ_SPAN_FROM_STR("\"hello\\\"world\""), &tl));
  ASSERT(tl != NULL);
  ASSERT_EQ(1, tl->size);
  ASSERT_EQ(TOKEN_STRING_LITERAL, tl->tokens[0].kind);
  free_token_list(tl);
  tl = NULL;

  ASSERT_EQ(0, tokenize(AZ_SPAN_FROM_STR("'\\''a"), &tl));
  ASSERT(tl != NULL);
  ASSERT_EQ(2, tl->size);
  ASSERT_EQ(TOKEN_CHAR_LITERAL, tl->tokens[0].kind);
  ASSERT_EQ(TOKEN_IDENTIFIER, tl->tokens[1].kind);
  free_token_list(tl);
  tl = NULL;

  ASSERT_EQ(0, tokenize(AZ_SPAN_FROM_STR("@`$"), &tl));
  ASSERT(tl != NULL);
  ASSERT_EQ(3, tl->size);
  ASSERT_EQ(TOKEN_OTHER, tl->tokens[0].kind);
  ASSERT_EQ(TOKEN_OTHER, tl->tokens[1].kind);
  ASSERT_EQ(TOKEN_OTHER, tl->tokens[2].kind);
  free_token_list(tl);

  PASS();
}

TEST tokenize_realloc(void) {
  enum { n = 1025 };
  char long_string[n];
  size_t i;
  struct TokenList *tl = NULL;

  for (i = 0; i < n - 2; i += 2) {
    long_string[i] = 'a';
    long_string[i + 1] = ' ';
  }
  long_string[n - 1] = '\0';

  ASSERT_EQ(0, tokenize(az_span_create_from_str(long_string), &tl));
  ASSERT(tl != NULL);
  ASSERT_GT(tl->size, 64);
  free_token_list(tl);
  PASS();
}

TEST tokenize_operators(void) {
  const az_span code = AZ_SPAN_FROM_STR("{ } ; , . / : ? ~ ! & * + - ^ |");
  struct TokenList *tl = NULL;
  ASSERT_EQ(0, tokenize(code, &tl));
  ASSERT(tl != NULL);
  ASSERT_GT(tl->size, 15);
  free_token_list(tl);
  PASS();
}

TEST tokenize_more_unterminated(void) {
  struct TokenList *tl = NULL;

  ASSERT_EQ(0, tokenize(AZ_SPAN_FROM_STR("int x; /*"), &tl));
  ASSERT(tl != NULL);
  ASSERT_GT(tl->size, 0);
  ASSERT_EQ(TOKEN_COMMENT, tl->tokens[tl->size - 1].kind);
  free_token_list(tl);
  tl = NULL;

  ASSERT_EQ(0, tokenize(AZ_SPAN_FROM_STR("const char* s = \"abc"), &tl));
  ASSERT(tl != NULL);
  ASSERT_GT(tl->size, 0);
  ASSERT_EQ(TOKEN_STRING_LITERAL, tl->tokens[tl->size - 1].kind);
  free_token_list(tl);
  tl = NULL;

  ASSERT_EQ(0, tokenize(AZ_SPAN_FROM_STR("char c = 'a"), &tl));
  ASSERT(tl != NULL);
  ASSERT_GT(tl->size, 0);
  ASSERT_EQ(TOKEN_CHAR_LITERAL, tl->tokens[tl->size - 1].kind);
  free_token_list(tl);

  PASS();
}

TEST tokenize_escaped_backslash_in_string(void) {
  struct TokenList *tl = NULL;
  ASSERT_EQ(0, tokenize(AZ_SPAN_FROM_STR("\"foo\\\\\""), &tl));
  ASSERT(tl != NULL);
  ASSERT_EQ(1, tl->size);
  ASSERT_EQ(TOKEN_STRING_LITERAL, tl->tokens[0].kind);
  free_token_list(tl);
  PASS();
}

TEST tokenize_tricky_comments(void) {
  const az_span code = AZ_SPAN_FROM_STR("/**/ /* a /* b */ c */");
  struct TokenList *tl = NULL;
  ASSERT_EQ(0, tokenize(code, &tl));
  ASSERT(tl != NULL);
  ASSERT_GTE(tl->size, 2);
  ASSERT_EQ(TOKEN_COMMENT, tl->tokens[0].kind);
  ASSERT_EQ(TOKEN_WHITESPACE, tl->tokens[1].kind);
  ASSERT_EQ(TOKEN_COMMENT, tl->tokens[2].kind);
  free_token_list(tl);
  PASS();
}

TEST tokenize_various_literals(void) {
  const az_span code = AZ_SPAN_FROM_STR("'\\'' '\\\\' \"\\\\\" \"\\\\\\\"\"");
  struct TokenList *tl = NULL;

  ASSERT_EQ(0, tokenize(code, &tl));
  ASSERT(tl != NULL);
  ASSERT_EQ(7, tl->size);
  ASSERT_EQ(TOKEN_CHAR_LITERAL, tl->tokens[0].kind);
  ASSERT_EQ(TOKEN_CHAR_LITERAL, tl->tokens[2].kind);
  ASSERT_EQ(TOKEN_STRING_LITERAL, tl->tokens[4].kind);
  ASSERT_EQ(TOKEN_STRING_LITERAL, tl->tokens[6].kind);

  free_token_list(tl);
  PASS();
}

TEST tokenize_escaped_quote_in_string(void) {
  struct TokenList *tl = NULL;
  ASSERT_EQ(0, tokenize(AZ_SPAN_FROM_STR("\"foo\\\"\""), &tl));
  ASSERT(tl != NULL);
  ASSERT_EQ(1, tl->size);
  ASSERT_EQ(TOKEN_STRING_LITERAL, tl->tokens[0].kind);
  free_token_list(tl);
  PASS();
}

TEST test_token_to_cstr_edge_cases(void) {
  struct Token tok;
  char buf[5];

  tok.start = (const uint8_t *)"hello world";
  tok.length = 11;

  token_to_cstr(buf, sizeof(buf), &tok);
  ASSERT_STR_EQ("hell", buf);

  ASSERT_EQ(NULL, token_to_cstr(buf, 0, &tok));

  PASS();
}

TEST tokenize_identifier_with_underscore(void) {
  const az_span code = AZ_SPAN_FROM_STR("_my_var another_var an_other_");
  struct TokenList *tl = NULL;
  char buf[64];

  ASSERT_EQ(0, tokenize(code, &tl));
  ASSERT(tl != NULL);
  ASSERT_EQ(5, tl->size);

  ASSERT_EQ(TOKEN_IDENTIFIER, tl->tokens[0].kind);
  ASSERT_STR_EQ("_my_var", token_to_cstr(buf, sizeof(buf), &tl->tokens[0]));

  ASSERT_EQ(TOKEN_IDENTIFIER, tl->tokens[2].kind);
  ASSERT_STR_EQ("another_var", token_to_cstr(buf, sizeof(buf), &tl->tokens[2]));

  free_token_list(tl);
  PASS();
}

TEST tokenize_failure_handling(void) {
  /* Test NULL out pointer argument */
  ASSERT_EQ(EINVAL, tokenize(AZ_SPAN_FROM_STR(""), NULL));
  PASS();
}

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
  RUN_TEST(tokenize_operators);
  RUN_TEST(tokenize_more_unterminated);
  RUN_TEST(tokenize_escaped_backslash_in_string);
  RUN_TEST(tokenize_escaped_quote_in_string);
  RUN_TEST(tokenize_tricky_comments);
  RUN_TEST(tokenize_various_literals);
  RUN_TEST(test_token_to_cstr_edge_cases);
  RUN_TEST(tokenize_identifier_with_underscore);
  RUN_TEST(tokenize_failure_handling);
}

#endif /* !TEST_TOKENIZER_H */
