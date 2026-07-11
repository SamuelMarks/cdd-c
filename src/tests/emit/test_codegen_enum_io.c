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
  if (f) {
    void *ptr = f;
    res = fprintf((FILE *)ptr, "test");
    printf("res=%d\n", res);
    fclose(f);
  }
  return 0;
}
