#include "c_cdd_utils.h"
#include <ctype.h>
#include <stdio.h>

#define MIN_NAME 22

void print_escaped(const char *name, char *s) {
  char *ch;
  size_t i;
  printf("%s", name);
  for (i = 0; i < MIN_NAME - strlen(name); i++)
    putchar(' ');
  printf("= \"");
  if (s == NULL)
    printf("(null)");
  else
    for (ch = s; *ch; (ch)++)
      if (iscntrl(*ch) || *ch == '\\' || *ch == '\"' || *ch == '\'')
        printf("\\%03o", *ch);
      else
        putchar(ch[0]);
  puts("\"");
}

void print_escaped_span(const char *const name, const az_span span) {
  const uint8_t *const span_ptr = az_span_ptr(span);
  size_t i;
  printf("%s", name);
  for (i = 0; i < MIN_NAME - strlen(name); i++)
    putchar(' ');
  printf("= \"");
  for (i = 0; i < az_span_size(span); i++)
    if (iscntrl(span_ptr[i]) || span_ptr[i] == '\\' ||
        span_ptr[i] == '\"' || span_ptr[i] == '\'')
      printf("\\%03o", span_ptr[i]);
    else
      putchar(span_ptr[i]);
  puts("\"");
}

#undef MIN_NAME
