#include "src/classes/parse/cdd_token.h"
#include <stdio.h>
int main() {
  printf("LPAREN=%d RPAREN=%d LBRACKET=%d RBRACKET=%d OTHER=%d\n",
         CDD_TOKEN_LPAREN, CDD_TOKEN_RPAREN, CDD_TOKEN_LBRACKET,
         CDD_TOKEN_RBRACKET, CDD_TOKEN_OTHER);
}
