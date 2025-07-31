#include <stdio.h>
#include <stdlib.h>

#include "cdd_helpers.h"

void cdd_precondition_failed(void) { fputs("cdd_precondition_failed", stderr); }

int write_to_file(const char *const filename, const char *const contents) {
  FILE *fh;
  int rc = 0, rc1;
  if (filename == NULL || contents == NULL)
    return EXIT_FAILURE;
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  errno_t err = fopen_s(&fh, filename, "w");
  if (err != 0 || fh == NULL) {
    fprintf(stderr, "Failed to open for writing %s\n", filename);
    return EXIT_FAILURE;
  }
#else
  fh = fopen(filename, "w");
  if (fh == NULL)
    return EXIT_FAILURE;
#endif
  rc = fputs(contents, fh);
  if (rc >= 0)
    rc = EXIT_SUCCESS;
  else
    fprintf(stderr, "Failure to write to %s\n", filename);
  rc1 = fclose(fh);
  return rc == EXIT_SUCCESS ? rc1 : rc;
}

/*
void debug_tokenized_with_mock(const struct tokenizer_az_span_elem *tokenized,
                               size_t *i,
                               const struct StrTokenizerKind *tokenized_l,
                               size_t tokenized_l_n) {
  struct tokenizer_az_span_elem *iter;
  for (iter = (struct tokenizer_az_span_elem *)tokenized->list, *i = 0;
       iter != NULL; iter = iter->next, (*i)++) {
    const size_t n = az_span_size(iter->span) + 1;
    char *iter_s = malloc(n);
    char *s0, *s1;
    if (iter_s == NULL)
      exit(ENOMEM);
    az_span_to_str(iter_s, n, iter->span);
    asprintf(&s0, "tokenized_str_l[%" NUM_LONG_FMT "u]", *i);
    asprintf(&s1, "iter_s[%" NUM_LONG_FMT "u]       ", *i);
    if (*i < tokenized_l_n)
      print_escaped(s0, (char *)tokenized_l[*i].s);
    print_escaped(s1, iter_s);
    putchar('\n');
    // ASSERT_STR_EQ(iter_s, tokenized_str_l[i++]);
    free(s0);
    free(s1);
    free(iter_s);
  }
}
*/
