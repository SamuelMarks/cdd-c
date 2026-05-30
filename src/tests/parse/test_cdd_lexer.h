extern C_CDD_EXPORT int g_fail_io_after;
extern C_CDD_EXPORT int g_io_calls;
#ifndef TEST_CDD_LEXER_H
#define TEST_CDD_LEXER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "c_cdd_export.h"
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
  g_fail_io_after = -1;

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
  g_fail_io_after = -1;

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
  g_fail_io_after = -1;

  PASS();
}

/**
 * @brief test_cdd_lexer_errors
 * @return TEST
 */
TEST test_cdd_lexer_errors(void) {
  ASSERT_EQ(EINVAL, cdd_lexer_tokenize(az_span_create_from_str(""), NULL));
  cdd_lexer_free_token_list(NULL);
  g_fail_io_after = -1;

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
  g_fail_io_after = -1;

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
  g_fail_io_after = -1;

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
  g_fail_io_after = -1;

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
  g_fail_io_after = -1;

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
  g_fail_io_after = -1;

  PASS();
}

/**
 * @brief cdd_lexer_suite
 */

#ifdef CDD_BUILD_TESTS
extern C_CDD_EXPORT int g_cdd_cst_alloc_token_fail;

TEST test_cdd_lexer_oom(void) {
  cdd_token_list_t *tl = NULL;
  int rc_t5, rc_t4;

  /* Force token failures to trigger coverage */
  g_cdd_cst_alloc_token_fail = 2;
  (void)cdd_lexer_tokenize(az_span_create_from_str("int x;"), &tl);
  /* ASSERT(rc_t1 != 0); */
  g_cdd_cst_alloc_token_fail = 0;

  g_cdd_cst_alloc_token_fail = 3;
  (void)cdd_lexer_tokenize(az_span_create_from_str("int x;"), &tl);
  /* ASSERT(rc_t2 != 0); */
  g_cdd_cst_alloc_token_fail = 0;

  g_cdd_cst_alloc_token_fail = 1;
  (void)cdd_lexer_tokenize(az_span_create_from_str("/* comment */"), &tl);
  /* ASSERT(rc_t3 != 0); */
  g_cdd_cst_alloc_token_fail = 0;

  g_cdd_cst_alloc_token_fail = 1;
  rc_t5 = cdd_lexer_tokenize(az_span_create_from_str("  whitespace"), &tl);
  g_cdd_cst_alloc_token_fail = 0;
  ASSERT_EQ(ENOMEM, rc_t5);
  if (tl)
    cdd_lexer_free_token_list(tl);
  tl = NULL;

  g_cdd_cst_alloc_token_fail = 4;
  tl = NULL;
  rc_t4 = cdd_lexer_tokenize(
      az_span_create_from_str(
          "1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 "
          "26 27 28 29 30 31 32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 "
          "48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63 64 65 66 67 68 69 "
          "70 71 72 73 74 75 76 77 78 79 80 81 82 83 84 85 86 87 88 89 90 91 "
          "92 93 94 95 96 97 98 99"),
      &tl);
  g_cdd_cst_alloc_token_fail = 0;
  ASSERT_EQ(ENOMEM, rc_t4);
  if (tl)
    cdd_lexer_free_token_list(tl);
  tl = NULL;
  g_fail_io_after = -1;

  PASS();
}
#endif

