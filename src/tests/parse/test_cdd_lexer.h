#ifndef TEST_CDD_LEXER_H
#define TEST_CDD_LEXER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include <greatest.h>
#include <string.h>
#include "classes/parse/cdd_lexer.h"
/* clang-format on */

TEST test_cdd_lexer_basic(void) {
  cdd_token_list_t *list = NULL;
  const char *code = "int main() { /* comment */\n  return 0;\n}";
  int rc = cdd_lexer_tokenize(az_span_create_from_str((char *)code), &list);

  ASSERT_EQ(0, rc);
  ASSERT(list != NULL);
  ASSERT_EQ(9, list->size); /* int, main, (, ), {, return, 0, ;, } */

  /* int */
  ASSERT_EQ(CDD_TOKEN_KEYWORD_INT, list->tokens[0].kind);
  ASSERT(list->tokens[0].leading_trivia == NULL);
  ASSERT(list->tokens[0].trailing_trivia != NULL);
  ASSERT_EQ(TRIVIA_WHITESPACE, list->tokens[0].trailing_trivia->kind);

  /* main */
  ASSERT_EQ(CDD_TOKEN_IDENTIFIER, list->tokens[1].kind);

  /* return */
  ASSERT_EQ(CDD_TOKEN_KEYWORD_RETURN, list->tokens[5].kind);
  ASSERT(list->tokens[5].leading_trivia !=
         NULL); /* The newline and indent before return */

  cdd_lexer_free_token_list(list);
  PASS();
}

TEST test_cdd_lexer_empty(void) {
  cdd_token_list_t *list = NULL;
  int rc = cdd_lexer_tokenize(az_span_create_from_str(""), &list);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(0, list->size);
  cdd_lexer_free_token_list(list);
  PASS();
}

TEST test_cdd_lexer_trivia_only(void) {
  cdd_token_list_t *list = NULL;
  int rc =
      cdd_lexer_tokenize(az_span_create_from_str("   \n  // comment "), &list);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(1, list->size); /* EOF token */
  ASSERT_EQ(CDD_TOKEN_EOF, list->tokens[0].kind);
  ASSERT(list->tokens[0].leading_trivia != NULL);
  cdd_lexer_free_token_list(list);
  PASS();
}

TEST test_cdd_lexer_errors(void) {
  ASSERT_EQ(EINVAL, cdd_lexer_tokenize(az_span_create_from_str(""), NULL));
  cdd_lexer_free_token_list(NULL);
  PASS();
}

TEST test_cdd_lexer_strings(void) {
  cdd_token_list_t *list = NULL;
  int rc = cdd_lexer_tokenize(
      az_span_create_from_str("\"hello \\\" world\" 'a'"), &list);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(2, list->size);
  ASSERT_EQ(CDD_TOKEN_STRING, list->tokens[0].kind);
  ASSERT_EQ(CDD_TOKEN_CHAR, list->tokens[1].kind);
  cdd_lexer_free_token_list(list);
  PASS();
}

TEST test_cdd_lexer_symbols(void) {
  cdd_token_list_t *list = NULL;
  int rc = cdd_lexer_tokenize(
      az_span_create_from_str("== != = ! + - * / . , ; [ ] ( ) { }"), &list);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(17, list->size);
  cdd_lexer_free_token_list(list);
  PASS();
}

SUITE(cdd_lexer_suite) {
  RUN_TEST(test_cdd_lexer_basic);
  RUN_TEST(test_cdd_lexer_empty);
  RUN_TEST(test_cdd_lexer_trivia_only);
  RUN_TEST(test_cdd_lexer_errors);
  RUN_TEST(test_cdd_lexer_strings);
  RUN_TEST(test_cdd_lexer_symbols);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CDD_LEXER_H */
