#ifndef C_CDD_TESTS_TEST_LITERAL_STR_H
#define C_CDD_TESTS_TEST_LITERAL_STR_H

#include <greatest.h>

#include <c_str_precondition_internal.h>

#include <c_cdd_utils.h>
#include <cst.h>

#include <cdd_helpers.h>

TEST x_test_double_literal_str_tokenized(void) {
  static const char literal_str_src[] = "\"foo\";\n"
                                        "\"bar can\";\n";
  const az_span literal_str_span =
      az_span_create_from_str((char *)literal_str_src);
  struct tokenizer_az_span_list *tokenized;
  struct tokenizer_az_span_elem *iter;
  enum { n = 6 };
  size_t i;
  struct StrTokenizerKind tokenized_l[n] = {
      {"\"foo\"", DOUBLE_QUOTED},     {";", TERMINATOR}, {"\n", WHITESPACE},
      {"\"bar can\"", DOUBLE_QUOTED}, {";", TERMINATOR}, {"\n", WHITESPACE}};
  tokenizer(literal_str_span, &tokenized);

  ASSERT_EQ(tokenized->size, n);

  for (iter = (struct tokenizer_az_span_elem *)tokenized->list, i = 0;
       iter != NULL; iter = iter->next, i++) {
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
  tokenizer_az_span_list_cleanup(tokenized);
  ASSERT_EQ(tokenized->size, 0);
  ASSERT_EQ(tokenized->list, NULL);
  PASS();
}

TEST x_test_single_literal_str_tokenized(void) {
  static const char literal_str_src[] = "'a';\n"
                                        "'\\n';\n"
                                        "'\\'\n";
  const az_span literal_str_span =
      az_span_create_from_str((char *)literal_str_src);
  struct tokenizer_az_span_list *tokenized;
  struct tokenizer_az_span_elem *iter;
  enum { n = 8 };
  size_t i;
  struct StrTokenizerKind tokenized_l[n] = {
      {"'a'", SINGLE_QUOTED},   {";", TERMINATOR}, {"\n", WHITESPACE},
      {"'\\n'", SINGLE_QUOTED}, {";", TERMINATOR}, {"\n", WHITESPACE},
      {"'\\'", SINGLE_QUOTED},  {"\n", WHITESPACE}};
  tokenizer(literal_str_span, &tokenized);

  ASSERT_EQ(tokenized->size, n);

  for (iter = (struct tokenizer_az_span_elem *)tokenized->list, i = 0;
       iter != NULL; iter = iter->next, i++) {
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
  tokenizer_az_span_list_cleanup(tokenized);
  ASSERT_EQ(tokenized->size, 0);
  ASSERT_EQ(tokenized->list, NULL);
  PASS();
}

TEST x_test_literal_str_concat_tokenized(void) {
  static const char literal_str_src[] = "\"catt\"\"catt\"\n"
                                        "\"cut\"\n\"cut\"\n";
  const az_span literal_str_span =
      az_span_create_from_str((char *)literal_str_src);
  struct tokenizer_az_span_list *tokenized;
  struct tokenizer_az_span_elem *iter;
  enum { n = 7 };
  size_t i;

  struct StrTokenizerKind tokenized_l[n] = {
      {"\"catt\"", DOUBLE_QUOTED}, {"\"catt\"", DOUBLE_QUOTED},
      {"\n", WHITESPACE},          {"\"cut\"", DOUBLE_QUOTED},
      {"\n", WHITESPACE},          {"\"cut\"", DOUBLE_QUOTED},
      {"\n", WHITESPACE}};
  tokenizer(literal_str_span, &tokenized);

  ASSERT_EQ(tokenized->size, n);

  for (iter = (struct tokenizer_az_span_elem *)tokenized->list, i = 0;
       iter != NULL; iter = iter->next, i++) {
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
  tokenizer_az_span_list_cleanup(tokenized);
  ASSERT_EQ(tokenized->size, 0);
  ASSERT_EQ(tokenized->list, NULL);
  PASS();
}

TEST x_test_literal_str_tokenized(void) {
  static const char literal_str_src[70] = "\"foo\";\n"
                                          "'a';\n"
                                          "'\\n';\n"
                                          "'\\'\n"
                                          "\"bar can\";\n"
                                          "\"cat\" \"cat\"\n"
                                          "\"catt\"\"catt\"\n"
                                          "\"cut\"\n\"cut\"\n";
  const az_span literal_str_span =
      az_span_create_from_str((char *)literal_str_src);
  struct tokenizer_az_span_list *tokenized;
  struct tokenizer_az_span_elem *iter;
  enum { n = 25 };
  size_t i;

  struct StrTokenizerKind tokenized_l[n] = {
      {"\"foo\"", DOUBLE_QUOTED},  {";", TERMINATOR},
      {"\n", WHITESPACE},          {"'a'", SINGLE_QUOTED},
      {";", TERMINATOR},           {"\n", WHITESPACE},
      {"'\\n'", SINGLE_QUOTED},    {";", TERMINATOR},
      {"\n", WHITESPACE},          {"'\\'", SINGLE_QUOTED},
      {"\n", WHITESPACE},          {"\"bar can\"", DOUBLE_QUOTED},
      {";", TERMINATOR},           {"\n", WHITESPACE},
      {"\"cat\"", DOUBLE_QUOTED},  {" ", WHITESPACE},
      {"\"cat\"", DOUBLE_QUOTED},  {"\n", WHITESPACE},
      {"\"catt\"", DOUBLE_QUOTED}, {"\"catt\"", DOUBLE_QUOTED},
      {"\n", WHITESPACE},          {"\"cut\"", DOUBLE_QUOTED},
      {"\n", WHITESPACE},          {"\"cut\"", DOUBLE_QUOTED},
      {"\n", WHITESPACE},
  };
  tokenizer(literal_str_span, &tokenized);

  ASSERT_EQ(tokenized->size, n);

  for (iter = (struct tokenizer_az_span_elem *)tokenized->list, i = 0;
       iter != NULL; iter = iter->next, i++) {
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
  tokenizer_az_span_list_cleanup(tokenized);
  ASSERT_EQ(tokenized->size, 0);
  ASSERT_EQ(tokenized->list, NULL);
  PASS();
}

SUITE(literal_str_suite) {
  az_precondition_failed_set_callback(cdd_precondition_failed);
  RUN_TEST(x_test_double_literal_str_tokenized);
  RUN_TEST(x_test_single_literal_str_tokenized);
  RUN_TEST(x_test_literal_str_tokenized);
  RUN_TEST(x_test_literal_str_concat_tokenized);
}

#endif /* !C_CDD_TESTS_TEST_LITERAL_STR_H */
