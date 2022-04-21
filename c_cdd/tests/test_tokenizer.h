#ifndef C_CDD_TESTS_TEST_TOKENIZER_H
#define C_CDD_TESTS_TEST_TOKENIZER_H

#include <greatest.h>

#include "cst.h"

static const char decl_src[] = "unsigned long long int/*foo bar*/a = 5;"
                               "static signed int/*foo bar*/b[] = {5};";

TEST x_test_declaration_tokenizer(void) {
  const struct str_elem *scanned = scanner(decl_src);
  const struct str_elem *tokens = tokenizer((struct str_elem *) scanned);
  PASS();
}

SUITE(tokenizer_suite) {
  RUN_TEST(x_test_declaration_tokenizer);
}

#endif /* !C_CDD_TESTS_TEST_TOKENIZER_H */
