#include "cst_parser_helpers.h"
#include "c_cdd_utils.h"
#include "cst_parser_types.h"
#include "str_includes.h"
#include "tokenizer_types.h"
#include <stdio.h>

size_t eatFunction(const struct tokenizer_az_span_arr *const tokens_arr,
                   const size_t start_index, const size_t first_lbrace,
                   const struct cst_node_arr *const *const *cst_arr) {
  struct tokenizer_az_span_elem *token;
  size_t i, lbrace = 1, rbrace = 0;
  struct Function *const function = malloc(sizeof *function);
  puts("\n<FUNCTION>");
  for (token = tokens_arr->elem, i = first_lbrace;
       az_span_ptr(token->span) != NULL; token++, i++) {
    {
      char *s;
      asprintf(&s, "eatFunction [%02lu]: %s", i,
               TokenizerKind_to_str(token->kind));
      print_escaped_span(s, token->span);
      free(s);
    }
    switch (token->kind) {
    case LBRACE:
      ++lbrace;
      goto DEFAULT;
    case RBRACE:
      if (lbrace == ++rbrace) {
        puts("%</FUNCTION>\n");
        return i;
      }
    case LPAREN:
      /* first non-whitespace non-ASTERISK WORD is the name */
      {
        size_t j;
        char *s;

        for (j = 0; j > i && (token - ++j)->kind == WORD;) {
        }

        function->name = (token - j)->span;
        function->pos_start = start_index;

        asprintf(&s, "previous eatFunction [%02lu]: %s", j,
                 TokenizerKind_to_str((token - j)->kind));
        print_escaped_span(s, (token - j)->span);
        free(s);
      }
    default:
    DEFAULT : {
      struct tokenizer_az_span_elem *const tok = tokens_arr->elem + start_index;
      tok->kind = token->kind, tok->span = token->span;
    }
    }
    /*assert(false);*/ /* Syntax error */

    /*
    if (az_span_ptr((*token)->span) != NULL && (*token)->kind != UNKNOWN_SCAN) {
      char *s;
      asprintf(&s, "eatFunction::token->span[%" NUM_LONG_FMT "u]", i);
      print_escaped_span(s, (*token)->span);
      free(s);
    } else {
      printf("eatFunction::token->span[%" NUM_LONG_FMT "u]\n", i);
      if (i > 50)
        exit(5);
    }
    */
  }

  (**cst_arr)->elem[(**cst_arr)->size].kind = Function;
  (**cst_arr)->elem[(**cst_arr)->size].type = function;
  /* cst_arr->elem[cst_arr->size].type = (void*)function; */

  puts("</FUNCTION>\n");
  return i + 1;
}
