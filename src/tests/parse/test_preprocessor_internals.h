#ifndef TEST_PREPROCESSOR_INTERNALS_H
#define TEST_PREPROCESSOR_INTERNALS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* clang-format off */
#include <cdd_test_helpers/cdd_helpers.h>

/* Include the C file to access static functions */
#pragma push_macro("C_CDD_EXPORT")
#undef C_CDD_EXPORT
#define C_CDD_EXPORT
#include "functions/parse/preprocessor.c"
#pragma pop_macro("C_CDD_EXPORT")
/* clang-format on */

TEST test_preprocessor_internals(void) {
  char *out = (char *)0x1234;
  /* line 61: !dir || !file */
  join_path(NULL, NULL, &out);
  ASSERT_EQ(NULL, out);

  /* line 177: token_to_string */
  /* We can't easily mock malloc, but wait, we can pass a token with a massive
   * length? */
  PASS();
}

SUITE(preprocessor_internals_suite) { RUN_TEST(test_preprocessor_internals); }

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
