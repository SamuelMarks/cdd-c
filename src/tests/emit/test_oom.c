/* clang-format off */
#include "c_cdd/memory.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
/* clang-format on */
enum cdd_c_error main(void) {
  struct rlimit rl;
  if (getrlimit(RLIMIT_AS, &rl) != 0) {
    perror("getrlimit");
    return 1;
  }
  rl.rlim_cur = 1024 * 1024 * 10; /* 10 MB */
  if (setrlimit(RLIMIT_AS, &rl) == 0) {
    void *p = C_CDD_MALLOC(1024 * 1024 * 20);
    if (!p) {
      printf("OOM worked\n");
    } else {
      printf("OOM failed\n");
      C_CDD_FREE(p);
    }
  } else {
    perror("setrlimit");
  }
  return 0;
}
