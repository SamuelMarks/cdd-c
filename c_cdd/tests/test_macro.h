#ifndef C_CDD_TESTS_TEST_MACRO_H
#define C_CDD_TESTS_TEST_MACRO_H

#include <greatest.h>

#include <c_str_precondition_internal.h>

#include <c_cdd_utils.h>
#include <cst.h>

#include <cdd_helpers.h>

static const char macro_src[] = "# define foo bar\n"
                                "#ifdef FOO\n"
                                "# define CAT(bar,foo)(bar ## foo)\n"
                                "#define HAZ\\\nFOO\n";

x_test_macro_tokenized(void) {
  const az_span macro_span = az_span_create_from_str((char *)macro_src);
  struct tokenizer_az_span_list *const tokenized = tokenizer(macro_span);
  struct tokenizer_az_span_elem *iter;
  enum { n = 4 };
  size_t i;
  struct StrTokenizerKind tokenized_l[n] = {
      {"# define foo bar\n", MACRO},
      {"#ifdef FOO\n", MACRO},
      {"# define CAT(bar,foo)(bar ## foo)\n", MACRO},
      {"#define HAZ\\\nFOO\n", MACRO}};

  ASSERT_GT(i, 0);

  ASSERT_EQ(tokenized->size, n);

  for (iter = (struct tokenizer_az_span_elem *)tokenized->list, i = 0;
       iter != NULL; iter = iter->next, i++) {
    const size_t n = az_span_size(iter->span) + 1;
    char *iter_s = malloc(n);
    az_span_to_str(iter_s, n, iter->span);
    ASSERT_STR_EQ(tokenized_l[i].s, iter_s);
    ASSERT_EQ(tokenized_l[i].kind, iter->kind);
    free(iter_s);
  }
  ASSERT_EQ(tokenized->size, i);
  tokenizer_az_span_list_cleanup(tokenized);
  ASSERT_EQ(tokenized->size, 0);
  ASSERT_EQ(tokenized->list, NULL);
  PASS();
}

SUITE(macro_suite) {
  az_precondition_failed_set_callback(cdd_precondition_failed);
  RUN_TEST(x_test_macro_tokenized);
}

#endif /* !C_CDD_TESTS_TEST_MACRO_H */
