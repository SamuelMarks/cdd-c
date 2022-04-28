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
  for (ch = s; *ch; (ch)++)
    if (iscntrl(*ch) || *ch == '\\' || *ch == '\"' || *ch == '\'')
      printf("\\%03o", *ch);
    else
      putchar(ch[0]);
  puts("\"");
}

void print_escaped_span(const char *name, az_span span) {
  size_t i;
  printf("%s", name);
  for (i = 0; i < MIN_NAME - strlen(name); i++)
    putchar(' ');
  printf("= \"");
  for (i = 0; i < az_span_size(span); i++)
    if (iscntrl(az_span_ptr(span)[i]) || az_span_ptr(span)[i] == '\\' ||
        az_span_ptr(span)[i] == '\"' || az_span_ptr(span)[i] == '\'')
      printf("\\%03o", az_span_ptr(span)[i]);
    else
      putchar(az_span_ptr(span)[i]);
  puts("\"");
}

#undef MIN_NAME
