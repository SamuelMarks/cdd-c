#include "cst_parser_helpers.h"
#include "c_cdd_utils.h"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#include <c89stringutils_string_extras.h>
#define NUM_LONG_FMT "z"
#else
#define NUM_LONG_FMT "l"
#include <stdio.h>
#endif /* defined(WIN32) || defined(_WIN32) || defined(__WIN32__) ||           \
          defined(__NT__) */

size_t eatFunction(struct tokenizer_az_span_element *arr, size_t start_index,
                   size_t first_lbrace, struct tokenizer_az_span_elem ***cur,
                   struct parse_cst_list *ll) {
  struct tokenizer_az_span_element *token;
  size_t i/*, lbrace, rbrace*/;
  puts("<FUNCTION>");
  for (token = arr, i = 0; token != NULL && az_span_ptr(token->span) != NULL;
       token++, i++) {
    char *s;
    asprintf(&s, "eatFunction::token->span[%" NUM_LONG_FMT "u]", i);
    print_escaped_span(s, token->span);
    free(s);
  }
  puts("</FUNCTION>");
  return first_lbrace + 1;
}
