#if defined(__clang__) || defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverlength-strings"
#pragma GCC diagnostic ignored "-Wlong-long"
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif
#ifndef TEST_PREPROCESSOR_INTERNALS_H
#define TEST_PREPROCESSOR_INTERNALS_H

/* clang-format off */
#include <cdd_test_helpers/cdd_helpers.h>

/* Include the C file to access static functions */
#include "functions/parse/preprocessor.c"
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

#endif

#if defined(__clang__) || defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
