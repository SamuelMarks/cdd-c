#ifndef C_CDD_TESTS_TEST_TOKENIZER_H
#define C_CDD_TESTS_TEST_TOKENIZER_H

#include <greatest.h>

#include "cst.h"

static const char decl_src[] = "unsigned long long int/*foo bar*/a = 5;"
                               "static signed int/*foo bar*/b[] = {5};";

TEST x_test_declaration_tokenizer(void) {
  const az_span decl_span = az_span_create_from_str((char *)decl_src);
  struct tokenizer_az_span_list *const tokenized = tokenizer(decl_span);
  const struct parse_cst_list *tokens =
      cst_parser((struct tokenizer_az_span_list *)tokenized->list);
  PASS();
}

SUITE(tokenizer_suite) { RUN_TEST(x_test_declaration_tokenizer); }

#endif /* !C_CDD_TESTS_TEST_TOKENIZER_H */
