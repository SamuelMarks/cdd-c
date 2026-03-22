#ifndef TEST_PRAGMA_H
#define TEST_PRAGMA_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include <greatest.h>
#include <stdlib.h>
#include <string.h>

#include "functions/parse/str.h"
#include "functions/parse/tokenizer.h"
/* clang-format on */

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
  char *_ast_destringize_0 = NULL;
  char *res = (c_cdd_destringize("\"simple\"", &_ast_destringize_0),
               _ast_destringize_0);
  ASSERT(res);
  ASSERT_STR_EQ("simple", res);
  free(res);
  PASS();
}

TEST test_destringize_escaped_quote(void) {
  char *_ast_destringize_1 = NULL;
  /* "foo\"bar" -> foo"bar */
  const char *input = "\"foo\\\"bar\"";
  char *res =
      (c_cdd_destringize(input, &_ast_destringize_1), _ast_destringize_1);
  ASSERT(res);
  ASSERT_STR_EQ("foo\"bar", res);
  free(res);
  PASS();
}

TEST test_destringize_escaped_backslash(void) {
  char *_ast_destringize_2 = NULL;
  /* "path\\to" -> path\to */
  const char *input = "\"path\\\\to\"";
  char *res =
      (c_cdd_destringize(input, &_ast_destringize_2), _ast_destringize_2);
  ASSERT(res);
  ASSERT_STR_EQ("path\\to", res);
  free(res);
  PASS();
}

TEST test_destringize_wide_literal(void) {
  char *_ast_destringize_3 = NULL;
  /* L"wide" -> wide */
  char *res =
      (c_cdd_destringize("L\"wide\"", &_ast_destringize_3), _ast_destringize_3);
  ASSERT(res);
  ASSERT_STR_EQ("wide", res);
  free(res);
  PASS();
}

TEST test_destringize_mixed(void) {
  char *_ast_destringize_4 = NULL;
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
  char *res =
      (c_cdd_destringize(input, &_ast_destringize_4), _ast_destringize_4);

  ASSERT(res);
  ASSERT_STR_EQ("a\\\"b", res); /* C String comparison: a\"b */
  free(res);
  PASS();
}

TEST test_destringize_invalids(void) {
  char *_ast_destringize_5 = NULL;
  char *_ast_destringize_6 = NULL;
  char *_ast_destringize_7 = NULL;
  ASSERT_EQ(NULL,
            (c_cdd_destringize(NULL, &_ast_destringize_5), _ast_destringize_5));
  ASSERT_EQ(NULL, (c_cdd_destringize("unquoted", &_ast_destringize_6),
                   _ast_destringize_6));
  ASSERT_EQ(NULL,
            (c_cdd_destringize("'c'", &_ast_destringize_7),
             _ast_destringize_7)); /* Char literal not supported by this */
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

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_PRAGMA_H */
