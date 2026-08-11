/* clang-format off */
#include <stdio.h>
#include <stdlib.h>
#ifdef _WIN32
#include <direct.h>
#define getcwd _getcwd
#else
#ifndef _MSC_VER
#ifndef _MSC_VER
#include <unistd.h>
#endif /* _MSC_VER */
#endif /* _MSC_VER */
#endif
/* clang-format on */
int test_cwd(void) {
  char buf[1024];
  getcwd(buf, sizeof(buf));
  printf("CWD IS: %s\n", buf);
  return 0;
}
