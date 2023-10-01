#ifndef C_CDD_TESTS_TEST_COMMENT_H
#define C_CDD_TESTS_TEST_COMMENT_H

#include <greatest.h>

#include <c_str_precondition_internal.h>

#include <c_cdd_utils.h>
#include <cst.h>

#include <cdd_helpers.h>

static const char comment_src[] = "// C++ comment\n"
                                  "/* C comment 0 */"
                                  "/* C comment 1 */"
                                  "/* C comment*\\/ fin */";

TEST x_test_comment_tokenized(void) {
  const az_span comment_span = az_span_create_from_str((char *)comment_src);
  struct tokenizer_az_span_arr tokenized_stack = {NULL, 0};
  struct tokenizer_az_span_arr *tokenized = &tokenized_stack;
  struct tokenizer_az_span_elem *iter;
  enum { n = 4 };
  size_t i;
  struct StrTokenizerKind tokenized_l[n] = {
      {"// C++ comment\n", CPP_COMMENT},
      {"/* C comment 0 */", C_COMMENT},
      {"/* C comment 1 */", C_COMMENT},
      {"/* C comment*\\/ fin */", C_COMMENT}};
  tokenizer(comment_span, &tokenized);

  ASSERT_EQ(tokenized->size, n);

  for (iter = tokenized->elem, i = 0;
       iter != NULL && az_span_ptr(iter->span) != NULL; iter++, i++) {
    const size_t n = az_span_size(iter->span) + 1;
    char *iter_s = malloc(n);
    if (iter_s == NULL)
      exit(ENOMEM);
    az_span_to_str(iter_s, n, iter->span);
    ASSERT_STR_EQ(tokenized_l[i].s, iter_s);
    ASSERT_STR_EQ(TokenizerKind_to_str(tokenized_l[i].kind),
                  TokenizerKind_to_str(iter->kind));
    ASSERT_EQ(tokenized_l[i].kind, iter->kind);
    free(iter_s);
  }
  ASSERT_EQ(tokenized->size, i);
  tokenizer_az_span_elem_arr_cleanup(tokenized);
  ASSERT_EQ(tokenized->size, 0);
  ASSERT_EQ(tokenized->elem, NULL);
  PASS();
}

SUITE(comment_suite) {
  az_precondition_failed_set_callback(cdd_precondition_failed);
  RUN_TEST(x_test_comment_tokenized);
}

#endif /* !C_CDD_TESTS_TEST_COMMENT_H */
