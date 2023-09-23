#ifndef C_CDD_TESTS_TEST_WHITESPACE_H
#define C_CDD_TESTS_TEST_WHITESPACE_H

#include <greatest.h>

#include <c_str_precondition_internal.h>

#include <c_cdd_utils.h>
#include <cst.h>

#include <cdd_helpers.h>

static const char whitespace_src[] = "\n\r\v"
                                     "/* C comment 0 */"
                                     "\n"
                                     "/* C comment*\\/ fin */";

TEST x_test_whitespace_tokenized(void) {
  const az_span whitespace_span =
      az_span_create_from_str((char *)whitespace_src);
  struct tokenizer_az_span_arr *tokenized;
  struct tokenizer_az_span_elem *iter;
  enum { n = 4 };
  size_t i;
  struct StrTokenizerKind tokenized_l[n] = {
      {"\n\r\v", WHITESPACE},
      {"/* C comment 0 */", C_COMMENT},
      {"\n", WHITESPACE},
      {"/* C comment*\\/ fin */", C_COMMENT}};

  tokenizer(whitespace_span, &tokenized);
  ASSERT_EQ(tokenized->size, n);

  for (iter = tokenized->elem, i = 0; iter != NULL; iter++, i++) {
    const size_t n = az_span_size(iter->span) + 1;
    char *iter_s = malloc(n);
    if (iter_s == NULL)
      exit(ENOMEM);
    az_span_to_str(iter_s, n, iter->span);
    ASSERT_STR_EQ(tokenized_l[i].s, iter_s);
    ASSERT_EQ(tokenized_l[i].kind, iter->kind);
    free(iter_s);
  }
  ASSERT_EQ(tokenized->size, i);
  tokenizer_az_span_elem_arr_cleanup(tokenized);
  ASSERT_EQ(tokenized->size, 0);
  ASSERT_EQ(tokenized->elem, NULL);
  PASS();
}

SUITE(whitespace_suite) {
  az_precondition_failed_set_callback(cdd_precondition_failed);
  RUN_TEST(x_test_whitespace_tokenized);
}

#endif /* !C_CDD_TESTS_TEST_WHITESPACE_H */
