/**
 * @file test_tokenizer_trigraphs.h
 * @brief Unit tests for tokenizer trigraphs.
 */

#ifndef TEST_TOKENIZER_TRIGRAPHS_H
#define TEST_TOKENIZER_TRIGRAPHS_H

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

/* Helper setup */

static cdd_c_error_t tokenize_string(const char *s,
                                     struct TokenList **_out_val) {
  struct TokenList *tl = NULL;
  az_span span = az_span_create_from_str((char *)(size_t)s);
  cdd_c_error_t rc = tokenize(span, &tl);
  if (rc != CDD_C_SUCCESS) {
    return rc;
  }
  *_out_val = tl;
  return CDD_C_SUCCESS;
}

TEST test_trigraph_basic(void) {
  struct TokenList *_ast_tokenize_string_0;

  /* ??= is # */

  struct TokenList *tl =
      (tokenize_string("?\?= include", &_ast_tokenize_string_0),
       _ast_tokenize_string_0);

  ASSERT(tl);

  ASSERT_EQ(TOKEN_HASH, tl->tokens[0].kind);

  ASSERT_EQ(TOKEN_IDENTIFIER, tl->tokens[2].kind); /* index 1 is WS */

  free_token_list(tl);
  g_fail_io_after = -1;

  PASS();
}

TEST test_splice_basic(void) {
  struct TokenList *_ast_tokenize_string_1;

  /* i\nnt -> int */

  struct TokenList *tl =
      (tokenize_string("i\\\nnt x;", &_ast_tokenize_string_1),
       _ast_tokenize_string_1);

  ASSERT(tl);

  ASSERT_EQ(TOKEN_KEYWORD_INT, tl->tokens[0].kind);

  ASSERT_EQ(TOKEN_WHITESPACE, tl->tokens[1].kind);

  ASSERT_EQ(TOKEN_IDENTIFIER, tl->tokens[2].kind);

  free_token_list(tl);
  g_fail_io_after = -1;

  PASS();
}

TEST test_trigraph_splice_interaction(void) {
  struct TokenList *_ast_tokenize_string_2;

  /* Edge case: ??/ is backslash. ??/ followed by newline is a splice. */

  /* i??/\nnt -> int */

  struct TokenList *tl =
      (tokenize_string("i?\?/\nnt x;", &_ast_tokenize_string_2),
       _ast_tokenize_string_2);

  ASSERT(tl);

  ASSERT_EQ(TOKEN_KEYWORD_INT, tl->tokens[0].kind);

  free_token_list(tl);
  g_fail_io_after = -1;

  PASS();
}

TEST test_splice_does_not_create_trigraph(void) {
  struct TokenList *_ast_tokenize_string_3;

  /* ?\n?=  -> ? ? = (Tokens) */

  /* Because splice happens in Phase 2, Trigraph in Phase 1. */

  /* Input: ? \ \n ? = */

  /* Phase 1: ? \ \n ? = (No trigraphs) */

  /* Phase 2: ? ? = (Splice removes \ \n) */

  /* Phase 3: Tokenize ? ? = */

  /* ? ? = is usually QUESTION QUESTION ASSIGN unless merged by ??? logic? */

  /* C spec 6.4 Lexical elements: "The source file is decomposed into

   * preprocessing tokens" */

  /* ? is a punctuator. ? is punctuator. = is punctuator. */

  /* It is NOT a TOKEN_HASH */

  struct TokenList *tl = (tokenize_string("?\\\n?=", &_ast_tokenize_string_3),
                          _ast_tokenize_string_3);

  ASSERT(tl);

  /* Should be QUESTION, QUESTION, ASSIGN. Not HASH */

  ASSERT_EQ(TOKEN_QUESTION, tl->tokens[0].kind);

  ASSERT_EQ(TOKEN_QUESTION, tl->tokens[1].kind);

  ASSERT_EQ(TOKEN_ASSIGN, tl->tokens[2].kind);

  free_token_list(tl);
  g_fail_io_after = -1;

  PASS();
}

TEST test_matches_string_with_splice(void) {
  struct TokenList *_ast_tokenize_string_4;
  int _ast_token_matches_string_0 = 0;

  struct TokenList *tl =
      (tokenize_string("RE\\\nTURN", &_ast_tokenize_string_4),
       _ast_tokenize_string_4);

  ASSERT(tl);

  ASSERT_EQ(

      TOKEN_IDENTIFIER,

      tl->tokens[0].kind); /* RETURN is keyword only if exact match usually? */

  /* Wait, "return" is keyword. "RE\nTURN" is "RETURN". */

  /* Current identify_keyword simple implementation relies on span_equals_str

     which is strict raw content. So "RE\nTURN" becomes IDENTIFIER, not KEYWORD

     using the optimized identify func unless we normalized in identify_keyword.

     But token_matches_string should handle semantic match. */

  ASSERT((token_matches_string(&tl->tokens[0], "RETURN",
                               &_ast_token_matches_string_0),
          _ast_token_matches_string_0));

  free_token_list(tl);
  g_fail_io_after = -1;

  PASS();
}

