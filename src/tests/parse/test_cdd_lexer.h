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

/**
 * @brief test_cdd_lexer_basic
 * @return TEST
 */
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

/**
 * @brief test_cdd_lexer_empty
 * @return TEST
 */
TEST test_cdd_lexer_empty(void) {
  cdd_token_list_t *list = NULL;
  int rc = cdd_lexer_tokenize(az_span_create_from_str(""), &list);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(0, list->size);
  cdd_lexer_free_token_list(list);
  PASS();
}

/**
 * @brief test_cdd_lexer_trivia_only
 * @return TEST
 */
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

/**
 * @brief test_cdd_lexer_errors
 * @return TEST
 */
TEST test_cdd_lexer_errors(void) {
  ASSERT_EQ(EINVAL, cdd_lexer_tokenize(az_span_create_from_str(""), NULL));
  cdd_lexer_free_token_list(NULL);
  PASS();
}

/**
 * @brief test_cdd_lexer_strings
 * @return TEST
 */
TEST test_cdd_lexer_strings(void) {
  cdd_token_list_t *list = NULL;
  int rc = cdd_lexer_tokenize(
      az_span_create_from_str("\"hello \\\" world\" 'a' \"line1\\\nline2\""),
      &list);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(3, list->size);
  ASSERT_EQ(CDD_TOKEN_STRING, list->tokens[0].kind);
  ASSERT_EQ(CDD_TOKEN_CHAR, list->tokens[1].kind);
  ASSERT_EQ(CDD_TOKEN_STRING, list->tokens[2].kind);
  ASSERT_EQ(14,
            list->tokens[2].length); /* "line1\<newline>line2" -> 14 bytes */
  cdd_lexer_free_token_list(list);
  PASS();
}

/**
 * @brief test_cdd_lexer_symbols
 * @return TEST
 */
TEST test_cdd_lexer_symbols(void) {
  cdd_token_list_t *list = NULL;
  int rc = cdd_lexer_tokenize(
      az_span_create_from_str("== != = ! + - * / . , ; [ ] ( ) { }"), &list);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(17, list->size);
  cdd_lexer_free_token_list(list);
  PASS();
}

/**
 * @brief test_cdd_lexer_gnu_extensions
 * @return TEST
 */
TEST test_cdd_lexer_gnu_extensions(void) {
  cdd_token_list_t *list = NULL;
  int rc =
      cdd_lexer_tokenize(az_span_create_from_str(
                             "__int128 typeof __typeof__ __auto_type __label__ "
                             "__complex__ __real__ __imag__"),
                         &list);
  ASSERT_EQ(0, rc);
  ASSERT_EQ(8, list->size);
  ASSERT_EQ(CDD_TOKEN_KEYWORD___INT128, list->tokens[0].kind);
  ASSERT_EQ(CDD_TOKEN_KEYWORD_TYPEOF, list->tokens[1].kind);
  ASSERT_EQ(CDD_TOKEN_KEYWORD_TYPEOF, list->tokens[2].kind);
  ASSERT_EQ(CDD_TOKEN_KEYWORD___AUTO_TYPE, list->tokens[3].kind);
  ASSERT_EQ(CDD_TOKEN_KEYWORD___LABEL__, list->tokens[4].kind);
  ASSERT_EQ(CDD_TOKEN_KEYWORD___COMPLEX__, list->tokens[5].kind);
  ASSERT_EQ(CDD_TOKEN_KEYWORD___REAL__, list->tokens[6].kind);
  ASSERT_EQ(CDD_TOKEN_KEYWORD___IMAG__, list->tokens[7].kind);
  cdd_lexer_free_token_list(list);
  PASS();
}

/**
 * @brief test_cdd_lexer_multiline_macro
 * @return TEST
 */
TEST test_cdd_lexer_multiline_macro(void) {
  cdd_token_list_t *list = NULL;
  const char *code = "#define FOO(x) \\\n  do { \\\n    x++; // incr \\\n  } "
                     "while(0)\nint main(){}";
  int rc = cdd_lexer_tokenize(az_span_create_from_str((char *)code), &list);

  ASSERT_EQ(0, rc);
  ASSERT(list != NULL);
  /* The macro should be a single token up to while(0) */
  ASSERT_EQ(CDD_TOKEN_PREPROC_DEFINE, list->tokens[0].kind);
  ASSERT_EQ(57, list->tokens[0].length);
  /* Next token should be int */
  ASSERT_EQ(CDD_TOKEN_KEYWORD_INT, list->tokens[1].kind);

  cdd_lexer_free_token_list(list);
  PASS();
}

/**
 * @brief test_cdd_lexer_include_next
 * @return TEST
 */
TEST test_cdd_lexer_include_next(void) {
  cdd_token_list_t *list = NULL;
  const char *code = "#include_next <stdio.h>\n";
  int rc = cdd_lexer_tokenize(az_span_create_from_str((char *)code), &list);

  ASSERT_EQ(0, rc);
  ASSERT(list != NULL);
  ASSERT_EQ(CDD_TOKEN_PREPROC_INCLUDE, list->tokens[0].kind);

  cdd_lexer_free_token_list(list);
  PASS();
}

/**
 * @brief cdd_lexer_suite
 */
SUITE(cdd_lexer_suite) {
  RUN_TEST(test_cdd_lexer_basic);
  RUN_TEST(test_cdd_lexer_empty);
  RUN_TEST(test_cdd_lexer_trivia_only);
  RUN_TEST(test_cdd_lexer_errors);
  RUN_TEST(test_cdd_lexer_strings);
  RUN_TEST(test_cdd_lexer_symbols);
  RUN_TEST(test_cdd_lexer_gnu_extensions);
  RUN_TEST(test_cdd_lexer_multiline_macro);
  RUN_TEST(test_cdd_lexer_include_next);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CDD_LEXER_H */
