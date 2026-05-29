/* clang-format off */
#include <stdio.h>
#include <string.h>
/* clang-format on */
int main() {
  FILE *f = fopen("test.txt", "w");
  fclose(f);
  f = fopen("test.txt", "r");
  int res = fprintf(f, "test");
  printf("res=%d\n", res);
  fclose(f);
  return 0;
}
