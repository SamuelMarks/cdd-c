#include <stdio.h>

#include "ll.h"

#include "c_cdd_utils.h"
#include "cdd_helpers.h"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#define NUM_LONG_FMT "z"
#else
#define NUM_LONG_FMT "l"
#endif

void cdd_precondition_failed(void) { fputs("cdd_precondition_failed", stderr); }

void debug_tokenized_with_mock(const struct tokenizer_az_span_list *tokenized,
                               size_t *i,
                               const struct StrTokenizerKind *tokenized_l,
                               size_t tokenized_l_n) {
  struct tokenizer_az_span_elem *iter;
  for (iter = (struct tokenizer_az_span_elem *)tokenized->list, *i = 0;
       iter != NULL; iter = iter->next, (*i)++) {
    const size_t n = az_span_size(iter->span) + 1;
    char *iter_s = malloc(n);
    char *s0, *s1;
    az_span_to_str(iter_s, n, iter->span);
    asprintf(&s0, "tokenized_str_l[%" NUM_LONG_FMT "u]", *i);
    asprintf(&s1, "iter_s[%" NUM_LONG_FMT "u]       ", *i);
    if (*i < tokenized_l_n)
      print_escaped(s0, (char *)tokenized_l[*i].s);
    print_escaped(s1, iter_s);
    putchar('\n');
    /*ASSERT_STR_EQ(iter_s, tokenized_str_l[i++]);*/
    free(s0);
    free(s1);
    free(iter_s);
  }
}
