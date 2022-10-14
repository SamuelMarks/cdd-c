#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>

#include "c_cdd_utils.h"

#define MIN_NAME 34

void print_escaped(const char *const name, char *const s) {
  const size_t name_n = strlen(name);
  char *ch;
  size_t i;
  printf("%s", name);
  for (i = 0; i < (name_n > MIN_NAME ? 0 : MIN_NAME - name_n); i++)
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
  const size_t name_n = strlen(name);
  const size_t span_n = az_span_size(span);
  size_t i;
  printf("%s", name);
  for (i = 0; i < (name_n > MIN_NAME ? 0 : MIN_NAME - name_n); i++)
    putchar(' ');
  if (span_ptr == NULL || span_n == 0 || span_n > INT32_MAX) {
    puts("= (null)");
    return;
  }
  printf("= \"");
  for (i = 0; i < span_n; i++)
    if (iscntrl(span_ptr[i]) || span_ptr[i] == '\\' || span_ptr[i] == '\"' ||
        span_ptr[i] == '\'')
      printf("\\%03o", span_ptr[i]);
    else
      putchar(span_ptr[i]);
  puts("\"");
}

void print_escaped_spans(uint8_t *format, ...) {
  /* Quotes strings chars and indents first string param
   * + appends " = " to it; supports `az_span`s with "%Q"
   * doesn't support "%03d" or other fancier syntax
   * Strings, chars, and spans are all escaped. */
  bool first = true;
  size_t i;
  va_list ap;
  va_start(ap, format);

#define OUT stdout

  while (*format) {
    switch (*format++) {
    case 'c':
      fprintf(OUT, "\'%c\'", (char)va_arg(ap, int));
      break;
    case 'd':
    case 'i':
      fprintf(OUT, "%d", va_arg(ap, int));
      break;
    case 'o':
      fprintf(OUT, "%o", va_arg(ap, int));
      break;
    case 'u':
      fprintf(OUT, "%u", va_arg(ap, unsigned));
      break;
    case 'x':
      fprintf(OUT, "%x", va_arg(ap, unsigned));
      break;
    case 'X':
      fprintf(OUT, "%X", va_arg(ap, unsigned));
      break;
    case 'e':
      fprintf(OUT, "%e", va_arg(ap, double));
      break;
    case 'E':
      fprintf(OUT, "%E", va_arg(ap, double));
      break;
    case 'f':
      fprintf(OUT, "%f", va_arg(ap, double));
      break;
    case 'F':
      fprintf(OUT, "%F", va_arg(ap, double));
      break;
    case 'g':
      fprintf(OUT, "%g", va_arg(ap, double));
      break;
    case 'G':
      fprintf(OUT, "%G", va_arg(ap, double));
      break;
    case 'a':
      fprintf(OUT, "%a", va_arg(ap, double));
      break;
    case 'A':
      fprintf(OUT, "%A", va_arg(ap, double));
      break;
    case 'n':
      fprintf(OUT, "%n", va_arg(ap, int *));
      break;
    case 'p':
      fprintf(OUT, "%p", va_arg(ap, void *));
      break;
    case 's':
    case 'S':
    case 'Z':
      if (first) {
        const char *const s = va_arg(ap, char *);
        const size_t n = strlen(s);
        first = false;
        fprintf(OUT, "%s", s);
        for (i = 0; i < (n > MIN_NAME ? 0 : MIN_NAME - n); i++)
          putc(' ', OUT);
        fprintf(OUT, "= ");
      } else
        fprintf(OUT, "\"%s\"", va_arg(ap, char *));
      break;
    case '%':
      putc('%', OUT);
      break;
    case 'Q': {
      /* Custom for az_span */

      const az_span span = va_arg(ap, az_span);
      const uint8_t *const span_ptr = az_span_ptr(span);

      fputc('"', OUT);
      for (i = 0; i < az_span_size(span); i++)
        if (iscntrl(span_ptr[i]) || span_ptr[i] == '\\' ||
            span_ptr[i] == '\"' || span_ptr[i] == '\'')
          fprintf(OUT, "\\%03o", span_ptr[i]);
        else
          fputc(span_ptr[i], OUT);
      fputc('"', OUT);
      break;
    }
      va_end(ap);
#undef OUT
    }
  }
}

#undef MIN_NAME