TEST test_all_remaining_trigraphs(void) {
  struct TokenList *tl = NULL;
  cdd_c_error_t rc;

  /* Test ??( [ */
  rc = tokenize_string("?\?(", &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(tl);
  ASSERT_EQ(TOKEN_LBRACKET, tl->tokens[0].kind);
  free_token_list(tl);

  /* Test ??) ] */
  rc = tokenize_string("?\?)", &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(tl);
  ASSERT_EQ(TOKEN_RBRACKET, tl->tokens[0].kind);
  free_token_list(tl);

  /* Test ??' ^ */
  rc = tokenize_string("?\?'", &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(tl);
  ASSERT_EQ(TOKEN_CARET, tl->tokens[0].kind);
  free_token_list(tl);

  /* Test ??< { */
  rc = tokenize_string("?\?<", &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(tl);
  ASSERT_EQ(TOKEN_LBRACE, tl->tokens[0].kind);
  free_token_list(tl);

  /* Test ??> } */
  rc = tokenize_string("?\?>", &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(tl);
  ASSERT_EQ(TOKEN_RBRACE, tl->tokens[0].kind);
  free_token_list(tl);

  /* Test ??! | */
  rc = tokenize_string("?\?!", &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(tl);
  ASSERT_EQ(TOKEN_PIPE, tl->tokens[0].kind);
  free_token_list(tl);

  /* Test ??- ~ */
  rc = tokenize_string("?\?-", &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(tl);
  ASSERT_EQ(TOKEN_TILDE, tl->tokens[0].kind);
  free_token_list(tl);

  /* Test non-trigraph ??z */
  rc = tokenize_string("?\?z", &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(tl);
  ASSERT_EQ(TOKEN_QUESTION, tl->tokens[0].kind);
  ASSERT_EQ(TOKEN_QUESTION, tl->tokens[1].kind);
  ASSERT_EQ(TOKEN_IDENTIFIER, tl->tokens[2].kind);
  free_token_list(tl);

  g_fail_io_after = -1;
  PASS();
}

TEST test_splice_crlf_and_eof(void) {
  struct TokenList *tl = NULL;
  cdd_c_error_t rc;

  /* Test CRLF splice: \ \r \n */
  rc = tokenize_string("i\\\r\nnt y;", &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(tl);
  ASSERT_EQ(TOKEN_KEYWORD_INT, tl->tokens[0].kind);
  ASSERT_EQ(TOKEN_WHITESPACE, tl->tokens[1].kind);
  ASSERT_EQ(TOKEN_IDENTIFIER, tl->tokens[2].kind);
  free_token_list(tl);

  /* Test trailing backslash before EOF */
  rc = tokenize_string("x\\", &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(tl);
  ASSERT_EQ(TOKEN_IDENTIFIER, tl->tokens[0].kind);
  ASSERT_EQ(TOKEN_OTHER, tl->tokens[1].kind);
  free_token_list(tl);

  /* Test backslash-r at EOF */
  rc = tokenize_string("x\\\r", &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(tl);
  ASSERT_EQ(TOKEN_IDENTIFIER, tl->tokens[0].kind);
  free_token_list(tl);

  /* Test backslash-r not followed by newline */
  rc = tokenize_string("x\\\ra", &tl);
  ASSERT_EQ(CDD_C_SUCCESS, rc);
  ASSERT(tl);
  ASSERT_EQ(TOKEN_IDENTIFIER, tl->tokens[0].kind);
  free_token_list(tl);

  g_fail_io_after = -1;
  PASS();
}

SUITE(tokenizer_trigraphs_suite) {

  RUN_TEST(test_trigraph_basic);

  RUN_TEST(test_splice_basic);

  RUN_TEST(test_trigraph_splice_interaction);

  RUN_TEST(test_splice_does_not_create_trigraph);

  RUN_TEST(test_matches_string_with_splice);
  RUN_TEST(test_all_remaining_trigraphs);
  RUN_TEST(test_splice_crlf_and_eof);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_TOKENIZER_TRIGRAPHS_H */
