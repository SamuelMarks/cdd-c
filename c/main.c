#include <stdio.h>

extern int yyparse();
extern FILE *yyin;
extern int yylex();
extern void yyerror(const char *);

int main(int argc, char *argv[]) {
  yyin = fopen(argv[1], "r");
  if (!yyin) {
    printf("couldn't open file for reading\n");
    return 1;
  }
  /*yydebug = 1;*/
  return yyparse();
}
