#ifndef TEST_PRAGMA_H
#define TEST_PRAGMA_H

#include <greatest.h>
#include <stdlib.h>
#include <string.h>

#include "str_utils.h"
#include "tokenizer.h"

TEST test_tokenize_pragma_op(void) {
  struct TokenList *tl = NULL;
  /* _Pragma ( "pack(1)" ) */
  const char *code = "_Pragma(\"pack(1)\")";
  az_span span = az_span_create_from_str((char *)code);
  int rc = tokenize(span, &tl);

  ASSERT_EQ(0, rc);
  ASSERT(tl != NULL);
  ASSERT_EQ(4, tl->size);

  ASSERT_EQ(TOKEN_KEYWORD_PRAGMA_OP, tl->tokens[0].kind);
  ASSERT_EQ(TOKEN_LPAREN, tl->tokens[1].kind);
  ASSERT_EQ(TOKEN_STRING_LITERAL, tl->tokens[2].kind);
  ASSERT_EQ(TOKEN_RPAREN, tl->tokens[3].kind);

  free_token_list(tl);
  PASS();
}

TEST test_destringize_basic(void) {
  char *res = c_cdd_destringize("\"simple\"");
  ASSERT(res);
  ASSERT_STR_EQ("simple", res);
  free(res);
  PASS();
}

TEST test_destringize_escaped_quote(void) {
  /* "foo\"bar" -> foo"bar */
  const char *input = "\"foo\\\"bar\"";
  char *res = c_cdd_destringize(input);
  ASSERT(res);
  ASSERT_STR_EQ("foo\"bar", res);
  free(res);
  PASS();
}

TEST test_destringize_escaped_backslash(void) {
  /* "path\\to" -> path\to */
  const char *input = "\"path\\\\to\"";
  char *res = c_cdd_destringize(input);
  ASSERT(res);
  ASSERT_STR_EQ("path\\to", res);
  free(res);
  PASS();
}

TEST test_destringize_wide_literal(void) {
  /* L"wide" -> wide */
  char *res = c_cdd_destringize("L\"wide\"");
  ASSERT(res);
  ASSERT_STR_EQ("wide", res);
  free(res);
  PASS();
}

TEST test_destringize_mixed(void) {
  /* "a\\\"b" -> a\"b */
  /* The tokenizer produces the literal including quotes and internal escapes.
   */
  /* Literal in C file: "a\\\"b" */
  /* In memory char[]: a, \, ", b */
  /* Goal output: a\"b? No. */
  /*
     Spec:
     \\ -> \
     \" -> "
  */
  /* Input String: "a\\\"b" (Length 8 chars including quotes) */
  /* index 0: " */
  /* index 1: a */
  /* index 2: \ */
  /* index 3: \ (escaped backslash) */
  /* index 4: \ */
  /* index 5: " (escaped quote) */
  /* index 6: b */
  /* index 7: " */

  /* Decoding:
     a -> a
     \\ -> \
     \" -> "
     b -> b
     Result: a\"b
  */
  const char *input = "\"a\\\\\\\"b\""; /* C string: "a\\\"b" */
  char *res = c_cdd_destringize(input);

  ASSERT(res);
  ASSERT_STR_EQ("a\\\"b", res); /* C String comparison: a\"b */
  free(res);
  PASS();
}

TEST test_destringize_invalids(void) {
  ASSERT_EQ(NULL, c_cdd_destringize(NULL));
  ASSERT_EQ(NULL, c_cdd_destringize("unquoted"));
  ASSERT_EQ(NULL,
            c_cdd_destringize("'c'")); /* Char literal not supported by this */
  PASS();
}

SUITE(pragma_suite) {
  RUN_TEST(test_tokenize_pragma_op);
  RUN_TEST(test_destringize_basic);
  RUN_TEST(test_destringize_escaped_quote);
  RUN_TEST(test_destringize_escaped_backslash);
  RUN_TEST(test_destringize_wide_literal);
  RUN_TEST(test_destringize_mixed);
  RUN_TEST(test_destringize_invalids);
}

#endif /* TEST_PRAGMA_H */
