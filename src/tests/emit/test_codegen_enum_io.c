/* clang-format off */
#include <stdio.h>
#include <string.h>
/* clang-format on */
enum cdd_c_error main(void) {
  FILE *f;
  int res;
  f = fopen("test.txt", "w");
  if (f)
    fclose(f);
  f = fopen("test.txt", "r");
  /* cppcheck-suppress writeReadOnlyFile */
  res = fprintf(f, "test");
  printf("res=%d\n", res);
  if (f)
    fclose(f);
  return 0;
}
