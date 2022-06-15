#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>

#if defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__)
#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif /* !MIN */
#else
#include <sys/param.h>
#endif /* defined(_WIN32) || defined(__WIN32__) || defined(__WINDOWS__) */

#include "c_cdd_utils.h"

#define MIN_NAME 22

void print_escaped(const char *name, char *s) {
  const size_t name_n = strlen(name);
  char *ch;
  size_t i;
  printf("%s", name);
  for (i = 0; i < MIN(MIN_NAME - name_n, name_n); i++)
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
  size_t i;
  printf("%s", name);
  for (i = 0; i < MIN(MIN_NAME - name_n, name_n); i++)
    putchar(' ');
  printf("= \"");
  for (i = 0; i < az_span_size(span); i++)
    if (iscntrl(span_ptr[i]) || span_ptr[i] == '\\' || span_ptr[i] == '\"' ||
        span_ptr[i] == '\'')
      printf("\\%03o", span_ptr[i]);
    else
      putchar(span_ptr[i]);
  puts("\"");
}

void print_escaped_spans(uint8_t *format, ...) {
  va_list ap;
  va_start(ap, format);

#define OUT stdout

  while (*format) {
    switch (*format++) {
    case 'c':
      putc((char)va_arg(ap, int), OUT);
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
      fputs(va_arg(ap, char *), OUT);
      break;
    case '%':
      putc('%', OUT);
      break;
    }
  }
  va_end(ap);
#undef OUT
}

#undef MIN_NAME
