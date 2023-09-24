#ifndef C_CDD_TESTS_TEST_TOKENIZER_H
#define C_CDD_TESTS_TEST_TOKENIZER_H

#include <greatest.h>

#include "cst.h"

static const char decl_src[] = "unsigned long long int/*foo bar*/a = 5;"
                               "static signed int/*foo bar*/b[] = {5};";

TEST x_test_declaration_tokenizer(void) {
  const az_span decl_span = az_span_create_from_str((char *)decl_src);
  struct tokenizer_az_span_arr tokenized_stack = {NULL, 0};
  struct tokenizer_az_span_arr *tokenized = &tokenized_stack;
  tokenizer(decl_span, &tokenized);
  {
    struct cst_node_arr *tokens;
    cst_parser(tokenized, &tokens);
  }
  PASS();
}

SUITE(tokenizer_suite) { RUN_TEST(x_test_declaration_tokenizer); }

#endif /* !C_CDD_TESTS_TEST_TOKENIZER_H */