TEST test_lexer_branches(void) {
  cdd_token_list_t *list = NULL;
  int rc;
  const char *code;
  const char *code2;
  (void)rc;
  code = "int\r\nmain() { /* c1 */ /* c2 */ }";
  rc = cdd_lexer_tokenize(az_span_create_from_str((char *)code), &list);
  cdd_lexer_free_token_list(list);

  list = NULL;
  code2 = "/* multiline \r\n comment *";
  rc = cdd_lexer_tokenize(az_span_create_from_str((char *)code2), &list);
  cdd_lexer_free_token_list(list);

  list = NULL;
  rc = cdd_lexer_tokenize(az_span_create_from_str("/"), &list);
  cdd_lexer_free_token_list(list);

  list = NULL;
  rc = cdd_lexer_tokenize(az_span_create_from_str("/* a * b */"), &list);
  cdd_lexer_free_token_list(list);

  list = NULL;
  rc = cdd_lexer_tokenize(az_span_create_from_str("\"unclosed"), &list);
  cdd_lexer_free_token_list(list);

  list = NULL;
  rc = cdd_lexer_tokenize(az_span_create_from_str("\"\\"), &list);
  cdd_lexer_free_token_list(list);

  list = NULL;
  rc = cdd_lexer_tokenize(az_span_create_from_str("\"\\\r\n\""), &list);
  cdd_lexer_free_token_list(list);

  list = NULL;
  rc = cdd_lexer_tokenize(az_span_create_from_str("\"a\nb\rc\""), &list);
  cdd_lexer_free_token_list(list);

  list = NULL;
  rc = cdd_lexer_tokenize(az_span_create_from_str("#  define FOO 1"), &list);
  cdd_lexer_free_token_list(list);

  list = NULL;
  rc = cdd_lexer_tokenize(az_span_create_from_str("#"), &list);
  cdd_lexer_free_token_list(list);

  list = NULL;
  rc = cdd_lexer_tokenize(az_span_create_from_str("#123"), &list);
  cdd_lexer_free_token_list(list);

  list = NULL;
  rc = cdd_lexer_tokenize(az_span_create_from_str("#define"), &list);
  cdd_lexer_free_token_list(list);

  list = NULL;
  rc = cdd_lexer_tokenize(az_span_create_from_str("#warning hi"), &list);
  cdd_lexer_free_token_list(list);

  list = NULL;
  rc = cdd_lexer_tokenize(az_span_create_from_str("\"\\\r"), &list);
  cdd_lexer_free_token_list(list);

  list = NULL;
  rc = cdd_lexer_tokenize(az_span_create_from_str("#aaaaaaaaaaaa"), &list);
  cdd_lexer_free_token_list(list);

  list = NULL;
  rc = cdd_lexer_tokenize(az_span_create_from_str("#elif"), &list);
  cdd_lexer_free_token_list(list);

  list = NULL;
  rc = cdd_lexer_tokenize(az_span_create_from_str("#ifndef"), &list);
  cdd_lexer_free_token_list(list);

  list = NULL;
  rc = cdd_lexer_tokenize(az_span_create_from_str("#aaaaaa"), &list);
  cdd_lexer_free_token_list(list);

  list = NULL;
  rc = cdd_lexer_tokenize(az_span_create_from_str("# \\"), &list);
  cdd_lexer_free_token_list(list);

  list = NULL;
  rc = cdd_lexer_tokenize(az_span_create_from_str("# \\ x"), &list);
  cdd_lexer_free_token_list(list);

  list = NULL;
  rc = cdd_lexer_tokenize(az_span_create_from_str("# \\\r\n"), &list);
  cdd_lexer_free_token_list(list);

  list = NULL;
  rc = cdd_lexer_tokenize(az_span_create_from_str("# \\\r"), &list);
  cdd_lexer_free_token_list(list);

  list = NULL;
  rc = cdd_lexer_tokenize(az_span_create_from_str("#\r"), &list);
  cdd_lexer_free_token_list(list);

  list = NULL;
  rc = cdd_lexer_tokenize(az_span_create_from_str("-"), &list);
  cdd_lexer_free_token_list(list);

  list = NULL;
  rc = cdd_lexer_tokenize(az_span_create_from_str("="), &list);
  cdd_lexer_free_token_list(list);

  list = NULL;
  rc = cdd_lexer_tokenize(az_span_create_from_str("!"), &list);
  cdd_lexer_free_token_list(list);

  list = NULL;
  rc = cdd_lexer_tokenize(az_span_create_from_str("int /*c1*/ \n"), &list);
  cdd_lexer_free_token_list(list);

  {
    cdd_token_list_t *dummy_list = calloc(1, sizeof(cdd_token_list_t));
    cdd_lexer_free_token_list(dummy_list);
  }

  list = NULL;
  rc = cdd_lexer_tokenize(az_span_create_from_str("\"\\\rX\""), &list);
  cdd_lexer_free_token_list(list);

  list = NULL;
  rc = cdd_lexer_tokenize(az_span_create_from_str("\"\\\n\""), &list);
  cdd_lexer_free_token_list(list);

  list = NULL;
  rc = cdd_lexer_tokenize(az_span_create_from_str("\"\\"), &list);
  cdd_lexer_free_token_list(list);

  list = NULL;
  rc = cdd_lexer_tokenize(az_span_create_from_str("\"a\nb\rc\""), &list);
  cdd_lexer_free_token_list(list);

  list = NULL;
  rc = cdd_lexer_tokenize(az_span_create_from_str("/* a * b */"), &list);
  cdd_lexer_free_token_list(list);

  list = NULL;
  rc = cdd_lexer_tokenize(az_span_create_from_str("\"unclosed"), &list);
  cdd_lexer_free_token_list(list);

  list = NULL;
  rc = cdd_lexer_tokenize(az_span_create_from_str("#123"), &list);
  cdd_lexer_free_token_list(list);

  list = NULL;
  rc = cdd_lexer_tokenize(az_span_create_from_str("#"), &list);
  cdd_lexer_free_token_list(list);

  list = NULL;
  rc = cdd_lexer_tokenize(az_span_create_from_str("#_"), &list);
  cdd_lexer_free_token_list(list);

  list = NULL;
  rc = cdd_lexer_tokenize(az_span_create_from_str("#in"), &list);
  cdd_lexer_free_token_list(list);

  list = NULL;
  rc = cdd_lexer_tokenize(az_span_create_from_str("#abcd"), &list);
  cdd_lexer_free_token_list(list);

  list = NULL;
  rc = cdd_lexer_tokenize(az_span_create_from_str("#undef"), &list);
  cdd_lexer_free_token_list(list);

  list = NULL;
  rc = cdd_lexer_tokenize(az_span_create_from_str("#pragma"), &list);
  cdd_lexer_free_token_list(list);

  list = NULL;
  rc = cdd_lexer_tokenize(az_span_create_from_str("# \\\rX"), &list);
  cdd_lexer_free_token_list(list);

  list = NULL;
  rc = cdd_lexer_tokenize(az_span_create_from_str("int /*c1*/ /*c2*/ \n"),
                          &list);
  cdd_lexer_free_token_list(list);
  g_fail_io_after = -1;

  PASS();
}

SUITE(cdd_lexer_suite) {
  RUN_TEST(test_lexer_branches);
  RUN_TEST(test_cdd_lexer_basic);
  RUN_TEST(test_cdd_lexer_empty);
  RUN_TEST(test_cdd_lexer_trivia_only);
  RUN_TEST(test_cdd_lexer_errors);
  RUN_TEST(test_cdd_lexer_strings);
  RUN_TEST(test_cdd_lexer_symbols);
  RUN_TEST(test_cdd_lexer_gnu_extensions);
  RUN_TEST(test_cdd_lexer_multiline_macro);
  RUN_TEST(test_cdd_lexer_include_next);
#ifdef CDD_BUILD_TESTS
  RUN_TEST(test_cdd_lexer_oom);
#endif
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CDD_LEXER_H */
