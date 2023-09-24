#include "cst_parser_helpers.h"
#include "tokenizer_types.h"
#include <stdio.h>

size_t eatFunction(struct tokenizer_az_span_elem *const *const arr,
                   const size_t start_index, const size_t first_lbrace) {
  struct tokenizer_az_span_elem *const *token;
  size_t i, lbrace = 1, rbrace = 0;
  puts("\n<FUNCTION>");
  for (token = arr, i = first_lbrace; *token != NULL; token++, i++) {
    switch ((**token).kind) {
    case LBRACE:
      ++lbrace;
      goto DEFAULT;
    case RBRACE:
      if (lbrace == ++rbrace) {
        puts("%</FUNCTION>\n");
        return i;
      }
    default:
    DEFAULT:
      (*arr + start_index)->kind = (**token).kind,
              (*arr + start_index)->span = (**token).span;
      break;
    }
    /*assert(false);*/ /* Syntax error */

    /*
    if (az_span_ptr((**token).span) != NULL && (**token).kind != UNKNOWN_SCAN) {
      char *s;
      asprintf(&s, "eatFunction::token->span[%" NUM_LONG_FMT "u]", i);
      print_escaped_span(s, (**token).span);
      free(s);
    } else {
      printf("eatFunction::token->span[%" NUM_LONG_FMT "u]\n", i);
      if (i > 50)
        exit(5);
    }
    */
  }
  puts("$</FUNCTION>\n");
  return first_lbrace + 1;
}
