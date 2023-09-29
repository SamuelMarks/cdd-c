#ifndef C_CDD_TESTS_TEST_STRUCT_H
#define C_CDD_TESTS_TEST_STRUCT_H

#include <greatest.h>

#include "cst.h"

static const char one_structs_src[] = "struct Haz {\n"
                                      "  const char *bzr;\n"
                                      "};\n";

static const char two_structs_src[] = "struct Haz {\n"
                                      "  const char *bzr;\n"
                                      "};\n"
                                      "\n"
                                      "struct Foo {\n"
                                      "  const char *bar;\n"
                                      "  int can;\n"
                                      "  struct Haz *haz;\n"
                                      "};\n";

TEST x_test_one_structs_tokenized(void) {
  const az_span one_structs_span =
      az_span_create_from_str((char *)one_structs_src);
  struct tokenizer_az_span_arr tokenized_stack = {NULL, 0};
  struct tokenizer_az_span_arr *tokenized = &tokenized_stack;
  enum { n = 17 };
  size_t i;

  struct tokenizer_az_span_elem *iter;

  struct StrTokenizerKind tokenized_l[n] = {
      {"struct", WORD},  {" ", WHITESPACE},  {"Haz", WORD},
      {" ", WHITESPACE}, {"{", LBRACE},      {"\n  ", WHITESPACE},
      {"const", WORD},   {" ", WHITESPACE},  {"char", WORD},
      {" ", WHITESPACE}, {"*", ASTERISK},    {"bzr", WORD},
      {";", TERMINATOR}, {"\n", WHITESPACE}, {"}", RBRACE},
      {";", TERMINATOR}, {"\n", WHITESPACE}};

  tokenizer(one_structs_span, &tokenized);

  ASSERT_EQ(tokenized->size, n);

  for (iter = tokenized->elem, i = 0; iter != NULL; iter++, i++) {
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

TEST x_test_two_structs_tokenized(void) {
  const az_span two_structs_span =
      az_span_create_from_str((char *)two_structs_src);
  struct tokenizer_az_span_arr tokenized_stack = {NULL, 0};
  struct tokenizer_az_span_arr *tokenized = &tokenized_stack;
  enum { n = 47 };
  size_t i;

  struct tokenizer_az_span_elem *iter;

  struct StrTokenizerKind tokenized_l[n] = {
      {"struct", WORD},     {" ", WHITESPACE},    {"Haz", WORD},
      {" ", WHITESPACE},    {"{", LBRACE},        {"\n  ", WHITESPACE},
      {"const", WORD},      {" ", WHITESPACE},    {"char", WORD},
      {" ", WHITESPACE},    {"*", ASTERISK},      {"bzr", WORD},
      {";", TERMINATOR},    {"\n", WHITESPACE},   {"}", RBRACE},
      {";", TERMINATOR},    {"\n\n", WHITESPACE}, {"struct", WORD},
      {" ", WHITESPACE},    {"Foo", WORD},        {" ", WHITESPACE},
      {"{", LBRACE},        {"\n  ", WHITESPACE}, {"const", WORD},
      {" ", WHITESPACE},    {"char", WORD},       {" ", WHITESPACE},
      {"*", ASTERISK},      {"bar", WORD},        {";", TERMINATOR},
      {"\n  ", WHITESPACE}, {"int", WORD},        {" ", WHITESPACE},
      {"can", WORD},        {";", TERMINATOR},    {"\n  ", WHITESPACE},
      {"struct", WORD},     {" ", WHITESPACE},    {"Haz", WORD},
      {" ", WHITESPACE},    {"*", ASTERISK},      {"haz", WORD},
      {";", TERMINATOR},    {"\n", WHITESPACE},   {"}", RBRACE},
      {";", TERMINATOR},    {"\n", WHITESPACE}};

  tokenizer(two_structs_span, &tokenized);

  ASSERT_EQ(tokenized->size, n);

  for (iter = tokenized->elem, i = 0; iter != NULL; iter++, i++) {
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

SUITE(struct_suite) {
  RUN_TEST(x_test_one_structs_tokenized);
  RUN_TEST(x_test_two_structs_tokenized);
}

#endif /* !C_CDD_TESTS_TEST_STRUCT_H */
