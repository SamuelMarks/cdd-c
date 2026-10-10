#ifndef TEST_DSL_TOKENIZER_EOF_COMMENT_H
#define TEST_DSL_TOKENIZER_EOF_COMMENT_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "../../include/dsl/cdd_dsl_tokenizer.h"
#include "../cdd_test_helpers/cdd_helpers.h"
/* clang-format on */

TEST test_dsl_tokenizer_eof_comment(void) {
  cdd_dsl_token_list_t list;
  cdd_c_error_t rc;
  const char *source = "// EOF comment";

  rc = cdd_dsl_tokenize(source, &list);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");
  ASSERT_EQ_FMT(CDD_DSL_TOKEN_COMMENT_LINE, list.tokens[0].kind, "%d");

  cdd_dsl_token_list_free(&list);
  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_DSL_TOKENIZER_EOF_COMMENT_H */
