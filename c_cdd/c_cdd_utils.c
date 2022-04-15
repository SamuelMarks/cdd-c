#include "c_cdd_utils.h"
#include <ctype.h>
#include <stdio.h>

void print_escaped(const char *name, char *s) {
#define MIN_NAME 22
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
#undef MIN_NAME
}
