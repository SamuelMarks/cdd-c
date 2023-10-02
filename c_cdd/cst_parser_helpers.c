#include "cst_parser_helpers.h"
#include "tokenizer_types.h"
#include <stdio.h>

size_t eatFunction(const struct tokenizer_az_span_arr *const tokens_arr,
                   const size_t start_index, const size_t first_lbrace) {
  struct tokenizer_az_span_elem *token;
  size_t i, lbrace = 1, rbrace = 0;
  puts("\n<FUNCTION>");
  for (token = tokens_arr->elem, i = first_lbrace;
       az_span_ptr(token->span) != NULL; token++, i++) {
    switch (token->kind) {
    case LBRACE:
      ++lbrace;
      goto DEFAULT;
    case RBRACE:
      if (lbrace == ++rbrace) {
        puts("%</FUNCTION>\n");
        return i;
      }
    default:
    DEFAULT : {
      struct tokenizer_az_span_elem *const tok = tokens_arr->elem + start_index;
      tok->kind = token->kind, tok->span = token->span;
    }
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
