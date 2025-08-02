#include <greatest.h>

/*#include "test_comment.h"
#include "test_function.h"
#include "test_literal_str.h"
#include "test_macro.h"*/
#include "test_dataclasses.h"
#include "test_json_from_and_to.h"
#include "test_parsing.h"
/*#include "test_struct.h"
#include "test_tokenizer.h"
#include "test_whitespace.h"*/

/* Add definitions that need to be in the test runner's main file. */
GREATEST_MAIN_DEFS();

int main(int argc, char **argv) {
  GREATEST_MAIN_BEGIN();
  /*
  RUN_SUITE(comment_suite);
  RUN_SUITE(macro_suite);
  RUN_SUITE(literal_str_suite);
  RUN_SUITE(whitespace_suite);
  RUN_SUITE(function_suite);
  RUN_SUITE(struct_suite);
  RUN_SUITE(tokenizer_suite);
  */
  RUN_SUITE(parsing_suite);
  RUN_SUITE(json_from_and_to_suite);
  RUN_SUITE(dataclasses_suite);
  GREATEST_MAIN_END();
}
