/* clang-format off */
#include <stdio.h>
#include <string.h>
/* clang-format on */
cdd_c_error_t main(void) {
  FILE *f;
  int res;
#if defined(_MSC_VER)
  if (fopen_s(&f, "test.txt", "w") != 0)
    f = NULL;
#else
  f = fopen("test.txt", "w");
#endif
  if (f)
    fclose(f);
#if defined(_MSC_VER)
  if (fopen_s(&f, "test.txt", "r") != 0)
    f = NULL;
#else
  f = fopen("test.txt", "r");
#endif
  if (f) {
    void *ptr = f;
    res = fprintf((FILE *)ptr, "test");
    printf("res=%d\n", res);
    fclose(f);
  }
  return 0;
}
