#include <stdio.h>
#include <stdlib.h>

extern int yyparse();
extern FILE *yyin;
extern int yylex();
extern void yyerror(const char *);

int main(int argc, char *argv[]) {
#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) ||                         \
    defined(__STDC_LIB_EXT1__) && __STDC_WANT_LIB_EXT1__
  {
    errno_t err = fopen_s(&yyin, argv[1], "r");
    if (err != 0 || yyin == NULL) {
      fprintf(stderr, "couldn't open file for reading %s\n", argv[1]);
      return EXIT_FAILURE;
    }
  }
#else
  yyin = fopen(argv[1], "r");
  if (!yyin) {
    printf("couldn't open file for reading\n");
    return 1;
  }
#endif
  /*yydebug = 1;*/
  return yyparse();
}
