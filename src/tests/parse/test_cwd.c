/* clang-format off */#include "c_cdd/safe_crt_msvc.h"

#include <stdio.h>
#include <stdlib.h>
#ifdef _WIN32
#include <direct.h>
#define getcwd _getcwd
#else
#include <unistd.h>
#endif
/* clang-format on */
int test_cwd(void) {
  char buf[1024];
  getcwd(buf, sizeof(buf));
  printf("CWD IS: %s\n", buf);
  return 0;
}
