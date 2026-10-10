#ifndef TEST_DSL_TOKENIZER_BRANCHES_H
#define TEST_DSL_TOKENIZER_BRANCHES_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "../../include/dsl/cdd_dsl_tokenizer.h"
#include "../cdd_test_helpers/cdd_helpers.h"
/* clang-format on */

TEST test_dsl_tokenizer_comments(void) {
  cdd_dsl_token_list_t list;
  cdd_c_error_t rc;
  const char *source = "// line comment\n/* block comment */";

  rc = cdd_dsl_tokenize(source, &list);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");
  ASSERT_EQ_FMT(CDD_DSL_TOKEN_COMMENT_LINE, list.tokens[0].kind, "%d");
  ASSERT_EQ_FMT(CDD_DSL_TOKEN_WHITESPACE, list.tokens[1].kind, "%d");
  ASSERT_EQ_FMT(CDD_DSL_TOKEN_COMMENT_BLOCK, list.tokens[2].kind, "%d");

  cdd_dsl_token_list_free(&list);
  PASS();
}

TEST test_dsl_tokenizer_escapes(void) {
  cdd_dsl_token_list_t list;
  cdd_c_error_t rc;
  const char *source = "#!ws(\" \") #!comment(\"/* */\")";

  rc = cdd_dsl_tokenize(source, &list);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");
  ASSERT_EQ_FMT(CDD_DSL_TOKEN_ESCAPE_WS, list.tokens[0].kind, "%d");
  ASSERT_EQ_FMT(CDD_DSL_TOKEN_STRING, list.tokens[1].kind, "%d");

  cdd_dsl_token_list_free(&list);
  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_DSL_TOKENIZER_BRANCHES_H */
