#include "cst_parser_helpers.h"
#include <stdio.h>

size_t eatFunction(struct tokenizer_az_span_element **arr, size_t start_index,
                   size_t first_lbrace,
                   struct tokenizer_az_span_elem ***tokenized_cur_ptr,
                   struct parse_cst_list *ll) {
  struct tokenizer_az_span_element **token;
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
      tokenizer_az_span_list_push(&ll->size, tokenized_cur_ptr, (**token).kind,
                                  (**token).span);
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
