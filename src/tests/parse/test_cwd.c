/* clang-format off */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
/* clang-format on */
int test_cwd(void) {
  char buf[1024];
  getcwd(buf, sizeof(buf));
  printf("CWD IS: %s\n", buf);
  return 0;
}
