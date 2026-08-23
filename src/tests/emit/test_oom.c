#if defined(__clang__) || defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverlength-strings"
#pragma GCC diagnostic ignored "-Wlong-long"
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
#pragma GCC diagnostic ignored "-Wunused-variable"
#endif
/* clang-format off */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
/* clang-format on */
cdd_c_error_t main(void) {
  struct rlimit rl;
  if (getrlimit(RLIMIT_AS, &rl) != 0) {
    perror("getrlimit");
    return 1;
  }
  rl.rlim_cur = 1024 * 1024 * 10; /* 10 MB */
  if (setrlimit(RLIMIT_AS, &rl) == 0) {
    void *p = malloc(1024 * 1024 * 20);
    if (!p) {
      printf("OOM worked\n");
    } else {
      printf("OOM failed\n");
      free(p);
    }
  } else {
    perror("setrlimit");
  }
  return 0;
}

#if defined(__clang__) || defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
