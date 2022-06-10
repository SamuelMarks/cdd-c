#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS 1
#endif /* _MSC_VER */

#include <greatest.h>

#include "test_comment.h"
#include "test_function.h"
#include "test_literal_str.h"
#include "test_ll.h"
#include "test_macro.h"
#include "test_struct.h"
#include "test_tokenizer.h"

/* Add definitions that need to be in the test runner's main file. */
GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  RUN_SUITE(ll_suite);
  RUN_SUITE(comment_suite);
  RUN_SUITE(macro_suite);
  RUN_SUITE(literal_str_suite);
  /*RUN_SUITE(function_suite);*/
  /*RUN_SUITE(struct_suite);*/
  /*RUN_SUITE(tokenizer_suite);*/
  GREATEST_MAIN_END();
}
