#ifndef TEST_TOKENIZER_TRIGRAPHS_H
#define TEST_TOKENIZER_TRIGRAPHS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <greatest.h>

#include "tokenizer.h"

/* Helper setup */
static struct TokenList *tokenize_string(const char *s) {
  struct TokenList *tl = NULL;
  az_span span = az_span_create_from_str((char *)s);
  if (tokenize(span, &tl) != 0)
    return NULL;
  return tl;
}

TEST test_trigraph_basic(void) {
  /* ??= is # */
  struct TokenList *tl = tokenize_string("??= include");
  ASSERT(tl);
  ASSERT_EQ(TOKEN_HASH, tl->tokens[0].kind);
  ASSERT_EQ(TOKEN_IDENTIFIER, tl->tokens[2].kind); /* index 1 is WS */
  free_token_list(tl);
  PASS();
}

TEST test_splice_basic(void) {
  /* i\nnt -> int */
  struct TokenList *tl = tokenize_string("i\\\nnt x;");
  ASSERT(tl);
  ASSERT_EQ(TOKEN_KEYWORD_INT, tl->tokens[0].kind);
  ASSERT_EQ(TOKEN_WHITESPACE, tl->tokens[1].kind);
  ASSERT_EQ(TOKEN_IDENTIFIER, tl->tokens[2].kind);
  free_token_list(tl);
  PASS();
}

TEST test_trigraph_splice_interaction(void) {
  /* Edge case: ??/ is backslash. ??/ followed by newline is a splice. */
  /* i??/\nnt -> int */
  struct TokenList *tl = tokenize_string("i??/\nnt x;");
  ASSERT(tl);
  ASSERT_EQ(TOKEN_KEYWORD_INT, tl->tokens[0].kind);
  free_token_list(tl);
  PASS();
}

TEST test_splice_does_not_create_trigraph(void) {
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

  struct TokenList *tl = tokenize_string("?\\\n?=");
  ASSERT(tl);
  /* Should be QUESTION, QUESTION, ASSIGN. Not HASH */
  ASSERT_EQ(TOKEN_QUESTION, tl->tokens[0].kind);
  ASSERT_EQ(TOKEN_QUESTION, tl->tokens[1].kind);
  ASSERT_EQ(TOKEN_ASSIGN, tl->tokens[2].kind);

  free_token_list(tl);
  PASS();
}

TEST test_matches_string_with_splice(void) {
  struct TokenList *tl = tokenize_string("RE\\\nTURN");
  ASSERT(tl);
  ASSERT_EQ(
      TOKEN_IDENTIFIER,
      tl->tokens[0].kind); /* RETURN is keyword only if exact match usually? */
  /* Wait, "return" is keyword. "RE\nTURN" is "RETURN". */
  /* Current identify_keyword simple implementation relies on span_equals_str
     which is strict raw content. So "RE\nTURN" becomes IDENTIFIER, not KEYWORD
     using the optimized identify func unless we normalized in identify_keyword.
     But token_matches_string should handle semantic match. */

  ASSERT(token_matches_string(&tl->tokens[0], "RETURN"));

  free_token_list(tl);
  PASS();
}

SUITE(tokenizer_trigraphs_suite) {
  RUN_TEST(test_trigraph_basic);
  RUN_TEST(test_splice_basic);
  RUN_TEST(test_trigraph_splice_interaction);
  RUN_TEST(test_splice_does_not_create_trigraph);
  RUN_TEST(test_matches_string_with_splice);
}

#endif /* TEST_TOKENIZER_TRIGRAPHS_H */
