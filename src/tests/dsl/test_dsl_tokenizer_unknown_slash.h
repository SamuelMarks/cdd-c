#ifndef TEST_DSL_TOKENIZER_UNKNOWN_SLASH_H
#define TEST_DSL_TOKENIZER_UNKNOWN_SLASH_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include "../../include/dsl/cdd_dsl_tokenizer.h"
#include "../cdd_test_helpers/cdd_helpers.h"
/* clang-format on */

TEST test_dsl_tokenizer_unknown_slash(void) {
  cdd_dsl_token_list_t list;
  cdd_c_error_t rc;
  const char *source = "/ /";

  rc = cdd_dsl_tokenize(source, &list);
  ASSERT_EQ_FMT(CDD_C_SUCCESS, rc, "%d");
  ASSERT_EQ_FMT(CDD_DSL_TOKEN_UNKNOWN, list.tokens[0].kind, "%d");
  ASSERT_EQ_FMT(CDD_DSL_TOKEN_UNKNOWN, list.tokens[2].kind,
                "%d"); /* Second / is index 2, due to WS */

  cdd_dsl_token_list_free(&list);
  PASS();
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_DSL_TOKENIZER_UNKNOWN_SLASH_H */
